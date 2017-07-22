//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// VSPAEROScreen.cpp: implementation of the VSPAEROScreen class.
//
//////////////////////////////////////////////////////////////////////

#include "VSPAEROScreen.h"
#include "VSPAEROPlotScreen.h"
#include "StringUtil.h"
#include "FileUtil.h"
#include <FL/fl_ask.H>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
#define VSPAERO_SCREEN_WIDTH 610
#define VSPAERO_SCREEN_HEIGHT 650
#define VSPAERO_EXECUTE_CONSTANT_HEIGHT 210

VSPAEROScreen::VSPAEROScreen( ScreenMgr* mgr ) : TabScreen( mgr, VSPAERO_SCREEN_WIDTH,
    VSPAERO_SCREEN_HEIGHT, "VSPAERO", VSPAERO_EXECUTE_CONSTANT_HEIGHT )
{
    m_NumVarAngle = 0;
    m_NumVarDeflection = 0;

    m_FLTK_Window->callback( staticCloseCB, this );

    m_SolverPair = make_pair( &VSPAEROMgr, this );  //solverpair type
    m_ViewerPair = make_pair( this, VSPAERO_VIEWER );   //monitorpair type

    int window_border_width = 5;
    int group_border_width = 2;

    //==== Constant Console Area ====//
    m_ConstantAreaLayout.SetGroupAndScreen( m_FLTK_Window, this );
    m_ConstantAreaLayout.AddY( m_ConstantAreaLayout.GetRemainY()
        - 10 * m_ConstantAreaLayout.GetStdHeight()
        - 2 * m_ConstantAreaLayout.GetGapHeight() );
    m_ConstantAreaLayout.AddX( window_border_width );
    m_ConstantAreaLayout.AddYGap();

    // Console
    m_ConstantAreaLayout.AddSubGroupLayout( m_ConsoleLayout, m_ConstantAreaLayout.GetRemainX() - window_border_width,
        m_ConstantAreaLayout.GetRemainY() );

    m_SolverDisplay = m_ConsoleLayout.AddFlTextDisplay( m_ConsoleLayout.GetRemainY() - 3 * m_ConsoleLayout.GetStdHeight() - window_border_width );
    m_SolverBuffer = new Fl_Text_Buffer;
    m_SolverDisplay->buffer( m_SolverBuffer );

    m_ConsoleLayout.SetButtonWidth( m_ConsoleLayout.GetW() / 2 );

    m_ConsoleLayout.SetSameLineFlag( true );
    m_ConsoleLayout.SetFitWidthFlag( false );

    m_ConsoleLayout.AddButton( m_SolverButton, "Launch Solver" );
    m_ConsoleLayout.AddButton( m_KillSolverButton, "Kill Solver" );

    m_ConsoleLayout.ForceNewLine();

    m_ConsoleLayout.AddButton( m_PlotButton, "Show Results Mgr" );
    m_ConsoleLayout.AddButton( m_ViewerButton, "Launch Viewer" );

    m_ConsoleLayout.ForceNewLine();

    m_ConsoleLayout.SetFitWidthFlag( true );
    m_ConsoleLayout.SetSameLineFlag( false );
    //m_ExecuteLayout.AddButton( m_ExportResultsToMatlabButton, "Export to *.m" );
    m_ConsoleLayout.AddButton( m_ExportResultsToCsvButton, "Export to *.csv" );
    m_ExportResultsToCsvButton.Deactivate();

    // Execute
    int execute_height = m_ConstantAreaLayout.GetStdHeight();
    m_ConstantAreaLayout.AddSubGroupLayout( m_ExecuteLayout, m_ConstantAreaLayout.GetW(), execute_height );

    //==== Overview Tab ====//
    Fl_Group* overview_tab = AddTab( "Overview" );
    Fl_Group* overview_group = AddSubGroup( overview_tab, window_border_width );
    m_OverviewLayout.SetGroupAndScreen( overview_group, this );

    // Column layout
    int left_col_width = (m_OverviewLayout.GetW()-window_border_width) / 2;
    int col_height = m_OverviewLayout.GetH();
    m_OverviewLayout.AddSubGroupLayout( m_LeftColumnLayout, left_col_width, col_height );

    m_OverviewLayout.AddX( m_LeftColumnLayout.GetW() + window_border_width );

    int right_col_width = m_OverviewLayout.GetRemainX();
    m_OverviewLayout.AddSubGroupLayout( m_RightColumnLayout, right_col_width, col_height );

    // Set column YGap Width
    m_LeftColumnLayout.SetGapHeight( group_border_width );
    m_RightColumnLayout.SetGapHeight( group_border_width );

    // Case Setup Layout
    m_LeftColumnLayout.AddSubGroupLayout( m_CaseSetupLayout,
        m_LeftColumnLayout.GetW(),
        4 * m_LeftColumnLayout.GetStdHeight() );
    m_LeftColumnLayout.AddY( m_CaseSetupLayout.GetH() );

    m_CaseSetupLayout.AddDividerBox( "Case Setup" );

    m_CaseSetupLayout.SetSameLineFlag( true );
    m_CaseSetupLayout.SetFitWidthFlag( false );

    // Analysis method radio button group setup
    m_CaseSetupLayout.SetButtonWidth( m_CaseSetupLayout.GetW() / 2 );
    m_CaseSetupLayout.AddButton( m_AeroMethodToggleVLM, "Vortex Lattice (VLM)" );
    m_CaseSetupLayout.AddButton( m_AeroMethodTogglePanel, "Panel Method" );

    m_AeroMethodToggleGroup.Init( this );
    m_AeroMethodToggleGroup.AddButton( m_AeroMethodToggleVLM.GetFlButton() );
    m_AeroMethodToggleGroup.AddButton( m_AeroMethodTogglePanel.GetFlButton() );

    vector< int > val_map;
    val_map.push_back( vsp::VORTEX_LATTICE );
    val_map.push_back( vsp::PANEL );
    m_AeroMethodToggleGroup.SetValMapVec( val_map );

    m_CaseSetupLayout.ForceNewLine();

    m_CaseSetupLayout.InitWidthHeightVals();
    m_CaseSetupLayout.SetSameLineFlag( false );
    m_CaseSetupLayout.SetFitWidthFlag( true );
    m_CaseSetupLayout.AddChoice( m_GeomSetChoice, "Geometry Set:" );

    m_CaseSetupLayout.AddYGap();

    m_CaseSetupLayout.AddButton( m_PreviewDegenButton, "Preview VLM Geometry" );

    m_LeftColumnLayout.AddYGap();
    m_LeftColumnLayout.AddYGap();

    // Reference Quantities
    m_LeftColumnLayout.AddSubGroupLayout( m_RefLengthLayout,
        m_LeftColumnLayout.GetW(),
        6 * m_LeftColumnLayout.GetStdHeight() );
    m_LeftColumnLayout.AddY( m_RefLengthLayout.GetH() );

    m_RefLengthLayout.AddDividerBox( "Reference Area, Lengths" );

    m_RefLengthLayout.SetSameLineFlag( true );
    m_RefLengthLayout.SetFitWidthFlag( false );

    m_RefLengthLayout.SetButtonWidth( m_RefLengthLayout.GetW() / 2 );

    m_RefLengthLayout.AddButton( m_RefManualToggle, "Manual" );
    m_RefLengthLayout.AddButton( m_RefChoiceToggle, "From Model" );
    m_RefLengthLayout.ForceNewLine();

    m_RefLengthLayout.InitWidthHeightVals();

    m_RefLengthLayout.SetSameLineFlag( false );
    m_RefLengthLayout.SetFitWidthFlag( true );

    m_RefLengthLayout.AddChoice( m_RefWingChoice, "Ref. Wing" );

    m_RefLengthLayout.AddSlider( m_SrefSlider, "Sref", 1000.0, "%7.3f" );
    m_RefLengthLayout.AddSlider( m_brefSlider, "bref", 100.0, "%7.3f" );
    m_RefLengthLayout.AddSlider( m_crefSlider, "cref", 100.0, "%7.3f" );

    m_RefToggle.Init( this );
    m_RefToggle.AddButton( m_RefManualToggle.GetFlButton() );
    m_RefToggle.AddButton( m_RefChoiceToggle.GetFlButton() );

    m_LeftColumnLayout.AddYGap();

    // CG
    m_LeftColumnLayout.AddSubGroupLayout( m_MomentRefLayout,
        m_LeftColumnLayout.GetW(),
        6 * m_LeftColumnLayout.GetStdHeight() );
    m_LeftColumnLayout.AddY( m_MomentRefLayout.GetH() );

    m_MomentRefLayout.AddDividerBox( "Moment Reference Position" );

    m_MomentRefLayout.SetButtonWidth( 125 );

    m_MomentRefLayout.SetSameLineFlag( true );
    m_MomentRefLayout.AddChoice( m_CGSetChoice, "Mass Set:",  m_MomentRefLayout.GetButtonWidth() );
    m_MomentRefLayout.SetFitWidthFlag( false );
    m_MomentRefLayout.AddButton( m_MassPropButton, "Calc CG" );
    m_MomentRefLayout.ForceNewLine();

    m_MomentRefLayout.SetSameLineFlag( false );
    m_MomentRefLayout.SetFitWidthFlag( true );

    m_MomentRefLayout.InitWidthHeightVals();

    m_MomentRefLayout.AddSlider( m_NumSliceSlider, "Num Slices", 100, "%4.0f" );
    m_MomentRefLayout.AddSlider( m_XcgSlider, "Xref", 100.0, "%7.3f" );
    m_MomentRefLayout.AddSlider( m_YcgSlider, "Yref", 100.0, "%7.3f" );
    m_MomentRefLayout.AddSlider( m_ZcgSlider, "Zref", 100.0, "%7.3f" );

    m_LeftColumnLayout.AddYGap();

    // Flow Condition
    m_RightColumnLayout.AddSubGroupLayout( m_FlowCondLayout,
        m_RightColumnLayout.GetW(),
        4 * m_RightColumnLayout.GetStdHeight() );
    m_RightColumnLayout.AddY( m_FlowCondLayout.GetH() );

    m_FlowCondLayout.AddDividerBox( "Flow Condition" );
    m_FlowCondLayout.SetSameLineFlag( false );
    m_FlowCondLayout.SetFitWidthFlag( true );

    m_FlowCondLayout.AddInputEvenSpacedVector( m_AlphaStartInput, m_AlphaEndInput, m_AlphaNptsInput, "Alpha", "%7.3f" );
    m_FlowCondLayout.AddInputEvenSpacedVector( m_BetaStartInput, m_BetaEndInput, m_BetaNptsInput, "Beta", "%7.3f" );
    m_FlowCondLayout.AddInputEvenSpacedVector( m_MachStartInput, m_MachEndInput, m_MachNptsInput, "Mach", "%7.3f" );

    m_RightColumnLayout.AddYGap();

    // Control Surface Angle Layout
    int deflection_angle_scroll_h = 150;
    m_RightColumnLayout.AddDividerBox( "Control Group Angles" ); //add the divider box outside the layout so the scroll works properly

    m_RightColumnLayout.AddSubGroupLayout( m_DeflectionAngleLayout,
        m_RightColumnLayout.GetW(),
        m_RightColumnLayout.GetRemainY() );

    m_DeflectionAngleScroll = AddSubScroll( m_DeflectionAngleLayout.GetGroup(), group_border_width );
    m_DeflectionAngleScroll->type( Fl_Scroll::VERTICAL_ALWAYS );
    m_DeflectionAngleScroll->resize( m_DeflectionAngleLayout.GetX(), m_DeflectionAngleLayout.GetY(),
        m_DeflectionAngleLayout.GetW(), m_DeflectionAngleLayout.GetH() );

    m_RightColumnLayout.AddY( deflection_angle_scroll_h );

    //==== Advanced Controls Tab ====//
    Fl_Group* advanced_tab = AddTab( "Advanced" );
    Fl_Group* advanced_group = AddSubGroup( advanced_tab, window_border_width );

    m_AdvancedLayout.SetGroupAndScreen( advanced_group, this );

    m_AdvancedLayout.AddSubGroupLayout( m_AdvancedLeftLayout,
        (advanced_group->w()-window_border_width) / 2,
        advanced_group->h() );

    m_AdvancedLayout.AddX( m_AdvancedLeftLayout.GetW() + window_border_width );
    m_AdvancedLayout.AddSubGroupLayout( m_AdvancedRightLayout,
        (advanced_group->w()-window_border_width) / 2,
        advanced_group->h() );

    // Advanced Case Setup Layout
    m_AdvancedLeftLayout.AddSubGroupLayout( m_AdvancedCaseSetupLayout,
        m_AdvancedLeftLayout.GetW(),
        8 * m_AdvancedLeftLayout.GetStdHeight() );
    m_AdvancedLeftLayout.AddY( m_AdvancedCaseSetupLayout.GetH() );

    m_AdvancedCaseSetupLayout.AddDividerBox( "Advanced Case Setup" );
    //  Degengeom output file selection, used for VLM & Panel methods
    int labelButtonWidth = 60;
    int fileButtonWidth = 25;
    int inputWidth = m_AdvancedCaseSetupLayout.GetW() - labelButtonWidth - fileButtonWidth;
    m_AdvancedCaseSetupLayout.SetFitWidthFlag( false );
    m_AdvancedCaseSetupLayout.SetSameLineFlag( true );
    m_AdvancedCaseSetupLayout.SetButtonWidth( labelButtonWidth );
    m_AdvancedCaseSetupLayout.SetInputWidth( inputWidth );

    m_AdvancedCaseSetupLayout.AddOutput( m_DegenFileName, "Degen" );

    m_AdvancedCaseSetupLayout.SetButtonWidth( fileButtonWidth );

    m_AdvancedCaseSetupLayout.AddButton( m_DegenFileButton, "..." );
    m_AdvancedCaseSetupLayout.ForceNewLine();

    //  CompGeom output file selection, used for Panel method only
    m_AdvancedCaseSetupLayout.SetButtonWidth( labelButtonWidth );
    m_AdvancedCaseSetupLayout.SetInputWidth( inputWidth );

    m_AdvancedCaseSetupLayout.AddOutput( m_CompGeomFileName, "Panel" );

    m_AdvancedCaseSetupLayout.SetButtonWidth( fileButtonWidth );

    m_AdvancedCaseSetupLayout.AddButton( m_CompGeomFileButton, "..." );
    m_AdvancedCaseSetupLayout.ForceNewLine();

    m_AdvancedCaseSetupLayout.SetFitWidthFlag( true );
    m_AdvancedCaseSetupLayout.SetSameLineFlag( false );

    m_AdvancedCaseSetupLayout.SetButtonWidth( 80 );
    m_AdvancedCaseSetupLayout.SetInputWidth( 50 );

    m_AdvancedCaseSetupLayout.AddSlider( m_NCPUSlider, "Num CPU", 10.0, "%3.0f" );
    m_AdvancedCaseSetupLayout.AddButton( m_BatchCalculationToggle, "Batch Calculation" );
    m_AdvancedCaseSetupLayout.AddButton( m_JacobiPreconditionToggle, "Jacobi Preconditioner");
    m_AdvancedCaseSetupLayout.AddButton( m_SymmetryToggle, "X-Z Symmetry" );
    m_AdvancedCaseSetupLayout.AddButton(m_Write2DFEMToggle, "Write 2D FEM");

    // Wake Layout
    m_AdvancedLeftLayout.AddSubGroupLayout( m_WakeLayout,
        m_AdvancedLeftLayout.GetW(),
        4 * m_AdvancedLeftLayout.GetStdHeight() );
    m_AdvancedLeftLayout.AddY( m_WakeLayout.GetH() );

    m_WakeLayout.AddDividerBox( "Wake" );
    m_WakeLayout.AddSlider( m_WakeNumIterSlider, "Num It.", 10, "%3.0f" );
    m_WakeLayout.AddSlider( m_WakeAvgStartIterSlider, "Avg Start It.", 11, "%3.0f" );
    m_WakeLayout.AddSlider( m_WakeSkipUntilIterSlider, "Skip Until It.", 11, "%3.0f" );

    // Other Setup Parms Layout
    m_AdvancedLeftLayout.AddSubGroupLayout( m_OtherParmsLayout,
        m_AdvancedLeftLayout.GetW(),
        8 * m_AdvancedLeftLayout.GetStdHeight() );
    m_AdvancedLeftLayout.AddY( m_OtherParmsLayout.GetH() );

    int togglewidth = 15;
    int labelwidth = 120;
    int inputwidth = 50;
    m_OtherParmsLayout.AddDividerBox( "Other" );
    m_OtherParmsLayout.SetSameLineFlag(true);
    m_OtherParmsLayout.SetFitWidthFlag(false);
    m_OtherParmsLayout.SetInputWidth(inputwidth);
    m_OtherParmsLayout.SetSliderWidth( m_OtherParmsLayout.GetW() - inputwidth - togglewidth - labelwidth - 20 );
    m_OtherParmsLayout.SetButtonWidth(togglewidth);
    m_OtherParmsLayout.AddButton( m_ClmaxToggle, "" );
    m_OtherParmsLayout.SetButtonWidth( labelwidth );
    m_OtherParmsLayout.AddSlider( m_ClmaxSlider, "Clmax", 10, "%2.3f" );
    m_OtherParmsLayout.ForceNewLine();
    m_OtherParmsLayout.SetButtonWidth(togglewidth);
    m_OtherParmsLayout.AddButton( m_MaxTurningToggle, "" );
    m_OtherParmsLayout.SetButtonWidth( labelwidth );
    m_OtherParmsLayout.AddSlider( m_MaxTurningSlider, "Max Turning Angle", 100, "%3.3f" );
    m_OtherParmsLayout.ForceNewLine();
    m_OtherParmsLayout.SetButtonWidth(togglewidth);
    m_OtherParmsLayout.AddButton( m_FarDistToggle, "" );
    m_OtherParmsLayout.SetButtonWidth( labelwidth );
    m_OtherParmsLayout.AddSlider( m_FarDistSlider, "Far Field Dist", 1e3, "%7.2f" );
    m_OtherParmsLayout.ForceNewLine();
    m_OtherParmsLayout.AddYGap();

    // Unsteady Setup
    int button_width = 130;
    m_AdvancedRightLayout.AddSubGroupLayout( m_UnsteadyLayout,
        m_AdvancedRightLayout.GetW(),
        m_AdvancedRightLayout.GetH());
    m_UnsteadyLayout.SetButtonWidth( button_width );
    m_UnsteadyLayout.AddDividerBox( "Unsteady" );
    m_UnsteadyLayout.AddButton( m_StabilityCalcToggle, "Stability Calculation" );
    m_UnsteadyLayout.AddChoice( m_StabilityTypeChoice, "Type" );
    m_StabilityTypeChoice.AddItem( "Stability" );
    m_StabilityTypeChoice.AddItem( "P Analysis" );
    m_StabilityTypeChoice.AddItem( "Q Analysis" );
    m_StabilityTypeChoice.AddItem( "R Analysis" );
    //m_StabilityTypeChoice.AddItem( "Heave" ); // To Be Implemented
    //m_StabilityTypeChoice.AddItem( "Impulse" ); // To Be Implemented
    m_StabilityTypeChoice.UpdateItems();

    //==== Rotor Disk Tab ==== //
    Fl_Group* rotor_tab = AddTab( "Rotor" );
    Fl_Group* rotor_group = AddSubGroup( rotor_tab, window_border_width );
    m_PropGeneralLayout.SetGroupAndScreen( rotor_group, this );

    // Prop General Layout
    m_PropGeneralLayout.AddDividerBox( "Rotor Disk General Settings" );
    m_PropGeneralLayout.AddSlider( m_VinfSlider, "Vinf", 100, "%7.2f" );
    m_PropGeneralLayout.AddSlider( m_RhoSlider, "Rho", 1, "%2.5g" );

    m_PropGeneralLayout.AddYGap();

    // Prop Element Layout
    int prop_elem_browser_h = 200;
    m_PropGeneralLayout.AddSubGroupLayout( m_PropElemLayout,
        m_PropGeneralLayout.GetW(),
        6 * m_PropGeneralLayout.GetStdHeight() + prop_elem_browser_h );
    m_PropGeneralLayout.AddY( m_PropElemLayout.GetH() );

    m_PropElemLayout.SetSameLineFlag( false );
    m_PropElemLayout.SetFitWidthFlag( true );

    m_PropElemLayout.AddDividerBox( "Rotor Disk Element Settings" );

    m_PropElemBrowser = m_PropElemLayout.AddFlBrowser( 0 );
    m_PropElemBrowser->resize( m_PropElemLayout.GetX(), m_PropElemLayout.GetY(), m_PropElemLayout.GetW(), prop_elem_browser_h );
    m_PropElemBrowser->type( FL_SELECT_BROWSER );
    m_PropElemBrowser->labelfont( 13 );
    m_PropElemBrowser->textsize( 12 );
    m_PropElemBrowser->callback( staticScreenCB, this );

    m_PropElemLayout.AddY( prop_elem_browser_h );

    int input_width = 60;
    int XYZ_button_width = 20;
    int else_button_width = 60;
    int browser_augment = 40;

    m_PropElemLayout.SetButtonWidth( else_button_width );
    m_PropElemLayout.SetInputWidth( input_width );
    m_PropElemLayout.AddOutput( m_PropElemDia, "Dia." );
    m_PropElemLayout.AddSlider( m_PropElemHubDia, "Hub Dia.", 100, "%3.3f" );
    m_PropElemLayout.AddSlider( m_PropElemRPM, "RPM", 10000, "%7.2f" );
    m_PropElemLayout.AddSlider( m_PropElemCT, "CT", 1, "%2.3f" );
    m_PropElemLayout.AddSlider( m_PropElemCP, "CP", 1, "%2.3f" );

    m_PropGeneralLayout.AddYGap();

    //==== Control Grouping Tab ====//
    int main_browser_w = 180;
    int browser_h = 130;
    int component_browser_w = 200;
    int browser_h_augment = 20;
    int main_browser_spacing = 18;
    int items_browsers_spacing = 2;
    Fl_Group* cs_grouping_tab = AddTab( "Control Grouping" );
    Fl_Group* cs_grouping_group = AddSubGroup( cs_grouping_tab, window_border_width );
    m_ControlSurfaceLayout.SetGroupAndScreen( cs_grouping_group, this );

    // Grouping Browsers Layout
    m_ControlSurfaceLayout.AddSubGroupLayout( m_CSGroupingLayout,
        m_ControlSurfaceLayout.GetW(),
        5 * m_ControlSurfaceLayout.GetStdHeight() + browser_h );
    m_ControlSurfaceLayout.AddY( m_CSGroupingLayout.GetH() );

    m_CSGroupingLayout.AddDividerBox( "Control Surface Grouping" );
    m_CSGroupingLayout.SetButtonWidth( 100 );
    m_CSGroupingLayout.AddLabel( "User Groups", main_browser_w );
    m_CSGroupingLayout.AddX( main_browser_spacing );
    m_CSGroupingLayout.AddLabel( "Available Control Surfaces", component_browser_w );
    m_CSGroupingLayout.AddX( items_browsers_spacing );
    m_CSGroupingLayout.AddLabel( " Grouped Control Surfaces", component_browser_w );
    m_CSGroupingLayout.ForceNewLine();
    m_CSGroupingLayout.SetSameLineFlag( true );
    m_CSGroupingLayout.SetFitWidthFlag( false );

    m_CSGroupBrowser = m_CSGroupingLayout.AddFlBrowser( 0 );
    m_CSGroupBrowser->resize( m_CSGroupingLayout.GetX(), m_CSGroupingLayout.GetY(), main_browser_w, browser_h + browser_h_augment );
    m_CSGroupBrowser->type( FL_SELECT_BROWSER );
    m_CSGroupBrowser->labelfont( 13 );
    m_CSGroupBrowser->textsize( 12 );
    m_CSGroupBrowser->callback( staticScreenCB, this );

    m_CSGroupingLayout.AddX( main_browser_w );
    m_CSGroupingLayout.AddX( main_browser_spacing ); // Space Between Main Browser and Available Items Browser

    m_UngroupedCSBrowser = m_CSGroupingLayout.AddFlBrowser( 0 );
    m_UngroupedCSBrowser->resize( m_CSGroupingLayout.GetX(), m_CSGroupingLayout.GetY(), component_browser_w, browser_h );
    m_UngroupedCSBrowser->type( FL_MULTI_BROWSER );
    m_UngroupedCSBrowser->labelfont( 13 );
    m_UngroupedCSBrowser->textsize( 12 );
    m_UngroupedCSBrowser->callback( staticScreenCB, this );

    m_CSGroupingLayout.AddX( component_browser_w );
    m_CSGroupingLayout.AddX( items_browsers_spacing );

    m_GroupedCSBrowser = m_CSGroupingLayout.AddFlBrowser( 0 );
    m_GroupedCSBrowser->resize( m_CSGroupingLayout.GetX(), m_CSGroupingLayout.GetY(), component_browser_w, browser_h );
    m_GroupedCSBrowser->type( FL_MULTI_BROWSER );
    m_GroupedCSBrowser->labelfont( 13 );
    m_GroupedCSBrowser->textsize( 12 );
    m_GroupedCSBrowser->callback( staticScreenCB, this );

    m_CSGroupingLayout.ForceNewLine();
    m_CSGroupingLayout.SetY( m_CSGroupingLayout.GetY() + browser_h - browser_h_augment );

    m_CSGroupingLayout.AddX( main_browser_w );
    m_CSGroupingLayout.AddX( main_browser_spacing ); // Space Between Main Browser and Available Items Browser

    m_CSGroupingLayout.SetButtonWidth( component_browser_w );
    m_CSGroupingLayout.AddButton( m_AddSelectedCSButton, "Add Selected" );
    m_CSGroupingLayout.AddX( items_browsers_spacing );
    m_CSGroupingLayout.AddButton( m_RemoveSelectedCSButton, "Remove Selected" );
    m_CSGroupingLayout.ForceNewLine();

    m_CSGroupingLayout.SetButtonWidth( main_browser_w );
    m_CSGroupingLayout.SetButtonWidth( m_CSGroupingLayout.GetButtonWidth() / 2 );
    m_CSGroupingLayout.AddButton( m_AddCSGroupButton, "Add" );
    m_CSGroupingLayout.AddButton( m_RemoveCSGroupButton, "Remove" );

    m_CSGroupingLayout.AddX( main_browser_spacing );
    m_CSGroupingLayout.SetButtonWidth( component_browser_w );
    m_CSGroupingLayout.AddButton( m_AddAllCSButton, "Add All" );
    m_CSGroupingLayout.AddX( items_browsers_spacing );
    m_CSGroupingLayout.AddButton( m_RemoveAllCSButton, "Remove All" );
    m_CSGroupingLayout.ForceNewLine();
    m_CSGroupingLayout.AddYGap();

    m_CSGroupingLayout.SetSameLineFlag( false );
    m_CSGroupingLayout.SetFitWidthFlag( true );

    m_CSGroupingLayout.AddButton( m_AutoGroupTrigger, "Auto Group Remaining Control Surfaces" );

    m_ControlSurfaceLayout.AddYGap();

    // Control Surface Details Layout
    m_ControlSurfaceLayout.AddSubGroupLayout( m_CSGroupDetailsLayout,
        m_ControlSurfaceLayout.GetW(),
        2* m_ControlSurfaceLayout.GetStdHeight() );
    m_ControlSurfaceLayout.AddY( m_CSGroupDetailsLayout.GetH() );

    m_CSGroupDetailsLayout.AddDividerBox( "Current Control Surface Group Details" );

    m_CSGroupDetailsLayout.SetButtonWidth( 110 );
    m_CSGroupDetailsLayout.AddInput( m_GroupEditNameInput, "Group Name" );

    // Deflection Gain Scroll Layout
    m_ControlSurfaceLayout.AddDividerBox( "Deflection Gain per Surface" );

    m_ControlSurfaceLayout.AddSubGroupLayout( m_CSGroupGainScrollLayout,
        m_ControlSurfaceLayout.GetW(),
        m_ControlSurfaceLayout.GetRemainY() );

    m_DeflectionGainScroll = AddSubScroll( m_CSGroupGainScrollLayout.GetGroup(), group_border_width );
    m_DeflectionGainScroll->type( Fl_Scroll::VERTICAL_ALWAYS );
    m_DeflectionGainScroll->resize( m_ControlSurfaceLayout.GetX(), m_ControlSurfaceLayout.GetY(),
        m_CSGroupGainScrollLayout.GetW(), m_CSGroupGainScrollLayout.GetH() );

    //==== Viewer Tab ====//
    Fl_Group* viewer_tab = AddTab( "Viewer Console" );
    Fl_Group* viewer_group = AddSubGroup( viewer_tab, window_border_width );
    m_ViewerLayout.SetGroupAndScreen( viewer_group, this );

    m_ViewerLayout.AddDividerBox( "VSPAERO Viewer Console" );

    m_ViewerDisplay = m_ViewerLayout.AddFlTextDisplay( m_ViewerLayout.GetRemainY() - m_ViewerLayout.GetStdHeight() );
    m_ViewerBuffer = new Fl_Text_Buffer;
    m_ViewerDisplay->buffer( m_ViewerBuffer );


    // Show the starting tab
    overview_tab->show();

    // Flags to control Kill thread functionality
    m_SolverThreadIsRunning = false;
}

