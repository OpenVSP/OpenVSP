//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

//******************************************************************************
//    
//   Screen Base Class
//   
//   J.R. Gloudemans - 4/21/03
//******************************************************************************

#ifndef FUSESCREEN_H
#define FUSESCREEN_H

#include <stdio.h>
#include <FL/Fl.H> 
#include "geom.h"
#include "gui_device.h"
#include "vspScreen.h"
#include "vspGlWindow.h"
#include "fuseFlScreen.h"
#include <FL/Fl_File_Chooser.H>
#include "loadjpg.h"


class Input;
class Slider;
class Slider_adj_range;
class ScreenMgr;
class Fuse_geom; 
class fuse_xsec;

class FuseScreen : public CompScreen
{
public:
	FuseScreen(ScreenMgr* mgrPtr);
	virtual ~FuseScreen();

	void show(Geom* geomPtr);
	void hide();
	void screenCB( Fl_Widget* w );
	void closeCB( Fl_Widget* w );
	void position( int x, int y );
	void setTitle( const char* name );

	void parm_changed( Parm* parm );
	void updateName();

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

	FuseUI* fuseUI;
	XSecGlWindow* glWin;

	JpgData jpgImgData;

	void setImlColor( int r, int g, int b );

	Slider* xsHeightSlider;
	Input* xsHeightInput;
	Slider* xsWidthSlider;
	Input* xsWidthInput;
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

	//==== Profile Stuff ====//
	Slider* profileTanAngSlider;
	Input* profileTanAngInput;

	Slider* profileTanStr1Slider;
	Input* profileTanStr1Input;

	Slider* profileTanStr2Slider;
	Input* profileTanStr2Input;

	//==== Fuse Shape Parms ====//
	Slider* lengthSlider;
	Input*  lengthInput;
	Slider* camberSlider;
	Input*  camberInput;
	Slider* camberLocSlider;
	Input*  camberLocInput;
	Slider* aftOffsetSlider;
	Input*  aftOffsetInput;
	Slider* noseAngleSlider;
	Input*  noseAngleInput;
	Slider* noseStrSlider;
	Input*  noseStrInput;
	Slider* noseRhoSlider;
	Input*  noseRhoInput;
	Slider* aftRhoSlider;
	Input*  aftRhoInput;

	//==== Fuse Shape Parms ====//
	Slider* topThickSlider;
	Input*  topThickInput;
	Slider* botThickSlider;
	Input*  botThickInput;
	Slider* sideThickSlider;
	Input*  sideThickInput;

	ParmButton* xsHeightButton;
	ParmButton* xsWidthButton;
	ParmButton* xsZOffsetButton;
	ParmButton* xsWidthOffsetButton;
	ParmButton* xsCornerRadButton;
	ParmButton* xsTopTanAngButton;
	ParmButton* xsBotTanAngButton;
	ParmButton* xsLocButton;

	ParmButton* lengthButton;
	ParmButton* camberButton;
	ParmButton* camberLocButton;
	ParmButton* aftOffsetButton;
	ParmButton* noseAngleButton;
	ParmButton* noseStrButton;
	ParmButton* noseRhoButton;
	ParmButton* aftRhoButton;

	Fuse_geom* currGeom;
	fuse_xsec* curr_fxsec;
};



#endif  


     



