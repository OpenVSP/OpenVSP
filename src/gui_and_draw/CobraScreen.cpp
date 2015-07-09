//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#include "CobraScreen.h"
#include "ScreenMgr.h"
#include "CobraGeom.h"

//==== Constructor ====//
CobraScreen::CobraScreen( ScreenMgr* mgr ) : GeomScreen( mgr, 400, 800, "Cobra" )
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
void CobraScreen::Show()
{
    if ( Update() )
    {
        m_FLTK_Window->show();
    }
}

//==== Update Disk Screen ====//
bool CobraScreen::Update()
{
    assert( m_ScreenMgr );

    Geom* geom_ptr = m_ScreenMgr->GetCurrGeom();
    if ( !geom_ptr || geom_ptr->GetType().m_Type != COBRA_GEOM_TYPE )
    {
        Hide();
        return false;
    }

    GeomScreen::Update();

    //==== Update Cobra Specific Parms ====//
    CobraGeom* cobra_ptr = dynamic_cast< CobraGeom* >( geom_ptr );
    assert( cobra_ptr );
    m_LengthSlider.Update( cobra_ptr->m_Length.GetID() );
    m_FineSlider.Update( cobra_ptr->m_FineRatio.GetID() );

    return true;
}


//==== Non Menu Callbacks ====//
void CobraScreen::CallBack( Fl_Widget *w )
{
    GeomScreen::CallBack( w );
}




