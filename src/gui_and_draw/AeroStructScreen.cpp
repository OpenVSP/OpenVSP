//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#include "AeroStructScreen.h"
#include "ScreenMgr.h"
#include "ScreenBase.h"
#include "ParmMgr.h"

#include "AeroStructMgr.h"
#include "VSPAEROScreen.h"
#include "StructScreen.h"
#include "StructureMgr.h"
#include "FeaStructure.h"
#include "FeaMeshMgr.h"

#include <FL/fl_ask.H>


//==== Constructor ====//
AeroStructScreen::AeroStructScreen( ScreenMgr* mgr ) : BasicScreen( mgr, 300, 600, "Aero Structure Coupled Analysis" )
{

    m_GlobalLayout.SetGroupAndScreen( m_FLTK_Window, this );
    m_GlobalLayout.AddY( 25 );

    m_GlobalLayout.AddDividerBox( "VSPAERO" );

    m_GlobalLayout.AddButton( m_ShowVSPAEROGUI, "Show VSPAERO GUI" );
    m_GlobalLayout.AddButton( m_ExecuteVSPAERO, "Execute VSPAERO" );

    m_GlobalLayout.AddYGap();
    m_GlobalLayout.AddDividerBox( "FEA Mesh" );

    m_GlobalLayout.AddChoice( m_StructureChoice, "Structure" );

    m_GlobalLayout.AddButton( m_ShowFEAMeshGUI, "Show FEA Mesh GUI" );
    m_GlobalLayout.AddButton( m_ExecuteFEAMesh, "Generate FEA Mesh" );

    m_ConsoleDisplay = m_GlobalLayout.AddFlTextDisplay( 100 );
    m_ConsoleBuffer = new Fl_Text_Buffer;
    m_ConsoleDisplay->buffer( m_ConsoleBuffer );
    m_FLTK_Window->resizable( m_ConsoleDisplay );

}

AeroStructScreen::~AeroStructScreen()
{
    m_ConsoleDisplay->buffer( NULL );
    delete m_ConsoleBuffer;
}

//==== Update Screen ====//
bool AeroStructScreen::Update()
{
    Vehicle *veh = VehicleMgr.GetVehicle();
    if ( !veh )
    {
        assert( false );
        return false;
    }

    AeroStructMgr.Update();

    VSPAEROScreen * AeroScreen = dynamic_cast < VSPAEROScreen* > ( m_ScreenMgr->GetScreen( ScreenMgr::VSP_VSPAERO_SCREEN ) );

    if ( AeroScreen )
    {
        if ( veh->GetVSPAEROFound() && !AeroScreen->m_SolverThreadIsRunning )
        {
            m_ExecuteVSPAERO.Activate();
        }
        else
        {
            m_ExecuteVSPAERO.Deactivate();
        }
    }

    if ( !AeroStructMgr.m_ADBFileFound )
    {
        m_ExecuteVSPAERO.SetColor( FL_RED );
    }
    else
    {
        m_ExecuteVSPAERO.SetColor( FL_BACKGROUND_COLOR );
    }

    if ( FeaMeshMgr.GetFeaMeshInProgress() )
    {
        m_ExecuteFEAMesh.Deactivate();
    }
    else
    {
        m_ExecuteFEAMesh.Activate();
    }

    if ( !AeroStructMgr.m_FEAMeshFileFound )
    {
        m_ExecuteFEAMesh.SetColor( FL_RED );
    }
    else
    {
        m_ExecuteFEAMesh.SetColor( FL_BACKGROUND_COLOR );
    }

    if ( veh->GetLOADSFound() && AeroStructMgr.m_ADBFileFound && AeroStructMgr.m_FEAMeshFileFound )
    {
        m_ExecuteLoads.Activate();
    }
    else
    {
        m_ExecuteLoads.Deactivate();
    }

    if ( !AeroStructMgr.m_FEAInputFileFound )
    {
        m_ExecuteLoads.SetColor( FL_RED );
    }
    else
    {
        m_ExecuteLoads.SetColor( FL_BACKGROUND_COLOR );
    }

    if ( AeroStructMgr.GetCalculiXFound() && AeroStructMgr.m_FEAInputFileFound )
    {
        m_ExecuteCalculiX.Activate();
    }
    else
    {
        m_ExecuteCalculiX.Deactivate();
    }

    if ( !AeroStructMgr.m_FEASolutionFileFound )
    {
        m_ExecuteCalculiX.SetColor( FL_RED );
    }
    else
    {
        m_ExecuteCalculiX.SetColor( FL_BACKGROUND_COLOR );
    }

    if ( veh->GetVIEWERFound() && AeroStructMgr.m_FEASolutionFileFound && AeroStructMgr.m_ADBFileFound )
    {
        m_ExecuteViewer.Activate();
    }
    else
    {
        m_ExecuteViewer.Deactivate();
    }


    m_StructureChoice.ClearItems();
    // Populate browser with added structures
    vector< FeaStructure* > structVec = StructureMgr.GetAllFeaStructs();
    if ( structVec.size() > 0 )
    {
        for ( int i = 0; i < (int)structVec.size(); i++ )
        {
            string struct_name = structVec[i]->GetName();
            int struct_surf_ind = structVec[i]->GetFeaStructMainSurfIndx();
            string parent_geom_name;
            char str[1000];

            Geom* parent = veh->FindGeom( structVec[i]->GetParentGeomID() );
            if ( parent )
            {
                parent_geom_name = parent->GetName();
            }

            sprintf( str, "%s:%s:Surf_%d", struct_name.c_str(), parent_geom_name.c_str(), struct_surf_ind );
            m_StructureChoice.AddItem( str );
        }
    }
    m_StructureChoice.UpdateItems();

    m_StructureChoice.Update( StructureMgr.m_CurrStructIndex.GetID() );




    m_FLTK_Window->redraw();
    return true;
}

