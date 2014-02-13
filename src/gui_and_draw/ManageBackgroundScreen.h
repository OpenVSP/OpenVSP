#ifndef _VSP_GUI_BACKGROUND_MANAGER_SCREEN_H
#define _VSP_GUI_BACKGROUND_MANAGER_SCREEN_H

#include "backgroundScreen.h"
#include "ScreenBase.h"

class ScreenMgr;
class ManageBackgroundScreen : public VspScreen
{
public:
    ManageBackgroundScreen( ScreenMgr * mgr );
    virtual ~ManageBackgroundScreen();

public:
    virtual void Show();
    virtual void Hide();
    virtual bool Update();

    void CallBack( Fl_Widget * w );
    static void staticCB( Fl_Widget * w, void * data )
    {
        static_cast<ManageBackgroundScreen *>( data )->CallBack( w );
    }

protected:
    BackgroundUI * m_BackgroundUI;
};
#endif