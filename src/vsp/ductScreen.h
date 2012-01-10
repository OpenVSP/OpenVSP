//******************************************************************************
//    
//   Duct Screen Class
//   
//   J.R. Gloudemans - 4/21/03
//******************************************************************************

#ifndef DUCTSCREEN_H
#define DUCTSCREEN_H

#include <stdio.h>
#include <FL/Fl.H> 
#include "geom.h"
#include "gui_device.h"
#include "vspScreen.h"
#include "vspGlWindow.h"
#include "ductFlScreen.h"
#include <FL/Fl_File_Chooser.H>

class Input;
class Slider;
class Slider_adj_range;
class ScreenMgr;
class DuctGeom; 


class DuctScreen : public CompScreen
{
public:
	DuctScreen(ScreenMgr* mgrPtr);
	virtual ~DuctScreen();

	void show(Geom* geomPtr);
	void hide();
	void screenCB( Fl_Widget* w );
	void closeCB( Fl_Widget* w );
	void position( int x, int y );
	void setTitle( const char* name );
	void parm_changed( Parm* parm );
	void updateName();


protected:

	DuctUI* ductUI;

	Slider_adj_range* lengthSlider;
	Input* lengthInput;
	Slider_adj_range* inletOutletSlider;
	Input* inletOutletInput;
	Slider_adj_range* inletDiaSlider;
	Input* inletDiaInput;

	Input* inletAreaInput;
	Input* outletDiaInput;
	Input* outletAreaInput;
	Input* chordInput;

	//==== Airfoil Stuff ====//
	XSecGlWindow* glWin;

	Slider* afCamberSlider;
	Input* afCamberInput;
	Slider* afCamberLocSlider;
	Input* afCamberLocInput;
	Slider* afThickSlider;
	Input* afThickInput;
	Slider* afThickLocSlider;
	Input* afThickLocInput;
	Slider* afIdealClSlider;
	Input* afIdealClInput;
	Slider* afASlider;
	Input* afAInput;

	Input* leRadiusInput;

	ParmButton* lengthButton;
	ParmButton* inletOutletButton;
	ParmButton* inletDiaButton;

	ParmButton* afCamberButton;
	ParmButton* afCamberLocButton;
	ParmButton* afThickButton;
	ParmButton* afThickLocButton;
	ParmButton* afIdealClButton;
	ParmButton* afAButton;

	DuctGeom* currGeom;

};






#endif  


     



