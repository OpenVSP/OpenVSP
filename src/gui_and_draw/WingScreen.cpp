//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#include "WingScreen.h"
#include "ScreenMgr.h"
#include "ParmMgr.h"

using namespace vsp;

//==== Constructor ====//
WingScreen::WingScreen( ScreenMgr* mgr ) : BlendScreen( mgr, 400, 680 + 40, "Wing" )
{
    m_CurrDisplayGroup = NULL;

    Fl_Group* plan_tab = AddTab( "Plan", 3 );
    Fl_Group* plan_group = AddSubGroup( plan_tab, 5 );

    m_PlanLayout.SetGroupAndScreen( plan_group, this );

    m_PlanLayout.AddDividerBox( "Total Planform" );

    m_PlanLayout.SetButtonWidth( 100 );
    m_PlanLayout.AddSlider( m_PlanSpanSlider, "Span", 10, "%6.5f" );
    m_PlanLayout.AddSlider( m_PlanProjSpanSlider, "Proj Span", 10, "%6.5f" );
    m_PlanLayout.AddSlider( m_PlanChordSlider, "Chord", 10, "%6.5f" );
    m_PlanLayout.AddSlider( m_PlanAreaSlider, "Area", 10, "%6.5f" );
    m_PlanLayout.AddOutput( m_PlanAROutput, "Aspect Ratio", "%6.5f" );

    m_PlanLayout.AddYGap();
    
    m_PlanLayout.AddDividerBox( "Tip Treatment" );
    m_PlanLayout.AddSlider( m_CapTessSlider, "Cap Tess", 10, "%3.0f" );

    m_PlanLayout.AddYGap();

    m_RootCapTypeChoice.AddItem("None");
    m_RootCapTypeChoice.AddItem("Flat");
    m_RootCapTypeChoice.AddItem("Round");
    m_RootCapTypeChoice.AddItem("Edge");
    m_RootCapTypeChoice.AddItem("Sharp");
    m_PlanLayout.AddChoice(m_RootCapTypeChoice, "Root Cap Type");

    m_PlanLayout.AddSlider( m_RootCapLenSlider, "Length", 1, "%6.5f" );
    m_PlanLayout.AddSlider( m_RootCapOffsetSlider, "Offset", 1, "%6.5f" );
    m_PlanLayout.AddSlider( m_RootCapStrengthSlider, "Strength", 1, "%6.5f" );
    m_PlanLayout.AddButton( m_RootCapSweepFlagButton, "Sweep Stretch" );

    m_PlanLayout.AddYGap();

    m_TipCapTypeChoice.AddItem("None");
    m_TipCapTypeChoice.AddItem("Flat");
    m_TipCapTypeChoice.AddItem("Round");
    m_TipCapTypeChoice.AddItem("Edge");
    m_TipCapTypeChoice.AddItem("Sharp");
    m_PlanLayout.AddChoice(m_TipCapTypeChoice, "Tip Cap Type");

    m_PlanLayout.AddSlider( m_TipCapLenSlider, "Length", 1, "%6.5f" );
    m_PlanLayout.AddSlider( m_TipCapOffsetSlider, "Offset", 1, "%6.5f" );
    m_PlanLayout.AddSlider( m_TipCapStrengthSlider, "Strength", 1, "%6.5f" );
    m_PlanLayout.AddButton( m_TipCapSweepFlagButton, "Sweep Stretch" );

    m_PlanLayout.AddYGap();
    m_PlanLayout.AddDividerBox( "Root Incidence" );
    m_PlanLayout.AddSlider( m_IncidenceSlider, "Incidence", 10, "%6.5f" );
    m_PlanLayout.AddSlider( m_IncidenceLocSlider, "Incidence Loc", 10, "%6.5f" );

    m_PlanLayout.AddYGap();
    m_PlanLayout.AddDividerBox( "Tessellation Control" );
    m_PlanLayout.AddSlider( m_LEClusterSlider, "LE Clustering", 1, "%6.5f" );
    m_PlanLayout.AddSlider( m_TEClusterSlider, "TE Clustering", 1, "%6.5f" );
    m_PlanLayout.AddYGap();
    m_PlanLayout.SetButtonWidth( 200 );
    m_PlanLayout.AddOutput( m_SmallPanelWOutput, "Minimum LE/TE Panel Width","%6.4g" );
    m_PlanLayout.AddOutput( m_MaxGrowthOutput, "Maximum Growth Ratio", "%6.3f" );

    m_PlanLayout.AddYGap();
    m_PlanLayout.AddDividerBox( "Wing Loft Control" );

    m_PlanLayout.SetFitWidthFlag( false );
    m_PlanLayout.SetSameLineFlag( true );
    m_PlanLayout.AddLabel( "Twist Reference:", 170 );
    m_PlanLayout.SetButtonWidth( m_PlanLayout.GetRemainX() / 2 );
    m_PlanLayout.AddButton( m_TwistRelativeToggle, "Rel" );
    m_PlanLayout.AddButton( m_TwistAbsoluteToggle, "Abs" );
    m_PlanLayout.ForceNewLine();

    m_TwistAbsRelToggle.Init( this );
    m_TwistAbsRelToggle.AddButton( m_TwistAbsoluteToggle.GetFlButton() );
    m_TwistAbsRelToggle.AddButton( m_TwistRelativeToggle.GetFlButton() );

    m_PlanLayout.SetFitWidthFlag( false );
    m_PlanLayout.SetSameLineFlag( true );
    m_PlanLayout.AddLabel( "Dihedral Reference:", 170 );
    m_PlanLayout.SetButtonWidth( m_PlanLayout.GetRemainX() / 2 );
    m_PlanLayout.AddButton( m_DihedralRelativeToggle, "Rel" );
    m_PlanLayout.AddButton( m_DihedralAbsoluteToggle, "Abs" );
    m_PlanLayout.ForceNewLine();

    m_DihedralAbsRelToggle.Init( this );
    m_DihedralAbsRelToggle.AddButton( m_DihedralAbsoluteToggle.GetFlButton() );
    m_DihedralAbsRelToggle.AddButton( m_DihedralRelativeToggle.GetFlButton() );

    m_PlanLayout.SetFitWidthFlag( true );
    m_PlanLayout.SetSameLineFlag( false );

    m_PlanLayout.AddButton( m_RotateFoilMatchDihedral, "Rotate All Foils To Match Dihedral" );
    m_PlanLayout.AddButton( m_RotateRootFoilMatchDihedral, "Rotate Wing Root Foil To Match Dihedral" );
    m_PlanLayout.AddButton( m_CorrectFoil, "Correct Foil Thickness For Dihedral Rotation" );

    Fl_Group* sect_tab = AddTab( "Sect", 4 );
    Fl_Group* sect_group = AddSubGroup( sect_tab, 5 );

    m_SectionLayout.SetGroupAndScreen( sect_group, this );
    m_SectionLayout.AddDividerBox( "Wing Section" );

    m_SectionLayout.AddIndexSelector( m_SectIndexSelector );
    m_SectionLayout.AddYGap();

    m_SectionLayout.SetFitWidthFlag( false );
    m_SectionLayout.SetSameLineFlag( true );

    m_SectionLayout.SetButtonWidth( m_SectionLayout.GetRemainX() / 5 );

    m_SectionLayout.AddButton( m_SplitSectButton, "Split" );
    m_SectionLayout.AddButton( m_CutSectButton, "Cut" );
    m_SectionLayout.AddButton( m_CopySectButton, "Copy" );
    m_SectionLayout.AddButton( m_PasteSectButton, "Paste" );
    m_SectionLayout.AddButton( m_InsertSectButton, "Insert" );

    m_SectionLayout.ForceNewLine();
    m_SectionLayout.AddYGap();

    m_SectionLayout.SetFitWidthFlag( true );
    m_SectionLayout.SetSameLineFlag( false );

    m_SectionLayout.SetButtonWidth( 4*m_SectionLayout.GetRemainX()/5 );
    m_SectionLayout.AddOutput( m_NumSectOutput, "Num Sections" );
    m_SectionLayout.AddYGap();

    m_SectionLayout.SetButtonWidth( 74 );

    m_SectionLayout.AddDividerBox( "Interpolated XSecs" );
    m_SectionLayout.AddSlider( m_SectUTessSlider, "Num U", 20, " %5.0f" );
    m_SectionLayout.AddSlider( m_RootClusterSlider, "Rt. Cluster", 1, "%6.5f" );
    m_SectionLayout.AddSlider( m_TipClusterSlider, "Tip Cluster", 1, "%6.5f" );

    m_SectionLayout.AddYGap();

    m_SectionLayout.AddDividerBox( "Section Planform" );

    m_SectionLayout.SetFitWidthFlag( true );
    m_SectionLayout.SetSameLineFlag( false );

    vector < string > wsect_driver_labels;
    wsect_driver_labels.resize( vsp::NUM_WSECT_DRIVER );

    wsect_driver_labels[vsp::AR_WSECT_DRIVER] = string( "AR" );
    wsect_driver_labels[vsp::SPAN_WSECT_DRIVER] = "Span";
    wsect_driver_labels[vsp::AREA_WSECT_DRIVER] = "Area";
    wsect_driver_labels[vsp::TAPER_WSECT_DRIVER] = "Taper";
    wsect_driver_labels[vsp::AVEC_WSECT_DRIVER] = "Ave C";
    wsect_driver_labels[vsp::ROOTC_WSECT_DRIVER] = "Root C";
    wsect_driver_labels[vsp::TIPC_WSECT_DRIVER] = "Tip C";
    wsect_driver_labels[vsp::SECSWEEP_WSECT_DRIVER] = "Sec SW";

    m_SectionLayout.SetButtonWidth( 50 );

    m_WingDriverGroupBank.SetDriverGroup( &m_DefaultWingDriverGroup );
    m_SectionLayout.AddDriverGroupBank( m_WingDriverGroupBank, wsect_driver_labels, 10, "%6.5f" );

    m_SectionLayout.SetButtonWidth( 4*m_SectionLayout.GetRemainX()/5 );

    m_SectionLayout.AddOutput( m_SectProjSpanOutput, "Projected Span", "%6.4f" );

    m_SectionLayout.AddYGap();
    m_SectionLayout.SetButtonWidth( 74 );
    m_SectionLayout.AddDividerBox( "Sweep" );

    m_SectionLayout.AddSlider( m_SweepSlider, "Sweep", 10, "%6.5f" );
    m_SectionLayout.AddSlider( m_SweepLocSlider, "Sweep Loc", 10, "%6.5f" );
    m_SectionLayout.AddSlider( m_SecSweepLocSlider, "Sec SW Loc", 10, "%6.5f" );

    m_SectionLayout.AddYGap();
    m_SectionLayout.AddDividerBox( "Twist" );
    m_SectionLayout.AddSlider( m_TwistSlider, "Twist", 10, "%6.5f" );
    m_SectionLayout.AddSlider( m_TwistLocSlider, "Twist Loc", 10, "%6.5f" );

    m_SectionLayout.SetFitWidthFlag( true );
    m_SectionLayout.SetSameLineFlag( false );

    m_SectionLayout.AddYGap();
    m_SectionLayout.AddDividerBox( "Dihedral" );
    m_SectionLayout.AddSlider( m_DihedralSlider, "Dihedral", 10, "%6.5f" );

    m_SectionLayout.SetButtonWidth( m_SectionLayout.GetRemainX() );
    m_SectionLayout.AddYGap();
    m_SectionLayout.AddButton( m_RotateThisFoilMatchDihedral, "Rotate Section Tip Foil To Match Dihedral" );
    m_SectionLayout.SetButtonWidth( 74 );

    // Xsec layout after index selector has been defined in base class
    m_XSecLayout.SetFitWidthFlag(false);
    m_XSecLayout.SetSameLineFlag(true);
    m_XSecLayout.SetButtonWidth((m_XSecLayout.GetRemainX() - 10) / 2);
    m_XSecLayout.AddButton(m_CopyAfButton, "Copy");
    m_XSecLayout.AddX(10);
    m_XSecLayout.AddButton(m_PasteAfButton, "Paste");
    m_XSecLayout.ForceNewLine();
    m_XSecLayout.AddYGap();

    // Update default labels 
    m_XSecTab->copy_label( "Airfoil" );
    m_XSecDivider->copy_label( "Airfoil Section" );

    m_XSecDriversActive = false;
    AddXSecLayout();

    //==== TE Trim ====//

    Fl_Group* modify_tab = AddTab( "Modify" );
    Fl_Group* modify_group = AddSubGroup( modify_tab, 5 );

    m_ModifyLayout.SetButtonWidth( 70 );

    m_ModifyLayout.SetGroupAndScreen( modify_group, this );
    m_ModifyLayout.AddDividerBox( "Airfoil Section" );

    m_ModifyLayout.AddIndexSelector( m_AfModIndexSelector );

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

    m_SubSurfChoice.AddItem( SubSurface::GetTypeName( vsp::SS_CONTROL ), vsp::SS_CONTROL );
    m_SubSurfChoice.UpdateItems();
}

