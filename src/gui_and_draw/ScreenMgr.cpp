//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// VehicleMgr.cpp: implementation of the Vehicle Class and Vehicle Mgr Singleton.
//
//////////////////////////////////////////////////////////////////////

#include "AdvLinkScreen.h"
#include "BEMOptionsScreen.h"
#include "CfdMeshScreen.h"
#include "ClippingScreen.h"
#include "CompGeomScreen.h"
#include "DegenGeomScreen.h"
#include "DesignVarScreen.h"
#include "DXFOptionsScreen.h"
#include "ExportScreen.h"
#include "FeaStructScreen.h"
#include "FitModelScreen.h"
#include "IGESOptionsScreen.h"
#include "ImportScreen.h"
#include "ManageBackgroundScreen.h"
#include "ManageCORScreen.h"
#include "ManageGeomScreen.h"
#include "ManageMeasureScreen.h"
#include "ManageLightingScreen.h"
#include "ManageTextureScreen.h"
#include "ManageViewScreen.h"
#include "MassPropScreen.h"
#include "MaterialEditScreen.h"
#include "SnapToScreen.h"
#include "ParasiteDragScreen.h"
#include "ParmDebugScreen.h"
#include "ParmLinkScreen.h"
#include "ParmScreen.h"
#include "ProjectionScreen.h"
#include "PSliceScreen.h"
#include "ScreenshotScreen.h"
#include "SetEditorScreen.h"
#include "STEPOptionsScreen.h"
#include "STLOptionsScreen.h"
#include "SVGOptionsScreen.h"
#include "TypeEditorScreen.h"
#include "UserParmScreen.h"
#include "VarPresetScreen.h"
#include "VSPAEROPlotScreen.h"
#include "VSPAEROScreen.h"
#include "WaveDragScreen.h"

#define UPDATE_TIME (1.0/30.0)

//==== Constructor ====//
ScreenMgr::ScreenMgr( Vehicle* vPtr )
{
    if ( vPtr )
    {
        m_VehiclePtr = vPtr;
        Init();
    }
    m_UpdateFlag = true;
    MessageBase::Register( string( "ScreenMgr" ) );

    Fl::scheme( "GTK+" );
    Fl::add_timeout( UPDATE_TIME, StaticTimerCB, this );
    Fl::add_handler( GlobalHandler );

    m_RunGUI = true;

    m_ShowPlotScreenOnce = false;

}

//==== Destructor ====//
ScreenMgr::~ScreenMgr()
{
    for ( int i = 0 ; i < ( int )m_ScreenVec.size() ; i++ )
    {
        delete m_ScreenVec[i];
    }
    m_ScreenVec.clear();
    MessageMgr::getInstance().UnRegister( this );

}

//==== Force Update ====//
void ScreenMgr::ForceUpdate()
{
    m_UpdateFlag = false;
    UpdateAllScreens();
}

//==== Timer Callback ====//
void ScreenMgr::TimerCB()
{
    if ( m_UpdateFlag )
    {
        if (m_ShowPlotScreenOnce)
        {
            m_ShowPlotScreenOnce = false;
            m_ScreenVec[VSP_MAIN_SCREEN]->Show();   //set main screen as "current" before show
            m_ScreenVec[VSP_VSPAERO_PLOT_SCREEN]->Show();
        }
        m_UpdateFlag = false;
        UpdateAllScreens();
    }

    Fl::repeat_timeout( UPDATE_TIME, StaticTimerCB, this );
}


//==== Set Update Flag ====//
void ScreenMgr::SetUpdateFlag( bool flag )
{
    m_UpdateFlag = flag;
}

//==== Message Callbacks ====//
void ScreenMgr::MessageCallback( const MessageBase* from, const MessageData& data )
{
    if ( data.m_String == string( "UpdateAllScreens" ) )
    {
        SetUpdateFlag( true );
    }
    else if ( data.m_String == string( "VSPAEROSolverMessage" ) )
    {
        VSPAEROScreen* scr = ( VSPAEROScreen* ) m_ScreenVec[VSP_VSPAERO_SCREEN];
        if ( scr )
        {
            for ( int i = 0; i < (int)data.m_StringVec.size(); i++ )
            {
                scr->AddOutputText( scr->GetDisplay( VSPAERO_SOLVER ), data.m_StringVec[i] );
            }
        }
    }
    else if ( data.m_String == string( "FEAMessage" ) )
    {
        FeaStructScreen* scr = ( FeaStructScreen* ) m_ScreenVec[VSP_FEA_MESH_SCREEN];
        if ( scr )
        {
            for ( int i = 0; i < (int)data.m_StringVec.size(); i++ )
            {
                scr->AddOutputText( data.m_StringVec[i] );
            }
        }
    }
    else if ( data.m_String == string( "Error" ) )
    {
        const char* msg = data.m_StringVec[0].c_str();
        fl_message( "%s", ( char* )msg );
    }
    else if ( data.m_String == string( "CheckCollisionKey" ) )
    {
        SnapTo* snap = VehicleMgr.GetVehicle()->GetSnapToPtr();
        if ( snap )
        {
            if ( Fl::event_alt()  )
                snap->m_CollisionDetection = true;
            else
                snap->m_CollisionDetection = false;
        }
    }
}

