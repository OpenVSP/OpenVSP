//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

//
// Programmatic main GUI window
// Rob McDonald
//
//////////////////////////////////////////////////////////////////////

#ifdef WIN32
#undef _HAS_STD_BYTE
#define _HAS_STD_BYTE 0
#endif

#include "main.h"
#include "MainVSPScreen.h"
#include "MainGLWindow.h"
#include "ScreenMgr.h"
#include "GraphicEngine.h"
#include "Display.h"
#include "Camera.h"
#include "LayoutMgr.h"
#include "Viewport.h"
#include "Background.h"
#include <FL/fl_ask.H>
#include "ManageCORScreen.h"
#include "ManageGeomScreen.h"
#include "ManageViewScreen.h"
#include "FileUtil.h"

#include "CfdMeshScreen.h"
#include "SurfaceIntersectionScreen.h"
#include "StructScreen.h"
#include "StructAssemblyScreen.h"

using namespace vsp;

using namespace vsp;

//==== Constructor ====//
MainVSPScreen::MainVSPScreen( ScreenMgr* mgr ) : ActionScreen( mgr )
{
    m_FLTK_Window = NULL;
    m_GlWin = NULL;

    m_ShowXYZArrow = true;
    m_ShowBorder = true;

    int x, y, w, h, side;
    w = 500;
    h = 500;

    //==== Window ====//
    m_FLTK_Window = new VSP_Window( w, h );
    m_FLTK_Window->resizable( m_FLTK_Window );
    VspScreen::SetFlWindow( m_FLTK_Window );

    Fl::screen_xywh( x, y, w, h );

    // Figure out which is smaller, half the screen width or the height
    if ( 0.5 * w < 0.9 * h )
    {
        side = (int)(0.9 * h);
    }
    else
    {
        side = (int)(0.5 * w);
    }

    m_FLTK_Window->resize( x + 10, y + 30, side, side );

    m_FLTK_Window->callback( staticCloseCB, this );

    Fl_Group * grp = new Fl_Group( 0, 0, side, side );
    m_FLTK_Window->add( grp );
    m_GenLayout.SetGroupAndScreen( grp, this );

    m_MenuBar = m_GenLayout.AddFlMenuBar( 30 );

    m_NewMenuItem.Init( this, m_MenuBar, "File/New" );
    m_OpenMenuItem.Init( this, m_MenuBar, "File/Open..." );
    m_SaveMenuItem.Init( this, m_MenuBar, "File/Save...", FL_COMMAND + 's' );
    m_SaveAsMenuItem.Init( this, m_MenuBar, "File/Save As..." );
    m_SaveSetMenuItem.Init( this, m_MenuBar, "File/Save Set..." );
    m_InsertMenuItem.Init( this, m_MenuBar, "File/Insert..." );
    m_ImportMenuItem.Init( mgr, m_MenuBar, "File/Import...", vsp::VSP_IMPORT_SCREEN );
    m_ExportMenuItem.Init( mgr, m_MenuBar, "File/Export...", vsp::VSP_EXPORT_SCREEN );
    // m_SetTempDirMenuItem.Init( this, m_MenuBar, "File/Set Temp Dir..." );
    m_RunScriptMenuItem.Init( this, m_MenuBar, "File/Run Script..." );
    m_PreferencesMenuItem.Init( mgr, m_MenuBar, "File/Preferences...", vsp::VSP_PREFERENCES_SCREEN );
    m_ExitMenuItem.Init( this, m_MenuBar, "File/Exit" );
    m_StopGUIMenuItem.Init( this, m_MenuBar, "File/Stop GUI" );
    m_StopGUIMenuItem.Hide();

    m_UndoMenuItem.Init( this, m_MenuBar, "Edit/Undo Parameter Change", FL_COMMAND + 'z' );
    m_CutMenuItem.Init( this, m_MenuBar, "Edit/Cut", FL_COMMAND + 'x' );
    m_CopyMenuItem.Init( this, m_MenuBar, "Edit/Copy", FL_COMMAND + 'c' );
    m_PasteMenuItem.Init( this, m_MenuBar, "Edit/Paste", FL_COMMAND + 'v' );
    m_DeleteMenuItem.Init( this, m_MenuBar, "Edit/Delete" );
    m_SelAllMenuItem.Init( this, m_MenuBar, "Edit/Select All", FL_COMMAND + 'a' );
    m_PickMenuItem.Init( this, m_MenuBar, "Edit/Toggle Pick Mode", 'p' );

    m_OneMenuItem.Init( this, m_MenuBar, "Window/One" );
    m_FourMenuItem.Init( this, m_MenuBar, "Window/Four" );
    m_TwoHMenuItem.Init( this, m_MenuBar, "Window/Two Horizontal" );
    m_TwoVMenuItem.Init( this, m_MenuBar, "Window/Two Vertical" );
    m_BackgroundMenuItem.Init( mgr, m_MenuBar, "Window/Background...", vsp::VSP_BACKGROUND_SCREEN );
    m_Background3DMenuItem.Init( mgr, m_MenuBar, "Window/3D Background...", vsp::VSP_BACKGROUND3D_SCREEN );
    m_ScreenshotMenuItem.Init( mgr, m_MenuBar, "Window/Screenshot...", vsp::VSP_SCREENSHOT_SCREEN );
    m_AxisMenuItem.Init( this, m_MenuBar, "Window/Axis Toggle" );
    m_AxisMenuItem.Update( m_ShowXYZArrow );
    m_BorderMenuItem.Init( this, m_MenuBar, "Window/Border Toggle" );
    m_BorderMenuItem.Update( m_ShowBorder );

    m_TopMenuItem.Init( this, m_MenuBar, "View/Top", FL_F + 5 );
    m_FrontMenuItem.Init( this, m_MenuBar, "View/Front", FL_F + 6 );
    m_LeftSideMenuItem.Init( this, m_MenuBar, "View/Left", FL_F + 7 );
    m_LeftIsoMenuItem.Init( this, m_MenuBar, "View/Left Iso", FL_F + 8 );
    m_BottomMenuItem.Init( this, m_MenuBar, "View/Bottom", FL_F + 9 );
    m_RearMenuItem.Init( this, m_MenuBar, "View/Rear", FL_F + 10 );
    m_RightSideMenuItem.Init( this, m_MenuBar, "View/Right", FL_F + 11 );
    m_RightIsoMenuItem.Init( this, m_MenuBar, "View/Right Iso", FL_F + 12 );
    m_SetCORMenuItem.Init( this, m_MenuBar, "View/Set Rotation Center", 'r' );
    m_CenterMenuItem.Init( this, m_MenuBar, "View/Center", 'c' );
    m_CenterAllMenuItem.Init( this, m_MenuBar, "View/Center All", 'C' );
    m_FitViewMenuItem.Init( this, m_MenuBar, "View/Fit On Screen", 'f' );
    m_FitAllViewMenuItem.Init( this, m_MenuBar, "View/Fit All On Screen", 'F' );
    m_ResetAllViewMenuItem.Init( this, m_MenuBar, "View/Reset All", 'R' );
    m_AdjustMenuItem.Init( mgr, m_MenuBar, "View/Adjust...", vsp::VSP_VIEW_SCREEN );
    // m_AntialiasMenuItem.Init( this, m_MenuBar, "View/New" );

    m_GeomMenuItem.Init( mgr, m_MenuBar, "Model/Geometry...", vsp::VSP_MANAGE_GEOM_SCREEN, vsp::VSP_NERF_MANAGE_GEOM_SCREEN );
    m_SetEditMenuItem.Init( mgr, m_MenuBar, "Model/Set Editor...", vsp::VSP_SET_EDITOR_SCREEN );
    m_LinkMenuItem.Init( mgr, m_MenuBar, "Model/Link...", vsp::VSP_PARM_LINK_SCREEN );
    m_DesVarMenuItem.Init( mgr, m_MenuBar, "Model/Design Variables...", vsp::VSP_DESIGN_VAR_SCREEN );
    m_LabelMenuItem.Init( mgr, m_MenuBar, "Model/Measure...", vsp::VSP_MEASURE_SCREEN );
    m_LightMenuItem.Init( mgr, m_MenuBar, "Model/Lighting...", vsp::VSP_LIGHTING_SCREEN );
    m_ClipMenuItem.Init( mgr, m_MenuBar, "Model/Clipping...", vsp::VSP_CLIPPING_SCREEN );
    m_TextureMenuItem.Init( mgr, m_MenuBar, "Model/Texture...", vsp::VSP_MANAGE_TEXTURE_SCREEN );
    m_AdvLinkMenuItem.Init( mgr, m_MenuBar, "Model/Adv Link...", vsp::VSP_ADV_LINK_SCREEN );
    m_UserParmMenuItem.Init( mgr, m_MenuBar, "Model/User Parms...", vsp::VSP_USER_PARM_SCREEN );
    m_FitModelMenuItem.Init( mgr, m_MenuBar, "Model/Fit Model...", vsp::VSP_FIT_MODEL_SCREEN );
    m_SnapToMenuItem.Init( mgr, m_MenuBar, "Model/Snap To...", vsp::VSP_SNAP_TO_SCREEN );
    m_VarPresetMenuItem.Init( mgr, m_MenuBar, "Model/Variable Presets...", vsp::VSP_VAR_PRESET_SCREEN );

    m_CompGeomMenuItem.Init( mgr, m_MenuBar, "Analysis/CompGeom...", vsp::VSP_COMP_GEOM_SCREEN );
    m_PSliceMenuItem.Init( mgr, m_MenuBar, "Analysis/Planar Slice...", vsp::VSP_PSLICE_SCREEN );
    m_MassPropMenuItem.Init( mgr, m_MenuBar, "Analysis/Mass Prop...", vsp::VSP_MASS_PROP_SCREEN );
    m_ProjectionMenuItem.Init( mgr, m_MenuBar, "Analysis/Projected Area...", vsp::VSP_PROJECTION_SCREEN );
    m_DegenGeomMenuItem.Init( mgr, m_MenuBar, "Analysis/DegenGeom...", vsp::VSP_DEGEN_GEOM_SCREEN );
    m_SurfIntMenuItem.Init( mgr, m_MenuBar, "Analysis/Trimmed Surfaces...", vsp::VSP_SURFACE_INTERSECTION_SCREEN );
    m_CFDMeshMenuItem.Init( mgr, m_MenuBar, "Analysis/CFD Mesh...", vsp::VSP_CFD_MESH_SCREEN );

    m_StructMenuItem.Init( mgr, m_MenuBar, "Analysis/Structure/FEA Structure...", vsp::VSP_STRUCT_SCREEN );
    m_StructAssemblyMenuItem.Init( mgr, m_MenuBar, "Analysis/Structure/FEA Assembly...", vsp::VSP_STRUCT_ASSEMBLY_SCREEN );
    m_AeroStructMenuItem.Init( mgr, m_MenuBar, "Analysis/Structure/Aero Structural...", vsp::VSP_AERO_STRUCT_SCREEN );

    m_VSPAEROMenuItem.Init( mgr, m_MenuBar, "Analysis/Aero/VSPAERO...", vsp::VSP_VSPAERO_SCREEN );
    m_WaveDragMenuItem.Init( mgr, m_MenuBar, "Analysis/Aero/Wave Drag...", vsp::VSP_WAVEDRAG_SCREEN );
    m_ParasiteDragMenuItem.Init( mgr, m_MenuBar, "Analysis/Aero/Parasite Drag...", vsp::VSP_PARASITE_DRAG_SCREEN );


    // m_AboutMenuItem.Init( mgr, m_MenuBar, "Help/About...", ScreenMgr:: );
    // m_OnlineHelpMenuItem.Init( mgr, m_MenuBar, "Help/Online Help...", ScreenMgr:: );
    // m_VersionMenuItem.Init( mgr, m_MenuBar, "Help/Check Latest Version...", ScreenMgr:: );

    BoldEntries( "File" );
    BoldEntries( "Edit" );
    BoldEntries( "Window" );
    BoldEntries( "View" );
    BoldEntries( "Model" );
    BoldEntries( "Analysis" );
    BoldEntries( "Undo" );

    int glh = m_GenLayout.GetRemainY() - 20;

    m_GlWin = new VSPGUI::VspGlWindow( m_GenLayout.GetX(), m_GenLayout.GetY(), m_GenLayout.GetW(), glh, mgr, DrawObj::VSP_MAIN_SCREEN );

    m_GenLayout.GetGroup()->add( m_GlWin );
    m_GenLayout.GetGroup()->resizable( m_GlWin );
    m_GenLayout.AddY( glh );

    int filew = m_GenLayout.GetW() * .6;

    m_FileNameBox = new Fl_Box(m_GenLayout.GetX(), m_GenLayout.GetY(), filew, 20, "File Name:");
    m_FileNameBox->box(FL_EMBOSSED_FRAME);
    m_FileNameBox->labelsize(15);
    m_FileNameBox->align( Fl_Align( FL_ALIGN_CLIP | FL_ALIGN_RIGHT | FL_ALIGN_INSIDE ) );


    m_GenLayout.AddX( filew );

    m_TitleBox = new Fl_Box(m_GenLayout.GetX(), m_GenLayout.GetY(), m_GenLayout.GetRemainX(), 20, VSPVERSION3 );
    m_TitleBox->box(FL_EMBOSSED_BOX);
    m_TitleBox->labeltype(FL_EMBOSSED_LABEL);
    m_TitleBox->labelsize(15);
    m_TitleBox->labelcolor(FL_DARK_BLUE);
    m_TitleBox->align(Fl_Align(FL_ALIGN_CLIP));

    SetFileLabel( VehicleMgr.GetVehicle()->GetVSP3FileName() );
}