//==== Show Wing Screen ====//
WingScreen::~WingScreen()
{
    delete m_CSTUpCoeffScroll;
    delete m_CSTLowCoeffScroll;
}

//==== Show Wing Screen ====//
void WingScreen::Show()
{
    if ( Update() )
    {
        m_FLTK_Window->show();
    }
}

//==== Update Wing Screen ====//
bool WingScreen::Update()
{
    assert( m_ScreenMgr );
    char str[256];

    Geom* geom_ptr = m_ScreenMgr->GetCurrGeom();
    if ( !geom_ptr || geom_ptr->GetType().m_Type != MS_WING_GEOM_TYPE )
    {
        Hide();
        return false;
    }

    BlendScreen::Update();
    m_NumUSlider.Deactivate();

    WingGeom* wing_ptr = dynamic_cast< WingGeom* >( geom_ptr );
    assert( wing_ptr );

    //==== Plan Parms ===//
    m_PlanSpanSlider.Update( wing_ptr->m_TotalSpan.GetID() );
    m_PlanProjSpanSlider.Update( wing_ptr->m_TotalProjSpan.GetID() );
    m_PlanChordSlider.Update( wing_ptr->m_TotalChord.GetID() );
    m_PlanAreaSlider.Update( wing_ptr->m_TotalArea.GetID() );

    m_LEClusterSlider.Update( wing_ptr->m_LECluster.GetID() );
    m_TEClusterSlider.Update( wing_ptr->m_TECluster.GetID() );

    m_SmallPanelWOutput.Update( wing_ptr->m_SmallPanelW.GetID() );
    m_MaxGrowthOutput.Update( wing_ptr->m_MaxGrowth.GetID() );
    m_PlanAROutput.Update( wing_ptr->m_TotalAR.GetID() );

    m_RootCapTypeChoice.Update( wing_ptr->m_CapUMinOption.GetID() );
    m_TipCapTypeChoice.Update( wing_ptr->m_CapUMaxOption.GetID() );

    m_CapTessSlider.Update( wing_ptr->m_CapUMinTess.GetID() );

    m_RootCapLenSlider.Update( wing_ptr->m_CapUMinLength.GetID() );
    m_RootCapOffsetSlider.Update( wing_ptr->m_CapUMinOffset.GetID() );
    m_RootCapStrengthSlider.Update( wing_ptr->m_CapUMinStrength.GetID() );
    m_RootCapSweepFlagButton.Update( wing_ptr->m_CapUMinSweepFlag.GetID() );

    m_RootCapLenSlider.Deactivate();
    m_RootCapOffsetSlider.Deactivate();
    m_RootCapStrengthSlider.Deactivate();
    m_RootCapSweepFlagButton.Deactivate();

    switch( wing_ptr->m_CapUMinOption() ){
        case NO_END_CAP:
            break;
        case FLAT_END_CAP:
            break;
        case ROUND_END_CAP:
            m_RootCapLenSlider.Activate();
            m_RootCapOffsetSlider.Activate();
            m_RootCapSweepFlagButton.Activate();
            break;
        case EDGE_END_CAP:
            m_RootCapLenSlider.Activate();
            m_RootCapOffsetSlider.Activate();
            m_RootCapSweepFlagButton.Activate();
            break;
        case SHARP_END_CAP:
            m_RootCapLenSlider.Activate();
            m_RootCapOffsetSlider.Activate();
            m_RootCapStrengthSlider.Activate();
            m_RootCapSweepFlagButton.Activate();
            break;
    }

    m_TipCapLenSlider.Update( wing_ptr->m_CapUMaxLength.GetID() );
    m_TipCapOffsetSlider.Update( wing_ptr->m_CapUMaxOffset.GetID() );
    m_TipCapStrengthSlider.Update( wing_ptr->m_CapUMaxStrength.GetID() );
    m_TipCapSweepFlagButton.Update( wing_ptr->m_CapUMaxSweepFlag.GetID() );

    m_TipCapLenSlider.Deactivate();
    m_TipCapOffsetSlider.Deactivate();
    m_TipCapStrengthSlider.Deactivate();
    m_TipCapSweepFlagButton.Deactivate();

    switch( wing_ptr->m_CapUMaxOption() ){
        case NO_END_CAP:
            break;
        case FLAT_END_CAP:
            break;
        case ROUND_END_CAP:
            m_TipCapLenSlider.Activate();
            m_TipCapOffsetSlider.Activate();
            m_TipCapSweepFlagButton.Activate();
            break;
        case EDGE_END_CAP:
            m_TipCapLenSlider.Activate();
            m_TipCapOffsetSlider.Activate();
            m_TipCapSweepFlagButton.Activate();
            break;
        case SHARP_END_CAP:
            m_TipCapLenSlider.Activate();
            m_TipCapOffsetSlider.Activate();
            m_TipCapStrengthSlider.Activate();
            m_TipCapSweepFlagButton.Activate();
            break;
    }

    WingSect* root_sect = dynamic_cast<WingSect*>(wing_ptr->GetXSec( 0 ));

    if ( root_sect )
    {
        m_IncidenceSlider.Update( root_sect->m_Twist.GetID() );
        m_IncidenceLocSlider.Update( root_sect->m_TwistLoc.GetID() );
        m_RotateRootFoilMatchDihedral.Update( root_sect->m_RotateMatchDiedralFlag.GetID() );
    }

    sprintf( str, "       %d", wing_ptr->NumXSec()-1 );
    m_NumSectOutput.Update(  str );

    ////==== Wing Section Index Display ====//
    int ws_index = wing_ptr->m_ActiveWingSection();
    m_SectIndexSelector.Update( wing_ptr->m_ActiveWingSection.GetID() );

    WingSect* wing_sect = dynamic_cast<WingSect*>(wing_ptr->GetXSec( ws_index ));

    if ( wing_sect )
    {
        m_SectUTessSlider.Update( wing_sect->m_SectTessU.GetID() );

        m_RootClusterSlider.Update( wing_sect->m_RootCluster.GetID() );
        m_TipClusterSlider.Update( wing_sect->m_TipCluster.GetID() );

        m_WingDriverGroupBank.SetDriverGroup( &wing_sect->m_DriverGroup );
        vector< string > parm_ids = wing_sect->GetDriverParms();
        //wing_sect->m_DriverGroup.UpdateGroup( parm_ids );
        m_WingDriverGroupBank.Update( parm_ids );

        m_SweepSlider.Update( wing_sect->m_Sweep.GetID() );
        m_SweepLocSlider.Update( wing_sect->m_SweepLoc.GetID() );
        m_SecSweepLocSlider.Update( wing_sect->m_SecSweepLoc.GetID() );
        m_TwistSlider.Update( wing_sect->m_Twist.GetID() );
        m_TwistLocSlider.Update( wing_sect->m_TwistLoc.GetID() );

        m_DihedralSlider.Update( wing_sect->m_Dihedral.GetID() );

        m_DihedralAbsRelToggle.Update( wing_ptr->m_RelativeDihedralFlag.GetID() );
        m_TwistAbsRelToggle.Update( wing_ptr->m_RelativeTwistFlag.GetID() );

        m_RotateFoilMatchDihedral.Update( wing_ptr->m_RotateAirfoilMatchDiedralFlag.GetID() );
        m_RotateThisFoilMatchDihedral.Update( wing_sect->m_RotateMatchDiedralFlag.GetID() );

        m_CorrectFoil.Update( wing_ptr->m_CorrectAirfoilThicknessFlag.GetID() );

        m_SectProjSpanOutput.Update( wing_sect->m_ProjectedSpan.GetID() );

    }

    //==== XSec Index Display ===//
    int xsid = wing_ptr->m_ActiveXSec();
    m_AfModIndexSelector.Update( wing_ptr->m_ActiveXSec.GetID() ); // Set same as m_XSecIndexSelector

    WingSect* ws = ( WingSect* ) wing_ptr->GetXSec( xsid );
    if ( ws )
    {
        XSecCurve* xsc = ws->GetXSecCurve();
        if ( xsc )
        {
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

            m_TECapLengthSlider.Deactivate();
            m_TECapOffsetSlider.Deactivate();
            m_TECapStrengthSlider.Deactivate();

            switch( xsc->m_TECapType() ){
                case FLAT_END_CAP:
                    break;
                case ROUND_END_CAP:
                    m_TECapLengthSlider.Activate();
                    m_TECapOffsetSlider.Activate();
                    break;
                case EDGE_END_CAP:
                    m_TECapLengthSlider.Activate();
                    m_TECapOffsetSlider.Activate();
                    break;
                case SHARP_END_CAP:
                    m_TECapLengthSlider.Activate();
                    m_TECapOffsetSlider.Activate();
                    m_TECapStrengthSlider.Activate();
                    break;
            }

            m_LECapChoice.Update( xsc->m_LECapType.GetID() );

            m_LECapLengthSlider.Update( xsc->m_LECapLength.GetID() );
            m_LECapOffsetSlider.Update( xsc->m_LECapOffset.GetID() );
            m_LECapStrengthSlider.Update( xsc->m_LECapStrength.GetID() );

            m_LECapLengthSlider.Deactivate();
            m_LECapOffsetSlider.Deactivate();
            m_LECapStrengthSlider.Deactivate();

            switch( xsc->m_LECapType() ){
                case FLAT_END_CAP:
                    break;
                case ROUND_END_CAP:
                    m_LECapLengthSlider.Activate();
                    m_LECapOffsetSlider.Activate();
                    break;
                case EDGE_END_CAP:
                    m_LECapLengthSlider.Activate();
                    m_LECapOffsetSlider.Activate();
                    break;
                case SHARP_END_CAP:
                    m_LECapLengthSlider.Activate();
                    m_LECapOffsetSlider.Activate();
                    m_LECapStrengthSlider.Activate();
                    break;
            }

            m_AFThetaSlider.Update( xsc->m_Theta.GetID() );
            m_AFScaleSlider.Update( xsc->m_Scale.GetID() );
            m_AFDeltaXSlider.Update( xsc->m_DeltaX.GetID() );
            m_AFDeltaYSlider.Update( xsc->m_DeltaY.GetID() );
            m_AFShiftLESlider.Update( xsc->m_ShiftLE.GetID() );
        }
    }
    return true;
}

