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

#include "ScreenMgr.h"
#include "ScreenBase.h"
#include "GuiDevice.h"
#include "Cartesian.H"
#include "WaveDragMgr.h"

#include <math.h>

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

protected:

    Fl_Color icolor( int icomp );

    //==== Run Tab ====//
    GroupLayout m_RunLayout;

    SliderAdjRangeInput m_NumSlicesSlider;
    SliderAdjRangeInput m_NumRotSectsSlider;
    Choice m_SelectedSetChoice;
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

    GroupLayout m_LegendLayout;
    Fl_Scroll* m_LegendGroup;

    //==== Canvas ====//
    GroupLayout m_JustPlotLayout;

    Vsp_Canvas* m_canvas;

    //==== Flow Tab ====//
    GroupLayout m_FlowLayout;
    Fl_Check_Browser* m_SubSurfFlowBrowser;

    //==== Execute ====//
    GroupLayout m_ExecuteLayout;
    TriggerButton m_StartSlicingTrigger;
    StringOutput m_CDwResult;

    bool m_SetMaxDragFlag;

};

#endif  // WAVEDRAGSCREEN_H