MainVSPScreen::~MainVSPScreen()
{
    delete m_GlWin;
}

//==== Show Main VSP Screen ====//
void MainVSPScreen::Show()
{
    if ( m_FLTK_Window )
    {
        ActionScreen::Show();
    }

    if ( m_GlWin )
    {
        m_GlWin->show();
    }
}

void MainVSPScreen::Hide()
{
    if ( m_FLTK_Window )
    {
        m_FLTK_Window->hide();
    }

    if ( m_GlWin )
    {
        m_GlWin->ResetDrawObjsGeomChangedFlags( true );
        m_GlWin->clearScene();
        m_GlWin->hide();
    }
}

//==== Update Main VSP Screen ====//
bool MainVSPScreen::Update()
{
    ActionScreen::Update();

    // Not sure all three of these are needed.
    m_GlWin->update();
    m_GlWin->redraw();

    m_AxisMenuItem.Update( m_ShowXYZArrow );
    m_BorderMenuItem.Update( m_ShowBorder );

    m_FLTK_Window->redraw();
    return true;
}

//==== Resize the main window ====//
void MainVSPScreen::ResizeWindow( int width, int height )
{
    m_FLTK_Window->resize( m_FLTK_Window->x_root(), m_FLTK_Window->y_root(), width, height );
}

