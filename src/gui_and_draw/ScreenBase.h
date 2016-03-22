//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// VehicleMgr.h: interface for the Vehicle Class and Vehicle Mgr Singleton.
// J.R Gloudemans
//
//////////////////////////////////////////////////////////////////////

#if !defined(SCREENBASE__INCLUDED_)
#define SCREENBASE__INCLUDED_

#include <FL/Fl.H>
#include <FL/Fl_Browser.H>
#include <FL/Fl_Check_Browser.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Tabs.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Scroll.H>

#include <vector>
#include <map>

#include "GuiDevice.h"
#include "GroupLayout.h"
#include "SubGLWindow.h"
#include "VSPWindow.h"

class ScreenMgr;
class Vehicle;
class Geom;

//====VspScreen ====//
class  VspScreen
{
public:

    VspScreen( ScreenMgr* mgr  );
    virtual ~VspScreen();

    virtual void CallBack( Fl_Widget *w )               {}
    virtual void GuiDeviceCallBack( GuiDevice* device ) {}
    static void staticCB( Fl_Widget *w, void* data )
    {
        static_cast< VspScreen* >( data )->CallBack( w );
    }

    virtual void CloseCallBack( Fl_Widget *w )          {}
    static void staticCloseCB( Fl_Widget *w, void* data )
    {
        static_cast< VspScreen* >( data )->CloseCallBack( w );
    }

    virtual void SetFlWindow( Fl_Double_Window* win )
    {
        m_FLTK_Window = win;
    }
    virtual Fl_Double_Window* GetFlWindow()
    {
        return m_FLTK_Window;
    }
    virtual VSP_Window* GetVSPWindow()
    {
        return dynamic_cast<VSP_Window*>(m_FLTK_Window);
    }
    virtual void Show();
    virtual bool IsShown();
    virtual void Hide();
    virtual bool Update()
    {
        return false;
    }
    ScreenMgr* GetScreenMgr()
    {
        return m_ScreenMgr;
    }

    /*!
    * Return Feedback Group Name.  Feedback Group Name identifies which GUI
    * is waitting on feedback.  By default, the names is "".  The name can
    * be any string.  You can set GUIs to the same name and getting the
    * same feedbacks.
    */
    virtual std::string getFeedbackGroupName();

protected:

    ScreenMgr* m_ScreenMgr;

    Fl_Double_Window* m_FLTK_Window;


};

class ActionScreen : public VspScreen
{
public:
    ActionScreen( ScreenMgr* mgr );
    virtual ~ActionScreen();

    virtual void ActionCB( void * data )         {}
};

//==== Basic Screen ====//
class BasicScreen : public VspScreen
{
public:

    BasicScreen( ScreenMgr* mgr, int w, int h, const string & title  );
    virtual ~BasicScreen();

    virtual void SetTitle( const string& title );

    virtual bool Update()
    {
        return false;
    }

protected:

    Fl_Box* m_FL_TitleBox;

    string m_Title;

};

//==== Tab Screen ====//
class TabScreen : public BasicScreen
{
public:

    TabScreen( ScreenMgr* mgr, int w, int h, const string & title, int baseymargin = 0 );
    virtual ~TabScreen();

    virtual bool Update()
    {
        return false;
    }

    virtual Fl_Group* AddTab( const string& title );
    virtual Fl_Group* AddTab( const string& title, int indx );
    virtual Fl_Group* GetTab( int index );

    virtual void AddTab( Fl_Group* grp );
    virtual void AddTab( Fl_Group* grp, int indx );
    virtual void RemoveTab( Fl_Group* grp );

    //==== Create A Sub Group In Tab - With Border in Pixels ====//
    virtual Fl_Group* AddSubGroup( Fl_Group* group, int border  );

    virtual Fl_Scroll* AddSubScroll( Fl_Group* group, int border, int lessh = 0  );

protected:

    virtual Fl_Group* MakeTab( const string& title );

    enum { TAB_H = 25 };

    Fl_Tabs* m_MenuTabs;

    vector< Fl_Group* > m_TabGroupVec;

};

