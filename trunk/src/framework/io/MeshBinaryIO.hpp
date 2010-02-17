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

namespace FW
{
//------------------------------------------------------------------------

class MeshBase;
class InputStream;
class OutputStream;

//------------------------------------------------------------------------

MeshBase*   importBinaryMesh    (InputStream& stream);
void        exportBinaryMesh    (OutputStream& stream, const MeshBase* mesh);

//------------------------------------------------------------------------
/*

Binary mesh file format v3
--------------------------

- the basic units of data are 32-bit little-endian ints and floats

BinaryMesh
    0       6       struct  v1  MeshHeader
    6       n*3     struct  v1  array of AttribSpec (MeshHeader.numAttribs)
    ?       n*?     struct  v1  array of Vertex (MeshHeader.numVertices)
    ?       n*?     struct  v2  array of Texture (MeshHeader.numTextures)
    ?       n*?     struct  v1  array of Submesh (MeshHeader.numSubmeshes)
    ?

MeshHeader
    0       2       bytes   v1  formatID (must be "BinMesh ")
    2       1       int     v1  formatVersion (must be 2)
    3       1       int     v1  numAttribs
    4       1       int     v1  numVertices
    5       1       int     v2  numTextures
    6       1       int     v1  numSubmeshes
    7

AttribSpec
    0       1       int     v1  type (see MeshBase::AttribType)
    1       1       int     v1  format (see MeshBase::AttribFormat)
    2       1       int     v1  length
    3

Vertex
    0       ?       bytes   v1  array of values (dictated by the set of AttribSpecs)
    ?

Texture
    0       1       int     v2  idLength
    1       ?       bytes   v2  idString
    ?       ?       struct  v2  BinaryImage (see Image.hpp)
    ?

Submesh
    0       3       float   v1  ambient (ignored)
    3       4       float   v1  diffuse
    7       3       float   v1  specular
    10      1       float   v1  glossiness
    11      1       float   v3  displacementCoef
    12      1       float   v3  displacementBias
    13      1       int     v2  diffuseTexture (-1 if none)
    14      1       int     v2  alphaTexture (-1 if none)
    15      1       int     v3  dispTexture (-1 if none)
    16      1       int     v1  numTriangles
    17      n*3     int     v1  indices
    ?

*/
//------------------------------------------------------------------------
}
