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
#include "ScreenBase.h"
#include "GuiDevice.h"
#include "Vehicle.h"
#include "ScreenMgr.h"
#include "FeaPart.h"

#include <deque>
using namespace std;


class FeaStructScreen : public VspScreen
{
public:
    FeaStructScreen( ScreenMgr* mgr );
    virtual ~FeaStructScreen();

    virtual void Show();

    virtual void CallBack( Fl_Widget* w );

    void closeCB( Fl_Widget* w );
    static void staticCloseCB( Fl_Widget *w, void* data )
    {
        ( ( FeaStructScreen* )data )->closeCB( w );
    }

    void addOutputText( const char* text );
    string truncateFileName( const string &fn, int len );

    bool Update();


private:
    Fl_Text_Buffer m_TextBuffer;

//  XSecGlWindow* m_UpSkinGLWin;
//  XSecGlWindow* m_LowSkinGLWin;

    SliderInput m_DefEdgeSlider;
    SliderInput m_MinSizeSlider;
    SliderInput m_MaxGapSlider;
    SliderInput m_NumCircSegSlider;
    SliderInput m_GrowRatioSlider;
    SliderInput m_ThickScaleSlider;

    SliderInput m_RibThickSlider;
    SliderInput m_RibPosSlider;
    SliderInput m_RibSweepSlider;
    SliderInput m_RibDensitySlider;

    SliderInput m_SparThickSlider;
    SliderInput m_SparPosSlider;
    SliderInput m_SparSweepSlider;
    SliderInput m_SparDensitySlider;

    SliderInput m_UpThickSlider;
    SliderInput m_UpDensitySlider;
    SliderInput m_UpDefThickSlider;
    SliderInput m_UpSpliceLineLocSlider;
    SliderInput m_UpSpliceLocSlider;
    SliderInput m_UpSpliceThickSlider;

    SliderInput m_LowThickSlider;
    SliderInput m_LowDensitySlider;
    SliderInput m_LowDefThickSlider;
    SliderInput m_LowSpliceLineLocSlider;
    SliderInput m_LowSpliceLocSlider;
    SliderInput m_LowSpliceThickSlider;

    SliderInput m_pmXPosSlider;
    SliderInput m_pmYPosSlider;
    SliderInput m_pmZPosSlider;

    Vehicle* m_Vehicle;

    FEAStructUI* m_FeaStructUI;

};

#endif
