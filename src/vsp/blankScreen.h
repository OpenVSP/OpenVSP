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

#ifndef BLANKSCREEN_H
#define BLANKSCREEN_H

#include <stdio.h>
#include <FL/Fl.H> 
#include "geom.h"
#include "gui_device.h"
#include "vspScreen.h"
#include "vspGlWindow.h"
#include "blankFlScreen.h"
#include <FL/Fl_File_Chooser.H>
#include "loadjpg.h"

class Input;
class Slider;
class Slider_adj_range;
class ScreenMgr;
class BlankGeom; 

class BlankScreen : public VspScreen
{
public:
	BlankScreen(ScreenMgr* mgrPtr);
	virtual ~BlankScreen();

	void show(Geom* geomPtr);
	void hide();
	void screenCB( Fl_Widget* w );
	void closeCB( Fl_Widget* w );
	void position( int x, int y );
	void setTitle( const char* name );

	void parm_changed( Parm* parm );
	void updateName();

protected:

	BlankUI* blankUI;

	//==== General Stuff ====//
	Slider_adj_range* xLocSlider;
	Slider_adj_range* yLocSlider;
	Slider_adj_range* zLocSlider;
	Input* xLocInput;
	Input* yLocInput;
	Input* zLocInput;

	Slider_adj_range* xRotSlider;
	Slider_adj_range* yRotSlider;
	Slider_adj_range* zRotSlider;
	Input* xRotInput;
	Input* yRotInput;
	Input* zRotInput;

	Slider* uSlider;
	Slider* vSlider;
	Input* uInput;
	Input* vInput;

	Input* massInput;

	ParmButton* xLocButton;
	ParmButton* yLocButton;
	ParmButton* zLocButton;

	ParmButton* xRotButton;
	ParmButton* yRotButton;
	ParmButton* zRotButton;

	BlankGeom* currGeom;
};



#endif  


     



