/*
 *  Copyright 2009-2010 NVIDIA Corporation
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */
 
#include "gpu/GLContext.hpp"
#include "gpu/Buffer.hpp"
#include "gui/Window.hpp"
#include "gui/Image.hpp"

using namespace FW;

//------------------------------------------------------------------------

#define FW_MIN_TEMP_TEXTURES        16
#define FW_MAX_TEMP_TEXTURE_BYTES   (64 << 20)

//------------------------------------------------------------------------

const char* const       GLContext::s_defaultFontName    = "Arial";
const S32               GLContext::s_defaultFontSize    = 16;
const U32               GLContext::s_defaultFontStyle   = FontStyle_Bold;

bool                    GLContext::s_inited             = false;
HWND                    GLContext::s_shareHWND          = NULL;
HDC                     GLContext::s_shareHDC           = NULL;
HGLRC                   GLContext::s_shareHGLRC         = NULL;
GLContext*              GLContext::s_headless           = NULL;
GLContext*              GLContext::s_current            = NULL;
bool                    GLContext::s_stereoAvailable    = false;

GLContext::TempTexture  GLContext::s_tempTextures       = { &s_tempTextures, &s_tempTextures, 0, 0 };
Hash<Vec2i, GLContext::TempTexture*>* GLContext::s_tempTexHash = NULL;
S32                     GLContext::s_tempTexBytes       = 0;
Hash<String, GLContext::Program*>* GLContext::s_programs = NULL;

//------------------------------------------------------------------------

GLContext::Program::Program(const String& vertexSource, const String& fragmentSource)
{
    init(vertexSource, 0, 0, 0, "", fragmentSource);
}

//------------------------------------------------------------------------

GLContext::Program::Program(
    const String& vertexSource,
    GLenum geomInputType, GLenum geomOutputType, int geomVerticesOut, const String& geometrySource,
    const String& fragmentSource)
{
    init(vertexSource, geomInputType, geomOutputType, geomVerticesOut, geometrySource, fragmentSource);
}

//------------------------------------------------------------------------

GLContext::Program::~Program(void)
{
    glDeleteProgram(m_glProgram);
    glDeleteShader(m_glVertexShader);
    glDeleteShader(m_glGeometryShader);
    glDeleteShader(m_glFragmentShader);
}

//------------------------------------------------------------------------

GLint GLContext::Program::getAttribLoc(const String& name) const
{
    return glGetAttribLocation(m_glProgram, name.getPtr());
}

//------------------------------------------------------------------------

GLint GLContext::Program::getUniformLoc(const String& name) const
{
    return glGetUniformLocation(m_glProgram, name.getPtr());
}

//------------------------------------------------------------------------

void GLContext::Program::use(void)
{
    glUseProgram(m_glProgram);
}

//------------------------------------------------------------------------

void GLContext::Program::init(
    const String& vertexSource,
    GLenum geomInputType, GLenum geomOutputType, int geomVerticesOut, const String& geometrySource,
    const String& fragmentSource)
{
    GLContext::staticInit();
    m_glProgram = glCreateProgram();

    // Setup vertex shader.

    m_glVertexShader = createShader(GL_VERTEX_SHADER, vertexSource);
    glAttachShader(m_glProgram, m_glVertexShader);

    // Setup geometry shader (GL_ARB_geometry_shader4).

    if (geometrySource.getLength() == 0)
        m_glGeometryShader = 0;
    else
    {
        m_glGeometryShader = createShader(GL_GEOMETRY_SHADER_ARB, geometrySource);
        glAttachShader(m_glProgram, m_glGeometryShader);

        if (!GL_FUNC_AVAILABLE(glProgramParameteriARB))
            fail("glProgramParameteriARB() not available!");
        glProgramParameteriARB(m_glProgram, GL_GEOMETRY_INPUT_TYPE_ARB, geomInputType);
        glProgramParameteriARB(m_glProgram, GL_GEOMETRY_OUTPUT_TYPE_ARB, geomOutputType);
        glProgramParameteriARB(m_glProgram, GL_GEOMETRY_VERTICES_OUT_ARB, geomVerticesOut);
    }

    // Setup fragment shader.

    m_glFragmentShader = createShader(GL_FRAGMENT_SHADER, fragmentSource);
    glAttachShader(m_glProgram, m_glFragmentShader);

    // Link and display errors.

    glLinkProgram(m_glProgram);
    GLint status = 0;
    glGetProgramiv(m_glProgram, GL_LINK_STATUS, &status);
    if (!status)
    {
        GLint infoLen = 0;
        glGetProgramiv(m_glProgram, GL_INFO_LOG_LENGTH, &infoLen);
        if (!infoLen)
            fail("glLinkProgram() failed!");

        Array<char> info(NULL, infoLen);
        info[0] = '\0';
        glGetProgramInfoLog(m_glProgram, infoLen, &infoLen, info.getPtr());
        fail("glLinkProgram() failed!\n\n%s", info.getPtr());
    }
    GLContext::checkErrors();
}

//------------------------------------------------------------------------

