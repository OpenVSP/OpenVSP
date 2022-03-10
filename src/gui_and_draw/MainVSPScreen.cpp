//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

//
// Programmatic main GUI window
// Rob McDonald
//
//////////////////////////////////////////////////////////////////////

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
    m_ImportMenuItem.Init( mgr, m_MenuBar, "File/Import...", ScreenMgr::VSP_IMPORT_SCREEN );
    m_ExportMenuItem.Init( mgr, m_MenuBar, "File/Export...", ScreenMgr::VSP_EXPORT_SCREEN );
    // m_SetTempDirMenuItem.Init( this, m_MenuBar, "File/Set Temp Dir..." );
    m_RunScriptMenuItem.Init( this, m_MenuBar, "File/Run Script..." );
    m_ExitMenuItem.Init( this, m_MenuBar, "File/Exit" );

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
    m_BackgroundMenuItem.Init( mgr, m_MenuBar, "Window/Background...", ScreenMgr::VSP_BACKGROUND_SCREEN );
    m_ScreenshotMenuItem.Init( mgr, m_MenuBar, "Window/Screenshot...", ScreenMgr::VSP_SCREENSHOT_SCREEN );
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
    m_CenterMenuItem.Init( this, m_MenuBar, "View/Center", 'c' );
    m_SetCORMenuItem.Init( this, m_MenuBar, "View/Set Rotation Center", 'r' );
    m_FitViewMenuItem.Init( this, m_MenuBar, "View/Fit On Screen", 'f' );
    m_AdjustMenuItem.Init( mgr, m_MenuBar, "View/Adjust...", ScreenMgr::VSP_VIEW_SCREEN );
    // m_AntialiasMenuItem.Init( this, m_MenuBar, "View/New" );

    m_GeomMenuItem.Init( mgr, m_MenuBar, "Model/Geometry...", ScreenMgr::VSP_MANAGE_GEOM_SCREEN );
    m_SetEditMenuItem.Init( mgr, m_MenuBar, "Model/Set Editor...", ScreenMgr::VSP_SET_EDITOR_SCREEN );
    m_LinkMenuItem.Init( mgr, m_MenuBar, "Model/Link...", ScreenMgr::VSP_PARM_LINK_SCREEN );
    m_DesVarMenuItem.Init( mgr, m_MenuBar, "Model/Design Variables...", ScreenMgr::VSP_DESIGN_VAR_SCREEN );
    m_LabelMenuItem.Init( mgr, m_MenuBar, "Model/Measure...", ScreenMgr::VSP_MEASURE_SCREEN );
    m_LightMenuItem.Init( mgr, m_MenuBar, "Model/Lighting...", ScreenMgr::VSP_LIGHTING_SCREEN );
    m_ClipMenuItem.Init( mgr, m_MenuBar, "Model/Clipping...", ScreenMgr::VSP_CLIPPING_SCREEN );
    m_TextureMenuItem.Init( mgr, m_MenuBar, "Model/Texture...", ScreenMgr::VSP_MANAGE_TEXTURE_SCREEN );
    m_AdvLinkMenuItem.Init( mgr, m_MenuBar, "Model/Adv Link...", ScreenMgr::VSP_ADV_LINK_SCREEN );
    m_UserParmMenuItem.Init( mgr, m_MenuBar, "Model/User Parms...", ScreenMgr::VSP_USER_PARM_SCREEN );
    m_FitModelMenuItem.Init( mgr, m_MenuBar, "Model/Fit Model...", ScreenMgr::VSP_FIT_MODEL_SCREEN );
    m_SnapToMenuItem.Init( mgr, m_MenuBar, "Model/Snap To...", ScreenMgr::VSP_SNAP_TO_SCREEN );
    m_VarPresetMenuItem.Init( mgr, m_MenuBar, "Model/Variable Presets...", ScreenMgr::VSP_VAR_PRESET_SCREEN );

    m_CompGeomMenuItem.Init( mgr, m_MenuBar, "Analysis/CompGeom...", ScreenMgr::VSP_COMP_GEOM_SCREEN );
    m_PSliceMenuItem.Init( mgr, m_MenuBar, "Analysis/Planar Slice...", ScreenMgr::VSP_PSLICE_SCREEN );
    m_MassPropMenuItem.Init( mgr, m_MenuBar, "Analysis/Mass Prop...", ScreenMgr::VSP_MASS_PROP_SCREEN );
    m_ProjectionMenuItem.Init( mgr, m_MenuBar, "Analysis/Projected Area...", ScreenMgr::VSP_PROJECTION_SCREEN );
    m_SurfIntMenuItem.Init( mgr, m_MenuBar, "Analysis/Trimmed Surfaces...", ScreenMgr::VSP_SURFACE_INTERSECTION_SCREEN );
    m_CFDMeshMenuItem.Init( mgr, m_MenuBar, "Analysis/CFD Mesh...", ScreenMgr::VSP_CFD_MESH_SCREEN );
    m_StructMenuItem.Init( mgr, m_MenuBar, "Analysis/FEA Mesh...", ScreenMgr::VSP_STRUCT_SCREEN );
    m_DegenGeomMenuItem.Init( mgr, m_MenuBar, "Analysis/DegenGeom...", ScreenMgr::VSP_DEGEN_GEOM_SCREEN );
    m_VSPAEROMenuItem.Init( mgr, m_MenuBar, "Analysis/VSPAERO...", ScreenMgr::VSP_VSPAERO_SCREEN );
    m_WaveDragMenuItem.Init( mgr, m_MenuBar, "Analysis/Wave Drag...", ScreenMgr::VSP_WAVEDRAG_SCREEN );
    m_ParasiteDragMenuItem.Init( mgr, m_MenuBar, "Analysis/Parasite Drag...", ScreenMgr::VSP_PARASITE_DRAG_SCREEN );


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
    m_FileNameBox->labelsize(10);
    m_FileNameBox->align( Fl_Align( FL_ALIGN_CLIP | FL_ALIGN_RIGHT | FL_ALIGN_INSIDE ) );

    SetFileLabel( VehicleMgr.GetVehicle()->GetVSP3FileName() );

    m_GenLayout.AddX( filew );

    m_TitleBox = new Fl_Box(m_GenLayout.GetX(), m_GenLayout.GetY(), m_GenLayout.GetRemainX(), 20, VSPVERSION3 );
    m_TitleBox->box(FL_EMBOSSED_BOX);
    m_TitleBox->labeltype(FL_EMBOSSED_LABEL);
    m_TitleBox->labelsize(15);
    m_TitleBox->labelcolor(FL_DARK_BLUE);
    m_TitleBox->align(Fl_Align(FL_ALIGN_CLIP));

    m_FLTK_Window->label( VSPVERSION2 );
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
        m_FLTK_Window->show();
    }

    if ( m_GlWin )
    {
        m_GlWin->show();
    }
}