//==== Init All Screens ====//
void ScreenMgr::Init()
{
    //==== Build All Screens ====//
    m_ScreenVec.resize( VSP_NUM_SCREENS );
    m_ScreenVec[VSP_ADV_LINK_SCREEN] = new AdvLinkScreen( this );
    m_ScreenVec[VSP_BACKGROUND_SCREEN] = new ManageBackgroundScreen( this );
    m_ScreenVec[VSP_BEM_OPTIONS_SCREEN] = new BEMOptionsScreen( this );
    m_ScreenVec[VSP_CFD_MESH_SCREEN] = new CfdMeshScreen( this );
    m_ScreenVec[VSP_CLIPPING_SCREEN] = new ClippingScreen( this );
    m_ScreenVec[VSP_COMP_GEOM_SCREEN] = new CompGeomScreen( this );
    m_ScreenVec[VSP_COR_SCREEN] = new ManageCORScreen( this );
    m_ScreenVec[VSP_DEGEN_GEOM_SCREEN] = new DegenGeomScreen( this );
    m_ScreenVec[VSP_DESIGN_VAR_SCREEN] = new DesignVarScreen( this );
    m_ScreenVec[VSP_DXF_OPTIONS_SCREEN] = new DXFOptionsScreen( this);
    m_ScreenVec[VSP_EXPORT_SCREEN] = new ExportScreen( this );
    m_ScreenVec[VSP_FEA_MESH_SCREEN] = new FeaStructScreen( this );
    m_ScreenVec[VSP_FIT_MODEL_SCREEN] = new FitModelScreen( this );
    m_ScreenVec[VSP_IGES_OPTIONS_SCREEN] = new IGESOptionsScreen( this );
    m_ScreenVec[VSP_IMPORT_SCREEN] = new ImportScreen( this );
    m_ScreenVec[VSP_MEASURE_SCREEN] = new ManageMeasureScreen( this );
    m_ScreenVec[VSP_LIGHTING_SCREEN] = new ManageLightingScreen( this );
    m_ScreenVec[VSP_MAIN_SCREEN] = new MainVSPScreen( this  );
    m_ScreenVec[VSP_MANAGE_GEOM_SCREEN] = new ManageGeomScreen( this );
    m_ScreenVec[VSP_MANAGE_TEXTURE_SCREEN] = new ManageTextureScreen( this );
    m_ScreenVec[VSP_MASS_PROP_SCREEN] = new MassPropScreen( this );
    m_ScreenVec[VSP_MATERIAL_EDIT_SCREEN] = new MaterialEditScreen( this );
    m_ScreenVec[VSP_SNAP_TO_SCREEN] = new SnapToScreen( this );
    m_ScreenVec[VSP_PARASITE_DRAG_SCREEN] = new ParasiteDragScreen( this );
    m_ScreenVec[VSP_PARM_DEBUG_SCREEN] = new ParmDebugScreen( this );
    m_ScreenVec[VSP_PARM_LINK_SCREEN] = new ParmLinkScreen( this );
    m_ScreenVec[VSP_PARM_SCREEN] = new ParmScreen( this );
    m_ScreenVec[VSP_PROJECTION_SCREEN] = new ProjectionScreen( this );
    m_ScreenVec[VSP_PSLICE_SCREEN] = new PSliceScreen( this );
    m_ScreenVec[VSP_SCREENSHOT_SCREEN] = new ScreenshotScreen( this );
    m_ScreenVec[VSP_SET_EDITOR_SCREEN] = new SetEditorScreen( this );
    m_ScreenVec[VSP_STEP_OPTIONS_SCREEN] = new STEPOptionsScreen( this );
    m_ScreenVec[VSP_STL_OPTIONS_SCREEN] = new STLOptionsScreen( this );
    m_ScreenVec[VSP_SVG_OPTIONS_SCREEN] = new SVGOptionsScreen( this );
    m_ScreenVec[VSP_TYPE_EDITOR_SCREEN] = new TypeEditorScreen( this );
    m_ScreenVec[VSP_USER_PARM_SCREEN] = new UserParmScreen( this );
    m_ScreenVec[VSP_VIEW_SCREEN] = new ManageViewScreen( this );
    m_ScreenVec[VSP_VAR_PRESET_SCREEN] = new VarPresetScreen( this );
    m_ScreenVec[VSP_VSPAERO_PLOT_SCREEN] = new VSPAEROPlotScreen( this );
    m_ScreenVec[VSP_VSPAERO_SCREEN] = new VSPAEROScreen( this );
    m_ScreenVec[VSP_WAVEDRAG_SCREEN] = new WaveDragScreen( this );
    m_ScreenVec[VSP_XSEC_SCREEN] = new XSecViewScreen( this );

    m_ScreenVec[VSP_MAIN_SCREEN]->Show();

    // Set manage geom screen to show up to the main screen as the default.
    int x,y,w,h1,h2;
    x = m_ScreenVec[VSP_MAIN_SCREEN]->GetFlWindow()->x();
    y = m_ScreenVec[VSP_MAIN_SCREEN]->GetFlWindow()->y();
    w = m_ScreenVec[VSP_MAIN_SCREEN]->GetFlWindow()->w();
    h1 = m_ScreenVec[VSP_MAIN_SCREEN]->GetFlWindow()->h();

    m_ScreenVec[VSP_MANAGE_GEOM_SCREEN]->GetFlWindow()->position(x+w+5,y);

    h2 = m_ScreenVec[VSP_XSEC_SCREEN]->GetFlWindow()->h();
    m_ScreenVec[VSP_XSEC_SCREEN]->GetFlWindow()->position( x + w + 5, y + h1 - h2 );

    x = m_ScreenVec[VSP_MANAGE_GEOM_SCREEN]->GetFlWindow()->x();
    y = m_ScreenVec[VSP_MANAGE_GEOM_SCREEN]->GetFlWindow()->y();
    w = m_ScreenVec[VSP_MANAGE_GEOM_SCREEN]->GetFlWindow()->w();

    VSP_Window::SetGeomX( x + w );
    VSP_Window::SetGeomY( y );

    for ( int i = 0 ; i < ( int )m_ScreenVec.size() ; i++ )
    {
        if( i != VSP_MAIN_SCREEN && i != VSP_COR_SCREEN )
        {
            m_ScreenVec[i]->GetFlWindow()->set_non_modal();
        }
    }

    // Show() after setting non_modal, as modality can not change if window shown.
    m_ScreenVec[VSP_MANAGE_GEOM_SCREEN]->Show();
}

