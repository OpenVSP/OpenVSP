//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// VspCaptiveRaise.cpp: draws a subwindow above its siblings through the platform's windows.  FLTK draws a subwindow
// above those shown before it and has no call to change that order.  Kept apart, as the platform headers define
// names that clash with OpenVSP's.
//
//////////////////////////////////////////////////////////////////////

#include <FL/Fl_Window.H>
#include <FL/platform.H>

#if defined( __APPLE__ ) && !defined( FLTK_USE_X11 )
#include <objc/message.h>
#include <objc/runtime.h>
#endif

bool CaptiveCanRaiseNative()
{
#if defined( _WIN32 ) || ( defined( __APPLE__ ) && !defined( FLTK_USE_X11 ) )
    return true;
#elif defined( FLTK_USE_X11 )
    return fl_x11_display() != nullptr;
#else
    return false;
#endif
}

bool CaptiveRaiseNative( Fl_Window* w )
{
    Fl_Window* parent = w->window();
    if ( !parent || !w->shown() || !parent->shown() )
    {
        return false;
    }

#if defined( _WIN32 )
    SetWindowPos( fl_xid( w ), HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE );
    return true;
#elif defined( __APPLE__ ) && !defined( FLTK_USE_X11 )
    // A subwindow is a child window of its parent's; the child added last is drawn above the others
    id child = reinterpret_cast< id >( fl_xid( w ) );
    id par = reinterpret_cast< id >( fl_xid( parent ) );
    typedef void ( *SendWindow )( id, SEL, id );
    typedef void ( *SendWindowOrder )( id, SEL, id, long );
    const long window_above = 1;
    reinterpret_cast< SendWindow >( objc_msgSend )( par, sel_registerName( "removeChildWindow:" ), child );
    reinterpret_cast< SendWindowOrder >( objc_msgSend )( par, sel_registerName( "addChildWindow:ordered:" ), child,
                                                          window_above );
    return true;
#elif defined( FLTK_USE_X11 )
    // Under Wayland, FLTK's subsurfaces are out of reach
    Display* d = fl_x11_display();
    if ( !d )
    {
        return false;
    }
    XRaiseWindow( d, fl_x11_xid( w ) );
    return true;
#else
    return false;
#endif
}
