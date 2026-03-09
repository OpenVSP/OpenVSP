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
#include "WingGeom.h"
#include "RoutingGeom.h"

#include <assert.h>


//==== Constructor ====//
GearScreen::GearScreen( ScreenMgr* mgr ) : GeomScreen( mgr, 450, 800, "Gear" )
{
    Fl_Group* design_tab = AddTab( "Design" );
    Fl_Group* design_group = AddSubGroup( design_tab, 5 );

    Fl_Group* bogie_tab = AddTab( "Bogie" );
    Fl_Group* bogie_group = AddSubGroup( bogie_tab, 5 );

    Fl_Group* tire_tab = AddTab( "Tire" );
    Fl_Group* tire_group = AddSubGroup( tire_tab, 5 );

    Fl_Group* retract_tab = AddTab( "Retract" );
    Fl_Group* retract_group = AddSubGroup( retract_tab, 5 );

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

    m_GearConfigurationChoice.AddItem( "Down", vsp::GEAR_CONFIGURATION_DOWN );
    m_GearConfigurationChoice.AddItem( "Up", vsp::GEAR_CONFIGURATION_UP );
    m_GearConfigurationChoice.AddItem( "Up and Down", vsp::GEAR_CONFIGURATION_UP_AND_DOWN );
    m_GearConfigurationChoice.AddItem( "Intermediate", vsp::GEAR_CONFIGURATION_INTERMEDIATE );
    m_GearConfigurationChoice.AddItem( "All", vsp::GEAR_CONFIGURATION_ALL );

    m_DesignLayout.AddChoice( m_GearConfigurationChoice, "Gear Configuration" );

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
    static int bogie_widths[] = { 150, 150, 150, 0 }; // widths for each column

    m_BogieBrowser = m_BogieLayout.AddColResizeBrowser( bogie_widths, 3, 100 );
    m_BogieBrowser->Init( this, m_BogieLayout.GetGroup() );
    m_BogieLayout.AddInput( m_BogieNameInput, "Name:" );

    m_BogieLayout.SetSameLineFlag( true );
    m_BogieLayout.SetFitWidthFlag( false );
    m_BogieLayout.SetButtonWidth( m_BogieLayout.GetW() / 2.0 );

    m_BogieLayout.AddButton( m_AddBogieButton, "Add" );

    m_BogieLayout.ForceNewLine();

    m_BogieLayout.AddButton( m_RemoveBogieButton, "Delete" );
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
    m_BogieLayout.AddSlider( m_NAcrossSlider, "Across", 10, "%6.0f" );

    m_BogieLayout.AddChoice( m_SpacingTypeChoice, "Mode" );
    m_SpacingTypeChoice.AddItem( "Center Distance (Model)", vsp::BOGIE_CENTER_DIST );
    m_SpacingTypeChoice.AddItem( "Center Distance Fraction", vsp::BOGIE_CENTER_DIST_FRAC );
    m_SpacingTypeChoice.AddItem( "Gap (Model)", vsp::BOGIE_GAP );
    m_SpacingTypeChoice.AddItem( "Gap Fraction", vsp::BOGIE_GAP_FRAC );
    m_SpacingTypeChoice.UpdateItems();

    m_BogieLayout.AddSlider( m_SpacingSlider, "Spacing", 10, "%6.5f" );
    m_BogieLayout.AddSlider( m_SpacingGapSlider, "Spacing Gap", 10, "%6.5f" );

    m_BogieLayout.AddYGap();
    m_BogieLayout.AddSlider( m_NTandemSlider, "Tandem", 10, "%6.0f" );

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

    // Tire
    m_TireLayout.SetGroupAndScreen( tire_group, this );

    int inToggleButtonWidth = 35;
    int modelToggleButtonWidth = 60;
    int bw = m_TireLayout.GetButtonWidth();
    int iw = m_TireLayout.GetInputWidth();
    int bw2 = 40;
    int iw2 = 50;

    m_TireLayout.AddChoice( m_TireBogieChoice, "Bogie" );

    m_TireLayout.AddYGap();

    m_TireLayout.AddDividerBox( "Tire" );
    m_TireLayout.SetSameLineFlag( true );

    m_TireLayout.SetFitWidthFlag( false );
    m_TireLayout.SetButtonWidth( inToggleButtonWidth );
    m_TireLayout.AddButton( m_TireDiameterInToggleButton, "in" );
    m_TireLayout.SetButtonWidth( modelToggleButtonWidth );
    m_TireLayout.AddButton( m_TireDiameterModelToggleButton, "Model" );
    m_TireLayout.SetFitWidthFlag( true );
    m_TireLayout.SetButtonWidth( bw );
    m_TireLayout.AddSlider( m_TireDiameterSlider, "Diameter", 1, "%6.5f" );

    m_TireDiameterToggleGroup.Init( this );
    m_TireDiameterToggleGroup.AddButton( m_TireDiameterInToggleButton.GetFlButton() );
    m_TireDiameterToggleGroup.AddButton( m_TireDiameterModelToggleButton.GetFlButton() );


    m_TireLayout.ForceNewLine();


    m_TireLayout.SetFitWidthFlag( false );
    m_TireLayout.SetButtonWidth( inToggleButtonWidth );
    m_TireLayout.AddButton( m_TireWidthInToggleButton, "in" );
    m_TireLayout.SetButtonWidth( modelToggleButtonWidth );
    m_TireLayout.AddButton( m_TireWidthModelToggleButton, "Model" );
    m_TireLayout.SetFitWidthFlag( true );
    m_TireLayout.SetButtonWidth( bw );
    m_TireLayout.AddSlider( m_TireWidthSlider, "Width", 1, "%6.5f" );

    m_TireWidthToggleGroup.Init( this );
    m_TireWidthToggleGroup.AddButton( m_TireWidthInToggleButton.GetFlButton() );
    m_TireWidthToggleGroup.AddButton( m_TireWidthModelToggleButton.GetFlButton() );


    m_TireLayout.ForceNewLine();
    m_TireLayout.SetInputWidth( iw2 );

    m_TireLayout.SetSameLineFlag( false );
    m_TireLayout.AddSlider( m_PlyRatingSlider, "Ply Rating", 10, "%6.5f" );
    m_TireLayout.AddSlider( m_SpeedRatingSlider, "Speed Rating", 10, "%6.5f" );

    m_TireModeChoice.AddItem( "Full TRA", vsp::TIRE_TRA );
    m_TireModeChoice.AddItem( "TRA w/ Faired Flange", vsp::TIRE_FAIR_FLANGE );
    m_TireModeChoice.AddItem( "TRA w/ Faired Wheel", vsp::TIRE_FAIR_WHEEL );
    m_TireModeChoice.AddItem( "Balloon Tire", vsp::TIRE_BALLOON );
    m_TireModeChoice.AddItem( "Balloon Tire w/ Wheel", vsp::TIRE_BALLOON_WHEEL );
    m_TireModeChoice.AddItem( "Balloon Tire w/ Faired Wheel", vsp::TIRE_BALLOON_FAIR_WHEEL );
    m_TireLayout.AddChoice( m_TireModeChoice, "Tire Mode" );

    m_TireLayout.AddYGap();

    m_TireLayout.AddDividerBox( "Rim" );
    m_TireLayout.SetSameLineFlag( true );

    m_TireLayout.SetFitWidthFlag( false );
    m_TireLayout.SetButtonWidth( inToggleButtonWidth );
    m_TireLayout.AddButton( m_DrimInToggleButton, "in" );
    m_TireLayout.SetButtonWidth( modelToggleButtonWidth );
    m_TireLayout.AddButton( m_DrimModelToggleButton, "Model" );
    m_TireLayout.AddButton( m_DrimFracToggleButton, "Frac D" );
    m_TireLayout.SetFitWidthFlag( true );
    m_TireLayout.SetButtonWidth( bw2 );
    m_TireLayout.AddSlider( m_DrimSlider, "Drim", 1, "%6.4f" );

    m_DrimToggleGroup.Init( this );
    m_DrimToggleGroup.AddButton( m_DrimInToggleButton.GetFlButton() );
    m_DrimToggleGroup.AddButton( m_DrimModelToggleButton.GetFlButton() );
    m_DrimToggleGroup.AddButton( m_DrimFracToggleButton.GetFlButton() );


    m_TireLayout.ForceNewLine();


    m_TireLayout.SetFitWidthFlag( false );
    m_TireLayout.SetButtonWidth( inToggleButtonWidth );
    m_TireLayout.AddButton( m_WrimInToggleButton, "in" );
    m_TireLayout.SetButtonWidth( modelToggleButtonWidth );
    m_TireLayout.AddButton( m_WrimModelToggleButton, "Model" );
    m_TireLayout.AddButton( m_WrimFracToggleButton, "Frac W" );
    m_TireLayout.SetFitWidthFlag( true );
    m_TireLayout.SetButtonWidth( bw2 );
    m_TireLayout.AddSlider( m_WrimSlider, "Wrim", 1, "%6.4f" );

    m_WrimToggleGroup.Init( this );
    m_WrimToggleGroup.AddButton( m_WrimInToggleButton.GetFlButton() );
    m_WrimToggleGroup.AddButton( m_WrimModelToggleButton.GetFlButton() );
    m_WrimToggleGroup.AddButton( m_WrimFracToggleButton.GetFlButton() );

    m_TireLayout.ForceNewLine();
    m_TireLayout.AddYGap();

    m_TireLayout.SetSameLineFlag( false );
    m_TireLayout.AddDividerBox( "Shoulder" );
    m_TireLayout.SetSameLineFlag( true );

    m_TireLayout.SetFitWidthFlag( false );
    m_TireLayout.SetButtonWidth( inToggleButtonWidth );
    m_TireLayout.AddButton( m_HsInToggleButton, "in" );
    m_TireLayout.SetButtonWidth( modelToggleButtonWidth );
    m_TireLayout.AddButton( m_HsModelToggleButton, "Model" );
    m_TireLayout.AddButton( m_HsFracToggleButton, "Frac H" );
    m_TireLayout.SetFitWidthFlag( true );
    m_TireLayout.SetButtonWidth( bw2 );
    m_TireLayout.AddSlider( m_HsSlider, "Hs", 1, "%6.4f" );

    m_HsToggleGroup.Init( this );
    m_HsToggleGroup.AddButton( m_HsInToggleButton.GetFlButton() );
    m_HsToggleGroup.AddButton( m_HsModelToggleButton.GetFlButton() );
    m_HsToggleGroup.AddButton( m_HsFracToggleButton.GetFlButton() );

    m_TireLayout.ForceNewLine();


    m_TireLayout.SetFitWidthFlag( false );
    m_TireLayout.SetButtonWidth( inToggleButtonWidth );
    m_TireLayout.AddButton( m_WsInToggleButton, "in" );
    m_TireLayout.SetButtonWidth( modelToggleButtonWidth );
    m_TireLayout.AddButton( m_WsModelToggleButton, "Model" );
    m_TireLayout.AddButton( m_WsFracToggleButton, "Frac W" );
    m_TireLayout.SetFitWidthFlag( true );
    m_TireLayout.SetButtonWidth( bw2 );
    m_TireLayout.AddSlider( m_WsSlider, "Ws", 1, "%6.4f" );

    m_WsToggleGroup.Init( this );
    m_WsToggleGroup.AddButton( m_WsInToggleButton.GetFlButton() );
    m_WsToggleGroup.AddButton( m_WsModelToggleButton.GetFlButton() );
    m_WsToggleGroup.AddButton( m_WsFracToggleButton.GetFlButton() );

    m_TireLayout.ForceNewLine();
    m_TireLayout.AddYGap();

    m_TireLayout.SetSameLineFlag( false );
    m_TireLayout.AddDividerBox( "Static Loaded Radius" );
    m_TireLayout.SetSameLineFlag( true );
    m_TireLayout.SetInputWidth( iw );

    m_TireLayout.SetFitWidthFlag( false );
    m_TireLayout.SetButtonWidth( inToggleButtonWidth + modelToggleButtonWidth + bw2 );

    m_TireLayout.AddButton( m_DeflectionToggleButton, "Delfection Frac H" );
    m_TireLayout.SetFitWidthFlag( true );
    m_TireLayout.SetButtonWidth( 0 );
    m_TireLayout.AddSlider( m_DeflectionSlider, "", 1, "%6.5f" );

    m_TireLayout.ForceNewLine();

    m_TireLayout.SetFitWidthFlag( false );
    m_TireLayout.SetButtonWidth( inToggleButtonWidth );
    m_TireLayout.AddButton( m_SLRInToggleButton, "in" );
    m_TireLayout.SetButtonWidth( modelToggleButtonWidth );
    m_TireLayout.AddButton( m_SLRModelToggleButton, "Model" );
    m_TireLayout.SetFitWidthFlag( true );
    m_TireLayout.SetButtonWidth( bw2 );
    m_TireLayout.AddSlider( m_SLRSlider, "SLR", 1, "%6.5f" );

    m_SLRToggleGroup.Init( this );
    m_SLRToggleGroup.AddButton( m_SLRInToggleButton.GetFlButton() );
    m_SLRToggleGroup.AddButton( m_SLRModelToggleButton.GetFlButton() );
    m_SLRToggleGroup.AddButton( m_DeflectionToggleButton.GetFlButton() );


    //==== Retract Layout ====//
    m_RetractLayout.SetGroupAndScreen( retract_group, this );

    int border = 5;
    int actionToggleButtonWidth = 40;
    int normalButtonWidth = 110;

    m_RetractLayout.SetChoiceButtonWidth( normalButtonWidth + border );

    m_RetractLayout.AddChoice( m_RetBogieChoice, "Bogie" );

    m_RetModeChoice.AddItem( "Stowed Position", vsp::GEAR_STOWED_POSITION );
    m_RetModeChoice.AddItem( "Gear Mechanism", vsp::GEAR_MECHANISM );
    m_RetractLayout.AddChoice( m_RetModeChoice, "Mode" );

    m_RetractLayout.AddYGap();

    // Add tabs
    m_RetractTabs = new Fl_Tabs( m_RetractLayout.GetX(), m_RetractLayout.GetY(), m_RetractLayout.GetRemainX(), m_RetractLayout.GetRemainY() );
    m_RetractTabs->labelcolor( FL_BLUE );

    int rx, ry, rw, rh;
    m_RetractTabs->client_area( rx, ry, rw, rh, TAB_H );

    int x = rx + border;
    int y = ry + border;
    int w = rw - 2 * border;
    int h = rh - 2 * border;



    Fl_Group* stow_grp = new Vsp_Group( rx, ry, rw, rh );
    stow_grp->copy_label( "Stow" );
    stow_grp->selection_color( FL_GRAY );
    stow_grp->labelfont( 1 );
    stow_grp->labelcolor( FL_BLACK );

    m_RetractTabs->add( stow_grp );

    Fl_Group* stow_sub_group = new Fl_Group( x, y, w, h );
    stow_grp->add( stow_sub_group );



    Fl_Group* mech_grp = new Vsp_Group( rx, ry, rw, rh );
    mech_grp->copy_label( "Mechanism (1/2)" );
    mech_grp->selection_color( FL_GRAY );
    mech_grp->labelfont( 1 );
    mech_grp->labelcolor( FL_BLACK );

    m_RetractTabs->add( mech_grp );

    Fl_Group* mech_sub_group = new Fl_Group( x, y, w, h );
    mech_grp->add( mech_sub_group );



    Fl_Group* mech2_grp = new Vsp_Group( rx, ry, rw, rh );
    mech2_grp->copy_label( "Mechanism (2/2)" );
    mech2_grp->selection_color( FL_GRAY );
    mech2_grp->labelfont( 1 );
    mech2_grp->labelcolor( FL_BLACK );

    m_RetractTabs->add( mech2_grp );

    Fl_Group* mech2_sub_group = new Fl_Group( x, y, w, h );
    mech2_grp->add( mech2_sub_group );



    stow_grp->show();

    // Add m_StowLayout to tab.
    m_StowLayout.SetGroupAndScreen( stow_sub_group, this );
    m_MechanismLayout.SetGroupAndScreen( mech_sub_group, this );
    m_Mechanism2Layout.SetGroupAndScreen( mech2_sub_group, this );


    m_StowLayout.AddYGap();

    m_StowLayout.AddDividerBox( "Transforms" );


    m_StowLayout.SetFitWidthFlag( false );
    m_StowLayout.SetSameLineFlag( true );
    m_StowLayout.AddLabel( "Coord System:", m_StowLayout.GetW() - 2 * m_StowLayout.GetInputWidth() );
    m_StowLayout.SetButtonWidth( m_StowLayout.GetInputWidth() );
    m_StowLayout.AddButton( m_StowXFormRelativeToggle, "Rel" );
    m_StowLayout.AddButton( m_StowXFormAbsoluteToggle, "Abs" );
    m_StowLayout.ForceNewLine();

    m_StowXFormAbsRelToggle.Init( this );
    m_StowXFormAbsRelToggle.AddButton( m_StowXFormAbsoluteToggle.GetFlButton() );
    m_StowXFormAbsRelToggle.AddButton( m_StowXFormRelativeToggle.GetFlButton() );

    m_StowLayout.SetFitWidthFlag( true );
    m_StowLayout.SetSameLineFlag( false );
    m_StowLayout.AddYGap();

    m_StowLayout.SetButtonWidth( 110 );
    m_StowLayout.AddSlider( m_StowXLocSlider, "XLoc", 10.0, "%7.3f" );
    m_StowLayout.AddSlider( m_StowYLocSlider, "YLoc", 10.0, "%7.3f" );
    m_StowLayout.AddSlider( m_StowZLocSlider, "ZLoc", 10.0, "%7.3f" );
    m_StowLayout.AddYGap();
    m_StowLayout.AddSlider( m_StowXRotSlider, "XRot", 10.0, "%7.3f" );
    m_StowLayout.AddSlider( m_StowYRotSlider, "YRot", 10.0, "%7.3f" );
    m_StowLayout.AddSlider( m_StowZRotSlider, "ZRot", 10.0, "%7.3f" );
    m_StowLayout.AddYGap();

    m_StowLayout.InitWidthHeightVals();
    m_StowLayout.SetFitWidthFlag( true );
    m_StowLayout.SetSameLineFlag( false );
    m_StowLayout.AddDividerBox( "Attach To Parent" );

    m_StowGeomPicker.AddExcludeType( PT_CLOUD_GEOM_TYPE );
    m_StowLayout.AddGeomPicker( m_StowGeomPicker );

    m_StowLayout.AddChoice( m_StowSurfChoice, "Surface" );

    // init AttachLayout and AttachLayoutSub; hierarchy permits leaving none buttons active on orphaned attach geoms
    int labelw = 74;
    int buttonw = ( m_StowLayout.GetW() - labelw ) / 6;
    m_StowLayout.AddSubGroupLayout( m_StowAttachLayout, m_StowLayout.GetW(), 11 * m_StowAttachLayout.GetStdHeight() + 5 * m_StowAttachLayout.GetGapHeight() );
    m_StowAttachLayout.AddSubGroupLayout( m_StowAttachLayoutSelections, m_StowAttachLayout.GetW(), 11 * m_StowAttachLayout.GetStdHeight() + 5 * m_StowAttachLayout.GetGapHeight() );
    m_StowAttachLayout.AddSubGroupLayout( m_StowAttachLayoutTransHeader, buttonw + labelw, m_StowAttachLayout.GetStdHeight() );
    m_StowAttachLayout.ForceNewLine();
    m_StowAttachLayout.AddYGap();
    m_StowAttachLayout.AddSubGroupLayout( m_StowAttachLayoutRotHeader, buttonw + labelw, m_StowAttachLayout.GetStdHeight() );

    m_StowAttachLayoutTransHeader.SetButtonWidth( buttonw );
    m_StowAttachLayoutTransHeader.SetFitWidthFlag( false );
    m_StowAttachLayoutTransHeader.SetSameLineFlag( true );

    m_StowAttachLayoutRotHeader.SetButtonWidth( buttonw );
    m_StowAttachLayoutRotHeader.SetFitWidthFlag( false );
    m_StowAttachLayoutRotHeader.SetSameLineFlag( true );

    m_StowAttachLayoutSelections.SetButtonWidth( buttonw );
    m_StowAttachLayoutSelections.SetFitWidthFlag( false );
    m_StowAttachLayoutSelections.SetSameLineFlag( true );

    char etaMN[7];
    int indx = 0;
    indx += fl_utf8encode( 951, &etaMN[ indx ] ); // Greek character eta
    etaMN[ indx ] = 'M';
    etaMN[ indx + 1 ] = 'N';
    etaMN[ indx + 2 ] = 0;

    m_StowAttachLayoutTransHeader.AddLabel( "Translate:", labelw );
    m_StowAttachLayoutTransHeader.AddButton( m_StowTransNoneButton, "Contact" );

    m_StowAttachLayoutSelections.AddX( labelw + buttonw );
    m_StowAttachLayoutSelections.AddButton( m_StowTransCompButton, "Comp" );
    m_StowAttachLayoutSelections.AddButton( m_StowTransUVButton, "UW" );
    m_StowAttachLayoutSelections.AddButton( m_StowTransRSTButton, "RST" );
    m_StowAttachLayoutSelections.AddButton( m_StowTransLMNButton, "LMN" );
    m_StowAttachLayoutSelections.AddButton( m_StowTransEtaMNButton, etaMN );
    m_StowAttachLayoutSelections.ForceNewLine();
    m_StowAttachLayoutSelections.AddYGap();

    m_StowTransToggleGroup.Init( this );
    m_StowTransToggleGroup.AddButton( m_StowTransNoneButton.GetFlButton() );
    m_StowTransToggleGroup.AddButton( m_StowTransCompButton.GetFlButton() );
    m_StowTransToggleGroup.AddButton( m_StowTransUVButton.GetFlButton() );
    m_StowTransToggleGroup.AddButton( m_StowTransRSTButton.GetFlButton() );
    m_StowTransToggleGroup.AddButton( m_StowTransLMNButton.GetFlButton() );
    m_StowTransToggleGroup.AddButton( m_StowTransEtaMNButton.GetFlButton() );

    m_StowAttachLayoutRotHeader.AddLabel( "Rotate:", labelw );
    m_StowAttachLayoutRotHeader.AddButton( m_StowRotNoneButton, "Contact" );

    m_StowAttachLayoutSelections.AddX( labelw + buttonw );
    m_StowAttachLayoutSelections.AddButton( m_StowRotCompButton, "Comp" );
    m_StowAttachLayoutSelections.AddButton( m_StowRotUVButton, "UW" );
    m_StowAttachLayoutSelections.AddButton( m_StowRotRSTButton, "RST" );
    m_StowAttachLayoutSelections.AddButton( m_StowRotLMNButton, "LMN" );
    m_StowAttachLayoutSelections.AddButton( m_StowRotEtaMNButton, etaMN );
    m_StowAttachLayoutSelections.ForceNewLine();
    m_StowAttachLayoutSelections.AddYGap();

    m_StowRotToggleGroup.Init( this );
    m_StowRotToggleGroup.AddButton( m_StowRotNoneButton.GetFlButton() );
    m_StowRotToggleGroup.AddButton( m_StowRotCompButton.GetFlButton() );
    m_StowRotToggleGroup.AddButton( m_StowRotUVButton.GetFlButton() );
    m_StowRotToggleGroup.AddButton( m_StowRotRSTButton.GetFlButton() );
    m_StowRotToggleGroup.AddButton( m_StowRotLMNButton.GetFlButton() );
    m_StowRotToggleGroup.AddButton( m_StowRotEtaMNButton.GetFlButton() );


    m_StowAttachLayoutSelections.SetButtonWidth( actionToggleButtonWidth );
    m_StowAttachLayoutSelections.AddButton( m_StowU01Toggle, "01" );
    m_StowAttachLayoutSelections.AddButton( m_StowU0NToggle, "0N" );

    m_StowAttachLayoutSelections.SetFitWidthFlag( true );

    m_StowAttachLayoutSelections.SetButtonWidth( normalButtonWidth - 2 * actionToggleButtonWidth );
    m_StowAttachLayoutSelections.AddSlider( m_StowAttachUSlider, "U", 1, " %7.6f" );

    m_StowUScaleToggleGroup.Init( this );
    m_StowUScaleToggleGroup.AddButton( m_StowU0NToggle.GetFlButton() ); // 0 false added first
    m_StowUScaleToggleGroup.AddButton( m_StowU01Toggle.GetFlButton() ); // 1 true added second

    m_StowAttachLayoutSelections.ForceNewLine();
    m_StowAttachLayoutSelections.SetFitWidthFlag( true );
    m_StowAttachLayoutSelections.SetSameLineFlag( false );

    m_StowAttachLayoutSelections.SetButtonWidth( normalButtonWidth );

    m_StowAttachLayoutSelections.AddSlider( m_StowAttachVSlider, "W", 1, " %7.6f" );
    m_StowAttachLayoutSelections.AddYGap();

    m_StowAttachLayoutSelections.SetFitWidthFlag( false );
    m_StowAttachLayoutSelections.SetSameLineFlag( true );

    m_StowAttachLayoutSelections.SetButtonWidth( actionToggleButtonWidth );
    m_StowAttachLayoutSelections.AddButton( m_StowR01Toggle, "01" );
    m_StowAttachLayoutSelections.AddButton( m_StowR0NToggle, "0N" );

    m_StowAttachLayoutSelections.SetFitWidthFlag( true );

    m_StowAttachLayoutSelections.SetButtonWidth( normalButtonWidth - 2 * actionToggleButtonWidth );
    m_StowAttachLayoutSelections.AddSlider( m_StowAttachRSlider, "R", 1, " %7.6f" );

    m_StowRScaleToggleGroup.Init( this );
    m_StowRScaleToggleGroup.AddButton( m_StowR0NToggle.GetFlButton() ); // 0 false added first
    m_StowRScaleToggleGroup.AddButton( m_StowR01Toggle.GetFlButton() ); // 1 true added second

    m_StowAttachLayoutSelections.ForceNewLine();
    m_StowAttachLayoutSelections.SetFitWidthFlag( true );
    m_StowAttachLayoutSelections.SetSameLineFlag( false );

    m_StowAttachLayoutSelections.SetButtonWidth( normalButtonWidth );

    m_StowAttachLayoutSelections.AddSlider( m_StowAttachSSlider, "S", 1, " %7.6f" );
    m_StowAttachLayoutSelections.AddSlider( m_StowAttachTSlider, "T", 1, " %7.6f" );
    m_StowAttachLayoutSelections.AddYGap();

    m_StowAttachLayoutSelections.SetFitWidthFlag( false );
    m_StowAttachLayoutSelections.SetSameLineFlag( true );

    m_StowAttachLayoutSelections.SetButtonWidth( actionToggleButtonWidth );
    m_StowAttachLayoutSelections.AddButton( m_StowL01Toggle, "01" );
    m_StowAttachLayoutSelections.AddButton( m_StowL0LenToggle, "0D" );

    m_StowAttachLayoutSelections.SetFitWidthFlag( true );

    m_StowAttachLayoutSelections.SetButtonWidth( normalButtonWidth - 2 * actionToggleButtonWidth );
    m_StowAttachLayoutSelections.AddSlider( m_StowAttachLSlider, "L", 1, " %7.6f" );

    m_StowLScaleToggleGroup.Init( this );
    m_StowLScaleToggleGroup.AddButton( m_StowL0LenToggle.GetFlButton() ); // 0 false added first
    m_StowLScaleToggleGroup.AddButton( m_StowL01Toggle.GetFlButton() ); // 1 true added second

    m_StowAttachLayoutSelections.ForceNewLine();
    m_StowAttachLayoutSelections.SetFitWidthFlag( true );
    m_StowAttachLayoutSelections.SetSameLineFlag( false );

    m_StowAttachLayoutSelections.SetButtonWidth( normalButtonWidth );

    char eta[5];
    indx = 0;
    indx += fl_utf8encode( 951, &eta[ indx ] ); // Greek character eta
    eta[ indx ] = 0;

    m_StowAttachLayoutSelections.AddSlider( m_StowAttachEtaSlider, eta, 1, " %7.6f" );

    m_StowAttachLayoutSelections.AddSlider( m_StowAttachMSlider, "M", 1, " %7.6f" );
    m_StowAttachLayoutSelections.AddSlider( m_StowAttachNSlider, "N", 1, " %7.6f" );


    m_MechanismLayout.SetFitWidthFlag( true );
    m_MechanismLayout.SetSameLineFlag( false );


    m_MechanismLayout.SetButtonWidth( normalButtonWidth );

    m_MechanismLayout.AddDividerBox( "Intermediate Position" );
    m_MechanismLayout.AddSlider( m_MechKRetractSlider, "Retract", 10.0, "%7.3f" );

    m_MechanismLayout.AddYGap();
    m_MechanismLayout.AddDividerBox( "Primary Motion" );
    m_MechanismLayout.AddSlider( m_MechRetAngleSlider, "Retract Angle", 10.0, "%7.3f" );

    m_MechanismLayout.AddYGap();

    m_MechanismLayout.AddDividerBox( "Trunnion Point" );

    m_MechanismLayout.SetFitWidthFlag( false );
    m_MechanismLayout.SetSameLineFlag( true );
    m_MechanismLayout.AddLabel( "Coord System:", m_MechanismLayout.GetW() - 2 * m_MechanismLayout.GetInputWidth() );
    m_MechanismLayout.SetButtonWidth( m_StowLayout.GetInputWidth() );
    m_MechanismLayout.AddButton( m_MechXFormRelativeToggle, "Rel" );
    m_MechanismLayout.AddButton( m_MechXFormAbsoluteToggle, "Abs" );
    m_MechanismLayout.ForceNewLine();

    m_MechXFormAbsRelToggle.Init( this );
    m_MechXFormAbsRelToggle.AddButton( m_MechXFormAbsoluteToggle.GetFlButton() );
    m_MechXFormAbsRelToggle.AddButton( m_MechXFormRelativeToggle.GetFlButton() );

    m_MechanismLayout.SetFitWidthFlag( true );
    m_MechanismLayout.SetSameLineFlag( false );
    m_MechanismLayout.AddYGap();

    m_MechanismLayout.SetButtonWidth( normalButtonWidth );

    m_MechanismLayout.AddSlider( m_MechXLocSlider, "XLoc", 10.0, "%7.3f" );
    m_MechanismLayout.AddSlider( m_MechYLocSlider, "YLoc", 10.0, "%7.3f" );
    m_MechanismLayout.AddSlider( m_MechZLocSlider, "ZLoc", 10.0, "%7.3f" );
    m_MechanismLayout.AddYGap();

    m_MechanismLayout.InitWidthHeightVals();
    m_MechanismLayout.SetFitWidthFlag( true );
    m_MechanismLayout.SetSameLineFlag( false );
    m_MechanismLayout.AddDividerBox( "Attach To Parent" );

    m_MechGeomPicker.AddExcludeType( PT_CLOUD_GEOM_TYPE );
    m_MechanismLayout.AddGeomPicker( m_MechGeomPicker );

    m_MechanismLayout.AddChoice( m_MechSurfChoice, "Surface" );

    // init AttachLayout and AttachLayoutSub; hierarchy permits leaving none buttons active on orphaned attach geoms
    labelw = 74;
    buttonw = ( m_MechanismLayout.GetW() - labelw ) / 6;
    m_MechanismLayout.AddSubGroupLayout( m_MechAttachLayout, m_MechanismLayout.GetW(), 10 * m_MechAttachLayout.GetStdHeight() + 3 * m_MechAttachLayout.GetGapHeight() );
    m_MechAttachLayout.AddSubGroupLayout( m_MechAttachLayoutTransHeader, buttonw + labelw, m_MechAttachLayout.GetStdHeight() );
    m_MechAttachLayout.AddSubGroupLayout( m_MechAttachLayoutSelections, m_MechAttachLayout.GetW(), 9 * m_MechAttachLayout.GetStdHeight() + 3 * m_MechAttachLayout.GetGapHeight() );

    m_MechAttachLayout.ForceNewLine();
    m_MechAttachLayout.AddYGap();

    m_MechanismLayout.AddY( m_MechAttachLayout.GetH() );

    m_MechAttachLayoutTransHeader.SetButtonWidth( buttonw );
    m_MechAttachLayoutTransHeader.SetFitWidthFlag( false );
    m_MechAttachLayoutTransHeader.SetSameLineFlag( true );

    m_MechAttachLayoutSelections.SetButtonWidth( buttonw );
    m_MechAttachLayoutSelections.SetFitWidthFlag( false );
    m_MechAttachLayoutSelections.SetSameLineFlag( true );

    indx = 0;
    indx += fl_utf8encode( 951, &etaMN[ indx ] ); // Greek character eta
    etaMN[ indx ] = 'M';
    etaMN[ indx + 1 ] = 'N';
    etaMN[ indx + 2 ] = 0;

    m_MechAttachLayoutTransHeader.AddLabel( "Translate:", labelw );
    m_MechAttachLayoutTransHeader.AddButton( m_MechTransNoneButton, "Contact" );

    m_MechAttachLayoutSelections.AddX( labelw + buttonw );
    m_MechAttachLayoutSelections.AddButton( m_MechTransCompButton, "Comp" );
    m_MechAttachLayoutSelections.AddButton( m_MechTransUVButton, "UW" );
    m_MechAttachLayoutSelections.AddButton( m_MechTransRSTButton, "RST" );
    m_MechAttachLayoutSelections.AddButton( m_MechTransLMNButton, "LMN" );
    m_MechAttachLayoutSelections.AddButton( m_MechTransEtaMNButton, etaMN );
    m_MechAttachLayoutSelections.ForceNewLine();
    m_MechAttachLayoutSelections.AddYGap();

    m_MechTransToggleGroup.Init( this );
    m_MechTransToggleGroup.AddButton( m_MechTransNoneButton.GetFlButton() );
    m_MechTransToggleGroup.AddButton( m_MechTransCompButton.GetFlButton() );
    m_MechTransToggleGroup.AddButton( m_MechTransUVButton.GetFlButton() );
    m_MechTransToggleGroup.AddButton( m_MechTransRSTButton.GetFlButton() );
    m_MechTransToggleGroup.AddButton( m_MechTransLMNButton.GetFlButton() );
    m_MechTransToggleGroup.AddButton( m_MechTransEtaMNButton.GetFlButton() );

    m_MechAttachLayoutSelections.SetButtonWidth( actionToggleButtonWidth );
    m_MechAttachLayoutSelections.AddButton( m_MechU01Toggle, "01" );
    m_MechAttachLayoutSelections.AddButton( m_MechU0NToggle, "0N" );

    m_MechAttachLayoutSelections.SetFitWidthFlag( true );

    m_MechAttachLayoutSelections.SetButtonWidth( normalButtonWidth - 2 * actionToggleButtonWidth );
    m_MechAttachLayoutSelections.AddSlider( m_MechAttachUSlider, "U", 1, " %7.6f" );

    m_MechUScaleToggleGroup.Init( this );
    m_MechUScaleToggleGroup.AddButton( m_MechU0NToggle.GetFlButton() ); // 0 false added first
    m_MechUScaleToggleGroup.AddButton( m_MechU01Toggle.GetFlButton() ); // 1 true added second

    m_MechAttachLayoutSelections.ForceNewLine();
    m_MechAttachLayoutSelections.SetFitWidthFlag( true );
    m_MechAttachLayoutSelections.SetSameLineFlag( false );

    m_MechAttachLayoutSelections.SetButtonWidth( normalButtonWidth );

    m_MechAttachLayoutSelections.AddSlider( m_MechAttachVSlider, "W", 1, " %7.6f" );
    m_MechAttachLayoutSelections.AddYGap();

    m_MechAttachLayoutSelections.SetFitWidthFlag( false );
    m_MechAttachLayoutSelections.SetSameLineFlag( true );

    m_MechAttachLayoutSelections.SetButtonWidth( actionToggleButtonWidth );
    m_MechAttachLayoutSelections.AddButton( m_MechR01Toggle, "01" );
    m_MechAttachLayoutSelections.AddButton( m_MechR0NToggle, "0N" );

    m_MechAttachLayoutSelections.SetFitWidthFlag( true );

    m_MechAttachLayoutSelections.SetButtonWidth( normalButtonWidth - 2 * actionToggleButtonWidth );
    m_MechAttachLayoutSelections.AddSlider( m_MechAttachRSlider, "R", 1, " %7.6f" );

    m_MechRScaleToggleGroup.Init( this );
    m_MechRScaleToggleGroup.AddButton( m_MechR0NToggle.GetFlButton() ); // 0 false added first
    m_MechRScaleToggleGroup.AddButton( m_MechR01Toggle.GetFlButton() ); // 1 true added second

    m_MechAttachLayoutSelections.ForceNewLine();
    m_MechAttachLayoutSelections.SetFitWidthFlag( true );
    m_MechAttachLayoutSelections.SetSameLineFlag( false );

    m_MechAttachLayoutSelections.SetButtonWidth( normalButtonWidth );

    m_MechAttachLayoutSelections.AddSlider( m_MechAttachSSlider, "S", 1, " %7.6f" );
    m_MechAttachLayoutSelections.AddSlider( m_MechAttachTSlider, "T", 1, " %7.6f" );
    m_MechAttachLayoutSelections.AddYGap();

    m_MechAttachLayoutSelections.SetFitWidthFlag( false );
    m_MechAttachLayoutSelections.SetSameLineFlag( true );

    m_MechAttachLayoutSelections.SetButtonWidth( actionToggleButtonWidth );
    m_MechAttachLayoutSelections.AddButton( m_MechL01Toggle, "01" );
    m_MechAttachLayoutSelections.AddButton( m_MechL0LenToggle, "0D" );

    m_MechAttachLayoutSelections.SetFitWidthFlag( true );

    m_MechAttachLayoutSelections.SetButtonWidth( normalButtonWidth - 2 * actionToggleButtonWidth );
    m_MechAttachLayoutSelections.AddSlider( m_MechAttachLSlider, "L", 1, " %7.6f" );

    m_MechLScaleToggleGroup.Init( this );
    m_MechLScaleToggleGroup.AddButton( m_MechL0LenToggle.GetFlButton() ); // 0 false added first
    m_MechLScaleToggleGroup.AddButton( m_MechL01Toggle.GetFlButton() ); // 1 true added second

    m_MechAttachLayoutSelections.ForceNewLine();
    m_MechAttachLayoutSelections.SetFitWidthFlag( true );
    m_MechAttachLayoutSelections.SetSameLineFlag( false );

    m_MechAttachLayoutSelections.SetButtonWidth( normalButtonWidth );

    indx = 0;
    indx += fl_utf8encode( 951, &eta[ indx ] ); // Greek character eta
    eta[ indx ] = 0;

    m_MechAttachLayoutSelections.AddSlider( m_MechAttachEtaSlider, eta, 1, " %7.6f" );

    m_MechAttachLayoutSelections.AddSlider( m_MechAttachMSlider, "M", 1, " %7.6f" );
    m_MechAttachLayoutSelections.AddSlider( m_MechAttachNSlider, "N", 1, " %7.6f" );

    m_MechanismLayout.AddYGap();

    m_MechanismLayout.AddDividerBox( "Trunnion Axis" );

    m_MechanismLayout.SetButtonWidth( normalButtonWidth );

    m_MechanismLayout.AddSlider( m_MechXAxisSlider, "XDir", 10.0, "%7.3f" );
    m_MechanismLayout.AddSlider( m_MechYAxisSlider, "YDir", 10.0, "%7.3f" );
    m_MechanismLayout.AddSlider( m_MechZAxisSlider, "ZDir", 10.0, "%7.3f" );
    m_MechanismLayout.AddYGap();


    m_Mechanism2Layout.SetButtonWidth( normalButtonWidth );

    m_Mechanism2Layout.AddDividerBox( "Intermediate Position" );
    m_Mechanism2Layout.AddSlider( m_Mech2KRetractSlider, "Retract", 10.0, "%7.3f" );

    m_Mechanism2Layout.AddYGap();
    m_Mechanism2Layout.AddDividerBox( "Knee" );
    m_Mechanism2Layout.AddSlider( m_MechKneePosSlider, "Position", 10.0, "%7.3f" );
    m_Mechanism2Layout.AddSlider( m_MechKneeAngleSlider, "Rotation", 10.0, "%7.3f" );
    m_Mechanism2Layout.AddSlider( m_MechKneeDownAngleSlider, "Bend Angle", 10.0, "%7.3f" );
    m_Mechanism2Layout.AddSlider( m_MechKneeClockAngleSlider, "Azimuth Angle", 10.0, "%7.3f" );
    m_Mechanism2Layout.AddSlider( m_MechKneeElevationAngleSlider, "Elevation Angle", 10.0, "%7.3f" );

    m_Mechanism2Layout.AddYGap();
    m_Mechanism2Layout.AddDividerBox( "Wrist" );
    m_Mechanism2Layout.AddSlider( m_MechTwistAngleSlider, "Twist Angle", 10.0, "%7.3f" );
    m_Mechanism2Layout.AddSlider( m_MechRollAngleSlider, "Roll Angle", 10.0, "%7.3f" );
    m_Mechanism2Layout.AddSlider( m_MechBogieAngleSlider, "Bogie Angle", 10.0, "%7.3f" );
    m_Mechanism2Layout.AddSlider( m_MechStrutDLSlider, "Strut Extension", 10.0, "%7.3f" );

}


