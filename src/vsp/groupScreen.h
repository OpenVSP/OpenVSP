// groupScreen.h: interface for the groupScreen class.
// J.R. Gloudemans
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_GROUPSCREEN_H__A640D440_8CFD_11D7_AC31_0003473A025A__INCLUDED_)
#define AFX_GROUPSCREEN_H__A640D440_8CFD_11D7_AC31_0003473A025A__INCLUDED_

#include <stdio.h>
#include <FL/Fl.H> 
#include "geom.h"
#include "gui_device.h"
#include "vspScreen.h"
#include "groupFlScreen.h"

class GroupScreen : public VspScreen  
{
public:
	GroupScreen(ScreenMgr* mgr, Aircraft* airPtr);
	virtual ~GroupScreen();

	void show(Geom* geomPtr)					{}		// Dont need
	void show( vector< Geom* > gVec );
	void hide();
	void screenCB( Fl_Widget* w );
	void closeCB( Fl_Widget* w );
	void position( int x, int y );
	void setTitle( const char* name );
	void parm_changed( Parm* parm )				{}

protected:

	GroupUI* groupUI;

	GeomGroup* geomGroup;
	vector< Geom* > geomVec;

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

	Slider* scaleSlider;
	Input* scaleInput;

};

#endif 

