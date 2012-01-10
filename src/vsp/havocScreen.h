//******************************************************************************
//    
//   HavocScreen 
//   
//   J.R. Gloudemans - 4/21/03
//******************************************************************************

#ifndef HAVOCSCREEN_H
#define HAVOCSCREEN_H

#include <stdio.h>
#include <FL/Fl.H> 
#include "geom.h"
#include "gui_device.h"
#include "vspScreen.h"
#include "vspGlWindow.h"
#include "havocFlScreen.h"
#include <FL/Fl_File_Chooser.H>
#include "loadjpg.h"


class Input;
class Slider;
class Slider_adj_range;
class ScreenMgr;
class Havoc_geom; 

class HavocScreen : public CompScreen
{
public:
	HavocScreen(ScreenMgr* mgrPtr);
	virtual ~HavocScreen();

	void show(Geom* geomPtr);
	void hide();
	void screenCB( Fl_Widget* w );
	void closeCB( Fl_Widget* w );
	void position( int x, int y );
	void setTitle( const char* name );

	void parm_changed( Parm* parm );
	void updateName();

protected:

	HavocUI* havocUI;

	Slider* lengthSlider;
	Input* lengthInput;
	Slider* alSlider;
	Input* alInput;
	Slider* arSlider;
	Input* arInput;
	Slider* apmSlider;
	Input* apmInput;
	Slider* apnSlider;
	Input* apnInput;
	Slider* lpiovlSlider;
	Input* lpiovlInput;
	Slider* peraSlider;
	Input* peraInput;

	Slider* mexp1Slider;
	Input* mexp1Input;
	Slider* mexp2Slider;
	Input* mexp2Input;
	Slider* mexp3Slider;
	Input* mexp3Input;
	Slider* mexp4Slider;
	Input* mexp4Input;
	Slider* nexp1Slider;
	Input* nexp1Input;
	Slider* nexp2Slider;
	Input* nexp2Input;
	Slider* nexp3Slider;
	Input* nexp3Input;
	Slider* nexp4Slider;
	Input* nexp4Input;

	Slider* plnSlider;
	Input*  plnInput;
	Slider* pleSlider;
	Input*  pleInput;
	Slider* buSlider;
	Input*  buInput;
	Slider* blSlider;
	Input*  blInput;
	Slider* umSlider;
	Input*  umInput;
	Slider* unSlider;
	Input*  unInput;
	Slider* lmSlider;
	Input*  lmInput;
	Slider* lnSlider;
	Input*  lnInput;
	Slider* gumSlider;
	Input*  gumInput;
	Slider* thetaSlider;
	Input*  thetaInput;
	Slider* ptasSlider;
	Input*  ptasInput;
	Slider* bueSlider;
	Input*  bueInput;
	Slider* bleSlider;
	Input*  bleInput;

	ParmButton* lengthButton;
	ParmButton* alButton;
	ParmButton* arButton;
	ParmButton* apmButton;
	ParmButton* apnButton;
	ParmButton* lpiovlButton;
	ParmButton* peraButton;

	ParmButton* mexp1Button;
	ParmButton* mexp2Button;
	ParmButton* mexp3Button;
	ParmButton* mexp4Button;
	ParmButton* nexp1Button;
	ParmButton* nexp2Button;
	ParmButton* nexp3Button;
	ParmButton* nexp4Button;

	ParmButton* plnButton;
	ParmButton* pleButton;
	ParmButton* buButton;
	ParmButton* blButton;
	ParmButton* umButton;
	ParmButton* unButton;
	ParmButton* lmButton;
	ParmButton* lnButton;
	ParmButton* gumButton;
	ParmButton* thetaButton;
	ParmButton* ptasButton;
	ParmButton* bueButton;
	ParmButton* bleButton;

	Havoc_geom* currGeom;
};



#endif  


     



