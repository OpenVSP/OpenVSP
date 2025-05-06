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
#include "Display.h"
#include "GearGeom.h"
#include "StlHelper.h"

//==== Constructor ====//
ClearanceScreen::ClearanceScreen( ScreenMgr* mgr ) : GeomScreen( mgr, 400, 657 + 25, "Clearance" )
{
    Fl_Group* design_tab = AddTab( "Design" );
    Fl_Group* design_group = AddSubGroup( design_tab, 5 );

    m_DesignLayout.SetGroupAndScreen( design_group, this );
    m_DesignLayout.AddDividerBox( "Design" );

    //==== Design ====//

    m_ClearanceModeChoice.AddItem( "Rotor Tip Path", vsp::AUX_GEOM_ROTOR_TIP_PATH );
    m_ClearanceModeChoice.AddItem( "Rotor Burst", vsp::AUX_GEOM_ROTOR_BURST );
    m_ClearanceModeChoice.AddItem( "3pt Ground Plane", vsp::AUX_GEOM_THREE_PT_GROUND );
    m_ClearanceModeChoice.AddItem( "2pt Ground Plane", vsp::AUX_GEOM_TWO_PT_GROUND );
    m_ClearanceModeChoice.AddItem( "1pt Ground Plane", vsp::AUX_GEOM_ONE_PT_GROUND );
    m_ClearanceModeChoice.AddItem( "Rotor 1/3 Fragment", vsp::AUX_GEOM_ROTOR_BURST );
    m_ClearanceModeChoice.AddItem( "Rotor Intermediate Fragment", vsp::AUX_GEOM_ROTOR_BURST );
    m_ClearanceModeChoice.AddItem( "Tire Spray Cone", vsp::AUX_GEOM_ROTOR_BURST );
    m_ClearanceModeChoice.AddItem( "Pilot Vision", vsp::AUX_GEOM_ROTOR_BURST );
    m_ClearanceModeChoice.AddItem( "Conical Field Of View", vsp::AUX_GEOM_ROTOR_BURST );
    m_ClearanceModeChoice.UpdateItems();

    m_DesignLayout.AddChoice( m_ClearanceModeChoice, "Mode" );

    m_DesignLayout.AddYGap();

    m_DesignLayout.AddSubGroupLayout( m_RotorTipPathLayput, m_DesignLayout.GetW(), m_DesignLayout.GetRemainY() );
    m_DesignLayout.AddSubGroupLayout( m_RotorBurstLayout, m_DesignLayout.GetW(), m_DesignLayout.GetRemainY() );
    m_DesignLayout.AddSubGroupLayout( m_3ptGroundPlaneLayout, m_DesignLayout.GetW(), m_DesignLayout.GetRemainY() );
    m_DesignLayout.AddSubGroupLayout( m_2ptGroundPlaneLayout, m_DesignLayout.GetW(), m_DesignLayout.GetRemainY() );
    m_DesignLayout.AddSubGroupLayout( m_1ptGroundPlaneLayout, m_DesignLayout.GetW(), m_DesignLayout.GetRemainY() );

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

    DisplayGroup( nullptr );
}

