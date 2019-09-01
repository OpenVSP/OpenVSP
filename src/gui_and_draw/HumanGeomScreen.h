//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// HumanGeomScreen.h: UI for Ellipsoid Geom
// Rob McDonald
//
//////////////////////////////////////////////////////////////////////

#if !defined(HUMANGEOM_SCREEN__INCLUDED_)
#define HUMANGEOM_SCREEN__INCLUDED_

#include "ScreenBase.h"
#include "GuiDevice.h"

#include <FL/Fl.H>

// #define SHOW_SHOW_SKEL_BUTTON

class HumanGeomScreen : public GeomScreen
{
public:
    HumanGeomScreen( ScreenMgr* mgr );
    virtual ~HumanGeomScreen()                            {}

    virtual void Show();
    virtual bool Update();

    virtual void CallBack( Fl_Widget *w );
    virtual void GuiDeviceCallBack( GuiDevice* d );

protected:

    GroupLayout m_AnthropoLayout;

    Choice m_LenUnitChoice;
    Choice m_MassUnitChoice;

    Choice m_GenderChoice;

    SliderAdjRangeInput m_StatureSlider;
    SliderAdjRangeInput m_StaturePctSlider;

    SliderAdjRangeInput m_BMISlider;
    SliderAdjRangeInput m_BMIPctSlider;
    SliderAdjRangeInput m_MassSlider;

    SliderAdjRangeInput m_AgeSlider;
    SliderAdjRangeInput m_SitFracSlider;

#ifdef SHOW_SHOW_SKEL_BUTTON
    ToggleButton m_ShowSkelButton;
#endif

    GroupLayout m_PoseLayout;

    Choice m_PresetPoseChoice;
    TriggerButton m_SetPose;

    SliderAdjRangeInput m_BackSlider;
    SliderAdjRangeInput m_WaistSlider;

    ToggleButton m_RLSymButton;

    SliderAdjRangeInput m_ElbowRtSlider;
    SliderAdjRangeInput m_ShoulderABADRtSlider;
    SliderAdjRangeInput m_ShoulderFERtSlider;
    SliderAdjRangeInput m_ShoulderIERtSlider;

    SliderAdjRangeInput m_HipABADRtSlider;
    SliderAdjRangeInput m_HipFERtSlider;
    SliderAdjRangeInput m_KneeRtSlider;
    SliderAdjRangeInput m_AnkleRtSlider;

    SliderAdjRangeInput m_ElbowLtSlider;
    SliderAdjRangeInput m_ShoulderABADLtSlider;
    SliderAdjRangeInput m_ShoulderFELtSlider;
    SliderAdjRangeInput m_ShoulderIELtSlider;

    SliderAdjRangeInput m_HipABADLtSlider;
    SliderAdjRangeInput m_HipFELtSlider;
    SliderAdjRangeInput m_KneeLtSlider;
    SliderAdjRangeInput m_AnkleLtSlider;


};


#endif // !defined(HUMANGEOM_SCREEN__INCLUDED_)
