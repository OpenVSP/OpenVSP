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
#include <FL/Fl_Text_Buffer.H>

using std::string;
using std::vector;

class AdvLinkScreen : public BasicScreen
{
public:
    AdvLinkScreen( ScreenMgr* mgr );
    virtual ~AdvLinkScreen();
    void Show();
    void Hide();
    bool Update();

    void CallBack( Fl_Widget *w );
    static void staticScreenCB( Fl_Widget *w, void* data )
    {
        ( static_cast <AdvLinkScreen*> ( data ) )->CallBack( w );
    }

    void TextCallBack( int pos, int nInserted, int nDeleted, int nRestyled, const char* deletedText );
    static void staticTextCB( int pos, int nInserted, int nDeleted, int nRestyled, const char* deletedText, void* data )
    {
        ( static_cast <AdvLinkScreen*> ( data ) )->TextCallBack( pos, nInserted, nDeleted, nRestyled, deletedText );
    }


    void GuiDeviceCallBack( GuiDevice* gui_device );

protected:

    Fl_Browser* m_LinkBrowser;
    ColResizeBrowser* m_InputBrowser;
    ColResizeBrowser* m_OutputBrowser;
    Fl_Text_Editor* m_CodeEditor;
    Fl_Text_Buffer* m_CodeBuffer;

    
    Fl_Color m_Lt_Red;

    GroupLayout m_BigGroup;

    GroupLayout m_LinkBrowserGroup;
    GroupLayout m_LinkAddDelGroup;

    StringInput m_NameInput;

    TriggerButton m_AddLink;
    TriggerButton m_DelLink;
    TriggerButton m_DelAllLink;

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

    TriggerButton m_CompileCode;

    TriggerButton m_SaveCode;
    TriggerButton m_ReadCode;

    int m_InputBrowserSelect;
    int m_OutputBrowserSelect;

};


#endif
