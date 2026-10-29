//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// MdcProtubScreen.h: UI for MDC_PROTUB Geom
//
//////////////////////////////////////////////////////////////////////

#if !defined(PROTUB_SCREEN__INCLUDED_)
#define PROTUB_SCREEN__INCLUDED_

#include "ScreenBase.h"
#include "GuiDevice.h"

#include <FL/Fl.H>

#include "MdcProtubGeom.h"

class MdcProtubScreen : public GeomScreen
{
public:
    MdcProtubScreen( ScreenMgr* mgr );
    virtual ~MdcProtubScreen()                            {}

    virtual void Show();
    virtual bool Update();

    virtual void CallBack( Fl_Widget *w );
    virtual void GuiDeviceCallBack( GuiDevice* device );

protected:

    GroupLayout m_DesignLayout;

    Choice m_PTypeChoice;
    TriggerButton m_SetDefaultButton;
    // XPROT is the XForm tab's X location.
    SliderAdjRangeInput m_XProtSlider;
    SliderAdjRangeInput m_RBodySlider;

    SliderAdjRangeInput m_NLocSlider;
    ToggleButton m_PhiProFlagToggle;
    IndexSelector m_LocIndexSelector;
    SliderAdjRangeInput m_PhiProSlider;

    // LPROT, WPROT, HPROT and OPROT of each member in turn.
    SliderAdjRangeInput m_LProtSlider[MdcProtubGeom::MAX_MEM];
    SliderAdjRangeInput m_WProtSlider[MdcProtubGeom::MAX_MEM];
    SliderAdjRangeInput m_HProtSlider[MdcProtubGeom::MAX_MEM];
    SliderAdjRangeInput m_OProtSlider[MdcProtubGeom::MAX_MEM];
};

#endif // !defined(PROTUB_SCREEN__INCLUDED_)
