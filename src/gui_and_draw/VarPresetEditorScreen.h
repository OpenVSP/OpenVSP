//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// VarPresetEditorScreen.h: interface for the variable presets.
//
//////////////////////////////////////////////////////////////////////

#ifndef VARPRESETEDITORSCREEN_H
#define VARPRESETEDITORSCREEN_H

#include "ScreenMgr.h"
#include "ScreenBase.h"
#include "Vehicle.h"
#include "GuiDevice.h"
#include "VarPresetMgr.h"

#include <FL/Fl.H>
#include <FL/Fl_File_Chooser.H>

using namespace std;

class VarPresetEditorScreen : public TabScreen
{
public:
    VarPresetEditorScreen( ScreenMgr* mgr );
    virtual ~VarPresetEditorScreen();

    bool Update();
    virtual void RebuildAdjustTab();
    virtual void RebuildMenus();
    virtual void UpdateVarBrowser();
    virtual void CheckSaveStatus();
    virtual void EnableDisableWidgets();

    void Show();
    void Hide();

    void CallBack( Fl_Widget *w );
    static void staticScreenCB( Fl_Widget *w, void* data )
    {
        ( ( VarPresetEditorScreen* )data )->CallBack( w );
    }

    virtual void GuiDeviceCallBack( GuiDevice* device );

protected:

    GroupLayout m_ChangeStateLayout;
    GroupLayout m_GroupsLayout;


    // Apply tab
    Choice m_GroupChoice;
    Choice m_SettingChoice;
    TriggerButton m_ApplyButton;

    // Group tab
    StringInput m_GroupInput;
    Fl_Browser* m_GroupBrowser;

    TriggerButton m_AddGroupButton;
    TriggerButton m_RenameGroupButton;
    TriggerButton m_DeleteGroupButton;
    TriggerButton m_ClearGroupsButton;


    ParmPicker m_ParmPicker;

    TriggerButton m_AddVarButton;
    TriggerButton m_DelVarButton;
    TriggerButton m_ClearVarsButton;

    ColResizeBrowser* m_VarBrowser;


    // Setting tab


    GroupLayout m_SettingLayout;
    GroupLayout m_AdjustLayout;

    StringInput m_SettingInput;
    Fl_Browser* m_SettingBrowser;

    TriggerButton m_Apply2Button;
    TriggerButton m_AddSettingButton;
    TriggerButton m_RenameSettingButton;
    TriggerButton m_DeleteSettingButton;
    TriggerButton m_ClearSettingsButton;

    TriggerButton m_UpdateSettingButton;

    Fl_Scroll* m_AdjustScrollGroup;

    vector < SliderAdjRangeInput > m_ParmSliderVec;
    std::size_t m_ParmListHash;



    int m_GroupIndex;
    string m_PrevGID;
    vector < string > m_GroupIDs;

    int m_SettingIndex;
    string m_PrevSID;
    vector < string > m_SettingIDs;

    int m_VarIndex;
    vector < string > m_ParmIDs;
};

#endif