VSPAEROScreen::~VSPAEROScreen()
{
}

bool VSPAEROScreen::Update()
{
    Vehicle *veh = VehicleMgr.GetVehicle();

    VSPAEROMgr.Update();

    if (veh)
    {
        UpdateRefWing();

        UpdateSetChoiceLists();

        UpdateCaseSetupDevices();

        UpdateReferenceQuantitiesDevices();

        UpdateCGDevices();

        UpdateFlowConditionDevices();
        UpdateVSPAEROButtons();

        UpdateAdvancedTabDevices();

        UpdatePropElemDevices();

        UpdateOtherSetupParms();

        UpdateDeflectionAngleScrollGroup();
        UpdateDeflectionGainScrollGroup();

        UpdatePropElemBrowser();
        UpdateControlSurfaceBrowsers();
    }

    m_FLTK_Window->redraw();

    return false;
}


void VSPAEROScreen::Show()
{
    m_ScreenMgr->SetUpdateFlag( true );
    m_FLTK_Window->show();
}

void VSPAEROScreen::Hide()
{
    m_FLTK_Window->hide();
    m_ScreenMgr->SetUpdateFlag( true );
}


void VSPAEROScreen::CallBack( Fl_Widget* w )
{
    if ( w == m_PropElemBrowser )
    {
        PropElemBrowserCallback();
    }
    else if ( w == m_CSGroupBrowser )
    {
        ControlSurfaceGroupBrowserCallback();
    }
    else if ( w == m_UngroupedCSBrowser )
    {
        UngroupedCSBrowserCallback();
    }
    else if ( w == m_GroupedCSBrowser )
    {
        GroupedCSBrowserCallback();
    }

    m_ScreenMgr->SetUpdateFlag( true );
}

