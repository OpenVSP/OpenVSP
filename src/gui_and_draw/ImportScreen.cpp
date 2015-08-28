//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//////////////////////////////////////////////////////////////////////

#include "ImportScreen.h"
#include "ScreenMgr.h"
#include "EventMgr.h"
#include "Vehicle.h"
#include "StlHelper.h"
#include "APIDefines.h"
using namespace vsp;

#include <assert.h>

//==== Constructor ====//
ImportScreen::ImportScreen( ScreenMgr* mgr ) : BasicScreen( mgr , 150, 25 + 5*20 + 1*15 + 2*6, "Import" )
{
    m_MainLayout.SetGroupAndScreen( m_FLTK_Window, this );
    m_MainLayout.AddX( 5 );
    m_MainLayout.AddY( 25 );
    m_MainLayout.AddSubGroupLayout( m_GenLayout, m_MainLayout.GetRemainX() - 5, m_MainLayout.GetRemainY() );

    m_GenLayout.SetChoiceButtonWidth( 50 );

    m_GenLayout.AddDividerBox( "File Format" );
    m_GenLayout.AddYGap();

    m_GenLayout.AddButton( m_TRIButton, "Cart3D (.tri)" );
    m_GenLayout.AddButton( m_STLButton, "Stereolith (.stl)" );
    m_GenLayout.AddButton( m_NASCARTButton, "NASCART (.dat)" );
    m_GenLayout.AddButton( m_XSecButton, "XSec (*.hrm)" );
    m_GenLayout.AddButton( m_PTSButton, "Point Cloud (.pts)" );
}

//==== Destructor ====//
ImportScreen::~ImportScreen()
{
}

//==== Import File ====//
void ImportScreen::ImportFile( string & in_file, int type )
{
    Vehicle *veh = VehicleMgr.GetVehicle();

    if ( type == IMPORT_STL )
    {
        in_file = m_ScreenMgr->GetSelectFileScreen()->FileChooser( "Import STL file?", "*.stl" );
    }
    else if ( type == IMPORT_NASCART )
    {
        in_file = m_ScreenMgr->GetSelectFileScreen()->FileChooser( "Import NASCART file?", "*.dat" );
    }
    else if ( type == IMPORT_CART3D_TRI )
    {
        in_file = m_ScreenMgr->GetSelectFileScreen()->FileChooser( "Import Cart3D Tri File?", "*.tri" );
    }
    else if ( type == IMPORT_XSEC_MESH )
    {
        in_file = m_ScreenMgr->GetSelectFileScreen()->FileChooser( "Import XSec File?", "*.hrm" );
    }
    else if ( type == IMPORT_PTS )
    {
        in_file = m_ScreenMgr->GetSelectFileScreen()->FileChooser( "Import Points File?", "*.pts" );
    }
    else
    {
        return;
    }

    if ( in_file.size() != 0 && in_file[ in_file.size() - 1] != '/' )
    {
        veh->ImportFile( in_file, type );
    }

    m_ScreenMgr->SetUpdateFlag( true );
}

//==== Callbacks ===//
void ImportScreen::CallBack( Fl_Widget *w )
{
    m_ScreenMgr->SetUpdateFlag( true );
}

void ImportScreen::GuiDeviceCallBack( GuiDevice* device )
{
    string in_file;

    if ( device == &m_STLButton )
    {
        ImportFile( in_file, IMPORT_STL );
    }
    else if ( device == &m_NASCARTButton )
    {
        ImportFile( in_file, IMPORT_NASCART );
    }
    else if ( device == &m_TRIButton )
    {
        ImportFile( in_file, IMPORT_CART3D_TRI );
    }
    else if ( device ==  &m_XSecButton )
    {
        ImportFile( in_file, IMPORT_XSEC_MESH );
    }
    else if ( device == &m_PTSButton )
    {
        ImportFile( in_file, IMPORT_PTS );
    }

    m_ScreenMgr->SetUpdateFlag( true );
}
