//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// ConformalScreen.h: UI for Pod Geom
// J.R Gloudemans
//
//////////////////////////////////////////////////////////////////////

#if !defined(CONFORMALSCREEN__INCLUDED_)
#define CONFORMALSCREEN__INCLUDED_

#include "ScreenBase.h"
#include "GuiDevice.h"

#include <FL/Fl.H>

class ConformalScreen : public GeomScreen
{
public:
    ConformalScreen( ScreenMgr* mgr );
    virtual ~ConformalScreen()                            {}

    virtual void Show();
    virtual bool Update();

    virtual void CallBack( Fl_Widget *w );

protected:

    GroupLayout m_DesignLayout;
    GroupLayout m_TrimGroup;
    GroupLayout m_SideGroup;
    GroupLayout m_WingGroup;

    SliderAdjRangeInput m_OffsetSlider;

    ToggleButton m_UTrimToggle;
    SliderAdjRangeInput m_UTrimMinSlider;
    SliderAdjRangeInput m_UTrimMaxSlider;

    ToggleButton m_UMinUTrimButton;
    ToggleButton m_UminLTrimButton;
    ToggleButton m_UMinEtaTrimButton;
    ToggleRadioGroup m_UMinTrimToggleGroup;

    ToggleButton m_L01MinTrimToggle;
    ToggleButton m_L0LenMinTrimToggle;
    ToggleRadioGroup m_LMinScaleTrimToggleGroup;

    SliderAdjRange2Input m_LTrimMinSlider;

    ToggleButton m_UMaxUTrimButton;
    ToggleButton m_UmaxLTrimButton;
    ToggleButton m_UmaxEtaTrimButton;
    ToggleRadioGroup m_UMaxTrimToggleGroup;

    ToggleButton m_L01MaxTrimToggle;
    ToggleButton m_L0LenMaxTrimToggle;
    ToggleRadioGroup m_LMaxScaleTrimToggleGroup;

    SliderAdjRange2Input m_LTrimMaxSlider;

    SliderAdjRangeInput m_EtaTrimMinSlider;
    SliderAdjRangeInput m_EtaTrimMaxSlider;

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

    ToggleButton m_V1TrimToggle;
    SliderAdjRangeInput m_V1TrimMinSlider;
    SliderAdjRangeInput m_V1TrimMaxSlider;

    ToggleButton m_V2TrimToggle;
    SliderAdjRangeInput m_V2TrimMinSlider;
    SliderAdjRangeInput m_V2TrimMaxSlider;

    //ToggleButton m_SpanTrimToggle;
    //SliderAdjRangeInput m_SpanTrimMinSlider;
    //SliderAdjRangeInput m_SpanTrimMaxSlider;

    ToggleButton m_ChordTrimToggle;
    SliderAdjRangeInput m_ChordTrimMinSlider;
    SliderAdjRangeInput m_ChordTrimMaxSlider;

    ToggleButton m_Side1TrimToggle;
    SliderAdjRangeInput m_Side1TrimSlider;

    ToggleButton m_Side2TrimToggle;
    SliderAdjRangeInput m_Side2TrimSlider;

    ToggleButton m_Side3TrimToggle;
    SliderAdjRangeInput m_Side3TrimSlider;

    ToggleButton m_Side4TrimToggle;
    SliderAdjRangeInput m_Side4TrimSlider;

};


#endif // !defined(PODSCREEN__INCLUDED_)
