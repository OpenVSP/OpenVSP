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

#include "GuiDevice.h"
#include "GroupLayout.h"
#include "SubGLWindow.h"
#include "VSPWindow.h"
#include "AttributeEditor.h"
#include "AttributeManager.h"

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
    virtual bool Update();
    virtual void GetCollIDs( vector < string > &collIDVec ) {};

    ScreenMgr* GetScreenMgr()
    {
        return m_ScreenMgr;
    }

    virtual void SetScreenType( int t )
    {
        m_ScreenType = t;
    }

    const int GetScreenType()
    {
        return m_ScreenType;
    }

    /*!
    * Return Feedback Group Name.  Feedback Group Name identifies which GUI
    * is waiting on feedback.  By default, the names is "".  The name can
    * be any string.  You can set GUIs to the same name and getting the
    * same feedbacks.
    */
    virtual std::string getFeedbackGroupName();

protected:

    ScreenMgr* m_ScreenMgr;

    Fl_Double_Window* m_FLTK_Window;

    int m_ScreenType;

};

class ActionScreen : public VspScreen
{
public:
    ActionScreen( ScreenMgr* mgr );
    virtual ~ActionScreen();

    virtual void ActionCB( void * data )         {}

    virtual bool Update()
    {
        VspScreen::Update();
        return false;
    }
};

//==== Basic Screen ====//
class BasicScreen : public VspScreen
{
public:

    BasicScreen( ScreenMgr* mgr, int w, int h, const string & title, const string & helpfile = string() );
    virtual ~BasicScreen();

    virtual void SetTitle( const string& title );
    string GetTitle();

    virtual bool Update()
    {
        VspScreen::Update();
        return false;
    }

    void HelpCallBack( Fl_Widget *w );
    static void staticHelpCB( Fl_Widget *w, void* data )
    {
        ( static_cast <BasicScreen*> ( data ) )->HelpCallBack( w );
    }

protected:

    Fl_Box* m_FL_TitleBox;
    Fl_Button* m_MasterHelpButton;

    string m_Title;
    string m_HelpFile;

};

//==== Tab Screen ====//
class TabScreen : public BasicScreen
{
public:

    TabScreen( ScreenMgr* mgr, int w, int h, const string & title, const string & helpfile = string(), int baseymargin = 0, int basexmargin = 0 );
    virtual ~TabScreen();

    virtual bool Update()
    {
        BasicScreen::Update();
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

    virtual Fl_Scroll* AddSubScroll( Fl_Group* group, int border, int lessh = 0, int starty = 0 );

protected:

    virtual Fl_Group* MakeTab( const string& title );

    enum { TAB_H = 25 };

    Fl_Tabs* m_MenuTabs;

    vector< Fl_Group* > m_TabGroupVec;

};

//==== Vehicle Screen ====//
class VehScreen : public TabScreen
{
public:
    VehScreen( ScreenMgr* mgr, int w = 400, int h = 677 , const string & title = "Vehicle");

    virtual ~VehScreen(){}

    virtual bool Update( );
    virtual void Show( );
    virtual void CallBack( Fl_Widget *w );
    virtual void CloseCallBack( Fl_Widget *w );
    static void staticScreenCB( Fl_Widget* w, void* data )
    {
        ( (VehScreen*)data )->CallBack( w );
    }
    virtual void GuiDeviceCallBack( GuiDevice* device );

    virtual void GetCollIDs( vector < string > &collIDVec );

    //=====Attribute Editor Tab=====//
    GroupLayout m_AttributeLayout;
    AttributeEditor m_AttributeEditor;
};

//==== Geom Screen ====//
class GeomScreen : public TabScreen
{
public:
    GeomScreen( ScreenMgr* mgr, int w, int h, const string & title, const string & helpfile = string() );
    virtual ~GeomScreen()                               {}

    virtual bool Update( );
    virtual void Show( );
    virtual void CallBack( Fl_Widget *w );
    virtual void CloseCallBack( Fl_Widget *w );
    static void staticScreenCB( Fl_Widget* w, void* data )
    {
        ( (GeomScreen*)data )->CallBack( w );
    }
    virtual void GuiDeviceCallBack( GuiDevice* device );
    virtual void SubSurfDispGroup( GroupLayout * group );

    virtual void UpdateMaterialNames();

    virtual void GetCollIDs( vector < string > &collIDVec );

