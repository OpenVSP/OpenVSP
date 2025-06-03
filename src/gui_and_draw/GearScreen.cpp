//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#include "GearScreen.h"
#include "ScreenMgr.h"
#include "GearGeom.h"
#include "Vehicle.h"
#include "ParmMgr.h"

#include <assert.h>


//==== Constructor ====//
Gearcreen::Gearcreen( ScreenMgr* mgr ) : GeomScreen( mgr, 600, 700 + 200, "Gear" )
{
    Fl_Group* design_tab = AddTab( "Design" );
    Fl_Group* design_group = AddSubGroup( design_tab, 5 );

    Fl_Group* bogie_tab = AddTab( "Bogie" );
    Fl_Group* bogie_group = AddSubGroup( bogie_tab, 5 );

    Fl_Group* tire_tab = AddTab( "Tire" );
    Fl_Group* tire_group = AddSubGroup( tire_tab, 5 );

    // Design
    m_DesignLayout.SetGroupAndScreen( design_group, this );

    m_DesignLayout.SetButtonWidth( m_DesignLayout.GetChoiceButtonWidth() );

    m_DesignLayout.AddDividerBox( "Ground Plane Representation" );

    m_DesignLayout.SetSameLineFlag( true );
    m_DesignLayout.SetFitWidthFlag( false );
    m_DesignLayout.AddButton( m_AutoPlaneSizeToggle, "Auto" );
    m_DesignLayout.SetFitWidthFlag( true );
    m_DesignLayout.SetFitWidthFlag( true );
    m_DesignLayout.AddSlider( m_PlaneSizeSlider, "Plane Size", 10.0, "%6.5f" );

    m_DesignLayout.ForceNewLine();
    m_DesignLayout.SetSameLineFlag( false );

    m_DesignLayout.AddButton( m_IncludeNominalGroundPlaneToggle, "Include Nominal Ground Plane" );

    m_DesignLayout.AddYGap();
    m_DesignLayout.AddDividerBox( "Model Units" );

    m_ModelUnitsChoice.AddItem( "mm", vsp::LEN_MM );
    m_ModelUnitsChoice.AddItem( "cm", vsp::LEN_CM );
    m_ModelUnitsChoice.AddItem( "m", vsp::LEN_M );
    m_ModelUnitsChoice.AddItem( "in", vsp::LEN_IN );
    m_ModelUnitsChoice.AddItem( "ft", vsp::LEN_FT );
    m_ModelUnitsChoice.AddItem( "yd", vsp::LEN_YD );
    m_ModelUnitsChoice.UpdateItems();

    m_DesignLayout.AddChoice( m_ModelUnitsChoice, "Model Units" );


    m_DesignLayout.AddYGap();
    m_DesignLayout.AddDividerBox( "Center of Gravity" );



    m_DesignLayout.SetFitWidthFlag( false );
    m_DesignLayout.SetSameLineFlag( true );
    m_DesignLayout.AddLabel( "Coord System:", m_DesignLayout.GetW() - 2 * m_DesignLayout.GetInputWidth() );
    m_DesignLayout.SetButtonWidth( m_DesignLayout.GetInputWidth() );
    m_DesignLayout.AddButton( m_CGLocalToggle, "Local" );
    m_DesignLayout.AddButton( m_CGGlobalToggle, "Global" );
    m_DesignLayout.ForceNewLine();

    m_CGRadioGroup.Init( this );
    m_CGRadioGroup.AddButton( m_CGGlobalToggle.GetFlButton() ); // False
    m_CGRadioGroup.AddButton( m_CGLocalToggle.GetFlButton() );  // True

    m_DesignLayout.SetFitWidthFlag( true );
    m_DesignLayout.SetSameLineFlag( false );

    m_DesignLayout.SetButtonWidth( m_DesignLayout.GetChoiceButtonWidth() );

    m_DesignLayout.AddSlider( m_XCGMinSlider, "X Min", 100, "%6.5f" );
    m_DesignLayout.AddSlider( m_XCGNominalSlider, "X Nominal", 100, "%6.5f" );
    m_DesignLayout.AddSlider( m_XCGMaxSlider, "X Max", 100, "%6.5f" );

    m_DesignLayout.AddYGap();
    m_DesignLayout.AddSlider( m_YCGMinSlider, "Y Min", 100, "%6.5f" );
    m_DesignLayout.AddSlider( m_YCGNominalSlider, "Y Nominal", 100, "%6.5f" );
    m_DesignLayout.AddSlider( m_YCGMaxSlider, "Y Max", 100, "%6.5f" );

    m_DesignLayout.AddYGap();
    m_DesignLayout.AddSlider( m_ZCGMinSlider, "Z Min", 100, "%6.5f" );
    m_DesignLayout.AddSlider( m_ZCGNominalSlider, "Z Nominal", 100, "%6.5f" );
    m_DesignLayout.AddSlider( m_ZCGMaxSlider, "Z Max", 100, "%6.5f" );


    // Bogie
    m_BogieLayout.SetGroupAndScreen( bogie_group, this );

    // Pointer for the widths of each column in the browser to support resizing
    // Last column width must be 0
    static int bogie_widths[] = { 200, 200, 200, 0 }; // widths for each column

    m_BogieBrowser = m_BogieLayout.AddColResizeBrowser( bogie_widths, 3, 100 );
    m_BogieBrowser->callback( staticScreenCB, this );
    m_BogieLayout.AddInput( m_BogieNameInput, "Name:" );

    m_BogieLayout.SetSameLineFlag( true );
    m_BogieLayout.SetFitWidthFlag( false );
    m_BogieLayout.SetButtonWidth( m_BogieLayout.GetW() / 2.0 );

    m_BogieLayout.AddButton( m_AddBogieButton, "Add" );
    m_BogieLayout.AddButton( m_RemoveBogieButton, "Delete" );

    m_BogieLayout.ForceNewLine();

    m_BogieLayout.AddButton( m_RenameBogieButton, "Rename" );
    m_BogieLayout.AddButton( m_RemoveAllBogiesButton, "Delete All" );

    m_BogieLayout.ForceNewLine();

    m_BogieLayout.AddButton( m_ShowAllBogiesButton, "Show All" );
    m_BogieLayout.AddButton( m_HideAllBogiesButton, "Hide All" );

    m_BogieLayout.ForceNewLine();

    m_BogieLayout.SetSameLineFlag( false );
    m_BogieLayout.SetFitWidthFlag( true );


    m_BogieLayout.AddYGap();

    m_BogieLayout.AddDividerBox( "Configuration" );

    m_BogieLayout.AddButton( m_SymmetricalButton, "Symmetrical" );

    m_BogieLayout.AddYGap();
    m_BogieLayout.AddSlider( m_NAcrossSlider, "Num Across", 10, "%6.0f" );

    m_BogieLayout.AddChoice( m_SpacingTypeChoice, "Mode" );
    m_SpacingTypeChoice.AddItem( "Center Distance (Model)", vsp::BOGIE_CENTER_DIST );
    m_SpacingTypeChoice.AddItem( "Center Distance Fraction", vsp::BOGIE_CENTER_DIST_FRAC );
    m_SpacingTypeChoice.AddItem( "Gap (Model)", vsp::BOGIE_GAP );
    m_SpacingTypeChoice.AddItem( "Gap Fraction", vsp::BOGIE_GAP_FRAC );
    m_SpacingTypeChoice.UpdateItems();

    m_BogieLayout.AddSlider( m_SpacingSlider, "Spacing", 10, "%6.5f" );
    m_BogieLayout.AddSlider( m_SpacingGapSlider, "Spacing Gap", 10, "%6.5f" );

    m_BogieLayout.AddYGap();
    m_BogieLayout.AddSlider( m_NTandemSlider, "Num Tandem", 10, "%6.0f" );

    m_BogieLayout.AddChoice( m_PitchTypeChoice, "Mode" );
    m_PitchTypeChoice.AddItem( "Center Distance (Model)", vsp::BOGIE_CENTER_DIST );
    m_PitchTypeChoice.AddItem( "Center Distance Fraction", vsp::BOGIE_CENTER_DIST_FRAC );
    m_PitchTypeChoice.AddItem( "Gap (Model)", vsp::BOGIE_GAP );
    m_PitchTypeChoice.AddItem( "Gap Fraction", vsp::BOGIE_GAP_FRAC );
    m_PitchTypeChoice.UpdateItems();

    m_BogieLayout.AddSlider( m_PitchSlider, "Pitch", 10, "%6.5f" );
    m_BogieLayout.AddSlider( m_PitchGapSlider, "Pitch Gap", 10, "%6.5f" );

    m_BogieLayout.AddYGap();
    m_BogieLayout.AddDividerBox( "Contact Point" );

    m_BogieLayout.AddSlider( m_TireXSlider, "X Contact", 10, "%6.5f" );
    m_BogieLayout.AddSlider( m_TireYSlider, "Y Contact", 10, "%6.5f" );
    m_BogieLayout.AddSlider( m_TireZSlider, "Z Above Ground", 10, "%6.5f" );

    m_BogieLayout.AddYGap();
    m_BogieLayout.AddDividerBox( "Steering" );

    m_BogieLayout.AddSlider( m_SteeringAngleSlider, "Steering Angle", 1, "%6.5f" );

    m_BogieLayout.AddYGap();
    m_BogieLayout.AddDividerBox( "Suspension Travel" );

    m_BogieLayout.AddSlider( m_TravelXSlider, "X Compression Direction", 1, "%6.5f" );
    m_BogieLayout.AddSlider( m_TravelYSlider, "Y Compression Direction", 1, "%6.5f" );
    m_BogieLayout.AddSlider( m_TravelZSlider, "Z Compression Direction", 1, "%6.5f" );
    m_BogieLayout.AddSlider( m_TravelCompressSlider, "Compression Travel", 10, "%6.5f" );
    m_BogieLayout.AddSlider( m_TravelExtendSlider, "Extension Travel", 10, "%6.5f" );

    m_BogieLayout.AddYGap();
    m_BogieLayout.AddDividerBox( "Bogie Rotation Limits" );
    m_BogieLayout.AddSlider( m_BogieThetaMinSlider, "Min", 1, "%6.5f" );
    m_BogieLayout.AddSlider( m_BogieThetaMaxSlider, "Max", 1, "%6.5f" );

    m_BogieLayout.AddYGap();
    m_BogieLayout.AddDividerBox( "Bogie Position" );
    m_BogieLayout.AddButton( m_DrawNominalButton, "Draw Nominal" );
    m_BogieLayout.AddSlider( m_TravelSlider, "Suspension Travel", 10, "%6.5f" );
    m_BogieLayout.AddSlider( m_BogieThetaSlider, "Bogie Angle", 10, "%6.5f" );

    // Tire
    m_TireGroup.SetGroupAndScreen( tire_group, this );

    int inToggleButtonWidth = 35;
    int modelToggleButtonWidth = 60;
    int bw = m_TireGroup.GetButtonWidth();

    m_TireGroup.AddChoice( m_TireBogieChoice, "Bogie" );

    m_TireGroup.AddYGap();

    m_TireGroup.AddDividerBox( "Tire" );
    m_TireGroup.SetSameLineFlag( true );

    m_TireGroup.SetFitWidthFlag( false );
    m_TireGroup.SetButtonWidth( inToggleButtonWidth );
    m_TireGroup.AddButton( m_TireDiameterInToggleButton, "in" );
    m_TireGroup.SetButtonWidth( modelToggleButtonWidth );
    m_TireGroup.AddButton( m_TireDiameterModelToggleButton, "Model" );
    m_TireGroup.SetFitWidthFlag( true );
    m_TireGroup.SetButtonWidth( bw );
    m_TireGroup.AddSlider( m_TireDiameterSlider, "Diameter", 1, "%6.5f" );

    m_TireDiameterToggleGroup.Init( this );
    m_TireDiameterToggleGroup.AddButton( m_TireDiameterInToggleButton.GetFlButton() );
    m_TireDiameterToggleGroup.AddButton( m_TireDiameterModelToggleButton.GetFlButton() );


    m_TireGroup.ForceNewLine();


    m_TireGroup.SetFitWidthFlag( false );
    m_TireGroup.SetButtonWidth( inToggleButtonWidth );
    m_TireGroup.AddButton( m_TireWidthInToggleButton, "in" );
    m_TireGroup.SetButtonWidth( modelToggleButtonWidth );
    m_TireGroup.AddButton( m_TireWidthModelToggleButton, "Model" );
    m_TireGroup.SetFitWidthFlag( true );
    m_TireGroup.SetButtonWidth( bw );
    m_TireGroup.AddSlider( m_TireWidthSlider, "Width", 1, "%6.5f" );

    m_TireWidthToggleGroup.Init( this );
    m_TireWidthToggleGroup.AddButton( m_TireWidthInToggleButton.GetFlButton() );
    m_TireWidthToggleGroup.AddButton( m_TireWidthModelToggleButton.GetFlButton() );


    m_TireGroup.ForceNewLine();

    m_TireGroup.SetSameLineFlag( false );
    m_TireGroup.AddSlider( m_PlyRatingSlider, "Ply Rating", 10, "%6.5f" );

    m_TireGroup.AddButton( m_FairedFlangeButton, "Fair Wheel" );

    m_TireGroup.AddYGap();

    m_TireGroup.AddDividerBox( "Rim" );
    m_TireGroup.SetSameLineFlag( true );

    m_TireGroup.SetFitWidthFlag( false );
    m_TireGroup.SetButtonWidth( inToggleButtonWidth );
    m_TireGroup.AddButton( m_DrimInToggleButton, "in" );
    m_TireGroup.SetButtonWidth( modelToggleButtonWidth );
    m_TireGroup.AddButton( m_DrimModelToggleButton, "Model" );
    m_TireGroup.AddButton( m_DrimFracToggleButton, "Frac D" );
    m_TireGroup.SetFitWidthFlag( true );
    m_TireGroup.SetButtonWidth( bw );
    m_TireGroup.AddSlider( m_DrimSlider, "Drim", 1, "%6.5f" );

    m_DrimToggleGroup.Init( this );
    m_DrimToggleGroup.AddButton( m_DrimInToggleButton.GetFlButton() );
    m_DrimToggleGroup.AddButton( m_DrimModelToggleButton.GetFlButton() );
    m_DrimToggleGroup.AddButton( m_DrimFracToggleButton.GetFlButton() );


    m_TireGroup.ForceNewLine();


    m_TireGroup.SetFitWidthFlag( false );
    m_TireGroup.SetButtonWidth( inToggleButtonWidth );
    m_TireGroup.AddButton( m_WrimInToggleButton, "in" );
    m_TireGroup.SetButtonWidth( modelToggleButtonWidth );
    m_TireGroup.AddButton( m_WrimModelToggleButton, "Model" );
    m_TireGroup.AddButton( m_WrimFracToggleButton, "Frac W" );
    m_TireGroup.SetFitWidthFlag( true );
    m_TireGroup.SetButtonWidth( bw );
    m_TireGroup.AddSlider( m_WrimSlider, "Wrim", 1, "%6.5f" );

    m_WrimToggleGroup.Init( this );
    m_WrimToggleGroup.AddButton( m_WrimInToggleButton.GetFlButton() );
    m_WrimToggleGroup.AddButton( m_WrimModelToggleButton.GetFlButton() );
    m_WrimToggleGroup.AddButton( m_WrimFracToggleButton.GetFlButton() );

    m_TireGroup.ForceNewLine();
    m_TireGroup.AddYGap();

    m_TireGroup.SetSameLineFlag( false );
    m_TireGroup.AddDividerBox( "Shoulder" );
    m_TireGroup.SetSameLineFlag( true );

    m_TireGroup.SetFitWidthFlag( false );
    m_TireGroup.SetButtonWidth( inToggleButtonWidth );
    m_TireGroup.AddButton( m_HsInToggleButton, "in" );
    m_TireGroup.SetButtonWidth( modelToggleButtonWidth );
    m_TireGroup.AddButton( m_HsModelToggleButton, "Model" );
    m_TireGroup.AddButton( m_HsFracToggleButton, "Frac H" );
    m_TireGroup.SetFitWidthFlag( true );
    m_TireGroup.SetButtonWidth( bw );
    m_TireGroup.AddSlider( m_HsSlider, "Hs", 1, "%6.5f" );

    m_HsToggleGroup.Init( this );
    m_HsToggleGroup.AddButton( m_HsInToggleButton.GetFlButton() );
    m_HsToggleGroup.AddButton( m_HsModelToggleButton.GetFlButton() );
    m_HsToggleGroup.AddButton( m_HsFracToggleButton.GetFlButton() );

    m_TireGroup.ForceNewLine();


    m_TireGroup.SetFitWidthFlag( false );
    m_TireGroup.SetButtonWidth( inToggleButtonWidth );
    m_TireGroup.AddButton( m_WsInToggleButton, "in" );
    m_TireGroup.SetButtonWidth( modelToggleButtonWidth );
    m_TireGroup.AddButton( m_WsModelToggleButton, "Model" );
    m_TireGroup.AddButton( m_WsFracToggleButton, "Frac W" );
    m_TireGroup.SetFitWidthFlag( true );
    m_TireGroup.SetButtonWidth( bw );
    m_TireGroup.AddSlider( m_WsSlider, "Ws", 1, "%6.5f" );

    m_WsToggleGroup.Init( this );
    m_WsToggleGroup.AddButton( m_WsInToggleButton.GetFlButton() );
    m_WsToggleGroup.AddButton( m_WsModelToggleButton.GetFlButton() );
    m_WsToggleGroup.AddButton( m_WsFracToggleButton.GetFlButton() );

    m_TireGroup.ForceNewLine();
    m_TireGroup.AddYGap();

    m_TireGroup.SetSameLineFlag( false );
    m_TireGroup.AddDividerBox( "Static Loaded Radius" );
    m_TireGroup.SetSameLineFlag( true );

    m_TireGroup.SetFitWidthFlag( false );
    m_TireGroup.SetButtonWidth( inToggleButtonWidth + modelToggleButtonWidth + bw );

    m_TireGroup.AddButton( m_DeflectionToggleButton, "Delfection Frac H" );
    m_TireGroup.SetFitWidthFlag( true );
    m_TireGroup.SetButtonWidth( 0 );
    m_TireGroup.AddSlider( m_DeflectionSlider, "", 1, "%6.5f" );

    m_TireGroup.ForceNewLine();

    m_TireGroup.SetFitWidthFlag( false );
    m_TireGroup.SetButtonWidth( inToggleButtonWidth );
    m_TireGroup.AddButton( m_SLRInToggleButton, "in" );
    m_TireGroup.SetButtonWidth( modelToggleButtonWidth );
    m_TireGroup.AddButton( m_SLRModelToggleButton, "Model" );
    m_TireGroup.SetFitWidthFlag( true );
    m_TireGroup.SetButtonWidth( bw );
    m_TireGroup.AddSlider( m_SLRSlider, "SLR", 1, "%6.5f" );

    m_SLRToggleGroup.Init( this );
    m_SLRToggleGroup.AddButton( m_SLRInToggleButton.GetFlButton() );
    m_SLRToggleGroup.AddButton( m_SLRModelToggleButton.GetFlButton() );
    m_SLRToggleGroup.AddButton( m_DeflectionToggleButton.GetFlButton() );

}