void MainVSPScreen::BoldEntries( string mpath )
{
    Fl_Menu_Item* item = NULL;
    item = (Fl_Menu_Item*) m_MenuBar->find_item( mpath.c_str() );
    if ( item )
    {
        item->labelfont( FL_HELVETICA_BOLD );
    }
}

void MainVSPScreen::CallBack( Fl_Widget* w )
{
    assert( m_ScreenMgr );
    m_ScreenMgr->SetUpdateFlag( true );
}

void MainVSPScreen::GuiDeviceCallBack( GuiDevice* device )
{
    assert( m_ScreenMgr );
    m_ScreenMgr->SetUpdateFlag( true );
}

void MainVSPScreen::CloseCallBack( Fl_Widget *w )
{
    if ( m_StopGUIMenuItem.IsShown() )
    {
        m_ScreenMgr->SetRunGui( false );
    }
    else if ( m_ExitMenuItem.IsShown() )
    {
        ExitVSP();
    }
}

void MainVSPScreen::SetFileLabel( string fname )
{
    string label = "File Name: ";
    label.append( fname );
    m_FileNameBox->copy_label( label.c_str() );

    string title = string( VSPVERSION2 ) + "     " + GetFilename( fname );
    m_FLTK_Window->copy_label( title.c_str() );
}