    //==== Group Layouts ====//
    GroupLayout m_GenLayout;

    GroupLayout m_XFormLayout;
    GroupLayout m_SubSurfLayout;
    GroupLayout m_MassPropLayout;

    GroupLayout m_SymmLayout;
    GroupLayout m_AttachLayout;

    //==== Names, Color, Material ====//
    StringInput m_NameInput;
    ColorPicker m_ColorPicker;
    Choice m_MaterialChoice;
    TriggerButton m_CustomMaterialButton;
    Choice m_ExportNameChoice;

    //==== Tessellation ====//
    SliderInput m_NumUSlider;
    SliderInput m_NumWSlider;

    //==== Negative Volume Props ====//
    ToggleButton m_NegativeVolumeBtn;

    Fl_Check_Browser* m_SetBrowser;

    //===== Attributes =====//
    AttributeEditor m_AttributeEditor;

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
    SliderAdjRangeInput m_AxialNSlider;

    SliderAdjRangeInput m_ScaleSlider;
    TriggerButton m_ScaleResetButton;
    TriggerButton m_ScaleAcceptButton;

    //==== Attachments
    ToggleButton m_TransNoneButton;
    ToggleButton m_TransCompButton;
    ToggleButton m_TransUVButton;
    ToggleButton m_TransRSTButton;
    ToggleButton m_TransLMNButton;
    ToggleButton m_TransEtaMNButton;
    ToggleRadioGroup m_TransToggleGroup;

    ToggleButton m_RotNoneButton;
    ToggleButton m_RotCompButton;
    ToggleButton m_RotUVButton;
    ToggleButton m_RotRSTButton;
    ToggleButton m_RotLMNButton;
    ToggleButton m_RotEtaMNButton;
    ToggleRadioGroup m_RotToggleGroup;

    ToggleButton m_U01Toggle;
    ToggleButton m_U0NToggle;
    ToggleRadioGroup m_UScaleToggleGroup;

    ToggleButton m_R01Toggle;
    ToggleButton m_R0NToggle;
    ToggleRadioGroup m_RScaleToggleGroup;

    ToggleButton m_L01Toggle;
    ToggleButton m_L0LenToggle;
    ToggleRadioGroup m_LScaleToggleGroup;

    SliderAdjRange2Input m_AttachUSlider;
    SliderAdjRangeInput m_AttachVSlider;
    SliderAdjRange2Input m_AttachRSlider;
    SliderAdjRangeInput m_AttachSSlider;
    SliderAdjRangeInput m_AttachTSlider;
    SliderAdjRange2Input m_AttachLSlider;
    SliderAdjRangeInput m_AttachMSlider;
    SliderAdjRangeInput m_AttachNSlider;
    SliderAdjRangeInput m_AttachEtaSlider;

    //====== SubSurface Tab =====//
    int m_SubSurfTab_ind;
    GroupLayout* m_CurSubDispGroup;
    ColResizeBrowser* m_SubSurfBrowser;
    TriggerButton m_DelSubSurfButton;
    TriggerButton m_AddSubSurfButton;
    Choice m_SubSurfChoice;
    Choice m_SubSurfSelectSurface;
    int m_SSCurrMainSurfIndx;

    GroupLayout m_SSCommonGroup;
    StringInput m_SubNameInput;

    // SS_Line
    GroupLayout m_SSLineGroup;
    SliderAdjRangeInput m_SSLineConstSlider; // Either Constant U or W
    SliderAdjRangeInput m_SSLineConstSlider0N; // Either Constant U or W
    ToggleButton m_SSLineConstUButton;
    ToggleButton m_SSLineConstWButton;
    ToggleRadioGroup m_SSLineConstToggleGroup;

    ToggleButton m_SSLineGreaterToggle;
    ToggleButton m_SSLineLessToggle;
    ToggleRadioGroup m_SSLineTestToggleGroup;

    ToggleButton m_SSLine01Toggle;
    ToggleButton m_SSLine0NToggle;
    ToggleRadioGroup m_SSLineScaleToggleGroup;
    AttributeEditor m_SSLineAttrEditor;

