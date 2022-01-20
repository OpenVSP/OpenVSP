//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// VSPAEROPlotScreen.cpp: implementation of the VSPAEROPlotScreen class.
//
//////////////////////////////////////////////////////////////////////

#include "VSPAEROPlotScreen.h"
#include "float.h"

#include "VspUtil.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

// Needs to fit on 1024x768 native resolution
#define VSPAERO_PLOT_SCREEN_WIDTH 950   //<1024
#define VSPAERO_PLOT_SCREEN_HEIGHT 700  //<768

VSPAEROPlotScreen::VSPAEROPlotScreen( ScreenMgr* mgr ) : TabScreen( mgr, VSPAERO_PLOT_SCREEN_WIDTH, VSPAERO_PLOT_SCREEN_HEIGHT, "Results Manager - VSPAERO" )
{
    m_FLTK_Window->callback( staticCloseCB, this );

    int windowBorderWidth = 5;
    int groupBorderWidth = 2;

    int totalWidth = VSPAERO_PLOT_SCREEN_WIDTH - 2 * windowBorderWidth;
    int totalHeight = VSPAERO_PLOT_SCREEN_HEIGHT - 2 * windowBorderWidth;
    int controlWidth = 200;


    //==== Convergence Tab ====//
    m_ConvergenceTab = AddTab( "Convergence" );
    Fl_Group* convergencePlotGroup = AddSubGroup( m_ConvergenceTab, windowBorderWidth );
    m_ConvergenceLayout.SetGroupAndScreen( convergencePlotGroup, this );

    m_ConvergenceLayout.AddX( groupBorderWidth );
    m_ConvergenceLayout.AddY( groupBorderWidth );

    // Control layout
    m_ConvergenceLayout.AddSubGroupLayout( m_ConvergenceControlLayout, controlWidth, m_ConvergenceLayout.GetH() - 2 * groupBorderWidth );

    // layout the heights of the control layout
    int rowHeight = 20;
    int yDataSelectHeight = 11 * rowHeight;
    int legendHeight = 6 * rowHeight;
    int actionButtonHeight = 6 * rowHeight;          //space reserved for action buttons at the bottom
    //  remaining space is used for the flow condition browser
    int flowConditionSelectHeight = m_ConvergenceLayout.GetH() - 2 * groupBorderWidth - yDataSelectHeight - legendHeight - actionButtonHeight - groupBorderWidth;

    GroupLayout convergenceYDataSelectLayout;
    m_ConvergenceControlLayout.AddSubGroupLayout( convergenceYDataSelectLayout, m_ConvergenceControlLayout.GetW(), yDataSelectHeight );
    convergenceYDataSelectLayout.AddDividerBox( "Y-Data" );
    m_ConvergenceYDataBrowser = convergenceYDataSelectLayout.AddFlBrowser( convergenceYDataSelectLayout.GetRemainY() );
    m_ConvergenceYDataBrowser->callback( staticScreenCB, this );
    m_ConvergenceYDataBrowser->type( FL_MULTI_BROWSER );
    m_ConvergenceControlLayout.AddY( convergenceYDataSelectLayout.GetH() + 2 * groupBorderWidth );

    GroupLayout convergenceFlowConditionLayout;
    m_ConvergenceControlLayout.AddSubGroupLayout( convergenceFlowConditionLayout, m_ConvergenceControlLayout.GetW(), flowConditionSelectHeight );
    convergenceFlowConditionLayout.AddDividerBox( "Flow Condition" );
    m_ConvergenceFlowConditionBrowser = convergenceFlowConditionLayout.AddFlBrowser( convergenceFlowConditionLayout.GetRemainY() );
    m_ConvergenceFlowConditionBrowser->callback( staticScreenCB, this );
    m_ConvergenceFlowConditionBrowser->type( FL_MULTI_BROWSER );
    m_ConvergenceControlLayout.AddY( convergenceFlowConditionLayout.GetH() + 2 * groupBorderWidth );

    m_ConvergenceControlLayout.AddDividerBox( "Legend" );
    m_ConvergenceLegendGroup = m_ConvergenceControlLayout.AddFlScroll( legendHeight - rowHeight );
    m_ConvergenceLegendGroup->type( Fl_Scroll::VERTICAL_ALWAYS );
    m_ConvergenceLegendLayout.SetGroupAndScreen( m_ConvergenceLegendGroup, this );
    m_ConvergenceControlLayout.AddYGap();

    // Action buttons
    GroupLayout convergenceActionLayout;
    m_ConvergenceControlLayout.AddSubGroupLayout( convergenceActionLayout, m_ConvergenceControlLayout.GetW(), actionButtonHeight );
    convergenceActionLayout.AddDividerBox( "Actions:" );
    convergenceActionLayout.AddButton( m_ConvergenceYDataResidualToggle, "Residual log10(|Y(i)-Y(i-1) |)" );
    m_ConvergenceYDataResidualToggle.GetFlButton()->set();      //turn this on by default

    convergenceActionLayout.SetSameLineFlag( true );
    int actionToggleButtonWidth = 15;
    int actionSliderButtonWidth = 35;

    convergenceActionLayout.SetFitWidthFlag( false );
    convergenceActionLayout.SetButtonWidth( actionToggleButtonWidth );
    convergenceActionLayout.AddButton( m_ConvergenceManualXMinToggle, "" );
    convergenceActionLayout.SetButtonWidth( actionSliderButtonWidth );
    convergenceActionLayout.SetFitWidthFlag( true );
    convergenceActionLayout.AddSlider( m_ConvergenceXMinSlider, "Xmin", 1.0, "%g" );

    convergenceActionLayout.ForceNewLine();

    convergenceActionLayout.SetFitWidthFlag( false );
    convergenceActionLayout.SetButtonWidth( actionToggleButtonWidth );
    convergenceActionLayout.AddButton( m_ConvergenceManualXMaxToggle, "" );
    convergenceActionLayout.SetButtonWidth( actionSliderButtonWidth );
    convergenceActionLayout.SetFitWidthFlag( true );
    convergenceActionLayout.AddSlider( m_ConvergenceXMaxSlider, "Xmax", 1.0, "%g" );

    convergenceActionLayout.ForceNewLine();

    convergenceActionLayout.SetFitWidthFlag( false );
    convergenceActionLayout.SetButtonWidth( actionToggleButtonWidth );
    convergenceActionLayout.AddButton( m_ConvergenceManualYMinToggle, "" );
    convergenceActionLayout.SetButtonWidth( actionSliderButtonWidth );
    convergenceActionLayout.SetFitWidthFlag( true );
    convergenceActionLayout.AddSlider( m_ConvergenceYMinSlider, "Ymin", 1.0, "%g" );

    convergenceActionLayout.ForceNewLine();

    convergenceActionLayout.SetFitWidthFlag( false );
    convergenceActionLayout.SetButtonWidth( actionToggleButtonWidth );
    convergenceActionLayout.AddButton( m_ConvergenceManualYMaxToggle, "" );
    convergenceActionLayout.SetButtonWidth( actionSliderButtonWidth );
    convergenceActionLayout.SetFitWidthFlag( true );
    convergenceActionLayout.AddSlider( m_ConvergenceYMaxSlider, "Ymax", 1.0, "%g" );
    convergenceActionLayout.InitWidthHeightVals();

    // Plot layout
    int plotWidth = totalWidth - controlWidth - 2 * groupBorderWidth;
    int plotTopBottomMargin = 25;
    int plotSideMargin = 25;
    m_ConvergenceLayout.AddX( controlWidth + 2 * groupBorderWidth );
    m_ConvergenceLayout.AddSubGroupLayout( m_ConvergencePlotLayout, plotWidth, m_ConvergenceLayout.GetH() - 2 * groupBorderWidth );
    m_ConvergencePlotLayout.AddX( plotSideMargin );
    m_ConvergencePlotLayout.AddY( plotTopBottomMargin );
    m_ConvergencePlotCanvas = m_ConvergencePlotLayout.AddCanvas( m_ConvergencePlotLayout.GetW() - 2 * plotSideMargin, m_ConvergencePlotLayout.GetH() - 2 * plotTopBottomMargin,
                              0, 1, 0, 1, //xMin, xMax, yMin, yMax,
                              "", "[X]", "[Y]" );
    m_ConvergencePlotCanvas->align( FL_ALIGN_TOP );
    m_ConvergencePlotCanvas->current_x()->label_format( "%g" );
    m_ConvergencePlotCanvas->current_y()->label_format( "%g" );

    //==== Load Distribution Tab ====//
    m_LoadDistTab = AddTab( "Load Dist." );
    Fl_Group* loadDistPlotGroup = AddSubGroup( m_LoadDistTab, windowBorderWidth );
    m_LoadDistLayout.SetGroupAndScreen( loadDistPlotGroup, this );

    m_LoadDistLayout.AddX( groupBorderWidth );
    m_LoadDistLayout.AddY( groupBorderWidth );

    // Control layout
    m_LoadDistLayout.AddSubGroupLayout( m_LoadDistControlLayout, controlWidth, m_LoadDistLayout.GetH() - 2 * groupBorderWidth );

    // layout the heights of the control layout
    yDataSelectHeight = 8 * rowHeight;

    flowConditionSelectHeight = m_LoadDistLayout.GetH() - 2 * windowBorderWidth - yDataSelectHeight - legendHeight - actionButtonHeight - 6 * m_LoadDistLayout.GetStdHeight();

    GroupLayout yDataSelectLayout;
    m_LoadDistControlLayout.AddSubGroupLayout( yDataSelectLayout, m_LoadDistControlLayout.GetW(), yDataSelectHeight );
    yDataSelectLayout.AddDividerBox( "Y-Data" );
    m_LoadDistYDataBrowser = yDataSelectLayout.AddFlBrowser( yDataSelectLayout.GetRemainY() );
    m_LoadDistYDataBrowser->callback( staticScreenCB, this );
    m_LoadDistYDataBrowser->type( FL_MULTI_BROWSER );
    m_LoadDistControlLayout.AddY( yDataSelectLayout.GetH() + 2 * groupBorderWidth );

    GroupLayout loadChoiceLayout;
    m_LoadDistControlLayout.AddSubGroupLayout( loadChoiceLayout, m_LoadDistControlLayout.GetW(), 2 * m_LoadDistLayout.GetStdHeight() - 2 * groupBorderWidth );
    loadChoiceLayout.AddDividerBox( "Data Type Selection" );

    loadChoiceLayout.SetSameLineFlag( true );
    loadChoiceLayout.SetFitWidthFlag( false );
    loadChoiceLayout.SetButtonWidth( loadChoiceLayout.GetRemainX() / 2 );
    loadChoiceLayout.AddButton( m_LoadTypeToggle, "Load" );
    loadChoiceLayout.AddButton( m_BladeTypeToggle, "Blade" );
    m_LoadDataTypeRadio.AddButton( m_LoadTypeToggle.GetFlButton() );
    m_LoadDataTypeRadio.AddButton( m_BladeTypeToggle.GetFlButton() );
    m_LoadDataTypeRadio.Init( this );
    loadChoiceLayout.ForceNewLine();
    loadChoiceLayout.SetSameLineFlag( false );
    loadChoiceLayout.SetFitWidthFlag( true );

    m_LoadDistControlLayout.AddY( loadChoiceLayout.GetH() + 2 * groupBorderWidth );

    GroupLayout loadDistSelectLayout;
    m_LoadDistControlLayout.AddSubGroupLayout( loadDistSelectLayout, m_LoadDistControlLayout.GetW(), flowConditionSelectHeight - m_LoadDistLayout.GetStdHeight() + 2 * windowBorderWidth );
    loadDistSelectLayout.AddDividerBox( "Group/Rotor Selection" );
    m_LoadDistSelectBrowser = loadDistSelectLayout.AddFlBrowser( loadDistSelectLayout.GetRemainY() );
    m_LoadDistSelectBrowser->callback( staticScreenCB, this );
    m_LoadDistSelectBrowser->type( FL_MULTI_BROWSER );
    m_LoadDistControlLayout.AddY( loadDistSelectLayout.GetH() + 2 * groupBorderWidth );

    GroupLayout flowConditionLayout;
    m_LoadDistControlLayout.AddSubGroupLayout( flowConditionLayout, m_LoadDistControlLayout.GetW(), flowConditionSelectHeight );
    flowConditionLayout.AddDividerBox( "Flow Condition" );
    m_LoadDistFlowConditionBrowser = flowConditionLayout.AddFlBrowser( flowConditionLayout.GetRemainY() );
    m_LoadDistFlowConditionBrowser->callback( staticScreenCB, this );
    m_LoadDistFlowConditionBrowser->type( FL_MULTI_BROWSER );
    m_LoadDistControlLayout.AddY( flowConditionLayout.GetH() + 2 * groupBorderWidth );

    m_LoadDistControlLayout.AddDividerBox( "Legend" );
    m_LoadDistLegendGroup = m_LoadDistControlLayout.AddFlScroll( legendHeight - rowHeight );
    m_LoadDistLegendGroup->type( Fl_Scroll::VERTICAL_ALWAYS );
    m_LoadDistLegendLayout.SetGroupAndScreen( m_LoadDistLegendGroup, this );
    m_LoadDistControlLayout.AddYGap();

    // Action buttons
    GroupLayout loadDistActionLayout;
    m_LoadDistControlLayout.AddSubGroupLayout( loadDistActionLayout, m_LoadDistControlLayout.GetW(), actionButtonHeight );
    loadDistActionLayout.AddDividerBox( "Actions:" );

    loadDistActionLayout.SetSameLineFlag( true );

    loadDistActionLayout.SetFitWidthFlag( false );
    loadDistActionLayout.SetButtonWidth( actionToggleButtonWidth );
    loadDistActionLayout.AddButton( m_LoadDistManualXMinToggle, "" );
    loadDistActionLayout.SetButtonWidth( actionSliderButtonWidth );
    loadDistActionLayout.SetFitWidthFlag( true );
    loadDistActionLayout.AddSlider( m_LoadDistXMinSlider, "Xmin", 1.0, "%g" );

    loadDistActionLayout.ForceNewLine();

    loadDistActionLayout.SetFitWidthFlag( false );
    loadDistActionLayout.SetButtonWidth( actionToggleButtonWidth );
    loadDistActionLayout.AddButton( m_LoadDistManualXMaxToggle, "" );
    loadDistActionLayout.SetButtonWidth( actionSliderButtonWidth );
    loadDistActionLayout.SetFitWidthFlag( true );
    loadDistActionLayout.AddSlider( m_LoadDistXMaxSlider, "Xmax", 1.0, "%g" );

    loadDistActionLayout.ForceNewLine();

    loadDistActionLayout.SetFitWidthFlag( false );
    loadDistActionLayout.SetButtonWidth( actionToggleButtonWidth );
    loadDistActionLayout.AddButton( m_LoadDistManualYMinToggle, "" );
    loadDistActionLayout.SetButtonWidth( actionSliderButtonWidth );
    loadDistActionLayout.SetFitWidthFlag( true );
    loadDistActionLayout.AddSlider( m_LoadDistYMinSlider, "Ymin", 1.0, "%g" );

    loadDistActionLayout.ForceNewLine();

    loadDistActionLayout.SetFitWidthFlag( false );
    loadDistActionLayout.SetButtonWidth( actionToggleButtonWidth );
    loadDistActionLayout.AddButton( m_LoadDistManualYMaxToggle, "" );
    loadDistActionLayout.SetButtonWidth( actionSliderButtonWidth );
    loadDistActionLayout.SetFitWidthFlag( true );
    loadDistActionLayout.AddSlider( m_LoadDistYMaxSlider, "Ymax", 1.0, "%g" );
    loadDistActionLayout.InitWidthHeightVals();

    // Plot layout
    m_LoadDistLayout.AddX( controlWidth + 2 * groupBorderWidth );
    m_LoadDistLayout.AddSubGroupLayout( m_LoadDistPlotLayout, plotWidth, m_LoadDistLayout.GetH() - 2 * groupBorderWidth );
    m_LoadDistPlotLayout.AddX( plotSideMargin );
    m_LoadDistPlotLayout.AddY( plotTopBottomMargin );
    m_LoadDistPlotCanvas = m_LoadDistPlotLayout.AddCanvas( m_LoadDistPlotLayout.GetW() - 2 * plotSideMargin, m_LoadDistPlotLayout.GetH() - 2 * plotTopBottomMargin,
                           0, 1, 0, 1, //xMin, xMax, yMin, yMax,
                           "", "[X]", "[Y]" );
    m_LoadDistPlotCanvas->align( FL_ALIGN_TOP );
    m_LoadDistPlotCanvas->current_x()->label_format( "%g" );
    m_LoadDistPlotCanvas->current_y()->label_format( "%g" );

    //==== Sweep Tab ====//
    m_SweepTab = AddTab( "Sweep" );
    Fl_Group* sweepPlotGroup = AddSubGroup( m_SweepTab, 5 );
    m_SweepLayout.SetGroupAndScreen( sweepPlotGroup, this );

    m_SweepLayout.AddX( groupBorderWidth );
    m_SweepLayout.AddY( groupBorderWidth );

    // Control layout
    m_SweepLayout.AddSubGroupLayout( m_SweepControlLayout, controlWidth, m_SweepLayout.GetH() - 2 * groupBorderWidth );

    GroupLayout sweepXYDataSelectLayout;
    m_SweepControlLayout.AddSubGroupLayout( sweepXYDataSelectLayout, m_SweepControlLayout.GetW(), yDataSelectHeight );
    //  X Data browser
    GroupLayout sweepXDataSelectLayout;
    sweepXYDataSelectLayout.AddSubGroupLayout( sweepXDataSelectLayout, sweepXYDataSelectLayout.GetW() / 2, sweepXYDataSelectLayout.GetH() );
    sweepXDataSelectLayout.AddDividerBox( "X-Data" );
    m_SweepXDataBrowser = sweepXDataSelectLayout.AddFlBrowser( sweepXDataSelectLayout.GetRemainY() );
    m_SweepXDataBrowser->callback( staticScreenCB, this );
    m_SweepXDataBrowser->type( FL_MULTI_BROWSER );
    sweepXYDataSelectLayout.AddX( sweepXDataSelectLayout.GetW() );
    //  Y Data browser
    GroupLayout sweepYDataSelectLayout;
    sweepXYDataSelectLayout.AddSubGroupLayout( sweepYDataSelectLayout, sweepXYDataSelectLayout.GetW() / 2, sweepXYDataSelectLayout.GetH() );
    sweepYDataSelectLayout.AddDividerBox( "Y-Data" );
    m_SweepYDataBrowser = sweepYDataSelectLayout.AddFlBrowser( sweepYDataSelectLayout.GetRemainY() );
    m_SweepYDataBrowser->callback( staticScreenCB, this );
    m_SweepYDataBrowser->type( FL_MULTI_BROWSER );
    m_SweepControlLayout.AddY( sweepYDataSelectLayout.GetH() + 2 * groupBorderWidth );

    GroupLayout sweepFlowConditionLayout;
    m_SweepControlLayout.AddSubGroupLayout( flowConditionLayout, m_SweepControlLayout.GetW(), flowConditionSelectHeight );
    flowConditionLayout.AddDividerBox( "Flow Condition" );
    m_SweepFlowConditionBrowser = flowConditionLayout.AddFlBrowser( flowConditionLayout.GetRemainY() );
    m_SweepFlowConditionBrowser->callback( staticScreenCB, this );
    m_SweepFlowConditionBrowser->type( FL_MULTI_BROWSER );
    m_SweepControlLayout.AddY( flowConditionLayout.GetH() + 2 * groupBorderWidth );

    m_SweepControlLayout.AddDividerBox( "Legend" );
    m_SweepLegendGroup = m_SweepControlLayout.AddFlScroll( legendHeight - rowHeight );
    m_SweepLegendGroup->type( Fl_Scroll::VERTICAL_ALWAYS );
    m_SweepLegendLayout.SetGroupAndScreen( m_SweepLegendGroup, this );
    m_SweepControlLayout.AddYGap();

    // Action buttons
    GroupLayout sweepActionLayout;
    m_SweepControlLayout.AddSubGroupLayout( sweepActionLayout, m_SweepControlLayout.GetW(), actionButtonHeight );
    sweepActionLayout.AddDividerBox( "Actions:" );

    sweepActionLayout.SetSameLineFlag( true );

    sweepActionLayout.SetFitWidthFlag( false );
    sweepActionLayout.SetButtonWidth( actionToggleButtonWidth );
    sweepActionLayout.AddButton( m_SweepManualXMinToggle, "" );
    sweepActionLayout.SetButtonWidth( actionSliderButtonWidth );
    sweepActionLayout.SetFitWidthFlag( true );
    sweepActionLayout.AddSlider( m_SweepXMinSlider, "Xmin", 1.0, "%g" );

    sweepActionLayout.ForceNewLine();

    sweepActionLayout.SetFitWidthFlag( false );
    sweepActionLayout.SetButtonWidth( actionToggleButtonWidth );
    sweepActionLayout.AddButton( m_SweepManualXMaxToggle, "" );
    sweepActionLayout.SetButtonWidth( actionSliderButtonWidth );
    sweepActionLayout.SetFitWidthFlag( true );
    sweepActionLayout.AddSlider( m_SweepXMaxSlider, "Xmax", 1.0, "%g" );

    sweepActionLayout.ForceNewLine();

    sweepActionLayout.SetFitWidthFlag( false );
    sweepActionLayout.SetButtonWidth( actionToggleButtonWidth );
    sweepActionLayout.AddButton( m_SweepManualYMinToggle, "" );
    sweepActionLayout.SetButtonWidth( actionSliderButtonWidth );
    sweepActionLayout.SetFitWidthFlag( true );
    sweepActionLayout.AddSlider( m_SweepYMinSlider, "Ymin", 1.0, "%g" );

    sweepActionLayout.ForceNewLine();

    sweepActionLayout.SetFitWidthFlag( false );
    sweepActionLayout.SetButtonWidth( actionToggleButtonWidth );
    sweepActionLayout.AddButton( m_SweepManualYMaxToggle, "" );
    sweepActionLayout.SetButtonWidth( actionSliderButtonWidth );
    sweepActionLayout.SetFitWidthFlag( true );
    sweepActionLayout.AddSlider( m_SweepYMaxSlider, "Ymax", 1.0, "%g" );
    sweepActionLayout.InitWidthHeightVals();

    // Plot layout
    m_SweepLayout.AddX( controlWidth + 2 * groupBorderWidth );
    m_SweepLayout.AddSubGroupLayout( m_SweepPlotLayout, plotWidth, m_SweepLayout.GetH() - 2 * groupBorderWidth );
    m_SweepPlotLayout.AddX( plotSideMargin );
    m_SweepPlotLayout.AddY( plotTopBottomMargin );
    m_SweepPlotCanvas = m_SweepPlotLayout.AddCanvas( m_SweepPlotLayout.GetW() - 2 * plotSideMargin, m_SweepPlotLayout.GetH() - 2 * plotTopBottomMargin,
                        0, 1, 0, 1, //xMin, xMax, yMin, yMax,
                        "", "[X]", "[Y]" );
    m_SweepPlotCanvas->align( FL_ALIGN_TOP );
    m_SweepPlotCanvas->current_x()->label_format( "%g" );
    m_SweepPlotCanvas->current_y()->label_format( "%g" );

    //==== Cp Slice Tab ====//
    m_CpSliceTab = AddTab( "Cp Slice" );
    Fl_Group* CpSlicePlotGroup = AddSubGroup( m_CpSliceTab, windowBorderWidth );
    m_CpSliceLayout.SetGroupAndScreen( CpSlicePlotGroup, this );

    m_CpSliceLayout.AddX( groupBorderWidth );
    m_CpSliceLayout.AddY( groupBorderWidth );

    // Control layout
    m_CpSliceLayout.AddSubGroupLayout( m_CpSliceControlLayout, controlWidth, m_CpSliceLayout.GetH() - 2 * groupBorderWidth );

    int control_x = m_CpSliceControlLayout.GetX();
    yDataSelectHeight = 8 * rowHeight;
    int PosTypeHeight = 4 * rowHeight;

    GroupLayout CutSelectLayout;
    m_CpSliceControlLayout.AddSubGroupLayout( CutSelectLayout, m_CpSliceControlLayout.GetW(), yDataSelectHeight );
    CutSelectLayout.AddDividerBox( "Slice" );
    m_CpSliceCutBrowser = CutSelectLayout.AddFlBrowser( CutSelectLayout.GetRemainY() );
    m_CpSliceCutBrowser->callback( staticScreenCB, this );
    m_CpSliceCutBrowser->type( FL_MULTI_BROWSER );
    m_CpSliceControlLayout.AddY( CutSelectLayout.GetH() + 2 * groupBorderWidth );

    GroupLayout CutPosChoiceLayout;
    m_CpSliceControlLayout.AddSubGroupLayout( CutPosChoiceLayout, m_CpSliceControlLayout.GetW() , PosTypeHeight );
    CutPosChoiceLayout.AddDividerBox( "X-Axis Plotting" );

    CutPosChoiceLayout.SetChoiceButtonWidth( 3 * CutPosChoiceLayout.GetW() / 4 );

    m_CpSlicePosTypeChoiceVec.resize( 3 );
    m_CpSlicePosTypeChoiceVec[vsp::X_DIR] = &m_XCpSlicePosTypeChoice;
    m_CpSlicePosTypeChoiceVec[vsp::Y_DIR] = &m_YCpSlicePosTypeChoice;
    m_CpSlicePosTypeChoiceVec[vsp::Z_DIR] = &m_ZCpSlicePosTypeChoice;

    for ( size_t i = 0; i < m_CpSlicePosTypeChoiceVec.size(); i++ )
    {
        m_CpSlicePosTypeChoiceVec[i]->AddItem( "X" );
        m_CpSlicePosTypeChoiceVec[i]->AddItem( "Y" );
        m_CpSlicePosTypeChoiceVec[i]->AddItem( "Z" );
    }

    CutPosChoiceLayout.AddChoice( m_XCpSlicePosTypeChoice, "X Cut Position Axis" );
    CutPosChoiceLayout.AddChoice( m_YCpSlicePosTypeChoice, "Y Cut Position Axis" );
    CutPosChoiceLayout.AddChoice( m_ZCpSlicePosTypeChoice, "Z Cut Position Axis" );

    m_XCpSlicePosTypeChoice.UpdateItems();
    m_YCpSlicePosTypeChoice.UpdateItems();
    m_ZCpSlicePosTypeChoice.UpdateItems();

    m_CpSliceControlLayout.AddY( CutPosChoiceLayout.GetH() + 2 * groupBorderWidth );

    GroupLayout flowCaseLayout;
    m_CpSliceControlLayout.AddSubGroupLayout( flowCaseLayout, m_CpSliceControlLayout.GetW(), flowConditionSelectHeight - rowHeight - 2 * groupBorderWidth );
    flowCaseLayout.AddDividerBox( "Flow Condition" );
    m_CpSliceCaseBrowser = flowCaseLayout.AddFlBrowser( flowCaseLayout.GetRemainY() );
    m_CpSliceCaseBrowser->callback( staticScreenCB, this );
    m_CpSliceCaseBrowser->type( FL_MULTI_BROWSER );
    m_CpSliceControlLayout.AddY( flowCaseLayout.GetH() + 2 * groupBorderWidth );

    m_CpSliceControlLayout.AddDividerBox( "Legend" );
    m_CpSliceLegendGroup = m_CpSliceControlLayout.AddFlScroll( legendHeight - rowHeight );
    m_CpSliceLegendGroup->type( Fl_Scroll::VERTICAL_ALWAYS );
    m_CpSliceLegendLayout.SetGroupAndScreen( m_CpSliceLegendGroup, this );
    m_CpSliceControlLayout.AddYGap();

    // Action buttons
    GroupLayout CpSliceActionLayout;
    m_CpSliceControlLayout.AddSubGroupLayout( CpSliceActionLayout, m_CpSliceControlLayout.GetW(), actionButtonHeight );
    CpSliceActionLayout.AddDividerBox( "Actions:" );

    CpSliceActionLayout.SetSameLineFlag( true );

    CpSliceActionLayout.SetFitWidthFlag( false );
    CpSliceActionLayout.SetButtonWidth( actionToggleButtonWidth );
    CpSliceActionLayout.AddButton( m_CpSliceManualXMinToggle, "" );
    CpSliceActionLayout.SetButtonWidth( actionSliderButtonWidth );
    CpSliceActionLayout.SetFitWidthFlag( true );
    CpSliceActionLayout.AddSlider( m_CpSliceXMinSlider, "Xmin", 1.0, "%g" );

    CpSliceActionLayout.ForceNewLine();

    CpSliceActionLayout.SetFitWidthFlag( false );
    CpSliceActionLayout.SetButtonWidth( actionToggleButtonWidth );
    CpSliceActionLayout.AddButton( m_CpSliceManualXMaxToggle, "" );
    CpSliceActionLayout.SetButtonWidth( actionSliderButtonWidth );
    CpSliceActionLayout.SetFitWidthFlag( true );
    CpSliceActionLayout.AddSlider( m_CpSliceXMaxSlider, "Xmax", 1.0, "%g" );

    CpSliceActionLayout.ForceNewLine();

    CpSliceActionLayout.SetFitWidthFlag( false );
    CpSliceActionLayout.SetButtonWidth( actionToggleButtonWidth );
    CpSliceActionLayout.AddButton( m_CpSliceManualYMinToggle, "" );
    CpSliceActionLayout.SetButtonWidth( actionSliderButtonWidth );
    CpSliceActionLayout.SetFitWidthFlag( true );
    CpSliceActionLayout.AddSlider( m_CpSliceYMinSlider, "Ymin", 1.0, "%g" );

    CpSliceActionLayout.ForceNewLine();

    CpSliceActionLayout.SetFitWidthFlag( false );
    CpSliceActionLayout.SetButtonWidth( actionToggleButtonWidth );
    CpSliceActionLayout.AddButton( m_CpSliceManualYMaxToggle, "" );
    CpSliceActionLayout.SetButtonWidth( actionSliderButtonWidth );
    CpSliceActionLayout.SetFitWidthFlag( true );
    CpSliceActionLayout.AddSlider( m_CpSliceYMaxSlider, "Ymax", 1.0, "%g" );
    CpSliceActionLayout.InitWidthHeightVals();

    CpSliceActionLayout.SetFitWidthFlag( false );
    CpSliceActionLayout.ForceNewLine();
    CpSliceActionLayout.SetButtonWidth( CpSliceActionLayout.GetW() / 2 );

    CpSliceActionLayout.AddButton( m_CpSliceFlipYToggle, "Flip Y Axis" );
    CpSliceActionLayout.AddButton( m_CpSlicePlotLinesToggle, "Plot Lines" );

    // Plot layout
    m_CpSliceLayout.AddX( controlWidth + 2 * groupBorderWidth );
    m_CpSliceLayout.AddSubGroupLayout( m_CpSlicePlotLayout, plotWidth, m_CpSliceLayout.GetH() - 2 * groupBorderWidth );
    m_CpSlicePlotLayout.AddX( plotSideMargin );
    m_CpSlicePlotLayout.AddY( plotTopBottomMargin );
    m_CpSlicePlotCanvas = m_CpSlicePlotLayout.AddCanvas( m_CpSlicePlotLayout.GetW() - 2 * plotSideMargin, m_CpSlicePlotLayout.GetH() - 2 * plotTopBottomMargin,
                                                           0, 1, 0, 1, //xMin, xMax, yMin, yMax,
                                                           "", "[X]", "[Y]" );
    m_CpSlicePlotCanvas->align( FL_ALIGN_TOP );
    m_CpSlicePlotCanvas->current_x()->label_format( "%g" );
    m_CpSlicePlotCanvas->current_y()->label_format( "%g" );

    //==== Unsteady Tab ====//
    m_UnsteadyTab = AddTab( "Unsteady" );
    Fl_Group* unsteadyPlotGroup = AddSubGroup( m_UnsteadyTab, windowBorderWidth );
    m_UnsteadyLayout.SetGroupAndScreen( unsteadyPlotGroup, this );

    m_UnsteadyLayout.AddX( groupBorderWidth );
    m_UnsteadyLayout.AddY( groupBorderWidth );

    // Control layout
    m_UnsteadyLayout.AddSubGroupLayout( m_UnsteadyControlLayout, controlWidth, m_UnsteadyLayout.GetH() - windowBorderWidth );

    // layout the heights of the control layout
    yDataSelectHeight = 8 * rowHeight;

    flowConditionSelectHeight = m_UnsteadyLayout.GetH() - 2 * windowBorderWidth - yDataSelectHeight - legendHeight - actionButtonHeight - 6 * m_UnsteadyLayout.GetStdHeight();

    GroupLayout unsteadyYDataSelectLayout;
    m_UnsteadyControlLayout.AddSubGroupLayout( unsteadyYDataSelectLayout, m_UnsteadyControlLayout.GetW(), yDataSelectHeight );
    unsteadyYDataSelectLayout.AddDividerBox( "Y-Data" );
    m_UnsteadyYDataBrowser = unsteadyYDataSelectLayout.AddFlBrowser( unsteadyYDataSelectLayout.GetRemainY() );
    m_UnsteadyYDataBrowser->callback( staticScreenCB, this );
    m_UnsteadyYDataBrowser->type( FL_MULTI_BROWSER );
    m_UnsteadyControlLayout.AddY( unsteadyYDataSelectLayout.GetH() + 2 * groupBorderWidth );

    GroupLayout unsteadyGroupChoiceLayout;
    m_UnsteadyControlLayout.AddSubGroupLayout( unsteadyGroupChoiceLayout, m_UnsteadyControlLayout.GetW(), 2 * m_UnsteadyControlLayout.GetStdHeight() - 2 * groupBorderWidth );
    unsteadyGroupChoiceLayout.AddDividerBox( "Data Type Selection" );

    unsteadyGroupChoiceLayout.SetSameLineFlag( true );
    unsteadyGroupChoiceLayout.SetFitWidthFlag( false );
    unsteadyGroupChoiceLayout.SetButtonWidth( unsteadyGroupChoiceLayout.GetRemainX() / 3 );
    unsteadyGroupChoiceLayout.AddButton( m_UnsteadyHistoryToggle, "History" );
    unsteadyGroupChoiceLayout.AddButton( m_UnsteadyGroupToggle, "Group" );
    unsteadyGroupChoiceLayout.AddButton( m_UnsteadyRotorToggle, "Rotor" );
    m_UnsteadyDataTypeRadio.AddButton( m_UnsteadyHistoryToggle.GetFlButton() );
    m_UnsteadyDataTypeRadio.AddButton( m_UnsteadyGroupToggle.GetFlButton() );
    m_UnsteadyDataTypeRadio.AddButton( m_UnsteadyRotorToggle.GetFlButton() );
    m_UnsteadyDataTypeRadio.Init( this );
    unsteadyGroupChoiceLayout.ForceNewLine();
    unsteadyGroupChoiceLayout.SetSameLineFlag( false );
    unsteadyGroupChoiceLayout.SetFitWidthFlag( true );

    m_UnsteadyControlLayout.AddY( unsteadyGroupChoiceLayout.GetH() + 2 * groupBorderWidth );

    GroupLayout unsteadySelectLayout;
    m_UnsteadyControlLayout.AddSubGroupLayout( unsteadySelectLayout, m_UnsteadyControlLayout.GetW(), flowConditionSelectHeight - m_UnsteadyLayout.GetStdHeight() + 2 * windowBorderWidth );
    unsteadySelectLayout.AddDividerBox( "Group/Rotor Selection" );
    m_UnsteadySelectBrowser = unsteadySelectLayout.AddFlBrowser( unsteadySelectLayout.GetRemainY() );
    m_UnsteadySelectBrowser->callback( staticScreenCB, this );
    m_UnsteadySelectBrowser->type( FL_MULTI_BROWSER );
    m_UnsteadyControlLayout.AddY( unsteadySelectLayout.GetH() + 2 * groupBorderWidth );

    GroupLayout unsteadyFlowConditionLayout;
    m_UnsteadyControlLayout.AddSubGroupLayout( unsteadyFlowConditionLayout, m_UnsteadyControlLayout.GetW(), flowConditionSelectHeight );
    unsteadyFlowConditionLayout.AddDividerBox( "Flow Condition" );
    m_UnsteadyFlowConditionBrowser = unsteadyFlowConditionLayout.AddFlBrowser( unsteadyFlowConditionLayout.GetRemainY() );
    m_UnsteadyFlowConditionBrowser->callback( staticScreenCB, this );
    m_UnsteadyFlowConditionBrowser->type( FL_MULTI_BROWSER );
    m_UnsteadyControlLayout.AddY( unsteadyFlowConditionLayout.GetH() + 2 * groupBorderWidth );

    m_UnsteadyControlLayout.AddDividerBox( "Legend" );
    m_UnsteadyLegendGroup = m_UnsteadyControlLayout.AddFlScroll( legendHeight - rowHeight );
    m_UnsteadyLegendGroup->type( Fl_Scroll::VERTICAL_ALWAYS );
    m_UnsteadyLegendLayout.SetGroupAndScreen( m_UnsteadyLegendGroup, this );
    m_UnsteadyControlLayout.AddYGap();

    // Action buttons
    GroupLayout unsteadyActionLayout;
    m_UnsteadyControlLayout.AddSubGroupLayout( unsteadyActionLayout, m_UnsteadyControlLayout.GetW(), actionButtonHeight );
    unsteadyActionLayout.AddDividerBox( "Actions:" );

    unsteadyActionLayout.SetSameLineFlag( true );

    unsteadyActionLayout.SetFitWidthFlag( false );
    unsteadyActionLayout.SetButtonWidth( actionToggleButtonWidth );
    unsteadyActionLayout.AddButton( m_UnsteadyManualXMinToggle, "" );
    unsteadyActionLayout.SetButtonWidth( actionSliderButtonWidth );
    unsteadyActionLayout.SetFitWidthFlag( true );
    unsteadyActionLayout.AddSlider( m_UnsteadyXMinSlider, "Xmin", 1.0, "%g" );

    unsteadyActionLayout.ForceNewLine();

    unsteadyActionLayout.SetFitWidthFlag( false );
    unsteadyActionLayout.SetButtonWidth( actionToggleButtonWidth );
    unsteadyActionLayout.AddButton( m_UnsteadyManualXMaxToggle, "" );
    unsteadyActionLayout.SetButtonWidth( actionSliderButtonWidth );
    unsteadyActionLayout.SetFitWidthFlag( true );
    unsteadyActionLayout.AddSlider( m_UnsteadyXMaxSlider, "Xmax", 1.0, "%g" );

    unsteadyActionLayout.ForceNewLine();

    unsteadyActionLayout.SetFitWidthFlag( false );
    unsteadyActionLayout.SetButtonWidth( actionToggleButtonWidth );
    unsteadyActionLayout.AddButton( m_UnsteadyManualYMinToggle, "" );
    unsteadyActionLayout.SetButtonWidth( actionSliderButtonWidth );
    unsteadyActionLayout.SetFitWidthFlag( true );
    unsteadyActionLayout.AddSlider( m_UnsteadyYMinSlider, "Ymin", 1.0, "%g" );

    unsteadyActionLayout.ForceNewLine();

    unsteadyActionLayout.SetFitWidthFlag( false );
    unsteadyActionLayout.SetButtonWidth( actionToggleButtonWidth );
    unsteadyActionLayout.AddButton( m_UnsteadyManualYMaxToggle, "" );
    unsteadyActionLayout.SetButtonWidth( actionSliderButtonWidth );
    unsteadyActionLayout.SetFitWidthFlag( true );
    unsteadyActionLayout.AddSlider( m_UnsteadyYMaxSlider, "Ymax", 1.0, "%g" );
    unsteadyActionLayout.InitWidthHeightVals();

    // Plot layout
    m_UnsteadyLayout.AddX( controlWidth + 2 * groupBorderWidth );
    m_UnsteadyLayout.AddSubGroupLayout( m_UnsteadyPlotLayout, plotWidth, m_ConvergenceLayout.GetH() - 2 * groupBorderWidth );
    m_UnsteadyPlotLayout.AddX( plotSideMargin );
    m_UnsteadyPlotLayout.AddY( plotTopBottomMargin );
    m_UnsteadyPlotCanvas = m_UnsteadyPlotLayout.AddCanvas( m_UnsteadyPlotLayout.GetW() - 2 * plotSideMargin, m_UnsteadyPlotLayout.GetH() - 2 * plotTopBottomMargin,
                                                                 0, 1, 0, 1, //xMin, xMax, yMin, yMax,
                                                                 "", "[X]", "[Y]" );
    m_UnsteadyPlotCanvas->align( FL_ALIGN_TOP );
    m_UnsteadyPlotCanvas->current_x()->label_format( "%g" );
    m_UnsteadyPlotCanvas->current_y()->label_format( "%g" );

    SetDefaultView();

    m_PercentLegendColor = 0.8;
}

