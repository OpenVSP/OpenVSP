//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// MdcBodScreen.h: UI for MDC_BOD Geom
//
//////////////////////////////////////////////////////////////////////

#if !defined(MDC_BOD_SCREEN__INCLUDED_)
#define MDC_BOD_SCREEN__INCLUDED_

#include "ScreenBase.h"
#include "GuiDevice.h"

#include <FL/Fl.H>

class MdcBodScreen : public GeomScreen
{
public:
    MdcBodScreen( ScreenMgr* mgr );
    virtual ~MdcBodScreen()                            {}

    virtual void Show();
    virtual bool Update();

    virtual void CallBack( Fl_Widget *w );
    virtual void GuiDeviceCallBack( GuiDevice* device );

protected:

    GroupLayout m_DesignLayout;

    TriggerButton m_ConvertToStackButton;

    Choice m_NamelistChoice;
    // XO is the XForm tab's X location.
    SliderAdjRangeInput m_XOSlider;

    Choice m_TNoseChoice;
    SliderAdjRangeInput m_PowerSlider;
    SliderAdjRangeInput m_LNoseSlider;
    SliderAdjRangeInput m_DNoseSlider;
    SliderAdjRangeInput m_WNoseSlider;
    SliderAdjRangeInput m_ENoseSlider;
    SliderAdjRangeInput m_BNoseSlider;
    ToggleButton m_TruncToggle;
    SliderAdjRangeInput m_SlpNosSlider;
    SliderAdjRangeInput m_ZTipSlider;
    SliderAdjRangeInput m_ZNoseSlider;

    SliderAdjRangeInput m_LCentrSlider;
    ToggleButton m_CentrDefaultToggle;
    SliderAdjRangeInput m_DCentrSlider;
    SliderAdjRangeInput m_WCentrSlider;
    SliderAdjRangeInput m_ECentrSlider;
    SliderAdjRangeInput m_ZCentrSlider;

    Choice m_TAftChoice;
    SliderAdjRangeInput m_LAftSlider;
    SliderAdjRangeInput m_DAftSlider;
    SliderAdjRangeInput m_WAftSlider;
    SliderAdjRangeInput m_EAftSlider;
    SliderAdjRangeInput m_ZAftSlider;
    SliderAdjRangeInput m_SlpTalSlider;

    SliderAdjRangeInput m_DExitSlider;
};

#endif // !defined(MDC_BOD_SCREEN__INCLUDED_)
