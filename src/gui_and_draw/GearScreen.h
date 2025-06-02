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
    virtual void UpdateBogieBrowser();

    virtual void CallBack( Fl_Widget *w );
    static void staticScreenCB( Fl_Widget *w, void* data )
    {
        ( ( Gearcreen* )data )->CallBack( w );
    }

    virtual void GuiDeviceCallBack( GuiDevice* device );

protected:

    GroupLayout m_DesignLayout;
    GroupLayout m_BogieLayout;
    GroupLayout m_TireGroup;

    Choice m_ModelUnitsChoice;

    ToggleButton m_AutoPlaneSizeToggle;
    SliderAdjRangeInput m_PlaneSizeSlider;

    ToggleButton m_IncludeNominalGroundPlaneToggle;

    ToggleButton m_CGLocalToggle;
    ToggleButton m_CGGlobalToggle;
    ToggleRadioGroup m_CGRadioGroup;

    SliderAdjRange2Input m_XCGMinSlider;
    SliderAdjRange2Input m_XCGNominalSlider;
    SliderAdjRange2Input m_XCGMaxSlider;
    SliderAdjRange2Input m_YCGMinSlider;
    SliderAdjRange2Input m_YCGNominalSlider;
    SliderAdjRange2Input m_YCGMaxSlider;
    SliderAdjRange2Input m_ZCGMinSlider;
    SliderAdjRange2Input m_ZCGNominalSlider;
    SliderAdjRange2Input m_ZCGMaxSlider;

    // Bogie
    TriggerButton m_AddBogieButton;
    TriggerButton m_RenameBogieButton;
    TriggerButton m_RemoveBogieButton;
    TriggerButton m_RemoveAllBogiesButton;

    TriggerButton m_ShowAllBogiesButton;
    TriggerButton m_HideAllBogiesButton;

    StringInput m_BogieNameInput;

    ColResizeBrowser* m_BogieBrowser;

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

    SliderAdjRangeInput m_TravelXSlider;
    SliderAdjRangeInput m_TravelYSlider;
    SliderAdjRangeInput m_TravelZSlider;

    SliderAdjRangeInput m_SteeringAngleSlider;

    SliderAdjRangeInput m_TravelCompressSlider;
    SliderAdjRangeInput m_TravelExtendSlider;

    SliderAdjRangeInput m_BogieThetaMaxSlider;
    SliderAdjRangeInput m_BogieThetaMinSlider;

    SliderAdjRangeInput m_TravelSlider;
    SliderAdjRangeInput m_BogieThetaSlider;

    ToggleButton m_DrawNominalButton;

    // Tire
    Choice m_TireBogieChoice;

    ToggleButton m_TireDiameterInToggleButton;
    ToggleButton m_TireDiameterModelToggleButton;
    ToggleRadioGroup m_TireDiameterToggleGroup;
    SliderAdjRange2Input m_TireDiameterSlider;

    ToggleButton m_TireWidthInToggleButton;
    ToggleButton m_TireWidthModelToggleButton;
    ToggleRadioGroup m_TireWidthToggleGroup;
    SliderAdjRange2Input m_TireWidthSlider;

    ToggleButton m_DrimInToggleButton;
    ToggleButton m_DrimModelToggleButton;
    ToggleButton m_DrimFracToggleButton;
    ToggleRadioGroup m_DrimToggleGroup;
    SliderAdjRange3Input m_DrimSlider;

    ToggleButton m_WrimInToggleButton;
    ToggleButton m_WrimModelToggleButton;
    ToggleButton m_WrimFracToggleButton;
    ToggleRadioGroup m_WrimToggleGroup;
    SliderAdjRange3Input m_WrimSlider;

    SliderAdjRangeInput m_PlyRatingSlider;

    ToggleButton m_HsInToggleButton;
    ToggleButton m_HsModelToggleButton;
    ToggleButton m_HsFracToggleButton;
    ToggleRadioGroup m_HsToggleGroup;
    SliderAdjRange3Input m_HsSlider;

    ToggleButton m_WsInToggleButton;
    ToggleButton m_WsModelToggleButton;
    ToggleButton m_WsFracToggleButton;
    ToggleRadioGroup m_WsToggleGroup;
    SliderAdjRange3Input m_WsSlider;

    ToggleButton m_SLRInToggleButton;
    ToggleButton m_SLRModelToggleButton;
    ToggleButton m_DeflectionToggleButton;
    ToggleRadioGroup m_SLRToggleGroup;
    SliderAdjRangeInput m_DeflectionSlider;
    SliderAdjRange2Input m_SLRSlider;

};


#endif // !defined(GEARSCREEN__INCLUDED_)
