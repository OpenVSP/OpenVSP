//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// parmPickerScreen.h: interface for the parmPickerScreen class.
//
//////////////////////////////////////////////////////////////////////

#ifndef PARMPICKERSCREEN_H
#define PARMPICKERSCREEN_H

#include "parmPickerFlScreen.h"
#include "screenMgr.h"
#include "aircraft.h"
#include "gui_device.h"

#define DES_FILE 0
#define XDDM_FILE 1

using namespace std;

class ParmPickerScreen  : public VspScreen
{
public:
	ParmPickerScreen(ScreenMgr* mgr, Aircraft* airPtr);
	virtual ~ParmPickerScreen();

	virtual void hide();
	virtual void closeCB( Fl_Widget* w );
	virtual void setTitle( const char* name );
	virtual void parm_changed( Parm* parm )				{}

	virtual void show();
	virtual void show(Geom* geomPtr);

	bool isShown()								{ return !!parmPickerUI->UIWindow->shown(); }

	void position( int x, int y )				{ parmPickerUI->UIWindow->position( x, y ); }

	static void staticScreenCB( Fl_Widget *w, void* data ) { ((ParmPickerScreen*)data)->screenCB( w ); }
	virtual void screenCB( Fl_Widget* w );

	virtual void compGroupChange();
	virtual void update();

	virtual void RemoveAllRefs( GeomBase* g );

protected:

	Aircraft* aircraftPtr;
	ParmPickerUI* parmPickerUI;

	int fileType;
};

#endif	// PARMPICKERSCREEN_H
