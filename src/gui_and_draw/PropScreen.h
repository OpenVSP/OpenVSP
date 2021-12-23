//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// PropScreen.h: UI for Propeller Geom
// Rob McDonald
//
//////////////////////////////////////////////////////////////////////

#if !defined(PROPSCREEN__INCLUDED_)
#define PROPSCREEN__INCLUDED_

#include "ScreenBase.h"
#include "GuiDevice.h"
#include "GroupLayout.h"

#include <FL/Fl.H>

class PropScreen : public XSecScreen
{
public:
    PropScreen( ScreenMgr* mgr );
    virtual ~PropScreen();

    virtual void Show();
    virtual bool Update();

    virtual void CallBack( Fl_Widget *w );
    virtual void GuiDeviceCallBack( GuiDevice* d );

protected:

    enum { CHORD, TWIST, RAKE, SKEW, SWEEP, AXIAL, TANGENTIAL, THICK, CLI };

    GroupLayout m_DesignLayout;

    SliderAdjRangeInput m_PropDiameterSlider;

    SliderAdjRangeInput m_NBladeSlider;

    SliderAdjRangeInput m_PropRotateSlider;
    SliderAdjRangeInput m_Beta34Slider;
    SliderAdjRangeInput m_FeatherSlider;
    SliderAdjRangeInput m_PreconeSlider;

    ToggleButton m_Beta34Toggle;
    ToggleButton m_FeatherToggle;
    ToggleRadioGroup m_BetaToggle;

    SliderAdjRangeInput m_ConstructSlider;
    SliderAdjRangeInput m_FeatherOffsetSlider;
    SliderAdjRangeInput m_FeatherAxisSlider;

    ToggleButton m_ReverseToggle;
    ToggleButton m_CylindricalSectionsToggle;

    SliderAdjRangeInput m_AFLimitSlider;
    Output m_AFOutput;
    Output m_CLiOutput;
    Output m_SolidityOutput;
    Output m_TSolidityOutput;
    Output m_PSolidityOutput;
    Output m_ChordOutput;
    Output m_TChordOutput;
    Output m_PChordOutput;

    Choice m_PropModeChoice;

    TriggerButton m_InsertXSec;
    TriggerButton m_CutXSec;
    TriggerButton m_CopyXSec;
    TriggerButton m_PasteXSec;

    FractParmSlider m_XSecRadSlider;

    // Overall planform tab
    GroupLayout m_ModifyLayout;

    IndexSelector m_XSecModIndexSelector;

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

    GroupLayout m_BladeLayout;

    TriggerButton m_ApproxAllButton;
    TriggerButton m_ResetThickness;
    Choice m_CurveChoice;
    int m_EditCurve;

    PCurveEditor m_CurveEditor;

    GroupLayout m_MoreLayout;

    SliderAdjRangeInput m_RFoldSlider;
    SliderAdjRangeInput m_AxFoldSlider;
    SliderAdjRangeInput m_OffFoldSlider;
    SliderAdjRangeInput m_AzFoldSlider;
    SliderAdjRangeInput m_ElFoldSlider;
    SliderAdjRangeInput m_FoldAngleSlider;

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

    SliderAdjRangeInput m_LEClusterSlider;
    SliderAdjRangeInput m_TEClusterSlider;
    SliderAdjRangeInput m_RootClusterSlider;
    SliderAdjRangeInput m_TipClusterSlider;
    Output m_SmallPanelWOutput;
    Output m_MaxGrowthOutput;

};


#endif // !defined(PROPSCREEN__INCLUDED_)
