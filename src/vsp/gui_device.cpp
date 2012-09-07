//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

//******************************************************************************
//    
//   Input/Display Gui_device Base Class
//   
//   J.R. Gloudemans - 8/26/93
//******************************************************************************

#include "gui_device.h"
#include "scriptMgr.h"
#include "parmLinkMgr.h"

//==== Constructor =====//
Gui_device::Gui_device(VspScreen* scrn_in)
{
  screen = scrn_in;
  parm = 0;
  script_text = "";
  value_type = Parm::VALUE_DOUBLE;
}

//==== Destructor =====//
Gui_device::~Gui_device()
{
}

void Gui_device::trigger_screen(Parm* parm_ptr)
{
  screen->parm_changed(parm_ptr);
}

//******************************************************************************
//			Slider 
//******************************************************************************

//==== Constructor =====//
Slider::Slider( VspScreen* scrn_in, Fl_Slider* sWidget ) : Gui_device( scrn_in )
{
	sliderWidget = sWidget;
	sliderWidget->callback( staticDeviceCB, this );
	sliderWidget->when(FL_WHEN_CHANGED | FL_WHEN_RELEASE);
}

//==== Destructor =====//
Slider::~Slider()
{
}

//==== Set Parm Ptr =====//
void Slider::set_parm_ptr(Parm* parm_in)
{
  if (parm)
    parm->del_device((Gui_device*)this);

  parm = parm_in;

  if (parm)
    parm->add_device((Gui_device*)this);

  // for script output
  if (parm && (parm->get_script().get_length() == 0))
	  parm->set_script(script_text, value_type);

  init();
}

//==== Initialize Slider =====//
void Slider::init()
{
  if ( !parm ) return;

  double new_val = parm->get();

  sliderWidget->bounds( parm->get_lower(), parm->get_upper() );
  sliderWidget->value( new_val );

  if ( parm->get_active_flag() )
    activate();
  else
    deactivate();
}


//==== Update Slider - Triggered By Linked Parm =====//
void Slider::update()
{
  if ( !parm ) return;

  double new_val = parm->get();
//printf("Slider::update %d\n", this);
  sliderWidget->value( new_val );
}

//==== Update Slider - Triggered By User =====//
void Slider::user_trigger()
{
  if ( !parm ) return;

  double new_val = sliderWidget->value();
//printf("Slider::user_trigger %d\n", this);

  parm->set_from_device((Gui_device*)this, new_val, !Fl::event_state(FL_BUTTONS));
}

//==== Activate Slider  =====//
void Slider::activate()
{
	sliderWidget->activate();

}

//==== Activate Slider  =====//
void Slider::deactivate()
{
	sliderWidget->deactivate();
}

//==== Slider - Callback =====//
void Slider::deviceCB( Fl_Widget* w )
{
	if ( w == sliderWidget )
		user_trigger();
}


//******************************************************************************
//			Log Slider 
//******************************************************************************

//==== Constructor =====//
LogSlider::LogSlider( VspScreen* scrn_in, Fl_Slider* sWidget ) : Slider( scrn_in, sWidget )
{
}

//==== Destructor =====//
LogSlider::~LogSlider()
{
}

//==== Initialize Slider =====//
void LogSlider::init()
{
  if ( !parm ) return;

  double new_val = parm->get();

  double lb = log10(parm->get_lower());
  double ub = log10(parm->get_upper());

  sliderWidget->bounds( lb, ub );
  sliderWidget->value( log10(new_val) );

  if ( parm->get_active_flag() )
    activate();
  else
    deactivate();
}

//==== Update Slider - Triggered By Linked Parm =====//
void LogSlider::update()
{
  if ( !parm ) return;

  double new_val = parm->get();
  sliderWidget->value( log10(new_val) );
}

//==== Update Slider - Triggered By User =====//
void LogSlider::user_trigger()
{

  if ( !parm ) return;

  double new_val = pow( 10, sliderWidget->value() );
  parm->set_from_device((Gui_device*)this, new_val, !Fl::event_state(FL_BUTTONS));
}


//******************************************************************************
//			Slider Adjustable Range   
//******************************************************************************

//==== Constructor =====//
Slider_adj_range::Slider_adj_range(VspScreen* scrn_in, Fl_Slider* sld_in, 
				Fl_Button* left_but, Fl_Button* right_but, double offset_init):
				Slider( scrn_in, sld_in )
{
	minButton = left_but;
	minButton->callback( staticDeviceCB, this );

	maxButton = right_but;
	maxButton->callback( staticDeviceCB, this );

	offset = offset_init;
	min_stop_flag = SAR_NO_STOP;
	max_stop_flag = SAR_NO_STOP;
}

//==== Destructor =====//
Slider_adj_range::~Slider_adj_range()
{

}

