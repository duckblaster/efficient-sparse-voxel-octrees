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
#include "gui/CommonControls.hpp"
#include "base/Timer.hpp"

namespace FW
{
//------------------------------------------------------------------------

class MeshBase;

//------------------------------------------------------------------------

class CameraControls : public Window::Listener, public CommonControls::StateObject
{
public:

    //------------------------------------------------------------------------

    enum Feature
    {
        Feature_AlignYButton        = 1 << 0,
        Feature_AlignZButton        = 1 << 1,
        Feature_KeepAlignToggle     = 1 << 2,
        Feature_SpeedSlider         = 1 << 3,
        Feature_FOVSlider           = 1 << 4,
        Feature_NearSlider          = 1 << 5,
        Feature_FarSlider           = 1 << 6,
        Feature_StereoControls      = 1 << 7,

        Feature_None                = 0,
        Feature_All                 = (1 << 8) - 1,
        Feature_Default             = Feature_All & ~Feature_StereoControls
    };

    //------------------------------------------------------------------------

public:
                        CameraControls      (CommonControls* commonControls = NULL, U32 features = Feature_Default);
    virtual             ~CameraControls     (void);

    virtual bool        handleEvent         (const Window::Event& ev); // must be before the listener that queries things
    virtual void        readState           (StateDump& d);
    virtual void        writeState          (StateDump& d) const;

    const Vec3f&        getPosition         (void) const        { return m_position; }
    void                setPosition         (const Vec3f& v)    { m_position = v; repaint(); }
    const Vec3f&        getForward          (void) const        { return m_forward; }
    void                setForward          (const Vec3f& v)    { m_forward = v; repaint(); }
    const Vec3f&        getUp               (void) const        { return m_up; }
    void                setUp               (const Vec3f& v)    { m_up = v; repaint(); }
    bool                getKeepAligned      (void) const        { return m_keepAligned; }
    void                setKeepAligned      (bool v)            { m_keepAligned = v; }
    F32                 getSpeed            (void) const        { return m_speed; }
    void                setSpeed            (F32 v)             { m_speed = v; }
    F32                 getFOV              (void) const        { return m_fov; }
    void                setFOV              (F32 v)             { m_fov = v; repaint(); }
    F32                 getNear             (void) const        { return m_near; }
    void                setNear             (F32 v)             { m_near = v; repaint(); }
    F32                 getFar              (void) const        { return m_far; }
    void                setFar              (F32 v)             { m_far = v; repaint(); }

    Mat3f               getOrientation      (void) const;
    Mat4f               getCameraToWorld    (void) const;
    Mat4f               getWorldToCamera    (void) const;
    Mat4f               getCameraToClip     (void) const        { return Mat4f::perspective(m_fov, m_near, m_far); }
    Mat4f               getWorldToClip      (void) const        { return getCameraToClip() * getWorldToCamera(); }
    Mat4f               getCameraToLeftEye  (void) const        { Mat4f m; m.m02 = m_stereoConvergence; m.m03 = m_stereoSeparation; return m; }
    Mat4f               getCameraToRightEye (void) const        { return getCameraToLeftEye().inverted(); }

    void                initDefaults        (void);
    void                initForMesh         (const MeshBase* mesh);

    String              encodeSignature     (void) const;
    void                decodeSignature     (const String& sig);

    void                addGUIControls      (void);             // done automatically on Window.addListener(this)
    void                removeGUIControls   (void);

private:
    bool                hasFeature          (Feature feature)   { return ((m_features & feature) != 0); }
    void                repaint             (void)              { if (m_window) m_window->repaint(); }

    static void         encodeBits          (String& dst, U32 v);
    static U32          decodeBits          (const char*& src);

    static void         encodeFloat         (String& dst, F32 v);
    static F32          decodeFloat         (const char*& src);

    static void         encodeDirection     (String& dst, const Vec3f& v);
    static Vec3f        decodeDirection     (const char*& src);

private:
                        CameraControls      (const CameraControls&); // forbidden
    CameraControls&     operator=           (const CameraControls&); // forbidden

private:
    CommonControls*     m_commonControls;
    U32                 m_features;
    Window*             m_window;
    Timer               m_timer;

    Vec3f               m_position;
    Vec3f               m_forward;
    Vec3f               m_up;

    bool                m_keepAligned;
    F32                 m_speed;
    F32                 m_fov;
    F32                 m_near;
    F32                 m_far;

    bool                m_dragLeft;
    bool                m_dragMiddle;
    bool                m_dragRight;
    bool                m_alignY;
    bool                m_alignZ;

    bool                m_enableStereo;
    F32                 m_stereoSeparation;
    F32                 m_stereoConvergence;
};

//------------------------------------------------------------------------
}