GLuint GLContext::Program::createShader(GLenum type, const String& source)
{
    GLuint shader = glCreateShader(type);
    const char* sourcePtr = source.getPtr();
    int sourceLen = source.getLength();
    glShaderSource(shader, 1, &sourcePtr, &sourceLen);
    glCompileShader(shader);

    GLint status = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (!status)
    {
        const char* typeStr;
        switch (type)
        {
        case GL_VERTEX_SHADER:          typeStr = "GL_VERTEX_SHADER"; break;
        case GL_GEOMETRY_SHADER_ARB:    typeStr = "GL_GEOMETRY_SHADER_ARB"; break;
        case GL_FRAGMENT_SHADER:        typeStr = "GL_FRAGMENT_SHADER"; break;
        default:                        typeStr = "???"; break;
        }

        GLint infoLen = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
        if (!infoLen)
            fail("glCompileShader(%s) failed!", typeStr);

        Array<char> info(NULL, infoLen);
        info[0] = '\0';
        glGetShaderInfoLog(shader, infoLen, &infoLen, info.getPtr());
        fail("glCompileShader(%s) failed!\n\n%s", typeStr, info.getPtr());
    }

    GLContext::checkErrors();
    return shader;
}

//------------------------------------------------------------------------

GLContext::GLContext(HDC hdc, HGLRC hglrc)
{
    FW_ASSERT(hdc && hglrc);
    staticInit();
    init(hdc, hglrc);
}

//------------------------------------------------------------------------

GLContext::GLContext(HDC hdc, const Config& config)
{
    FW_ASSERT(hdc);
    staticInit();
    m_config = config;

    // Choose pixel format.

    int formatIdx;
    if (!choosePixelFormat(formatIdx, hdc, config))
        fail("No appropriate pixel format found!");

    // Apply pixel format.

    PIXELFORMATDESCRIPTOR pfd;
    if (DescribePixelFormat(hdc, formatIdx, sizeof(pfd), &pfd) == 0)
        failWin32Error("DescribePixelFormat");
    if (!SetPixelFormat(hdc, formatIdx, &pfd))
        failWin32Error("SetPixelFormat");

    // Create WGL context.

    HGLRC hglrc = wglCreateContext(hdc);
    if (!hglrc)
        fail("wglCreateContext() failed!");
    if (!wglShareLists(s_shareHGLRC, hglrc))
        fail("wglShareLists() failed!");

    // Initialize.

    init(hdc, hglrc);
}

//------------------------------------------------------------------------

GLContext::~GLContext(void)
{
    DeleteObject(m_memdc);
    DeleteObject(m_vgFont);

    if (s_current != s_headless)
    {
        if (s_current == this)
            s_headless->makeCurrent();
        wglDeleteContext(m_hglrc);
    }
}

//------------------------------------------------------------------------

void GLContext::makeCurrent(void)
{
    if (s_current != this)
    {
        checkErrors();

        if (!wglMakeCurrent(m_hdc, m_hglrc))
            failWin32Error("wglMakeCurrent");
        s_current = this;

        checkErrors();
    }
}

//------------------------------------------------------------------------

void GLContext::swapBuffers(void)
{
    checkErrors();
    if (GL_FUNC_AVAILABLE(wglSwapIntervalEXT))
        wglSwapIntervalEXT(0); // WGL_EXT_swap_control
    SwapBuffers(m_hdc);
}

//------------------------------------------------------------------------

void GLContext::setView(const Vec2i& pos, const Vec2i& size)
{
    FW_ASSERT(size.x > 0 && size.y > 0);
    glViewport(pos.x, pos.y, size.x, size.y);
    m_viewPos = pos;
    m_viewSize = size;
    m_viewScale = Vec2f(2.0f) / Vec2f(size);
}

//------------------------------------------------------------------------

Mat4f GLContext::xformMouseToUser(const Mat4f& userToClip) const
{
    return userToClip.inv()
        .postScale(Vec3f(1.0f, -1.0f, 1.0f))
        .postXlate(Vec3f(-1.0f, -1.0f, 0.0f))
        .postScale(Vec3f(m_viewScale, 1.0f))
        .postXlate(Vec3f(0.5f, 0.5f, 0.0f));
}

//------------------------------------------------------------------------

void GLContext::setAttrib(int loc, int size, GLenum type, int stride, Buffer* buffer, const void* pointer)
{
    if (loc < 0)
        return;

    glBindBuffer(GL_ARRAY_BUFFER, (buffer) ? buffer->getGLBuffer() : 0);
    glEnableVertexAttribArray(loc);
    glVertexAttribPointer(loc, size, type, GL_FALSE, stride, pointer);
    m_numAttribs = max(m_numAttribs, loc + 1);
}

//------------------------------------------------------------------------

void GLContext::resetAttribs(void)
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    for (int i = 0; i < m_numAttribs; i++)
        glDisableVertexAttribArray(i);
    m_numAttribs = 0;
}

//------------------------------------------------------------------------

