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

#include "Vec3d.h"
#include "Parm.h"
#include "Vehicle.h"
#include "VehicleMgr.h"
#include <vector>
#include <string>
#include <map>

using std::string;
using std::vector;
using std::map;

class Parm;
class VspScreen;

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


class GuiDevice
{
public:

    GuiDevice();

    virtual void Init( VspScreen* screen );
    virtual void Update( const string& parm_id );
    virtual void Activate()             {}
    virtual void Deactivate()           {}
    virtual string GetParmID()
    {
        return m_ParmID;
    }
    virtual int GetType()
    {
        return m_Type;
    }

    virtual void DeviceCB( Fl_Widget* w ) = 0;
    static void StaticDeviceCB( Fl_Widget *w, void* data )
    {
        static_cast< GuiDevice* >( data )->DeviceCB( w );
    }

protected:

    virtual bool CheckValUpdate( double val );
    virtual Parm* SetParmID( const string& parm_id );
    virtual void SetValAndLimits( Parm* parm_ptr ) = 0;

    int m_Type;
    VspScreen* m_Screen;
    bool m_NewParmFlag;
    string m_ParmID;
    double m_LastVal;

};

//==== Parm Button ====//
class ParmButton : public GuiDevice
{
public:

    ParmButton();
    virtual void Init( VspScreen* screen, Fl_Button* button );
    virtual void Update( const string& parm_id );
    virtual void Activate();
    virtual void Deactivate();
    virtual void DeviceCB( Fl_Widget* w );
    virtual void SetButtonNameUpdate( bool flag )
    {
        m_ButtonNameUpdate = flag;
    }

protected:

    virtual void SetValAndLimits( Parm* parm_ptr );
    Fl_Button* m_Button;
    bool m_ButtonNameUpdate;
};

//==== Input ====//
class Input : public GuiDevice
{
public:
    Input();
    virtual void Init( VspScreen* screen, Fl_Input* input, const char* format, Fl_Button* parm_button = NULL );
    virtual void Activate();
    virtual void Deactivate();

    virtual void SetFormat( const char* format )
    {
        m_Format = format;
    }
    virtual void DeviceCB( Fl_Widget* w );
    virtual void SetButtonNameUpdate( bool flag )
    {
        m_ParmButton.SetButtonNameUpdate( flag );
    }

protected:

    virtual void SetValAndLimits( Parm* parm_ptr );

    Fl_Input* m_Input;
    char m_Str[256];
    string m_Format;

    bool m_ParmButtonFlag;
    ParmButton m_ParmButton;

};

//==== Slider ====//
class Slider : public GuiDevice
{
public:
    Slider();
    virtual void Init( VspScreen* screen, Fl_Slider* slider_widget, double range );
    virtual void Activate();
    virtual void Deactivate();
    virtual void DeviceCB( Fl_Widget* w );
    virtual void SetRange( double range )
    {
        m_Range = range;
    }

protected:

    virtual void SetValAndLimits( Parm* parm_ptr );

    Fl_Slider* m_Slider;
    double m_Range;
    double m_MinBound;
    double m_MaxBound;

};

//==== Log Slider ====//
class LogSlider : public Slider
{
public:
    LogSlider();
    virtual void DeviceCB( Fl_Widget* w );

protected:

    virtual void SetValAndLimits( Parm* parm_ptr );

};


//==== Slider Adjustable Range ====//
class SliderAdjRange : public Slider
{
public:
    SliderAdjRange();

    virtual void Init( VspScreen* screen, Fl_Slider* slider, Fl_Button* lbutton,
                       Fl_Button* rbutton, double range );

    virtual void Activate();
    virtual void Deactivate();
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
    virtual void Activate();
    virtual void Deactivate();
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
    virtual void Activate();
    virtual void Deactivate();
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
    virtual void Activate();
    virtual void Deactivate();
    virtual void DeviceCB( Fl_Widget* w );
    virtual void SetValue( int value )
    {
        m_value = value;
    }
    virtual int GetValue()
    {
        return m_value;
    }
    Fl_Button *m_Button;

protected:

    virtual void SetValAndLimits( Parm* p );

    int m_value;
};

//==== Toggle Button ====//
class ToggleButton : public GuiDevice
{
public:
    ToggleButton();
    virtual ~ToggleButton()         {}
    virtual void Init( VspScreen* screen, Fl_Button* button );
    virtual void Activate();
    virtual void Deactivate();
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
    virtual void Activate();
    virtual void Deactivate();
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
    virtual void Activate();
    virtual void Deactivate();
    virtual void DeviceCB( Fl_Widget* w );

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
    virtual void Init( VspScreen* screen, Fl_Counter* counter );
    virtual void Activate();
    virtual void Deactivate();
    virtual void DeviceCB( Fl_Widget* w );

protected:

    virtual void SetValAndLimits( Parm* p );
    Fl_Counter* m_Counter;
};

//==== Choice ====//
class Choice : public GuiDevice
{
public:

    Choice();
    virtual void Init( VspScreen* screen, Fl_Choice* fl_choice, Fl_Button* parm_button = NULL );
    virtual void Activate();
    virtual void Deactivate();
    virtual void DeviceCB( Fl_Widget* w );

    virtual void SetVal( int val );
    virtual int  GetVal();

    virtual void ClearItems()
    {
        m_Items.clear();
    }
    virtual void AddItem( const string& item )
    {
        m_Items.push_back( item );
    }
    virtual vector< string > GetItems()
    {
        return m_Items;
    }
    virtual void SetButtonNameUpdate( bool flag )
    {
        m_ParmButton.SetButtonNameUpdate( flag );
    }

protected:

    virtual void SetValAndLimits( Parm* parm_ptr );

    Fl_Choice* m_Choice;

