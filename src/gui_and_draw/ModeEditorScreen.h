//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// ModeEditorScreen: View and Edit Modes
// Rob McDonald
//
//////////////////////////////////////////////////////////////////////

#if !defined(VSPMODEEDITORSCREEN__INCLUDED_)
#define VSPMODEEDITORSCREEN__INCLUDED_

#include "ScreenBase.h"
#include "ScreenMgr.h"

using namespace std;

class ModeEditorScreen : public BasicScreen
{
public:
    ModeEditorScreen( ScreenMgr* mgr );
    virtual ~ModeEditorScreen()                          {}
    void Show();
    void Hide();
    bool Update();

    void UpdateModeBrowser();
    void UpdateSettingBrowser();
    void UpdateVarPresetChoices();

    void CallBack( Fl_Widget* w );
    static void staticScreenCB( Fl_Widget *w, void* data )
    {
        ( ( ModeEditorScreen* )data )->CallBack( w );
    }

    virtual void CloseCallBack( Fl_Widget* w );

    void GuiDeviceCallBack( GuiDevice* device );

protected:

    int m_SelectedModeIndex;
    ColResizeBrowser* m_ModeBrowser;
    vector < string > m_ModeIDs;

    int m_SelectedSettingIndex;
    ColResizeBrowser* m_SettingBrowser;
    int m_NumSetting;

    GroupLayout m_MainLayout;
    GroupLayout m_GenLayout;


    StringInput m_ModeNameInput;

    TriggerButton m_AddMode;
    TriggerButton m_DeleteMode;
    TriggerButton m_DeleteAllModes;

    Choice m_NormalSetChoice;
    Choice m_DegenSetChoice;

    Choice m_GroupChoice;
    vector < string > m_GroupIDs;
    int m_GroupChoiceIndex;
    Choice m_SettingChoice;
    vector < string > m_SettingIDs;
    int m_SettingChoiceIndex;

    TriggerButton m_AddSetting;
    TriggerButton m_DeleteSetting;
    TriggerButton m_DeleteAllSettings;

    TriggerButton m_ApplyAndShowOnlyMode;

};
#endif //VSPMODEEDITORSCREEN__INCLUDED_
