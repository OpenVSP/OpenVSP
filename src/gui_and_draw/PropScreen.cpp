//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#include "PropScreen.h"
#include "ScreenMgr.h"
#include "PropGeom.h"
#include "ParmMgr.h"

using namespace vsp;


//==== Constructor ====//
PropScreen::PropScreen( ScreenMgr* mgr ) : XSecScreen( mgr, 460, 800, "Propeller", string(), string(), "Airfoil Alias", false ) // combine xsec and xseccurve name controls
{
    m_CurrDisplayGroup = nullptr;

    Fl_Group* design_tab = AddTab( "Design", 4 );

    Fl_Group* design_group = AddSubGroup( design_tab, 5 );

    m_DesignLayout.SetGroupAndScreen( design_group, this );

    m_DesignLayout.AddDividerBox( "Modeling" );
    m_PropModeChoice.AddItem( "Blades" );
    m_PropModeChoice.AddItem( "Both" );
    m_PropModeChoice.AddItem( "Disk" );
    m_DesignLayout.AddChoice( m_PropModeChoice, "Prop Mode" );
    m_DesignLayout.AddYGap();

    m_DesignLayout.AddDividerBox( "Blade Direction Marker" );
    m_DesignLayout.SetFitWidthFlag( false );
    m_DesignLayout.SetSameLineFlag( true );
    m_DesignLayout.SetButtonWidth( 15 );
    m_DesignLayout.AddButton( m_MarkerScaleToggle, "" );
    m_DesignLayout.SetFitWidthFlag( true );
    m_DesignLayout.SetButtonWidth( 100 - 15 );
    m_DesignLayout.AddSlider( m_MarkerScaleSlider, "Fraction", 100, "%6.5f" );
    m_DesignLayout.ForceNewLine();
    m_DesignLayout.SetSameLineFlag( false );
    m_DesignLayout.AddYGap();

    m_DesignLayout.AddDividerBox( "Design" );

    m_DesignLayout.SetButtonWidth( 100 );
    m_DesignLayout.SetChoiceButtonWidth( 100 );

    m_DesignLayout.AddSlider( m_PropDiameterSlider, "Diameter", 10, "%6.5f" );

    m_DesignLayout.AddSlider( m_NBladeSlider, "Num Blades", 100, "%5.0f" );

    m_DesignLayout.AddButton( m_CylindricalSectionsToggle, "Cylindrical Foils" );

    m_DesignLayout.SetFitWidthFlag( false );
    m_DesignLayout.SetSameLineFlag( true );
    m_DesignLayout.SetButtonWidth( 45 );
    m_DesignLayout.AddButton( m_ReverseToggle, "Rev" );
    m_DesignLayout.SetFitWidthFlag( true );
    m_DesignLayout.SetButtonWidth( 100 - 45 );
    m_DesignLayout.AddSlider( m_PropRotateSlider, "Rotate", 100, "%6.5f" );
    m_DesignLayout.ForceNewLine();

    m_DesignLayout.SetFitWidthFlag( false );
    m_DesignLayout.SetSameLineFlag( true );
    m_DesignLayout.SetButtonWidth( 15 );
    m_DesignLayout.AddButton( m_Beta34Toggle, "" );
    m_DesignLayout.SetFitWidthFlag( true );
    m_DesignLayout.SetButtonWidth( 100 - 15 );
    m_DesignLayout.AddSlider( m_Beta34Slider, "Beta 3/4", 100, "%6.5f" );
    m_DesignLayout.ForceNewLine();

    m_DesignLayout.SetFitWidthFlag( false );
    m_DesignLayout.SetSameLineFlag( true );
    m_DesignLayout.SetButtonWidth( 15 );
    m_DesignLayout.AddButton( m_FeatherToggle, "" );
    m_DesignLayout.SetFitWidthFlag( true );
    m_DesignLayout.SetButtonWidth( 100 - 15 );
    m_DesignLayout.AddSlider( m_FeatherSlider, "Feather", 100, "%6.5f" );
    m_DesignLayout.ForceNewLine();

    m_DesignLayout.SetSameLineFlag( false );
    m_DesignLayout.SetButtonWidth( 100 );

    m_DesignLayout.AddSlider( m_PreconeSlider, "Pre-cone", 10, "%5.3f" );
    m_DesignLayout.AddSlider( m_ConstructSlider, "Construct X/C", 1, "%5.3f" );
    m_DesignLayout.AddSlider( m_FeatherAxisSlider, "Feather Axis", 1, "%5.3f" );
    m_DesignLayout.AddSlider( m_FeatherOffsetSlider, "Feather Offset", 1, "%5.3f" );

    m_BetaToggle.Init( this );
    m_BetaToggle.AddButton( m_FeatherToggle.GetFlButton() );
    m_BetaToggle.AddButton( m_Beta34Toggle.GetFlButton() );

    m_DesignLayout.AddYGap();

    m_DesignLayout.AddDividerBox( "Prop / Rotor Metrics" );

    m_DesignLayout.SetSameLineFlag( true );
    m_DesignLayout.AddOutput( m_AFOutput, "Activity Factor", "%6.2f", m_DesignLayout.GetW() / 2 );
    m_DesignLayout.SetButtonWidth( 50 );
    m_DesignLayout.AddSlider( m_AFLimitSlider, "r_0", 1, "%6.5f" );
    m_DesignLayout.SetButtonWidth( 100 );

    m_DesignLayout.ForceNewLine();
    m_DesignLayout.SetSameLineFlag( false );

    m_DesignLayout.AddOutput( m_CLiOutput, "CLi", "%6.2f" );

    m_DesignLayout.SetButtonWidth( m_DesignLayout.GetW() / 6 );
    m_DesignLayout.SetInputWidth( m_DesignLayout.GetButtonWidth() );

    m_DesignLayout.SetFitWidthFlag( false );
    m_DesignLayout.SetSameLineFlag( true );

    m_DesignLayout.AddOutput( m_ChordOutput, "C/R", "%7.4f" );
    m_DesignLayout.AddOutput( m_TChordOutput, "C_T/R", "%7.4f" );
    m_DesignLayout.AddOutput( m_PChordOutput, "C_P/R", "%7.4f" );

    char buf[16];
    int indx = 0;
    indx += fl_utf8encode( 963, &buf[ indx ] ); // Greek character sigma
    buf[ indx ] = 0;
    m_DesignLayout.ForceNewLine();
    m_DesignLayout.AddOutput( m_SolidityOutput, buf, "%7.4f" );

    buf[ indx ] = '_';
    buf[ indx + 1 ] = 'T';
    buf[ indx + 2 ] = 0;
    m_DesignLayout.AddOutput( m_TSolidityOutput, buf, "%7.4f" );

    buf[ indx + 1 ] = 'P';
    m_DesignLayout.AddOutput( m_PSolidityOutput, buf, "%7.4f" );

    m_DesignLayout.ForceNewLine();
    m_DesignLayout.SetFitWidthFlag( true );
    m_DesignLayout.SetSameLineFlag( false );

    m_DesignLayout.SetButtonWidth( 100 );

    m_DesignLayout.AddYGap();

    //==== Blade Tab ====//
    Fl_Group* plot_tab = AddTab( "Blade" );

    Fl_Group* plot_group = AddSubGroup( plot_tab, 5 );

    m_BladeLayout.SetGroupAndScreen( plot_group, this );

    m_BladeLayout.SetSameLineFlag( false );
    m_BladeLayout.SetFitWidthFlag( true );

    m_BladeLayout.AddDividerBox( "Curve Editor" );

    m_CurveChoice.AddItem( "Chord" );
    m_CurveChoice.AddItem( "Twist" );
    m_CurveChoice.AddItem( "Rake" );
    m_CurveChoice.AddItem( "Skew" );
    m_CurveChoice.AddItem( "Sweep" );
    m_CurveChoice.AddItem( "Axial" );
    m_CurveChoice.AddItem( "Tangential" );
    m_CurveChoice.AddItem( "Thick" );
    m_CurveChoice.AddItem( "CLi" );

    m_EditCurve = CHORD;

    m_BladeLayout.SetFitWidthFlag( false );
    m_BladeLayout.SetSameLineFlag( true );

    m_BladeLayout.SetChoiceButtonWidth( m_BladeLayout.GetButtonWidth() );

    m_BladeLayout.SetSliderWidth( 0.5 * m_BladeLayout.GetW() - m_BladeLayout.GetButtonWidth() );
    m_BladeLayout.AddChoice( m_CurveChoice, "Curve" );
    m_BladeLayout.SetButtonWidth( 0.5 * m_BladeLayout.GetRemainX() );
    m_BladeLayout.AddButton( m_ApproxAllButton, "Approximate All" );
    m_BladeLayout.AddButton( m_ResetThickness, "Reset Thick" );

    m_BladeLayout.ForceNewLine();

    m_BladeLayout.InitWidthHeightVals();

    m_BladeLayout.SetSameLineFlag( false );
    m_BladeLayout.SetFitWidthFlag( true );

    m_BladeLayout.AddPCurveEditor( m_CurveEditor );

    
    // Xsec layout after index selector has been defined in base class
    m_XSecLayout.SetFitWidthFlag( false );
    m_XSecLayout.SetSameLineFlag( true );
    m_XSecLayout.SetButtonWidth( ( m_XFormLayout.GetRemainX() - 30 ) / 4 );
    m_XSecLayout.AddButton( m_InsertXSec, "Insert" );
    m_XSecLayout.AddX( 10 );
    m_XSecLayout.AddButton( m_CutXSec, "Cut" );
    m_XSecLayout.AddX( 10 );
    m_XSecLayout.AddButton( m_CopyXSec, "Copy" );
    m_XSecLayout.AddX( 10 );
    m_XSecLayout.AddButton( m_PasteXSec, "Paste" );
    m_XSecLayout.ForceNewLine();
    m_XSecLayout.AddYGap();

    m_XSecLayout.SetFitWidthFlag( true );
    m_XSecLayout.SetSameLineFlag( false );

    m_XSecLayout.AddYGap();

    m_XSecLayout.SetButtonWidth( 50 );
    m_XSecLayout.AddSlider( m_XSecRadSlider, "Radius", 1.0, "%6.5f" );
    m_XSecLayout.AddYGap();

    m_XSecLayout.InitWidthHeightVals();

    m_XSecDriversActive = false;
    AddXSecLayout(false); // Skip Point XSec type

    //==== Section Modify ====//

    Fl_Group* modify_tab = AddTab( "Modify" );
    Fl_Group* modify_group = AddSubGroup( modify_tab, 5 );

    m_ModifyLayout.SetButtonWidth( 70 );

    m_ModifyLayout.SetGroupAndScreen( modify_group, this );
    m_ModifyLayout.AddDividerBox( "Cross Section" );

    m_ModifyLayout.AddIndexSelector( m_XSecModIndexSelector );

    int stdwidth = m_ModifyLayout.GetButtonWidth();
    int btnwidth = 5 * stdwidth / 6;
    m_ModifyLayout.SetButtonWidth( 2 * btnwidth ); // 2x math operations here to get same rounding error as the IndexSelector buttonwidth
    string label = m_XSecCurveAliasLabel;
    m_ModifyLayout.AddInput( m_ModXSecCurveNameInput, label.c_str() );
    m_ModifyLayout.SetButtonWidth( stdwidth );

    m_ModifyLayout.AddYGap();

    m_ModifyLayout.InitWidthHeightVals();
    m_ModifyLayout.AddDividerBox( "Shift, Rotate, & Scale" );

    m_ModifyLayout.AddSlider( m_AFDeltaXSlider, "Delta X/C", 1, "%7.3f" );
    m_ModifyLayout.AddSlider( m_AFDeltaYSlider, "Delta Y/C", 1, "%7.3f" );
    m_ModifyLayout.AddSlider( m_AFThetaSlider, "Theta", 20, "%7.3f" );
    m_ModifyLayout.AddSlider( m_AFScaleSlider, "Scale", 1, "%7.3f" );
    m_ModifyLayout.AddSlider( m_AFShiftLESlider, "Shift LE", 1, "%7.3f" );

    m_ModifyLayout.SetChoiceButtonWidth( 74 );

    m_ModifyLayout.AddYGap();

    m_ModifyLayout.AddDividerBox( "Leading Edge" );
    m_LECapChoice.AddItem( "FLAT", vsp::FLAT_END_CAP );
    m_LECapChoice.AddItem( "ROUND", vsp::ROUND_END_CAP );
    m_LECapChoice.AddItem( "EDGE", vsp::EDGE_END_CAP );
    m_LECapChoice.AddItem( "SHARP", vsp::SHARP_END_CAP );

    m_ModifyLayout.AddChoice( m_LECapChoice, "Cap" );

    m_ModifyLayout.AddSlider( m_LECapLengthSlider, "Length", 10.0, "%6.5f" );
    m_ModifyLayout.AddSlider( m_LECapOffsetSlider, "Offset", 10.0, "%6.5f" );
    m_ModifyLayout.AddSlider( m_LECapStrengthSlider, "Strength", 10.0, "%6.5f" );

    m_LECloseChoice.AddItem( "NONE" );
    m_LECloseChoice.AddItem( "SKEW LOWER" );
    m_LECloseChoice.AddItem( "SKEW UPPER" );
    m_LECloseChoice.AddItem( "SKEW BOTH" );

    m_ModifyLayout.SetFitWidthFlag( true );
    m_ModifyLayout.SetSameLineFlag( true );

    m_ModifyLayout.AddYGap();

    m_ModifyLayout.AddChoice( m_LECloseChoice, "Closure:", m_ModifyLayout.GetButtonWidth() * 2 );

    m_ModifyLayout.SetFitWidthFlag( false );
    m_ModifyLayout.AddButton( m_LECloseABSButton, "Abs" );
    m_ModifyLayout.AddButton( m_LECloseRELButton, "Rel" );

    m_LECloseGroup.Init( this );
    m_LECloseGroup.AddButton( m_LECloseABSButton.GetFlButton() );
    m_LECloseGroup.AddButton( m_LECloseRELButton.GetFlButton() );

    vector< int > close_val_map;
    close_val_map.push_back( vsp::ABS );
    close_val_map.push_back( vsp::REL );
    m_LECloseGroup.SetValMapVec( close_val_map );

    m_ModifyLayout.ForceNewLine();

    m_ModifyLayout.SetFitWidthFlag( true );
    m_ModifyLayout.SetSameLineFlag( false );

    m_ModifyLayout.AddSlider( m_CloseLEThickSlider, "T", 10.0, "%6.5f" );

    m_ModifyLayout.AddYGap();

    m_LETrimChoice.AddItem( "NONE" );
    m_LETrimChoice.AddItem( "X" );
    m_LETrimChoice.AddItem( "THICK" );

    m_ModifyLayout.SetFitWidthFlag( true );
    m_ModifyLayout.SetSameLineFlag( true );

    m_ModifyLayout.AddChoice( m_LETrimChoice, "Trim:", m_ModifyLayout.GetButtonWidth() * 2 );

    m_ModifyLayout.SetFitWidthFlag( false );
    m_ModifyLayout.AddButton( m_LETrimABSButton, "Abs" );
    m_ModifyLayout.AddButton( m_LETrimRELButton, "Rel" );

    m_LETrimGroup.Init( this );
    m_LETrimGroup.AddButton( m_LETrimABSButton.GetFlButton() );
    m_LETrimGroup.AddButton( m_LETrimRELButton.GetFlButton() );

    vector< int > trim_val_map;
    trim_val_map.push_back( vsp::ABS );
    trim_val_map.push_back( vsp::REL );
    m_LETrimGroup.SetValMapVec( trim_val_map );

    m_ModifyLayout.ForceNewLine();

    m_ModifyLayout.SetFitWidthFlag( true );
    m_ModifyLayout.SetSameLineFlag( false );

    m_ModifyLayout.AddSlider( m_TrimLEXSlider, "X", 10.0, "%6.5f" );
    m_ModifyLayout.AddSlider( m_TrimLEThickSlider, "T", 10.0, "%6.5f" );

    m_ModifyLayout.AddYGap();

    m_ModifyLayout.AddDividerBox( "Trailing Edge" );
    m_TECapChoice.AddItem( "FLAT", vsp::FLAT_END_CAP );
    m_TECapChoice.AddItem( "ROUND", vsp::ROUND_END_CAP );
    m_TECapChoice.AddItem( "EDGE", vsp::EDGE_END_CAP );
    m_TECapChoice.AddItem( "SHARP", vsp::SHARP_END_CAP );

    m_ModifyLayout.AddChoice( m_TECapChoice, "Cap:" );

    m_ModifyLayout.AddSlider( m_TECapLengthSlider, "Length", 10.0, "%6.5f" );
    m_ModifyLayout.AddSlider( m_TECapOffsetSlider, "Offset", 10.0, "%6.5f" );
    m_ModifyLayout.AddSlider( m_TECapStrengthSlider, "Strength", 10.0, "%6.5f" );

    m_TECloseChoice.AddItem( "NONE" );
    m_TECloseChoice.AddItem( "SKEW LOWER" );
    m_TECloseChoice.AddItem( "SKEW UPPER" );
    m_TECloseChoice.AddItem( "SKEW BOTH" );
    m_TECloseChoice.AddItem( "EXTRAPOLATE" );

    m_ModifyLayout.SetFitWidthFlag( true );
    m_ModifyLayout.SetSameLineFlag( true );

    m_ModifyLayout.AddYGap();

    m_ModifyLayout.AddChoice( m_TECloseChoice, "Closure:", m_ModifyLayout.GetButtonWidth() * 2 );

    m_ModifyLayout.SetFitWidthFlag( false );
    m_ModifyLayout.AddButton( m_TECloseABSButton, "Abs" );
    m_ModifyLayout.AddButton( m_TECloseRELButton, "Rel" );

    m_TECloseGroup.Init( this );
    m_TECloseGroup.AddButton( m_TECloseABSButton.GetFlButton() );
    m_TECloseGroup.AddButton( m_TECloseRELButton.GetFlButton() );

    m_TECloseGroup.SetValMapVec( close_val_map );

    m_ModifyLayout.ForceNewLine();

    m_ModifyLayout.SetFitWidthFlag( true );
    m_ModifyLayout.SetSameLineFlag( false );

    m_ModifyLayout.AddSlider( m_CloseTEThickSlider, "T", 10.0, "%6.5f" );

    m_ModifyLayout.AddYGap();

    m_TETrimChoice.AddItem( "NONE" );
    m_TETrimChoice.AddItem( "X" );
    m_TETrimChoice.AddItem( "THICK" );

    m_ModifyLayout.SetFitWidthFlag( true );
    m_ModifyLayout.SetSameLineFlag( true );

    m_ModifyLayout.AddChoice( m_TETrimChoice, "Trim:", m_ModifyLayout.GetButtonWidth() * 2 );

    m_ModifyLayout.SetFitWidthFlag( false );
    m_ModifyLayout.AddButton( m_TETrimABSButton, "Abs" );
    m_ModifyLayout.AddButton( m_TETrimRELButton, "Rel" );

    m_TETrimGroup.Init( this );
    m_TETrimGroup.AddButton( m_TETrimABSButton.GetFlButton() );
    m_TETrimGroup.AddButton( m_TETrimRELButton.GetFlButton() );

    m_TETrimGroup.SetValMapVec( trim_val_map );

    m_ModifyLayout.ForceNewLine();

    m_ModifyLayout.SetFitWidthFlag( true );
    m_ModifyLayout.SetSameLineFlag( false );

    m_ModifyLayout.AddSlider( m_TrimTEXSlider, "X", 10.0, "%6.5f" );
    m_ModifyLayout.AddSlider( m_TrimTEThickSlider, "T", 10.0, "%6.5f" );

    m_ModifyLayout.AddYGap();

    m_TEFlapChoice.AddItem( "NONE", vsp::FLAP_NONE );
    m_TEFlapChoice.AddItem( "PLAIN", vsp::FLAP_PLAIN );

    m_ModifyLayout.SetFitWidthFlag( true );
    m_ModifyLayout.SetSameLineFlag( true );

    m_ModifyLayout.AddChoice( m_TEFlapChoice, "Flap:", m_ModifyLayout.GetButtonWidth() * 2 );

    m_ModifyLayout.SetFitWidthFlag( false );
    m_ModifyLayout.AddButton( m_TEFlapABSButton, "Abs" );
    m_ModifyLayout.AddButton( m_TEFlapRELButton, "Rel" );

    m_TEFlapGroup.Init( this );
    m_TEFlapGroup.AddButton( m_TEFlapABSButton.GetFlButton() );
    m_TEFlapGroup.AddButton( m_TEFlapRELButton.GetFlButton() );

    m_TEFlapGroup.SetValMapVec( trim_val_map );

    m_ModifyLayout.ForceNewLine();

    m_ModifyLayout.SetFitWidthFlag( true );
    m_ModifyLayout.SetSameLineFlag( false );

    m_ModifyLayout.AddSlider( m_FlapTEXSlider, "X", 10.0, "%6.5f" );
    m_ModifyLayout.AddSlider( m_FlapTEYSlider, "Y/T", 10.0, "%6.5f" );
    m_ModifyLayout.AddSlider( m_FlapTEDeflectionSlider, "Deflection", 10.0, "%6.5f" );

    Fl_Group* more_tab = AddTab("More" );
    Fl_Group* more_group = AddSubGroup(more_tab, 5 );

    m_MoreLayout.SetGroupAndScreen(more_group, this );

    m_MoreLayout.SetButtonWidth(100 );
    m_MoreLayout.SetChoiceButtonWidth(100 );

    m_MoreLayout.AddDividerBox("Folding" );

    m_MoreLayout.SetFitWidthFlag( false );
    m_MoreLayout.SetSameLineFlag( true );

    m_MoreLayout.SetButtonWidth( 15 );
    m_MoreLayout.AddButton( m_IndividualBladeFoldToggle, "");

    m_MoreLayout.SetButtonWidth( 50 );
    m_MoreLayout.SetSliderWidth( 50 );
    m_MoreLayout.AddCounter( m_ActiveBladeIndexSelector, "Blade" );


    m_MoreLayout.SetButtonWidth( 100 );
    m_MoreLayout.SetFitWidthFlag( true );

    m_MoreLayout.AddSlider( m_FoldAngleSlider, "Angle", 100, "%5.3f" );
    m_MoreLayout.ForceNewLine();


    m_MoreLayout.SetFitWidthFlag( true );
    m_MoreLayout.SetSameLineFlag( false );


    m_MoreLayout.AddSlider(m_RFoldSlider, "Radial/R", 1, "%5.3f" );
    m_MoreLayout.AddSlider(m_AxFoldSlider, "Axial/R", 1, "%5.3f" );
    m_MoreLayout.AddSlider(m_OffFoldSlider, "Offset/R", 1, "%5.3f" );
    m_MoreLayout.AddSlider(m_AzFoldSlider, "Azimuth", 100, "%5.3f" );
    m_MoreLayout.AddSlider(m_ElFoldSlider, "Elevation", 100, "%5.3f" );

    m_MoreLayout.SetButtonWidth(100 );

    m_MoreLayout.AddYGap();

    m_MoreLayout.AddDividerBox( "Blade Spacing" );

    m_MoreLayout.SetFitWidthFlag( true );
    m_MoreLayout.SetSameLineFlag( true );

    m_BladeAziModeChoice.AddItem( "Uniform", PROP_AZI_UNIFORM );
    m_BladeAziModeChoice.AddItem( "Free", PROP_AZI_FREE );
    m_BladeAziModeChoice.AddItem( "Balanced", PROP_AZI_BALANCED );
    m_BladeAziModeChoice.UpdateItems();

    m_MoreLayout.AddChoice( m_BladeAziModeChoice, "Mode", m_MoreLayout.GetButtonWidth() );
    m_MoreLayout.SetFitWidthFlag( false );
    m_MoreLayout.AddButton( m_BladeAziDeltaButton, "Delta" );
    m_MoreLayout.ForceNewLine();

    m_MoreLayout.SetFitWidthFlag( true );
    m_MoreLayout.SetSameLineFlag( false );

    int ht = m_MoreLayout.GetStdHeight() * 5;
    m_MoreLayout.AddSubGroupLayout( m_BladeAziLayout, m_MoreLayout.GetRemainX(), ht );
    m_MoreLayout.AddY( ht );

    m_BladeAziScroll = m_BladeAziLayout.AddFlScroll( m_BladeAziLayout.GetRemainY() );
    m_BladeAziScroll->type( Fl_Scroll::VERTICAL_ALWAYS );
    m_BladeAziScroll->box( FL_BORDER_BOX );

    m_BladeAziLayout.SetSameLineFlag( false );
    m_BladeAziLayout.SetFitWidthFlag( true );

    m_FLTK_Window->resizable( m_BladeAziScroll );

    m_MoreLayout.SetFitWidthFlag( true );
    m_MoreLayout.SetSameLineFlag( true );
    m_MoreLayout.AddOutput( m_BalanceX1Output, "X1 Balance", "%5.3g", m_MoreLayout.GetW() / 2.0 );
    m_MoreLayout.AddOutput( m_BalanceX2Output, "X2 Balance", "%5.3g", m_MoreLayout.GetW() / 2.0 );
    m_MoreLayout.ForceNewLine();

    m_MoreLayout.SetFitWidthFlag( true );
    m_MoreLayout.SetSameLineFlag( false );

    m_MoreLayout.AddYGap();

    m_MoreLayout.AddDividerBox("Tip Treatment" );
    m_MoreLayout.AddSlider(m_CapTessSlider, "Cap Tess", 10, "%3.0f" );

    m_MoreLayout.AddYGap();

    m_RootCapTypeChoice.AddItem( "None", vsp::NO_END_CAP );
    m_RootCapTypeChoice.AddItem( "Flat", vsp::FLAT_END_CAP );
    m_RootCapTypeChoice.AddItem( "Round", vsp::ROUND_END_CAP );
    m_RootCapTypeChoice.AddItem( "Edge", vsp::EDGE_END_CAP );
    m_RootCapTypeChoice.AddItem( "Sharp", vsp::SHARP_END_CAP );
    m_RootCapTypeChoice.AddItem( "Round Ext None", vsp::ROUND_EXT_END_CAP_NONE );
    m_RootCapTypeChoice.AddItem( "Round Ext LE", vsp::ROUND_EXT_END_CAP_LE );
    m_RootCapTypeChoice.AddItem( "Round Ext TE", vsp::ROUND_EXT_END_CAP_TE );
    m_RootCapTypeChoice.AddItem( "Round Ext Both", vsp::ROUND_EXT_END_CAP_BOTH );
    m_MoreLayout.AddChoice(m_RootCapTypeChoice, "Root Cap Type");

    m_MoreLayout.AddSlider(m_RootCapLenSlider, "Length", 1, "%6.5f" );
    m_MoreLayout.AddSlider(m_RootCapOffsetSlider, "Offset", 1, "%6.5f" );
    m_MoreLayout.AddSlider(m_RootCapStrengthSlider, "Strength", 1, "%6.5f" );
    m_MoreLayout.AddButton(m_RootCapSweepFlagButton, "Sweep Stretch" );

    m_MoreLayout.AddYGap();

    m_TipCapTypeChoice.AddItem( "None", vsp::NO_END_CAP );
    m_TipCapTypeChoice.AddItem( "Flat", vsp::FLAT_END_CAP );
    m_TipCapTypeChoice.AddItem( "Round", vsp::ROUND_END_CAP );
    m_TipCapTypeChoice.AddItem( "Edge", vsp::EDGE_END_CAP );
    m_TipCapTypeChoice.AddItem( "Sharp", vsp::SHARP_END_CAP );
    m_TipCapTypeChoice.AddItem( "Round Ext None", vsp::ROUND_EXT_END_CAP_NONE );
    m_TipCapTypeChoice.AddItem( "Round Ext LE", vsp::ROUND_EXT_END_CAP_LE );
    m_TipCapTypeChoice.AddItem( "Round Ext TE", vsp::ROUND_EXT_END_CAP_TE );
    m_TipCapTypeChoice.AddItem( "Round Ext Both", vsp::ROUND_EXT_END_CAP_BOTH );
    m_MoreLayout.AddChoice(m_TipCapTypeChoice, "Tip Cap Type");

    m_MoreLayout.AddSlider(m_TipCapLenSlider, "Length", 1, "%6.5f" );
    m_MoreLayout.AddSlider(m_TipCapOffsetSlider, "Offset", 1, "%6.5f" );
    m_MoreLayout.AddSlider(m_TipCapStrengthSlider, "Strength", 1, "%6.5f" );
    m_MoreLayout.AddButton(m_TipCapSweepFlagButton, "Sweep Stretch" );

    m_MoreLayout.AddYGap();

    m_MoreLayout.AddDividerBox("Tessellation Control" );
    m_MoreLayout.AddSlider(m_LEClusterSlider, "LE Clustering", 1, "%6.5f" );
    m_MoreLayout.AddSlider(m_TEClusterSlider, "TE Clustering", 1, "%6.5f" );
    m_MoreLayout.AddSlider(m_RootClusterSlider, "Root Clustering", 1, "%6.5f" );
    m_MoreLayout.AddSlider(m_TipClusterSlider, "Tip Clustering", 1, "%6.5f" );

    m_MoreLayout.AddYGap();
    m_MoreLayout.SetButtonWidth(200 );
    m_MoreLayout.AddOutput(m_SmallPanelWOutput, "Minimum LE/TE Panel Width", "%6.4g" );
    m_MoreLayout.AddOutput(m_MaxGrowthOutput, "Maximum Growth Ratio", "%6.3f" );

    m_SubSurfChoice.AddItem( SubSurface::GetTypeName( vsp::SS_CONTROL ), vsp::SS_CONTROL );
    m_SubSurfChoice.UpdateItems();
}

