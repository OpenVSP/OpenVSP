//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#include "ConformalScreen.h"
#include "ScreenMgr.h"
#include "ConformalGeom.h"
#include "Vehicle.h"
#include "ParmMgr.h"

#include <assert.h>


//==== Constructor ====//
ConformalScreen::ConformalScreen( ScreenMgr* mgr ) : GeomScreen( mgr, 300, 625, "Conformal" )
{
    Fl_Group* design_tab = AddTab( "Design" );
    Fl_Group* design_group = AddSubGroup( design_tab, 5 );

    m_DesignLayout.SetGroupAndScreen( design_group, this );
    m_DesignLayout.AddDividerBox( "Design" );

    //==== Design ====//
    m_DesignLayout.AddSlider( m_OffsetSlider, "Offset", 0.1, "%7.3f" );
    m_DesignLayout.AddYGap();

    m_DesignLayout.AddDividerBox("UV Trim");

    m_DesignLayout.AddSubGroupLayout( m_TrimGroup, m_DesignLayout.GetW(), m_DesignLayout.GetH() );

    int start_y = m_TrimGroup.GetY();
    m_TrimGroup.AddButton( m_UTrimToggle, "Trim U");
    m_TrimGroup.AddSlider( m_UTrimMinSlider, "U Min", 1.0, "%5.4f" );
    m_TrimGroup.AddSlider( m_UTrimMaxSlider, "U Max", 1.0, "%5.4f" );
    m_TrimGroup.AddYGap();

    m_TrimGroup.AddButton(m_V1TrimToggle, "Trim V1");
    m_TrimGroup.AddSlider( m_V1TrimMinSlider, "V1 Begin", 1.0, "%5.4f" );
    m_TrimGroup.AddSlider( m_V1TrimMaxSlider, "V1 End", 1.0, "%5.4f" );
    m_TrimGroup.AddYGap();
    m_TrimGroup.AddButton(m_V2TrimToggle, "Trim V2");
    m_TrimGroup.AddSlider( m_V2TrimMinSlider, "V2 Begin", 1.0, "%5.4f" );
    m_TrimGroup.AddSlider( m_V2TrimMaxSlider, "V2 End", 1.0, "%5.4f" );
    m_TrimGroup.AddYGap();
    int offset_y = m_TrimGroup.GetY() - start_y;

    m_DesignLayout.AddY( offset_y );

    m_DesignLayout.AddSubGroupLayout( m_WingGroup, m_DesignLayout.GetW(), 200 );
    m_WingGroup.AddYGap();
    m_WingGroup.AddDividerBox("Wing Trim");

    //m_WingGroup.AddButton( m_SpanTrimToggle, "Trim Span");
    //m_WingGroup.AddSlider( m_SpanTrimMinSlider, "Span Min", 1.0, "%5.4f" );
    //m_WingGroup.AddSlider( m_SpanTrimMaxSlider, "Span Max", 1.0, "%5.4f" );
    //m_WingGroup.AddYGap();

    m_WingGroup.AddButton( m_ChordTrimToggle, "Trim Chord");
    m_WingGroup.AddSlider( m_ChordTrimMinSlider, "Chord Min", 1.0, "%5.4f" );
    m_WingGroup.AddSlider( m_ChordTrimMaxSlider, "Chord Max", 1.0, "%5.4f" );
    m_WingGroup.AddYGap();

}


//==== Show Pod Screen ====//
void ConformalScreen::Show()
{
    if ( Update() )
    {
        m_FLTK_Window->show();
    }
}

//==== Update Pod Screen ====//
bool ConformalScreen::Update()
{
    assert( m_ScreenMgr );

    Geom* geom_ptr = m_ScreenMgr->GetCurrGeom();
    if ( !geom_ptr || geom_ptr->GetType().m_Type != CONFORMAL_GEOM_TYPE )
    {
        Hide();
        return false;
    }

    GeomScreen::Update();

    m_SymmLayout.GetGroup()->deactivate();
    m_AttachLayout.GetGroup()->deactivate();

    //==== Update Conformal Specific Parms ====//
    ConformalGeom* conformal_ptr = dynamic_cast< ConformalGeom* >( geom_ptr );
    assert( conformal_ptr );

    m_OffsetSlider.Update( conformal_ptr->m_Offset.GetID() );

    m_UTrimToggle.Update( conformal_ptr->m_UTrimFlag.GetID() );
    m_UTrimMinSlider.Update( conformal_ptr->m_UTrimMin.GetID() );
    m_UTrimMaxSlider.Update( conformal_ptr->m_UTrimMax.GetID() );

    m_V1TrimToggle.Update( conformal_ptr->m_V1TrimFlag.GetID() );
    m_V1TrimMinSlider.Update( conformal_ptr->m_V1TrimBegin.GetID() );
    m_V1TrimMaxSlider.Update( conformal_ptr->m_V1TrimEnd.GetID() );

    m_V2TrimToggle.Update( conformal_ptr->m_V2TrimFlag.GetID() );
    m_V2TrimMinSlider.Update( conformal_ptr->m_V2TrimBegin.GetID() );
    m_V2TrimMaxSlider.Update( conformal_ptr->m_V2TrimEnd.GetID() );

    if ( !conformal_ptr->IsWingParent() )
    {
        m_WingGroup.Hide();
    }
    else
    {
        m_WingGroup.Show();
        m_ChordTrimToggle.Update( conformal_ptr->m_ChordTrimFlag.GetID() );
        m_ChordTrimMinSlider.Update( conformal_ptr->m_ChordTrimMin.GetID() );
        m_ChordTrimMaxSlider.Update( conformal_ptr->m_ChordTrimMax.GetID() );

         if ( conformal_ptr->m_ChordTrimFlag() )
        {
            m_V1TrimToggle.Deactivate();
            m_V1TrimMinSlider.Deactivate();
            m_V1TrimMaxSlider.Deactivate();
            m_V2TrimToggle.Deactivate();
            m_V2TrimMinSlider.Deactivate();
            m_V2TrimMaxSlider.Deactivate();
        }

    }

    return true;
}


//==== Non Menu Callbacks ====//
void ConformalScreen::CallBack( Fl_Widget *w )
{
    GeomScreen::CallBack( w );
}