//==== Show Pod Screen ====//
void GearScreen::Show()
{
    if ( Update() )
    {
        m_FLTK_Window->show();
    }
}

//==== Update Disk Screen ====//
bool GearScreen::Update()
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

    m_GearConfigurationChoice.Update( gear_ptr->m_GearConfigMode.GetID() );

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

    UpdateStowSurfChoice();

    UpdateMechSurfChoice();


    Bogie* bogie_ptr = gear_ptr->GetCurrentBogie();

    if ( bogie_ptr )
    {
        // Bogie
        m_BogieNameInput.Update( bogie_ptr->GetName() );

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
        m_SteeringAngleSlider.Activate();
        m_TravelCompressSlider.Activate();
        m_TravelExtendSlider.Activate();
        m_BogieThetaMaxSlider.Activate();
        m_BogieThetaMinSlider.Activate();

        // Tire
        m_TireDiameterToggleGroup.Activate();
        m_TireDiameterSlider.Activate();
        m_TireWidthToggleGroup.Activate();
        m_TireWidthSlider.Activate();
        m_TireModeChoice.Activate();
        m_DrimToggleGroup.Activate();
        m_DrimSlider.Activate();
        m_WrimToggleGroup.Activate();
        m_WrimSlider.Activate();
        m_PlyRatingSlider.Activate();
        m_SpeedRatingSlider.Activate();
        m_HsToggleGroup.Activate();
        m_HsSlider.Activate();
        m_WsToggleGroup.Activate();
        m_WsSlider.Activate();
        m_SLRToggleGroup.Activate();
        m_DeflectionSlider.Activate();
        m_SLRSlider.Activate();

        m_StowXFormAbsRelToggle.Activate();
        m_StowXLocSlider.Activate();
        m_StowYLocSlider.Activate();
        m_StowZLocSlider.Activate();
        m_StowXRotSlider.Activate();
        m_StowYRotSlider.Activate();
        m_StowZRotSlider.Activate();
        m_StowTransToggleGroup.Activate();
        m_StowRotToggleGroup.Activate();
        m_StowUScaleToggleGroup.Activate();
        m_StowRScaleToggleGroup.Activate();
        m_StowLScaleToggleGroup.Activate();
        m_StowAttachUSlider.Activate();
        m_StowAttachVSlider.Activate();
        m_StowAttachRSlider.Activate();
        m_StowAttachSSlider.Activate();
        m_StowAttachTSlider.Activate();
        m_StowAttachLSlider.Activate();
        m_StowAttachMSlider.Activate();
        m_StowAttachNSlider.Activate();
        m_StowAttachEtaSlider.Activate();

        m_MechXFormAbsRelToggle.Activate();
        m_MechXLocSlider.Activate();
        m_MechYLocSlider.Activate();
        m_MechZLocSlider.Activate();
        m_MechTransToggleGroup.Activate();
        m_MechUScaleToggleGroup.Activate();
        m_MechRScaleToggleGroup.Activate();
        m_MechLScaleToggleGroup.Activate();
        m_MechAttachUSlider.Activate();
        m_MechAttachVSlider.Activate();
        m_MechAttachRSlider.Activate();
        m_MechAttachSSlider.Activate();
        m_MechAttachTSlider.Activate();
        m_MechAttachLSlider.Activate();
        m_MechAttachMSlider.Activate();
        m_MechAttachNSlider.Activate();
        m_MechAttachEtaSlider.Activate();

        m_StowLayout.GetGroup()->activate();
        m_MechanismLayout.GetGroup()->activate();
        m_Mechanism2Layout.GetGroup()->activate();

        if ( bogie_ptr->m_RetMode() == vsp::GEAR_STOWED_POSITION )
        {
            m_MechanismLayout.GetGroup()->deactivate();
            m_Mechanism2Layout.GetGroup()->deactivate();
        }
        else
        {
            m_StowLayout.GetGroup()->deactivate();
        }

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

        m_TireModeChoice.Update( bogie_ptr->m_TireMode.GetID() );

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
        m_SpeedRatingSlider.Update( bogie_ptr->m_SpeedRating.GetID() );

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
            m_DeflectionSlider.SetButtonName(string(""));

            m_SLRSlider.Activate();
            m_SLRSlider.ActivateInput1();
            m_DeflectionSlider.Deactivate();
        }
        else if ( bogie_ptr->m_SLRMode() == vsp::TIRE_DIM_MODEL )
        {
            m_SLRSlider.Update( 2, bogie_ptr->m_StaticRadiusIn.GetID(), bogie_ptr->m_StaticRadiusModel.GetID() );
            m_DeflectionSlider.Update( bogie_ptr->m_DeflectionPct.GetID() );
            m_DeflectionSlider.SetButtonName(string(""));

            m_SLRSlider.Activate();
            m_SLRSlider.ActivateInput2();
            m_DeflectionSlider.Deactivate();
        }
        else // TIRE_DIM_FRAC
        {
            m_SLRSlider.Update( 1, bogie_ptr->m_StaticRadiusIn.GetID(), bogie_ptr->m_StaticRadiusModel.GetID() );
            m_DeflectionSlider.Update( bogie_ptr->m_DeflectionPct.GetID() );
            m_DeflectionSlider.SetButtonName(string(""));

            m_SLRSlider.Deactivate();
            m_DeflectionSlider.Activate();
        }


        if ( bogie_ptr->m_TireMode() == vsp::TIRE_FAIR_WHEEL )
        {
            m_WrimToggleGroup.Deactivate();
            m_WrimSlider.Deactivate();

            m_PlyRatingSlider.Deactivate();
        }
        else if ( bogie_ptr->m_TireMode() == vsp::TIRE_BALLOON ||
                  bogie_ptr->m_TireMode() == vsp::TIRE_BALLOON_WHEEL ||
                  bogie_ptr->m_TireMode() == vsp::TIRE_BALLOON_FAIR_WHEEL )
        {
            m_WrimToggleGroup.Deactivate();
            m_WrimSlider.Deactivate();

            m_PlyRatingSlider.Deactivate();
            m_SpeedRatingSlider.Deactivate();
            m_HsToggleGroup.Deactivate();
            m_HsSlider.Deactivate();
            m_WsToggleGroup.Deactivate();
            m_WsSlider.Deactivate();

            m_SLRToggleGroup.Deactivate();
            m_DeflectionSlider.Deactivate();
            m_SLRSlider.Deactivate();

            if ( bogie_ptr->m_TireMode() == vsp::TIRE_BALLOON ||
                 bogie_ptr->m_TireMode() == vsp::TIRE_BALLOON_FAIR_WHEEL )
            {
                m_DrimToggleGroup.Deactivate();
                m_DrimSlider.Deactivate();
            }
        }


        Vehicle* veh = VehicleMgr.GetVehicle();

        bool wing_parent = false;
        bool routing_parent = false;
        bool orphaned_trans = false;
        bool orphaned_rot = false;

        if ( veh )
        {
            Geom* parent = veh->FindGeom( bogie_ptr->GetStowParentID() );

            if ( parent )
            {
                WingGeom* wing_ptr = dynamic_cast< WingGeom* >( parent );
                if ( wing_ptr )
                {
                    wing_parent = true;
                }

                RoutingGeom* routing_ptr = dynamic_cast< RoutingGeom* >( parent );
                if ( routing_ptr )
                {
                    routing_parent = true;
                }
            }
        }

        m_RetModeChoice.Update( bogie_ptr->m_RetMode.GetID() );


        m_StowGeomPicker.SetGeomChoice( bogie_ptr->GetStowParentID() );

        m_StowSurfChoice.Update( bogie_ptr->m_StowSurfIndx.GetID() );


        //===== Rel of Abs ====//
        m_StowXFormAbsRelToggle.Update( bogie_ptr->m_StowAbsRelFlag.GetID() );
        if ( bogie_ptr->m_StowAbsRelFlag() ==  vsp::REL )
        {
            m_StowXLocSlider.Update( 1, bogie_ptr->m_StowXRelLoc.GetID(), bogie_ptr->m_StowXLoc.GetID() );
            m_StowYLocSlider.Update( 1, bogie_ptr->m_StowYRelLoc.GetID(), bogie_ptr->m_StowYLoc.GetID() );
            m_StowZLocSlider.Update( 1, bogie_ptr->m_StowZRelLoc.GetID(), bogie_ptr->m_StowZLoc.GetID() );
            m_StowXRotSlider.Update( 1, bogie_ptr->m_StowXRelRot.GetID(), bogie_ptr->m_StowXRot.GetID() );
            m_StowYRotSlider.Update( 1, bogie_ptr->m_StowYRelRot.GetID(), bogie_ptr->m_StowYRot.GetID() );
            m_StowZRotSlider.Update( 1, bogie_ptr->m_StowZRelRot.GetID(), bogie_ptr->m_StowZRot.GetID() );
        }
        else
        {
            m_StowXLocSlider.Update( 2, bogie_ptr->m_StowXRelLoc.GetID(), bogie_ptr->m_StowXLoc.GetID() );
            m_StowYLocSlider.Update( 2, bogie_ptr->m_StowYRelLoc.GetID(), bogie_ptr->m_StowYLoc.GetID() );
            m_StowZLocSlider.Update( 2, bogie_ptr->m_StowZRelLoc.GetID(), bogie_ptr->m_StowZLoc.GetID() );
            m_StowXRotSlider.Update( 2, bogie_ptr->m_StowXRelRot.GetID(), bogie_ptr->m_StowXRot.GetID() );
            m_StowYRotSlider.Update( 2, bogie_ptr->m_StowYRelRot.GetID(), bogie_ptr->m_StowYRot.GetID() );
            m_StowZRotSlider.Update( 2, bogie_ptr->m_StowZRelRot.GetID(), bogie_ptr->m_StowZRot.GetID() );
        }

        //==== Attachments ====//
        m_StowAttachUSlider.Activate();
        m_StowUScaleToggleGroup.Activate();
        m_StowAttachVSlider.Activate();
        m_StowAttachRSlider.Activate();
        m_StowRScaleToggleGroup.Activate();
        m_StowAttachSSlider.Activate();
        m_StowAttachTSlider.Activate();
        m_StowAttachLSlider.Activate();
        m_StowLScaleToggleGroup.Activate();
        m_StowAttachMSlider.Activate();
        m_StowAttachNSlider.Activate();
        m_StowAttachEtaSlider.Activate();

        m_StowTransToggleGroup.Update( bogie_ptr->m_StowTransAttachFlag.GetID() );
        m_StowRotToggleGroup.Update( bogie_ptr->m_StowRotAttachFlag.GetID() );
        m_StowUScaleToggleGroup.Update( bogie_ptr->m_StowU01.GetID() );

        if ( bogie_ptr->m_StowU01() )
        {
            m_StowAttachUSlider.Update( 1, bogie_ptr->m_StowULoc.GetID(), bogie_ptr->m_StowU0NLoc.GetID());
        }
        else
        {
            m_StowAttachUSlider.Update( 2, bogie_ptr->m_StowULoc.GetID(), bogie_ptr->m_StowU0NLoc.GetID());
        }

        m_StowAttachVSlider.Update( bogie_ptr->m_StowWLoc.GetID() );
        m_StowRScaleToggleGroup.Update( bogie_ptr->m_StowR01.GetID() );

        if ( bogie_ptr->m_StowR01() )
        {
            m_StowAttachRSlider.Update( 1, bogie_ptr->m_StowRLoc.GetID(), bogie_ptr->m_StowR0NLoc.GetID());
        }
        else
        {
            m_StowAttachRSlider.Update( 2, bogie_ptr->m_StowRLoc.GetID(), bogie_ptr->m_StowR0NLoc.GetID());
        }

        m_StowAttachSSlider.Update( bogie_ptr->m_StowSLoc.GetID() );
        m_StowAttachTSlider.Update( bogie_ptr->m_StowTLoc.GetID() );
        m_StowLScaleToggleGroup.Update( bogie_ptr->m_StowL01.GetID() );

        if ( bogie_ptr->m_StowL01() )
        {
            m_StowAttachLSlider.Update( 1, bogie_ptr->m_StowLLoc.GetID(), bogie_ptr->m_StowL0LenLoc.GetID());
        }
        else
        {
            m_StowAttachLSlider.Update( 2, bogie_ptr->m_StowLLoc.GetID(), bogie_ptr->m_StowL0LenLoc.GetID());
        }

        m_StowAttachMSlider.Update( bogie_ptr->m_StowMLoc.GetID() );
        m_StowAttachNSlider.Update( bogie_ptr->m_StowNLoc.GetID() );

        m_StowAttachEtaSlider.Update( bogie_ptr->m_StowEtaLoc.GetID() );
        if ( bogie_ptr->m_StowTransAttachFlag() != vsp::ATTACH_TRANS_UV && bogie_ptr->m_StowRotAttachFlag() != vsp::ATTACH_ROT_UV )
        {
            m_StowAttachUSlider.Deactivate();
            m_StowUScaleToggleGroup.Deactivate();
            m_StowAttachVSlider.Deactivate();
        }

        if ( bogie_ptr->m_StowTransAttachFlag() != vsp::ATTACH_TRANS_RST && bogie_ptr->m_StowRotAttachFlag() != vsp::ATTACH_ROT_RST )
        {
            m_StowAttachRSlider.Deactivate();
            m_StowRScaleToggleGroup.Deactivate();
            m_StowAttachSSlider.Deactivate();
            m_StowAttachTSlider.Deactivate();
        }

        if ( bogie_ptr->m_StowTransAttachFlag() != vsp::ATTACH_TRANS_LMN && bogie_ptr->m_StowRotAttachFlag() != vsp::ATTACH_ROT_LMN )
        {
            m_StowAttachLSlider.Deactivate();
            m_StowLScaleToggleGroup.Deactivate();
        }

        if ( bogie_ptr->m_StowTransAttachFlag() != vsp::ATTACH_TRANS_LMN && bogie_ptr->m_StowRotAttachFlag() != vsp::ATTACH_ROT_LMN &&
             bogie_ptr->m_StowTransAttachFlag() != vsp::ATTACH_TRANS_EtaMN && bogie_ptr->m_StowRotAttachFlag() != vsp::ATTACH_TRANS_EtaMN )
        {
            m_StowAttachMSlider.Deactivate();
            m_StowAttachNSlider.Deactivate();
        }

        if ( bogie_ptr->m_StowTransAttachFlag() != vsp::ATTACH_TRANS_EtaMN && bogie_ptr->m_StowRotAttachFlag() != vsp::ATTACH_ROT_EtaMN )
        {
            m_StowAttachEtaSlider.Deactivate();
        }

        if ( wing_parent )
        {
            m_StowTransEtaMNButton.Activate();
            m_StowRotEtaMNButton.Activate();
        }
        else
        {
            m_StowTransEtaMNButton.Deactivate();
            m_StowRotEtaMNButton.Deactivate();
            m_StowAttachEtaSlider.Deactivate();
        }

        // figure out proper integration of orphan ctrls deactivation... filter into fine tooth section carefully
        if ( veh )
        {
            Geom* parent = veh->FindGeom( bogie_ptr->GetStowParentID() );

            // Handle orphaned trans/rot attachment flags; ensure the associated header groups are active
            if ( !parent && bogie_ptr->m_StowTransAttachFlag() != vsp::ATTACH_TRANS_NONE )
            {
                orphaned_trans = true;
            }
            if ( !parent && bogie_ptr->m_StowRotAttachFlag() != vsp::ATTACH_TRANS_NONE )
            {
                orphaned_rot = true;
            }

            // Enable Attachment layouts
            m_StowAttachLayout.GetGroup()->activate();
            m_StowAttachLayoutSelections.GetGroup()->activate();
            m_StowAttachLayoutRotHeader.GetGroup()->activate();
            m_StowAttachLayoutTransHeader.GetGroup()->activate();

            // if no parent OR invalid type of parent, hide attach groups
            if ( !( parent && !routing_parent && !bogie_ptr->IsStowParentJoint() ) )
            {
                // it not an orphaned geom, deactivate the whole attach layout
                if ( !( orphaned_trans || orphaned_rot ) )
                {
                    m_StowAttachLayout.GetGroup()->deactivate();
                }
                // if it IS an orphaned geom, deactivate attachment selections, keeping headers open for the orphaned attach type
                else
                {
                    m_StowAttachLayoutSelections.GetGroup()->deactivate();
                    m_StowAttachLayoutRotHeader.GetGroup()->deactivate();
                    m_StowAttachLayoutTransHeader.GetGroup()->deactivate();
                    if ( orphaned_trans )
                    {
                        m_StowAttachLayoutTransHeader.GetGroup()->activate();
                    }
                    if ( orphaned_rot )
                    {
                        m_StowAttachLayoutRotHeader.GetGroup()->activate();
                    }
                }
            }
        }













        if ( veh )
        {
            Geom* parent = veh->FindGeom( bogie_ptr->GetMechParentID() );

            if ( parent )
            {
                WingGeom* wing_ptr = dynamic_cast< WingGeom* >( parent );
                if ( wing_ptr )
                {
                    wing_parent = true;
                }

                RoutingGeom* routing_ptr = dynamic_cast< RoutingGeom* >( parent );
                if ( routing_ptr )
                {
                    routing_parent = true;
                }
            }
        }

        m_RetModeChoice.Update( bogie_ptr->m_RetMode.GetID() );


        m_MechGeomPicker.SetGeomChoice( bogie_ptr->GetMechParentID() );

        m_MechSurfChoice.Update( bogie_ptr->m_MechSurfIndx.GetID() );


        //===== Rel of Abs ====//
        m_MechXFormAbsRelToggle.Update( bogie_ptr->m_MechAbsRelFlag.GetID() );
        if ( bogie_ptr->m_MechAbsRelFlag() ==  vsp::REL )
        {
            m_MechXLocSlider.Update( 1, bogie_ptr->m_MechXRelLoc.GetID(), bogie_ptr->m_MechXLoc.GetID() );
            m_MechYLocSlider.Update( 1, bogie_ptr->m_MechYRelLoc.GetID(), bogie_ptr->m_MechYLoc.GetID() );
            m_MechZLocSlider.Update( 1, bogie_ptr->m_MechZRelLoc.GetID(), bogie_ptr->m_MechZLoc.GetID() );
        }
        else
        {
            m_MechXLocSlider.Update( 2, bogie_ptr->m_MechXRelLoc.GetID(), bogie_ptr->m_MechXLoc.GetID() );
            m_MechYLocSlider.Update( 2, bogie_ptr->m_MechYRelLoc.GetID(), bogie_ptr->m_MechYLoc.GetID() );
            m_MechZLocSlider.Update( 2, bogie_ptr->m_MechZRelLoc.GetID(), bogie_ptr->m_MechZLoc.GetID() );
        }

        //==== Attachments ====//
        m_MechAttachUSlider.Activate();
        m_MechUScaleToggleGroup.Activate();
        m_MechAttachVSlider.Activate();
        m_MechAttachRSlider.Activate();
        m_MechRScaleToggleGroup.Activate();
        m_MechAttachSSlider.Activate();
        m_MechAttachTSlider.Activate();
        m_MechAttachLSlider.Activate();
        m_MechLScaleToggleGroup.Activate();
        m_MechAttachMSlider.Activate();
        m_MechAttachNSlider.Activate();
        m_MechAttachEtaSlider.Activate();

        m_MechTransToggleGroup.Update( bogie_ptr->m_MechTransAttachFlag.GetID() );
        m_MechUScaleToggleGroup.Update( bogie_ptr->m_MechU01.GetID() );

        if ( bogie_ptr->m_MechU01() )
        {
            m_MechAttachUSlider.Update( 1, bogie_ptr->m_MechULoc.GetID(), bogie_ptr->m_MechU0NLoc.GetID());
        }
        else
        {
            m_MechAttachUSlider.Update( 2, bogie_ptr->m_MechULoc.GetID(), bogie_ptr->m_MechU0NLoc.GetID());
        }

        m_MechAttachVSlider.Update( bogie_ptr->m_MechWLoc.GetID() );
        m_MechRScaleToggleGroup.Update( bogie_ptr->m_MechR01.GetID() );

        if ( bogie_ptr->m_MechR01() )
        {
            m_MechAttachRSlider.Update( 1, bogie_ptr->m_MechRLoc.GetID(), bogie_ptr->m_MechR0NLoc.GetID());
        }
        else
        {
            m_MechAttachRSlider.Update( 2, bogie_ptr->m_MechRLoc.GetID(), bogie_ptr->m_MechR0NLoc.GetID());
        }

        m_MechAttachSSlider.Update( bogie_ptr->m_MechSLoc.GetID() );
        m_MechAttachTSlider.Update( bogie_ptr->m_MechTLoc.GetID() );
        m_MechLScaleToggleGroup.Update( bogie_ptr->m_MechL01.GetID() );

        if ( bogie_ptr->m_MechL01() )
        {
            m_MechAttachLSlider.Update( 1, bogie_ptr->m_MechLLoc.GetID(), bogie_ptr->m_MechL0LenLoc.GetID());
        }
        else
        {
            m_MechAttachLSlider.Update( 2, bogie_ptr->m_MechLLoc.GetID(), bogie_ptr->m_MechL0LenLoc.GetID());
        }

        m_MechAttachMSlider.Update( bogie_ptr->m_MechMLoc.GetID() );
        m_MechAttachNSlider.Update( bogie_ptr->m_MechNLoc.GetID() );

        m_MechAttachEtaSlider.Update( bogie_ptr->m_MechEtaLoc.GetID() );
        if ( bogie_ptr->m_MechTransAttachFlag() != vsp::ATTACH_TRANS_UV )
        {
            m_MechAttachUSlider.Deactivate();
            m_MechUScaleToggleGroup.Deactivate();
            m_MechAttachVSlider.Deactivate();
        }

        if ( bogie_ptr->m_MechTransAttachFlag() != vsp::ATTACH_TRANS_RST )
        {
            m_MechAttachRSlider.Deactivate();
            m_MechRScaleToggleGroup.Deactivate();
            m_MechAttachSSlider.Deactivate();
            m_MechAttachTSlider.Deactivate();
        }

        if ( bogie_ptr->m_MechTransAttachFlag() != vsp::ATTACH_TRANS_LMN )
        {
            m_MechAttachLSlider.Deactivate();
            m_MechLScaleToggleGroup.Deactivate();
        }

        if ( bogie_ptr->m_MechTransAttachFlag() != vsp::ATTACH_TRANS_LMN &&
             bogie_ptr->m_MechTransAttachFlag() != vsp::ATTACH_TRANS_EtaMN )
        {
            m_MechAttachMSlider.Deactivate();
            m_MechAttachNSlider.Deactivate();
        }

        if ( bogie_ptr->m_MechTransAttachFlag() != vsp::ATTACH_TRANS_EtaMN )
        {
            m_MechAttachEtaSlider.Deactivate();
        }

        if ( wing_parent )
        {
            m_MechTransEtaMNButton.Activate();
        }
        else
        {
            m_MechTransEtaMNButton.Deactivate();
            m_MechAttachEtaSlider.Deactivate();
        }

        // figure out proper integration of orphan ctrls deactivation... filter into fine tooth section carefully
        if ( veh )
        {
            Geom* parent = veh->FindGeom( bogie_ptr->GetMechParentID() );

            // Handle orphaned trans/rot attachment flags; ensure the associated header groups are active
            if ( !parent && bogie_ptr->m_MechTransAttachFlag() != vsp::ATTACH_TRANS_NONE )
            {
                orphaned_trans = true;
            }

            // Enable Attachment layouts
            m_MechAttachLayout.GetGroup()->activate();
            m_MechAttachLayoutSelections.GetGroup()->activate();
            m_MechAttachLayoutTransHeader.GetGroup()->activate();

            // if no parent OR invalid type of parent, hide attach groups
            if ( !( parent && !routing_parent && !bogie_ptr->IsMechParentJoint() ) )
            {
                // it not an orphaned geom, deactivate the whole attach layout
                if ( !( orphaned_trans || orphaned_rot ) )
                {
                    m_MechAttachLayout.GetGroup()->deactivate();
                }
                // if it IS an orphaned geom, deactivate attachment selections, keeping headers open for the orphaned attach type
                else
                {
                    m_MechAttachLayoutSelections.GetGroup()->deactivate();
                    m_MechAttachLayoutTransHeader.GetGroup()->deactivate();
                    if ( orphaned_trans )
                    {
                        m_MechAttachLayoutTransHeader.GetGroup()->activate();
                    }
                }
            }
        }

















        m_MechXAxisSlider.Update( bogie_ptr->m_MechXAxis.GetID() );
        m_MechYAxisSlider.Update( bogie_ptr->m_MechYAxis.GetID() );
        m_MechZAxisSlider.Update( bogie_ptr->m_MechZAxis.GetID() );

        m_MechKRetractSlider.Update( bogie_ptr->m_MechKRetract.GetID() );
        m_Mech2KRetractSlider.Update( bogie_ptr->m_MechKRetract.GetID() );

        m_MechKneePosSlider.Update( bogie_ptr->m_MechKneePos.GetID() );
        m_MechKneeAngleSlider.Update( bogie_ptr->m_MechKneeAngle.GetID() );
        m_MechKneeClockAngleSlider.Update( bogie_ptr->m_MechKneeAzimuthAngle.GetID() );
        m_MechKneeElevationAngleSlider.Update( bogie_ptr->m_MechKneeElevationAngle.GetID() );
        m_MechKneeDownAngleSlider.Update( bogie_ptr->m_MechKneeDownAngle.GetID() );

        m_MechRetAngleSlider.Update( bogie_ptr->m_MechRetAngle.GetID() );
        m_MechTwistAngleSlider.Update( bogie_ptr->m_MechTwistAngle.GetID() );
        m_MechRollAngleSlider.Update( bogie_ptr->m_MechRollAngle.GetID() );
        m_MechBogieAngleSlider.Update( bogie_ptr->m_MechBogieAngle.GetID() );

        m_MechStrutDLSlider.Update( bogie_ptr->m_MechStrutDL.GetID() );


    }
    else
    {
        // Bogie
        m_BogieNameInput.Update( "" );

        m_StowGeomPicker.SetGeomChoice( "" );
        m_MechGeomPicker.SetGeomChoice( "" );

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
        m_SteeringAngleSlider.Deactivate();
        m_TravelCompressSlider.Deactivate();
        m_TravelExtendSlider.Deactivate();
        m_BogieThetaMaxSlider.Deactivate();
        m_BogieThetaMinSlider.Deactivate();

        // Tire
        m_TireDiameterToggleGroup.Deactivate();
        m_TireDiameterSlider.Deactivate();
        m_TireWidthToggleGroup.Deactivate();
        m_TireWidthSlider.Deactivate();
        m_TireModeChoice.Deactivate();
        m_PlyRatingSlider.Deactivate();
        m_SpeedRatingSlider.Deactivate();
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

        m_StowLayout.GetGroup()->deactivate();
        m_MechanismLayout.GetGroup()->deactivate();
        m_Mechanism2Layout.GetGroup()->deactivate();
    }

    m_StowGeomPicker.Update();
    m_MechGeomPicker.Update();

    if ( !bogies.empty() )
    {
        m_RemoveAllBogiesButton.Activate();
        m_ShowAllBogiesButton.Activate();
        m_HideAllBogiesButton.Activate();
    }
    else
    {
        m_RemoveAllBogiesButton.Deactivate();
        m_ShowAllBogiesButton.Deactivate();
        m_HideAllBogiesButton.Deactivate();
    }

    return true;
}

