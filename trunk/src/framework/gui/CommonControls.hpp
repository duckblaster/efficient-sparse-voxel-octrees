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
#include "gui/Window.hpp"
#include "base/Timer.hpp"
#include "base/Array.hpp"
#include "base/Hash.hpp"

namespace FW
{
//------------------------------------------------------------------------

class StateDump;

//------------------------------------------------------------------------

class CommonControls : public Window::Listener
{
public:

    //------------------------------------------------------------------------

    enum Feature
    {
        Feature_CloseOnEsc          = 1 << 0,
        Feature_CloseOnAltF4        = 1 << 1,
        Feature_RepaintOnF5         = 1 << 2,
        Feature_ShowFPSOnF9         = 1 << 3,
        Feature_HideControlsOnF10   = 1 << 4,
        Feature_FullScreenOnF11     = 1 << 5,
        Feature_ScreenshotOnPrtScn  = 1 << 6,
        Feature_LoadStateOnNum      = 1 << 7,
        Feature_SaveStateOnAltNum   = 1 << 8,

        Feature_None                = 0,
        Feature_All                 = (1 << 9) - 1,
        Feature_Default             = Feature_All
    };

    //------------------------------------------------------------------------

    class StateObject
    {
    public:
                            StateObject         (void)          {}
        virtual             ~StateObject        (void)          {}

        virtual void        readState           (StateDump& d) = 0;
        virtual void        writeState          (StateDump& d) const = 0;
    };

private:
    struct Key;

    //------------------------------------------------------------------------

    struct Message
    {
        String          string;
        String          volatileID;
        F32             highlightTime;
    };

    //------------------------------------------------------------------------

    struct Toggle
    {
        bool*           boolTarget;
        S32*            enumTarget;
        S32             enumValue;
        bool            isButton;
        bool            isSeparator;
        Key*            key;
        String          title;
        F32             highlightTime;

        bool            visible;
        Vec2f           pos;
        Vec2f           size;
    };

    //------------------------------------------------------------------------

    struct Slider
    {
        F32*            floatTarget;
        S32*            intTarget;
        F32             slack;
        F32             minValue;
        F32             maxValue;
        bool            isExponential;
        Key*            increaseKey;
        Key*            decreaseKey;
        String          format;
        F32             speed;
        F32             highlightTime;

        bool            visible;
        bool            stackWithPrevious;
        Vec2f           pos;
        Vec2f           size;
        Vec2f           blockPos;
        Vec2f           blockSize;
    };

    //------------------------------------------------------------------------

    struct Key
    {
        String          id;
        Array<Toggle*>  toggles;
        Array<Slider*>  sliderIncrease;
        Array<Slider*>  sliderDecrease;
    };

    //------------------------------------------------------------------------

public:
                    CommonControls      (U32 features = Feature_Default);
    virtual         ~CommonControls     (void);

    virtual bool    handleEvent         (const Window::Event& ev);

    void            message             (const String& str, const String& volatileID = "");

    void            addToggle           (bool* target, const String& key, const String& title)              { FW_ASSERT(target); addToggle(target, NULL, 0, false, key, title); }
    void            addToggle           (S32* target, S32 value, const String& key, const String& title)    { FW_ASSERT(target); addToggle(NULL, target, value, false, key, title); }
    void            addButton           (bool* target, const String& key, const String& title)              { FW_ASSERT(target); addToggle(target, NULL, 0, true, key, title); }
    void            addButton           (S32* target, S32 value, const String& key, const String& title)    { FW_ASSERT(target); addToggle(NULL, target, value, true, key, title); }
    void            addSeparator        (void)                                                              { addToggle(NULL, NULL, 0, false, "", ""); }
    void            setControlVisibility(bool visible)              { m_controlVisibility = visible; } // applies to next addXxx()

