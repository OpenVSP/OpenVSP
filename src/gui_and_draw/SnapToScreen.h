//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// SnapToScreen.h: interface to control internal component packing
//
//////////////////////////////////////////////////////////////////////

#ifndef SNAPTOSCREEN_H
#define SNAPTOSCREEN_H

#include "ScreenMgr.h"
#include "ScreenBase.h"
#include "Vehicle.h"
#include "GuiDevice.h"

class SnapToScreen : public BasicScreen
{
public:
    SnapToScreen( ScreenMgr* mgr );
    virtual ~SnapToScreen();

    void Show();
    bool Update();

    void CallBack( Fl_Widget *w );
    static void staticScreenCB( Fl_Widget *w, void* data )
    {
        ( ( SnapToScreen* )data )->CallBack( w );
    }
    virtual void CloseCallBack( Fl_Widget *w );
    virtual void GuiDeviceCallBack( GuiDevice* device );

    bool ShowSnapToScreen();

protected:

    GroupLayout m_GenLayout;
    GroupLayout m_BorderLayout;

    ParmPicker m_ParmPicker;
    SliderAdjRangeInput m_ValSlider;

    SliderAdjRangeInput m_TargetMinDistSlider;

    TriggerButton m_IncVal;
    TriggerButton m_DecVal;
    StringOutput m_MinDistOutput;

    Choice m_SetChoice;
    Choice m_MethodChoice;


 };

#endif  // SNAPTOSCREEN_H