void ClearanceScreen::DisplayGroup( GroupLayout* group )
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

    m_CurrDisplayGroup = group;

    if (group)
    {
        group->Show();
    }
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

    UpdateGroundPlaneChoices();

    m_SymmLayout.GetGroup()->deactivate();
    m_AttachLayout.GetGroup()->deactivate();

    //==== Update Clearance Specific Parms ====//
    ClearanceGeom* clearance_ptr = dynamic_cast< ClearanceGeom* >( geom_ptr );
    assert( clearance_ptr );

    if ( clearance_ptr )
    {
        m_ClearanceModeChoice.Update( clearance_ptr->m_ClearanceMode.GetID() );

        if ( clearance_ptr->m_ClearanceMode() == vsp::AUX_GEOM_ROTOR_TIP_PATH )
        {
            DisplayGroup( &m_RotorTipPathLayput );

            m_RTP_AutoDiamToggleButton.Update( clearance_ptr->m_AutoDiam.GetID() );
            m_RTP_DiameterSlider.Update( clearance_ptr->m_Diameter.GetID() );
            m_RTP_FlapRadiusFractSlider.Update( clearance_ptr->m_FlapRadiusFract.GetID() );
            m_RTP_ThetaThrustSlider.Update( clearance_ptr->m_ThetaThrust.GetID() );
            m_RTP_ThetaAntiThrustSlider.Update( clearance_ptr->m_ThetaAntiThrust.GetID() );
        }
        else if ( clearance_ptr->m_ClearanceMode() == vsp::AUX_GEOM_ROTOR_BURST )
        {
            DisplayGroup( &m_RotorBurstLayout );

            m_RB_AutoDiamToggleButton.Update( clearance_ptr->m_AutoDiam.GetID() );
            m_RB_DiameterSlider.Update( clearance_ptr->m_Diameter.GetID() );
            m_RB_FlapRadiusFractSlider.Update( clearance_ptr->m_FlapRadiusFract.GetID() );
            m_RB_ThetaThrustSlider.Update( clearance_ptr->m_ThetaThrust.GetID() );
            m_RB_ThetaAntiThrustSlider.Update( clearance_ptr->m_ThetaAntiThrust.GetID() );
            m_RB_RootLengthSlider.Update( clearance_ptr->m_RootLength.GetID() );
            m_RB_RootOffsetSlider.Update( clearance_ptr->m_RootOffset.GetID() );
        }
        else if ( clearance_ptr->m_ClearanceMode() == vsp::AUX_GEOM_THREE_PT_GROUND )
        {
            DisplayGroup( &m_3ptGroundPlaneLayout );

            m_3ptBogie1SymmChoice.Update( clearance_ptr->m_ContactPt1_Isymm.GetID() );
            m_3ptBogie1SuspensionModeChoice.Update( clearance_ptr->m_ContactPt1_SuspensionMode.GetID() );
            m_3ptBogie1TireModeChoice.Update( clearance_ptr->m_ContactPt1_TireMode.GetID() );

            m_3ptBogie2SymmChoice.Update( clearance_ptr->m_ContactPt2_Isymm.GetID() );
            m_3ptBogie2SuspensionModeChoice.Update( clearance_ptr->m_ContactPt2_SuspensionMode.GetID() );
            m_3ptBogie2TireModeChoice.Update( clearance_ptr->m_ContactPt2_TireMode.GetID() );

            m_3ptBogie3SymmChoice.Update( clearance_ptr->m_ContactPt3_Isymm.GetID() );
            m_3ptBogie3SuspensionModeChoice.Update( clearance_ptr->m_ContactPt3_SuspensionMode.GetID() );
            m_3ptBogie3TireModeChoice.Update( clearance_ptr->m_ContactPt3_TireMode.GetID() );
        }
        else if ( clearance_ptr->m_ClearanceMode() == vsp::AUX_GEOM_TWO_PT_GROUND )
        {
            DisplayGroup( &m_2ptGroundPlaneLayout );

            m_2ptBogie1SymmChoice.Update( clearance_ptr->m_ContactPt1_Isymm.GetID() );
            m_2ptBogie1SuspensionModeChoice.Update( clearance_ptr->m_ContactPt1_SuspensionMode.GetID() );
            m_2ptBogie1TireModeChoice.Update( clearance_ptr->m_ContactPt1_TireMode.GetID() );

            m_2ptBogie2SymmChoice.Update( clearance_ptr->m_ContactPt2_Isymm.GetID() );
            m_2ptBogie2SuspensionModeChoice.Update( clearance_ptr->m_ContactPt2_SuspensionMode.GetID() );
            m_2ptBogie2TireModeChoice.Update( clearance_ptr->m_ContactPt2_TireMode.GetID() );

            m_2ptBogieThetaSlider.Update( clearance_ptr->m_BogieTheta.GetID() );
            m_2ptWheelThetaSlider.Update( clearance_ptr->m_WheelTheta.GetID() );
        }
        else if ( clearance_ptr->m_ClearanceMode() == vsp::AUX_GEOM_ONE_PT_GROUND )
        {
            DisplayGroup( &m_1ptGroundPlaneLayout );

            m_1ptBogie1SymmChoice.Update( clearance_ptr->m_ContactPt1_Isymm.GetID() );
            m_1ptBogie1SuspensionModeChoice.Update( clearance_ptr->m_ContactPt1_SuspensionMode.GetID() );
            m_1ptBogie1TireModeChoice.Update( clearance_ptr->m_ContactPt1_TireMode.GetID() );

            m_1ptBogieThetaSlider.Update( clearance_ptr->m_BogieTheta.GetID() );
            m_1ptWheelThetaSlider.Update( clearance_ptr->m_WheelTheta.GetID() );
            m_1ptRollThetaSlider.Update( clearance_ptr->m_RollTheta.GetID() );

        }
    }

    return true;
}

