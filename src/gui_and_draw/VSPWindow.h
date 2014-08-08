//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// Custom FLTK Window type to allow windows to be attached to each other
// Alex Gary
//
//////////////////////////////////////////////////////////////////////

#ifndef VSPWINDOW_H_
#define VSPWINDOW_H_

#include <FL/Fl_Double_Window.H>
#include <vector>

//==== Custom Window Type for the Geom Windows ====//
class VSP_Window : public Fl_Double_Window
{
public:
    VSP_Window(int w, int h, const char* title= 0);
    VSP_Window(int X, int Y, int W, int H, const char *l = 0);

    enum { WIN_ATT_NONE, WIN_ATT_LSIDE, WIN_ATT_RSIDE, WIN_ATT_TOP, WIN_ATT_BOTTOM, WIN_ATT_NUM_TYPES };
    enum { WIN_ALN_NONE, WIN_ALN_LEFT, WIN_ALN_RIGHT, WIN_ALN_TOP, WIN_ALN_BOTTOM, WIN_ALN_NUM_TYPES };

    void SetGeomScreenFlag( bool f )
    {
        m_GeomScreen = f;
    }
    bool GetGeomScreenFlag()
    {
        return m_GeomScreen;
    }
    int GetAttachType()
    {
        return m_AttachType;
    }
    void SetAttachType( int t);
    int GetAlignType()
    {
        return m_AlignType;
    }
    void SetAlignType( int t);

    void AddChild( VSP_Window* win );
    void RemoveChild( VSP_Window* win);
    void SetParent( VSP_Window* win );

    void resize( int X, int Y, int W, int H );
    void resizeFromParent( int X, int Y, int W, int H);
    void resizeFromChild( int X, int Y, int W, int H, VSP_Window * child);
    void position( int X, int Y);
    void show();
    int handle( int fl_event );
    virtual void UpdateChildren( VSP_Window* child = NULL);
    virtual void UpdateParent();

    static void SetGeomX( int X )
    {
        m_x = X;
    }
    static void SetGeomY( int Y )
    {
        m_y = Y;
    }

protected:

    bool m_GeomScreen;
    static int m_x;
    static int m_y;
    std::vector< VSP_Window* > m_Children;
    VSP_Window* m_Parent;

    int m_AttachType;
    int m_AlignType;

};


#endif /* VSPWINDOW_H_ */
