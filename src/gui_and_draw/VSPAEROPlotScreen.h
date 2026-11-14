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
#include "ResultsPlotCanvas.h"

#include <map>

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

    void ApplyDefaultView();

    bool m_DefaultViewPending;
    bool m_SelectDefaultData;
    std::map < Fl_Browser*, vector < string > > m_ListedRowKeys;  // Key of each row a browser lists
    double m_PercentLegendColor;

    //==== Utility functions ====//
    //  plotting helpers
    void UpdateConvergenceAutoManualAxisLimits();
    void UpdateLoadDistAutoManualAxisLimits();
    void UpdateSweepAutoManualAxisLimits();
    void UpdateCpSliceAutoManualAxisLimits();
    void UpdateUnsteadyAutoManualAxisLimits();
    static string MakeAxisLabelStr( const vector <string> &dataSetNames );
    static void UpdateAxisLimits( Ca_Canvas * canvas, const vector <double> &xDoubleData, const vector <double> &yDoubleData, bool expand_only );
    static void UpdateSingleAxisLimits( Ca_Axis_ * tAxis, const vector <double> &doubleData, bool expandOnly, bool keepZero = false );
    //  general utility
    // Pick the runs to plot: of each kind, the newest when a run is new, then as checked.
    void UpdateRuns();

    // Results of name among the plotted runs' Results, oldest run first.  Cp slices come from the
    // plotted Cp slice sources; other Results not made by a VSPAERO run from all Results.
    void GetRunResultIDs( const string & name, vector < string > & ids );
    int NumRunResults( const string & name );
    Results* FindRunResults( const string & name, int index = 0 );
    string FindRunResultsID( const string & name, int index = 0 );
    string FindLatestRunResultsID( const string & name );

    // Name of a run: its nickname or "Run N", N counting every listed run.  RunPrefix is empty
    // while one run is plotted; CpSlicePrefix while one Cp slice source is.
    string RunLabel( const string & run_id );
    string RunPrefix( Results* res );
    string CpSlicePrefix( const string & case_id );

    // Style of series iseries of nseries drawn from res.  With several runs plotted, the run
    // sets marker and dash as the Results plot's source does.
    void RunSeriesStyle( Results* res, int iseries, int nseries, Fl_Color & color, int & linestyle, int & marker );

    // Lists each flow condition of the plotted runs' Results named name once, or only the first
    // condition, which is then selected.  Rows keep their selection by text.  selected_ids gets
    // the Results of the selected rows, row by row, row_of_id their row among the selected rows,
    // and num_rows the number of selected rows.
    void UpdateConditionBrowser( Fl_Browser* browser, const string & name, bool first_only, vector < string > & selected_ids,
                                 std::map < string, int > & row_of_id, int & num_rows );
    // Lists the Results ids one row per distinct key, as UpdateConditionBrowser does.
    void ListUniqueRows( Fl_Browser* browser, const vector < string > & ids, const vector < string > & keys, const vector < string > & texts,
                         bool first_only, vector < string > & selected_ids, std::map < string, int > & row_of_id, int & num_rows );
    static string FlowConditionKey( Results* res );

    void ConstructFlowConditionString( char *strbuf, size_t str_siz, Results *res, bool includeResultId, bool include_recref );

    //==== Convergence Tab ====//
    Fl_Group* m_ConvergenceTab;
    GroupLayout m_ConvergenceLayout;
    GroupLayout m_ConvergenceControlLayout;
    GroupLayout m_ConvergencePlotLayout;
    ResultsPlotCanvas* m_ConvergencePlotCanvas;
    PlotLimitParms m_ConvergenceLimits;

    GroupLayout m_ConvergenceLegendLayout;
    Fl_Scroll* m_ConvergenceLegendGroup;
    int m_ConvergenceNLines;
    int m_ConvergenceiPlot;

    ToggleButton m_ConvergenceYDataResidualToggle;
    VspBrowser* m_ConvergenceYDataBrowser;
    VspBrowser* m_ConvergenceFlowConditionBrowser;
    vector< string > m_ConvergenceFlowConditionSelectedResultIDs;
    std::map < string, int > m_ConvergenceConditionRow;
    int m_ConvergenceNumConditions;

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
    void PlotConvergence( const string &resultID, vector <string> yDataSetNames, bool expand_only, int icase );

    //==== Load Distribution Tab ====//
    Fl_Group* m_LoadDistTab;
    GroupLayout m_LoadDistLayout;
    GroupLayout m_LoadDistControlLayout;
    GroupLayout m_LoadDistPlotLayout;
    ResultsPlotCanvas* m_LoadDistPlotCanvas;
    PlotLimitParms m_LoadDistLimits;

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

    IndexSelector m_LoadDistTimeIndexSelector;
    StringOutput m_LoadDistTimeOutput;

    VspBrowser* m_LoadDistYDataBrowser;
    VspBrowser* m_LoadDistSelectBrowser;
    VspBrowser* m_LoadDistFlowConditionBrowser;
    vector< string > m_LoadDistFlowConditionSelectedResultIDs;
    std::map < string, int > m_LoadDistConditionRow;
    int m_LoadDistNumConditions;

    void UpdateLoadDistFlowConditionBrowser();
    void UpdateLoadDistYDataBrowser();
    void UpdateLoadDistSelectionBrowser();
    void RedrawLoadDistPlot();
    void PlotLoadDistribution( const string &resultID, vector <string> yDataSetNames, bool expand_only, int icase );

    vector < int > m_LoadSelectedBladeVec; // vector of selected rotor blades

    //==== Sweep Tab ====//
    Fl_Group* m_SweepTab;
    GroupLayout m_SweepLayout;
    GroupLayout m_SweepControlLayout;
    GroupLayout m_SweepPlotLayout;
    ResultsPlotCanvas* m_SweepPlotCanvas;
    PlotLimitParms m_SweepLimits;

    GroupLayout m_SweepLegendLayout;
    Fl_Scroll* m_SweepLegendGroup;

    VspBrowser* m_SweepXDataBrowser;
    VspBrowser* m_SweepYDataBrowser;
    VspBrowser* m_SweepFlowConditionBrowser;
    vector< string > m_SweepFlowConditionSelectedResultIDs;
    std::map < string, int > m_SweepConditionRow;
    int m_SweepNumConditions;

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
    ResultsPlotCanvas* m_CpSlicePlotCanvas;
    PlotLimitParms m_CpSliceLimits;

    GroupLayout m_CpSliceLegendLayout;
    Fl_Scroll* m_CpSliceLegendGroup;

    VspBrowser* m_CpSliceCutBrowser;
    vector < Choice* > m_CpSlicePosTypeChoiceVec;
    Choice m_XCpSlicePosTypeChoice;
    Choice m_YCpSlicePosTypeChoice;
    Choice m_ZCpSlicePosTypeChoice;
    VspBrowser* m_CpSliceCaseBrowser;
    vector< string > m_CpSliceCaseSelectedResultIDs;
    std::map < string, int > m_CpSliceCaseRow;
    int m_CpSliceNumCases;
    vector< string > m_CpSliceCutSelectedResultIDs;
    std::map < string, int > m_CpSliceCutRow;
    int m_CpSliceNumCuts;

    vector < string > m_CpSliceSourceIDs;           // CpSlice_Wrapper Results plotted, oldest first
    std::map < string, int > m_CpSliceSourceOfCase;  // Index in m_CpSliceSourceIDs of each slice
    std::map < string, string > m_CpSliceOwner;     // Runs list entry of each source

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


    void UpdateCpSliceCaseBrowser();
    static void ConstructCpSliceCaseString( char *strbuf, size_t strsiz, Results *res );
    static void ConstructCpSliceCutString( char *strbuf, size_t strsiz, Results *res );
    void UpdateCpSliceCutBrowser();

    void RedrawCpSlicePlot();

    //==== Unsteady Tab ====//
    Fl_Group* m_UnsteadyTab;

    //==== Runs Tab ====//
    Fl_Group* m_RunsTab;
    GroupLayout m_RunsLayout;
    ColCheckBrowser* m_RunsBrowser;
    TriggerButton m_RunsNewestButton;
    TriggerButton m_RunsAllButton;

    vector < string > m_RunIDs;          // VSPAERO_Wrapper Results plotted, oldest first
    vector < string > m_KnownRunIDs;     // Every VSPAERO_Wrapper Result when the runs were last updated
    vector < string > m_SliceRunIDs;     // Cp slices made apart from a run plotted, oldest first
    vector < string > m_KnownSliceIDs;   // Every Cp slice made apart from a run
    vector < string > m_ListedRunIDs;    // Runs browser lines after the header
    string m_NicknameRunID;              // Run whose name is being edited
    std::map < string, int > m_RunOfResult;  // Index in m_RunIDs of the run each Result belongs to
    GroupLayout m_UnsteadyLayout;
    GroupLayout m_UnsteadyControlLayout;
    GroupLayout m_UnsteadyPlotLayout;
    ResultsPlotCanvas* m_UnsteadyPlotCanvas;
    PlotLimitParms m_UnsteadyLimits;

    GroupLayout m_UnsteadyLegendLayout;
    Fl_Scroll* m_UnsteadyLegendGroup;
    int m_UnsteadyNLines;
    int m_UnsteadyiPlot;

    VspBrowser* m_UnsteadyYDataBrowser;
    VspBrowser* m_UnsteadySelectBrowser;
    VspBrowser* m_UnsteadyFlowConditionBrowser;
    vector< string > m_UnsteadyFlowConditionSelectedResultIDs;
    std::map < string, int > m_UnsteadyConditionRow;
    int m_UnsteadyNumConditions;

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
    void PlotUnsteady( const string &resultID, vector <string> yDataSetNames, bool expand_only, int icase );

    ToggleRadioGroup m_UnsteadyDataTypeRadio;
    ToggleButton m_UnsteadyHistoryToggle;
    ToggleButton m_UnsteadyGroupToggle;
    ToggleButton m_UnsteadyRotorToggle;

    vector < int > m_UnsteadySelectedTypeVec; // vector of selected unsteady groups or rotors

};

#endif  // VSPAEROPlotScreen_H