void MainVSPScreen::ExitVSP()
{
    switch( fl_choice("VSP is exiting. Save or discard your changes.", "Cancel", "Discard", "Save") )
    {
        case(0):
            return;

        case(1):
            exit( 0 );

        case(2):
            string savefile = VehicleMgr.GetVehicle()->GetVSP3FileName();

            if ( savefile.compare( "Unnamed.vsp3" ) == 0 )
            {
                savefile = m_ScreenMgr->FileChooser( "Save VSP File", "*.vsp3", vsp::SAVE );
            }

            if ( savefile.compare( "" ) != 0 )
            {
                VehicleMgr.GetVehicle()->SetVSP3FileName( savefile );
                VehicleMgr.GetVehicle()->WriteXMLFile( savefile, SET_ALL );
                exit( 0 );
            }
    }
}

void MainVSPScreen::ActionCB( void * data )
{
    if ( !m_GlWin || !m_FLTK_Window || !m_ScreenMgr )
        return;

    if ( data == &m_NewMenuItem )
    {
        VehicleMgr.GetVehicle()->Renew();

        SetFileLabel( VehicleMgr.GetVehicle()->GetVSP3FileName() );
        m_GlWin->getGraphicEngine()->getDisplay()->setCOR( 0.0, 0.0, 0.0 );
    }
    else if ( data == &m_OpenMenuItem )
    {
        string openfile = m_ScreenMgr->FileChooser( "Open VSP File", "*.vsp3" );
        if ( openfile.compare( "" ) != 0 )
        {
            VehicleMgr.GetVehicle()->Renew();
            VehicleMgr.GetVehicle()->SetVSP3FileName( openfile );
            VehicleMgr.GetVehicle()->ReadXMLFile( openfile );

            SetFileLabel( openfile );

            BndBox bbox = VehicleMgr.GetVehicle()->GetBndBox();
            vec3d p = bbox.GetCenter();
            m_GlWin->getGraphicEngine()->getDisplay()->setCOR( (float)-p.x(), (float)-p.y(), (float)-p.z() );
        }
    }
    else if ( data == &m_SaveMenuItem )
    {
        string savefile = VehicleMgr.GetVehicle()->GetVSP3FileName();

        if ( savefile.compare( "Unnamed.vsp3" ) == 0 )
        {
            savefile = m_ScreenMgr->FileChooser( "Save VSP File", "*.vsp3", vsp::SAVE );
        }

        if ( savefile.compare( "" ) != 0 )
        {
            VehicleMgr.GetVehicle()->SetVSP3FileName( savefile );
            VehicleMgr.GetVehicle()->WriteXMLFile( savefile, vsp::SET_ALL );

            SetFileLabel( savefile );
        }
    }
    else if ( data == &m_SaveAsMenuItem )
    {
        string savefile = m_ScreenMgr->FileChooser( "Save VSP File As", "*.vsp3", vsp::SAVE );
        if ( savefile.compare( "" ) != 0 )
        {
            VehicleMgr.GetVehicle()->SetVSP3FileName( savefile );
            VehicleMgr.GetVehicle()->WriteXMLFile( savefile, vsp::SET_ALL );

            SetFileLabel( savefile );
        }
    }
    else if ( data == &m_SaveSetMenuItem )
    {
        int set = m_ScreenMgr->GetPickSetScreen()->PickSet( "Pick Save Set" );

        if( set >= 0 )
        {
            string savefile = m_ScreenMgr->FileChooser( "Save VSP Set File As", "*.vsp3", vsp::SAVE );
            if ( savefile.compare( "" ) != 0 )
            {
                string oldfile = VehicleMgr.GetVehicle()->GetVSP3FileName();
                VehicleMgr.GetVehicle()->SetVSP3FileName( savefile );
                VehicleMgr.GetVehicle()->WriteXMLFile( savefile, set );
                VehicleMgr.GetVehicle()->SetVSP3FileName( oldfile );
            }
        }
    }
    else if ( data == &m_InsertMenuItem )
    {
        string openfile = m_ScreenMgr->FileChooser( "Insert VSP File", "*.vsp3" );
        if ( openfile.compare( "" ) != 0 )
        {
            VehicleMgr.GetVehicle()->ReadXMLFileGeomsOnly( openfile );
        }
    }
    // else if ( data == &m_SetTempDirMenuItem )
    // {
    // }
    else if ( data == &m_RunScriptMenuItem )
    {
        string scriptfile = m_ScreenMgr->FileChooser( "Script File To Run", "*.vspscript" );
        if ( scriptfile.compare( "" ) != 0 )
        {
            VehicleMgr.GetVehicle()->RunScript( scriptfile );
        }
    }
    else if ( data == &m_PreferencesMenuItem )
    {

    }
    else if ( data == &m_ExitMenuItem )
    {
        ExitVSP();
    }
    else if ( data == &m_StopGUIMenuItem )
    {
        m_ScreenMgr->SetRunGui( false );
    }
    else if ( data == &m_UndoMenuItem )
    {
        VehicleMgr.GetVehicle()->UnDo();
    }
    else if ( data == &m_CutMenuItem )
    {
        VehicleMgr.GetVehicle()->CutActiveGeomVec();
    }
    else if ( data == &m_CopyMenuItem )
    {
        VehicleMgr.GetVehicle()->CopyActiveGeomVec();
    }
    else if ( data == &m_PasteMenuItem )
    {
        VehicleMgr.GetVehicle()->PasteClipboard();
    }
    else if ( data == &m_DeleteMenuItem )
    {
        VehicleMgr.GetVehicle()->DeleteActiveGeomVec();
    }
    else if ( data == &m_SelAllMenuItem )
    {
        VehicleMgr.GetVehicle()->SetActiveGeomVec( VehicleMgr.GetVehicle()->GetGeomVec() );

        ManageGeomScreen * geomScreen = NULL;
        geomScreen = dynamic_cast<ManageGeomScreen *>
        ( m_ScreenMgr->GetScreen( vsp::VSP_MANAGE_GEOM_SCREEN ) );
        if( geomScreen )
        {
            geomScreen->ShowHideGeomScreens();
        }
    }
    else if ( data == &m_PickMenuItem )
    {
        ManageGeomScreen * geomScreen = NULL;
        geomScreen = dynamic_cast<ManageGeomScreen *>
        ( m_ScreenMgr->GetScreen( vsp::VSP_MANAGE_GEOM_SCREEN ) );
        if( geomScreen )
        {
            geomScreen->TriggerPickSwitch();
        }
    }
    else if ( data == &m_OneMenuItem )
    {
        m_GlWin->setWindowLayout( 1, 1 );
        SetViewAxis( m_ShowXYZArrow );
        SetShowBorders( m_ShowBorder );
    }
    else if ( data == &m_FourMenuItem )
    {
        m_GlWin->setWindowLayout( 2, 2 );
        SetViewAxis( m_ShowXYZArrow );
        SetShowBorders( m_ShowBorder );
    }
    else if ( data == &m_TwoHMenuItem )
    {
        m_GlWin->setWindowLayout( 1, 2 );
        SetViewAxis( m_ShowXYZArrow );
        SetShowBorders( m_ShowBorder );
    }
    else if ( data == &m_TwoVMenuItem )
    {
        m_GlWin->setWindowLayout( 2, 1 );
        SetViewAxis( m_ShowXYZArrow );
        SetShowBorders( m_ShowBorder );
    }
    else if ( data == &m_AxisMenuItem )
    {
        SetViewAxis( !m_ShowXYZArrow );
    }
    else if ( data == &m_BorderMenuItem )
    {
        SetShowBorders( !m_ShowBorder );
    }
    else if ( data == &m_TopMenuItem )
    {
        SetView( VSPGraphic::Common::VSP_CAM_TOP );
    }
    else if ( data == &m_FrontMenuItem )
    {
        SetView( VSPGraphic::Common::VSP_CAM_FRONT );
    }
    else if ( data == &m_LeftSideMenuItem )
    {
        SetView( VSPGraphic::Common::VSP_CAM_LEFT );
    }
    else if ( data == &m_LeftIsoMenuItem )
    {
        SetView( VSPGraphic::Common::VSP_CAM_LEFT_ISO );
    }
    else if ( data == &m_BottomMenuItem )
    {
        SetView( VSPGraphic::Common::VSP_CAM_BOTTOM );
    }
    else if ( data == &m_RearMenuItem )
    {
        SetView( VSPGraphic::Common::VSP_CAM_REAR );
    }
    else if ( data == &m_RightSideMenuItem )
    {
        SetView( VSPGraphic::Common::VSP_CAM_RIGHT );
    }
    else if ( data == &m_RightIsoMenuItem )
    {
        SetView( VSPGraphic::Common::VSP_CAM_RIGHT_ISO );
    }
    else if ( data == &m_CenterMenuItem )
    {
        SetView( VSPGraphic::Common::VSP_CAM_CENTER );
    }
    else if ( data == &m_CenterAllMenuItem )
    {
        std::vector< VSPGraphic::Viewport *> vpts = m_GlWin->getGraphicEngine()->getDisplay()->getLayoutMgr()->getViewports();

        for ( int i = 0; i < vpts.size(); i++ )
        {
            vpts[i]->getCamera()->changeView( VSPGraphic::Common::VSP_CAM_CENTER );
        }

        ManageViewScreen * viewScreen = NULL;
        viewScreen = dynamic_cast< ManageViewScreen* >
        ( m_ScreenMgr->GetScreen( vsp::VSP_VIEW_SCREEN ) );

        if( viewScreen )
        {
            if ( viewScreen->IsShown() )
            {
                viewScreen->UpdateCOR();
                viewScreen->UpdatePan();
            }
        }
    }
    else if ( data == &m_SetCORMenuItem )
    {
        ManageCORScreen * corScreen = NULL;
        corScreen = dynamic_cast<ManageCORScreen *>
        ( m_ScreenMgr->GetScreen( vsp::VSP_COR_SCREEN ) );
        if( corScreen )
        {
            corScreen->EnableSelection();
        }
    }
    else if ( data == &m_FitViewMenuItem || data == &m_FitAllViewMenuItem )
    {
        FitView( data == &m_FitAllViewMenuItem );
    }
    else if ( data == &m_ResetAllViewMenuItem )
    {
        ResetViews();
    }
    // else if ( m_AntialiasMenuItem )
    // {
    // }

    m_ScreenMgr->SetUpdateFlag( true );
}

