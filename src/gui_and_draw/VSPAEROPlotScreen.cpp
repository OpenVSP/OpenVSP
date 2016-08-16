//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// VSPAEROPlotScreen.cpp: implementation of the VSPAEROPlotScreen class.
//
//////////////////////////////////////////////////////////////////////

#include "VSPAEROPlotScreen.h"
#include "APIDefines.h"
#include "StringUtil.h"
#include "FileUtil.h"
#include "float.h"

#include "Util.h"

#include <utility>
#include <string>
#include <algorithm>


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
    //  remaining space is used for the flow condition borwser
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
    int plotSideMargin = 20;
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

    GroupLayout yDataSelectLayout;
    m_LoadDistControlLayout.AddSubGroupLayout( yDataSelectLayout, m_LoadDistControlLayout.GetW(), yDataSelectHeight );
    yDataSelectLayout.AddDividerBox( "Y-Data" );
    m_LoadDistYDataBrowser = yDataSelectLayout.AddFlBrowser( yDataSelectLayout.GetRemainY() );
    m_LoadDistYDataBrowser->callback( staticScreenCB, this );
    m_LoadDistYDataBrowser->type( FL_MULTI_BROWSER );
    m_LoadDistControlLayout.AddY( yDataSelectLayout.GetH() + 2 * groupBorderWidth );

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
    m_SweepXDataDividerBox = sweepXDataSelectLayout.AddDividerBox( "X-Data" );
    m_SweepXDataBrowser = sweepXDataSelectLayout.AddFlBrowser( sweepXDataSelectLayout.GetRemainY() );
    m_SweepXDataBrowser->callback( staticScreenCB, this );
    m_SweepXDataBrowser->type( FL_MULTI_BROWSER );
    sweepXYDataSelectLayout.AddX( sweepXDataSelectLayout.GetW() );
    //  Y Data browser
    GroupLayout sweepYDataSelectLayout;
    sweepXYDataSelectLayout.AddSubGroupLayout( sweepYDataSelectLayout, sweepXYDataSelectLayout.GetW() / 2, sweepXYDataSelectLayout.GetH() );
    m_SweepYDataDividerBox = sweepYDataSelectLayout.AddDividerBox( "Y-Data" );
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

    SetDefaultView();
}

VSPAEROPlotScreen::~VSPAEROPlotScreen()
{
}

void VSPAEROPlotScreen::SetDefaultView()
{

    switch (VSPAEROMgr.m_AnalysisMethod.Get() )
    {
    case vsp::VSPAERO_ANALYSIS_METHOD::VORTEX_LATTICE:
        m_LoadDistTab->show();
        break;
    case vsp::VSPAERO_ANALYSIS_METHOD::PANEL:
        m_ConvergenceTab->show();
        //m_LoadDistTab->hide();
        break;
    default:
        break;
    }

    m_SelectDefaultData = true;
}

bool VSPAEROPlotScreen::Update()
{
    //TODO Update only the tab that is visible

    // Update single plot canvas
    UpdateConvergenceFlowConditionBrowser();
    UpdateConvergenceYDataBrowser();
    RedrawConvergencePlot();
    UpdateConvergenceAutoManualAxisLimits();

    // Update load distribution
    // Let's check to see what analysis method was used on the first result found
    // note that only VSPAEROMgr clear alls VSPAERO_* results from the results manager each time it's run all analyses in the results 'should' have the same analysis method
    string resultName = "VSPAERO_Load";
    Results* res = ResultsMgr.FindResults( resultName, 0 );
    if ( res )
    {
        // Load distribution plots are supported only in certain modes (Panel method is not currently supported)
        if ( res->FindPtr( "AnalysisMethod" )->GetInt(0) == vsp::VSPAERO_ANALYSIS_METHOD::VORTEX_LATTICE )
        {
            m_LoadDistTab->activate();
        }
        else
        {
            m_LoadDistTab->deactivate();
        }
    }
    UpdateLoadDistFlowConditionBrowser();
    UpdateLoadDistYDataBrowser();
    RedrawLoadDistPlot();
    UpdateLoadDistAutoManualAxisLimits();

    // Update sweep condition plot canvas
    UpdateSweepFlowConditionBrowser();
    UpdateSweepXYDataBrowser();
    RedrawSweepPlot();
    UpdateSweepAutoManualAxisLimits();

    m_FLTK_Window->redraw();

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
                if ( m_SweepYDataBrowser->selected( iCase ) & !oneSelected )
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
                if ( m_SweepXDataBrowser->selected( iCase ) & !oneSelected )
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
            ConstructFlowConditionString( strbuf, res, false );
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
            // Load distribution plots are supported only in certain modes (Panel method is not currently supported)
            if ( res->FindPtr( "AnalysisMethod" )->GetInt(0) == vsp::VSPAERO_ANALYSIS_METHOD::VORTEX_LATTICE )
            {
                char strbuf[1024];
                ConstructFlowConditionString( strbuf, res, false );
                m_LoadDistFlowConditionBrowser->add( strbuf );
                if( m_SelectDefaultData )   //select ALL flow conditions
                {
                    m_LoadDistFlowConditionSelectedResultIDs.push_back( res->GetID() );
                    m_LoadDistFlowConditionBrowser->select( iCase + 1 ); //account for browser using 1-based indexing
                }
                else if ( iCase < wasSelected.size() ) // restore original row selections
                {
                    if ( wasSelected[iCase] )
                    {
                        m_LoadDistFlowConditionSelectedResultIDs.push_back( res->GetID() );
                        m_LoadDistFlowConditionBrowser->select( iCase + 1 ); //account for browser using 1-based indexing
                    }
                }
            }
        }   //if( res )
    }   //for (unsigned int iCase=0; iCase<numCases; iCase++)

    m_LoadDistFlowConditionBrowser->position( scrollPos );
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
            ConstructFlowConditionString( strbuf, res, false );
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

