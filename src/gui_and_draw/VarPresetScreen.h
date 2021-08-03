//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// VarPresetScreen.h: interface for the design variable menu.
//
//////////////////////////////////////////////////////////////////////

#ifndef VARPRESETSCREEN_H
#define VARPRESETSCREEN_H

#include "ScreenMgr.h"
#include "ScreenBase.h"
#include "Vehicle.h"
#include "GuiDevice.h"
#include "VarPresetMgr.h"

#include <FL/Fl.H>
#include <FL/Fl_File_Chooser.H>

using namespace std;

class VarPresetScreen : public TabScreen
{
public:
    VarPresetScreen( ScreenMgr* mgr );
    virtual ~VarPresetScreen();

    bool Update();
    virtual void RebuildAdjustTab();
    virtual void RebuildMenus( int g_index );
    virtual void CheckSaveStatus( int g_index, int s_index );

    void Show();
    void Hide();

    void CallBack( Fl_Widget *w );
    static void staticScreenCB( Fl_Widget *w, void* data )
    {
        ( ( VarPresetScreen* )data )->CallBack( w );
    }

    virtual void GuiDeviceCallBack( GuiDevice* device );

protected:

    int m_NVarLast;

    GroupLayout m_ChangeStateLayout;
    GroupLayout m_PickLayout;

    Fl_Scroll* m_AdjustGroup;
    GroupLayout m_AdjustLayout;

    ParmPicker m_ParmPicker;
    Choice m_GroupChoice;
    Choice m_SettingChoice;
    StringInput m_GroupInput;
    StringInput m_SettingInput;

    TriggerButton m_AddVarButton;
    TriggerButton m_DelVarButton;

    ColResizeBrowser* m_VarBrowser;

    TriggerButton m_AddGroupButton;
    TriggerButton m_AddSettingButton;
    TriggerButton m_DeleteButton;
    TriggerButton m_ApplyButton;
    TriggerButton m_SaveButton;

    vector < SliderAdjRangeInput > m_ParmSliderVec;
};

#endif
