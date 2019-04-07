//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// STEPStructureOptionsScreen.h: interface for controlling STEP export options.
//
//////////////////////////////////////////////////////////////////////

#ifndef STEPSTRUCTUREOPTIONSSCREEN_H
#define STEPSTRUCTUREOPTIONSSCREEN_H

#include "ScreenMgr.h"
#include "ScreenBase.h"
#include "Vehicle.h"
#include "GuiDevice.h"

class STEPStructureOptionsScreen : public BasicScreen
{
public:
    STEPStructureOptionsScreen( ScreenMgr* mgr );
    virtual ~STEPStructureOptionsScreen();

    void Show();
    bool Update();

    void CallBack( Fl_Widget *w );
    static void staticScreenCB( Fl_Widget *w, void* data )
    {
        ( ( STEPStructureOptionsScreen* )data )->CallBack( w );
    }
    virtual void CloseCallBack( Fl_Widget *w );
    virtual void GuiDeviceCallBack( GuiDevice* device );

    bool ShowSTEPOptionsScreen();

protected:

    GroupLayout m_GenLayout;

    Choice m_StructureChoice;

    ToggleButton m_SplitSurfsToggle;
    ToggleButton m_ToCubicToggle;
    SliderInput m_ToCubicTolSlider;

    int m_PrevStructureChoice;
    bool m_PrevSplit;
    bool m_PrevCubic;
    double m_PrevToCubicTol;

    TriggerButton m_OkButton;
    TriggerButton m_CancelButton;

    bool m_OkFlag;
};

#endif  // STEPSTRUCTUREOPTIONSSCREEN_H