void ClearanceScreen::UpdateGroundPlaneChoices()
{
    Vehicle *veh = VehicleMgr.GetVehicle();

    Geom* geom_ptr = m_ScreenMgr->GetCurrGeom();
    if ( !geom_ptr )
    {
        return;
    }

    ClearanceGeom* clearance_ptr = dynamic_cast< ClearanceGeom* >( geom_ptr );
    assert( clearance_ptr );

    m_BogieIDVec.clear();
    m_3ptBogie1Choice.ClearItems();
    m_3ptBogie2Choice.ClearItems();
    m_3ptBogie3Choice.ClearItems();
    m_2ptBogie1Choice.ClearItems();
    m_2ptBogie2Choice.ClearItems();
    m_1ptBogie1Choice.ClearItems();

    if ( clearance_ptr->m_ClearanceMode() == vsp::AUX_GEOM_THREE_PT_GROUND ||
         clearance_ptr->m_ClearanceMode() == vsp::AUX_GEOM_TWO_PT_GROUND ||
         clearance_ptr->m_ClearanceMode() == vsp::AUX_GEOM_ONE_PT_GROUND )
    {
        Geom* parent_geom = veh->FindGeom( clearance_ptr->GetParentID() );

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
            }
            m_3ptBogie1Choice.UpdateItems();
            m_3ptBogie2Choice.UpdateItems();
            m_3ptBogie3Choice.UpdateItems();
            m_2ptBogie1Choice.UpdateItems();
            m_2ptBogie2Choice.UpdateItems();
            m_1ptBogie1Choice.UpdateItems();

            int indx = vector_find_val( m_BogieIDVec, clearance_ptr->m_ContactPt1_ID );
            if ( indx >= 0 && indx < m_BogieIDVec.size() )
            {
                m_3ptBogie1Choice.SetVal( indx );
                m_2ptBogie1Choice.SetVal( indx );
                m_1ptBogie1Choice.SetVal( indx );
            }
            else if ( m_BogieIDVec.size() > 0 )
            {
                clearance_ptr->m_ContactPt1_ID = m_BogieIDVec[0];
                m_3ptBogie1Choice.SetVal( 0 );
                m_2ptBogie1Choice.SetVal( 0 );
                m_1ptBogie1Choice.SetVal( 0 );
            }

            indx = vector_find_val( m_BogieIDVec, clearance_ptr->m_ContactPt2_ID );
            if ( indx >= 0 && indx < m_BogieIDVec.size() )
            {
                m_3ptBogie2Choice.SetVal( indx );
                m_2ptBogie2Choice.SetVal( indx );
            }
            else if ( m_BogieIDVec.size() > 0 )
            {
                clearance_ptr->m_ContactPt2_ID = m_BogieIDVec[0];
                m_3ptBogie2Choice.SetVal( 0 );
                m_2ptBogie2Choice.SetVal( 0 );
            }

            indx = vector_find_val( m_BogieIDVec, clearance_ptr->m_ContactPt3_ID );
            if ( indx >= 0 && indx < m_BogieIDVec.size() )
            {
                m_3ptBogie3Choice.SetVal( indx );
            }
            else if ( m_BogieIDVec.size() > 0 )
            {
                clearance_ptr->m_ContactPt3_ID = m_BogieIDVec[0];
                m_3ptBogie3Choice.SetVal( 0 );
            }



            m_3ptBogie1SymmChoice.ClearItems();
            m_2ptBogie1SymmChoice.ClearItems();
            m_1ptBogie1SymmChoice.ClearItems();

            Bogie *b1 = gear->GetBogie( clearance_ptr->m_ContactPt1_ID );
            if ( b1 )
            {
                m_3ptBogie1SymmChoice.AddItem( "ISym = 0", 0 );
                m_2ptBogie1SymmChoice.AddItem( "ISym = 0", 0 );
                m_1ptBogie1SymmChoice.AddItem( "ISym = 0", 0 );
                if ( b1->m_Symmetrical() )
                {
                    m_3ptBogie1SymmChoice.AddItem( "ISym = 1", 1 );
                    m_2ptBogie1SymmChoice.AddItem( "ISym = 1", 1 );
                    m_1ptBogie1SymmChoice.AddItem( "ISym = 1", 1 );
                }
            }
            m_3ptBogie1SymmChoice.UpdateItems();
            m_3ptBogie1SymmChoice.SetVal( clearance_ptr->m_ContactPt1_Isymm() );
            m_2ptBogie1SymmChoice.UpdateItems();
            m_2ptBogie1SymmChoice.SetVal( clearance_ptr->m_ContactPt1_Isymm() );
            m_1ptBogie1SymmChoice.UpdateItems();
            m_1ptBogie1SymmChoice.SetVal( clearance_ptr->m_ContactPt1_Isymm() );


            m_3ptBogie2SymmChoice.ClearItems();
            m_2ptBogie2SymmChoice.ClearItems();

            Bogie *b2 = gear->GetBogie( clearance_ptr->m_ContactPt2_ID );
            if ( b2 )
            {
                m_3ptBogie2SymmChoice.AddItem( "ISym = 0", 0 );
                m_2ptBogie2SymmChoice.AddItem( "ISym = 0", 0 );
                if ( b2->m_Symmetrical() )
                {
                    m_3ptBogie2SymmChoice.AddItem( "ISym = 1", 1 );
                    m_2ptBogie2SymmChoice.AddItem( "ISym = 1", 1 );
                }
            }
            m_3ptBogie2SymmChoice.UpdateItems();
            m_3ptBogie2SymmChoice.SetVal( clearance_ptr->m_ContactPt2_Isymm() );
            m_2ptBogie2SymmChoice.UpdateItems();
            m_2ptBogie2SymmChoice.SetVal( clearance_ptr->m_ContactPt2_Isymm() );


            m_3ptBogie3SymmChoice.ClearItems();

            Bogie *b3 = gear->GetBogie( clearance_ptr->m_ContactPt3_ID );
            if ( b3 )
            {
                m_3ptBogie3SymmChoice.AddItem( "ISym = 0", 0 );
                if ( b3->m_Symmetrical() )
                {
                    m_3ptBogie3SymmChoice.AddItem( "ISym = 1", 1 );
                }
            }
            m_3ptBogie3SymmChoice.UpdateItems();
            m_3ptBogie3SymmChoice.SetVal( clearance_ptr->m_ContactPt3_Isymm() );


        }
    }


}

