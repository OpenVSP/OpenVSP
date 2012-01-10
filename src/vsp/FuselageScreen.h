//******************************************************************************
//    
//   Fuselage Screen
//   
//   J.R. Gloudemans - 9/22/09
//******************************************************************************

#ifndef FUSELAGESCREEN_H
#define FUSELAGESCREEN_H

#include <stdio.h>
#include <FL/Fl.H> 
#include "geom.h"
#include "gui_device.h"
#include "vspScreen.h"
#include "vspGlWindow.h"
#include "fuselageFlScreen.h"
#include <FL/Fl_File_Chooser.H>
#include "loadjpg.h"


class Input;
class Slider;
class Slider_adj_range;
class ScreenMgr;
class FuselageGeom; 
class FuselageXSec;

class FuselageScreen : public CompScreen
{
public:
	FuselageScreen(ScreenMgr* mgrPtr);
	virtual ~FuselageScreen();

	void show(Geom* geomPtr);
	void hide();
	void screenCB( Fl_Widget* w );
	void closeCB( Fl_Widget* w );
	void position( int x, int y );
	void setTitle( const char* name );

	void parm_changed( Parm* parm );
	void updateName();

	void updateAbsXSecLocVal();

	void setActiveShapeGUI();

	void s_set_id(int src, int id);
	void s_xsec_add(int src);
	void s_xsec_cut(int src);
	void s_xsec_copy(int src);
	void s_xsec_paste(int src);
	void s_type(int src, int val);

	void s_xsec_num_interp1(int src, int val);
	void s_xsec_num_interp2(int src, int val);
	void s_xsec_dec_num_all(int src);
	void s_xsec_inc_num_all(int src);
	void s_xsec_nose_rho_on(int src, int val);
	void s_xsec_aft_rho_on(int src, int val);

protected:

	FuselageUI* fuseUI;
	XSecGlWindow* glWin;

	JpgData jpgImgData;

	Slider* xsHeightSlider;
	Input* xsHeightInput;
	Slider* xsWidthSlider;
	Input* xsWidthInput;
	Slider* xsYOffsetSlider;
	Input* xsYOffsetInput;
	Slider* xsZOffsetSlider;
	Input* xsZOffsetInput;
	Slider* xsWidthOffsetSlider;
	Input* xsWidthOffsetInput;
	Slider* xsCornerRadSlider;
	Input* xsCornerRadInput;
	Slider* xsTopTanAngSlider;
	Input* xsTopTanAngInput;
	Slider* xsBotTanAngSlider;
	Input* xsBotTanAngInput;

	Slider* xsTanStrSlider;
	Input* xsTanStrInput;

	Slider* xsLocSlider;
	Input* xsLocInput;

	//==== Shape Tangent Ang/Str Stuff ====//
	Slider* topAngSlider;
	Input* topAngInput;
	Slider* topStr1Slider;
	Input* topStr1Input;
	Slider* topStr2Slider;
	Input* topStr2Input;

	Slider* botAngSlider;
	Input* botAngInput;
	Slider* botStr1Slider;
	Input* botStr1Input;
	Slider* botStr2Slider;
	Input* botStr2Input;

	Slider* leftAngSlider;
	Input* leftAngInput;
	Slider* leftStr1Slider;
	Input* leftStr1Input;
	Slider* leftStr2Slider;
	Input* leftStr2Input;

	Slider* rightAngSlider;
	Input* rightAngInput;
	Slider* rightStr1Slider;
	Input* rightStr1Input;
	Slider* rightStr2Slider;
	Input* rightStr2Input;

	//==== Fuse Shape Parms ====//
	Slider* lengthSlider;
	Input*  lengthInput;

	ParmButton* xsHeightButton;
	ParmButton* xsWidthButton;
	ParmButton* xsYOffsetButton;
	ParmButton* xsZOffsetButton;
	ParmButton* xsWidthOffsetButton;
	ParmButton* xsCornerRadButton;
	ParmButton* xsTopTanAngButton;
	ParmButton* xsBotTanAngButton;
	ParmButton* xsLocButton;

	ParmButton* topAngButton;
	ParmButton* topStr1Button;
	ParmButton* topStr2Button;

	ParmButton* botAngButton;
	ParmButton* botStr1Button;
	ParmButton* botStr2Button;

	ParmButton* leftAngButton;
	ParmButton* leftStr1Button;
	ParmButton* leftStr2Button;

	ParmButton* rightAngButton;
	ParmButton* rightStr1Button;
	ParmButton* rightStr2Button;

	ParmButton* lengthButton;

	FuselageGeom* currGeom;
	FuselageXSec* curr_fxsec;
};



#endif  


     



