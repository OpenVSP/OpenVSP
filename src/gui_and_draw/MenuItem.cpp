
#include <MenuItem.h>
#include "ScreenMgr.h"

void ScreenMenuItem::Init( ScreenMgr *mgr, Fl_Sys_Menu_Bar *mbar, std::string mpath, int sid )
{
    m_ScreenMgr = mgr;
    m_MenuBar = mbar;
    m_MenuPath = mpath;
    m_ScreenID = sid;

    m_MenuBar->add( m_MenuPath.c_str(), "", staticShowScreenCB, this );

}

void ScreenMenuItem::ShowScreenCallBack( void* data )
{
    assert( m_ScreenMgr );
    m_ScreenMgr->ShowScreen( m_ScreenID );
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
