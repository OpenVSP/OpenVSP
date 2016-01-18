//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// PackingScreen.h: interface to control internal component packing
//
//////////////////////////////////////////////////////////////////////

#ifndef PACKINGSCREEN_H
#define PACKINGSCREEN_H

#include "ScreenMgr.h"
#include "ScreenBase.h"
#include "Vehicle.h"
#include "GuiDevice.h"

class PackingScreen : public BasicScreen
{
public:
    PackingScreen( ScreenMgr* mgr );
    virtual ~PackingScreen();

    void Show();
    bool Update();

    void CallBack( Fl_Widget *w );
    static void staticScreenCB( Fl_Widget *w, void* data )
    {
        ( ( PackingScreen* )data )->CallBack( w );
    }
    virtual void CloseCallBack( Fl_Widget *w );
    virtual void GuiDeviceCallBack( GuiDevice* device );

    bool ShowPackingScreen();

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

#endif  // PACKINGSCREEN_H
