//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// PreferencesScreen.h: interface for setting preferences.
//
//////////////////////////////////////////////////////////////////////

#ifndef PREFERENCESSCREEN_H
#define PREFERENCESSCREEN_H

#include "ScreenMgr.h"
#include "ScreenBase.h"
#include "Vehicle.h"
#include "GuiDevice.h"

using namespace std;

class PreferencesScreen : public TabScreen
{
public:
    PreferencesScreen( ScreenMgr* mgr );
    virtual ~PreferencesScreen();

    void Show();
    void Hide();
    bool Update();

    void CallBack( Fl_Widget *w );
    static void staticScreenCB( Fl_Widget *w, void* data )
    {
        ( ( PreferencesScreen* )data )->CallBack( w );
    }

    virtual void GuiDeviceCallBack( GuiDevice* device );

    int GetFCType();
    void SetFCType( int fc );

protected:

    // The settings when the window opened, for Revert.
    int m_FCTypeAtShow;

    // General tab.
    GroupLayout m_GenLayout;
    Choice m_FCTypeChoice;

    // Bottom of the window.
    GroupLayout m_BottomLayout;
    TriggerButton m_DefaultsButton;
    TriggerButton m_RevertButton;
    TriggerButton m_CloseButton;
};

#endif  // PREFERENCESSCREEN_H
