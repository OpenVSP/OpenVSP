//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#include "PtCloudScreen.h"
#include "PtCloudGeom.h"
#include "ScreenMgr.h"


//==== Constructor ====//
PtCloudScreen::PtCloudScreen( ScreenMgr* mgr ) : GeomScreen( mgr, 300, 525, "Point Cloud" )
{

    RemoveTab( GetTab( m_SubSurfTab_ind ) );

    Fl_Group* project_tab = AddTab( "Project" );
    Fl_Group* project_group = AddSubGroup( project_tab, 5 );

    m_ProjectLayout.SetGroupAndScreen( project_group, this );
    m_ProjectLayout.AddDividerBox( "Project Points to Geom" );
    m_ProjectLayout.AddYGap();

    m_GeomPicker.AddExcludeType( MESH_GEOM_TYPE );
    m_GeomPicker.AddExcludeType( HUMAN_GEOM_TYPE );
    m_GeomPicker.AddExcludeType( PT_CLOUD_GEOM_TYPE );
    m_GeomPicker.AddExcludeType( WIRE_FRAME_GEOM_TYPE );
    m_GeomPicker.AddExcludeType( BLANK_GEOM_TYPE );
    m_GeomPicker.AddExcludeType( HINGE_GEOM_TYPE );
    m_ProjectLayout.AddGeomPicker( m_GeomPicker );

    m_ProjectLayout.AddChoice( m_SurfChoice, "Surface" );

    m_ProjectLayout.AddChoice( m_DirChoice, "Direction" );

    m_DirChoice.AddItem( "X" );
    m_DirChoice.AddItem( "Y" );
    m_DirChoice.AddItem( "Z" );
    m_DirChoice.UpdateItems();

    m_ProjectLayout.AddYGap();
    m_ProjectLayout.AddButton( m_ProjectButton, "Project" );

}


//==== Show Blank Screen ====//
void PtCloudScreen::Show()
{
    if ( Update() )
    {
        m_FLTK_Window->show();
    }
}

bool PtCloudScreen::Update()
{
    assert( m_ScreenMgr );
    Vehicle* vPtr = VehicleMgr.GetVehicle();
    assert( vPtr );

    Geom* geom_ptr = m_ScreenMgr->GetCurrGeom();
    if ( !geom_ptr || geom_ptr->GetType().m_Type != PT_CLOUD_GEOM_TYPE )
    {
        Hide();
        return false;
    }

    GeomScreen::Update();

    //==== Update Point Cloud Specific Parms ====//
    PtCloudGeom* pt_cloud_geom_ptr = dynamic_cast< PtCloudGeom* >( geom_ptr );
    assert( pt_cloud_geom_ptr );

    m_GeomPicker.Update();

    m_SurfChoice.ClearItems();

    Geom* geom = vPtr->FindGeom( m_GeomPicker.GetGeomChoice() );

    if ( geom )
    {
        int nsurf = geom->GetNumTotalSurfs();
        char str[256];
        for ( int i = 0; i < nsurf; ++i )
        {
            sprintf( str, "Surf_%d", i );
            m_SurfChoice.AddItem( str );
        }
        m_SurfChoice.UpdateItems();
    }

    return true;
}

//==== Non Menu Callbacks ====//
void PtCloudScreen::CallBack( Fl_Widget *w )
{
    GeomScreen::CallBack( w );
}

void PtCloudScreen::GuiDeviceCallBack( GuiDevice* device )
{
    assert( m_ScreenMgr );

    Geom* geom_ptr = m_ScreenMgr->GetCurrGeom();
    if ( !geom_ptr || geom_ptr->GetType().m_Type != PT_CLOUD_GEOM_TYPE )
    {
        return;
    }

    PtCloudGeom* pt_cloud_geom_ptr = dynamic_cast< PtCloudGeom* >( geom_ptr );
    assert( pt_cloud_geom_ptr );

    if ( device == &m_ProjectButton )
    {
        if ( pt_cloud_geom_ptr )
        {
            string geomid = m_GeomPicker.GetGeomChoice();
            int surfid = m_SurfChoice.GetVal();
            int idir = m_DirChoice.GetVal();

            pt_cloud_geom_ptr->ProjectPts( geomid, surfid, idir );
        }
    }

    GeomScreen::GuiDeviceCallBack( device );
}
