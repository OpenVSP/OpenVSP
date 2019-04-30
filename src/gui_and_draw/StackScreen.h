//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// FuselageScreen.h: UI for Fuselage Geom
// J.R Gloudemans
//
//////////////////////////////////////////////////////////////////////

#if !defined(STACKSCREEN__INCLUDED_)
#define STACKSCREEN__INCLUDED_

#include "ScreenBase.h"
#include "GuiDevice.h"
#include "GroupLayout.h"

#include <FL/Fl.H>

class StackScreen : public SkinScreen
{
public:
    StackScreen( ScreenMgr* mgr );
    virtual ~StackScreen();

    virtual void Show();
    virtual bool Update();

    virtual void CallBack( Fl_Widget *w );
    virtual void GuiDeviceCallBack( GuiDevice* d );
    
    virtual void RebuildCSTGroup( CSTAirfoil* cst_xs );

protected:

    GroupLayout m_DesignLayout;

    Choice m_DesignPolicyChoice;

    Choice m_NoseCapTypeChoice;
    SliderAdjRangeInput m_NoseCapLenSlider;
    SliderAdjRangeInput m_NoseCapOffsetSlider;
    SliderAdjRangeInput m_NoseCapStrengthSlider;
    ToggleButton m_NoseCapSweepFlagButton;

    Choice m_TailCapTypeChoice;
    SliderAdjRangeInput m_TailCapLenSlider;
    SliderAdjRangeInput m_TailCapOffsetSlider;
    SliderAdjRangeInput m_TailCapStrengthSlider;
    ToggleButton m_TailCapSweepFlagButton;

    SliderAdjRangeInput m_CapTessSlider;

    GroupLayout m_XSecLayout;

    IndexSelector m_XSecIndexSelector;

    TriggerButton m_InsertXSec;
    TriggerButton m_CutXSec;
    TriggerButton m_CopyXSec;
    TriggerButton m_PasteXSec;

    SliderInput m_SectUTessSlider;

    SliderAdjRangeInput m_XSecXDeltaSlider;
    SliderAdjRangeInput m_XSecYDeltaSlider;
    SliderAdjRangeInput m_XSecZDeltaSlider;
    SliderAdjRangeInput m_XSecXRotSlider;
    SliderAdjRangeInput m_XSecYRotSlider;
    SliderAdjRangeInput m_XSecZRotSlider;
    SliderAdjRangeInput m_XSecSpinSlider;

    Choice m_XSecTypeChoice;
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
    CheckButton m_FourSharpTEButton;

    GroupLayout m_SixSeriesGroup;
    StringOutput m_SixNameOutput;
    CheckButton m_SixInvertButton;
    Choice m_SixSeriesChoice;
    SliderAdjRangeInput m_SixChordSlider;
    SliderAdjRangeInput m_SixThickChordSlider;
    SliderAdjRangeInput m_SixIdealClSlider;
    SliderAdjRangeInput m_SixASlider;

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

    GroupLayout m_CSTAirfoilGroup;

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
    CheckButton m_CSTEqArcLenButton;

    GroupLayout m_VKTGroup;
    SliderAdjRangeInput m_VKTChordSlider;
    SliderAdjRangeInput m_VKTEpsilonSlider;
    SliderAdjRangeInput m_VKTKappaSlider;
    SliderAdjRangeInput m_VKTTauSlider;
    CheckButton m_VKTInvertButton;

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

    GroupLayout m_FiveDigitGroup;
    StringOutput m_FiveNameOutput;
    CheckButton m_FiveInvertButton;
    SliderAdjRangeInput m_FiveChordSlider;
    SliderAdjRangeInput m_FiveThickChordSlider;
    SliderAdjRangeInput m_FiveCLiSlider;
    SliderAdjRangeInput m_FiveCamberLocSlider;
    CheckButton m_FiveSharpTEButton;

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

    GroupLayout* m_CurrDisplayGroup;
    void DisplayGroup( GroupLayout* group );

};


#endif // !defined(STACKSCREEN__INCLUDED_)
