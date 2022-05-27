//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
// J.R Gloudemans
//
//////////////////////////////////////////////////////////////////////

#if !defined(GUIDEVICE__INCLUDED_)
#define GUIDEVICE__INCLUDED_


#include <FL/Fl.H>
#include <FL/Fl_Slider.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Round_Button.H>
#include <FL/Fl_Repeat_Button.H>
#include <FL/Fl_Float_Input.H>
#include <FL/Fl_Int_Input.H>
#include <FL/Fl_Output.H>
#include <FL/Fl_Counter.H>
#include <FL/Fl_Tabs.H>
#include <FL/Fl_Browser.H>
#include <FL/Fl_Tree.H>
#include <FL/Fl_Tree_Item.H>
#include <FL/Fl_Tree_Prefs.H>
#include <FL/Fl_Scroll.H>

#include "Cartesian.H"

#include "Vec3d.h"
#include "Parm.h"
#include "Vehicle.h"
#include "VehicleMgr.h"
#include "PCurve.h"

#include <vector>
#include <string>
#include <map>

using std::string;
using std::vector;
using std::map;

class Parm;
class VspScreen;
class GroupLayout;

//====GuiDevice - Handles Interaction Between Parms and FLTK Widgets ====//

//  Handles interaction between parms and FLTK widgets.  Some GuiDevices
//  just encapsulate multiple widgets and provide a callback to VspScreen
//  thu GuiDeviceCallBack.
//
//  ParmButton      (Parm)          Button with label.
//  Input           (Parm/IntParm)  Number input.
//  Slider          (Parm/IntParm)  Normal slider
//  Log Slider      (Parm/IntParm)  Log10 slider
//  SliderAdjRange  (Parm/IntParm)  Slider with additional buttons to adjust range
//  Check Button    (BoolParm)      Toggle type button with check mark
//  CheckButtonBit  (IntParm)       Check button with val (0 or val)
//  RadioButton     (IntParm)       Round radio button
//  Toggle Button   (BoolParm)      Toggle type button with light
//  ToggleRadioGroup (IntParm)      Group of toggle buttons that act like radio buttons.
//  TriggerButton   (None)          Button that triggers a GuiDeviceCallBack to VspScreen (no Parm)
//  Counter         (Parm)          Fltk counter widget
//  Choice          (IntParm)       Fltk choice widget
//  SliderInput     (Parm)          Combo of Slider (or LogSlider) and Input and optional Parm Button
//  SliderAdjRangeInput (Parm)      Combo of SliderAdjRange and Input and optional Parm Button
//  SliderAdjRange2Input (2 Parms)  Combo of SliderAdjRange and two Inputs and optional Parm Button.
//                                  Display two different parms in each window (only one active at a time)
//  FractParmSlider (FractParm)     Combo of SliderAdjRange and two Inputs and optional Parm Button.
//                                      The second input display the fractional value.
//  StringInput     (None)          Set and display text (string) triggers a GuiDeviceCallBack to VspScreen
//  IndexSelector   (IntParm)       Display and set integers. Composed of input and 5 arrow buttons.
//  ColorPicker     (None)          Display, edit and pick colors.  Composed of buttons and sliders.
//                                      Triggers a GuiDeviceCallBack to VspScreen
//  ParmPicker      (None)          Container, group, and parm pulldowns for selecting parameters
//  DriverGroupBank (None)          Bank of SliderInputs with radio button controls to implement driver group
//  SkinControl     (2 Parms)       Compound control for fuselage skinning
//  SkinOutput      (None)          Fuselage skinning compound output
//  ColResizeBrowser (None)         Fl_Browser with resizeable columns. Based on the code provided at Erco's 
//                                      FLTK Cheat Page: http://seriss.com/people/erco/fltk/#Fl_Resize_Browser

void fltk_unicode_subscripts( string & str );
void fltk_unicode_plusminus( string & str );

class VspSlider : public Fl_Slider
{
public:
    VspSlider(int x, int y, int w, int h, const char *label = 0 );
    int handle( int event );

    bool GetButtonPush()                    { return m_ButtonPush; }
    void SetButtonPush( bool f )            { m_ButtonPush = false; }
protected:

    bool m_ButtonPush;

};

class VspButton : public Fl_Button
{
public:
    VspButton(int X, int Y, int W, int H, const char *L = 0);
    int handle( int event );
};

class Vsp_Group : public Fl_Group
{
public:
    Vsp_Group( int x, int y, int w, int h );
    int handle( int event );

    void SetAllowDrop( bool d )     { m_AllowDrop = d; }
protected:
    bool m_AllowDrop;
};

class Vsp_Canvas : public Ca_Canvas
{
public:
    Vsp_Canvas(int x, int y, int w, int h, const char *label=0);

    int handle( int event );

    void SetMainY();
    void SetSecondaryY();
    void HideSecondaryY();
    void ShowSecondaryY();

    Ca_Y_Axis* m_Ymain;
    Ca_Y_Axis* m_Ysecondary;