//==== Update All Displayed Screens ====//
void ScreenMgr::UpdateAllScreens()
{
//static int last_tics = timeGetTime();
//int del_tics = timeGetTime() - last_tics;
//last_tics = timeGetTime();
//printf("Update Screens %d\n",  del_tics );
    for ( int i = 0 ; i < ( int )m_ScreenVec.size() ; i++ )
    {
        //===== Force Update Of ManageGeomScreen ====//
        if ( m_ScreenVec[i]->IsShown() || (i == VSP_MANAGE_GEOM_SCREEN) )
        {
            m_ScreenVec[i]->Update();
        }
    }
}

//==== Show Screen ====//
void ScreenMgr::ShowScreen( int id )
{
    if ( id >= 0 && id < VSP_NUM_SCREENS )
    {
        m_ScreenVec[id]->Show();
    }
}

//==== Hide Screen ====//
void ScreenMgr::HideScreen( int id )
{
    if ( id >= 0 && id < VSP_NUM_SCREENS )
    {
        m_ScreenVec[id]->Hide();
    }
}

//==== Get Current Geometry To Edit ====//
Geom* ScreenMgr::GetCurrGeom()
{
    vector< Geom* > select_vec = GetVehiclePtr()->GetActiveGeomPtrVec();

    if ( select_vec.size() != 1 )
    {
        return NULL;
    }
    return select_vec[0];
}

VspScreen * ScreenMgr::GetScreen( int id )
{
    if( id >= 0 && id < VSP_NUM_SCREENS )
    {
        return m_ScreenVec[id];
    }
    // Should not reach here.
    assert( false );
    return NULL;
}

//==== Create Message Pop-Up ====//
void MessageBox( void * data )
{
    fl_message( "%s", ( char* )data );
}

//==== Create Pop-Up Message Window ====//
void ScreenMgr::Alert( const char * message )
{
    Fl::awake( MessageBox, ( void* )message );
}

int ScreenMgr::GlobalHandler(int event)
{
    if (Fl::event()==FL_SHORTCUT && Fl::event_key()==FL_Escape)
    {
        Vehicle* vPtr = VehicleMgr.GetVehicle();
        if ( vPtr )
        {
            vector< string > none;
            vPtr->SetActiveGeomVec( none );
            MessageMgr::getInstance().Send( "ScreenMgr", "UpdateAllScreens" );
        }
        return 1;
    }
    return 0;
}
