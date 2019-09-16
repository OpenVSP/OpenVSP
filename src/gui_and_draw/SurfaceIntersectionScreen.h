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

    void LoadSetChoice();

    void CallBack( Fl_Widget *w );
    virtual void CloseCallBack( Fl_Widget *w );
    static void staticScreenCB( Fl_Widget *w, void* data )
    {
        ( ( SurfaceIntersectionScreen* )data )->CallBack( w );
    }

    virtual void GuiDeviceCallBack( GuiDevice* device );

    void AddOutputText( const string &text );
    static string truncateFileName( const string &fn, int len );
    void LoadDrawObjs( vector< DrawObj* > &draw_obj_vec );

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

    //===== Global Tab Items =====//

    ToggleButton m_IntersectSubsurfaces;

    Choice m_UseSet;

    //===== Display Tab Items =====//

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

    ToggleButton m_SrfFile;
    ToggleButton m_XYZIntCurves;

    TriggerButton m_SelectCurvFile;
    TriggerButton m_SelectPlot3DFile;
    TriggerButton m_SelectSrfFile;

    StringOutput m_CurvOutput;
    StringOutput m_Plot3DOutput;
    StringOutput m_SrfOutput;

    ToggleButton m_ExportRaw;

    //===== Console Items =====//

    Fl_Text_Display *m_ConsoleDisplay;
    Fl_Text_Buffer *m_ConsoleBuffer;

    TriggerButton m_IntersectAndExport;

    ProcessUtil m_IntersectProcess;
    ProcessUtil m_MonitorProcess;

private:

    void CreateGlobalTab();
    void CreateDisplayTab();
    void CreateOutputTab();

    void UpdateGlobalTab();
    void UpdateDisplayTab();
    void UpdateOutputTab();

    void GuiDeviceGlobalTabCallback( GuiDevice* device );
    void GuiDeviceOutputTabCallback( GuiDevice* device );

    Vehicle* m_Vehicle;
};

#endif //SURFINTERSECTSCREEN_H