    int m_Wwide;
    int m_Wnarrow;
};



class GuiDevice
{
public:

    GuiDevice();

    virtual void Init( VspScreen* screen );
    virtual void Update( const string& parm_id );
    virtual void Activate();
    virtual void Deactivate();
    virtual string GetParmID()          { return m_ParmID; }
    virtual int GetType()               { return m_Type; }
    virtual void SetWidth( int w ) ;
    virtual int GetWidth();
    virtual int GetX();
    virtual void SetX( int x );
    virtual void OffsetX( int x );
    virtual void SetIndex( int i )      { m_Index = i; }
    virtual int  GetIndex()             { return m_Index; }

    virtual void DeviceCB( Fl_Widget* w ) = 0;
    static void StaticDeviceCB( Fl_Widget *w, void* data )
    {
        static_cast< GuiDevice* >( data )->DeviceCB( w );
    }

protected:

    virtual bool CheckValUpdate( double val );
    virtual Parm* SetParmID( const string& parm_id );
    virtual void SetValAndLimits( Parm* parm_ptr ) = 0;

    //==== First Widget Is Assumed Resizable For Set Width =====//
    virtual void AddWidget( Fl_Widget* w, bool resizable_flag = false );
    virtual void ClearAllWidgets()                      { m_WidgetVec.clear(); }

    int m_Type;
    int m_Index;
    VspScreen* m_Screen;
    bool m_NewParmFlag;
    string m_ParmID;
    double m_LastVal;

    int m_ResizableWidgetIndex;
    vector< Fl_Widget* > m_WidgetVec;

};

//==== Parm Button ====//
class ParmButton : public GuiDevice
{
public:

    ParmButton();
    virtual void Init( VspScreen* screen, VspButton* button );
    virtual void Update( const string& parm_id );
    virtual void UpdateButtonName( const string & name );
    virtual void DeviceCB( Fl_Widget* w );
    virtual void SetButtonNameUpdate( bool flag )   { m_ButtonNameUpdate = flag; }

    virtual void SetLabelColor( Fl_Color color )
    {
        m_Button->labelcolor( color );
    }

    virtual void ResetLabelColor()
    {
        m_Button->labelcolor( FL_BLACK );
    }

protected:

    virtual void SetValAndLimits( Parm* parm_ptr );
    VspButton* m_Button;
    bool m_ButtonNameUpdate;
};

//==== Input ====//
class Input : public GuiDevice
{
public:
    Input();
    virtual void Init( VspScreen* screen, Fl_Input* input, const char* format, VspButton* parm_button = NULL );

    virtual void SetFormat( const char* format )
    {
        m_Format = format;
    }
    virtual void DeviceCB( Fl_Widget* w );
    virtual void SetButtonNameUpdate( bool flag )
    {
        m_ParmButton.SetButtonNameUpdate( flag );
    }

    virtual void SetLabelColor( Fl_Color color )
    {
        m_ParmButton.SetLabelColor( color );
    }

    virtual void ResetLabelColor()
    {
        m_ParmButton.ResetLabelColor();
    }

protected:

    virtual void SetValAndLimits( Parm* parm_ptr );

    Fl_Input* m_Input;
    char m_Str[256];
    string m_Format;

    bool m_ParmButtonFlag;
    ParmButton m_ParmButton;

};

//==== Output ====//
class Output : public GuiDevice
{
public:
    Output();
    virtual void DeviceCB( Fl_Widget* w )               {}
    virtual void Init( VspScreen* screen, Fl_Output* output, const char* format, VspButton* parm_button = NULL );

    virtual void SetFormat( const char* format )
    {
        if ( m_Format.compare( format ) != 0 )
        {
            m_Format = format;
            m_NewFormat = true;
        }
    }

    virtual void SetSuffix( string sfx )
    {
        if ( m_Suffix.compare( sfx ) != 0 )
        {
            m_Suffix = sfx;
            m_NewFormat = true;
        }
    }

    virtual void SetButtonNameUpdate( bool flag )
    {
        m_ParmButton.SetButtonNameUpdate( flag );
    }

protected:

    virtual void SetValAndLimits( Parm* parm_ptr );

    Fl_Output* m_Output;
    char m_Str[256];
    string m_Format;
    string m_Suffix;
    bool m_NewFormat;

    bool m_ParmButtonFlag;
    ParmButton m_ParmButton;

};

//==== Slider ====//
class Slider : public GuiDevice
{
public:
    Slider();
    virtual void Init( VspScreen* screen, Fl_Slider* slider_widget, double range, bool log_slider = false );
    virtual void DeviceCB( Fl_Widget* w );
    virtual void SetRange( double range )
    {
        m_Range = range;
    }

    virtual void SetMinBound( double minb )
    {
        m_MinBound = minb;
        m_Range = m_MaxBound - m_MinBound;
        m_Slider->bounds( m_MinBound, m_MaxBound );
    }