void VSPAEROScreen::CloseCallBack( Fl_Widget* w )
{
    Hide();
}

#ifdef WIN32
DWORD WINAPI monitorfun( LPVOID data )
#else
void * monitorfun( void *data )
#endif
{
    monitorpair *mp = ( monitorpair * ) data;

    VSPAEROScreen *vs = mp->first;
    int id = mp->second;

    if( vs )
    {
        Fl_Text_Display *display = vs->GetDisplay( id );
        ProcessUtil *pu = vs->GetProcess( id );
        if( pu && display )
        {
            int bufsize = 1000;
            char *buf;
            buf = ( char* ) malloc( sizeof( char ) * ( bufsize + 1 ) );

            unsigned long nread = 1;

            bool runflag = pu->IsRunning();
            while( runflag || nread > 0 )
            {
                nread = 0;
                pu->ReadStdoutPipe( buf, bufsize, &nread );

                if( nread > 0 )
                {
                    buf[nread] = 0;
                    StringUtil::change_from_to( buf, '\r', '\n' );

                    Fl::lock();
                    // Any FL calls must occur between Fl::lock() and Fl::unlock().
                    vs->AddOutputText( display, buf );
                    Fl::unlock();
                }

                if( runflag )
                {
                    SleepForMilliseconds( 100 );
                }
                runflag = pu->IsRunning();
            }

#ifdef WIN32
            CloseHandle( pu->m_StdoutPipe[0] );
            pu->m_StdoutPipe[0] = NULL;
#else
            close( pu->m_StdoutPipe[0] );
            pu->m_StdoutPipe[0] = -1;
#endif
            vs->GetScreenMgr()->SetUpdateFlag( true );

            free( buf );
        }
    }
    return 0;
}