//==== Show Pod Screen ====//
void Gearcreen::Show()
{
    if ( Update() )
    {
        m_FLTK_Window->show();
    }
}

//==== Update Disk Screen ====//
bool Gearcreen::Update()
{
    assert( m_ScreenMgr );

    Geom* geom_ptr = m_ScreenMgr->GetCurrGeom();
    if ( !geom_ptr || geom_ptr->GetType().m_Type != GEAR_GEOM_TYPE )
    {
        Hide();
        return false;
    }

    GeomScreen::Update();

    //==== Update Pod Specific Parms ====//
    GearGeom* gear_ptr = dynamic_cast< GearGeom* >( geom_ptr );
    assert( gear_ptr );

    m_ModelUnitsChoice.Update( gear_ptr->m_ModelLenUnits.GetID() );

    m_AutoPlaneSizeToggle.Update( gear_ptr->m_AutoPlaneFlag.GetID() );
    m_PlaneSizeSlider.Update( gear_ptr->m_PlaneSize.GetID() );

    m_IncludeNominalGroundPlaneToggle.Update( gear_ptr->m_IncludeNominalGroundPlane.GetID() );

    if ( gear_ptr->m_AutoPlaneFlag() )
    {
        m_PlaneSizeSlider.Deactivate();
    }
    else
    {
        m_PlaneSizeSlider.Activate();
    }

    m_CGRadioGroup.Update( gear_ptr->m_CGLocalFlag.GetID() );

    if ( gear_ptr->m_CGLocalFlag() )
    {
        m_XCGMinSlider.Update( 1, gear_ptr->m_XCGMinLocal.GetID(), gear_ptr->m_XCGMinGlobal.GetID() );
        m_XCGNominalSlider.Update( 1, gear_ptr->m_XCGNominalLocal.GetID(), gear_ptr->m_XCGNominalGlobal.GetID() );
        m_XCGMaxSlider.Update( 1, gear_ptr->m_XCGMaxLocal.GetID(), gear_ptr->m_XCGMaxGlobal.GetID() );

        m_YCGMinSlider.Update( 1, gear_ptr->m_YCGMinLocal.GetID(), gear_ptr->m_YCGMinGlobal.GetID() );
        m_YCGNominalSlider.Update( 1, gear_ptr->m_YCGNominalLocal.GetID(), gear_ptr->m_YCGNominalGlobal.GetID() );
        m_YCGMaxSlider.Update( 1, gear_ptr->m_YCGMaxLocal.GetID(), gear_ptr->m_YCGMaxGlobal.GetID() );

        m_ZCGMinSlider.Update( 1, gear_ptr->m_ZCGMinLocal.GetID(), gear_ptr->m_ZCGMinGlobal.GetID() );
        m_ZCGNominalSlider.Update( 1, gear_ptr->m_ZCGNominalLocal.GetID(), gear_ptr->m_ZCGNominalGlobal.GetID() );
        m_ZCGMaxSlider.Update( 1, gear_ptr->m_ZCGMaxLocal.GetID(), gear_ptr->m_ZCGMaxGlobal.GetID() );

        m_XCGMinSlider.ActivateInput1();
        m_XCGNominalSlider.ActivateInput1();
        m_XCGMaxSlider.ActivateInput1();

        m_YCGMinSlider.ActivateInput1();
        m_YCGNominalSlider.ActivateInput1();
        m_YCGMaxSlider.ActivateInput1();

        m_ZCGMinSlider.ActivateInput1();
        m_ZCGNominalSlider.ActivateInput1();
        m_ZCGMaxSlider.ActivateInput1();
    }
    else
    {
        m_XCGMinSlider.Update( 2, gear_ptr->m_XCGMinLocal.GetID(), gear_ptr->m_XCGMinGlobal.GetID() );
        m_XCGNominalSlider.Update( 2, gear_ptr->m_XCGNominalLocal.GetID(), gear_ptr->m_XCGNominalGlobal.GetID() );
        m_XCGMaxSlider.Update( 2, gear_ptr->m_XCGMaxLocal.GetID(), gear_ptr->m_XCGMaxGlobal.GetID() );

        m_YCGMinSlider.Update( 2, gear_ptr->m_YCGMinLocal.GetID(), gear_ptr->m_YCGMinGlobal.GetID() );
        m_YCGNominalSlider.Update( 2, gear_ptr->m_YCGNominalLocal.GetID(), gear_ptr->m_YCGNominalGlobal.GetID() );
        m_YCGMaxSlider.Update( 2, gear_ptr->m_YCGMaxLocal.GetID(), gear_ptr->m_YCGMaxGlobal.GetID() );

        m_ZCGMinSlider.Update( 2, gear_ptr->m_ZCGMinLocal.GetID(), gear_ptr->m_ZCGMinGlobal.GetID() );
        m_ZCGNominalSlider.Update( 2, gear_ptr->m_ZCGNominalLocal.GetID(), gear_ptr->m_ZCGNominalGlobal.GetID() );
        m_ZCGMaxSlider.Update( 2, gear_ptr->m_ZCGMaxLocal.GetID(), gear_ptr->m_ZCGMaxGlobal.GetID() );

        m_XCGMinSlider.ActivateInput2();
        m_XCGNominalSlider.ActivateInput2();
        m_XCGMaxSlider.ActivateInput2();

        m_YCGMinSlider.ActivateInput2();
        m_YCGNominalSlider.ActivateInput2();
        m_YCGMaxSlider.ActivateInput2();

        m_ZCGMinSlider.ActivateInput2();
        m_ZCGNominalSlider.ActivateInput2();
        m_ZCGMaxSlider.ActivateInput2();
    }


    std::vector < Bogie * > bogies = gear_ptr->GetBogieVec();

    UpdateBogieBrowser();


    Bogie* bogie_ptr = gear_ptr->GetCurrentBogie();

    if ( bogie_ptr )
    {
        // Bogie
        m_RenameBogieButton.Activate();
        m_RemoveBogieButton.Activate();
        m_SymmetricalButton.Activate();
        m_NAcrossSlider.Activate();
        m_NTandemSlider.Activate();
        m_SpacingTypeChoice.Activate();
        m_SpacingSlider.Activate();
        m_SpacingGapSlider.Activate();
        m_PitchTypeChoice.Activate();
        m_PitchSlider.Activate();
        m_PitchGapSlider.Activate();
        m_TireXSlider.Activate();
        m_TireYSlider.Activate();
        m_TireZSlider.Activate();
        m_TravelXSlider.Activate();
        m_TravelYSlider.Activate();
        m_TravelZSlider.Activate();
        m_TravelCompressSlider.Activate();
        m_TravelExtendSlider.Activate();

        // Tire
        m_TireDiameterToggleGroup.Activate();
        m_TireDiameterSlider.Activate();
        m_TireWidthToggleGroup.Activate();
        m_TireWidthSlider.Activate();
        m_DrimToggleGroup.Activate();
        m_DrimSlider.Activate();
        m_WrimToggleGroup.Activate();
        m_WrimSlider.Activate();
        m_HsToggleGroup.Activate();
        m_HsSlider.Activate();
        m_WsToggleGroup.Activate();
        m_WsSlider.Activate();
        m_SLRToggleGroup.Activate();
        m_DeflectionSlider.Activate();
        m_SLRSlider.Activate();




        m_SymmetricalButton.Update( bogie_ptr->m_Symmetrical.GetID() );

        m_NAcrossSlider.Update( bogie_ptr->m_NAcross.GetID() );

        m_SpacingTypeChoice.Update( bogie_ptr->m_SpacingType.GetID() );

        if ( bogie_ptr->m_SpacingType() == vsp::BOGIE_CENTER_DIST || bogie_ptr->m_SpacingType() == vsp::BOGIE_GAP )
        {
            m_SpacingSlider.Update( 1, bogie_ptr->m_Spacing.GetID(), bogie_ptr->m_SpacingFrac.GetID() );
            m_SpacingGapSlider.Update( 1, bogie_ptr->m_SpacingGap.GetID(), bogie_ptr->m_SpacingGapFrac.GetID() );
            m_SpacingSlider.ActivateInput1();
            m_SpacingGapSlider.ActivateInput1();
        }
        else
        {
            m_SpacingSlider.Update( 2, bogie_ptr->m_Spacing.GetID(), bogie_ptr->m_SpacingFrac.GetID() );
            m_SpacingGapSlider.Update( 2, bogie_ptr->m_SpacingGap.GetID(), bogie_ptr->m_SpacingGapFrac.GetID() );
            m_SpacingSlider.ActivateInput2();
            m_SpacingGapSlider.ActivateInput2();
        }


        if ( bogie_ptr->m_NAcross() > 1 )
        {
            m_SpacingTypeChoice.Activate();

            if ( bogie_ptr->m_SpacingType() == vsp::BOGIE_CENTER_DIST || bogie_ptr->m_SpacingType() == vsp::BOGIE_CENTER_DIST_FRAC )
            {
                if ( bogie_ptr->m_SpacingType() == vsp::BOGIE_CENTER_DIST )
                {
                    m_SpacingSlider.ActivateInput1();
                }
                else
                {
                    m_SpacingSlider.ActivateInput2();
                }
                m_SpacingGapSlider.Deactivate();
            }
            else
            {
                m_SpacingSlider.Deactivate();
                if ( bogie_ptr->m_SpacingType() == vsp::BOGIE_GAP )
                {
                    m_SpacingGapSlider.ActivateInput1();
                }
                else
                {
                    m_SpacingGapSlider.ActivateInput2();
                }
            }
        }
        else
        {
            m_SpacingTypeChoice.Deactivate();
            m_SpacingSlider.Deactivate();
            m_SpacingGapSlider.Deactivate();
        }



        m_NTandemSlider.Update( bogie_ptr->m_NTandem.GetID() );

        m_PitchTypeChoice.Update( bogie_ptr->m_PitchType.GetID() );

        if ( bogie_ptr->m_PitchType() == vsp::BOGIE_CENTER_DIST || bogie_ptr->m_PitchType() == vsp::BOGIE_GAP )
        {
            m_PitchSlider.Update( 1, bogie_ptr->m_Pitch.GetID(), bogie_ptr->m_PitchFrac.GetID() );
            m_PitchGapSlider.Update( 1, bogie_ptr->m_PitchGap.GetID(), bogie_ptr->m_PitchGapFrac.GetID() );
        }
        else
        {
            m_PitchSlider.Update( 2, bogie_ptr->m_Pitch.GetID(), bogie_ptr->m_PitchFrac.GetID() );
            m_PitchGapSlider.Update( 2, bogie_ptr->m_PitchGap.GetID(), bogie_ptr->m_PitchGapFrac.GetID() );
        }

        if ( bogie_ptr->m_NTandem() > 1 )
        {
            m_PitchTypeChoice.Activate();

            if ( bogie_ptr->m_PitchType() == vsp::BOGIE_CENTER_DIST || bogie_ptr->m_PitchType() == vsp::BOGIE_CENTER_DIST_FRAC )
            {
                if ( bogie_ptr->m_PitchType() == vsp::BOGIE_CENTER_DIST )
                {
                    m_PitchSlider.ActivateInput1();
                }
                else
                {
                    m_PitchSlider.ActivateInput2();
                }
                m_PitchGapSlider.Deactivate();
            }
            else
            {
                m_PitchSlider.Deactivate();
                if ( bogie_ptr->m_PitchType() == vsp::BOGIE_GAP )
                {
                    m_PitchGapSlider.ActivateInput1();
                }
                else
                {
                    m_PitchGapSlider.ActivateInput2();
                }
            }
        }
        else
        {
            m_PitchTypeChoice.Deactivate();
            m_PitchSlider.Deactivate();
            m_PitchGapSlider.Deactivate();
        }

        m_TireXSlider.Update( bogie_ptr->m_XContactPt.GetID() );
        m_TireYSlider.Update( bogie_ptr->m_YContactPt.GetID() );
        m_TireZSlider.Update( bogie_ptr->m_ZAboveGround.GetID() );

        m_SteeringAngleSlider.Update( bogie_ptr->m_SteeringAngle.GetID() );

        m_TravelXSlider.Update( bogie_ptr->m_TravelX.GetID() );
        m_TravelYSlider.Update( bogie_ptr->m_TravelY.GetID() );
        m_TravelZSlider.Update( bogie_ptr->m_TravelZ.GetID() );
        m_TravelCompressSlider.Update( bogie_ptr->m_TravelCompressed.GetID() );
        m_TravelExtendSlider.Update( bogie_ptr->m_TravelExtended.GetID() );

        m_BogieThetaMinSlider.Update( bogie_ptr->m_BogieThetaMin.GetID() );
        m_BogieThetaMaxSlider.Update( bogie_ptr->m_BogieThetaMax.GetID() );

        m_DrawNominalButton.Update( bogie_ptr->m_DrawNominal.GetID() );

        m_TravelSlider.Update( bogie_ptr->m_Travel.GetID() );
        m_BogieThetaSlider.Update( bogie_ptr->m_BogieTheta.GetID() );

        m_FairedFlangeButton.Update( bogie_ptr->m_FairedFlange.GetID() );

        m_TireDiameterToggleGroup.Update( bogie_ptr->m_DiameterMode.GetID() );
        if ( bogie_ptr->m_DiameterMode() == vsp::TIRE_DIM_IN )
        {
            m_TireDiameterSlider.Update( 1, bogie_ptr->m_DiameterIn.GetID(), bogie_ptr->m_DiameterModel.GetID() );
            m_TireDiameterSlider.ActivateInput1();
        }
        else // TIRE_DIM_MODEL
        {
            m_TireDiameterSlider.Update( 2, bogie_ptr->m_DiameterIn.GetID(), bogie_ptr->m_DiameterModel.GetID() );
            m_TireDiameterSlider.ActivateInput2();
        }

        m_TireWidthToggleGroup.Update( bogie_ptr->m_WidthMode.GetID() );
        if ( bogie_ptr->m_WidthMode() == vsp::TIRE_DIM_IN )
        {
            m_TireWidthSlider.Update( 1, bogie_ptr->m_WidthIn.GetID(), bogie_ptr->m_WidthModel.GetID() );
            m_TireWidthSlider.ActivateInput1();
        }
        else // TIRE_DIM_MODEL
        {
            m_TireWidthSlider.Update( 2, bogie_ptr->m_WidthIn.GetID(), bogie_ptr->m_WidthModel.GetID() );
            m_TireWidthSlider.ActivateInput2();
        }

        m_DrimToggleGroup.Update( bogie_ptr->m_DrimMode.GetID() );
        if ( bogie_ptr->m_DrimMode() == vsp::TIRE_DIM_IN )
        {
            m_DrimSlider.Update( 1, bogie_ptr->m_DrimIn.GetID(), bogie_ptr->m_DrimModel.GetID(), bogie_ptr->m_DrimFrac.GetID() );
            m_DrimSlider.ActivateInput1();
        }
        else if ( bogie_ptr->m_DrimMode() == vsp::TIRE_DIM_MODEL )
        {
            m_DrimSlider.Update( 2, bogie_ptr->m_DrimIn.GetID(), bogie_ptr->m_DrimModel.GetID(), bogie_ptr->m_DrimFrac.GetID() );
            m_DrimSlider.ActivateInput2();
        }
        else // TIRE_DIM_FRAC
        {
            m_DrimSlider.Update( 3, bogie_ptr->m_DrimIn.GetID(), bogie_ptr->m_DrimModel.GetID(), bogie_ptr->m_DrimFrac.GetID() );
            m_DrimSlider.ActivateInput3();
        }

        m_WrimToggleGroup.Update( bogie_ptr->m_WrimMode.GetID() );
        if ( bogie_ptr->m_WrimMode() == vsp::TIRE_DIM_IN )
        {
            m_WrimSlider.Update( 1, bogie_ptr->m_WrimIn.GetID(), bogie_ptr->m_WrimModel.GetID(), bogie_ptr->m_WrimFrac.GetID() );
            m_WrimSlider.ActivateInput1();
        }
        else if ( bogie_ptr->m_WrimMode() == vsp::TIRE_DIM_MODEL )
        {
            m_WrimSlider.Update( 2, bogie_ptr->m_WrimIn.GetID(), bogie_ptr->m_WrimModel.GetID(), bogie_ptr->m_WrimFrac.GetID() );
            m_WrimSlider.ActivateInput2();
        }
        else // TIRE_DIM_FRAC
        {
            m_WrimSlider.Update( 3, bogie_ptr->m_WrimIn.GetID(), bogie_ptr->m_WrimModel.GetID(), bogie_ptr->m_WrimFrac.GetID() );
            m_WrimSlider.ActivateInput3();
        }

        m_PlyRatingSlider.Update( bogie_ptr->m_PlyRating.GetID() );

        m_HsToggleGroup.Update( bogie_ptr->m_HsMode.GetID() );
        if ( bogie_ptr->m_HsMode() == vsp::TIRE_DIM_IN )
        {
            m_HsSlider.Update( 1, bogie_ptr->m_HsIn.GetID(), bogie_ptr->m_HsModel.GetID(), bogie_ptr->m_HsFrac.GetID() );
            m_HsSlider.ActivateInput1();
        }
        else if ( bogie_ptr->m_HsMode() == vsp::TIRE_DIM_MODEL )
        {
            m_HsSlider.Update( 2, bogie_ptr->m_HsIn.GetID(), bogie_ptr->m_HsModel.GetID(), bogie_ptr->m_HsFrac.GetID() );
            m_HsSlider.ActivateInput2();
        }
        else // TIRE_DIM_FRAC
        {
            m_HsSlider.Update( 3, bogie_ptr->m_HsIn.GetID(), bogie_ptr->m_HsModel.GetID(), bogie_ptr->m_HsFrac.GetID() );
            m_HsSlider.ActivateInput3();
        }

        m_WsToggleGroup.Update( bogie_ptr->m_WsMode.GetID() );
        if ( bogie_ptr->m_WsMode() == vsp::TIRE_DIM_IN )
        {
            m_WsSlider.Update( 1, bogie_ptr->m_WsIn.GetID(), bogie_ptr->m_WsModel.GetID(), bogie_ptr->m_WsFrac.GetID() );
            m_WsSlider.ActivateInput1();
        }
        else if ( bogie_ptr->m_WsMode() == vsp::TIRE_DIM_MODEL )
        {
            m_WsSlider.Update( 2, bogie_ptr->m_WsIn.GetID(), bogie_ptr->m_WsModel.GetID(), bogie_ptr->m_WsFrac.GetID() );
            m_WsSlider.ActivateInput2();
        }
        else // TIRE_DIM_FRAC
        {
            m_WsSlider.Update( 3, bogie_ptr->m_WsIn.GetID(), bogie_ptr->m_WsModel.GetID(), bogie_ptr->m_WsFrac.GetID() );
            m_WsSlider.ActivateInput3();
        }

        m_SLRToggleGroup.Update( bogie_ptr->m_SLRMode.GetID() );
        if ( bogie_ptr->m_SLRMode() == vsp::TIRE_DIM_IN )
        {
            m_SLRSlider.Update( 1, bogie_ptr->m_StaticRadiusIn.GetID(), bogie_ptr->m_StaticRadiusModel.GetID() );
            m_DeflectionSlider.Update( bogie_ptr->m_DeflectionPct.GetID() );

            m_SLRSlider.Activate();
            m_SLRSlider.ActivateInput1();
            m_DeflectionSlider.Deactivate();
        }
        else if ( bogie_ptr->m_SLRMode() == vsp::TIRE_DIM_MODEL )
        {
            m_SLRSlider.Update( 2, bogie_ptr->m_StaticRadiusIn.GetID(), bogie_ptr->m_StaticRadiusModel.GetID() );
            m_DeflectionSlider.Update( bogie_ptr->m_DeflectionPct.GetID() );

            m_SLRSlider.Activate();
            m_SLRSlider.ActivateInput2();
            m_DeflectionSlider.Deactivate();
        }
        else // TIRE_DIM_FRAC
        {
            m_SLRSlider.Update( 1, bogie_ptr->m_StaticRadiusIn.GetID(), bogie_ptr->m_StaticRadiusModel.GetID() );
            m_DeflectionSlider.Update( bogie_ptr->m_DeflectionPct.GetID() );

            m_SLRSlider.Deactivate();
            m_DeflectionSlider.Activate();
        }

    }
    else
    {
        // Bogie
        m_RenameBogieButton.Deactivate();
        m_RemoveBogieButton.Deactivate();
        m_SymmetricalButton.Deactivate();
        m_NAcrossSlider.Deactivate();
        m_NTandemSlider.Deactivate();
        m_SpacingTypeChoice.Deactivate();
        m_SpacingSlider.Deactivate();
        m_SpacingGapSlider.Deactivate();
        m_PitchTypeChoice.Deactivate();
        m_PitchSlider.Deactivate();
        m_PitchGapSlider.Deactivate();
        m_TireXSlider.Deactivate();
        m_TireYSlider.Deactivate();
        m_TireZSlider.Deactivate();
        m_TravelXSlider.Deactivate();
        m_TravelYSlider.Deactivate();
        m_TravelZSlider.Deactivate();
        m_TravelCompressSlider.Deactivate();
        m_TravelExtendSlider.Deactivate();

        // Tire
        m_TireDiameterToggleGroup.Deactivate();
        m_TireDiameterSlider.Deactivate();
        m_TireWidthToggleGroup.Deactivate();
        m_TireWidthSlider.Deactivate();
        m_DrimToggleGroup.Deactivate();
        m_DrimSlider.Deactivate();
        m_WrimToggleGroup.Deactivate();
        m_WrimSlider.Deactivate();
        m_HsToggleGroup.Deactivate();
        m_HsSlider.Deactivate();
        m_WsToggleGroup.Deactivate();
        m_WsSlider.Deactivate();
        m_SLRToggleGroup.Deactivate();
        m_DeflectionSlider.Deactivate();
        m_SLRSlider.Deactivate();
    }

    return true;
}

