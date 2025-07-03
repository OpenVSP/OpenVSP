// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#include <Applications/GTApplicationsPCH.h>
#include <Applications/GLX/Window.h>
#include <X11/Xlib.h>

// XWindows has a Window data structure, so the implementations of the
// Geometric Tools class Window must be enclosed in "namespace gte".
// If instead you had "using namespace gte;", g++ will complain about
// the ambiguity of Window.

namespace gte
{
    Window::Parameters::Parameters()
        :
        display(nullptr),
        window(0),
        deviceCreationFlags(0)
    {
    }

    Window::Parameters::Parameters(std::wstring const& inTitle,
        int32_t inXOrigin, int32_t inYOrigin, int32_t inXSize, int32_t inYSize)
        :
        WindowApplication::Parameters(inTitle, inXOrigin, inYOrigin, inXSize, inYSize),
        display(nullptr),
        window(0),
        deviceCreationFlags(0)
    {
    }

    Window::~Window()
    {
    }

    Window::Window(Parameters& parameters)
        :
        WindowApplication(parameters),
        mDisplay(parameters.display),
        mWindow(parameters.window),
        mButtonDown{ false, false, false, false, false, false, false, false },
        mShiftDown(false),
        mControlDown(false),
        mAltDown(false),
        mCommandDown(false),
        mEngine(std::static_pointer_cast<GraphicsEngine>(mBaseEngine))
    {
    }

    void Window::SetTitle(std::wstring const& title)
    {
        WindowApplication::SetTitle(title);
        std::string narrowTitle(title.begin(), title.end());
        XStoreName(mDisplay, mWindow, narrowTitle.c_str());
    }

    void Window::ShowWindow()
    {
        XMapWindow(mDisplay, mWindow);
    }

    void Window::SetMousePosition(int32_t x, int32_t y)
    {
        XWarpPointer(mDisplay, 0, mWindow, 0, 0, 0, 0, x, y);
        XFlush(mDisplay);
    }

    void Window::GetMousePosition(int32_t& x, int32_t& y) const
    {
        XID rootWindow, childWindow;
        int32_t rootX, rootY;
        uint32_t modifier;
        XQueryPointer(mDisplay, mWindow, &rootWindow, &childWindow,
            &rootX, &rootY, &x, &y, &modifier);
    }

    void Window::OnClose()
    {
        XDestroyWindow(mDisplay, mWindow);
    }

    int32_t Window::ProcessedEvent()
    {
        if (!XPending(mDisplay))
        {
            return EVT_NONE_PENDING;
        }

        XEvent evt;
        XNextEvent(mDisplay, &evt);

        if (evt.type == ButtonPress || evt.type == ButtonRelease)
        {
            if (evt.xbutton.button == Button4)
            {
                OnMouseWheel(1, evt.xbutton.x, evt.xbutton.y,
                    evt.xbutton.state);
            }
            else if (evt.xbutton.button == Button5)
            {
                OnMouseWheel(-1, evt.xbutton.x, evt.xbutton.y,
                    evt.xbutton.state);
            }
            else
            {
                OnMouseClick(evt.xbutton.button, evt.xbutton.type,
                    evt.xbutton.x, evt.xbutton.y, evt.xbutton.state);

                mButtonDown[evt.xbutton.button] = (evt.type == ButtonPress);
            }
            return EVT_PROCESSED;
        }


        if (evt.type == MotionNotify)
        {
            int32_t button = MOUSE_NONE;
            for (int32_t i = MOUSE_LEFT; i <= MOUSE_RIGHT; ++i)
            {
                if (mButtonDown[i])
                {
                    button = i;
                    break;
                }
            }
            OnMouseMotion(button, evt.xmotion.x, evt.xmotion.y, evt.xmotion.state);
            return EVT_PROCESSED;
        }

        if (evt.type == KeyPress || evt.type == KeyRelease)
        {
            int32_t keysyms_per_keycode_return;
            KeySym* pKeySym = XGetKeyboardMapping(mDisplay,
                evt.xkey.keycode, 1, &keysyms_per_keycode_return);
            KeySym& keySym = *pKeySym;
            int32_t key = (keySym & 0x00FF);

            // Quit application if the KEY_ESCAPE key is pressed.
            if (key == KEY_ESCAPE)
            {
                XFree(pKeySym);
                return EVT_QUIT;
            }

            // Adjust for special keys from the key pad or the number pad.
            if ((keySym & 0xFF00) != 0)
            {
                if (0x50 <= key && key <= 0x57)
                {
                    // keypad Home, {L,U,R,D}Arrow, Pg{Up,Dn}, End
                    key += 0x45;
                }
                else if (key == 0x63)
                {
                    // keypad Insert
                    key = 0x9e;
                }
                else if (key == 0xFF)
                {
                    // keypad Delete
                    key = 0x9f;
                }
                else if (key == 0xE1 || key == 0xE2)
                {
                    // L-shift or R-shift
                    key = KEY_SHIFT;
                    mShiftDown = (evt.type == KeyPress);
                }
                else if (key == 0xE3 || key == 0xE4)
                {
                    // L-ctrl or R-ctrl
                    key = KEY_CONTROL;
                    mControlDown = (evt.type == KeyPress);
                }
                else if (key == 0xE9 || key == 0xEA)
                {
                    // L-alt or R-alt
                    key = KEY_ALT;
                    mAltDown = (evt.type == KeyPress);
                }
                else if (key == 0xEB || key == 0xEC)
                {
                    key = KEY_COMMAND;
                    mCommandDown = (evt.type == KeyPress);
                }
            }

            if ((KEY_HOME <= key && key <= KEY_END)
                || (KEY_F1 <= key && key <= KEY_F12)
                || (KEY_SHIFT <= key && key <= KEY_COMMAND))
            {
                if (evt.type == KeyPress)
                {
                    OnKeyDown(key, evt.xbutton.x, evt.xbutton.y);
                }
                else
                {
                    OnKeyUp(key, evt.xbutton.x, evt.xbutton.y);
                }
            }
            else
            {
                if (evt.type == KeyPress)
                {
                    // Get key-modifier state.  Adjust for shift state.
                    uint8_t ucKey = static_cast<uint8_t>(key);
                    if (mShiftDown && 'a' <= ucKey && ucKey <= 'z')
                    {
                        ucKey = static_cast<uint8_t>(key - 32);
                    }
                    OnCharPress(ucKey, evt.xbutton.x, evt.xbutton.y);
                }
            }
            XFree(pKeySym);
            return EVT_PROCESSED;
        }

        if (evt.type == Expose)
        {
            OnDisplay();
            return EVT_PROCESSED;
        }

        if (evt.type == ConfigureNotify)
        {
            OnMove(evt.xconfigure.x, evt.xconfigure.y);
            OnResize(evt.xconfigure.width, evt.xconfigure.height);
            return EVT_PROCESSED;
        }

        if (evt.type == ClientMessage)
        {
            Atom* wmDelete = nullptr;
            int32_t count;
            if (XGetWMProtocols(mDisplay, mWindow, &wmDelete, &count))
            {
                if ((unsigned long)evt.xclient.data.l[0] == *wmDelete)
                {
                    return EVT_QUIT;
                }
            }
        }

        return EVT_NONE_PENDING;
    }

