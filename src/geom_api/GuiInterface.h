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
    void PopupMsg( const std::string &message );
    void UpdateGUI();

    void Lock();
    void Unlock();

    bool IsEventLoopRunning() const;

    void ScreenGrab( const std::string & fname, int w, int h, bool transparentBG, bool autocrop );
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