//==== Geom Screen ====//
class GeomScreen : public TabScreen
{
public:
    GeomScreen( ScreenMgr* mgr, int w, int h, const string & title );
    virtual ~GeomScreen()                               {}

    virtual bool Update( );
    virtual void CallBack( Fl_Widget *w );
    virtual void GuiDeviceCallBack( GuiDevice* device );
    virtual void SubSurfDispGroup( GroupLayout * group );

    virtual void UpdateMaterialNames();

    //==== Group Layouts ====//
    GroupLayout m_GenLayout;
    GroupLayout m_Density;
    GroupLayout m_Shell;

    GroupLayout m_XFormLayout;
    GroupLayout m_SubSurfLayout;

    //==== Names, Color, Material ====//
    StringInput m_NameInput;
    ColorPicker m_ColorPicker;
    Choice m_MaterialChoice;
    TriggerButton m_CustomMaterialButton;
    Choice m_ExportNameChoice;

    //==== Tesselation ====//
    SliderInput m_NumUSlider;
    SliderInput m_NumWSlider;

    //==== Mass Props ====//
    Input m_DensityInput;
    Input m_ShellMassAreaInput;
    ToggleButton m_ThinShellButton;
    Counter m_PriorCounter;

    //==== Negative Volume Props ====//
    ToggleButton m_NegativeVolumeBtn;

    Fl_Check_Browser* m_SetBrowser;

    //==== XForms ====//
    ToggleButton m_XFormAbsoluteToggle;
    ToggleButton m_XFormRelativeToggle;
    ToggleRadioGroup m_XFormAbsRelToggle;

    SliderAdjRange2Input m_XLocSlider;
    SliderAdjRange2Input m_YLocSlider;
    SliderAdjRange2Input m_ZLocSlider;

    SliderAdjRange2Input m_XRotSlider;
    SliderAdjRange2Input m_YRotSlider;
    SliderAdjRange2Input m_ZRotSlider;

    SliderAdjRangeInput m_RotOriginSlider;

    Choice m_SymAncestorChoice;
    ToggleButton m_SymAncestorOriginToggle;
    ToggleButton m_SymAncestorObjectToggle;
    ToggleRadioGroup m_SymAncestorOriginObjectToggle;
    CheckButtonBit m_XYSymToggle;
    CheckButtonBit m_XZSymToggle;
    CheckButtonBit m_YZSymToggle;

    ToggleButton m_AxialNoneToggle;
    ToggleButton m_AxialXToggle;
    ToggleButton m_AxialYToggle;
    ToggleButton m_AxialZToggle;
    ToggleRadioGroup m_AxialToggleGroup;
    SliderInput m_AxialNSlider;

    SliderInput m_ScaleSlider;
    TriggerButton m_ScaleResetButton;
    TriggerButton m_ScaleAcceptButton;

    //==== Attachments
    ToggleButton m_TransNoneButton;
    ToggleButton m_TransCompButton;
    ToggleButton m_TransUVButton;
    ToggleRadioGroup m_TransToggleGroup;

    ToggleButton m_RotNoneButton;
    ToggleButton m_RotCompButton;
    ToggleButton m_RotUVButton;
    ToggleRadioGroup m_RotToggleGroup;

    SliderInput m_AttachUSlider;
    SliderInput m_AttachVSlider;

    //====== SubSurface Tab =====//
    int m_SubSurfTab_ind;
    GroupLayout* m_CurSubDispGroup;
    Fl_Browser* m_SubSurfBrowser;
    TriggerButton m_DelSubSurfButton;
    TriggerButton m_AddSubSurfButton;
    Choice m_SubSurfChoice;
    Choice m_SubSurfSelectSurface;
    int m_SSCurrMainSurfIndx;

    GroupLayout m_SSCommonGroup;
    StringInput m_SubNameInput;

    // SS_Line
    GroupLayout m_SSLineGroup;
    SliderInput m_SSLineConstSlider; // Either Constant U or W
    ToggleButton m_SSLineConstUButton;
    ToggleButton m_SSLineConstWButton;
    ToggleRadioGroup m_SSLineConstToggleGroup;