void MainVSPScreen::ScreenGrab( const string & fname, int w, int h, bool transparentBG, bool autocrop )
{
    if ( m_GlWin )
    {
        bool framebufferSupported = true;
        if ( !glewIsSupported( "GL_ARB_framebuffer_object" ) )
        {
            framebufferSupported = false;
        }

        m_GlWin->getGraphicEngine()->dumpScreenImage( fname, w, h, transparentBG, autocrop, framebufferSupported, VSPGraphic::GraphicEngine::PNG );
    }
}

void MainVSPScreen::SetViewAxis( bool vaxis )
{
    if ( m_GlWin )
    {
        m_ShowXYZArrow = vaxis;

        vector< VSPGraphic::Viewport * > vports = m_GlWin->getGraphicEngine()->getDisplay()->getLayoutMgr()->getViewports();
        for ( int i = 0; i < vports.size(); i++ )
        {
            if ( vports[i] )
            {
                vports[i]->showXYZArrows( m_ShowXYZArrow );
            }
        }
    }
}

void MainVSPScreen::SetShowBorders( bool brdr )
{
    if ( m_GlWin )
    {
        m_ShowBorder = brdr;

        vector< VSPGraphic::Viewport * > vports = m_GlWin->getGraphicEngine()->getDisplay()->getLayoutMgr()->getViewports();
        for ( int i = 0; i < vports.size(); i++ )
        {
            if ( vports[i] )
            {
                vports[i]->showBorders( m_ShowBorder );
            }
        }
    }
}

