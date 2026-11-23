//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// VspCaptiveWindow.h: screens shown as windows inside the main window, over the 3D view
//
//////////////////////////////////////////////////////////////////////

#ifndef VSPCAPTIVEWINDOW_H_
#define VSPCAPTIVEWINDOW_H_

#include <FL/Fl_Group.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>

#include <string>

class VspScreen;
class VspCaptiveFrame;

// A window inside the main window that holds a screen's window, told when the screen resizes it
class VspBodyHolder
{
public:
    virtual ~VspBodyHolder() {}
    virtual void BodyResized() = 0;
};

// The part of the main window the captive windows lie in, over the 3D view.  Its first child is the 3D view, which
// fills it, and the others are the frames and panels, last on top.  As the main window resizes, the view takes the
// layer's size and the frames keep theirs and are kept inside it.
class VspCaptiveLayer : public Fl_Group
{
public:
    VspCaptiveLayer( int x, int y, int w, int h );

    // Puts the 3D view, of the layer's size less the strip kept at its right, under the layer's windows, so the pointer
    // over none of them is over the view
    void SetView( Fl_Window* view );

    // Keeps a strip this wide along the layer's right edge clear of the 3D view, which takes the rest
    void SetViewRight( int w );

    void resize( int x, int y, int w, int h ) override;
    int handle( int event ) override;

    // The frame on top at a point in the window, or nullptr
    VspCaptiveFrame* FrameAt( int x, int y );

    // The window shown on top at a point in the window -- a frame, or another window in the layer -- or nullptr
    Fl_Window* WindowAt( int x, int y );

    // The frame drawn on top, or nullptr if none is shown
    VspCaptiveFrame* TopFrame();

    // Draws the title bar of the frame on top as active and the others as inactive
    void UpdateActive();

    // Draws the layer's windows in the order of its children, the last on top: false where the platform does not
    // allow it.  FLTK's model draws a window shown last above its siblings, which Windows does not, so the order
    // is set here whenever one of them is shown.
    bool Restack();

private:
    Fl_Window* m_View;
    int m_ViewRight;
};

// A window inside the main window that holds a screen's window, with a title bar to move it by, a close button and
// edges to size it by
class VspCaptiveFrame : public Fl_Double_Window, public VspBodyHolder
{
public:
    VspCaptiveFrame( VspScreen* screen, Fl_Window* body, VspCaptiveLayer* layer, int x, int y );
    ~VspCaptiveFrame() override;

    // The width of the edges and the height of the title bar
    static const int EDGE = 4;
    static const int TITLE_H = 22;

    void ShowFrame();
    void HideFrame();
    bool FrameShown();

    // Draws the frame above the others
    void Raise();

    void SetTitle( const std::string &title );
    void SetActive( bool active );

    // Sizes the frame to its body after the body's screen resized it
    void BodyResized() override;

    // Keeps the frame inside its layer
    void Clamp();

    // Hides the frame and takes the body out of it, placed on the screen where it was shown in the frame
    Fl_Window* ReleaseBody();
    // Takes the body out of the frame as it is deleted, leaving it as it is
    void ForgetBody();

    Fl_Window* GetBody()
    {
        return m_Body;
    }

    int handle( int event ) override;
    void resize( int x, int y, int w, int h ) override;

    // The frame that holds a widget, or nullptr
    static VspCaptiveFrame* FrameOf( Fl_Widget* w );

    // Raises the layer's frame clicked and the frame typed in, through FLTK's event dispatch
    static void InstallRaiseHook( VspCaptiveLayer* layer );

protected:

    enum { EDGE_L = 1, EDGE_R = 2, EDGE_T = 4, EDGE_B = 8 };

    int EdgesAt( int ex, int ey );
    bool InBody( int ex, int ey );
    bool CanSize();
    void SizeLimits( int &minw, int &minh, int &maxw, int &maxh );
    void Layout();
    void SetEdgeCursor( int edges );
    void KeepGeomPosition();

    static void CloseCB( Fl_Widget* w, void* data );
    static int Dispatch( int event, Fl_Window* w );

    VspScreen* m_Screen;
    Fl_Window* m_Body;
    VspCaptiveLayer* m_Layer;

    // Whether the body had a border as a window of its own, which FLTK takes from a subwindow
    bool m_BodyBorder;

    Fl_Box* m_TitleBox;
    Fl_Button* m_CloseButton;
    std::string m_Title;

    bool m_InLayout;

    // The drag under way: none, a move or the edges being sized
    bool m_Moving;
    int m_SizeEdges;
    bool m_EdgeCursor;
    int m_StartMX;
    int m_StartMY;
    int m_StartX;
    int m_StartY;
    int m_StartW;
    int m_StartH;
};

// Draws a shown subwindow above its siblings without hiding it, where the platform allows: false if not
bool CaptiveRaiseNative( Fl_Window* w );
bool CaptiveCanRaiseNative();

// Shows every subwindow of the group that is visible and not yet shown
void CaptiveShowSubwindows( Fl_Group* g );

#endif // VSPCAPTIVEWINDOW_H_