void GLContext::strokeLine(const Vec4f& p0, const Vec4f& p1, U32 abgr)
{
    Vec4f v0 = m_vgXform * p0;
    Vec4f v1 = m_vgXform * p1;
    Vec2f dir = (v1.getXY() / v1.w - v0.getXY() / v0.w).normalize();
    Vec4f x0 = Vec4f(dir * m_viewScale * v0.w, 0.0f, 0.0f);
    Vec4f y0 = Vec4f(dir.perp() * m_viewScale * v0.w, 0.0f, 0.0f);
    Vec4f x1 = Vec4f(dir * m_viewScale * v1.w, 0.0f, 0.0f);
    Vec4f y1 = Vec4f(dir.perp() * m_viewScale * v1.w, 0.0f, 0.0f);

    VGVertex vertices[] =
    {
        { v0, 1.0f }, { v0 - x0 - y0, 0.0f }, { v0 - x0 + y0, 0.0f },
        { v0, 1.0f }, { v0 - x0 + y0, 0.0f }, { v1 + x1 + y1, 0.0f },
        { v0, 1.0f }, { v0 - x0 - y0, 0.0f }, { v1, 1.0f },
        { v1, 1.0f }, { v1 + x1 - y1, 0.0f }, { v1 + x1 + y1, 0.0f },
        { v1, 1.0f }, { v1 + x1 - y1, 0.0f }, { v0 - x0 - y0, 0.0f },
        { v1, 1.0f }, { v1 + x1 + y1, 0.0f }, { v0, 1.0f },
    };
    drawVG(vertices, FW_ARRAY_SIZE(vertices), abgr);
}

//------------------------------------------------------------------------

void GLContext::fillRect(const Vec4f& pos, const Vec2f& localSize, const Vec2f& screenSize, U32 abgr)
{
    Vec4f v0 = m_vgXform * pos;
    Vec4f x1 = Vec4f(Vec4f(m_vgXform.getCol(0)).getXY().normalize() * m_viewScale, 0.0f, 0.0f);
    Vec4f y1 = Vec4f(Vec4f(m_vgXform.getCol(1)).getXY().normalize() * m_viewScale, 0.0f, 0.0f);
    Vec4f x0 = (m_vgXform * Vec4f(localSize.x, 0.0f, 0.0f, 0.0f) + x1 * (screenSize.x - 1.0f)) * 0.5f;
    Vec4f y0 = (m_vgXform * Vec4f(0.0f, localSize.y, 0.0f, 0.0f) + y1 * (screenSize.y - 1.0f)) * 0.5f;

    VGVertex vertices[] =
    {
        { v0 - x0 - y0, 1.0f }, { v0 - x0 - y0 - x1 - y1, 0.0f }, { v0 + x0 - y0, 1.0f },
        { v0 + x0 - y0, 1.0f }, { v0 + x0 - y0 + x1 - y1, 0.0f }, { v0 + x0 + y0, 1.0f },
        { v0 + x0 + y0, 1.0f }, { v0 + x0 + y0 + x1 + y1, 0.0f }, { v0 - x0 + y0, 1.0f },
        { v0 - x0 + y0, 1.0f }, { v0 - x0 + y0 - x1 + y1, 0.0f }, { v0 - x0 - y0, 1.0f },
        { v0 - x0 - y0, 1.0f }, { v0 - x0 - y0 - x1 - y1, 0.0f }, { v0 - x0 + y0 - x1 + y1, 0.0f },
        { v0 + x0 - y0, 1.0f }, { v0 + x0 - y0 + x1 - y1, 0.0f }, { v0 - x0 - y0 - x1 - y1, 0.0f },
        { v0 + x0 + y0, 1.0f }, { v0 + x0 + y0 + x1 + y1, 0.0f }, { v0 + x0 - y0 + x1 - y1, 0.0f },
        { v0 - x0 + y0, 1.0f }, { v0 - x0 + y0 - x1 + y1, 0.0f }, { v0 + x0 + y0 + x1 + y1, 0.0f },
        { v0 - x0 - y0, 1.0f }, { v0 + x0 - y0, 1.0f }, { v0 - x0 + y0, 1.0f },
        { v0 + x0 - y0, 1.0f }, { v0 - x0 + y0, 1.0f }, { v0 + x0 + y0, 1.0f },
    };
    drawVG(vertices, FW_ARRAY_SIZE(vertices), abgr);
}

//------------------------------------------------------------------------

