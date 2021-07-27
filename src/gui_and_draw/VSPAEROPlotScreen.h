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
    double m_PercentLegendColor;

    //==== Utility functions ====//
    //  plotting helpers
    void UpdateConvergenceAutoManualAxisLimits();
    void UpdateLoadDistAutoManualAxisLimits();
    void UpdateSweepAutoManualAxisLimits();
    void UpdateCpSliceAutoManualAxisLimits();
    void UpdateUnsteadyAutoManualAxisLimits();
    static string MakeAxisLabelStr( vector <string> dataSetNames );
    void UpdateAxisLimits( Ca_Canvas * canvas, vector <double> xDoubleData, vector <double> yDoubleData, bool expand_only );
    static void UpdateSingleAxisLimits( Ca_Axis_ * tAxis, vector <double> doubleData, bool expandOnly, bool keepZero = false );
    //  general utility
    static void ConstructFlowConditionString( char * strbuf, Results * res, bool includeResultId, bool include_recref );

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

    ToggleRadioGroup m_LoadDataTypeRadio;
    ToggleButton m_LoadTypeToggle;
    ToggleButton m_BladeTypeToggle;

    ToggleButton m_LoadDistManualXMinToggle;
    ToggleButton m_LoadDistManualXMaxToggle;
    ToggleButton m_LoadDistManualYMinToggle;
    ToggleButton m_LoadDistManualYMaxToggle;
    SliderAdjRangeInput m_LoadDistXMinSlider;
    SliderAdjRangeInput m_LoadDistXMaxSlider;
    SliderAdjRangeInput  m_LoadDistYMinSlider;
    SliderAdjRangeInput  m_LoadDistYMaxSlider;

    Fl_Browser * m_LoadDistYDataBrowser;
    Fl_Browser* m_LoadDistSelectBrowser;
    Fl_Browser * m_LoadDistFlowConditionBrowser;
    vector< string > m_LoadDistFlowConditionSelectedResultIDs;

    void UpdateLoadDistFlowConditionBrowser();
    void UpdateLoadDistYDataBrowser();
    void UpdateLoadDistSelectionBrowser();
    void RedrawLoadDistPlot();
    void PlotLoadDistribution( string resultID, vector <string> yDataSetNames, bool expand_only, int icase );

    vector < int > m_LoadSelectedBladeVec; // vector of selected rotor blades

    //==== Sweep Tab ====//
    Fl_Group* m_SweepTab;
    GroupLayout m_SweepLayout;
    GroupLayout m_SweepControlLayout;
    GroupLayout m_SweepPlotLayout;
    Ca_Canvas*  m_SweepPlotCanvas;

    GroupLayout m_SweepLegendLayout;
    Fl_Scroll* m_SweepLegendGroup;

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

    //==== Cp Slice Tab ====//
    Fl_Group* m_CpSliceTab;
    GroupLayout m_CpSliceLayout;
    GroupLayout m_CpSliceControlLayout;
    GroupLayout m_CpSlicePlotLayout;
    Ca_Canvas*  m_CpSlicePlotCanvas;

    GroupLayout m_CpSliceLegendLayout;
    Fl_Scroll* m_CpSliceLegendGroup;

    Fl_Browser * m_CpSliceCutBrowser;
    vector < Choice* > m_CpSlicePosTypeChoiceVec;
    Choice m_XCpSlicePosTypeChoice;
    Choice m_YCpSlicePosTypeChoice;
    Choice m_ZCpSlicePosTypeChoice;
    Fl_Browser * m_CpSliceCaseBrowser;
    vector< string > m_CpSliceCaseSelectedResultIDs;
    map < int, vector < string > >  m_CpSliceCutResultIDMap;
    vector< string > m_CpSliceCutSelectedResultIDs;

    ToggleButton m_CpSliceManualXMinToggle;
    ToggleButton m_CpSliceManualXMaxToggle;
    ToggleButton m_CpSliceManualYMinToggle;
    ToggleButton m_CpSliceManualYMaxToggle;
    SliderAdjRangeInput m_CpSliceXMinSlider;
    SliderAdjRangeInput m_CpSliceXMaxSlider;
    SliderAdjRangeInput  m_CpSliceYMinSlider;
    SliderAdjRangeInput  m_CpSliceYMaxSlider;

    ToggleButton m_CpSliceFlipYToggle;
    ToggleButton m_CpSlicePlotLinesToggle;

    int m_NumCpCuts; // Number of CpSlice Cuts
    int m_NumCpCases; // Number of Flow Condition Cases

    void UpdateCpSliceCaseBrowser();
    static void ConstructCpSliceCaseString( char* strbuf, Results* res, int case_num = 1 );
    void UpdateCpSliceCutBrowser();

    void RedrawCpSlicePlot();

    //==== Unsteady Tab ====//
    Fl_Group* m_UnsteadyTab;
    GroupLayout m_UnsteadyLayout;
    GroupLayout m_UnsteadyControlLayout;
    GroupLayout m_UnsteadyPlotLayout;
    Ca_Canvas*  m_UnsteadyPlotCanvas;

    GroupLayout m_UnsteadyLegendLayout;
    Fl_Scroll* m_UnsteadyLegendGroup;
    int m_UnsteadyNLines;
    int m_UnsteadyiPlot;

    Fl_Browser * m_UnsteadyYDataBrowser;
    Fl_Browser * m_UnsteadySelectBrowser;
    Fl_Browser * m_UnsteadyFlowConditionBrowser;
    vector< string > m_UnsteadyFlowConditionSelectedResultIDs;

    ToggleButton m_UnsteadyManualXMinToggle;
    ToggleButton m_UnsteadyManualXMaxToggle;
    ToggleButton m_UnsteadyManualYMinToggle;
    ToggleButton m_UnsteadyManualYMaxToggle;
    SliderAdjRangeInput m_UnsteadyXMinSlider;
    SliderAdjRangeInput m_UnsteadyXMaxSlider;
    SliderAdjRangeInput  m_UnsteadyYMinSlider;
    SliderAdjRangeInput  m_UnsteadyYMaxSlider;

    void UpdateUnsteadyFlowConditionBrowser();
    void UpdateUnsteadyYDataBrowser();
    void UpdateUnsteadySelectionBrowser();

    void RedrawUnsteadyPlot();
    void PlotUnsteady( string resultID, vector <string> yDataSetNames, bool expand_only, int icase );

    ToggleRadioGroup m_UnsteadyDataTypeRadio;
    ToggleButton m_UnsteadyHistoryToggle;
    ToggleButton m_UnsteadyGroupToggle;
    ToggleButton m_UnsteadyRotorToggle;

    vector < int > m_UnsteadySelectedTypeVec; // vector of selected unsteady groups or rotors

};

#endif  // VSPAEROPlotScreen_H
