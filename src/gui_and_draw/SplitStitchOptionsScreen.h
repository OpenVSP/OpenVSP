//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// SplitStitchOptionsScreen.h: options for the split and stitched STEP and IGES exports, which
// are the Surface Intersection settings.
//
//////////////////////////////////////////////////////////////////////

#ifndef SPLITSTITCHOPTIONSSCREEN_H
#define SPLITSTITCHOPTIONSSCREEN_H

#include "ScreenMgr.h"
#include "ScreenBase.h"
#include "Vehicle.h"
#include "GuiDevice.h"

class SplitStitchOptionsScreen : public BasicScreen
{
public:
    SplitStitchOptionsScreen( ScreenMgr* mgr );
    virtual ~SplitStitchOptionsScreen();

    void Show();
    bool Update();

    void CallBack( Fl_Widget *w );
    static void staticScreenCB( Fl_Widget *w, void* data )
    {
        ( ( SplitStitchOptionsScreen* )data )->CallBack( w );
    }
    virtual void CloseCallBack( Fl_Widget *w );
    virtual void GuiDeviceCallBack( GuiDevice* device );

    // Show the options for STEP, or for IGES; returns whether OK was pressed
    bool ShowSplitStitchOptionsScreen( bool step_flag );

protected:

    void RestorePrev();

    GroupLayout m_GenLayout;

    Choice m_LenUnitChoice;
    SliderAdjRangeInput m_TolSlider;

    ToggleButton m_STEPShell;
    ToggleButton m_STEPBREP;
    ToggleRadioGroup m_STEPRepGroup;

    ToggleButton m_ToCubicToggle;
    SliderAdjRangeInput m_ToCubicTolSlider;

    ToggleButton m_LabelIDToggle;
    ToggleButton m_LabelNameToggle;
    ToggleButton m_LabelSurfNoToggle;
    ToggleButton m_LabelSplitNoToggle;
    Choice m_LabelDelimChoice;

    TriggerButton m_OkButton;
    TriggerButton m_CancelButton;

    bool m_STEPFlag;

    int m_PrevUnit;
    double m_PrevTol;
    int m_PrevRep;
    bool m_PrevCubic;
    double m_PrevToCubicTol;
    bool m_PrevLabelID;
    bool m_PrevLabelName;
    bool m_PrevLabelSurfNo;
    bool m_PrevLabelSplitNo;
    int m_PrevLabelDelim;

    bool m_OkFlag;
};

#endif // SPLITSTITCHOPTIONSSCREEN_H