void Gearcreen::UpdateBogieBrowser()
{
    char str[255];

    Geom* geom_ptr = m_ScreenMgr->GetCurrGeom();

    GearGeom* gear_ptr = dynamic_cast< GearGeom* >( geom_ptr );

    if ( !gear_ptr )
    {
        return;
    }

    std::vector < Bogie * > bogies = gear_ptr->GetBogieVec();

    int h_pos = m_BogieBrowser->hposition();
    int v_pos = m_BogieBrowser->vposition();
    m_BogieBrowser->clear();

    m_BogieBrowser->column_char( ':' );

    snprintf( str, sizeof( str ),  "@b@.NAME:@b@.CONFIGURATION:@b@.TIRE" );
    m_BogieBrowser->add( str );

    m_TireBogieChoice.ClearItems();
    m_TireBogieChoice.AddItem( "None Selected", -1 );
    for( int i = 0; i < ( int )bogies.size(); i++ )
    {
        if ( bogies[i] )
        {
            string bogieNamed = bogies[i]->GetDesignation( ":" );
            string bogieName = bogies[i]->GetDesignation();

            m_BogieBrowser->add( bogieNamed.c_str() );
            m_TireBogieChoice.AddItem( bogieName.c_str(), i );
        }
    }
    m_TireBogieChoice.UpdateItems();

    int index = gear_ptr->GetCurrBogieIndex();
    if ( index >= 0 && index < ( int )bogies.size() )
    {
        m_BogieBrowser->select( index + 2 );
        m_TireBogieChoice.SetVal( index );
    }
    m_BogieBrowser->hposition( h_pos );
    m_BogieBrowser->vposition( v_pos );
}

