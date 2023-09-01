//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#include "NGonMeshScreen.h"
#include "NGonMeshGeom.h"
#include "ScreenMgr.h"


//==== Constructor ====//
NGonMeshScreen::NGonMeshScreen( ScreenMgr* mgr ) : GeomScreen( mgr, 300, 525, "NGon Mesh" )
{
    RemoveTab( GetTab( m_SubSurfTab_ind ) );

    Fl_Group* other_tab = AddTab( "Other" );
    Fl_Group* other_group = AddSubGroup( other_tab, 5 );

    m_OtherLayout.SetGroupAndScreen( other_group, this );

    m_OtherLayout.AddDividerBox( "Actions" );
    m_OtherLayout.AddYGap();

    m_OtherLayout.AddButton( m_TriangulateButton, "Triangulate" );
    m_OtherLayout.AddYGap();
    m_OtherLayout.AddButton( m_ReportButton, "Report" );
    m_OtherLayout.AddYGap();
    m_OtherLayout.AddButton( m_WriteVSPGEOMButton, "Write VSPGEOM" );
    m_OtherLayout.AddYGap();

}


//==== Show Blank Screen ====//
void NGonMeshScreen::Show()
{
    if ( Update() )
    {
        m_FLTK_Window->show();
    }
}

bool NGonMeshScreen::Update()
{
    assert( m_ScreenMgr );
    Vehicle* vPtr = VehicleMgr.GetVehicle();
    assert( vPtr );

    Geom* geom_ptr = m_ScreenMgr->GetCurrGeom();
    if ( !geom_ptr || geom_ptr->GetType().m_Type != NGON_GEOM_TYPE )
    {
        Hide();
        return false;
    }

    GeomScreen::Update();

    //==== Update NGonMesh Specific Parms ====//
    NGonMeshGeom* ngon_mesh_geom_ptr = dynamic_cast< NGonMeshGeom* >( geom_ptr );
    assert( ngon_mesh_geom_ptr );

    return true;
}

//==== Non Menu Callbacks ====//
void NGonMeshScreen::CallBack( Fl_Widget *w )
{
    GeomScreen::CallBack( w );
}

void NGonMeshScreen::GuiDeviceCallBack( GuiDevice* device )
{
    assert( m_ScreenMgr );

    Geom* geom_ptr = m_ScreenMgr->GetCurrGeom();
    if ( !geom_ptr || geom_ptr->GetType().m_Type != NGON_GEOM_TYPE )
    {
        return;
    }

    NGonMeshGeom* ngon_mesh_geom_ptr = dynamic_cast< NGonMeshGeom* >( geom_ptr );
    assert( ngon_mesh_geom_ptr );

    if ( !ngon_mesh_geom_ptr )
    {
        return;
    }

    if ( device == &m_WriteVSPGEOMButton )
    {
        string newfile = m_ScreenMgr->FileChooser( "Write VSPGeom Files?", "*.vspgeom", vsp::SAVE );

        if ( newfile.size() != 0 && newfile[ newfile.size() - 1] != '/' )
        {
            vector < string > all_files;
            ngon_mesh_geom_ptr->WriteVSPGEOM( newfile, all_files );
        }
    }
    else if ( device == & m_TriangulateButton )
    {
        ngon_mesh_geom_ptr->Triangulate();
    }
    else if ( device == &m_ReportButton )
    {
        ngon_mesh_geom_ptr->Report();
    }


    GeomScreen::GuiDeviceCallBack( device );
}
