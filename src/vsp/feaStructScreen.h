//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// feaStructScreen.h: 
// J.R. Gloudemans
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FEASTRUCTSCREEN_H__015D1220_857A_11D7_AC31_0003473A025A__INCLUDED_)
#define AFX_FEASTRUCTSCREEN_H__015D1220_857A_11D7_AC31_0003473A025A__INCLUDED_

#include "feaStructFlScreen.h"
#include "vspScreen.h"
#include "aircraft.h"
#include "screenMgr.h"
#include "part.h"

#include <deque>
using namespace std;


class FeaStructScreen
{
public:
	FeaStructScreen(ScreenMgr* mgr, Aircraft* airPtr);
	virtual ~FeaStructScreen();

	virtual void show();
	bool isShown()								{ return !!feaStructUI->UIWindow->shown(); }

	void position( int x, int y )				{ feaStructUI->UIWindow->position( x, y ); }

	static void staticScreenCB( Fl_Widget *w, void* data ) { ((FeaStructScreen*)data)->screenCB( w ); }
	virtual void screenCB( Fl_Widget* w );
	void parm_changed( Parm* parm )				{}

	void closeCB( Fl_Widget* w );
	static void staticCloseCB( Fl_Widget *w, void* data ) { ((FeaStructScreen*)data)->closeCB( w ); }

	void addOutputText( const char* text );
	Stringc truncateFileName( const char* fn, int len );
	void setMeshExportFlags();

	void update();


private:
	Fl_Text_Buffer m_TextBuffer;
	XSecGlWindow* m_UpSkinGLWin;
	XSecGlWindow* m_LowSkinGLWin;

	SliderInputCombo* m_DefEdgeSlider;
	SliderInputCombo* m_ThickScaleSlider;

	SliderInputCombo* m_RibThickSlider;
	SliderInputCombo* m_RibPosSlider;
	SliderInputCombo* m_RibSweepSlider;
	SliderInputCombo* m_RibDensitySlider;

	SliderInputCombo* m_SparThickSlider;
	SliderInputCombo* m_SparPosSlider;
	SliderInputCombo* m_SparSweepSlider;
	SliderInputCombo* m_SparDensitySlider;

	SliderInputCombo* m_UpThickSlider;
	SliderInputCombo* m_UpDensitySlider;
	SliderInputCombo* m_UpDefThickSlider;
	SliderInputCombo* m_UpSpliceLineLocSlider;
	SliderInputCombo* m_UpSpliceLocSlider;
	SliderInputCombo* m_UpSpliceThickSlider;

	SliderInputCombo* m_LowThickSlider;
	SliderInputCombo* m_LowDensitySlider;
	SliderInputCombo* m_LowDefThickSlider;
	SliderInputCombo* m_LowSpliceLineLocSlider;
	SliderInputCombo* m_LowSpliceLocSlider;
	SliderInputCombo* m_LowSpliceThickSlider;

	SliderInputCombo* m_pmXPosSlider;
	SliderInputCombo* m_pmYPosSlider;
	SliderInputCombo* m_pmZPosSlider;

	ScreenMgr* screenMgrPtr;
	Aircraft* aircraftPtr;
	FEAStructUI* feaStructUI;

};

#endif 
