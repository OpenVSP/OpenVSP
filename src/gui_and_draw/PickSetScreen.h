//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// PickSetScreen: Export/save a particular set to a *.vsp3 file
// Initially created by J.R Gloudemans. Updated in v3 by Justin Gravett, ESAero
//
//////////////////////////////////////////////////////////////////////

#if !defined(VSPPICKSETSCREEN__INCLUDED_)
#define VSPPICKSETSCREEN__INCLUDED_

#include "ScreenBase.h"

class PickSetScreen : public BasicScreen
{
public:

    PickSetScreen( ScreenMgr* mgr );
    virtual ~PickSetScreen()                            {}
    void Show();
    void Hide();
    bool Update();

    void LoadSetChoice();

    int PickSet( const string & title );

    void GuiDeviceCallBack( GuiDevice* device );

protected:

    GroupLayout m_MainLayout;
    GroupLayout m_BorderLayout;

    Choice m_PickSetChoice;
    TriggerButton m_AcceptButton;
    TriggerButton m_CancelButton;

    int m_SelectedSetIndex;
    bool m_AcceptFlag;

};

#endif //VSPPICKSETSCREEN__INCLUDED_
