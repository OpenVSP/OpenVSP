//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// cfdMeshScreen.h: interface for the geomScreen class.
// J.R Gloudemans
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CFDMESHSCREEN_H__015D1220_857A_11D7_AC31_0003473A025A__INCLUDED_)
#define AFX_CFDMESHSCREEN_H__015D1220_857A_11D7_AC31_0003473A025A__INCLUDED_

#include "cfdMeshFlScreen.h"
#include "aircraft.h"
#include "screenMgr.h"
#include "part.h"
#include "stringc.h"

#include <deque>
using namespace std;

class CfdMeshScreen  
{
public:
	CfdMeshScreen(ScreenMgr* mgr, Aircraft* airPtr);
	virtual ~CfdMeshScreen();

	virtual void show();

	bool isShown()								{ return !!cfdMeshUI->UIWindow->shown(); }

	void position( int x, int y )				{ cfdMeshUI->UIWindow->position( x, y ); }

	static void staticScreenCB( Fl_Widget *w, void* data ) { ((CfdMeshScreen*)data)->screenCB( w ); }
	virtual void screenCB( Fl_Widget* w );
	void parm_changed( Parm* parm )				{}

	void addOutputText( const char* text );

	void update();
	void setMeshExportFlags();
	Stringc truncateFileName( const char* fn, int len );


private:

	int currSourceType;

	Fl_Text_Buffer m_TextBuffer;

	SliderInputCombo* m_LengthSlider;
	SliderInputCombo* m_RadiusSlider;

	SliderInputCombo* m_Length2Slider;
	SliderInputCombo* m_Radius2Slider;

	SliderInputCombo* m_GlobalEdgeSizeSlider;
	SliderInputCombo* m_MinEdgeSizeSlider;
	SliderInputCombo* m_MaxGapSizeSlider;
	SliderInputCombo* m_NumCircSegmentSlider;
	SliderInputCombo* m_GrowRatioSlider;

	SliderInputCombo* m_FarXScaleSlider;
	SliderInputCombo* m_FarYScaleSlider;
	SliderInputCombo* m_FarZScaleSlider;

	ScreenMgr* screenMgrPtr;
	Aircraft* aircraftPtr;
	CFDMeshUI* cfdMeshUI;
};

#endif 
