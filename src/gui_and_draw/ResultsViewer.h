//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
// Created by Rob McDonald on 6/11/24.
//
//////////////////////////////////////////////////////////////////////

#ifndef VSP_TOP_RESULTSVIEWER_H
#define VSP_TOP_RESULTSVIEWER_H

#include "ScreenBase.h"

#include "SpreadSheetWidget.h"
#include "ResultsPlotCanvas.h"

class Results;

// Text for a Result in lists: nickname and case name when it has them, type, time and description.
string GetResultsListText( Results* res );
// Time a Result was made, as Results lists show it.
string GetResultsTimeText( Results* res );
// Keeps the checked runs among all, in the order of all, and checks only the newest when a run
// is new to known, as plots of one run at a time did, or when every checked run is gone.
// Returns whether the checked runs changed.
bool PickRuns( const vector < string > & all, vector < string > & known, vector < string > & checked );

class ResultsViewer : public TabScreen
{
public:
    ResultsViewer( ScreenMgr* mgr );
    virtual ~ResultsViewer();

    void Show();
    void Hide();
    bool Update();
    void UpdateResultsChoice();
    void UpdateDataBrowser();

    void CallBack( Fl_Widget *w );
    virtual void CloseCallBack( Fl_Widget *w );
    static void staticScreenCB( Fl_Widget *w, void* data )
    {
        ( ( ResultsViewer* )data )->CallBack( w );
    }

    virtual void GuiDeviceCallBack( GuiDevice* device );

    void DisplayGroup( GroupLayout* group );

    void SetSelectedResult( string rid );
    void SetLastResult();

    static void staticLimitsCB( void* data, double xmin, double xmax, double ymin, double ymax, bool reset )
    {
        ( ( ResultsViewer* )data )->LimitsCallBack( xmin, xmax, ymin, ymax, reset );
    }
    static void staticReadoutCB( void* data, const string & text )
    {
        ( ( ResultsViewer* )data )->ReadoutCallBack( text );
    }

protected:

    void BuildPlotTab( Fl_Group* tab );

    void UpdatePlotTab();
    void UpdatePlotChoice();
    void UpdateCurveBrowser( ResultsPlot* plot );
    void UpdatePlotResultsBrowser( ResultsCurve* curve );
    void UpdatePlotVarLists( ResultsCurve* curve );
    void UpdatePlotRunChoice( ResultsCurve* curve );
    void UpdatePlotAxes( ResultsPlot* plot );
    void RedrawPlot( ResultsPlot* plot );

    void LimitsCallBack( double xmin, double xmax, double ymin, double ymax, bool reset );
    void ReadoutCallBack( const string & text );

    void ExportPlotCSV( const string & fname );
    void SavePlotImage( string fname );

    GroupLayout m_MainLayout;
    GroupLayout m_BorderLayout;

    GroupLayout* m_CurrDisplayGroup;

    GroupLayout m_IntVectorLayout;
    GroupLayout m_DoubleVectorLayout;
    GroupLayout m_StringVectorLayout;
    GroupLayout m_Vec3dVectorLayout;
    GroupLayout m_DoubleMatrixLayout;


    Choice m_ResultsChoice;

    TriggerButton m_ExportCSVButton;
    TriggerButton m_DeleteButton;
    TriggerButton m_DeleteAllButton;
    StringInput m_NicknameInput;

    ColResizeBrowser* m_DataBrowser;

    SpreadColumnInt *m_IntVectorSpreadColumn;
    SpreadColumnDouble *m_DoubleVectorSpreadColumn;
    SpreadColumnString *m_StringVectorSpreadColumn;
    SpreadColumnVec3d *m_Vec3dVectorSpreadColumn;
    SpreadSheetDouble *m_DoubleMatrixSpreadSheet;


    vector < string > m_ResIDvec;

    vector < pair < string, int > > m_DataNameIndexVec;


    int m_ResultsViewerSelect;
    int m_DataBrowserSelect;

    //==== Plot Tab ====//
    GroupLayout m_PlotTabLayout;
    GroupLayout m_PlotControlLayout;
    GroupLayout m_PlotAreaLayout;

    Choice m_PlotChoice;
    StringInput m_PlotNameInput;
    TriggerButton m_AddPlotButton;
    TriggerButton m_CopyPlotButton;
    TriggerButton m_DeletePlotButton;
    TriggerButton m_SavePlotFileButton;
    TriggerButton m_LoadPlotFileButton;

    VspBrowser* m_CurveBrowser;
    TriggerButton m_AddCurveButton;
    TriggerButton m_DeleteCurveButton;
    TriggerButton m_DeleteAllCurvesButton;

    VspBrowser* m_PlotResultsBrowser;
    Choice m_PlotRunChoice;
    TriggerButton m_ChooseRunsButton;
    Choice m_PlotModeChoice;
    Choice m_PlotElementChoice;
    Choice m_PlotMatrixChoice;
    Choice m_PlotXChoice;
    VspBrowser* m_PlotYBrowser;

    ToggleButton m_XMinToggle;
    ToggleButton m_XMaxToggle;
    ToggleButton m_YMinToggle;
    ToggleButton m_YMaxToggle;
    Input m_XMinInput;
    Input m_XMaxInput;
    Input m_YMinInput;
    Input m_YMaxInput;
    ToggleButton m_XLogToggle;
    ToggleButton m_YLogToggle;
    TriggerButton m_AutoScaleButton;

    ToggleButton m_ShowLinesToggle;
    ToggleButton m_ShowPointsToggle;
    Choice m_LegendChoice;

    TriggerButton m_ExportPlotCSVButton;
    TriggerButton m_SavePlotButton;

    Fl_Group* m_PlotTab;
    Fl_Group* m_PlotGroup;
    ResultsPlotCanvas* m_PlotCanvas;
    Fl_Box* m_ReadoutBox;
    Fl_Box* m_WarningBox;

    vector < ResultsSource > m_PlotSources;  // Results browser lines
    vector < string > m_PlotYLabels;         // Y browser lines
    vector < string > m_PlotXLabels;         // X choice items after Index
    string m_PlotEditCurveID;                // Curve the controls showed last

    vector < ResultsSeries > m_Series;        // Series as drawn
    vector < double > m_AutoLimits;          // xmin, xmax, ymin, ymax from the data
    string m_PlotSignature;
};


#endif //VSP_TOP_RESULTSVIEWER_H
