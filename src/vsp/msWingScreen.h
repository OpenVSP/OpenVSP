//******************************************************************************
//    
//   Screen Base Class
//   
//   J.R. Gloudemans - 4/21/03
//******************************************************************************

#ifndef MS_WINGSCREEN_H
#define MS_WINGSCREEN_H

#include <stdio.h>
#include <FL/Fl.H> 
#include "geom.h"
#include "ms_wing.h"
#include "gui_device.h"
#include "vspScreen.h"
#include "vspGlWindow.h"
#include "msWingFlScreen.h"
#include <FL/Fl_File_Chooser.H>

class Input;
class Slider;
class Slider_adj_range;
class ScreenMgr;
 
class MsWingScreen : public CompScreen
{
public:
	MsWingScreen(ScreenMgr* mgrPtr);
	virtual ~MsWingScreen();

	void show(Geom* geomPtr);
	void hide();
	void screenCB( Fl_Widget* w );
	void closeCB( Fl_Widget* w );
	void position( int x, int y );
	void setTitle( const char* name );

	void parm_changed( Parm* parm );
	void updateName();


protected:

	MsWingUI* msWingUI;
	XSecGlWindow* glWin;

	Slider_adj_range* totalSpanSlider;
	Input* totalSpanInput;
	Slider_adj_range* totalProjSpanSlider;
	Input* totalProjSpanInput;
	Slider_adj_range* totalChordSlider;
	Input* totalChordInput;
	Slider_adj_range* totalAreaSlider;
	Input* totalAreaInput;

	Input* totalARInput;
	Slider_adj_range* sweepOffsetSlider;
	Input* sweepOffsetInput;

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

	Input* sectProjSpanOutput;

	Slider* sweepSlider;
	Input* sweepInput;
	Slider* sweepLocSlider;
	Input* sweepLocInput;
	Slider* twistSlider;
	Input* twistInput;
	Slider* twistLocSlider;
	Input* twistLocInput;

	Slider* dihed1Slider;
	Input* dihed1Input;
	Slider* dihed2Slider;
	Input* dihed2Input;
	Slider* dihedCrv1Slider;
	Input* dihedCrv1Input;
	Slider* dihedCrv2Slider;
	Input* dihedCrv2Input;
	Slider* dihedCrv1StrSlider;
	Input* dihedCrv1StrInput;
	Slider* dihedCrv2StrSlider;
	Input* dihedCrv2StrInput;

	Input* degPerSegInput;
	Input* maxNumSegsInput;

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
	Slider* afSlatChordSlider;
	Input* afSlatChordInput;
	Slider* afSlatAngleSlider;
	Input* afSlatAngleInput;
	Slider* afFlapChordSlider;
	Input* afFlapChordInput;
	Slider* afFlapAngleSlider;
	Input* afFlapAngleInput;

	Input* leRadiusInput;

	ParmButton* totalSpanButton;
	ParmButton* totalProjSpanButton;
	ParmButton* totalChordButton;
	ParmButton* totalAreaButton;
	ParmButton* sweepOffsetButton;

	ParmButton* ARButton;
	ParmButton* TRButton;
	ParmButton* areaButton;
	ParmButton* spanButton;
	ParmButton* TCButton;
	ParmButton* RCButton;

	ParmButton* sweepButton;
	ParmButton* sweepLocButton;
	ParmButton* twistButton;
	ParmButton* twistLocButton;

	ParmButton* dihed1Button;
	ParmButton* dihed2Button;
	ParmButton* dihedCrv1Button;
	ParmButton* dihedCrv2Button;
	ParmButton* dihedCrv1StrButton;
	ParmButton* dihedCrv2StrButton;

	ParmButton* afCamberButton;
	ParmButton* afCamberLocButton;
	ParmButton* afThickButton;
	ParmButton* afThickLocButton;
	ParmButton* afIdealClButton;
	ParmButton* afAButton;

	ParmButton* afSlatChordButton;
	ParmButton* afSlatAngleButton;
	ParmButton* afFlapChordButton;
	ParmButton* afFlapAngleButton;

	Ms_wing_geom* currGeom;

};



#endif  


     