//==== Show Pod Screen ====//
PropScreen::~PropScreen()
{
}

void PropScreen::RedrawBladeAzimuthSliders( int num_sliders )
{
    if ( num_sliders != m_BladeAziSliderVec.size() )
    {
        m_BladeAziScroll->clear();
        m_BladeAziLayout.SetGroup( m_BladeAziScroll );

        m_BladeAziSliderVec.clear();
        m_BladeAziSliderVec.resize( num_sliders, SliderAdjRange2Input() );

        for ( int i = 0; i < num_sliders; i++ )
        {
            m_BladeAziLayout.AddSlider( m_BladeAziSliderVec[i], "AUTO_UPDATE", 10, "%9.4f" );
        }
    }
}

//==== Show Pod Screen ====//
void PropScreen::Show()
{
    if ( Update() )
    {
        XSecScreen::Show();
    }
}

//==== Update Pod Screen ====//
bool PropScreen::Update()
{
    assert( m_ScreenMgr );

    Geom* geom_ptr = m_ScreenMgr->GetCurrGeom();
    if ( !geom_ptr || geom_ptr->GetType().m_Type != PROP_GEOM_TYPE )
    {
        Hide();
        return false;
    }

    XSecScreen::Update();

    PropGeom* propeller_ptr = dynamic_cast< PropGeom* >( geom_ptr );
    assert( propeller_ptr );

    //==== Design ====//
    m_PropDiameterSlider.Update( propeller_ptr->m_Diameter.GetID() );
    m_NBladeSlider.Update( propeller_ptr->m_Nblade.GetID() );

    m_PropRotateSlider.Update( propeller_ptr->m_Rotate.GetID() );

    m_ReverseToggle.Update( propeller_ptr->m_ReverseFlag.GetID() );
    m_CylindricalSectionsToggle.Update( propeller_ptr->m_CylindricalSectionsFlag.GetID() );

    m_Beta34Slider.Update( propeller_ptr->m_Beta34.GetID() );
    m_FeatherSlider.Update( propeller_ptr->m_Feather.GetID() );
    m_PreconeSlider.Update( propeller_ptr->m_Precone.GetID() );

    m_BetaToggle.Update( propeller_ptr->m_UseBeta34Flag.GetID() );

    m_ConstructSlider.Update( propeller_ptr->m_Construct.GetID() );

    m_FeatherAxisSlider.Update( propeller_ptr->m_FeatherAxis.GetID() );
    m_FeatherOffsetSlider.Update( propeller_ptr->m_FeatherOffset.GetID() );

    if ( propeller_ptr->m_UseBeta34Flag() == 1 )
    {
        m_Beta34Slider.Activate();
        m_FeatherSlider.Deactivate();
    }
    else
    {
        m_Beta34Slider.Deactivate();
        m_FeatherSlider.Activate();
    }

    m_AFLimitSlider.Update( propeller_ptr->m_AFLimit.GetID() );

    char str[255] = {};
    m_AFOutput.Update( propeller_ptr->m_AF.GetID() );
    m_CLiOutput.Update( propeller_ptr->m_CLi.GetID() );
    m_SolidityOutput.Update( propeller_ptr->m_Solidity.GetID() );
    m_TSolidityOutput.Update( propeller_ptr->m_TSolidity.GetID() );
    m_PSolidityOutput.Update( propeller_ptr->m_PSolidity.GetID() );
    m_ChordOutput.Update( propeller_ptr->m_Chord.GetID() );
    m_TChordOutput.Update( propeller_ptr->m_TChord.GetID() );
    m_PChordOutput.Update( propeller_ptr->m_PChord.GetID() );

    m_MarkerScaleToggle.Update( propeller_ptr->m_TipMarkerScaleFlag.GetID() );
    m_MarkerScaleSlider.Update( propeller_ptr->m_TipMarkerScale.GetID() );

    m_PropModeChoice.Update( propeller_ptr->m_PropMode.GetID() );

    m_RFoldSlider.Update( propeller_ptr->m_RadFoldAxis.GetID() );
    m_AxFoldSlider.Update( propeller_ptr->m_AxialFoldAxis.GetID() );
    m_OffFoldSlider.Update( propeller_ptr->m_OffsetFoldAxis.GetID() );
    m_AzFoldSlider.Update( propeller_ptr->m_AzimuthFoldDir.GetID() );
    m_ElFoldSlider.Update( propeller_ptr->m_ElevationFoldDir.GetID() );

    m_IndividualBladeFoldToggle.Update( propeller_ptr->m_IndividualBladeFoldFlag.GetID() );
    m_ActiveBladeIndexSelector.Update( propeller_ptr->m_ActiveBlade.GetID() );

    if ( propeller_ptr->m_ActiveBlade() > 1 && propeller_ptr->m_ActiveBlade() <= propeller_ptr->m_Nblade() )
    {
        int index = propeller_ptr->m_ActiveBlade() - 2;
        m_FoldAngleSlider.Update( propeller_ptr->m_FoldAngleParmVec[ index ]->GetID() );
    }
    else
    {
        m_FoldAngleSlider.Update( propeller_ptr->m_FoldAngle.GetID() );
    }

    if ( propeller_ptr->m_IndividualBladeFoldFlag() )
    {
        m_ActiveBladeIndexSelector.Activate();
    }
    else
    {
        m_ActiveBladeIndexSelector.Deactivate();
    }

    int nslider = propeller_ptr->m_Nblade() - 1;
    RedrawBladeAzimuthSliders( nslider );

    m_BladeAziModeChoice.Update( propeller_ptr->m_BladeAzimuthMode.GetID() );
    m_BladeAziDeltaButton.Update( propeller_ptr->m_BladeAzimuthDeltaFlag.GetID() );

    m_BalanceX1Output.Update( propeller_ptr->m_BalanceX1.GetID() );
    m_BalanceX2Output.Update( propeller_ptr->m_BalanceX2.GetID() );

    for ( int i = 0; i < nslider; i++ )
    {
        if ( !( propeller_ptr->m_BladeAzimuthDeltaFlag() ) )
        {
            m_BladeAziSliderVec[i].Update( 1, propeller_ptr->m_BladeAzimuthParmVec[i]->GetID(), propeller_ptr->m_BladeDeltaAzimuthParmVec[i]->GetID() );
        }
        else
        {
            m_BladeAziSliderVec[i].Update( 2, propeller_ptr->m_BladeAzimuthParmVec[i]->GetID(), propeller_ptr->m_BladeDeltaAzimuthParmVec[i]->GetID() );
        }
    }


    m_LEClusterSlider.Update( propeller_ptr->m_LECluster.GetID() );
    m_TEClusterSlider.Update( propeller_ptr->m_TECluster.GetID() );
    m_RootClusterSlider.Update( propeller_ptr->m_RootCluster.GetID() );
    m_TipClusterSlider.Update( propeller_ptr->m_TipCluster.GetID() );

    m_SmallPanelWOutput.Update( propeller_ptr->m_SmallPanelW.GetID() );

    m_MaxGrowthOutput.Update( propeller_ptr->m_MaxGrowth.GetID() );

    m_RootCapTypeChoice.Update( propeller_ptr->m_CapUMinOption.GetID() );
    m_TipCapTypeChoice.Update( propeller_ptr->m_CapUMaxOption.GetID() );

    if ( propeller_ptr->m_CapUMinOption() == NO_END_CAP &&
         propeller_ptr->m_CapUMaxOption() == NO_END_CAP )
    {
        m_CapTessSlider.Deactivate();
    }
    else
    {
        m_CapTessSlider.Update( propeller_ptr->m_CapUMinTess.GetID() );
    }

    m_RootCapLenSlider.Update( propeller_ptr->m_CapUMinLength.GetID() );
    m_RootCapOffsetSlider.Update( propeller_ptr->m_CapUMinOffset.GetID() );
    m_RootCapStrengthSlider.Update( propeller_ptr->m_CapUMinStrength.GetID() );
    m_RootCapSweepFlagButton.Update( propeller_ptr->m_CapUMinSweepFlag.GetID() );

    UpdateCapControls( propeller_ptr->m_CapUMinOption(), m_RootCapLenSlider, m_RootCapOffsetSlider, m_RootCapStrengthSlider, m_RootCapSweepFlagButton );

    m_TipCapLenSlider.Update( propeller_ptr->m_CapUMaxLength.GetID() );
    m_TipCapOffsetSlider.Update( propeller_ptr->m_CapUMaxOffset.GetID() );
    m_TipCapStrengthSlider.Update( propeller_ptr->m_CapUMaxStrength.GetID() );
    m_TipCapSweepFlagButton.Update( propeller_ptr->m_CapUMaxSweepFlag.GetID() );

    UpdateCapControls( propeller_ptr->m_CapUMaxOption(), m_TipCapLenSlider, m_TipCapOffsetSlider, m_TipCapStrengthSlider, m_TipCapSweepFlagButton );

    switch ( m_EditCurve )
    {
    case CHORD:
        m_CurveEditor.Update( &propeller_ptr->m_ChordCurve );
        break;
    case TWIST:
        m_CurveEditor.Update( &propeller_ptr->m_TwistCurve );
        break;
    case RAKE:
        m_CurveEditor.Update( &propeller_ptr->m_RakeCurve );
        break;
    case SKEW:
        m_CurveEditor.Update( &propeller_ptr->m_SkewCurve );
        break;
    case SWEEP:
        m_CurveEditor.Update( &propeller_ptr->m_SweepCurve );
        break;
    case AXIAL:
        m_CurveEditor.Update( &propeller_ptr->m_AxialCurve );
        break;
    case TANGENTIAL:
        m_CurveEditor.Update( &propeller_ptr->m_TangentialCurve );
        break;
    case THICK:
        m_CurveEditor.Update( &propeller_ptr->m_ThickCurve, &propeller_ptr->m_ChordCurve, string( "Thick/R" ) );
        break;
    case CLI:
        m_CurveEditor.Update( &propeller_ptr->m_CLICurve );
        break;
    }

    //==== Skin & XSec Index Display ===//
    int xsid = propeller_ptr->m_ActiveXSec();
    m_XSecIndexSelector.Update( propeller_ptr->m_ActiveXSec.GetID() );
    m_XSecModIndexSelector.Update( propeller_ptr->m_ActiveXSec.GetID() );

    PropXSec* xs = ( PropXSec* ) propeller_ptr->GetXSec( xsid );
    if ( xs )
    {
        bool firstxs = xsid == 0;
        bool lastxs = xsid == ( propeller_ptr->GetXSecSurf( 0 )->NumXSec() - 1 );

        //==== XSec ====//
        m_XSecRadSlider.Update( xs->m_RadiusFrac.GetID() );

        if ( lastxs )
        {
            m_XSecRadSlider.Deactivate();
        }

        XSecCurve* xsc = xs->GetXSecCurve();
        if ( xsc )
        {
            if (xsc->GetType() == XS_POINT)
            {
                printf( "Error\n" );
            }

            m_ModXSecCurveNameInput.Update( xsc->GetGroupAlias() );

            m_TECloseChoice.Update( xsc->m_TECloseType.GetID() );
            m_TECloseGroup.Update( xsc->m_TECloseAbsRel.GetID() );

            if ( xsc->m_TECloseType() != CLOSE_NONE )
            {
                m_TECloseABSButton.Activate();
                m_TECloseRELButton.Activate();
                m_CloseTEThickSlider.Activate();

                if ( xsc->m_TECloseAbsRel() == ABS )
                {
                    xsc->m_TECloseThick.Activate();
                    xsc->m_TECloseThickChord.Deactivate();
                }
                else
                {
                    xsc->m_TECloseThick.Deactivate();
                    xsc->m_TECloseThickChord.Activate();
                }
            }
            else
            {
                m_TECloseABSButton.Deactivate();
                m_TECloseRELButton.Deactivate();
                m_CloseTEThickSlider.Deactivate();
                xsc->m_TECloseThick.Deactivate();
                xsc->m_TECloseThickChord.Deactivate();
            }

            if ( xsc->m_TECloseAbsRel() == ABS )
            {
                m_CloseTEThickSlider.Update( 1, xsc->m_TECloseThick.GetID(), xsc->m_TECloseThickChord.GetID() );
            }
            else
            {
                m_CloseTEThickSlider.Update( 2, xsc->m_TECloseThick.GetID(), xsc->m_TECloseThickChord.GetID() );
            }

            m_LECloseChoice.Update( xsc->m_LECloseType.GetID() );
            m_LECloseGroup.Update( xsc->m_LECloseAbsRel.GetID() );

            if ( xsc->m_LECloseType() != CLOSE_NONE )
            {
                m_LECloseABSButton.Activate();
                m_LECloseRELButton.Activate();
                m_CloseLEThickSlider.Activate();

                if ( xsc->m_LECloseAbsRel() == ABS )
                {
                    xsc->m_LECloseThick.Activate();
                    xsc->m_LECloseThickChord.Deactivate();
                }
                else
                {
                    xsc->m_LECloseThick.Deactivate();
                    xsc->m_LECloseThickChord.Activate();
                }
            }
            else
            {
                m_LECloseABSButton.Deactivate();
                m_LECloseRELButton.Deactivate();
                m_CloseLEThickSlider.Deactivate();
                xsc->m_LECloseThick.Deactivate();
                xsc->m_LECloseThickChord.Deactivate();
            }

            if ( xsc->m_LECloseAbsRel() == ABS )
            {
                m_CloseLEThickSlider.Update( 1, xsc->m_LECloseThick.GetID(), xsc->m_LECloseThickChord.GetID() );
            }
            else
            {
                m_CloseLEThickSlider.Update( 2, xsc->m_LECloseThick.GetID(), xsc->m_LECloseThickChord.GetID() );
            }

            m_TETrimChoice.Update( xsc->m_TETrimType.GetID() );
            m_TETrimGroup.Update( xsc->m_TETrimAbsRel.GetID() );

            m_TrimTEXSlider.Deactivate();
            m_TrimTEThickSlider.Deactivate();
            m_TETrimABSButton.Deactivate();
            m_TETrimRELButton.Deactivate();

            xsc->m_TETrimX.Deactivate();
            xsc->m_TETrimXChord.Deactivate();
            xsc->m_TETrimThickChord.Deactivate();
            xsc->m_TETrimThick.Deactivate();

            if ( xsc->m_TETrimType() != TRIM_NONE )
            {
                m_TETrimABSButton.Activate();
                m_TETrimRELButton.Activate();
            }

            if ( xsc->m_TETrimType() == TRIM_X )
            {
                if ( xsc->m_TETrimAbsRel() == ABS )
                {
                    xsc->m_TETrimX.Activate();
                }
                else
                {
                    xsc->m_TETrimXChord.Activate();
                }
            }
            else if ( xsc->m_TETrimType() == TRIM_THICK )
            {
                if ( xsc->m_TETrimAbsRel() == ABS )
                {
                    xsc->m_TETrimThick.Activate();
                }
                else
                {
                    xsc->m_TETrimThickChord.Activate();
                }
            }

            if ( xsc->m_TETrimAbsRel() == ABS )
            {
                m_TrimTEXSlider.Update( 1, xsc->m_TETrimX.GetID(), xsc->m_TETrimXChord.GetID() );
                m_TrimTEThickSlider.Update( 1, xsc->m_TETrimThick.GetID(), xsc->m_TETrimThickChord.GetID() );
            }
            else
            {
                m_TrimTEXSlider.Update( 2, xsc->m_TETrimX.GetID(), xsc->m_TETrimXChord.GetID() );
                m_TrimTEThickSlider.Update( 2, xsc->m_TETrimThick.GetID(), xsc->m_TETrimThickChord.GetID() );
            }

            m_LETrimChoice.Update( xsc->m_LETrimType.GetID() );
            m_LETrimGroup.Update( xsc->m_LETrimAbsRel.GetID() );

            m_TrimLEXSlider.Deactivate();
            m_TrimLEThickSlider.Deactivate();
            m_LETrimABSButton.Deactivate();
            m_LETrimRELButton.Deactivate();

            xsc->m_LETrimX.Deactivate();
            xsc->m_LETrimXChord.Deactivate();
            xsc->m_LETrimThickChord.Deactivate();
            xsc->m_LETrimThick.Deactivate();

            if ( xsc->m_LETrimType() != TRIM_NONE )
            {
                m_LETrimABSButton.Activate();
                m_LETrimRELButton.Activate();
            }

            if ( xsc->m_LETrimType() == TRIM_X )
            {
                if ( xsc->m_LETrimAbsRel() == ABS )
                {
                    xsc->m_LETrimX.Activate();
                }
                else
                {
                    xsc->m_LETrimXChord.Activate();
                }
            }
            else if ( xsc->m_LETrimType() == TRIM_THICK )
            {
                if ( xsc->m_LETrimAbsRel() == ABS )
                {
                    xsc->m_LETrimThick.Activate();
                }
                else
                {
                    xsc->m_LETrimThickChord.Activate();
                }
            }

            if ( xsc->m_LETrimAbsRel() == ABS )
            {
                m_TrimLEXSlider.Update( 1, xsc->m_LETrimX.GetID(), xsc->m_LETrimXChord.GetID() );
                m_TrimLEThickSlider.Update( 1, xsc->m_LETrimThick.GetID(), xsc->m_LETrimThickChord.GetID() );
            }
            else
            {
                m_TrimLEXSlider.Update( 2, xsc->m_LETrimX.GetID(), xsc->m_LETrimXChord.GetID() );
                m_TrimLEThickSlider.Update( 2, xsc->m_LETrimThick.GetID(), xsc->m_LETrimThickChord.GetID() );
            }

            m_TECapChoice.Update( xsc->m_TECapType.GetID() );

            m_TECapLengthSlider.Update( xsc->m_TECapLength.GetID() );
            m_TECapOffsetSlider.Update( xsc->m_TECapOffset.GetID() );
            m_TECapStrengthSlider.Update( xsc->m_TECapStrength.GetID() );

            UpdateCapControls( xsc->m_TECapType(), m_TECapLengthSlider, m_TECapOffsetSlider, m_TECapStrengthSlider );

            m_LECapChoice.Update( xsc->m_LECapType.GetID() );

            m_LECapLengthSlider.Update( xsc->m_LECapLength.GetID() );
            m_LECapOffsetSlider.Update( xsc->m_LECapOffset.GetID() );
            m_LECapStrengthSlider.Update( xsc->m_LECapStrength.GetID() );

            UpdateCapControls( xsc->m_LECapType(), m_LECapLengthSlider, m_LECapOffsetSlider, m_LECapStrengthSlider );


            m_TEFlapChoice.Update( xsc->m_TEFlapType.GetID() );
            m_TEFlapGroup.Update( xsc->m_TEFlapAbsRel.GetID() );

            m_FlapTEXSlider.Activate();
            m_FlapTEYSlider.Activate();
            m_FlapTEDeflectionSlider.Activate();
            m_TEFlapABSButton.Activate();
            m_TEFlapRELButton.Activate();

            xsc->m_TEFlapX.Activate();
            xsc->m_TEFlapXChord.Activate();
            xsc->m_TEFlapYFrac.Activate();
            xsc->m_TEFlapDeflection.Activate();

            if ( xsc->m_TEFlapType() == vsp::TRIM_NONE )
            {
                m_FlapTEXSlider.Deactivate();
                m_FlapTEYSlider.Deactivate();
                m_FlapTEDeflectionSlider.Deactivate();
                m_TEFlapABSButton.Deactivate();
                m_TEFlapRELButton.Deactivate();

                xsc->m_TEFlapX.Deactivate();
                xsc->m_TEFlapXChord.Deactivate();
                xsc->m_TEFlapYFrac.Deactivate();
                xsc->m_TEFlapDeflection.Deactivate();
            }
            else
            {
                if ( xsc->m_TEFlapAbsRel() == vsp::ABS )
                {
                    xsc->m_TEFlapXChord.Deactivate();
                }
                else
                {
                    xsc->m_TEFlapX.Deactivate();
                }
            }

            if ( xsc->m_TEFlapAbsRel() == vsp::ABS )
            {
                m_FlapTEXSlider.Update( 1, xsc->m_TEFlapX.GetID(), xsc->m_TEFlapXChord.GetID() );
            }
            else
            {
                m_FlapTEXSlider.Update( 2, xsc->m_TEFlapX.GetID(), xsc->m_TEFlapXChord.GetID() );
            }

            m_FlapTEYSlider.Update( xsc->m_TEFlapYFrac.GetID() );
            m_FlapTEDeflectionSlider.Update( xsc->m_TEFlapDeflection.GetID() );


            m_AFThetaSlider.Update( xsc->m_Theta.GetID() );
            m_AFScaleSlider.Update( xsc->m_Scale.GetID() );
            m_AFDeltaXSlider.Update( xsc->m_DeltaX.GetID() );
            m_AFDeltaYSlider.Update( xsc->m_DeltaY.GetID() );
            m_AFShiftLESlider.Update( xsc->m_ShiftLE.GetID() );
        }
    }

    return true;
}