void MainVSPScreen::SetBackground( double r, double g, double b )
{
    if ( m_GlWin ) {

        VSPGraphic::Viewport *viewport = m_GlWin->getGraphicEngine()->getDisplay()->getViewport();
        if (viewport)
        {
            viewport->getBackground()->setRed((float) (r));
            viewport->getBackground()->setGreen((float) (g));
            viewport->getBackground()->setBlue((float) (b));
        }
    }
}

void MainVSPScreen::SetView( int view )
{
    m_GlWin->setView( static_cast < VSPGraphic::Common::VSPenum > ( view ) );

    ManageViewScreen * viewScreen = NULL;
    viewScreen = dynamic_cast< ManageViewScreen* >
    ( m_ScreenMgr->GetScreen( vsp::VSP_VIEW_SCREEN ) );

    if( viewScreen )
    {
        if ( viewScreen->IsShown() )
        {
            viewScreen->UpdateCOR();
            viewScreen->UpdatePan();
            viewScreen->UpdateRotations();
        }
    }
}

void MainVSPScreen::FitView( bool all )
{
    BndBox bbox;

    bool vehvisible = VehicleMgr.GetVehicle()->GetVisibleBndBox( bbox );

    bool cfdvisible = false;
    // Load visible boundign box from CfdMeshScreen.
    CfdMeshScreen * cfdScreen = dynamic_cast< CfdMeshScreen* >
    ( m_ScreenMgr->GetScreen( vsp::VSP_CFD_MESH_SCREEN ) );
    if( cfdScreen )
    {
        cfdvisible = cfdScreen->GetVisBndBox( bbox );
    }

    bool surfvisible = false;
    // Load visible boundign box from SurfaceIntersectionScreen.
    SurfaceIntersectionScreen * surfScreen = dynamic_cast< SurfaceIntersectionScreen* >
    ( m_ScreenMgr->GetScreen( vsp::VSP_SURFACE_INTERSECTION_SCREEN ) );
    if( surfScreen )
    {
        surfvisible = surfScreen->GetVisBndBox( bbox );
    }

    bool structvisible = false;
    // Load visible boundign box from FeaStructScreen.
    StructScreen * structScreen = dynamic_cast< StructScreen* >
    ( m_ScreenMgr->GetScreen( vsp::VSP_STRUCT_SCREEN ) );
    if( structScreen )
    {
        structvisible = structScreen->GetVisBndBox( bbox );
    }

    bool assemblyvisible = false;
    // Load visible boundign box from FeaStructAssemblyScreen.
    StructAssemblyScreen * structAssemblyScreen = dynamic_cast< StructAssemblyScreen* >
    ( m_ScreenMgr->GetScreen( vsp::VSP_STRUCT_ASSEMBLY_SCREEN ) );
    if( structAssemblyScreen )
    {
        assemblyvisible = structAssemblyScreen->GetVisBndBox( bbox );
    }

    if ( vehvisible || cfdvisible || surfvisible || structvisible || assemblyvisible ) // Check for any visible objects
    {
        vec3d p = bbox.GetCenter();
        double d = bbox.DiagDist();

        std::vector< VSPGraphic::Viewport *> vpts = m_GlWin->getGraphicEngine()->getDisplay()->getLayoutMgr()->getViewports();

        int wid = m_GlWin->pixel_w();
        int ht = m_GlWin->pixel_h();

        for ( int i = 0; i < vpts.size(); i++ )
        {
            wid = MIN( wid, vpts[i]->width() );
            ht = MIN( ht, vpts[i]->height() );
        }

        float z = d * ( wid < ht ? 1.f / wid : 1.f / ht );

        if ( all )
        {
            for ( int i = 0; i < vpts.size(); i++ )
            {
                vpts[i]->getCamera()->setCOR( -p.x(), -p.y(), -p.z() );
                vpts[i]->getCamera()->relativePan( 0.0f, 0.0f );
                vpts[i]->getCamera()->relativeZoom( z );
            }
        }
        else
        {
            m_GlWin->getGraphicEngine()->getDisplay()->setCOR( -p.x(), -p.y(), -p.z() );
            m_GlWin->getGraphicEngine()->getDisplay()->relativePan( 0.0f, 0.0f );
            m_GlWin->getGraphicEngine()->getDisplay()->getCamera()->relativeZoom( z );
        }

        ManageViewScreen * viewScreen = NULL;
        viewScreen = dynamic_cast< ManageViewScreen* >
        ( m_ScreenMgr->GetScreen( vsp::VSP_VIEW_SCREEN ) );

        if ( viewScreen->IsShown() )
        {
            viewScreen->UpdateCOR();
            viewScreen->UpdatePan();
            viewScreen->UpdateZoom();
        }
    }
}

