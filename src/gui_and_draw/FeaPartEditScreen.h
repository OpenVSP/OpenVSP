//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// FeaPartEditScreen.h FeaPart editor screen.
// Justin Gravett
//
//////////////////////////////////////////////////////////////////////

#if !defined(FEAPARTEDITSCREEN__INCLUDED_)
#define FEAPARTEDITSCREEN__INCLUDED_

#include "ScreenMgr.h"
#include "ScreenBase.h"

class FeaPartEditScreen : public BasicScreen
{
public:
    FeaPartEditScreen( ScreenMgr* mgr );
    virtual ~FeaPartEditScreen();
    virtual bool Update();
    virtual void GuiDeviceCallBack( GuiDevice* device );
    virtual void CloseCallBack( Fl_Widget *w );

    virtual void FeaPartDispGroup( GroupLayout* group );

    virtual void UpdateFeaPropertyChoice();
    virtual void UpdatePerpendicularRibChoice();
    virtual void UpdateFixPointParentChoice();
    virtual void UpdateFeaPartPropertyIndex( Choice* property_choice );
    virtual void UpdateCapPropertyIndex( Choice* property_choice );
    virtual void UpdateUnitLabels();

private:

    GroupLayout m_GenLayout;
    GroupLayout* m_CurFeaPartDispGroup;
    GroupLayout m_SkinEditLayout;
    GroupLayout m_SliceEditLayout;
    GroupLayout m_RibEditLayout;
    GroupLayout m_SparEditLayout;
    GroupLayout m_FixPointEditLayout;
    GroupLayout m_FeaSSLineGroup;
    GroupLayout m_FeaSSRecGroup;
    GroupLayout m_FeaSSEllGroup;
    GroupLayout m_FeaSSConGroup;

    //===== FeaPart Tab Items =====//

    StringInput m_FeaPartNameInput;
    TriggerButton m_EditFeaPartButton;
    TriggerButton m_AddFeaPartButton;
    TriggerButton m_DelFeaPartButton;
    ToggleButton m_ShellToggle;
    ToggleButton m_CapToggle;
    ToggleButton m_ShellCapToggle;
    ToggleRadioGroup m_ShellCapToggleGroup;

    //===== Skin =====//
    ToggleButton m_RemoveSkinTrisToggle;
    Choice m_SkinPropertyChoice;

    //===== Slice =====//
    Choice m_SlicePropertyChoice;
    Choice m_SliceOrientationChoice;
    Choice m_SlicePosTypeChoice;
    SliderAdjRangeInput m_SliceCenterLocSlider;
    TriggerButton m_SlicePosUnit;
    ToggleButton m_SliceXAxisToggle;
    ToggleButton m_SliceYAxisToggle;
    ToggleButton m_SliceZAxisToggle;
    ToggleRadioGroup m_SliceRotAxisToggleGroup;
    SliderAdjRangeInput m_SliceXRotSlider;
    SliderAdjRangeInput m_SliceYRotSlider;
    SliderAdjRangeInput m_SliceZRotSlider;
    Choice m_SliceCapPropertyChoice;

    //===== Rib =====//
    Choice m_RibPropertyChoice;
    Choice m_RibPerpendicularEdgeChoice;
    Choice m_RibPosTypeChoice;
    SliderAdjRangeInput m_RibPosSlider;
    TriggerButton m_RibPosUnit;
    SliderAdjRangeInput m_RibThetaSlider;
    Choice m_RibCapPropertyChoice;

    //===== Spar =====//
    Choice m_SparPropertyChoice;
    Choice m_SparPosTypeChoice;
    SliderAdjRangeInput m_SparPosSlider;
    TriggerButton m_SparPosUnit;
    ToggleButton m_SparSectionLimitToggle;
    IndexSelector m_SparSectIndexSelector;
    SliderAdjRangeInput m_SparThetaSlider;
    Choice m_SparCapPropertyChoice;