VSPAEROPlotScreen::~VSPAEROPlotScreen()
{
}

void VSPAEROPlotScreen::SetDefaultView()
{
    string resultName = "VSPAERO_History";
    Results* res = ResultsMgr.FindResults( resultName, 0 );
    int analysis_type = -1;
    if ( res )
    {
        analysis_type = res->FindPtr( "AnalysisMethod" )->GetInt( 0 );
    }

    resultName = "VSPAERO_Stab";
    res = ResultsMgr.FindResults( resultName, 0 );
    int stab_type = -1;
    if ( res )
    {
        stab_type = res->FindPtr( "StabilityType" )->GetInt( 0 );
    }

    resultName = "VSPAERO_Group";
    int num_group = ResultsMgr.GetNumResults( resultName );

    // TODO: Identify default view from results, not parms
    if ( num_group > 0 || stab_type >= vsp::STABILITY_P_ANALYSIS )
    {
        m_UnsteadyTab->show();

        if ( num_group > 0 )
        {
            VSPAEROMgr.m_UnsteadyGroupSelectType.Set( VSPAEROMgr.ROTOR_SELECT_TYPE );
        }
        else
        {
            VSPAEROMgr.m_UnsteadyGroupSelectType.Set( VSPAEROMgr.HISTORY_SELECT_TYPE );
        }
    }
    else
    {
        m_LoadDistTab->show();
    }

    m_SelectDefaultData = true;
}