//==== Initialize Slider_adj_range - Called from set_parm_ptr =====//
void Slider_adj_range::init()
{
  if ( !parm ) return;

  double new_val = parm->get();
  double tol = offset*TOL_PERCENT;

  abs_min_stop = parm->get_lower() + tol;
  abs_max_stop = parm->get_upper() - tol;

  min_stop = new_val - offset;
  max_stop = new_val + offset;

  if ( new_val <= abs_min_stop )
    {
      min_stop_flag = SAR_ABS_STOP;
	  minButton->label("|");
      min_stop = abs_min_stop;
    }
  else
    {
      min_stop_flag = SAR_NO_STOP;
	  minButton->label(">");
     }

  if ( new_val > abs_max_stop )
    {
      max_stop_flag = SAR_ABS_STOP;
	  maxButton->label("|");
      max_stop = abs_max_stop;
    }
  else
    {
      max_stop_flag = SAR_NO_STOP;
 	  maxButton->label("<");
    }

  min_stop = MAX(abs_min_stop, min_stop);
  max_stop = MIN(abs_max_stop, max_stop);

  sliderWidget->bounds( min_stop-tol, max_stop+tol );
  sliderWidget->value( new_val );

 
  if ( parm->get_active_flag() )
    activate();
  else
    deactivate();
}

//==== Update Slider_adj_range - Triggered By Linked Parm =====//
void Slider_adj_range::update()
{
  if ( !parm ) return;

  double new_val = parm->get();

  if ( new_val < min_stop || new_val > max_stop )
    {
      init();
    }
  else
    {
 	  minButton->label(">");
 	  maxButton->label("<");
      min_stop_flag = SAR_NO_STOP;
      max_stop_flag = SAR_NO_STOP;
	  sliderWidget->value( new_val );
   }
}

//==== Update Slider_adj_range - Triggered By Mult Devices Hooked To One Parm =====//
void Slider_adj_range::update_from_device()
{
  if ( !parm ) return;

  double new_val = parm->get();

  if ( new_val < min_stop || new_val > max_stop )
    {
      init();
    }
  else
    {
 	  minButton->label(">");
 	  maxButton->label("<");
      min_stop_flag = SAR_NO_STOP;
      max_stop_flag = SAR_NO_STOP;
	  sliderWidget->value( new_val );
    }
}

//==== Update Slider_adj_range - Triggered By User =====//
void Slider_adj_range::user_trigger()
{
  if ( !parm ) return;

// printf("Slider_adj_range::user_trigger %d\n", this);

  double new_val = sliderWidget->value();
  parm->set_from_device((Gui_device*)this, new_val, !Fl::event_state(FL_BUTTONS));

  if ( new_val <= abs_min_stop )
    {
      min_stop_flag = SAR_ABS_STOP;
	  minButton->label("|");
      min_stop = abs_min_stop;
    }
  else if ( new_val <= min_stop )
    {
	  minButton->label("<");
      min_stop_flag = SAR_STOP;
    }
  else if ( new_val >= abs_max_stop )
   {
      max_stop_flag = SAR_ABS_STOP;
	  maxButton->label("|");
      max_stop = abs_max_stop;
    }
  else if ( new_val >= max_stop )
    {
	  maxButton->label(">");
      max_stop_flag = SAR_STOP;
    }
  else
    {
	  minButton->label(">");
	  maxButton->label("<");
      min_stop_flag = SAR_NO_STOP;
      max_stop_flag = SAR_NO_STOP;
    }

}

//==== Min Button - Triggered By User =====//
void Slider_adj_range::min_but_trigger()
{
  if ( min_stop_flag == SAR_ABS_STOP )
    return;

  double new_val = parm->get();
  if ( min_stop_flag == SAR_STOP )
    {
      min_stop =  new_val - (offset*BUTTON_CHANGE_FRACT);
      new_val  = min_stop;
      if ( min_stop < abs_min_stop )
        {
          min_stop_flag = SAR_ABS_STOP;
		  minButton->label("|");
          min_stop = abs_min_stop;
        }
      parm->set_from_device((Gui_device*)this, min_stop, 1);
    }
  else
    {
      min_stop += (new_val - min_stop)*BUTTON_CHANGE_FRACT;
      max_stop -= (max_stop - new_val)*BUTTON_CHANGE_FRACT;
    }
  offset = max_stop - min_stop;

  double tol = offset*TOL_PERCENT;

  sliderWidget->bounds( min_stop-tol, max_stop+tol );
  sliderWidget->value( new_val );
}

