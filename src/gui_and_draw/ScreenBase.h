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

    virtual void SetFlWindow( Fl_Double_Window* win )
    {
        m_FLTK_Window = win;
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

protected:

    ScreenMgr* m_ScreenMgr;

    Fl_Double_Window* m_FLTK_Window;


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

};

//==== Tab Screen ====//
class TabScreen : public BasicScreen
{
public:

    TabScreen( ScreenMgr* mgr, int w, int h, const string & title  );
    virtual ~TabScreen();

    virtual bool Update()
    {
        return false;
    }

    virtual Fl_Group* AddTab( const string& title );
    virtual Fl_Group* GetTab( int index );

    virtual void AddTab( Fl_Group* grp );
    virtual void RemoveTab( Fl_Group* grp );

    //==== Create A Sub Group In Tab - With Border in Pixels ====//
    virtual Fl_Group* AddSubGroup( Fl_Group* group, int border  );
    virtual Fl_Scroll* AddSubScroll( Fl_Group* group, int border  );


protected:

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

    //==== Group Layouts ====//
    GroupLayout m_GenLayout;
    GroupLayout m_Density;
    GroupLayout m_Shell;

    GroupLayout m_XFormLayout;

    //==== Names, Color, Material ====//
    StringInput m_NameInput;
    ColorPicker m_ColorPicker;
    Choice m_MaterialChoice;
    Choice m_ExportNameChoice;

    //==== Tesselation ====//
    SliderInput m_NumUSlider;
    SliderInput m_NumWSlider;

    //==== Mass Props ====//
    Input m_DensityInput;
    Input m_ShellMassAreaInput;
    ToggleButton m_ThinShellButton;
    Counter m_PriorCounter;

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


};


#endif // !defined(SCREENBASE__INCLUDED_)
