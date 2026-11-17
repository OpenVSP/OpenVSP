//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// Custom FLTK Window type for the screens
// Alex Gary
//
//////////////////////////////////////////////////////////////////////

#include <FL/Fl.H>
#include <FL/Fl_Input_.H>
#include "VSPWindow.h"

bool valid_hotkey( int event_length, int key )
{
    if ( event_length == 1 )
    {
        return true;
    }
    if ( key == FL_Enter || key == FL_KP_Enter )
    {
        return true;
    }
    for ( int fkey = FL_F + 1; fkey < FL_F_Last; fkey++ )
    {
        if ( fkey == key )
        {
            return true;
        }
    }
    return false;
}

int VSP_Window::m_x = 1;
int VSP_Window::m_y = 1;
int VSP_Window::m_screen = -1;

VSP_Window::VSP_Window(int w, int h, const char* title) : Fl_Double_Window( w, h, title )
{
    m_GeomScreen = false;
    m_ResizeCB = nullptr;
    m_ResizeData = nullptr;
    m_KeyCB = nullptr;
    m_KeyCBData = nullptr;
}

VSP_Window::VSP_Window(int x, int y, int w, int h, const char* l) : Fl_Double_Window(x , y, w, h, l)
{
    m_GeomScreen = false;
    m_ResizeCB = nullptr;
    m_ResizeData = nullptr;
    m_KeyCB = nullptr;
    m_KeyCBData = nullptr;
}

void VSP_Window::show()
{
    if ( m_GeomScreen )
    {
        // A window not yet shown takes its coordinates in the scale of the screen it is assigned to.
        if ( !shown() && m_screen >= 0 )
        {
            screen_num( m_screen );
        }
        position( m_x, m_y);
    }

    Fl_Double_Window::show();
}


void VSP_Window::resize( int x_in, int y_in, int w_in, int h_in )
{
    if ( m_GeomScreen && shown() )
    {
        m_x = x_in;
        m_y = y_in;
        m_screen = screen_num();
    }

    Fl_Double_Window::resize(x_in,y_in,w_in,h_in);

    if ( m_ResizeCB )
    {
        m_ResizeCB( this, m_ResizeData );
    }
}

// Cuts, copies or pastes in the text field with the keyboard, which takes those keys only with Cmd on macOS
bool VSP_Window::TextEditKey( int key )
{
    Fl_Input_* in = dynamic_cast< Fl_Input_* >( Fl::focus() );
    if ( !in )
    {
        return false;
    }
    if ( key == 'c' || key == 'x' )
    {
        in->copy( 1 );
    }
    if ( key == 'x' && !in->readonly() )
    {
        in->cut();
    }
    if ( key == 'v' && !in->readonly() )
    {
        Fl::paste( *in, 1 );
    }
    return true;
}

int VSP_Window::handle( int fl_event )
{
#ifdef __APPLE__
    // Ctrl with X, C or V does what Cmd does: a shortcut nothing in the window took is offered
    // to it again as Cmd.  This covers the window's own widgets, such as the Geom Browser's Copy;
    // hidden items of the main menu cover the menu's shortcuts.
    if ( fl_event == FL_SHORTCUT && ( Fl::event_state() & FL_CTRL ) && !( Fl::event_state() & FL_META ) )
    {
        int key = Fl::event_key();
        if ( key == 'x' || key == 'c' || key == 'v' )
        {
            if ( TextEditKey( key ) )
            {
                return 1;
            }
            if ( Fl_Double_Window::handle( fl_event ) )
            {
                return 1;
            }
            int state = Fl::e_state;
            Fl::e_state = ( state & ~FL_CTRL ) | FL_META;
            int used = Fl_Double_Window::handle( fl_event );
            Fl::e_state = state;
            return used;
        }
    }
#endif

    int x = Fl::event_x();
    int y = Fl::event_y();

    switch( fl_event )
    {
        case FL_KEYDOWN:
        {
            if ( m_KeyCB && valid_hotkey( Fl::event_length(), Fl::event_key() ) )
            {
                m_KeyCB( this, m_KeyCBData );
            }
        }
//    case FL_ENTER:
//        cursor( FL_CURSOR_CROSS );
//        return 1;
//
//    case FL_LEAVE:
//        cursor( FL_CURSOR_DEFAULT );
//        return 1;
//
//    case FL_PUSH:
//        printf("%d, %d\n",x,y);
//        return 1;
    }
    return Fl_Double_Window::handle( fl_event );
}

void VSP_Window::SetKeyCallback( Fl_Callback* cb, void* p )
{
    m_KeyCB = cb;
    m_KeyCBData = p;
}

void VSP_Window::SetResizeCallback( Fl_Callback* cb, void* p )
{
    m_ResizeCB = cb;
    m_ResizeData = p;
}