void WingScreen::GuiDeviceCallBack( GuiDevice* gui_device )
{
    //==== Find Fuselage Ptr ====//
    Geom* geom_ptr = m_ScreenMgr->GetCurrGeom();
    if ( !geom_ptr || geom_ptr->GetType().m_Type != MS_WING_GEOM_TYPE )
    {
        return;
    }
    WingGeom* wing_ptr = dynamic_cast< WingGeom* >( geom_ptr );
    assert( wing_ptr );

    if ( gui_device == &m_SplitSectButton )
    {
        int wsid = wing_ptr->m_ActiveWingSection();
        wing_ptr->SplitWingSect( wsid );
        wing_ptr->Update();
    }
    else if ( gui_device == &m_CutSectButton )
    {
        int wsid = wing_ptr->m_ActiveWingSection();
        wing_ptr->CutWingSect( wsid );
        wing_ptr->Update();
    }
    else if ( gui_device == &m_CopySectButton )
    {
        int wsid = wing_ptr->m_ActiveWingSection();
        wing_ptr->CopyWingSect( wsid );
        wing_ptr->Update();
    }
    else if ( gui_device == &m_PasteSectButton )
    {
        int wsid = wing_ptr->m_ActiveWingSection();
        wing_ptr->PasteWingSect( wsid );
        wing_ptr->Update();
    }
    else if ( gui_device == &m_InsertSectButton )
    {
        int wsid = wing_ptr->m_ActiveWingSection();
        wing_ptr->InsertWingSect( wsid );
        wing_ptr->Update();
    }
    else if (gui_device == &m_CopyAfButton)
    {
        int afid = wing_ptr->m_ActiveXSec();
        wing_ptr->CopyAirfoil( afid );
    }
    else if (gui_device == &m_PasteAfButton)
    {
        int afid = wing_ptr->m_ActiveXSec();
        wing_ptr->PasteAirfoil( afid );
        wing_ptr->Update();
    }
    else if ( gui_device == &m_TestDriverGroupButton )
    {
        int wsid = wing_ptr->m_ActiveWingSection();

        WingSect* wing_sect = dynamic_cast<WingSect*>(wing_ptr->GetXSec( wsid ));

        if ( wing_sect )
        {
            vector< string > parm_ids = wing_sect->GetDriverParms();
            wing_sect->m_DriverGroup.Test( parm_ids, 1e-4 );
        }
    }

    BlendScreen::GuiDeviceCallBack( gui_device );
}

//==== Fltk  Callbacks ====//
void WingScreen::CallBack( Fl_Widget *w )
{
    BlendScreen::CallBack(w);
}