bool VSPAEROPlotScreen::Update()
{
    string resultName = "VSPAERO_Stab";
    Results* res = ResultsMgr.FindResults( resultName, 0 );
    bool stabFlag = false;
    int stabType = -1;
    if ( res )
    {
        stabFlag = true;
        stabType = res->FindPtr( "StabilityType" )->GetInt( 0 );
    }

    resultName = "VSPAERO_Group";
    int num_group = ResultsMgr.GetNumResults( resultName );

    resultName = "VSPAERO_Rotor";
    int num_rotor = ResultsMgr.GetNumResults( resultName );

    if ( num_group > 0 || num_rotor > 0 )
    {
        stabFlag = true;
    }

    if ( !stabFlag || ( stabFlag && stabType == vsp::STABILITY_DEFAULT ) )
    {
        m_UnsteadyTab->deactivate();
        m_ConvergenceTab->activate();

        // Update single plot canvas
        UpdateConvergenceFlowConditionBrowser();
        UpdateConvergenceYDataBrowser();
        RedrawConvergencePlot();
        UpdateConvergenceAutoManualAxisLimits();

        m_UnsteadyFlowConditionBrowser->clear();
        m_UnsteadySelectBrowser->clear();
        m_UnsteadyYDataBrowser->clear();
        m_UnsteadyPlotCanvas->clear();
        m_UnsteadyLegendGroup->clear();
    }
    else // P, Q, or R unsteady analysis
    {
        m_ConvergenceTab->deactivate();
        m_ConvergenceFlowConditionBrowser->clear();
        m_ConvergenceYDataBrowser->clear();
        m_ConvergencePlotCanvas->clear();
        m_ConvergenceLegendGroup->clear();

        m_UnsteadyTab->activate();

        m_UnsteadyDataTypeRadio.Update( VSPAEROMgr.m_UnsteadyGroupSelectType.GetID() );

        if ( num_group == 0 )
        {
            VSPAEROMgr.m_UnsteadyGroupSelectType.Set( VSPAEROMgr.HISTORY_SELECT_TYPE );
            m_UnsteadyDataTypeRadio.Deactivate();
        }
        else
        {
            m_UnsteadyDataTypeRadio.Activate();
        }

        // Update unsteady plot canvas
        UpdateUnsteadyFlowConditionBrowser();
        UpdateUnsteadyYDataBrowser();
        UpdateUnsteadySelectionBrowser();
        RedrawUnsteadyPlot();
        UpdateUnsteadyAutoManualAxisLimits();
    }

    // Update load distribution
    // Let's check to see what analysis method was used on the first result found
    // note that only VSPAEROMgr clear all VSPAERO_* results from the results manager each time it's run all analyses in the results 'should' have the same analysis method
    resultName = "VSPAERO_Load";
    res = ResultsMgr.FindResults( resultName, 0 );
    if ( res )
    {
        m_LoadDistTab->activate();

        m_LoadDataTypeRadio.Update( VSPAEROMgr.m_LoadDistSelectType.GetID() );

        if ( num_rotor == 0 )
        {
            VSPAEROMgr.m_LoadDistSelectType.Set( VSPAEROMgr.LOAD_SELECT_TYPE );
            m_LoadDataTypeRadio.Deactivate();
        }
        else
        {
            m_LoadDataTypeRadio.Activate();
        }

        UpdateLoadDistFlowConditionBrowser();
        UpdateLoadDistYDataBrowser();
        UpdateLoadDistSelectionBrowser();
        RedrawLoadDistPlot();
        UpdateLoadDistAutoManualAxisLimits();
    }
    else
    {
        m_LoadDistTab->deactivate();
        m_LoadDistFlowConditionBrowser->clear();
        m_LoadDistYDataBrowser->clear();
        m_LoadDistPlotCanvas->clear();
        m_LoadDistLegendGroup->clear();
    }

    // Update sweep condition plot canvas
    UpdateSweepFlowConditionBrowser();
    UpdateSweepXYDataBrowser();
    RedrawSweepPlot();
    UpdateSweepAutoManualAxisLimits();

    resultName = "CpSlice_Wrapper";
    res = ResultsMgr.FindResults( resultName, 0 );
    if ( res )
    {
        m_CpSliceTab->activate();
        UpdateCpSliceCutBrowser();
        UpdateCpSliceCaseBrowser();
        RedrawCpSlicePlot();
        UpdateCpSliceAutoManualAxisLimits();
    }
    else
    {
        m_CpSliceTab->deactivate();
        m_CpSliceCutBrowser->clear();
        m_CpSliceCaseBrowser->clear();
        m_CpSlicePlotCanvas->clear();
        m_CpSliceLegendGroup->clear();
    }

    m_FLTK_Window->redraw();

    //Check to see if SelectCpSliceBrowser in VSPAERO screen is occupied with slices
    //if not, we deactivate the CP Slice tab in the VSPAERO results Manager
     if ( VSPAEROMgr.GetCpSliceVec().size() < 1 )
     {
         m_CpSliceTab->deactivate();
     }
     else
     {
         m_CpSliceTab->activate();
     }

    return false;

}

void VSPAEROPlotScreen::UpdateConvergenceAutoManualAxisLimits()
{
    Ca_Axis_ *  t_Axis;

    m_ConvergenceManualXMinToggle.Update( VSPAEROMgr.m_ConvergenceXMinIsManual.GetID() );
    m_ConvergenceManualXMaxToggle.Update( VSPAEROMgr.m_ConvergenceXMaxIsManual.GetID() );
    m_ConvergenceXMinSlider.Update( VSPAEROMgr.m_ConvergenceXMin.GetID() );
    m_ConvergenceXMaxSlider.Update( VSPAEROMgr.m_ConvergenceXMax.GetID() );
    t_Axis = m_ConvergencePlotCanvas->current_x();
    if( t_Axis )
    {
        // Minimum
        if( VSPAEROMgr.m_ConvergenceXMinIsManual() )
        {
            // MANUAL
            m_ConvergenceXMinSlider.Activate();
            t_Axis->minimum( VSPAEROMgr.m_ConvergenceXMin.Get() );
        }
        else
        {
            // AUTO
            m_ConvergenceXMinSlider.Deactivate();
            VSPAEROMgr.m_ConvergenceXMin = t_Axis->minimum();
        }
        // Maximum
        if( VSPAEROMgr.m_ConvergenceXMaxIsManual() )
        {
            // MANUAL
            m_ConvergenceXMaxSlider.Activate();
            t_Axis->maximum( VSPAEROMgr.m_ConvergenceXMax.Get() );
        }
        else
        {
            // AUTO
            m_ConvergenceXMaxSlider.Deactivate();
            VSPAEROMgr.m_ConvergenceXMax = t_Axis->maximum();
        }
    }
    m_ConvergenceManualYMinToggle.Update( VSPAEROMgr.m_ConvergenceYMinIsManual.GetID() );
    m_ConvergenceManualYMaxToggle.Update( VSPAEROMgr.m_ConvergenceYMaxIsManual.GetID() );
    m_ConvergenceYMinSlider.Update( VSPAEROMgr.m_ConvergenceYMin.GetID() );
    m_ConvergenceYMaxSlider.Update( VSPAEROMgr.m_ConvergenceYMax.GetID() );
    t_Axis = m_ConvergencePlotCanvas->current_y();
    if( t_Axis )
    {
        // Minimum
        if( VSPAEROMgr.m_ConvergenceYMinIsManual() )
        {
            // MANUAL
            m_ConvergenceYMinSlider.Activate();
            t_Axis->minimum( VSPAEROMgr.m_ConvergenceYMin.Get() );
        }
        else
        {
            // AUTO
            m_ConvergenceYMinSlider.Deactivate();
            VSPAEROMgr.m_ConvergenceYMin = t_Axis->minimum();
        }
        // Maximum
        if( VSPAEROMgr.m_ConvergenceYMaxIsManual() )
        {
            // MANUAL
            m_ConvergenceYMaxSlider.Activate();
            t_Axis->maximum( VSPAEROMgr.m_ConvergenceYMax.Get() );
        }
        else
        {
            // AUTO
            m_ConvergenceYMaxSlider.Deactivate();
            VSPAEROMgr.m_ConvergenceYMax = t_Axis->maximum();
        }
    }
}

void VSPAEROPlotScreen::UpdateLoadDistAutoManualAxisLimits()
{
    Ca_Axis_ *  t_Axis;

    m_LoadDistManualXMinToggle.Update( VSPAEROMgr.m_LoadDistXMinIsManual.GetID() );
    m_LoadDistManualXMaxToggle.Update( VSPAEROMgr.m_LoadDistXMaxIsManual.GetID() );
    m_LoadDistXMinSlider.Update( VSPAEROMgr.m_LoadDistXMin.GetID() );
    m_LoadDistXMaxSlider.Update( VSPAEROMgr.m_LoadDistXMax.GetID() );
    t_Axis = m_LoadDistPlotCanvas->current_x();
    if( t_Axis )
    {
        // Minimum
        if( VSPAEROMgr.m_LoadDistXMinIsManual() )
        {
            // MANUAL
            m_LoadDistXMinSlider.Activate();
            t_Axis->minimum( VSPAEROMgr.m_LoadDistXMin.Get() );
        }
        else
        {
            // AUTO
            m_LoadDistXMinSlider.Deactivate();
            VSPAEROMgr.m_LoadDistXMin = t_Axis->minimum();
        }
        // Maximum
        if( VSPAEROMgr.m_LoadDistXMaxIsManual() )
        {
            // MANUAL
            m_LoadDistXMaxSlider.Activate();
            t_Axis->maximum( VSPAEROMgr.m_LoadDistXMax.Get() );
        }
        else
        {
            // AUTO
            m_LoadDistXMaxSlider.Deactivate();
            VSPAEROMgr.m_LoadDistXMax = t_Axis->maximum();
        }
    }
    m_LoadDistManualYMinToggle.Update( VSPAEROMgr.m_LoadDistYMinIsManual.GetID() );
    m_LoadDistManualYMaxToggle.Update( VSPAEROMgr.m_LoadDistYMaxIsManual.GetID() );
    m_LoadDistYMinSlider.Update( VSPAEROMgr.m_LoadDistYMin.GetID() );
    m_LoadDistYMaxSlider.Update( VSPAEROMgr.m_LoadDistYMax.GetID() );
    t_Axis = m_LoadDistPlotCanvas->current_y();
    if( t_Axis )
    {
        // Minimum
        if( VSPAEROMgr.m_LoadDistYMinIsManual() )
        {
            // MANUAL
            m_LoadDistYMinSlider.Activate();
            t_Axis->minimum( VSPAEROMgr.m_LoadDistYMin.Get() );
        }
        else
        {
            // AUTO
            m_LoadDistYMinSlider.Deactivate();
            VSPAEROMgr.m_LoadDistYMin = t_Axis->minimum();
        }
        // Maximum
        if( VSPAEROMgr.m_LoadDistYMaxIsManual() )
        {
            // MANUAL
            m_LoadDistYMaxSlider.Activate();
            t_Axis->maximum( VSPAEROMgr.m_LoadDistYMax.Get() );
        }
        else
        {
            // AUTO
            m_LoadDistYMaxSlider.Deactivate();
            VSPAEROMgr.m_LoadDistYMax = t_Axis->maximum();
        }
    }
}

