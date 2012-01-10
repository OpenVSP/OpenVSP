//******************************************************************************
//    
//   Engine Screen Class
//   
//   J.R. Gloudemans - 4/21/03
//******************************************************************************

#ifndef ENGINESCREEN_H
#define ENGINESCREEN_H

#include <stdio.h>
#include <FL/Fl.H> 
#include "geom.h"
#include "gui_device.h"
#include "vspScreen.h"
#include "vspGlWindow.h"
#include "engineFlScreen.h"

class Input;
class Slider;
class Slider_adj_range;
class ScreenMgr;
class EngineGeom; 


class EngineScreen : public CompScreen
{
public:
	EngineScreen(ScreenMgr* mgrPtr);
	virtual ~EngineScreen();

	void show(Geom* geomPtr);
	void hide();
	void screenCB( Fl_Widget* w );
	void closeCB( Fl_Widget* w );
	void position( int x, int y );
	void setTitle( const char* name );
	void parm_changed( Parm* parm );
	void updateName();


protected:

	EngineUI* engineUI;

	Slider_adj_range* radTipSlider;
	Input* radTipInput;
	Slider_adj_range* maxTipSlider;
	Input* maxTipInput;
	Slider_adj_range* hubTipSlider;
	Input* hubTipInput;
	Slider_adj_range* lengthSlider;
	Input* lengthInput;

	Slider_adj_range* exitAreaSlider;
	Input* exitAreaInput;
	Slider_adj_range* nozLengthSlider;
	Input* nozLengthInput;

	Slider_adj_range* ductXOffSlider;
	Input* ductXOffInput;
	Slider_adj_range* ductYOffSlider;
	Input* ductYOffInput;
	Slider_adj_range* ductShapeSlider;
	Input* ductShapeInput;

	Slider_adj_range* cowlLenSlider;
	Input* cowlLenInput;
	Slider_adj_range* engThrtRatioSlider;
	Input* engThrtRatioInput;
	Slider_adj_range* hlThrtRatioSlider;
	Input* hlThrtRatioInput;
	Slider_adj_range* lipFineSlider;
	Input* lipFineInput;
	Slider_adj_range* hwRatioSlider;
	Input* hwRatioInput;
	Slider_adj_range* upShapeSlider;
	Input* upShapeInput;
	Slider_adj_range* lowShapeSlider;
	Input* lowShapeInput;
	Slider_adj_range* inlXRotSlider;
	Input* inlXRotInput;
	Slider_adj_range* scarfSlider;
	Input* scarfInput;

	ParmButton* radTipButton;
	ParmButton* maxTipButton;
	ParmButton* hubTipButton;
	ParmButton* lengthButton;
	ParmButton* exitAreaButton;
	ParmButton* nozLengthButton;
	ParmButton* ductXOffButton;
	ParmButton* ductYOffButton;
	ParmButton* ductShapeButton;
	ParmButton* cowlLenButton;
	ParmButton* engThrtRatioButton;
	ParmButton* hlThrtRatioButton;
	ParmButton* lipFineButton;
	ParmButton* hwRatioButton;
	ParmButton* upShapeButton;
	ParmButton* lowShapeButton;
	ParmButton* inlXRotButton;
	ParmButton* scarfButton;

	EngineGeom* currGeom;

};



#endif  


     



