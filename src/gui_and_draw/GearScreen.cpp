//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#include "GearScreen.h"
#include "ScreenMgr.h"
#include "GearGeom.h"
#include "Vehicle.h"
#include "ParmMgr.h"

#include <assert.h>


//==== Constructor ====//
Gearcreen::Gearcreen( ScreenMgr* mgr ) : GeomScreen( mgr, 300, 525, "Gear" )
{
    Fl_Group* design_tab = AddTab( "Design" );
    Fl_Group* design_group = AddSubGroup( design_tab, 5 );

    m_DesignLayout.SetGroupAndScreen( design_group, this );
    m_DesignLayout.AddDividerBox( "Design" );

    //==== Design ====//
    m_DesignLayout.AddSlider( m_LengthSlider, "Length", 10, "%7.3f" );
    m_DesignLayout.AddSlider( m_FineSlider, "Fine Ratio", 5, "%7.3f" );
    m_DesignLayout.AddYGap();

}


//==== Show Pod Screen ====//
void Gearcreen::Show()
{
    if ( Update() )
    {
        m_FLTK_Window->show();
    }
}

//==== Update Disk Screen ====//
bool Gearcreen::Update()
{
    assert( m_ScreenMgr );

    Geom* geom_ptr = m_ScreenMgr->GetCurrGeom();
    if ( !geom_ptr || geom_ptr->GetType().m_Type != GEAR_GEOM_TYPE )
    {
        Hide();
        return false;
    }

    GeomScreen::Update();

    //==== Update Pod Specific Parms ====//
    GearGeom* pod_ptr = dynamic_cast< GearGeom* >( geom_ptr );
    assert( pod_ptr );
//    m_LengthSlider.Update( pod_ptr->m_Length.GetID() );
//    m_FineSlider.Update( pod_ptr->m_FineRatio.GetID() );
//  m_NumBaseSlider.Update( pod_ptr->m_BaseU.GetID() );

    return true;
}


//==== Non Menu Callbacks ====//
void Gearcreen::CallBack( Fl_Widget *w )
{
    GeomScreen::CallBack( w );
}




