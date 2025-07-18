//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// SurfaceIntersectionScreen.h: interface for the SurfaceIntersectionMgr class.
//
//////////////////////////////////////////////////////////////////////

#ifndef SURFINTERSECTSCREEN_H
#define SURFINTERSECTSCREEN_H

#include "ScreenMgr.h"
#include "ScreenBase.h"
#include "Vehicle.h"
#include "GuiDevice.h"
#include "ProcessUtil.h"

using namespace std;

class SurfaceIntersectionScreen : public TabScreen
{
public:
    SurfaceIntersectionScreen( ScreenMgr* mgr );
    virtual ~SurfaceIntersectionScreen();

    bool Update();
    void Show();
    void Hide();

    void CallBack( Fl_Widget *w );
    virtual void CloseCallBack( Fl_Widget *w );
    static void staticScreenCB( Fl_Widget *w, void* data )
    {
        ( ( SurfaceIntersectionScreen* )data )->CallBack( w );
    }

    virtual void GuiDeviceCallBack( GuiDevice* device );

    void AddOutputText( const string &text );
    void LoadDrawObjs( vector< DrawObj* > &draw_obj_vec );
    virtual bool GetVisBndBox( BndBox &bbox );

    ProcessUtil* getIntersectProcess()
    {
        return &m_IntersectProcess;
    }

protected:

    GroupLayout m_GlobalTabLayout;
    GroupLayout m_DisplayTabLayout;
    GroupLayout m_OutputTabLayout;
    GroupLayout m_ConsoleLayout;
    GroupLayout m_BorderConsoleLayout;
    GroupLayout m_WakesTabLayout;

    Fl_Group* m_WakesTab;

    //===== Global Tab Items =====//

    ToggleButton m_IntersectSubsurfaces;

    ToggleButton m_SetToggle;
    Choice m_UseSet;
    Choice m_UseDegenSet;

    ToggleRadioGroup m_ModeSetToggleGroup;
    ToggleButton m_ModeToggle;
    Choice m_ModeChoice;
    vector < string > m_ModeIDs;

    ToggleButton m_ToCubicToggle;
    SliderAdjRangeInput m_ToCubicTolSlider;

    //===== Display Tab Items =====//

    ToggleButton m_ShowWakePreview;

    ToggleButton m_DrawIsect;
    ToggleButton m_DrawBorder;

    ToggleButton m_ShowRaw;
    ToggleButton m_ShowBinAdapt;
    SliderAdjRangeInput m_RelCurveTolSlider;

    ToggleButton m_ShowCurve;
    ToggleButton m_ShowPts;

    //===== Output Tab Items =====//

    ToggleButton m_CurvFile;
    ToggleButton m_Plot3DFile;
    ToggleButton m_IGESFile;
    ToggleButton m_STEPFile;

    ToggleButton m_SrfFile;
    ToggleButton m_XYZIntCurves;

    TriggerButton m_SelectCurvFile;
    TriggerButton m_SelectPlot3DFile;
    TriggerButton m_SelectSrfFile;
    TriggerButton m_SelectIGESFile;
    TriggerButton m_SelectSTEPFile;

    StringOutput m_CurvOutput;
    StringOutput m_Plot3DOutput;
    StringOutput m_SrfOutput;
    StringOutput m_IGESOutput;
    StringOutput m_STEPOutput;

    ToggleButton m_ExportRaw;

    SliderAdjRangeInput m_STEPTolSlider;
    //ToggleButton m_SplitSubSurfsToggle;
    //ToggleButton m_STEPMergePointsToggle;
    ToggleRadioGroup m_STEPRepGroup;
    ToggleButton m_STEPShell;
    ToggleButton m_STEPBREP;

    Choice m_LenUnitChoice;
    ToggleButton m_LabelIDToggle;
    ToggleButton m_LabelNameToggle;
    ToggleButton m_LabelSurfNoToggle;
    ToggleButton m_LabelSplitNoToggle;
    Choice m_LabelDelimChoice;

    //===== Wake Tab Items =====//

    SliderAdjRangeInput m_ScaleWake;
    SliderAdjRangeInput m_WakeAngle;

    Choice m_Comp;

    ToggleButton m_AddWake;

    //===== Console Items =====//

    Fl_Terminal *m_ConsoleDisplay;

    TriggerButton m_IntersectAndExport;

    ProcessUtil m_IntersectProcess;
    ProcessUtil m_MonitorProcess;

private:

    void CreateGlobalTab();
    void CreateDisplayTab();
    void CreateOutputTab();
    void CreateWakesTab();

    void UpdateGlobalTab();
    void UpdateDisplayTab();
    void UpdateOutputTab();
    void UpdateWakesTab();

    void GuiDeviceGlobalTabCallback( GuiDevice* device );
    void GuiDeviceOutputTabCallback( GuiDevice* device );
    void GuiDeviceWakesTabCallback( GuiDevice* device );

    Vehicle* m_Vehicle;

    vector< string > m_WingGeomVec;
};

#endif //SURFINTERSECTSCREEN_H
