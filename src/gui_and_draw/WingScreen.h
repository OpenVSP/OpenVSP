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

class WingScreen : public GeomScreen
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
    StringOutput m_PlanAROutput;

    Choice m_RootCapTypeChoice;
    Choice m_TipCapTypeChoice;
    SliderAdjRangeInput m_CapTessSlider;

    SliderAdjRangeInput m_IncidenceSlider;
    SliderAdjRangeInput m_IncidenceLocSlider;

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

    GroupLayout m_CircleGroup;
    SliderAdjRangeInput m_DiameterSlider;

    GroupLayout m_EllipseGroup;
    SliderAdjRangeInput m_EllipseHeightSlider;
    SliderAdjRangeInput m_EllipseWidthSlider;

    GroupLayout m_RoundedRectGroup;
    SliderAdjRangeInput m_RRHeightSlider;
    SliderAdjRangeInput m_RRWidthSlider;
    SliderAdjRangeInput m_RRRadiusSlider;

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
    SliderAdjRangeInput m_FourCamberLocSlider;

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

    // Overall planform tab
    GroupLayout m_ModifyLayout;

    IndexSelector m_AfModIndexSelector;

    Choice m_TECloseChoice;
    ToggleButton m_TECloseABSButton;
    ToggleButton m_TECloseRELButton;
    ToggleRadioGroup m_TECloseGroup;

    SliderAdjRangeInput m_CloseThickSlider;
    SliderAdjRangeInput m_CloseThickChordSlider;

    Choice m_TETrimChoice;
    ToggleButton m_TETrimABSButton;
    ToggleButton m_TETrimRELButton;
    ToggleRadioGroup m_TETrimGroup;

    SliderAdjRangeInput m_TrimXSlider;
    SliderAdjRangeInput m_TrimXChordSlider;
    SliderAdjRangeInput m_TrimThickSlider;
    SliderAdjRangeInput m_TrimThickChordSlider;

    SliderAdjRangeInput m_AFThetaSlider;
    SliderAdjRangeInput m_AFScaleSlider;
    SliderAdjRangeInput m_AFDeltaXSlider;
    SliderAdjRangeInput m_AFDeltaYSlider;
    SliderAdjRangeInput m_AFShiftLESlider;

    GroupLayout* m_CurrDisplayGroup;
    void DisplayGroup( GroupLayout* group );

};


#endif // !defined(WINGSCREEN__INCLUDED_)
