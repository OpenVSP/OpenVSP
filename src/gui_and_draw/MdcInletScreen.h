//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// MdcInletScreen.h: UI for MDC_INLET Geom
//
//////////////////////////////////////////////////////////////////////

#if !defined(INLET_SCREEN__INCLUDED_)
#define INLET_SCREEN__INCLUDED_

#include "ScreenBase.h"
#include "GuiDevice.h"

#include <FL/Fl.H>

#include "MdcInletGeom.h"

class MdcInletScreen : public GeomScreen
{
public:
    MdcInletScreen( ScreenMgr* mgr );
    virtual ~MdcInletScreen()                            {}

    virtual void Show();
    virtual bool Update();

    virtual void CallBack( Fl_Widget *w );
    virtual void GuiDeviceCallBack( GuiDevice* device );

protected:

    GroupLayout m_DesignLayout;

    Choice m_PresetChoice;
    TriggerButton m_SetPresetButton;
    Choice m_InTypeChoice;
    // XINLT is the XForm tab's X location.
    SliderAdjRangeInput m_XInltSlider;
    SliderAdjRangeInput m_RBodySlider;
    ToggleButton m_CoverToggle;
    SliderAdjRangeInput m_RampSlider;

    SliderAdjRangeInput m_HDivSlider;
    SliderAdjRangeInput m_XDivSlider;
    SliderAdjRangeInput m_LDivSlider;

    SliderAdjRangeInput m_NInSlider;
    ToggleButton m_PhiFlagToggle;
    IndexSelector m_InletSelector;
    SliderAdjRangeInput m_PhiSlider;

    SliderAdjRangeInput m_XSlider[MdcInletGeom::NUM_STN - 1];
    SliderAdjRangeInput m_HSlider[MdcInletGeom::NUM_STN - 1];
    SliderAdjRangeInput m_WSlider[MdcInletGeom::NUM_STN];

};

#endif // !defined(INLET_SCREEN__INCLUDED_)