#ifdef WIN32
DWORD WINAPI solver_thread_fun( LPVOID data )
#else
void * solver_thread_fun( void *data )
#endif
{
    solverpair *sp = ( solverpair * ) data;

    VSPAEROMgrSingleton* vsmgr = sp->first;
    VSPAEROScreen *vsscreen = sp->second;

    if( vsmgr && vsscreen )
    {
        vsscreen->m_SolverThreadIsRunning = true;

        // EXECUTE SOLVER
        vsmgr->ComputeSolver();

        vsscreen->m_SolverThreadIsRunning = false;

        vsscreen->GetScreenMgr()->SetUpdateFlag( true );
    }

    return 0;
}


void VSPAEROScreen::GuiDeviceCallBack( GuiDevice* device )
{
    assert( m_ScreenMgr );

    Vehicle *veh = VehicleMgr.GetVehicle();

    if( veh )
    {
        //TODO add callback to determine if the setup file text has been edited

        if ( device == &m_SolverButton )
        {
            if( veh->GetVSPAEROCmd().empty() || VSPAEROMgr.IsSolverRunning() )
            { /* Do nothing. Should not be reachable, button should be deactivated.*/ }
            else
            {
                // Clear out previous results
                VSPAEROMgr.ClearAllPreviousResults();

                VSPAEROMgr.ComputeGeometry();

                VSPAEROMgr.CreateSetupFile();

                // Clear the solver console
                m_SolverBuffer->text( "" );

                //Show the plot screen (if not unsteady analysis)
                if (VSPAEROMgr.m_StabilityType() != 1 && VSPAEROMgr.m_AnalysisMethod() == vsp::PANEL)
                {
                    m_ScreenMgr->m_ShowPlotScreenOnce = false;
                }
                else
                {
                    m_ScreenMgr->m_ShowPlotScreenOnce = true;   //deferred show of plot screen

                    VSPAEROPlotScreen * vspapscreen = ( VSPAEROPlotScreen * )m_ScreenMgr->GetScreen( ScreenMgr::VSP_VSPAERO_PLOT_SCREEN );
                    if( vspapscreen )
                    {
                        vspapscreen->SetDefaultView();
                        vspapscreen->Update();
                    }
                }

                m_SolverProcess.StartThread( solver_thread_fun, ( void* ) &m_SolverPair );

            }
        }
        else if ( device == &m_ViewerButton )
        {
            if( veh->GetVIEWERCmd().empty() || !FileExist( VSPAEROMgr.m_DegenFileFull ) || m_ViewerProcess.IsRunning() )
            { /* Do nothing. Should not be reachable, button should be deactivated.*/ }
            else
            {
                vector<string> args;
                args.push_back( VSPAEROMgr.m_ModelNameBase );

                m_ViewerProcess.ForkCmd( veh->GetExePath(), veh->GetVIEWERCmd(), args );

                m_ViewerBuffer->text( "" );
                m_ViewerMonitor.StartThread( monitorfun, ( void* ) &m_ViewerPair );

            }
        }
        else if( device == &m_KillSolverButton )
        {
            VSPAEROMgr.KillSolver();
        }
        else if( device == &m_PlotButton )
        {
            m_ScreenMgr->m_ShowPlotScreenOnce = true;   //deferred show of plot screen
        }
        else if ( device == &m_PreviewDegenButton )
        {
            DisplayDegenCamberPreview();
        }
        else if( device == &m_RefWingChoice )
        {
            int id = m_RefWingChoice.GetVal();
            VSPAEROMgr.m_RefGeomID = m_WingGeomVec[id];
        }
        else if( device == &m_GeomSetChoice )
        {
            VSPAEROMgr.m_GeomSet = m_GeomSetChoice.GetVal();
        }
        else if( device == &m_DegenFileButton )
        {
            veh->setExportFileName( vsp::DEGEN_GEOM_CSV_TYPE, m_ScreenMgr->GetSelectFileScreen()->FileChooser( "Select degen geom CSV output file.", "*.csv" ) );
        }
        else if( device == &m_CompGeomFileButton )
        {
            veh->setExportFileName( vsp::VSPAERO_PANEL_TRI_TYPE, m_ScreenMgr->GetSelectFileScreen()->FileChooser( "Select comp geom TRI output file.", "*.tri" ) );
        }
        else if( device == &m_CGSetChoice )
        {
            VSPAEROMgr.m_CGGeomSet = m_CGSetChoice.GetVal();
        }
        else if( device == &m_MassPropButton )
        {
            string id = veh->MassPropsAndFlatten( m_CGSetChoice.GetVal(), VSPAEROMgr.m_NumMassSlice(), false, false );
            veh->DeleteGeom( id );

            VSPAEROMgr.m_Xcg = veh->m_CG.x();
            VSPAEROMgr.m_Ycg = veh->m_CG.y();
            VSPAEROMgr.m_Zcg = veh->m_CG.z();
        }
        else if( device == &m_ExportResultsToCsvButton )
        {
            string fileName = m_ScreenMgr->GetSelectFileScreen()->FileChooser( "Select CSV File", "*.csv" );
            if ( fileName.size() > 0 )
            {
                int status = VSPAEROMgr.ExportResultsToCSV( fileName );
                if ( status != vsp::VSP_OK )
                {
                    fl_alert( "File export failed\nFile: %s", fileName.c_str() );
                }
            }
        }
        else if ( device == &m_StabilityTypeChoice )
        {
            VSPAEROMgr.m_StabilityType = m_StabilityTypeChoice.GetVal();
        }
        else if ( device == &m_GroupEditNameInput )
        {
            VSPAEROMgr.SetCurrentCSGroupName( m_GroupEditNameInput.GetString() );
        }
        else if ( device == &m_AddCSGroupButton )
        {
            VSPAEROMgr.AddControlSurfaceGroup();
        }
        else if ( device == &m_RemoveCSGroupButton )
        {
            VSPAEROMgr.RemoveControlSurfaceGroup();
        }
        else if ( device == &m_AddSelectedCSButton )
        {
            VSPAEROMgr.AddSelectedToCSGroup();
        }
        else if ( device == &m_AddAllCSButton )
        {
            VSPAEROMgr.AddAllToCSGroup();
        }
        else if ( device == &m_RemoveSelectedCSButton )
        {
            VSPAEROMgr.RemoveSelectedFromCSGroup();
        }
        else if ( device == &m_RemoveAllCSButton )
        {
            VSPAEROMgr.RemoveAllFromCSGroup();
        }
        else if ( device == &m_AutoGroupTrigger )
        {
            VSPAEROMgr.InitControlSurfaceGroups();
        }
    }
    m_ScreenMgr->SetUpdateFlag( true );
}

