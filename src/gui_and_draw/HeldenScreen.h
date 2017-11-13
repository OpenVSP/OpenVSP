//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#ifndef HELDENSCREEN_H
#define HELDENSCREEN_H

#include "ScreenBase.h"
#include "ScreenMgr.h"
#include "GuiDevice.h"

using namespace std;

class HeldenScreen : public BasicScreen
{
public:
    HeldenScreen( ScreenMgr * mgr );
    virtual ~HeldenScreen();

    void Show();
    void Hide();
    bool Update();

    virtual void GuiDeviceCallBack( GuiDevice* device );

    virtual void CallBack( Fl_Widget* w );
    virtual void CloseCallBack( Fl_Widget *w );
    static void staticScreenCB( Fl_Widget *w, void* data )
    {
        ( ( HeldenScreen* )data )->CallBack( w );
    }

    void LoadSetChoice( Choice & c, int & index );

protected:

    GroupLayout m_MainLayout;

};
#endif //HELDENSCREEN_H
