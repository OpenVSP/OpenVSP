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

    virtual void RebuildCSTGroup( CSTAirfoil* cst_xs );

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

    SliderInput m_SectUTessSlider;
    SliderAdjRangeInput m_RootClusterSlider;
    SliderAdjRangeInput m_TipClusterSlider;

    WingDriverGroup m_DefaultWingDriverGroup;
    DriverGroupBank m_WingDriverGroupBank;

    TriggerButton m_TestDriverGroupButton;

    StringOutput m_SectProjSpanOutput;

    SliderAdjRangeInput m_SweepSlider;
    SliderAdjRangeInput m_SweepLocSlider;
    SliderAdjRangeInput m_SecSweepLocSlider;

    SliderAdjRangeInput m_TwistSlider;
    SliderAdjRangeInput m_TwistLocSlider;
    ToggleButton m_TwistAbsoluteToggle;
    ToggleButton m_TwistRelativeToggle;
    ToggleRadioGroup m_TwistAbsRelToggle;

    SliderAdjRangeInput m_DihedralSlider;
    ToggleButton m_DihedralAbsoluteToggle;
    ToggleButton m_DihedralRelativeToggle;
    ToggleRadioGroup m_DihedralAbsRelToggle;
    ToggleButton m_RotateFoilMatchDihedral;

    // Airfoil tab
    GroupLayout m_AfLayout;

    IndexSelector m_AfIndexSelector;

    TriggerButton m_CopyAfButton;
    TriggerButton m_PasteAfButton;

    Choice m_AfTypeChoice;
    TriggerButton m_ShowXSecButton;

    GroupLayout m_PointGroup;

    GroupLayout m_SuperGroup;
    SliderAdjRangeInput m_SuperHeightSlider;
    SliderAdjRangeInput m_SuperWidthSlider;
    SliderAdjRangeInput m_SuperMSlider;
    SliderAdjRangeInput m_SuperNSlider;
    ToggleButton m_SuperToggleSym;
    SliderAdjRangeInput m_SuperM_botSlider;
    SliderAdjRangeInput m_SuperN_botSlider;
    SliderAdjRangeInput m_SuperMaxWidthLocSlider;

    GroupLayout m_CircleGroup;
    SliderAdjRangeInput m_DiameterSlider;

    GroupLayout m_EllipseGroup;
    SliderAdjRangeInput m_EllipseHeightSlider;
    SliderAdjRangeInput m_EllipseWidthSlider;

    GroupLayout m_RoundedRectGroup;
    SliderAdjRangeInput m_RRHeightSlider;
    SliderAdjRangeInput m_RRWidthSlider;
    SliderAdjRangeInput m_RRRadiusSlider;
    ToggleButton m_RRKeyCornerButton;
    SliderAdjRangeInput m_RRSkewSlider;
    SliderAdjRangeInput m_RRKeystoneSlider;

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
    CheckButton m_FourEqArcLenButton;
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
    SliderAdjRangeInput m_WedgeThickLocSlider;

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
    CheckButton m_CSTEqArcLenButton;

    GroupLayout m_VKTGroup;
    SliderAdjRangeInput m_VKTChordSlider;
    SliderAdjRangeInput m_VKTEpsilonSlider;
    SliderAdjRangeInput m_VKTKappaSlider;
    SliderAdjRangeInput m_VKTTauSlider;
    CheckButton m_VKTInvertButton;

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

    GroupLayout* m_CurrDisplayGroup;
    void DisplayGroup( GroupLayout* group );

};


#endif // !defined(WINGSCREEN__INCLUDED_)
