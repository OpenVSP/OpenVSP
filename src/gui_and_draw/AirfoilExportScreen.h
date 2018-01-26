//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// AirfoilExportScreen.h: interface for controlling airfoil export options.
//
//////////////////////////////////////////////////////////////////////

#ifndef AIRFOILEXPORTSCREEN_H
#define AIRFOILEXPORTSCREEN_H

#include "ScreenMgr.h"
#include "ScreenBase.h"
#include "Vehicle.h"
#include "GuiDevice.h"

class AirfoilExportScreen : public BasicScreen
{
    public:
    AirfoilExportScreen( ScreenMgr* mgr );
    virtual ~AirfoilExportScreen();

    void Show();
    bool Update();

    void CallBack( Fl_Widget *w );

    virtual void GuiDeviceCallBack( GuiDevice* device );

    bool ShowAirfoilExportScreen();

    protected:

    GroupLayout m_GenLayout;

    ToggleButton m_SeligToggle;
    ToggleButton m_BezierToggle;
    ToggleRadioGroup m_AirfoilTypeGroup;

    ToggleButton m_AppendGeomIDToggle;

    SliderAdjRangeInput m_WTessFactorSlider;

    TriggerButton m_OkButton;
    TriggerButton m_CancelButton;

    bool m_OkFlag;
};

#endif // AIRFOILEXPORTSCREEN_H