    // SS_Rectangle
    GroupLayout m_SSRecGroup;
    SliderAdjRangeInput m_SSRecCentUSlider;
    SliderAdjRangeInput m_SSRecCentWSlider;
    SliderAdjRangeInput m_SSRecULenSlider;
    SliderAdjRangeInput m_SSRecWLenSlider;
    SliderAdjRangeInput m_SSRecThetaSlider;
    ToggleButton m_SSRecInsideButton;
    ToggleButton m_SSRecOutsideButton;
    ToggleRadioGroup m_SSRecTestToggleGroup;
    AttributeEditor m_SSRecAttrEditor;

    // SS_Ellipse
    GroupLayout m_SSEllGroup;
    SliderAdjRangeInput m_SSEllTessSlider;
    SliderAdjRangeInput m_SSEllCentUSlider;
    SliderAdjRangeInput m_SSEllCentWSlider;
    SliderAdjRangeInput m_SSEllULenSlider;
    SliderAdjRangeInput m_SSEllWLenSlider;
    SliderAdjRangeInput m_SSEllThetaSlider;
    ToggleButton m_SSEllInsideButton;
    ToggleButton m_SSEllOutsideButton;
    ToggleRadioGroup m_SSEllTestToggleGroup;
    AttributeEditor m_SSEllAttrEditor;

    // SS_Control
    GroupLayout m_SSConGroup;
    SliderAdjRangeInput m_SSConUSSlider;
    SliderAdjRangeInput m_SSConUESlider;

    ToggleButton m_SSConEtaButton;
    SliderAdjRangeInput m_SSConEtaSSlider;
    SliderAdjRangeInput m_SSConEtaESlider;

    SliderAdjRange2Input m_SSConSLenSlider;
    SliderAdjRange2Input m_SSConELenSlider;

    ToggleButton m_SSConSAbsButton;
    ToggleButton m_SSConSRelButton;
    ToggleRadioGroup m_SSConSAbsRelToggleGroup;

    ToggleButton m_SSConSEConstButton;

    ToggleButton m_SSConLEFlagButton;

    ToggleButton m_SSConInsideButton;
    ToggleButton m_SSConOutsideButton;
    ToggleRadioGroup m_SSConTestToggleGroup;
    Choice m_SSConSurfTypeChoice;

    SliderAdjRangeInput m_SSConSAngleSlider;
    SliderAdjRangeInput m_SSConEAngleSlider;
    ToggleButton m_SSConSAngleButton;
    ToggleButton m_SSConEAngleButton;
    ToggleButton m_SSConSameAngleButton;

    SliderAdjRangeInput m_SSConTessSlider;

    AttributeEditor m_SSConAttrEditor;

    // SS_FiniteLine
    GroupLayout m_SSFLineGroup;
    SliderAdjRangeInput m_SSFLineUStartSlider;
    SliderAdjRangeInput m_SSFLineUEndSlider;
    SliderAdjRangeInput m_SSFLineWStartSlider;
    SliderAdjRangeInput m_SSFLineWEndSlider;
    AttributeEditor m_SSFLineAttrEditor;

    //======Mass Prop Tab=====//
    int m_MassPropTab_ind;

    Input m_DensityInput;
    Input m_ShellMassAreaInput;
    ToggleButton m_ThinShellButton;
    Counter m_PriorCounter;

    SliderAdjRangeInput m_MassSlider;

    SliderAdjRangeInput m_CGxSlider;
    SliderAdjRangeInput m_CGySlider;
    SliderAdjRangeInput m_CGzSlider;

    SliderAdjRangeInput m_IxxSlider;
    SliderAdjRangeInput m_IyySlider;
    SliderAdjRangeInput m_IzzSlider;
    SliderAdjRangeInput m_IxySlider;
    SliderAdjRangeInput m_IxzSlider;
    SliderAdjRangeInput m_IyzSlider;

protected:
    bool m_RotActive;
    string m_GeomTypeName;
};

//==== Skin Screen ====//
class XSecScreen : public GeomScreen
{
public:
    XSecScreen(ScreenMgr* mgr, int w, int h, const string & title, const string & helpfile = string(), const string & xsnamelabel = string("Sect Alias"), const string & xscnamelabel = string("XSec Alias"), bool add_xs_btn = true );
    virtual ~XSecScreen() {}

    virtual bool Update();
    virtual void CallBack(Fl_Widget *w);
    virtual void GuiDeviceCallBack(GuiDevice* device);
    static void staticScreenCB(Fl_Widget *w, void* data)
    {
        ((static_cast <XSecScreen*>(data))->CallBack(w));
    }

