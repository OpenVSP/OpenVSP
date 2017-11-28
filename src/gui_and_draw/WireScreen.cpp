//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#include "WireScreen.h"
#include "WireGeom.h"
#include "ScreenMgr.h"
#include "ParmMgr.h"


//==== Constructor ====//
WireScreen::WireScreen( ScreenMgr* mgr ) : GeomScreen( mgr, 300, 525, "Wireframe" )
{

    RemoveTab( GetTab( m_SubSurfTab_ind ) );

    Fl_Group* wire_tab = AddTab( "WireFrame" );
    Fl_Group* wire_group = AddSubGroup( wire_tab, 5 );

    m_WireLayout.SetGroupAndScreen( wire_group, this );

    m_WireLayout.AddDividerBox( "Wireframe Characteristics" );

    m_TypeChoice.AddItem( "Lifting" );
    m_TypeChoice.AddItem( "Non-Lifting" );
    m_WireLayout.AddChoice( m_TypeChoice, "Type" );

    m_WireLayout.AddYGap();
    m_WireLayout.AddDividerBox( "Normal Vector" );
    m_WireLayout.AddButton( m_InvertButton, "Invert" );

    m_WireLayout.AddYGap();
    m_WireLayout.AddDividerBox( "Swap Point Ordering" );
    m_WireLayout.AddButton( m_SwapIJButton, "Swap I/J" );

    m_WireLayout.AddYGap();
    m_WireLayout.AddDividerBox( "Reverse" );

    m_WireLayout.AddButton( m_RevIButton, "Reverse I" );
    m_WireLayout.AddButton( m_RevJButton, "Reverse J" );

    m_WireLayout.AddYGap();
    m_WireLayout.AddDividerBox( "Start/End Skip" );

    m_WireLayout.AddIndexSelector( m_ISkipStartIndexSelector, "I Start Skip" );
    m_WireLayout.AddIndexSelector( m_ISkipEndIndexSelector, "I End Skip" );
    m_WireLayout.AddIndexSelector( m_JSkipStartIndexSelector, "J Start Skip" );
    m_WireLayout.AddIndexSelector( m_JSkipEndIndexSelector, "J End Skip" );

    m_WireLayout.AddYGap();
    m_WireLayout.AddDividerBox( "Stride" );

    m_WireLayout.AddIndexSelector( m_IStrideIndexSelector, "I Stride" );
    m_WireLayout.AddIndexSelector( m_JStrideIndexSelector, "J Stride" );
}


//==== Show Blank Screen ====//
void WireScreen::Show()
{
    if ( Update() )
    {
        m_FLTK_Window->show();
    }
}

bool WireScreen::Update()
{
    assert( m_ScreenMgr );

    Geom* geom_ptr = m_ScreenMgr->GetCurrGeom();
    if ( !geom_ptr || geom_ptr->GetType().m_Type != WIRE_FRAME_GEOM_TYPE )
    {
        Hide();
        return false;
    }

    GeomScreen::Update();

    //==== Update Point Cloud Specific Parms ====//
    WireGeom* wire_geom_ptr = dynamic_cast< WireGeom* >( geom_ptr );
    assert( wire_geom_ptr );

    m_TypeChoice.Update( wire_geom_ptr->m_WireType.GetID() );
    m_InvertButton.Update( wire_geom_ptr->m_InvertFlag.GetID() );
    m_SwapIJButton.Update( wire_geom_ptr->m_SwapIJFlag.GetID() );
    m_RevIButton.Update( wire_geom_ptr->m_RevIFlag.GetID() );
    m_RevJButton.Update( wire_geom_ptr->m_RevJFlag.GetID() );

    m_ISkipStartIndexSelector.Update( wire_geom_ptr->m_ISkipStart.GetID() );
    m_ISkipEndIndexSelector.Update( wire_geom_ptr->m_ISkipEnd.GetID() );
    m_JSkipStartIndexSelector.Update( wire_geom_ptr->m_JSkipStart.GetID() );
    m_JSkipEndIndexSelector.Update( wire_geom_ptr->m_JSkipEnd.GetID() );

    m_IStrideIndexSelector.Update( wire_geom_ptr->m_IStride.GetID() );
    m_JStrideIndexSelector.Update( wire_geom_ptr->m_JStride.GetID() );

    return true;
}

//==== Non Menu Callbacks ====//
void WireScreen::CallBack( Fl_Widget *w )
{
    GeomScreen::CallBack( w );
}
