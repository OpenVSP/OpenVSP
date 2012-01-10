//******************************************************************************
//    
//   Cross Section Screen Class
//   
//   J.R. Gloudemans - 4/21/03
//******************************************************************************

#ifndef HRMSCREEN_H
#define HRMSCREEN_H

#include <stdio.h>
#include <FL/Fl.H> 
#include "geom.h"
#include "gui_device.h"
#include "vspScreen.h"
#include "vspGlWindow.h"
#include "hrmFlScreen.h"
#include <FL/Fl_File_Chooser.H>

class Input;
class Slider;
class Slider_adj_range;
class ScreenMgr;
class VorGeom; 


class HrmScreen : public CompScreen
{
public:
	HrmScreen(ScreenMgr* mgrPtr);
	virtual ~HrmScreen();

	void show(Geom* geomPtr);
	void hide();
	void screenCB( Fl_Widget* w );
	void closeCB( Fl_Widget* w );
	void position( int x, int y );
	void setTitle( const char* name );
	void parm_changed( Parm* parm )				{}

protected:

	HrmUI* hrmUI;
	VorGeom* currGeom;

};






#endif  


     