    virtual void SetMaxBound( double maxb )
    {
        m_MaxBound = maxb;
        m_Range = m_MaxBound - m_MinBound;
        m_Slider->bounds( m_MinBound, m_MaxBound );
    }

protected:

    virtual void SetValAndLimits( Parm* parm_ptr );

    VspSlider* m_Slider;
//    Fl_Slider* m_OldSlider;
    double m_Range;
    double m_MinBound;
    double m_MaxBound;

    bool m_LogSliderFlag;

};

//==== Slider Adjustable Range ====//
class SliderAdjRange : public Slider
{
public:
    SliderAdjRange();

    virtual void Init( VspScreen* screen, Fl_Slider* slider, Fl_Button* lbutton,
                       Fl_Button* rbutton, double range, bool log_slider = false );

    virtual void DeviceCB( Fl_Widget* w );


protected:


    virtual void SetValAndLimits( Parm* parm_ptr );
    virtual void MinButtonCB( Parm* parm_ptr );
    virtual void MaxButtonCB( Parm* parm_ptr );
    virtual void FindStopState( Parm* parm_ptr );

    Fl_Button* m_MinButton;
    Fl_Button* m_MaxButton;

    enum { SAR_NO_STOP, SAR_STOP, SAR_ABS_STOP };


    int m_MinStopState;
    int m_MaxStopState;
    double m_ButtonChangeFract;
    double m_Tol;


};


//==== Check Button ====//
class CheckButton : public GuiDevice
{
public:

    CheckButton();
    virtual void Init( VspScreen* screen, Fl_Check_Button* button );
    virtual void DeviceCB( Fl_Widget* w );

protected:

    virtual void SetValAndLimits( Parm* parm_ptr );

    Fl_Check_Button* m_Button;

};

//==== Check Button Bit Flag ====//
class CheckButtonBit : public GuiDevice
{
public:

    CheckButtonBit();
    virtual void Init( VspScreen* screen, Fl_Button* button, int value );
    virtual void DeviceCB( Fl_Widget* w );

protected:

    virtual void SetValAndLimits( Parm* parm_ptr );

    Fl_Button* m_Button;
    int m_value;

};

//==== Radio Button ====//
class RadioButton : public GuiDevice
{
public:
    RadioButton();
    virtual void Init( VspScreen* screen, Fl_Button* button, int value );
    virtual void DeviceCB( Fl_Widget* w );
    virtual void SetValue( int value )
    {
        m_value = value;
    }
    virtual int GetValue()
    {
        return m_value;
    }
    virtual Fl_Button* GetFlButton()
    {
        return m_Button;
    }
    Fl_Button *m_Button;

protected:

    virtual void SetValAndLimits( Parm* p );

    int m_value;
};

//==== Toggle Button ====//
// WARNING: Updating a ToggleButton with an IntParm when the range of the IntParm 
// is 0 to 1 will cause the button to initially appear Off when the Parm value is 
// 1. If this is the case, the IntParm should be changed to a BoolParm if the 
// ToggleButton is not part of a ToggleRadioGroup.
class ToggleButton : public GuiDevice
{
public:
    ToggleButton();
    virtual ~ToggleButton()         {}
    virtual void Init( VspScreen* screen, Fl_Button* button );
    virtual void DeviceCB( Fl_Widget* w );

    virtual Fl_Button* GetFlButton()
    {
        return m_Button;
    }

protected:

    virtual void SetValAndLimits( Parm* p );
    Fl_Button* m_Button;
};

//==== Toggle Button Radio Group ====//
class ToggleRadioGroup : public GuiDevice
{
public:
    ToggleRadioGroup();
    virtual ~ToggleRadioGroup()         {}
    virtual void Init( VspScreen* screen );
    virtual void AddButton( Fl_Button* button );
    virtual void ClearButtons();
    virtual void DeviceCB( Fl_Widget* w );

    virtual void SetValMapVec( vector< int > & val_map_vec );

protected:

    virtual void SetValAndLimits( Parm* p );
    vector< Fl_Button* > m_ButtonVec;
    vector< int > m_ValMapVec;
};


//==== Trigger Button ====//
class TriggerButton : public GuiDevice
{
public:
    TriggerButton();
    virtual ~TriggerButton() {}
    virtual void Init( VspScreen* screen, Fl_Button* button );
    virtual void DeviceCB( Fl_Widget* w );
    virtual void SetColor( Fl_Color c );
    virtual Fl_Button* GetFlButton();

protected:

    virtual void SetValAndLimits( Parm* p )                      {} // Do Nothing
    Fl_Button* m_Button;
};

//==== Counter ====//
class Counter : public GuiDevice
{
public:
    Counter();
    virtual ~Counter() {}
    virtual void Init( VspScreen* screen, Fl_Counter* counter, VspButton* parm_button = NULL );
    virtual void DeviceCB( Fl_Widget* w );

protected:

    virtual void SetValAndLimits( Parm* p );
    Fl_Counter* m_Counter;

