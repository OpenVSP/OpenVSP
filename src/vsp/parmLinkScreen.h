//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// parmLinkScreen.h: interface for the geomScreen class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PARMLINKCREEN_H__015D1220_857A_11D7_AC31_0003473A025A__INCLUDED_)
#define AFX_PARMLINKCREEN_H__015D1220_857A_11D7_AC31_0003473A025A__INCLUDED_

#include "parmLinkFlScreen.h"
#include "screenMgr.h"
#include "aircraft.h"
#include "gui_device.h"

using namespace std;

class ParmLinkScreen  : public VspScreen
{
public:
	ParmLinkScreen(ScreenMgr* mgr, Aircraft* airPtr);
	virtual ~ParmLinkScreen();

	virtual void hide();
	virtual void closeCB( Fl_Widget* w );
	virtual void setTitle( const char* name );
	virtual void parm_changed( Parm* parm )				{}

	virtual void show();
	virtual void show(Geom* geomPtr);

	bool isShown()								{ return !!parmLinkUI->UIWindow->shown(); }

	void position( int x, int y )				{ parmLinkUI->UIWindow->position( x, y ); }

	static void staticScreenCB( Fl_Widget *w, void* data ) { ((ParmLinkScreen*)data)->screenCB( w ); }
	virtual void screenCB( Fl_Widget* w );

	virtual void compGroupLinkChange();
	virtual void update();

	virtual void RegisterParmButton( ParmButton* b )	{ m_ParmButtonVec.push_back( b ); }
	virtual void RemoveAllRefs( GeomBase* g );

protected:

	Aircraft* aircraftPtr;
	ParmLinkUI* parmLinkUI;

	SliderInputCombo* m_OffsetSlider;
	SliderInputCombo* m_ScaleSlider;
	SliderInputCombo* m_LowerLimitSlider;
	SliderInputCombo* m_UpperLimitSlider;

	vector< ParmButton* > m_ParmButtonVec;

	Slider* m_User1Slider;
	Input* m_User1Input;
	Slider* m_User2Slider;
	Input* m_User2Input;
	Slider* m_User3Slider;
	Input* m_User3Input;
	Slider* m_User4Slider;
	Input* m_User4Input;
	Slider* m_User5Slider;
	Input* m_User5Input;
	Slider* m_User6Slider;
	Input* m_User6Input;
	Slider* m_User7Slider;
	Input* m_User7Input;
	Slider* m_User8Slider;
	Input* m_User8Input;

	ParmButton* m_User1Button;
	ParmButton* m_User2Button;
	ParmButton* m_User3Button;
	ParmButton* m_User4Button;
	ParmButton* m_User5Button;
	ParmButton* m_User6Button;
	ParmButton* m_User7Button;
	ParmButton* m_User8Button;

	


};

#endif 
