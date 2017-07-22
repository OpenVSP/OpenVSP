//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#include "EllipsoidScreen.h"
#include "ScreenMgr.h"
#include "EllipsoidGeom.h"
#include "ParmMgr.h"


//==== Constructor ====//
EllipsoidScreen::EllipsoidScreen( ScreenMgr* mgr ) : GeomScreen( mgr, 300, 525, "Ellipsoid" )
{
    Fl_Group* design_tab = AddTab( "Design" );
    Fl_Group* design_group = AddSubGroup( design_tab, 5 );

    m_DesignLayout.SetGroupAndScreen( design_group, this );
    m_DesignLayout.AddDividerBox( "Design" );

    //==== Design ====//
    m_DesignLayout.AddSlider( m_ARadiusSlider, "A Radius", 10, "%7.3f" );
    m_DesignLayout.AddSlider( m_BRadiusSlider, "B Radius", 10, "%7.3f" );
    m_DesignLayout.AddSlider( m_CRadiusSlider, "C Radius", 10, "%7.3f" );

    m_DesignLayout.AddYGap();

}


//==== Show Ellipsoid Screen ====//
void EllipsoidScreen::Show()
{
    if ( Update() )
    {
        m_FLTK_Window->show();
    }
}

//==== Update Ellipsoid Screen ====//
bool EllipsoidScreen::Update()
{
    assert( m_ScreenMgr );

    Geom* geom_ptr = m_ScreenMgr->GetCurrGeom();
    if ( !geom_ptr || geom_ptr->GetType().m_Type != ELLIPSOID_GEOM_TYPE )
    {
        Hide();
        return false;
    }

    GeomScreen::Update();

    //==== Update Ellipsoid Specific Parms ====//
    EllipsoidGeom* ellipsoid_ptr = dynamic_cast< EllipsoidGeom* >( geom_ptr );
    assert( ellipsoid_ptr );
    m_ARadiusSlider.Update( ellipsoid_ptr->m_Aradius.GetID() );
    m_BRadiusSlider.Update( ellipsoid_ptr->m_Bradius.GetID() );
    m_CRadiusSlider.Update( ellipsoid_ptr->m_Cradius.GetID() );

    return true;
}


//==== Non Menu Callbacks ====//
void EllipsoidScreen::CallBack( Fl_Widget *w )
{
    GeomScreen::CallBack( w );
}




