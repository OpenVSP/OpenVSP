//******************************************************************************
//    
//   Mesh Screen Class
//   
//   J.R. Gloudemans - 4/21/03
//******************************************************************************

#ifndef MESHSCREEN_H
#define MESHSCREEN_H

#include <stdio.h>
#include <FL/Fl.H> 
#include "geom.h"
#include "gui_device.h"
#include "vspScreen.h"
#include "vspGlWindow.h"
#include "meshFlScreen.h"
#include <FL/Fl_File_Chooser.H>

class Input;
class Slider;
class Slider_adj_range;
class ScreenMgr;
class MeshGeom; 


class MeshScreen : public CompScreen
{
public:
	MeshScreen(ScreenMgr* mgrPtr);
	virtual ~MeshScreen();

	void show(Geom* geomPtr);
	void hide();
	void screenCB( Fl_Widget* w );
	void closeCB( Fl_Widget* w );
	void position( int x, int y );
	void setTitle( const char* name );
	void parm_changed( Parm* parm )				{}

protected:

	MeshUI* meshUI;

	Slider_adj_range* lengthSlider;
	Input* lengthInput;

	MeshGeom* currGeom;

};






#endif  


     



