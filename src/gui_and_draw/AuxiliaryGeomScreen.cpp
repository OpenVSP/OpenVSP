//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#include "AuxiliaryGeomScreen.h"
#include "ScreenMgr.h"
#include "AuxiliaryGeom.h"
#include "APIDefines.h"
#include "Display.h"
#include "GearGeom.h"
#include "StlHelper.h"

//==== Constructor ====//
AuxiliaryGeomScreen::AuxiliaryGeomScreen( ScreenMgr* mgr ) : GeomScreen( mgr, 400, 657 + 25, "Auxiliary" )
{
    Fl_Group* design_tab = AddTab( "Design" );
    Fl_Group* design_group = AddSubGroup( design_tab, 5 );

    m_DesignLayout.SetGroupAndScreen( design_group, this );
    m_DesignLayout.AddDividerBox( "Design" );

    //==== Design ====//

    m_AuxiliaryGeomModeChoice.AddItem( "Rotor Tip Path", vsp::AUX_GEOM_ROTOR_TIP_PATH );
    m_AuxiliaryGeomModeChoice.AddItem( "Rotor Burst", vsp::AUX_GEOM_ROTOR_BURST );
    m_AuxiliaryGeomModeChoice.AddItem( "3pt Ground Plane", vsp::AUX_GEOM_THREE_PT_GROUND );
    m_AuxiliaryGeomModeChoice.AddItem( "2pt Ground Plane", vsp::AUX_GEOM_TWO_PT_GROUND );
    m_AuxiliaryGeomModeChoice.AddItem( "1pt Ground Plane", vsp::AUX_GEOM_ONE_PT_GROUND );
    m_AuxiliaryGeomModeChoice.AddItem( "3pt Composite Clearance Envelope", vsp::AUX_GEOM_THREE_PT_CCE );
    m_AuxiliaryGeomModeChoice.AddItem( "Rotor 1/3 Fragment", vsp::AUX_GEOM_ROTOR_BURST );
    m_AuxiliaryGeomModeChoice.AddItem( "Rotor Intermediate Fragment", vsp::AUX_GEOM_ROTOR_BURST );
    m_AuxiliaryGeomModeChoice.AddItem( "Tire Spray Cone", vsp::AUX_GEOM_ROTOR_BURST );
    m_AuxiliaryGeomModeChoice.AddItem( "Pilot Vision", vsp::AUX_GEOM_ROTOR_BURST );
    m_AuxiliaryGeomModeChoice.AddItem( "Conical Field Of View", vsp::AUX_GEOM_ROTOR_BURST );
    m_AuxiliaryGeomModeChoice.UpdateItems();

    m_DesignLayout.AddChoice( m_AuxiliaryGeomModeChoice, "Mode" );

    m_DesignLayout.AddYGap();

    m_DesignLayout.AddSubGroupLayout( m_RotorTipPathLayput, m_DesignLayout.GetW(), m_DesignLayout.GetRemainY() );
    m_DesignLayout.AddSubGroupLayout( m_RotorBurstLayout, m_DesignLayout.GetW(), m_DesignLayout.GetRemainY() );
    m_DesignLayout.AddSubGroupLayout( m_3ptGroundPlaneLayout, m_DesignLayout.GetW(), m_DesignLayout.GetRemainY() );
    m_DesignLayout.AddSubGroupLayout( m_2ptGroundPlaneLayout, m_DesignLayout.GetW(), m_DesignLayout.GetRemainY() );
    m_DesignLayout.AddSubGroupLayout( m_1ptGroundPlaneLayout, m_DesignLayout.GetW(), m_DesignLayout.GetRemainY() );
    m_DesignLayout.AddSubGroupLayout( m_3ptCCELayout, m_DesignLayout.GetW(), m_DesignLayout.GetRemainY() );

    m_RotorTipPathLayput.AddButton( m_RTP_AutoDiamToggleButton, "Automatic Diameter" );
    m_RotorTipPathLayput.AddSlider( m_RTP_DiameterSlider, "Diameter", 1.0, "%5.4f" );
    m_RotorTipPathLayput.AddSlider( m_RTP_FlapRadiusFractSlider, "r_flap/R", 1.0, "%5.4f" );
    m_RotorTipPathLayput.AddSlider( m_RTP_ThetaThrustSlider, "Theta Thrust", 1.0, "%5.4f" );
    m_RotorTipPathLayput.AddSlider( m_RTP_ThetaAntiThrustSlider, "Theta Anti Thrust", 1.0, "%5.4f" );


    m_RotorBurstLayout.AddButton( m_RB_AutoDiamToggleButton, "Automatic Diameter" );
    m_RotorBurstLayout.AddSlider( m_RB_DiameterSlider, "Diameter", 1.0, "%5.4f" );
    m_RotorBurstLayout.AddSlider( m_RB_FlapRadiusFractSlider, "r_flap/R", 1.0, "%5.4f" );
    m_RotorBurstLayout.AddSlider( m_RB_ThetaThrustSlider, "Theta Thrust", 1.0, "%5.4f" );
    m_RotorBurstLayout.AddSlider( m_RB_ThetaAntiThrustSlider, "Theta Anti Thrust", 1.0, "%5.4f" );
    m_RotorBurstLayout.AddSlider( m_RB_RootLengthSlider, "Root Length", 1.0, "%5.4f" );
    m_RotorBurstLayout.AddSlider( m_RB_RootOffsetSlider, "Root Offset", 1.0, "%5.4f" );



    m_3ptBogie1SuspensionModeChoice.AddItem( "Nominal", vsp::GEAR_SUSPENSION_NOMINAL );
    m_3ptBogie1SuspensionModeChoice.AddItem( "Compressed", vsp::GEAR_SUSPENSION_COMPRESSED );
    m_3ptBogie1SuspensionModeChoice.AddItem( "Extended", vsp::GEAR_SUSPENSION_EXTENDED );
    m_3ptBogie1SuspensionModeChoice.UpdateItems();

    m_3ptBogie1TireModeChoice.AddItem( "Static Load", vsp::TIRE_STATIC_LODED_CONTACT );
    m_3ptBogie1TireModeChoice.AddItem( "Unloaded", vsp::TIRE_NOMINAL_CONTACT );
    m_3ptBogie1TireModeChoice.AddItem( "Grown", vsp::TIRE_GROWTH_CONTACT );
    m_3ptBogie1TireModeChoice.AddItem( "Flat", vsp::TIRE_FLAT_CONTACT );
    m_3ptBogie1TireModeChoice.UpdateItems();

    m_3ptGroundPlaneLayout.AddChoice( m_3ptBogie1Choice, "Bogie 1" );
    m_3ptGroundPlaneLayout.AddChoice( m_3ptBogie1SymmChoice, "I Symm" );
    m_3ptGroundPlaneLayout.AddChoice( m_3ptBogie1SuspensionModeChoice, "Suspension Mode" );
    m_3ptGroundPlaneLayout.AddChoice( m_3ptBogie1TireModeChoice, "Tire Mode" );

    m_3ptGroundPlaneLayout.AddYGap();

    m_3ptBogie2SuspensionModeChoice.AddItem( "Nominal", vsp::GEAR_SUSPENSION_NOMINAL );
    m_3ptBogie2SuspensionModeChoice.AddItem( "Compressed", vsp::GEAR_SUSPENSION_COMPRESSED );
    m_3ptBogie2SuspensionModeChoice.AddItem( "Extended", vsp::GEAR_SUSPENSION_EXTENDED );
    m_3ptBogie2SuspensionModeChoice.UpdateItems();

    m_3ptBogie2TireModeChoice.AddItem( "Static Load", vsp::TIRE_STATIC_LODED_CONTACT );
    m_3ptBogie2TireModeChoice.AddItem( "Unloaded", vsp::TIRE_NOMINAL_CONTACT );
    m_3ptBogie2TireModeChoice.AddItem( "Grown", vsp::TIRE_GROWTH_CONTACT );
    m_3ptBogie2TireModeChoice.AddItem( "Flat", vsp::TIRE_FLAT_CONTACT );
    m_3ptBogie2TireModeChoice.UpdateItems();

    m_3ptGroundPlaneLayout.AddChoice( m_3ptBogie2Choice, "Bogie 2" );
    m_3ptGroundPlaneLayout.AddChoice( m_3ptBogie2SymmChoice, "I Symm" );
    m_3ptGroundPlaneLayout.AddChoice( m_3ptBogie2SuspensionModeChoice, "Suspension Mode" );
    m_3ptGroundPlaneLayout.AddChoice( m_3ptBogie2TireModeChoice, "Tire Mode" );

    m_3ptGroundPlaneLayout.AddYGap();

    m_3ptBogie3SuspensionModeChoice.AddItem( "Nominal", vsp::GEAR_SUSPENSION_NOMINAL );
    m_3ptBogie3SuspensionModeChoice.AddItem( "Compressed", vsp::GEAR_SUSPENSION_COMPRESSED );
    m_3ptBogie3SuspensionModeChoice.AddItem( "Extended", vsp::GEAR_SUSPENSION_EXTENDED );
    m_3ptBogie3SuspensionModeChoice.UpdateItems();

    m_3ptBogie3TireModeChoice.AddItem( "Static Load", vsp::TIRE_STATIC_LODED_CONTACT );
    m_3ptBogie3TireModeChoice.AddItem( "Unloaded", vsp::TIRE_NOMINAL_CONTACT );
    m_3ptBogie3TireModeChoice.AddItem( "Grown", vsp::TIRE_GROWTH_CONTACT );
    m_3ptBogie3TireModeChoice.AddItem( "Flat", vsp::TIRE_FLAT_CONTACT );
    m_3ptBogie3TireModeChoice.UpdateItems();

    m_3ptGroundPlaneLayout.AddChoice( m_3ptBogie3Choice, "Bogie 3" );
    m_3ptGroundPlaneLayout.AddChoice( m_3ptBogie3SymmChoice, "I Symm" );
    m_3ptGroundPlaneLayout.AddChoice( m_3ptBogie3SuspensionModeChoice, "Suspension Mode" );
    m_3ptGroundPlaneLayout.AddChoice( m_3ptBogie3TireModeChoice, "Tire Mode" );


    m_2ptBogie1SuspensionModeChoice.AddItem( "Nominal", vsp::GEAR_SUSPENSION_NOMINAL );
    m_2ptBogie1SuspensionModeChoice.AddItem( "Compressed", vsp::GEAR_SUSPENSION_COMPRESSED );
    m_2ptBogie1SuspensionModeChoice.AddItem( "Extended", vsp::GEAR_SUSPENSION_EXTENDED );
    m_2ptBogie1SuspensionModeChoice.UpdateItems();

    m_2ptBogie1TireModeChoice.AddItem( "Static Load", vsp::TIRE_STATIC_LODED_CONTACT );
    m_2ptBogie1TireModeChoice.AddItem( "Unloaded", vsp::TIRE_NOMINAL_CONTACT );
    m_2ptBogie1TireModeChoice.AddItem( "Grown", vsp::TIRE_GROWTH_CONTACT );
    m_2ptBogie1TireModeChoice.AddItem( "Flat", vsp::TIRE_FLAT_CONTACT );
    m_2ptBogie1TireModeChoice.UpdateItems();

    m_2ptGroundPlaneLayout.AddChoice( m_2ptBogie1Choice, "Bogie 1" );
    m_2ptGroundPlaneLayout.AddChoice( m_2ptBogie1SymmChoice, "I Symm" );
    m_2ptGroundPlaneLayout.AddChoice( m_2ptBogie1SuspensionModeChoice, "Suspension Mode" );
    m_2ptGroundPlaneLayout.AddChoice( m_2ptBogie1TireModeChoice, "Tire Mode" );

    m_2ptGroundPlaneLayout.AddYGap();

    m_2ptBogie2SuspensionModeChoice.AddItem( "Nominal", vsp::GEAR_SUSPENSION_NOMINAL );
    m_2ptBogie2SuspensionModeChoice.AddItem( "Compressed", vsp::GEAR_SUSPENSION_COMPRESSED );
    m_2ptBogie2SuspensionModeChoice.AddItem( "Extended", vsp::GEAR_SUSPENSION_EXTENDED );
    m_2ptBogie2SuspensionModeChoice.UpdateItems();

    m_2ptBogie2TireModeChoice.AddItem( "Static Load", vsp::TIRE_STATIC_LODED_CONTACT );
    m_2ptBogie2TireModeChoice.AddItem( "Unloaded", vsp::TIRE_NOMINAL_CONTACT );
    m_2ptBogie2TireModeChoice.AddItem( "Grown", vsp::TIRE_GROWTH_CONTACT );
    m_2ptBogie2TireModeChoice.AddItem( "Flat", vsp::TIRE_FLAT_CONTACT );
    m_2ptBogie2TireModeChoice.UpdateItems();

    m_2ptGroundPlaneLayout.AddChoice( m_2ptBogie2Choice, "Bogie 2" );
    m_2ptGroundPlaneLayout.AddChoice( m_2ptBogie2SymmChoice, "I Symm" );
    m_2ptGroundPlaneLayout.AddChoice( m_2ptBogie2SuspensionModeChoice, "Suspension Mode" );
    m_2ptGroundPlaneLayout.AddChoice( m_2ptBogie2TireModeChoice, "Tire Mode" );

    m_2ptGroundPlaneLayout.AddSlider( m_2ptBogieThetaSlider, "Bogie Theta", 10, "%5.4f" );
    m_2ptGroundPlaneLayout.AddSlider( m_2ptWheelThetaSlider, "Wheel Theta", 10, "%5.4f" );

    m_1ptBogie1SuspensionModeChoice.AddItem( "Nominal", vsp::GEAR_SUSPENSION_NOMINAL );
    m_1ptBogie1SuspensionModeChoice.AddItem( "Compressed", vsp::GEAR_SUSPENSION_COMPRESSED );
    m_1ptBogie1SuspensionModeChoice.AddItem( "Extended", vsp::GEAR_SUSPENSION_EXTENDED );
    m_1ptBogie1SuspensionModeChoice.UpdateItems();

    m_1ptBogie1TireModeChoice.AddItem( "Static Load", vsp::TIRE_STATIC_LODED_CONTACT );
    m_1ptBogie1TireModeChoice.AddItem( "Unloaded", vsp::TIRE_NOMINAL_CONTACT );
    m_1ptBogie1TireModeChoice.AddItem( "Grown", vsp::TIRE_GROWTH_CONTACT );
    m_1ptBogie1TireModeChoice.AddItem( "Flat", vsp::TIRE_FLAT_CONTACT );
    m_1ptBogie1TireModeChoice.UpdateItems();

    m_1ptGroundPlaneLayout.AddChoice( m_1ptBogie1Choice, "Bogie 1" );
    m_1ptGroundPlaneLayout.AddChoice( m_1ptBogie1SymmChoice, "I Symm" );
    m_1ptGroundPlaneLayout.AddChoice( m_1ptBogie1SuspensionModeChoice, "Suspension Mode" );
    m_1ptGroundPlaneLayout.AddChoice( m_1ptBogie1TireModeChoice, "Tire Mode" );

    m_1ptGroundPlaneLayout.AddSlider( m_1ptBogieThetaSlider, "Bogie Theta", 10, "%5.4f" );
    m_1ptGroundPlaneLayout.AddSlider( m_1ptWheelThetaSlider, "Wheel Theta", 10, "%5.4f" );
    m_1ptGroundPlaneLayout.AddSlider( m_1ptRollThetaSlider, "Roll Theta", 10, "%5.4f" );




    m_3ptCCEBogie1SuspensionModeChoice.AddItem( "Nominal", vsp::GEAR_SUSPENSION_NOMINAL );
    m_3ptCCEBogie1SuspensionModeChoice.AddItem( "Compressed", vsp::GEAR_SUSPENSION_COMPRESSED );
    m_3ptCCEBogie1SuspensionModeChoice.AddItem( "Extended", vsp::GEAR_SUSPENSION_EXTENDED );
    m_3ptCCEBogie1SuspensionModeChoice.UpdateItems();

    m_3ptCCEBogie1TireModeChoice.AddItem( "Static Load", vsp::TIRE_STATIC_LODED_CONTACT );
    m_3ptCCEBogie1TireModeChoice.AddItem( "Unloaded", vsp::TIRE_NOMINAL_CONTACT );
    m_3ptCCEBogie1TireModeChoice.AddItem( "Grown", vsp::TIRE_GROWTH_CONTACT );
    m_3ptCCEBogie1TireModeChoice.AddItem( "Flat", vsp::TIRE_FLAT_CONTACT );
    m_3ptCCEBogie1TireModeChoice.UpdateItems();

    m_3ptCCELayout.AddChoice( m_3ptCCEBogie1Choice, "Nose Gear Bogie" );
    m_3ptCCELayout.AddChoice( m_3ptCCEBogie1SymmChoice, "I Symm" );
    m_3ptCCELayout.AddChoice( m_3ptCCEBogie1SuspensionModeChoice, "Suspension Mode" );
    m_3ptCCELayout.AddChoice( m_3ptCCEBogie1TireModeChoice, "Tire Mode" );

    m_3ptCCELayout.AddYGap();

    m_3ptCCEBogie2SuspensionModeChoice.AddItem( "Nominal", vsp::GEAR_SUSPENSION_NOMINAL );
    m_3ptCCEBogie2SuspensionModeChoice.AddItem( "Compressed", vsp::GEAR_SUSPENSION_COMPRESSED );
    m_3ptCCEBogie2SuspensionModeChoice.AddItem( "Extended", vsp::GEAR_SUSPENSION_EXTENDED );
    m_3ptCCEBogie2SuspensionModeChoice.UpdateItems();

    m_3ptCCEBogie2TireModeChoice.AddItem( "Static Load", vsp::TIRE_STATIC_LODED_CONTACT );
    m_3ptCCEBogie2TireModeChoice.AddItem( "Unloaded", vsp::TIRE_NOMINAL_CONTACT );
    m_3ptCCEBogie2TireModeChoice.AddItem( "Grown", vsp::TIRE_GROWTH_CONTACT );
    m_3ptCCEBogie2TireModeChoice.AddItem( "Flat", vsp::TIRE_FLAT_CONTACT );
    m_3ptCCEBogie2TireModeChoice.UpdateItems();

    m_3ptCCELayout.AddChoice( m_3ptCCEBogie2Choice, "Main Bogie 1" );
    m_3ptCCELayout.AddChoice( m_3ptCCEBogie2SymmChoice, "I Symm" );
    m_3ptCCELayout.AddChoice( m_3ptCCEBogie2SuspensionModeChoice, "Suspension Mode" );
    m_3ptCCELayout.AddChoice( m_3ptCCEBogie2TireModeChoice, "Tire Mode" );

    m_3ptCCELayout.AddYGap();

    m_3ptCCEBogie3SuspensionModeChoice.AddItem( "Nominal", vsp::GEAR_SUSPENSION_NOMINAL );
    m_3ptCCEBogie3SuspensionModeChoice.AddItem( "Compressed", vsp::GEAR_SUSPENSION_COMPRESSED );
    m_3ptCCEBogie3SuspensionModeChoice.AddItem( "Extended", vsp::GEAR_SUSPENSION_EXTENDED );
    m_3ptCCEBogie3SuspensionModeChoice.UpdateItems();

    m_3ptCCEBogie3TireModeChoice.AddItem( "Static Load", vsp::TIRE_STATIC_LODED_CONTACT );
    m_3ptCCEBogie3TireModeChoice.AddItem( "Unloaded", vsp::TIRE_NOMINAL_CONTACT );
    m_3ptCCEBogie3TireModeChoice.AddItem( "Grown", vsp::TIRE_GROWTH_CONTACT );
    m_3ptCCEBogie3TireModeChoice.AddItem( "Flat", vsp::TIRE_FLAT_CONTACT );
    m_3ptCCEBogie3TireModeChoice.UpdateItems();

    m_3ptCCELayout.AddChoice( m_3ptCCEBogie3Choice, "Main Bogie 2" );
    m_3ptCCELayout.AddChoice( m_3ptCCEBogie3SymmChoice, "I Symm" );
    m_3ptCCELayout.AddChoice( m_3ptCCEBogie3SuspensionModeChoice, "Suspension Mode" );
    m_3ptCCELayout.AddChoice( m_3ptCCEBogie3TireModeChoice, "Tire Mode" );

    m_3ptCCELayout.AddYGap();

    m_3ptCCELayout.SetSameLineFlag( true );
    m_3ptCCELayout.SetFitWidthFlag( false );
    m_3ptCCELayout.SetButtonWidth( m_3ptCCELayout.GetChoiceButtonWidth() );
    m_3ptCCELayout.AddButton( m_ReadCCEFileButton, "Read File" );

    m_CCEUnitChoice.AddItem( "mm", vsp::LEN_MM );
    m_CCEUnitChoice.AddItem( "cm", vsp::LEN_CM );
    m_CCEUnitChoice.AddItem( "m", vsp::LEN_M );
    m_CCEUnitChoice.AddItem( "in", vsp::LEN_IN );
    m_CCEUnitChoice.AddItem( "ft", vsp::LEN_FT );
    m_CCEUnitChoice.AddItem( "yd", vsp::LEN_YD );
    m_CCEUnitChoice.AddItem( "Consistent", vsp::LEN_UNITLESS );
    m_CCEUnitChoice.UpdateItems();

    m_3ptCCELayout.SetFitWidthFlag( true );
    m_3ptCCELayout.SetChoiceButtonWidth( m_3ptCCELayout.GetW() - m_3ptCCELayout.GetButtonWidth() - 100 );
    m_3ptCCELayout.AddChoice( m_CCEUnitChoice, "Envelope Length Units", m_3ptCCELayout.GetButtonWidth() );
    m_3ptCCELayout.ForceNewLine();

    m_3ptCCELayout.SetSameLineFlag( false );

    m_3ptCCELayout.AddYGap();
    m_3ptCCELayout.AddSlider( m_CCEMainGearOffsetSlider, "Main Gear Offset", 10, "%5.4f" );

    DisplayGroup( nullptr );
}

