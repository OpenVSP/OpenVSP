//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

//******************************************************************************
//    
//   Vorview Screen Class
//   
//   J.R. Gloudemans - 11/5/04
//******************************************************************************

#ifndef VORVIEWSCREEN_H
#define VORVIEWSCREEN_H

#include "screenMgr.h"
#include "aircraft.h"
#include "vorviewFlScreen.h"
#include "vorlaxOutScreen.h"
#include <FL/Fl.H>

#include <deque>

using namespace std;

class VorGeom;

class InputValLink
{
public:

	InputValLink(double* v, Fl_Input* i);
	InputValLink(int* v, Fl_Input* i);

	int* ival;
	double* dval;
	Fl_Input* input;

	void updateVal();
	void updateInput();
};

class InputVecLink
{
public:

	InputVecLink( vector<double*> v, Fl_Roller* roll, Fl_Button* lbut, Fl_Button* rbut, 
						    vector< Fl_Input*> ivec, vector< Fl_Button*> lvec );

	void updateVal();
	void updateInput();

	static void staticScreenCB( Fl_Widget *w, void* data ) { ((InputVecLink*)data)->screenCB( w ); }
	virtual void screenCB( Fl_Widget* w );



protected:

	int startIndex;

	vector<double*> dvalVec;
	Fl_Roller* roller;
	Fl_Button* leftButton;
	Fl_Button* rightButton;
	vector< Fl_Input* > inputVec;
	vector< Fl_Button* > labelVec;
	vector< char* > labelStrVec;

};

class VorviewScreen : public VspScreen
{
public:

	VorviewScreen(ScreenMgr* mgrPtr, Aircraft* airPtr, VspGlWindow* winPtr);
	virtual ~VorviewScreen() {}

	virtual void show(Geom* geomPtr);		
	virtual void update();

	static void staticScreenCB( Fl_Widget *w, void* data ) { ((VorviewScreen*)data)->screenCB( w ); }
	virtual void screenCB( Fl_Widget* w );

	void hide();
	void closeCB( Fl_Widget* w );
	void position( int x, int y );
	void setTitle( const char* name );
	void parm_changed( Parm* parm );
	void updateName();

	//==== Execute Vorlax Stuff ====//
	static void staticRunVorlaxCB( void* data )				{ ((VorviewScreen*)data)->runVorlaxCB(); }
	void runVorlaxCB();	
	void runVorlax();

	void updateVorlaxProgress( double fract );

protected:
	
	ScreenMgr* screenMgrPtr;
	Aircraft* aircraftPtr;
	VspGlWindow* glWinPtr;

	VorviewUI* vorviewUI;
	VorlaxOutUI* vorlaxOutUI;

	VorGeom* vorGeom;

	vector< InputValLink* > ivLinkVec;

	InputVecLink* machNumLink;
	InputVecLink* alphaNumLink;
	InputVecLink* atrimNumLink;
	InputVecLink* cltrimNumLink;
	InputVecLink* cmtrimNumLink;

	int cpMinClampVal, cpMaxClampVal;

	Stringc titleName;

	SliderInputCombo* m_KeySliceYPosSlider;
	SliderInputCombo* m_EditKeySliceYPosSlider;

	bool m_DisplayResults;

};

#endif

