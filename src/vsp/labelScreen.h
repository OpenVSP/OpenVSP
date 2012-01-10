//******************************************************************************
//    
//   Label Screen Class
//   
//   J.R. Gloudemans - 8/12/03
//******************************************************************************

#ifndef LABELSCREEN_H
#define LABELSCREEN_H

#include "screenMgr.h"
#include "aircraft.h"
#include "labelFlScreen.h"

#include <deque>

using namespace std;

class LabelScreen
{
public:

	LabelScreen(ScreenMgr* mgrPtr, Aircraft* airPtr, VspGlWindow* winPtr);
	virtual ~LabelScreen() {}

	virtual void show()			{ if (labelUI) labelUI->UIWindow->show(); }
	virtual void update();
//	virtual void draw();

	virtual void loadLabelBrowser();
	virtual vector< LabelGeom* > getSelectedLabels();
	virtual LabelGeom* getActiveLabel();
	virtual void selectLabelBrowser( LabelGeom* label );
	virtual void selectVertex(VertexID vert, float mx, float my);
	virtual void setTrack(int on);
	virtual int getTrackFlag()								{ return track; }
	virtual void setHighlight(int on);
	virtual int getHighlightFlag()								{ return highlight; }
	void cleanRulers(); // make sure rulers are left in a stable state when selection is switched

	static Stringc getNameString(vector< LabelGeom* > selVec);

	static void staticScreenCB( Fl_Widget *w, void* data ) { ((LabelScreen*)data)->screenCB( w ); }
	virtual void screenCB( Fl_Widget* w );

	vector< LabelGeom* > s_get_label_vec(deque< Stringc > selectVec);
	virtual void s_add(int src, int type, Stringc name);
	virtual void s_remove(int src, vector< LabelGeom* > removeVec);
	virtual void s_select_all(int src);
	virtual void s_select(int src, vector< LabelGeom* > selectVec);
	virtual void s_vert_script(VertexID vert, Stringc command);

protected:
	
	ScreenMgr* screenMgrPtr;
	Aircraft* aircraftPtr;
	VspGlWindow* glWinPtr;

	LabelUI* labelUI;

	int track;
	int highlight;
};

#endif


