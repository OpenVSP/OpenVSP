//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// VehicleMgr.h: interface for the Vehicle Class and Vehicle Mgr Singleton.
// J.R Gloudemans
//
//////////////////////////////////////////////////////////////////////

#if !defined(GUIINTERFACE__INCLUDED_)
#define GUIINTERFACE__INCLUDED_

#include <string>

class ScreenMgr;
class Vehicle;


//==== GUI Interface ====//
class GuiInterface
{
private:
    GuiInterface();
    ~GuiInterface();
    GuiInterface( GuiInterface const& copy ) = delete;          // Not Implemented
    GuiInterface& operator=( GuiInterface const& copy ) = delete; // Not Implemented

    ScreenMgr* m_ScreenMgr;
    bool m_EventLoopRunning;
public:
    static GuiInterface& getInstance()
    {
        static GuiInterface instance;
        return instance;
    }
    void InitGUI( Vehicle* vPtr );
    void StartGUI( );
    void StartGUIAPI( );
    void StopGUI();

    // Shows the GUI and writes a PNG and an EPS of every screen and tab page into dir, the images at
    // scale times the screens' size.  A screen of vsp::GUI_VSP_SCREEN or a Geom screen of
    // vsp::GUI_GEOM_SCREEN, if not -1, is the only one written.  The model gains a Geom of every type
    // it lacks.  Returns the number of files that could not be written, or -1 when none was written.
    int DumpScreens( int screen, int geom_screen, const std::string &dir, double scale );
    void PopupMsg( const std::string &message );
    void UpdateGUI();

    void Lock();
    void Unlock();

    bool IsEventLoopRunning() const;

    void ScreenGrab( const std::string & fname, int w, int h, bool transparentBG, bool autocrop );
    // False when the file cannot be written; without a GUI it does nothing
    bool VectorScreenGrab( const std::string & fname, int filetype, bool autocrop, bool scaletopage, double pagewidth, double pageheight, int pageunits );
    void SetViewAxis( bool vaxis );
    void SetShowBorders( bool brdr );
    void SetBackground( double r, double g, double b );

    void SetAllViews( int view );
    void SetView( int viewport, int view );
    void FitAllViews();
    void ResetViews();
    void SetWindowLayout( int r, int c );

    void EnableStopGUIMenuItem();
    void DisableStopGUIMenuItem();

    void SetGUIElementDisable( int e, bool state );
    void SetGUIScreenDisable( int s, bool state );
    void SetGeomScreenDisable( int s, bool state );

    void HideScreen( int s );
    void ShowScreen( int s );
};

#endif // !defined(GUIINTERFACE__INCLUDED_)
