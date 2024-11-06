//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// VehicleMgr.cpp: implementation of the Vehicle Class and Vehicle Mgr Singleton.
//
//////////////////////////////////////////////////////////////////////

#ifdef WIN32
#undef _HAS_STD_BYTE
#define _HAS_STD_BYTE 0
#endif

#include "FileUtil.h"

#include "AdvLinkScreen.h"
#include "AdvLinkVarRenameScreen.h"
#include "AeroStructScreen.h"
#include "AirfoilExportScreen.h"
#include "BEMOptionsScreen.h"
#include "CfdMeshScreen.h"
#include "ClippingScreen.h"
#include "CompGeomScreen.h"
#include "CurveEditScreen.h"
#include "DegenGeomScreen.h"
#include "DesignVarScreen.h"
#include "DXFOptionsScreen.h"
#include "ExportScreen.h"
#include "FeaPartEditScreen.h"
#include "FitModelScreen.h"
#include "IGESOptionsScreen.h"
#include "IGESStructureOptionsScreen.h"
#include "ImportScreen.h"
#include "ManageBackgroundScreen.h"
#include "ManageBackground3DScreen.h"
#include "ManageCORScreen.h"
#include "ManageGeomScreen.h"
#include "ManageMeasureScreen.h"
#include "ManageLightingScreen.h"
#include "ManageTextureScreen.h"
#include "ManageViewScreen.h"
#include "MassPropScreen.h"
#include "MaterialEditScreen.h"
#include "NerfManageGeomScreen.h"
#include "SnapToScreen.h"
#include "ParasiteDragScreen.h"
#include "ParmDebugScreen.h"
#include "ParmLinkScreen.h"
#include "ParmScreen.h"
#include "PreferencesScreen.h"
#include "ProjectionScreen.h"
#include "PSliceScreen.h"
#include "ScreenshotScreen.h"
#include "SetEditorScreen.h"
#include "STEPOptionsScreen.h"
#include "STEPStructureOptionsScreen.h"
#include "STLOptionsScreen.h"
#include "StructScreen.h"
#include "StructAssemblyScreen.h"
#include "SurfaceIntersectionScreen.h"
#include "SVGOptionsScreen.h"
#include "CustomGeomExportScreen.h"
#include "UserParmScreen.h"
#include "VarPresetScreen.h"
#include "VSPAEROPlotScreen.h"
#include "VSPAEROScreen.h"
#include "WaveDragScreen.h"

