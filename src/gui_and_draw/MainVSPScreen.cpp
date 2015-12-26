//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// VehicleMgr.cpp: implementation of the Vehicle Class and Vehicle Mgr Singleton.
//
//////////////////////////////////////////////////////////////////////

#include "MainVSPScreen.h"
#include "ManageViewScreen.h"

#include "ScreenMgr.h"
#include "VehicleMgr.h"
#include "CfdMeshMgr.h"
#include "AdvLinkMgr.h"
#include "MainGLWindow.h"
#include "SelectFileScreen.h"
#include "GraphicEngine.h"
#include "APIDefines.h"
#include "main.h"
#include <FL/fl_ask.H>
#include "Display.h"

using namespace vsp;
using VSPGUI::VspGlWindow;

#include "Common.h"

//==== Constructor ====//
MainVSPScreen::MainVSPScreen( ScreenMgr* mgr ) : VspScreen( mgr )
{
    MainWinUI* ui = m_MainUI = new MainWinUI();

    m_FLTK_Window = ui->winShell;

    int x, y, width, h, side;
    Fl::screen_xywh( x, y, width, h );

    // Figure out which is smaller, half the screen width or the height
    if ( 0.5 * width < 0.9 * h )
    {
        side = (int)(0.9 * h);
    }
    else
    {
        side = (int)(0.5 * width);
    }
    m_FLTK_Window->resize( x + 10, y + 30, side, side );

    AddMenuCallBack( m_MainUI->NewMenu );
    AddMenuCallBack( m_MainUI->OpenMenu );
    AddMenuCallBack( m_MainUI->SaveMenu );
    AddMenuCallBack( m_MainUI->SaveAsMenu );
    AddMenuCallBack( m_MainUI->SaveSetMenu );
    AddMenuCallBack( m_MainUI->InsertFileMenu );
    AddMenuCallBack( m_MainUI->ImportMenu );
    AddMenuCallBack( m_MainUI->ExportMenu );
    AddMenuCallBack( m_MainUI->TempDirMenu );
    AddMenuCallBack( m_MainUI->VersionMenu );
    AddMenuCallBack( m_MainUI->RunScriptMenu  );
    AddMenuCallBack( m_MainUI->ExitMenu );

    AddMenuCallBack( m_MainUI->GeomModifyMenu );
    AddMenuCallBack( m_MainUI->TypeEditorMenu  );
    AddMenuCallBack( m_MainUI->UserParmMenu  );
    AddMenuCallBack( m_MainUI->SetEditorMenu  );
    AddMenuCallBack( m_MainUI->StructureMenu );
    AddMenuCallBack( m_MainUI->CompGeomMenu );
    AddMenuCallBack( m_MainUI->DegenGeomMenu );
    AddMenuCallBack( m_MainUI->CFDMeshGeomMenu );
    AddMenuCallBack( m_MainUI->ParmLinkMenu );
    AddMenuCallBack( m_MainUI->ParmDebugMenu );
    AddMenuCallBack( m_MainUI->DesignVarMenu );
    AddMenuCallBack( m_MainUI->FitModelMenu );
    AddMenuCallBack( m_MainUI->MassPropMenu );
    AddMenuCallBack( m_MainUI->AwaveMenu );
    AddMenuCallBack( m_MainUI->VSPAEROMenu );
    AddMenuCallBack( m_MainUI->PSliceMenu );
    AddMenuCallBack( m_MainUI->AdvParmLinkMenu );

    AddMenuCallBack( m_MainUI->OneScreenMenu );
    AddMenuCallBack( m_MainUI->FourScreenMenu );
    AddMenuCallBack( m_MainUI->TwoHorzScreenMenu );
    AddMenuCallBack( m_MainUI->TwoVertScreenMenu );
    AddMenuCallBack( m_MainUI->BackgroundMenu );

    AddMenuCallBack( m_MainUI->LightingMenu );
    AddMenuCallBack( m_MainUI->ClippingMenu );
    AddMenuCallBack( m_MainUI->LabelsMenu );
    AddMenuCallBack( m_MainUI->ScreenShotMenu );

    AddMenuCallBack( m_MainUI->TopViewMenu );
    AddMenuCallBack( m_MainUI->FrontViewMenu );
    AddMenuCallBack( m_MainUI->LeftSideViewMenu );
    AddMenuCallBack( m_MainUI->LeftIsoViewMenu );
    AddMenuCallBack( m_MainUI->BottomViewMenu );
    AddMenuCallBack( m_MainUI->BackViewMenu );
    AddMenuCallBack( m_MainUI->RightSideViewMenu );
    AddMenuCallBack( m_MainUI->RightIsoViewMenu );
    AddMenuCallBack( m_MainUI->CenterViewMenu );
    AddMenuCallBack( m_MainUI->AdjustViewMenu );
    AddMenuCallBack( m_MainUI->AntialiasMenu );
    AddMenuCallBack( m_MainUI->TextureMenu );

    AddMenuCallBack( m_MainUI->RevertMenu );

    AddMenuCallBack( m_MainUI->ShowAbout );
    AddMenuCallBack( m_MainUI->ShowHelpWebsite );

    AddMenuCallBack( m_MainUI->ReturnToAPI );
    HideReturnToAPI();

    //==== Add Gl Window to Main Window ====//
    Fl_Widget* w = m_MainUI->GlWinGroup;
    m_MainUI->GlWinGroup->begin();
    m_GlWin = new VspGlWindow( w->x(), w->y(), w->w(), w->h(), mgr, DrawObj::VSP_MAIN_SCREEN );
    m_MainUI->GlWinGroup->end();

    SetFileLabel( VehicleMgr.GetVehicle()->GetVSP3FileName() );

    m_MainUI->winShell->label( VSPVERSION2 );
    m_MainUI->TitleBox->label( VSPVERSION3 );

    m_MainUI->winShell->callback(staticCloseCB, this);

    m_MainUI->ParmDebugMenu->hide();
}

