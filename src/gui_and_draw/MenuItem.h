//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
// Rob McDonald
//
//

#if !defined(MENUITEM__INCLUDED_)
#define MENUITEM__INCLUDED_

#include <string>
#include <FL/Fl_Sys_Menu_Bar.H>

class ScreenMgr;
class ActionScreen;

class BaseMenuItem
{
public:

protected:
    Fl_Sys_Menu_Bar *m_MenuBar;
    std::string m_MenuPath;
};

class ScreenMenuItem : BaseMenuItem
{
public:

    virtual void Init( ScreenMgr *mgr, Fl_Sys_Menu_Bar *mbar, std::string mpath, int sid );

    void ShowScreenCallBack( void* data );

    static void staticShowScreenCB( Fl_Widget *w, void* data )
    {
        ( static_cast <ScreenMenuItem*> ( data ) )->ShowScreenCallBack( data );
    }

protected:

    ScreenMgr* m_ScreenMgr;

    int m_ScreenID;

};

class ActionMenuItem : public BaseMenuItem
{
public:

    virtual void Init( ActionScreen *ascreen, Fl_Sys_Menu_Bar *mbar, std::string mpath, int scut = 0 );

    void ActionCallBack( void* data );
    static void staticActionCB( Fl_Widget *w, void* data )
    {
        ( static_cast <ActionMenuItem*> ( data ) )->ActionCallBack( data );
    }

protected:

    ActionScreen *m_ActionScreen;

};

class VSPCheckMenuItem : ActionMenuItem
{
public:

    void Init( ActionScreen *ascreen, Fl_Sys_Menu_Bar *mbar, std::string mpath, int scut = 0 );

    static void staticActionCB( Fl_Widget *w, void* data )
    {
        ( static_cast <VSPCheckMenuItem*> ( data ) )->ActionCallBack( data );
    }

    void Update( bool val );

protected:
};

#endif // !defined(MENUITEM__INCLUDED_)