    bool m_ParmButtonFlag;
    ParmButton m_ParmButton;

};

//==== Choice ====//
class Choice : public GuiDevice
{
public:

    Choice();
    virtual void Init( VspScreen* screen, Fl_Choice* fl_choice, VspButton* parm_button = NULL );
    virtual void DeviceCB( Fl_Widget* w );

    virtual void SetVal( int val );
    virtual int  GetVal();

    virtual void ClearItems()
    {
        m_Items.clear();
        m_Vals.clear();
    }
    virtual void AddItem( const string& item )
    {
        m_Items.push_back( item );
        m_Vals.push_back( m_Vals.size() );
    }

    virtual void AddItem( const string& item, int val )
    {
        m_Items.push_back( item );
        m_Vals.push_back( val );
    }

    virtual void SetFlag( int indx, int flag );
    virtual int GetFlag( int indx );
    virtual void SetFlagByVal( int val, int flag );
    virtual int GetFlagByVal( int val );
    virtual void ClearFlags();

    virtual vector< string > GetItems()
    {
        return m_Items;
    }
    virtual void UpdateItems( bool keepsetting = false );

    virtual void SetButtonNameUpdate( bool flag )
    {
        m_ParmButton.SetButtonNameUpdate( flag );
    }
    virtual void SetWidth( int w );

    virtual Fl_Choice* GetFlChoice()
    {
        return m_Choice;
    }

protected:

    virtual int ValToIndex( int val );
    virtual int IndexToVal( int indx );

    virtual void SetValAndLimits( Parm* parm_ptr );

    Fl_Choice* m_Choice;

    bool m_ParmButtonFlag;
    ParmButton m_ParmButton;

    vector< string > m_Items;
    vector< int > m_Flags;
    vector< int > m_Vals;
};


//==== Slider Input Combo ====//
class SliderInput : public GuiDevice
{
public:
    virtual void Init( VspScreen* screen, Fl_Slider* slider_widget, Fl_Input* input,
                       double range, const char* format, VspButton* parm_button = NULL,
                       bool log_slider = false );
    virtual void Update( const string& parm_id );
    virtual void SetRange( double range )
    {
        m_Slider.SetRange( range );
    }
    virtual void SetFormat( const char* format )
    {
        m_Input.SetFormat( format );
    }
    virtual void SetButtonNameUpdate( bool flag )
    {
        m_ParmButton.SetButtonNameUpdate( flag );
    }
    virtual void SetIndex( int index )
    {
        m_Slider.SetIndex( index );
        m_Input.SetIndex( index );
        m_ParmButton.SetIndex( index );
    }
    virtual void DeviceCB( Fl_Widget* w )           {}

protected:

    virtual void SetValAndLimits( Parm* )           {}

    Slider m_Slider;
    Input  m_Input;

    bool m_ParmButtonFlag;
    ParmButton m_ParmButton;
};


//==== Slider Input Combo ====//
class SliderAdjRangeInput : public GuiDevice
{
public:
    virtual void Init( VspScreen* screen, Fl_Slider* slider, Fl_Button* lbutton,
                       Fl_Button* rbutton, Fl_Input* input, double range, const char* format,
                       VspButton* parm_button = NULL, bool log_slider = false );

    virtual void Update( const string& parm_id );
    virtual void SetRange( double range )
    {
        m_Slider.SetRange( range );
    }
    virtual void SetMinBound( double minb )
    {
        m_Slider.SetMinBound( minb );
    }
    virtual void SetMaxBound( double maxb )
    {
        m_Slider.SetMaxBound( maxb );
    }
    virtual void SetFormat( const char* format )
    {
        m_Input.SetFormat( format );
    }
    virtual void SetButtonName( const string & name )
    {
        m_ParmButton.UpdateButtonName( name );
    }
    virtual void SetButtonNameUpdate( bool flag )
    {
        m_ParmButton.SetButtonNameUpdate( flag );
    }
    virtual void SetIndex( int index )
    {
        m_Slider.SetIndex( index );
        m_Input.SetIndex( index );
        m_ParmButton.SetIndex( index );
    }
    virtual void DeviceCB( Fl_Widget* w )           {}

    virtual void SetLabelColor( Fl_Color color )
    {
        m_ParmButton.SetLabelColor( color );
    }

    virtual void ResetLabelColor()
    {
        m_ParmButton.ResetLabelColor();
    }

protected:

    virtual void SetValAndLimits( Parm* )           {}


    SliderAdjRange m_Slider;
    Input  m_Input;

    bool m_ParmButtonFlag;
    ParmButton m_ParmButton;

};

