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
 
#include "io/MeshWavefrontIO.hpp"
#include "io/File.hpp"
#include "base/Hash.hpp"

using namespace FW;

//------------------------------------------------------------------------

#define WAVEFRONT_DEBUG 0

//------------------------------------------------------------------------

namespace FW
{

struct Material : public MeshBase::Material
{
    S32                     submesh;
};

struct TextureSpec
{
    Texture                 texture;
    F32                     base;
    F32                     gain;
};

struct ImportState
{
    Mesh<VertexPNT>*        mesh;

    Array<Vec3f>            positions;
    Array<Vec2f>            texCoords;
    Array<Vec3f>            normals;

    Hash<VertexPNT, S32>    vertexHash;
    Hash<String, Material>  materialHash;
};

template <class T> bool equals  (const VertexPNT& a, const VertexPNT& b);
template <class T> U32 hash     (const VertexPNT& value);

static bool     parseFloats     (const char*& ptr, F32* values, int num);
static bool     parseTexture    (const char*& ptr, TextureSpec& value, const String& dirName);

static void     loadMtl         (ImportState& s, BufferedInputStream& mtlIn, const String& dirName);
static void     loadObj         (ImportState& s, BufferedInputStream& objIn, const String& dirName);

}

//------------------------------------------------------------------------

template <class T> bool FW::equals(const VertexPNT& a, const VertexPNT& b)
{
    return (equals<Vec3f>(a.p, b.p) && equals<Vec3f>(a.n, b.n) && equals<Vec2f>(a.t, b.t));
}

//------------------------------------------------------------------------

template <class T> U32 FW::hash(const VertexPNT& value)
{
    return hashBits(hash<Vec3f>(value.p), hash<Vec3f>(value.n), hash<Vec2f>(value.t));
}

//------------------------------------------------------------------------

bool FW::parseFloats(const char*& ptr, F32* values, int num)
{
    FW_ASSERT(values);
    const char* tmp = ptr;
    for (int i = 0; i < num; i++)
    {
        if (i)
            parseSpace(tmp);
        if (!parseFloat(tmp, values[i]))
            return false;
    }
    ptr = tmp;
    return true;
}

//------------------------------------------------------------------------

bool FW::parseTexture(const char*& ptr, TextureSpec& value, const String& dirName)
{
    // Initialize result.

    String name;
    value.texture   = Texture();
    value.base      = 0.0f;
    value.gain      = 1.0f;

    // Parse options.

    while (*ptr)
    {
        parseSpace(ptr);
        if ((parseLiteral(ptr, "-blendu ") || parseLiteral(ptr, "-blendv ") || parseLiteral(ptr, "-cc ") || parseLiteral(ptr, "-clamp ")) && parseSpace(ptr))
        {
            if (!parseLiteral(ptr, "on") && !parseLiteral(ptr, "off"))
                return false;
        }
        else if (parseLiteral(ptr, "-mm ") && parseSpace(ptr))
        {
            if (!parseFloat(ptr, value.base) || !parseSpace(ptr) || !parseFloat(ptr, value.gain))
                return false;
        }
        else if ((parseLiteral(ptr, "-o ") || parseLiteral(ptr, "-s ") || parseLiteral(ptr, "-t ")) && parseSpace(ptr))
        {
            F32 tmp[2];
            if (!parseFloats(ptr, tmp, 2))
                return false;
            parseSpace(ptr);
            parseFloat(ptr, tmp[0]);
        }
        else if ((parseLiteral(ptr, "-texres ") || parseLiteral(ptr, "-bm ")) && parseSpace(ptr))
        {
            F32 tmp;
            if (!parseFloat(ptr, tmp))
                return false;
        }
        else
        {
            if (*ptr == '-' || name.getLength())
                return false;
            while (*ptr && (*ptr != '-' || !name.endsWith(' ')))
                name.append(*ptr++);
        }
    }

    // Process file name.

    while (name.startsWith('/'))
        name = name.substring(1);
    while (name.endsWith(' '))
        name = name.substring(0, name.getLength() - 1);
    if (!name.getLength())
        return false;
    name = dirName + '/' + name;

    // Import texture.

    value.texture = Texture::import(name);
    return true;
}

//------------------------------------------------------------------------