// VSPAEROScreen::AddOutputText( Fl_Text_Display *display, const char *text )
//     This is used for the Solver tab to show the current results of the solver in the GUI
void VSPAEROScreen::AddOutputText( Fl_Text_Display *display, const string &text )
{
    if ( display )
    {
        // Added lock(), unlock() calls to avoid heap corruption while updating the text and rapidly scrolling with the mouse wheel inside the text display
        Fl::lock();

        display->buffer()->append( text.c_str() );
        display->insert_position( display->buffer()->length() );

        display->show_insert_position();

        Fl::unlock();
    }
}

ProcessUtil* VSPAEROScreen::GetProcess( int id )
{
    if( id == VSPAERO_SOLVER )
    {
        return &m_SolverProcess;
    }
    else if( id == VSPAERO_VIEWER )
    {
        return &m_ViewerProcess;
    }
    else if( id == VSPAERO_SOLVER_MONITOR )
    {
        return &m_SolverMonitor;
    }
    else
    {
        return NULL;
    }
}

Fl_Text_Display* VSPAEROScreen::GetDisplay( int id )
{
    if( id == VSPAERO_SOLVER )
    {
        return m_SolverDisplay;
    }
    else if( id == VSPAERO_VIEWER )
    {
        return m_ViewerDisplay;
    }
    else
    {
        return NULL;
    }
}

void VSPAEROScreen::UpdateRefWing()
{
    Vehicle* veh = VehicleMgr.GetVehicle();
    // Reference Wing Choice
    //    find & list all Wing type geometries
    vector <string> geomVec = veh->GetGeomVec();

    m_RefWingChoice.ClearItems();
    m_WingGeomVec.clear();
    map <string, int> WingCompIDMap;
    int iwing = 0;

    if (!VSPAEROMgr.m_RefFlag())
    {
        m_RefWingChoice.Deactivate();
    }
    else
    {
        m_RefWingChoice.Activate();
        for (int i = 0; i < (int)geomVec.size(); i++)
        {
            char str[256];
            Geom* g = veh->FindGeom(geomVec[i]);
            if (g)
            {
                sprintf(str, "%d_%s", i, g->GetName().c_str());

                if (g->GetType().m_Type == MS_WING_GEOM_TYPE)
                {
                    m_RefWingChoice.AddItem(str);
                    WingCompIDMap[geomVec[i]] = iwing;
                    m_WingGeomVec.push_back(geomVec[i]);
                    iwing++;
                }
            }
        }
        m_RefWingChoice.UpdateItems();
    }

    //    Update selected value
    string refGeomID = VSPAEROMgr.m_RefGeomID;
    if (refGeomID.length() == 0 && m_WingGeomVec.size() > 0)
    {
        // Handle case default case.
        refGeomID = m_WingGeomVec[0];
        VSPAEROMgr.m_RefGeomID = refGeomID;
        // Re-trigger reference quantity update with default component.
        VSPAEROMgr.Update();
    }
    m_RefWingChoice.SetVal(WingCompIDMap[refGeomID]);
}

