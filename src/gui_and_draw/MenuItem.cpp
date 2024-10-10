
#include <MenuItem.h>
#include "ScreenMgr.h"

BaseMenuItem::BaseMenuItem()
{
    m_MenuBar = nullptr;
}

void BaseMenuItem::Hide()
{
    if ( m_MenuBar )
    {
        Fl_Menu_Item * mi = (Fl_Menu_Item*) m_MenuBar->find_item( m_MenuPath.c_str() );
        if ( mi )
        {
            mi->hide();
        }
    }
}

void BaseMenuItem::Show()
{
    if ( m_MenuBar )
    {
        Fl_Menu_Item * mi = (Fl_Menu_Item*) m_MenuBar->find_item( m_MenuPath.c_str() );
        if ( mi )
        {
            mi->show();
        }
    }
}

bool BaseMenuItem::IsShown()
{
    if ( m_MenuBar )
    {
        Fl_Menu_Item * mi = (Fl_Menu_Item*) m_MenuBar->find_item( m_MenuPath.c_str() );
        if ( mi )
        {
            return mi->visible();
        }
    }
    return false;
}

ScreenMenuItem::ScreenMenuItem()
{
    m_ScreenMgr = nullptr;
    m_ScreenID = -1;
    m_SecondaryScreenID = -1;
}

void ScreenMenuItem::Init( ScreenMgr *mgr, Fl_Sys_Menu_Bar *mbar, std::string mpath, int sid, int secondary_sid )
{
    m_ScreenMgr = mgr;
    m_MenuBar = mbar;
    m_MenuPath = mpath;
    m_ScreenID = sid;
    m_SecondaryScreenID = secondary_sid;

    m_MenuBar->add( m_MenuPath.c_str(), "", staticShowScreenCB, this );

}

void ScreenMenuItem::ShowScreenCallBack( void* data )
{
    assert( m_ScreenMgr );
    m_ScreenMgr->ShowScreen( m_ScreenID );
    if ( m_SecondaryScreenID >= 0 )
    {
        m_ScreenMgr->ShowScreen( m_SecondaryScreenID );
    }
}

ActionMenuItem::ActionMenuItem()
{
    m_ActionScreen = nullptr;
}

void ActionMenuItem::Init( ActionScreen *ascreen, Fl_Sys_Menu_Bar *mbar, std::string mpath, int scut )
{
    m_ActionScreen = ascreen;
    m_MenuBar = mbar;
    m_MenuPath = mpath;

    m_MenuBar->add( m_MenuPath.c_str(), scut, staticActionCB, this );

}

void ActionMenuItem::ActionCallBack( void* data )
{
    m_ActionScreen->ActionCB( data );
}

void VSPCheckMenuItem::Init( ActionScreen *ascreen, Fl_Sys_Menu_Bar *mbar, std::string mpath, int scut )
{
    m_ActionScreen = ascreen;
    m_MenuBar = mbar;
    m_MenuPath = mpath;

    m_MenuBar->add( m_MenuPath.c_str(), scut, staticActionCB, this, FL_MENU_TOGGLE );
}

void VSPCheckMenuItem::Update( bool val )
{
    Fl_Menu_Item * mitem = (Fl_Menu_Item *) m_MenuBar->find_item( m_MenuPath.c_str() );

    if ( mitem )
    {
        if ( val )
        {
            mitem->check();
        }
        else
        {
            mitem->uncheck();
        }
    }
}