    virtual void GetCollIDs( vector < string > &collIDVec );

    void AddXSecLayout(bool include_point_type = true); // Default flag to include point type but allow PropGeom to skip it

    virtual void DisplayGroup(GroupLayout* group);
    virtual void RebuildCSTGroup(CSTAirfoil* cst_xs);

    AttributeEditor m_XsecAttributeEditor;

protected:
    GroupLayout* m_CurrDisplayGroup;

    GroupLayout m_XSecLayout;

    // Tab and divider pointers saved in memory so WingGeom can rename them
    Fl_Group* m_XSecTab;
    Fl_Box* m_XSecDivider;

    IndexSelector m_XSecIndexSelector; // Not used for BORGeom
    StringInput m_XSecNameInput;
    StringInput m_XSecCurveNameInput;

    Choice m_XSecTypeChoice;
    TriggerButton m_ShowXSecButton;

    TriggerButton m_ConvertCEDITButton;
    GroupLayout m_ConvertCEDITGroup;

    HWXSecCurveDriverGroup m_DefaultXSecDriverGroup;
    DXSecCurveDriverGroup m_CircleXSecDriverGroup;

    GroupLayout m_EditCEDITButtonGroup;
    TriggerButton m_EditCEDITButton;

    GroupLayout m_EditCEDITGroup;
    DriverGroupBank m_EditCEDITXSecDriverGroupBank;
    SliderAdjRangeInput m_EditCEDITHeightSlider;
    SliderAdjRangeInput m_EditCEDITWidthSlider;

    GroupLayout m_PointGroup;

    GroupLayout m_SuperGroup;
    DriverGroupBank m_SuperXSecDriverGroupBank;
    SliderAdjRangeInput m_SuperHeightSlider;
    SliderAdjRangeInput m_SuperWidthSlider;
    SliderAdjRangeInput m_SuperMSlider;
    SliderAdjRangeInput m_SuperNSlider;
    ToggleButton m_SuperToggleSym;
    SliderAdjRangeInput m_SuperM_botSlider;
    SliderAdjRangeInput m_SuperN_botSlider;
    SliderAdjRangeInput m_SuperMaxWidthLocSlider;

    GroupLayout m_CircleGroup;
    DriverGroupBank m_CircleXSecDriverGroupBank;
    SliderAdjRangeInput m_DiameterSlider;

    GroupLayout m_EllipseGroup;
    DriverGroupBank m_EllipseXSecDriverGroupBank;
    SliderAdjRangeInput m_EllipseHeightSlider;
    SliderAdjRangeInput m_EllipseWidthSlider;

    GroupLayout m_RoundedRectGroup;
    DriverGroupBank m_RRXSecDriverGroupBank;
    SliderAdjRangeInput m_RRHeightSlider;
    SliderAdjRangeInput m_RRWidthSlider;
    ToggleButton m_RRRadNoSymToggle;
    ToggleButton m_RRRadRLSymToggle;
    ToggleButton m_RRRadTBSymToggle;
    ToggleButton m_RRRadAllSymToggle;
    ToggleRadioGroup m_RRRadSymRadioGroup;
    SliderAdjRangeInput m_RRRadiusBRSlider; // Bottom Right
    SliderAdjRangeInput m_RRRadiusBLSlider; // Bottom Left
    SliderAdjRangeInput m_RRRadiusTLSlider; // Top Left
    SliderAdjRangeInput m_RRRadiusTRSlider; // Top Right
    ToggleButton m_RRKeyCornerButton;
    SliderAdjRangeInput m_RRSkewSlider;
    SliderAdjRangeInput m_RRKeystoneSlider;
    SliderAdjRangeInput m_RRVSkewSlider;

    GroupLayout m_GenGroup;
    DriverGroupBank m_GenXSecDriverGroupBank;
    SliderAdjRangeInput m_GenHeightSlider;
    SliderAdjRangeInput m_GenWidthSlider;
    SliderAdjRangeInput m_GenMaxWidthLocSlider;
    SliderAdjRangeInput m_GenCornerRadSlider;
    SliderAdjRangeInput m_GenTopTanAngleSlider;
    SliderAdjRangeInput m_GenBotTanAngleSlider;
    SliderAdjRangeInput m_GenTopStrSlider;
    SliderAdjRangeInput m_GenBotStrSlider;
    SliderAdjRangeInput m_GenUpStrSlider;
    SliderAdjRangeInput m_GenLowStrSlider;

