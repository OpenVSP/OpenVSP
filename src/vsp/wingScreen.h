//******************************************************************************
//    
//   Screen Base Class
//   
//   J.R. Gloudemans - 4/21/03
//******************************************************************************

#ifndef WINGSCREEN_H
#define WINGSCREEN_H

#include <stdio.h>
#include <FL/Fl.H> 
#include "geom.h"
#include "wingGeom.h"
#include "gui_device.h"
#include "vspScreen.h"
#include "vspGlWindow.h"
#include "wingFlScreen.h"
#include <FL/Fl_File_Chooser.H>


class Input;
class Slider;
class Slider_adj_range;
class ScreenMgr;
 

class WingScreen : public CompScreen
{
public:
	WingScreen(ScreenMgr* mgrPtr);
	virtual ~WingScreen();

	void show(Geom* geomPtr);
	void hide();
	void screenCB( Fl_Widget* w );
	void closeCB( Fl_Widget* w );
	void position( int x, int y );
	void setTitle( const char* name );

	void parm_changed( Parm* parm );
	void updateName();


protected:

	WingUI* wingUI;
	XSecGlWindow* glWin;

	Slider* ARSlider;
	Input* ARInput;
	Slider* TRSlider;
	Input* TRInput;
	Slider* areaSlider;
	Input* areaInput;
	Slider* spanSlider;
	Input* spanInput;
	Slider* TCSlider;
	Input* TCInput;
	Slider* RCSlider;
	Input* RCInput;

	Slider* sweepSlider;
	Input* sweepInput;
	Slider* sweepLocSlider;
	Input* sweepLocInput;

	Input* totalWingAreaInput;

	Slider* twistLocSlider;
	Input* twistLocInput;
	Slider* inTwistSlider;
	Input* inTwistInput;
	Slider* inDihedSlider;
	Input* inDihedInput;
	Slider* midTwistSlider;
	Input* midTwistInput;
	Slider* midDihedSlider;
	Input* midDihedInput;
	Slider* outTwistSlider;
	Input* outTwistInput;
	Slider* outDihedSlider;
	Input* outDihedInput;

	
	Slider* strakeSweepSlider;
	Input* strakeSweepInput;
	Slider* strakeSpanSlider;
	Input* strakeSpanInput;

	Slider* aftSweepSlider;
	Input* aftSweepInput;
	Slider* aftSpanSlider;
	Input* aftSpanInput;

	Slider* deflScaleSlider;
	Input* deflScaleInput;
	Slider* twistScaleSlider;
	Input* twistScaleInput;

	//==== Airfoil Stuff ====//
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

	WingGeom* currGeom;

};



#endif  


     