//==== Slider 2 Input Combo ====//
class SliderAdjRange2Input
{
public:
    virtual void Init( VspScreen* screen, Fl_Slider* slider, Fl_Button* lbutton,
                       Fl_Button* rbutton, Fl_Input* input1, Fl_Input* input2,
                       double range, const char* format, VspButton* parm_button = NULL );
    virtual void Update( int slider_id, const string& parm_id_in1, const string& parm_id_in2 );
    virtual void SetRange( double range )
    {
        m_Slider.SetRange( range );
    }
    virtual void SetFormat( const char* format )
    {
        m_Input1.SetFormat( format );
        m_Input2.SetFormat( format );
    }
    virtual void SetButtonNameUpdate( bool flag )
    {
        m_ParmButton.SetButtonNameUpdate( flag );
    }
    virtual void SetIndex( int index )
    {
        m_Slider.SetIndex( index );
        m_Input1.SetIndex( index );
        m_Input2.SetIndex( index );
        m_ParmButton.SetIndex( index );
    }


    virtual void Activate();
    virtual void Deactivate();


protected:
    SliderAdjRange m_Slider;
    Input m_Input1;
    Input m_Input2;

    bool m_ParmButtonFlag;
    ParmButton m_ParmButton;
};


//==== Fract Slider Input Input Combo  ====//
class FractParmSlider : public GuiDevice
{
public:
    FractParmSlider();

    virtual void Init( VspScreen* screen, Fl_Slider* slider, Fl_Button* lbutton,
                       Fl_Button* rbutton, Fl_Input* fract_input, Fl_Input* result_input,
                       double range, const char* format, VspButton* parm_button = NULL );

    virtual void Update( const string& parm_id );
    virtual void SetRange( double range )
    {
        m_Slider.SetRange( range );
    }
    virtual void SetFractFormat( const char* format )
    {
        m_FractInput.SetFormat( format );
    }
//  virtual void SetResultFormat( const char* format )  { m_ResultFlInput.SetFormat( format ); }

    virtual void DeviceCB( Fl_Widget* w );
    virtual void SetButtonNameUpdate( bool flag )
    {
        m_ParmButton.SetButtonNameUpdate( flag );
    }
    virtual void SetIndex( int index )
    {
        m_Slider.SetIndex( index );
        m_FractInput.SetIndex( index );
    }


protected:

    SliderAdjRange m_Slider;
    Input  m_FractInput;

    Fl_Input* m_ResultFlInput;
    char m_Str[256];
    string m_Format;

    bool m_ParmButtonFlag;
    ParmButton m_ParmButton;

    virtual void SetValAndLimits( Parm* parm_ptr );

};

////==== Radio Button Group ====//
//class RadioButtonGroup
//{
//public:
//  RadioButtonGroup();
//  ~RadioButtonGroup();
//  virtual void AddButton(VspScreen* screen, Fl_Round_Button* button, int value);
//  virtual void Update(const string& parm_id);
//  virtual void Activate();
//  virtual void Deactivate();
//
//protected:
//
//  std::vector<RadioButton*> m_buttonVec;
//};

//==== String Input  ====//
class StringInput : public GuiDevice
{
public:
    StringInput()
    {
        m_String = "";
        m_Input = NULL;
    }
    virtual void DeviceCB( Fl_Widget* w );

    virtual void Init( VspScreen* screen, Fl_Input* input );
    virtual void Update( const string & val );

    virtual string GetString()
    {
        return m_String;
    }

    void SetTextFont( Fl_Font font );

protected:

    virtual void SetValAndLimits( Parm* parm_ptr )      {}

    string m_String;
    Fl_Input* m_Input;
};



//==== String Output =====//
class StringOutput : public GuiDevice
{
public:
    StringOutput()
    {
        m_String = "";
        m_Output = NULL;
    }
    virtual void DeviceCB( Fl_Widget* w )               {}

    virtual void Init( VspScreen* screen, Fl_Output* output, Fl_Button* button = NULL );
    virtual void Update( const string & val );

    virtual string GetString()
    {
        return m_String;
    }

protected:

    virtual void SetValAndLimits( Parm* parm_ptr )      {}

    string m_String;
    Fl_Output* m_Output;
};


//==== Index Selector  ====//
class IndexSelector : public GuiDevice
{
public:

    IndexSelector();
    virtual void Init( VspScreen* screen, Fl_Button* ll_but,  Fl_Button* l_but,
                       Fl_Int_Input* input, Fl_Button* r_but, Fl_Button* rr_but,
                       VspButton* parm_button = NULL );
    virtual void DeviceCB( Fl_Widget* w );

    virtual void Update( const string & val );

    virtual void SetWidth( int w ) ;

    virtual void SetIndex( int index );
    virtual int  GetIndex()                 { return m_Index; }

    virtual void SetMinMaxLimits( int min, int max );
    virtual void SetBigSmallIncrements( int big_inc, int small_inc );

    virtual int GetMinIndex()               { return m_MinIndex; }
    virtual int GetMaxIndex()               { return m_MaxIndex; }

protected:

    virtual void SetValAndLimits( Parm* parm_ptr );

    Fl_Button* m_LLButton;
    Fl_Button* m_LButton;
    Fl_Int_Input* m_Input;
    Fl_Button* m_RButton;
    Fl_Button* m_RRButton;

