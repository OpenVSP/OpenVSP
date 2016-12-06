//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#include "MeshScreen.h"
#include "ScreenMgr.h"
#include "MeshGeom.h"
#include "EventMgr.h"
#include "ParmMgr.h"


//==== Constructor ====//
MeshScreen::MeshScreen( ScreenMgr* mgr ) : GeomScreen( mgr, 300, 525, "Mesh" )
{
    RemoveTab( GetTab( m_SubSurfTab_ind ) );

    Fl_Group* other_tab = AddTab( "Other" );
    Fl_Group* other_group = AddSubGroup( other_tab, 5 );

    m_OtherLayout.SetGroupAndScreen( other_group, this );
    m_OtherLayout.AddDividerBox( "Convert to Point Cloud" );
    m_OtherLayout.AddYGap();

    m_OtherLayout.AddButton( m_ConvertButton, "Convert" );

    m_OtherLayout.AddYGap();
    m_OtherLayout.AddDividerBox( "Visualize Meshes" );
    m_OtherLayout.AddYGap();

    m_OtherLayout.AddButton( m_ViewMeshToggle, "Mesh" );
    m_OtherLayout.AddButton( m_ViewSliceToggle, "Slices" );

}


//==== Show Mesh Screen ====//
void MeshScreen::Show()
{
    if ( Update() )
    {
        m_FLTK_Window->show();
    }
}

//==== Update Mesh Screen ====//
bool MeshScreen::Update()
{
    assert( m_ScreenMgr );

    Geom* geom_ptr = m_ScreenMgr->GetCurrGeom();
    if ( !geom_ptr || geom_ptr->GetType().m_Type != MESH_GEOM_TYPE )
    {
        Hide();
        return false;
    }

    GeomScreen::Update();

    MeshGeom* mesh_ptr = dynamic_cast< MeshGeom* >( geom_ptr );

    m_ViewMeshToggle.Update( mesh_ptr->m_ViewMeshFlag.GetID() );
    m_ViewSliceToggle.Update( mesh_ptr->m_ViewSliceFlag.GetID() );

    return true;
}


//==== Non Menu Callbacks ====//
void MeshScreen::CallBack( Fl_Widget *w )
{
    GeomScreen::CallBack( w );
}

void MeshScreen::GuiDeviceCallBack( GuiDevice* gui_device )
{
    Geom* geom_ptr = m_ScreenMgr->GetCurrGeom();
    if ( !geom_ptr || geom_ptr->GetType().m_Type != MESH_GEOM_TYPE )
    {
        return;
    }
    MeshGeom* mesh_ptr = dynamic_cast< MeshGeom* >( geom_ptr );
    assert( mesh_ptr );

    if ( gui_device == &m_ConvertButton )
    {
        mesh_ptr->CreatePtCloudGeom();
    }
}