void GLContext::strokeRect(const Vec4f& pos, const Vec2f& localSize, const Vec2f& screenSize, U32 abgr)
{
    Vec4f v0 = m_vgXform * pos;
    Vec4f x1 = Vec4f(Vec4f(m_vgXform.getCol(0)).getXY().normalize() * m_viewScale, 0.0f, 0.0f);
    Vec4f y1 = Vec4f(Vec4f(m_vgXform.getCol(1)).getXY().normalize() * m_viewScale, 0.0f, 0.0f);
    Vec4f x0 = (m_vgXform * Vec4f(localSize.x, 0.0f, 0.0f, 0.0f) + x1 * screenSize.x) * 0.5f;
    Vec4f y0 = (m_vgXform * Vec4f(0.0f, localSize.y, 0.0f, 0.0f) + y1 * screenSize.y) * 0.5f;

    VGVertex vertices[] =
    {
        { v0 - x0 - y0, 1.0f }, { v0 - x0 - y0 - x1 - y1, 0.0f }, { v0 + x0 - y0, 1.0f },
        { v0 + x0 - y0, 1.0f }, { v0 + x0 - y0 + x1 - y1, 0.0f }, { v0 + x0 + y0, 1.0f },
        { v0 + x0 + y0, 1.0f }, { v0 + x0 + y0 + x1 + y1, 0.0f }, { v0 - x0 + y0, 1.0f },
        { v0 - x0 + y0, 1.0f }, { v0 - x0 + y0 - x1 + y1, 0.0f }, { v0 - x0 - y0, 1.0f },
        { v0 - x0 - y0, 1.0f }, { v0 - x0 - y0 - x1 - y1, 0.0f }, { v0 - x0 + y0 - x1 + y1, 0.0f },
        { v0 + x0 - y0, 1.0f }, { v0 + x0 - y0 + x1 - y1, 0.0f }, { v0 - x0 - y0 - x1 - y1, 0.0f },
        { v0 + x0 + y0, 1.0f }, { v0 + x0 + y0 + x1 + y1, 0.0f }, { v0 + x0 - y0 + x1 - y1, 0.0f },
        { v0 - x0 + y0, 1.0f }, { v0 - x0 + y0 - x1 + y1, 0.0f }, { v0 + x0 + y0 + x1 + y1, 0.0f },
        { v0 - x0 - y0, 1.0f }, { v0 - x0 - y0 + x1 + y1, 0.0f }, { v0 + x0 - y0, 1.0f },
        { v0 + x0 - y0, 1.0f }, { v0 + x0 - y0 - x1 + y1, 0.0f }, { v0 + x0 + y0, 1.0f },
        { v0 + x0 + y0, 1.0f }, { v0 + x0 + y0 - x1 - y1, 0.0f }, { v0 - x0 + y0, 1.0f },
        { v0 - x0 + y0, 1.0f }, { v0 - x0 + y0 + x1 - y1, 0.0f }, { v0 - x0 - y0, 1.0f },
        { v0 - x0 - y0, 1.0f }, { v0 - x0 - y0 + x1 + y1, 0.0f }, { v0 - x0 + y0 + x1 - y1, 0.0f },
        { v0 + x0 - y0, 1.0f }, { v0 + x0 - y0 - x1 + y1, 0.0f }, { v0 - x0 - y0 + x1 + y1, 0.0f },
        { v0 + x0 + y0, 1.0f }, { v0 + x0 + y0 - x1 - y1, 0.0f }, { v0 + x0 - y0 - x1 + y1, 0.0f },
        { v0 - x0 + y0, 1.0f }, { v0 - x0 + y0 + x1 - y1, 0.0f }, { v0 + x0 + y0 - x1 - y1, 0.0f },
    };
    drawVG(vertices, FW_ARRAY_SIZE(vertices), abgr);
}

//------------------------------------------------------------------------

void GLContext::setFont(const String& name, int size, U32 style)
{
    FW_ASSERT(size > 0);

    LOGFONT lf;
    lf.lfHeight         = size;
    lf.lfWidth          = 0;
    lf.lfEscapement     = 0;
    lf.lfOrientation    = 0;
    lf.lfWeight         = ((style & FontStyle_Bold) != 0) ? FW_BOLD : FW_NORMAL;
    lf.lfItalic         = ((style & FontStyle_Italic) != 0);
    lf.lfUnderline      = false;
    lf.lfStrikeOut      = false;
    lf.lfCharSet        = ANSI_CHARSET;
    lf.lfOutPrecision   = OUT_DEFAULT_PRECIS;
    lf.lfClipPrecision  = CLIP_DEFAULT_PRECIS;
    lf.lfQuality        = PROOF_QUALITY;
    lf.lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
    memcpy(lf.lfFaceName, name.getPtr(), name.getLength() + 1);

    HFONT font = CreateFontIndirect(&lf);
    if (!font)
        failWin32Error("CreateFontIndirect");

    setFont(font);
}

//------------------------------------------------------------------------

Vec2i GLContext::getStringSize(const String& str)
{
    SIZE size;
    if (!GetTextExtentPoint32(m_memdc, str.getPtr(), str.getLength(), &size))
        failWin32Error("GetTextExtentPoint32");
    return Vec2i(size.cx + m_vgFontMetrics.tmOverhang, size.cy);
}

//------------------------------------------------------------------------

Vec2i GLContext::drawString(const String& str, const Vec4f& pos, const Vec2f& align, U32 abgr)
{
    Vec2i strSize = getStringSize(str);
    if (strSize.x <= 0 || strSize.y <= 0)
        return strSize;

    glPushAttrib(GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT);
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    const Vec2i& texSize = uploadString(str, strSize);
    Vec4f pivot = m_vgXform * pos;
    Vec2f adjust = -align * Vec2f(strSize);
    drawString(pivot + Vec4f(adjust * m_viewScale * pivot.w, 0.0f, 0.0f), strSize, texSize, Vec4f::fromABGR(abgr));
    glPopAttrib();
    return strSize;
}

//------------------------------------------------------------------------

Vec2i GLContext::drawLabel(const String& str, const Vec4f& pos, const Vec2f& align, U32 abgr)
{
    Vec2i strSize = getStringSize(str);
    if (strSize.x <= 0 || strSize.y <= 0)
        return strSize;

    glPushAttrib(GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT);
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    Vec4f fgColor = Vec4f::fromABGR(abgr);
    Vec4f bgColor(0.0f, 0.0f, 0.0f, sqr(fgColor.w));
    const Vec2i& texSize = uploadString(str, strSize);
    Vec4f pivot = m_vgXform * pos;
    Vec2f adjust = -align * Vec2f(strSize);

    for (int j = -1; j <= 1; j++)
        for (int k = -1; k <= 1; k++)
            drawString(pivot + Vec4f((adjust + Vec2f((F32)j, (F32)k)) * m_viewScale * pivot.w, 0.0f, 0.0f), strSize, texSize, bgColor);
    drawString(pivot + Vec4f(adjust * m_viewScale * pivot.w, 0.0f, 0.0f), strSize, texSize, fgColor);
    glPopAttrib();
    return strSize;
}