    GroupLayout m_FourSeriesGroup;
    StringOutput m_FourNameOutput;
    CheckButton m_FourInvertButton;
    SliderAdjRangeInput m_FourChordSlider;
    SliderAdjRangeInput m_FourThickChordSlider;
    SliderAdjRangeInput m_FourCamberSlider;
    SliderAdjRangeInput m_FourCLiSlider;
    ToggleButton m_FourCamberButton;
    ToggleButton m_FourCLiButton;
    ToggleRadioGroup m_FourCamberGroup;
    SliderAdjRangeInput m_FourCamberLocSlider;
    CheckButton m_FourSharpTEButton;
    TriggerButton m_FourFitCSTButton;
    Counter m_FourDegreeCounter;

    GroupLayout m_SixSeriesGroup;
    StringOutput m_SixNameOutput;
    CheckButton m_SixInvertButton;
    Choice m_SixSeriesChoice;
    SliderAdjRangeInput m_SixChordSlider;
    SliderAdjRangeInput m_SixThickChordSlider;
    SliderAdjRangeInput m_SixIdealClSlider;
    SliderAdjRangeInput m_SixASlider;
    TriggerButton m_SixFitCSTButton;
    Counter m_SixDegreeCounter;

    GroupLayout m_BiconvexGroup;
    SliderAdjRangeInput m_BiconvexChordSlider;
    SliderAdjRangeInput m_BiconvexThickChordSlider;

    GroupLayout m_WedgeGroup;
    SliderAdjRangeInput m_WedgeChordSlider;
    SliderAdjRangeInput m_WedgeThickChordSlider;
    SliderAdjRangeInput m_WedgeZCamberSlider;
    ToggleButton m_WedgeSymmThickButton;
    SliderAdjRangeInput m_WedgeThickLocSlider;
    SliderAdjRangeInput m_WedgeThickLocLowSlider;
    SliderAdjRangeInput m_WedgeFlatUpSlider;
    SliderAdjRangeInput m_WedgeFlatLowSlider;
    SliderAdjRangeInput m_WedgeUForeUpSlider;
    SliderAdjRangeInput m_WedgeUForeLowSlider;
    SliderAdjRangeInput m_WedgeDuUpSlider;
    SliderAdjRangeInput m_WedgeDuLowSlider;
    CheckButton m_WedgeInvertButton;

    GroupLayout m_FuseFileGroup;
    DriverGroupBank m_FuseFileXSecDriverGroupBank;
    SliderAdjRangeInput m_FileHeightSlider;
    SliderAdjRangeInput m_FileWidthSlider;
    TriggerButton m_ReadFuseFileButton;

    GroupLayout m_AfFileGroup;
    TriggerButton m_AfReadFileButton;
    StringOutput m_AfFileNameOutput;
    CheckButton m_AfFileInvertButton;
    SliderAdjRangeInput m_AfFileChordSlider;
    SliderAdjRangeInput m_AfFileThickChordSlider;
    Output m_AfFileBaseThickChordOutput;
    TriggerButton m_AfFileFitCSTButton;
    Counter m_AfFileDegreeCounter;

    GroupLayout m_CSTAirfoilGroup;

    Output m_CSTThickChordOutput;

    SliderAdjRangeInput m_CSTChordSlider;

    TriggerButton m_UpDemoteButton;
    StringOutput m_UpDegreeOutput;
    TriggerButton m_UpPromoteButton;

    Fl_Scroll* m_CSTUpCoeffScroll;
    GroupLayout m_CSTUpCoeffLayout;

    TriggerButton m_LowDemoteButton;
    StringOutput m_LowDegreeOutput;
    TriggerButton m_LowPromoteButton;

    vector < SliderAdjRangeInput > m_UpCoeffSliderVec;

    Fl_Scroll* m_CSTLowCoeffScroll;
    GroupLayout m_CSTLowCoeffLayout;

    vector < SliderAdjRangeInput > m_LowCoeffSliderVec;

    CheckButton m_CSTInvertButton;
    CheckButton m_CSTContLERadButton;

