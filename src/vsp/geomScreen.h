//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// geomScreen.h: interface for the geomScreen class.
// J.R. Gloudemans
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_GEOMSCREEN_H__015D1220_857A_11D7_AC31_0003473A025A__INCLUDED_)
#define AFX_GEOMSCREEN_H__015D1220_857A_11D7_AC31_0003473A025A__INCLUDED_

#include "geomBrowserScreen.h"
#include "aircraft.h"
#include "screenMgr.h"

#include <deque>
using namespace std;

class GeomScreen  
{
public:
	GeomScreen(ScreenMgr* mgr, Aircraft* airPtr);
	virtual ~GeomScreen();

	virtual void show();
	virtual void hide();

	void position( int x, int y )				{ geomUI->UIWindow->position( x, y ); }
	int x()										{ return geomUI->UIWindow->x(); }
	int y()										{ return geomUI->UIWindow->y(); }
	int w()										{ return geomUI->UIWindow->w(); }
	int h()										{ return geomUI->UIWindow->h(); }

	static void staticScreenCB( Fl_Widget *w, void* data ) { ((GeomScreen*)data)->screenCB( w ); }
	virtual void screenCB( Fl_Widget* w );
	void parm_changed( Parm* parm )				{}

	void update();
	void loadGeomBrowser();
	bool isParentSelected( Geom* geomPtr, vector< Geom* > & selVec );
	void selectGeomBrowser( Geom* geom );
	void loadActiveGeomOutput();
	void deselectGeomBrowser();

	void setGeomDisplayFlag( int flag );

	vector< Geom* > getSelectedGeomVec();
	Stringc getNameString(vector< Geom* > selVec);


	vector< Geom* > s_get_geom(deque< Stringc > selectVec);
	void s_add_geom(int src, int type, Stringc name);
	void s_cut(int src, deque< Stringc > partVec);
	void s_copy(int src, deque< Stringc > partVec);
	void s_paste(int src);
	void s_select_gui();
	void s_select(int src, deque< Stringc > partVec);
	void s_select_all(int src);
	void s_select_none(int src);

private:

	ScreenMgr* screenMgrPtr;
	Aircraft* aircraftPtr;
	GeomUI* geomUI;
	Geom* lastSelectedGeom;
	bool collaspeFlag;
	int lastTopLine;
};

#endif // !defined(AFX_GEOMSCREEN_H__015D1220_857A_11D7_AC31_0003473A025A__INCLUDED_)
