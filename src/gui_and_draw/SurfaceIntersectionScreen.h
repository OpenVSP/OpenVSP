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
    string truncateFileName( const string &fn, int len );
    void LoadDrawObjs( vector< DrawObj* > &draw_obj_vec );

    ProcessUtil* getCfdMeshProcess()
    {
        return &m_CFDMeshProcess;
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


    //===== Output Tab Items =====//

    ToggleButton m_SrfFile;
    ToggleButton m_XYZIntCurves;

    TriggerButton m_SelectSrfFile;

    StringOutput m_SrfOutput;

    //===== Console Items =====//

    Fl_Text_Display *m_ConsoleDisplay;
    Fl_Text_Buffer *m_ConsoleBuffer;

    TriggerButton m_MeshAndExport;

    ProcessUtil m_CFDMeshProcess;
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

    vector< string > m_GeomVec;
    vector< string > m_WingGeomVec;

};

#endif //SURFINTERSECTSCREEN_H
