//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// XRotorOptionsScreen.h: interface for controlling XRotor export options.
//
//////////////////////////////////////////////////////////////////////

#ifndef XROTOROPTIONSSCREEN_H
#define XROTOROPTIONSSCREEN_H

#include "ScreenMgr.h"
#include "ScreenBase.h"
#include "Vehicle.h"
#include "GuiDevice.h"

class XRotorOptionsScreen : public BasicScreen
{
public:
    XRotorOptionsScreen( ScreenMgr* mgr );
    virtual ~XRotorOptionsScreen();

    void Show();
    bool Update();

    void CallBack( Fl_Widget *w );
    virtual void GuiDeviceCallBack( GuiDevice* device );

    bool ShowXRotorOptionsScreen();

protected:

    GroupLayout m_GenLayout;

    GeomPicker m_PropGeomPicker;

    Choice m_LenUnitChoice;
    SliderAdjRangeInput m_VinfSlider;
    SliderAdjRangeInput m_RPMSlider;
    SliderAdjRangeInput m_AltSlider;
    ToggleButton m_FreeWakeToggle;

    SliderAdjRangeInput m_NumAeroSlider;
    SliderAdjRangeInput m_dCLdASlider;
    SliderAdjRangeInput m_CLmaxSlider;
    SliderAdjRangeInput m_CLminSlider;
    SliderAdjRangeInput m_dCLdAStallSlider;
    SliderAdjRangeInput m_dCLStallSlider;
    SliderAdjRangeInput m_CmSlider;
    SliderAdjRangeInput m_McritSlider;
    SliderAdjRangeInput m_CDminSlider;
    SliderAdjRangeInput m_CLCDminSlider;
    SliderAdjRangeInput m_dCDdCL2Slider;
    SliderAdjRangeInput m_ReRefSlider;
    SliderAdjRangeInput m_ReExpSlider;

    TriggerButton m_OkButton;
    TriggerButton m_CancelButton;

    bool m_OkFlag;
};

#endif  // XROTOROPTIONSSCREEN_H