    int32_t const WindowApplication::KEY_ESCAPE = 0x1B;
    int32_t const WindowApplication::KEY_HOME = 0x95;
    int32_t const WindowApplication::KEY_LEFT = 0x96;
    int32_t const WindowApplication::KEY_UP = 0x97;
    int32_t const WindowApplication::KEY_RIGHT = 0x98;
    int32_t const WindowApplication::KEY_DOWN = 0x99;
    int32_t const WindowApplication::KEY_PAGE_UP = 0x9A;
    int32_t const WindowApplication::KEY_PAGE_DOWN = 0x9B;
    int32_t const WindowApplication::KEY_END = 0x9C;
    int32_t const WindowApplication::KEY_INSERT = 0x9E;
    int32_t const WindowApplication::KEY_DELETE = 0x9F;
    int32_t const WindowApplication::KEY_F1 = 0xBE;
    int32_t const WindowApplication::KEY_F2 = 0xBF;
    int32_t const WindowApplication::KEY_F3 = 0xC0;
    int32_t const WindowApplication::KEY_F4 = 0xC1;
    int32_t const WindowApplication::KEY_F5 = 0xC2;
    int32_t const WindowApplication::KEY_F6 = 0xC3;
    int32_t const WindowApplication::KEY_F7 = 0xC4;
    int32_t const WindowApplication::KEY_F8 = 0xC5;
    int32_t const WindowApplication::KEY_F9 = 0xC6;
    int32_t const WindowApplication::KEY_F10 = 0xC7;
    int32_t const WindowApplication::KEY_F11 = 0xC8;
    int32_t const WindowApplication::KEY_F12 = 0xC9;
    int32_t const WindowApplication::KEY_BACKSPACE = 0x08;
    int32_t const WindowApplication::KEY_TAB = 0x09;
    int32_t const WindowApplication::KEY_ENTER = 0x0D;
    int32_t const WindowApplication::KEY_RETURN = 0x0D;

    int32_t const WindowApplication::KEY_SHIFT = 0xE1;  // L-shift
    int32_t const WindowApplication::KEY_CONTROL = 0xE3;  // L-ctrl
    int32_t const WindowApplication::KEY_ALT = 0xE9;  // L-alt
    int32_t const WindowApplication::KEY_COMMAND = 0xEB;  // L-command

    int32_t const WindowApplication::MOUSE_NONE = 0x0000;
    int32_t const WindowApplication::MOUSE_LEFT = 0x0001;
    int32_t const WindowApplication::MOUSE_MIDDLE = 0x0002;
    int32_t const WindowApplication::MOUSE_RIGHT = 0x0003;
    int32_t const WindowApplication::MOUSE_DOWN = 0x0004;
    int32_t const WindowApplication::MOUSE_UP = 0x0005;

    int32_t const WindowApplication::MODIFIER_CONTROL = 0x0004;
    int32_t const WindowApplication::MODIFIER_LBUTTON = 0x0001;
    int32_t const WindowApplication::MODIFIER_MBUTTON = 0x0002;
    int32_t const WindowApplication::MODIFIER_RBUTTON = 0x0003;
    int32_t const WindowApplication::MODIFIER_SHIFT = 0x0001;
}
