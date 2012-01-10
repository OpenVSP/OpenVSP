// structureScreen.h: interface for the geomScreen class.
// J.R. Gloudemans
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_STRUCTURESCREEN_H__015D1220_857A_11D7_AC31_0003473A025A__INCLUDED_)
#define AFX_STRUCTURESCREEN_H__015D1220_857A_11D7_AC31_0003473A025A__INCLUDED_

#include "structureBrowserScreen.h"
#include "aircraft.h"
#include "screenMgr.h"
#include "part.h"

#include <deque>
using namespace std;

class StructureScreen  
{
public:
	StructureScreen(ScreenMgr* mgr, Aircraft* airPtr);
	virtual ~StructureScreen();

	virtual void show();

	bool isShown()								{ return !!structureUI->UIWindow->shown(); }

	void position( int x, int y )				{ structureUI->UIWindow->position( x, y ); }

	static void staticScreenCB( Fl_Widget *w, void* data ) { ((StructureScreen*)data)->screenCB( w ); }
	virtual void screenCB( Fl_Widget* w );
	void parm_changed( Parm* parm )				{}

	static void staticTimerCB( void* data )					{ ((StructureScreen*)data)->timerCB(); 
																Fl::repeat_timeout(1.0, staticTimerCB, data ); }

	virtual void timerCB();						

	void update();


private:

	int currPartType;

	ScreenMgr* screenMgrPtr;
	Aircraft* aircraftPtr;
	StructureUI* structureUI;
};

#endif 
