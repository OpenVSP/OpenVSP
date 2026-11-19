//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// MdcFinScreen.h: UI for MDC_FIN Geom
//
//////////////////////////////////////////////////////////////////////

#if !defined(FINSET_SCREEN__INCLUDED_)
#define FINSET_SCREEN__INCLUDED_

#include "ScreenBase.h"
#include "GuiDevice.h"
#include "RelAbsSlider.h"

#include <FL/Fl.H>

class MdcFinScreen : public GeomScreen
{
public:
    MdcFinScreen( ScreenMgr* mgr );
    virtual ~MdcFinScreen()                            {}

    virtual void Show();
    virtual bool Update();

    virtual void CallBack( Fl_Widget *w );
    virtual void GuiDeviceCallBack( GuiDevice* device );

protected:

    GroupLayout m_FinSetLayout;
    GroupLayout m_PlanformLayout;
    GroupLayout m_SectionLayout;

    // Fin Set
    Choice m_PresetChoice;
    TriggerButton m_SetPresetButton;
    Choice m_SecTypChoice;
    SliderAdjRangeInput m_NPanelSlider;
    SliderAdjRangeInput m_RBodySlider;
    ToggleButton m_BuryRootToggle;
    Choice m_BuryRootTypeChoice;
    ToggleButton m_PhiFFlagToggle;
    IndexSelector m_PanelSelector;
    SliderAdjRangeInput m_PhiFSlider;
    SliderAdjRangeInput m_GamSlider;

    // Planform
    SliderAdjRangeInput m_NSpanSlider;
    IndexSelector m_StationSelector;
    SliderAdjRangeInput m_SSpanSlider;
    SliderAdjRangeInput m_ChordSlider;
    SliderAdjRangeInput m_XLESlider;
    ToggleButton m_XLEFlagToggle;
    SliderAdjRangeInput m_SweepSlider;
    SliderAdjRangeInput m_StaSlider;
    SliderAdjRangeInput m_SectTessUSlider;
    SliderAdjRangeInput m_LERSlider;
    Choice m_LERTypeChoice;
    SliderAdjRangeInput m_CFOCSlider;

    // Section
    IndexSelector m_SectionStationSelector;
    SliderAdjRangeInput m_ZUpperSlider;
    RelAbsSlider m_ZLowerSlider;
    SliderAdjRangeInput m_LMaxUSlider;
    RelAbsSlider m_LMaxLSlider;
    SliderAdjRangeInput m_LFlatUSlider;
    RelAbsSlider m_LFlatLSlider;

    Choice m_NacaSerChoice;
    SliderAdjRangeInput m_NacaDesSlider;
    SliderAdjRangeInput m_NacaModSlider;
    ToggleButton m_NacaSeaToggle;
    SliderAdjRangeInput m_NacaMeanSlider;
    SliderAdjRangeInput m_NacaXMaxSlider;
    SliderAdjRangeInput m_NacaFlatSlider;

    TriggerButton m_ConvertToWingButton;
    TriggerButton m_ReadUserButton;
};

#endif // !defined(FINSET_SCREEN__INCLUDED_)