void MainVSPScreen::ResetViews()
{
    std::vector< VSPGraphic::Viewport *> vpts = m_GlWin->getGraphicEngine()->getDisplay()->getLayoutMgr()->getViewports();
    int nview = vpts.size();

    std::vector < VSPGraphic::Common::VSPenum > vOrder;
    // Set View order for viewports.
    // Note:  This order is duplicated from LayoutMgr::LayoutMgr()
    switch ( nview )
    {
        case 1:
            vOrder.push_back( VSPGraphic::Common::VSP_CAM_TOP );
            break;
        case 2:
            vOrder.push_back( VSPGraphic::Common::VSP_CAM_FRONT );
            vOrder.push_back( VSPGraphic::Common::VSP_CAM_TOP );
            break;
        case 4:
        default:
            vOrder.push_back( VSPGraphic::Common::VSP_CAM_LEFT_ISO );
            vOrder.push_back( VSPGraphic::Common::VSP_CAM_TOP );
            vOrder.push_back( VSPGraphic::Common::VSP_CAM_FRONT );
            vOrder.push_back( VSPGraphic::Common::VSP_CAM_LEFT );
            break;
    }

    for ( int i = 0; i < nview; i++ )
    {
        vpts[i]->getCamera()->changeView( vOrder[ i ] );
    }

    ManageViewScreen * viewScreen = NULL;
    viewScreen = dynamic_cast< ManageViewScreen* >
    ( m_ScreenMgr->GetScreen( vsp::VSP_VIEW_SCREEN ) );

    if( viewScreen )
    {
        if ( viewScreen->IsShown() )
        {
            viewScreen->UpdateCOR();
            viewScreen->UpdatePan();
        }
    }
}

