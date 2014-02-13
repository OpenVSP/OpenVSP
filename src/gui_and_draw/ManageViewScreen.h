#ifndef _VSP_GUI_VIEW_MANAGER_SCREEN_H
#define _VSP_GUI_VIEW_MANAGER_SCREEN_H

#include "ScreenBase.h"
#include "viewScreen.h"

class ManageViewScreen : public VspScreen
{
public:
    ManageViewScreen( ScreenMgr * mgr );
    virtual ~ManageViewScreen();

public:
    virtual void Show();
    virtual void Hide();
    virtual bool Update();

    void CallBack( Fl_Widget * w );
    static void staticCB( Fl_Widget * w, void * data )
    {
        static_cast<ManageViewScreen *>( data )->CallBack( w );
    }

protected:
    ViewUI * m_ViewUI;
};
#endif