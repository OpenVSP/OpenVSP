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
    void AddArmSliders( GroupLayout &layout, const string &title, int bw, SliderAdjRangeInput &shabad, SliderAdjRangeInput &shfe,
                        SliderAdjRangeInput &shie, SliderAdjRangeInput &elbow, SliderAdjRangeInput &forearm,
                        SliderAdjRangeInput &wrist, SliderAdjRangeInput &wristdev );
    void AddLegSliders( GroupLayout &layout, const string &title, int bw, SliderAdjRangeInput &hipabad, SliderAdjRangeInput &hipfe,
                        SliderAdjRangeInput &hipie, SliderAdjRangeInput &knee, SliderAdjRangeInput &ankle );


    GroupLayout m_AnthropoLayout;

    Choice m_LenUnitChoice;
    Choice m_MassUnitChoice;

    Choice m_RepresentationChoice;

    Choice m_GenderChoice;

    SliderAdjRangeInput m_StatureSlider;
    SliderAdjRangeInput m_StaturePctSlider;

    SliderAdjRangeInput m_BMISlider;
    SliderAdjRangeInput m_BMIPctSlider;
    SliderAdjRangeInput m_MassSlider;
    Output m_VolumeOutput;
    ToggleButton m_AutoDensityToggle;

    SliderAdjRangeInput m_AgeSlider;
    SliderAdjRangeInput m_SitFracSlider;

#ifdef SHOW_SHOW_SKEL_BUTTON
    ToggleButton m_ShowSkelButton;
#endif

    GroupLayout m_PoseLayout;
    GroupLayout m_ArmRtLayout;
    GroupLayout m_ArmLtLayout;
    GroupLayout m_LegRtLayout;
    GroupLayout m_LegLtLayout;

    Choice m_PresetPoseChoice;

    Choice m_PoseMethodChoice;
    SliderAdjRangeInput m_ARAPMaxIterSlider;
    SliderAdjRangeInput m_ARAPTolSlider;
    SliderAdjRangeInput m_FASTIterSlider;
    TriggerButton m_SetPose;

    SliderAdjRangeInput m_BackSlider;
    SliderAdjRangeInput m_WaistSlider;
    SliderAdjRangeInput m_TorsoTwistSlider;
    SliderAdjRangeInput m_TorsoBendSlider;

    SliderAdjRangeInput m_NodSlider;
    SliderAdjRangeInput m_RotateHeadSlider;
    SliderAdjRangeInput m_HeadTiltSlider;

    ToggleButton m_RLSymButton;

    SliderAdjRangeInput m_WristRtSlider;
    SliderAdjRangeInput m_WristDevRtSlider;
    SliderAdjRangeInput m_ForearmRtSlider;
    SliderAdjRangeInput m_ElbowRtSlider;
    SliderAdjRangeInput m_ShoulderABADRtSlider;
    SliderAdjRangeInput m_ShoulderFERtSlider;
    SliderAdjRangeInput m_ShoulderIERtSlider;

    SliderAdjRangeInput m_HipABADRtSlider;
    SliderAdjRangeInput m_HipFERtSlider;
    SliderAdjRangeInput m_HipIERtSlider;
    SliderAdjRangeInput m_KneeRtSlider;
    SliderAdjRangeInput m_AnkleRtSlider;

    SliderAdjRangeInput m_WristLtSlider;
    SliderAdjRangeInput m_WristDevLtSlider;
    SliderAdjRangeInput m_ForearmLtSlider;
    SliderAdjRangeInput m_ElbowLtSlider;
    SliderAdjRangeInput m_ShoulderABADLtSlider;
    SliderAdjRangeInput m_ShoulderFELtSlider;
    SliderAdjRangeInput m_ShoulderIELtSlider;

    SliderAdjRangeInput m_HipABADLtSlider;
    SliderAdjRangeInput m_HipFELtSlider;
    SliderAdjRangeInput m_HipIELtSlider;
    SliderAdjRangeInput m_KneeLtSlider;
    SliderAdjRangeInput m_AnkleLtSlider;


};


#endif // !defined(HUMANGEOM_SCREEN__INCLUDED_)