void VSPAEROPlotScreen::UpdateSweepAutoManualAxisLimits()
{
    Ca_Axis_ *  t_Axis;

    m_SweepManualXMinToggle.Update( VSPAEROMgr.m_SweepXMinIsManual.GetID() );
    m_SweepManualXMaxToggle.Update( VSPAEROMgr.m_SweepXMaxIsManual.GetID() );
    m_SweepXMinSlider.Update( VSPAEROMgr.m_SweepXMin.GetID() );
    m_SweepXMaxSlider.Update( VSPAEROMgr.m_SweepXMax.GetID() );
    t_Axis = m_SweepPlotCanvas->current_x();
    if( t_Axis )
    {
        // Minimum
        if( VSPAEROMgr.m_SweepXMinIsManual() )
        {
            // MANUAL
            m_SweepXMinSlider.Activate();
            t_Axis->minimum( VSPAEROMgr.m_SweepXMin.Get() );
        }
        else
        {
            // AUTO
            m_SweepXMinSlider.Deactivate();
            VSPAEROMgr.m_SweepXMin = t_Axis->minimum();
        }
        // Maximum
        if( VSPAEROMgr.m_SweepXMaxIsManual() )
        {
            // MANUAL
            m_SweepXMaxSlider.Activate();
            t_Axis->maximum( VSPAEROMgr.m_SweepXMax.Get() );
        }
        else
        {
            // AUTO
            m_SweepXMaxSlider.Deactivate();
            VSPAEROMgr.m_SweepXMax = t_Axis->maximum();
        }
    }
    m_SweepManualYMinToggle.Update( VSPAEROMgr.m_SweepYMinIsManual.GetID() );
    m_SweepManualYMaxToggle.Update( VSPAEROMgr.m_SweepYMaxIsManual.GetID() );
    m_SweepYMinSlider.Update( VSPAEROMgr.m_SweepYMin.GetID() );
    m_SweepYMaxSlider.Update( VSPAEROMgr.m_SweepYMax.GetID() );
    t_Axis = m_SweepPlotCanvas->current_y();
    if( t_Axis )
    {
        // Minimum
        if( VSPAEROMgr.m_SweepYMinIsManual() )
        {
            // MANUAL
            m_SweepYMinSlider.Activate();
            t_Axis->minimum( VSPAEROMgr.m_SweepYMin.Get() );
        }
        else
        {
            // AUTO
            m_SweepYMinSlider.Deactivate();
            VSPAEROMgr.m_SweepYMin = t_Axis->minimum();
        }
        // Maximum
        if( VSPAEROMgr.m_SweepYMaxIsManual() )
        {
            // MANUAL
            m_SweepYMaxSlider.Activate();
            t_Axis->maximum( VSPAEROMgr.m_SweepYMax.Get() );
        }
        else
        {
            // AUTO
            m_SweepYMaxSlider.Deactivate();
            VSPAEROMgr.m_SweepYMax = t_Axis->maximum();
        }
    }



}

void VSPAEROPlotScreen::UpdateCpSliceAutoManualAxisLimits()
{
    Ca_Axis_ *  t_Axis;

    m_CpSliceManualXMinToggle.Update( VSPAEROMgr.m_CpSliceXMinIsManual.GetID() );
    m_CpSliceManualXMaxToggle.Update( VSPAEROMgr.m_CpSliceXMaxIsManual.GetID() );
    m_CpSliceXMinSlider.Update( VSPAEROMgr.m_CpSliceXMin.GetID() );
    m_CpSliceXMaxSlider.Update( VSPAEROMgr.m_CpSliceXMax.GetID() );
    t_Axis = m_CpSlicePlotCanvas->current_x();
    if ( t_Axis )
    {
        // Minimum
        if ( VSPAEROMgr.m_CpSliceXMinIsManual() )
        {
            // MANUAL
            m_CpSliceXMinSlider.Activate();
            t_Axis->minimum( VSPAEROMgr.m_CpSliceXMin.Get() );
        }
        else
        {
            // AUTO
            m_CpSliceXMinSlider.Deactivate();
            VSPAEROMgr.m_CpSliceXMin = t_Axis->minimum();
        }
        // Maximum
        if ( VSPAEROMgr.m_CpSliceXMaxIsManual() )
        {
            // MANUAL
            m_CpSliceXMaxSlider.Activate();
            t_Axis->maximum( VSPAEROMgr.m_CpSliceXMax.Get() );
        }
        else
        {
            // AUTO
            m_CpSliceXMaxSlider.Deactivate();
            VSPAEROMgr.m_CpSliceXMax = t_Axis->maximum();
        }
    }
    m_CpSliceManualYMinToggle.Update( VSPAEROMgr.m_CpSliceYMinIsManual.GetID() );
    m_CpSliceManualYMaxToggle.Update( VSPAEROMgr.m_CpSliceYMaxIsManual.GetID() );
    m_CpSliceYMinSlider.Update( VSPAEROMgr.m_CpSliceYMin.GetID() );
    m_CpSliceYMaxSlider.Update( VSPAEROMgr.m_CpSliceYMax.GetID() );
    t_Axis = m_CpSlicePlotCanvas->current_y();
    if ( t_Axis )
    {
        // Minimum
        if ( VSPAEROMgr.m_CpSliceYMinIsManual() )
        {
            // MANUAL
            m_CpSliceYMinSlider.Activate();
            t_Axis->minimum( VSPAEROMgr.m_CpSliceYMin.Get() );
        }
        else
        {
            // AUTO
            m_CpSliceYMinSlider.Deactivate();
            VSPAEROMgr.m_CpSliceYMin = t_Axis->minimum();
        }
        // Maximum
        if ( VSPAEROMgr.m_CpSliceYMaxIsManual() )
        {
            // MANUAL
            m_CpSliceYMaxSlider.Activate();
            t_Axis->maximum( VSPAEROMgr.m_CpSliceYMax.Get() );
        }
        else
        {
            // AUTO
            m_CpSliceYMaxSlider.Deactivate();
            VSPAEROMgr.m_CpSliceYMax = t_Axis->maximum();
        }
    }

    // Update Flip Y Axis
    m_CpSliceFlipYToggle.Update( VSPAEROMgr.m_CpSliceYAxisFlipFlag.GetID() );
    m_CpSlicePlotLinesToggle.Update( VSPAEROMgr.m_CpSlicePlotLinesFlag.GetID() );
}

void VSPAEROPlotScreen::UpdateUnsteadyAutoManualAxisLimits()
{
    Ca_Axis_ *  t_Axis;

    m_UnsteadyManualXMinToggle.Update( VSPAEROMgr.m_UnsteadyXMinIsManual.GetID() );
    m_UnsteadyManualXMaxToggle.Update( VSPAEROMgr.m_UnsteadyXMaxIsManual.GetID() );
    m_UnsteadyXMinSlider.Update( VSPAEROMgr.m_UnsteadyXMin.GetID() );
    m_UnsteadyXMaxSlider.Update( VSPAEROMgr.m_UnsteadyXMax.GetID() );
    t_Axis = m_UnsteadyPlotCanvas->current_x();
    if ( t_Axis )
    {
        // Minimum
        if ( VSPAEROMgr.m_UnsteadyXMinIsManual() )
        {
            // MANUAL
            m_UnsteadyXMinSlider.Activate();
            t_Axis->minimum( VSPAEROMgr.m_UnsteadyXMin.Get() );
        }
        else
        {
            // AUTO
            m_UnsteadyXMinSlider.Deactivate();
            VSPAEROMgr.m_UnsteadyXMin = t_Axis->minimum();
        }
        // Maximum
        if ( VSPAEROMgr.m_UnsteadyXMaxIsManual() )
        {
            // MANUAL
            m_UnsteadyXMaxSlider.Activate();
            t_Axis->maximum( VSPAEROMgr.m_UnsteadyXMax.Get() );
        }
        else
        {
            // AUTO
            m_UnsteadyXMaxSlider.Deactivate();
            VSPAEROMgr.m_UnsteadyXMax = t_Axis->maximum();
        }
    }
    m_UnsteadyManualYMinToggle.Update( VSPAEROMgr.m_UnsteadyYMinIsManual.GetID() );
    m_UnsteadyManualYMaxToggle.Update( VSPAEROMgr.m_UnsteadyYMaxIsManual.GetID() );
    m_UnsteadyYMinSlider.Update( VSPAEROMgr.m_UnsteadyYMin.GetID() );
    m_UnsteadyYMaxSlider.Update( VSPAEROMgr.m_UnsteadyYMax.GetID() );
    t_Axis = m_UnsteadyPlotCanvas->current_y();
    if ( t_Axis )
    {
        // Minimum
        if ( VSPAEROMgr.m_UnsteadyYMinIsManual() )
        {
            // MANUAL
            m_UnsteadyYMinSlider.Activate();
            t_Axis->minimum( VSPAEROMgr.m_UnsteadyYMin.Get() );
        }
        else
        {
            // AUTO
            m_UnsteadyYMinSlider.Deactivate();
            VSPAEROMgr.m_UnsteadyYMin = t_Axis->minimum();
        }
        // Maximum
        if ( VSPAEROMgr.m_UnsteadyYMaxIsManual() )
        {
            // MANUAL
            m_UnsteadyYMaxSlider.Activate();
            t_Axis->maximum( VSPAEROMgr.m_UnsteadyYMax.Get() );
        }
        else
        {
            // AUTO
            m_UnsteadyYMaxSlider.Deactivate();
            VSPAEROMgr.m_UnsteadyYMax = t_Axis->maximum();
        }
    }
}

void VSPAEROPlotScreen::Show()
{
    m_ScreenMgr->SetUpdateFlag( true );
    m_FLTK_Window->show();

}

void VSPAEROPlotScreen::Hide()
{
    m_FLTK_Window->hide();
    m_ScreenMgr->SetUpdateFlag( true );
}

//==== Callbacks ====//
void VSPAEROPlotScreen::CallBack( Fl_Widget* w )
{
    m_SelectDefaultData = false;

    // Handle multi-select of sweep browsers
    if ( w == m_SweepXDataBrowser )
    {
        // count the number of selected elements
        //if more than 1 set the y browser selection to 1
        int countSelected = 0;
        for ( unsigned int iCase = 1; iCase <= m_SweepXDataBrowser->size(); iCase++ )
        {
            if( m_SweepXDataBrowser->selected( iCase ) )
            {
                countSelected++;
            }
        }
        if ( countSelected > 1 )
        {
            bool oneSelected = false;
            for ( unsigned int iCase = 1; iCase <= m_SweepYDataBrowser->size(); iCase++ )
            {
                if ( m_SweepYDataBrowser->selected( iCase ) && !oneSelected )
                {
                    oneSelected = true;
                }
                else
                {
                    m_SweepYDataBrowser->select( iCase, 0 ); //deselect
                }
            }
        }
    }
    if ( w == m_SweepYDataBrowser )
    {
        // count the number of selected elements
        //if more than 1 set the y browser selection to 1
        int countSelected = 0;
        for ( unsigned int iCase = 1; iCase <= m_SweepYDataBrowser->size(); iCase++ )
        {
            if( m_SweepYDataBrowser->selected( iCase ) )
            {
                countSelected++;
            }
        }
        if ( countSelected > 1 )
        {
            bool oneSelected = false;
            for ( unsigned int iCase = 1; iCase <= m_SweepXDataBrowser->size(); iCase++ )
            {
                if ( m_SweepXDataBrowser->selected( iCase ) && !oneSelected )
                {
                    oneSelected = true;
                }
                else
                {
                    m_SweepXDataBrowser->select( iCase, 0 ); //deselect
                }
            }
        }
    }
    m_ScreenMgr->SetUpdateFlag( true );
}

void VSPAEROPlotScreen::CloseCallBack( Fl_Widget* w )
{
    Hide();
}

void VSPAEROPlotScreen::GuiDeviceCallBack( GuiDevice* device )
{
    assert( m_ScreenMgr );

    m_SelectDefaultData = false;

    m_ScreenMgr->SetUpdateFlag( true );
}


void VSPAEROPlotScreen::UpdateConvergenceFlowConditionBrowser()
{
    // keeps track of the previously selected rows (browser uses 1-based indexing)
    vector<bool> wasSelected;
    for ( unsigned int iCase = 1; iCase <= m_ConvergenceFlowConditionBrowser->size(); iCase++ )
    {
        wasSelected.push_back( m_ConvergenceFlowConditionBrowser->selected( iCase ) );
    }

    int scrollPos = m_ConvergenceFlowConditionBrowser->position();
    m_ConvergenceFlowConditionBrowser->clear();
    m_ConvergenceFlowConditionSelectedResultIDs.clear();

    string resultName = "VSPAERO_History";

    int numCases = ResultsMgr.GetNumResults( resultName );

    for ( unsigned int iCase = 0; iCase < numCases; iCase++ )
    {
        Results* res = ResultsMgr.FindResults( resultName, iCase );
        if( res )
        {
            char strbuf[1024];
            ConstructFlowConditionString( strbuf, res, false, true );
            m_ConvergenceFlowConditionBrowser->add( strbuf );
            if( m_SelectDefaultData )   //select ALL flow conditions
            {
                m_ConvergenceFlowConditionSelectedResultIDs.push_back( res->GetID() );
                m_ConvergenceFlowConditionBrowser->select( iCase + 1 ); //account for browser using 1-based indexing
            }
            else if ( iCase < wasSelected.size() ) // restore original row selections
            {
                if ( wasSelected[iCase] )
                {
                    m_ConvergenceFlowConditionSelectedResultIDs.push_back( res->GetID() );
                    m_ConvergenceFlowConditionBrowser->select( iCase + 1 ); //account for browser using 1-based indexing
                }
            }
        }   //if( res )
    }   //for (unsigned int iCase=0; iCase<numCases; iCase++)

    m_ConvergenceFlowConditionBrowser->position( scrollPos );

}

void VSPAEROPlotScreen::UpdateLoadDistFlowConditionBrowser()
{
    // keeps track of the previously selected rows (browser uses 1-based indexing)
    vector<bool> wasSelected;
    for ( unsigned int iCase = 1; iCase <= m_LoadDistFlowConditionBrowser->size(); iCase++ )
    {
        wasSelected.push_back( m_LoadDistFlowConditionBrowser->selected( iCase ) );
    }

    int scrollPos = m_LoadDistFlowConditionBrowser->position();
    m_LoadDistFlowConditionBrowser->clear();
    m_LoadDistFlowConditionSelectedResultIDs.clear();

    string resultName = "VSPAERO_Load";

    int numCases = ResultsMgr.GetNumResults( resultName );
    for ( unsigned int iCase = 0; iCase < numCases; iCase++ )
    {
        Results* res = ResultsMgr.FindResults( resultName, iCase );
        if( res )
        {
            char strbuf[1024];
            ConstructFlowConditionString( strbuf, res, false, false );
            m_LoadDistFlowConditionBrowser->add( strbuf );
            if ( VSPAEROMgr.m_LoadDistSelectType.Get() == VSPAEROMgr.LOAD_SELECT_TYPE )
            {
                if ( m_SelectDefaultData )   //select ALL flow conditions
                {
                    m_LoadDistFlowConditionSelectedResultIDs.push_back( res->GetID() );
                    m_LoadDistFlowConditionBrowser->select( iCase + 1 ); //account for browser using 1-based indexing
                }
                else if ( iCase < wasSelected.size() && wasSelected[iCase] ) // restore original row selections
                {
                    m_LoadDistFlowConditionSelectedResultIDs.push_back( res->GetID() );
                    m_LoadDistFlowConditionBrowser->select( iCase + 1 ); //account for browser using 1-based indexing
                }
            }
            else if ( VSPAEROMgr.m_LoadDistSelectType.Get() == VSPAEROMgr.BLADE_SELECT_TYPE )
            {
                m_LoadDistFlowConditionSelectedResultIDs.push_back( res->GetID() );
                m_LoadDistFlowConditionBrowser->select( iCase + 1 );
                break; // Only list the first flow condition, since the others are ignored in the group and rotor output files
            }
        }   //if( res )
    }   //for (unsigned int iCase=0; iCase<numCases; iCase++)

    m_LoadDistFlowConditionBrowser->position( scrollPos );
}