void GearScreen::UpdateBogieBrowser()
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
    m_RetBogieChoice.ClearItems();
    m_RetBogieChoice.AddItem( "None Selected", -1 );
    for( int i = 0; i < ( int )bogies.size(); i++ )
    {
        if ( bogies[i] )
        {
            string bogieNamed = bogies[i]->GetDesignation( ":" );
            string bogieName = bogies[i]->GetDesignation();

            m_BogieBrowser->add( bogieNamed.c_str() );
            m_TireBogieChoice.AddItem( bogieName.c_str(), i );
            m_RetBogieChoice.AddItem( bogieName.c_str(), i );
        }
    }
    m_TireBogieChoice.UpdateItems();
    m_RetBogieChoice.UpdateItems();

    int index = gear_ptr->GetCurrBogieIndex();
    if ( index >= 0 && index < ( int )bogies.size() )
    {
        m_BogieBrowser->select( index + 2 );
        m_TireBogieChoice.SetVal( index );
        m_RetBogieChoice.SetVal( index );
    }
    m_BogieBrowser->hposition( h_pos );
    m_BogieBrowser->vposition( v_pos );
}

void GearScreen::UpdateStowSurfChoice()
{
    m_StowSurfChoice.ClearItems();

    Geom* geom_ptr = m_ScreenMgr->GetCurrGeom();
    GearGeom* gear_ptr = dynamic_cast< GearGeom* >( geom_ptr );

    if ( gear_ptr )
    {
        Bogie* bogie_ptr = gear_ptr->GetCurrentBogie();

        if ( bogie_ptr )
        {
            Vehicle* vPtr = VehicleMgr.GetVehicle();

            Geom* parent_geom = vPtr->FindGeom( bogie_ptr->GetStowParentID() );

            if ( parent_geom )
            {
                char str[255];
                int nsurf = parent_geom->GetNumTotalSurfs();

                for ( int i = 0; i < nsurf; ++i )
                {
                    snprintf( str, sizeof( str ),  "Surf_%d", i );
                    m_StowSurfChoice.AddItem( str );
                }
                m_StowSurfChoice.UpdateItems();


                if( bogie_ptr->m_StowSurfIndx() < 0 || bogie_ptr->m_StowSurfIndx() >= nsurf )
                {
                    bogie_ptr->m_StowSurfIndx = 0;
                }
                m_StowSurfChoice.SetVal( bogie_ptr->m_StowSurfIndx() );

            }
        }
    }
}

