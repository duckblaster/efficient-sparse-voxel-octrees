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

namespace FW
{

//------------------------------------------------------------------------

#define RCK_TRACE_BLOCK_WIDTH   32
#define RCK_TRACE_BLOCK_HEIGHT  2

//------------------------------------------------------------------------

enum RenderFlags
{
    RenderFlags_CoarsePass              = 1 << 0,
    RenderFlags_UseCoarseData           = 1 << 1,
    RenderFlags_VisualizeIterations     = 1 << 2,
    RenderFlags_VisualizeRaycastLevel   = 1 << 3,
};

//------------------------------------------------------------------------

enum AttachSlot
{
    AttachSlot_Contour = 0,
    AttachSlot_Attribute,
    AttachSlot_AO,

    AttachSlot_Max
};

//------------------------------------------------------------------------
// PerfCounter_Instructions counts SASS instructions, excluding the
// following cases where dual issue is assumed:
//
//      - MOV between registers
//      - FMUL between registers
//      - FRCP of a register
//------------------------------------------------------------------------

#define PERF_COUNTER_LIST(X) \
    X(Instructions) \
    X(Iterations) \
    X(Intersect) \
    X(Push) \
    X(PushStore) \
    X(Advance) \
    X(Pop) \
    X(GlobalAccesses) \
    X(GlobalBytes) \
    X(GlobalTransactions) \
    X(LocalAccesses) \
    X(LocalBytes) \
    X(LocalTransactions)

enum PerfCounter
{
#define X(NAME) PerfCounter_ ## NAME,
    PERF_COUNTER_LIST(X)
#undef X
    PerfCounter_Max
};

//------------------------------------------------------------------------

struct OctreeMatrices
{
    Mat4f           viewportToCamera;
    Mat4f           cameraToOctree;
    Mat4f           octreeToWorld;
    F32             pixelInOctree;      // average size of a near-plane-pixel in the octree

    Mat4f           worldToOctree;
    Mat3f           octreeToWorldN;     // normal transformation matrix
    Vec3f           cameraPosition;     // camera position in world space
    Mat4f           octreeToViewport;
    Mat4f           viewportToOctreeN;  // matrix for transforming frustum planes
};

//------------------------------------------------------------------------

struct RenderInput
{
    Vec2i           frameSize;
    U32             flags;
    S32             batchSize;          // number of warps per batch
    S32             aaRays;             // number of AA rays per pixel for normal pass
    F32             maxVoxelSize;       // in pixels
    F32             brightness;
    S32             coarseSize;         // block size for coarse data
    Vec2i           coarseFrameSize;    // coarse data buffer size
    S32             numPrimaryRays;     // includes aa rays
    S32             totalWork;          // numPrimaryRays * numFrameRepeats
    CUdeviceptr     frame;
    CUdeviceptr     frameCoarse;        // contains tmin as float
    CUdeviceptr     aaSampleBuffer;     // individual aa samples
    CUdeviceptr     rootNode;
    CUdeviceptr     activeWarps;
    CUdeviceptr     perfCounters;
    OctreeMatrices  octreeMatrices;
};

//------------------------------------------------------------------------
}
