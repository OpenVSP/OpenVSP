//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

//TypeEditorScreen: View and Edit Types Of Geom
// J.R Gloudemans
//
//////////////////////////////////////////////////////////////////////

#if !defined(VSPTYPEEDITORSCREEN__INCLUDED_)
#define VSPTYPEEDITORSCREEN__INCLUDED_

#include "ScreenMgr.h"
#include "CustomGeom.h"
#include "ScreenBase.h"
#include "GuiDevice.h"

using std::string;
using std::vector;

class TypeEditorScreen : public BasicScreen
{
public:
    TypeEditorScreen( ScreenMgr* mgr );
    virtual ~TypeEditorScreen()                         {}

    void Show();
    void Hide();
    bool Update();

    virtual void GuiDeviceCallBack( GuiDevice* device );
    void CallBack( Fl_Widget *w );
    virtual void CloseCallBack( Fl_Widget* w );


protected:

    GroupLayout m_MainLayout;
    GroupLayout m_BorderLayout;

    Choice m_GeomChoice;

    Fl_Browser* m_TypeBrowser;
    Fl_Browser* m_CustomScriptsBrowser;

    TriggerButton m_AddToggle;
    TriggerButton m_DeleteToggle;
    TriggerButton m_CustomScriptToggle;

    StringInput m_TypeNameInput;

private:
    int m_TypeBrowserIndex;
    int m_GeomIndex;
    int m_ScriptIndex;

};
#endif