    GroupLayout m_VKTGroup;
    SliderAdjRangeInput m_VKTChordSlider;
    SliderAdjRangeInput m_VKTEpsilonSlider;
    SliderAdjRangeInput m_VKTKappaSlider;
    SliderAdjRangeInput m_VKTTauSlider;
    CheckButton m_VKTInvertButton;
    Output m_VKTThickChordOutput;
    TriggerButton m_VKTFitCSTButton;
    Counter m_VKTDegreeCounter;

    GroupLayout m_FourDigitModGroup;
    StringOutput m_FourModNameOutput;
    CheckButton m_FourModInvertButton;
    SliderAdjRangeInput m_FourModChordSlider;
    SliderAdjRangeInput m_FourModThickChordSlider;
    SliderAdjRangeInput m_FourModCamberSlider;
    SliderAdjRangeInput m_FourModCLiSlider;
    ToggleButton m_FourModCamberButton;
    ToggleButton m_FourModCLiButton;
    ToggleRadioGroup m_FourModCamberGroup;
    SliderAdjRangeInput m_FourModCamberLocSlider;
    SliderAdjRangeInput m_FourModThicknessLocSlider;
    SliderAdjRangeInput m_FourModLERadIndexSlider;
    CheckButton m_FourModSharpTEButton;
    TriggerButton m_FourModFitCSTButton;
    Counter m_FourModDegreeCounter;

    GroupLayout m_FiveDigitGroup;
    StringOutput m_FiveNameOutput;
    CheckButton m_FiveInvertButton;
    SliderAdjRangeInput m_FiveChordSlider;
    SliderAdjRangeInput m_FiveThickChordSlider;
    SliderAdjRangeInput m_FiveCLiSlider;
    SliderAdjRangeInput m_FiveCamberLocSlider;
    CheckButton m_FiveSharpTEButton;
    TriggerButton m_FiveFitCSTButton;
    Counter m_FiveDegreeCounter;

    GroupLayout m_FiveDigitModGroup;
    StringOutput m_FiveModNameOutput;
    CheckButton m_FiveModInvertButton;
    SliderAdjRangeInput m_FiveModChordSlider;
    SliderAdjRangeInput m_FiveModThickChordSlider;
    SliderAdjRangeInput m_FiveModCLiSlider;
    SliderAdjRangeInput m_FiveModCamberLocSlider;
    SliderAdjRangeInput m_FiveModThicknessLocSlider;
    SliderAdjRangeInput m_FiveModLERadIndexSlider;
    CheckButton m_FiveModSharpTEButton;
    TriggerButton m_FiveModFitCSTButton;
    Counter m_FiveModDegreeCounter;

    GroupLayout m_OneSixSeriesGroup;
    StringOutput m_OneSixSeriesNameOutput;
    CheckButton m_OneSixSeriesInvertButton;
    SliderAdjRangeInput m_OneSixSeriesChordSlider;
    SliderAdjRangeInput m_OneSixSeriesThickChordSlider;
    SliderAdjRangeInput m_OneSixSeriesCLiSlider;
    SliderAdjRangeInput m_OneSixSeriesCamberLocSlider;
    SliderAdjRangeInput m_OneSixSeriesThicknessLocSlider;
    SliderAdjRangeInput m_OneSixSeriesLERadIndexSlider;
    CheckButton m_OneSixSeriesSharpTEButton;
    TriggerButton m_OneSixSeriesFitCSTButton;
    Counter m_OneSixSeriesDegreeCounter;

    // Variables used for slight differences in GUIs

    string m_XSecAliasLabel;
    string m_XSecCurveAliasLabel;

    bool m_XSecNameInputActive; // Flag to determine if XSecNameInput is used in this level of XSecScreen vs. added later in a derived class
    bool m_XSecNameInputControlled; // Flag to determine if the GUI has separate controls between XSec and XSecCurve name inputs

    bool m_XSecDriversActive; // Flag indicating if the width, area, etc. drivers should be shown

};

//==== Skin Screen ====//
class SkinScreen : public XSecScreen
{
public:
    SkinScreen( ScreenMgr* mgr, int w, int h, const string & title, const string & helpfile = string() );
    virtual ~SkinScreen()                               {}

    virtual bool Update( );
    virtual void CallBack( Fl_Widget *w );
    virtual void GuiDeviceCallBack( GuiDevice* device );
    static void staticScreenCB( Fl_Widget *w, void* data )
    {
        ( ( static_cast <SkinScreen*>( data ) )->CallBack( w ) );
    }

protected:

    GroupLayout m_SkinLayout;

