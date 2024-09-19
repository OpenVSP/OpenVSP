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

    m_ClearanceModeChoice.AddItem( "Rotor Tip Path", vsp::CLEARANCE_ROTOR_TIP_PATH );
    m_ClearanceModeChoice.AddItem( "Rotor Burst", vsp::CLEARANCE_ROTOR_BURST );
    m_ClearanceModeChoice.AddItem( "Rotor 1/3 Fragment", vsp::CLEARANCE_ROTOR_BURST );
    m_ClearanceModeChoice.AddItem( "Rotor Intermediate Fragment", vsp::CLEARANCE_ROTOR_BURST );
    m_ClearanceModeChoice.AddItem( "Tire Spray Cone", vsp::CLEARANCE_ROTOR_BURST );
    m_ClearanceModeChoice.AddItem( "Pilot Vision", vsp::CLEARANCE_ROTOR_BURST );
    m_ClearanceModeChoice.AddItem( "Conical Field Of View", vsp::CLEARANCE_ROTOR_BURST );
    m_ClearanceModeChoice.UpdateItems();

    m_DesignLayout.AddChoice( m_ClearanceModeChoice, "Mode" );

    m_DesignLayout.AddButton( m_AutoDiamToggleButton, "Automatic Diameter" );

    m_DesignLayout.AddSlider( m_DiameterSlider, "Diameter", 1.0, "%5.4f" );

    m_DesignLayout.AddSlider( m_FlapRadiusFractSlider, "r_flap/R", 1.0, "%5.4f" );

    m_DesignLayout.AddSlider( m_RootLengthSlider, "Root Length", 1.0, "%5.4f" );

    m_DesignLayout.AddSlider( m_RootOffsetSlider, "Root Offset", 1.0, "%5.4f" );

    m_DesignLayout.AddSlider( m_ThetaThrustSlider, "Theta Thrust", 1.0, "%5.4f" );

    m_DesignLayout.AddSlider( m_ThetaAntiThrustSlider, "Theta Anti Thrust", 1.0, "%5.4f" );

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


    m_ClearanceModeChoice.Update( clearance_ptr->m_ClearanceMode.GetID() );

    m_AutoDiamToggleButton.Update( clearance_ptr->m_AutoDiam.GetID() );

    m_DiameterSlider.Update( clearance_ptr->m_Diameter.GetID() );

    m_FlapRadiusFractSlider.Update( clearance_ptr->m_FlapRadiusFract.GetID() );

    m_RootLengthSlider.Update( clearance_ptr->m_RootLength.GetID() );

    m_RootOffsetSlider.Update( clearance_ptr->m_RootOffset.GetID() );

    m_ThetaThrustSlider.Update( clearance_ptr->m_ThetaThrust.GetID() );

    m_ThetaAntiThrustSlider.Update( clearance_ptr->m_ThetaAntiThrust.GetID() );

    return true;
}


//==== Non Menu Callbacks ====//
void ClearanceScreen::CallBack( Fl_Widget *w )
{
    GeomScreen::CallBack( w );
}