//------------------------------------------------------------------------

void GLContext::drawModalMessage(const String& msg)
{
    glPushAttrib(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDrawBuffer(GL_BACK);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);

    Mat4f oldXform = setVGXform(Mat4f());
    HFONT oldFont = m_vgFont;
    m_vgFont = NULL;
    setFont("Arial", 32, GLContext::FontStyle_Normal);

    drawString(msg, Vec4f(Vec3f(0.0f), 1.0f), Vec2f(0.5f), 0xFFFFFFFF);

    setVGXform(oldXform);
    setFont(oldFont);
    glPopAttrib();
}

//------------------------------------------------------------------------

void GLContext::drawImage(Image& image, const Vec4f& pos, const Vec2f& align, bool topToBottom)
{
    const Vec2i& imgSize = image.getSize();
    if (imgSize.min() <= 0)
        return;

    Buffer& buf = image.getBuffer();
    ImageFormat format = image.getFormat().getGLFormat();
    const ImageFormat::StaticFormat* sf = format.getStaticFormat();

    glActiveTexture(GL_TEXTURE0);
    const Vec2i& texSize = bindTempTexture(imgSize);

    // Format is not supported by GL => convert and upload.

    if (image.getFormat() != format || image.getStride() != imgSize.x * format.getBPP())
    {
        Image converted(imgSize, format);
        converted = image;
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, imgSize.x, imgSize.y, sf->glFormat, sf->glType, converted.getPtr());
    }

    // Data is already on the GPU => transfer to the texture.

    else if (buf.getOwner() == Buffer::GL || (buf.getOwner() == Buffer::Cuda && (buf.getHints() & Buffer::Hint_CudaGL) != 0))
    {
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, buf.getGLBuffer());
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, imgSize.x, imgSize.y, sf->glFormat, sf->glType, NULL);
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
    }

    // Otherwise => upload.

    else
    {
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, imgSize.x, imgSize.y, sf->glFormat, sf->glType, buf.getPtr());
    }

    // Determine orientation.

    Vec4f posLo = m_vgXform * pos;
    Vec2f posRange = Vec2f(imgSize) * m_viewScale * posLo.w;
    posLo -= Vec4f(align * posRange, 0.0f, 0.0f);
    Vec2f posHi = posLo.getXY() + posRange;

    if (topToBottom)
        swap(posLo.y, posHi.y);

    // Draw texture.

    glPushAttrib(GL_ENABLE_BIT);
    glDisable(GL_CULL_FACE);
    drawTexture(0, posLo, posHi, Vec2f(0.0f), Vec2f(imgSize) / Vec2f(texSize));
    glPopAttrib();
    checkErrors();
}

//------------------------------------------------------------------------

GLContext::Program* GLContext::getProgram(const String& id) const
{
    Program* const* prog = (s_programs) ? s_programs->search(id) : NULL;
    return (prog) ? *prog : NULL;
}

//------------------------------------------------------------------------

void GLContext::setProgram(const String& id, Program* prog)
{
    if (!s_programs)
        s_programs = new Hash<String, Program*>;
    if (s_programs->contains(id))
        delete s_programs->remove(id);
    if (prog)
        s_programs->add(id, prog);
}

//------------------------------------------------------------------------

void GLContext::staticInit(void)
{
    if (s_inited)
        return;
    s_inited = true;

    // Create window for the share context.

    s_shareHWND = Window::createHWND();
    s_shareHDC = GetDC(s_shareHWND);
    if (!s_shareHDC)
        failWin32Error("GetDC");

    // Set pixel format.

    PIXELFORMATDESCRIPTOR pfd;
    FW::memset(&pfd, 0, sizeof(pfd));
    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;

    int formatIdx = ChoosePixelFormat(s_shareHDC, &pfd);
    if (!formatIdx)
        failWin32Error("ChoosePixelFormat");
    if (!DescribePixelFormat(s_shareHDC, formatIdx, sizeof(pfd), &pfd))
        failWin32Error("DescribePixelFormat");
    if (!SetPixelFormat(s_shareHDC, formatIdx, &pfd))
        failWin32Error("SetPixelFormat");

    // Create WGL context.

    s_shareHGLRC = wglCreateContext(s_shareHDC);
    if (!s_shareHGLRC)
        failWin32Error("wglCreateContext");
    if (!wglMakeCurrent(s_shareHDC, s_shareHGLRC))
        failWin32Error("wglMakeCurrent");

    // Check GL version.

    String glVersion = (const char*)glGetString(GL_VERSION);
    int dotIdx = glVersion.indexOf('.');
    if (dotIdx < 1 || glVersion[dotIdx - 1] < '2')
        fail("OpenGL 2.0 or later is required!");

    // Import extension functions.

#if FW_USE_GLEW
    GLenum err = glewInit();
    if (err != GLEW_OK)
        fail("glewInit() failed: %s!", glewGetErrorString(err));
#else
    initGLImports();
#endif

    // Create wrapper GLContext.

    FW_ASSERT(!s_headless);
    s_headless = new GLContext(s_shareHDC, s_shareHGLRC);

    // Determine whether stereo is available.

    Config stereoConfig;
    stereoConfig.isStereo = true;
    s_stereoAvailable = s_headless->choosePixelFormat(formatIdx, s_shareHDC, stereoConfig);
}

