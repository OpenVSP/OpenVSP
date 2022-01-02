//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// SetEditorScreen: View and Edit Geom Sets
// J.R Gloudemans
//
//////////////////////////////////////////////////////////////////////

#if !defined(VSPSETEDITORSCREEN__INCLUDED_)
#define VSPSETEDITORSCREEN__INCLUDED_

#include "ScreenBase.h"
#include "ScreenMgr.h"

using namespace std;

class SetEditorScreen : public BasicScreen
{
public:
    SetEditorScreen( ScreenMgr* mgr );
    virtual ~SetEditorScreen()                          {}
    void Show();
    void Hide();
    bool Update();

    //This callback responds to events from windows, groups, and widgets 
    void CallBack( Fl_Widget* w );
    static void staticScreenCB( Fl_Widget *w, void* data )
    {
        ( ( SetEditorScreen* )data )->CallBack( w );
    }
    //This callback responds to close widget events
    virtual void CloseCallBack( Fl_Widget* w );
    //This callback responds to events from GUI Devices buttons and input fields
    void GuiDeviceCallBack( GuiDevice* device );

protected:

    //This is used get user select and compare to existing sets. 
    //Index gets changed via callbacks and then utilized in updates 
    int m_SelectedSetIndex;
  
    //Browser widget that displays a selectable list of items 
    Fl_Browser* m_SetBrowser;
    //Check browser widget that displays a checkable list of items
    Fl_Check_Browser* m_SetSelectBrowser;

    //Main layout that holds all other GroupLayouts
    GroupLayout m_MainLayout;
    //m_BorderLayout helps center and align other GroupLayout within m_MainLayout
    GroupLayout m_BorderLayout;
    //m_LeftLayout is to be used to hold m_setBrowser
    GroupLayout m_LeftLayout;
    //m_RightLayout is to be used to hold m_SetSelectBrowser
    GroupLayout m_RightLayout;

    //This gives user an input field to name a set
    StringInput m_SetNameInput;

    //These buttons are for copying checked sets and pasting them to another set
    TriggerButton m_CopySet;
    TriggerButton m_PasteSet;

    //These buttons are for selecting all or none of the checkboxs in m_SetSelectBrowser
    TriggerButton m_SelectAll;
    TriggerButton m_UnselectAll;

    //This button is for user to execute the highlight function
    TriggerButton m_HighlightSet;

    //We use these indexs for CopySet and PasteSet Functions
    int m_CopyIndex;
    int m_PasteIndex;

    //toggle for when copying geoms if we copy sets as well
    ToggleButton m_CopySetToggle;

};
#endif //VSPSETEDITORSCREEN__INCLUDED_
