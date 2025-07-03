// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.02.02

#include <Applications/GTApplicationsPCH.h>
#include <Applications/MSW/Window.h>
using namespace gte;

Window::Parameters::Parameters()
    :
    handle(nullptr),
    parent(nullptr),
    hscrollBar(false),
    vscrollBar(false),
    deviceCreationFlags(0)
{
}

Window::Parameters::Parameters(std::wstring const& inTitle,
    int32_t inXOrigin, int32_t inYOrigin, int32_t inXSize, int32_t inYSize)
    :
    WindowApplication::Parameters(inTitle, inXOrigin, inYOrigin, inXSize, inYSize),
    handle(nullptr),
	parent(nullptr),
    hscrollBar(false),
    vscrollBar(false),
    deviceCreationFlags(0)
{
}

Window::~Window()
{
#if defined(GTE_USE_DIRECTX)
    if (mEngine)
    {
        auto dxengine = std::static_pointer_cast<DX11Engine>(mEngine);
        dxengine->ExitFullscreen();
    }
#endif
}

Window::Window(Parameters& parameters)
    :
    WindowApplication(parameters),
    mHandle(parameters.handle),
    mEngine(std::static_pointer_cast<GraphicsEngine>(mBaseEngine))
{
    // If scroll bars are requested, the Window-derived class constructor
    // should call the function Window::SetScrollInterval(...) to set the
    // scroll bar range.  Increments and decrements may be set separately
    // directly via class members.
    mHasScroll[0] = parameters.hscrollBar;
    mHasScroll[1] = parameters.vscrollBar;
    for (int32_t i = 0; i < 2; ++i)
    {
        ZeroMemory(&mScrollInfo[i], sizeof(SCROLLINFO));
        mScrollInfo[i].cbSize = sizeof(SCROLLINFO);
        mScrollInfo[i].fMask = SIF_ALL;
        GetScrollInfo(mHandle, i, &mScrollInfo[i]);
        mScrollLoResDelta[i] = 1;
        mScrollHiResDelta[i] = 1;
    }
}

void Window::SetTitle(std::wstring const& title)
{
    WindowApplication::SetTitle(title);
    SetWindowText(mHandle, title.c_str());
}

void Window::SetMousePosition(int32_t x, int32_t y)
{
    POINT point = { static_cast<LONG>(x), static_cast<LONG>(y) };
    ClientToScreen(mHandle, &point);
    SetCursorPos(point.x, point.y);
}

void Window::GetMousePosition(int32_t& x, int32_t& y) const
{
    POINT point;
    GetCursorPos(&point);
    ScreenToClient(mHandle, &point);
    x = static_cast<int32_t>(point.x);
    y = static_cast<int32_t>(point.y);
}

void Window::OnClose()
{
    PostQuitMessage(0);
}

void Window::SetScrollInterval(int32_t bar, int32_t minValue, int32_t maxValue)
{
    mScrollInfo[bar].fMask = SIF_RANGE;
    mScrollInfo[bar].nMin = minValue;
    mScrollInfo[bar].nMax = maxValue;
    SetScrollInfo(mHandle, bar, &mScrollInfo[bar], FALSE);
}

void Window::GetScrollInterval(int32_t bar, int32_t& minValue, int32_t& maxValue) const
{
    mScrollInfo[bar].fMask = SIF_RANGE;
    GetScrollInfo(mHandle, bar, &mScrollInfo[bar]);
    minValue = mScrollInfo[bar].nMin;
    maxValue = mScrollInfo[bar].nMax;
}

int32_t Window::SetScrollPosition(int32_t bar, int32_t value)
{
    mScrollInfo[bar].fMask = SIF_POS;
    mScrollInfo[bar].nPos = value;
    return SetScrollInfo(mHandle, bar, &mScrollInfo[bar], FALSE);
}

int32_t Window::GetScrollPosition(int32_t bar) const
{
    mScrollInfo[bar].fMask = SIF_POS;
    GetScrollInfo(mHandle, bar, &mScrollInfo[bar]);
    return mScrollInfo[bar].nPos;
}

int32_t Window::OnScrollIncrementLoRes(int32_t bar)
{
    SCROLLINFO& info = mScrollInfo[bar];
    info.fMask = SIF_POS;
    GetScrollInfo(mHandle, bar, &info);
    int32_t delta = info.nMax - info.nPos;
    if (delta > 0)
    {
        delta = std::min(delta, mScrollLoResDelta[bar]);
        info.nPos += delta;
        SetScrollInfo(mHandle, bar, &info, TRUE);
    }
    return delta;
}

int32_t Window::OnScrollDecrementLoRes(int32_t bar)
{
    SCROLLINFO& info = mScrollInfo[bar];
    info.fMask = SIF_POS;
    GetScrollInfo(mHandle, bar, &info);
    int32_t delta = info.nPos - info.nMin;
    if (delta > 0)
    {
        delta = std::min(delta, mScrollLoResDelta[bar]);
        info.nPos -= delta;
        SetScrollInfo(mHandle, bar, &info, TRUE);
    }
    return delta;
}

