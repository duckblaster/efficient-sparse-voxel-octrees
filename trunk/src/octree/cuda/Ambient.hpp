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
#include "base/Math.hpp"
#include "base/DLLImports.hpp"
#include "Render.hpp" // for a number of tracing constants

namespace FW
{

//------------------------------------------------------------------------

#define AMBK_BLOCK_WIDTH        32
#define AMBK_BLOCK_HEIGHT       2

//------------------------------------------------------------------------

struct AmbientRequest
{
    Vec3i           pos;                // node position
    S32             level;              // node level (child level)
};

struct AmbientResult
{
    Vec3f           ao;                 // ao result
};

struct AmbientInput
{
    S32             numRequests;        // number of nodes to process
    S32             raysPerNode;        // number of AO rays to cast, maximum is 256
    F32             rayLength;          // ray length (scene size is 1.0)
    CUdeviceptr     requestPtr;         // requests be here
    CUdeviceptr     resultPtr;          // results go here
    CUdeviceptr     rootNode;           // hierarchy root node
    CUdeviceptr     activeWarps;        // for tracking active warps
    OctreeMatrices  octreeMatrices;
};

//------------------------------------------------------------------------
}
