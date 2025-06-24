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
#include "CurveEditScreen.h"
#include "Display.h"
#include "GearGeom.h"
#include "StlHelper.h"

//==== Constructor ====//
AuxiliaryGeomScreen::AuxiliaryGeomScreen( ScreenMgr* mgr ) : GeomScreen( mgr, 400, 800, "Auxiliary" )
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
    m_DesignLayout.AddSubGroupLayout( m_SuperConeXSecLayout, m_DesignLayout.GetW(), m_DesignLayout.GetRemainY() );

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

    //==== XSec ====//
    m_SuperConeXSecLayout.AddDividerBox( "Cross Section" );

    m_SuperConeXSecLayout.AddYGap();

    m_SuperConeXSecLayout.AddDividerBox( "Type" );

    m_XSecTypeChoice.AddItem( "POINT", vsp::XS_POINT );
    m_XSecTypeChoice.AddItem( "CIRCLE", vsp::XS_CIRCLE );
    m_XSecTypeChoice.AddItem( "ELLIPSE", vsp::XS_ELLIPSE );
    m_XSecTypeChoice.AddItem( "SUPER_ELLIPSE", vsp::XS_SUPER_ELLIPSE );
    m_XSecTypeChoice.AddItem( "ROUNDED_RECTANGLE", vsp::XS_ROUNDED_RECTANGLE );
    m_XSecTypeChoice.AddItem( "GENERAL_FUSE", vsp::XS_GENERAL_FUSE );
    m_XSecTypeChoice.AddItem( "FUSE_FILE", vsp::XS_FILE_FUSE );
    m_XSecTypeChoice.AddItem( "FOUR_SERIES", vsp::XS_FOUR_SERIES );
    m_XSecTypeChoice.AddItem( "SIX_SERIES", vsp::XS_SIX_SERIES );
    m_XSecTypeChoice.AddItem( "BICONVEX", vsp::XS_BICONVEX );
    m_XSecTypeChoice.AddItem( "WEDGE", vsp::XS_WEDGE );
    m_XSecTypeChoice.AddItem( "EDIT_CURVE", vsp::XS_EDIT_CURVE );
    m_XSecTypeChoice.AddItem( "AF_FILE", vsp::XS_FILE_AIRFOIL );
    m_XSecTypeChoice.AddItem( "CST_AIRFOIL", vsp::XS_CST_AIRFOIL );
    m_XSecTypeChoice.AddItem( "KARMAN_TREFFTZ", vsp::XS_VKT_AIRFOIL );
    m_XSecTypeChoice.AddItem( "FOUR_DIGIT_MOD", vsp::XS_FOUR_DIGIT_MOD );
    m_XSecTypeChoice.AddItem( "FIVE_DIGIT", vsp::XS_FIVE_DIGIT );
    m_XSecTypeChoice.AddItem( "FIVE_DIGIT_MOD", vsp::XS_FIVE_DIGIT_MOD );
    m_XSecTypeChoice.AddItem( "16_SERIES", vsp::XS_ONE_SIX_SERIES );
    m_XSecTypeChoice.AddItem( "AC25_773", vsp::XS_AC25_773 );

    m_SuperConeXSecLayout.SetFitWidthFlag( true );
    m_SuperConeXSecLayout.SetSameLineFlag( false );

    m_SuperConeXSecLayout.AddChoice( m_XSecTypeChoice, "Choose Type:" );

    m_SuperConeXSecLayout.SetFitWidthFlag( false );
    m_SuperConeXSecLayout.SetSameLineFlag( true );

    m_SuperConeXSecLayout.SetButtonWidth( m_SuperConeXSecLayout.GetW() / 2 );
    m_SuperConeXSecLayout.AddButton( m_ShowXSecButton, "Show" );

    m_SuperConeXSecLayout.AddSubGroupLayout( m_ConvertCEDITGroup, m_SuperConeXSecLayout.GetW(), m_SuperConeXSecLayout.GetStdHeight() );
    m_ConvertCEDITGroup.SetButtonWidth( m_SuperConeXSecLayout.GetW() / 2 );
    m_ConvertCEDITGroup.SetFitWidthFlag( false );
    m_ConvertCEDITGroup.AddButton( m_ConvertCEDITButton, "Convert CEDIT" );

    m_SuperConeXSecLayout.AddSubGroupLayout( m_EditCEDITGroup, m_SuperConeXSecLayout.GetW(), m_SuperConeXSecLayout.GetStdHeight() );
    m_EditCEDITGroup.SetFitWidthFlag( false );
    m_EditCEDITGroup.SetButtonWidth( m_SuperConeXSecLayout.GetW() / 2 );
    m_EditCEDITGroup.AddButton( m_EditCEDITButton, "Edit Curve" );

    m_SuperConeXSecLayout.ForceNewLine();

    m_SuperConeXSecLayout.SetFitWidthFlag( true );
    m_SuperConeXSecLayout.SetSameLineFlag( false );

    m_SuperConeXSecLayout.AddYGap();

    //==== Circle XSec ====//
    m_SuperConeXSecLayout.AddSubGroupLayout( m_CircleGroup, m_SuperConeXSecLayout.GetW(), m_SuperConeXSecLayout.GetRemainY() );
    m_CircleGroup.AddSlider(  m_DiameterSlider, "Diameter", 10, "%6.5f" );

    //==== Ellipse XSec ====//
    m_SuperConeXSecLayout.AddSubGroupLayout( m_EllipseGroup, m_SuperConeXSecLayout.GetW(), m_SuperConeXSecLayout.GetRemainY() );
    m_EllipseGroup.AddSlider(  m_EllipseHeightSlider, "Height", 10, "%6.5f" );
    m_EllipseGroup.AddSlider(  m_EllipseWidthSlider, "Width", 10, "%6.5f" );

    //==== AC 25.773-1 XSec ====//
    m_SuperConeXSecLayout.AddSubGroupLayout( m_AC25773Group, m_SuperConeXSecLayout.GetW(), m_SuperConeXSecLayout.GetRemainY() );
    m_AC25773Group.AddChoice( m_AC25773SeatChoice, "Pilot Seat" );

    m_AC25773SeatChoice.AddItem( "Left", vsp::XSEC_LEFT_SIDE );
    m_AC25773SeatChoice.AddItem( "Right", vsp::XSEC_RIGHT_SIDE );
    m_AC25773SeatChoice.UpdateItems();

    //==== Super XSec ====//
    m_SuperConeXSecLayout.AddSubGroupLayout( m_SuperGroup, m_SuperConeXSecLayout.GetW(), m_SuperConeXSecLayout.GetRemainY() );
    m_SuperGroup.AddSlider( m_SuperHeightSlider, "Height", 10, "%6.5f" );
    m_SuperGroup.AddSlider( m_SuperWidthSlider,  "Width", 10, "%6.5f" );
    m_SuperGroup.AddYGap();
    m_SuperGroup.AddSlider( m_SuperMaxWidthLocSlider, "MaxWLoc", 2, "%6.5f" );
    m_SuperGroup.AddYGap();
    m_SuperGroup.AddSlider( m_SuperMSlider, "M", 10, "%6.5f" );
    m_SuperGroup.AddSlider( m_SuperNSlider, "N", 10, "%6.5f" );
    m_SuperGroup.AddYGap();
    m_SuperGroup.AddButton( m_SuperToggleSym, "T/B Symmetric Exponents" );
    m_SuperGroup.AddSlider( m_SuperM_botSlider, "M Bottom", 10, "%6.5f" );
    m_SuperGroup.AddSlider( m_SuperN_botSlider, "N Bottom", 10, "%6.5f" );

    //==== Rounded Rect ====//
    m_SuperConeXSecLayout.AddSubGroupLayout( m_RoundedRectGroup, m_SuperConeXSecLayout.GetW(), m_SuperConeXSecLayout.GetRemainY() );
    m_RoundedRectGroup.AddSlider( m_RRHeightSlider, "Height", 10, "%6.5f" );
    m_RoundedRectGroup.AddSlider( m_RRWidthSlider,  "Width", 10, "%6.5f" );
    m_RoundedRectGroup.AddYGap();
    m_RoundedRectGroup.AddSlider( m_RRSkewSlider, "Skew", 2, "%6.5f");
    m_RoundedRectGroup.AddSlider( m_RRVSkewSlider, "VSkew", 2, "%6.5f" );
    m_RoundedRectGroup.AddSlider( m_RRKeystoneSlider, "Keystone", 1, "%6.5f");

    m_RoundedRectGroup.AddYGap();
    m_RoundedRectGroup.SetSameLineFlag( true );
    m_RoundedRectGroup.SetFitWidthFlag( false );

    int oldbw = m_RoundedRectGroup.GetButtonWidth();

    m_RoundedRectGroup.AddLabel( "Symmetry:", oldbw );

    m_RoundedRectGroup.SetButtonWidth( m_RoundedRectGroup.GetRemainX() / 4 );
    m_RoundedRectGroup.AddButton( m_RRRadNoSymToggle, "None" );
    m_RoundedRectGroup.AddButton( m_RRRadRLSymToggle, "R//L" );
    m_RoundedRectGroup.AddButton( m_RRRadTBSymToggle, "T//B" );
    m_RoundedRectGroup.AddButton( m_RRRadAllSymToggle, "All" );

    m_RRRadSymRadioGroup.Init( this );
    m_RRRadSymRadioGroup.AddButton( m_RRRadNoSymToggle.GetFlButton() );
    m_RRRadSymRadioGroup.AddButton( m_RRRadRLSymToggle.GetFlButton() );
    m_RRRadSymRadioGroup.AddButton( m_RRRadTBSymToggle.GetFlButton() );
    m_RRRadSymRadioGroup.AddButton( m_RRRadAllSymToggle.GetFlButton() );

    m_RoundedRectGroup.ForceNewLine();
    m_RoundedRectGroup.SetSameLineFlag( false );
    m_RoundedRectGroup.SetFitWidthFlag( true );

    m_RoundedRectGroup.SetButtonWidth( oldbw );

    m_RoundedRectGroup.AddSlider( m_RRRadiusTRSlider, "TR Radius", 10, "%6.5f" );
    m_RoundedRectGroup.AddSlider( m_RRRadiusTLSlider, "TL Radius", 10, "%6.5f" );
    m_RoundedRectGroup.AddSlider( m_RRRadiusBLSlider, "BL Radius", 10, "%6.5f" );
    m_RoundedRectGroup.AddSlider( m_RRRadiusBRSlider, "BR Radius", 10, "%6.5f" );
    m_RoundedRectGroup.AddYGap();

    m_RoundedRectGroup.AddButton( m_RRKeyCornerButton, "Key Corner" );

    //==== General Fuse XSec ====//
    m_SuperConeXSecLayout.AddSubGroupLayout( m_GenGroup, m_SuperConeXSecLayout.GetW(), m_SuperConeXSecLayout.GetRemainY() );
    m_GenGroup.AddSlider( m_GenHeightSlider, "Height", 10, "%6.5f" );
    m_GenGroup.AddSlider( m_GenWidthSlider, "Width", 10, "%6.5f" );
    m_GenGroup.AddYGap();
    m_GenGroup.AddSlider( m_GenMaxWidthLocSlider, "MaxWLoc", 1, "%6.5f" );
    m_GenGroup.AddSlider( m_GenCornerRadSlider, "CornerRad", 1, "%6.5f" );
    m_GenGroup.AddYGap();
    m_GenGroup.AddSlider( m_GenTopTanAngleSlider, "TopTanAng", 90, "%7.5f" );
    m_GenGroup.AddSlider( m_GenBotTanAngleSlider, "BotTanAng", 90, "%7.5f" );
    m_GenGroup.AddYGap();
    m_GenGroup.AddSlider( m_GenTopStrSlider, "TopStr", 1, "%7.5f" );
    m_GenGroup.AddSlider( m_GenBotStrSlider, "BotStr", 1, "%7.5f" );
    m_GenGroup.AddSlider( m_GenUpStrSlider, "UpStr", 1, "%7.5f" );
    m_GenGroup.AddSlider( m_GenLowStrSlider, "LowStr", 1, "%7.5f" );

    //==== Four Series AF ====//
    m_SuperConeXSecLayout.AddSubGroupLayout( m_FourSeriesGroup, m_SuperConeXSecLayout.GetW(), m_SuperConeXSecLayout.GetRemainY() );
    m_FourSeriesGroup.AddOutput( m_FourNameOutput, "Name" );
    m_FourSeriesGroup.AddYGap();
    m_FourSeriesGroup.AddSlider( m_FourChordSlider, "Chord", 10, "%7.3f" );
    m_FourSeriesGroup.AddSlider( m_FourThickChordSlider, "T/C", 1, "%7.5f" );
    m_FourSeriesGroup.AddYGap();

    int actionToggleButtonWidth = 15;
    int actionSliderButtonWidth = m_FourSeriesGroup.GetButtonWidth() - actionToggleButtonWidth;

    m_FourSeriesGroup.SetSameLineFlag( true );

    m_FourSeriesGroup.SetFitWidthFlag( false );
    m_FourSeriesGroup.SetButtonWidth( actionToggleButtonWidth );
    m_FourSeriesGroup.AddButton( m_FourCamberButton, "" );
    m_FourSeriesGroup.SetButtonWidth( actionSliderButtonWidth );
    m_FourSeriesGroup.SetFitWidthFlag( true );
    m_FourSeriesGroup.AddSlider( m_FourCamberSlider, "Camber", 0.2, "%7.5f" );

    m_FourSeriesGroup.ForceNewLine();

    m_FourSeriesGroup.SetFitWidthFlag( false );
    m_FourSeriesGroup.SetButtonWidth( actionToggleButtonWidth );
    m_FourSeriesGroup.AddButton( m_FourCLiButton, "" );
    m_FourSeriesGroup.SetButtonWidth( actionSliderButtonWidth );
    m_FourSeriesGroup.SetFitWidthFlag( true );
    m_FourSeriesGroup.AddSlider( m_FourCLiSlider, "Ideal CL", 1, "%7.5f" );

    m_FourSeriesGroup.ForceNewLine();

    m_FourSeriesGroup.SetSameLineFlag( false );
    m_FourSeriesGroup.SetButtonWidth( actionSliderButtonWidth + actionToggleButtonWidth );

    m_FourCamberGroup.Init( this );
    m_FourCamberGroup.AddButton( m_FourCamberButton.GetFlButton() );
    m_FourCamberGroup.AddButton( m_FourCLiButton.GetFlButton() );

    vector< int > camb_val_map;
    camb_val_map.push_back( vsp::MAX_CAMB );
    camb_val_map.push_back( vsp::DESIGN_CL );
    m_FourCamberGroup.SetValMapVec( camb_val_map );

    m_FourSeriesGroup.AddSlider( m_FourCamberLocSlider, "CamberLoc", 1, "%7.5f" );
    m_FourSeriesGroup.AddYGap();
    m_FourSeriesGroup.AddButton( m_FourInvertButton, "Invert Airfoil" );
    m_FourSeriesGroup.AddYGap();
    m_FourSeriesGroup.AddButton( m_FourSharpTEButton, "Sharpen TE" );

    m_FourSeriesGroup.AddYGap();
    m_FourSeriesGroup.SetSameLineFlag( true );
    m_FourSeriesGroup.SetFitWidthFlag( false );
    m_FourSeriesGroup.SetButtonWidth( 125 );
    m_FourSeriesGroup.AddButton( m_FourFitCSTButton, "Fit CST" );
    m_FourSeriesGroup.InitWidthHeightVals();
    m_FourSeriesGroup.SetFitWidthFlag( true );
    m_FourSeriesGroup.AddCounter( m_FourDegreeCounter, "Degree", 125 );

    //==== Six Series AF ====//
    m_SuperConeXSecLayout.AddSubGroupLayout( m_SixSeriesGroup, m_SuperConeXSecLayout.GetW(), m_SuperConeXSecLayout.GetRemainY() );
    m_SixSeriesGroup.AddOutput( m_SixNameOutput, "Name" );
    m_SixSeriesGroup.AddYGap();

    m_SixSeriesChoice.AddItem( "63-" );
    m_SixSeriesChoice.AddItem( "64-" );
    m_SixSeriesChoice.AddItem( "65-" );
    m_SixSeriesChoice.AddItem( "66-" );
    m_SixSeriesChoice.AddItem( "67-" );
    m_SixSeriesChoice.AddItem( "63A" );
    m_SixSeriesChoice.AddItem( "64A" );
    m_SixSeriesChoice.AddItem( "65A" );
    m_SixSeriesGroup.AddChoice( m_SixSeriesChoice, "Series" );

    m_SixSeriesGroup.AddYGap();

    m_SixSeriesGroup.AddSlider( m_SixChordSlider, "Chord", 10, "%7.3f" );
    m_SixSeriesGroup.AddSlider( m_SixThickChordSlider, "T/C", 1, "%7.5f" );
    m_SixSeriesGroup.AddYGap();
    m_SixSeriesGroup.AddSlider( m_SixIdealClSlider, "Ideal CL", 1, "%7.5f" );
    m_SixSeriesGroup.AddSlider( m_SixASlider, "a", 1, "%7.5f" );
    m_SixSeriesGroup.AddYGap();
    m_SixSeriesGroup.AddButton( m_SixInvertButton, "Invert Airfoil" );
    m_SixSeriesGroup.AddYGap();
    m_SixSeriesGroup.SetSameLineFlag( true );
    m_SixSeriesGroup.SetFitWidthFlag( false );
    m_SixSeriesGroup.SetButtonWidth( 125 );
    m_SixSeriesGroup.AddButton( m_SixFitCSTButton, "Fit CST" );
    m_SixSeriesGroup.InitWidthHeightVals();
    m_SixSeriesGroup.SetFitWidthFlag( true );
    m_SixSeriesGroup.AddCounter( m_SixDegreeCounter, "Degree", 125 );

    //==== Biconvex AF ====//
    m_SuperConeXSecLayout.AddSubGroupLayout( m_BiconvexGroup, m_SuperConeXSecLayout.GetW(), m_SuperConeXSecLayout.GetRemainY() );
    m_BiconvexGroup.AddSlider( m_BiconvexChordSlider, "Chord", 10, "%7.3f" );
    m_BiconvexGroup.AddSlider( m_BiconvexThickChordSlider, "T/C", 1, "%7.5f" );

    //==== Wedge AF ====//
    m_SuperConeXSecLayout.AddSubGroupLayout( m_WedgeGroup, m_SuperConeXSecLayout.GetW(), m_SuperConeXSecLayout.GetRemainY() );
    m_WedgeGroup.AddSlider( m_WedgeChordSlider, "Chord", 10, "%7.3f" );
    m_WedgeGroup.AddSlider( m_WedgeThickChordSlider, "T/C", 1, "%7.5f" );
    m_WedgeGroup.AddYGap();
    m_WedgeGroup.AddSlider( m_WedgeThickLocSlider, "Thick X", 1, "%7.5f" );
    m_WedgeGroup.AddSlider( m_WedgeFlatUpSlider, "Flat Up", 1, "%7.5f" );
    m_WedgeGroup.AddYGap();
    m_WedgeGroup.AddButton( m_WedgeSymmThickButton, "Symm Thickness" );
    m_WedgeGroup.AddSlider( m_WedgeThickLocLowSlider, "Thick X Low", 1, "%7.5f" );
    m_WedgeGroup.AddSlider( m_WedgeFlatLowSlider, "Flat Low", 1, "%7.5f" );
    m_WedgeGroup.AddYGap();
    m_WedgeGroup.AddSlider( m_WedgeZCamberSlider, "Camber", 1, "%7.5f" );
    m_WedgeGroup.AddYGap();
    m_WedgeGroup.AddSlider( m_WedgeUForeUpSlider, "U Fwd Up", 1, "%7.5f" );
    m_WedgeGroup.AddSlider( m_WedgeUForeLowSlider, "U Fwd Low", 1, "%7.5f" );
    m_WedgeGroup.AddSlider( m_WedgeDuUpSlider, "dU Flat Up", 1, "%7.5f" );
    m_WedgeGroup.AddSlider( m_WedgeDuLowSlider, "dU Flat Low", 1, "%7.5f" );
    m_WedgeGroup.AddYGap();
    m_WedgeGroup.AddButton( m_WedgeInvertButton, "Invert Airfoil" );

    //==== Fuse File ====//
    m_SuperConeXSecLayout.AddSubGroupLayout( m_FuseFileGroup, m_SuperConeXSecLayout.GetW(), m_SuperConeXSecLayout.GetRemainY() );
    m_FuseFileGroup.AddButton( m_ReadFuseFileButton, "Read File" );
    m_FuseFileGroup.AddYGap();
    m_FuseFileGroup.AddSlider( m_FileHeightSlider, "Height", 10, "%7.3f" );
    m_FuseFileGroup.AddSlider( m_FileWidthSlider, "Width", 10, "%7.3f" );

    //==== Airfoil File ====//
    m_SuperConeXSecLayout.AddSubGroupLayout( m_AfFileGroup, m_SuperConeXSecLayout.GetW(), m_SuperConeXSecLayout.GetRemainY() );
    m_AfFileGroup.AddButton( m_AfReadFileButton, "Read File" );
    m_AfFileGroup.AddYGap();
    m_AfFileGroup.AddOutput( m_AfFileNameOutput, "Name" );
    m_AfFileGroup.AddYGap();
    m_AfFileGroup.AddSlider( m_AfFileChordSlider, "Chord", 10, "%7.3f" );
    m_AfFileGroup.AddSlider( m_AfFileThickChordSlider, "T/C", 1, "%7.5f" );
    m_AfFileGroup.AddOutput( m_AfFileBaseThickChordOutput, "Base T/C", "%7.5f" );
    m_AfFileGroup.AddYGap();
    m_AfFileGroup.AddButton( m_AfFileInvertButton, "Invert Airfoil" );
    m_AfFileGroup.AddYGap();
    m_AfFileGroup.SetSameLineFlag( true );
    m_AfFileGroup.SetFitWidthFlag( false );
    m_AfFileGroup.SetButtonWidth( 125 );
    m_AfFileGroup.AddButton( m_AfFileFitCSTButton, "Fit CST" );
    m_AfFileGroup.InitWidthHeightVals();
    m_AfFileGroup.SetFitWidthFlag( true );
    m_AfFileGroup.AddCounter( m_AfFileDegreeCounter, "Degree", 125 );

    //==== CST Airfoil ====//
    m_SuperConeXSecLayout.AddSubGroupLayout( m_CSTAirfoilGroup, m_SuperConeXSecLayout.GetW(), m_SuperConeXSecLayout.GetRemainY() );

    m_CSTAirfoilGroup.AddOutput( m_CSTThickChordOutput, "T/C", "%7.5f" );

    m_CSTAirfoilGroup.AddButton( m_CSTContLERadButton, "Enforce Continuous LE Radius" );
    m_CSTAirfoilGroup.AddButton( m_CSTInvertButton, "Invert Airfoil" );

    m_CSTAirfoilGroup.AddYGap();
    m_CSTAirfoilGroup.AddSlider( m_CSTChordSlider, "Chord", 10, "%7.3f");

    m_CSTAirfoilGroup.AddYGap();
    m_CSTAirfoilGroup.AddDividerBox( "Upper Surface" );

    m_CSTAirfoilGroup.SetSameLineFlag( true );
    m_CSTAirfoilGroup.SetFitWidthFlag( true );

    m_CSTAirfoilGroup.AddOutput( m_UpDegreeOutput, "Degree", m_CSTAirfoilGroup.GetButtonWidth() * 2 );
    m_CSTAirfoilGroup.SetFitWidthFlag( false );
    m_CSTAirfoilGroup.AddButton( m_UpDemoteButton, "Demote" );
    m_CSTAirfoilGroup.AddButton( m_UpPromoteButton, "Promote" );

    m_CSTAirfoilGroup.ForceNewLine();

    m_CSTAirfoilGroup.SetSameLineFlag( false );
    m_CSTAirfoilGroup.SetFitWidthFlag( true );

    m_CSTUpCoeffScroll = m_CSTAirfoilGroup.AddFlScroll( 60 );

    m_CSTUpCoeffScroll->type( Fl_Scroll::VERTICAL_ALWAYS );
    m_CSTUpCoeffScroll->box( FL_BORDER_BOX );
    m_CSTUpCoeffLayout.SetGroupAndScreen( m_CSTUpCoeffScroll, this );

    m_CSTAirfoilGroup.AddYGap();

    m_CSTAirfoilGroup.AddDividerBox( "Lower Surface" );

    m_CSTAirfoilGroup.SetSameLineFlag( true );
    m_CSTAirfoilGroup.SetFitWidthFlag( true );

    m_CSTAirfoilGroup.AddOutput( m_LowDegreeOutput, "Degree", m_CSTAirfoilGroup.GetButtonWidth() * 2 );
    m_CSTAirfoilGroup.SetFitWidthFlag( false );
    m_CSTAirfoilGroup.AddButton( m_LowDemoteButton, "Demote" );
    m_CSTAirfoilGroup.AddButton( m_LowPromoteButton, "Promote" );

    m_CSTAirfoilGroup.ForceNewLine();

    m_CSTAirfoilGroup.SetSameLineFlag( false );
    m_CSTAirfoilGroup.SetFitWidthFlag( true );

    m_CSTLowCoeffScroll = m_CSTAirfoilGroup.AddFlScroll( 60 );
    m_CSTLowCoeffScroll->type( Fl_Scroll::VERTICAL_ALWAYS );
    m_CSTLowCoeffScroll->box( FL_BORDER_BOX );
    m_CSTLowCoeffLayout.SetGroupAndScreen( m_CSTLowCoeffScroll, this );

    //==== VKT AF ====//
    m_SuperConeXSecLayout.AddSubGroupLayout( m_VKTGroup, m_SuperConeXSecLayout.GetW(), m_SuperConeXSecLayout.GetRemainY() );
    m_VKTGroup.AddSlider( m_VKTChordSlider, "Chord", 10, "%7.3f" );
    m_VKTGroup.AddYGap();
    m_VKTGroup.AddSlider( m_VKTEpsilonSlider, "Epsilon", 1, "%7.5f" );
    m_VKTGroup.AddSlider( m_VKTKappaSlider, "Kappa", 1, "%7.5f" );
    m_VKTGroup.AddSlider( m_VKTTauSlider, "Tau", 10, "%7.5f" );
    m_VKTGroup.AddOutput( m_VKTThickChordOutput, "T/C", "%7.5f" );
    m_VKTGroup.AddYGap();
    m_VKTGroup.AddButton( m_VKTInvertButton, "Invert Airfoil" );
    m_VKTGroup.AddYGap();
    m_VKTGroup.SetSameLineFlag( true );
    m_VKTGroup.SetFitWidthFlag( false );
    m_VKTGroup.SetButtonWidth( 125 );
    m_VKTGroup.AddButton( m_VKTFitCSTButton, "Fit CST" );
    m_VKTGroup.InitWidthHeightVals();
    m_VKTGroup.SetFitWidthFlag( true );
    m_VKTGroup.AddCounter( m_VKTDegreeCounter, "Degree", 125 );

    //==== Four Series AF ====//
    m_SuperConeXSecLayout.AddSubGroupLayout( m_FourDigitModGroup, m_SuperConeXSecLayout.GetW(), m_SuperConeXSecLayout.GetRemainY() );
    m_FourDigitModGroup.AddOutput( m_FourModNameOutput, "Name" );
    m_FourDigitModGroup.AddYGap();
    m_FourDigitModGroup.AddSlider( m_FourModChordSlider, "Chord", 10, "%7.3f" );
    m_FourDigitModGroup.AddSlider( m_FourModThickChordSlider, "T/C", 1, "%7.5f" );
    m_FourDigitModGroup.AddYGap();

    m_FourDigitModGroup.SetSameLineFlag( true );

    m_FourDigitModGroup.SetFitWidthFlag( false );
    m_FourDigitModGroup.SetButtonWidth( actionToggleButtonWidth );
    m_FourDigitModGroup.AddButton( m_FourModCamberButton, "" );
    m_FourDigitModGroup.SetButtonWidth( actionSliderButtonWidth );
    m_FourDigitModGroup.SetFitWidthFlag( true );
    m_FourDigitModGroup.AddSlider( m_FourModCamberSlider, "Camber", 0.2, "%7.5f" );

    m_FourDigitModGroup.ForceNewLine();

    m_FourDigitModGroup.SetFitWidthFlag( false );
    m_FourDigitModGroup.SetButtonWidth( actionToggleButtonWidth );
    m_FourDigitModGroup.AddButton( m_FourModCLiButton, "" );
    m_FourDigitModGroup.SetButtonWidth( actionSliderButtonWidth );
    m_FourDigitModGroup.SetFitWidthFlag( true );
    m_FourDigitModGroup.AddSlider( m_FourModCLiSlider, "Ideal CL", 1, "%7.5f" );

    m_FourDigitModGroup.ForceNewLine();

    m_FourDigitModGroup.SetSameLineFlag( false );
    m_FourDigitModGroup.SetButtonWidth( actionSliderButtonWidth + actionToggleButtonWidth );

    m_FourModCamberGroup.Init( this );
    m_FourModCamberGroup.AddButton( m_FourModCamberButton.GetFlButton() );
    m_FourModCamberGroup.AddButton( m_FourModCLiButton.GetFlButton() );

    m_FourModCamberGroup.SetValMapVec( camb_val_map );

    m_FourDigitModGroup.AddSlider( m_FourModCamberLocSlider, "CamberLoc", 1, "%7.5f" );
    m_FourDigitModGroup.AddYGap();
    m_FourDigitModGroup.AddSlider( m_FourModThicknessLocSlider, "T/CLoc", 0.5, "%7.5f" );
    m_FourDigitModGroup.AddSlider( m_FourModLERadIndexSlider, "LERadIndx", 5, "%7.5f" );
    m_FourDigitModGroup.AddYGap();
    m_FourDigitModGroup.AddButton( m_FourModInvertButton, "Invert Airfoil" );
    m_FourDigitModGroup.AddYGap();
    m_FourDigitModGroup.AddButton( m_FourModSharpTEButton, "Sharpen TE" );
    m_FourDigitModGroup.AddYGap();
    m_FourDigitModGroup.SetSameLineFlag( true );
    m_FourDigitModGroup.SetFitWidthFlag( false );
    m_FourDigitModGroup.SetButtonWidth( 125 );
    m_FourDigitModGroup.AddButton( m_FourModFitCSTButton, "Fit CST" );
    m_FourDigitModGroup.InitWidthHeightVals();
    m_FourDigitModGroup.SetFitWidthFlag( true );
    m_FourDigitModGroup.AddCounter( m_FourModDegreeCounter, "Degree", 125 );

    //==== Five Digit AF ====//
    m_SuperConeXSecLayout.AddSubGroupLayout( m_FiveDigitGroup, m_SuperConeXSecLayout.GetW(), m_SuperConeXSecLayout.GetRemainY() );
    m_FiveDigitGroup.AddOutput( m_FiveNameOutput, "Name" );
    m_FiveDigitGroup.AddYGap();
    m_FiveDigitGroup.AddSlider( m_FiveChordSlider, "Chord", 10, "%7.3f" );
    m_FiveDigitGroup.AddSlider( m_FiveThickChordSlider, "T/C", 1, "%7.5f" );
    m_FiveDigitGroup.AddYGap();
    m_FiveDigitGroup.AddSlider( m_FiveCLiSlider, "Ideal CL", 1, "%7.5f" );
    m_FiveDigitGroup.AddSlider( m_FiveCamberLocSlider, "CamberLoc", 1, "%7.5f" );
    m_FiveDigitGroup.AddYGap();
    m_FiveDigitGroup.AddButton( m_FiveInvertButton, "Invert Airfoil" );
    m_FiveDigitGroup.AddYGap();
    m_FiveDigitGroup.AddButton( m_FiveSharpTEButton, "Sharpen TE" );
    m_FiveDigitGroup.AddYGap();
    m_FiveDigitGroup.SetSameLineFlag( true );
    m_FiveDigitGroup.SetFitWidthFlag( false );
    m_FiveDigitGroup.SetButtonWidth( 125 );
    m_FiveDigitGroup.AddButton( m_FiveFitCSTButton, "Fit CST" );
    m_FiveDigitGroup.InitWidthHeightVals();
    m_FiveDigitGroup.SetFitWidthFlag( true );
    m_FiveDigitGroup.AddCounter( m_FiveDegreeCounter, "Degree", 125 );

    //==== Five Digit Mod AF ====//
    m_SuperConeXSecLayout.AddSubGroupLayout( m_FiveDigitModGroup, m_SuperConeXSecLayout.GetW(), m_SuperConeXSecLayout.GetRemainY() );
    m_FiveDigitModGroup.AddOutput( m_FiveModNameOutput, "Name" );
    m_FiveDigitModGroup.AddYGap();
    m_FiveDigitModGroup.AddSlider( m_FiveModChordSlider, "Chord", 10, "%7.3f" );
    m_FiveDigitModGroup.AddSlider( m_FiveModThickChordSlider, "T/C", 1, "%7.5f" );
    m_FiveDigitModGroup.AddYGap();
    m_FiveDigitModGroup.AddSlider( m_FiveModCLiSlider, "Ideal CL", 1, "%7.5f" );
    m_FiveDigitModGroup.AddSlider( m_FiveModCamberLocSlider, "CamberLoc", 1, "%7.5f" );
    m_FiveDigitModGroup.AddYGap();
    m_FiveDigitModGroup.AddSlider( m_FiveModThicknessLocSlider, "T/CLoc", 0.5, "%7.5f" );
    m_FiveDigitModGroup.AddSlider( m_FiveModLERadIndexSlider, "LERadIndx", 5, "%7.5f" );
    m_FiveDigitModGroup.AddYGap();
    m_FiveDigitModGroup.AddButton( m_FiveModInvertButton, "Invert Airfoil" );
    m_FiveDigitModGroup.AddYGap();
    m_FiveDigitModGroup.AddButton( m_FiveModSharpTEButton, "Sharpen TE" );
    m_FiveDigitModGroup.AddYGap();
    m_FiveDigitModGroup.SetSameLineFlag( true );
    m_FiveDigitModGroup.SetFitWidthFlag( false );
    m_FiveDigitModGroup.SetButtonWidth( 125 );
    m_FiveDigitModGroup.AddButton( m_FiveModFitCSTButton, "Fit CST" );
    m_FiveDigitModGroup.InitWidthHeightVals();
    m_FiveDigitModGroup.SetFitWidthFlag( true );
    m_FiveDigitModGroup.AddCounter( m_FiveModDegreeCounter, "Degree", 125 );

    //==== 16 Series AF ====//
    m_SuperConeXSecLayout.AddSubGroupLayout( m_OneSixSeriesGroup, m_SuperConeXSecLayout.GetW(), m_SuperConeXSecLayout.GetRemainY() );
    m_OneSixSeriesGroup.AddOutput( m_OneSixSeriesNameOutput, "Name" );
    m_OneSixSeriesGroup.AddYGap();
    m_OneSixSeriesGroup.AddSlider( m_OneSixSeriesChordSlider, "Chord", 10, "%7.3f" );
    m_OneSixSeriesGroup.AddSlider( m_OneSixSeriesThickChordSlider, "T/C", 1, "%7.5f" );
    m_OneSixSeriesGroup.AddYGap();
    m_OneSixSeriesGroup.AddSlider( m_OneSixSeriesCLiSlider, "Ideal CL", 1, "%7.5f" );
    m_OneSixSeriesGroup.AddYGap();
    m_OneSixSeriesGroup.AddButton( m_OneSixSeriesInvertButton, "Invert Airfoil" );
    m_OneSixSeriesGroup.AddYGap();
    m_OneSixSeriesGroup.AddButton( m_OneSixSeriesSharpTEButton, "Sharpen TE" );
    m_OneSixSeriesGroup.AddYGap();
    m_OneSixSeriesGroup.SetSameLineFlag( true );
    m_OneSixSeriesGroup.SetFitWidthFlag( false );
    m_OneSixSeriesGroup.SetButtonWidth( 125 );
    m_OneSixSeriesGroup.AddButton( m_OneSixSeriesFitCSTButton, "Fit CST" );
    m_OneSixSeriesGroup.InitWidthHeightVals();
    m_OneSixSeriesGroup.SetFitWidthFlag( true );
    m_OneSixSeriesGroup.AddCounter( m_OneSixSeriesDegreeCounter, "Degree", 125 );

    //==== Add Attributes for Xsecs ====//
    vector < int > y_vals;
    y_vals.push_back( m_SuperGroup.GetY() );
    y_vals.push_back( m_CircleGroup.GetY() );
    y_vals.push_back( m_EllipseGroup.GetY() );
    y_vals.push_back( m_AC25773Group.GetY() );
    y_vals.push_back( m_RoundedRectGroup.GetY() );
    y_vals.push_back( m_GenGroup.GetY() );
    y_vals.push_back( m_FourSeriesGroup.GetY() );
    y_vals.push_back( m_SixSeriesGroup.GetY() );
    y_vals.push_back( m_BiconvexGroup.GetY() );
    y_vals.push_back( m_WedgeGroup.GetY() );
    y_vals.push_back( m_FuseFileGroup.GetY() );
    y_vals.push_back( m_AfFileGroup.GetY() );
    y_vals.push_back( m_CSTAirfoilGroup.GetY() );
    y_vals.push_back( m_CSTLowCoeffLayout.GetY() );
    y_vals.push_back( m_VKTGroup.GetY() );
    y_vals.push_back( m_FourDigitModGroup.GetY() );
    y_vals.push_back( m_FiveDigitGroup.GetY() );
    y_vals.push_back( m_FiveDigitModGroup.GetY() );
    y_vals.push_back( m_OneSixSeriesGroup.GetY() );

    m_SuperConeXSecLayout.SetY( *max_element( y_vals.begin(), y_vals.end() ) );
    m_XsecAttributeEditor.Init( &m_SuperConeXSecLayout, m_SuperConeXSecLayout.GetGroup(), this, staticScreenCB, true, m_GenLayout.GetY(), 100 );

    DisplayXSCGroup( nullptr );
    DisplayGroup( nullptr );
}

