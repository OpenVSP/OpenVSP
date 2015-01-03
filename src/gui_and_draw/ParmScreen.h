//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// parmScreen.h: interface for the geomScreen class.
// J.R. Gloudemans
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PARMSCREEN_H__015D1220_857A_11D7_AC31_0003473A025A__INCLUDED_)
#define AFX_PARMSCREEN_H__015D1220_857A_11D7_AC31_0003473A025A__INCLUDED_

#include "ScreenBase.h"
#include "GuiDevice.h"

#include "ScreenMgr.h"

using std::string;

class ParmScreen : public TabScreen
{
public:
    ParmScreen( ScreenMgr* mgr );
    virtual ~ParmScreen()           {}

    void Show();
    void Hide();
    bool Update();

    void CallBack( Fl_Widget *w );
    static void staticScreenCB( Fl_Widget *w, void* data )
    {
        ( ( ParmScreen* )data )->CallBack( w );
    }

    void GuiDeviceCallBack( GuiDevice* gui_device );

protected:


    GroupLayout m_InfoLayout;
    StringOutput m_NameString;
    StringOutput m_GroupString;
    StringOutput m_DescString;

    StringOutput m_MinValString;
    StringOutput m_CurrValString;
    StringOutput m_MaxValString;

    GroupLayout m_LinkLayout;
    Fl_Browser* m_LinkToBrowser;
    Fl_Browser* m_LinkFromBrowser;

    GroupLayout m_AdvLinkLayout;
    Fl_Browser* m_AdvLinkInputBrowser;
    Fl_Browser* m_AdvLinkOutputBrowser;

};

#endif
