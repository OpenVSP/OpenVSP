//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// Create Edit Adv Links
// J.R Gloudemans
//
//////////////////////////////////////////////////////////////////////

#if !defined(VSPTYPEADVLINKSCREEN__INCLUDED_)
#define VSPTYPEADVLINKSCREEN__INCLUDED_

#include "ScreenBase.h"
#include "GuiDevice.h"

#include <FL/Fl.H>
#include "advLinkFlScreen.h"

using std::string;
using std::vector;

class AdvLinkScreen : public VspScreen
{
public:
    AdvLinkScreen( ScreenMgr* mgr );
    virtual ~AdvLinkScreen()                         {}
    void Show();
    void Hide();
    bool Update();

    void CallBack( Fl_Widget *w );
    static void staticScreenCB( Fl_Widget *w, void* data )
    {
        ( ( AdvLinkScreen* )data )->CallBack( w );
    }

    void GuiDeviceCallBack( GuiDevice* gui_device );

protected:

    AdvLinkUI* m_AdvLinkUI;
    Fl_Browser* m_LinkBrowser;
    Fl_Browser* m_InputBrowser;
    Fl_Browser* m_OutputBrowser;

    GroupLayout m_BigGroup;

    StringInput m_NameInput;

    TriggerButton m_AddLink;
    TriggerButton m_DelLink;
    TriggerButton m_DelAllLink;
    TriggerButton m_SaveLink;
    TriggerButton m_LoadLink;

    ParmPicker m_ParmPicker;
    StringInput m_VarNameInput;

    TriggerButton m_PickInput;
    TriggerButton m_PickOutput;

    GroupLayout m_InputGroup;
    GroupLayout m_OutputGroup;


    TriggerButton m_DelInput;
    TriggerButton m_DelAllInput;
    TriggerButton m_DelOutput;
    TriggerButton m_DelAllOutput;



};


#endif
