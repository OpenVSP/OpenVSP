#ifndef HERMITESCREEN_H
#define HERMITESCREEN_H

#include <stdio.h>
#include <FL/Fl.H> 
#include "geom.h"
#include "gui_device.h"
#include "cabinLayoutFLScreen.h"

class Input;
class Slider;
class Slider_adj_range;
class ScreenMgr;
class CabinLayoutGeom;

class CabinLayoutScreen : public CompScreen
{
public:
	CabinLayoutScreen(ScreenMgr* mgrPtr);
	virtual ~CabinLayoutScreen();

	void show(Geom* geomPtr);
	void hide();
	void screenCB( Fl_Widget* w );
	void closeCB( Fl_Widget* w );
	void position( int x, int y );
	void setTitle( const char* name );
	void parm_changed( Parm* parm )	{}

protected:

	CabinLayoutUI* cabinLayoutUI;

//	Fl_Button *loadHermiteFileButton;
//	Fl_Output *hermiteFilePathOutput;

	CabinLayoutGeom* currGeom;

};


#endif