MainVSPScreen::~MainVSPScreen()
{
    delete m_GlWin;
}

//==== Show Main VSP Screen ====//
void MainVSPScreen::AddMenuCallBack( Fl_Menu_Item* fl_menu )
{
    fl_menu->callback( staticMenuCB, this );
}


//==== Show Main VSP Screen ====//
void MainVSPScreen::Show()
{
    m_FLTK_Window->show();
    m_GlWin->show();
}

//==== Hide Main VSP Screen ====//
void MainVSPScreen::Hide()
{
}

//==== Resize the main window ====//
void MainVSPScreen::ResizeWindow( int width, int height )
{
    m_FLTK_Window->resize( m_FLTK_Window->x_root(), m_FLTK_Window->y_root(), width, height );
}

//==== Update Main VSP Screen ====//
bool MainVSPScreen::Update()
{
    // Not sure all three of these are needed.
    m_GlWin->update();
    m_GlWin->redraw();
    m_MainUI->winShell->redraw();
    return true;
}


//==== Non Menu Callbacks ====//
void MainVSPScreen::CallBack( Fl_Widget *w )
{

}

void MainVSPScreen::MenuCallBack( Fl_Widget *w )
{
    Fl_Menu_* mw = static_cast< Fl_Menu_* >( w );
    const Fl_Menu_Item* m = mw->mvalue();

    if ( m == m_MainUI->OneScreenMenu )
    {
        m_GlWin->setWindowLayout( 1, 1 );
        m_GlWin->redraw();

        m_ScreenMgr->SetUpdateFlag( true );
    }
    else if ( m == m_MainUI->FourScreenMenu )
    {
        m_GlWin->setWindowLayout( 2, 2 );
        m_GlWin->redraw();

        m_ScreenMgr->SetUpdateFlag( true );
    }
    else if ( m == m_MainUI->TwoHorzScreenMenu )
    {
        m_GlWin->setWindowLayout( 1, 2 );
        m_GlWin->redraw();

        m_ScreenMgr->SetUpdateFlag( true );
    }
    else if ( m == m_MainUI->TwoVertScreenMenu )
    {
        m_GlWin->setWindowLayout( 2, 1 );
        m_GlWin->redraw();

        m_ScreenMgr->SetUpdateFlag( true );
    }
    else if ( m == m_MainUI->ScreenShotMenu )
    {
        m_ScreenMgr->ShowScreen( ScreenMgr::VSP_SCREENSHOT_SCREEN );
    }
    else if ( m == m_MainUI->TopViewMenu )
    {
        m_GlWin->setView( VSPGraphic::Common::VSP_CAM_TOP );
        m_GlWin->redraw();
    }
    else if ( m == m_MainUI->FrontViewMenu )
    {
        m_GlWin->setView( VSPGraphic::Common::VSP_CAM_FRONT );
        m_GlWin->redraw();
    }
    else if ( m == m_MainUI->LeftSideViewMenu )
    {
        m_GlWin->setView( VSPGraphic::Common::VSP_CAM_LEFT );
        m_GlWin->redraw();
    }
    else if ( m == m_MainUI->LeftIsoViewMenu )
    {
        m_GlWin->setView( VSPGraphic::Common::VSP_CAM_LEFT_ISO );
        m_GlWin->redraw();
    }
    else if ( m == m_MainUI->BottomViewMenu )
    {
        m_GlWin->setView( VSPGraphic::Common::VSP_CAM_BOTTOM );
        m_GlWin->redraw();
    }
    else if ( m == m_MainUI->BackViewMenu )
    {
        m_GlWin->setView( VSPGraphic::Common::VSP_CAM_REAR );
        m_GlWin->redraw();
    }
    else if ( m == m_MainUI->RightSideViewMenu )
    {
        m_GlWin->setView( VSPGraphic::Common::VSP_CAM_RIGHT );
        m_GlWin->redraw();
    }
    else if ( m == m_MainUI->RightIsoViewMenu )
    {
        m_GlWin->setView( VSPGraphic::Common::VSP_CAM_RIGHT_ISO );
        m_GlWin->redraw();
    }
    else if ( m == m_MainUI->CenterViewMenu )
    {
        m_GlWin->setView( VSPGraphic::Common::VSP_CAM_CENTER );
        m_GlWin->redraw();
    }
    else if ( m == m_MainUI->AdjustViewMenu )
    {
        m_ScreenMgr->ShowScreen( ScreenMgr::VSP_VIEW_SCREEN );
    }
    else if ( m == m_MainUI->BackgroundMenu )
    {
        m_ScreenMgr->ShowScreen( ScreenMgr::VSP_BACKGROUND_SCREEN );
    }
    else if ( m == m_MainUI->ExportMenu )
    {
        m_ScreenMgr->ShowScreen( ScreenMgr::VSP_EXPORT_SCREEN );
    }
    else if ( m == m_MainUI->ImportMenu )
    {
        m_ScreenMgr->ShowScreen( ScreenMgr::VSP_IMPORT_SCREEN );
    }
    else if ( m == m_MainUI->GeomModifyMenu )
    {
        m_ScreenMgr->ShowScreen( ScreenMgr::VSP_MANAGE_GEOM_SCREEN );
    }
    else if ( m == m_MainUI->RevertMenu )
    {
        VehicleMgr.GetVehicle()->UnDo();
    }
    else if ( m == m_MainUI->SetEditorMenu )
    {
        m_ScreenMgr->ShowScreen( ScreenMgr::VSP_SET_EDITOR_SCREEN );
    }
    else if ( m == m_MainUI->TypeEditorMenu )
    {
        m_ScreenMgr->ShowScreen( ScreenMgr::VSP_TYPE_EDITOR_SCREEN );
    }
    else if ( m == m_MainUI->UserParmMenu )
    {
        m_ScreenMgr->ShowScreen( ScreenMgr::VSP_USER_PARM_SCREEN );
    }
    else if ( m == m_MainUI->TextureMenu )
    {
        m_ScreenMgr->ShowScreen( ScreenMgr::VSP_MANAGE_TEXTURE_SCREEN );
    }
    else if ( m == m_MainUI->ParmLinkMenu )
    {
        m_ScreenMgr->ShowScreen( ScreenMgr::VSP_PARM_LINK_SCREEN );
    }
    else if ( m == m_MainUI->AdvParmLinkMenu )
    {
        m_ScreenMgr->ShowScreen( ScreenMgr::VSP_ADV_LINK_SCREEN );
    }
    else if ( m == m_MainUI->LightingMenu )
    {
        m_ScreenMgr->ShowScreen( ScreenMgr::VSP_LIGHTING_SCREEN );
    }
    else if ( m == m_MainUI->ClippingMenu )
    {
        m_ScreenMgr->ShowScreen( ScreenMgr::VSP_CLIPPING_SCREEN );
    }
    else if ( m == m_MainUI->LabelsMenu )
    {
        m_ScreenMgr->ShowScreen( ScreenMgr::VSP_LABEL_SCREEN );
    }
    else if ( m == m_MainUI->ParmDebugMenu )
    {
        m_ScreenMgr->ShowScreen( ScreenMgr::VSP_PARM_DEBUG_SCREEN );
    }
    else if ( m == m_MainUI->DesignVarMenu )
    {
        m_ScreenMgr->ShowScreen( ScreenMgr::VSP_DESIGN_VAR_SCREEN );
    }
    else if ( m == m_MainUI->FitModelMenu )
    {
        m_ScreenMgr->ShowScreen( ScreenMgr::VSP_FIT_MODEL_SCREEN );
    }
    else if ( m == m_MainUI->CompGeomMenu )
    {
        m_ScreenMgr->ShowScreen( ScreenMgr::VSP_COMP_GEOM_SCREEN );
    }
    else if ( m == m_MainUI->DegenGeomMenu )
    {
        m_ScreenMgr->ShowScreen( ScreenMgr::VSP_DEGEN_GEOM_SCREEN );
    }
    else if ( m == m_MainUI->CFDMeshGeomMenu )
    {
        m_ScreenMgr->ShowScreen( ScreenMgr::VSP_CFD_MESH_SCREEN );
    }
    else if ( m == m_MainUI->MassPropMenu )
    {
        m_ScreenMgr->ShowScreen( ScreenMgr::VSP_MASS_PROP_SCREEN );
    }
    else if ( m == m_MainUI->PSliceMenu )
    {
        m_ScreenMgr->ShowScreen( ScreenMgr::VSP_PSLICE_SCREEN );
    }
    else if ( m == m_MainUI->AwaveMenu )
    {
        m_ScreenMgr->ShowScreen( ScreenMgr::VSP_AWAVE_SCREEN );
    }
    else if ( m == m_MainUI->VSPAEROMenu )
    {
        m_ScreenMgr->ShowScreen( ScreenMgr::VSP_VSPAERO_SCREEN );
    }
    else if ( m == m_MainUI->StructureMenu )
    {
        m_ScreenMgr->Alert( "Wing structure support not implemented at this time." );
//      m_ScreenMgr->ShowScreen( ScreenMgr::VSP_FEA_MESH_SCREEN );
    }
    else if ( m == m_MainUI->NewMenu )
    {
        VehicleMgr.GetVehicle()->Renew();

        SetFileLabel( VehicleMgr.GetVehicle()->GetVSP3FileName() );
        m_GlWin->getGraphicEngine()->getDisplay()->setCOR( 0.0, 0.0, 0.0 );

        m_ScreenMgr->SetUpdateFlag( true );
    }
    else if ( m == m_MainUI->OpenMenu )
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

            m_ScreenMgr->SetUpdateFlag( true );
        }
    }
    else if ( m == m_MainUI->SaveMenu )
    {
        string savefile = VehicleMgr.GetVehicle()->GetVSP3FileName();

        if ( savefile.compare( "Unnamed.vsp3" ) == 0 )
        {
            savefile = m_ScreenMgr->GetSelectFileScreen()->FileChooser( "Save VSP File", "*.vsp3" );
        }

        if ( savefile.compare( "" ) != 0 )
        {
            savefile = CheckAddVSP3Ext( savefile );
            VehicleMgr.GetVehicle()->SetVSP3FileName( savefile );
            VehicleMgr.GetVehicle()->WriteXMLFile( savefile, SET_ALL );

            SetFileLabel( savefile );

            m_ScreenMgr->SetUpdateFlag( true );
        }
    }
    else if ( m == m_MainUI->SaveAsMenu )
    {
        string savefile = m_ScreenMgr->GetSelectFileScreen()->FileChooser( "Save VSP File As", "*.vsp3" );
        if ( savefile.compare( "" ) != 0 )
        {
            savefile = CheckAddVSP3Ext( savefile );
            VehicleMgr.GetVehicle()->SetVSP3FileName( savefile );
            VehicleMgr.GetVehicle()->WriteXMLFile( savefile, SET_ALL );

            SetFileLabel( savefile );

            m_ScreenMgr->SetUpdateFlag( true );
        }
    }
    else if ( m == m_MainUI->SaveSetMenu )
    {
        int set = m_ScreenMgr->GetPickSetScreen()->PickSet( "Pick Save Set" );

        if( set >= 0 )
        {
            string savefile = m_ScreenMgr->GetSelectFileScreen()->FileChooser( "Save VSP Set File As", "*.vsp3" );
            if ( savefile.compare( "" ) != 0 )
            {
                savefile = CheckAddVSP3Ext( savefile );
                VehicleMgr.GetVehicle()->WriteXMLFile( savefile, set );
            }
        }
    }
    else if ( m == m_MainUI->InsertFileMenu )
    {
        string openfile = m_ScreenMgr->GetSelectFileScreen()->FileChooser( "Insert VSP File", "*.vsp3" );
        if ( openfile.compare( "" ) != 0 )
        {
            VehicleMgr.GetVehicle()->ReadXMLFile( openfile );
            m_ScreenMgr->SetUpdateFlag( true );
        }
    }
    else if ( m == m_MainUI->RunScriptMenu )
    {
        string scriptfile = m_ScreenMgr->GetSelectFileScreen()->FileChooser( "Script File To Run", "*.vspscript" );
        if ( scriptfile.compare( "" ) != 0 )
        {
            VehicleMgr.GetVehicle()->RunScript( scriptfile );
            m_ScreenMgr->SetUpdateFlag( true );
        }
    }
    else if ( m == m_MainUI->ExitMenu )
    {
        ExitVSP();
    }
    else if ( m == m_MainUI->ReturnToAPI )
    {
        m_ScreenMgr->SetRunGui( false );
    }
}

string MainVSPScreen::CheckAddVSP3Ext( string fname )
{
    string ext = ".vsp3";
    string fext = fname.substr( fname.length() - ext.length(), ext.length() );

    // STL string tolower
    transform( fext.begin(), fext.end(), fext.begin(), ::tolower );

    if ( fext.compare( ext ) != 0 )
    {
        fname.append( ".vsp3" );
    }

    return fname;
}

void MainVSPScreen::SetFileLabel( string fname )
{
    string label = "File Name: ";
    label.append( fname );
    m_MainUI->FileNameBox->copy_label( label.c_str() );
}

void MainVSPScreen::CloseCallBack( Fl_Widget *w )
{
    ExitVSP();
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
				savefile = CheckAddVSP3Ext( savefile );
				VehicleMgr.GetVehicle()->SetVSP3FileName( savefile );
				VehicleMgr.GetVehicle()->WriteXMLFile( savefile, SET_ALL );
				exit( 0 );
			}
    }
}

