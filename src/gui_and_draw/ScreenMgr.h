//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// VehicleMgr.h: interface for the Vehicle Class and Vehicle Mgr Singleton.
// J.R Gloudemans
//
//////////////////////////////////////////////////////////////////////

#if !defined(SCREENMGR__INCLUDED_)
#define SCREENMGR__INCLUDED_

#include <FL/Fl_Native_File_Chooser.H>

#include "ScreenBase.h"
#include "Vehicle.h"
#include "MessageMgr.h"
#include "SelectFileScreen.h"
#include "PickSetScreen.h"
#include "MainVSPScreen.h"

#include <vector>
#include <string>
#include <mutex>
#include <condition_variable>

//==== ScreenMgr ====//
class ScreenMgr : MessageBase
{
public:
    ScreenMgr( Vehicle* vPtr );
    virtual ~ScreenMgr();

    virtual void FirstShow();

    virtual void ShowScreen( int id );
    virtual void HideScreen( int id );
    virtual Vehicle* GetVehiclePtr()
    {
        return m_VehiclePtr;
    }
    virtual Geom* GetCurrGeom();
    virtual void SetUpdateFlag( bool flag );
    virtual void ForceUpdate();
    virtual void Alert( const string &message );

    void LaunchSystemDefaultBrowser( const string &url );
    void HelpSystemDefaultBrowser( const string &file );

    string FileChooser( const string &title, const string &filter, int mode = vsp::OPEN, const string &dir = string() );

    PickSetScreen* GetPickSetScreen()
    {
        return (PickSetScreen *)m_ScreenVec[vsp::VSP_PICK_SET_SCREEN];
    }

    /*!
    * Get Screen from id.  If id does not exist, return NULL.
    */
    VspScreen* GetScreen( int id );

    virtual void MessageCallback( const MessageBase* from, const MessageData& data );


    bool CheckRunGui();
    void SetRunGui( bool r );
    void EnableStopGUIMenuItem();
    void DisableStopGUIMenuItem();
    void EnableExitMenuItem();
    void DisableExitMenuItem();

    void APIHideScreensImplementation();
    void APIShowScreensImplementation();
    void APIHideScreens();
    void APIShowScreens();

    void APIUpdateGUIImplementation();
    void APIUpdateGUI();

    void APIScreenGrabImplementation( const string & fname, int w, int h, bool transparentBG, bool autocrop );
    void APIScreenGrab( const string & fname, int w, int h, bool transparentBG, bool autocrop );

    bool IsGUIElementDisabled( int e ) const;
    void SetGUIElementDisable( int e, bool state );
    bool IsGUIScreenDisabled( int s ) const;
    void SetGUIScreenDisable( int s, bool state );
    bool IsGeomScreenDisabled( int s ) const;
    void SetGeomScreenDisable( int s, bool state );

    bool m_ShowPlotScreenOnce;

    mutex m_TaskMutex;
    condition_variable m_TaskCV;
    bool m_TaskComplete;

    mutex m_ScreenGrabMutex;
    condition_variable m_ScreenGrabCV;
    bool m_ScreenGrabComplete;

    void LoadSetChoice( Choice &choice, const int &selected, bool includeNone = false, int istart = 0 );
    void LoadSetChoice( const vector < Choice* > &choicevec, const vector <int> &selectedvec, bool includeNone = false, int istart = 0 );

    void LoadSetChoice( Choice &choice, const string &selectedID, bool includeNone = false );
    void LoadSetChoice( const vector < Choice* > &choicevec, const vector <string> &selectedIDvec, bool includeNone = false, int istart = 0 );

    void LoadModeChoice( Choice & choice, vector < string > &modeIDVec, int selected );
    void LoadModeChoice( Choice & choice, vector < string > &modeIDVec, const string &selModeID );

protected:

    void Init();

    static int GlobalHandler(int event);

    bool m_UpdateFlag;
    virtual void UpdateAllScreens();

    Vehicle* m_VehiclePtr;
    vector< VspScreen* > m_ScreenVec;
    vector< bool > m_APIScreenOpenVec;

    string NativeFileChooser( const string &title, const string &filter, int mode = vsp::OPEN, const string &dir = string() );
    Fl_Native_File_Chooser * m_NativeFileChooser;

    virtual void TimerCB();
    static void StaticTimerCB( void* data )
    {
        static_cast< ScreenMgr* >( data )->TimerCB();
    }

    bool m_RunGUI;

    unsigned long m_UpdateCount;

    vector < bool > m_DisabledGUIElements;
    vector < bool > m_DisabledGUIScreens;
    vector < bool > m_DisabledGeomScreens;
};


#endif // !defined(SCREENMGR__INCLUDED_)