void FW::loadMtl(ImportState& s, BufferedInputStream& mtlIn, const String& dirName)
{
    Material* mat = NULL;
    while (!hasError())
    {
        const char* line = mtlIn.readLine(true, true);
        if (!line)
            break;

        const char* ptr = line;
        parseSpace(ptr);
        bool valid = false;

        if (!*ptr || parseLiteral(ptr, "#"))
        {
            valid = true;
        }
        else if (parseLiteral(ptr, "newmtl ") && parseSpace(ptr) && *ptr) // material name
        {
            if (!s.materialHash.contains(ptr))
            {
                mat = &s.materialHash.add(ptr);
                mat->submesh = -1;
            }
            valid = true;
        }
        else if (!mat)
        {
            setError("No current material in Wavefront MTL: '%s'!", line);
        }
        else if (parseLiteral(ptr, "Ka ") && parseSpace(ptr)) // ambient color
        {
            F32 tmp[3];
            if (parseLiteral(ptr, "spectral ") || parseLiteral(ptr, "xyz "))
                valid = true;
            else if (parseFloats(ptr, tmp, 3) && parseSpace(ptr) && !*ptr)
                valid = true;
        }
        else if (parseLiteral(ptr, "Kd ") && parseSpace(ptr)) // diffuse color
        {
            if (parseLiteral(ptr, "spectral ") || parseLiteral(ptr, "xyz "))
                valid = true;
            else if (parseFloats(ptr, mat->diffuse.getPtr(), 3) && parseSpace(ptr) && !*ptr)
                valid = true;
        }
        else if (parseLiteral(ptr, "Ks ") && parseSpace(ptr)) // specular color
        {
            if (parseLiteral(ptr, "spectral ") || parseLiteral(ptr, "xyz "))
                valid = true;
            else if (parseFloats(ptr, mat->specular.getPtr(), 3) && parseSpace(ptr) && !*ptr)
                valid = true;
        }
        else if (parseLiteral(ptr, "d ") && parseSpace(ptr)) // alpha
        {
            if (parseFloat(ptr, mat->diffuse.w) && parseSpace(ptr) && !*ptr)
                valid = true;
        }
        else if (parseLiteral(ptr, "Ns ") && parseSpace(ptr)) // glossiness
        {
            if (parseFloat(ptr, mat->glossiness) && parseSpace(ptr) && !*ptr)
                valid = true;
            if (mat->glossiness <= 0.0f)
            {
                mat->glossiness = 1.0f;
                mat->specular.setZero();
            }
        }
        else if (parseLiteral(ptr, "map_Kd ")) // diffuse texture
        {
            TextureSpec tex;
            valid = parseTexture(ptr, tex, dirName);
            mat->textures[MeshBase::TextureType_Diffuse] = tex.texture;
        }
        else if (parseLiteral(ptr, "map_d ") || parseLiteral(ptr, "map_D ") || // alpha texture
                 parseLiteral(ptr, "map_Ka ")) // hack: allow ambient texture as alpha
        {
            TextureSpec tex;
            valid = parseTexture(ptr, tex, dirName);
            mat->textures[MeshBase::TextureType_Alpha] = tex.texture;
        }
        else if (parseLiteral(ptr, "bump ") || parseLiteral(ptr, "map_bump ") || parseLiteral(ptr, "map_Bump ") || // bump map
                 parseLiteral(ptr, "disp ")) // displacement map
        {
            TextureSpec tex;
            valid = parseTexture(ptr, tex, dirName);
            mat->displacementCoef = tex.gain;
            mat->displacementBias = tex.base * tex.gain;
            mat->textures[MeshBase::TextureType_Displacement] = tex.texture;
        }
        else
#if WAVEFRONT_DEBUG
        if (parseLiteral(ptr, "vp ") ||             // parameter space vertex
            parseLiteral(ptr, "Kf ") ||             // transmission color
            parseLiteral(ptr, "illum ") ||          // illumination model
            parseLiteral(ptr, "d -halo ") ||        // orientation-dependent alpha
            parseLiteral(ptr, "sharpness ") ||      // reflection sharpness
            parseLiteral(ptr, "Ni ") ||             // index of refraction
            parseLiteral(ptr, "map_Ks ") ||         // specular texture
            parseLiteral(ptr, "map_Ns ") ||         // glossiness texture
            parseLiteral(ptr, "map_aat ") ||        // texture antialiasing
            parseLiteral(ptr, "decal ") ||          // blended texture
            parseLiteral(ptr, "disp ") ||           // displacement texture
            parseLiteral(ptr, "refl ") ||           // reflection texture
            parseLiteral(ptr, "Km ") ||             // ???
            parseLiteral(ptr, "Tr ") ||             // ???
            parseLiteral(ptr, "Tf ") ||             // ???
            parseLiteral(ptr, "Ke ") ||             // ???
            parseLiteral(ptr, "pointgroup ") ||     // ???
            parseLiteral(ptr, "pointdensity ") ||   // ???
            parseLiteral(ptr, "smooth") ||          // ???
            parseLiteral(ptr, "R "))                // ???
#endif
        {
            valid = true;
        }

        if (!valid)
            setError("Invalid line in Wavefront MTL: '%s'!", line);
    }
}