    bool m_ParmButtonFlag;
    ParmButton m_ParmButton;

    vector< string > m_Items;


};


//==== Slider Input Combo ====//
class SliderInput
{
public:
    virtual void Init( VspScreen* screen, Fl_Slider* slider_widget, Fl_Input* input,
                       double range, const char* format, Fl_Button* parm_button = NULL,
                       bool log_slider = false );
    virtual void Update( const string& parm_id );
    virtual void Activate();
    virtual void Deactivate();
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

protected:

    Slider m_Slider;
    Input  m_Input;

    bool m_LogSliderFlag;
    LogSlider m_LogSlider;

    bool m_ParmButtonFlag;
    ParmButton m_ParmButton;
};


//==== Slider Input Combo ====//
class SliderAdjRangeInput : public GuiDevice
{
public:
    virtual void Init( VspScreen* screen, Fl_Slider* slider, Fl_Button* lbutton,
                       Fl_Button* rbutton, Fl_Input* input, double range, const char* format,
                       Fl_Button* parm_button = NULL );

    virtual void Update( const string& parm_id );
    virtual void Activate();
    virtual void Deactivate();
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
    virtual void DeviceCB( Fl_Widget* w )           {}

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
                       double range, const char* format, Fl_Button* parm_button = NULL );
    virtual void Update( int slider_id, const string& parm_id_in1, const string& parm_id_in2 );
    virtual void Activate();
    virtual void Deactivate();
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
                       double range, const char* format, Fl_Button* parm_button = NULL );

    virtual void Update( const string& parm_id );
    virtual void Activate();
    virtual void Deactivate();
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
    StringInput()                               {}
    virtual void DeviceCB( Fl_Widget* w );

    virtual void Init( VspScreen* screen, Fl_Input* input );
    virtual void Update( const string & val );

    virtual void Activate();
    virtual void Deactivate();

    virtual string GetString()
    {
        return m_String;
    }


protected:

    virtual void SetValAndLimits( Parm* parm_ptr )      {}

    string m_String;
    Fl_Input* m_Input;
};

//==== String Output =====//
class StringOutput : public GuiDevice
{
public:
    StringOutput()                                      {}
    virtual void DeviceCB( Fl_Widget* w )               {}

    virtual void Init( VspScreen* screen, Fl_Output* output );
    virtual void Update( const string & val );

    virtual void Activate()
    {
        m_Output->activate();
    }
    virtual void Deactivate()
    {
        m_Output->deactivate();
    }
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
                       Fl_Int_Input* input, Fl_Button* r_but, Fl_Button* rr_but );
    virtual void Activate();
    virtual void Deactivate();
    virtual void DeviceCB( Fl_Widget* w );

    virtual void SetIndex( int index );
    virtual int  GetIndex()
    {
        return m_Index;
    }

    virtual void SetMinMaxLimits( int min, int max );
    virtual void SetBigSmallIncrements( int big_inc, int small_inc );

protected:

    virtual void SetValAndLimits( Parm* parm_ptr );

    Fl_Button* m_LLButton;
    Fl_Button* m_LButton;
    Fl_Int_Input* m_Input;
    Fl_Button* m_RButton;
    Fl_Button* m_RRButton;

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

    virtual void Update( vec3d & rgb );
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
    }
    virtual void Activate();
    virtual void Deactivate();
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
    virtual void Activate();
    virtual void Deactivate();

    void BuildLinkableParmData();

    vector< string > FindParmNames( vector< string > & parm_id_vec );

    string GetParmChoice()
    {
        return m_ParmIDChoice;
    };
    void SetParmChoice( const string &pid )
    {
        m_ParmIDChoice = pid;
    };

protected:

    virtual void SetValAndLimits( Parm* parm_ptr )      {}

    Fl_Choice* m_ContainerChoice;
    Fl_Choice* m_GroupChoice;
    Fl_Choice* m_ParmChoice;

    string m_ParmIDChoice;

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
    Fl_Check_Button* contButtonL,
    Fl_Check_Button* contButtonR,
    Fl_Check_Button* setButtonL,
    Fl_Check_Button* setButtonR,
    Fl_Slider* sliderL,
    Fl_Slider* sliderR,
    Fl_Input* inputL,
    Fl_Input* inputR,
    Fl_Button* parm_button,
    double range, const char* format);

    virtual void Update( const string& parmL_id, const string& parmR_id );
    virtual void Activate();
    virtual void Deactivate();

protected:

    virtual void SetValAndLimits( Parm* parm_ptr )      {}

    Fl_Check_Button* m_ContButtonL;
    Fl_Check_Button* m_ContButtonR;
    Fl_Check_Button* m_SetButtonL;
    Fl_Check_Button* m_SetButtonR;

    Slider m_SliderL;
    Slider m_SliderR;
    Input  m_InputL;
    Input  m_InputR;

    ParmButton m_ParmButton;
};


//==== Fuselage Skinning Output =====//
class SkinOutput : public GuiDevice
{
public:

    enum { C0, C1, C2, NONE };

    SkinOutput();
    virtual void DeviceCB( Fl_Widget* w );

    virtual void Init( VspScreen* screen, Fl_Output* contL, Fl_Output* order, Fl_Output* contR, const vector< Fl_Button* > &buttons );
    virtual void Update( int contL, int order, int contR );

    virtual void Activate();
    virtual void Deactivate();

    virtual string ContStr( int cont );

protected:

    virtual void SetValAndLimits( Parm* parm_ptr )      {}

    string m_contL;
    string m_order;
    string m_contR;

    Fl_Output* m_ContLOutput;
    Fl_Output* m_OrderOutput;
    Fl_Output* m_ContROutput;

    vector< Fl_Button* > m_Buttons;
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

};

#endif // !defined(GUIDEVICE__INCLUDED_)
