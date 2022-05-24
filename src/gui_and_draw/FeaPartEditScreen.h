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

    virtual void CallBack( Fl_Widget* w );
    static void staticScreenCB( Fl_Widget *w, void* data )
    {
        ( ( FeaPartEditScreen* )data )->CallBack( w );
    }

    virtual void FeaPartDispGroup( GroupLayout* group );
    virtual void UpdateFeaPropertyChoice();
    virtual void UpdatePerpendicularRibChoice();
    virtual void UpdateFixPointParentChoice();
    virtual void UpdateTrimPartChoice();
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
    GroupLayout m_TrimEditLayout;
    GroupLayout m_FeaSSLineGroup;
    GroupLayout m_FeaSSRecGroup;
    GroupLayout m_FeaSSEllGroup;
    GroupLayout m_FeaSSConGroup;
    GroupLayout m_FeaSSLineArrayGroup;
    GroupLayout m_FeaSSFLineGroup;

    //===== General =====//
    StringInput m_FeaPartNameInput;
    Choice m_ShellPropertyChoice;
    Choice m_OrientationChoice;
    Choice m_CapPropertyChoice;
    ToggleButton m_ShellToggle;
    ToggleButton m_CapToggle;
    ToggleButton m_ShellCapToggle;
    ToggleRadioGroup m_ShellCapToggleGroup;

    //===== Skin =====//
    ToggleButton m_RemoveSkinToggle;

    //===== Slice =====//
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

    //===== Rib =====//
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
    ToggleButton m_RibRotateDihedralToggle;

    vector < string > m_PerpendicularEdgeVec;

    //===== Spar =====//
    ToggleButton m_SparPosRelToggle;
    ToggleButton m_SparPosAbsToggle;
    ToggleRadioGroup m_SparPosTypeToggleGroup;
    SliderAdjRange2Input m_SparPosSlider;
    ToggleButton m_SparSetPerChordToggle;
    SliderAdjRangeInput m_SparRootChordSlider;
    SliderAdjRangeInput m_SparTipChordSlider;
    TriggerButton m_SparPosUnit;
    ToggleButton m_SparSectionLimitToggle;
    IndexSelector m_SparStartSectIndexSelector;
    IndexSelector m_SparEndSectIndexSelector;
    SliderAdjRangeInput m_SparThetaSlider;
    ToggleButton m_SparTrimToBBoxToggle;

    //===== Fix Point =====//
    Choice m_FixPointParentSurfChoice;
    SliderAdjRangeInput m_FixPointULocSlider;
    SliderAdjRangeInput m_FixPointWLocSlider;
    ToggleButton m_FixPointMassToggle;
    SliderAdjRangeInput m_FixPointMassSlider;
    TriggerButton m_FixPointMassUnit;

    vector < string > m_FixPointParentIDVec;

    //===== Dome =====//
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

    //===== RibArray =====//
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
    ToggleButton m_RibArrayRotateDihedralToggle;
    TriggerButton m_IndividualizeRibArrayButton;

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
    TriggerButton m_IndividualizeSliceArrayButton;

    //===== Trim =====//

    ColResizeBrowser* m_TrimPartBrowser;

    vector < string > m_TrimPartChoiceIDVec;
    Choice m_TrimPartChoice;
    ToggleButton m_FlipTrimDirButton;
    TriggerButton m_AddTrimPartButton;
    TriggerButton m_DeleteTrimPartButton;

    int m_ActiveTrimPartIndex;
    int m_SelectedTrimPartChoice;

    //===== SubSurfaces =====//

    // SS_Line
    SliderAdjRangeInput m_FeaSSLineConstSlider; // Either Constant U or W
    ToggleButton m_FeaSSLineConstUButton;
    ToggleButton m_FeaSSLineConstWButton;
    ToggleRadioGroup m_FeaSSLineConstToggleGroup;
    ToggleButton m_FeaSSLineGreaterToggle;
    ToggleButton m_FeaSSLineLessToggle;
    ToggleButton m_FeaSSLineNoneToggle;
    ToggleRadioGroup m_FeaSSLineTestToggleGroup;

    // SS_Rectangle
    SliderAdjRangeInput m_FeaSSRecCentUSlider;
    SliderAdjRangeInput m_FeaSSRecCentWSlider;
    SliderAdjRangeInput m_FeaSSRecULenSlider;
    SliderAdjRangeInput m_FeaSSRecWLenSlider;
    SliderAdjRangeInput m_FeaSSRecThetaSlider;
    ToggleButton m_FeaSSRecInsideButton;
    ToggleButton m_FeaSSRecOutsideButton;
    ToggleButton m_FeaSSRecNoneButton;
    ToggleRadioGroup m_FeaSSRecTestToggleGroup;

    // SS_Ellipse
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

    // SS_Control
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
    ToggleButton m_FeaSSLineArrayNegDirToggle;
    ToggleButton m_FeaSSLineArrayPosDirToggle;
    ToggleRadioGroup m_FeaSSLineArrayPosNegDirToggleGroup;
    TriggerButton m_IndividualizeSSLineArrayButton;

    //===== SSFiniteLine =====//

    SliderAdjRangeInput m_FeaSSFLineUStartSlider;
    SliderAdjRangeInput m_FeaSSFLineUEndSlider;
    SliderAdjRangeInput m_FeaSSFLineWStartSlider;
    SliderAdjRangeInput m_FeaSSFLineWEndSlider;


};

#endif // !defined(FEAPARTEDITSCREEN__INCLUDED_)