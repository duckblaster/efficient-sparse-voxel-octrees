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
 
#include "io/MeshBinaryIO.hpp"
#include "3d/Mesh.hpp"
#include "io/Stream.hpp"
#include "io/ImageBinaryIO.hpp"

using namespace FW;

//------------------------------------------------------------------------

MeshBase* FW::importBinaryMesh(InputStream& stream)
{
    MeshBase* mesh = new MeshBase;

    // MeshHeader.

    char formatID[9];
    stream.readFully(formatID, 8);
    formatID[8] = '\0';
    if (String(formatID) != "BinMesh ")
        setError("Not a binary mesh file!");

    S32 version;
    stream >> version;

    int numTex;
    switch (version)
    {
    case 1:     numTex = 0; break;
    case 2:     numTex = MeshBase::TextureType_Alpha + 1; break;
    case 3:     numTex = MeshBase::TextureType_Displacement + 1; break;
    default:    numTex = 0; setError("Unsupported binary mesh version!"); break;
    }

    S32 numAttribs, numVertices, numSubmeshes, numTextures = 0;
    stream >> numAttribs >> numVertices >> numSubmeshes;
    if (version >= 2)
        stream >> numTextures;
    if (numAttribs < 0 || numVertices < 0 || numSubmeshes < 0 || numTextures < 0)
        setError("Corrupt binary mesh data!");

    // Array of AttribSpec.

    for (int i = 0; i < numAttribs && !hasError(); i++)
    {
        S32 type, format, length;
        stream >> type >> format >> length;
        if (type < 0 || format < 0 || format >= MeshBase::AttribFormat_Max || length < 1 || length > 4)
            setError("Corrupt binary mesh data!");
        else
            mesh->addAttrib((MeshBase::AttribType)type, (MeshBase::AttribFormat)format, length);
    }

    // Array of Vertex.

    if (!hasError())
    {
        mesh->resetVertices(numVertices);
        stream.readFully(mesh->getMutableVertexPtr(), numVertices * mesh->vertexStride());
    }

    // Array of Texture.

    Array<Texture> textures(NULL, numTextures);
    for (int i = 0; i < numTextures && !hasError(); i++)
    {
        String id;
        stream >> id;
        Image* image = importBinaryImage(stream);
        textures[i] = Texture::find(id);
        if (textures[i].exists())
            delete image;
        else
            textures[i] = Texture(image, id);
    }

    // Array of Submesh.

    for (int i = 0; i < numSubmeshes && !hasError(); i++)
    {
        mesh->addSubmesh();
        MeshBase::Material& mat = mesh->material(i);
        Vec3f ambient;
        stream >> ambient >> mat.diffuse >> mat.specular >> mat.glossiness;
        if (version >= 3)
            stream >> mat.displacementCoef >> mat.displacementBias;

        for (int j = 0; j < numTex; j++)
        {
            S32 texIdx;
            stream >> texIdx;
            if (texIdx < -1 || texIdx >= numTextures)
                setError("Corrupt binary mesh data!");
            else if (texIdx != -1)
                mat.textures[j] = textures[texIdx];
        }

        S32 numTriangles;
        stream >> numTriangles;
        if (numTriangles < 0)
            setError("Corrupt binary mesh data!");
        else
        {
            Array<Vec3i>& inds = mesh->mutableIndices(i);
            inds.reset(numTriangles);
            stream.readFully(inds.getPtr(), inds.getNumBytes());
        }
    }

    // Handle errors.

    if (hasError())
    {
        delete mesh;
        return NULL;
    }
    return mesh;
}

//------------------------------------------------------------------------

void FW::exportBinaryMesh(OutputStream& stream, const MeshBase* mesh)
{
    FW_ASSERT(mesh);

    // Collapse duplicate textures.

    int numTex = MeshBase::TextureType_Displacement + 1;
    Array<Texture> textures;
    Hash<const Image*, S32> texHash;
    texHash.add(NULL, -1);

    for (int i = 0; i < mesh->numSubmeshes(); i++)
    {
        const MeshBase::Material& mat = mesh->material(i);
        for (int j = 0; j < numTex; j++)
        {
            const Image* key = mat.textures[j].getImage();
            if (texHash.contains(key))
                continue;

            texHash.add(key, textures.getSize());
            textures.add(mat.textures[j]);
        }
    }

    // MeshHeader.

    stream.write("BinMesh ", 8);
    stream << (S32)3 << (S32)mesh->numAttribs() << (S32)mesh->numVertices() << (S32)mesh->numSubmeshes() << (S32)textures.getSize();

    // Array of AttribSpec.

    for (int i = 0; i < mesh->numAttribs(); i++)
    {
        const MeshBase::AttribSpec& spec = mesh->attribSpec(i);
        stream << (S32)spec.type << (S32)spec.format << spec.length;
    }

    // Array of Vertex.

    stream.write(mesh->getVertexPtr(), mesh->numVertices() * mesh->vertexStride());

    // Array of Texture.

    for (int i = 0; i < textures.getSize(); i++)
    {
        stream << textures[i].getID();
        exportBinaryImage(stream, textures[i].getImage());
    }

    // Array of Submesh.

    for (int i = 0; i < mesh->numSubmeshes(); i++)
    {
        const Array<Vec3i>& inds = mesh->indices(i);
        const MeshBase::Material& mat = mesh->material(i);
        stream << Vec3f(0.0f) << mat.diffuse << mat.specular << mat.glossiness;
        stream << mat.displacementCoef << mat.displacementBias;

        for (int j = 0; j < numTex; j++)
            stream << texHash[mat.textures[j].getImage()];
        stream << (S32)inds.getSize();
        stream.write(inds.getPtr(), inds.getNumBytes());
    }
}

//------------------------------------------------------------------------
