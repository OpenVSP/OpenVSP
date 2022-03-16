//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// FuselageScreen.h: UI for Fuselage Geom
// J.R Gloudemans
//
//////////////////////////////////////////////////////////////////////

#if !defined(STACKSCREEN__INCLUDED_)
#define STACKSCREEN__INCLUDED_

#include "ScreenBase.h"
#include "GuiDevice.h"
#include "GroupLayout.h"

#include <FL/Fl.H>

class StackScreen : public ChevronScreen
{
public:
    StackScreen( ScreenMgr* mgr );
    virtual ~StackScreen();

    virtual void Show();
    virtual bool Update();

    virtual void CallBack( Fl_Widget *w );
    virtual void GuiDeviceCallBack( GuiDevice* d );

protected:

    GroupLayout m_DesignLayout;

    Choice m_DesignPolicyChoice;

    Choice m_NoseCapTypeChoice;
    SliderAdjRangeInput m_NoseCapLenSlider;
    SliderAdjRangeInput m_NoseCapOffsetSlider;
    SliderAdjRangeInput m_NoseCapStrengthSlider;
    ToggleButton m_NoseCapSweepFlagButton;

    Choice m_TailCapTypeChoice;
    SliderAdjRangeInput m_TailCapLenSlider;
    SliderAdjRangeInput m_TailCapOffsetSlider;
    SliderAdjRangeInput m_TailCapStrengthSlider;
    ToggleButton m_TailCapSweepFlagButton;

    SliderAdjRangeInput m_CapTessSlider;

    TriggerButton m_InsertXSec;
    TriggerButton m_CutXSec;
    TriggerButton m_CopyXSec;
    TriggerButton m_PasteXSec;

    SliderAdjRangeInput m_SectUTessSlider;

    SliderAdjRangeInput m_XSecXDeltaSlider;
    SliderAdjRangeInput m_XSecYDeltaSlider;
    SliderAdjRangeInput m_XSecZDeltaSlider;
    SliderAdjRangeInput m_XSecXRotSlider;
    SliderAdjRangeInput m_XSecYRotSlider;
    SliderAdjRangeInput m_XSecZRotSlider;
    SliderAdjRangeInput m_XSecSpinSlider;

};


#endif // !defined(STACKSCREEN__INCLUDED_)
