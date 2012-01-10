//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// parmScreen.h: interface for the geomScreen class.
// J.R. Gloudemans
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PARMSCREEN_H__015D1220_857A_11D7_AC31_0003473A025A__INCLUDED_)
#define AFX_PARMSCREEN_H__015D1220_857A_11D7_AC31_0003473A025A__INCLUDED_

#include "parmFlScreen.h"
#include "screenMgr.h"
#include "aircraft.h"
#include "gui_device.h"

using namespace std;

class ParmScreen  
{
public:
	ParmScreen(ScreenMgr* mgr, Aircraft* airPtr);
	virtual ~ParmScreen();

	virtual void show( Parm* p );

	bool isShown()								{ return !!parmUI->UIWindow->shown(); }

	void position( int x, int y )				{ parmUI->UIWindow->position( x, y ); }

	static void staticScreenCB( Fl_Widget *w, void* data ) { ((ParmScreen*)data)->screenCB( w ); }
	virtual void screenCB( Fl_Widget* w );

	virtual void update(Parm* p);

	void SelectToLink( int sel );
	void SelectFromLink( int sel );

protected:

	Parm* currParm;
	ScreenMgr* screenMgrPtr;
	Aircraft* aircraftPtr;
	ParmUI* parmUI;

	char nameStr[256];

};

#endif 