    //===== Fix Point =====//
    Choice m_FixPointParentSurfChoice;
    SliderAdjRangeInput m_FixPointULocSlider;
    SliderAdjRangeInput m_FixPointWLocSlider;
    ToggleButton m_FixPointMassToggle;
    SliderAdjRangeInput m_FixPointMassSlider;
    TriggerButton m_FixPointMassUnit;

    //===== SubSurfaces =====//

    // SS_Line
    Choice m_FeaSSLinePropertyChoice;
    SliderInput m_FeaSSLineConstSlider; // Either Constant U or W
    ToggleButton m_FeaSSLineConstUButton;
    ToggleButton m_FeaSSLineConstWButton;
    ToggleRadioGroup m_FeaSSLineConstToggleGroup;
    ToggleButton m_FeaSSLineGreaterToggle;
    ToggleButton m_FeaSSLineLessToggle;
    ToggleButton m_FeaSSLineNoneToggle;
    ToggleRadioGroup m_FeaSSLineTestToggleGroup;
    Choice m_FeaSSLineCapPropertyChoice;

    // SS_Rectangle
    Choice m_FeaSSRecPropertyChoice;
    SliderAdjRangeInput m_FeaSSRecCentUSlider;
    SliderAdjRangeInput m_FeaSSRecCentWSlider;
    SliderAdjRangeInput m_FeaSSRecULenSlider;
    SliderAdjRangeInput m_FeaSSRecWLenSlider;
    SliderAdjRangeInput m_FeaSSRecThetaSlider;
    ToggleButton m_FeaSSRecInsideButton;
    ToggleButton m_FeaSSRecOutsideButton;
    ToggleButton m_FeaSSRecNoneButton;
    ToggleRadioGroup m_FeaSSRecTestToggleGroup;
    Choice m_FeaSSRecCapPropertyChoice;

    // SS_Ellipse
    Choice m_FeaSSEllPropertyChoice;
    SliderAdjRangeInput m_FeaSSEllCentUSlider;
    SliderAdjRangeInput m_FeaSSEllCentWSlider;
    SliderAdjRangeInput m_FeaSSEllULenSlider;
    SliderAdjRangeInput m_FeaSSEllWLenSlider;
    SliderAdjRangeInput m_FeaSSEllTessSlider;
    SliderAdjRangeInput m_FeaSSEllThetaSlider;
    ToggleButton m_FeaSSEllInsideButton;
    ToggleButton m_FeaSSEllOutsideButton;
    ToggleButton m_FeaSSEllNoneButton;
    ToggleRadioGroup m_FeaSSEllTestToggleGroup;
    Choice m_FeaSSEllCapPropertyChoice;

    // SS_Control
    Choice m_FeaSSConPropertyChoice;
    SliderAdjRangeInput m_FeaSSConUSSlider;
    SliderAdjRangeInput m_FeaSSConUESlider;
    SliderAdjRangeInput m_FeaSSConSFracSlider;
    SliderAdjRangeInput m_FeaSSConSLenSlider;
    SliderAdjRangeInput m_FeaSSConEFracSlider;
    SliderAdjRangeInput m_FeaSSConELenSlider;
    ToggleButton m_FeaSSConSAbsButton;
    ToggleButton m_FeaSSConSRelButton;
    ToggleRadioGroup m_FeaSSConSAbsRelToggleGroup;
    ToggleButton m_FeaSSConSEConstButton;
    ToggleButton m_FeaSSConLEFlagButton;
    ToggleButton m_FeaSSConInsideButton;
    ToggleButton m_FeaSSConOutsideButton;
    ToggleButton m_FeaSSConNoneButton;
    ToggleRadioGroup m_FeaSSConTestToggleGroup;
    Choice m_FeaSSConSurfTypeChoice;
    Choice m_FeaSSConCapPropertyChoice;

    vector < string > m_FixPointParentIDVec;
    vector < string > m_PerpendicularEdgeVec;

};

#endif // !defined(FEAPARTEDITSCREEN__INCLUDED_)