//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#include "BlankScreen.h"
#include "BlankGeom.h"
#include "ScreenMgr.h"


//==== Constructor ====//
BlankScreen::BlankScreen( ScreenMgr* mgr ) : GeomScreen( mgr, 400, 800, "Blank", string(""), false )
{

}


//==== Show Blank Screen ====//
void BlankScreen::Show()
{
    if ( Update() )
    {
        GeomScreen::Show();
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

    return true;
}

//==== Non Menu Callbacks ====//
void BlankScreen::CallBack( Fl_Widget *w )
{
    GeomScreen::CallBack( w );
}
