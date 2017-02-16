//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#include "ExportScreen.h"
#include "ScreenMgr.h"
#include "StlHelper.h"
#include "STEPOptionsScreen.h"
#include "IGESOptionsScreen.h"
#include "STLOptionsScreen.h"
#include "BEMOptionsScreen.h"
#include "DXFOptionsScreen.h"
#include "SVGOptionsScreen.h"
using namespace vsp;

//==== Constructor ====//
ExportScreen::ExportScreen( ScreenMgr* mgr ) : BasicScreen( mgr, 150, 25 + (1+14)*20 + 2*15 + 4*6, "Export" )
{
    m_SelectedSetIndex = 0;

    m_MainLayout.SetGroupAndScreen( m_FLTK_Window, this );
    m_MainLayout.AddX( 5 );
    m_MainLayout.AddY( 25 );
    m_MainLayout.AddSubGroupLayout( m_GenLayout, m_MainLayout.GetRemainX() - 5, m_MainLayout.GetRemainY() );

    m_GenLayout.SetChoiceButtonWidth( 50 );

    m_GenLayout.AddDividerBox( "Export Set" );
    m_GenLayout.AddYGap();

    m_GenLayout.AddChoice( m_ExportSetChoice, "Set:" );
    m_GenLayout.AddYGap();

    m_GenLayout.AddDividerBox( "File Format" );
    m_GenLayout.AddYGap();

    m_GenLayout.AddButton( m_XSecButton, "XSec (*.hrm)" );
    m_GenLayout.AddButton( m_Plot3DButton, "PLOT3D (.p3d)" );
    m_GenLayout.AddButton( m_STLButton, "Stereolith (.stl)" );
    m_GenLayout.AddButton( m_NASCARTButton, "NASCART (.dat)" );
    m_GenLayout.AddButton( m_TRIButton, "Cart3D (.tri)" );
    m_GenLayout.AddButton( m_GMSHButton, "Gmsh (.msh)" );
    m_GenLayout.AddButton( m_POVButton, "POVRAY (.pov)" );
    m_GenLayout.AddButton( m_X3DButton, "X3D (.x3d)" );
    m_GenLayout.AddButton( m_STEPButton, "STEP (.stp)" );
    m_GenLayout.AddButton( m_IGESButton, "IGES (.igs)" );
    m_GenLayout.AddButton( m_BEMButton, "Blade Element (.bem)" );
    m_GenLayout.AddButton( m_DXFButton, "AutoCAD (.dxf)" );
    m_GenLayout.AddButton( m_SVGButton, "SVG (.svg)" );
    m_GenLayout.AddButton( m_FacetButton, "Xpatch (.facet)" );
}

//==== Update Screen ====//
bool ExportScreen::Update()
{
    LoadSetChoice();

    return true;
}

//==== Show Screen ====//
void ExportScreen::Show()
{
    Update();
    m_FLTK_Window->show();
}

//==== Hide Screen ====//
void ExportScreen::Hide()
{
    m_FLTK_Window->hide();
}

//==== Load Type Choice ====//
void ExportScreen::LoadSetChoice()
{
    Vehicle *veh = VehicleMgr.GetVehicle();

    m_ExportSetChoice.ClearItems();

    vector <string> setVec = veh->GetSetNameVec();
    for ( int i = 0; i < setVec.size(); i++ )
    {
        m_ExportSetChoice.AddItem( setVec[i] );
    }
    m_ExportSetChoice.UpdateItems();

    m_ExportSetChoice.SetVal( m_SelectedSetIndex );
}

