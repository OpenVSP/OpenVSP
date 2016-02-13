//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

//
// Programmatic main GUI window
// Rob McDonald
//
//////////////////////////////////////////////////////////////////////

#if !defined(MAINSCREEN__INCLUDED_)
#define MAINSCREEN__INCLUDED_

#include "MenuItem.h"
#include "ScreenBase.h"
#include "GuiDevice.h"
#include "FL/Fl_Box.h"

namespace VSPGUI
{
class VspGlWindow;
}

class SelectFileScreen;

class MainVSPScreen : public ActionScreen
{
public:
    MainVSPScreen( ScreenMgr* mgr );
    virtual ~MainVSPScreen();

    void Show();
    bool Update();

    virtual void CallBack( Fl_Widget *w );
    virtual void CloseCallBack( Fl_Widget *w );
    virtual void GuiDeviceCallBack( GuiDevice* device );

    void ResizeWindow( int width, int height );

    VSPGUI::VspGlWindow * GetGLWindow()
    {
        return m_GlWin;
    }

protected:

    virtual void ActionCB( void * data );

    void BoldEntries( string mpath );

    void SetFileLabel( string fname );

    virtual void ExitVSP();

    ActionMenuItem m_FileMenuItem;

    ActionMenuItem m_NewMenuItem;
    ActionMenuItem m_OpenMenuItem;
    ActionMenuItem m_SaveMenuItem;
    ActionMenuItem m_SaveAsMenuItem;
    ActionMenuItem m_SaveSetMenuItem;
    ActionMenuItem m_InsertMenuItem;
    ScreenMenuItem m_ImportMenuItem;
    ScreenMenuItem m_ExportMenuItem;
    // ActionMenuItem m_SetTempDirMenuItem;
    ActionMenuItem m_RunScriptMenuItem;
    ActionMenuItem m_ExitMenuItem;

    ActionMenuItem m_UndoMenuItem;
    ActionMenuItem m_CutMenuItem;
    ActionMenuItem m_CopyMenuItem;
    ActionMenuItem m_PasteMenuItem;
    ActionMenuItem m_DeleteMenuItem;
    ActionMenuItem m_SelAllMenuItem;
    ActionMenuItem m_PickMenuItem;

    ActionMenuItem m_OneMenuItem;
    ActionMenuItem m_FourMenuItem;
    ActionMenuItem m_TwoHMenuItem;
    ActionMenuItem m_TwoVMenuItem;
    ScreenMenuItem m_BackgroundMenuItem;
    ScreenMenuItem m_ScreenshotMenuItem;
    ActionMenuItem m_AxisMenuItem;

    ActionMenuItem m_TopMenuItem;
    ActionMenuItem m_FrontMenuItem;
    ActionMenuItem m_LeftSideMenuItem;
    ActionMenuItem m_LeftIsoMenuItem;
    ActionMenuItem m_BottomMenuItem;
    ActionMenuItem m_RearMenuItem;
    ActionMenuItem m_RightSideMenuItem;
    ActionMenuItem m_RightIsoMenuItem;
    ActionMenuItem m_CenterMenuItem;
    ScreenMenuItem m_AdjustMenuItem;
    ActionMenuItem m_SetCORMenuItem;
    ActionMenuItem m_FitViewMenuItem;
    // ActionMenuItem m_AntialiasMenuItem;

    ScreenMenuItem m_GeomMenuItem;
    ScreenMenuItem m_TypeEditMenuItem;
    ScreenMenuItem m_SetEditMenuItem;
    ScreenMenuItem m_LinkMenuItem;
    ScreenMenuItem m_DesVarMenuItem;
    ScreenMenuItem m_LabelMenuItem;
    ScreenMenuItem m_LightMenuItem;
    ScreenMenuItem m_ClipMenuItem;
    ScreenMenuItem m_TextureMenuItem;
    ScreenMenuItem m_AdvLinkMenuItem;
    ScreenMenuItem m_UserParmMenuItem;
    ScreenMenuItem m_FitModelMenuItem;

    ScreenMenuItem m_CompGeomMenuItem;
    ScreenMenuItem m_AWAVEMenuItem;
    ScreenMenuItem m_PSliceMenuItem;
    ScreenMenuItem m_MassPropMenuItem;
    ScreenMenuItem m_CFDMeshMenuItem;
    // ScreenMenuItem m_WingStructMenuItem;
    ScreenMenuItem m_DegenGeomMenuItem;
    ScreenMenuItem m_VSPAEROMenuItem;

    // ScreenMenuItem m_AboutMenuItem;
    // ScreenMenuItem m_OnlineHelpMenuItem;
    // ScreenMenuItem m_VersionMenuItem;


    GroupLayout m_GenLayout;

    Fl_Sys_Menu_Bar *m_MenuBar;
    VSPGUI::VspGlWindow * m_GlWin;
    Fl_Box * m_TitleBox;
    Fl_Box * m_FileNameBox;

    bool m_ShowXYZArrow;

};


#endif // !defined(MAINSCREEN__INCLUDED_)
