//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#include "PtCloudScreen.h"
#include "PtCloudGeom.h"
#include "ScreenMgr.h"
#include "Vehicle.h"
#include "ParmMgr.h"

#include <assert.h>


//==== Constructor ====//
PtCloudScreen::PtCloudScreen( ScreenMgr* mgr ) : GeomScreen( mgr, 300, 525, "Point Cloud" )
{

    RemoveTab( GetTab( m_SubSurfTab_ind ) );
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

    return true;
}

//==== Non Menu Callbacks ====//
void PtCloudScreen::CallBack( Fl_Widget *w )
{
    GeomScreen::CallBack( w );
}