void AuxiliaryGeomScreen::DisplayGroup( GroupLayout* group )
{
    if ( group != &m_SuperConeXSecLayout )
    {
        DisplayXSCGroup( nullptr );
    }

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
    m_SuperConeXSecLayout.Hide();

    m_CurrDisplayGroup = group;

    if (group)
    {
        group->Show();
    }
}

void AuxiliaryGeomScreen::DisplayXSCGroup( GroupLayout* group )
{
    if ( m_CurrXSCDisplayGroup == group )
    {
        return;
    }

    m_SuperGroup.Hide();
    m_CircleGroup.Hide();
    m_EllipseGroup.Hide();
    m_AC25773Group.Hide();
    m_RoundedRectGroup.Hide();
    m_GenGroup.Hide();
    m_FourSeriesGroup.Hide();
    m_SixSeriesGroup.Hide();
    m_BiconvexGroup.Hide();
    m_WedgeGroup.Hide();
    m_FuseFileGroup.Hide();
    m_AfFileGroup.Hide();
    m_CSTAirfoilGroup.Hide();
    m_VKTGroup.Hide();
    m_FourDigitModGroup.Hide();
    m_FiveDigitGroup.Hide();
    m_FiveDigitModGroup.Hide();
    m_OneSixSeriesGroup.Hide();

    m_CurrDisplayGroup = group;

    if ( group )
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
        else if ( auxiliary_ptr->m_AuxuliaryGeomMode() == vsp::AUX_GEOM_SUPER_CONE )
        {
            DisplayGroup( &m_SuperConeXSecLayout );

            XSecCurve* xsc = auxiliary_ptr->GetXSecCurve();
            if ( xsc )
            {
                XSecViewScreen *xsscreen = dynamic_cast < XSecViewScreen* > ( m_ScreenMgr->GetScreen( vsp::VSP_XSEC_SCREEN ) );

                if ( xsscreen )
                {
                    xsscreen->SetXSecCurve( xsc );
                }

                m_XsecAttributeEditor.SetEditorCollID( xsc->GetAttrCollection()->GetID() );
                m_XsecAttributeEditor.Update();

                m_XSecTypeChoice.SetVal( xsc->GetType() );

                if ( xsc->GetType() == vsp::XS_POINT )
                {
                    DisplayXSCGroup( nullptr );
                }
                else if ( xsc->GetType() == vsp::XS_SUPER_ELLIPSE )
                {
                    DisplayXSCGroup( &m_SuperGroup );

                    SuperXSec* super_xs = dynamic_cast< SuperXSec* >( xsc );
                    assert( super_xs );
                    m_SuperHeightSlider.Update( super_xs->m_Height.GetID() );
                    m_SuperWidthSlider.Update( super_xs->m_Width.GetID() );
                    m_SuperMSlider.Update( super_xs->m_M.GetID() );
                    m_SuperNSlider.Update( super_xs->m_N.GetID() );
                    m_SuperToggleSym.Update( super_xs->m_TopBotSym.GetID() );
                    m_SuperM_botSlider.Update( super_xs->m_M_bot.GetID() );
                    m_SuperN_botSlider.Update( super_xs->m_N_bot.GetID() );
                    m_SuperMaxWidthLocSlider.Update( super_xs->m_MaxWidthLoc.GetID() );

                    if ( super_xs->m_TopBotSym() )
                    {
                        m_SuperM_botSlider.Deactivate();
                        m_SuperN_botSlider.Deactivate();
                    }
                    else if ( !super_xs->m_TopBotSym() )
                    {
                        m_SuperM_botSlider.Activate();
                        m_SuperN_botSlider.Activate();
                    }
                }
                else if ( xsc->GetType() == vsp::XS_CIRCLE )
                {
                    DisplayXSCGroup( &m_CircleGroup );
                    CircleXSec* circle_xs = dynamic_cast< CircleXSec* >( xsc );
                    assert( circle_xs );

                    m_DiameterSlider.Update( circle_xs->m_Diameter.GetID() );
                }
                else if ( xsc->GetType() == vsp::XS_ELLIPSE )
                {
                    DisplayXSCGroup( & m_EllipseGroup );

                    EllipseXSec* ellipse_xs = dynamic_cast< EllipseXSec* >( xsc );
                    m_EllipseHeightSlider.Update( ellipse_xs->m_Height.GetID() );
                    m_EllipseWidthSlider.Update( ellipse_xs->m_Width.GetID() );
                }
                else if ( xsc->GetType() == vsp::XS_AC25_773 )
                {
                    DisplayXSCGroup( & m_AC25773Group );

                    AC25_773XSec* pilotview_xs = dynamic_cast< AC25_773XSec* >( xsc );
                    m_AC25773SeatChoice.Update( pilotview_xs->m_CockpitSide.GetID() );
                }
                else if ( xsc->GetType() == vsp::XS_ROUNDED_RECTANGLE )
                {
                    DisplayXSCGroup( & m_RoundedRectGroup );
                    RoundedRectXSec* rect_xs = dynamic_cast< RoundedRectXSec* >( xsc );
                    assert( rect_xs );

                    m_RRHeightSlider.Update( rect_xs->m_Height.GetID() );
                    m_RRWidthSlider.Update( rect_xs->m_Width.GetID() );
                    m_RRRadSymRadioGroup.Update( rect_xs->m_RadiusSymmetryType.GetID() );
                    m_RRRadiusBRSlider.Update( rect_xs->m_RadiusBR.GetID() );
                    m_RRRadiusBLSlider.Update( rect_xs->m_RadiusBL.GetID() );
                    m_RRRadiusTLSlider.Update( rect_xs->m_RadiusTL.GetID() );
                    m_RRRadiusTRSlider.Update( rect_xs->m_RadiusTR.GetID() );
                    m_RRKeyCornerButton.Update( rect_xs->m_KeyCornerParm.GetID() );
                    m_RRSkewSlider.Update( rect_xs->m_Skew.GetID() );
                    m_RRKeystoneSlider.Update( rect_xs->m_Keystone.GetID() );
                    m_RRVSkewSlider.Update( rect_xs->m_VSkew.GetID() );

                    if ( rect_xs->m_RadiusSymmetryType.Get() == vsp::SYM_NONE )
                    {
                        m_RRRadiusBRSlider.Activate();
                        m_RRRadiusBLSlider.Activate();
                        m_RRRadiusTLSlider.Activate();
                    }
                    else if ( rect_xs->m_RadiusSymmetryType.Get() == vsp::SYM_RL )
                    {
                        m_RRRadiusBRSlider.Activate();
                        m_RRRadiusBLSlider.Deactivate();
                        m_RRRadiusTLSlider.Deactivate();
                    }
                    else if ( rect_xs->m_RadiusSymmetryType.Get() == vsp::SYM_TB )
                    {
                        m_RRRadiusBRSlider.Deactivate();
                        m_RRRadiusTLSlider.Activate();
                        m_RRRadiusBLSlider.Deactivate();
                    }
                    else if ( rect_xs->m_RadiusSymmetryType.Get() == vsp::SYM_ALL )
                    {
                        m_RRRadiusBRSlider.Deactivate();
                        m_RRRadiusBLSlider.Deactivate();
                        m_RRRadiusTLSlider.Deactivate();
                    }
                }
                else if ( xsc->GetType() == vsp::XS_GENERAL_FUSE )
                {
                    DisplayXSCGroup( &m_GenGroup );
                    GeneralFuseXSec* gen_xs = dynamic_cast< GeneralFuseXSec* >( xsc );
                    assert( gen_xs );

                    m_GenHeightSlider.Update( gen_xs->m_Height.GetID() );
                    m_GenWidthSlider.Update( gen_xs->m_Width.GetID() );
                    m_GenMaxWidthLocSlider.Update( gen_xs->m_MaxWidthLoc.GetID() );
                    m_GenCornerRadSlider.Update( gen_xs->m_CornerRad.GetID() );
                    m_GenTopTanAngleSlider.Update( gen_xs->m_TopTanAngle.GetID() );
                    m_GenBotTanAngleSlider.Update( gen_xs->m_BotTanAngle.GetID() );
                    m_GenTopStrSlider.Update( gen_xs->m_TopStr.GetID() );
                    m_GenBotStrSlider.Update( gen_xs->m_BotStr.GetID() );
                    m_GenUpStrSlider.Update( gen_xs->m_UpStr.GetID() );
                    m_GenLowStrSlider.Update( gen_xs->m_LowStr.GetID() );
                }
                else if ( xsc->GetType() == vsp::XS_FOUR_SERIES )
                {
                    DisplayXSCGroup( &m_FourSeriesGroup );
                    FourSeries* fs_xs = dynamic_cast< FourSeries* >( xsc );
                    assert( fs_xs );

                    m_FourChordSlider.Update( fs_xs->m_Chord.GetID() );
                    m_FourThickChordSlider.Update( fs_xs->m_ThickChord.GetID() );
                    m_FourCamberSlider.Update( fs_xs->m_Camber.GetID() );
                    m_FourCLiSlider.Update( fs_xs->m_IdealCl.GetID() );
                    m_FourCamberGroup.Update( fs_xs->m_CamberInputFlag.GetID() );
                    if ( fs_xs->m_CamberInputFlag() == vsp::MAX_CAMB )
                    {
                        m_FourCamberSlider.Activate();
                        m_FourCLiSlider.Deactivate();
                    }
                    else
                    {
                        m_FourCamberSlider.Deactivate();
                        m_FourCLiSlider.Activate();
                    }
                    m_FourCamberLocSlider.Update( fs_xs->m_CamberLoc.GetID() );
                    m_FourInvertButton.Update( fs_xs->m_Invert.GetID() );
                    m_FourNameOutput.Update( fs_xs->GetAirfoilName() );
                    m_FourSharpTEButton.Update( fs_xs->m_SharpTE.GetID() );
                    m_FourDegreeCounter.Update( fs_xs->m_FitDegree.GetID() );
                }
                else if ( xsc->GetType() == vsp::XS_SIX_SERIES )
                {
                    DisplayXSCGroup( &m_SixSeriesGroup );
                    SixSeries* ss_xs = dynamic_cast< SixSeries* >( xsc );
                    assert( ss_xs );

                    m_SixChordSlider.Update( ss_xs->m_Chord.GetID() );
                    m_SixThickChordSlider.Update( ss_xs->m_ThickChord.GetID() );
                    m_SixIdealClSlider.Update( ss_xs->m_IdealCl.GetID() );
                    m_SixASlider.Update( ss_xs->m_A.GetID() );

                    m_SixInvertButton.Update( ss_xs->m_Invert.GetID() );
                    m_SixNameOutput.Update( ss_xs->GetAirfoilName() );
                    m_SixSeriesChoice.Update( ss_xs->m_Series.GetID() );
                    m_SixDegreeCounter.Update( ss_xs->m_FitDegree.GetID() );
                }
                else if ( xsc->GetType() == vsp::XS_BICONVEX )
                {
                    DisplayXSCGroup( &m_BiconvexGroup );
                    Biconvex* bi_xs = dynamic_cast< Biconvex* >( xsc );
                    assert( bi_xs );

                    m_BiconvexChordSlider.Update( bi_xs->m_Chord.GetID() );
                    m_BiconvexThickChordSlider.Update( bi_xs->m_ThickChord.GetID() );
                }
                else if ( xsc->GetType() == vsp::XS_WEDGE )
                {
                    DisplayXSCGroup( &m_WedgeGroup );
                    Wedge* we_xs = dynamic_cast< Wedge* >( xsc );
                    assert( we_xs );

                    m_WedgeChordSlider.Update( we_xs->m_Chord.GetID() );
                    m_WedgeThickChordSlider.Update( we_xs->m_ThickChord.GetID() );
                    m_WedgeThickLocSlider.Update( we_xs->m_ThickLoc.GetID() );
                    m_WedgeZCamberSlider.Update( we_xs->m_ZCamber.GetID() );
                    m_WedgeSymmThickButton.Update( we_xs->m_SymmThick.GetID() );
                    m_WedgeThickLocLowSlider.Update( we_xs->m_ThickLocLow.GetID() );
                    m_WedgeFlatUpSlider.Update( we_xs->m_FlatUp.GetID() );
                    m_WedgeFlatLowSlider.Update( we_xs->m_FlatLow.GetID() );
                    m_WedgeUForeUpSlider.Update( we_xs->m_UForeUp.GetID() );
                    m_WedgeUForeLowSlider.Update( we_xs->m_UForeLow.GetID() );
                    m_WedgeDuUpSlider.Update( we_xs->m_DuUp.GetID() );
                    m_WedgeDuLowSlider.Update( we_xs->m_DuLow.GetID() );
                    m_WedgeInvertButton.Update( we_xs->m_Invert.GetID() );

                    if ( we_xs->m_SymmThick() )
                    {
                        m_WedgeThickLocLowSlider.Deactivate();
                        m_WedgeFlatLowSlider.Deactivate();
                    }
                }
                else if ( xsc->GetType() == vsp::XS_FILE_FUSE )
                {
                    DisplayXSCGroup( &m_FuseFileGroup );
                    FileXSec* file_xs = dynamic_cast< FileXSec* >( xsc );
                    assert( file_xs );
                    m_FileHeightSlider.Update( file_xs->m_Height.GetID() );
                    m_FileWidthSlider.Update( file_xs->m_Width.GetID() );
                }
                else if ( xsc->GetType() == vsp::XS_FILE_AIRFOIL )
                {
                    DisplayXSCGroup( &m_AfFileGroup );
                    FileAirfoil* affile_xs = dynamic_cast< FileAirfoil* >( xsc );
                    assert( affile_xs );

                    m_AfFileChordSlider.Update( affile_xs->m_Chord.GetID() );
                    m_AfFileThickChordSlider.Update( affile_xs->m_ThickChord.GetID() );
                    m_AfFileBaseThickChordOutput.Update( affile_xs->m_BaseThickness.GetID() );
                    m_AfFileInvertButton.Update( affile_xs->m_Invert.GetID() );
                    m_AfFileNameOutput.Update( affile_xs->GetAirfoilName() );
                    m_AfFileDegreeCounter.Update( affile_xs->m_FitDegree.GetID() );
                }
                else if ( xsc->GetType() == vsp::XS_CST_AIRFOIL )
                {
                    DisplayXSCGroup( &m_CSTAirfoilGroup );
                    CSTAirfoil* cst_xs = dynamic_cast< CSTAirfoil* >( xsc );
                    assert( cst_xs );

                    int num_up = cst_xs->m_UpDeg() + 1;
                    int num_low = cst_xs->m_LowDeg() + 1;

                    char str[255];
                    snprintf( str, sizeof( str ),  "%d", cst_xs->m_UpDeg() );
                    m_UpDegreeOutput.Update( str );
                    snprintf( str, sizeof( str ),  "%d", cst_xs->m_LowDeg() );
                    m_LowDegreeOutput.Update( str );

                    m_CSTChordSlider.Update(cst_xs->m_Chord.GetID());
                    m_CSTInvertButton.Update( cst_xs->m_Invert.GetID() );
                    m_CSTContLERadButton.Update( cst_xs->m_ContLERad.GetID() );
                    m_CSTThickChordOutput.Update( cst_xs->m_ThickChord.GetID() );

                    if ( ( m_UpCoeffSliderVec.size() != num_up ) || ( m_LowCoeffSliderVec.size() != num_low ) )
                    {
                        RebuildCSTGroup( cst_xs );
                    }

                    for ( int i = 0; i < num_up; i++ )
                    {
                        Parm *p = cst_xs->m_UpCoeffParmVec[i];
                        if ( p )
                        {
                            m_UpCoeffSliderVec[i].Update( p->GetID() );
                        }
                    }

                    for ( int i = 0; i < num_low; i++ )
                    {
                        Parm *p = cst_xs->m_LowCoeffParmVec[i];
                        if ( p )
                        {
                            m_LowCoeffSliderVec[i].Update( p->GetID() );
                        }
                    }

                    if ( cst_xs->m_ContLERad() && num_low > 0 )
                    {
                        m_LowCoeffSliderVec[0].Deactivate();
                    }
                }
                else if ( xsc->GetType() == vsp::XS_VKT_AIRFOIL )
                {
                    DisplayXSCGroup( &m_VKTGroup );
                    VKTAirfoil* vkt_xs = dynamic_cast< VKTAirfoil* >( xsc );
                    assert( vkt_xs );

                    m_VKTChordSlider.Update( vkt_xs->m_Chord.GetID() );
                    m_VKTEpsilonSlider.Update( vkt_xs->m_Epsilon.GetID() );
                    m_VKTKappaSlider.Update( vkt_xs->m_Kappa.GetID() );
                    m_VKTTauSlider.Update( vkt_xs->m_Tau.GetID() );
                    m_VKTInvertButton.Update( vkt_xs->m_Invert.GetID() );
                    m_VKTDegreeCounter.Update( vkt_xs->m_FitDegree.GetID() );
                    m_VKTThickChordOutput.Update( vkt_xs->m_ThickChord.GetID() );
                }
                else if ( xsc->GetType() == vsp::XS_FOUR_DIGIT_MOD )
                {
                    DisplayXSCGroup( &m_FourDigitModGroup );
                    FourDigMod* fs_xs = dynamic_cast< FourDigMod* >( xsc );
                    assert( fs_xs );

                    m_FourModChordSlider.Update( fs_xs->m_Chord.GetID() );
                    m_FourModThickChordSlider.Update( fs_xs->m_ThickChord.GetID() );
                    m_FourModCamberSlider.Update( fs_xs->m_Camber.GetID() );
                    m_FourModCLiSlider.Update( fs_xs->m_IdealCl.GetID() );
                    m_FourModCamberGroup.Update( fs_xs->m_CamberInputFlag.GetID() );
                    if ( fs_xs->m_CamberInputFlag() == vsp::MAX_CAMB )
                    {
                        m_FourModCamberSlider.Activate();
                        m_FourModCLiSlider.Deactivate();
                    }
                    else
                    {
                        m_FourModCamberSlider.Deactivate();
                        m_FourModCLiSlider.Activate();
                    }
                    m_FourModCamberLocSlider.Update( fs_xs->m_CamberLoc.GetID() );
                    m_FourModInvertButton.Update( fs_xs->m_Invert.GetID() );
                    m_FourModNameOutput.Update( fs_xs->GetAirfoilName() );
                    m_FourModThicknessLocSlider.Update( fs_xs->m_ThickLoc.GetID() );
                    m_FourModLERadIndexSlider.Update( fs_xs->m_LERadIndx.GetID() );
                    m_FourModSharpTEButton.Update( fs_xs->m_SharpTE.GetID() );
                    m_FourModDegreeCounter.Update( fs_xs->m_FitDegree.GetID() );
                }
                else if ( xsc->GetType() == vsp::XS_FIVE_DIGIT )
                {
                    DisplayXSCGroup( &m_FiveDigitGroup );
                    FiveDig* fs_xs = dynamic_cast< FiveDig* >( xsc );
                    assert( fs_xs );

                    m_FiveChordSlider.Update( fs_xs->m_Chord.GetID() );
                    m_FiveThickChordSlider.Update( fs_xs->m_ThickChord.GetID() );
                    m_FiveCLiSlider.Update( fs_xs->m_IdealCl.GetID() );
                    m_FiveCamberLocSlider.Update( fs_xs->m_CamberLoc.GetID() );
                    m_FiveInvertButton.Update( fs_xs->m_Invert.GetID() );
                    m_FiveNameOutput.Update( fs_xs->GetAirfoilName() );
                    m_FiveSharpTEButton.Update( fs_xs->m_SharpTE.GetID() );
                    m_FiveDegreeCounter.Update( fs_xs->m_FitDegree.GetID() );
                }
                else if ( xsc->GetType() == vsp::XS_FIVE_DIGIT_MOD )
                {
                    DisplayXSCGroup( &m_FiveDigitModGroup );
                    FiveDigMod* fs_xs = dynamic_cast< FiveDigMod* >( xsc );
                    assert( fs_xs );

                    m_FiveModChordSlider.Update( fs_xs->m_Chord.GetID() );
                    m_FiveModThickChordSlider.Update( fs_xs->m_ThickChord.GetID() );
                    m_FiveModCLiSlider.Update( fs_xs->m_IdealCl.GetID() );
                    m_FiveModCamberLocSlider.Update( fs_xs->m_CamberLoc.GetID() );
                    m_FiveModInvertButton.Update( fs_xs->m_Invert.GetID() );
                    m_FiveModNameOutput.Update( fs_xs->GetAirfoilName() );
                    m_FiveModThicknessLocSlider.Update( fs_xs->m_ThickLoc.GetID() );
                    m_FiveModLERadIndexSlider.Update( fs_xs->m_LERadIndx.GetID() );
                    m_FiveModSharpTEButton.Update( fs_xs->m_SharpTE.GetID() );
                    m_FiveModDegreeCounter.Update( fs_xs->m_FitDegree.GetID() );
                }
                else if ( xsc->GetType() == vsp::XS_ONE_SIX_SERIES )
                {
                    DisplayXSCGroup( &m_OneSixSeriesGroup );
                    OneSixSeries* fs_xs = dynamic_cast< OneSixSeries* >( xsc );
                    assert( fs_xs );

                    m_OneSixSeriesChordSlider.Update( fs_xs->m_Chord.GetID() );
                    m_OneSixSeriesThickChordSlider.Update( fs_xs->m_ThickChord.GetID() );
                    m_OneSixSeriesCLiSlider.Update( fs_xs->m_IdealCl.GetID() );
                    m_OneSixSeriesInvertButton.Update( fs_xs->m_Invert.GetID() );
                    m_OneSixSeriesNameOutput.Update( fs_xs->GetAirfoilName() );
                    m_OneSixSeriesSharpTEButton.Update( fs_xs->m_SharpTE.GetID() );
                    m_OneSixSeriesDegreeCounter.Update( fs_xs->m_FitDegree.GetID() );
                }
                else if ( xsc->GetType() == vsp::XS_EDIT_CURVE )
                {
                    CurveEditScreen *ceditcreen = dynamic_cast < CurveEditScreen* > ( m_ScreenMgr->GetScreen( vsp::VSP_CURVE_EDIT_SCREEN ) );

                    if ( ceditcreen )
                    {
                        ceditcreen->SetXSecCurve( xsc );
                    }

                    m_EditCEDITGroup.Show();
                    m_ConvertCEDITGroup.Hide();
                    DisplayXSCGroup( nullptr );
                }

                if ( xsc->GetType() != vsp::XS_EDIT_CURVE )
                {
                    m_EditCEDITGroup.Hide();
                    m_ConvertCEDITGroup.Show();
                }
            }
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
    else if (device == &m_XSecTypeChoice)
    {
        int t = m_XSecTypeChoice.GetVal();
        auxiliary_ptr->SetXSecCurveType( t );

        if ( t == vsp::XS_EDIT_CURVE )
        {
            m_ScreenMgr->ShowScreen( vsp::VSP_CURVE_EDIT_SCREEN );
        }
    }
    else if ( device == &m_ShowXSecButton )
    {
        m_ScreenMgr->ShowScreen( vsp::VSP_XSEC_SCREEN );
    }
    else if ( device == &m_ConvertCEDITButton )
    {
        EditCurveXSec* edit_xsec = auxiliary_ptr->ConvertToEdit();

        if ( edit_xsec )
        {
            CurveEditScreen *ceditcreen = dynamic_cast < CurveEditScreen* > ( m_ScreenMgr->GetScreen( vsp::VSP_CURVE_EDIT_SCREEN ) );

            if ( ceditcreen )
            {
                ceditcreen->SetXSecCurve( edit_xsec );
            }

            m_ScreenMgr->ShowScreen( vsp::VSP_CURVE_EDIT_SCREEN );
        }
    }
    else if ( device == &m_EditCEDITButton )
    {
        m_ScreenMgr->ShowScreen( vsp::VSP_CURVE_EDIT_SCREEN );
    }
    else if ( device == &m_ReadFuseFileButton  )
    {
        XSecCurve* xsc = auxiliary_ptr->GetXSecCurve();
        if ( xsc )
        {
            if ( xsc->GetType() == vsp::XS_FILE_FUSE  )
            {
                FileXSec* file_xs = dynamic_cast< FileXSec* >( xsc );
                assert( file_xs );
                string newfile = m_ScreenMgr->FileChooser( "Fuselage Cross Section", "*.fxs" );

                file_xs->ReadXsecFile( newfile );
                file_xs->Update();
                auxiliary_ptr->Update();
            }
        }
    }
    else if ( device == &m_AfReadFileButton   )
    {
        XSecCurve* xsc = auxiliary_ptr->GetXSecCurve();
        if ( xsc )
        {
            if ( xsc->GetType() == vsp::XS_FILE_AIRFOIL  )
            {
                FileAirfoil* affile_xs = dynamic_cast< FileAirfoil* >( xsc );
                assert( affile_xs );
                string newfile = m_ScreenMgr->FileChooser( "Airfoil File", "*.{af,dat}"  );

                affile_xs->ReadFile( newfile );
                affile_xs->Update();
                auxiliary_ptr->Update();
            }
        }
    }
    else if ( device == &m_UpPromoteButton )
    {
        XSecCurve* xsc = auxiliary_ptr->GetXSecCurve();
        if ( xsc )
        {
            if ( xsc->GetType() == vsp::XS_CST_AIRFOIL )
            {
                CSTAirfoil* cst_xs = dynamic_cast<CSTAirfoil*>( xsc );
                assert( cst_xs );

                cst_xs->PromoteUpper();
                cst_xs->Update();
                auxiliary_ptr->Update();
            }
        }
    }
    else if ( device == &m_LowPromoteButton )
    {
        XSecCurve* xsc = auxiliary_ptr->GetXSecCurve();
        if ( xsc )
        {
            if ( xsc->GetType() == vsp::XS_CST_AIRFOIL )
            {
                CSTAirfoil* cst_xs = dynamic_cast<CSTAirfoil*>( xsc );
                assert( cst_xs );

                cst_xs->PromoteLower();
                cst_xs->Update();
                auxiliary_ptr->Update();
            }
        }
    }
    else if ( device == &m_UpDemoteButton )
    {
        XSecCurve* xsc = auxiliary_ptr->GetXSecCurve();
        if ( xsc )
        {
            if ( xsc->GetType() == vsp::XS_CST_AIRFOIL )
            {
                CSTAirfoil* cst_xs = dynamic_cast<CSTAirfoil*>( xsc );
                assert( cst_xs );

                cst_xs->DemoteUpper();
                cst_xs->Update();
                auxiliary_ptr->Update();
            }
        }
    }
    else if ( device == &m_LowDemoteButton )
    {
        XSecCurve* xsc = auxiliary_ptr->GetXSecCurve();
        if ( xsc )
        {
            if ( xsc->GetType() == vsp::XS_CST_AIRFOIL )
            {
                CSTAirfoil* cst_xs = dynamic_cast<CSTAirfoil*>( xsc );
                assert( cst_xs );

                cst_xs->DemoteLower();
                cst_xs->Update();
                auxiliary_ptr->Update();
            }
        }
    }
    else if ( ( device == &m_FourFitCSTButton ) ||
            ( device == &m_SixFitCSTButton ) ||
            ( device == &m_AfFileFitCSTButton ) ||
            ( device == &m_VKTFitCSTButton ) ||
            ( device == &m_FourModFitCSTButton ) ||
            ( device == &m_FiveFitCSTButton ) ||
            ( device == &m_FiveModFitCSTButton ) ||
            ( device == &m_OneSixSeriesFitCSTButton ) )
    {
        XSecCurve* xsc = auxiliary_ptr->GetXSecCurve();
        if ( xsc )
        {
            Airfoil* af_xs = dynamic_cast<Airfoil*>( xsc );

            if ( af_xs )
            {
                VspCurve c = af_xs->GetOrigCurve();
                int deg = af_xs->m_FitDegree();

                //bor_ptr->SetActiveAirfoilType( XS_CST_AIRFOIL );
                auxiliary_ptr->SetXSecCurveType( vsp::XS_CST_AIRFOIL );

                XSecCurve* newxsc = auxiliary_ptr->GetXSecCurve();
                if ( newxsc )
                {
                    if ( newxsc->GetType() == vsp::XS_CST_AIRFOIL )
                    {
                        CSTAirfoil* cst_xs = dynamic_cast<CSTAirfoil*>( newxsc );
                        assert( cst_xs );

                        cst_xs->FitCurve( c, deg );

                        cst_xs->Update();
                        auxiliary_ptr->Update();
                    }
                }
            }
        }
    }
    m_XsecAttributeEditor.GuiDeviceCallBack( device );
}

void AuxiliaryGeomScreen::RebuildCSTGroup( CSTAirfoil* cst_xs)
{
    if ( !cst_xs )
    {
        return;
    }

    if ( !m_CSTUpCoeffScroll || !m_CSTLowCoeffScroll )
    {
        return;
    }

    m_CSTUpCoeffScroll->clear();
    m_CSTUpCoeffLayout.SetGroup( m_CSTUpCoeffScroll );
    m_CSTUpCoeffLayout.InitWidthHeightVals();

    m_UpCoeffSliderVec.clear();

    unsigned int num_up = cst_xs->m_UpDeg() + 1;

    m_UpCoeffSliderVec.resize( num_up );

    for ( int i = 0; i < num_up; i++ )
    {
        m_CSTUpCoeffLayout.AddSlider( m_UpCoeffSliderVec[i], "AUTO_UPDATE", 2, "%9.5f" );
    }




    m_CSTLowCoeffScroll->clear();
    m_CSTLowCoeffLayout.SetGroup( m_CSTLowCoeffScroll );
    m_CSTLowCoeffLayout.InitWidthHeightVals();

    m_LowCoeffSliderVec.clear();

    unsigned int num_low = cst_xs->m_LowDeg() + 1;

    m_LowCoeffSliderVec.resize( num_low );


    for ( int i = 0; i < num_low; i++ )
    {
        m_CSTLowCoeffLayout.AddSlider( m_LowCoeffSliderVec[i], "AUTO_UPDATE", 2, "%9.5f" );
    }
}



