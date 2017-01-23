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
    SliderInput m_ToCubicTolSlider;
    ToggleButton m_TrimTEToggle;

    int m_PrevUnit;
    bool m_PrevSplit;
    bool m_PrevSplitSub;
    bool m_PrevCubic;
    double m_PrevToCubicTol;
    bool m_PrevTrimTE;

    TriggerButton m_OkButton;
    TriggerButton m_CancelButton;

    bool m_OkFlag;
};

#endif  // IGESOPTIONSSCREEN_H