void AuxiliaryGeomScreen::DisplayGroup( GroupLayout* group )
{
    if ( m_CurrDisplayGroup == group )
    {
        return;
    }

    m_RotorTipPathLayput.Hide();
    m_RotorBurstLayout.Hide();
    m_3ptGroundPlaneLayout.Hide();
    m_2ptGroundPlaneLayout.Hide();
    m_1ptGroundPlaneLayout.Hide();
    m_3ptCCELayout.Hide();

    m_CurrDisplayGroup = group;

    if (group)
    {
        group->Show();
    }
}

//==== Show Pod Screen ====//
void AuxiliaryGeomScreen::Show()
{
    if ( Update() )
    {
        GeomScreen::Show();
    }
}

//==== Update Pod Screen ====//
bool AuxiliaryGeomScreen::Update()
{
    assert( m_ScreenMgr );

    Geom* geom_ptr = m_ScreenMgr->GetCurrGeom();
    if ( !geom_ptr || geom_ptr->GetType().m_Type != AUXILIARY_GEOM_TYPE )
    {
        Hide();
        return false;
    }

    GeomScreen::Update();

    UpdateGroundPlaneChoices();

    m_SymmLayout.GetGroup()->deactivate();
    m_AttachLayout.GetGroup()->deactivate();

    //==== Update Auxiliary Specific Parms ====//
    AuxiliaryGeom* auxiliary_ptr = dynamic_cast< AuxiliaryGeom* >( geom_ptr );
    assert( auxiliary_ptr );

    if ( auxiliary_ptr )
    {
        m_AuxiliaryGeomModeChoice.Update( auxiliary_ptr->m_AuxuliaryGeomMode.GetID() );

        if ( auxiliary_ptr->m_AuxuliaryGeomMode() == vsp::AUX_GEOM_ROTOR_TIP_PATH )
        {
            DisplayGroup( &m_RotorTipPathLayput );

            m_RTP_AutoDiamToggleButton.Update( auxiliary_ptr->m_AutoDiam.GetID() );
            m_RTP_DiameterSlider.Update( auxiliary_ptr->m_Diameter.GetID() );
            m_RTP_FlapRadiusFractSlider.Update( auxiliary_ptr->m_FlapRadiusFract.GetID() );
            m_RTP_ThetaThrustSlider.Update( auxiliary_ptr->m_ThetaThrust.GetID() );
            m_RTP_ThetaAntiThrustSlider.Update( auxiliary_ptr->m_ThetaAntiThrust.GetID() );
        }
        else if ( auxiliary_ptr->m_AuxuliaryGeomMode() == vsp::AUX_GEOM_ROTOR_BURST )
        {
            DisplayGroup( &m_RotorBurstLayout );

            m_RB_AutoDiamToggleButton.Update( auxiliary_ptr->m_AutoDiam.GetID() );
            m_RB_DiameterSlider.Update( auxiliary_ptr->m_Diameter.GetID() );
            m_RB_FlapRadiusFractSlider.Update( auxiliary_ptr->m_FlapRadiusFract.GetID() );
            m_RB_ThetaThrustSlider.Update( auxiliary_ptr->m_ThetaThrust.GetID() );
            m_RB_ThetaAntiThrustSlider.Update( auxiliary_ptr->m_ThetaAntiThrust.GetID() );
            m_RB_RootLengthSlider.Update( auxiliary_ptr->m_RootLength.GetID() );
            m_RB_RootOffsetSlider.Update( auxiliary_ptr->m_RootOffset.GetID() );
        }
        else if ( auxiliary_ptr->m_AuxuliaryGeomMode() == vsp::AUX_GEOM_THREE_PT_GROUND )
        {
            DisplayGroup( &m_3ptGroundPlaneLayout );

            m_3ptBogie1SymmChoice.Update( auxiliary_ptr->m_ContactPt1_Isymm.GetID() );
            m_3ptBogie1SuspensionModeChoice.Update( auxiliary_ptr->m_ContactPt1_SuspensionMode.GetID() );
            m_3ptBogie1TireModeChoice.Update( auxiliary_ptr->m_ContactPt1_TireMode.GetID() );

            m_3ptBogie2SymmChoice.Update( auxiliary_ptr->m_ContactPt2_Isymm.GetID() );
            m_3ptBogie2SuspensionModeChoice.Update( auxiliary_ptr->m_ContactPt2_SuspensionMode.GetID() );
            m_3ptBogie2TireModeChoice.Update( auxiliary_ptr->m_ContactPt2_TireMode.GetID() );

            m_3ptBogie3SymmChoice.Update( auxiliary_ptr->m_ContactPt3_Isymm.GetID() );
            m_3ptBogie3SuspensionModeChoice.Update( auxiliary_ptr->m_ContactPt3_SuspensionMode.GetID() );
            m_3ptBogie3TireModeChoice.Update( auxiliary_ptr->m_ContactPt3_TireMode.GetID() );
        }
        else if ( auxiliary_ptr->m_AuxuliaryGeomMode() == vsp::AUX_GEOM_TWO_PT_GROUND )
        {
            DisplayGroup( &m_2ptGroundPlaneLayout );

            m_2ptBogie1SymmChoice.Update( auxiliary_ptr->m_ContactPt1_Isymm.GetID() );
            m_2ptBogie1SuspensionModeChoice.Update( auxiliary_ptr->m_ContactPt1_SuspensionMode.GetID() );
            m_2ptBogie1TireModeChoice.Update( auxiliary_ptr->m_ContactPt1_TireMode.GetID() );

            m_2ptBogie2SymmChoice.Update( auxiliary_ptr->m_ContactPt2_Isymm.GetID() );
            m_2ptBogie2SuspensionModeChoice.Update( auxiliary_ptr->m_ContactPt2_SuspensionMode.GetID() );
            m_2ptBogie2TireModeChoice.Update( auxiliary_ptr->m_ContactPt2_TireMode.GetID() );

            m_2ptBogieThetaSlider.Update( auxiliary_ptr->m_BogieTheta.GetID() );
            m_2ptWheelThetaSlider.Update( auxiliary_ptr->m_WheelTheta.GetID() );
        }
        else if ( auxiliary_ptr->m_AuxuliaryGeomMode() == vsp::AUX_GEOM_ONE_PT_GROUND )
        {
            DisplayGroup( &m_1ptGroundPlaneLayout );

            m_1ptBogie1SymmChoice.Update( auxiliary_ptr->m_ContactPt1_Isymm.GetID() );
            m_1ptBogie1SuspensionModeChoice.Update( auxiliary_ptr->m_ContactPt1_SuspensionMode.GetID() );
            m_1ptBogie1TireModeChoice.Update( auxiliary_ptr->m_ContactPt1_TireMode.GetID() );

            m_1ptBogieThetaSlider.Update( auxiliary_ptr->m_BogieTheta.GetID() );
            m_1ptWheelThetaSlider.Update( auxiliary_ptr->m_WheelTheta.GetID() );
            m_1ptRollThetaSlider.Update( auxiliary_ptr->m_RollTheta.GetID() );

        }
        else if ( auxiliary_ptr->m_AuxuliaryGeomMode() == vsp::AUX_GEOM_THREE_PT_CCE )
        {
            DisplayGroup( &m_3ptCCELayout );

            m_3ptCCEBogie1SymmChoice.Update( auxiliary_ptr->m_ContactPt1_Isymm.GetID() );
            m_3ptCCEBogie1SuspensionModeChoice.Update( auxiliary_ptr->m_ContactPt1_SuspensionMode.GetID() );
            m_3ptCCEBogie1TireModeChoice.Update( auxiliary_ptr->m_ContactPt1_TireMode.GetID() );

            m_3ptCCEBogie2SymmChoice.Update( auxiliary_ptr->m_ContactPt2_Isymm.GetID() );
            m_3ptCCEBogie2SuspensionModeChoice.Update( auxiliary_ptr->m_ContactPt2_SuspensionMode.GetID() );
            m_3ptCCEBogie2TireModeChoice.Update( auxiliary_ptr->m_ContactPt2_TireMode.GetID() );

            m_3ptCCEBogie3SymmChoice.Update( auxiliary_ptr->m_ContactPt3_Isymm.GetID() );
            m_3ptCCEBogie3SuspensionModeChoice.Update( auxiliary_ptr->m_ContactPt3_SuspensionMode.GetID() );
            m_3ptCCEBogie3TireModeChoice.Update( auxiliary_ptr->m_ContactPt3_TireMode.GetID() );

            m_CCEUnitChoice.Update( auxiliary_ptr->m_CCEUnits.GetID() );

            m_CCEMainGearOffsetSlider.Update( auxiliary_ptr->m_CCEMainGearOffset.GetID() );
        }

    }

    return true;
}