//==== Max Button - Triggered By User =====//
void Slider_adj_range::max_but_trigger()
{
  if ( max_stop_flag == SAR_ABS_STOP )
    return;

  double new_val = parm->get();

  if ( max_stop_flag == SAR_STOP )
    {
      max_stop =  new_val + (offset*BUTTON_CHANGE_FRACT);
      new_val  = max_stop;
      if ( max_stop > abs_max_stop )
        {
          max_stop_flag = SAR_ABS_STOP;
		  maxButton->label("|");
          max_stop = abs_max_stop;
        }
      parm->set_from_device((Gui_device*)this, max_stop, 1);
    }
  else
    {
      min_stop += (new_val - min_stop)*BUTTON_CHANGE_FRACT;
      max_stop -= (max_stop - new_val)*BUTTON_CHANGE_FRACT;
    }
  offset = max_stop - min_stop;

  double tol = offset*TOL_PERCENT;

  sliderWidget->bounds( min_stop-tol, max_stop+tol );
  sliderWidget->value( new_val );
}

//==== Activate Slider  =====//
void Slider_adj_range::activate()
{
	sliderWidget->activate();
	minButton->activate();
	maxButton->activate();

	sliderWidget->color(fl_rgb_color(255, 255, 255));

}

//==== Activate Slider  =====//
void Slider_adj_range::deactivate()
{
	sliderWidget->deactivate();
	minButton->deactivate();
	maxButton->deactivate();

	sliderWidget->color(fl_rgb_color(235, 235, 235));

}

//==== Slider - Callback =====//
void Slider_adj_range::deviceCB( Fl_Widget* w )
{
	if ( w == sliderWidget )
		user_trigger();
	else if ( w == minButton )
		min_but_trigger();
	else if ( w == maxButton )
		max_but_trigger();

}

//******************************************************************************
//			Input 
//******************************************************************************
//==== Constructor =====//
Input::Input( VspScreen* scrn_in, Fl_Input* iWidget ) : Gui_device( scrn_in )
{
	inputWidget = iWidget;
	inputWidget->callback( staticDeviceCB, this );
	inputWidget->when( FL_WHEN_RELEASE | FL_WHEN_ENTER_KEY );

	format = Stringc("%1.4f");

}

//==== Destructor =====//
Input::~Input()
{
}

//==== Set Parm Ptr =====//
void Input::set_parm_ptr(Parm* parm_in)
{
  if (parm)
    parm->del_device((Gui_device*)this);

  parm = parm_in;

  if (parm)
    parm->add_device((Gui_device*)this);

  // for script output
  if (parm && (parm->get_script().get_length() == 0))
	  parm->set_script(script_text, value_type);

  init();
}


//==== Initialize Input =====//
void Input::init()
{
  if ( !parm ) return;

  double new_val = parm->get();


  sprintf(inpStr, format, new_val);

 // inputWidget->value(inpStr );
  inputWidget->static_value(inpStr );
  inputWidget->redraw();

  if ( parm->get_active_flag() )
    activate();
  else
    deactivate();
}

//==== Update Input - Triggered By Linked Parm =====//
void Input::update()
{
  if ( !parm ) return;

  double new_val = parm->get();

  sprintf(inpStr, format, new_val);

  inputWidget->static_value(inpStr );
  inputWidget->redraw();

//  sprintf(inpStr, "%2.1f", 23.2);
//  inputWidget->value(str, strlen(str));

//if ( new_val > 2.1999 && new_val < 2.20001 )
//{ printf("Input %s \n", str ); }
//else

}


//==== Update Input - Triggered By User =====//
void Input::user_trigger()
{
  if ( !parm ) return;

  float val;
  const char* str = inputWidget->value();
  sscanf(str, "%f", &val);

  double new_val = parm->set_from_device((Gui_device*)this, (double)val, 1);

  init();

}

//==== Slider - Callback =====//
void Input::deviceCB( Fl_Widget* w )
{
	if ( w == inputWidget )
	{
		user_trigger();
	}
}

//==== Activate Input  =====//
void Input::activate()
{
	inputWidget->activate();
	inputWidget->color(fl_rgb_color(255, 255, 255));
}

//==== Deactivate Input  =====//
void Input::deactivate()
{
	inputWidget->deactivate();
	inputWidget->color(fl_rgb_color(235, 235, 235));
}
 
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
SliderInputCombo::SliderInputCombo( Fl_Slider* slider, Fl_Input* input )
{
	m_Slider = slider;
	m_Input  = input;

	m_Val = 0.5;
	m_Min = 0.0;
	m_Max = 1.0;
	m_StartRange = 0.5;
	m_Range = 0.5;
}

SliderInputCombo::~SliderInputCombo()
{
}

void SliderInputCombo::SetCallback( Fl_Callback* c, void* p )
{
	if ( m_Slider )			m_Slider->callback( c, p );
	if ( m_Input )			m_Input->callback( c, p );
}

