//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// WingScreen.h: UI for Wing Geom
// J.R Gloudemans
//
//////////////////////////////////////////////////////////////////////

#if !defined(WINGSCREEN__INCLUDED_)
#define WINGSCREEN__INCLUDED_

#include "ScreenBase.h"
#include "GuiDevice.h"
#include "GroupLayout.h"
#include "WingGeom.h"

#include <FL/Fl.H>

class WingScreen : public BlendScreen
{
public:
    WingScreen( ScreenMgr* mgr );
    virtual ~WingScreen();

    virtual void Show();
    virtual bool Update();

    virtual void CallBack( Fl_Widget *w );
    virtual void GuiDeviceCallBack( GuiDevice* d );

protected:

    // Overall planform tab
    GroupLayout m_PlanLayout;

    SliderAdjRangeInput m_PlanSpanSlider;
    SliderAdjRangeInput m_PlanProjSpanSlider;
    SliderAdjRangeInput m_PlanChordSlider;
    SliderAdjRangeInput m_PlanAreaSlider;
    Output m_PlanAROutput;

    Choice m_RootCapTypeChoice;
    SliderAdjRangeInput m_RootCapLenSlider;
    SliderAdjRangeInput m_RootCapOffsetSlider;
    SliderAdjRangeInput m_RootCapStrengthSlider;
    ToggleButton m_RootCapSweepFlagButton;

    Choice m_TipCapTypeChoice;
    SliderAdjRangeInput m_TipCapLenSlider;
    SliderAdjRangeInput m_TipCapOffsetSlider;
    SliderAdjRangeInput m_TipCapStrengthSlider;
    ToggleButton m_TipCapSweepFlagButton;

    SliderAdjRangeInput m_CapTessSlider;

    SliderAdjRangeInput m_IncidenceSlider;
    SliderAdjRangeInput m_IncidenceLocSlider;

    SliderAdjRangeInput m_LEClusterSlider;
    SliderAdjRangeInput m_TEClusterSlider;
    Output m_SmallPanelWOutput;
    Output m_MaxGrowthOutput;

    // Wing section tab
    GroupLayout m_SectionLayout;

    IndexSelector m_SectIndexSelector;

    StringOutput m_NumSectOutput;

    TriggerButton m_SplitSectButton;
    TriggerButton m_CutSectButton;
    TriggerButton m_CopySectButton;
    TriggerButton m_PasteSectButton;
    TriggerButton m_InsertSectButton;

    SliderAdjRangeInput m_SectUTessSlider;
    SliderAdjRangeInput m_RootClusterSlider;
    SliderAdjRangeInput m_TipClusterSlider;

    ToggleButton m_TwistAbsoluteToggle;
    ToggleButton m_TwistRelativeToggle;
    ToggleRadioGroup m_TwistAbsRelToggle;

    ToggleButton m_RotateFoilMatchDihedral;
    ToggleButton m_RotateRootFoilMatchDihedral;
    ToggleButton m_CorrectFoil;

    ToggleButton m_DihedralAbsoluteToggle;
    ToggleButton m_DihedralRelativeToggle;
    ToggleRadioGroup m_DihedralAbsRelToggle;

    WingDriverGroup m_DefaultWingDriverGroup;
    DriverGroupBank m_WingDriverGroupBank;

    TriggerButton m_TestDriverGroupButton;

    Output m_SectProjSpanOutput;

    SliderAdjRangeInput m_SweepSlider;
    SliderAdjRangeInput m_SweepLocSlider;
    SliderAdjRangeInput m_SecSweepLocSlider;

    SliderAdjRangeInput m_TwistSlider;
    SliderAdjRangeInput m_TwistLocSlider;

    SliderAdjRangeInput m_DihedralSlider;
    ToggleButton m_RotateThisFoilMatchDihedral;

    TriggerButton m_CopyAfButton;
    TriggerButton m_PasteAfButton;

    // Overall planform tab
    GroupLayout m_ModifyLayout;

    IndexSelector m_AfModIndexSelector;

    Choice m_TECloseChoice;
    ToggleButton m_TECloseABSButton;
    ToggleButton m_TECloseRELButton;
    ToggleRadioGroup m_TECloseGroup;

    SliderAdjRange2Input m_CloseTEThickSlider;

    Choice m_TETrimChoice;
    ToggleButton m_TETrimABSButton;
    ToggleButton m_TETrimRELButton;
    ToggleRadioGroup m_TETrimGroup;

    SliderAdjRange2Input m_TrimTEXSlider;
    SliderAdjRange2Input m_TrimTEThickSlider;

    Choice m_TECapChoice;
    SliderAdjRangeInput m_TECapLengthSlider;
    SliderAdjRangeInput m_TECapOffsetSlider;
    SliderAdjRangeInput m_TECapStrengthSlider;

    Choice m_LECloseChoice;
    ToggleButton m_LECloseABSButton;
    ToggleButton m_LECloseRELButton;
    ToggleRadioGroup m_LECloseGroup;

    SliderAdjRange2Input m_CloseLEThickSlider;

    Choice m_LETrimChoice;
    ToggleButton m_LETrimABSButton;
    ToggleButton m_LETrimRELButton;
    ToggleRadioGroup m_LETrimGroup;

    SliderAdjRange2Input m_TrimLEXSlider;
    SliderAdjRange2Input m_TrimLEThickSlider;

    Choice m_LECapChoice;
    SliderAdjRangeInput m_LECapLengthSlider;
    SliderAdjRangeInput m_LECapOffsetSlider;
    SliderAdjRangeInput m_LECapStrengthSlider;

    SliderAdjRangeInput m_AFThetaSlider;
    SliderAdjRangeInput m_AFScaleSlider;
    SliderAdjRangeInput m_AFDeltaXSlider;
    SliderAdjRangeInput m_AFDeltaYSlider;
    SliderAdjRangeInput m_AFShiftLESlider;

};


#endif // !defined(WINGSCREEN__INCLUDED_)