//==== Non Menu Callbacks ====//
void Gearcreen::CallBack( Fl_Widget *w )
{
    Geom* geom_ptr = m_ScreenMgr->GetCurrGeom();

    GearGeom* gear_ptr = dynamic_cast< GearGeom* >( geom_ptr );

    if ( w == m_BogieBrowser )
    {
        int sel = m_BogieBrowser->value();
        gear_ptr->SetCurrBogieIndex( sel - 2 );

        Bogie * bogie = gear_ptr->GetCurrentBogie();
        if ( bogie )
        {
            m_BogieNameInput.Update( bogie->GetName() );
        }
    }

    GeomScreen::CallBack( w );
}

void Gearcreen::GuiDeviceCallBack( GuiDevice* device )
{
    Geom* geom_ptr = m_ScreenMgr->GetCurrGeom();

    GearGeom* gear_ptr = dynamic_cast< GearGeom* >( geom_ptr );

    if ( device == &m_AddBogieButton )
    {
        Bogie * bogie = gear_ptr->CreateAndAddBogie();
        if ( bogie )
        {
            if ( m_BogieNameInput.GetString().empty() )
            {
                bogie->SetName( "New Gear" );
            }
            else
            {
                bogie->SetName( m_BogieNameInput.GetString() );
            }
        }
        gear_ptr->Update();
    }
    else if ( device == &m_RemoveBogieButton )
    {
        gear_ptr->DelBogie( gear_ptr->GetCurrBogieIndex() );
    }
    else if ( device == &m_RemoveAllBogiesButton )
    {
        gear_ptr->DelAllBogies();
    }
    else if ( device == &m_ShowAllBogiesButton )
    {
        gear_ptr->ShowAllBogies();
    }
    else if ( device == &m_HideAllBogiesButton )
    {
        gear_ptr->HideAllBogies();
    }
    else if ( device == &m_RenameBogieButton )
    {
        Bogie * bogie = gear_ptr->GetCurrentBogie();
        if ( bogie )
        {
            bogie->SetName( m_BogieNameInput.GetString() );
        }
    }
    else if ( device == &m_TireBogieChoice )
    {
        gear_ptr->SetCurrBogieIndex( m_TireBogieChoice.GetVal() );
    }

    GeomScreen::GuiDeviceCallBack( device );
}



