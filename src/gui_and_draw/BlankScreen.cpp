//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#include "BlankScreen.h"
#include "BlankGeom.h"
#include "ScreenMgr.h"
#include "EventMgr.h"
#include "ParmMgr.h"


//==== Constructor ====//
BlankScreen::BlankScreen( ScreenMgr* mgr ) : GeomScreen( mgr, 300, 525, "Blank" )
{
    Fl_Group* mass_tab = AddTab( "Mass" );
    Fl_Group* mass_group = AddSubGroup( mass_tab, 5 );

    m_MassLayout.SetGroupAndScreen( mass_group, this );

    m_MassLayout.AddDividerBox( "Point Mass" );
    m_MassLayout.AddYGap();

    m_MassLayout.SetFitWidthFlag( false );
    m_MassLayout.SetSameLineFlag( true );
    m_MassLayout.SetButtonWidth( 100 );
    m_MassLayout.AddButton( m_PointMassButton, "Point Mass" );
    m_MassLayout.SetButtonWidth( 50 );
    m_MassLayout.SetInputWidth( m_MassLayout.GetRemainX() - 50 );
    m_MassLayout.AddInput( m_MassInput, "Mass", " %7.5f" );

    RemoveTab( GetTab( m_SubSurfTab_ind ) );

}


//==== Show Blank Screen ====//
void BlankScreen::Show()
{
    if ( Update() )
    {
        m_FLTK_Window->show();
    }
}

bool BlankScreen::Update()
{
    assert( m_ScreenMgr );

    Geom* geom_ptr = m_ScreenMgr->GetCurrGeom();
    if ( !geom_ptr || geom_ptr->GetType().m_Type != BLANK_GEOM_TYPE )
    {
        Hide();
        return false;
    }

    GeomScreen::Update();

    //==== Update Blank Specific Parms ====//
    BlankGeom* blank_geom_ptr = dynamic_cast< BlankGeom* >( geom_ptr );
    assert( blank_geom_ptr );
    m_PointMassButton.Update( blank_geom_ptr->m_PointMassFlag.GetID() );
    m_MassInput.Update( blank_geom_ptr->m_PointMass.GetID() );

    return true;
}

//==== Non Menu Callbacks ====//
void BlankScreen::CallBack( Fl_Widget *w )
{
    GeomScreen::CallBack( w );
}
