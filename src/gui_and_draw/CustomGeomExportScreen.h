//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// CustomGeomExportScreen: Export a *.vsppart file from the current model
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

class CustomGeomExportScreen : public BasicScreen
{
public:
    CustomGeomExportScreen( ScreenMgr* mgr );
    virtual ~CustomGeomExportScreen()                         {}

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

    TriggerButton m_SaveScriptToggle;
    TriggerButton m_CancelToggle;

private:

    int m_ScriptIndex;

};
#endif
