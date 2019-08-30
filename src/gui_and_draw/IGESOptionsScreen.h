//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// IGESOptionsScreen.h: interface for controlling IGES export options.
//
//////////////////////////////////////////////////////////////////////

#ifndef IGESOPTIONSSCREEN_H
#define IGESOPTIONSSCREEN_H

#include "ScreenMgr.h"
#include "ScreenBase.h"
#include "Vehicle.h"
#include "GuiDevice.h"

class IGESOptionsScreen : public BasicScreen
{
public:
    IGESOptionsScreen( ScreenMgr* mgr );
    virtual ~IGESOptionsScreen();

    void Show();
    bool Update();

    void CallBack( Fl_Widget *w );
    static void staticScreenCB( Fl_Widget *w, void* data )
    {
        ( ( IGESOptionsScreen* )data )->CallBack( w );
    }
    virtual void CloseCallBack( Fl_Widget *w );
    virtual void GuiDeviceCallBack( GuiDevice* device );

    bool ShowIGESOptionsScreen();

protected:

    GroupLayout m_GenLayout;

    Choice m_LenUnitChoice;
    ToggleButton m_SplitSurfsToggle;
    ToggleButton m_SplitSubSurfsToggle;
    ToggleButton m_ToCubicToggle;
    SliderAdjRangeInput m_ToCubicTolSlider;
    ToggleButton m_TrimTEToggle;
    ToggleButton m_PropExportOriginToggle;

    ToggleButton m_LabelIDToggle;
    ToggleButton m_LabelNameToggle;
    ToggleButton m_LabelSurfNoToggle;
    ToggleButton m_LabelSplitNoToggle;
    Choice m_LabelDelimChoice;

    int m_PrevUnit;
    bool m_PrevSplit;
    bool m_PrevSplitSub;
    bool m_PrevCubic;
    double m_PrevToCubicTol;
    bool m_PrevTrimTE;
    bool m_PrevPropExportOrigin;

    bool m_PrevLabelID;
    bool m_PrevLabelName;
    bool m_PrevLabelSurfNo;
    bool m_PrevLabelSplitNo;
    int m_PrevLabelDelim;

    TriggerButton m_OkButton;
    TriggerButton m_CancelButton;

    bool m_OkFlag;
};

#endif  // IGESOPTIONSSCREEN_H
