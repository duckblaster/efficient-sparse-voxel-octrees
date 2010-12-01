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
 
#pragma once
#include "base/Defs.hpp"

#include <cuda.h>

#if !FW_CUDA
#   define _WIN32_WINNT 0x0501
#   define WIN32_LEAN_AND_MEAN
#   define _WINMM_
#   include <windows.h>
#   undef min
#   undef max

#   pragma warning(push,3)
#   include <mmsystem.h>
#   pragma warning(pop)

#   define _SHLWAPI_
#   include <shlwapi.h>
#endif

//------------------------------------------------------------------------

#define FW_USE_GLEW 0

//------------------------------------------------------------------------

namespace FW
{
#if !FW_CUDA
void    setCudaDLLName      (const String& name);
void    initDLLImports      (void);
void    initGLImports       (void);
void    deinitDLLImports    (void);
#endif
}

//------------------------------------------------------------------------
// CUDA definitions.
//------------------------------------------------------------------------

#if (CUDA_VERSION < 3020)
typedef unsigned int    CUsize_t;
#else
typedef size_t          CUsize_t;
#endif

//------------------------------------------------------------------------
// GL definitions.
//------------------------------------------------------------------------

#if (!FW_CUDA && FW_USE_GLEW)
#   define GL_FUNC_AVAILABLE(NAME) (NAME != NULL)
#   define GLEW_STATIC
#   include "3rdparty/glew/include/GL/glew.h"
#   include "3rdparty/glew/include/GL/wglew.h"
#   include <cudaGL.h>

#elif (!FW_CUDA && !FW_USE_GLEW)
#   define GL_FUNC_AVAILABLE(NAME) (isAvailable_ ## NAME())
#   include <GL/gl.h>
#   include <cudaGL.h>

typedef char            GLchar;
typedef ptrdiff_t       GLintptr;
typedef ptrdiff_t       GLsizeiptr;
typedef unsigned int    GLhandleARB;

#define GL_ALPHA32F_ARB                     0x8816
#define GL_ARRAY_BUFFER                     0x8892
#define GL_BUFFER_SIZE                      0x8764
#define GL_COLOR_ATTACHMENT0                0x8CE0
#define GL_COLOR_ATTACHMENT1                0x8CE1
#define GL_COLOR_ATTACHMENT2                0x8CE2
#define GL_COMPILE_STATUS                   0x8B81
#define GL_DEPTH_ATTACHMENT                 0x8D00
#define GL_ELEMENT_ARRAY_BUFFER             0x8893
#define GL_FRAGMENT_SHADER                  0x8B30
#define GL_FRAMEBUFFER                      0x8D40
#define GL_FUNC_ADD                         0x8006
#define GL_GENERATE_MIPMAP                  0x8191
#define GL_GEOMETRY_INPUT_TYPE_ARB          0x8DDB
#define GL_GEOMETRY_OUTPUT_TYPE_ARB         0x8DDC
#define GL_GEOMETRY_SHADER_ARB              0x8DD9
#define GL_GEOMETRY_VERTICES_OUT_ARB        0x8DDA
#define GL_INFO_LOG_LENGTH                  0x8B84
#define GL_INVALID_FRAMEBUFFER_OPERATION    0x0506
#define GL_LINK_STATUS                      0x8B82
#define GL_PIXEL_PACK_BUFFER                0x88EB
#define GL_PIXEL_UNPACK_BUFFER              0x88EC
#define GL_RENDERBUFFER                     0x8D41
#define GL_RGB32F                           0x8815
#define GL_RGBA32F                          0x8814
#define GL_RGBA32UI                         0x8D70
#define GL_RGBA_INTEGER                     0x8D99
#define GL_STATIC_DRAW                      0x88E4
#define GL_DYNAMIC_COPY                     0x88EA
#define GL_TEXTURE0                         0x84C0
#define GL_TEXTURE1                         0x84C1
#define GL_TEXTURE_3D                       0x806F
#define GL_TEXTURE_CUBE_MAP                 0x8513
#define GL_TEXTURE_CUBE_MAP_POSITIVE_X      0x8515
#define GL_UNSIGNED_SHORT_5_5_5_1           0x8034
#define GL_UNSIGNED_SHORT_5_6_5             0x8363
#define GL_VERTEX_SHADER                    0x8B31
#define GL_ARRAY_BUFFER_BINDING             0x8894
#define GL_READ_FRAMEBUFFER                 0x8CA8
#define GL_DRAW_FRAMEBUFFER                 0x8CA9
#define GL_TEXTURE_MAX_ANISOTROPY_EXT       0x84FE

#define WGL_ACCELERATION_ARB                0x2003
#define WGL_ACCUM_BITS_ARB                  0x201D
#define WGL_ALPHA_BITS_ARB                  0x201B
#define WGL_AUX_BUFFERS_ARB                 0x2024
#define WGL_BLUE_BITS_ARB                   0x2019
#define WGL_DEPTH_BITS_ARB                  0x2022
#define WGL_DOUBLE_BUFFER_ARB               0x2011
#define WGL_DRAW_TO_WINDOW_ARB              0x2001
#define WGL_FULL_ACCELERATION_ARB           0x2027
#define WGL_GREEN_BITS_ARB                  0x2017
#define WGL_PIXEL_TYPE_ARB                  0x2013
#define WGL_RED_BITS_ARB                    0x2015
#define WGL_SAMPLES_ARB                     0x2042
#define WGL_STENCIL_BITS_ARB                0x2023
#define WGL_STEREO_ARB                      0x2012
#define WGL_SUPPORT_OPENGL_ARB              0x2010
#define WGL_TYPE_RGBA_ARB                   0x202B
#define WGL_NUMBER_OVERLAYS_ARB             0x2008
#define WGL_NUMBER_UNDERLAYS_ARB            0x2009

#endif

//------------------------------------------------------------------------

#if !FW_CUDA
#   define FW_DLL_IMPORT_RETV(RET, CALL, NAME, PARAMS, PASS)    bool isAvailable_ ## NAME(void);
#   define FW_DLL_IMPORT_VOID(RET, CALL, NAME, PARAMS, PASS)    bool isAvailable_ ## NAME(void);
#   define FW_DLL_IMPORT_CUV2(RET, CALL, NAME, PARAMS, PASS)    bool isAvailable_ ## NAME(void);
#   define FW_DLL_DECLARE_RETV(RET, CALL, NAME, PARAMS, PASS)   bool isAvailable_ ## NAME(void); RET CALL NAME PARAMS;
#   define FW_DLL_DECLARE_VOID(RET, CALL, NAME, PARAMS, PASS)   bool isAvailable_ ## NAME(void); RET CALL NAME PARAMS;
#   include "base/DLLImports.inl"
#   undef FW_DLL_IMPORT_RETV
#   undef FW_DLL_IMPORT_VOID
#   undef FW_DLL_IMPORT_CUV2
#   undef FW_DLL_DECLARE_RETV
#   undef FW_DLL_DECLARE_VOID
#endif

//------------------------------------------------------------------------
