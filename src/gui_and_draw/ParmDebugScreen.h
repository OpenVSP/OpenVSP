//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// ParmDebugScreen.h: interface for the ParmDebugScreen class.
//
//////////////////////////////////////////////////////////////////////

#ifndef PARMDEBUGSCREEN_H
#define PARMDEBUGSCREEN_H

#include "ScreenMgr.h"
#include "ScreenBase.h"
#include "Vehicle.h"
#include "GuiDevice.h"

#include <FL/Fl_Text_Buffer.H>
#include <FL/Fl_Text_Display.H>

#define DES_FILE 0
#define XDDM_FILE 1

using namespace std;

class ParmDebugScreen : public BasicScreen
{
public:
    ParmDebugScreen( ScreenMgr* mgr );
    virtual ~ParmDebugScreen();

    void Show();
    void Hide();
    bool Update();

    void CallBack( Fl_Widget *w );
    static void staticScreenCB( Fl_Widget *w, void* data )
    {
        ( ( ParmDebugScreen* )data )->CallBack( w );
    }

    virtual void GuiDeviceCallBack( GuiDevice* device );

protected:

    GroupLayout m_GenLayout;

    ParmPicker m_ParmPicker;
    SliderAdjRangeInput m_ParmSlider;

    StringOutput m_ContainerOutput;
    StringOutput m_GroupOutput;
    StringOutput m_ParmOutput;
    StringOutput m_DescOutput;
    StringOutput m_MinOutput;
    StringOutput m_MaxOutput;
    StringOutput m_ValOutput;
};

#endif  // PARMPICKERSCREEN_H
