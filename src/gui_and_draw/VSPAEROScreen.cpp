//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// VSPAEROScreen.cpp: implementation of the VSPAEROScreen class.
//
//////////////////////////////////////////////////////////////////////

#include "VSPAEROScreen.h"
#include "VSPAEROPlotScreen.h"
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
    m_FLTK_Window->resizable( m_SolverDisplay );

    m_ConsoleLayout.SetButtonWidth( m_ConsoleLayout.GetW() / 2 );

    m_ConsoleLayout.SetSameLineFlag( true );
    m_ConsoleLayout.SetFitWidthFlag( false );

    m_ConsoleLayout.AddButton( m_SolverButton, "Launch Solver" );
    m_ConsoleLayout.AddButton( m_KillSolverButton, "Kill Solver" );

    m_ConsoleLayout.ForceNewLine();

    m_ConsoleLayout.AddButton( m_PlotButton, "Show Results Mgr" );
    m_ConsoleLayout.AddButton( m_ViewerButton, "Launch Viewer" );

    m_ConsoleLayout.ForceNewLine();

    m_ConsoleLayout.AddButton( m_LoadExistingResultsButton, "Load Previous Results" );
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
        5 * m_RightColumnLayout.GetStdHeight() );
    m_RightColumnLayout.AddY( m_FlowCondLayout.GetH() );

    m_FlowCondLayout.AddDividerBox( "Flow Condition" );
    m_FlowCondLayout.SetSameLineFlag( false );
    m_FlowCondLayout.SetFitWidthFlag( true );

    m_FlowCondLayout.SetButtonWidth( m_FlowCondLayout.GetButtonWidth() + 10 );

    m_FlowCondLayout.AddInputEvenSpacedVector( m_AlphaStartInput, m_AlphaEndInput, m_AlphaNptsInput, "Alpha", "%7.3f" );
    m_FlowCondLayout.AddInputEvenSpacedVector( m_BetaStartInput, m_BetaEndInput, m_BetaNptsInput, "Beta", "%7.3f" );
    m_FlowCondLayout.AddInputEvenSpacedVector( m_MachStartInput, m_MachEndInput, m_MachNptsInput, "Mach", "%7.3f" );
    m_FlowCondLayout.AddInputEvenSpacedVector( m_ReCrefStartInput, m_ReCrefEndInput, m_ReCrefNptsInput, "ReCref", "%g" );

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
        9 * m_AdvancedLeftLayout.GetStdHeight() + m_AdvancedLeftLayout.GetGapHeight() );
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

    m_AdvancedCaseSetupLayout.AddButton( m_EnableAlternateFormat, "Use Alternate File Format" );
    m_AdvancedCaseSetupLayout.AddYGap();

    m_AdvancedCaseSetupLayout.SetButtonWidth( 80 );
    m_AdvancedCaseSetupLayout.SetInputWidth( 50 );

    m_AdvancedCaseSetupLayout.AddSlider( m_NCPUSlider, "Num CPU", 10.0, "%3.0f" );

    m_AdvancedCaseSetupLayout.SetFitWidthFlag( false );
    m_AdvancedCaseSetupLayout.SetSameLineFlag( true );

    m_AdvancedCaseSetupLayout.SetButtonWidth( m_AdvancedCaseSetupLayout.GetW() / 2 );

    m_AdvancedCaseSetupLayout.AddButton( m_BatchCalculationToggle, "Batch Calculation" );
    m_AdvancedCaseSetupLayout.AddButton( m_SymmetryToggle, "X-Z Symmetry" );

    m_AdvancedCaseSetupLayout.ForceNewLine();
    m_AdvancedCaseSetupLayout.SetFitWidthFlag( true );
    m_AdvancedCaseSetupLayout.SetSameLineFlag( false );

    m_AdvancedCaseSetupLayout.SetButtonWidth( 80 );

    m_PreconditionChoice.AddItem( "Matrix" );
    m_PreconditionChoice.AddItem( "Jacobi" );
    m_PreconditionChoice.AddItem( "SSOR" );
    m_AdvancedCaseSetupLayout.AddChoice( m_PreconditionChoice, "Preconditioner");

    m_AdvancedCaseSetupLayout.AddButton( m_KTCorrectionToggle, "2nd Order Karman-Tsien Mach Correction" );
    m_AdvancedCaseSetupLayout.AddButton(m_Write2DFEMToggle, "Write 2D FEM");

    // Wake Layout
    m_AdvancedLeftLayout.AddSubGroupLayout( m_WakeLayout,
        m_AdvancedLeftLayout.GetW(),
        4 * m_AdvancedLeftLayout.GetStdHeight() );
    m_AdvancedLeftLayout.AddY( m_WakeLayout.GetH() );

    m_WakeLayout.AddDividerBox( "Wake" );
    m_WakeLayout.AddButton( m_FixedWakeToggle, "Fixed Wake" );

    m_WakeLayout.SetButtonWidth( 80 ); // Match with m_NCPUSlider
    m_WakeLayout.SetInputWidth( 50 );

    m_WakeLayout.AddSlider( m_WakeNumIterSlider, "Num It.", 10, "%3.0f" );
    m_WakeLayout.AddSlider( m_NumWakeNodeSlider, "Wake Nodes", 128, "%3.0f" );

    // Other Setup Parms Layout
    m_AdvancedLeftLayout.AddSubGroupLayout( m_OtherParmsLayout,
        m_AdvancedLeftLayout.GetW(),
        9 * m_AdvancedLeftLayout.GetStdHeight() );
    m_AdvancedLeftLayout.AddY( m_OtherParmsLayout.GetH() );

    int togglewidth = 15;
    int labelwidth = 120;
    int inputwidth = 50;
    m_OtherParmsLayout.AddDividerBox( "Other" );
    m_OtherParmsLayout.SetButtonWidth( 80 ); // Match with m_NCPUSlider
    m_OtherParmsLayout.SetInputWidth( 50 );
    m_OtherParmsLayout.AddChoice( m_ClmaxChoice, "Stall Model" );
    m_ClmaxChoice.AddItem( "Off", vsp::CLMAX_OFF );
    m_ClmaxChoice.AddItem( "2D Clmax", vsp::CLMAX_2D );
    m_ClmaxChoice.AddItem( "Carlson Pressure Correlation", vsp::CLMAX_CARLSON );
    m_ClmaxChoice.UpdateItems();
    m_OtherParmsLayout.SetButtonWidth( labelwidth + togglewidth );
    m_OtherParmsLayout.AddSlider( m_ClmaxSlider, "Clmax", 10, "%2.3f" );
    m_OtherParmsLayout.SetSameLineFlag(true);
    m_OtherParmsLayout.SetFitWidthFlag(false);
    m_OtherParmsLayout.SetInputWidth(inputwidth);
    m_OtherParmsLayout.SetSliderWidth( m_OtherParmsLayout.GetW() - inputwidth - togglewidth - labelwidth - 20 );
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
    m_OtherParmsLayout.SetButtonWidth( togglewidth );
    m_OtherParmsLayout.AddButton( m_GroundEffectToggle, "" );
    m_OtherParmsLayout.SetButtonWidth( labelwidth );
    m_OtherParmsLayout.AddSlider( m_GroundEffectSlider, "Ground Effect Dist", 1e3, "%7.2f" );
    m_OtherParmsLayout.ForceNewLine();
    m_OtherParmsLayout.AddYGap();

    // Propeller and Stability Setup
    int button_width = 130;
    m_AdvancedRightLayout.AddSubGroupLayout( m_PropAndStabLayout,
        m_AdvancedRightLayout.GetW(),
        4 * m_AdvancedRightLayout.GetStdHeight() );
    m_AdvancedRightLayout.AddY( m_PropAndStabLayout.GetH() );
    m_PropAndStabLayout.SetButtonWidth( m_PropAndStabLayout.GetRemainX() / 2 );
    m_PropAndStabLayout.AddDividerBox( "Propeller Representation" );

    m_PropAndStabLayout.SetSameLineFlag( true );
    m_PropAndStabLayout.SetFitWidthFlag( false );
    m_PropAndStabLayout.AddButton( m_ActuatorDiskToggle, "Actuator Disk" );
    m_PropAndStabLayout.AddButton( m_RotateBladesToggle, "Rotating Blades" );
    m_PropAndStabLayout.ForceNewLine();

    m_PropAndStabLayout.AddYGap();
    m_PropAndStabLayout.SetSameLineFlag( false );
    m_PropAndStabLayout.SetFitWidthFlag( true );
    m_PropAndStabLayout.AddDividerBox( "Run Mode" );
    m_PropAndStabLayout.SetSameLineFlag( true );
    m_PropAndStabLayout.SetFitWidthFlag( false );

    m_PropAndStabLayout.SetChoiceButtonWidth( m_PropAndStabLayout.GetRemainX() / 2 );
    m_PropAndStabLayout.SetSliderWidth( m_PropAndStabLayout.GetRemainX() / 2 );

    m_PropAndStabLayout.AddChoice( m_StabilityTypeChoice, "Stability Type" );
    m_StabilityTypeChoice.AddItem( "Off" );
    m_StabilityTypeChoice.AddItem( "Steady" );
    m_StabilityTypeChoice.AddItem( "P Analysis" );
    m_StabilityTypeChoice.AddItem( "Q Analysis" );
    m_StabilityTypeChoice.AddItem( "R Analysis" );
    //m_StabilityTypeChoice.AddItem( "Heave" ); // To Be Implemented
    //m_StabilityTypeChoice.AddItem( "Impulse" ); // To Be Implemented
    m_StabilityTypeChoice.UpdateItems();
    m_PropAndStabLayout.ForceNewLine();

    // Advanced Flow
    m_AdvancedRightLayout.AddSubGroupLayout( m_FlowCondLayout,
                                             m_AdvancedRightLayout.GetW(),
                                             5 * m_AdvancedRightLayout.GetStdHeight() + 5 );
    m_AdvancedRightLayout.AddY( m_FlowCondLayout.GetH() );

    m_FlowCondLayout.AddDividerBox( "Advanced Flow Conditions" );
    m_FlowCondLayout.AddSlider( m_VinfSlider, "Vinf", 100, "%7.2f" );

    m_FlowCondLayout.SetSameLineFlag( true );
    m_FlowCondLayout.SetFitWidthFlag( false );

    button_width = m_FlowCondLayout.GetButtonWidth();
    m_FlowCondLayout.SetButtonWidth( togglewidth );
    m_FlowCondLayout.AddButton( m_ActivateVRefToggle, "" );
    m_FlowCondLayout.SetButtonWidth( button_width - togglewidth );
    m_FlowCondLayout.SetFitWidthFlag( true );
    m_FlowCondLayout.AddSlider( m_VRefSlider, "VRef", 100, "%7.2f" );
    m_FlowCondLayout.ForceNewLine();
    m_FlowCondLayout.SetButtonWidth( button_width );

    m_FlowCondLayout.SetSameLineFlag( false );
    m_FlowCondLayout.AddSlider( m_MachRefSlider, "MachRef", 1000, "%7.3g" );

    m_FlowCondLayout.AddYGap();

    m_FlowCondLayout.AddSlider( m_RhoSlider, "Rho", 1, "%2.5g" );

    m_AdvancedRightLayout.AddSubGroupLayout( m_CpSlicerLayout,
                                             m_AdvancedRightLayout.GetW(),
                                             10 * m_AdvancedRightLayout.GetStdHeight() );
    m_AdvancedRightLayout.AddY( m_CpSlicerLayout.GetH() );

    m_CpSlicerLayout.AddDividerBox( "Slicer" );

    m_CpSlicerLayout.SetSameLineFlag( true );
    m_CpSlicerLayout.SetFitWidthFlag( false );

    m_CpSlicerLayout.SetButtonWidth( m_CpSlicerLayout.GetRemainX() / 2 );

    m_CpSlicerLayout.AddButton( m_CpSlicerActivateToggle, "Activate Slicer" );
    m_CpSlicerLayout.AddButton( m_CpSliceLastADBButton, "Slice Latest *.adb File" );

    m_CpSlicerLayout.ForceNewLine();
    m_CpSlicerLayout.SetSameLineFlag( false );
    m_CpSlicerLayout.SetFitWidthFlag( true );

    // Pointer for the widths of each column in the browser to support resizing
    // Last column width must be 0
    static int cp_col_widths[] = { ( m_CpSlicerLayout.GetW() / 3 ) + 10, m_CpSlicerLayout.GetW() / 3, ( m_CpSlicerLayout.GetW() / 3 ) + 10, 0 }; // widths for each column

    int CpBrowserHeight = 75;
    m_CpSliceBrowser = m_CpSlicerLayout.AddColResizeBrowser( cp_col_widths, 3, CpBrowserHeight );
    m_CpSliceBrowser->callback( staticScreenCB, this );

    m_CpSlicerLayout.SetSameLineFlag( true );
    m_CpSlicerLayout.SetFitWidthFlag( false );

    m_CpSlicerLayout.SetButtonWidth( m_CpSlicerLayout.GetRemainX() / 3 );

    m_CpSlicerLayout.AddButton( m_AddCpSliceButton, "Add Slice" );
    m_CpSlicerLayout.AddButton( m_DeleteCpSliceButton, "Delete Slice" );
    m_CpSlicerLayout.AddButton( m_DeleteAllCpSliceButton, "Delete All" );

    m_CpSlicerLayout.AddYGap();
    m_CpSlicerLayout.ForceNewLine();

    m_CpSlicerLayout.AddSubGroupLayout( m_CpSlicerSubLayout,
                                        m_CpSlicerLayout.GetW(),
                                        10 * m_CpSlicerLayout.GetStdHeight() );
    m_CpSlicerLayout.AddY( m_CpSlicerSubLayout.GetH() );

    m_CpSlicerSubLayout.SetButtonWidth( m_CpSlicerLayout.GetRemainX() / 3 );
    m_CpSlicerSubLayout.SetChoiceButtonWidth( m_CpSlicerLayout.GetRemainX() / 3 );
    int input_width = m_CpSlicerSubLayout.GetInputWidth();
    m_CpSlicerSubLayout.SetInputWidth( m_CpSlicerLayout.GetRemainX() / 3 );

    m_CpSlicerSubLayout.SetSameLineFlag( true );
    m_CpSlicerSubLayout.SetFitWidthFlag( false );

    m_CpSlicerSubLayout.AddInput( m_CpSliceNameInput, "Name" );
    m_CpSlicerSubLayout.AddButton( m_CpSliceShowToggle, "Show Cut" );

    m_CpSlicerSubLayout.ForceNewLine();
    m_CpSlicerSubLayout.SetSameLineFlag( false );
    m_CpSlicerSubLayout.SetFitWidthFlag( true );
    m_CpSlicerSubLayout.SetInputWidth( input_width );

    m_CpSliceTypeChoice.AddItem( "X" );
    m_CpSliceTypeChoice.AddItem( "Y" );
    m_CpSliceTypeChoice.AddItem( "Z" );
    m_CpSlicerSubLayout.AddChoice( m_CpSliceTypeChoice, "Slice Type" );
    m_CpSliceTypeChoice.UpdateItems();

    m_CpSlicerSubLayout.AddSlider( m_CpSliceLocation, "Position", 100, "%7.3f" );

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

    //==== Rotor Disk Tab ==== //
    m_RotorDiskTab = AddTab( "Disk" );
    Fl_Group* rotor_group = AddSubGroup( m_RotorDiskTab, window_border_width );
    m_PropGeneralLayout.SetGroupAndScreen( rotor_group, this );

    // Prop Element Layout
    int prop_elem_browser_h = 200;
    m_PropGeneralLayout.AddSubGroupLayout( m_PropElemLayout,
        m_PropGeneralLayout.GetW(),
        6 * m_PropGeneralLayout.GetStdHeight() + prop_elem_browser_h );
    m_PropGeneralLayout.AddY( m_PropElemLayout.GetH() );

    m_PropElemLayout.SetSameLineFlag( false );
    m_PropElemLayout.SetFitWidthFlag( true );

    m_PropElemLayout.AddDividerBox( "Rotor Disk Element Settings" );

    // Initial column widths
    static int prop_col_widths[] = { 73, 122, 73, 73, 73, 73, 73, 73, 0 };

    m_PropElemBrowser = m_PropElemLayout.AddColResizeBrowser( prop_col_widths, 8, prop_elem_browser_h );
    m_PropElemBrowser->callback( staticScreenCB, this );

    input_width = 60;
    int XYZ_button_width = 20;
    int else_button_width = 120;
    int browser_augment = 40;

    m_PropElemLayout.SetButtonWidth( else_button_width );
    m_PropElemLayout.SetInputWidth( input_width );
    m_PropElemLayout.AddOutput( m_PropElemDia, "Dia." );

    m_PropElemLayout.SetFitWidthFlag( false );
    m_PropElemLayout.SetSameLineFlag( true );
    m_PropElemLayout.SetButtonWidth( else_button_width / 2 );

    m_PropElemLayout.AddButton( m_PropAutoHubDia, "Auto" );

    m_PropElemLayout.SetFitWidthFlag( true );

    m_PropElemLayout.AddSlider( m_PropElemHubDia, "Hub Dia.", 100, "%3.3f" );

    m_PropElemLayout.SetButtonWidth( else_button_width );
    m_PropElemLayout.ForceNewLine();
    m_PropElemLayout.SetSameLineFlag( false );

    m_PropElemLayout.AddSlider( m_PropElemRPM, "RPM", 10000, "%7.2f" );
    m_PropElemLayout.AddSlider( m_PropElemCT, "CT", 1, "%2.3f" );
    m_PropElemLayout.AddSlider( m_PropElemCP, "CP", 1, "%2.3f" );

    m_PropGeneralLayout.AddYGap();

    //==== Unsteady Group Tab ====//
    m_PropellerTab = AddTab( "Propeller" );
    Fl_Group* unsteady_group = AddSubGroup( m_PropellerTab, window_border_width );

    m_UnsteadyGroupLayout.SetGroupAndScreen( unsteady_group, this );

    m_UnsteadyGroupLayout.AddSubGroupLayout( m_UnsteadyGroupLeftLayout, ( unsteady_group->w() - window_border_width ) / 2,
                                             12 * m_UnsteadyGroupLeftLayout.GetStdHeight() );

    m_UnsteadyGroupLayout.AddX( m_UnsteadyGroupLeftLayout.GetW() + window_border_width );
    m_UnsteadyGroupLayout.AddSubGroupLayout( m_UnsteadyGroupRightLayout,
        ( unsteady_group->w() - window_border_width ) / 2,
                                             12 * m_UnsteadyGroupLeftLayout.GetStdHeight() );

    m_UnsteadyGroupLeftLayout.AddDividerBox( "Time" );

    m_UnsteadyGroupLeftLayout.SetButtonWidth( m_UnsteadyGroupLeftLayout.GetRemainX() / 3 );

    m_UnsteadyGroupLeftLayout.AddButton( m_AutoTimeStepToggle, "Auto Time Step" );
    m_UnsteadyGroupLeftLayout.AddSlider( m_AutoTimeNumRevSlider, "Num Revs", 10, "%4.0f" );

    m_UnsteadyGroupLeftLayout.AddYGap();

    m_UnsteadyGroupLeftLayout.AddSlider( m_TimeStepSizeSlider, "Time Step", 2.0, "%7.5f" );
    m_UnsteadyGroupLeftLayout.AddSlider( m_NumTimeStepSlider, "Num Time Step", 100, "%4.0f" );

    m_UnsteadyGroupLeftLayout.AddYGap();

    m_UnsteadyGroupLeftLayout.AddDividerBox( "Advanced" );

    m_UnsteadyGroupLeftLayout.SetSameLineFlag( true );
    m_UnsteadyGroupLeftLayout.SetFitWidthFlag( false );
    m_UnsteadyGroupLeftLayout.SetSliderWidth( m_UnsteadyGroupLeftLayout.GetRemainX() -
                                              m_UnsteadyGroupLeftLayout.GetButtonWidth() -
                                              2 * m_UnsteadyGroupLeftLayout.GetRangeButtonWidth() -
                                              m_UnsteadyGroupLeftLayout.GetInputWidth() );
    m_UnsteadyGroupLeftLayout.AddButton( m_HoverRampToggle, "Hover Ramp" );
    m_UnsteadyGroupLeftLayout.SetButtonWidth( 0 );
    m_UnsteadyGroupLeftLayout.AddSlider( m_HoverRampSlider, "/", 1e3, "%7.3f" ); // Placeholder label, since otherwise the parm name is automatically used

    m_UnsteadyGroupLeftLayout.ForceNewLine();
    m_UnsteadyGroupLeftLayout.SetSameLineFlag( false );
    m_UnsteadyGroupLeftLayout.SetFitWidthFlag( true );

    m_UnsteadyGroupLeftLayout.AddButton( m_FromSteadyStateToggle, "From Steady State" );
    m_UnsteadyGroupLeftLayout.AddYGap();

    m_UnsteadyGroupLeftLayout.AddDividerBox( "Noise Analysis" );

    m_UnsteadyGroupLeftLayout.SetSameLineFlag( true );
    m_UnsteadyGroupLeftLayout.SetFitWidthFlag( false );

    m_UnsteadyGroupLeftLayout.SetButtonWidth( m_UnsteadyGroupLeftLayout.GetRemainX() / 2 );
    m_UnsteadyGroupLeftLayout.AddButton( m_NoiseCalcToggle, "Enable Noise Calc" );
    m_UnsteadyGroupLeftLayout.AddButton( m_NoiseCalcTrigger, "Analyze Latest *.adb" );
    m_UnsteadyGroupLeftLayout.ForceNewLine();

    m_UnsteadyGroupLeftLayout.SetSameLineFlag( false );
    m_UnsteadyGroupLeftLayout.SetFitWidthFlag( true );

    m_UnsteadyGroupLeftLayout.SetChoiceButtonWidth( m_UnsteadyGroupLeftLayout.GetRemainX() / 2 );
    m_UnsteadyGroupLeftLayout.AddChoice( m_NoiseCalcChoice, "Noise Type" );
    m_NoiseCalcChoice.AddItem( "Flyby" );
    m_NoiseCalcChoice.AddItem( "Footprint" );
    m_NoiseCalcChoice.AddItem( "Steady" );
    m_NoiseCalcChoice.UpdateItems();

    m_UnsteadyGroupLeftLayout.AddChoice( m_NoiseUnitChoice, "Model Length Unit" );
    m_NoiseUnitChoice.AddItem( "SI" );
    m_NoiseUnitChoice.AddItem( "English" );
    m_NoiseUnitChoice.UpdateItems();

    m_UnsteadyGroupLeftLayout.ForceNewLine();
    m_UnsteadyGroupLeftLayout.AddYGap();

    m_UnsteadyGroupRightLayout.AddDividerBox( "Propellers" );

    // Pointer for the widths of each column in the browser to support resizing
    // Last column width must be 0
    static int unsteady_col_widths[] = { 128, 104, 73, 0 }; // widths for each column

    int UnsteadyBrowserHeight = 125;
    m_UnsteadyGroupBrowser = m_UnsteadyGroupRightLayout.AddColResizeBrowser( unsteady_col_widths, 3, UnsteadyBrowserHeight );
    m_UnsteadyGroupBrowser->callback( staticScreenCB, this );

    m_UnsteadyGroupRightLayout.SetButtonWidth( m_UnsteadyGroupRightLayout.GetRemainX() / 3 );
    m_UnsteadyGroupRightLayout.AddSlider( m_RPMSlider, "RPM", 100, "%7.3f" );

    m_UnsteadyGroupRightLayout.AddButton( m_UniformRPMToggle, "Uniform RPM" );

    m_UnsteadyGroupLayout.AddY( m_UnsteadyGroupLeftLayout.GetH() + window_border_width );

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
    m_SolverDisplay->buffer( NULL );
    delete m_SolverBuffer;
    m_ViewerDisplay->buffer( NULL );
    delete m_ViewerBuffer;
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
        UpdateCpSlices();
        UpdateCpSliceBrowser();

        UpdatePropElemDevices();

        UpdateOtherSetupParms();

        UpdateDeflectionAngleScrollGroup();
        UpdateDeflectionGainScrollGroup();

        UpdatePropElemBrowser();
        UpdateControlSurfaceBrowsers();

        UpdateUnsteadyGroups();
        UpdateUnsteadyGroupBrowser();
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
    else if ( w == m_CpSliceBrowser )
    {
        CpSliceBrowserCallback();
    }
    else if ( w == m_UnsteadyGroupBrowser )
    {
        UnsteadyGroupBrowserCallback();
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

            BUF_READ_TYPE nread = 1;

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

        vsscreen->GetScreenMgr()->m_ShowPlotScreenOnce = true;   //deferred show of plot screen

        // Display default view settings
        VSPAEROPlotScreen* vspapscreen = (VSPAEROPlotScreen*)vsscreen->GetScreenMgr()->GetScreen( ScreenMgr::VSP_VSPAERO_PLOT_SCREEN );
        if ( vspapscreen )
        {
            vspapscreen->SetDefaultView();
        }

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

                // Clear the solver console
                m_SolverBuffer->text( "" );

                // Check for transonic Mach numbers and warn the user if found
                double transonic_mach_min = 0.8;
                double transonic_mach_max = 1.2;
                double mach_delta = 0.0;
                vector < double > mach_vec( VSPAEROMgr.m_MachNpts.Get() );

                // Identify Mach flow condition vector
                if( VSPAEROMgr.m_MachNpts.Get() > 1 )
                {
                    mach_delta = ( VSPAEROMgr.m_MachEnd.Get() - VSPAEROMgr.m_MachStart.Get() ) / ( VSPAEROMgr.m_MachNpts.Get() - 1.0 );
                }
                for( size_t iMach = 0; iMach < VSPAEROMgr.m_MachNpts.Get(); iMach++ )
                {
                    mach_vec[iMach] = VSPAEROMgr.m_MachStart.Get() + double( iMach ) * mach_delta;

                    if( mach_vec[iMach] > transonic_mach_min && mach_vec[iMach] < transonic_mach_max )
                    {
                        AddOutputText( m_SolverDisplay, "WARNING: Possible transonic Mach number detected - transonic flow is not supported.\n\n" );
                        break;
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

                string command = veh->GetExePath() + string("/") + veh->GetVIEWERCmd() + string(" ")
                        + VSPAEROMgr.m_ModelNameBase + string("\n");
                m_ViewerBuffer->text( command.c_str() );

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
            DisplayDegenVLMPreview();
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
            int file_type = vsp::DEGEN_GEOM_CSV_TYPE;
            string file_ext = "*.csv";
            string message = "Select degen geom CSV output file.";

            if ( VSPAEROMgr.m_AlternateInputFormatFlag() )
            {
                file_type = vsp::VSPAERO_VSPGEOM_TYPE;
                file_ext = "*.vspgeom";
                message = "Select degen geom VSPGEOM output file.";
            }

            veh->setExportFileName( file_type, m_ScreenMgr->GetSelectFileScreen()->FileChooser( message.c_str(), file_ext.c_str() ) );
        }
        else if( device == &m_CompGeomFileButton )
        {
            int file_type = vsp::VSPAERO_VSPGEOM_TYPE;
            string file_ext = "*.vspgeom";
            string message = "Select comp geom VSPGEOM output file.";

            if ( VSPAEROMgr.m_AlternateInputFormatFlag() )
            {
                file_type = vsp::VSPAERO_PANEL_TRI_TYPE;
                file_ext = "*.tri";
                message = "Select comp geom TRI output file.";
            }

            veh->setExportFileName( file_type, m_ScreenMgr->GetSelectFileScreen()->FileChooser( message.c_str(), file_ext.c_str() ) );
        }
        else if ( device == &m_LoadExistingResultsButton )
        {
            if ( VSPAEROMgr.LoadExistingVSPAEROResults() != string() )
            {
                m_ScreenMgr->m_ShowPlotScreenOnce = true;   //deferred show of plot screen

                // Display default view settings
                VSPAEROPlotScreen* vspapscreen = (VSPAEROPlotScreen*)m_ScreenMgr->GetScreen( ScreenMgr::VSP_VSPAERO_PLOT_SCREEN );
                if ( vspapscreen )
                {
                    vspapscreen->SetDefaultView();
                }
            }
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
        else if ( device == &m_CpSliceLastADBButton )
        {
            // Clear out previous results
            VSPAEROMgr.ClearCpSliceResults();

            VSPAEROMgr.ComputeCpSlices();

            VSPAEROMgr.ComputeQuadTreeSlices();
        }
        else if ( device == &m_AddCpSliceButton )
        {
            CpSlice* slice = VSPAEROMgr.AddCpSlice();

            if ( slice )
            {
                VSPAEROMgr.SetCurrentCpSliceIndex( VSPAEROMgr.GetCpSliceVec().size() - 1 );
            }
        }
        else if ( device == &m_DeleteCpSliceButton )
        {
            VSPAEROMgr.DelCpSlice( VSPAEROMgr.GetCurrentCpSliceIndex() );

            if ( VSPAEROMgr.ValidCpSliceInd( VSPAEROMgr.GetCurrentCpSliceIndex() - 1 ) )
            {
                VSPAEROMgr.SetCurrentCpSliceIndex( VSPAEROMgr.GetCurrentCpSliceIndex() );
            }
            else
            {
                VSPAEROMgr.SetCurrentCpSliceIndex( -1 );
            }
        }
        else if ( device == &m_DeleteAllCpSliceButton )
        {
            VSPAEROMgr.ClearCpSliceVec();
            VSPAEROMgr.SetCurrentCpSliceIndex( -1 );
        }
        else if ( device == &m_CpSliceNameInput )
        {
            CpSlice* slice = VSPAEROMgr.GetCpSlice( VSPAEROMgr.GetCurrentCpSliceIndex() );
            if ( slice )
            {
                slice->SetName( m_CpSliceNameInput.GetString() );
            }
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
        else if ( device == &m_NoiseCalcTrigger )
        {
            VSPAEROMgr.ExecuteNoiseAnalysis( NULL, VSPAEROMgr.m_NoiseCalcType(), VSPAEROMgr.m_NoiseUnits() );
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

    if ( VSPAEROMgr.m_AlternateInputFormatFlag.Get() )
    {
        m_DegenFileName.Update( veh->getExportFileName( vsp::VSPAERO_VSPGEOM_TYPE ) );
        m_CompGeomFileName.Update( veh->getExportFileName( vsp::VSPAERO_PANEL_TRI_TYPE ) );
    }
    else
    {
        m_DegenFileName.Update( veh->getExportFileName( vsp::DEGEN_GEOM_CSV_TYPE ) );
        m_CompGeomFileName.Update( veh->getExportFileName( vsp::VSPAERO_VSPGEOM_TYPE ) );
    }

    m_NCPUSlider.Update(VSPAEROMgr.m_NCPU.GetID());
    m_BatchCalculationToggle.Update(VSPAEROMgr.m_BatchModeFlag.GetID());
    m_PreconditionChoice.Update(VSPAEROMgr.m_Precondition.GetID());
    m_KTCorrectionToggle.Update( VSPAEROMgr.m_KTCorrection.GetID() );
    m_SymmetryToggle.Update( VSPAEROMgr.m_Symmetry.GetID() );
    m_Write2DFEMToggle.Update( VSPAEROMgr.m_Write2DFEMFlag.GetID() );
    m_EnableAlternateFormat.Update( VSPAEROMgr.m_AlternateInputFormatFlag.GetID() );

    // Wake Options
    m_FixedWakeToggle.Update( VSPAEROMgr.m_FixedWakeFlag.GetID() );
    m_WakeNumIterSlider.Update(VSPAEROMgr.m_WakeNumIter.GetID());
    m_NumWakeNodeSlider.Update( VSPAEROMgr.m_NumWakeNodes.GetID() );

    bool time_dependent = false;
    if ( VSPAEROMgr.m_RotateBladesFlag() || VSPAEROMgr.m_StabilityType() == vsp::STABILITY_P_ANALYSIS ||
         VSPAEROMgr.m_StabilityType() == vsp::STABILITY_Q_ANALYSIS || VSPAEROMgr.m_StabilityType() == vsp::STABILITY_R_ANALYSIS )
    {
        time_dependent = true;
    }

    if ( time_dependent )
    {
        m_FixedWakeToggle.Deactivate();
    }
    else
    {
        m_FixedWakeToggle.Activate();
    }

    if ( time_dependent || VSPAEROMgr.m_FixedWakeFlag() )
    {
        m_WakeNumIterSlider.Deactivate();
    }
    else
    {
        m_WakeNumIterSlider.Activate();
    }

    // Other Set Up Parms
    m_ClmaxChoice.Update( VSPAEROMgr.m_ClMaxToggle.GetID() );
    m_ClmaxSlider.Update( VSPAEROMgr.m_ClMax.GetID() );
    m_MaxTurningToggle.Update( VSPAEROMgr.m_MaxTurnToggle.GetID() );
    m_MaxTurningSlider.Update( VSPAEROMgr.m_MaxTurnAngle.GetID() );
    m_FarDistToggle.Update( VSPAEROMgr.m_FarDistToggle.GetID() );
    m_FarDistSlider.Update( VSPAEROMgr.m_FarDist.GetID() );
    m_GroundEffectToggle.Update( VSPAEROMgr.m_GroundEffectToggle.GetID() );
    m_GroundEffectSlider.Update( VSPAEROMgr.m_GroundEffect.GetID() );

    // Stability
    if (VSPAEROMgr.m_Symmetry())
    {
        VSPAEROMgr.m_StabilityType.Set( vsp::STABILITY_OFF );
    }

    m_StabilityTypeChoice.Update( VSPAEROMgr.m_StabilityType.GetID() );

    m_ActuatorDiskToggle.Update( VSPAEROMgr.m_ActuatorDiskFlag.GetID() );
    m_RotateBladesToggle.Update( VSPAEROMgr.m_RotateBladesFlag.GetID() );

    if ( VSPAEROMgr.GetRotorDiskVec().size() == 0 )
    {
        m_ActuatorDiskToggle.Deactivate();
    }
    else
    {
        m_ActuatorDiskToggle.Activate();
    }

    if ( VSPAEROMgr.NumUnsteadyRotorGroups() == 0 )
    {
        m_RotateBladesToggle.Deactivate();
    }
    else
    {
        m_RotateBladesToggle.Activate();
    }

    if ( VSPAEROMgr.m_RotateBladesFlag.Get() )
    {
        m_PropellerTab->activate();
        m_RotorDiskTab->deactivate();
    }
    else if ( VSPAEROMgr.m_ActuatorDiskFlag.Get() )
    {
        m_PropellerTab->deactivate();
        m_RotorDiskTab->activate();
    }
    else
    {
        m_PropellerTab->deactivate();
        m_RotorDiskTab->deactivate();
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
    // ReCref
    m_ReCrefStartInput.Update( VSPAEROMgr.m_ReCrefStart.GetID() );
    m_ReCrefEndInput.Update( VSPAEROMgr.m_ReCrefEnd.GetID() );
    m_ReCrefNptsInput.Update( VSPAEROMgr.m_ReCrefNpts.GetID() );
    if ( VSPAEROMgr.m_ReCrefNpts.Get() == 1 )
    {
        m_ReCrefEndInput.Deactivate();
    }
    else if ( VSPAEROMgr.m_ReCrefNpts.Get() > 1 )
    {
        m_ReCrefEndInput.Activate();
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
    if (index >= 0 && index < rotordiskvec.size())
    {
        rotordiskvec[ index ]->m_Diameter.Activate();
        // m_HubDiameter parm activated/deactivated automatically in VSPAEROMgr.UpdateRotorDisks()
        rotordiskvec[ index ]->m_RPM.Activate();
        rotordiskvec[ index ]->m_CP.Activate();
        rotordiskvec[ index ]->m_CT.Activate();

        m_PropElemDia.Update(to_string(rotordiskvec[ index ]->m_Diameter()));
        rotordiskvec[ index ]->m_HubDiameter.SetUpperLimit( rotordiskvec[ index ]->m_Diameter() );
        m_PropElemHubDia.SetMaxBound( rotordiskvec[ index ]->m_Diameter() );
        m_PropElemHubDia.Update( rotordiskvec[ index ]->m_HubDiameter.GetID() );
        m_PropAutoHubDia.Update( rotordiskvec[index]->m_AutoHubDiaFlag.GetID() );
        m_PropElemRPM.Update( rotordiskvec[ index ]->m_RPM.GetID());
        m_PropElemCP.Update( rotordiskvec[ index ]->m_CP.GetID());
        m_PropElemCT.Update( rotordiskvec[ index ]->m_CT.GetID());
    }

    if (index > rotordiskvec.size())
    {
        VSPAEROMgr.SetCurrentRotorDiskIndex( -1 );
    }
}

void VSPAEROScreen::UpdatePropElemBrowser()
{
    char str[256];
    int h_pos = m_PropElemBrowser->hposition();
    int v_pos = m_PropElemBrowser->position();
    m_PropElemBrowser->clear();

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

    m_PropElemBrowser->hposition( h_pos );
    m_PropElemBrowser->position( v_pos );
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
    if ( VSPAEROMgr.GetCurrentCSGroupIndex() != -1 )
    {
        // For loop on a vector of ungrouped control surfaces
        vector < VspAeroControlSurf > ungrouped_cs = VSPAEROMgr.GetAvailableCSVec();

        for ( size_t i = 0; i < ungrouped_cs.size(); ++i )
        {
            m_UngroupedCSBrowser->add( ungrouped_cs[i].fullName.c_str() );
        }
        for ( size_t i = 0; i < VSPAEROMgr.GetSelectedUngroupedItems().size(); ++i )
        {
            SelectUngroupedListBrowser( VSPAEROMgr.GetSelectedUngroupedItems()[i] );
        }
    }
}

void VSPAEROScreen::UpdateOtherSetupParms()
{
    m_VinfSlider.Update( VSPAEROMgr.m_Vinf.GetID() );
    m_RhoSlider.Update( VSPAEROMgr.m_Rho.GetID() );
    m_ActivateVRefToggle.Update( VSPAEROMgr.m_ManualVrefFlag.GetID() );
    m_VRefSlider.Update( VSPAEROMgr.m_Vref.GetID() );
    m_MachRefSlider.Update( VSPAEROMgr.m_Machref.GetID() );

    if ( !VSPAEROMgr.m_ManualVrefFlag() )
    {
        m_VRefSlider.Deactivate();
        m_MachRefSlider.Deactivate();
    }
    else
    {
        m_VRefSlider.Activate();
        m_MachRefSlider.Activate();
    }

    if ( VSPAEROMgr.m_RotateBladesFlag() )
    {
        m_AlphaNptsInput.Deactivate();
        m_BetaNptsInput.Deactivate();
        m_MachNptsInput.Deactivate();
        m_StabilityTypeChoice.Deactivate();
    }
    else
    {
        m_AlphaNptsInput.Activate();
        m_BetaNptsInput.Activate();
        m_MachNptsInput.Activate();
        m_StabilityTypeChoice.Activate();
    }

    if ( VSPAEROMgr.m_RotateBladesFlag() || VSPAEROMgr.m_StabilityType.Get() >= vsp::STABILITY_P_ANALYSIS )
    {
        m_ReCrefNptsInput.Deactivate();
    }
    else
    {
        m_ReCrefNptsInput.Activate();
    }

    if ( VSPAEROMgr.m_RotateBladesFlag.Get() || VSPAEROMgr.m_ActuatorDiskFlag.Get() || VSPAEROMgr.m_StabilityType.Get() > vsp::STABILITY_OFF )
    {
        m_VinfSlider.Activate();
        m_ActivateVRefToggle.Activate();
        m_RhoSlider.Activate();
    }
    else
    {
        m_VinfSlider.Deactivate();
        m_ActivateVRefToggle.Deactivate();
        m_RhoSlider.Deactivate();
    }
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
    vector < VspAeroControlSurf > ungrouped_vec = VSPAEROMgr.GetAvailableCSVec();
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
    if ( cur_index > 0 && cur_index <= VSPAEROMgr.GetAvailableCSVec().size() )
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

void VSPAEROScreen::DisplayDegenVLMPreview()
{
    Vehicle* veh = m_ScreenMgr->GetVehiclePtr();

    if ( !veh )
    {
        return;
    }

    vector< Geom* > geom_vec = veh->FindGeomVec( veh->GetGeomVec() );
    for ( size_t i = 0; i < (int)geom_vec.size(); i++ )
    {
        if ( geom_vec[i]->GetSetFlag( VSPAEROMgr.m_GeomSet() ) )
        {
            for ( size_t j = 0; j < geom_vec[i]->GetNumMainSurfs(); j++ )
            {
                int surf_type = geom_vec[i]->GetMainSurfType(j);

                if ( surf_type == vsp::DISK_SURF )
                {
                    geom_vec[i]->m_GuiDraw.SetDisplayType( vsp::DISPLAY_TYPE::DISPLAY_DEGEN_SURF );
                }
                else if ( surf_type == vsp::WING_SURF )
                {
                    geom_vec[i]->m_GuiDraw.SetDisplayType( vsp::DISPLAY_TYPE::DISPLAY_DEGEN_CAMBER );
                }
                else
                {
                    geom_vec[i]->m_GuiDraw.SetDisplayType( vsp::DISPLAY_TYPE::DISPLAY_DEGEN_PLATE );
                }
            }

            geom_vec[i]->m_GuiDraw.SetDrawType( vsp::DRAW_TYPE::GEOM_DRAW_SHADE );
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

void VSPAEROScreen::UpdateCpSlices()
{
    m_CpSlicerActivateToggle.Update( VSPAEROMgr.m_CpSliceFlag.GetID() );

    if ( VSPAEROMgr.GetCpSliceVec().size() <= 0 )
    {
        m_CpSlicerActivateToggle.Deactivate();
    }
    else
    {
        m_CpSlicerActivateToggle.Activate();
    }

    CpSlice* slice = VSPAEROMgr.GetCpSlice( VSPAEROMgr.GetCurrentCpSliceIndex() );
    if ( slice )
    {
        m_CpSlicerSubLayout.Show();
        m_CpSliceNameInput.Update( slice->GetName() );
        m_CpSliceTypeChoice.Update( slice->m_CutType.GetID() );
        m_CpSliceLocation.Update( slice->m_CutPosition.GetID() );
        m_CpSliceShowToggle.Update( slice->m_DrawCutFlag.GetID() );
    }
    else
    {
        m_CpSlicerSubLayout.Hide();
    }
}

void VSPAEROScreen::UpdateCpSliceBrowser()
{
    char str[256];
    int h_pos = m_CpSliceBrowser->hposition();
    int v_pos = m_CpSliceBrowser->position();
    m_CpSliceBrowser->clear();

    m_CpSliceBrowser->column_char( ':' );         // use : as the column character

    m_CpSliceBrowser->add( "@b@.Name:@b@.Type:@b@.Position" );

    for ( size_t i = 0; i < VSPAEROMgr.GetCpSliceVec().size(); ++i )
    {
        CpSlice* slice = VSPAEROMgr.GetCpSliceVec()[i];
        if ( slice )
        {
            char type = 88 + slice->m_CutType(); // ASCII X: 88; Y: 89; Z: 90

            sprintf( str, "%s:%c:%4.2f", slice->GetName().c_str(), type, slice->m_CutPosition() );
            m_CpSliceBrowser->add( str );
        }
    }

    SelectCpSliceBrowser( VSPAEROMgr.GetCurrentCpSliceIndex() + 2 );

    m_CpSliceBrowser->hposition( h_pos );
    m_CpSliceBrowser->position( v_pos );
}

void VSPAEROScreen::SelectCpSliceBrowser( int cur_index )
{
    if ( cur_index > 0 )
    {
        //==== Select If Match ====//
        m_CpSliceBrowser->select( cur_index );

        //==== Position Browser ====//
        m_CpSliceBrowser->topline( cur_index );
    }
}

void VSPAEROScreen::CpSliceBrowserCallback()
{
    //==== Find Last Selected CpSlice ====//
    int last = m_CpSliceBrowser->value();
    if ( last >= 2 )
    {
        VSPAEROMgr.SetCurrentCpSliceIndex( last - 2 );
    }
}

void VSPAEROScreen::UpdateUnsteadyGroups()
{
    m_AutoTimeStepToggle.Update( VSPAEROMgr.m_AutoTimeStepFlag.GetID() );
    m_AutoTimeNumRevSlider.Update( VSPAEROMgr.m_AutoTimeNumRevs.GetID() );
    m_TimeStepSizeSlider.Update( VSPAEROMgr.m_TimeStepSize.GetID() );
    m_NumTimeStepSlider.Update( VSPAEROMgr.m_NumTimeSteps.GetID() );
    m_HoverRampToggle.Update( VSPAEROMgr.m_HoverRampFlag.GetID() );
    m_HoverRampSlider.Update( VSPAEROMgr.m_HoverRamp.GetID() );
    m_FromSteadyStateToggle.Update( VSPAEROMgr.m_FromSteadyState.GetID() );
    m_NoiseCalcToggle.Update( VSPAEROMgr.m_NoiseCalcFlag.GetID() );
    m_NoiseCalcChoice.Update( VSPAEROMgr.m_NoiseCalcType.GetID() );
    m_UniformRPMToggle.Update( VSPAEROMgr.m_UniformPropRPMFlag.GetID() );

    if ( VSPAEROMgr.m_RotateBladesFlag() )
    {

        if ( !VSPAEROMgr.m_HoverRampFlag.Get() )
        {
            m_HoverRampSlider.Deactivate();
        }
        else
        {
            m_HoverRampSlider.Activate();
        }

        if ( VSPAEROMgr.m_AutoTimeStepFlag.Get() )
        {
            m_AutoTimeNumRevSlider.Activate();
            m_TimeStepSizeSlider.Deactivate();
            m_NumTimeStepSlider.Deactivate();
        }
        else
        {
            m_AutoTimeNumRevSlider.Deactivate();
            m_TimeStepSizeSlider.Activate();
            m_NumTimeStepSlider.Activate();
        }
    }

    int group_index = VSPAEROMgr.GetCurrentUnsteadyGroupIndex();

    UnsteadyGroup* group = VSPAEROMgr.GetUnsteadyGroup( group_index );
    if ( group )
    {
        m_RPMSlider.Update( group->m_RPM.GetID() );

        if ( group->m_GeomPropertyType() == group->GEOM_ROTOR )
        {
            m_RPMSlider.Activate();
        }
        else
        {
            m_RPMSlider.Deactivate();
        }
    }
}

void VSPAEROScreen::UpdateUnsteadyGroupBrowser()
{
    int h_pos = m_UnsteadyGroupBrowser->hposition();
    int v_pos = m_UnsteadyGroupBrowser->position();
    m_UnsteadyGroupBrowser->clear();

    m_UnsteadyGroupBrowser->column_char( ':' );         // use : as the column character
    m_UnsteadyGroupBrowser->add( "@b@.Name:@b@.Surf:@b@.RPM" );

    Vehicle* veh = VehicleMgr.GetVehicle();
    if ( !veh )
    {
        return;
    }

    for ( size_t i = 0; i < VSPAEROMgr.NumUnsteadyGroups(); i++ )
    {
        UnsteadyGroup* group = VSPAEROMgr.GetUnsteadyGroup( i );
        if ( group && group->m_GeomPropertyType() == group->GEOM_ROTOR )
        {
            char str[256];

            int surf_index = group->GetCompSurfPairVec()[0].second; // Only 1 component in rotor groups
            sprintf( str, "%s:%d:%4.3f", group->GetName().c_str(), surf_index, group->m_RPM.Get() );
            m_UnsteadyGroupBrowser->add( str );

            int jump = 0;
            if ( VSPAEROMgr.NumUnsteadyGroups() == VSPAEROMgr.NumUnsteadyRotorGroups() )
            {
                jump  = 2;
            }
            else
            {
                jump  = 1;
            }

            SelectUnsteadyGroupBrowser( VSPAEROMgr.GetCurrentUnsteadyGroupIndex() + jump );
        }
    }

    m_UnsteadyGroupBrowser->hposition( h_pos );
    m_UnsteadyGroupBrowser->position( v_pos );
}
void VSPAEROScreen::SelectUnsteadyGroupBrowser( int cur_index )
{
    if ( cur_index > 0 )
    {
        //==== Select If Match ====//
        m_UnsteadyGroupBrowser->select( cur_index );

        //==== Position Browser ====//
        m_UnsteadyGroupBrowser->topline( cur_index );
    }
}

void VSPAEROScreen::UnsteadyGroupBrowserCallback()
{
    //==== Find Last Selected Group ====//
    int last = m_UnsteadyGroupBrowser->value();

    // Account for the fixed component group being in the vector but not the browser
    if ( VSPAEROMgr.NumUnsteadyGroups() == VSPAEROMgr.NumUnsteadyRotorGroups() )
    {
        last -= 2;
    }
    else
    {
        last -= 1;
    }

    UnsteadyGroup* group = VSPAEROMgr.GetUnsteadyGroup( last );

    if ( last >= 0 && group )
    {
        VSPAEROMgr.SetCurrentUnsteadyGroupIndex( last );
    }

    VSPAEROMgr.HighlightSelected( VSPAEROMgr.UNSTEADY_GROUP );
}
