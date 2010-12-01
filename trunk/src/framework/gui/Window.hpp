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
#include "base/Array.hpp"
#include "base/String.hpp"
#include "base/Math.hpp"
#include "base/Hash.hpp"
#include "gui/Keys.hpp"
#include "base/DLLImports.hpp"
#include "gpu/GLContext.hpp"

namespace FW
{
//------------------------------------------------------------------------

class Window
{
public:

    //------------------------------------------------------------------------

    enum EventType
    {
        EventType_AddListener,      /* Listener has been added to a window. */
        EventType_RemoveListener,   /* Listener has been removed from a window. */
        EventType_Close,            /* User has tried to close the window. */
        EventType_Resize,           /* The window has been resized. */
        EventType_KeyDown,          /* User has pressed a key (or mouse button). */
        EventType_KeyUp,            /* User has released a key (or mouse button). */
        EventType_Char,             /* User has typed a character. */
        EventType_Mouse,            /* User has moved the mouse. */
        EventType_Paint,            /* Window contents need to be painted. */
        EventType_PrePaint,         /* Before processing EventType_Paint. */
        EventType_PostPaint,        /* After processing EventType_Paint. */
    };

    //------------------------------------------------------------------------

    struct Event
    {
        EventType           type;
        String              key;            /* Empty unless EventType_KeyDown or EventType_KeyUp. */
        S32                 keyUnicode;     /* 0 unless EventType_KeyDown or EventType_KeyUp or if special key. */
        S32                 chr;            /* Zero unless EventType_Text. */
        bool                mouseKnown;     /* Unchanged unless EventType_Mouse. */
        Vec2i               mousePos;       /* Unchanged unless EventType_Mouse. */
        Vec2i               mouseDelta;     /* Zero unless EventType_Mouse. */
        bool                mouseDragging;  /* One or more mouse buttons are down. */
        Window*             window;
    };

    //------------------------------------------------------------------------

    class Listener
    {
    public:
                            Listener            (void)          {}
        virtual             ~Listener           (void)          {}

        virtual bool        handleEvent         (const Event& ev) = 0;
    };

    //------------------------------------------------------------------------

public:
                            Window              (void);
                            ~Window             (void);

    void                    setTitle            (const String& title);

    void                    setSize             (const Vec2i& size);
    Vec2i                   getSize             (void) const;

    void                    setVisible          (bool visible);
    bool                    isVisible           (void) const    { return m_isVisible; }
    void                    setFullScreen       (bool isFull);
    bool                    isFullScreen        (void) const    { return m_isFullScreen; }
    void                    toggleFullScreen    (void)          { setFullScreen(!isFullScreen()); }
    void                    realize             (void);

    void                    setGLConfig         (const GLContext::Config& config);
    const GLContext::Config& getGLConfig        (void) const    { return m_glConfig; }
    GLContext*              getGL               (void); // also makes the context current

    void                    repaint             (void);
    void                    repaintNow          (void);
    void                    requestClose        (void);

    void                    addListener         (Listener* listener);
    void                    removeListener      (Listener* listener);

    bool                    isKeyDown           (const String& key) const { return m_keysDown.contains(key); }
    bool                    isMouseKnown        (void) const    { return m_mouseKnown; }
    bool                    isMouseDragging     (void) const    { return (m_mouseDragCount != 0); }
    const Vec2i&            getMousePos         (void) const    { return m_mousePos; }

    void                    showMessageDialog   (const String& title, const String& text);
    String                  showFileDialog      (const String& title, bool save, const String& filters = "", const String& initialDir = "", bool forceInitialDir = false); // filters = "ext:Title,foo;bar:Title"
    String                  showFileLoadDialog  (const String& title, const String& filters = "", const String& initialDir = "", bool forceInitialDir = false) { return showFileDialog(title, false, filters, initialDir, forceInitialDir); }
    String                  showFileSaveDialog  (const String& title, const String& filters = "", const String& initialDir = "", bool forceInitialDir = false) { return showFileDialog(title, true, filters, initialDir, forceInitialDir); }

    void                    showModalMessage    (const String& msg);

    static void             staticInit          (void);
    static void             staticDeinit        (void);
    static HWND             createHWND          (void);
    static UPTR             setWindowLong       (HWND hwnd, int idx, UPTR value);
    static int              getNumOpen          (void)          { return (s_inited) ? s_open->getSize() : 0; }
    static void             realizeAll          (void);
    static void             pollMessages        (void);

private:
    Event                   createSimpleEvent   (EventType type) { return createGenericEvent(type, "", 0, m_mouseKnown, m_mousePos); }
    Event                   createKeyEvent      (bool down, const String& key) { return createGenericEvent((down) ? EventType_KeyDown : EventType_KeyUp, key, 0, m_mouseKnown, m_mousePos); }
    Event                   createCharEvent     (S32 chr)       { return createGenericEvent(EventType_Char, "", chr, m_mouseKnown, m_mousePos); }
    Event                   createMouseEvent    (bool mouseKnown, const Vec2i& mousePos) { return createGenericEvent(EventType_Mouse, "", 0, mouseKnown, mousePos); }
    Event                   createGenericEvent  (EventType type, const String& key, S32 chr, bool mouseKnown, const Vec2i& mousePos);
    void                    postEvent           (const Event& ev);

    void                    create              (void);
    void                    destroy             (void);
    void                    recreate            (void);

    static LRESULT CALLBACK staticWindowProc    (HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT                 windowProc          (UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
                            Window              (const Window&); // forbidden
    Window&                 operator=           (const Window&); // forbidden

private:
    static bool             s_inited;
    static Array<Window*>*  s_open;
    static bool             s_ignoreRepaint;    // Prevents re-entering repaintNow() on Win32 or OpenGL failure.

    HWND                    m_hwnd;
    HDC                     m_hdc;

    GLContext::Config       m_glConfig;
    bool                    m_glConfigDirty;
    GLContext*              m_gl;

    bool                    m_isRealized;
    bool                    m_isVisible;
    Array<Listener*>        m_listeners;

    String                  m_title;
    bool                    m_isFullScreen;
    Vec2i                   m_pendingSize;
    DWORD                   m_origStyle;
    RECT                    m_origRect;

    Set<String>             m_keysDown;
    bool                    m_pendingKeyFlush;

    bool                    m_mouseKnown;
    Vec2i                   m_mousePos;
    S32                     m_mouseDragCount;
    S32                     m_mouseWheelAcc;

    Vec2i                   m_prevSize;
};

//------------------------------------------------------------------------
}
