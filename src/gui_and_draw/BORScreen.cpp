//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#include "BORScreen.h"
#include "ScreenMgr.h"
#include "BORGeom.h"
#include "ParmMgr.h"


//==== Constructor ====//
BORScreen::BORScreen( ScreenMgr* mgr ) : GeomScreen( mgr, 300, 525, "BOR" )
{
    Fl_Group* design_tab = AddTab( "Design" );
    Fl_Group* design_group = AddSubGroup( design_tab, 5 );

    m_DesignLayout.SetGroupAndScreen( design_group, this );
    m_DesignLayout.AddDividerBox( "Design" );

    //==== Design ====//
    m_DesignLayout.AddSlider( m_DiameterSlider, "Diameter", 10, "%7.3f" );
    m_DesignLayout.AddSlider( m_LengthSlider, "Length", 10, "%7.3f" );

    m_DesignLayout.AddYGap();

}


//==== Show BOR Screen ====//
void BORScreen::Show()
{
    if ( Update() )
    {
        m_FLTK_Window->show();
    }
}

//==== Update BOR Screen ====//
bool BORScreen::Update()
{
    assert( m_ScreenMgr );

    Geom* geom_ptr = m_ScreenMgr->GetCurrGeom();
    if ( !geom_ptr || geom_ptr->GetType().m_Type != BOR_GEOM_TYPE )
    {
        Hide();
        return false;
    }

    GeomScreen::Update();

    //==== Update BOR Specific Parms ====//
    BORGeom* bor_ptr = dynamic_cast< BORGeom* >( geom_ptr );
    assert( bor_ptr );
    m_DiameterSlider.Update( bor_ptr->m_Diameter.GetID() );
    m_LengthSlider.Update( bor_ptr->m_Length.GetID() );

    return true;
}


//==== Non Menu Callbacks ====//
void BORScreen::CallBack( Fl_Widget *w )
{
    GeomScreen::CallBack( w );
}




