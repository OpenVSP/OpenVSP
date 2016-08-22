//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// VSPAEROPlotScreen.h: interface for launching VSPAERO.
//
//////////////////////////////////////////////////////////////////////

#ifndef VSPAEROPLOTSCREEN_H
#define VSPAEROPLOTSCREEN_H

#include "ScreenMgr.h"
#include "ScreenBase.h"
#include "Vehicle.h"
#include "GuiDevice.h"
#include "Cartesian.H"
#include "ProcessUtil.h"
#include "VSPAEROMgr.h"

using namespace std;

class VSPAEROPlotScreen;

class VSPAEROPlotScreen : public TabScreen
{
public:
    VSPAEROPlotScreen( ScreenMgr* mgr );
    virtual ~VSPAEROPlotScreen();

    void Show();
    void Hide();
    bool Update();

    void CallBack( Fl_Widget *w );
    void CloseCallBack( Fl_Widget* w );
    static void staticScreenCB( Fl_Widget *w, void* data )
    {
        ( ( VSPAEROPlotScreen* )data )->CallBack( w );
    }

    virtual void GuiDeviceCallBack( GuiDevice* device );

    void SetDefaultView();

protected:

    bool m_SelectDefaultData;

    //==== Utility functions ====//
    //  plotting helpers
    void UpdateConvergenceAutoManualAxisLimits();
    void UpdateLoadDistAutoManualAxisLimits();
    void UpdateSweepAutoManualAxisLimits();
    string MakeAxisLabelStr( vector <string> dataSetNames );
    void UpdateAxisLimits( Ca_Canvas * canvas, vector <double> xDoubleData, vector <double> yDoubleData, bool expand_only );
    void UpdateSingleAxisLimits( Ca_Axis_ * tAxis, vector <double> doubleData, bool expandOnly, bool keepZero = false );
    //  general utility
    void ConstructFlowConditionString( char * strbuf, Results * res, bool includeResultId);

    //==== Convergence Tab ====//
    Fl_Group* m_ConvergenceTab;
    GroupLayout m_ConvergenceLayout;
    GroupLayout m_ConvergenceControlLayout;
    GroupLayout m_ConvergencePlotLayout;
    Ca_Canvas*  m_ConvergencePlotCanvas;

    GroupLayout m_ConvergenceLegendLayout;
    Fl_Scroll* m_ConvergenceLegendGroup;
    int m_ConvergenceNLines;
    int m_ConvergenceiPlot;

    ToggleButton m_ConvergenceYDataResidualToggle;
    ToggleButton m_ConvergenceYDataYLogToggle;
    Fl_Browser * m_ConvergenceYDataBrowser;
    Fl_Browser * m_ConvergenceFlowConditionBrowser;
    vector< string > m_ConvergenceFlowConditionSelectedResultIDs;

    ToggleButton m_ConvergenceManualXMinToggle;
    ToggleButton m_ConvergenceManualXMaxToggle;
    ToggleButton m_ConvergenceManualYMinToggle;
    ToggleButton m_ConvergenceManualYMaxToggle;
    SliderAdjRangeInput m_ConvergenceXMinSlider;
    SliderAdjRangeInput m_ConvergenceXMaxSlider;
    SliderAdjRangeInput  m_ConvergenceYMinSlider;
    SliderAdjRangeInput  m_ConvergenceYMaxSlider;

    void UpdateConvergenceFlowConditionBrowser();
    void UpdateConvergenceYDataBrowser();

    void RedrawConvergencePlot();
    void PlotConvergence( string resultID, vector <string> yDataSetNames, bool expand_only, int icase );

    //==== Load Distribution Tab ====//
    Fl_Group* m_LoadDistTab;
    GroupLayout m_LoadDistLayout;
    GroupLayout m_LoadDistControlLayout;
    GroupLayout m_LoadDistPlotLayout;
    Ca_Canvas*  m_LoadDistPlotCanvas;

    GroupLayout m_LoadDistLegendLayout;
    Fl_Scroll* m_LoadDistLegendGroup;
    int m_LoadDistNLines;
    int m_LoadDistiPlot;

    ToggleButton m_LoadDistManualXMinToggle;
    ToggleButton m_LoadDistManualXMaxToggle;
    ToggleButton m_LoadDistManualYMinToggle;
    ToggleButton m_LoadDistManualYMaxToggle;
    SliderAdjRangeInput m_LoadDistXMinSlider;
    SliderAdjRangeInput m_LoadDistXMaxSlider;
    SliderAdjRangeInput  m_LoadDistYMinSlider;
    SliderAdjRangeInput  m_LoadDistYMaxSlider;

    Fl_Browser * m_LoadDistYDataBrowser;
    Fl_Browser * m_LoadDistFlowConditionBrowser;
    vector< string > m_LoadDistFlowConditionSelectedResultIDs;

    void UpdateLoadDistFlowConditionBrowser();
    void UpdateLoadDistYDataBrowser();

    void RedrawLoadDistPlot();
    void PlotLoadDistribution( string resultID, vector <string> yDataSetNames, bool expand_only, int icase );
    void ExportToCSV();

    //==== Sweep Tab ====//
    Fl_Group* m_SweepTab;
    GroupLayout m_SweepLayout;
    GroupLayout m_SweepControlLayout;
    GroupLayout m_SweepPlotLayout;
    Ca_Canvas*  m_SweepPlotCanvas;
    vector <double> m_SweepPlotXminMax;

    GroupLayout m_SweepLegendLayout;
    Fl_Scroll* m_SweepLegendGroup;

    Fl_Box * m_SweepXDataDividerBox;
    Fl_Box * m_SweepYDataDividerBox;
    Fl_Browser * m_SweepXDataBrowser;
    Fl_Browser * m_SweepYDataBrowser;
    Fl_Browser * m_SweepFlowConditionBrowser;
    vector< string > m_SweepFlowConditionSelectedResultIDs;

    ToggleButton m_SweepManualXMinToggle;
    ToggleButton m_SweepManualXMaxToggle;
    ToggleButton m_SweepManualYMinToggle;
    ToggleButton m_SweepManualYMaxToggle;
    SliderAdjRangeInput m_SweepXMinSlider;
    SliderAdjRangeInput m_SweepXMaxSlider;
    SliderAdjRangeInput  m_SweepYMinSlider;
    SliderAdjRangeInput  m_SweepYMaxSlider;

    void UpdateSweepFlowConditionBrowser();
    void UpdateSweepXYDataBrowser();

    void RedrawSweepPlot();

};

#endif  // VSPAEROPlotScreen_H