//------------------------------------------------------------------------

void GLContext::staticDeinit(void)
{
    if (!s_inited)
        return;
    s_inited = false;

    while (s_tempTextures.next != &s_tempTextures)
    {
        TempTexture* tex = s_tempTextures.next;
        s_tempTextures.next = tex->next;
        glDeleteTextures(1, &tex->handle);
        delete tex;
    }
    delete s_tempTexHash;
    s_tempTextures.prev = &s_tempTextures;
    s_tempTexHash = NULL;

    if (s_programs)
    {
        for (int i = s_programs->firstSlot(); i != -1; i = s_programs->nextSlot(i))
            delete s_programs->getSlot(i).value;
        delete s_programs;
        s_programs = NULL;
    }

    FW_ASSERT(s_headless);
    delete s_headless;
    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(s_shareHGLRC);
    ReleaseDC(s_shareHWND, s_shareHDC);
    DestroyWindow(s_shareHWND);

    s_shareHWND     = NULL;
    s_shareHDC      = NULL;
    s_shareHGLRC    = NULL;
    s_headless      = NULL;
    s_current       = NULL;
}

//------------------------------------------------------------------------

void GLContext::checkErrors(void)
{
    if (!s_current)
        return;

    GLenum err = glGetError();
    const char* name;
    switch (err)
    {
    case GL_NO_ERROR:                       name = NULL; break;
    case GL_INVALID_ENUM:                   name = "GL_INVALID_ENUM"; break;
    case GL_INVALID_VALUE:                  name = "GL_INVALID_VALUE"; break;
    case GL_INVALID_OPERATION:              name = "GL_INVALID_OPERATION"; break;
    case GL_STACK_OVERFLOW:                 name = "GL_STACK_OVERFLOW"; break;
    case GL_STACK_UNDERFLOW:                name = "GL_STACK_UNDERFLOW"; break;
    case GL_OUT_OF_MEMORY:                  name = "GL_OUT_OF_MEMORY"; break;
    case GL_INVALID_FRAMEBUFFER_OPERATION:  name = "GL_INVALID_FRAMEBUFFER_OPERATION"; break;
    default:                                name = "unknown"; break;
    }

    if (name)
        fail("Caught GL error 0x%04x (%s)!", err, name);
}

//------------------------------------------------------------------------

bool GLContext::choosePixelFormat(int& formatIdx, HDC hdc, const Config& config)
{
    Array<Vec2i> attribs; // WGL_ARB_pixel_format
    attribs.add(Vec2i(WGL_DRAW_TO_WINDOW_ARB,   1));
    attribs.add(Vec2i(WGL_ACCELERATION_ARB,     WGL_FULL_ACCELERATION_ARB));
    attribs.add(Vec2i(WGL_SUPPORT_OPENGL_ARB,   1));
    attribs.add(Vec2i(WGL_DOUBLE_BUFFER_ARB,    1));
    attribs.add(Vec2i(WGL_PIXEL_TYPE_ARB,       WGL_TYPE_RGBA_ARB));

    attribs.add(Vec2i(WGL_RED_BITS_ARB,         8));
    attribs.add(Vec2i(WGL_GREEN_BITS_ARB,       8));
    attribs.add(Vec2i(WGL_BLUE_BITS_ARB,        8));
    attribs.add(Vec2i(WGL_ALPHA_BITS_ARB,       0));
    attribs.add(Vec2i(WGL_ACCUM_BITS_ARB,       0));
    attribs.add(Vec2i(WGL_DEPTH_BITS_ARB,       24));
    attribs.add(Vec2i(WGL_STENCIL_BITS_ARB,     8));
    attribs.add(Vec2i(WGL_AUX_BUFFERS_ARB,      0));

    if (config.isStereo)
        attribs.add(Vec2i(WGL_STEREO_ARB, 1));
    if (config.numSamples)
        attribs.add(Vec2i(WGL_SAMPLES_ARB, config.numSamples)); // WGL_ARB_multisample

    attribs.add(0);

    UINT numFormats = 0;
    if (!GL_FUNC_AVAILABLE(wglChoosePixelFormatARB))
        fail("wglChoosePixelFormatARB() not available!");
    if (!wglChoosePixelFormatARB(hdc, &attribs[0].x, NULL, 1, &formatIdx, &numFormats))
        failWin32Error("wglChoosePixelFormatARB");
    return (numFormats != 0);
}

//------------------------------------------------------------------------

void GLContext::init(HDC hdc, HGLRC hglrc)
{
    FW_ASSERT(hdc && hglrc);

    // Initialize members.

    m_hdc           = hdc;
    m_hglrc         = hglrc;

    m_viewPos       = 0;
    m_viewSize      = 1;
    m_viewScale     = 2.0f;
    m_numAttribs    = 0;

    m_vgFont        = NULL;

    // Setup text rendering.

    m_memdc = CreateCompatibleDC(m_hdc);
    if (!m_memdc)
        failWin32Error("CreateCompatibleDC");

    if (SetTextAlign(m_memdc, TA_TOP | TA_LEFT) == GDI_ERROR)
        failWin32Error("SetTextAlign");

    if (SetBkColor(m_memdc, RGB(0x00, 0x00, 0xFF)) == CLR_INVALID)
        failWin32Error("SetTextColor");

    if (SetTextColor(m_memdc, RGB(0xFF, 0xFF, 0x00)) == CLR_INVALID)
        failWin32Error("SetTextColor");

    setDefaultFont();

    // Initialize GL state.

    GLContext* oldContext = s_current;
    makeCurrent();

    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    checkErrors();

    if (oldContext)
        oldContext->makeCurrent();
}