void SliderInputCombo::SetVal( double v )
{
	m_Val = v;
	if ( m_Val < m_Min ) m_Val = m_Min;
	if ( m_Val > m_Max ) m_Val = m_Max;
}

void SliderInputCombo::SetRange( double v )
{
	m_StartRange = v;
	m_Range = m_StartRange;
	if ( m_Slider )
	{
		m_Slider->range( m_Val - m_Range, m_Val + m_Range );
		m_Slider->value( m_Val );
	}
}

void SliderInputCombo::SetLimits( double min, double max )
{
	m_Min = min;
	m_Max = max;
	if ( m_Val < min )	m_Val = m_Min;
	if ( m_Val > max )	m_Val = m_Max;
}

void SliderInputCombo::Init( double min, double max, double val, double range )
{
	SetLimits( min, max );
	SetVal( val );
	SetRange( range );
	UpdateGui();
}

void SliderInputCombo::UpdateGui()
{
	UpdateGui( m_Input );
	UpdateGui( m_Slider );
}

void SliderInputCombo::UpdateGui( Fl_Widget* w )
{
	char str[256];
	if ( m_Input && w == m_Input )
	{
		sprintf( str, " %7.5f ", m_Val );
		m_Input->value(str);
	}
	if ( m_Slider && w == m_Slider )
	{
		m_Slider->range( max( m_Val - m_Range, m_Min ), min( m_Val + m_Range, m_Max ) );
		m_Slider->value( m_Val );
	}
}

void SliderInputCombo::Deactivate()
{
	m_Input->deactivate();
	m_Slider->deactivate();
}

void SliderInputCombo::Activate()
{
	m_Input->activate();
	m_Slider->activate();
}

bool SliderInputCombo::GuiChanged( Fl_Widget* w )
{
	if ( w == m_Input )
	{
		double val = atof( m_Input->value() );

		//==== Figure Out Range ====//
		if ( val < m_Val - m_Range || val > m_Val + m_Range )
			m_Range = m_StartRange;
		else 
			m_Range = m_Range*0.5;

		SetVal( val );
		UpdateGui( m_Slider );
		if ( fabs( val - m_Val ) > DBL_EPSILON )
			UpdateGui( m_Input );
		return true;
	}
	else if ( w == m_Slider )
	{
		m_Val = m_Slider->value();
		UpdateGui( m_Input );
		return true;
	}
	return false;
}
//******************************************************************************
//			Parm Button
//******************************************************************************

//==== Constructor =====//
ParmButton::ParmButton( VspScreen* scrn_in, Fl_Button* sWidget ) : Gui_device( scrn_in )
{
	m_RegFlag = false;
	buttonWidget = sWidget;
	buttonWidget->callback( staticDeviceCB, this );
	buttonWidget->when(FL_WHEN_RELEASE);
	m_DefColor = buttonWidget->color();

	m_LinkedColor = 250;
	m_UnLinkedColor = 251;

	Fl::set_color( (Fl_Color)m_LinkedColor, 240, 226, 220 );		// Lt Blue
	Fl::set_color( (Fl_Color)m_UnLinkedColor, 236, 233, 240 );		// Lt Red

	
}

//==== Destructor =====//
ParmButton::~ParmButton()
{
}

//==== Set Parm Ptr =====//
void ParmButton::set_parm_ptr(Parm* parm_in)
{
	if (parm)
		parm->del_device((Gui_device*)this);

	parm = parm_in;

    if (parm)
        parm->add_device((Gui_device*)this);

	init();
}



//==== Initialize Button =====//
void ParmButton::init()
{
  if ( !parm ) return;

  if ( !m_RegFlag )
  {
	m_RegFlag = true;
	parmLinkMgrPtr->RegisterParmButton( this );
  }

  if ( parm->get_linked_flag() )
	buttonWidget->color( m_LinkedColor );
  else
	buttonWidget->color( m_UnLinkedColor );

  if ( parm->get_active_flag() )
    activate();
  else
    deactivate();

  buttonWidget->damage( FL_DAMAGE_ALL );
}

//==== Update Button - =====//
void ParmButton::update()
{
  if ( !parm ) return;

  init();

}

//==== Update Button  =====//
void ParmButton::user_trigger()
{
  if ( !parm ) return;

  int px = Fl::event_x_root() ;
  int py = Fl::event_y_root() ;
  screen->getScreenMgr()->showParmScreen(parm, px, py);

}

//==== Activate Slider  =====//
void ParmButton::activate()
{
//	buttonWidget->activate();
}

//==== Activate Slider  =====//
void ParmButton::deactivate()
{
//	buttonWidget->deactivate();
}

//==== Slider - Callback =====//
void ParmButton::deviceCB( Fl_Widget* w )
{
	if ( w == buttonWidget )
		user_trigger();
}
