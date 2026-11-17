//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// Custom FLTK Window type for the screens
// Alex Gary
//
//////////////////////////////////////////////////////////////////////

#ifndef VSPWINDOW_H_
#define VSPWINDOW_H_

#include <FL/Fl_Double_Window.H>

//==== Valid Hotkeys are all chars and function keys, excludes shift, ctrl, alt, etc. ====//
bool valid_hotkey( int event_length, int key );

//==== Custom Window Type for the Geom Windows ====//
class VSP_Window : public Fl_Double_Window
{
public:
    VSP_Window(int w, int h, const char* title= 0);
    VSP_Window(int X, int Y, int W, int H, const char *l = 0);

    // Ctrl or Cmd with X, C or V given to the text field that has the keyboard, if one does: true if it took the key
    static bool TextEditKey( int key );

    void SetGeomScreenFlag( bool f )
    {
        m_GeomScreen = f;
    }
    bool GetGeomScreenFlag()
    {
        return m_GeomScreen;
    }

    void resize( int X, int Y, int W, int H );
    void show();
    int handle( int fl_event );

    // The Geom screens share one place, kept by its upper-right corner, so screens of different widths line up
    // along their right edges
    static void SetGeomRight( int X )
    {
        m_Right = X;
    }
    static void SetGeomY( int Y )
    {
        m_y = Y;
    }
    static int GetGeomRight()
    {
        return m_Right;
    }
    static int GetGeomY()
    {
        return m_y;
    }
    static void SetGeomScreen( int n )
    {
        m_screen = n;
    }

    void SetResizeCallback( Fl_Callback* cb, void* p );
    void SetKeyCallback( Fl_Callback* cb, void* p );

protected:

    bool m_GeomScreen;
    static int m_Right;
    static int m_y;
    static int m_screen;

    Fl_Callback* m_KeyCB;
    void* m_KeyCBData;

    Fl_Callback* m_ResizeCB;
    void* m_ResizeData;

};


#endif /* VSPWINDOW_H_ */
