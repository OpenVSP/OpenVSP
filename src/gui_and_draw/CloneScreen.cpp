//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#include "CloneScreen.h"
#include "ScreenMgr.h"
#include "CloneGeom.h"


//==== Constructor ====//
CloneScreen::CloneScreen( ScreenMgr* mgr ) : GeomScreen( mgr, 400, 800, "Clone" )
{
    Fl_Group* clone_tab = AddTab( "Clone" );
    Fl_Group* clone_group = AddSubGroup( clone_tab, 5 );

    m_CloneLayout.SetGroupAndScreen( clone_group, this );

    m_CloneLayout.AddDividerBox( "Original" );
    m_CloneLayout.AddYGap();

}


//==== Show Clone Screen ====//
void CloneScreen::Show()
{
    if ( Update() )
    {
        GeomScreen::Show();
    }
}

//==== Update Clone Screen ====//
bool CloneScreen::Update()
{
    assert( m_ScreenMgr );

    Geom* geom_ptr = m_ScreenMgr->GetCurrGeom();
    if ( !geom_ptr || geom_ptr->GetType().m_Type != CLONE_GEOM_TYPE )
    {
        Hide();
        return false;
    }

    GeomScreen::Update();

    return true;
}


//==== Non Menu Callbacks ====//
void CloneScreen::CallBack( Fl_Widget *w )
{
    GeomScreen::CallBack( w );
}




