//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// STEPOptionsScreen.h: interface for controlling STEP export options.
//
//////////////////////////////////////////////////////////////////////

#ifndef STEPOPTIONSSCREEN_H
#define STEPOPTIONSSCREEN_H

#include "ScreenMgr.h"
#include "ScreenBase.h"
#include "Vehicle.h"
#include "GuiDevice.h"

class STEPOptionsScreen : public BasicScreen
{
public:
    STEPOptionsScreen( ScreenMgr* mgr );
    virtual ~STEPOptionsScreen();

    void Show();
    bool Update();

    void CallBack( Fl_Widget *w );
    static void staticScreenCB( Fl_Widget *w, void* data )
    {
        ( ( STEPOptionsScreen* )data )->CallBack( w );
    }
    virtual void CloseCallBack( Fl_Widget *w );
    virtual void GuiDeviceCallBack( GuiDevice* device );

    bool ShowSTEPOptionsScreen();

protected:

    GroupLayout m_GenLayout;

    Choice m_LenUnitChoice;
    SliderInput m_TolSlider;
    ToggleButton m_SplitSurfsToggle;
    ToggleButton m_SplitSubSurfsToggle;
    ToggleButton m_MergePointsToggle;
    ToggleButton m_ToCubicToggle;
    SliderInput m_ToCubicTolSlider;
    ToggleButton m_TrimTEToggle;
    ToggleButton m_PropExportOriginToggle;

    ToggleButton m_LabelIDToggle;
    ToggleButton m_LabelNameToggle;
    ToggleButton m_LabelSurfNoToggle;
    Choice m_LabelDelimChoice;

    int m_PrevUnit;
    double m_PrevTol;
    bool m_PrevSplit;
    bool m_PrevSplitSub;
    bool m_PrevMerge;
    bool m_PrevCubic;
    double m_PrevToCubicTol;
    bool m_PrevTrimTE;
    bool m_PrevPropExportOrigin;

    bool m_PrevLabelID;
    bool m_PrevLabelName;
    bool m_PrevLabelSurfNo;
    int m_PrevLabelDelim;

    TriggerButton m_OkButton;
    TriggerButton m_CancelButton;

    bool m_OkFlag;
};

#endif  // STEPOPTIONSSCREEN_H