//==== Non Menu Callbacks ====//
void ClearanceScreen::CallBack( Fl_Widget *w )
{
    GeomScreen::CallBack( w );
}

void ClearanceScreen::GuiDeviceCallBack( GuiDevice* device )
{
    GeomScreen::GuiDeviceCallBack( device );

    Geom* geom_ptr = m_ScreenMgr->GetCurrGeom();
    if ( !geom_ptr )
    {
        return;
    }

    ClearanceGeom* clearance_ptr = dynamic_cast< ClearanceGeom* >( geom_ptr );
    assert( clearance_ptr );

    if ( device == &m_3ptBogie1Choice )
    {
        int val = m_3ptBogie1Choice.GetVal();
        if ( val >= 0 && val < m_BogieIDVec.size() )
        {
            clearance_ptr->SetContactPt1ID( m_BogieIDVec[ val ] );
        }
        else
        {
            clearance_ptr->SetContactPt1ID( "" );
        }
    }
    else if ( device == &m_3ptBogie2Choice )
    {
        int val = m_3ptBogie2Choice.GetVal();
        if ( val >= 0 && val < m_BogieIDVec.size() )
        {
            clearance_ptr->SetContactPt2ID( m_BogieIDVec[ val ] );
        }
        else
        {
            clearance_ptr->SetContactPt2ID( "" );
        }
    }
    else if ( device == &m_3ptBogie3Choice )
    {
        int val = m_3ptBogie3Choice.GetVal();
        if ( val >= 0 && val < m_BogieIDVec.size() )
        {
            clearance_ptr->SetContactPt3ID( m_BogieIDVec[ val ] );
        }
        else
        {
            clearance_ptr->SetContactPt3ID( "" );
        }
    }
    else if ( device == &m_2ptBogie1Choice )
    {
        int val = m_2ptBogie1Choice.GetVal();
        if ( val >= 0 && val < m_BogieIDVec.size() )
        {
            clearance_ptr->SetContactPt1ID( m_BogieIDVec[ val ] );
        }
        else
        {
            clearance_ptr->SetContactPt1ID( "" );
        }
    }
    else if ( device == &m_2ptBogie2Choice )
    {
        int val = m_2ptBogie2Choice.GetVal();
        if ( val >= 0 && val < m_BogieIDVec.size() )
        {
            clearance_ptr->SetContactPt2ID( m_BogieIDVec[ val ] );
        }
        else
        {
            clearance_ptr->SetContactPt2ID( "" );
        }
    }
    else if ( device == &m_1ptBogie1Choice )
    {
        int val = m_1ptBogie1Choice.GetVal();
        if ( val >= 0 && val < m_BogieIDVec.size() )
        {
            clearance_ptr->SetContactPt1ID( m_BogieIDVec[ val ] );
        }
        else
        {
            clearance_ptr->SetContactPt1ID( "" );
        }
    }
}




