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
#include "NavDevice.h"
#include "FL/Fl_Box.H"
#include "FL/Fl_Button.H"
#include "FL/Fl_SVG_Image.H"

#include <vector>

namespace VSPGUI
{
class VspGlWindow;
struct NavSettings;
}

class SelectFileScreen;
class VspCaptiveLayer;

// How large the main window's toolbar is drawn.
enum ToolbarSize
{
    TOOLBAR_HIDDEN,
    TOOLBAR_SMALL,
    TOOLBAR_MEDIUM,
    TOOLBAR_LARGE
};

#define GL_WINDOW_DX 0
#define GL_WINDOW_DY 25

class MainVSPScreen : public ActionScreen
{
public:
    MainVSPScreen( ScreenMgr* mgr );
    virtual ~MainVSPScreen();

    void Show();
    void Hide();
    bool Update();

    virtual void CallBack( Fl_Widget *w );
    virtual void CloseCallBack( Fl_Widget *w );
    virtual void GuiDeviceCallBack( GuiDevice* device );

    void ResizeWindow( int pixel_w, int pixel_h );

    VSPGUI::VspGlWindow * GetGLWindow()
    {
        return m_GlWin;
    }

    // The 3D input device that moves the main window's view.
    VSPGUI::NavDevice * GetNavDevice()
    {
        return m_NavDevice;
    }

    // The part of the window over the 3D view that screens shown inside the main window lie in
    VspCaptiveLayer * GetCaptiveLayer()
    {
        return m_CaptiveLayer;
    }

    static int ConvertViewEnums( int v );
    void SetAllViews( int view );
    void SetView( int viewport, int view );
    void SetView( int view );
    void FitView( bool all );
    void AlignView( const vec3d & v );
    void AlignViewToGeom( string geom_id );
    void ResetViews();
    void SetWindowLayout( int r, int c );

    void ShowReturnToAPIImplementation();
    void HideReturnToAPIImplementation();
    void ShowExitImplementation();
    void HideExitImplementation();

    void ScreenGrab( const string & fname, int w, int h, bool transparentBG, bool autocrop );
    bool VectorScreenGrab( const string & fname, int filetype, bool autocrop, bool scaletopage, double pagewidth,
                           double pageheight, int pageunits );

    // The size on the page, in points, of a pixel of a w by h window: 1, or to fit the page
    static double VectorPageScale( int w, int h, bool scaletopage, double pagewidth, double pageheight,
                                   int pageunits );
    void SetViewAxis( bool vaxis );
    void SetShowBorders( bool brdr );
    void SetBackground( double r, double g, double b );

    // Rotation used by the next rotate gesture: NAV_ROTATE_BALL or NAV_ROTATE_TURNTABLE.
    void SetRotateMode( int mode );

    // Turn the view about the screen's axis until Z points up.
    void LevelZUp();

    // Apply new 3D interaction preferences.
    void SetNavSettings( const VSPGUI::NavSettings & settings );

    // The 3D input device's status changed.
    static void NavDeviceStatusCB( void * data );

    // Show the device's status on its bottom strip button and in the View menu.
    void UpdateDeviceButton();

    static Fl_SVG_Image * MakeIcon( const char * svg );

    // The toolbar's size, a ToolbarSize, kept in the preferences.
    void SetToolbarSize( int size );
    int GetToolbarSize();

    void EnableStopGUIMenuItem();
    void DisableStopGUIMenuItem();
    void EnableExitMenuItem();
    void DisableExitMenuItem();

protected:

    virtual void ActionCB( void * data );

    // The toolbar under the menu bar, at x, y, w wide and of a ToolbarSize; its buttons carry out
    // menu commands.
    Fl_Group * BuildToolbar( int x, int y, int w, int size );
    void AddToolButton( int & x, int y, int h, int action, const char * svg, const string & tip );
    void AddToolGap( int & x, int y, int h );
    // Show the toolbar's toggles as the commands they stand for are.
    void UpdateToolbar();
    static void staticToolCB( Fl_Widget * w, void * data );
    void ToolCB( Fl_Widget * w );

    void BoldEntries( const string &mpath );

    void SetFileLabel( const string &fname );

    virtual void NewModel();
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
    ScreenMenuItem m_PreferencesMenuItem;
    ActionMenuItem m_ExitMenuItem;
    ActionMenuItem m_StopGUIMenuItem;

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
    ScreenMenuItem m_Background3DMenuItem;
    ScreenMenuItem m_ScreenshotMenuItem;
    ScreenMenuItem m_VectorScreenshotMenuItem;
    VSPCheckMenuItem m_AxisMenuItem;
    VSPCheckMenuItem m_BorderMenuItem;