    ToggleButton m_SSLineGreaterToggle;
    ToggleButton m_SSLineLessToggle;
    ToggleRadioGroup m_SSLineTestToggleGroup;

    // SS_Rectangle
    GroupLayout m_SSRecGroup;
    SliderInput m_SSRecCentUSlider;
    SliderInput m_SSRecCentWSlider;
    SliderInput m_SSRecULenSlider;
    SliderInput m_SSRecWLenSlider;
    SliderAdjRangeInput m_SSRecThetaSlider;
    ToggleButton m_SSRecInsideButton;
    ToggleButton m_SSRecOutsideButton;
    ToggleRadioGroup m_SSRecTestToggleGroup;

    // SS_Ellipse
    GroupLayout m_SSEllGroup;
    SliderInput m_SSEllCentUSlider;
    SliderInput m_SSEllCentWSlider;
    SliderInput m_SSEllULenSlider;
    SliderInput m_SSEllWLenSlider;
    SliderInput m_SSEllTessSlider;
    SliderAdjRangeInput m_SSEllThetaSlider;
    ToggleButton m_SSEllInsideButton;
    ToggleButton m_SSEllOutsideButton;
    ToggleRadioGroup m_SSEllTestToggleGroup;

    // SS_Control
    GroupLayout m_SSConGroup;
    SliderInput m_SSConUSSlider;
    SliderInput m_SSConUESlider;
    SliderInput m_SSConSFracSlider;
    SliderAdjRangeInput m_SSConSLenSlider;
    SliderInput m_SSConEFracSlider;
    SliderAdjRangeInput m_SSConELenSlider;

    ToggleButton m_SSConSAbsButton;
    ToggleButton m_SSConSRelButton;
    ToggleRadioGroup m_SSConSAbsRelToggleGroup;

    ToggleButton m_SSConSEConstButton;

    ToggleButton m_SSConLEFlagButton;

    ToggleButton m_SSConInsideButton;
    ToggleButton m_SSConOutsideButton;
    ToggleRadioGroup m_SSConTestToggleGroup;
    Choice m_SSConSurfTypeChoice;
};

//==== Skin Screen ====//
class SkinScreen : public GeomScreen
{
public:
    SkinScreen( ScreenMgr* mgr, int w, int h, const string & title );
    virtual ~SkinScreen()                               {}

    virtual bool Update( );
    virtual void CallBack( Fl_Widget *w );
    virtual void GuiDeviceCallBack( GuiDevice* device );
    static void staticScreenCB( Fl_Widget *w, void* data )
    {
        ( ( SkinScreen* )data )->CallBack( w );
    }

protected:

    GroupLayout m_SkinLayout;

    IndexSelector m_SkinIndexSelector;

    ToggleButton m_AllSymButton;
    SkinHeader m_TopHeader;
    SkinControl m_TopAngleSkinControl;
    SkinControl m_TopSlewSkinControl;
    SkinControl m_TopStrengthSkinControl;
    SkinControl m_TopCurvatureSkinControl;

    SkinHeader m_RightHeader;
    SkinControl m_RightAngleSkinControl;
    SkinControl m_RightSlewSkinControl;
    SkinControl m_RightStrengthSkinControl;
    SkinControl m_RightCurvatureSkinControl;

    ToggleButton m_TBSymButton;
    SkinHeader m_BottomHeader;
    SkinControl m_BottomAngleSkinControl;
    SkinControl m_BottomSlewSkinControl;
    SkinControl m_BottomStrengthSkinControl;
    SkinControl m_BottomCurvatureSkinControl;

    ToggleButton m_RLSymButton;
    SkinHeader m_LeftHeader;
    SkinControl m_LeftAngleSkinControl;
    SkinControl m_LeftSlewSkinControl;
    SkinControl m_LeftStrengthSkinControl;
    SkinControl m_LeftCurvatureSkinControl;

};

class XSecViewScreen : public BasicScreen
{
public:
    XSecViewScreen( ScreenMgr* mgr );
    virtual ~XSecViewScreen() {}
    virtual bool Update();
    virtual void Show();

protected:

    VSPGUI::VspSubGlWindow * m_GlWin;
};

#endif // !defined(SCREENBASE__INCLUDED_)