//==== Update Main VSP Screen ====//
bool MainVSPScreen::Update()
{
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
    ExitVSP();
}

void MainVSPScreen::SetFileLabel( string fname )
{
    string label = "File Name: ";
    label.append( fname );
    m_FileNameBox->copy_label( label.c_str() );
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
                savefile = m_ScreenMgr->GetSelectFileScreen()->FileChooser( "Save VSP File", "*.vsp3" );
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
        string openfile = m_ScreenMgr->GetSelectFileScreen()->FileChooser( "Open VSP File", "*.vsp3" );
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
            savefile = m_ScreenMgr->GetSelectFileScreen()->FileChooser( "Save VSP File", "*.vsp3" );
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
        string savefile = m_ScreenMgr->GetSelectFileScreen()->FileChooser( "Save VSP File As", "*.vsp3" );
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
            string savefile = m_ScreenMgr->GetSelectFileScreen()->FileChooser( "Save VSP Set File As", "*.vsp3" );
            if ( savefile.compare( "" ) != 0 )
            {
                VehicleMgr.GetVehicle()->WriteXMLFile( savefile, set );
            }
        }
    }
    else if ( data == &m_InsertMenuItem )
    {
        string openfile = m_ScreenMgr->GetSelectFileScreen()->FileChooser( "Insert VSP File", "*.vsp3" );
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
        string scriptfile = m_ScreenMgr->GetSelectFileScreen()->FileChooser( "Script File To Run", "*.vspscript" );
        if ( scriptfile.compare( "" ) != 0 )
        {
            VehicleMgr.GetVehicle()->RunScript( scriptfile );
        }
    }
    else if ( data == &m_ExitMenuItem )
    {
        ExitVSP();
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
        ( m_ScreenMgr->GetScreen( ScreenMgr::VSP_MANAGE_GEOM_SCREEN ) );
        if( geomScreen )
        {
            geomScreen->ShowHideGeomScreens();
        }
    }
    else if ( data == &m_PickMenuItem )
    {
        ManageGeomScreen * geomScreen = NULL;
        geomScreen = dynamic_cast<ManageGeomScreen *>
        ( m_ScreenMgr->GetScreen( ScreenMgr::VSP_MANAGE_GEOM_SCREEN ) );
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
        m_GlWin->setView( VSPGraphic::Common::VSP_CAM_TOP );
        ManageViewScreen * viewScreen = NULL;
        viewScreen = dynamic_cast< ManageViewScreen* >
        ( m_ScreenMgr->GetScreen( ScreenMgr::VSP_VIEW_SCREEN ) );

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
    else if ( data == &m_FrontMenuItem )
    {
        m_GlWin->setView( VSPGraphic::Common::VSP_CAM_FRONT );
        ManageViewScreen * viewScreen = NULL;
        viewScreen = dynamic_cast< ManageViewScreen* >
        ( m_ScreenMgr->GetScreen( ScreenMgr::VSP_VIEW_SCREEN ) );

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
    else if ( data == &m_LeftSideMenuItem )
    {
        m_GlWin->setView( VSPGraphic::Common::VSP_CAM_LEFT );
        ManageViewScreen * viewScreen = NULL;
        viewScreen = dynamic_cast< ManageViewScreen* >
        ( m_ScreenMgr->GetScreen( ScreenMgr::VSP_VIEW_SCREEN ) );

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
    else if ( data == &m_LeftIsoMenuItem )
    {
        m_GlWin->setView( VSPGraphic::Common::VSP_CAM_LEFT_ISO );
        ManageViewScreen * viewScreen = NULL;
        viewScreen = dynamic_cast< ManageViewScreen* >
        ( m_ScreenMgr->GetScreen( ScreenMgr::VSP_VIEW_SCREEN ) );

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
    else if ( data == &m_BottomMenuItem )
    {
        m_GlWin->setView( VSPGraphic::Common::VSP_CAM_BOTTOM );
        ManageViewScreen * viewScreen = NULL;
        viewScreen = dynamic_cast< ManageViewScreen* >
        ( m_ScreenMgr->GetScreen( ScreenMgr::VSP_VIEW_SCREEN ) );

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
    else if ( data == &m_RearMenuItem )
    {
        m_GlWin->setView( VSPGraphic::Common::VSP_CAM_REAR );
        ManageViewScreen * viewScreen = NULL;
        viewScreen = dynamic_cast< ManageViewScreen* >
        ( m_ScreenMgr->GetScreen( ScreenMgr::VSP_VIEW_SCREEN ) );

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
    else if ( data == &m_RightSideMenuItem )
    {
        m_GlWin->setView( VSPGraphic::Common::VSP_CAM_RIGHT );
        ManageViewScreen * viewScreen = NULL;
        viewScreen = dynamic_cast< ManageViewScreen* >
        ( m_ScreenMgr->GetScreen( ScreenMgr::VSP_VIEW_SCREEN ) );

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
    else if ( data == &m_RightIsoMenuItem )
    {
        m_GlWin->setView( VSPGraphic::Common::VSP_CAM_RIGHT_ISO );
        ManageViewScreen * viewScreen = NULL;
        viewScreen = dynamic_cast< ManageViewScreen* >
        ( m_ScreenMgr->GetScreen( ScreenMgr::VSP_VIEW_SCREEN ) );

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
    else if ( data == &m_CenterMenuItem )
    {
        m_GlWin->setView( VSPGraphic::Common::VSP_CAM_CENTER );

        ManageViewScreen * viewScreen = NULL;
        viewScreen = dynamic_cast< ManageViewScreen* >
        ( m_ScreenMgr->GetScreen( ScreenMgr::VSP_VIEW_SCREEN ) );

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
        ( m_ScreenMgr->GetScreen( ScreenMgr::VSP_COR_SCREEN ) );
        if( corScreen )
        {
            corScreen->EnableSelection();
        }
    }
    else if ( data == &m_FitViewMenuItem )
    {
        BndBox bbox;

        if ( VehicleMgr.GetVehicle()->GetVisibleBndBox( bbox ) ) // Check for any visible objects
        {
            vec3d p = bbox.GetCenter();
            double d = bbox.DiagDist();
            int wid = m_GlWin->pixel_w();
            int ht = m_GlWin->pixel_h();

            float z = d * ( wid < ht ? 1.f / wid : 1.f / ht );

            m_GlWin->getGraphicEngine()->getDisplay()->setCOR( -p.x(), -p.y(), -p.z() );
            m_GlWin->getGraphicEngine()->getDisplay()->relativePan( 0.0f, 0.0f );
            m_GlWin->getGraphicEngine()->getDisplay()->getCamera()->relativeZoom( z );

            ManageViewScreen * viewScreen = NULL;
            viewScreen = dynamic_cast< ManageViewScreen* >
            ( m_ScreenMgr->GetScreen( ScreenMgr::VSP_VIEW_SCREEN ) );

            if ( viewScreen->IsShown() )
            {
                viewScreen->UpdateCOR();
                viewScreen->UpdatePan();
                viewScreen->UpdateZoom();
            }
        }
    }
    // else if ( m_AntialiasMenuItem )
    // {
    // }

    m_ScreenMgr->SetUpdateFlag( true );
}

void MainVSPScreen::ScreenGrab( const string & fname, int w, int h, bool transparentBG )
{
    if ( m_GlWin )
    {
        bool framebufferSupported = true;
        if ( !glewIsSupported( "GL_ARB_framebuffer_object" ) )
        {
            framebufferSupported = false;
        }

        m_GlWin->getGraphicEngine()->dumpScreenImage( fname, w, h, transparentBG, framebufferSupported, VSPGraphic::GraphicEngine::PNG );
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