//------------------------------------------------------------------------

void FW::loadObj(ImportState& s, BufferedInputStream& objIn, const String& dirName)
{
    int submesh = -1;
    int defaultSubmesh = -1;
    while (!hasError())
    {
        const char* line = objIn.readLine(true, true);
        if (!line)
            break;

        const char* ptr = line;
        parseSpace(ptr);
        bool valid = false;

        if (!*ptr || parseLiteral(ptr, "#"))
        {
            valid = true;
        }
        else if (parseLiteral(ptr, "v ") && parseSpace(ptr)) // position vertex
        {
            Vec3f v;
            if (parseFloats(ptr, v.getPtr(), 3) && parseSpace(ptr) && !*ptr)
            {
                s.positions.add(v);
                valid = true;
            }
        }
        else if (parseLiteral(ptr, "vt ") && parseSpace(ptr)) // texture vertex
        {
            Vec2f v;
            if (parseFloats(ptr, v.getPtr(), 2) && parseSpace(ptr))
            {
                F32 dummy;
                while (parseFloat(ptr, dummy) && parseSpace(ptr));

                if (!*ptr)
                {
                    s.texCoords.add(Vec2f(v.x, 1.0f - v.y));
                    valid = true;
                }
            }
        }
        else if (parseLiteral(ptr, "vn ") && parseSpace(ptr)) // normal vertex
        {
            Vec3f v;
            if (parseFloats(ptr, v.getPtr(), 3) && parseSpace(ptr) && !*ptr)
            {
                s.normals.add(v);
                valid = true;
            }
        }
        else if (parseLiteral(ptr, "f ") && parseSpace(ptr)) // face
        {
            Array<S32> verts;
            while (*ptr)
            {
                Vec3i ptn;
                if (!parseInt(ptr, ptn.x))
                    break;
                for (int i = 1; i < 4 && parseLiteral(ptr, "/"); i++)
                {
                    S32 tmp = 0;
                    parseInt(ptr, tmp);
                    if (i < 3)
                        ptn[i] = tmp;
                }
                parseSpace(ptr);

                Vec3i size(s.positions.getSize(), s.texCoords.getSize(), s.normals.getSize());
                for (int i = 0; i < 3; i++)
                {
                    if (ptn[i] > 0)
                        ptn[i]--;
                    else if (ptn[i] < 0)
                        ptn[i] += size[i];
                    else
                        ptn[i] = -1;

                    if (ptn[i] < 0 || ptn[i] >= size[i])
                        ptn[i] = -1;
                }

                VertexPNT v;
                v.p = (ptn.x == -1) ? Vec3f(0.0f) : s.positions[ptn.x];
                v.t = (ptn.y == -1) ? Vec2f(0.0f) : s.texCoords[ptn.y];
                v.n = (ptn.z == -1) ? Vec3f(0.0f) : s.normals[ptn.z];

                S32* idx = s.vertexHash.search(v);
                if (idx)
                    verts.add(*idx);
                else
                {
                    verts.add(s.vertexHash.add(v, s.mesh->numVertices()));
                    s.mesh->addVertex(v);
                }
            }
            if (!*ptr)
            {
                if (submesh == -1)
                {
                    if (defaultSubmesh == -1)
                        defaultSubmesh = s.mesh->addSubmesh();
                    submesh = defaultSubmesh;
                }
                Array<Vec3i>& indices = s.mesh->mutableIndices(submesh);
                for (int i = 2; i < verts.getSize(); i++)
                    indices.add(Vec3i(verts[0], verts[i - 1], verts[i]));
                valid = true;
            }
        }
        else if (parseLiteral(ptr, "usemtl ") && parseSpace(ptr)) // material name
        {
            Material* mat = s.materialHash.search(ptr);
            submesh = -1;
            if (mat)
            {
                if (mat->submesh == -1)
                {
                    mat->submesh = s.mesh->addSubmesh();
                    s.mesh->material(mat->submesh) = *mat;
                }
                submesh = mat->submesh;
            }
            valid = true;
        }
        else if (parseLiteral(ptr, "mtllib ") && parseSpace(ptr) && *ptr) // material library
        {
            if (dirName.getLength())
            {
                String fileName = dirName + "/" + ptr;
                File file(fileName, File::Read);
                BufferedInputStream mtlIn(file);
                loadMtl(s, mtlIn, fileName.getDirName());
#if (!WAVEFRONT_DEBUG)
                clearError();
#endif
            }
            valid = true;
        }
        else
#if WAVEFRONT_DEBUG
        if (parseLiteral(ptr, "vp ") ||         // parameter space vertex
            parseLiteral(ptr, "deg ") ||        // degree
            parseLiteral(ptr, "bmat ") ||       // basis matrix
            parseLiteral(ptr, "step ") ||       // step size
            parseLiteral(ptr, "cstype ") ||     // curve/surface type
            parseLiteral(ptr, "p ") ||          // point
            parseLiteral(ptr, "l ") ||          // line
            parseLiteral(ptr, "curv ") ||       // curve
            parseLiteral(ptr, "curv2 ") ||      // 2d curve
            parseLiteral(ptr, "surf ") ||       // surface
            parseLiteral(ptr, "parm ") ||       // curve/surface parameters
            parseLiteral(ptr, "trim ") ||       // curve/surface outer trimming loop
            parseLiteral(ptr, "hole ") ||       // curve/surface inner trimming loop
            parseLiteral(ptr, "scrv ") ||       // curve/surface special curve
            parseLiteral(ptr, "sp ") ||         // curve/surface special point
            parseLiteral(ptr, "end ") ||        // curve/surface end statement
            parseLiteral(ptr, "con ") ||        // surface connect
            parseLiteral(ptr, "g ") ||          // group name
            parseLiteral(ptr, "s ") ||          // smoothing group
            parseLiteral(ptr, "mg ") ||         // merging group
            parseLiteral(ptr, "o ") ||          // object name
            parseLiteral(ptr, "bevel ") ||      // bevel interpolation
            parseLiteral(ptr, "c_interp ") ||   // color interpolation
            parseLiteral(ptr, "d_interp ") ||   // dissolve interpolation
            parseLiteral(ptr, "lod ") ||        // level of detail
            parseLiteral(ptr, "shadow_obj ") || // shadow casting
            parseLiteral(ptr, "trace_obj ") ||  // ray tracing
            parseLiteral(ptr, "ctech ") ||      // curve approximation technique
            parseLiteral(ptr, "stech ") ||      // surface approximation technique
            parseLiteral(ptr, "g"))             // ???
#endif
        {
            valid = true;
        }

        if (!valid)
            setError("Invalid line in Wavefront OBJ: '%s'!", line);
    }
}

