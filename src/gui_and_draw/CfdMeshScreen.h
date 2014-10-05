//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// cfdMeshScreen.h: interface for the geomScreen class.
// J.R Gloudemans
//////////////////////////////////////////////////////////////////////

#if !defined(VSPCFDMESHSCREEN__INCLUDED_)
#define VSPCFDMESHSCREEN__INCLUDED_

#include "ScreenBase.h"
#include "GuiDevice.h"

#include "cfdMeshFlScreen.h"
#include "Vehicle.h"
#include "ScreenMgr.h"

#include <string>
#include <deque>
using namespace std;

class CfdMeshScreen : public VspScreen
{
public:
    CfdMeshScreen( ScreenMgr* mgr );
    virtual ~CfdMeshScreen();

    virtual bool Update();
    virtual void Show();
    virtual void Hide();

    void position( int x, int y )
    {
        m_CfdMeshUI->UIWindow->position( x, y );
    }

    void LoadSetChoice();

    void AddOutputText( const string &text );

    void CallBack( Fl_Widget *w );
    virtual void CloseCallBack( Fl_Widget *w );

    void parm_changed( Parm* parm )             {}

    string truncateFileName( const string &fn, int len );

    void LoadDrawObjs( vector< DrawObj* > &draw_obj_vec );

private:

    Fl_Text_Buffer m_TextBuffer;

    ToggleButton m_DrawMeshButton;
    ToggleButton m_DrawSourceButton;
    ToggleButton m_DrawFarButton;
    ToggleButton m_DrawFarPreButton;
    ToggleButton m_DrawBadButton;
    ToggleButton m_DrawSymmButton;
    ToggleButton m_DrawWakeButton;
    ToggleButton m_DrawTagsButton;

    ToggleButton m_DatToggleButton;
    ToggleButton m_KeyToggleButton;
    ToggleButton m_ObjToggleButton;
    ToggleButton m_PolyToggleButton;
    ToggleButton m_StlToggleButton;
    ToggleButton m_StlMultiSolidToggleButton;
    ToggleButton m_TriToggleButton;
    ToggleButton m_GmshToggleButton;
    ToggleButton m_SrfToggleButton;
    ToggleButton m_TkeyToggleButton;

    ToggleButton m_IntersectSubSurfsButton;

    SliderInput m_LengthSlider;
    SliderInput m_RadiusSlider;

    SliderInput m_Length2Slider;
    SliderInput m_Radius2Slider;

    SliderInput m_U1Slider;
    SliderInput m_W1Slider;

    SliderInput m_U2Slider;
    SliderInput m_W2Slider;

    SliderInput m_BodyEdgeSizeSlider;
    SliderInput m_MinEdgeSizeSlider;
    SliderInput m_MaxGapSizeSlider;
    SliderInput m_NumCircSegmentSlider;
    SliderInput m_GrowRatioSlider;

    SliderInput m_FarXScaleSlider;
    SliderInput m_FarYScaleSlider;
    SliderInput m_FarZScaleSlider;

//  FractParmSlider m_FarXScaleSlider;
//  FractParmSlider m_FarYScaleSlider;
//  FractParmSlider m_FarZScaleSlider;

    SliderInput m_FarXLocationSlider;
    SliderInput m_FarYLocationSlider;
    SliderInput m_FarZLocationSlider;

    SliderInput m_FarEdgeLengthSlider;
    SliderInput m_FarGapSizeSlider;
    SliderInput m_FarCircSegmentSlider;

    SliderInput m_WakeScaleSlider;
    SliderInput m_WakeAngleSlider;

    map< string, int > m_CompIDMap;
    map< string, int > m_WingCompIDMap;
    vector< string > m_GeomVec;
    vector< string > m_WingGeomVec;

    Vehicle* m_Vehicle;
    CFDMeshUI* m_CfdMeshUI;
};

#endif
