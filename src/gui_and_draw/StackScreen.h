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

class StackScreen : public SkinScreen
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

    // Overall planform tab
    GroupLayout m_ModifyLayout;

    IndexSelector m_XsecModIndexSelector;

    Choice m_ChevronModeChoice;

    SliderAdjRangeInput m_ChevTopAmpSlider;
    SliderAdjRangeInput m_ChevBottomAmpSlider;
    SliderAdjRangeInput m_ChevLeftAmpSlider;
    SliderAdjRangeInput m_ChevRightAmpSlider;

    SliderInput m_ChevNumberSlider;

    SliderAdjRangeInput m_ChevOnDutySlider;
    SliderAdjRangeInput m_ChevOffDutySlider;

    Choice m_ChevronExtentModeChoice;

    SliderAdjRangeInput m_ChevW01StartSlider;
    SliderAdjRangeInput m_ChevW01EndSlider;
    SliderAdjRangeInput m_ChevW01CenterSlider;
    SliderAdjRangeInput m_ChevW01WidthSlider;

    SliderAdjRangeInput m_ChevDirTopAngleSlider;
    SliderAdjRangeInput m_ChevDirBottomAngleSlider;
    SliderAdjRangeInput m_ChevDirRightAngleSlider;
    SliderAdjRangeInput m_ChevDirLeftAngleSlider;

    SliderAdjRangeInput m_ChevDirTopSlewSlider;
    SliderAdjRangeInput m_ChevDirBottomSlewSlider;
    SliderAdjRangeInput m_ChevDirRightSlewSlider;
    SliderAdjRangeInput m_ChevDirLeftSlewSlider;

    ToggleButton m_ChevAngleAllSymButton;
    ToggleButton m_ChevAngleTBSymButton;
    ToggleButton m_ChevAngleRLSymButton;

    SliderAdjRangeInput m_ChevValleyRadSlider;
    SliderAdjRangeInput m_ChevPeakRadSlider;

};


#endif // !defined(STACKSCREEN__INCLUDED_)
