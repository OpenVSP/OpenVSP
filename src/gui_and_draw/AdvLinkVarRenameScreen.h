//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// AdvLinkVarRenameScreen.h: interface for the renaming advance link variables.
//
//////////////////////////////////////////////////////////////////////

#ifndef ADVLINKVARRENAMESCREEN_H
#define ADVLINKVARRENAMESCREEN_H

#include "ScreenMgr.h"
#include "ScreenBase.h"
#include "Vehicle.h"
#include "GuiDevice.h"

#include <FL/Fl_Text_Buffer.H>
#include <FL/Fl_Text_Display.H>

using namespace std;

class AdvLinkVarRenameScreen : public BasicScreen
{
public:
    AdvLinkVarRenameScreen( ScreenMgr* mgr );
    virtual ~AdvLinkVarRenameScreen();

    void Show();
    void Hide();
    bool Update();

    void SetupAndShow( int index, const string &prevName, bool inFlag );

    void CallBack( Fl_Widget *w );
    static void staticScreenCB( Fl_Widget *w, void* data )
    {
        ( ( AdvLinkVarRenameScreen* )data )->CallBack( w );
    }

    virtual void GuiDeviceCallBack( GuiDevice* device );

protected:

    bool m_InputFlag;
    int m_VarIndex;
    string m_PrevName;

    GroupLayout m_GenLayout;
    GroupLayout m_BorderLayout;

    StringInput m_VarNameInput;

    TriggerButton m_OKPreserveCode;
    TriggerButton m_OKChangeCode;
    TriggerButton m_Cancel;
};

#endif  // ADVLINKVARRENAMESCREEN_H