//------------------------------------------------------------------------

Mesh<VertexPNT>* FW::importWavefrontMesh(BufferedInputStream& stream, const String& fileName)
{
    ImportState s;
    s.mesh = new Mesh<VertexPNT>;
    loadObj(s, stream, fileName.getDirName());
    s.mesh->compact();
    return s.mesh;
}

//------------------------------------------------------------------------

void FW::exportWavefrontMesh(BufferedOutputStream& stream, const MeshBase* mesh, const String& fileName)
{
    FW_ASSERT(mesh);
    Mesh<VertexPNT> pnt(*mesh);

    // Extract base name.

    String dirName = fileName.getDirName();
    String baseName = fileName.getFileName();
    int idx = baseName.indexOf('.');
    if (idx != -1)
        baseName = baseName.substring(0, idx);

    // Write OBJ file.

    if (baseName.getLength())
    {
        stream.writef("mtllib %s.mtl\n", baseName.getPtr());
        stream.writef("\n");
    }

    for (int i = 0; i < pnt.numVertices(); i++)
    {
        const VertexPNT& v = pnt.vertex(i);
        stream.writef("v %g %g %g\n", v.p.x, v.p.y, v.p.z);
    }
    stream.writef("\n");

    for (int i = 0; i < pnt.numVertices(); i++)
    {
        const VertexPNT& v = pnt.vertex(i);
        stream.writef("vt %g %g\n", v.t.x, 1.0f - v.t.y);
    }
    stream.writef("\n");

    for (int i = 0; i < pnt.numVertices(); i++)
    {
        const VertexPNT& v = pnt.vertex(i);
        stream.writef("vn %g %g %g\n", v.n.x, v.n.y, v.n.z);
    }

    for (int i = 0; i < pnt.numSubmeshes(); i++)
    {
        stream.writef("\n");
        if (baseName.getLength())
            stream.writef("usemtl %d\n", i);

        const Array<Vec3i>& tris = pnt.indices(i);
        for (int j = 0; j < tris.getSize(); j++)
        {
            Vec3i v = tris[j] + 1;
            stream.writef("f %d/%d/%d %d/%d/%d %d/%d/%d\n",
                v.x, v.x, v.x,
                v.y, v.y, v.y,
                v.z, v.z, v.z);
        }
    }

    // No base name => do not write materials.

    if (!baseName.getLength())
        return;

    // Hash textures and determine file names.

    Hash<const Image*, String> texImageHash;
    Set<String> texNameSet;

    for (int i = 0; i < pnt.numSubmeshes(); i++)
    {
        const MeshBase::Material& mat = pnt.material(i);
        for (int j = 0; j < MeshBase::TextureType_Max; j++)
        {
            const Texture& tex = mat.textures[j];
            if (!tex.exists() || texImageHash.contains(tex.getImage()))
                continue;

            // Extract name from ID.

            String name = tex.getID().getFileName();
            int idx = name.indexOf('.');
            if (idx != -1)
                name = name.substring(0, idx);

            // No name => generate one.

            if (!name.getLength())
                name = sprintf("tex%d", texImageHash.getSize());

            // Ensure that the name is unique.

            String oldName = name;
            for (int k = 0; texNameSet.contains(name); k++)
                name = sprintf("%s_%d", oldName.getPtr(), k);

            // Append format postfix.

            name += ".png";

            // Record.

            texImageHash.add(tex.getImage(), name);
            texNameSet.add(name);
        }
    }

    // Write MTL file.

    File mtlFile(dirName + '/' + baseName +  ".mtl", File::Create);
    BufferedOutputStream mtlOut(mtlFile);
    for (int i = 0; i < pnt.numSubmeshes(); i++)
    {
        if (i)
            mtlOut.writef("\n");

        const MeshBase::Material& mat = pnt.material(i);
        mtlOut.writef("newmtl %d\n", i);
        mtlOut.writef("Ka 0 0 0\n");
        mtlOut.writef("Kd %g %g %g\n", mat.diffuse.x, mat.diffuse.y, mat.diffuse.z);
        mtlOut.writef("d %g\n", mat.diffuse.w);
        mtlOut.writef("Ks %g %g %g\n", mat.specular.x, mat.specular.y, mat.specular.z);
        mtlOut.writef("Ns %g\n", mat.glossiness);

        if (texImageHash.contains(mat.textures[MeshBase::TextureType_Diffuse].getImage()))
            mtlOut.writef("map_Kd %s\n", texImageHash[mat.textures[MeshBase::TextureType_Diffuse].getImage()].getPtr());

        if (texImageHash.contains(mat.textures[MeshBase::TextureType_Alpha].getImage()))
            mtlOut.writef("map_d %s\n", texImageHash[mat.textures[MeshBase::TextureType_Alpha].getImage()].getPtr());

        if (texImageHash.contains(mat.textures[MeshBase::TextureType_Displacement].getImage()))
            mtlOut.writef("disp -mm %g %g %s\n",
                mat.displacementBias / mat.displacementCoef, mat.displacementCoef,
                texImageHash[mat.textures[MeshBase::TextureType_Displacement].getImage()].getPtr());
    }
    mtlOut.flush();

    // Write textures.

    for (int i = texImageHash.firstSlot(); i != -1; i = texImageHash.nextSlot(i))
    {
        const Image* texImage = texImageHash.getSlot(i).key;
        const String& texName = texImageHash.getSlot(i).value;
        exportImage(dirName + '/' + texName, texImage);
    }
}

//------------------------------------------------------------------------
