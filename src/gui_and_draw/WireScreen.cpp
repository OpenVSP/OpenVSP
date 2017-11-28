//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#include "WireScreen.h"
#include "WireGeom.h"
#include "ScreenMgr.h"
#include "ParmMgr.h"


//==== Constructor ====//
WireScreen::WireScreen( ScreenMgr* mgr ) : GeomScreen( mgr, 300, 525, "Wireframe" )
{

    RemoveTab( GetTab( m_SubSurfTab_ind ) );
}


//==== Show Blank Screen ====//
void WireScreen::Show()
{
    if ( Update() )
    {
        m_FLTK_Window->show();
    }
}

bool WireScreen::Update()
{
    assert( m_ScreenMgr );

    Geom* geom_ptr = m_ScreenMgr->GetCurrGeom();
    if ( !geom_ptr || geom_ptr->GetType().m_Type != WIRE_FRAME_GEOM_TYPE )
    {
        Hide();
        return false;
    }

    GeomScreen::Update();

    //==== Update Point Cloud Specific Parms ====//
    WireGeom* pt_cloud_geom_ptr = dynamic_cast< WireGeom* >( geom_ptr );
    assert( pt_cloud_geom_ptr );

    return true;
}

//==== Non Menu Callbacks ====//
void WireScreen::CallBack( Fl_Widget *w )
{
    GeomScreen::CallBack( w );
}
