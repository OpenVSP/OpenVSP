//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// ClearanceScreen.h: UI for Clearance Geom
// Rob McDonald
//
//////////////////////////////////////////////////////////////////////

#if !defined(CLEARANCESCREEN__INCLUDED_)
#define CLEARANCESCREEN__INCLUDED_

#include "ScreenBase.h"
#include "GuiDevice.h"

#include <FL/Fl.H>

class ClearanceScreen : public GeomScreen
{
public:
    ClearanceScreen( ScreenMgr* mgr );
    virtual ~ClearanceScreen()                            {}

    virtual void DisplayGroup( GroupLayout* group );

    virtual void Show();
    virtual bool Update();

    virtual void UpdateGroundPlaneChoices();

    virtual void CallBack( Fl_Widget *w );
    virtual void GuiDeviceCallBack( GuiDevice* device );

protected:

    GroupLayout m_DesignLayout;

    GroupLayout m_RotorTipPathLayput;
    GroupLayout m_RotorBurstLayout;
    GroupLayout m_3ptGroundPlaneLayout;
    GroupLayout m_2ptGroundPlaneLayout;


    Choice m_ClearanceModeChoice;

    // Rotor Tip Path
    ToggleButton m_RTP_AutoDiamToggleButton;

    SliderAdjRangeInput m_RTP_DiameterSlider;
    SliderAdjRangeInput m_RTP_FlapRadiusFractSlider;

    SliderAdjRangeInput m_RTP_ThetaThrustSlider;
    SliderAdjRangeInput m_RTP_ThetaAntiThrustSlider;

    // Rotor Burst
    ToggleButton m_RB_AutoDiamToggleButton;

    SliderAdjRangeInput m_RB_DiameterSlider;
    SliderAdjRangeInput m_RB_FlapRadiusFractSlider;

    SliderAdjRangeInput m_RB_ThetaThrustSlider;
    SliderAdjRangeInput m_RB_ThetaAntiThrustSlider;

    SliderAdjRangeInput m_RB_RootLengthSlider;
    SliderAdjRangeInput m_RB_RootOffsetSlider;

    // 3pt Ground Plane

    vector < string > m_BogieIDVec;

    Choice m_3ptBogie1Choice;
    Choice m_3ptBogie1SymmChoice;
    Choice m_3ptBogie1SuspensionModeChoice;
    Choice m_3ptBogie1TireModeChoice;

    Choice m_3ptBogie2Choice;
    Choice m_3ptBogie2SymmChoice;
    Choice m_3ptBogie2SuspensionModeChoice;
    Choice m_3ptBogie2TireModeChoice;

    Choice m_3ptBogie3Choice;
    Choice m_3ptBogie3SymmChoice;
    Choice m_3ptBogie3SuspensionModeChoice;
    Choice m_3ptBogie3TireModeChoice;

    // 2pt Ground Plane

    Choice m_2ptBogie1Choice;
    Choice m_2ptBogie1SymmChoice;
    Choice m_2ptBogie1SuspensionModeChoice;
    Choice m_2ptBogie1TireModeChoice;

    Choice m_2ptBogie2Choice;
    Choice m_2ptBogie2SymmChoice;
    Choice m_2ptBogie2SuspensionModeChoice;
    Choice m_2ptBogie2TireModeChoice;

    SliderAdjRangeInput m_2ptBogieThetaSlider;
    SliderAdjRangeInput m_2ptWheelThetaSlider;

    GroupLayout* m_CurrDisplayGroup;
};


#endif // !defined(CLEARANCESCREEN__INCLUDED_)