void VSPAEROPlotScreen::UpdateLoadDistSelectionBrowser()
{
    // keeps track of the previously selected rows (browser uses 1-based indexing)
    vector<bool> wasSelected;
    for ( unsigned int iCase = 1; iCase <= m_LoadDistSelectBrowser->size(); iCase++ )
    {
        wasSelected.push_back( m_LoadDistSelectBrowser->selected( iCase ) );
    }

    int scrollPos = m_LoadDistSelectBrowser->position();
    m_LoadDistSelectBrowser->clear();
    m_LoadSelectedBladeVec.clear();
    int num_case = 0;
    string prefix;
    string res_name;

    if ( VSPAEROMgr.m_LoadDistSelectType.Get() == VSPAEROMgrSingleton::LOAD_SELECT_TYPE )
    {
        m_LoadDistSelectBrowser->deactivate();
        m_LoadDistFlowConditionBrowser->activate();
    }
    else if ( VSPAEROMgr.m_LoadDistSelectType.Get() == VSPAEROMgrSingleton::BLADE_SELECT_TYPE )
    {
        res_name = "VSPAERO_Blade_Avg";
        num_case = ResultsMgr.GetNumResults( res_name );
        prefix = "Blade_";
        m_LoadDistSelectBrowser->activate();
        m_LoadDistFlowConditionBrowser->deactivate();
    }

    for ( unsigned int iCase = 0; iCase < num_case; iCase++ )
    {
        Results* res = ResultsMgr.FindResults( res_name, iCase );
        if ( res )
        {
            string name;
            NameValData* nvd = res->FindPtr( "Group_Name" );
            if ( nvd )
            {
                name = nvd->GetString( 0 );

                if ( VSPAEROMgr.m_LoadDistSelectType.Get() == VSPAEROMgrSingleton::BLADE_SELECT_TYPE && res->FindPtr( "Blade_Num" ) )
                {
                    name += ( "_Blade_" + to_string( res->FindPtr( "Blade_Num" )->GetInt( 0 ) ) );
                }
            }
            else
            {
                name = prefix + to_string( iCase );
            }

            m_LoadDistSelectBrowser->add( name.c_str() );

            if ( m_SelectDefaultData )   //select ALL flow conditions
            {
                m_LoadDistSelectBrowser->select( iCase + 1 ); //account for browser using 1-based indexing
                m_LoadSelectedBladeVec.push_back( iCase );
            }
            else if ( iCase < wasSelected.size() ) // restore original row selections
            {
                if ( wasSelected[iCase] )
                {
                    m_LoadDistSelectBrowser->select( iCase + 1 ); //account for browser using 1-based indexing
                    m_LoadSelectedBladeVec.push_back( iCase );
                }
            }
        }   //if( res )
    }   //for (unsigned int iCase=0; iCase<numCases; iCase++)

    m_LoadDistSelectBrowser->position( scrollPos );
}

void VSPAEROPlotScreen::UpdateSweepFlowConditionBrowser()
{
    // keeps track of the previously selected rows (browser uses 1-based indexing)
    vector<bool> wasSelected;
    for ( unsigned int iCase = 1; iCase <= m_SweepFlowConditionBrowser->size(); iCase++ )
    {
        wasSelected.push_back( m_SweepFlowConditionBrowser->selected( iCase ) );
    }

    int scrollPos = m_SweepFlowConditionBrowser->position();
    m_SweepFlowConditionBrowser->clear();
    m_SweepFlowConditionSelectedResultIDs.clear();

    string resultName = "VSPAERO_History";

    int numCases = ResultsMgr.GetNumResults( resultName );
    for ( unsigned int iCase = 0; iCase < numCases; iCase++ )
    {
        Results* res = ResultsMgr.FindResults( resultName, iCase );
        if( res )
        {
            char strbuf[1024];
            ConstructFlowConditionString( strbuf, res, false, true );
            m_SweepFlowConditionBrowser->add( strbuf );
            if( m_SelectDefaultData )   //select ALL flow conditions
            {
                m_SweepFlowConditionSelectedResultIDs.push_back( res->GetID() );
                m_SweepFlowConditionBrowser->select( iCase + 1 ); //account for browser using 1-based indexing
            }
            else if ( iCase < wasSelected.size() ) // restore original row selections
            {
                if ( wasSelected[iCase] )
                {
                    m_SweepFlowConditionSelectedResultIDs.push_back( res->GetID() );
                    m_SweepFlowConditionBrowser->select( iCase + 1 ); //account for browser using 1-based indexing
                }
            }
        }   //if( res )
    }   //for (unsigned int iCase=0; iCase<numCases; iCase++)

    m_SweepFlowConditionBrowser->position( scrollPos );
}

void VSPAEROPlotScreen::UpdateCpSliceCaseBrowser()
{
    // keeps track of the previously selected rows (browser uses 1-based indexing)
    vector<bool> wasSelected;
    for ( unsigned int iCase = 1; iCase <= m_CpSliceCaseBrowser->size(); iCase++ )
    {
        wasSelected.push_back( m_CpSliceCaseBrowser->selected( iCase ) );
    }

    int scrollPos = m_CpSliceCaseBrowser->position();
    m_CpSliceCaseBrowser->clear();
    m_CpSliceCaseSelectedResultIDs.clear();

    string resultName = "CpSlicer_Case";
    int case_num = 1;

    for ( unsigned int iCase = 0; iCase < m_NumCpCases; iCase++ )
    {
        Results* res = ResultsMgr.FindResults( resultName, iCase * m_NumCpCuts );
        if ( res )
        {
            char strbuf[1024];
            ConstructCpSliceCaseString( strbuf, res, case_num );
            case_num++;
            m_CpSliceCaseBrowser->add( strbuf );
            if ( m_SelectDefaultData )   //select ALL flow conditions
            {
                // Include results of all CpSlice flow condition cases for given cut
                for ( size_t j = 0; j < m_NumCpCuts; j++ )
                {
                    Results* slice_res = ResultsMgr.FindResults( resultName, iCase * m_NumCpCuts + j );
                    if ( slice_res )
                    {
                        m_CpSliceCaseSelectedResultIDs.push_back( slice_res->GetID() );
                    }
                }

                m_CpSliceCaseBrowser->select( iCase + 1 ); //account for browser using 1-based indexing
            }
            else if ( iCase < wasSelected.size() ) // restore original row selections
            {
                if ( wasSelected[iCase] )
                {
                    // Include results of all CpSlice flow condition cases for given cut
                    for ( size_t j = 0; j < m_NumCpCuts; j++ )
                    {
                        Results* slice_res = ResultsMgr.FindResults( resultName, iCase * m_NumCpCuts + j );
                        if ( slice_res )
                        {
                            m_CpSliceCaseSelectedResultIDs.push_back( slice_res->GetID() );
                        }
                    }

                    m_CpSliceCaseBrowser->select( iCase + 1 ); //account for browser using 1-based indexing
                }
            }
        }
    }

    m_CpSliceCaseBrowser->position( scrollPos );
}

void VSPAEROPlotScreen::UpdateUnsteadyFlowConditionBrowser()
{
    // keeps track of the previously selected rows (browser uses 1-based indexing)
    vector<bool> wasSelected;
    for ( unsigned int iCase = 1; iCase <= m_UnsteadyFlowConditionBrowser->size(); iCase++ )
    {
        wasSelected.push_back( m_UnsteadyFlowConditionBrowser->selected( iCase ) );
    }

    int scrollPos = m_UnsteadyFlowConditionBrowser->position();
    m_UnsteadyFlowConditionBrowser->clear();
    m_UnsteadyFlowConditionSelectedResultIDs.clear();

    string resultName = "VSPAERO_History";

    int numCases = ResultsMgr.GetNumResults( resultName );

    for ( unsigned int iCase = 0; iCase < numCases; iCase++ )
    {
        Results* res = ResultsMgr.FindResults( resultName, iCase );
        if ( res )
        {
            char strbuf[1024];
            ConstructFlowConditionString( strbuf, res, false, true );
            m_UnsteadyFlowConditionBrowser->add( strbuf );
            if ( VSPAEROMgr.m_UnsteadyGroupSelectType.Get() == VSPAEROMgr.HISTORY_SELECT_TYPE )
            {
                if ( m_SelectDefaultData )   //select ALL flow conditions
                {
                    m_UnsteadyFlowConditionSelectedResultIDs.push_back( res->GetID() );
                    m_UnsteadyFlowConditionBrowser->select( iCase + 1 ); //account for browser using 1-based indexing
                }
                else if ( iCase < wasSelected.size() ) // restore original row selections
                {
                    if ( wasSelected[iCase] )
                    {
                        m_UnsteadyFlowConditionSelectedResultIDs.push_back( res->GetID() );
                        m_UnsteadyFlowConditionBrowser->select( iCase + 1 ); //account for browser using 1-based indexing
                    }
                }
            }
            else
            {
                m_UnsteadyFlowConditionSelectedResultIDs.push_back( res->GetID() );
                m_UnsteadyFlowConditionBrowser->select( iCase + 1 );
                break; // Only list the first flow condition, since the others are ignored in the group and rotor output files
            }
        }   //if( res )
    }   //for (unsigned int iCase=0; iCase<numCases; iCase++)

    m_UnsteadyFlowConditionBrowser->position( scrollPos );

}

void VSPAEROPlotScreen::UpdateUnsteadySelectionBrowser()
{
    // keeps track of the previously selected rows (browser uses 1-based indexing)
    vector<bool> wasSelected;
    for ( unsigned int iCase = 1; iCase <= m_UnsteadySelectBrowser->size(); iCase++ )
    {
        wasSelected.push_back( m_UnsteadySelectBrowser->selected( iCase ) );
    }

    int scrollPos = m_UnsteadySelectBrowser->position();
    m_UnsteadySelectBrowser->clear();
    m_UnsteadySelectedTypeVec.clear();
    int num_case = 0;
    string prefix;
    string res_name;

    if ( VSPAEROMgr.m_UnsteadyGroupSelectType.Get() == VSPAEROMgrSingleton::GROUP_SELECT_TYPE )
    {
        res_name = "VSPAERO_Group";
        num_case = ResultsMgr.GetNumResults( "VSPAERO_Group" );
        prefix = "Group_";
        m_UnsteadySelectBrowser->activate();
        m_UnsteadyFlowConditionBrowser->deactivate();
    }
    else if ( VSPAEROMgr.m_UnsteadyGroupSelectType.Get() == VSPAEROMgrSingleton::ROTOR_SELECT_TYPE )
    {
        res_name = "VSPAERO_Rotor";
        num_case = ResultsMgr.GetNumResults( res_name );
        prefix = "Rotor_";
        m_UnsteadySelectBrowser->activate();
        m_UnsteadyFlowConditionBrowser->deactivate();
    }
    else if ( VSPAEROMgr.m_UnsteadyGroupSelectType.Get() == VSPAEROMgrSingleton::HISTORY_SELECT_TYPE )
    {
        m_UnsteadySelectBrowser->deactivate();
        m_UnsteadyFlowConditionBrowser->activate();
    }

    for ( unsigned int iCase = 0; iCase < num_case; iCase++ )
    {
        Results* res = ResultsMgr.FindResults( res_name, iCase );
        if ( res )
        {
            string name;
            NameValData* nvd = res->FindPtr( "Group_Name" );
            if ( nvd )
            {
                name = nvd->GetString( 0 );
            }
            else
            {
                name = prefix + to_string( iCase );
            }

            m_UnsteadySelectBrowser->add( name.c_str() );

            if ( m_SelectDefaultData )   //select ALL flow conditions
            {
                m_UnsteadySelectBrowser->select( iCase + 1 ); //account for browser using 1-based indexing
                m_UnsteadySelectedTypeVec.push_back( iCase );
            }
            else if ( iCase < wasSelected.size() ) // restore original row selections
            {
                if ( wasSelected[iCase] )
                {
                    m_UnsteadySelectBrowser->select( iCase + 1 ); //account for browser using 1-based indexing
                    m_UnsteadySelectedTypeVec.push_back( iCase );
                }
            }
        }   //if( res )
    }   //for (unsigned int iCase=0; iCase<numCases; iCase++)

    m_UnsteadySelectBrowser->position( scrollPos );
}

void VSPAEROPlotScreen::ConstructFlowConditionString( char * strbuf, Results * res, bool includeResultId, bool include_recref )
{
    if( strbuf && res )
    {
        NameValData* nvd;
        vector <double> dataVector;

        double alpha = 0;
        double beta = 0;
        double mach = 0;
        double recref = 0;

        nvd = res->FindPtr( "FC_AoA_" );
        if( nvd )
        {
            dataVector = nvd->GetDoubleData();
            alpha = dataVector[dataVector.size() - 1];
        }

        nvd = res->FindPtr( "FC_Beta_" );
        if( nvd )
        {
            dataVector = nvd->GetDoubleData();
            beta = dataVector[dataVector.size() - 1];
        }

        nvd = res->FindPtr( "FC_Mach_" );
        if( nvd )
        {
            dataVector = nvd->GetDoubleData();
            mach = dataVector[dataVector.size() - 1];
        }

        nvd = res->FindPtr( "FC_ReCref_" );
        if ( nvd )
        {
            dataVector = nvd->GetDoubleData();
            recref = dataVector[dataVector.size() - 1];

            if ( includeResultId )
            {
                sprintf( strbuf, "a=%.2g, b=%.2g, M=%.2g, Re=%.2g, resID=%s", alpha, beta, mach, recref, res->GetID().c_str() );
            }
            else
            {
                sprintf( strbuf, "a=%.2g, b=%.2g, M=%.2g, Re=%.2g", alpha, beta, mach, recref );
            }
        }
        else
        {
            if ( includeResultId )
            {
                sprintf( strbuf, "a=%.2g, b=%.2g, M=%.2g, resID=%s", alpha, beta, mach, res->GetID().c_str() );
            }
            else
            {
                sprintf( strbuf, "a=%.2g, b=%.2g, M=%.2g", alpha, beta, mach );
            }
        }
    }
}

void VSPAEROPlotScreen::ConstructCpSliceCaseString( char* strbuf, Results* res, int case_num )
{
    if ( strbuf && res )
    {
        NameValData* nvd;
        vector <double> dataVector;

        double alpha = 0;
        double beta = 0;
        double mach = 0;

        nvd = res->FindPtr( "Alpha" );
        if ( nvd )
        {
            dataVector = nvd->GetDoubleData();
            alpha = dataVector[dataVector.size() - 1];
        }

        nvd = res->FindPtr( "Beta" );
        if ( nvd )
        {
            dataVector = nvd->GetDoubleData();
            beta = dataVector[dataVector.size() - 1];
        }

        nvd = res->FindPtr( "Mach" );
        if ( nvd )
        {
            dataVector = nvd->GetDoubleData();
            mach = dataVector[dataVector.size() - 1];
        }

        sprintf( strbuf, "Case %d: a=%.2g, b=%.2g, M=%.2g", case_num, alpha, beta, mach );
    }
}

void VSPAEROPlotScreen::UpdateConvergenceYDataBrowser()
{
    // keeps track of the previously selected rows (browser uses 1-based indexing)
    vector<bool> wasSelected;
    for ( unsigned int iCase = 1; iCase <= m_ConvergenceYDataBrowser->size(); iCase++ )
    {
        wasSelected.push_back( m_ConvergenceYDataBrowser->selected( iCase ) );
    }

    int scrollPos = m_ConvergenceYDataBrowser->position();
    m_ConvergenceYDataBrowser->clear();

    string resultName = "VSPAERO_History";
    string resultID = ResultsMgr.FindLatestResultsID( resultName );
    vector < string > dataNames = ResultsMgr.GetAllDataNames( resultID );
    for ( unsigned int iDataName = 0; iDataName < dataNames.size(); iDataName++ )
    {
        if ( ( strncmp( dataNames[iDataName].c_str(), "FC_", 3 )   != 0 )  &&
                ( strcmp( dataNames[iDataName].c_str(), "WakeIter" ) != 0 )  &&
                ( strcmp( dataNames[iDataName].c_str(), "Mach" )     != 0 )  &&
                ( strcmp( dataNames[iDataName].c_str(), "Alpha" )    != 0 )  &&
                ( strcmp( dataNames[iDataName].c_str(), "Beta" )     != 0 )  &&
                ( strcmp( dataNames[iDataName].c_str(), "AnalysisMethod" ) != 0 ) )
        {
            m_ConvergenceYDataBrowser->add( dataNames[iDataName].c_str() );
        }
    }

    // restore original row selections
    for ( unsigned int iCase = 0; iCase < m_ConvergenceYDataBrowser->size(); iCase++ )
    {
        if( ( m_SelectDefaultData && strcmp( m_ConvergenceYDataBrowser->text( iCase + 1 ), "L/D" ) == 0 ) )
        {
            m_ConvergenceYDataBrowser->select( iCase + 1 ); //account for browser using 1-based indexing
        }
        if ( iCase < wasSelected.size() )
        {
            if ( wasSelected[iCase] )
            {
                m_ConvergenceYDataBrowser->select( iCase + 1 ); //account for browser using 1-based indexing
            }
        }
    }

    m_ConvergenceYDataBrowser->position( scrollPos );
}

