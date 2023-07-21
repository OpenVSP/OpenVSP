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

#include "ScreenBase.h"
#include "Vehicle.h"
#include "MessageMgr.h"
#include "SelectFileScreen.h"
#include "PickSetScreen.h"
#include "MainVSPScreen.h"

#include <vector>
#include <string>


//==== ScreenMgr ====//
class ScreenMgr : MessageBase
{
public:
    ScreenMgr( Vehicle* vPtr );
    virtual ~ScreenMgr();

    virtual void ShowScreen( int id );
    virtual void HideScreen( int id );
    virtual Vehicle* GetVehiclePtr()
    {
        return m_VehiclePtr;
    }
    virtual Geom* GetCurrGeom();
    virtual void SetUpdateFlag( bool flag );
    virtual void ForceUpdate();
    virtual void Alert( const char * message );

    SelectFileScreen * GetSelectFileScreen()
    {
        return ( SelectFileScreen * )m_ScreenVec[vsp::VSP_SELECT_FILE_SCREEN];
    }
    PickSetScreen* GetPickSetScreen()
    {
        return (PickSetScreen *)m_ScreenVec[vsp::VSP_PICK_SET_SCREEN];
    }

    /*!
    * Get Screen from id.  If id does not exist, return NULL.
    */
    VspScreen* GetScreen( int id );

    virtual void MessageCallback( const MessageBase* from, const MessageData& data );


    bool CheckRunGui()
    {
        return m_RunGUI;
    };
    void SetRunGui( bool r )
    {
        m_RunGUI = r;
    };
    void ShowReturnToAPI()
    {
//        ( ( MainVSPScreen* ) m_ScreenVec[vsp::VSP_MAIN_SCREEN] )->ShowReturnToAPI();
    };
    void HideReturnToAPI()
    {
//        ( ( MainVSPScreen* ) m_ScreenVec[vsp::VSP_MAIN_SCREEN] )->HideReturnToAPI();
    };

    bool m_ShowPlotScreenOnce;

protected:

    void Init();

    static int GlobalHandler(int event);

    bool m_UpdateFlag;
    virtual void UpdateAllScreens();

    Vehicle* m_VehiclePtr;
    vector< VspScreen* > m_ScreenVec;

    virtual void TimerCB();
    static void StaticTimerCB( void* data )
    {
        static_cast< ScreenMgr* >( data )->TimerCB();
    }

    bool m_RunGUI;
};


#endif // !defined(SCREENMGR__INCLUDED_)
