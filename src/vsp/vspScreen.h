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

#ifndef VSPSCREEN_H
#define VSPSCREEN_H

#include <stdio.h>
#include <FL/Fl.H> 
#include "geom.h"
#include "gui_device.h"
#include "podScreen.h"
#include "xsecGeomFlScreen.h"

class Input;
class Slider;
class LogSlider;
class Slider_adj_range;
class ParmButton;
class ScreenMgr;
 
class VspScreen
{
public:
     VspScreen(ScreenMgr* mgrPtr);
     virtual ~VspScreen();

	 virtual void show(Geom* geomPtr) = 0;
	 virtual void hide() = 0;

	 static void staticScreenCB( Fl_Widget *w, void* data ) { ((VspScreen*)data)->screenCB( w ); }
	 virtual void screenCB( Fl_Widget* w ) = 0;

	 virtual void position( int x, int y ) = 0;

	 virtual void setTitle( const char* name ) = 0;

	 virtual void parm_changed( Parm* parm ) = 0;		//jrg need this???

	 virtual void closeCB( Fl_Widget* w ) = 0;
	 static void staticCloseCB( Fl_Widget *w, void* data ) { ((VspScreen*)data)->closeCB( w ); }

	 ScreenMgr* getScreenMgr()						{ return screenMgrPtr; }

//	 void s_setname(int src, const char * name);

protected:

	ScreenMgr* screenMgrPtr;

	Stringc title;


};

class CompScreen : public VspScreen
{
public:
     CompScreen(ScreenMgr* mgrPtr);
     virtual ~CompScreen();

	 virtual void init();
	 virtual void show(Geom* geomPtr);
	 virtual void screenCB( Fl_Widget* w );

	 virtual void setColor( int r, int g, int b );

	 virtual void showTextureInfo(Geom* geomPtr);


protected:

	int initFlag;

	Geom* geomPtr;

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

	Slider_adj_range* rotOriginSlider;
	Input* rotOriginInput;

	Slider* uSlider;
	Slider* vSlider;
	Input* uInput;
	Input* vInput;

	Slider* numXsecsSlider;
	Slider* numPntsSlider;
	Input* numXsecsInput;
	Input* numPntsInput;

	LogSlider* scaleSlider;
	Input* scaleInput;

	Fl_Input *nameInput;

	Fl_Button *colorButton;
	Fl_Button *primColorButton;

	Fl_Choice *materialChoice;

	Fl_Light_Button *attachUVButton;
	Fl_Light_Button *attachFixedButton;
	Fl_Light_Button *attachMatrixButton;

	Fl_Value_Slider *redSlider;
	Fl_Value_Slider *greenSlider;
	Fl_Value_Slider *blueSlider;

	Fl_Button *redButton;
	Fl_Button *greenButton;
	Fl_Button *blueButton;
	Fl_Button *yellowButton;
	Fl_Button *magentaButton;
	Fl_Button *cyanButton;
	Fl_Button *blackButton;
	Fl_Button *darkBlueButton;
	Fl_Button *greyButton;

	Fl_Choice *symmetryChoice;

	Fl_Button *acceptScaleButton;
	Fl_Button *resetScaleButton;

	Fl_Menu_Item *noneSymMenu;
    Fl_Menu_Item *xySymMenu;
    Fl_Menu_Item *xzSymMenu;
    Fl_Menu_Item *yzSymMenu;
 
	Fl_Light_Button *absButton;
	Fl_Light_Button *relButton;

	Fl_Choice *outputNameChoice;
	Fl_Light_Button *outputEnableButton;


	Fl_Light_Button* shellEnableButton;
	Fl_Counter* priorityCounter;
	Input* densityInput;
	Input* shellMassAreaInput;


	ParmButton* xLocButton;
	ParmButton* yLocButton;
	ParmButton* zLocButton;

	ParmButton* xRotButton;
	ParmButton* yRotButton;
	ParmButton* zRotButton;

	ParmButton* originButton;

	ParmButton* numPntsButton;
	ParmButton* numXsecsButton;

	ParmButton* densityButton;
	ParmButton* shellMassAreaButton;



};

class PodScreen : public CompScreen
{
public:
	PodScreen(ScreenMgr* mgrPtr);
	virtual ~PodScreen();

	void show(Geom* geomPtr);
	void hide();
	void screenCB( Fl_Widget* w );
	void closeCB( Fl_Widget* w );
	void position( int x, int y );
	void setTitle( const char* name );
	void parm_changed( Parm* parm )				{}

protected:

	PodUI* podUI;

	Slider_adj_range* lengthSlider;
	Input* lengthInput;

	Slider_adj_range* fineSlider;
	Input* fineInput;

	PodGeom* currGeom;

	ParmButton* lengthButton;
	ParmButton* fineButton;

};

class XsecGeomScreen : public CompScreen
{
public:
	XsecGeomScreen(ScreenMgr* mgrPtr);
	virtual ~XsecGeomScreen();

	void show(Geom* geomPtr);
	void hide();
	void screenCB( Fl_Widget* w );
	void closeCB( Fl_Widget* w );
	void position( int x, int y );
	void setTitle( const char* name );
	void parm_changed( Parm* parm );
	void update_dims();

protected:

	XsecGeomUI* xsecGeomUI;
	XSecGeom* currGeom;

	Slider_adj_range* xScaleSlider;
	Input* xScaleInput;
	Slider_adj_range* yScaleSlider;
	Input* yScaleInput;
	Slider_adj_range* zScaleSlider;
	Input* zScaleInput;

	Input* uTanLimitInput;
	Input* wTanLimitInput;

};


#endif  


     



