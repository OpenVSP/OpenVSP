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

#ifndef EXTSCREEN_H
#define EXTSCREEN_H

#include <stdio.h>
#include <FL/Fl.H> 
#include "geom.h"
#include "gui_device.h"
#include "vspScreen.h"
#include "vspGlWindow.h"
#include "extStoreFlScreen.h"
#include <FL/Fl_File_Chooser.H>
#include "loadjpg.h"

class Input;
class Slider;
class Slider_adj_range;
class ScreenMgr;
class Ext_geom; 

class ExtScreen : public CompScreen
{
public:
	ExtScreen(ScreenMgr* mgrPtr);
	virtual ~ExtScreen();

	void show(Geom* geomPtr);
	void hide();
	void screenCB( Fl_Widget* w );
	void closeCB( Fl_Widget* w );
	void position( int x, int y );
	void setTitle( const char* name );

	void parm_changed( Parm* parm );
	void updateName();

	void s_type(int src, int val);
	void s_pylon(int src, int on);

protected:

	ExtUI* extUI;

	Slider* lengthSlider;
	Input* lengthInput;
	Slider* finessSlider;
	Input* finessInput;
	Slider* cdSlider;
	Input* cdInput;
	Slider* pylonHeightSlider;
	Input* pylonHeightInput;
	Slider* pylonCdSlider;
	Input* pylonCdInput;

	ParmButton* lengthButton;
	ParmButton* finessButton;
	ParmButton* pylonHeightButton;

	Ext_geom* currGeom;
};



#endif  


     