void VSPAEROScreen::UpdateSetChoiceLists()
{
    Vehicle* veh = VehicleMgr.GetVehicle();
    m_GeomSetChoice.ClearItems();
    m_CGSetChoice.ClearItems();

    vector <string> setVec = veh->GetSetNameVec();
    for (int iSet = 0; iSet < setVec.size(); iSet++)
    {
        m_GeomSetChoice.AddItem(setVec[iSet]);
        m_CGSetChoice.AddItem(setVec[iSet]);
    }
    m_GeomSetChoice.UpdateItems();
    m_CGSetChoice.UpdateItems();

    m_GeomSetChoice.SetVal(VSPAEROMgr.m_GeomSet());
    m_CGSetChoice.SetVal(VSPAEROMgr.m_CGGeomSet());
}

void VSPAEROScreen::UpdateCaseSetupDevices()
{
    m_AeroMethodToggleGroup.Update(VSPAEROMgr.m_AnalysisMethod.GetID());
    switch (VSPAEROMgr.m_AnalysisMethod.Get())
    {
    case vsp::VORTEX_LATTICE:

        m_DegenFileName.Activate();
        m_DegenFileButton.Activate();
        m_PreviewDegenButton.Activate();

        m_CompGeomFileName.Deactivate();
        m_CompGeomFileButton.Deactivate();

        break;

    case vsp::PANEL:

        m_DegenFileName.Deactivate();
        m_DegenFileButton.Deactivate();
        m_PreviewDegenButton.Deactivate();

        m_CompGeomFileName.Activate();
        m_CompGeomFileButton.Activate();

        break;

    default:
        //do nothing; this should not be reachable
        break;
    }
}

void VSPAEROScreen::UpdateReferenceQuantitiesDevices()
{
    m_RefToggle.Update(VSPAEROMgr.m_RefFlag.GetID());
    m_SrefSlider.Update(VSPAEROMgr.m_Sref.GetID());
    m_brefSlider.Update(VSPAEROMgr.m_bref.GetID());
    m_crefSlider.Update(VSPAEROMgr.m_cref.GetID());
}

void VSPAEROScreen::UpdateCGDevices()
{
    m_NumSliceSlider.Update(VSPAEROMgr.m_NumMassSlice.GetID());
    m_XcgSlider.Update(VSPAEROMgr.m_Xcg.GetID());
    m_YcgSlider.Update(VSPAEROMgr.m_Ycg.GetID());
    m_ZcgSlider.Update(VSPAEROMgr.m_Zcg.GetID());
}

void VSPAEROScreen::UpdateAdvancedTabDevices()
{
    Vehicle* veh = VehicleMgr.GetVehicle();
    m_DegenFileName.Update(veh->getExportFileName(vsp::DEGEN_GEOM_CSV_TYPE));
    m_CompGeomFileName.Update(veh->getExportFileName(vsp::VSPAERO_PANEL_TRI_TYPE));

    m_NCPUSlider.Update(VSPAEROMgr.m_NCPU.GetID());
    m_BatchCalculationToggle.Update(VSPAEROMgr.m_BatchModeFlag.GetID());
    m_JacobiPreconditionToggle.Update(VSPAEROMgr.m_JacobiPrecondition.GetID());
    m_SymmetryToggle.Update( VSPAEROMgr.m_Symmetry.GetID() );
    m_Write2DFEMToggle.Update( VSPAEROMgr.m_Write2DFEMFlag.GetID() );

    // Wake Options
    m_WakeNumIterSlider.Update(VSPAEROMgr.m_WakeNumIter.GetID());
    m_WakeAvgStartIterSlider.Update(VSPAEROMgr.m_WakeAvgStartIter.GetID());
    m_WakeSkipUntilIterSlider.Update(VSPAEROMgr.m_WakeSkipUntilIter.GetID());

    // Other Set Up Parms
    m_ClmaxToggle.Update( VSPAEROMgr.m_ClMaxToggle.GetID() );
    m_ClmaxSlider.Update( VSPAEROMgr.m_ClMax.GetID() );
    m_MaxTurningToggle.Update( VSPAEROMgr.m_MaxTurnToggle.GetID() );
    m_MaxTurningSlider.Update( VSPAEROMgr.m_MaxTurnAngle.GetID() );
    m_FarDistToggle.Update( VSPAEROMgr.m_FarDistToggle.GetID() );
    m_FarDistSlider.Update( VSPAEROMgr.m_FarDist.GetID() );

    // Stability
    if (VSPAEROMgr.m_Symmetry())
    {
        m_StabilityCalcToggle.Deactivate();
        VSPAEROMgr.m_StabilityCalcFlag.Set(false);
    }
    else
    {
        m_StabilityCalcToggle.Update(VSPAEROMgr.m_StabilityCalcFlag.GetID());
    }

    if (!VSPAEROMgr.m_StabilityCalcFlag())
    {
        m_StabilityTypeChoice.Deactivate();
    }
    else
    {
        m_StabilityTypeChoice.Activate();
    }

    VSPAEROMgr.UpdateSetupParmLimits();
}

void VSPAEROScreen::UpdateFlowConditionDevices()
{
    // Alpha
    m_AlphaStartInput.Update(VSPAEROMgr.m_AlphaStart.GetID());
    m_AlphaEndInput.Update(VSPAEROMgr.m_AlphaEnd.GetID());
    m_AlphaNptsInput.Update(VSPAEROMgr.m_AlphaNpts.GetID());
    if (VSPAEROMgr.m_AlphaNpts.Get() == 1)
    {
        m_AlphaEndInput.Deactivate();
    }
    else if (VSPAEROMgr.m_AlphaNpts.Get() > 1)
    {
        m_AlphaEndInput.Activate();
    }
    // Beta
    m_BetaStartInput.Update(VSPAEROMgr.m_BetaStart.GetID());
    m_BetaEndInput.Update(VSPAEROMgr.m_BetaEnd.GetID());
    m_BetaNptsInput.Update(VSPAEROMgr.m_BetaNpts.GetID());
    if (VSPAEROMgr.m_BetaNpts.Get() == 1)
    {
        m_BetaEndInput.Deactivate();
    }
    else if (VSPAEROMgr.m_BetaNpts.Get() > 1)
    {
        m_BetaEndInput.Activate();
    }
    // Mach
    m_MachStartInput.Update(VSPAEROMgr.m_MachStart.GetID());
    m_MachEndInput.Update(VSPAEROMgr.m_MachEnd.GetID());
    m_MachNptsInput.Update(VSPAEROMgr.m_MachNpts.GetID());
    if (VSPAEROMgr.m_MachNpts.Get() == 1)
    {
        m_MachEndInput.Deactivate();
    }
    else if (VSPAEROMgr.m_MachNpts.Get() > 1)
    {
        m_MachEndInput.Activate();
    }
}

void VSPAEROScreen::UpdateVSPAEROButtons()
{
    Vehicle* veh = VehicleMgr.GetVehicle();
    // Solver Button
    if (veh->GetVSPAEROCmd().empty() || m_SolverThreadIsRunning)
    {
        m_SolverButton.Deactivate();
    }
    else
    {
        m_SolverButton.Activate();
    }
    // Kill Solver Button
    if (m_SolverThreadIsRunning)
    {
        m_KillSolverButton.Activate();
    }
    else
    {
        m_KillSolverButton.Deactivate();
    }

    // Plot Window Button
    if (veh->GetVSPAEROCmd().empty() || !FileExist(VSPAEROMgr.m_SetupFile))
    {
        m_PlotButton.Deactivate();
    }
    else
    {
        m_PlotButton.Activate();
    }

    // Viewer Button
    if (veh->GetVIEWERCmd().empty() || m_SolverThreadIsRunning || m_ViewerProcess.IsRunning() || !FileExist(VSPAEROMgr.m_AdbFile))
    {
        m_ViewerButton.Deactivate();
    }
    else
    {
        m_ViewerButton.Activate();
    }

    // Export Button
    if (ResultsMgr.GetNumResults("VSPAERO_Wrapper") == 0)
    {
        m_ExportResultsToCsvButton.Deactivate();
    }
    else
    {
        m_ExportResultsToCsvButton.Activate();
    }
}