    bool m_ParmButtonFlag;
    ParmButton m_ParmButton;

    int m_Index;
    int m_MinIndex;
    int m_MaxIndex;
    int m_BigInc;
    int m_SmallInc;
    char m_Str[64];

};


class ColorPicker : public GuiDevice
{
public:

    ColorPicker();

    virtual void DeviceCB( Fl_Widget *w );

    virtual void Init( VspScreen* screen, Fl_Button* title, Fl_Button* result,
                       vector< Fl_Button* > buttons, Fl_Slider* rgb_sliders[3] );

    virtual void Update( const vec3d & rgb );
    virtual vec3d GetColor()
    {
        return m_Color;
    }
    virtual vec3d GetIndexRGB( int index );

protected:

    virtual void SetValAndLimits( Parm* parm_ptr )      {}

    Fl_Button* m_ColorResult;
    vector< Fl_Button* > m_ColorButtons;
    Fl_Slider* m_RGB_Sliders[3];

    vec3d m_Color;


};

class Group : public GuiDevice
{
public:
    Group();

    virtual void Init( Fl_Group* g )
    {
        m_Group = g;
        AddWidget( g );
    }
    virtual void Hide();
    virtual void Show();

    virtual void DeviceCB( Fl_Widget* w )           {}

protected:

    virtual void SetValAndLimits( Parm* parm_ptr )  {}

    Fl_Group* m_Group;
};

class Tab : public Group
{
public:
    Tab();

    virtual Fl_Group* GetGroup()
    {
        return m_Group;
    }

protected:

};

class ParmPicker : public GuiDevice
{
public:

    ParmPicker();


    virtual void DeviceCB( Fl_Widget *w );

    virtual void Init( VspScreen* screen, Fl_Choice* container_choice,
                       Fl_Choice* group_choice, Fl_Choice* parm_choice );

    virtual void Update( );

    static vector< string > FindParmNames( vector< string > & parm_id_vec );

    string GetParmChoice()
    {
        return m_ParmIDChoice;
    };
    void SetParmChoice( const string &pid )
    {
        m_ParmIDChoice = pid;
    };

    void Activate();
    void Deactivate();

protected:

    virtual void SetValAndLimits( Parm* parm_ptr )      {}

    Fl_Choice* m_ContainerChoice;
    Fl_Choice* m_GroupChoice;
    Fl_Choice* m_ParmChoice;

    string m_ParmIDChoice;

};

class ParmTreePicker : public GuiDevice
{
protected:
    struct ParmTreeData
    {
        bool m_Flag;
        Fl_Tree_Item *m_TreeItemPtr;
        Fl_Check_Button *m_Button;
    };
    typedef map< string, ParmTreeData > ParmTreeMap;
    typedef ParmTreeMap::iterator ParmTreeIt;

    struct GroupTreeData
    {
        string m_GroupName;
        bool m_Flag;
        Fl_Tree_Item *m_TreeItemPtr;
    };
    typedef vector< GroupTreeData > GroupTreeVec;

    struct ContainerTreeData
    {
        bool m_Flag;
        Fl_Tree_Item *m_TreeItemPtr;

        GroupTreeVec m_GroupVec;
        ParmTreeMap m_ParmMap;
    };
    typedef map< string, ContainerTreeData > ContainerTreeMap;
    typedef ContainerTreeMap::iterator ContainerTreeIt;

    ContainerTreeMap m_TreeData;

public:

    enum { NONE, SELECT, UNSELECT };

    ParmTreePicker();


    virtual void DeviceCB( Fl_Widget *w );

    virtual void Init( VspScreen* screen, Fl_Tree* parm_tree );

    virtual void Update( const vector< string > &selected_ids );

    void Activate();
    void Deactivate();

    int GetEventType()
    {
        return m_EventType;
    }
    string GetEventParm()
    {
        return m_EventParm;
    }

protected:

    int m_EventType;
    string m_EventParm;


    virtual void SetValAndLimits( Parm* parm_ptr )      {}

    Fl_Tree* m_ParmTree;
    Fl_Tree_Prefs m_ParmTreePrefs;

    virtual void RemoveParm( const string &ParmID, ContainerTreeData &td );
    virtual int FindGroup( Fl_Tree_Item* groupitem, const GroupTreeVec &m_GroupVec);
    virtual vector< int > FindGroup( const string &GroupName, const GroupTreeVec &m_GroupVec);
    virtual void CleanGroupVec(  GroupTreeVec &m_GroupVec );
    virtual void RemoveContainer( const string &ContID );
    virtual void AddParmEntry( const string &ParmID, ContainerTreeData &conttree, Fl_Tree_Item* groupitem );
    virtual void AddGroupEntry( const string &GroupName, ContainerTreeData &conttree );
    virtual void AddContEntry( const string &ContID );
    virtual void ResetFlag( bool flg );
    virtual void CleanGarbage();
    virtual void UpdateParmTree();
    virtual void UnselectAll();
};