void VSPAEROPlotScreen::UpdateLoadDistYDataBrowser()
{
    // keeps track of the previously selected rows (browser uses 1-based indexing)
    vector<bool> wasSelected;
    for ( unsigned int iCase = 1; iCase <= m_LoadDistYDataBrowser->size(); iCase++ )
    {
        wasSelected.push_back( m_LoadDistYDataBrowser->selected( iCase ) );
    }

    int scrollPos = m_LoadDistYDataBrowser->position();
    m_LoadDistYDataBrowser->clear();

    Results* load_res = ResultsMgr.FindResultsPtr( ResultsMgr.FindLatestResultsID( "VSPAERO_Load" ) );
    Results* res = 0;

    string resultID, default_res;

    if ( VSPAEROMgr.m_LoadDistSelectType.Get() == VSPAEROMgrSingleton::LOAD_SELECT_TYPE )
    {
        resultID = ResultsMgr.FindLatestResultsID( "VSPAERO_Load" );
        res = load_res;
        default_res = "cl*c/cref";
    }
    else if ( VSPAEROMgr.m_LoadDistSelectType.Get() == VSPAEROMgrSingleton::BLADE_SELECT_TYPE )
    {
        resultID = ResultsMgr.FindLatestResultsID( "VSPAERO_Blade_Avg" );
        res = ResultsMgr.FindResultsPtr( resultID );
        default_res = "CT_H";
    }

    if( res && load_res )
    {
        vector < string > dataNames = ResultsMgr.GetAllDataNames( resultID );
        for ( unsigned int iDataName = 0; iDataName < dataNames.size(); iDataName++ )
        {
            if ( ( strncmp( dataNames[iDataName].c_str(), "FC_", 3 )   != 0 )  &&
                    ( strcmp( dataNames[iDataName].c_str(), "FC_Alpha" ) != 0 )  &&
                    ( strcmp( dataNames[iDataName].c_str(), "FC_Beta" )  != 0 )  &&
                    ( strcmp( dataNames[iDataName].c_str(), "WingId" )   != 0 )  &&
                    ( strcmp( dataNames[iDataName].c_str(), "AnalysisMethod" ) != 0 ) &&
                    ( strcmp( dataNames[iDataName].c_str(), "Station" ) != 0 ) &&
                    ( strcmp( dataNames[iDataName].c_str(), "Group_Name" ) != 0 ) &&
                    ( strcmp( dataNames[iDataName].c_str(), "Rotor_Num" ) != 0 ) &&
                    ( strcmp( dataNames[iDataName].c_str(), "Blade_Num" ) != 0 ) )
            {
                m_LoadDistYDataBrowser->add( dataNames[iDataName].c_str() );
            }
        }
    }

    // restore original row selections
    for ( unsigned int iCase = 0; iCase < m_LoadDistYDataBrowser->size(); iCase++ )
    {
        if( ( m_SelectDefaultData && strcmp( m_LoadDistYDataBrowser->text( iCase + 1 ), default_res.c_str() ) == 0 ) )
        {
            m_LoadDistYDataBrowser->select( iCase + 1 ); //account for browser using 1-based indexing
        }
        if ( iCase < wasSelected.size() )
        {
            if ( wasSelected[iCase] )
            {
                m_LoadDistYDataBrowser->select( iCase + 1 ); //account for browser using 1-based indexing
            }
        }
    }

    m_LoadDistYDataBrowser->position( scrollPos );

}

void VSPAEROPlotScreen::UpdateSweepXYDataBrowser()
{
    // keeps track of the previously selected rows (browser uses 1-based indexing)
    vector<bool> wasSelectedX;
    for ( unsigned int iCase = 1; iCase <= m_SweepXDataBrowser->size(); iCase++ )
    {
        wasSelectedX.push_back( m_SweepXDataBrowser->selected( iCase ) );
    }
    vector<bool> wasSelectedY;
    for ( unsigned int iCase = 1; iCase <= m_SweepYDataBrowser->size(); iCase++ )
    {
        wasSelectedY.push_back( m_SweepYDataBrowser->selected( iCase ) );
    }

    int scrollPosXData = m_SweepXDataBrowser->position();
    m_SweepXDataBrowser->clear();

    int scrollPosYData = m_SweepYDataBrowser->position();
    m_SweepYDataBrowser->clear();

    string resultName = "VSPAERO_History";
    string resultID = ResultsMgr.FindLatestResultsID( resultName );
    vector < string > dataNames = ResultsMgr.GetAllDataNames( resultID );
    if ( strcmp( resultName.c_str(), "VSPAERO_History" ) == 0 )
    {
        for ( unsigned int iDataName = 0; iDataName < dataNames.size(); iDataName++ )
        {
            if ( ( strncmp( dataNames[iDataName].c_str(), "FC_", 3 )   != 0 )  &&
                    ( strcmp( dataNames[iDataName].c_str(), "WakeIter" ) != 0 )  &&
                    ( strcmp( dataNames[iDataName].c_str(), "AnalysisMethod" ) != 0 ) )
            {
                m_SweepXDataBrowser->add( dataNames[iDataName].c_str() );
                m_SweepYDataBrowser->add( dataNames[iDataName].c_str() );
            }
        }
    }

    // restore original row selections
    for ( unsigned int iCase = 0; iCase < m_SweepXDataBrowser->size(); iCase++ )
    {
        if( ( m_SelectDefaultData && strcmp( m_SweepXDataBrowser->text( iCase + 1 ), "CDtot" ) == 0 ) )
        {
            m_SweepXDataBrowser->select( iCase + 1 ); //account for browser using 1-based indexing
        }
        if ( iCase < wasSelectedX.size() )
        {
            if ( wasSelectedX[iCase] )
            {
                m_SweepXDataBrowser->select( iCase + 1 ); //account for browser using 1-based indexing
            }
        }
    }
    for ( unsigned int iCase = 0; iCase < m_SweepYDataBrowser->size(); iCase++ )
    {
        if( ( m_SelectDefaultData && strcmp( m_SweepYDataBrowser->text( iCase + 1 ), "CL" ) == 0 ) )
        {
            m_SweepYDataBrowser->select( iCase + 1 ); //account for browser using 1-based indexing
        }
        if ( iCase < wasSelectedY.size() )
        {
            if ( wasSelectedY[iCase] )
            {
                m_SweepYDataBrowser->select( iCase + 1 ); //account for browser using 1-based indexing
            }
        }
    }

    m_SweepXDataBrowser->position( scrollPosXData );
    m_SweepYDataBrowser->position( scrollPosYData );
}

void VSPAEROPlotScreen::UpdateCpSliceCutBrowser()
{
    // keeps track of the previously selected rows (browser uses 1-based indexing)
    vector<bool> wasSelected;
    for ( unsigned int iCase = 1; iCase <= m_CpSliceCutBrowser->size(); iCase++ )
    {
        wasSelected.push_back( m_CpSliceCutBrowser->selected( iCase ) );
    }

    int scrollPos = m_CpSliceCutBrowser->position();
    m_CpSliceCutBrowser->clear();
    m_CpSliceCutSelectedResultIDs.clear();
    m_CpSliceCutResultIDMap.clear();

    string wrapper_res_id = ResultsMgr.FindLatestResultsID( "CpSlice_Wrapper" );
    Results* wrapper_res = ResultsMgr.FindResultsPtr( wrapper_res_id );
    m_NumCpCuts = 0;

    if ( wrapper_res )
    {
        NameValData* nvd = wrapper_res->FindPtr( "Num_Cuts" );
        if ( nvd )
        {
            vector < int > dataVector = nvd->GetIntData();
            m_NumCpCuts = dataVector[dataVector.size() - 1];
        }
    }

    string resultName = "CpSlicer_Case";

    if ( m_NumCpCuts > 0 )
    {
        m_NumCpCases = ResultsMgr.GetNumResults( resultName ) / m_NumCpCuts;
    }
    else
    {
        m_NumCpCases = 0;
    }

    m_XCpSlicePosTypeChoice.Deactivate();
    m_YCpSlicePosTypeChoice.Deactivate();
    m_ZCpSlicePosTypeChoice.Deactivate();

    for ( unsigned int iCut = 0; iCut < m_NumCpCuts; iCut++ )
    {
        Results* res = ResultsMgr.FindResults( resultName, iCut );
        if ( res )
        {
            NameValData* nvd;
            vector < double > doubledata;
            vector < int > intdata;

            int type = 0;
            double location = 0;
            int cut_num = 0;

            nvd = res->FindPtr( "Cut_Type" );
            if ( nvd )
            {
                intdata = nvd->GetIntData();
                type = intdata[intdata.size() - 1];
            }

            nvd = res->FindPtr( "Cut_Loc" );
            if ( nvd )
            {
                doubledata = nvd->GetDoubleData();
                location = doubledata[doubledata.size() - 1];
            }

            nvd = res->FindPtr( "Cut_Num" );
            if ( nvd )
            {
                intdata = nvd->GetIntData();
                cut_num = intdata[intdata.size() - 1];
            }

            char type_char = 88 + type; // ASCII X: 88; Y: 89; Z: 90

            m_CpSlicePosTypeChoiceVec[type]->Activate();

            char strbuf[1024];
            sprintf( strbuf, "Cut %d: %c= %4.2f", cut_num, type_char, location );

            m_CpSliceCutBrowser->add( strbuf );

            if ( m_SelectDefaultData )   //select aLL flow conditions
            {
                // Include results of all CpSlice cuts for given flow condition case
                for ( size_t j = 0; j < m_NumCpCases; j++ )
                {
                    Results* slice_res = ResultsMgr.FindResults( resultName, iCut + ( j * m_NumCpCuts ) );
                    if ( slice_res )
                    {
                        m_CpSliceCutSelectedResultIDs.push_back( slice_res->GetID() );
                    }
                }

                m_CpSliceCutBrowser->select( iCut + 1 ); //account for browser using 1-based indexing
            }
            else if ( iCut < wasSelected.size() ) // restore original row selections
            {
                if ( wasSelected[iCut] )
                {
                    // Include results of all CpSlice cuts for given flow condition case
                    for ( size_t j = 0; j < m_NumCpCases; j++ )
                    {
                        Results* slice_res = ResultsMgr.FindResults( resultName, iCut + ( j * m_NumCpCuts ) );
                        if ( slice_res )
                        {
                            m_CpSliceCutSelectedResultIDs.push_back( slice_res->GetID() );
                        }
                    }

                    m_CpSliceCutBrowser->select( iCut + 1 ); //account for browser using 1-based indexing
                }
            }

            // Map each result ID to cut index
            for ( size_t j = 0; j < m_NumCpCases; j++ )
            {
                Results* slice_res = ResultsMgr.FindResults( resultName, iCut + ( j * m_NumCpCuts ) );
                if ( slice_res )
                {
                    m_CpSliceCutResultIDMap[iCut].push_back( slice_res->GetID() );
                }
            }
        }
    }

    m_CpSliceCutBrowser->position( scrollPos );
}

void VSPAEROPlotScreen::UpdateUnsteadyYDataBrowser()
{
    // keeps track of the previously selected rows (browser uses 1-based indexing)
    vector<bool> wasSelected;
    for ( unsigned int iCase = 1; iCase <= m_UnsteadyYDataBrowser->size(); iCase++ )
    {
        wasSelected.push_back( m_UnsteadyYDataBrowser->selected( iCase ) );
    }

    int scrollPos = m_UnsteadyYDataBrowser->position();
    m_UnsteadyYDataBrowser->clear();

    string default_res;
    string resultName;
    int res_ind = 0;

    if ( VSPAEROMgr.m_UnsteadyGroupSelectType.Get() == VSPAEROMgrSingleton::GROUP_SELECT_TYPE )
    {
        resultName = "VSPAERO_Group";
        // TODO: Get feedback of best default. Options: Cx, Cy, Cz, Cxo, Cyo,
        //  Czo, Cxi, Cyi, Czi, Cmx, Cmy , Cmz, EtaP, Cmxo, Cmxo, CP_H, Cmzo, 
        //  Cmxi, Cmyi, Cmzi, CL, CD, CLo, CDo, CLi, CDi 
        default_res = "CL";
    }
    else if ( VSPAEROMgr.m_UnsteadyGroupSelectType.Get() == VSPAEROMgrSingleton::ROTOR_SELECT_TYPE )
    {
        resultName = "VSPAERO_Rotor";
        // TODO: Get feedback of best default. Options: Diameter, RPM, Thrust, Thrusto, Thrusti,
        //  Moment, Momento, Momenti, J, CT, CQ , CP, EtaP, CT_H, CQ_H, CP_H, FOM, Angle 
        default_res = "Thrust";
    }
    else if ( VSPAEROMgr.m_UnsteadyGroupSelectType.Get() == VSPAEROMgrSingleton::HISTORY_SELECT_TYPE )
    {
        resultName = "VSPAERO_History";
        res_ind = 0;

        Results* res = ResultsMgr.FindResults( "VSPAERO_Stab", 0 );
        int stabType = 0;
        if ( res )
        {
            stabType = res->FindPtr( "StabilityType" )->GetInt( 0 );
        }

        if ( stabType == vsp::STABILITY_P_ANALYSIS )
        {
            default_res = "CMx";
        }
        else if ( stabType == vsp::STABILITY_Q_ANALYSIS )
        {
            default_res = "CMy";
        }
        else if ( stabType == vsp::STABILITY_R_ANALYSIS )
        {
            default_res = "CMz";
        }
        else
        {
            default_res = "CL"; // TODO: Get feedback of best default
        }
    }

    string resultID = ResultsMgr.FindLatestResultsID( resultName );
    vector < string > dataNames = ResultsMgr.GetAllDataNames( resultID );
    for ( unsigned int iDataName = 0; iDataName < dataNames.size(); iDataName++ )
    {
        if ( ( strncmp( dataNames[iDataName].c_str(), "FC_", 3 ) != 0 )  &&
            ( strcmp( dataNames[iDataName].c_str(), "WakeIter" ) != 0 )  &&
             ( strcmp( dataNames[iDataName].c_str(), "Mach" ) != 0 )  &&
             ( strcmp( dataNames[iDataName].c_str(), "Alpha" ) != 0 )  &&
             ( strcmp( dataNames[iDataName].c_str(), "Beta" ) != 0 )  &&
             ( strcmp( dataNames[iDataName].c_str(), "AnalysisMethod" ) != 0 ) &&
             ( strcmp( dataNames[iDataName].c_str(), "Time" ) != 0 ) &&
             ( strcmp( dataNames[iDataName].c_str(), "Group_Num" ) != 0 ) &&
             ( strcmp( dataNames[iDataName].c_str(), "Rotor_Num" ) != 0 ) &&
             ( strcmp( dataNames[iDataName].c_str(), "Group_Name" ) != 0 ) &&
             ( strcmp( dataNames[iDataName].c_str(), "RPM" ) != 0 ) ) // FIXME: Figure out why selecting rotor RPM causes the GUI to freeze
        {
            m_UnsteadyYDataBrowser->add( dataNames[iDataName].c_str() );
        }
    }

    // restore original row selections
    for ( unsigned int iCase = 0; iCase < m_UnsteadyYDataBrowser->size(); iCase++ )
    {
        if ( ( m_SelectDefaultData && strcmp( m_UnsteadyYDataBrowser->text( iCase + 1 ), default_res.c_str() ) == 0 ) )
        {
            m_UnsteadyYDataBrowser->select( iCase + 1 ); //account for browser using 1-based indexing
        }
        if ( iCase < wasSelected.size() )
        {
            if ( wasSelected[iCase] )
            {
                m_UnsteadyYDataBrowser->select( iCase + 1 ); //account for browser using 1-based indexing
            }
        }
    }

    m_UnsteadyYDataBrowser->position( scrollPos );
}

void VSPAEROPlotScreen::RedrawConvergencePlot()
{
    Ca_Canvas::current( m_ConvergencePlotCanvas );
    m_ConvergencePlotCanvas->clear();

    m_ConvergenceLegendGroup->clear();
    m_ConvergenceLegendLayout.SetGroup( m_ConvergenceLegendGroup );
    m_ConvergenceLegendLayout.InitWidthHeightVals();
    m_ConvergenceLegendLayout.SetButtonWidth( (int)(m_ConvergenceLegendLayout.GetW() * m_PercentLegendColor) );

    // Get selected y data names
    vector <string> yDataSetNames;
    for ( int i = 1 ; i <= m_ConvergenceYDataBrowser->size() ; i++ )
    {
        if ( m_ConvergenceYDataBrowser->selected( i ) )
        {
            yDataSetNames.push_back( m_ConvergenceYDataBrowser->text( i ) );
        }
    }

    m_ConvergenceNLines = m_ConvergenceFlowConditionSelectedResultIDs.size() * yDataSetNames.size();
    m_ConvergenceiPlot = 0;

    // Plot only if y data has been selected
    if ( m_ConvergenceNLines > 0 )
    {
        bool expandOnly = false;
        for ( unsigned int iCase = 0; iCase < m_ConvergenceFlowConditionSelectedResultIDs.size(); iCase++ )
        {
            PlotConvergence( m_ConvergenceFlowConditionSelectedResultIDs[iCase], yDataSetNames, expandOnly, iCase );
            expandOnly = true;
        }
    }
    //(Ca_Axis_ *) m_ConvergencePlotCanvas->current_y()->scale(CA_LOG);

    //m_ConvergencePlotCanvas->redraw();
}

void VSPAEROPlotScreen::RedrawLoadDistPlot()
{
    Ca_Canvas::current( m_LoadDistPlotCanvas );
    m_LoadDistPlotCanvas->clear();

    m_LoadDistLegendGroup->clear();
    m_LoadDistLegendLayout.SetGroup( m_LoadDistLegendGroup );
    m_LoadDistLegendLayout.InitWidthHeightVals();
    m_LoadDistLegendLayout.SetButtonWidth( (int)(m_LoadDistLegendLayout.GetW() * m_PercentLegendColor) );

    // Get selected y data names
    vector <string> yDataSetNames;
    for ( int i = 1 ; i <= m_LoadDistYDataBrowser->size() ; i++ )
    {
        if ( m_LoadDistYDataBrowser->selected( i ) )
        {
            yDataSetNames.push_back( m_LoadDistYDataBrowser->text( i ) );
        }
    }

    m_LoadDistNLines = yDataSetNames.size() * m_LoadDistFlowConditionSelectedResultIDs.size();
    if ( m_LoadSelectedBladeVec.size() > 0 ) // Since "Load" is not included in the vector
    {
        m_LoadDistNLines *= m_LoadSelectedBladeVec.size();
    }

    m_LoadDistiPlot = 0;

    // Plot only if y data has been selected
    if ( m_LoadDistNLines > 0 )
    {
        bool expandOnly = false;

        if ( VSPAEROMgr.m_LoadDistSelectType.Get() == VSPAEROMgrSingleton::LOAD_SELECT_TYPE )
        {
            for ( unsigned int iCase = 0; iCase < m_LoadDistFlowConditionSelectedResultIDs.size(); iCase++ )
            {
                PlotLoadDistribution( m_LoadDistFlowConditionSelectedResultIDs[iCase], yDataSetNames, expandOnly, iCase );
                expandOnly = true;
            }
        }
        else if ( VSPAEROMgr.m_LoadDistSelectType.Get() == VSPAEROMgrSingleton::BLADE_SELECT_TYPE )
        {
            for ( unsigned int iCase = 0; iCase < m_LoadSelectedBladeVec.size(); iCase++ )
            {
                PlotLoadDistribution( ResultsMgr.FindResultsID( "VSPAERO_Blade_Avg", m_LoadSelectedBladeVec[iCase] ), yDataSetNames, expandOnly, iCase );
                expandOnly = true;
            }
        }
    }
}

