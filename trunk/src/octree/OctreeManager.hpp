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
#include "io/OctreeFile.hpp"
#include "io/OctreeRuntime.hpp"
#include "build/BuilderBase.hpp"
#include "render/CudaRenderer.hpp"
#include "base/Timer.hpp"

namespace FW
{
//------------------------------------------------------------------------

class OctreeManager
{
public:
    enum
    {
        RuntimeSlackBytes       = 10 << 20,   // Minimum amount of free runtime memory.
        UpdateTimePct           = 50,
        UpdateTimeMaxMillis     = 200,
        MaxPrefetchSlices       = OctreeFile::MaxPrefetchSlices,
        MaxPrefetchBytesTotal   = OctreeFile::MaxPrefetchBytesTotal,
        MaxPrefetchBytesPending = 8 << 20,
        MaxAsyncBuildSlices     = BuilderBase::MaxAsyncBuildSlices
    };

    enum RenderMode
    {
        RenderMode_Mesh = 0,
        RenderMode_Cuda = 2,
    };

    enum BuilderType
    {
        BuilderType_Mesh = 0,

        BuilderType_Max
    };

public:
                        OctreeManager       (RenderMode renderMode = RenderMode_Mesh);
                        ~OctreeManager      (void);

    void                setMaxConcurrency   (int maxBuilderThreads) { FW_ASSERT(maxBuilderThreads > 0); m_maxBuilderThreads = maxBuilderThreads; }

    void                setRenderMode       (RenderMode renderMode);
    void                setDynamicLoad      (bool dynamicLoad)  { m_dynamicLoad = dynamicLoad; }
    void                setDynamicBuild     (bool dynamicBuild) { m_dynamicBuild = dynamicBuild; }
    void                setMaxLevels        (int maxLevels)     { m_maxLevels = maxLevels; }

    OctreeFile*         getFile             (void);
    OctreeRuntime*      getRuntime          (void);
    CudaRenderer*       getCudaRenderer     (void);
    BuilderBase*        getBuilder          (BuilderType type);

    void                clearRuntime        (void);
    void                destroyRuntime      (void);

    void                newFile             (void);
    void                loadFile            (const String& fileName, bool edit = false);
    void                saveFile            (const String& fileName, bool edit = false);
    void                editFile            (bool inPlace = false);
    bool                isEditable          (void) const        { return (!m_file || m_file->getMode() != File::Read); }
    void                rebuildFile         (BuilderType builderType, const BuilderBase::Params& params, int numLevelsToBuild = 0, const String& saveFileName = "");

    int                 addMesh             (MeshBase* mesh, BuilderType builderType, const BuilderBase::Params& params, int numLevelsToBuild = 0);

    void                renderObject        (GLContext* gl, int objectID, const Mat4f& worldToCamera, const Mat4f& projection);

    String              getStats            (void) const;

private:
    static int          allocateTmpFileID   (void);
    static void         freeTmpFileID       (int id);

    void                unloadFile          (bool freeID);

    void                renderInternal      (GLContext* gl, int objectID, const Mat4f& worldToCamera, const Mat4f& projection, F32 frameDelta);

    void                updateRuntime       (int objectID, const Vec3f& cameraInOctree, F32 timeLimit);
    void                prefetchSlices      (const Array<OctreeRuntime::FindResult>& slices, Timer& timer, F32 timeLimit);
    bool                loadSlices          (const Array<OctreeRuntime::FindResult>& slices, Timer& timer, F32 timeLimit, int objectID, const Vec3f& cameraInOctree);
    bool                buildSlices         (const Array<OctreeRuntime::FindResult>& slices, Timer& timer, F32 timeLimit);

private:
                        OctreeManager       (OctreeManager&); // forbidden
    OctreeManager&      operator=           (OctreeManager&); // forbidden

private:
    static S32          s_numTmpFileIDs;
    static S32          s_maxTmpFileIDs;
    static Array<S32>   s_freeTmpFileIDs;

    S32                 m_maxBuilderThreads;

    RenderMode          m_renderMode;
    bool                m_dynamicLoad;
    bool                m_dynamicBuild;
    S32                 m_maxLevels;

    S32                 m_tmpFileID;        // -1 if none
    OctreeFile*         m_file;
    BuilderBase*        m_builders[BuilderType_Max];

    OctreeRuntime*      m_cpuRuntime;
    OctreeRuntime*      m_cudaRuntime;
    CudaRenderer*       m_cudaRenderer;

    S32                 m_loadSliceID;
    S32                 m_loadSliceBytesDisk;
    S32                 m_loadSliceBytesMemory;

    Timer               m_frameDeltaTimer;
    Timer               m_frameTimer;
    Timer               m_updateTimer;
    Timer               m_renderTimer;
    F32                 m_loadBytesTotal;

    F32                 m_frameDeltaAvg;
    F32                 m_frameTimeAvg;
    F32                 m_updateTimeAvg;
    F32                 m_renderTimeAvg;
    F32                 m_loadBytesAvg;
};

//------------------------------------------------------------------------
}