void MainVSPScreen::ShowReturnToAPIImplementation()
{
    m_StopGUIMenuItem.Show();
#if FL_API_VERSION >= 10400 || (defined(FL_ABI_VERSION) && FL_ABI_VERSION >= 10304) // Before this version, update() was private.
    m_MenuBar->update();
#endif
}

void MainVSPScreen::HideReturnToAPIImplementation()
{
    m_StopGUIMenuItem.Hide();
#if FL_API_VERSION >= 10400 || (defined(FL_ABI_VERSION) && FL_ABI_VERSION >= 10304)
    m_MenuBar->update();
#endif
}

void MainVSPScreen::ShowExitImplementation()
{
    m_ExitMenuItem.Show();
#if FL_API_VERSION >= 10400 || (defined(FL_ABI_VERSION) && FL_ABI_VERSION >= 10304)
    m_MenuBar->update();
#endif
}

void MainVSPScreen::HideExitImplementation()
{
    m_ExitMenuItem.Hide();
#if FL_API_VERSION >= 10400 || (defined(FL_ABI_VERSION) && FL_ABI_VERSION >= 10304)
    m_MenuBar->update();
#endif
}

void ShowReturnToAPIHandler( void * data )
{
    MainVSPScreen * main_scr = (MainVSPScreen*) data;
    if ( main_scr )
    {
        main_scr->ShowReturnToAPIImplementation();
    }
}

void HideReturnToAPIHandler( void * data )
{
    MainVSPScreen * main_scr = (MainVSPScreen*) data;
    if ( main_scr )
    {
        main_scr->HideReturnToAPIImplementation();
    }
}

void ShowExitHandler( void * data )
{
    MainVSPScreen * main_scr = (MainVSPScreen*) data;
    if ( main_scr )
    {
        main_scr->ShowExitImplementation();
    }
}

void HideExitHandler( void * data )
{
    MainVSPScreen * main_scr = (MainVSPScreen*) data;
    if ( main_scr )
    {
        main_scr->HideExitImplementation();
    }
}

void MainVSPScreen::EnableStopGUIMenuItem()
{
    Fl::awake( ShowReturnToAPIHandler, ( void* )this );
}

void MainVSPScreen::DisableStopGUIMenuItem()
{
    Fl::awake( HideReturnToAPIHandler, ( void* )this );
}

void MainVSPScreen::EnableExitMenuItem()
{
    Fl::awake( ShowExitHandler, ( void* )this );
}

void MainVSPScreen::DisableExitMenuItem()
{
    Fl::awake( HideExitHandler, ( void* )this );
}
