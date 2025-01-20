//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// GearScreen.h: UI for Gear Geom
// Rob McDonald
//
//////////////////////////////////////////////////////////////////////

#if !defined(GEARSCREEN__INCLUDED_)
#define GEARSCREEN__INCLUDED_

#include "ScreenBase.h"
#include "GuiDevice.h"

#include <FL/Fl.H>

class Gearcreen : public GeomScreen
{
public:
    Gearcreen( ScreenMgr* mgr );
    virtual ~Gearcreen()                            {}

    virtual void Show();
    virtual bool Update();

    virtual void CallBack( Fl_Widget *w );
    static void staticScreenCB( Fl_Widget *w, void* data )
    {
        ( ( Gearcreen* )data )->CallBack( w );
    }

    virtual void GuiDeviceCallBack( GuiDevice* device );

protected:

    GroupLayout m_DesignLayout;
    GroupLayout m_TireGroup;


    TriggerButton m_AddBogieButton;
    TriggerButton m_RenameBogieButton;
    TriggerButton m_RemoveBogieButton;
    TriggerButton m_RemoveAllBogiesButton;

    TriggerButton m_ShowAllBogiesButton;
    TriggerButton m_HideAllBogiesButton;

    StringInput m_BogieNameInput;
    Fl_Browser* m_BogieBrowser;

    ToggleButton m_SymmetricalButton;

    SliderAdjRangeInput m_NAcrossSlider;
    SliderAdjRangeInput m_NTandemSlider;

    Choice m_SpacingTypeChoice;
    SliderAdjRange2Input m_SpacingSlider;
    SliderAdjRange2Input m_SpacingGapSlider;

    Choice m_PitchTypeChoice;
    SliderAdjRange2Input m_PitchSlider;
    SliderAdjRange2Input m_PitchGapSlider;

    SliderAdjRangeInput m_TireXSlider;
    SliderAdjRangeInput m_TireYSlider;
    SliderAdjRangeInput m_TireZSlider;

    SliderAdjRangeInput m_TireDiameterSlider;
    SliderAdjRangeInput m_TireWidthSlider;

    ToggleButton m_DrimToggleButton;
    ToggleButton m_DrimFracToggleButton;
    ToggleRadioGroup m_DrimToggleGroup;
    SliderAdjRange2Input m_DrimSlider;

    ToggleButton m_WrimToggleButton;
    ToggleButton m_WrimFracToggleButton;
    ToggleRadioGroup m_WrimToggleGroup;
    SliderAdjRange2Input m_WrimSlider;

    ToggleButton m_HsToggleButton;
    ToggleButton m_HsFracToggleButton;
    ToggleRadioGroup m_HsToggleGroup;
    SliderAdjRange2Input m_HsSlider;

    ToggleButton m_WsToggleButton;
    ToggleButton m_WsFracToggleButton;
    ToggleRadioGroup m_WsToggleGroup;
    SliderAdjRange2Input m_WsSlider;

};


#endif // !defined(GEARSCREEN__INCLUDED_)
