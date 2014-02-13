//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// VehicleMgr.h: interface for the Vehicle Class and Vehicle Mgr Singleton.
// J.R Gloudemans
//
//////////////////////////////////////////////////////////////////////

#if !defined(MAINSCREEN__INCLUDED_)
#define MAINSCREEN__INCLUDED_

#include "ScreenBase.h"

#include <FL/Fl.H>
#include "mainScreen.h"
#include "exportFileScreen.h"

namespace VSPGUI
{
class VspGlWindow;
}

class SelectFileScreen;

class MainVSPScreen : public VspScreen
{
public:
    MainVSPScreen( ScreenMgr* mgr );
    virtual ~MainVSPScreen();

    virtual void Show();
    virtual void Hide();
    virtual bool Update();

    void CallBack( Fl_Widget *w );
    static void staticCB( Fl_Widget *w, void* data )
    {
        static_cast< MainVSPScreen* >( data )->CallBack( w );
    }

    void AddMenuCallBack( Fl_Menu_Item* fl_menu );
    void MenuCallBack( Fl_Widget *w );
    static void staticMenuCB( Fl_Widget *w, void* data )
    {
        static_cast< MainVSPScreen* >( data )->MenuCallBack( w );
    }

    string CheckAddVSP3Ext( string fname );

    void SetFileLabel( string fname );
    /*!
    * Return reference to OpenGL Window.
    */
    VSPGUI::VspGlWindow * GetGLWindow()
    {
        return m_GlWin;
    }

    void ShowReturnToAPI()
    {
        m_MainUI->ReturnToAPI->show();
    }
    void HideReturnToAPI()
    {
        m_MainUI->ReturnToAPI->hide();
    }

protected:
    MainWinUI* m_MainUI;
    //VspGlWindow* m_GlWin;
    VSPGUI::VspGlWindow * m_GlWin;
    SelectFileScreen * m_selectFileScreen;
};


#endif // !defined(MAINSCREEN__INCLUDED_)