//------------------------------------------------------------------------

void GLContext::drawVG(const VGVertex* vertices, int numVertices, U32 abgr)
{
    // Convert color.

    Vec4f color = Vec4f::fromABGR(abgr);
    if (color.w <= 0.0f)
        return;

    // Create program.

    static const char* progId = "GLContext::drawVG";
    Program* prog = getProgram(progId);
    if (!prog)
    {
        prog = new Program(
            FW_GL_SHADER_SOURCE(
                uniform vec4 color;
                attribute vec4 pos;
                attribute float alpha;
                varying vec4 shadedColor;
                void main()
                {
                    gl_Position = pos;
                    shadedColor = vec4(color.rgb, color.a * alpha);
                }
            ),
            FW_GL_SHADER_SOURCE(
                varying vec4 shadedColor;
                void main()
                {
                    gl_FragColor = shadedColor;
                }
            ));
        setProgram(progId, prog);
    }

    // Setup state.

    glPushAttrib(GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT);
    glDisable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Draw.

    prog->use();
    setUniform(prog->getUniformLoc("color"), color);
    setAttrib(prog->getAttribLoc("pos"), 4, GL_FLOAT, sizeof(VGVertex), &vertices->pos);
    setAttrib(prog->getAttribLoc("alpha"), 1, GL_FLOAT, sizeof(VGVertex), &vertices->alpha);
    glDrawArrays(GL_TRIANGLES, 0, numVertices);
    resetAttribs();

    // Clean up.

    glPopAttrib();
    checkErrors();
}

//------------------------------------------------------------------------

void GLContext::setFont(HFONT font)
{
    FW_ASSERT(font);

    DeleteObject(m_vgFont);
    m_vgFont = font;

    if (!SelectObject(m_memdc, m_vgFont))
        failWin32Error("SelectObject");

    if (!GetTextMetrics(m_memdc, &m_vgFontMetrics))
        failWin32Error("GetTextMetrics");
}

//------------------------------------------------------------------------

const Vec2i& GLContext::uploadString(const String& str, const Vec2i& strSize)
{
    // Create word-oriented DIB.

    U8 bmi[sizeof(BITMAPINFOHEADER) + 3 * sizeof(DWORD)];
    BITMAPINFOHEADER* bmih  = (BITMAPINFOHEADER*)bmi;
    DWORD* masks            = (DWORD*)(bmih + 1);

    bmih->biSize            = sizeof(BITMAPINFOHEADER);
    bmih->biWidth           = strSize.x;
    bmih->biHeight          = strSize.y;
    bmih->biPlanes          = 1;
    bmih->biBitCount        = 32;
    bmih->biCompression     = BI_BITFIELDS;
    bmih->biSizeImage       = 0;
    bmih->biXPelsPerMeter   = 0;
    bmih->biYPelsPerMeter   = 0;
    bmih->biClrUsed         = 0;
    bmih->biClrImportant    = 0;
    masks[0]                = 0x00FF0000;
    masks[1]                = 0x0000FF00;
    masks[2]                = 0x000000FF;

    void* buffer;
    HBITMAP dib = CreateDIBSection(m_memdc, (BITMAPINFO*)bmi, DIB_RGB_COLORS, &buffer, NULL, 0);
    if (!dib)
        failWin32Error("CreateDIBSection");

    // Clear DIB.

    for (int i = strSize.x * strSize.y - 1; i >= 0; i--)
        ((U32*)buffer)[i] = 0x000000FF;

    // Draw string.

    if (!SelectObject(m_memdc, dib))
        failWin32Error("SelectObject");

    if (!TextOut(m_memdc, 0, 0, str.getPtr(), str.getLength()))
        failWin32Error("TextOut");

    // Upload to texture and destroy DIB.

    glActiveTexture(GL_TEXTURE0);
    const Vec2i& texSize = bindTempTexture(strSize);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, strSize.x, strSize.y, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
    DeleteObject(dib);
    return texSize;
}

//------------------------------------------------------------------------