void AuxiliaryGeomScreen::UpdateGroundPlaneChoices()
{
    Vehicle *veh = VehicleMgr.GetVehicle();

    Geom* geom_ptr = m_ScreenMgr->GetCurrGeom();
    if ( !geom_ptr )
    {
        return;
    }

    AuxiliaryGeom* auxiliary_ptr = dynamic_cast< AuxiliaryGeom* >( geom_ptr );
    assert( auxiliary_ptr );

    m_BogieIDVec.clear();
    m_3ptBogie1Choice.ClearItems();
    m_3ptBogie2Choice.ClearItems();
    m_3ptBogie3Choice.ClearItems();
    m_2ptBogie1Choice.ClearItems();
    m_2ptBogie2Choice.ClearItems();
    m_1ptBogie1Choice.ClearItems();
    m_3ptCCEBogie1Choice.ClearItems();
    m_3ptCCEBogie2Choice.ClearItems();
    m_3ptCCEBogie3Choice.ClearItems();

    if ( auxiliary_ptr->m_AuxuliaryGeomMode() == vsp::AUX_GEOM_THREE_PT_GROUND ||
         auxiliary_ptr->m_AuxuliaryGeomMode() == vsp::AUX_GEOM_TWO_PT_GROUND ||
         auxiliary_ptr->m_AuxuliaryGeomMode() == vsp::AUX_GEOM_ONE_PT_GROUND ||
         auxiliary_ptr->m_AuxuliaryGeomMode() == vsp::AUX_GEOM_THREE_PT_CCE )
    {
        Geom* parent_geom = veh->FindGeom( auxiliary_ptr->GetParentID() );

        GearGeom * gear = dynamic_cast< GearGeom* > ( parent_geom );
        if ( gear )
        {
            vector < Bogie* > bogie_vec = gear->GetBogieVec();

            for ( unsigned int i = 0; i < bogie_vec.size(); i++ )
            {
                m_BogieIDVec.push_back( bogie_vec[i]->GetID() );

                m_3ptBogie1Choice.AddItem( bogie_vec[i]->GetDesignation().c_str(), i );
                m_3ptBogie2Choice.AddItem( bogie_vec[i]->GetDesignation().c_str(), i );
                m_3ptBogie3Choice.AddItem( bogie_vec[i]->GetDesignation().c_str(), i );
                m_2ptBogie1Choice.AddItem( bogie_vec[i]->GetDesignation().c_str(), i );
                m_2ptBogie2Choice.AddItem( bogie_vec[i]->GetDesignation().c_str(), i );
                m_1ptBogie1Choice.AddItem( bogie_vec[i]->GetDesignation().c_str(), i );
                m_3ptCCEBogie1Choice.AddItem( bogie_vec[i]->GetDesignation().c_str(), i );
                m_3ptCCEBogie2Choice.AddItem( bogie_vec[i]->GetDesignation().c_str(), i );
                m_3ptCCEBogie3Choice.AddItem( bogie_vec[i]->GetDesignation().c_str(), i );
            }
            m_3ptBogie1Choice.UpdateItems();
            m_3ptBogie2Choice.UpdateItems();
            m_3ptBogie3Choice.UpdateItems();
            m_2ptBogie1Choice.UpdateItems();
            m_2ptBogie2Choice.UpdateItems();
            m_1ptBogie1Choice.UpdateItems();
            m_3ptCCEBogie1Choice.UpdateItems();
            m_3ptCCEBogie2Choice.UpdateItems();
            m_3ptCCEBogie3Choice.UpdateItems();

            int indx = vector_find_val( m_BogieIDVec, auxiliary_ptr->m_ContactPt1_ID );
            if ( indx >= 0 && indx < m_BogieIDVec.size() )
            {
                m_3ptBogie1Choice.SetVal( indx );
                m_2ptBogie1Choice.SetVal( indx );
                m_1ptBogie1Choice.SetVal( indx );
                m_3ptCCEBogie1Choice.SetVal( indx );
            }
            else if ( m_BogieIDVec.size() > 0 )
            {
                auxiliary_ptr->m_ContactPt1_ID = m_BogieIDVec[0];
                m_3ptBogie1Choice.SetVal( 0 );
                m_2ptBogie1Choice.SetVal( 0 );
                m_1ptBogie1Choice.SetVal( 0 );
                m_3ptCCEBogie1Choice.SetVal( 0 );
            }

            indx = vector_find_val( m_BogieIDVec, auxiliary_ptr->m_ContactPt2_ID );
            if ( indx >= 0 && indx < m_BogieIDVec.size() )
            {
                m_3ptBogie2Choice.SetVal( indx );
                m_2ptBogie2Choice.SetVal( indx );
                m_3ptCCEBogie2Choice.SetVal( indx );
            }
            else if ( m_BogieIDVec.size() > 0 )
            {
                auxiliary_ptr->m_ContactPt2_ID = m_BogieIDVec[0];
                m_3ptBogie2Choice.SetVal( 0 );
                m_2ptBogie2Choice.SetVal( 0 );
                m_3ptCCEBogie2Choice.SetVal( 0 );
            }

            indx = vector_find_val( m_BogieIDVec, auxiliary_ptr->m_ContactPt3_ID );
            if ( indx >= 0 && indx < m_BogieIDVec.size() )
            {
                m_3ptBogie3Choice.SetVal( indx );
                m_3ptCCEBogie3Choice.SetVal( indx );
            }
            else if ( m_BogieIDVec.size() > 0 )
            {
                auxiliary_ptr->m_ContactPt3_ID = m_BogieIDVec[0];
                m_3ptBogie3Choice.SetVal( 0 );
                m_3ptCCEBogie3Choice.SetVal( 0 );
            }



            m_3ptBogie1SymmChoice.ClearItems();
            m_2ptBogie1SymmChoice.ClearItems();
            m_1ptBogie1SymmChoice.ClearItems();
            m_3ptCCEBogie1SymmChoice.ClearItems();

            Bogie *b1 = gear->GetBogie( auxiliary_ptr->m_ContactPt1_ID );
            if ( b1 )
            {
                m_3ptBogie1SymmChoice.AddItem( "ISym = 0", 0 );
                m_2ptBogie1SymmChoice.AddItem( "ISym = 0", 0 );
                m_1ptBogie1SymmChoice.AddItem( "ISym = 0", 0 );
                m_3ptCCEBogie1SymmChoice.AddItem( "ISym = 0", 0 );
                if ( b1->m_Symmetrical() )
                {
                    m_3ptBogie1SymmChoice.AddItem( "ISym = 1", 1 );
                    m_2ptBogie1SymmChoice.AddItem( "ISym = 1", 1 );
                    m_1ptBogie1SymmChoice.AddItem( "ISym = 1", 1 );
                    m_3ptCCEBogie1SymmChoice.AddItem( "ISym = 1", 1 );
                }
            }
            m_3ptBogie1SymmChoice.UpdateItems();
            m_3ptBogie1SymmChoice.SetVal( auxiliary_ptr->m_ContactPt1_Isymm() );
            m_2ptBogie1SymmChoice.UpdateItems();
            m_2ptBogie1SymmChoice.SetVal( auxiliary_ptr->m_ContactPt1_Isymm() );
            m_1ptBogie1SymmChoice.UpdateItems();
            m_1ptBogie1SymmChoice.SetVal( auxiliary_ptr->m_ContactPt1_Isymm() );
            m_3ptCCEBogie1SymmChoice.UpdateItems();
            m_3ptCCEBogie1SymmChoice.SetVal( auxiliary_ptr->m_ContactPt1_Isymm() );


            m_3ptBogie2SymmChoice.ClearItems();
            m_2ptBogie2SymmChoice.ClearItems();
            m_3ptCCEBogie2SymmChoice.ClearItems();

            Bogie *b2 = gear->GetBogie( auxiliary_ptr->m_ContactPt2_ID );
            if ( b2 )
            {
                m_3ptBogie2SymmChoice.AddItem( "ISym = 0", 0 );
                m_2ptBogie2SymmChoice.AddItem( "ISym = 0", 0 );
                m_3ptCCEBogie2SymmChoice.AddItem( "ISym = 0", 0 );
                if ( b2->m_Symmetrical() )
                {
                    m_3ptBogie2SymmChoice.AddItem( "ISym = 1", 1 );
                    m_2ptBogie2SymmChoice.AddItem( "ISym = 1", 1 );
                    m_3ptCCEBogie2SymmChoice.AddItem( "ISym = 1", 1 );
                }
            }
            m_3ptBogie2SymmChoice.UpdateItems();
            m_3ptBogie2SymmChoice.SetVal( auxiliary_ptr->m_ContactPt2_Isymm() );
            m_2ptBogie2SymmChoice.UpdateItems();
            m_2ptBogie2SymmChoice.SetVal( auxiliary_ptr->m_ContactPt2_Isymm() );
            m_3ptCCEBogie2SymmChoice.UpdateItems();
            m_3ptCCEBogie2SymmChoice.SetVal( auxiliary_ptr->m_ContactPt2_Isymm() );


            m_3ptBogie3SymmChoice.ClearItems();
            m_3ptCCEBogie3SymmChoice.ClearItems();

            Bogie *b3 = gear->GetBogie( auxiliary_ptr->m_ContactPt3_ID );
            if ( b3 )
            {
                m_3ptBogie3SymmChoice.AddItem( "ISym = 0", 0 );
                m_3ptCCEBogie3SymmChoice.AddItem( "ISym = 0", 0 );
                if ( b3->m_Symmetrical() )
                {
                    m_3ptBogie3SymmChoice.AddItem( "ISym = 1", 1 );
                    m_3ptCCEBogie3SymmChoice.AddItem( "ISym = 1", 1 );
                }
            }
            m_3ptBogie3SymmChoice.UpdateItems();
            m_3ptBogie3SymmChoice.SetVal( auxiliary_ptr->m_ContactPt3_Isymm() );
            m_3ptCCEBogie3SymmChoice.UpdateItems();
            m_3ptCCEBogie3SymmChoice.SetVal( auxiliary_ptr->m_ContactPt3_Isymm() );


        }
    }


}

