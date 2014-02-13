//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// DesignVarScreen.h: interface for the design variable menu.
//
//////////////////////////////////////////////////////////////////////

#ifndef DESIGNVARSCREEN_H
#define DESIGNVARSCREEN_H

#include "ScreenMgr.h"
#include "ScreenBase.h"
#include "Vehicle.h"
#include "GuiDevice.h"
#include "DesignVarMgr.h"

using namespace std;

class DesignVarScreen : public TabScreen
{
public:
    DesignVarScreen( ScreenMgr* mgr );
    virtual ~DesignVarScreen();

    bool Update();
    virtual void RebuildAdjustTab();

    void Show();
    void Hide();

    void CallBack( Fl_Widget *w );
    static void staticScreenCB( Fl_Widget *w, void* data )
    {
        ( ( DesignVarScreen* )data )->CallBack( w );
    }

    virtual void GuiDeviceCallBack( GuiDevice* device );

protected:

    int m_NVarLast;

    GroupLayout m_PickLayout;

    Fl_Scroll* m_AdjustGroup;
    GroupLayout m_AdjustLayout;

    ParmPicker m_ParmPicker;

    ToggleButton m_XDDMVarButton;
    ToggleButton m_XDDMConstButton;
    ToggleRadioGroup m_XDDMGroup;

    TriggerButton m_AddVarButton;
    TriggerButton m_DelVarButton;

    Fl_Browser* varBrowser;

    Choice m_FileTypeChoice;

    TriggerButton m_SaveButton;
    TriggerButton m_LoadButton;

    vector < SliderAdjRangeInput > m_ParmSliderVec;

};

#endif