    ActionMenuItem m_TopMenuItem;
    ActionMenuItem m_FrontMenuItem;
    ActionMenuItem m_LeftSideMenuItem;
    ActionMenuItem m_LeftIsoMenuItem;
    ActionMenuItem m_BottomMenuItem;
    ActionMenuItem m_RearMenuItem;
    ActionMenuItem m_RightSideMenuItem;
    ActionMenuItem m_RightIsoMenuItem;
    VSPCheckMenuItem m_TurntableMenuItem;
    VSPCheckMenuItem m_BallMenuItem;
    VSPCheckMenuItem m_DeviceMenuItem;
    VSPCheckMenuItem m_PivotLockMenuItem;
    ActionMenuItem m_ZUpMenuItem;
    ActionMenuItem m_ZoomBoxMenuItem;
    ActionMenuItem m_CenterMenuItem;
    ActionMenuItem m_CenterAllMenuItem;
    ScreenMenuItem m_AdjustMenuItem;
    VSPCheckMenuItem m_ToolbarMenuItem;
    ActionMenuItem m_SetCORMenuItem;
    ActionMenuItem m_SetVNMenuItem;
    ActionMenuItem m_AlignGeomMenuItem;
    ActionMenuItem m_FitViewMenuItem;
    ActionMenuItem m_FitAllViewMenuItem;
    ActionMenuItem m_ResetAllViewMenuItem;
    // ActionMenuItem m_AntialiasMenuItem;

    ScreenMenuItem m_GeomMenuItem;
    ScreenMenuItem m_SetEditMenuItem;
    ScreenMenuItem m_VarPresetMenuItem;
    ScreenMenuItem m_ModeEditMenuItem;
    ScreenMenuItem m_LinkMenuItem;
    ScreenMenuItem m_DesVarMenuItem;
    ScreenMenuItem m_LabelMenuItem;
    ScreenMenuItem m_LightMenuItem;
    ScreenMenuItem m_ClipMenuItem;
    ScreenMenuItem m_TextureMenuItem;
    ScreenMenuItem m_AdvLinkMenuItem;
    ScreenMenuItem m_UserParmMenuItem;
    ScreenMenuItem m_FitModelMenuItem;
    ScreenMenuItem m_SnapToMenuItem;
    ScreenMenuItem m_AttributeMenuItem;
    ScreenMenuItem m_VehNotesMenuItem;

    ScreenMenuItem m_CompGeomMenuItem;
    ScreenMenuItem m_WaveDragMenuItem;
    ScreenMenuItem m_PSliceMenuItem;
    ScreenMenuItem m_MassPropMenuItem;
    ScreenMenuItem m_ProjectionMenuItem;
    ScreenMenuItem m_InterferenceMenuItem;
    ScreenMenuItem m_SurfIntMenuItem;
    ScreenMenuItem m_CFDMeshMenuItem;
    ScreenMenuItem m_StructMenuItem;
    ScreenMenuItem m_StructAssemblyMenuItem;
    ScreenMenuItem m_DegenGeomMenuItem;
    ScreenMenuItem m_VSPAEROMenuItem;
    ScreenMenuItem m_ParasiteDragMenuItem;
    ScreenMenuItem m_AeroStructMenuItem;

    // ScreenMenuItem m_AboutMenuItem;
    ActionMenuItem m_OnlineHelpMenuItem;
    ActionMenuItem m_TestParmMapMenuItem;
    ActionMenuItem m_TestParmContainerMapMenuItem;
    // ScreenMenuItem m_VersionMenuItem;


    GroupLayout m_GenLayout;

    Fl_Sys_Menu_Bar *m_MenuBar;
    VSPGUI::VspGlWindow * m_GlWin;
    VSPGUI::NavDevice * m_NavDevice;
    VspCaptiveLayer * m_CaptiveLayer;
    Fl_Box * m_TitleBox;
    Fl_Box * m_FileNameBox;
    Fl_Button * m_DeviceButton;

    Fl_Group * m_Toolbar;
    std::vector < Fl_Button * > m_ToolButtons;
    std::vector < int > m_ToolActions;
    int m_ToolbarSize;
    int m_ShownToolbarSize;     // The size Window > Toolbar shows it at again.
    int m_ToolW;
    int m_ToolIcon;
    Fl_SVG_Image * m_DeviceIconOn;
    Fl_Image * m_DeviceIconOff;
    std::string m_DeviceTip;

    bool m_ShowXYZArrow;
    bool m_ShowBorder;
};


#endif // !defined(MAINSCREEN__INCLUDED_)
