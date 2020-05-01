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
    virtual void Show();
    virtual void GuiDeviceCallBack( GuiDevice* device );
    virtual void CloseCallBack( Fl_Widget *w );

    virtual void FeaPartDispGroup( GroupLayout* group );
    virtual void UpdateFeaPropertyChoice();
    virtual void UpdatePerpendicularRibChoice();
    virtual void UpdateFixPointParentChoice();
    virtual void SetFeaPartPropertyIndex( Choice* property_choice );
    virtual void SetCapPropertyIndex( Choice* property_choice );
    virtual void UpdateUnitLabels();

private:

    GroupLayout m_GenLayout;
    GroupLayout* m_CurFeaPartDispGroup;
    GroupLayout m_SkinEditLayout;
    GroupLayout m_SliceEditLayout;
    GroupLayout m_RibEditLayout;
    GroupLayout m_SparEditLayout;
    GroupLayout m_FixPointEditLayout;
    GroupLayout m_DomeEditLayout;
    GroupLayout m_RibArrayEditLayout;
    GroupLayout m_SliceArrayEditLayout;
    GroupLayout m_FeaSSLineGroup;
    GroupLayout m_FeaSSRecGroup;
    GroupLayout m_FeaSSEllGroup;
    GroupLayout m_FeaSSConGroup;
    GroupLayout m_FeaSSLineArrayGroup;

    //===== General =====//
    StringInput m_FeaPartNameInput;

    //===== Skin =====//
    ToggleButton m_RemoveSkinTrisToggle;
    Choice m_SkinPropertyChoice;

    //===== Slice =====//
    Choice m_SlicePropertyChoice;
    Choice m_SliceOrientationChoice;
    ToggleButton m_SlicePosRelToggle;
    ToggleButton m_SlicePosAbsToggle;
    ToggleRadioGroup m_SlicePosTypeToggleGroup;
    SliderAdjRange2Input m_SliceCenterLocSlider;
    TriggerButton m_SlicePosUnit;
    ToggleButton m_SliceXAxisToggle;
    ToggleButton m_SliceYAxisToggle;
    ToggleButton m_SliceZAxisToggle;
    ToggleRadioGroup m_SliceRotAxisToggleGroup;
    SliderAdjRangeInput m_SliceXRotSlider;
    SliderAdjRangeInput m_SliceYRotSlider;
    SliderAdjRangeInput m_SliceZRotSlider;
    Choice m_SliceCapPropertyChoice;
    ToggleButton m_SliceShellToggle;
    ToggleButton m_SliceCapToggle;
    ToggleButton m_SliceShellCapToggle;
    ToggleRadioGroup m_SliceShellCapToggleGroup;

    //===== Rib =====//
    Choice m_RibPropertyChoice;
    ToggleButton m_RibNoNormalToggle;
    ToggleButton m_RibLENormalToggle;
    ToggleButton m_RibTENormalToggle;
    ToggleButton m_RibSparNormalToggle;
    ToggleRadioGroup m_RibNormalTypeGroup;
    Choice m_RibPerpendicularSparChoice;
    ToggleButton m_RibSectionLimitToggle;
    IndexSelector m_RibStartSectIndexSelector;
    IndexSelector m_RibEndSectIndexSelector;
    ToggleButton m_RibPosRelToggle;
    ToggleButton m_RibPosAbsToggle;
    ToggleRadioGroup m_RibPosTypeToggleGroup;
    SliderAdjRange2Input m_RibPosSlider;
    TriggerButton m_RibPosUnit;
    SliderAdjRangeInput m_RibThetaSlider;
    ToggleButton m_RibTrimToBBoxToggle;
    Choice m_RibCapPropertyChoice;
    ToggleButton m_RibShellToggle;
    ToggleButton m_RibCapToggle;
    ToggleButton m_RibShellCapToggle;
    ToggleRadioGroup m_RibShellCapToggleGroup;

    vector < string > m_PerpendicularEdgeVec;

    //===== Spar =====//
    Choice m_SparPropertyChoice;
    ToggleButton m_SparPosRelToggle;
    ToggleButton m_SparPosAbsToggle;
    ToggleRadioGroup m_SparPosTypeToggleGroup;
    SliderAdjRange2Input m_SparPosSlider;
    TriggerButton m_SparPosUnit;
    ToggleButton m_SparSectionLimitToggle;
    IndexSelector m_SparStartSectIndexSelector;
    IndexSelector m_SparEndSectIndexSelector;
    SliderAdjRangeInput m_SparThetaSlider;
    ToggleButton m_SparTrimToBBoxToggle;
    Choice m_SparCapPropertyChoice;
    ToggleButton m_SparShellToggle;
    ToggleButton m_SparCapToggle;
    ToggleButton m_SparShellCapToggle;
    ToggleRadioGroup m_SparShellCapToggleGroup;

    //===== Fix Point =====//
    Choice m_FixPointParentSurfChoice;
    SliderAdjRangeInput m_FixPointULocSlider;
    SliderAdjRangeInput m_FixPointWLocSlider;
    ToggleButton m_FixPointMassToggle;
    SliderAdjRangeInput m_FixPointMassSlider;
    TriggerButton m_FixPointMassUnit;

    vector < string > m_FixPointParentIDVec;

    //===== Dome =====//
    Choice m_DomePropertyChoice;
    ToggleButton m_DomeFlipDirButton;
    ToggleButton m_DomeSpineAttachButton;
    SliderAdjRangeInput m_DomeUSpineSlider;
    SliderAdjRangeInput m_DomeARadSlider;
    SliderAdjRangeInput m_DomeBRadSlider;
    SliderAdjRangeInput m_DomeCRadSlider;
    TriggerButton m_DomeARadUnit;
    TriggerButton m_DomeBRadUnit;
    TriggerButton m_DomeCRadUnit;
    SliderAdjRangeInput m_DomeXSlider;
    SliderAdjRangeInput m_DomeYSlider;
    SliderAdjRangeInput m_DomeZSlider;
    TriggerButton m_DomeXUnit;
    TriggerButton m_DomeYUnit;
    TriggerButton m_DomeZUnit;
    SliderAdjRangeInput m_DomeXRotSlider;
    SliderAdjRangeInput m_DomeYRotSlider;
    SliderAdjRangeInput m_DomeZRotSlider;
    Choice m_DomeCapPropertyChoice;
    ToggleButton m_DomeShellToggle;
    ToggleButton m_DomeCapToggle;
    ToggleButton m_DomeShellCapToggle;
    ToggleRadioGroup m_DomeShellCapToggleGroup;

    //===== RibArray =====//
    Choice m_RibArrayPropertyChoice;
    ToggleButton m_RibArrayNoNormalToggle;
    ToggleButton m_RibArrayLENormalToggle;
    ToggleButton m_RibArrayTENormalToggle;
    ToggleButton m_RibArraySparNormalToggle;
    ToggleRadioGroup m_RibArrayNormalTypeGroup;
    Choice m_RibArrayPerpendicularSparChoice;
    ToggleButton m_RibArraySectionLimitToggle;
    IndexSelector m_RibArrayStartSectIndexSelector;
    IndexSelector m_RibArrayEndSectIndexSelector;
    ToggleButton m_RibArrayPosRelToggle;
    ToggleButton m_RibArrayPosAbsToggle;
    ToggleRadioGroup m_RibArrayPosTypeToggleGroup;
    ToggleButton m_RibArrayPosDirToggle;
    ToggleButton m_RibArrayNegDirToggle;
    ToggleRadioGroup m_RibArrayPosNegDirToggleGroup;
    SliderAdjRange2Input m_RibArrayStartLocSlider;
    TriggerButton m_RibArrayStartLocUnit;
    SliderAdjRange2Input m_RibArrayEndLocSlider;
    TriggerButton m_RibArrayEndLocUnit;
    SliderAdjRange2Input m_RibArraySpacingSlider;
    TriggerButton m_RibArrayPosUnit;
    SliderAdjRangeInput m_RibArrayThetaSlider;
    ToggleButton m_RibArrayTrimToBBoxToggle;
    Choice m_RibArrayCapPropertyChoice;
    TriggerButton m_IndividualizeRibArrayButton;
    ToggleButton m_RibArrayShellToggle;
    ToggleButton m_RibArrayCapToggle;
    ToggleButton m_RibArrayShellCapToggle;
    ToggleRadioGroup m_RibArrayShellCapToggleGroup;

    //===== SliceArray =====//
    Choice m_SliceArrayOrientationChoice;
    ToggleButton m_SliceArrayPosRelToggle;
    ToggleButton m_SliceArrayPosAbsToggle;
    ToggleRadioGroup m_SliceArrayPosTypeToggleGroup;
    ToggleButton m_SliceArrayPosDirToggle;
    ToggleButton m_SliceArrayNegDirToggle;
    ToggleRadioGroup m_SliceArrayPosNegDirToggleGroup;
    SliderAdjRange2Input m_SliceArrayStartLocSlider;
    TriggerButton m_SliceArrayStartLocUnit;
    SliderAdjRange2Input m_SliceArrayEndLocSlider;
    TriggerButton m_SliceArrayEndLocUnit;
    SliderAdjRange2Input m_SliceArraySpacingSlider;
    TriggerButton m_SliceArrayPosUnit;
    ToggleButton m_SliceArrayXAxisToggle;
    ToggleButton m_SliceArrayYAxisToggle;
    ToggleButton m_SliceArrayZAxisToggle;
    ToggleRadioGroup m_SliceArrayRotAxisToggleGroup;
    SliderAdjRangeInput m_SliceArrayXRotSlider;
    SliderAdjRangeInput m_SliceArrayYRotSlider;
    SliderAdjRangeInput m_SliceArrayZRotSlider;
    ToggleButton m_SliceArrayShellToggle;
    ToggleButton m_SliceArrayCapToggle;
    ToggleButton m_SliceArrayShellCapToggle;
    ToggleRadioGroup m_SliceArrayShellCapToggleGroup;
    Choice m_SliceArrayPropertyChoice;
    Choice m_SliceArrayCapPropertyChoice;
    TriggerButton m_IndividualizeSliceArrayButton;

    //===== SubSurfaces =====//

    // SS_Line
    Choice m_FeaSSLinePropertyChoice;
    SliderAdjRangeInput m_FeaSSLineConstSlider; // Either Constant U or W
    ToggleButton m_FeaSSLineConstUButton;
    ToggleButton m_FeaSSLineConstWButton;
    ToggleRadioGroup m_FeaSSLineConstToggleGroup;
    ToggleButton m_FeaSSLineGreaterToggle;
    ToggleButton m_FeaSSLineLessToggle;
    ToggleButton m_FeaSSLineNoneToggle;
    ToggleRadioGroup m_FeaSSLineTestToggleGroup;
    Choice m_FeaSSLineCapPropertyChoice;
    ToggleButton m_FeaSSLineShellToggle;
    ToggleButton m_FeaSSLineCapToggle;
    ToggleButton m_FeaSSLineShellCapToggle;
    ToggleRadioGroup m_FeaSSLineShellCapToggleGroup;

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
    ToggleButton m_FeaSSRecShellToggle;
    ToggleButton m_FeaSSRecCapToggle;
    ToggleButton m_FeaSSRecShellCapToggle;
    ToggleRadioGroup m_FeaSSRecShellCapToggleGroup;

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
    ToggleButton m_FeaSSEllShellToggle;
    ToggleButton m_FeaSSEllCapToggle;
    ToggleButton m_FeaSSEllShellCapToggle;
    ToggleRadioGroup m_FeaSSEllShellCapToggleGroup;

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
    ToggleButton m_FeaSSConShellToggle;
    ToggleButton m_FeaSSConCapToggle;
    ToggleButton m_FeaSSConShellCapToggle;
    ToggleRadioGroup m_FeaSSConShellCapToggleGroup;
    SliderAdjRangeInput m_FeaSSConSAngleSlider;
    SliderAdjRangeInput m_FeaSSConEAngleSlider;
    ToggleButton m_FeaSSConSAngleButton;
    ToggleButton m_FeaSSConEAngleButton;
    ToggleButton m_FeaSSConSameAngleButton;
    SliderInput m_FeaSSConTessSlider;

    //===== SSLineArray =====//
    SliderAdjRangeInput m_FeaSSLineArraySpacingSlider;
    SliderAdjRangeInput m_FeaSSLineArrayStartLocSlider;
    SliderAdjRangeInput m_FeaSSLineArrayEndLocSlider;
    ToggleButton m_FeaSSLineArrayConstUButton;
    ToggleButton m_FeaSSLineArrayConstWButton;
    ToggleRadioGroup m_FeaSSLineArrayConstToggleGroup;
    Choice m_FeaSSLineArrayCapPropertyChoice;
    ToggleButton m_FeaSSLineArrayNegDirToggle;
    ToggleButton m_FeaSSLineArrayPosDirToggle;
    ToggleRadioGroup m_FeaSSLineArrayPosNegDirToggleGroup;
    TriggerButton m_IndividualizeSSLineArrayButton;
};

#endif // !defined(FEAPARTEDITSCREEN__INCLUDED_)