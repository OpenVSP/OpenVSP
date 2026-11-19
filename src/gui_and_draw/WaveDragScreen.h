//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
// WaveDragScreen.h
//
// Michael Waddington
//////////////////////////////////////////////////////////////////////

#ifndef WAVEDRAGSCREEN_H
#define WAVEDRAGSCREEN_H

#include <cmath>

#include "ScreenMgr.h"
#include "ScreenBase.h"
#include "ResultsPlotCanvas.h"
#include "GuiDevice.h"
#include "Cartesian.H"
#include "WaveDragMgr.h"

using namespace std;

class WaveDragScreen : public TabScreen
{
public:
    WaveDragScreen( ScreenMgr* mgr );
    virtual ~WaveDragScreen();

    void Show();
    void Hide();

    bool Update();

    void CallBack( Fl_Widget *w );
    void CloseCallBack( Fl_Widget* w );
    static void staticScreenCB( Fl_Widget *w, void* data )
    {
        ( ( WaveDragScreen* )data )->CallBack( w );
    }

    virtual void GuiDeviceCallBack( GuiDevice* device );

    void RedrawPlot();

    void LoadDrawObjs(vector< DrawObj* > & draw_obj_vec);

    bool hittest( int mx, double datax, int r );

    // List the runs and pick those to plot: the newest when a run is new, then as checked.
    void UpdateRuns();

    // Name of a run: its nickname or "Run N", N counting every listed run.
    string RunLabel( const string & run_id );

    static void staticLimitsCB( void* data, double xmin, double xmax, double ymin, double ymax, bool reset )
    {
        ( ( WaveDragScreen* )data )->LimitsCallBack( xmin, xmax, ymin, ymax, reset );
    }
    static bool staticPressCB( void* data, int event, int mx, int my )
    {
        return ( ( WaveDragScreen* )data )->PressCallBack( event, mx, my );
    }
    void LimitsCallBack( double xmin, double xmax, double ymin, double ymax, bool reset );
    bool PressCallBack( int event, int mx, int my );

protected:

    vector < string > m_ModeIDs;

    //==== Run Tab ====//
    GroupLayout m_RunLayout;

    SliderAdjRangeInput m_NumSlicesSlider;
    SliderAdjRangeInput m_NumRotSectsSlider;

    ToggleButton m_SetToggle;
    Choice m_SelectedSetChoice;

    ToggleRadioGroup m_ModeSetToggleGroup;
    ToggleButton m_ModeToggle;
    Choice m_ModeChoice;

    SliderAdjRangeInput m_MachNumberSlider;
    TriggerButton m_FileTrigger;
    StringOutput m_FileSelect;


    Choice m_RefWingChoice;
    vector <string> m_WingGeomVec;

    ToggleButton m_RefManualToggle;
    ToggleButton m_RefChoiceToggle;
    ToggleRadioGroup m_RefToggle;
    SliderAdjRangeInput m_SrefSlider;


    ToggleButton m_SymmToggle;

    //==== Plot Tab ====//
    GroupLayout m_PlotLayout;

    IndexSelector m_ThetaIndexSelector;
    StringOutput m_ThetaDegValResult;
    ToggleButton m_PlaneToggle;
    SliderAdjRangeInput m_SlicingLocSlider;
    TriggerButton m_LocDragTrigger;
    TriggerButton m_GlobalDragTrigger;
    Choice m_SelectedBodyRev;

    bool m_Dragging;

    ToggleButton m_AreaTotalButton;
    ToggleButton m_AreaComponentsButton;
    ToggleButton m_AreaBuildupButton;
    ToggleRadioGroup m_AreaGroup;

    ToggleButton m_PointToggle;


    //==== Canvas ====//
    GroupLayout m_JustPlotLayout;

    ResultsPlotCanvas* m_canvas;

    bool m_Zoomed;          // The plot shows zoomed or panned limits, not its own
    double m_ZoomXMin;
    double m_ZoomXMax;
    double m_ZoomYMin;
    double m_ZoomYMax;

    //==== Flow Tab ====//
    GroupLayout m_FlowLayout;
    Fl_Check_Browser* m_SubSurfFlowBrowser;

    //==== Runs Tab ====//
    GroupLayout m_RunsLayout;
    ColCheckBrowser* m_RunsBrowser;
    vector < string > m_ListedRunIDs;    // Runs browser lines after the header: every WaveDrag Result
    vector < string > m_KnownRunIDs;     // Every WaveDrag Result when the runs were last updated
    vector < string > m_RunIDs;          // Runs plotted, oldest first
    string m_NicknameRunID;              // Run whose name is being edited

    //==== Execute ====//
    GroupLayout m_ExecuteLayout;
    TriggerButton m_StartSlicingTrigger;
    StringOutput m_CDwResult;

    bool m_SetMaxDragFlag;

};

#endif  // WAVEDRAGSCREEN_H
