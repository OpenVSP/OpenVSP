//******************************************************************************
//    
//   Prop Screen Class
//   
//   J.R. Gloudemans - 4/21/03
//******************************************************************************

#ifndef PROPSCREEN_H
#define PROPSCREEN_H

#include <stdio.h>
#include <FL/Fl.H> 
#include "geom.h"
#include "gui_device.h"
#include "vspScreen.h"
#include "vspGlWindow.h"
#include "propFlScreen.h"
#include <FL/Fl_File_Chooser.H>

class Input;
class Slider;
class Slider_adj_range;
class ScreenMgr;
class PropGeom; 


class PropScreen : public CompScreen
{
public:
	PropScreen(ScreenMgr* mgrPtr);
	virtual ~PropScreen();

	void show(Geom* geomPtr);
	void hide();
	void screenCB( Fl_Widget* w );
	void closeCB( Fl_Widget* w );
	void position( int x, int y );
	void setTitle( const char* name );
	void parm_changed( Parm* parm );
	void updateName();

protected:

	PropUI* propUI;

	Slider_adj_range* diameterSlider;
	Input* diameterInput;
	Slider_adj_range* coneAngleSlider;
	Input* coneAngleInput;
	Slider_adj_range* pitchSlider;
	Input* pitchInput;

	//==== Section GUI ====//
	Slider_adj_range* chordSlider;
	Input* chordInput;
	Slider_adj_range* twistSlider;
	Input* twistInput;
	Slider_adj_range* locSlider;
	Input* locInput;
	Slider_adj_range* offSlider;
	Input* offInput;


	//==== Airfoil GUI ====//
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

	ParmButton* diameterButton;
	ParmButton* coneAngleButton;
	ParmButton* pitchButton;

	ParmButton* chordButton;
	ParmButton* twistButton;
	ParmButton* locButton;
	ParmButton* offButton;

	ParmButton* afCamberButton;
	ParmButton* afCamberLocButton;
	ParmButton* afThickButton;
	ParmButton* afThickLocButton;
	ParmButton* afIdealClButton;
	ParmButton* afAButton;


	PropGeom* currGeom;

};






#endif  


     



