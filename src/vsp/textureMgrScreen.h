//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// textureMgrScreen
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TEXTUREMGRSCREEN_H__015D1220_857A_11D7_AC31_0003473A025A__INCLUDED_)
#define AFX_TEXTUREMGRSCREEN_H__015D1220_857A_11D7_AC31_0003473A025A__INCLUDED_

#include "textureMgrFlScreen.h"
#include "aircraft.h"
#include "screenMgr.h"
#include "vspGlWindow.h"

class TexWinDraw : public DrawBase
{
public:
	TexWinDraw();
	virtual ~TexWinDraw();

	virtual void SetTexture( AppliedTex* tex );
	virtual void draw();
protected:

	AppliedTex* m_TexPtr;

};



class TextureMgrScreen  
{
public:
	TextureMgrScreen(ScreenMgr* mgr, Aircraft* airPtr);
	virtual ~TextureMgrScreen();

	virtual void show();

	bool isShown()								{ return !!textureMgrUI->UIWindow->shown(); }
	void position( int x, int y )				{ textureMgrUI->UIWindow->position( x, y ); }

	static void staticScreenCB( Fl_Widget *w, void* data ) { ((TextureMgrScreen*)data)->screenCB( w ); }
	virtual void screenCB( Fl_Widget* w );

	void parm_changed( Parm* parm )				{}
	void update();
	void activate();
	void deactivate();

private:

	ScreenMgr* screenMgrPtr;
	Aircraft* aircraftPtr;
	TextureMgrUI* textureMgrUI;

	SliderInputCombo* m_UPosSlider;
	SliderInputCombo* m_WPosSlider;
	SliderInputCombo* m_UScaleSlider;
	SliderInputCombo* m_WScaleSlider;
	SliderInputCombo* m_BrightSlider;
	SliderInputCombo* m_AlphaSlider;

	TexWinDraw m_TexWinDraw;
	XSecGlWindow* m_TexGLWin;


	int m_currGeomID;
};

#endif 