    IndexSelector m_SkinIndexSelector;

    StringInput m_SkinXSecCurveNameInput;

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

//==== Engine Model Screen ====//
class EngineModelScreen : public SkinScreen
{
public:
    EngineModelScreen( ScreenMgr* mgr, int w, int h, const string & title, const string & helpfile = string() );
    virtual ~EngineModelScreen()                               {}

    virtual void BuildEngineGUI( GroupLayout & layout );

    virtual bool Update( );
    virtual void CallBack( Fl_Widget *w );
    virtual void GuiDeviceCallBack( GuiDevice* device );

    static void staticScreenCB( Fl_Widget *w, void* data )
    {
        ( ( static_cast <EngineModelScreen*>( data ) )->CallBack( w ) );
    }

protected:

    Choice m_EngineGeomIOChoice;
    Choice m_EngineGeomInChoice;
    Choice m_EngineGeomOutChoice;

    ToggleRadioGroup m_EngineInFaceToggleGroup;
    ToggleButton m_EngineInFaceUToggle;
    ToggleButton m_EngineInFaceIndexToggle;

    ToggleRadioGroup m_EngineInLipToggleGroup;
    ToggleButton m_EngineInLipUToggle;
    ToggleButton m_EngineInLipIndexToggle;

    ToggleRadioGroup m_EngineOutFaceToggleGroup;
    ToggleButton m_EngineOutFaceUToggle;
    ToggleButton m_EngineOutFaceIndexToggle;

    ToggleRadioGroup m_EngineOutLipToggleGroup;
    ToggleButton m_EngineOutLipUToggle;
    ToggleButton m_EngineOutLipIndexToggle;

    SliderAdjRangeInput m_EngineInFaceUSlider;
    SliderAdjRangeInput m_EngineInLipUSlider;

    Counter m_EngineInFaceCounter;
    Counter m_EngineInLipCounter;

    SliderAdjRangeInput m_EngineOutFaceUSlider;
    SliderAdjRangeInput m_EngineOutLipUSlider;

    Counter m_EngineOutFaceCounter;
    Counter m_EngineOutLipCounter;

    Choice m_EngineInModeChoice;
    Choice m_EngineOutModeChoice;

    SliderAdjRangeInput m_EngineExtendDistanceSlider;
    Choice m_EngineAutoExtensionSetChoice;
    ToggleButton m_EngineAutoExtensionFlagButton;
    int m_SelectedEngineExtensionSetChoiceIndex;

};

//==== Chevron Screen ====//
class ChevronScreen : public EngineModelScreen
{
public:
    ChevronScreen( ScreenMgr* mgr, int w, int h, const string & title, const string & helpfile = string() );
    virtual ~ChevronScreen()                               {}

    virtual bool Update( );
    virtual void CallBack( Fl_Widget *w );
    virtual void GuiDeviceCallBack( GuiDevice* device );
    static void staticScreenCB( Fl_Widget *w, void* data )
    {
        ( ( static_cast <ChevronScreen*>( data ) )->CallBack( w ) );
    }

protected:
    GroupLayout m_ModifyLayout;

    IndexSelector m_XsecModIndexSelector;

    StringInput m_ChevronXSecCurveNameInput;

    Choice m_ChevronModeChoice;

    SliderAdjRangeInput m_ChevTopAmpSlider;
    SliderAdjRangeInput m_ChevBottomAmpSlider;
    SliderAdjRangeInput m_ChevLeftAmpSlider;
    SliderAdjRangeInput m_ChevRightAmpSlider;

    SliderInput m_ChevNumberSlider;

    SliderAdjRangeInput m_ChevOnDutySlider;
    SliderAdjRangeInput m_ChevOffDutySlider;

    Choice m_ChevronExtentModeChoice;

    Choice m_ChevW01StartChoice;
    SliderAdjRangeInput m_ChevW01StartSlider;
    Choice m_ChevW01EndChoice;
    SliderAdjRangeInput m_ChevW01EndSlider;
    Choice m_ChevW01CenterChoice;
    SliderAdjRangeInput m_ChevW01CenterSlider;
    SliderAdjRangeInput m_ChevW01WidthSlider;

    SliderAdjRangeInput m_ChevDirTopAngleSlider;
    SliderAdjRangeInput m_ChevDirBottomAngleSlider;
    SliderAdjRangeInput m_ChevDirRightAngleSlider;
    SliderAdjRangeInput m_ChevDirLeftAngleSlider;

