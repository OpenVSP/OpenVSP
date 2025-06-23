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
    virtual void SubSurfXSCDisplayGroup( GroupLayout* group );
    virtual void RebuildSSCSTGroup( CSTAirfoil* cst_xs );

    virtual void UpdateFeaPropertyChoice();
    virtual void UpdatePerpendicularRibChoice();
    virtual void UpdateFixPointParentChoice();
    virtual void UpdateTrimPartChoice();
    virtual void UpdateUnitLabels();
    virtual void UpdatePolySparPointBrowser( FeaPolySpar* polyspar );

private:

    GroupLayout m_MainLayout;
    GroupLayout m_GenLayout;
    GroupLayout* m_CurFeaPartDispGroup;
    GroupLayout m_SkinEditLayout;
    GroupLayout m_SliceEditLayout;
    GroupLayout m_RibEditLayout;
    GroupLayout m_SparEditLayout;
    GroupLayout m_PolySparEditLayout;
    GroupLayout m_MovePointLayout;
    GroupLayout m_PointBrowserLayout;
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
    GroupLayout m_FeaSSXSCGroup;

    GroupLayout* m_FeaSSXSCCurrDisplayGroup;

    //===== General =====//
    StringInput m_FeaPartNameInput;
    Choice m_ShellPropertyChoice;
    Choice m_OrientationChoice;
    Choice m_CapPropertyChoice;
    ToggleButton m_CreateBeamElementsToggle;
    vector < string > m_FeaPropertyIDVec;

    ToggleButton m_KeepShellElementsToggle;
    ToggleButton m_DelShellElementsToggle;
    ToggleRadioGroup m_KeepDelShellElementsToggleGroup;

    //===== Skin =====//
    ToggleButton m_RemoveSkinToggle;
    ToggleButton m_RemoveRootCapToggle;
    ToggleButton m_RemoveTipCapToggle;

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

    //===== PolySpar =====//
    ColResizeBrowser* m_PolySparPointBrowser;
    int m_PolySparPointBrowserSelect;

    TriggerButton m_AddPolySparPoint;
    TriggerButton m_InsertPolySparPoint;
    TriggerButton m_DelPolySparPoint;
    TriggerButton m_DelAllPolySparPoints;
    StringInput m_PolySparPtNameInput;

    TriggerButton m_MovePntUpButton;
    TriggerButton m_MovePntDownButton;
    TriggerButton m_MovePntTopButton;
    TriggerButton m_MovePntBotButton;

    ToggleButton m_PolySparPointU01Toggle;
    ToggleButton m_PolySparPointU0NToggle;
    ToggleButton m_PolySparPointEtaToggle;

    ToggleRadioGroup m_PolySparPointTypeToggleGroup;

    SliderAdjRangeInput m_PolySparPointU01Slider;
    SliderAdjRangeInput m_PolySparPointU0NSlider;
    SliderAdjRangeInput m_PolySparPointEtaSlider;
    SliderAdjRangeInput m_PolySparPointXoCSlider;

    //===== Fix Point =====//
    Choice m_FixPointTypeChoice;
    Choice m_FixPointParentSurfChoice;
    GeomPicker m_FixPointOtherGeomPicker;

    SliderAdjRangeInput m_FixPointULocSlider;
    SliderAdjRangeInput m_FixPointWLocSlider;
    SliderAdjRangeInput m_FixPointAbsXSlider;
    SliderAdjRangeInput m_FixPointAbsYSlider;
    SliderAdjRangeInput m_FixPointAbsZSlider;
    SliderAdjRangeInput m_FixPointDeltaXSlider;
    SliderAdjRangeInput m_FixPointDeltaYSlider;
    SliderAdjRangeInput m_FixPointDeltaZSlider;
    SliderAdjRangeInput m_FixPointDeltaUSlider;
    SliderAdjRangeInput m_FixPointDeltaVSlider;
    SliderAdjRangeInput m_FixPointDeltaNSlider;

    ToggleButton m_FixPointMassToggle;
    Input m_FixPointMassInput;
    Choice m_FixPointMassUnitChoice;
    Output m_FixPointMass_FEMOutput;
    TriggerButton m_FixPointMassUnit_FEM;

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


    // SS_XSecCurve
    SliderAdjRangeInput m_FeaSSXSCCentUSlider;
    SliderAdjRangeInput m_FeaSSXSCCentWSlider;

    ToggleButton m_FeaSSXSCInsideButton;
    ToggleButton m_FeaSSXSCOutsideButton;
    ToggleRadioGroup m_FeaSSXSCTestToggleGroup;





    Choice m_FeaSSXSecTypeChoice;
    TriggerButton m_FeaSSXSCShowXSecButton;

    TriggerButton m_FeaSSXSCConvertCEDITButton;
    GroupLayout m_FeaSSXSCConvertCEDITGroup;

    GroupLayout m_FeaSSXSCEditCEDITGroup;
    TriggerButton m_FeaSSXSCEditCEDITButton;

    GroupLayout m_FeaSSXSCPointGroup;

    GroupLayout m_FeaSSXSCCircleGroup;
    SliderAdjRangeInput m_FeaSSXSCDiameterSlider;

    GroupLayout m_FeaSSXSCEllipseGroup;
    SliderAdjRangeInput m_FeaSSXSCEllipseHeightSlider;
    SliderAdjRangeInput m_FeaSSXSCEllipseWidthSlider;

    GroupLayout m_FeaSSXSCAC25773Group;
    Choice m_FeaSSXSCAC25773SeatChoice;

    GroupLayout m_FeaSSXSCSuperGroup;
    SliderAdjRangeInput m_FeaSSXSCSuperHeightSlider;
    SliderAdjRangeInput m_FeaSSXSCSuperWidthSlider;
    SliderAdjRangeInput m_FeaSSXSCSuperMSlider;
    SliderAdjRangeInput m_FeaSSXSCSuperNSlider;
    ToggleButton m_FeaSSXSCSuperToggleSym;
    SliderAdjRangeInput m_FeaSSXSCSuperM_botSlider;
    SliderAdjRangeInput m_FeaSSXSCSuperN_botSlider;
    SliderAdjRangeInput m_FeaSSXSCSuperMaxWidthLocSlider;

    GroupLayout m_FeaSSXSCRoundedRectGroup;
    SliderAdjRangeInput m_FeaSSXSCRRHeightSlider;
    SliderAdjRangeInput m_FeaSSXSCRRWidthSlider;
    ToggleButton m_FeaSSXSCRRRadNoSymToggle;
    ToggleButton m_FeaSSXSCRRRadRLSymToggle;
    ToggleButton m_FeaSSXSCRRRadTBSymToggle;
    ToggleButton m_FeaSSXSCRRRadAllSymToggle;
    ToggleRadioGroup m_FeaSSXSCRRRadSymRadioGroup;
    SliderAdjRangeInput m_FeaSSXSCRRRadiusBRSlider; // Bottom Right
    SliderAdjRangeInput m_FeaSSXSCRRRadiusBLSlider; // Bottom Left
    SliderAdjRangeInput m_FeaSSXSCRRRadiusTLSlider; // Top Left
    SliderAdjRangeInput m_FeaSSXSCRRRadiusTRSlider; // Top Right
    ToggleButton m_FeaSSXSCRRKeyCornerButton;
    SliderAdjRangeInput m_FeaSSXSCRRSkewSlider;
    SliderAdjRangeInput m_FeaSSXSCRRKeystoneSlider;
    SliderAdjRangeInput m_FeaSSXSCRRVSkewSlider;

    GroupLayout m_FeaSSXSCGenGroup;
    SliderAdjRangeInput m_FeaSSXSCGenHeightSlider;
    SliderAdjRangeInput m_FeaSSXSCGenWidthSlider;
    SliderAdjRangeInput m_FeaSSXSCGenMaxWidthLocSlider;
    SliderAdjRangeInput m_FeaSSXSCGenCornerRadSlider;
    SliderAdjRangeInput m_FeaSSXSCGenTopTanAngleSlider;
    SliderAdjRangeInput m_FeaSSXSCGenBotTanAngleSlider;
    SliderAdjRangeInput m_FeaSSXSCGenTopStrSlider;
    SliderAdjRangeInput m_FeaSSXSCGenBotStrSlider;
    SliderAdjRangeInput m_FeaSSXSCGenUpStrSlider;
    SliderAdjRangeInput m_FeaSSXSCGenLowStrSlider;

    GroupLayout m_FeaSSXSCFourSeriesGroup;
    StringOutput m_FeaSSXSCFourNameOutput;
    CheckButton m_FeaSSXSCFourInvertButton;
    SliderAdjRangeInput m_FeaSSXSCFourChordSlider;
    SliderAdjRangeInput m_FeaSSXSCFourThickChordSlider;
    SliderAdjRangeInput m_FeaSSXSCFourCamberSlider;
    SliderAdjRangeInput m_FeaSSXSCFourCLiSlider;
    ToggleButton m_FeaSSXSCFourCamberButton;
    ToggleButton m_FeaSSXSCFourCLiButton;
    ToggleRadioGroup m_FeaSSXSCFourCamberGroup;
    SliderAdjRangeInput m_FeaSSXSCFourCamberLocSlider;
    CheckButton m_FeaSSXSCFourSharpTEButton;
    TriggerButton m_FeaSSXSCFourFitCSTButton;
    Counter m_FeaSSXSCFourDegreeCounter;

    GroupLayout m_FeaSSXSCSixSeriesGroup;
    StringOutput m_FeaSSXSCSixNameOutput;
    CheckButton m_FeaSSXSCSixInvertButton;
    Choice m_FeaSSXSCSixSeriesChoice;
    SliderAdjRangeInput m_FeaSSXSCSixChordSlider;
    SliderAdjRangeInput m_FeaSSXSCSixThickChordSlider;
    SliderAdjRangeInput m_FeaSSXSCSixIdealClSlider;
    SliderAdjRangeInput m_FeaSSXSCSixASlider;
    TriggerButton m_FeaSSXSCSixFitCSTButton;
    Counter m_FeaSSXSCSixDegreeCounter;

    GroupLayout m_FeaSSXSCBiconvexGroup;
    SliderAdjRangeInput m_FeaSSXSCBiconvexChordSlider;
    SliderAdjRangeInput m_FeaSSXSCBiconvexThickChordSlider;

    GroupLayout m_FeaSSXSCWedgeGroup;
    SliderAdjRangeInput m_FeaSSXSCWedgeChordSlider;
    SliderAdjRangeInput m_FeaSSXSCWedgeThickChordSlider;
    SliderAdjRangeInput m_FeaSSXSCWedgeZCamberSlider;
    ToggleButton m_FeaSSXSCWedgeSymmThickButton;
    SliderAdjRangeInput m_FeaSSXSCWedgeThickLocSlider;
    SliderAdjRangeInput m_FeaSSXSCWedgeThickLocLowSlider;
    SliderAdjRangeInput m_FeaSSXSCWedgeFlatUpSlider;
    SliderAdjRangeInput m_FeaSSXSCWedgeFlatLowSlider;
    SliderAdjRangeInput m_FeaSSXSCWedgeUForeUpSlider;
    SliderAdjRangeInput m_FeaSSXSCWedgeUForeLowSlider;
    SliderAdjRangeInput m_FeaSSXSCWedgeDuUpSlider;
    SliderAdjRangeInput m_FeaSSXSCWedgeDuLowSlider;
    CheckButton m_FeaSSXSCWedgeInvertButton;

    GroupLayout m_FeaSSXSCFuseFileGroup;
    TriggerButton m_FeaSSXSCReadFuseFileButton;
    SliderAdjRangeInput m_FeaSSXSCFileHeightSlider;
    SliderAdjRangeInput m_FeaSSXSCFileWidthSlider;

    GroupLayout m_FeaSSXSCAfFileGroup;
    TriggerButton m_FeaSSXSCAfReadFileButton;
    StringOutput m_FeaSSXSCAfFileNameOutput;
    CheckButton m_FeaSSXSCAfFileInvertButton;
    SliderAdjRangeInput m_FeaSSXSCAfFileChordSlider;
    SliderAdjRangeInput m_FeaSSXSCAfFileThickChordSlider;
    Output m_FeaSSXSCAfFileBaseThickChordOutput;
    TriggerButton m_FeaSSXSCAfFileFitCSTButton;
    Counter m_FeaSSXSCAfFileDegreeCounter;

    GroupLayout m_FeaSSXSCCSTAirfoilGroup;

    Output m_FeaSSXSCCSTThickChordOutput;

    SliderAdjRangeInput m_FeaSSXSCCSTChordSlider;

    TriggerButton m_FeaSSXSCUpDemoteButton;
    StringOutput m_FeaSSXSCUpDegreeOutput;
    TriggerButton m_FeaSSXSCUpPromoteButton;

    Fl_Scroll* m_FeaSSXSCCSTUpCoeffScroll;
    GroupLayout m_FeaSSXSCCSTUpCoeffLayout;

    TriggerButton m_FeaSSXSCLowDemoteButton;
    StringOutput m_FeaSSXSCLowDegreeOutput;
    TriggerButton m_FeaSSXSCLowPromoteButton;

    vector < SliderAdjRangeInput > m_FeaSSXSCUpCoeffSliderVec;

    Fl_Scroll* m_FeaSSXSCCSTLowCoeffScroll;
    GroupLayout m_FeaSSXSCCSTLowCoeffLayout;

    vector < SliderAdjRangeInput > m_FeaSSXSCLowCoeffSliderVec;

    CheckButton m_FeaSSXSCCSTInvertButton;
    CheckButton m_FeaSSXSCCSTContLERadButton;

    GroupLayout m_FeaSSXSCVKTGroup;
    SliderAdjRangeInput m_FeaSSXSCVKTChordSlider;
    SliderAdjRangeInput m_FeaSSXSCVKTEpsilonSlider;
    SliderAdjRangeInput m_FeaSSXSCVKTKappaSlider;
    SliderAdjRangeInput m_FeaSSXSCVKTTauSlider;
    CheckButton m_FeaSSXSCVKTInvertButton;
    Output m_FeaSSXSCVKTThickChordOutput;
    TriggerButton m_FeaSSXSCVKTFitCSTButton;
    Counter m_FeaSSXSCVKTDegreeCounter;

    GroupLayout m_FeaSSXSCFourDigitModGroup;
    StringOutput m_FeaSSXSCFourModNameOutput;
    CheckButton m_FeaSSXSCFourModInvertButton;
    SliderAdjRangeInput m_FeaSSXSCFourModChordSlider;
    SliderAdjRangeInput m_FeaSSXSCFourModThickChordSlider;
    SliderAdjRangeInput m_FeaSSXSCFourModCamberSlider;
    SliderAdjRangeInput m_FeaSSXSCFourModCLiSlider;
    ToggleButton m_FeaSSXSCFourModCamberButton;
    ToggleButton m_FeaSSXSCFourModCLiButton;
    ToggleRadioGroup m_FeaSSXSCFourModCamberGroup;
    SliderAdjRangeInput m_FeaSSXSCFourModCamberLocSlider;
    SliderAdjRangeInput m_FeaSSXSCFourModThicknessLocSlider;
    SliderAdjRangeInput m_FeaSSXSCFourModLERadIndexSlider;
    CheckButton m_FeaSSXSCFourModSharpTEButton;
    TriggerButton m_FeaSSXSCFourModFitCSTButton;
    Counter m_FeaSSXSCFourModDegreeCounter;

    GroupLayout m_FeaSSXSCFiveDigitGroup;
    StringOutput m_FeaSSXSCFiveNameOutput;
    CheckButton m_FeaSSXSCFiveInvertButton;
    SliderAdjRangeInput m_FeaSSXSCFiveChordSlider;
    SliderAdjRangeInput m_FeaSSXSCFiveThickChordSlider;
    SliderAdjRangeInput m_FeaSSXSCFiveCLiSlider;
    SliderAdjRangeInput m_FeaSSXSCFiveCamberLocSlider;
    CheckButton m_FeaSSXSCFiveSharpTEButton;
    TriggerButton m_FeaSSXSCFiveFitCSTButton;
    Counter m_FeaSSXSCFiveDegreeCounter;

    GroupLayout m_FeaSSXSCFiveDigitModGroup;
    StringOutput m_FeaSSXSCFiveModNameOutput;
    CheckButton m_FeaSSXSCFiveModInvertButton;
    SliderAdjRangeInput m_FeaSSXSCFiveModChordSlider;
    SliderAdjRangeInput m_FeaSSXSCFiveModThickChordSlider;
    SliderAdjRangeInput m_FeaSSXSCFiveModCLiSlider;
    SliderAdjRangeInput m_FeaSSXSCFiveModCamberLocSlider;
    SliderAdjRangeInput m_FeaSSXSCFiveModThicknessLocSlider;
    SliderAdjRangeInput m_FeaSSXSCFiveModLERadIndexSlider;
    CheckButton m_FeaSSXSCFiveModSharpTEButton;
    TriggerButton m_FeaSSXSCFiveModFitCSTButton;
    Counter m_FeaSSXSCFiveModDegreeCounter;

    GroupLayout m_FeaSSXSCOneSixSeriesGroup;
    StringOutput m_FeaSSXSCOneSixSeriesNameOutput;
    CheckButton m_FeaSSXSCOneSixSeriesInvertButton;
    SliderAdjRangeInput m_FeaSSXSCOneSixSeriesChordSlider;
    SliderAdjRangeInput m_FeaSSXSCOneSixSeriesThickChordSlider;
    SliderAdjRangeInput m_FeaSSXSCOneSixSeriesCLiSlider;
    SliderAdjRangeInput m_FeaSSXSCOneSixSeriesCamberLocSlider;
    SliderAdjRangeInput m_FeaSSXSCOneSixSeriesThicknessLocSlider;
    SliderAdjRangeInput m_FeaSSXSCOneSixSeriesLERadIndexSlider;
    CheckButton m_FeaSSXSCOneSixSeriesSharpTEButton;
    TriggerButton m_FeaSSXSCOneSixSeriesFitCSTButton;
    Counter m_FeaSSXSCOneSixSeriesDegreeCounter;


};

#endif // !defined(FEAPARTEDITSCREEN__INCLUDED_)