void GearScreen::UpdateMechSurfChoice()
{
    m_MechSurfChoice.ClearItems();

    Geom* geom_ptr = m_ScreenMgr->GetCurrGeom();
    GearGeom* gear_ptr = dynamic_cast< GearGeom* >( geom_ptr );

    if ( gear_ptr )
    {
        Bogie* bogie_ptr = gear_ptr->GetCurrentBogie();

        if ( bogie_ptr )
        {
            Vehicle* vPtr = VehicleMgr.GetVehicle();

            Geom* parent_geom = vPtr->FindGeom( bogie_ptr->GetMechParentID() );

            if ( parent_geom )
            {
                char str[255];
                int nsurf = parent_geom->GetNumTotalSurfs();

                for ( int i = 0; i < nsurf; ++i )
                {
                    snprintf( str, sizeof( str ),  "Surf_%d", i );
                    m_MechSurfChoice.AddItem( str );
                }
                m_MechSurfChoice.UpdateItems();


                if( bogie_ptr->m_MechSurfIndx() < 0 || bogie_ptr->m_MechSurfIndx() >= nsurf )
                {
                    bogie_ptr->m_MechSurfIndx = 0;
                }
                m_MechSurfChoice.SetVal( bogie_ptr->m_MechSurfIndx() );

            }
        }
    }
}