    SliderAdjRangeInput m_ChevDirTopSlewSlider;
    SliderAdjRangeInput m_ChevDirBottomSlewSlider;
    SliderAdjRangeInput m_ChevDirRightSlewSlider;
    SliderAdjRangeInput m_ChevDirLeftSlewSlider;

    ToggleButton m_ChevAngleAllSymButton;
    ToggleButton m_ChevAngleTBSymButton;
    ToggleButton m_ChevAngleRLSymButton;

    SliderAdjRangeInput m_ChevValleyRadSlider;
    SliderAdjRangeInput m_ChevPeakRadSlider;

};

class BlendScreen : public XSecScreen
{
public:
    BlendScreen( ScreenMgr* mgr, int w, int h, const string & title, const string & helpfile = string() );
    virtual ~BlendScreen()                               {}

    virtual bool Update( );
    virtual void CallBack( Fl_Widget *w );
    virtual void GuiDeviceCallBack( GuiDevice* gui_device );

    static void staticScreenCB( Fl_Widget *w, void* data )
    {
        ( static_cast <BlendScreen*> ( data ) )->CallBack( w );
    }

protected:

    GroupLayout m_BlendLayout;

    IndexSelector m_BlendIndexSelector;

    GroupLayout m_InLELayout;
    Choice m_InLEChoice;
    SliderAdjRangeInput m_InLESweep;
    SliderAdjRangeInput m_InLEDihedral;
    SliderAdjRangeInput m_InLEStrength;

    GroupLayout m_InTELayout;
    Choice m_InTEChoice;
    SliderAdjRangeInput m_InTESweep;
    SliderAdjRangeInput m_InTEDihedral;
    SliderAdjRangeInput m_InTEStrength;

    GroupLayout m_OutLELayout;
    Choice m_OutLEChoice;
    SliderAdjRangeInput m_OutLESweep;
    SliderAdjRangeInput m_OutLEDihedral;
    SliderAdjRangeInput m_OutLEStrength;

    GroupLayout m_OutTELayout;
    Choice m_OutTEChoice;
    SliderAdjRangeInput m_OutTESweep;
    SliderAdjRangeInput m_OutTEDihedral;
    SliderAdjRangeInput m_OutTEStrength;

    StringInput m_BlendXSecCurveNameInput;
};

class XSecViewScreen : public BasicScreen
{
public:
    XSecViewScreen( ScreenMgr* mgr );
    virtual ~XSecViewScreen() {}
    virtual bool Update();
    virtual void Show();
    virtual void GuiDeviceCallBack( GuiDevice* device );

protected:

    VSPGUI::VspSubGlWindow * m_GlWin;

    GroupLayout m_MainLayout;
    GroupLayout m_BorderLayout;
    GroupLayout m_ColorLayout;
    GroupLayout m_ImageLayout;

    ColorPicker m_ColorPicker;

    ToggleButton m_Image;

    StringOutput m_FileOutput;
    TriggerButton m_FileSelect;

    SliderAdjRangeInput m_WScale;
    SliderAdjRangeInput m_HScale;
    ToggleButton m_PreserveAspect;
    ToggleButton m_FlipImageToggle;

    SliderAdjRangeInput m_XOffset;
    SliderAdjRangeInput m_YOffset;

    TriggerButton m_ResetDefaults;
};

class FeaXSecScreen : public BasicScreen
{
    public:
    FeaXSecScreen( ScreenMgr* mgr );
    virtual ~FeaXSecScreen()
    {
    }
    virtual bool Update();
    virtual void Show();

    protected:

    VSPGUI::VspSubGlWindow * m_GlWin;
};

class Background3DPreviewScreen : public BasicScreen
{
public:
    Background3DPreviewScreen( ScreenMgr* mgr );
    virtual ~Background3DPreviewScreen()
    {
    }
    virtual bool Update();
    virtual void Show();
    virtual void SetZoom();

    virtual void ResizeCallBack( Fl_Widget *w );
    static void staticResizeCB( Fl_Widget *w, void* data )
    {
        static_cast< Background3DPreviewScreen* >( data )->ResizeCallBack( w );
    }

protected:

    VSPGUI::BG3DSubGlWindow * m_GlWin;
};

#endif // !defined(SCREENBASE__INCLUDED_)