void VSPAEROScreen::UpdatePropElemDevices()
{
    vector < RotorDisk* > rotordiskvec = VSPAEROMgr.GetRotorDiskVec();
    int index = VSPAEROMgr.GetCurrentRotorDiskIndex();
    if (index >= 0)
    {
        rotordiskvec[ index ]->m_Diameter.Activate();
        rotordiskvec[ index ]->m_HubDiameter.Activate();
        rotordiskvec[ index ]->m_RPM.Activate();
        rotordiskvec[ index ]->m_CP.Activate();
        rotordiskvec[ index ]->m_CT.Activate();

        m_PropElemDia.Update(to_string(rotordiskvec[ index ]->m_Diameter()));
        rotordiskvec[ index ]->m_HubDiameter.SetUpperLimit( rotordiskvec[ index ]->m_Diameter() );
        m_PropElemHubDia.SetMaxBound( rotordiskvec[ index ]->m_Diameter() );
        m_PropElemHubDia.Update( rotordiskvec[ index ]->m_HubDiameter.GetID() );
        m_PropElemRPM.Update( rotordiskvec[ index ]->m_RPM.GetID());
        m_PropElemCP.Update( rotordiskvec[ index ]->m_CP.GetID());
        m_PropElemCT.Update( rotordiskvec[ index ]->m_CT.GetID());
    }
}

void VSPAEROScreen::UpdatePropElemBrowser()
{
    char str[256];
    m_PropElemBrowser->clear();
    static int widths[] = { 35, 150, 40, 65, 65, 40, 40 }; // widths for each column
    m_PropElemBrowser->column_widths(widths);    // assign array to widget
    m_PropElemBrowser->column_char(':');         // use : as the column character

    sprintf(str, "@b@.INDX:@b@.NAME:@b@.DIA:@b@.HUB DIA:@b@.RPM:@b@.CP:@b@.CT");
    m_PropElemBrowser->add(str);
    for (size_t i = 0; i < VSPAEROMgr.GetRotorDiskVec().size(); ++i)
    {
        RotorDisk* curr_rot = VSPAEROMgr.GetRotorDiskVec()[i];
        if (curr_rot)
        {
            sprintf(str, "%i:%s:%4.2f:%4.2f:%6.1f:%4.2f:%4.2f", curr_rot->m_GroupSuffix,
                curr_rot->GetName().c_str(),
                curr_rot->m_Diameter(), curr_rot->m_HubDiameter(),
                curr_rot->m_RPM(), curr_rot->m_CP(), curr_rot->m_CT());
            m_PropElemBrowser->add(str);
        }
    }
    SelectPropBrowser(VSPAEROMgr.GetCurrentRotorDiskIndex() + 2);
}

void VSPAEROScreen::UpdateControlSurfaceBrowsers()
{
    char str[256];
    int curr_cs_index = VSPAEROMgr.GetCurrentCSGroupIndex();
    vector < ControlSurfaceGroup* > cs_group_vec = VSPAEROMgr.GetControlSurfaceGroupVec();
    m_CSGroupBrowser->clear();
    for (size_t i = 0; i < cs_group_vec.size(); ++i)
    {
        ControlSurfaceGroup* curr_cs_group = cs_group_vec[i];
        if (curr_cs_group)
        {
            sprintf( str, "%i %s", curr_cs_group->m_GroupSuffix, curr_cs_group->GetName().c_str());
            m_CSGroupBrowser->add( str );
        }
        vector < VspAeroControlSurf > sub_surf_vec = VSPAEROMgr.GetActiveCSVec();
        for (size_t j = 0; j < sub_surf_vec.size(); ++j)
        {
            VSPAEROMgr.RemoveFromUngrouped(sub_surf_vec[j].SSID, sub_surf_vec[j].iReflect);
        }
    }
    SelectControlSurfaceBrowser(curr_cs_index + 1);

    m_GroupedCSBrowser->clear();
    if (VSPAEROMgr.GetCurrentCSGroupIndex() != -1)
    {
        vector < VspAeroControlSurf > grouped_cs = VSPAEROMgr.GetActiveCSVec();
        m_GroupEditNameInput.Update(VSPAEROMgr.GetCurrentCSGGroupName().c_str());
        for (size_t i = 0; i < grouped_cs.size(); ++i)
        {
            m_GroupedCSBrowser->add(grouped_cs[i].fullName.c_str());
        }
        for (size_t i = 0; i < VSPAEROMgr.GetSelectedGroupedItems().size(); ++i)
        {
            SelectGroupedListBrowser(VSPAEROMgr.GetSelectedGroupedItems()[i]);
        }
    }
    else
    {
        m_DeflectionGainScroll->clear();
        m_GroupEditNameInput.Update("");
    }

    m_UngroupedCSBrowser->clear();
    // For loop on a vector of ungrouped control surfaces
    vector < VspAeroControlSurf > ungrouped_cs = VSPAEROMgr.GetUngroupedCSVec();
    for (size_t i = 0; i < ungrouped_cs.size(); ++i)
    {
        m_UngroupedCSBrowser->add(ungrouped_cs[i].fullName.c_str());
    }
    for (size_t i = 0; i < VSPAEROMgr.GetSelectedUngroupedItems().size(); ++i)
    {
        SelectUngroupedListBrowser(VSPAEROMgr.GetSelectedUngroupedItems()[i]);
    }
}

void VSPAEROScreen::UpdateOtherSetupParms()
{
    m_VinfSlider.Update( VSPAEROMgr.m_Vinf.GetID() );
    m_RhoSlider.Update( VSPAEROMgr.m_Rho.GetID() );
}

void VSPAEROScreen::UpdateDeflectionAngleScrollGroup()
{
    vector < ControlSurfaceGroup* > cs_group_vec = VSPAEROMgr.GetControlSurfaceGroupVec();

    if (cs_group_vec.size() != m_NumVarAngle)
    {
        m_DeflectionAngleScroll->clear();
        m_DeflectionAngleLayout.SetGroup( m_DeflectionAngleScroll );
        m_DeflectionAngleLayout.InitWidthHeightVals();

        m_DeflectionAngleToggleVec.clear();
        m_DeflectionAngleSliderVec.clear();

        m_DeflectionAngleToggleVec.resize(cs_group_vec.size());
        m_DeflectionAngleSliderVec.resize(cs_group_vec.size());

        m_DeflectionAngleLayout.SetFitWidthFlag(false);
        m_DeflectionAngleLayout.SetSameLineFlag(true);
        m_DeflectionAngleLayout.SetButtonWidth(10);

        m_DeflectionAngleLayout.SetSliderWidth(50);

        for (size_t i = 0; i < cs_group_vec.size(); ++i)
        {
            m_DeflectionAngleLayout.SetButtonWidth(15);
            m_DeflectionAngleLayout.AddButton(m_DeflectionAngleToggleVec[i], "");
            m_DeflectionAngleToggleVec[i].Update(cs_group_vec[i]->m_IsUsed.GetID());
            m_DeflectionAngleLayout.SetButtonWidth(145);
            m_DeflectionAngleLayout.AddSlider(m_DeflectionAngleSliderVec[i], cs_group_vec[i]->GetName().c_str(), 10, "%3.2f");
            m_DeflectionAngleSliderVec[i].Update(cs_group_vec[i]->m_DeflectionAngle.GetID());
            if (!cs_group_vec[i]->m_IsUsed())
            {
                m_DeflectionAngleSliderVec[i].Deactivate();
            }
            m_DeflectionAngleLayout.ForceNewLine();
        }

        m_NumVarAngle = cs_group_vec.size();
    }
    else
    {
        for (size_t i = 0; i < cs_group_vec.size(); ++i)
        {
            m_DeflectionAngleToggleVec[i].Update(cs_group_vec[i]->m_IsUsed.GetID());
            m_DeflectionAngleSliderVec[i].Update(cs_group_vec[i]->m_DeflectionAngle.GetID());

            if (!cs_group_vec[i]->m_IsUsed())
            {
                m_DeflectionAngleSliderVec[i].Deactivate();
            }
        }
    }
    UpdateControlSurfaceGroupNames();
}

