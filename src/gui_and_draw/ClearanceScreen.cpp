//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#include "ClearanceScreen.h"
#include "ScreenMgr.h"
#include "ClearanceGeom.h"
#include "APIDefines.h"

//==== Constructor ====//
ClearanceScreen::ClearanceScreen( ScreenMgr* mgr ) : GeomScreen( mgr, 400, 657 + 25, "Clearance" )
{
    Fl_Group* design_tab = AddTab( "Design" );
    Fl_Group* design_group = AddSubGroup( design_tab, 5 );

    m_DesignLayout.SetGroupAndScreen( design_group, this );
    m_DesignLayout.AddDividerBox( "Design" );

    //==== Design ====//


}


//==== Show Pod Screen ====//
void ClearanceScreen::Show()
{
    if ( Update() )
    {
        GeomScreen::Show();
    }
}

//==== Update Pod Screen ====//
bool ClearanceScreen::Update()
{
    assert( m_ScreenMgr );

    Geom* geom_ptr = m_ScreenMgr->GetCurrGeom();
    if ( !geom_ptr || geom_ptr->GetType().m_Type != CLEARANCE_GEOM_TYPE )
    {
        Hide();
        return false;
    }

    GeomScreen::Update();

    m_SymmLayout.GetGroup()->deactivate();
    m_AttachLayout.GetGroup()->deactivate();

    //==== Update Clearance Specific Parms ====//
    ClearanceGeom* clearance_ptr = dynamic_cast< ClearanceGeom* >( geom_ptr );
    assert( clearance_ptr );


    return true;
}


//==== Non Menu Callbacks ====//
void ClearanceScreen::CallBack( Fl_Widget *w )
{
    GeomScreen::CallBack( w );
}






