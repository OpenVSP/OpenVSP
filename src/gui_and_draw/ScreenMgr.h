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
    enum { VSP_ADV_LINK_SCREEN,
           VSP_AIRFOIL_CURVES_EXPORT_SCREEN,
           VSP_AIRFOIL_POINTS_EXPORT_SCREEN,
           VSP_BACKGROUND_SCREEN,
           VSP_BEM_OPTIONS_SCREEN,
           VSP_CFD_MESH_SCREEN,
           VSP_CLIPPING_SCREEN,
           VSP_COMP_GEOM_SCREEN,
           VSP_COR_SCREEN,
           VSP_DEGEN_GEOM_SCREEN,
           VSP_DESIGN_VAR_SCREEN,
           VSP_DXF_OPTIONS_SCREEN,
           VSP_EXPORT_SCREEN,
           VSP_FEA_PART_EDIT_SCREEN,
           VSP_FEA_XSEC_SCREEN,
           VSP_FIT_MODEL_SCREEN,
           VSP_IGES_OPTIONS_SCREEN,
           VSP_IGES_STRUCTURE_OPTIONS_SCREEN,
           VSP_IMPORT_SCREEN,
           VSP_LIGHTING_SCREEN,
           VSP_MANAGE_GEOM_SCREEN,
           VSP_MANAGE_TEXTURE_SCREEN,
           VSP_MASS_PROP_SCREEN,
           VSP_MATERIAL_EDIT_SCREEN,
           VSP_MEASURE_SCREEN,
           VSP_SNAP_TO_SCREEN,
           VSP_PARASITE_DRAG_SCREEN,
           VSP_PARM_DEBUG_SCREEN,
           VSP_PARM_LINK_SCREEN,
           VSP_PARM_SCREEN,
           VSP_PROJECTION_SCREEN,
           VSP_PSLICE_SCREEN,
           VSP_SCREENSHOT_SCREEN,
           VSP_SET_EDITOR_SCREEN,
           VSP_STEP_OPTIONS_SCREEN,
           VSP_STEP_STRUCTURE_OPTIONS_SCREEN,
           VSP_STL_OPTIONS_SCREEN,
           VSP_STRUCT_SCREEN,
           VSP_SURFACE_INTERSECTION_SCREEN,
           VSP_SVG_OPTIONS_SCREEN,
           VSP_TYPE_EDITOR_SCREEN,
           VSP_USER_PARM_SCREEN,
           VSP_VAR_PRESET_SCREEN,
           VSP_VIEW_SCREEN,
           VSP_VSPAERO_PLOT_SCREEN,
           VSP_VSPAERO_SCREEN,
           VSP_XSEC_SCREEN,
           VSP_WAVEDRAG_SCREEN,
           VSP_MAIN_SCREEN,   /* Leave at end of list, helps draw after update. */
           VSP_NUM_SCREENS
         };

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

    SelectFileScreen* GetSelectFileScreen()
    {
        return &m_SelectFileScreen;
    }
    PickSetScreen* GetPickSetScreen()
    {
        return &m_PickSetScreen;
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
//        ( ( MainVSPScreen* ) m_ScreenVec[VSP_MAIN_SCREEN] )->ShowReturnToAPI();
    };
    void HideReturnToAPI()
    {
//        ( ( MainVSPScreen* ) m_ScreenVec[VSP_MAIN_SCREEN] )->HideReturnToAPI();
    };

    bool m_ShowPlotScreenOnce;

protected:

    void Init();

    static int GlobalHandler(int event);

    bool m_UpdateFlag;
    virtual void UpdateAllScreens();

    Vehicle* m_VehiclePtr;
    vector< VspScreen* > m_ScreenVec;

    SelectFileScreen m_SelectFileScreen;
    PickSetScreen m_PickSetScreen;

    virtual void TimerCB();
    static void StaticTimerCB( void* data )
    {
        static_cast< ScreenMgr* >( data )->TimerCB();
    }

    bool m_RunGUI;
};


#endif // !defined(SCREENMGR__INCLUDED_)