void VSPAEROPlotScreen::RedrawSweepPlot()
{
    Ca_Canvas::current( m_SweepPlotCanvas );
    m_SweepPlotCanvas->clear();

    // Get selected dataset names
    vector <string> xDataSetNames;
    for ( int i = 1 ; i <= m_SweepXDataBrowser->size() ; i++ )
    {
        if ( m_SweepXDataBrowser->selected( i ) )
        {
            xDataSetNames.push_back( m_SweepXDataBrowser->text( i ) );
        }
    }
    vector <string> yDataSetNames;
    for ( int i = 1 ; i <= m_SweepYDataBrowser->size() ; i++ )
    {
        if ( m_SweepYDataBrowser->selected( i ) )
        {
            yDataSetNames.push_back( m_SweepYDataBrowser->text( i ) );
        }
    }

    int iplot = 0; // Serialized counter for legend colors/symbols
    int nlines = xDataSetNames.size() * yDataSetNames.size();

    m_SweepLegendGroup->clear();
    m_SweepLegendLayout.SetGroup( m_SweepLegendGroup );
    m_SweepLegendLayout.InitWidthHeightVals();
    m_SweepLegendLayout.SetButtonWidth( (int)(m_SweepLegendLayout.GetW() * m_PercentLegendColor) );

    //Redraw plot if data is available and selected
    bool expandOnly = false;
    int nPolarPoints = m_SweepFlowConditionSelectedResultIDs.size();
    if ( ( nPolarPoints > 0 ) && ( xDataSetNames.size() > 0 ) && ( yDataSetNames.size() > 0 ) )
    {
        for ( int iXData = 0; iXData < xDataSetNames.size(); iXData++ )
        {
            for ( int iYData = 0; iYData < yDataSetNames.size(); iYData++ )
            {
                vector <double> xDoubleData;
                vector <double> yDoubleData;
                NameValData* tResultDataPtr;
                int nData;
                for ( int iPolarPoint = 0; iPolarPoint < nPolarPoints; iPolarPoint++ )
                {
                    Results* res = ResultsMgr.FindResultsPtr( m_SweepFlowConditionSelectedResultIDs[iPolarPoint] );
                    if ( res )
                    {
                        //====Plot Stuff====//
                        tResultDataPtr = res->FindPtr( xDataSetNames[iXData] );
                        nData = tResultDataPtr->GetDoubleData().size();
                        xDoubleData.push_back( tResultDataPtr->GetDouble( nData - 1 ) );

                        tResultDataPtr = res->FindPtr( yDataSetNames[iYData] );
                        nData = tResultDataPtr->GetDoubleData().size();
                        yDoubleData.push_back( tResultDataPtr->GetDouble( nData - 1 ) );
                    }
                }

                Fl_Color c = ColorWheel( iplot, nlines );

                //add the data to the plot
                AddPointLine( xDoubleData, yDoubleData, 2, c, 4, StyleWheel( iplot ) );

                string legendstr = yDataSetNames[ iYData ] + " vs. " + xDataSetNames[ iXData ];
                m_SweepLegendLayout.AddLegendEntry( legendstr, c );
                iplot++;

                //Handle axis limits
                UpdateAxisLimits( m_SweepPlotCanvas, xDoubleData, yDoubleData, expandOnly );
                expandOnly = true;
            }
        }
        // Annotate
        //string xLabelStr = MakeAxisLabelStr(xDataSetNames);
        //string yLabelStr = MakeAxisLabelStr(yDataSetNames);
        //m_SweepPlotCanvas->current_x()->copy_label(xLabelStr.c_str());
        //m_SweepPlotCanvas->current_y()->copy_label(yLabelStr.c_str());
        if( xDataSetNames.size() == 1 )
        {
            m_SweepPlotCanvas->current_x()->copy_label( xDataSetNames[0].c_str() );
        }
        else
        {
            m_SweepPlotCanvas->current_x()->copy_label( "[multiple]" );
        }
        if( yDataSetNames.size() == 1 )
        {
            m_SweepPlotCanvas->current_y()->copy_label( yDataSetNames[0].c_str() );
        }
        else
        {
            m_SweepPlotCanvas->current_y()->copy_label( "[multiple]" );
        }

    }

}

void VSPAEROPlotScreen::RedrawCpSlicePlot()
{
    Ca_Canvas::current( m_CpSlicePlotCanvas );
    m_CpSlicePlotCanvas->clear();

    int iplot = 0; // Serialized counter for legend colors/symbols
    int nlines = m_CpSliceCaseSelectedResultIDs.size() * m_CpSliceCutSelectedResultIDs.size();

    m_CpSliceLegendGroup->clear();
    m_CpSliceLegendLayout.SetGroup( m_CpSliceLegendGroup );
    m_CpSliceLegendLayout.InitWidthHeightVals();
    m_CpSliceLegendLayout.SetButtonWidth( (int)( m_CpSliceLegendLayout.GetW() * m_PercentLegendColor ) );

    //Redraw plot if data is available and selected
    bool expandOnly = false;
    vector < int > pos_type_vec;

    if ( ( m_CpSliceCaseSelectedResultIDs.size() > 0 ) && ( m_CpSliceCutSelectedResultIDs.size() > 0 ) )
    {
        for ( size_t iCut = 0; iCut < m_CpSliceCutSelectedResultIDs.size(); iCut++ )
        {
            for ( int iCase = 0; iCase < m_CpSliceCaseSelectedResultIDs.size(); iCase++ )
            {
                if ( m_CpSliceCutSelectedResultIDs[iCut] == m_CpSliceCaseSelectedResultIDs[iCase] )
                {
                    NameValData* tResultDataPtr = NULL;
                    vector <double> CpData, locData;
                    int type = 0;
                    int cut_num = 0;
                    int case_num = 0;

                    Results* res = ResultsMgr.FindResultsPtr( m_CpSliceCutSelectedResultIDs[iCut] );
                    if ( res )
                    {
                        if ( VSPAEROMgr.GetCpSliceAnalysisType() == vsp::VORTEX_LATTICE )
                        {
                            tResultDataPtr = res->FindPtr( "dCp" );
                        }
                        else if ( VSPAEROMgr.GetCpSliceAnalysisType() == vsp::PANEL )
                        {
                            tResultDataPtr = res->FindPtr( "Cp" );
                        }

                        if ( tResultDataPtr )
                        {
                            CpData = tResultDataPtr->GetDoubleData();
                        }

                        tResultDataPtr = res->FindPtr( "Cut_Num" );
                        if ( tResultDataPtr )
                        {
                            cut_num = tResultDataPtr->GetIntData()[0];
                        }

                        tResultDataPtr = res->FindPtr( "Case" );
                        if ( tResultDataPtr )
                        {
                            case_num = tResultDataPtr->GetIntData()[0];
                        }

                        tResultDataPtr = res->FindPtr( "Cut_Type" );
                        if ( tResultDataPtr )
                        {
                            type = tResultDataPtr->GetIntData()[0];
                        }

                        if ( m_CpSlicePosTypeChoiceVec[type]->GetVal() == vsp::X_DIR )
                        {
                            tResultDataPtr = res->FindPtr( "X_Loc" );
                        }
                        else if ( m_CpSlicePosTypeChoiceVec[type]->GetVal() == vsp::Y_DIR )
                        {
                            tResultDataPtr = res->FindPtr( "Y_Loc" );
                        }
                        else if ( m_CpSlicePosTypeChoiceVec[type]->GetVal() == vsp::Z_DIR )
                        {
                            tResultDataPtr = res->FindPtr( "Z_Loc" );
                        }

                        pos_type_vec.push_back( m_CpSlicePosTypeChoiceVec[type]->GetVal() );

                        if ( tResultDataPtr )
                        {
                            locData = tResultDataPtr->GetDoubleData();
                        }
                    }

                    Fl_Color c = ColorWheel( iplot, nlines );

                    if ( VSPAEROMgr.m_CpSliceYAxisFlipFlag() )
                    {
                        vector < double > temp_vec;
                        temp_vec.resize( CpData.size() );
                        for ( size_t k = 0; k < CpData.size(); k++ )
                        {
                            temp_vec[k] = -1 * CpData[k];
                        }
                        CpData = temp_vec;
                    }

                    //add the data to the plot
                    if ( VSPAEROMgr.m_CpSlicePlotLinesFlag() )
                    {
                        AddPointLine( locData, CpData, 2, c, 4, StyleWheel( iplot ) );
                    }
                    else
                    {
                        AddPoint( locData, CpData, c, 4, StyleWheel( iplot ) );
                    }

                    char strbuf[1024];
                    sprintf( strbuf, "Cut %d, Case %d", cut_num, case_num );
                    m_CpSliceLegendLayout.AddLegendEntry( strbuf, c );
                    iplot++;

                    //Handle axis limits
                    UpdateAxisLimits( m_CpSlicePlotCanvas, locData, CpData, expandOnly );
                    expandOnly = true;
                }
            }
        }

        // Check for all same CpSlice cut type
        if ( std::adjacent_find( pos_type_vec.begin(), pos_type_vec.end(), std::not_equal_to<int>() ) == pos_type_vec.end() )
        {
            char strbuf[1024];
            sprintf( strbuf, "Position: %c", ( 88 + pos_type_vec[0] ) ); // ASCII X: 88; Y: 89; Z: 90

            m_CpSlicePlotCanvas->current_x()->copy_label( strbuf );
        }
        else
        {
            m_CpSlicePlotCanvas->current_x()->copy_label( "Position [multiple]" );
        }

        string y_label = "CP";

        if ( VSPAEROMgr.GetCpSliceAnalysisType() == vsp::VORTEX_LATTICE )
        {
            y_label = "d" + y_label;
        }

        if ( VSPAEROMgr.m_CpSliceYAxisFlipFlag() )
        {
            y_label = "-" + y_label;
        }

        m_CpSlicePlotCanvas->current_y()->copy_label( y_label.c_str() );
    }
}

void VSPAEROPlotScreen::RedrawUnsteadyPlot()
{
    Ca_Canvas::current( m_UnsteadyPlotCanvas );
    m_UnsteadyPlotCanvas->clear();

    m_UnsteadyLegendGroup->clear();
    m_UnsteadyLegendLayout.SetGroup( m_UnsteadyLegendGroup );
    m_UnsteadyLegendLayout.InitWidthHeightVals();
    m_UnsteadyLegendLayout.SetButtonWidth( (int)( m_UnsteadyLegendLayout.GetW() * m_PercentLegendColor ) );

    // Get selected y data names
    vector <string> yDataSetNames;
    for ( int i = 1; i <= m_UnsteadyYDataBrowser->size(); i++ )
    {
        if ( m_UnsteadyYDataBrowser->selected( i ) )
        {
            yDataSetNames.push_back( m_UnsteadyYDataBrowser->text( i ) );
        }
    }

    // Get selected rotor/groups
    vector <string> selectSetNames;
    for ( int i = 1; i <= m_UnsteadySelectBrowser->size(); i++ )
    {
        if ( m_UnsteadySelectBrowser->selected( i ) )
        {
            selectSetNames.push_back( m_UnsteadySelectBrowser->text( i ) );
        }
    }

    m_UnsteadyNLines = m_UnsteadyFlowConditionSelectedResultIDs.size() * yDataSetNames.size();
    if ( m_UnsteadySelectedTypeVec.size() > 0 ) // Since "History" is not included in the vector
    {
        m_UnsteadyNLines *= m_UnsteadySelectedTypeVec.size();
    }

    m_UnsteadyiPlot = 0;

    // Plot only if y data has been selected
    if ( m_UnsteadyNLines > 0 )
    {
        bool expandOnly = false;

        if ( VSPAEROMgr.m_UnsteadyGroupSelectType.Get() == VSPAEROMgrSingleton::GROUP_SELECT_TYPE )
        {
            for ( unsigned int iCase = 0; iCase < m_UnsteadySelectedTypeVec.size(); iCase++ )
            {
                PlotUnsteady( ResultsMgr.FindResultsID( "VSPAERO_Group", m_UnsteadySelectedTypeVec[iCase] ), yDataSetNames, expandOnly, iCase );
                expandOnly = true;
            }
        }
        else if ( VSPAEROMgr.m_UnsteadyGroupSelectType.Get() == VSPAEROMgrSingleton::ROTOR_SELECT_TYPE )
        {
            for ( unsigned int iCase = 0; iCase < m_UnsteadySelectedTypeVec.size(); iCase++ )
            {
                PlotUnsteady( ResultsMgr.FindResultsID( "VSPAERO_Rotor", m_UnsteadySelectedTypeVec[iCase] ), yDataSetNames, expandOnly, iCase );
                expandOnly = true;
            }
        }
        else if ( VSPAEROMgr.m_UnsteadyGroupSelectType.Get() == VSPAEROMgrSingleton::HISTORY_SELECT_TYPE )
        {
            for ( unsigned int iCase = 0; iCase < m_UnsteadyFlowConditionSelectedResultIDs.size(); iCase++ )
            {
                PlotUnsteady( m_UnsteadyFlowConditionSelectedResultIDs[iCase], yDataSetNames, expandOnly, iCase );
                expandOnly = true;
            }
        }
    }
}

string VSPAEROPlotScreen::MakeAxisLabelStr( vector <string> dataSetNames )
{
    string labelStr;
    for ( int iDataSet = 0; iDataSet < dataSetNames.size(); iDataSet++ )
    {
        labelStr = labelStr + dataSetNames[iDataSet].c_str();
        if ( iDataSet < ( int )dataSetNames.size() - 1 )
        {
            labelStr = labelStr + ", ";
        }
    }
    return labelStr;
}


void VSPAEROPlotScreen::PlotConvergence( string resultID, vector <string> yDataSetNames, bool expandOnly, int icase )
{

    Results* res = ResultsMgr.FindResultsPtr( resultID );
    if ( !res )
    {
        //TODO indicate that no data exists
        m_ConvergencePlotCanvas->current_y()->label( "[Y]" );
        m_ConvergencePlotCanvas->current_x()->label( "[X]" );
    }
    else if ( strcmp( res->GetName().c_str(), "VSPAERO_History" ) == 0 )
    {

        //  Get the X Data
        vector <double> xDoubleData_orig;
        NameValData* xResultDataPtr;
        xResultDataPtr = res->FindPtr( "WakeIter" );
        if ( (xResultDataPtr != NULL) )
        {
            vector <int> tIntData = xResultDataPtr->GetIntData();
            copy( tIntData.begin(), tIntData.end(), back_inserter( xDoubleData_orig ) );
        }
                
        // Get the Y-Data
        NameValData* yResultDataPtr;
        for ( int iDataSet = 0; iDataSet < ( int )yDataSetNames.size(); iDataSet++ )
        {
            yResultDataPtr = res->FindPtr( yDataSetNames[iDataSet] );
            if (  yResultDataPtr != NULL  )
            {
                vector <double> xDoubleData = xDoubleData_orig;
                vector <double> yDoubleData;
                if ( yResultDataPtr->GetType() == vsp::INT_DATA )
                {
                    vector <int> tIntData = yResultDataPtr->GetIntData();
                    copy( tIntData.begin(), tIntData.end(), back_inserter( yDoubleData ) );
                }
                else
                {
                    yDoubleData = yResultDataPtr->GetDoubleData();
                }

                if (  (xDoubleData.size() == yDoubleData.size()) && (xDoubleData.size()>0) )
                {
                    //normalize the iteration data w.r.t. the final value
                    if ( m_ConvergenceYDataResidualToggle.GetFlButton()->value() == 1 )
                    {
                        double diffY;
                        vector <double> tempY = yDoubleData;
                        unsigned int j = 1;
                        unsigned int jMax = ( unsigned int )tempY.size();
                        while( j < jMax )
                        {
                            diffY = tempY[j] - tempY[j - 1];
                            if ( diffY != 0 ) // inf protection on log10()
                            {
                                yDoubleData[j] = log10( std::abs( diffY ) );
                                j++;
                            }
                            else
                            {
                                //value did not change, eliminate the data point, this eliminates "jumping" on plot screen
                                xDoubleData.erase( xDoubleData.begin() + j );
                                yDoubleData.erase( yDoubleData.begin() + j );
                                tempY.erase( tempY.begin() + j );
                                jMax--;
                            }
                        }

                        // No residual for 1st data point, so remove it
                        if ( xDoubleData.size() > 0 )
                        {
                            xDoubleData.erase( xDoubleData.begin() );
                            yDoubleData.erase( yDoubleData.begin() );
                        }
                    }

                    // check again if there are still points to plot
                    if ( xDoubleData.size()>0 )
                    {
                        Fl_Color c = ColorWheel( m_ConvergenceiPlot, m_ConvergenceNLines );

                        //add the normalized data to the plot
                        AddPointLine( xDoubleData, yDoubleData, 2, c, 4, StyleWheel( m_ConvergenceiPlot ) );

                        char strbuf[100];
                        ConstructFlowConditionString( strbuf, res, false, true );
                        string legendstr = strbuf + string( "; Y: " ) + yDataSetNames[iDataSet];
                        m_ConvergenceLegendLayout.AddLegendEntry( legendstr, c );
                        m_ConvergenceiPlot++;

                        //Handle Axis limits
                        //  Auto adjust and expand limits for Y-Axis
                        if ( m_ConvergenceYDataResidualToggle.GetFlButton()->value() == 1 )
                        {
                            //Always show 0 on Y Axis if the data is normalized on a log scale
                            UpdateSingleAxisLimits( m_ConvergencePlotCanvas->current_y(), yDoubleData, expandOnly, true );
                        }
                        else
                        {
                            UpdateSingleAxisLimits( m_ConvergencePlotCanvas->current_y(), yDoubleData, expandOnly, true );
                        }
                    }
                }
                else
                {
                    fprintf( stderr, "WARNING: xDoubleData.size() and yDoubleData.size() must be equal AND greater than 0\n\txDoubleData.size()=%ld\n\tyDoubleData.size()=%ld\n\tFile: %s \tLine:%d\n", xDoubleData.size(), yDoubleData.size(), __FILE__, __LINE__ );
                }

                expandOnly = true;
            }
        }
                

        // Explicitly set the X-axis limits  to the wake iteration limits
        if ( xDoubleData_orig.size() > 0 )
        {
            m_ConvergencePlotCanvas->current_x()->minimum( xDoubleData_orig[0] );
            m_ConvergencePlotCanvas->current_x()->maximum( xDoubleData_orig[xDoubleData_orig.size() - 1] );
        }


        // Annotate axes
        string labelStr;
        labelStr.clear();
        if( yDataSetNames.size() == 1 )
        {
            labelStr = yDataSetNames[0];
        }
        else
        {
            labelStr = "[multiple]";
        }
        if ( m_ConvergenceYDataResidualToggle.GetFlButton()->value() == 1 )
        {
            labelStr = "log10(|" + labelStr + "|)";
        }
        m_ConvergencePlotCanvas->current_y()->copy_label( labelStr.c_str() );
        m_ConvergencePlotCanvas->current_x()->label( "Wake Iteration #" );

    }
    else
    {
        m_ConvergencePlotCanvas->label( "PLOT ERROR - INVALID RESULT TYPE" );
    }

}

