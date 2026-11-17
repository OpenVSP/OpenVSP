//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// VspSidePanel.h: the Geom Browser's place at the right of the 3D view, over it, with a tab that brings it back
//
//////////////////////////////////////////////////////////////////////

#ifndef VSPSIDEPANEL_H_
#define VSPSIDEPANEL_H_

#include "VspCaptiveWindow.h"

#include <vector>

class VspSideTab;

// A place inside the main window along the 3D view's right edge, over the view and its full height, that holds
// screens' windows and shows one at a time, in either window mode.  A close button at the right of the screen's title
// hides it, and a tab at the top of the view's right edge, also over the view, brings it back.
class VspSidePanel : public Fl_Double_Window, public VspBodyHolder
{
public:
    explicit VspSidePanel( VspCaptiveLayer* layer );
    ~VspSidePanel() override;

    // The width of the edges around the screen's window, and of the close button
    static const int EDGE = 2;
    static const int CLOSE_W = 20;

    // Takes a screen's window in, hidden, or gives it back as a window of its own, placed where it was on the screen at
    // the height it had
    void TakeScreen( VspScreen* screen );
    void ReleaseScreen( VspScreen* screen );
    // Lets go of a screen's window as the screen is deleted, leaving it as it is
    void ForgetScreen( VspScreen* screen );

    void ShowScreen( VspScreen* screen );
    void HideScreen( VspScreen* screen );
    bool ScreenShown( VspScreen* screen );

    // Hides the screen shown, or shows again the one shown last, or the first that shows
    void Toggle();

    // Puts the panel along the layer's right edge, its full height and as wide as the screen's window, and the tab at
    // the top of the edge while the panel is hidden
    void Place();

    // The width the panel takes for a screen's window of this width
    static int OuterW( int body_w )
    {
        return body_w + 2 * EDGE;
    }

    void BodyResized() override;

protected:

    int IndexOf( VspScreen* screen );
    void Forget( int i );

    static void CloseCB( Fl_Widget* w, void* data );

    VspCaptiveLayer* m_Layer;
    VspSideTab* m_Tab;

    std::vector< VspScreen* > m_Screens;
    // Whether each window had a border as a window of its own, which FLTK takes from a subwindow, and its height
    std::vector< bool > m_BodyBorders;
    std::vector< int > m_BodyHeights;
    // The close button the panel put in each window
    std::vector< Fl_Button* > m_CloseButtons;

    // The screen shown, and the one shown last, or -1
    int m_Shown;
    int m_LastShown;

    bool m_InPlace;
};

// The tab that brings the panel back: a window of its own, so it is drawn over the 3D view
class VspSideTab : public Fl_Double_Window
{
public:
    explicit VspSideTab( VspSidePanel* panel );

    static const int W = 14;
    static const int H = 56;
    // Below the top of the view
    static const int TOP = 8;

protected:

    static void PressCB( Fl_Widget* w, void* data );

    VspSidePanel* m_Panel;
    Fl_Button* m_Button;
};

#endif // VSPSIDEPANEL_H_
