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
#include "render/CudaRenderer.hpp"
#include "build/BuilderBase.hpp"
#include "base/Timer.hpp"
#include "gui/Window.hpp"
#include "gui/Image.hpp"
#include "3d/CameraControls.hpp"

namespace FW
{
//------------------------------------------------------------------------

class BenchmarkContext : public Window::Listener
{
public:
    enum
    {
        MaxPrefetchBytesTotal   = OctreeFile::MaxPrefetchBytesTotal
    };

public:
                        BenchmarkContext    (void);
    virtual             ~BenchmarkContext   (void);

    OctreeFile*         getFile             (void) const                { return m_file; }
    OctreeRuntime*      getRuntime          (void) const                { return m_runtime; }
    CudaRenderer*       getRenderer         (void) const                { return m_renderer; }

    void                setFile             (const String& fileName);
    void                clearRuntime        (void);
    void                load                (int numLevels = OctreeFile::UnitScale);

    void                setCamera           (const String& signature)   { m_camera.decodeSignature(signature); failIfError(); }
    Mat4f               getOctreeToWorld    (int objectID = 0) const;
    Mat4f               getWorldToCamera    (void) const                { return m_camera.getWorldToCamera(); }
    Mat4f               getProjection       (const Vec2i& frameSize) const;
    void                renderOctree        (Image& image, int objectID = 0) const;

    void                setWindowTitle      (const String& title)       { m_window->setTitle(title); }
    void                showImage           (Image& image);
    void                showOctree          (const Vec2i& frameSize, int objectID = 0);
    void                hideWindow          (void);

    virtual bool        handleEvent         (const Window::Event& ev);

    static BuilderBase::Params readBuildParams(const String& stateFile);

private:
                        BenchmarkContext    (const BenchmarkContext&); // forbidden
    BenchmarkContext&   operator=           (const BenchmarkContext&); // forbidden

private:
    OctreeFile*         m_file;
    OctreeRuntime*      m_runtime;
    CudaRenderer*       m_renderer;

    S32                 m_numLevels;

    CameraControls      m_camera;
    Window*             m_window;
    Image*              m_image;
};

//------------------------------------------------------------------------
}
