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

    Fl_Browser* m_CustomScriptsBrowser;

    TriggerButton m_CustomScriptToggle;

private:

    int m_ScriptIndex;

};
#endif
