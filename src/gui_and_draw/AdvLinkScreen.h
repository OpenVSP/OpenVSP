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

    string MakeDefaultName( const string & parmid );

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

    void AddInput( const string & parmid, const string & name );
    void AddOutput( const string & parmid, const string & name );

    void GuiDeviceCallBack( GuiDevice* gui_device );


    // Needs to be public.
    StringInput m_VarNameInput;

    int m_InputBrowserSelect;
    int m_OutputBrowserSelect;
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

    TriggerButton m_PickInput;
    TriggerButton m_PickOutput;

    GroupLayout m_InputGroup;
    GroupLayout m_MoveInputButtonLayout;
    GroupLayout m_InputBrowserLayout;
    GroupLayout m_OutputGroup;
    GroupLayout m_MoveOutputButtonLayout;
    GroupLayout m_OutputBrowserLayout;

    ToggleButton m_DefNameNameToggle;
    ToggleButton m_DefNameGroupToggle;
    ToggleButton m_DefNameContainerToggle;

    TriggerButton m_MoveInputUpButton;
    TriggerButton m_MoveInputDownButton;
    TriggerButton m_MoveInputTopButton;
    TriggerButton m_MoveInputBotButton;

    TriggerButton m_MoveOutputUpButton;
    TriggerButton m_MoveOutputDownButton;
    TriggerButton m_MoveOutputTopButton;
    TriggerButton m_MoveOutputBotButton;

    TriggerButton m_DelInput;
    TriggerButton m_DelAllInput;
    TriggerButton m_DelOutput;
    TriggerButton m_DelAllOutput;

    TriggerButton m_CompileCode;

    TriggerButton m_SaveCode;
    TriggerButton m_ReadCode;

};


#endif