//==== Show Screen ====//
void AeroStructScreen::Show()
{
    m_ScreenMgr->SetUpdateFlag( true );
    m_FLTK_Window->show();
}


//==== Hide Screen ====//
void AeroStructScreen::Hide()
{
    m_FLTK_Window->hide();
    m_ScreenMgr->SetUpdateFlag( true );
}

void AeroStructScreen::AddOutputText( const string &text )
{
    Fl::lock();
    m_ConsoleBuffer->append( text.c_str() );
    m_ConsoleDisplay->insert_position( m_ConsoleDisplay->buffer()->length() );
    m_ConsoleDisplay->show_insert_position();
    Fl::unlock();
}

//==== Callbacks ====//
void AeroStructScreen::CallBack( Fl_Widget *w )
{

//    if ( w == m_Foo )
    {

    }
    // else
    {
        return;
    }

    m_ScreenMgr->SetUpdateFlag( true );
}

//==== Gui Device CallBacks ====//
void AeroStructScreen::GuiDeviceCallBack( GuiDevice* gui_device )
{
    assert( m_ScreenMgr );

    if ( gui_device == &m_ShowVSPAEROGUI )
    {
        m_ScreenMgr->ShowScreen( ScreenMgr::VSP_VSPAERO_SCREEN );
    }
    else if( gui_device == &m_ExecuteVSPAERO )
    {
        VSPAEROScreen * AeroScreen = dynamic_cast < VSPAEROScreen* > ( m_ScreenMgr->GetScreen( ScreenMgr::VSP_VSPAERO_SCREEN ) );
        if ( AeroScreen )
        {
            AeroScreen->LaunchVSPAERO();
        }
    }
    else if ( gui_device == &m_ShowFEAMeshGUI )
    {
        m_ScreenMgr->ShowScreen( ScreenMgr::VSP_STRUCT_SCREEN );
    }
    else if ( gui_device == &m_ExecuteFEAMesh )
    {
        StructScreen * structscreen = dynamic_cast < StructScreen* > ( m_ScreenMgr->GetScreen( ScreenMgr::VSP_STRUCT_SCREEN ) );
        if ( structscreen )
        {
            structscreen->LaunchFEAMesh();
        }
    }
    else
    {
        return;
    }

    m_ScreenMgr->SetUpdateFlag( true );
}
