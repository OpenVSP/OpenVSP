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
ImportScreen::ImportScreen( ScreenMgr* mgr ) : VspScreen( mgr )
{
    ImportFileUI* ui = m_ImportUI = new ImportFileUI();
    m_FLTK_Window = ui->UIWindow;

    ui->nascartButton->callback( staticScreenCB, this );
    ui->sterolithButton->callback( staticScreenCB, this );
    ui->xsecButton->callback( staticScreenCB, this );
    ui->xsecSurfButton->callback( staticScreenCB, this );
    ui->Cart3DTriButton->callback( staticScreenCB, this );
}

//==== Destructor ====//
ImportScreen::~ImportScreen()
{
    delete m_ImportUI;
}

//==== Import File ====//
void ImportScreen::ImportFile( string & in_file, int type )
{
    Vehicle* veh = m_ScreenMgr->GetVehiclePtr();

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
    else
    {
        m_ImportUI->UIWindow->show();
        return;
    }

    if ( in_file.size() != 0 && in_file[ in_file.size() - 1] != '/' )
//jrg back() only in c++11 and remove strcmp
//  if ( in_file.compare("") != 0 && strcmp( &in_file.back(), "/") != 0 )
    {
        veh->ImportFile( in_file, type );
    }

    m_ImportUI->UIWindow->hide();
}

//==== Callbacks ===//
void ImportScreen::CallBack( Fl_Widget *w )
{
    Vehicle* veh = m_ScreenMgr->GetVehiclePtr();
    string in_file;

    if ( w == m_ImportUI->sterolithButton )
    {
        ImportFile( in_file, IMPORT_STL );
    }
    else if ( w == m_ImportUI->nascartButton )
    {
        ImportFile( in_file, IMPORT_NASCART );
    }
    else if ( w == m_ImportUI->Cart3DTriButton )
    {
        ImportFile( in_file, IMPORT_CART3D_TRI );
    }
    else if ( w == m_ImportUI->xsecButton )
    {
        ImportFile( in_file, IMPORT_XSEC_MESH );
    }

    m_ScreenMgr->SetUpdateFlag( true );
}
