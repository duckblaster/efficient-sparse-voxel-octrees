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
#include "base/Timer.hpp"
#include "io/OctreeFile.hpp"
#include "io/OctreeRuntime.hpp"
#include "gpu/CudaCompiler.hpp"
#include "cuda/Ambient.hpp"

namespace FW
{
//------------------------------------------------------------------------

class AmbientProcessor
{
public:
    enum
    {
            DefaultRaysPerNode  = 256,
            MinRaysPerBatch     = 512 << 10,
            MaxRaysPerBatch     = 2048 << 10,
            NumSlicesToPrefetch = 8
    };

public:
                        AmbientProcessor    (OctreeFile* file, int objectID);
                        ~AmbientProcessor   (void);

    void                setRayLength        (F32 length)    { m_rayLength = length; }
    void                setFlipNormals      (bool enable)   { m_flipNormals = enable; }

    void                run                 (void);

private:
    struct NodeInfo
    {
        Vec3i           pos;
        U32             validMask;
        bool            lastInStrip;
        bool            secondInPair;
    };

    struct SliceTask
    {
        OctreeSlice*    slice;
        Array<NodeInfo> nodes;
        S32*            attachData;
    };

    void                        processSlice        (OctreeSlice* slice, const Array<NodeInfo>& nodes);
    void                        initiateProcessing  (void);
    void                        finishProcessing    (void);

    S32&                        getWarpCounter      (void) { return *(S32*)m_module->getGlobal("g_warpCounter").getMutablePtr(); }
    AmbientInput&               getInput            (void) { return *(AmbientInput*)m_module->getGlobal("c_input").getMutablePtr(); }

private:
                                AmbientProcessor    (AmbientProcessor&); // forbidden
    AmbientProcessor&           operator=           (AmbientProcessor&); // forbidden

private:
    CudaCompiler                m_compiler;
    CudaModule*                 m_module;

    OctreeFile*                 m_file;
    int                         m_objectID;
    int                         m_raysPerNode;
    float                       m_rayLength;
    bool                        m_flipNormals;
    OctreeRuntime*              m_runtime;
    int                         m_numWarps;
    Buffer                      m_activeWarps;
    Buffer                      m_requestBuffer;
    Buffer                      m_resultBuffer;
    double                      m_kernelTime;
    CUevent                     m_kernelStartEvent;
    CUevent                     m_kernelEndEvent;
    S64                         m_requestsProcessed;
    Array<SliceTask*>           m_sliceTasks;
    int                         m_sliceTaskTotal;
    Array<SliceTask*>           m_procTasks;
    int                         m_procTaskTotal;
};

//------------------------------------------------------------------------
}