class DriverGroupBank : public GuiDevice
{
public:
    DriverGroupBank( );

    virtual void DeviceCB( Fl_Widget *w );

    virtual void Init( VspScreen* screen, vector< vector < Fl_Button* > > buttons, vector< SliderAdjRangeInput* > sliders );

    virtual void Update( vector< string > & parm_ids );
    virtual void Activate();
    virtual void Deactivate();

    virtual bool WhichButton( Fl_Widget *w, int &imatch, int &jmatch );

    void SetDriverGroup( DriverGroup *dg )
    {
        m_DriverGroup = dg;
    }
    DriverGroup* GetDriverGroup()
    {
        return m_DriverGroup;
    }

    void EnforceXSecGeomType( int geom_type ); // Called from CurveEditScreen to disable drivers for Wing and Prop

protected:

    virtual void SetValAndLimits( Parm* parm_ptr )      {}

    vector< vector < Fl_Button* > > m_Buttons;
    vector< SliderAdjRangeInput* > m_Sliders;

    DriverGroup *m_DriverGroup;
};

class SkinControl : public GuiDevice
{
public:
    SkinControl( );

    virtual void DeviceCB( Fl_Widget *w );

    virtual void Init( VspScreen* screen,
    Fl_Check_Button* setButtonL,
    Fl_Check_Button* setButtonEqual,
    Fl_Check_Button* setButtonR,
    Fl_Slider* sliderL,
    Fl_Button* minButtonL,
    Fl_Button* maxButtonL,
    Fl_Slider* sliderR,
    Fl_Button* minButtonR,
    Fl_Button* maxButtonR,
    Fl_Input* inputL,
    Fl_Input* inputR,
    VspButton* parm_button_L,
    VspButton* parm_button_R,
    double range, const char* format);

    virtual void Update( const string& parmL_id, const string& setL_id, const string& eq_id, const string& setR_id, const string& parmR_id );
    virtual void Activate();
    virtual void Deactivate();
    virtual void DeactivateLeft();
    virtual void DeactivateRight();
    virtual void DeactivateEqual();
    virtual void DeactivateSet();
    virtual void DeactivateLSet();
    virtual void DeactivateRSet();

protected:

    virtual void SetValAndLimits( Parm* parm_ptr )      {}

    CheckButton m_SetButtonL;
    CheckButton m_SetButtonEqual;
    CheckButton m_SetButtonR;

    SliderAdjRange m_SliderL;
    SliderAdjRange m_SliderR;
    Input  m_InputL;
    Input  m_InputR;

    ParmButton m_ParmButtonL;
    ParmButton m_ParmButtonR;
};

class SkinHeader : public GuiDevice
{
public:
    SkinHeader();
    ~SkinHeader();

    virtual void DeviceCB( Fl_Widget *w );

    virtual void Init( VspScreen* screen, Choice* cont_choice , const vector< VspButton* > &buttons );

    virtual void Activate();
    virtual void Deactivate();
    virtual void DeactiveContChoice();

    Choice* m_ContChoice;

protected:

    virtual void SetValAndLimits( Parm* parm_ptr )      {}

    vector< VspButton* > m_Buttons;
};

class GeomPicker : public GuiDevice
{
public:

    GeomPicker();


    virtual void DeviceCB( Fl_Widget *w );

    virtual void Init( VspScreen* screen, Fl_Choice* geom_choice );

    virtual void Update( );
    virtual void Activate();
    virtual void Deactivate();

    virtual void AddExcludeType( int type );
    virtual void ClearExcludeType();

    virtual void AddIncludeType( int type );
    virtual void ClearIncludeType();

    string GetGeomChoice()
    {
        return m_GeomIDChoice;
    };
    void SetGeomChoice( const string &gid )
    {
        m_GeomIDChoice = gid;
    };

protected:

    virtual void SetValAndLimits( Parm* parm_ptr )      {}

    Fl_Choice* m_GeomChoice;

    string m_GeomIDChoice;

    vector< string > m_GeomVec;

    Vehicle * m_Vehicle;

    vector < int > m_ExcludeTypes;
    vector < int > m_IncludeTypes;

};

// Base class for curve editing canvas
class PCurveEditor : public GuiDevice
{
public:

    PCurveEditor();
    ~PCurveEditor();

    // Initialize the member GUI devices and set their callbacks
    virtual void Init( VspScreen* screen, Vsp_Canvas* canvas, Fl_Scroll* ptscroll, Fl_Button* spbutton, Fl_Button* convbutton, Fl_Button* delbutton, Fl_Light_Button* delpickbutton, Fl_Light_Button* splitpickbutton, GroupLayout* ptlayout );

    // Used to set delete active or split active flags 
    virtual void DeviceCB( Fl_Widget* w );

    // Check if two locations are within an input radius 'r'. Used to identify
    // if the selected pixel on the canvas is close enough to a control point
    virtual bool hittest( int mx, int my, double datax, double datay, int r );

