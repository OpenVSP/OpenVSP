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
    void DisplayXSCGroup( GroupLayout* group );

    virtual void Show();
    virtual bool Update();

    virtual void UpdateGroundPlaneChoices();

    virtual void GetCollIDs( vector < string > &collIDVec );

    virtual void CallBack( Fl_Widget *w );
    virtual void GuiDeviceCallBack( GuiDevice* device );

    virtual void RebuildCSTGroup( CSTAirfoil* cst_xs );

protected:

    GroupLayout m_DesignLayout;

    GroupLayout m_RotorTipPathLayput;
    GroupLayout m_RotorBurstLayout;
    GroupLayout m_3ptGroundPlaneLayout;
    GroupLayout m_2ptGroundPlaneLayout;
    GroupLayout m_1ptGroundPlaneLayout;
    GroupLayout m_3ptCCELayout;
    GroupLayout m_SuperConeXSecLayout;

    Choice m_AuxiliaryGeomModeChoice;

    StringOutput m_AuxiliaryGeomModeNotesOutput;

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

    // Super Cone
    AttributeEditor m_XsecAttributeEditor;

    ToggleButton m_SCWorldAlignedToggleButton;
    ToggleButton m_SCObjectAlignedToggleButton;
    ToggleRadioGroup m_SCAxisAlignedRadioGroup;

    Choice m_XSecTypeChoice;
    TriggerButton m_ShowXSecButton;

    TriggerButton m_ConvertCEDITButton;
    GroupLayout m_ConvertCEDITGroup;

    GroupLayout m_EditCEDITGroup;
    TriggerButton m_EditCEDITButton;

    GroupLayout m_PointGroup;

    GroupLayout m_CircleGroup;
    SliderAdjRangeInput m_DiameterSlider;

    GroupLayout m_EllipseGroup;
    SliderAdjRangeInput m_EllipseHeightSlider;
    SliderAdjRangeInput m_EllipseWidthSlider;

    GroupLayout m_AC25773Group;
    Choice m_AC25773SeatChoice;

    GroupLayout m_SuperGroup;
    SliderAdjRangeInput m_SuperHeightSlider;
    SliderAdjRangeInput m_SuperWidthSlider;
    SliderAdjRangeInput m_SuperMSlider;
    SliderAdjRangeInput m_SuperNSlider;
    ToggleButton m_SuperToggleSym;
    SliderAdjRangeInput m_SuperM_botSlider;
    SliderAdjRangeInput m_SuperN_botSlider;
    SliderAdjRangeInput m_SuperMaxWidthLocSlider;

    GroupLayout m_RoundedRectGroup;
    SliderAdjRangeInput m_RRHeightSlider;
    SliderAdjRangeInput m_RRWidthSlider;
    ToggleButton m_RRRadNoSymToggle;
    ToggleButton m_RRRadRLSymToggle;
    ToggleButton m_RRRadTBSymToggle;
    ToggleButton m_RRRadAllSymToggle;
    ToggleRadioGroup m_RRRadSymRadioGroup;
    SliderAdjRangeInput m_RRRadiusBRSlider; // Bottom Right
    SliderAdjRangeInput m_RRRadiusBLSlider; // Bottom Left
    SliderAdjRangeInput m_RRRadiusTLSlider; // Top Left
    SliderAdjRangeInput m_RRRadiusTRSlider; // Top Right
    ToggleButton m_RRKeyCornerButton;
    SliderAdjRangeInput m_RRSkewSlider;
    SliderAdjRangeInput m_RRKeystoneSlider;
    SliderAdjRangeInput m_RRVSkewSlider;

    GroupLayout m_GenGroup;
    SliderAdjRangeInput m_GenHeightSlider;
    SliderAdjRangeInput m_GenWidthSlider;
    SliderAdjRangeInput m_GenMaxWidthLocSlider;
    SliderAdjRangeInput m_GenCornerRadSlider;
    SliderAdjRangeInput m_GenTopTanAngleSlider;
    SliderAdjRangeInput m_GenBotTanAngleSlider;
    SliderAdjRangeInput m_GenTopStrSlider;
    SliderAdjRangeInput m_GenBotStrSlider;
    SliderAdjRangeInput m_GenUpStrSlider;
    SliderAdjRangeInput m_GenLowStrSlider;

    GroupLayout m_FourSeriesGroup;
    StringOutput m_FourNameOutput;
    CheckButton m_FourInvertButton;
    SliderAdjRangeInput m_FourChordSlider;
    SliderAdjRangeInput m_FourThickChordSlider;
    SliderAdjRangeInput m_FourCamberSlider;
    SliderAdjRangeInput m_FourCLiSlider;
    ToggleButton m_FourCamberButton;
    ToggleButton m_FourCLiButton;
    ToggleRadioGroup m_FourCamberGroup;
    SliderAdjRangeInput m_FourCamberLocSlider;
    CheckButton m_FourSharpTEButton;
    TriggerButton m_FourFitCSTButton;
    Counter m_FourDegreeCounter;

    GroupLayout m_SixSeriesGroup;
    StringOutput m_SixNameOutput;
    CheckButton m_SixInvertButton;
    Choice m_SixSeriesChoice;
    SliderAdjRangeInput m_SixChordSlider;
    SliderAdjRangeInput m_SixThickChordSlider;
    SliderAdjRangeInput m_SixIdealClSlider;
    SliderAdjRangeInput m_SixASlider;
    TriggerButton m_SixFitCSTButton;
    Counter m_SixDegreeCounter;

    GroupLayout m_BiconvexGroup;
    SliderAdjRangeInput m_BiconvexChordSlider;
    SliderAdjRangeInput m_BiconvexThickChordSlider;

    GroupLayout m_WedgeGroup;
    SliderAdjRangeInput m_WedgeChordSlider;
    SliderAdjRangeInput m_WedgeThickChordSlider;
    SliderAdjRangeInput m_WedgeZCamberSlider;
    ToggleButton m_WedgeSymmThickButton;
    SliderAdjRangeInput m_WedgeThickLocSlider;
    SliderAdjRangeInput m_WedgeThickLocLowSlider;
    SliderAdjRangeInput m_WedgeFlatUpSlider;
    SliderAdjRangeInput m_WedgeFlatLowSlider;
    SliderAdjRangeInput m_WedgeUForeUpSlider;
    SliderAdjRangeInput m_WedgeUForeLowSlider;
    SliderAdjRangeInput m_WedgeDuUpSlider;
    SliderAdjRangeInput m_WedgeDuLowSlider;
    CheckButton m_WedgeInvertButton;

    GroupLayout m_FuseFileGroup;
    TriggerButton m_ReadFuseFileButton;
    SliderAdjRangeInput m_FileHeightSlider;
    SliderAdjRangeInput m_FileWidthSlider;

    GroupLayout m_AfFileGroup;
    TriggerButton m_AfReadFileButton;
    StringOutput m_AfFileNameOutput;
    CheckButton m_AfFileInvertButton;
    SliderAdjRangeInput m_AfFileChordSlider;
    SliderAdjRangeInput m_AfFileThickChordSlider;
    Output m_AfFileBaseThickChordOutput;
    TriggerButton m_AfFileFitCSTButton;
    Counter m_AfFileDegreeCounter;

    GroupLayout m_CSTAirfoilGroup;

    Output m_CSTThickChordOutput;

    SliderAdjRangeInput m_CSTChordSlider;

    TriggerButton m_UpDemoteButton;
    StringOutput m_UpDegreeOutput;
    TriggerButton m_UpPromoteButton;

    Fl_Scroll* m_CSTUpCoeffScroll;
    GroupLayout m_CSTUpCoeffLayout;

    TriggerButton m_LowDemoteButton;
    StringOutput m_LowDegreeOutput;
    TriggerButton m_LowPromoteButton;

    vector < SliderAdjRangeInput > m_UpCoeffSliderVec;

    Fl_Scroll* m_CSTLowCoeffScroll;
    GroupLayout m_CSTLowCoeffLayout;

    vector < SliderAdjRangeInput > m_LowCoeffSliderVec;

    CheckButton m_CSTInvertButton;
    CheckButton m_CSTContLERadButton;

    GroupLayout m_VKTGroup;
    SliderAdjRangeInput m_VKTChordSlider;
    SliderAdjRangeInput m_VKTEpsilonSlider;
    SliderAdjRangeInput m_VKTKappaSlider;
    SliderAdjRangeInput m_VKTTauSlider;
    CheckButton m_VKTInvertButton;
    Output m_VKTThickChordOutput;
    TriggerButton m_VKTFitCSTButton;
    Counter m_VKTDegreeCounter;

    GroupLayout m_FourDigitModGroup;
    StringOutput m_FourModNameOutput;
    CheckButton m_FourModInvertButton;
    SliderAdjRangeInput m_FourModChordSlider;
    SliderAdjRangeInput m_FourModThickChordSlider;
    SliderAdjRangeInput m_FourModCamberSlider;
    SliderAdjRangeInput m_FourModCLiSlider;
    ToggleButton m_FourModCamberButton;
    ToggleButton m_FourModCLiButton;
    ToggleRadioGroup m_FourModCamberGroup;
    SliderAdjRangeInput m_FourModCamberLocSlider;
    SliderAdjRangeInput m_FourModThicknessLocSlider;
    SliderAdjRangeInput m_FourModLERadIndexSlider;
    CheckButton m_FourModSharpTEButton;
    TriggerButton m_FourModFitCSTButton;
    Counter m_FourModDegreeCounter;

    GroupLayout m_FiveDigitGroup;
    StringOutput m_FiveNameOutput;
    CheckButton m_FiveInvertButton;
    SliderAdjRangeInput m_FiveChordSlider;
    SliderAdjRangeInput m_FiveThickChordSlider;
    SliderAdjRangeInput m_FiveCLiSlider;
    SliderAdjRangeInput m_FiveCamberLocSlider;
    CheckButton m_FiveSharpTEButton;
    TriggerButton m_FiveFitCSTButton;
    Counter m_FiveDegreeCounter;

    GroupLayout m_FiveDigitModGroup;
    StringOutput m_FiveModNameOutput;
    CheckButton m_FiveModInvertButton;
    SliderAdjRangeInput m_FiveModChordSlider;
    SliderAdjRangeInput m_FiveModThickChordSlider;
    SliderAdjRangeInput m_FiveModCLiSlider;
    SliderAdjRangeInput m_FiveModCamberLocSlider;
    SliderAdjRangeInput m_FiveModThicknessLocSlider;
    SliderAdjRangeInput m_FiveModLERadIndexSlider;
    CheckButton m_FiveModSharpTEButton;
    TriggerButton m_FiveModFitCSTButton;
    Counter m_FiveModDegreeCounter;

    GroupLayout m_OneSixSeriesGroup;
    StringOutput m_OneSixSeriesNameOutput;
    CheckButton m_OneSixSeriesInvertButton;
    SliderAdjRangeInput m_OneSixSeriesChordSlider;
    SliderAdjRangeInput m_OneSixSeriesThickChordSlider;
    SliderAdjRangeInput m_OneSixSeriesCLiSlider;
    SliderAdjRangeInput m_OneSixSeriesCamberLocSlider;
    SliderAdjRangeInput m_OneSixSeriesThicknessLocSlider;
    SliderAdjRangeInput m_OneSixSeriesLERadIndexSlider;
    CheckButton m_OneSixSeriesSharpTEButton;
    TriggerButton m_OneSixSeriesFitCSTButton;
    Counter m_OneSixSeriesDegreeCounter;


    GroupLayout* m_CurrDisplayGroup;
    GroupLayout* m_CurrXSCDisplayGroup;
};


#endif // !defined(AUXILIARYGEOMSCREEN__INCLUDED_)