void VSPAEROPlotScreen::ConstructFlowConditionString( char * strbuf, Results * res, bool includeResultId )
{
    if( strbuf && res )
    {
        NameValData* nvd;
        vector <double> dataVector;

        double alpha = 0;
        double beta = 0;
        double mach = 0;

        nvd = res->FindPtr( "FC_Alpha" );
        if( nvd )
        {
            dataVector = nvd->GetDoubleData();
            alpha = dataVector[dataVector.size() - 1];
        }

        nvd = res->FindPtr( "FC_Beta" );
        if( nvd )
        {
            dataVector = nvd->GetDoubleData();
            beta = dataVector[dataVector.size() - 1];
        }

        nvd = res->FindPtr( "FC_Mach" );
        if( nvd )
        {
            dataVector = nvd->GetDoubleData();
            mach = dataVector[dataVector.size() - 1];
        }

        if( includeResultId )
        {
            sprintf( strbuf, "a=%.2g, b=%.2g, M=%.2g, resID=%s", alpha, beta, mach, res->GetID().c_str() );
        }
        else
        {
            sprintf( strbuf, "a=%.2g, b=%.2g, M=%.2g", alpha, beta, mach );
        }

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
        if ( ( strcmp( dataNames[iDataName].c_str(), "FC_Mach" )  != 0 )  &
             ( strcmp( dataNames[iDataName].c_str(), "FC_Alpha" ) != 0 )  &
             ( strcmp( dataNames[iDataName].c_str(), "FC_Beta" )  != 0 )  &
             ( strcmp( dataNames[iDataName].c_str(), "WakeIter" ) != 0 )  &
             ( strcmp( dataNames[iDataName].c_str(), "Mach" )     != 0 )  &
             ( strcmp( dataNames[iDataName].c_str(), "Alpha" )    != 0 )  &
             ( strcmp( dataNames[iDataName].c_str(), "Beta" )     != 0 )  &
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

    string resultName = "VSPAERO_Load";
    string resultID = ResultsMgr.FindLatestResultsID( resultName );
     Results* res = ResultsMgr.FindResultsPtr(resultID);
    if( res )
    {
        // Load distribution plots are supported only in certain modes (Panel method is not currently supported)
        if ( res->FindPtr( "AnalysisMethod" )->GetInt(0) == vsp::VSPAERO_ANALYSIS_METHOD::VORTEX_LATTICE )
        {
            vector < string > dataNames = ResultsMgr.GetAllDataNames( resultID );
            for ( unsigned int iDataName = 0; iDataName < dataNames.size(); iDataName++ )
            {
                if ( ( strcmp( dataNames[iDataName].c_str(), "FC_Mach" )  != 0 )  &
                     ( strcmp( dataNames[iDataName].c_str(), "FC_Alpha" ) != 0 )  &
                     ( strcmp( dataNames[iDataName].c_str(), "FC_Beta" )  != 0 )  &
                     ( strcmp( dataNames[iDataName].c_str(), "WingId" )   != 0 )  &
                     ( strcmp( dataNames[iDataName].c_str(), "AnalysisMethod" ) != 0 ) )
                {
                    m_LoadDistYDataBrowser->add( dataNames[iDataName].c_str() );
                }
            }
        }
    }

    // restore original row selections
    for ( unsigned int iCase = 0; iCase < m_LoadDistYDataBrowser->size(); iCase++ )
    {
        if( ( m_SelectDefaultData && strcmp( m_LoadDistYDataBrowser->text( iCase + 1 ), "cl*c/cref" ) == 0 ) )
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
            if ( ( strcmp( dataNames[iDataName].c_str(), "FC_Mach" )  != 0 )  &
                 ( strcmp( dataNames[iDataName].c_str(), "FC_Alpha" ) != 0 )  &
                 ( strcmp( dataNames[iDataName].c_str(), "FC_Beta" )  != 0 )  &
                 ( strcmp( dataNames[iDataName].c_str(), "WakeIter" ) != 0 )  &
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


void VSPAEROPlotScreen::RedrawConvergencePlot()
{
    Ca_Canvas::current( m_ConvergencePlotCanvas );
    m_ConvergencePlotCanvas->clear();

    m_ConvergenceLegendGroup->clear();
    m_ConvergenceLegendLayout.SetGroup( m_ConvergenceLegendGroup );
    m_ConvergenceLegendLayout.InitWidthHeightVals();
    m_ConvergenceLegendLayout.SetButtonWidth( m_ConvergenceLegendLayout.GetW() * 0.75 );

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
    m_LoadDistLegendLayout.SetButtonWidth( m_LoadDistLegendLayout.GetW() * 0.75 );

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
    m_LoadDistiPlot = 0;

    // Plot only if y data has been selected
    if ( m_LoadDistNLines > 0 )
    {
        bool expandOnly = false;
        for ( unsigned int iCase = 0; iCase < m_LoadDistFlowConditionSelectedResultIDs.size(); iCase++ )
        {
            PlotLoadDistribution( m_LoadDistFlowConditionSelectedResultIDs[iCase], yDataSetNames, expandOnly, iCase );
            expandOnly = true;
        }
    }

    //m_LoadDistPlotCanvas->redraw();
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
    m_SweepLegendLayout.SetButtonWidth( m_SweepLegendLayout.GetW() * 0.75 );

    //Redraw plot if data is available and selected
    bool expandOnly = false;
    int nPolarPoints = m_SweepFlowConditionSelectedResultIDs.size();
    if ( ( nPolarPoints > 0 ) & ( xDataSetNames.size() > 0 ) & ( yDataSetNames.size() > 0 ) )
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
                        nData = res->GetNumData( xDataSetNames[iXData] );
                        xDoubleData.push_back( tResultDataPtr->GetDouble( nData - 1 ) );

                        //NameValData* tResultDataPtr;
                        tResultDataPtr = res->FindPtr( yDataSetNames[iYData] );
                        nData = res->GetNumData( yDataSetNames[iYData] );
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

        NameValData* xResultDataPtr;
        xResultDataPtr = res->FindPtr( "WakeIter" );

        NameValData* yResultDataPtr;

        string labelStr;
        for ( int iDataSet = 0; iDataSet < ( int )yDataSetNames.size(); iDataSet++ )
        {
            yResultDataPtr = res->FindPtr( yDataSetNames[iDataSet] );
            if ( ( xResultDataPtr != NULL ) & ( yResultDataPtr != NULL ) )
            {
                //often the X Data may be a vector of integers, if so convert the data to doubles first
                vector <double> xDoubleData;
                if ( xResultDataPtr->GetType() == vsp::INT_DATA )
                {
                    vector <int> tIntData = xResultDataPtr->GetIntData();
                    copy( tIntData.begin(), tIntData.end(), back_inserter( xDoubleData ) );
                }
                else
                {
                    xDoubleData = xResultDataPtr->GetDoubleData();
                }
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

                //normalize the iteration data w.r.t. the final value
                if ( m_ConvergenceYDataResidualToggle.GetFlButton()->value() == 1 )
                {
                    vector <double> diffY = yDoubleData;
                    for ( int j = 1; j < ( int )yDoubleData.size(); j++ )
                    {
                        diffY[j] = yDoubleData[j] - yDoubleData[j - 1];
                    }
                    for ( int j = 0; j < ( int )yDoubleData.size(); j++ )
                    {
                        yDoubleData[j] = log10( abs( ( diffY[j] ) ) + FLT_MIN ); //add FLT_MIN for negative inf protection on log10()
                    }
                }

                if ( xDoubleData.size() == yDoubleData.size() )
                {
                Fl_Color c = ColorWheel( m_ConvergenceiPlot, m_ConvergenceNLines );

                //add the normalized data to the plot
                AddPointLine( xDoubleData, yDoubleData, 2, c, 4, StyleWheel( m_ConvergenceiPlot ) );

                char strbuf[100];
                ConstructFlowConditionString( strbuf, res, false );
                string legendstr = strbuf + string("; Y: ") + yDataSetNames[iDataSet];
                m_ConvergenceLegendLayout.AddLegendEntry( legendstr, c );
                m_ConvergenceiPlot++;

                //Handle Axis limits
                //  Set explicit limits for interation # on X-Axis
                m_ConvergencePlotCanvas->current_x()->minimum( xDoubleData[0] );
                m_ConvergencePlotCanvas->current_x()->maximum( xDoubleData[xDoubleData.size() - 1] );
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
                else
                {
                    printf("WARNING: xDoubleData.size() != yDoubleData.size()\n\tFunction:void VSPAEROPlotScreen::PlotConvergence( string resultID, vector <string> yDataSetNames, bool expandOnly, int icase )\n\tFile:VSPAEROPlotScreen.cpp\n");
                }

                expandOnly = true;
            }
        }

        // Annotate axes
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
    else if ( strcmp( res->GetName().c_str(), "VSPAERO_Load" ) == 0 )
    {
        NameValData* wingIdResultDataPtr;
        wingIdResultDataPtr = res->FindPtr( "WingId" );

        NameValData* xResultDataPtr;
        xResultDataPtr = res->FindPtr( "Yavg" );

        NameValData* yResultDataPtr;
        string labelStr;
        for ( int iDataSet = 0; iDataSet < ( int )yDataSetNames.size(); iDataSet++ )
        {
            yResultDataPtr = res->FindPtr( yDataSetNames[iDataSet] );
            if( wingIdResultDataPtr && xResultDataPtr && yResultDataPtr )
            {
                // get unique indicies and generate loop over unique wings
                vector <int> wingIdIntDataRaw = wingIdResultDataPtr->GetIntData();
                vector <int> wingIdIntDataUnique;
                std::unique_copy( wingIdIntDataRaw.begin(), wingIdIntDataRaw.end(), std::back_inserter( wingIdIntDataUnique ) );

                Fl_Color c = ColorWheel( m_LoadDistiPlot, m_LoadDistNLines );

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
                ConstructFlowConditionString( strbuf, res, false );
                string legendstr = strbuf + string("; Y: ") + yDataSetNames[iDataSet];
                m_LoadDistLegendLayout.AddLegendEntry( legendstr, c );
                m_LoadDistiPlot++;
            }
        }

        // Annotate axes
        labelStr.clear();
        if( yDataSetNames.size() == 1 )
        {
            labelStr = yDataSetNames[0];
        }
        else
        {
            labelStr = "[multiple]";
        }
        m_LoadDistPlotCanvas->current_y()->copy_label( labelStr.c_str() );
        m_LoadDistPlotCanvas->current_x()->label( "Span Location: Y" );
    }
    else
    {
        m_LoadDistPlotCanvas->label( "PLOT ERROR - INVALID RESULT TYPE" );
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

    // Find min and max of vector
    auto dataMinMax = std::minmax_element( doubleData.begin(), doubleData.end() );
    double dataMin = dataMinMax.first[0];
    double dataMax = dataMinMax.second[0];
    double dataRange = ( dataMax - dataMin );
    double dataRangeScale = pow( 10.0, mag( dataRange ) );


    // Infinity protection - bounds the magnitude of dataMin and dataMax to single precision magnitudes
    dataMin = std::min(dataMin,(double)FLT_MAX);
    dataMin = std::max(dataMin,(double)-FLT_MAX);
    dataMax = std::min(dataMax,(double)FLT_MAX);
    dataMax = std::max(dataMax,(double)-FLT_MAX);

    // Calculate min and max number magnitudes or "scale"
    double axisMin, axisMax;
    if( keepZero )
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
    if( axisMin == axisMax || dataRange == 0 )
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