int32_t Window::OnScrollIncrementHiRes(int32_t bar)
{
    SCROLLINFO& info = mScrollInfo[bar];
    info.fMask = SIF_POS;
    GetScrollInfo(mHandle, bar, &info);
    int32_t delta = info.nMax - info.nPos;
    if (delta > 0)
    {
        delta = std::min(delta, mScrollHiResDelta[bar]);
        info.nPos += delta;
        SetScrollInfo(mHandle, bar, &info, TRUE);
    }
    return delta;
}

int32_t Window::OnScrollDecrementHiRes(int32_t bar)
{
    SCROLLINFO& info = mScrollInfo[bar];
    info.fMask = SIF_POS;
    GetScrollInfo(mHandle, bar, &info);
    int32_t delta = info.nPos - info.nMin;
    if (delta > 0)
    {
        delta = std::min(delta, mScrollHiResDelta[bar]);
        info.nPos -= delta;
        SetScrollInfo(mHandle, bar, &info, TRUE);
    }
    return delta;
}

int32_t Window::OnScrollTracking(int32_t bar)
{
    SCROLLINFO& info = mScrollInfo[bar];
    info.fMask = SIF_TRACKPOS;
    GetScrollInfo(mHandle, bar, &info);
    return info.nTrackPos;
}

int32_t Window::OnScrollEndTracking(int32_t bar)
{
    SCROLLINFO& info = mScrollInfo[bar];
    info.fMask = SIF_TRACKPOS;
    GetScrollInfo(mHandle, bar, &info);
    info.fMask = SIF_POS;
    info.nPos = info.nTrackPos;
    SetScrollInfo(mHandle, bar, &info, TRUE);
    return info.nTrackPos;
}


// Class-static constants for Microsoft Windows applications.
int32_t const WindowApplication::KEY_ESCAPE = VK_ESCAPE;
int32_t const WindowApplication::KEY_LEFT = VK_LEFT;
int32_t const WindowApplication::KEY_RIGHT = VK_RIGHT;
int32_t const WindowApplication::KEY_UP = VK_UP;
int32_t const WindowApplication::KEY_DOWN = VK_DOWN;
int32_t const WindowApplication::KEY_HOME = VK_HOME;
int32_t const WindowApplication::KEY_END = VK_END;
int32_t const WindowApplication::KEY_PAGE_UP = VK_PRIOR;
int32_t const WindowApplication::KEY_PAGE_DOWN = VK_NEXT;
int32_t const WindowApplication::KEY_INSERT = VK_INSERT;
int32_t const WindowApplication::KEY_DELETE = VK_DELETE;
int32_t const WindowApplication::KEY_F1 = VK_F1;
int32_t const WindowApplication::KEY_F2 = VK_F2;
int32_t const WindowApplication::KEY_F3 = VK_F3;
int32_t const WindowApplication::KEY_F4 = VK_F4;
int32_t const WindowApplication::KEY_F5 = VK_F5;
int32_t const WindowApplication::KEY_F6 = VK_F6;
int32_t const WindowApplication::KEY_F7 = VK_F7;
int32_t const WindowApplication::KEY_F8 = VK_F8;
int32_t const WindowApplication::KEY_F9 = VK_F9;
int32_t const WindowApplication::KEY_F10 = VK_F10;
int32_t const WindowApplication::KEY_F11 = VK_F11;
int32_t const WindowApplication::KEY_F12 = VK_F12;
int32_t const WindowApplication::KEY_BACKSPACE = VK_BACK;
int32_t const WindowApplication::KEY_TAB = VK_TAB;
int32_t const WindowApplication::KEY_ENTER = VK_RETURN;
int32_t const WindowApplication::KEY_RETURN = VK_RETURN;

int32_t const WindowApplication::KEY_SHIFT = VK_SHIFT;
int32_t const WindowApplication::KEY_CONTROL = VK_CONTROL;
int32_t const WindowApplication::KEY_ALT = 0;      // not currently handled
int32_t const WindowApplication::KEY_COMMAND = 0;  // not currently handled

int32_t const WindowApplication::MOUSE_NONE = 0;
int32_t const WindowApplication::MOUSE_LEFT = 1;
int32_t const WindowApplication::MOUSE_MIDDLE = 2;
int32_t const WindowApplication::MOUSE_RIGHT= 3;
int32_t const WindowApplication::MOUSE_UP = 0;
int32_t const WindowApplication::MOUSE_DOWN = 1;

int32_t const WindowApplication::MODIFIER_CONTROL = MK_CONTROL;
int32_t const WindowApplication::MODIFIER_LBUTTON = MK_LBUTTON;
int32_t const WindowApplication::MODIFIER_MBUTTON = MK_MBUTTON;
int32_t const WindowApplication::MODIFIER_RBUTTON = MK_RBUTTON;
int32_t const WindowApplication::MODIFIER_SHIFT = MK_SHIFT;
