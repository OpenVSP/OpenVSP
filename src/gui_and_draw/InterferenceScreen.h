//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// Create Edit Interference Checks
// Rob McDonald
//
//////////////////////////////////////////////////////////////////////

#if !defined(VSPTYPEINTERFERENCESCREEN__INCLUDED_)
#define VSPTYPEINTERFERENCESCREEN__INCLUDED_

#include "ScreenBase.h"
#include "GuiDevice.h"
#include "InterferenceMgr.h"

#include <FL/Fl.H>
#include <FL/Fl_Text_Buffer.H>

using std::string;
using std::vector;

class InterferenceScreen : public BasicScreen
{
public:
    InterferenceScreen( ScreenMgr* mgr );
    virtual ~InterferenceScreen();
    void Show();
    void Hide();
    bool Update();
    void UpdateInterferenceCheckBrowser();

    void CallBack( Fl_Widget *w );
    static void staticScreenCB( Fl_Widget *w, void* data )
    {
        ( static_cast <InterferenceScreen*> ( data ) )->CallBack( w );
    }

    void GuiDeviceCallBack( GuiDevice* gui_device );


protected:

    GroupLayout m_GenLayout;
    GroupLayout m_BorderLayout;

    ColResizeBrowser* m_InterferenceCheckBrowser;
    int m_InterferenceBrowserSelect;


    TriggerButton m_AddInterferenceCheck;
    TriggerButton m_DelInterferenceCheck;
    TriggerButton m_DelAllInterferenceChecks;

    StringInput m_ICNameInput;

};


#endif