//==== Non Menu Callbacks ====//
void GearScreen::CallBack( Fl_Widget *w )
{
    Geom* geom_ptr = m_ScreenMgr->GetCurrGeom();

    GearGeom* gear_ptr = dynamic_cast< GearGeom* >( geom_ptr );

    m_BogieBrowser->HidePopupInput();

    if ( w == m_BogieBrowser )
    {
        //==== Apply Popup Input if necessary ====//
        if ( m_BogieBrowser->GetCBReason() == BROWSER_CALLBACK_POPUP_ENTER )
        {
            Bogie * bogie = gear_ptr->GetCurrentBogie();
            if ( bogie )
            {
                string bname = m_BogieBrowser->GetPopupValue();
                bogie->SetName( bname.c_str() );
            }
        }

        //==== Select new Bogie ====//
        int sel = m_BogieBrowser->value();
        gear_ptr->SetCurrBogieIndex( sel - 2 );

        Bogie * bogie = gear_ptr->GetCurrentBogie();

        //==== Open up new Popup Input ====//
        if ( m_BogieBrowser->GetCBReason() == BROWSER_CALLBACK_POPUP_OPEN )
        {
            // double click open the popup input
            if ( bogie )
            {
                m_BogieBrowser->InsertPopupInput( bogie->GetName(), m_BogieBrowser->value() );
            }
        }
    }

    GeomScreen::CallBack( w );
}