    void            addSlider           (F32* target, F32 minValue, F32 maxValue, bool isExponential, const String& increaseKey, const String& decreaseKey, const String& format, F32 speed = 0.25f) { addSlider(target, NULL, minValue, maxValue, isExponential, increaseKey, decreaseKey, format, speed); }
    void            addSlider           (S32* target, S32 minValue, S32 maxValue, bool isExponential, const String& increaseKey, const String& decreaseKey, const String& format, F32 speed = 0.0f) { addSlider(NULL, target, (F32)minValue, (F32)maxValue, isExponential, increaseKey, decreaseKey, format, speed); }
    void            beginSliderStack    (void)                      { m_sliderStackBegun = true; m_sliderStackEmpty = true; }
    void            endSliderStack      (void)                      { m_sliderStackBegun = false; }

    void            removeControl       (const void* target);
    void            resetControls       (void);

    void            setStateFilePrefix  (const String& prefix)      { m_stateFilePrefix = prefix; }
    void            setScreenshotFilePrefix(const String& prefix)   { m_screenshotFilePrefix = prefix; }
    String          getStateFileName    (int idx) const             { return sprintf("%s%03d.dat", m_stateFilePrefix.getPtr(), idx); }
    String          getScreenshotFileName(void) const;

    void            addStateObject      (StateObject* obj)          { if (obj && !m_stateObjs.contains(obj)) m_stateObjs.add(obj); }
    void            removeStateObject   (StateObject* obj)          { m_stateObjs.removeItem(obj); }
    bool            loadState           (const String& fileName);
    bool            saveState           (const String& fileName);
    bool            loadStateDialog     (void);
    bool            saveStateDialog     (void);

    void            showControls        (bool show)                 { m_showControls = show; }
    void            showFPS             (bool show)                 { m_showFPS = show; }

    F32             getKeyBoost         (void) const;

private:
    bool            hasFeature          (Feature feature)           { return ((m_features & feature) != 0); }
    void            render              (GLContext* gl);

    void            addToggle           (bool* boolTarget, S32* enumTarget, S32 enumValue, bool isButton, const String& key, const String& title);
    void            addSlider           (F32* floatTarget, S32* intTarget, F32 minValue, F32 maxValue, bool isExponential, const String& increaseKey, const String& decreaseKey, const String& format, F32 speed);
    Key*            getKey              (const String& id);

    void            layout              (const Vec2f& viewSize, F32 fontHeight);
    void            clearActive         (void);
    void            updateActive        (const Vec2f& mousePos);
    F32             updateHighlightFade (F32* highlightTime);
    U32             fadeABGR            (U32 abgr, F32 fade);

    static void     selectToggle        (Toggle* t);

    F32             getSliderY          (const Slider* s, bool applySlack) const;
    void            setSliderY          (Slider* s, F32 y);
    static F32      getSliderValue      (const Slider* s, bool applySlack);
    static void     setSliderValue      (Slider* s, F32 v);
    static String   getSliderLabel      (const Slider* s);
    static void     sliderKeyDown       (Slider* s, int dir);

    static void     drawPanel           (GLContext* gl, const Vec2f& pos, const Vec2f& size, U32 interiorABGR, U32 topLeftABGR, U32 bottomRightABGR);

private:
                    CommonControls      (const CommonControls&); // forbidden
    CommonControls& operator=           (const CommonControls&); // forbidden

private:
    const U32       m_features;
    Window*         m_window;
    Timer           m_timer;

    bool            m_showControls;
    bool            m_showFPS;
    String          m_stateFilePrefix;
    String          m_screenshotFilePrefix;

    Array<Message>  m_messages;
    Array<Toggle*>  m_toggles;
    Array<Slider*>  m_sliders;
    Array<StateObject*> m_stateObjs;
    Hash<String, Key*> m_keyHash;

    bool            m_sliderStackBegun;
    bool            m_sliderStackEmpty;
    bool            m_controlVisibility;

    Vec2f           m_viewSize;
    F32             m_fontHeight;
    F32             m_rightX;

    S32             m_activeSlider;
    S32             m_activeToggle;
    bool            m_dragging;
    F32             m_avgFrameTime;
    bool            m_screenshot;
};

//------------------------------------------------------------------------
}
