//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#include "MdcInletScreen.h"
#include "ScreenMgr.h"

//==== Constructor ====//
MdcInletScreen::MdcInletScreen( ScreenMgr* mgr ) : GeomScreen( mgr, 400, 800, "MDC_INLET" )
{
    Fl_Group* design_tab = AddTab( "Design" );
    Fl_Group* design_group = AddSubGroup( design_tab, 5 );

    m_DesignLayout.SetGroupAndScreen( design_group, this );

    // Choices' labels as wide as sliders', so they line up.
    m_DesignLayout.SetChoiceButtonWidth( m_DesignLayout.GetButtonWidth() );

    m_DesignLayout.AddYGap();
    m_DesignLayout.AddDividerBox( "Inlet" );
    m_DesignLayout.AddChoice( m_InTypeChoice, "INTYPE" );
    m_InTypeChoice.AddItem( "2DTOP", MdcInletGeom::TOP2D );
    m_InTypeChoice.AddItem( "2DSIDE", MdcInletGeom::SIDE2D );
    m_InTypeChoice.AddItem( "AXI", MdcInletGeom::AXI );
    m_InTypeChoice.UpdateItems();
    m_DesignLayout.AddSlider( m_XInltSlider, "XINLT", 10, "%7.3f" );
    m_DesignLayout.AddSlider( m_RBodySlider, "RBODY", 10, "%7.3f" );
    m_DesignLayout.AddButton( m_CoverToggle, "COVER" );
    m_DesignLayout.AddSlider( m_RampSlider, "RAMP", 10, "%7.3f" );

    m_DesignLayout.AddYGap();
    m_DesignLayout.AddDividerBox( "Diverter" );
    m_DesignLayout.AddSlider( m_HDivSlider, "HDIV", 10, "%7.3f" );
    m_DesignLayout.AddSlider( m_XDivSlider, "XDIV", 10, "%7.3f" );
    m_DesignLayout.AddSlider( m_LDivSlider, "LDIV", 10, "%7.3f" );

    m_DesignLayout.AddYGap();
    m_DesignLayout.AddDividerBox( "Inlets" );
    m_DesignLayout.AddSlider( m_NInSlider, "NIN", 20, "%5.0f" );
    m_DesignLayout.AddButton( m_PhiFlagToggle, "PHI given" );
    m_DesignLayout.AddIndexSelector( m_InletSelector );
    m_DesignLayout.AddSlider( m_PhiSlider, "PHI", 360.0, "%7.3f" );

    for ( int i = 1; i <= MdcInletGeom::NUM_STN; i++ )
    {
        m_DesignLayout.AddYGap();
        m_DesignLayout.AddDividerBox( "Station " + to_string( i ) );
        if ( i > 1 )
        {
            m_DesignLayout.AddSlider( m_XSlider[i - 2], ( "X(" + to_string( i ) + ")" ).c_str(), 10, "%7.3f" );
            m_DesignLayout.AddSlider( m_HSlider[i - 2], ( "H(" + to_string( i ) + ")" ).c_str(), 10, "%7.3f" );
        }
        m_DesignLayout.AddSlider( m_WSlider[i - 1], ( "W(" + to_string( i ) + ")" ).c_str(), 10, "%7.3f" );
    }
}

//==== Show Screen ====//
void MdcInletScreen::Show()
{
    if ( Update() )
    {
        m_FLTK_Window->show();
    }
}

//==== Update Screen ====//
bool MdcInletScreen::Update()
{
    assert( m_ScreenMgr );

    Geom* geom_ptr = m_ScreenMgr->GetCurrGeom();
    if ( !geom_ptr || geom_ptr->GetType().m_Type != MDC_INLET_GEOM_TYPE )
    {
        Hide();
        return false;
    }

    GeomScreen::Update();

    MdcInletGeom* inlet = dynamic_cast< MdcInletGeom* >( geom_ptr );
    assert( inlet );

    m_InTypeChoice.Update( inlet->m_InType.GetID() );
    m_XInltSlider.Update( inlet->m_XRelLoc.GetID() );
    m_RBodySlider.Update( inlet->m_RBody.GetID() );
    m_CoverToggle.Update( inlet->m_Cover.GetID() );
    m_RampSlider.Update( inlet->m_Ramp.GetID() );

    m_HDivSlider.Update( inlet->m_HDiv.GetID() );
    m_XDivSlider.Update( inlet->m_XDiv.GetID() );
    m_LDivSlider.Update( inlet->m_LDiv.GetID() );

    m_NInSlider.Update( inlet->m_NIn.GetID() );
    m_PhiFlagToggle.Update( inlet->m_PhiFlag.GetID() );
    m_InletSelector.Update( inlet->m_Inlet.GetID() );

    // PHI of the inlet the selector shows, given only if PHI is.
    m_PhiSlider.Update( XSecSurfGeom::ParmIDAt( inlet->m_Phi, inlet->m_Inlet() - 1 ) );
    if ( !inlet->m_PhiFlag() )
    {
        m_PhiSlider.Deactivate();
    }

    for ( int i = 0; i < MdcInletGeom::NUM_STN - 1; i++ )
    {
        m_XSlider[i].Update( inlet->m_X[i].GetID() );
        m_HSlider[i].Update( inlet->m_H[i].GetID() );
        if ( inlet->m_InType() == MdcInletGeom::AXI )
        {
            m_HSlider[i].Deactivate();
        }
    }
    for ( int i = 0; i < MdcInletGeom::NUM_STN; i++ )
    {
        m_WSlider[i].Update( inlet->m_W[i].GetID() );
    }

    // Deactivate the inputs that shape nothing: RAMP for a covered inlet and the diverter's for an HDIV that is
    // not above zero.
    if ( inlet->m_Cover() )
    {
        m_RampSlider.Deactivate();
    }
    if ( inlet->m_HDiv() <= 0.0 )
    {
        m_XDivSlider.Deactivate();
        m_LDivSlider.Deactivate();
    }

    return true;
}

//==== Non Menu Callbacks ====//
void MdcInletScreen::CallBack( Fl_Widget *w )
{
    GeomScreen::CallBack( w );
}
