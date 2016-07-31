//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// BEMOptionsScreen.h: interface for controlling BEM export options.
//
//////////////////////////////////////////////////////////////////////

#ifndef BEMOPTIONSSCREEN_H
#define BEMOPTIONSSCREEN_H

#include "ScreenMgr.h"
#include "ScreenBase.h"
#include "Vehicle.h"
#include "GuiDevice.h"

class BEMOptionsScreen : public BasicScreen
{
public:
	BEMOptionsScreen( ScreenMgr* mgr );
    virtual ~BEMOptionsScreen();

    void Show();
    bool Update();

    void CallBack( Fl_Widget *w );
    static void staticScreenCB( Fl_Widget *w, void* data )
    {
        ( ( BEMOptionsScreen* )data )->CallBack( w );
    }
    virtual void CloseCallBack( Fl_Widget *w );
    virtual void GuiDeviceCallBack( GuiDevice* device );

    bool ShowBEMOptionsScreen();

protected:

    GroupLayout m_GenLayout;

    Choice m_LenUnitChoice;
    ToggleButton m_SplitSurfsToggle;
    ToggleButton m_ToCubicToggle;
    SliderInput m_ToCubicTolSlider;

    int m_PrevUnit;
    bool m_PrevSplit;
    bool m_PrevCubic;
    double m_PrevToCubicTol;

    TriggerButton m_OkButton;
    TriggerButton m_CancelButton;

    bool m_OkFlag;
};

#endif  // BEMOPTIONSSCREEN_H