//==== Non Menu Callbacks ====//
void AuxiliaryGeomScreen::CallBack( Fl_Widget *w )
{
    GeomScreen::CallBack( w );
}

void AuxiliaryGeomScreen::GuiDeviceCallBack( GuiDevice* device )
{
    GeomScreen::GuiDeviceCallBack( device );

    Geom* geom_ptr = m_ScreenMgr->GetCurrGeom();
    if ( !geom_ptr )
    {
        return;
    }

    AuxiliaryGeom* auxiliary_ptr = dynamic_cast< AuxiliaryGeom* >( geom_ptr );
    assert( auxiliary_ptr );

    if ( device == &m_3ptBogie1Choice )
    {
        int val = m_3ptBogie1Choice.GetVal();
        if ( val >= 0 && val < m_BogieIDVec.size() )
        {
            auxiliary_ptr->SetContactPt1ID( m_BogieIDVec[ val ] );
        }
        else
        {
            auxiliary_ptr->SetContactPt1ID( "" );
        }
    }
    else if ( device == &m_3ptBogie2Choice )
    {
        int val = m_3ptBogie2Choice.GetVal();
        if ( val >= 0 && val < m_BogieIDVec.size() )
        {
            auxiliary_ptr->SetContactPt2ID( m_BogieIDVec[ val ] );
        }
        else
        {
            auxiliary_ptr->SetContactPt2ID( "" );
        }
    }
    else if ( device == &m_3ptBogie3Choice )
    {
        int val = m_3ptBogie3Choice.GetVal();
        if ( val >= 0 && val < m_BogieIDVec.size() )
        {
            auxiliary_ptr->SetContactPt3ID( m_BogieIDVec[ val ] );
        }
        else
        {
            auxiliary_ptr->SetContactPt3ID( "" );
        }
    }
    else if ( device == &m_2ptBogie1Choice )
    {
        int val = m_2ptBogie1Choice.GetVal();
        if ( val >= 0 && val < m_BogieIDVec.size() )
        {
            auxiliary_ptr->SetContactPt1ID( m_BogieIDVec[ val ] );
        }
        else
        {
            auxiliary_ptr->SetContactPt1ID( "" );
        }
    }
    else if ( device == &m_2ptBogie2Choice )
    {
        int val = m_2ptBogie2Choice.GetVal();
        if ( val >= 0 && val < m_BogieIDVec.size() )
        {
            auxiliary_ptr->SetContactPt2ID( m_BogieIDVec[ val ] );
        }
        else
        {
            auxiliary_ptr->SetContactPt2ID( "" );
        }
    }
    else if ( device == &m_1ptBogie1Choice )
    {
        int val = m_1ptBogie1Choice.GetVal();
        if ( val >= 0 && val < m_BogieIDVec.size() )
        {
            auxiliary_ptr->SetContactPt1ID( m_BogieIDVec[ val ] );
        }
        else
        {
            auxiliary_ptr->SetContactPt1ID( "" );
        }
    }
    if ( device == &m_3ptCCEBogie1Choice )
    {
        int val = m_3ptCCEBogie1Choice.GetVal();
        if ( val >= 0 && val < m_BogieIDVec.size() )
        {
            auxiliary_ptr->SetContactPt1ID( m_BogieIDVec[ val ] );
        }
        else
        {
            auxiliary_ptr->SetContactPt1ID( "" );
        }
    }
    else if ( device == &m_3ptCCEBogie2Choice )
    {
        int val = m_3ptCCEBogie2Choice.GetVal();
        if ( val >= 0 && val < m_BogieIDVec.size() )
        {
            auxiliary_ptr->SetContactPt2ID( m_BogieIDVec[ val ] );
        }
        else
        {
            auxiliary_ptr->SetContactPt2ID( "" );
        }
    }
    else if ( device == &m_3ptCCEBogie3Choice )
    {
        int val = m_3ptCCEBogie3Choice.GetVal();
        if ( val >= 0 && val < m_BogieIDVec.size() )
        {
            auxiliary_ptr->SetContactPt3ID( m_BogieIDVec[ val ] );
        }
        else
        {
            auxiliary_ptr->SetContactPt3ID( "" );
        }
    }
    else if ( device == &m_ReadCCEFileButton )
    {
        string newfile = m_ScreenMgr->FileChooser( "Composite Clearance Envelope", "*.cce" );
        auxiliary_ptr->ReadCCEFile( newfile );
    }
}