#include "MainThreadIDMgr.h"

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

    m_NativeFileChooser = NULL;

    m_RunGUI = true;

    m_ShowPlotScreenOnce = false;
    m_UpdateCount = 0;

    m_DisabledGUIElements.resize( vsp::NUM_GDEV_TYPES, false );
    m_DisabledGUIScreens.resize( vsp::VSP_NUM_SCREENS, false );
    m_DisabledGeomScreens.resize( vsp::NUM_GEOM_SCREENS, false );

    SetGUIScreenDisable( vsp::VSP_NERF_MANAGE_GEOM_SCREEN, true );
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

    if ( m_NativeFileChooser )
    {
        delete m_NativeFileChooser;
    }
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
            m_ScreenVec[vsp::VSP_MAIN_SCREEN]->Show();   //set main screen as "current" before show
            m_ScreenVec[vsp::VSP_VSPAERO_PLOT_SCREEN]->Show();
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
        // printf( "UpdateAllScreens %u\n", m_UpdateCount );
        m_UpdateCount++;
        MessageMgr::getInstance().Send( "UpdateCountMgr", "UpdateAllScreens" );

        SetUpdateFlag( true );
    }
    else if ( data.m_String == string( "VSPAEROSolverMessage" ) )
    {
        VSPAEROScreen* scr = ( VSPAEROScreen* ) m_ScreenVec[vsp::VSP_VSPAERO_SCREEN];
        if ( scr )
        {
            for ( int i = 0; i < (int)data.m_StringVec.size(); i++ )
            {
                scr->AddOutputText( scr->GetDisplay( VSPAERO_SOLVER ), data.m_StringVec[i] );
            }
        }
        // Also dump to aero-structure console.
        AeroStructScreen* asscr = ( AeroStructScreen* ) m_ScreenVec[vsp::VSP_AERO_STRUCT_SCREEN];
        if ( asscr )
        {
            for ( int i = 0; i < (int)data.m_StringVec.size(); i++ )
            {
                asscr->AddOutputText( data.m_StringVec[i] );
            }
        }
    }
    else if ( data.m_String == string( "FEAMessage" ) )
    {
        StructScreen* scr = ( StructScreen* ) m_ScreenVec[vsp::VSP_STRUCT_SCREEN];
        if ( scr )
        {
            for ( int i = 0; i < (int)data.m_StringVec.size(); i++ )
            {
                scr->AddOutputText( data.m_StringVec[i] );
            }
        }
        // Also dump to aero-structure console.
        AeroStructScreen* asscr = ( AeroStructScreen* ) m_ScreenVec[vsp::VSP_AERO_STRUCT_SCREEN];
        if ( asscr )
        {
            for ( int i = 0; i < (int)data.m_StringVec.size(); i++ )
            {
                asscr->AddOutputText( data.m_StringVec[i] );
            }
        }
        // And to structures assembly console.
        StructAssemblyScreen* assyscr = ( StructAssemblyScreen* ) m_ScreenVec[vsp::VSP_STRUCT_ASSEMBLY_SCREEN];
        if ( assyscr )
        {
            for ( int i = 0; i < (int)data.m_StringVec.size(); i++ )
            {
                assyscr->AddOutputText( data.m_StringVec[i] );
            }
        }
    }
    else if ( data.m_String == string( "AeroStructMessage" ) )
    {
        AeroStructScreen* asscr = ( AeroStructScreen* ) m_ScreenVec[vsp::VSP_AERO_STRUCT_SCREEN];
        if ( asscr )
        {
            for ( int i = 0; i < (int)data.m_StringVec.size(); i++ )
            {
                asscr->AddOutputText( data.m_StringVec[i] );
            }
        }
    }
    else if ( data.m_String == string( "CFDMessage" ) )
    {
        CfdMeshScreen* scr = (CfdMeshScreen*)m_ScreenVec[vsp::VSP_CFD_MESH_SCREEN];
        if ( scr )
        {
            for ( int i = 0; i < (int)data.m_StringVec.size(); i++ )
            {
                scr->AddOutputText( data.m_StringVec[i] );
            }
        }
    }
    else if ( data.m_String == string( "SurfIntersectMessage" ) )
    {
        SurfaceIntersectionScreen* scr = (SurfaceIntersectionScreen*)m_ScreenVec[vsp::VSP_SURFACE_INTERSECTION_SCREEN];
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
        string message;
        for ( int i = 0; i < (int)data.m_StringVec.size(); i++ )
        {
            message.append( data.m_StringVec[i] );
        }

        const char* msg = message.c_str();
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

bool ScreenMgr::CheckRunGui()
{
    return m_RunGUI;
}

void ScreenMgr::SetRunGui( bool r )
{
    m_RunGUI = r;
}

void ScreenMgr::EnableStopGUIMenuItem()
{
    (( MainVSPScreen * ) m_ScreenVec[ vsp::VSP_MAIN_SCREEN ] )->EnableStopGUIMenuItem();
}

void ScreenMgr::DisableStopGUIMenuItem()
{
    (( MainVSPScreen * ) m_ScreenVec[ vsp::VSP_MAIN_SCREEN ] )->DisableStopGUIMenuItem();
}

void ScreenMgr::EnableExitMenuItem()
{
    (( MainVSPScreen * ) m_ScreenVec[ vsp::VSP_MAIN_SCREEN ] )->EnableExitMenuItem();
}

void ScreenMgr::DisableExitMenuItem()
{
    (( MainVSPScreen * ) m_ScreenVec[ vsp::VSP_MAIN_SCREEN ] )->DisableExitMenuItem();
}

void ScreenMgr::APIHideScreensImplementation()
{
    // De-select all Geoms
    Vehicle* veh = VehicleMgr.GetVehicle();
    if ( veh )
    {
        veh->ClearActiveGeom();
    }

    ManageGeomScreen * geomScreen = dynamic_cast<ManageGeomScreen *> ( m_ScreenVec[ vsp::VSP_MANAGE_GEOM_SCREEN ] );
    if( geomScreen )
    {
        geomScreen->ShowHideGeomScreens();
    }

    // Screens are not stored, so store them.  If this is called twice (without a Show), do not
    // clear the stored list.
    if ( m_APIScreenOpenVec.size() == 0 )
    {
        m_APIScreenOpenVec.resize( vsp::VSP_NUM_SCREENS, false );

        for ( int i = 0; i < vsp::VSP_NUM_SCREENS; i++ )
        {
            if ( i == vsp::VSP_COR_SCREEN ) // Closing this causes crashes.
            {
                continue;
            }

            if ( m_ScreenVec[ i ] )
            {
                m_APIScreenOpenVec[ i ] = m_ScreenVec[ i ]->IsShown();
                if ( m_APIScreenOpenVec[ i ] )
                {
                    m_ScreenVec[ i ]->Hide();
                }
            }
        }
    }
}

void ScreenMgr::APIShowScreensImplementation()
{
    if ( m_APIScreenOpenVec.size() <= vsp::VSP_NUM_SCREENS )
    {
        for ( int i = 0; i < m_APIScreenOpenVec.size(); i++ )
        {
            if ( i == vsp::VSP_COR_SCREEN )
            {
                continue;
            }

            if ( m_APIScreenOpenVec[ i ] )
            {
                if ( m_ScreenVec[ i ] )
                {
                    m_ScreenVec[ i ]->Show();
                }
            }
        }
    }

    // Clear stored state.
    m_APIScreenOpenVec.clear();
}

void APIHideScreenHandler( void * data )
{
    ScreenMgr * m_ScreenMgr = (ScreenMgr*) data;
    if ( m_ScreenMgr )
    {
        m_ScreenMgr->APIHideScreensImplementation();
    }
}

void APIShowScreenHandler( void * data )
{
    ScreenMgr * m_ScreenMgr = (ScreenMgr*) data;
    if ( m_ScreenMgr )
    {
        m_ScreenMgr->APIShowScreensImplementation();
    }
}

void ScreenMgr::APIHideScreens()
{
    Fl::awake( APIHideScreenHandler, ( void* )this );
}

void ScreenMgr::APIShowScreens()
{
    Fl::awake( APIShowScreenHandler, ( void* )this );
}

void ScreenMgr::APIScreenGrabImplementation( const string & fname, int w, int h, bool transparentBG, bool autocrop )
{
    // Make sure most current OpenGL data has been collected and scene has been redrawn.
    ( ( MainVSPScreen* ) m_ScreenVec[vsp::VSP_MAIN_SCREEN] )->Update();

    ( ( MainVSPScreen* ) m_ScreenVec[vsp::VSP_MAIN_SCREEN] )->ScreenGrab( fname, w, h, transparentBG, autocrop );

    // Set flag that screen grab has been completed.
    m_ScreenGrabComplete = true;
}

struct ScreenGrabStruct {
    string m_fname;
    int m_w;
    int m_h;
    bool m_TransparentBG;
    bool m_AutoCrop;
    ScreenMgr * m_ScrMgr;
};

void APIScreenGrabHandler( void * data )
{
    ScreenGrabStruct * sg = ( ScreenGrabStruct * ) data;

    std::unique_lock lk( sg->m_ScrMgr->m_ScreenGrabMutex );

    sg->m_ScrMgr->APIScreenGrabImplementation( sg->m_fname, sg->m_w, sg->m_h, sg->m_TransparentBG, sg->m_AutoCrop );

    lk.unlock();

    sg->m_ScrMgr->m_ScreenGrabCV.notify_one();

    delete sg;
}

void ScreenMgr::APIScreenGrab( const string & fname, int w, int h, bool transparentBG, bool autocrop )
{
    // Mark that screen grab has not been completed.
    m_ScreenGrabComplete = false;

    if ( MainThreadIDMgr.IsCurrentThreadMain() )
    {
        // Simple main thread code path.
        APIScreenGrabImplementation( fname, w, h, transparentBG, autocrop );
    }
    else
    {
        // Works for Python under Facde.
        ScreenGrabStruct *sg = new ScreenGrabStruct;
        sg->m_fname = fname;
        sg->m_w = w;
        sg->m_h = h;
        sg->m_TransparentBG = transparentBG;
        sg->m_AutoCrop = autocrop;
        sg->m_ScrMgr = this;

        // Queue screen grab to main thread.
        Fl::awake( APIScreenGrabHandler, sg );

        // Release lock to allow main thread to process queue.
        Fl::unlock();

        // Set up lock and mutex.
        std::unique_lock lk( m_ScreenGrabMutex );

        // Wait for change in screen grab flag.
        m_ScreenGrabCV.wait(lk, [this]
            {
                return m_ScreenGrabComplete;
            });

        // Re-acquire lock from main thread.
        Fl::lock();
    }
}

bool ScreenMgr::IsGUIElementDisabled( int e ) const
{
    if ( e >= 0 && e < m_DisabledGUIElements.size() )
    {
        return m_DisabledGUIElements[ e ];
    }
    return false;
}

void ScreenMgr::SetGUIElementDisable( int e, bool state )
{
    if ( e == vsp::ALL_GDEV_TYPES )
    {
        for ( int i = 0; i < m_DisabledGUIElements.size(); i++ )
        {
            m_DisabledGUIElements[ i ] = state;
        }
    }
    else
    {
        if ( e >= 0 && e < m_DisabledGUIElements.size() )
        {
            m_DisabledGUIElements[ e ] = state;
        }
    }
}

bool ScreenMgr::IsGUIScreenDisabled( int s ) const
{
    if ( s >= 0 && s < m_DisabledGUIScreens.size() )
    {
        return m_DisabledGUIScreens[ s ];
    }
    return false;
}

void ScreenMgr::SetGUIScreenDisable( int s, bool state )
{
    if ( s == vsp::VSP_ALL_SCREENS )
    {
        for ( int i = 0; i < m_DisabledGUIScreens.size(); i++ )
        {
            m_DisabledGUIScreens[ i ] = state;
        }
    }
    else
    {
        if ( s >= 0 && s < m_DisabledGUIScreens.size() )
        {
            m_DisabledGUIScreens[ s ] = state;
        }
    }
}

bool ScreenMgr::IsGeomScreenDisabled( int s ) const
{
    if ( s >= 0 && s < m_DisabledGeomScreens.size() )
    {
        return m_DisabledGeomScreens[ s ];
    }
    return false;
}

void ScreenMgr::SetGeomScreenDisable( int s, bool state )
{
    if ( s == vsp::ALL_GEOM_SCREENS )
    {
        for ( int i = 0; i < m_DisabledGeomScreens.size(); i++ )
        {
            m_DisabledGeomScreens[ i ] = state;
        }
    }
    else
    {
        if ( s >= 0 && s < m_DisabledGeomScreens.size() )
        {
            m_DisabledGeomScreens[ s ] = state;
        }
    }
}

//==== Init All Screens ====//
void ScreenMgr::Init()
{
    //==== Build All Screens ====//
    m_ScreenVec.resize( vsp::VSP_NUM_SCREENS );
    m_ScreenVec[vsp::VSP_ADV_LINK_SCREEN] = new AdvLinkScreen( this );
    m_ScreenVec[vsp::VSP_ADV_LINK_VAR_RENAME_SCREEN] = new AdvLinkVarRenameScreen( this );
    m_ScreenVec[vsp::VSP_AERO_STRUCT_SCREEN] = new AeroStructScreen( this );
    m_ScreenVec[vsp::VSP_AIRFOIL_CURVES_EXPORT_SCREEN] = new BezierAirfoilExportScreen( this );
    m_ScreenVec[vsp::VSP_AIRFOIL_POINTS_EXPORT_SCREEN] = new SeligAirfoilExportScreen( this );
    m_ScreenVec[vsp::VSP_BACKGROUND_SCREEN] = new ManageBackgroundScreen( this );
    m_ScreenVec[vsp::VSP_BACKGROUND3D_SCREEN] = new ManageBackground3DScreen( this );
    m_ScreenVec[vsp::VSP_BACKGROUND3D_PREVIEW_SCREEN] = new Background3DPreviewScreen( this );
    m_ScreenVec[vsp::VSP_BEM_OPTIONS_SCREEN] = new BEMOptionsScreen( this );
    m_ScreenVec[vsp::VSP_CFD_MESH_SCREEN] = new CfdMeshScreen( this );
    m_ScreenVec[vsp::VSP_CLIPPING_SCREEN] = new ClippingScreen( this );
    m_ScreenVec[vsp::VSP_COMP_GEOM_SCREEN] = new CompGeomScreen( this );
    m_ScreenVec[vsp::VSP_COR_SCREEN] = new ManageCORScreen( this );
    m_ScreenVec[vsp::VSP_CURVE_EDIT_SCREEN] = new CurveEditScreen( this );
    m_ScreenVec[vsp::VSP_DEGEN_GEOM_SCREEN] = new DegenGeomScreen( this );
    m_ScreenVec[vsp::VSP_DESIGN_VAR_SCREEN] = new DesignVarScreen( this );
    m_ScreenVec[vsp::VSP_DXF_OPTIONS_SCREEN] = new DXFOptionsScreen( this);
    m_ScreenVec[vsp::VSP_EXPORT_SCREEN] = new ExportScreen( this );
    m_ScreenVec[vsp::VSP_FEA_PART_EDIT_SCREEN] = new FeaPartEditScreen( this );
    m_ScreenVec[vsp::VSP_FEA_XSEC_SCREEN] = new FeaXSecScreen( this );
    m_ScreenVec[vsp::VSP_FIT_MODEL_SCREEN] = new FitModelScreen( this );
    m_ScreenVec[vsp::VSP_IGES_OPTIONS_SCREEN] = new IGESOptionsScreen( this );
    m_ScreenVec[vsp::VSP_IGES_STRUCTURE_OPTIONS_SCREEN] = new IGESStructureOptionsScreen( this );
    m_ScreenVec[vsp::VSP_EXPORT_CUSTOM_SCRIPT] = new CustomGeomExportScreen( this );
    m_ScreenVec[vsp::VSP_IMPORT_SCREEN] = new ImportScreen( this );
    m_ScreenVec[vsp::VSP_MEASURE_SCREEN] = new ManageMeasureScreen( this );
    m_ScreenVec[vsp::VSP_LIGHTING_SCREEN] = new ManageLightingScreen( this );
    m_ScreenVec[vsp::VSP_MAIN_SCREEN] = new MainVSPScreen( this  );
    m_ScreenVec[vsp::VSP_MANAGE_GEOM_SCREEN] = new ManageGeomScreen( this );
    m_ScreenVec[vsp::VSP_MANAGE_TEXTURE_SCREEN] = new ManageTextureScreen( this );
    m_ScreenVec[vsp::VSP_MASS_PROP_SCREEN] = new MassPropScreen( this );
    m_ScreenVec[vsp::VSP_MATERIAL_EDIT_SCREEN] = new MaterialEditScreen( this );
    m_ScreenVec[vsp::VSP_NERF_MANAGE_GEOM_SCREEN] = new NerfManageGeomScreen( this );
    m_ScreenVec[vsp::VSP_SNAP_TO_SCREEN] = new SnapToScreen( this );
    m_ScreenVec[vsp::VSP_PARASITE_DRAG_SCREEN] = new ParasiteDragScreen( this );
    m_ScreenVec[vsp::VSP_PARM_DEBUG_SCREEN] = new ParmDebugScreen( this );
    m_ScreenVec[vsp::VSP_PARM_LINK_SCREEN] = new ParmLinkScreen( this );
    m_ScreenVec[vsp::VSP_PARM_SCREEN] = new ParmScreen( this );
    m_ScreenVec[vsp::VSP_PICK_SET_SCREEN] = new PickSetScreen( this );
    m_ScreenVec[vsp::VSP_PREFERENCES_SCREEN] = new PreferencesScreen( this );
    m_ScreenVec[vsp::VSP_PROJECTION_SCREEN] = new ProjectionScreen( this );
    m_ScreenVec[vsp::VSP_PSLICE_SCREEN] = new PSliceScreen( this );
    m_ScreenVec[vsp::VSP_SCREENSHOT_SCREEN] = new ScreenshotScreen( this );
    m_ScreenVec[vsp::VSP_SELECT_FILE_SCREEN] = new SelectFileScreen( this );
    m_ScreenVec[vsp::VSP_SET_EDITOR_SCREEN] = new SetEditorScreen( this );
    m_ScreenVec[vsp::VSP_STEP_OPTIONS_SCREEN] = new STEPOptionsScreen( this );
    m_ScreenVec[vsp::VSP_STEP_STRUCTURE_OPTIONS_SCREEN] = new STEPStructureOptionsScreen( this );
    m_ScreenVec[vsp::VSP_STL_OPTIONS_SCREEN] = new STLOptionsScreen( this );
    m_ScreenVec[vsp::VSP_STRUCT_SCREEN] = new StructScreen( this );
    m_ScreenVec[vsp::VSP_STRUCT_ASSEMBLY_SCREEN] = new StructAssemblyScreen( this );
    m_ScreenVec[vsp::VSP_SURFACE_INTERSECTION_SCREEN] = new SurfaceIntersectionScreen( this );
    m_ScreenVec[vsp::VSP_SVG_OPTIONS_SCREEN] = new SVGOptionsScreen( this );
    m_ScreenVec[vsp::VSP_USER_PARM_SCREEN] = new UserParmScreen( this );
    m_ScreenVec[vsp::VSP_VIEW_SCREEN] = new ManageViewScreen( this );
    m_ScreenVec[vsp::VSP_VAR_PRESET_SCREEN] = new VarPresetScreen( this );
    m_ScreenVec[vsp::VSP_VSPAERO_PLOT_SCREEN] = new VSPAEROPlotScreen( this );
    m_ScreenVec[vsp::VSP_VSPAERO_SCREEN] = new VSPAEROScreen( this );
    m_ScreenVec[vsp::VSP_WAVEDRAG_SCREEN] = new WaveDragScreen( this );
    m_ScreenVec[vsp::VSP_XSEC_SCREEN] = new XSecViewScreen( this );

    m_ScreenVec[vsp::VSP_MAIN_SCREEN]->Show();

    // Set manage geom screen to show up to the main screen as the default.
    int x,y,w,h1,h2;
    x = m_ScreenVec[vsp::VSP_MAIN_SCREEN]->GetFlWindow()->x();
    y = m_ScreenVec[vsp::VSP_MAIN_SCREEN]->GetFlWindow()->y();
    w = m_ScreenVec[vsp::VSP_MAIN_SCREEN]->GetFlWindow()->w();
    h1 = m_ScreenVec[vsp::VSP_MAIN_SCREEN]->GetFlWindow()->h();

    m_ScreenVec[vsp::VSP_MANAGE_GEOM_SCREEN]->GetFlWindow()->position(x+w+5,y);
    m_ScreenVec[vsp::VSP_NERF_MANAGE_GEOM_SCREEN]->GetFlWindow()->position(x+w+5,y);

    h2 = m_ScreenVec[vsp::VSP_XSEC_SCREEN]->GetFlWindow()->h();
    m_ScreenVec[vsp::VSP_XSEC_SCREEN]->GetFlWindow()->position( x + w + 5, y + h1 - h2 );

    x = m_ScreenVec[vsp::VSP_MANAGE_GEOM_SCREEN]->GetFlWindow()->x();
    y = m_ScreenVec[vsp::VSP_MANAGE_GEOM_SCREEN]->GetFlWindow()->y();
    w = m_ScreenVec[vsp::VSP_MANAGE_GEOM_SCREEN]->GetFlWindow()->w();

    VSP_Window::SetGeomX( x + w );
    VSP_Window::SetGeomY( y );

    for ( int i = 0 ; i < ( int )m_ScreenVec.size() ; i++ )
    {
        m_ScreenVec[i]->SetScreenType( i );

        if( i != vsp::VSP_MAIN_SCREEN && i != vsp::VSP_COR_SCREEN )
        {
            m_ScreenVec[i]->GetFlWindow()->set_non_modal();
        }
    }

    // Show() after setting non_modal, as modality can not change if window shown.
    m_ScreenVec[vsp::VSP_MANAGE_GEOM_SCREEN]->Show();
    m_ScreenVec[vsp::VSP_NERF_MANAGE_GEOM_SCREEN]->Show();
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
        if ( m_ScreenVec[i]->IsShown() || (i == vsp::VSP_MANAGE_GEOM_SCREEN) )
        {
            m_ScreenVec[i]->Update();
        }
    }
}

