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
#include "BenchmarkContext.hpp"
#include "cuda/Render.hpp"

namespace FW
{

class Benchmark
{
public:
    struct Result
    {
        String          title;
        F32             mraysPerSecActual;
        F32             mraysPerSecWallclock;
        F32             mraysPerSecSimulated;
        F32             gigsPerSec;
        F32             renderWarps;
        F32             coarseWarps;
        F32             coarsePassPct;
        F32             threadCountersPerRay[PerfCounter_Max];
        F32             warpCountersPerRay[PerfCounter_Max];
    };

public:
                        Benchmark               (void);
                        ~Benchmark              (void);

    void                setFrameSize            (const Vec2i& value)            { m_frameSize = value; }
    void                setFramesPerLaunch      (S32 value)                     { m_framesPerLaunch = value; }
    void                setWarmupLaunches       (S32 value)                     { m_warmupLaunches = value; }
    void                setMeasureFrames        (S32 value)                     { m_measureFrames = value; }

    void                loadOctree              (const String& fileName, int numLevels = OctreeFile::UnitScale) { m_ctx.load(fileName, numLevels); }
    void                setCameras              (const Array<String>& value)    { m_cameras = value; }

    void                clearResults            (void)                          { m_results.clear(); }
    void                measure                 (const String& columnTitle, const CudaRenderer::Params& renderParams);
    void                printResults            (const String& majorTitle, const String& minorTitle);

private:
                        Benchmark               (const Benchmark&); // forbidden
    Benchmark&          operator=               (const Benchmark&); // forbidden

private:
    BenchmarkContext    m_ctx;

    Vec2i               m_frameSize;
    S32                 m_framesPerLaunch;
    S32                 m_warmupLaunches;
    S32                 m_measureFrames;
    Array<String>       m_cameras;

    Array<Result>       m_results;
};

//------------------------------------------------------------------------
}