    // Add the curve points to the canvas and connect them. CEDIT intermediate points are
    // drawn green. The currently selected point is highlighted red. 
    virtual void PlotData( vector< double > x_data, vector < double > y_data, int curve_type, Fl_Color color = FL_YELLOW );

    // Identify the min and max limits for the canvas and update accordingly
    virtual void UpdateAxisLimits( vector< double > x_data, vector < double > y_data, bool mag_round = true );

    // Clear and reinitialize the parm sliders. Add G1 continuity check boxes for CEDIT. 
    // This function must be called in the size of the curve parameter vectors chenges
    // and when a type conversion is performed
    virtual void RedrawXYSliders( int num_pts, int curve_type );

    // Search for a point index within radius 'r' to the input pixel location
    virtual int ihit( int mx, int my, int r );

    virtual void Update( PCurve *curve );
    virtual void Update( PCurve *curve, PCurve *curveb, string labelb );

    // GUI devices for the curve editor
    SliderAdjRangeInput m_SplitPtSlider;
    Fl_Button* m_SplitButton;
    Fl_Button* m_DelButton;
    Fl_Light_Button* m_DelPickButton;
    Fl_Light_Button* m_SplitPickButton;
    IndexSelector m_PntSelector;
    Choice m_ConvertChoice;
    Fl_Button* m_ConvertButton;
    StringOutput m_CurveType;

protected:

    virtual void SetValAndLimits( Parm* parm_ptr ) {}

    // Restricts the index selecttor from cycling through intermediate control 
    // points. If an intermediate control point is selected, the delete button
    // is deactivated
    virtual void UpdateIndexSelector( int curve_type );

    virtual void Update();

    int m_LastHit; // Index of the previously selected point

    Vsp_Canvas* m_canvas;
    Fl_Scroll* m_PtScroll;
    GroupLayout* m_PtLayout;

    // Vector of sliders to match the curve parameter vectors
    vector < vector < SliderAdjRangeInput > > m_SliderVecVec;
    vector < CheckButton > m_EnforceG1Vec;

    bool m_FreezeAxis; // Restrict the canvas from resizing when performing a click and drag operation

    // Flags to delete or split after the curve is clicked on
    bool m_DeleteActive;
    bool m_SplitActive;

    // Keeps track of previous curve type. If changed, redraw sliders
    int m_PrevCurveType;

    // Variables to help with CEDIT point selection restrictions
    int m_PrevIndex; // Maintains the previously selected point index
    bool m_CallbackFlag; // Flag that indicates if a Callback was issued prior to Update
    bool m_UpdateIndexSelector; // Flag used to indicate if the index selector should be updated

    PCurve *m_Curve;
    PCurve *m_CurveB;
    string m_LabelB;
};

class ColResizeBrowser : public Fl_Browser
{
public:

    ColResizeBrowser( int X, int Y, int W, int H, const char* L = 0 );


    // GET/SET COLUMN SEPARATOR LINE COLOR
    Fl_Color GetColSepColor() const {
        return( m_ColSepColor );
    }
    void SetColSepColor( Fl_Color val ) {
        m_ColSepColor = val;
    }

    // GET/SET DISPLAY OF COLUMN SEPARATOR LINES
    //     1: show lines, 0: don't show lines
    //
    bool GetShowColSepFlag() const {
        return( m_ShowColSepFlag );
    }
    void SetShowColSepFlag( bool val ) {
        m_ShowColSepFlag = val;
    }

    // GET/SET COLUMN WIDTHS ARRAY
    //    Just like fltk method, but array is non-const.
    //
    int* column_widths() const {
        return( m_Widths );
    }
    void column_widths( int* width_ptr ) {
        m_Widths = width_ptr;
        Fl_Browser::column_widths( width_ptr );
    }

    void num_col( size_t num )
    {
        m_NumCol = num;
    }

protected:

    int handle( int e ); // MANAGE EVENTS TO HANDLE COLUMN RESIZING

    void draw();

private:

    // CHANGE CURSOR
    //     Does nothing if cursor already set to value specified.
    void change_cursor( Fl_Cursor newcursor );

    // RETURN THE COLUMN MOUSE IS 'NEAR'
    //     Returns -1 if none.
    int which_col_near_mouse();

    // FORCE SCROLLBAR RECALC
    //    Prevents scrollbar from getting out of sync during column drags
    void recalc_hscroll();

    Fl_Color  m_ColSepColor;     // color of column separator lines 
    bool      m_ShowColSepFlag;  // flag to enable drawing column separators
    Fl_Cursor m_LastCursor;      // saved cursor state info
    int       m_DragCol;         // col# user is dragging (-1 = not dragging)
    int*      m_Widths;          // pointer to user's width[] array
    size_t    m_NumCol;          // number of columns
    int       m_HPos;            // Horizontal scroll position

};

#endif // !defined(GUIDEVICE__INCLUDED_)