//==== Show Screen ====//
void ScreenMgr::ShowScreen( int id )
{
    if ( id >= 0 && id < vsp::VSP_NUM_SCREENS )
    {
        m_ScreenVec[id]->Show();
    }
}

//==== Hide Screen ====//
void ScreenMgr::HideScreen( int id )
{
    if ( id >= 0 && id < vsp::VSP_NUM_SCREENS )
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
    if( id >= 0 && id < vsp::VSP_NUM_SCREENS )
    {
        return m_ScreenVec[id];
    }
    // Should not reach here.
    assert( false );
    return NULL;
}

void ScreenMgr::LaunchSystemDefaultBrowser( const string &url )
{
#if     defined(__APPLE__)
        system( string( "open " + url ).c_str() );
#elif   defined(_WIN32) || defined(WIN32)
        ShellExecute( NULL, "open", url.c_str(),
                      NULL, NULL, SW_SHOWNORMAL );
#else
        system( string( "xdg-open " + url ).c_str() );
#endif
}

void ScreenMgr::HelpSystemDefaultBrowser( const string &file )
{
    Vehicle *veh = VehicleMgr.GetVehicle();

    string exepath = veh->GetExePath();

    string url = "file://" + exepath + "/help/" + file;

    LaunchSystemDefaultBrowser( url );
}