void VSPAEROScreen::UpdateControlSurfaceGroupNames()
{
    // Prevent Long Group Names from spilling over passed button limits
    char str[256];
    vector < ControlSurfaceGroup* > cs_group_vec = VSPAEROMgr.GetControlSurfaceGroupVec();
    for (size_t i = 0; i < cs_group_vec.size(); ++i)
    {
        if (cs_group_vec[i]->GetName().size() >= 20)
        {
            sprintf(str, "%s...", cs_group_vec[i]->GetName().substr(0, 20).c_str());
            m_DeflectionAngleSliderVec[i].SetButtonName(str);
        }
        else
        {
            m_DeflectionAngleSliderVec[i].SetButtonName(cs_group_vec[i]->GetName().c_str());
        }
    }
}

void VSPAEROScreen::UpdateDeflectionGainScrollGroup()
{
    int button_width = 300;
    int input_width = 60;
    if ( VSPAEROMgr.GetCurrentCSGroupIndex() != -1 )
    {
        ControlSurfaceGroup* cs = VSPAEROMgr.GetControlSurfaceGroupVec()[ VSPAEROMgr.GetCurrentCSGroupIndex() ];
        vector < VspAeroControlSurf > cs_vec = VSPAEROMgr.GetActiveCSVec();

        if ( cs_vec.size() != m_NumVarDeflection )
        {
            m_DeflectionGainScroll->clear();
            m_CSGroupGainScrollLayout.SetGroup( m_DeflectionGainScroll );
            m_CSGroupGainScrollLayout.InitWidthHeightVals();

            m_DeflectionGainSliderVec.clear();

            m_DeflectionGainSliderVec.resize( cs_vec.size() );

            m_CSGroupGainScrollLayout.SetFitWidthFlag( true );
            m_CSGroupGainScrollLayout.SetSameLineFlag( false );
            m_CSGroupGainScrollLayout.SetButtonWidth( button_width );
            m_CSGroupGainScrollLayout.SetInputWidth( input_width );

            for ( size_t i = 0; i < cs_vec.size(); ++i )
            {
                m_CSGroupGainScrollLayout.AddSlider( m_DeflectionGainSliderVec[i], cs_vec[i].fullName.c_str(), 10.0, "%3.2f" );
                m_DeflectionGainSliderVec[i].Update( cs->m_DeflectionGainVec[i]->GetID() );
            }
            m_NumVarDeflection = cs_vec.size();
        }
        else
        {
            for ( size_t i = 0; i < cs_vec.size(); ++i )
            {
                m_DeflectionGainSliderVec[i].SetButtonName( cs_vec[i].fullName.c_str() );
                m_DeflectionGainSliderVec[i].Update( cs->m_DeflectionGainVec[i]->GetID() );
            }
        }
    }
    else
    {
        m_DeflectionGainScroll->clear();
        m_CSGroupGainScrollLayout.SetGroup( m_DeflectionGainScroll );
        m_CSGroupGainScrollLayout.InitWidthHeightVals();

        m_DeflectionGainSliderVec.clear();

        m_NumVarDeflection = 0;
    }
}

void VSPAEROScreen::PropElemBrowserCallback()
{
    //==== Find Last Selected Prop ====//
    int last = m_PropElemBrowser->value();
    if ( last >= 2 )
    {
        VSPAEROMgr.SetCurrentRotorDiskIndex( last - 2 );
    }
    VSPAEROMgr.HighlightSelected( VSPAEROMgr.ROTORDISK );
}

void VSPAEROScreen::SelectPropBrowser( int cur_index )
{
    if ( cur_index > 1 )
    {
        //==== Select If Match ====//
        m_PropElemBrowser->select( cur_index );

        //==== Position Browser ====//
        m_PropElemBrowser->topline( cur_index );
    }
}

void VSPAEROScreen::ControlSurfaceGroupBrowserCallback()
{
    int last = m_CSGroupBrowser->value();
    if ( last >= 1 )
    {
        if ( VSPAEROMgr.GetCurrentCSGroupIndex() != last - 1 )
        {
            VSPAEROMgr.SetCurrentCSGroupIndex( last - 1 );
            VSPAEROMgr.m_SelectedGroupedCS.clear();
            VSPAEROMgr.UpdateActiveControlSurfVec();
        }
    }
    VSPAEROMgr.HighlightSelected( VSPAEROMgr.CONTROL_SURFACE );
}

void VSPAEROScreen::SelectControlSurfaceBrowser( int cur_index )
{
    if ( cur_index > 0 )
    {
        //==== Select If Match ====//
        m_CSGroupBrowser->select( cur_index );

        //==== Position Browser ====//
        m_CSGroupBrowser->topline( cur_index );
    }
}

void VSPAEROScreen::UngroupedCSBrowserCallback()
{
    vector < int > selected;
    vector < VspAeroControlSurf > ungrouped_vec = VSPAEROMgr.GetUngroupedCSVec();
    if ( ungrouped_vec.size() != 0 )
    {
        for ( size_t i = 1; i <= m_UngroupedCSBrowser->size(); ++i )
        {
            if ( m_UngroupedCSBrowser->selected( i ) )
            {
                selected.push_back( i );
            }
        }
    }
    VSPAEROMgr.m_SelectedUngroupedCS = selected;
}

void VSPAEROScreen::SelectUngroupedListBrowser( int cur_index )
{
    if ( cur_index > 0 )
    {
        //==== Select If Match ====//
        m_UngroupedCSBrowser->select( cur_index );

        //==== Position Browser ====//
        m_UngroupedCSBrowser->topline( cur_index );
    }
}

void VSPAEROScreen::GroupedCSBrowserCallback()
{
    vector < int > selected;
    vector < VspAeroControlSurf > active_item_vec = VSPAEROMgr.GetActiveCSVec();
    if ( active_item_vec.size() != 0 )
    {
        for ( size_t i = 1; i <= m_GroupedCSBrowser->size(); ++i )
        {
            if ( m_GroupedCSBrowser->selected( i ) )
            {
                selected.push_back( i );
            }
        }
    }
    VSPAEROMgr.m_SelectedGroupedCS = selected;
}

void VSPAEROScreen::SelectGroupedListBrowser( int cur_index )
{
    if ( cur_index > 0 )
    {
        //==== Select If Match ====//
        m_GroupedCSBrowser->select( cur_index );

        //==== Position Browser ====//
        m_GroupedCSBrowser->topline( cur_index );
    }
}

void VSPAEROScreen::LoadDrawObjs( vector< DrawObj* > & draw_obj_vec )
{
    if( m_FLTK_Window->visible() )
    {
        VSPAEROMgr.LoadDrawObjs( draw_obj_vec );
    }
}

void VSPAEROScreen::DisplayDegenCamberPreview()
{
    Vehicle* veh = m_ScreenMgr->GetVehiclePtr();

    if ( !veh )
    {
        return;
    }

    vector< Geom* > geom_vec = veh->FindGeomVec( veh->GetGeomVec( false ) );
    for ( size_t i = 0; i < (int)geom_vec.size(); i++ )
    {
        if ( geom_vec[i]->GetSetFlag( VSPAEROMgr.m_GeomSet() ) )
        {
            vector < VspSurf > surf_vec;
            geom_vec[i]->GetMainSurfVec( surf_vec );

            for ( size_t j = 0; j < surf_vec.size(); j++ )
            {
                int surf_type = surf_vec[j].GetSurfType();

                if ( surf_type == vsp::DISK_SURF )
                {
                    geom_vec[i]->m_GuiDraw.SetDisplayType( GeomGuiDraw::DISPLAY_DEGEN_SURF );
                }
                else
                {
                    geom_vec[i]->m_GuiDraw.SetDisplayType( GeomGuiDraw::DISPLAY_DEGEN_CAMBER );
                }
            }

            geom_vec[i]->m_GuiDraw.SetDrawType( GeomGuiDraw::GEOM_DRAW_SHADE );
            geom_vec[i]->m_GuiDraw.SetDispSubSurfFlag( true );
            geom_vec[i]->SetSetFlag( vsp::SET_SHOWN, true );
            geom_vec[i]->SetSetFlag( vsp::SET_NOT_SHOWN, false );
        }
        else
        {
            // Do not show geoms not in VSPAEROMgr.m_GeomSet()
            geom_vec[i]->SetSetFlag( vsp::SET_SHOWN, false );
            geom_vec[i]->SetSetFlag( vsp::SET_NOT_SHOWN, true );
        }

    }
    m_ScreenMgr->SetUpdateFlag( true );
}