void VSPAEROPlotScreen::PlotLoadDistribution( string resultID, vector <string> yDataSetNames, bool expandOnly, int icase )
{
    Results* res = ResultsMgr.FindResultsPtr( resultID );
    if ( !res )
    {
        //TODO indicate that no data exists
        m_LoadDistPlotCanvas->current_y()->label( "[Y]" );
        m_LoadDistPlotCanvas->current_x()->label( "[X]" );
    }
    else if ( strcmp( res->GetName().c_str(), "VSPAERO_Load" ) == 0 || strcmp( res->GetName().c_str(), "VSPAERO_Blade_Avg" ) == 0 )
    {
        NameValData* wingIdResultDataPtr = 0;
        NameValData* xResultDataPtr = 0;
        NameValData* yResultDataPtr = 0;
        vector <double> xDoubleData_orig;
        string group_name, y_label;

        if ( VSPAEROMgr.m_LoadDistSelectType.Get() == VSPAEROMgrSingleton::LOAD_SELECT_TYPE )
        {
            wingIdResultDataPtr = res->FindPtr( "WingId" );
            xResultDataPtr = res->FindPtr( "Yavg" );
            y_label = "Span Location: Y";
        }
        else if ( VSPAEROMgr.m_LoadDistSelectType.Get() == VSPAEROMgrSingleton::BLADE_SELECT_TYPE )
        {
            xResultDataPtr = res->FindPtr( "Station" );
            y_label = "Station";

            if ( ( xResultDataPtr != NULL ) )
            {
                vector <int> tIntData = xResultDataPtr->GetIntData();
                copy( tIntData.begin(), tIntData.end(), back_inserter( xDoubleData_orig ) );
            }

            NameValData* name_nvd = res->FindPtr( "Group_Name" );

            if ( name_nvd )
            {
                group_name = name_nvd->GetString( 0 );

                // Replace "Surf" with "S" to shorten name
                string search = "Surf_";
                string replace = "S";

                StringUtil::replace_all( group_name, search, replace );
            }
        }

        for ( int iDataSet = 0; iDataSet < (int)yDataSetNames.size(); iDataSet++ )
        {
            yResultDataPtr = res->FindPtr( yDataSetNames[iDataSet] );
            if ( xResultDataPtr && yResultDataPtr )
            {
                Fl_Color c = ColorWheel( m_LoadDistiPlot, m_LoadDistNLines );
                string legendstr;

                if ( VSPAEROMgr.m_LoadDistSelectType.Get() == VSPAEROMgrSingleton::LOAD_SELECT_TYPE && wingIdResultDataPtr )
                {
                    // get unique indicies and generate loop over unique wings
                    vector <int> wingIdIntDataRaw = wingIdResultDataPtr->GetIntData();
                    vector <int> wingIdIntDataUnique;
                    std::unique_copy( wingIdIntDataRaw.begin(), wingIdIntDataRaw.end(), std::back_inserter( wingIdIntDataUnique ) );

                    // Collect data for each wing and plot individual line for each wing (assumes unsorted list)
                    for ( int iWing = 0; iWing < wingIdIntDataUnique.size(); iWing++ )
                    {
                        //collect the data for each wing checking each point
                        vector <double> xDoubleData;
                        vector <double> yDoubleData;
                        for ( unsigned int iPt = 0; iPt < wingIdIntDataRaw.size(); iPt++ )
                        {
                            if ( wingIdIntDataRaw[iPt] == wingIdIntDataUnique[iWing] )
                            {
                                xDoubleData.push_back( xResultDataPtr->GetDouble( iPt ) );
                                yDoubleData.push_back( yResultDataPtr->GetDouble( iPt ) );
                            }
                        }

                        //Sort data for this wing along Yavg dimension
                        // generate sorted indicies array
                        vector < int > Indx;                    Indx.assign( yDoubleData.size(), 0 );
                        for ( unsigned int iPt = 0; iPt < xDoubleData.size(); iPt++ )
                        {
                            Indx[iPt] = iPt;
                        }
                        std::sort( Indx.begin(), Indx.end(), [&xDoubleData]( int i, int j ) {return xDoubleData[i] < xDoubleData[j];} );
                        // copy data to sorted arrays
                        vector <double> xDoubleData_sorted;     xDoubleData_sorted.assign( xDoubleData.size(), 0 );
                        vector <double> yDoubleData_sorted;     yDoubleData_sorted.assign( yDoubleData.size(), 0 );
                        for ( unsigned int iPt = 0; iPt < xDoubleData.size(); iPt++ )
                        {
                            xDoubleData_sorted[iPt] = xDoubleData[Indx[iPt]];
                            yDoubleData_sorted[iPt] = yDoubleData[Indx[iPt]];
                        }

                        //add the data to the plot
                        AddPointLine( xDoubleData_sorted, yDoubleData_sorted, 2, c, 4, StyleWheel( m_LoadDistiPlot ) );

                        //Handle axis limits
                        UpdateAxisLimits( m_LoadDistPlotCanvas, xDoubleData_sorted, yDoubleData_sorted, expandOnly );
                        expandOnly = true;
                    }

                    char strbuf[100];
                    ConstructFlowConditionString( strbuf, res, false, false );
                    legendstr = strbuf;
                }
                else if ( VSPAEROMgr.m_LoadDistSelectType.Get() == VSPAEROMgrSingleton::BLADE_SELECT_TYPE )
                {
                    vector <double> yDoubleData;
                    if ( yResultDataPtr->GetType() == vsp::INT_DATA )
                    {
                        vector <int> tIntData = yResultDataPtr->GetIntData();
                        copy( tIntData.begin(), tIntData.end(), back_inserter( yDoubleData ) );
                    }
                    else
                    {
                        yDoubleData = yResultDataPtr->GetDoubleData();
                    }

                    // check again if there are still points to plot
                    if ( ( xDoubleData_orig.size() == yDoubleData.size() ) && ( xDoubleData_orig.size() > 0 ) )
                    {
                        //add the normalized data to the plot
                        AddPointLine( xDoubleData_orig, yDoubleData, 2, c, 4, StyleWheel( m_LoadDistiPlot ) );

                        legendstr = group_name;

                        if ( res->FindPtr( "Blade_Num" ) )
                        {
                            legendstr += ( "_B" + to_string( res->FindPtr( "Blade_Num" )->GetInt( 0 ) ) );
                        }

                        //Handle Axis limits
                        //  Auto adjust and expand limits for Y-Axis
                        //UpdateSingleAxisLimits( m_LoadDistPlotCanvas->current_y(), yDoubleData, expandOnly, false );

                        UpdateAxisLimits( m_LoadDistPlotCanvas, xDoubleData_orig, yDoubleData, expandOnly );
                        expandOnly = true;
                    }
                }

                legendstr += ( string( "; Y: " ) + yDataSetNames[iDataSet] );
                m_LoadDistLegendLayout.AddLegendEntry( legendstr, c );
                m_LoadDistiPlot++;
            }
        }

        // Annotate axes
        string x_label;
        if ( yDataSetNames.size() == 1 )
        {
            x_label = yDataSetNames[0];
        }
        else
        {
            x_label = "[multiple]";
        }
        m_LoadDistPlotCanvas->current_y()->copy_label( x_label.c_str() );
        m_LoadDistPlotCanvas->current_x()->copy_label( y_label.c_str() );
    }
    else
    {
        m_LoadDistPlotCanvas->label( "PLOT ERROR - INVALID RESULT TYPE" );
    }
}

void VSPAEROPlotScreen::PlotUnsteady( string resultID, vector <string> yDataSetNames, bool expandOnly, int icase )
{
    Results* res = ResultsMgr.FindResultsPtr( resultID );
    if ( !res )
    {
        //TODO indicate that no data exists
        m_UnsteadyPlotCanvas->current_y()->label( "[Y]" );
        m_UnsteadyPlotCanvas->current_x()->label( "[X]" );
    }
    else if ( strcmp( res->GetName().c_str(), "VSPAERO_History" ) == 0 || strcmp( res->GetName().c_str(), "VSPAERO_Rotor" ) == 0 || strcmp( res->GetName().c_str(), "VSPAERO_Group" ) == 0 )
    {
        //  Get the X Data
        vector <double> xDoubleData_orig;
        NameValData* xResultDataPtr;
        xResultDataPtr = res->FindPtr( "Time" );

        if ( ( xResultDataPtr != NULL ) )
        {
            vector <double> tDoubledata = xResultDataPtr->GetDoubleData();
            copy( tDoubledata.begin(), tDoubledata.end(), back_inserter( xDoubleData_orig ) );
        }

        string group_name;

        if ( VSPAEROMgr.m_UnsteadyGroupSelectType.Get() == VSPAEROMgrSingleton::GROUP_SELECT_TYPE ||
             VSPAEROMgr.m_UnsteadyGroupSelectType.Get() == VSPAEROMgrSingleton::ROTOR_SELECT_TYPE )
        {
            NameValData* name_nvd = res->FindPtr( "Group_Name" );

            if ( name_nvd )
            {
                group_name = name_nvd->GetString( 0 );

                // Replace "Surf" with "S" to shorten name
                string search = "Surf_";
                string replace = "S";

                StringUtil::replace_all( group_name, search, replace );
            }
        }

        // Get the Y-Data
        NameValData* yResultDataPtr;
        for ( int iDataSet = 0; iDataSet < (int)yDataSetNames.size(); iDataSet++ )
        {
            yResultDataPtr = res->FindPtr( yDataSetNames[iDataSet] );
            if ( yResultDataPtr != NULL )
            {
                vector <double> xDoubleData = xDoubleData_orig;
                vector <double> yDoubleData;
                if ( yResultDataPtr->GetType() == vsp::INT_DATA )
                {
                    vector <int> tIntData = yResultDataPtr->GetIntData();
                    copy( tIntData.begin(), tIntData.end(), back_inserter( yDoubleData ) );
                }
                else
                {
                    yDoubleData = yResultDataPtr->GetDoubleData();
                }

                if ( ( xDoubleData.size() == yDoubleData.size() ) && ( xDoubleData.size()>0 ) )
                {
                    // check again if there are still points to plot
                    if ( xDoubleData.size()>0 )
                    {
                        Fl_Color c = ColorWheel( m_UnsteadyiPlot, m_UnsteadyNLines );

                        //add the normalized data to the plot
                        AddPointLine( xDoubleData, yDoubleData, 2, c, 4, StyleWheel( m_UnsteadyiPlot ) );

                        char strbuf[100];
                        string legendstr;

                        if ( VSPAEROMgr.m_UnsteadyGroupSelectType.Get() == VSPAEROMgrSingleton::GROUP_SELECT_TYPE ||
                             VSPAEROMgr.m_UnsteadyGroupSelectType.Get() == VSPAEROMgrSingleton::ROTOR_SELECT_TYPE )
                        {
                            legendstr = group_name;
                        }
                        else if ( VSPAEROMgr.m_UnsteadyGroupSelectType.Get() == VSPAEROMgrSingleton::HISTORY_SELECT_TYPE )
                        {
                            ConstructFlowConditionString( strbuf, res, false, true );
                            legendstr = strbuf;
                        }

                        legendstr += ( string( "; Y: " ) + yDataSetNames[iDataSet] );

                        m_UnsteadyLegendLayout.AddLegendEntry( legendstr, c );
                        m_UnsteadyiPlot++;

                        //Handle Axis limits
                        //  Auto adjust and expand limits for Y-Axis
                        UpdateSingleAxisLimits( m_UnsteadyPlotCanvas->current_y(), yDoubleData, expandOnly, false );
                    }
                }
                else
                {
                    fprintf( stderr, "WARNING: xDoubleData.size() and yDoubleData.size() must be equal AND greater than 0\n\txDoubleData.size()=%ld\n\tyDoubleData.size()=%ld\n\tFile: %s \tLine:%d\n", xDoubleData.size(), yDoubleData.size(), __FILE__, __LINE__ );
                }

                expandOnly = true;
            }
        }

        // Explicitly set the X-axis limits  to the wake iteration limits
        if ( xDoubleData_orig.size() > 0 )
        {
            m_UnsteadyPlotCanvas->current_x()->minimum( xDoubleData_orig[0] );
            m_UnsteadyPlotCanvas->current_x()->maximum( xDoubleData_orig[xDoubleData_orig.size() - 1] );
        }

        // Annotate axes
        string labelStr;
        labelStr.clear();
        if ( yDataSetNames.size() == 1 )
        {
            labelStr = yDataSetNames[0];
        }
        else
        {
            labelStr = "[multiple]";
        }

        m_UnsteadyPlotCanvas->current_y()->copy_label( labelStr.c_str() );
        m_UnsteadyPlotCanvas->current_x()->label( "Time (sec)" );
    }
    else
    {
        m_UnsteadyPlotCanvas->label( "PLOT ERROR - INVALID RESULT TYPE" );
    }

}

void VSPAEROPlotScreen::UpdateAxisLimits( Ca_Canvas * canvas, vector <double> xDoubleData, vector <double> yDoubleData, bool expandOnly )
{
    UpdateSingleAxisLimits( canvas->current_x(), xDoubleData, expandOnly );
    UpdateSingleAxisLimits( canvas->current_y(), yDoubleData, expandOnly );
}

void VSPAEROPlotScreen::UpdateSingleAxisLimits( Ca_Axis_ * tAxis, vector <double> doubleData, bool expandOnly, bool keepZero )
{
    //TODO Get data from canvas and make this a part of the Canvas class
    if ( doubleData.size() > 0 )
    {

        // Find min and max of vector
        auto dataMinMax = std::minmax_element( doubleData.begin(), doubleData.end() );
        double dataMin = dataMinMax.first[0];
        double dataMax = dataMinMax.second[0];
        double dataRange = (dataMax - dataMin);
        double dataRangeScale = pow( 10.0, mag( dataRange ) );


        // Infinity protection - bounds the magnitude of dataMin and dataMax to single precision magnitudes
        dataMin = std::min( dataMin, (double)FLT_MAX );
        dataMin = std::max( dataMin, (double)-FLT_MAX );
        dataMax = std::min( dataMax, (double)FLT_MAX );
        dataMax = std::max( dataMax, (double)-FLT_MAX );

        // Calculate min and max number magnitudes or "scale"
        double axisMin, axisMax;
        if ( keepZero )
        {
            axisMin = magrounddn( dataMin );
            axisMax = magroundup( dataMax );
        }
        else
        {
            axisMin = floor2scale( dataMin, dataRangeScale );
            axisMax = ceil2scale( dataMax, dataRangeScale );
        }

        // Protection for if the magnitudes are identical (plot won't have any range unless this is here)
        if ( axisMin == axisMax || dataRange == 0 )
        {
            axisMin -= 1e-12;
            axisMax += 1e-12;
        }

        // Apply limits to axis
        if ( tAxis && axisMin < axisMax )
        {
            if ( expandOnly )
            {
                tAxis->minimum( std::min( tAxis->minimum(), axisMin ) );
                tAxis->maximum( std::max( tAxis->maximum(), axisMax ) );
            }
            else
            {
                tAxis->minimum( axisMin );
                tAxis->maximum( axisMax );
            }
        }
    }
}

