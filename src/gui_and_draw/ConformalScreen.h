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
    GroupLayout m_WingGroup;

    SliderAdjRangeInput m_OffsetSlider;

    ToggleButton m_UTrimToggle;
    SliderAdjRangeInput m_UTrimMinSlider;
    SliderAdjRangeInput m_UTrimMaxSlider;

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

};


#endif // !defined(PODSCREEN__INCLUDED_)
