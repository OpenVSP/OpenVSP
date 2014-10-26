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
#include "Vehicle.h"
#include "ParmMgr.h"

#include <assert.h>


//==== Constructor ====//
MeshScreen::MeshScreen( ScreenMgr* mgr ) : GeomScreen( mgr, 300, 525, "Mesh" )
{
    RemoveTab( GetTab( m_SubSurfTab_ind ) );

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

    return true;
}


//==== Non Menu Callbacks ====//
void MeshScreen::CallBack( Fl_Widget *w )
{
    GeomScreen::CallBack( w );
}