string ScreenMgr::FileChooser( const string &title, const string &filter, int mode, const string &dir )
{
    Fl_Preferences prefs( Fl_Preferences::USER, "openvsp.org", "VSP" );
    Fl_Preferences app( prefs, "Application" );

    int fc_type = vsp::FC_OPENVSP;
    app.get( "File_Chooser", fc_type, vsp::FC_OPENVSP );

    if ( fc_type == vsp::FC_OPENVSP )
    {
        SelectFileScreen * sfc = ( SelectFileScreen * ) m_ScreenVec[ vsp::VSP_SELECT_FILE_SCREEN ];
        return sfc->FileChooser( title, filter, mode, dir );
    }
    else // FC_NATIVE
    {
        return NativeFileChooser( title, filter, mode, dir );
    }
}

string ScreenMgr::NativeFileChooser( const string &title, const string &filter, int mode, const string &dir )
{
    if ( !m_NativeFileChooser )
    {
        m_NativeFileChooser = new Fl_Native_File_Chooser();
    }

    if ( mode == vsp::OPEN )
    {
        m_NativeFileChooser->type( Fl_Native_File_Chooser::BROWSE_FILE );
        m_NativeFileChooser->options( Fl_Native_File_Chooser::PREVIEW );
    }
    else
    {
        m_NativeFileChooser->type( Fl_Native_File_Chooser::BROWSE_SAVE_FILE );
        m_NativeFileChooser->options( Fl_Native_File_Chooser::NEW_FOLDER | Fl_Native_File_Chooser::SAVEAS_CONFIRM | Fl_Native_File_Chooser::USE_FILTER_EXT );
    }

    if ( !dir.empty() )
    {
        m_NativeFileChooser->directory( dir.c_str() );
    }

    bool forceext = true;
    if ( filter.find( ',' ) != string::npos ) // A comma was found
    {
        forceext = false;
    }

    m_NativeFileChooser->title( title.c_str() );
    m_NativeFileChooser->filter( filter.c_str() );

    string fname;
    switch ( m_NativeFileChooser->show() )
    {
        case -1:
            printf( "ERROR: %s\n", m_NativeFileChooser->errmsg() );
            break;
        case 1:
            // Cancel
            break;
        default:
            fname = string( m_NativeFileChooser->filename() );

            if ( forceext )
            {
                ::EnforceFilter( fname, filter );
            }

            break;
    }

    return fname;
}

//==== Create Message Pop-Up ====//
void MessageBox( void * data )
{
    fl_message( "%s", (char*)data );
    ((char*)data)[0] = '\0';
    delete[] (char*)data;
}

//==== Create Pop-Up Message Window ====//
void ScreenMgr::Alert( const string &message )
{
    char *buf = new char[ message.size() + 1 ];
    memcpy( buf, &message[0], message.size() );
    buf[message.size()] = '\0';
    Fl::awake( MessageBox, ( void* )buf );
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
