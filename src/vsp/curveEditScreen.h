//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// CurveEditScreen.h: interface for the geomScreen class.
// J.R. Gloudemans
//////////////////////////////////////////////////////////////////////

#ifndef CURVE_EDIT_H
#define CURVE_EDIT_H

#include "curveEditFlScreen.h"
#include "editCurve.h"
#include "screenMgr.h"
#include "vspGlWindow.h"
#include "gui_device.h"
#include "vspScreen.h"
#include <FL/Fl_File_Chooser.H>
#include "loadjpg.h"


class CurveEditScreen : public VspScreen 
{
public:
	CurveEditScreen(ScreenMgr* mgr, Aircraft* airPtr);
	virtual ~CurveEditScreen();

	void show(Geom* geomPtr);
	void hide();
	void closeCB( Fl_Widget* w );
	void setTitle( const char* name );

	virtual void show();
	virtual void setEditCurvePtr( EditCurve* ptr );

	void position( int x, int y )						{ curveEditUI->UIWindow->position( x, y ); }

	static void staticScreenCB( Fl_Widget *w, void* data ) { ((CurveEditScreen*)data)->screenCB( w ); }
	virtual void screenCB( Fl_Widget* w );
	void parm_changed( Parm* parm );

	void update();


private:

	ScreenMgr* screenMgrPtr;
	Aircraft* aircraftPtr;
	CurveEditUI* curveEditUI;

	XSecGlWindow* glWin;

	JpgData jpgImgData;

	EditCurve* editCurvePtr;

	Slider_adj_range* xPntSlider;
	Slider_adj_range* yPntSlider;
	Input* xPntInput;
	Input* yPntInput;

	Slider_adj_range* scaleTanSlider;
	Input* scaleTanInput;

	LogSlider* scaleXSlider;
	LogSlider* scaleYSlider;
	Input*  scaleXInput;
	Input*  scaleYInput;

	Input* widthInput;
	Input* heightInput;


};

#endif 
