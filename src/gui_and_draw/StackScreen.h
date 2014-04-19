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

class StackScreen : public GeomScreen
{
public:
    StackScreen( ScreenMgr* mgr );
    virtual ~StackScreen();

    virtual void Show();
    virtual bool Update();

    virtual void CallBack( Fl_Widget *w );
    virtual void GuiDeviceCallBack( GuiDevice* d );

protected:

    GroupLayout m_DesignLayout;

    GroupLayout m_SkinLayout;

    IndexSelector m_SkinIndexSelector;

    ToggleButton m_AllSymButton;
    SkinOutput m_TopSkinOutput;
    SkinControl m_TopAngleSkinControl;
    SkinControl m_TopStrengthSkinControl;
    SkinControl m_TopCurvatureSkinControl;

    SkinOutput m_RightSkinOutput;
    SkinControl m_RightAngleSkinControl;
    SkinControl m_RightStrengthSkinControl;
    SkinControl m_RightCurvatureSkinControl;

    ToggleButton m_TBSymButton;
    SkinOutput m_BottomSkinOutput;
    SkinControl m_BottomAngleSkinControl;
    SkinControl m_BottomStrengthSkinControl;
    SkinControl m_BottomCurvatureSkinControl;

    ToggleButton m_RLSymButton;
    SkinOutput m_LeftSkinOutput;
    SkinControl m_LeftAngleSkinControl;
    SkinControl m_LeftStrengthSkinControl;
    SkinControl m_LeftCurvatureSkinControl;



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

    Choice m_XSecTypeChoice;

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

    GroupLayout* m_CurrDisplayGroup;
    void DisplayGroup( GroupLayout* group );

};


#endif // !defined(STACKSCREEN__INCLUDED_)
