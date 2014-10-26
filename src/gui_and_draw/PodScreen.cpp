//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#include "PodScreen.h"
#include "ScreenMgr.h"
#include "PodGeom.h"
#include "EventMgr.h"
#include "Vehicle.h"
#include "ParmMgr.h"

#include <assert.h>


//==== Constructor ====//
PodScreen::PodScreen( ScreenMgr* mgr ) : GeomScreen( mgr, 300, 525, "Pod" )
{
    Fl_Group* design_tab = AddTab( "Design" );
    Fl_Group* design_group = AddSubGroup( design_tab, 5 );

    m_DesignLayout.SetGroupAndScreen( design_group, this );
    m_DesignLayout.AddDividerBox( "Design" );

    //==== Design ====//
    m_DesignLayout.AddSlider( m_LengthSlider, "Length", 10, "%7.3f" );
    m_DesignLayout.AddSlider( m_FineSlider, "Fine Ratio", 5, "%7.3f" );
    m_DesignLayout.AddYGap();

    m_DesignLayout.SetButtonWidth( 100 );
    m_DesignLayout.AddSlider( m_NumBaseSlider, "Num XSec Pnts", 33, "%5.0f" );
}


//==== Show Pod Screen ====//
void PodScreen::Show()
{
    if ( Update() )
    {
        m_FLTK_Window->show();
    }
}

//==== Update Pod Screen ====//
bool PodScreen::Update()
{
    assert( m_ScreenMgr );

    Geom* geom_ptr = m_ScreenMgr->GetCurrGeom();
    if ( !geom_ptr || geom_ptr->GetType().m_Type != POD_GEOM_TYPE )
    {
        Hide();
        return false;
    }

    GeomScreen::Update();

    //==== Update Pod Specific Parms ====//
    PodGeom* pod_ptr = dynamic_cast< PodGeom* >( geom_ptr );
    assert( pod_ptr );
    m_LengthSlider.Update( pod_ptr->m_Length.GetID() );
    m_FineSlider.Update( pod_ptr->m_FineRatio.GetID() );
//  m_NumBaseSlider.Update( pod_ptr->m_BaseU.GetID() );

    return true;
}


//==== Non Menu Callbacks ====//
void PodScreen::CallBack( Fl_Widget *w )
{
    GeomScreen::CallBack( w );
}




