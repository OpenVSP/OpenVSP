//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// AuxiliaryGeomScreen.h: UI for Auxiliary Geom
// Rob McDonald
//
//////////////////////////////////////////////////////////////////////

#if !defined(AUXILIARYGEOMSCREEN__INCLUDED_)
#define AUXILIARYGEOMSCREEN__INCLUDED_

#include "ScreenBase.h"
#include "GuiDevice.h"

#include <FL/Fl.H>

class AuxiliaryGeomScreen : public GeomScreen
{
public:
    AuxiliaryGeomScreen( ScreenMgr* mgr );
    virtual ~AuxiliaryGeomScreen()                            {}

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
    GroupLayout m_1ptGroundPlaneLayout;
    GroupLayout m_3ptCCELayout;

    Choice m_AuxiliaryGeomModeChoice;

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

    // 1pt Ground Plane

    Choice m_1ptBogie1Choice;
    Choice m_1ptBogie1SymmChoice;
    Choice m_1ptBogie1SuspensionModeChoice;
    Choice m_1ptBogie1TireModeChoice;

    SliderAdjRangeInput m_1ptBogieThetaSlider;
    SliderAdjRangeInput m_1ptWheelThetaSlider;
    SliderAdjRangeInput m_1ptRollThetaSlider;


    // 3pt CCE

    Choice m_3ptCCEBogie1Choice;
    Choice m_3ptCCEBogie1SymmChoice;
    Choice m_3ptCCEBogie1SuspensionModeChoice;
    Choice m_3ptCCEBogie1TireModeChoice;

    Choice m_3ptCCEBogie2Choice;
    Choice m_3ptCCEBogie2SymmChoice;
    Choice m_3ptCCEBogie2SuspensionModeChoice;
    Choice m_3ptCCEBogie2TireModeChoice;

    Choice m_3ptCCEBogie3Choice;
    Choice m_3ptCCEBogie3SymmChoice;
    Choice m_3ptCCEBogie3SuspensionModeChoice;
    Choice m_3ptCCEBogie3TireModeChoice;

    TriggerButton m_ReadCCEFileButton;
    Choice m_CCEUnitChoice;

    SliderAdjRangeInput m_CCEMainGearOffsetSlider;

    GroupLayout* m_CurrDisplayGroup;
};


#endif // !defined(AUXILIARYGEOMSCREEN__INCLUDED_)