void GLContext::drawString(const Vec4f& pos, const Vec2i& strSize, const Vec2i& texSize, const Vec4f& color)
{
    // Determine orientation.

    Vec2f posHi = pos.getXY() + Vec2f(strSize) * m_viewScale * pos.w;
    Vec2f texHi = Vec2f(strSize) / Vec2f(texSize);

    F32 posAttrib[16] =
    {
        pos.x, pos.y, pos.z, pos.w,
        posHi.x, pos.y, pos.z, pos.w,
        pos.x, posHi.y, pos.z, pos.w,
        posHi.x, posHi.y, pos.z, pos.w,
    };

    F32 texAttrib[8] =
    {
        0.0f, 0.0f,
        texHi.x, 0.0f,
        0.0f, texHi.y,
        texHi.x, texHi.y,
    };

    // Create program.

    static const char* progId = "GLContext::drawString";
    GLContext::Program* prog = getProgram(progId);
    if (!prog)
    {
        prog = new GLContext::Program(
            FW_GL_SHADER_SOURCE(
                attribute vec4 posAttrib;
                attribute vec2 texAttrib;
                varying vec2 texVarying;

                void main()
                {
                    gl_Position = posAttrib;
                    texVarying = texAttrib;
                }
            ),
            FW_GL_SHADER_SOURCE(
                uniform sampler2D texSampler;
                uniform vec4 colorUniform;
                uniform float brightnessUniform;
                varying vec2 texVarying;

                void main()
                {
                    vec4 tex = texture2D(texSampler, texVarying);
                    float alpha = mix(1.0 - max(tex.x, tex.w), tex.y, brightnessUniform);
                    gl_FragColor = vec4(colorUniform.xyz, colorUniform.w * alpha);
                }
            ));
        setProgram(progId, prog);
    }

    // Draw texture.

    prog->use();
    setUniform(prog->getUniformLoc("texSampler"), 0);
    setUniform(prog->getUniformLoc("colorUniform"), color);
    setUniform(prog->getUniformLoc("brightnessUniform"), (color.x + color.y + color.z) * (1.0f / 3.0f));
    setAttrib(prog->getAttribLoc("posAttrib"), 4, GL_FLOAT, 0, posAttrib);
    setAttrib(prog->getAttribLoc("texAttrib"), 2, GL_FLOAT, 0, texAttrib);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    resetAttribs();
}

//------------------------------------------------------------------------

const Vec2i& GLContext::bindTempTexture(const Vec2i& size)
{
    // Round size.

    Vec2i rounded = 1;
    while (rounded.x < size.x)
        rounded.x *= 2;
    while (rounded.y < size.y)
        rounded.y *= 2;
    if (rounded.x >= 512 * 512 / rounded.y)
        rounded = size;

    // No hash => create one.

    if (!s_tempTexHash)
        s_tempTexHash = new Hash<Vec2i, TempTexture*>;

    // Exists => move to LRU head and bind.

    TempTexture** found = s_tempTexHash->search(rounded);
    if (found)
    {
        TempTexture* tex = *found;
        tex->prev->next = tex->next;
        tex->next->prev = tex->prev;
        tex->prev = &s_tempTextures;
        tex->next = s_tempTextures.next;
        tex->prev->next = tex;
        tex->next->prev = tex;

        glBindTexture(GL_TEXTURE_2D, tex->handle);
        return tex->size;
    }

    // Destroy old textures to satisfy FW_MAX_TEMP_TEXTURE_BYTES.

    while (s_tempTexBytes > FW_MAX_TEMP_TEXTURE_BYTES && s_tempTexHash->getSize() > FW_MIN_TEMP_TEXTURES)
    {
        TempTexture* tex = s_tempTextures.prev;
        glDeleteTextures(1, &tex->handle);
        tex->prev->next = tex->next;
        tex->next->prev = tex->prev;
        s_tempTexBytes -= tex->bytes;
        s_tempTexHash->remove(tex->size);
        delete tex;
    }

    // Create new texture.

    TempTexture* tex = new TempTexture;
    tex->size = rounded;
    tex->bytes = rounded.x * rounded.y * 4;
    glGenTextures(1, &tex->handle);
    glBindTexture(GL_TEXTURE_2D, tex->handle);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, rounded.x, rounded.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

    // Add to LRU and hash.

    tex->prev = &s_tempTextures;
    tex->next = s_tempTextures.next;
    tex->prev->next = tex;
    tex->next->prev = tex;
    s_tempTexBytes += tex->bytes;
    s_tempTexHash->add(rounded, tex);
    return tex->size;
}

//------------------------------------------------------------------------

void GLContext::drawTexture(int unit, const Vec4f& posLo, const Vec2f& posHi, const Vec2f& texLo, const Vec2f& texHi)
{
    // Setup vertex attributes.

    F32 posAttrib[] =
    {
        posLo.x, posLo.y, posLo.z, posLo.w,
        posHi.x, posLo.y, posLo.z, posLo.w,
        posLo.x, posHi.y, posLo.z, posLo.w,
        posHi.x, posHi.y, posLo.z, posLo.w,
    };

    F32 texAttrib[] =
    {
        texLo.x, texLo.y,
        texHi.x, texLo.y,
        texLo.x, texHi.y,
        texHi.x, texHi.y,
    };

    // Create program.

    static const char* progId = "GLContext::drawTexture";
    GLContext::Program* prog = getProgram(progId);
    if (!prog)
    {
        prog = new GLContext::Program(
            FW_GL_SHADER_SOURCE(
                attribute vec4 posAttrib;
                attribute vec2 texAttrib;
                varying vec2 texVarying;
                void main()
                {
                    gl_Position = posAttrib;
                    texVarying = texAttrib;
                }
            ),
            FW_GL_SHADER_SOURCE(
                uniform sampler2D texSampler;
                varying vec2 texVarying;
                void main()
                {
                    gl_FragColor = texture2D(texSampler, texVarying);
                }
            ));
        setProgram(progId, prog);
    }

    // Draw texture.

    prog->use();
    setUniform(prog->getUniformLoc("texSampler"), unit);
    setAttrib(prog->getAttribLoc("posAttrib"), 4, GL_FLOAT, 0, posAttrib);
    setAttrib(prog->getAttribLoc("texAttrib"), 2, GL_FLOAT, 0, texAttrib);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    resetAttribs();
}

//------------------------------------------------------------------------
