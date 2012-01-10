//******************************************************************************
//    
//   Input/Display Device Base Class
//   
//   J.R. Gloudemans - 8/26/93
//******************************************************************************

#ifndef GUI_DEVICE_H
#define GUI_DEVICE_H

#include "FL/Fl_Slider.H"
#include "FL/Fl_Button.H"
#include "FL/Fl_Input.H"

#include "vspScreen.h"
#include "parm.h"
#include "util.h"

#define SAR_NO_STOP             0
#define SAR_STOP                1
#define SAR_ABS_STOP            2

#define BUTTON_CHANGE_FRACT 0.1
//#define TOL_PERCENT 0.00001
#define TOL_PERCENT 0.000001

class Parm;
class VspScreen;

class Gui_device
{
protected:

     Parm* parm;
     VspScreen* screen;
	 Stringc script_text;
	 int value_type;

public:
     
     Gui_device(VspScreen* scrn_in);
     virtual ~Gui_device();

public:
     virtual void set_parm_ptr(Parm* parm_in) = 0;
     virtual Parm* get_parm_ptr()		{ return(parm); }
     virtual void remove_parm_ptr()		{ parm = 0; }
	 virtual void trigger_screen(Parm* parm_ptr);

     virtual void init() = 0;
     virtual void update() = 0;
     virtual void update_from_device() = 0;
     virtual void user_trigger() = 0;

     virtual void activate() = 0;
     virtual void deactivate() = 0;

	 virtual void deviceCB( Fl_Widget* w ) = 0;
	 static void staticDeviceCB( Fl_Widget *w, void* data ) { ((Gui_device*)data)->deviceCB( w ); }

	 virtual void setScriptText(Stringc str, int type)	{ script_text = str; value_type = type; }
};

class Slider : public Gui_device
{
public:

	Slider( VspScreen* scrn_in, Fl_Slider* sWidget );
	virtual ~Slider();

protected:

	Fl_Slider* sliderWidget;

public:

    virtual void init();
    virtual void update();
    virtual void update_from_device()			{ update(); }
    virtual void user_trigger();
    virtual void activate();
    virtual void deactivate();

    virtual void set_parm_ptr(Parm* parm_in);
	virtual void deviceCB( Fl_Widget* w );

};

class LogSlider : public Slider
{
public:
	LogSlider( VspScreen* scrn_in, Fl_Slider* sWidget );
	virtual ~LogSlider();

    virtual void init();
    virtual void update();
    virtual void update_from_device()			{ update(); }
    virtual void user_trigger();
};

class Slider_adj_range : public Slider
{
public:

     Slider_adj_range(VspScreen* scrn_in, Fl_Slider* sld_in, Fl_Button* left_but,
                       Fl_Button* right_but, double offset_init);
     virtual ~Slider_adj_range();

protected:

	 Fl_Button* minButton;
	 Fl_Button* maxButton;

     double offset;		
						
     double min_stop;
     double max_stop;
     double abs_min_stop;
     double abs_max_stop;
     int min_stop_flag;
     int max_stop_flag;

public:
     virtual void init();
     virtual void update();
     virtual void update_from_device();
     virtual void user_trigger();
     virtual void min_but_trigger();
     virtual void max_but_trigger();

     virtual void activate();
     virtual void deactivate();

	 virtual void deviceCB( Fl_Widget* w );

};

class Input : public Gui_device
{
public:
     
     Input(VspScreen* scrn_in, Fl_Input* obj_in);
     virtual ~Input();

protected:

     Stringc format;
     Fl_Input* inputWidget;
	 char inpStr[256];

public:
     virtual Fl_Input* getWidgetPtr()			{  return(inputWidget); }
     virtual void init();
     virtual void update();
     virtual void update_from_device()			{ update(); }
     virtual void user_trigger();

     virtual void set_format(const char* format_in)	{ format = format_in; }

     virtual void set_parm_ptr(Parm* parm_in);
 
     virtual void activate();
     virtual void deactivate();
	 virtual void deviceCB( Fl_Widget* w );


};

//==== Slider Input Combo =====//
class SliderInputCombo
{
public:
	SliderInputCombo( Fl_Slider* slider, Fl_Input* input );
	virtual ~SliderInputCombo();

	virtual void SetCallback( Fl_Callback* c, void* p );

	virtual void SetVal( double v );
	virtual void SetRange( double v );
	virtual void SetLimits( double min, double max );

	virtual void Init( double min, double max, double val, double range );

	virtual bool GuiChanged( Fl_Widget* w );

	virtual void UpdateGui();
	virtual void UpdateGui( Fl_Widget* w );

	virtual void Deactivate();
	virtual void Activate();

	virtual double GetVal()						{ return m_Val; }

protected:

	Fl_Slider* m_Slider;
	Fl_Input*  m_Input;

	double m_Val;
	double m_Min;
	double m_Max;
	double m_StartRange;
	double m_Range;
};



class ParmButton : public Gui_device
{
public:

	ParmButton( VspScreen* scrn_in, Fl_Button* sWidget );
	virtual ~ParmButton();

protected:

	Fl_Button* buttonWidget;
	unsigned int m_DefColor;
	bool m_RegFlag;

	int m_LinkedColor;
	int m_UnLinkedColor;

public:

    virtual void init();
    virtual void update();
    virtual void update_from_device()			{ update(); }
    virtual void user_trigger();
    virtual void activate();
    virtual void deactivate();

    virtual void set_parm_ptr(Parm* parm_in);
	virtual void deviceCB( Fl_Widget* w );

};









#endif  


     