void PropScreen::GuiDeviceCallBack( GuiDevice* gui_device )
{
    //==== Find Propeller Ptr ====//
    Geom* geom_ptr = m_ScreenMgr->GetCurrGeom();
    if ( !geom_ptr || geom_ptr->GetType().m_Type != PROP_GEOM_TYPE )
    {
        return;
    }
    PropGeom* propeller_ptr = dynamic_cast< PropGeom* >( geom_ptr );
    assert( propeller_ptr );

    if ( gui_device == &m_CutXSec )
    {
        propeller_ptr->CutActiveXSec();
    }
    else if ( gui_device == &m_CopyXSec   )
    {
        propeller_ptr->CopyActiveXSec();
    }
    else if ( gui_device == &m_PasteXSec  )
    {
        propeller_ptr->PasteActiveXSec();
    }
    else if ( gui_device == &m_InsertXSec  )
    {
        propeller_ptr->InsertXSec( );
    }
    else if ( gui_device == &m_CurveChoice )
    {
        m_EditCurve = m_CurveChoice.GetVal();
    }
    else if ( gui_device == &m_ApproxAllButton )
    {
        propeller_ptr->ApproxCubicAllPCurves();
        propeller_ptr->Update();
    }
    else if ( gui_device == &m_ResetThickness )
    {
        propeller_ptr->ResetThickness();
        propeller_ptr->Update();
    }
    else if ( gui_device == &m_ModXSecCurveNameInput )
    {
        GeomXSec* xsec_geom_ptr = dynamic_cast<GeomXSec*>(geom_ptr);
        assert( xsec_geom_ptr );

        XSec* xs = xsec_geom_ptr->GetXSec( xsec_geom_ptr->m_ActiveXSec() );

        if (xs)
        {
            XSecCurve* xsc = xs->GetXSecCurve();
            if ( xsc )
            {
                xsc->SetGroupAlias( m_ModXSecCurveNameInput.GetString() );
            }
        }
        ParmMgr.SetDirtyFlag( true );
    }

    XSecScreen::GuiDeviceCallBack( gui_device );
}

//==== Fltk  Callbacks ====//
void PropScreen::CallBack( Fl_Widget *w )
{
    XSecScreen::CallBack( w );
}