void GearScreen::GuiDeviceCallBack( GuiDevice* device )
{
    Geom* geom_ptr = m_ScreenMgr->GetCurrGeom();

    GearGeom* gear_ptr = dynamic_cast< GearGeom* >( geom_ptr );

    m_BogieBrowser->HidePopupInput();

    if ( device == &m_AddBogieButton )
    {
        string bname = "BOGIE_" + to_string( gear_ptr->GetAllBogies().size() );
        Bogie * bogie = gear_ptr->CreateAndAddBogie();
        if ( bogie )
        {
            bogie->SetName( bname );
        }
        gear_ptr->Update();
    }
    else if ( device == &m_RemoveBogieButton )
    {
        gear_ptr->DelBogie( gear_ptr->GetCurrBogieIndex() );
        gear_ptr->Update();
    }
    else if ( device == &m_RemoveAllBogiesButton )
    {
        gear_ptr->DelAllBogies();
        gear_ptr->Update();
    }
    else if ( device == &m_ShowAllBogiesButton )
    {
        gear_ptr->ShowAllBogies();
    }
    else if ( device == &m_HideAllBogiesButton )
    {
        gear_ptr->HideAllBogies();
    }
    else if ( device == &m_BogieNameInput )
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
    else if ( device == &m_RetBogieChoice )
    {
        gear_ptr->SetCurrBogieIndex( m_RetBogieChoice.GetVal() );
    }
    else if ( device == & m_StowGeomPicker )
    {
        Bogie * bogie = gear_ptr->GetCurrentBogie();
        if ( bogie )
        {
            bogie->SetStowParentID( m_StowGeomPicker.GetGeomChoice() );
        }
    }
    else if ( device == & m_MechGeomPicker )
    {
        Bogie * bogie = gear_ptr->GetCurrentBogie();
        if ( bogie )
        {
            bogie->SetMechParentID( m_MechGeomPicker.GetGeomChoice() );
        }
    }


    GeomScreen::GuiDeviceCallBack( device );
}