void ExportScreen::ExportFile( string &newfile, int write_set, int type )
{
    Vehicle* veh = m_ScreenMgr->GetVehiclePtr();

    if ( type == EXPORT_XSEC )
    {
        newfile = m_ScreenMgr->GetSelectFileScreen()->FileChooser( "Write XSec File?", "*.hrm" );
    }
    else if ( type == EXPORT_PLOT3D )
    {
        newfile = m_ScreenMgr->GetSelectFileScreen()->FileChooser( "Write PLOT3D File?", "*.p3d" );
    }
    else if ( type == EXPORT_STL )
    {
        if ( (( STLOptionsScreen* ) m_ScreenMgr->GetScreen( ScreenMgr::VSP_STL_OPTIONS_SCREEN ))->ShowSTLOptionsScreen() )
        {
            newfile = m_ScreenMgr->GetSelectFileScreen()->FileChooser( "Write STL File?", "*.stl" );
        }
    }
    else if ( type == EXPORT_NASCART )
    {
        newfile = m_ScreenMgr->GetSelectFileScreen()->FileChooser( "Write NASCART Files?", "*.dat" );
    }
    else if ( type == EXPORT_CART3D )
    {
        newfile = m_ScreenMgr->GetSelectFileScreen()->FileChooser( "Write Cart3D Files?", "*.tri" );
    }
    else if ( type == EXPORT_GMSH )
    {
        newfile = m_ScreenMgr->GetSelectFileScreen()->FileChooser( "Write GMsh Files?", "*.msh" );
    }
    else if ( type == EXPORT_POVRAY )
    {
        newfile = m_ScreenMgr->GetSelectFileScreen()->FileChooser( "Write POVRAY File?", "*.pov" );
    }
    else if ( type == EXPORT_X3D )
    {
        newfile = m_ScreenMgr->GetSelectFileScreen()->FileChooser( "Write X3D File?", "*.x3d" );
    }
    else if ( type == EXPORT_STEP )
    {
        if ( (( STEPOptionsScreen* ) m_ScreenMgr->GetScreen( ScreenMgr::VSP_STEP_OPTIONS_SCREEN ))->ShowSTEPOptionsScreen() )
        {
            newfile = m_ScreenMgr->GetSelectFileScreen()->FileChooser( "Write STEP File?", "*.stp" );
        }
    }
    else if ( type == EXPORT_IGES )
    {
        if ( (( IGESOptionsScreen* ) m_ScreenMgr->GetScreen( ScreenMgr::VSP_IGES_OPTIONS_SCREEN ))->ShowIGESOptionsScreen() )
        {
            newfile = m_ScreenMgr->GetSelectFileScreen()->FileChooser( "Write IGES File?", "*.igs" );
        }
    }
    else if ( type == EXPORT_BEM )
    {
        if ( (( BEMOptionsScreen* ) m_ScreenMgr->GetScreen( ScreenMgr::VSP_BEM_OPTIONS_SCREEN ))->ShowBEMOptionsScreen() )
        {
            newfile = m_ScreenMgr->GetSelectFileScreen()->FileChooser( "Write Blade Element File?", "*.bem" );
        }
    }
    else if ( type == EXPORT_DXF )
    {
        if ( (( DXFOptionsScreen* ) m_ScreenMgr->GetScreen( ScreenMgr::VSP_DXF_OPTIONS_SCREEN ))->ShowDXFOptionsScreen() )
        {
            newfile = m_ScreenMgr->GetSelectFileScreen()->FileChooser( "Write DXF File?", "*.dxf" );
        }
    }
    else if ( type == EXPORT_SVG )
    {
        if ( (( SVGOptionsScreen* ) m_ScreenMgr->GetScreen( ScreenMgr::VSP_SVG_OPTIONS_SCREEN ))->ShowSVGOptionsScreen( write_set ) )
        {
            newfile = m_ScreenMgr->GetSelectFileScreen()->FileChooser( "Write SVG File?", "*.svg" );
        }
    }
    else if ( type == EXPORT_FACET )
    {
        newfile = m_ScreenMgr->GetSelectFileScreen()->FileChooser("Write Facet File?", "*.facet");
    }
    else if ( type == -1 )
    {
        return;
    }

    if ( newfile.size() != 0 && newfile[ newfile.size() - 1] != '/' )
    {
        veh->ExportFile( newfile, write_set, type );
    }

}

//==== Callbacks ====//
void ExportScreen::CallBack( Fl_Widget *w )
{
    m_ScreenMgr->SetUpdateFlag( true );
}

void ExportScreen::GuiDeviceCallBack( GuiDevice* device )
{
    string newfile;

    if ( device == &m_XSecButton )
    {
        ExportFile( newfile, m_SelectedSetIndex, EXPORT_XSEC );
    }
    else if ( device == &m_Plot3DButton )
    {
        ExportFile( newfile, m_SelectedSetIndex, EXPORT_PLOT3D );
    }
    else if (  device == &m_STLButton )
    {
        ExportFile( newfile, m_SelectedSetIndex, EXPORT_STL );
    }
    else if (  device == &m_NASCARTButton )
    {
        ExportFile( newfile, m_SelectedSetIndex, EXPORT_NASCART );
    }
    else if (  device == &m_TRIButton )
    {
        ExportFile( newfile, m_SelectedSetIndex, EXPORT_CART3D );
    }
    else if (  device == &m_GMSHButton )
    {
        ExportFile( newfile, m_SelectedSetIndex, EXPORT_GMSH );
    }
    else if (  device == &m_POVButton )
    {
        ExportFile( newfile, m_SelectedSetIndex, EXPORT_POVRAY );
    }
    else if (  device == &m_X3DButton )
    {
        ExportFile( newfile, m_SelectedSetIndex, EXPORT_X3D );
    }
    else if (  device == &m_STEPButton )
    {
        ExportFile( newfile, m_SelectedSetIndex, EXPORT_STEP );
    }
    else if ( device == &m_IGESButton )
    {
        ExportFile( newfile, m_SelectedSetIndex, EXPORT_IGES );
    }
    else if ( device == &m_BEMButton )
    {
        ExportFile( newfile, m_SelectedSetIndex, EXPORT_BEM );
    }
    else if ( device == &m_DXFButton )
    {
        ExportFile( newfile, m_SelectedSetIndex, EXPORT_DXF );
    }
    else if ( device == &m_SVGButton )
    {
        ExportFile( newfile, m_SelectedSetIndex, EXPORT_SVG );
    }
    else if ( device == &m_FacetButton )
    {
        ExportFile(newfile, m_SelectedSetIndex, EXPORT_FACET);
    }
    else if (  device == &m_ExportSetChoice )
    {
        m_SelectedSetIndex = m_ExportSetChoice.GetVal();
    }

    m_ScreenMgr->SetUpdateFlag( true );
}
