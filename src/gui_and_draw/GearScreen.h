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

class GearScreen : public GeomScreen
{
public:
    GearScreen( ScreenMgr* mgr );
    virtual ~GearScreen()                            {}

    virtual void Show();
    virtual bool Update();
    virtual void UpdateBogieBrowser();
    virtual void UpdateStowSurfChoice();
    virtual void UpdateMechSurfChoice();

    virtual void CallBack( Fl_Widget *w );
    static void staticScreenCB( Fl_Widget *w, void* data )
    {
        ( ( GearScreen* )data )->CallBack( w );
    }

    virtual void GuiDeviceCallBack( GuiDevice* device );

protected:

    GroupLayout m_DesignLayout;
    GroupLayout m_BogieLayout;
    GroupLayout m_TireLayout;
    GroupLayout m_RetractLayout;

    Fl_Tabs* m_RetractTabs;

    GroupLayout m_StowLayout;
    GroupLayout m_MechanismLayout;
    GroupLayout m_Mechanism2Layout;


    GroupLayout m_StowAttachLayout;
    GroupLayout m_StowAttachLayoutTransHeader;
    GroupLayout m_StowAttachLayoutRotHeader;
    GroupLayout m_StowAttachLayoutSelections;

    GroupLayout m_MechAttachLayout;
    GroupLayout m_MechAttachLayoutTransHeader;
    GroupLayout m_MechAttachLayoutSelections;

    Choice m_ModelUnitsChoice;

    ToggleButton m_AutoPlaneSizeToggle;
    SliderAdjRangeInput m_PlaneSizeSlider;

    ToggleButton m_IncludeNominalGroundPlaneToggle;
    Choice m_GearConfigurationChoice;

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

    // SliderAdjRangeInput m_TravelSlider;
    // SliderAdjRangeInput m_BogieThetaSlider;
    //
    // ToggleButton m_DrawNominalButton;

    // Tire
    Choice m_TireBogieChoice;

    Choice m_TireModeChoice;

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

    // Retracted
    Choice m_RetBogieChoice;

    Choice m_RetModeChoice;


    GeomPicker m_StowGeomPicker;
    Choice m_StowSurfChoice;

    ToggleButton m_StowXFormAbsoluteToggle;
    ToggleButton m_StowXFormRelativeToggle;
    ToggleRadioGroup m_StowXFormAbsRelToggle;

    SliderAdjRange2Input m_StowXLocSlider;
    SliderAdjRange2Input m_StowYLocSlider;
    SliderAdjRange2Input m_StowZLocSlider;

    SliderAdjRange2Input m_StowXRotSlider;
    SliderAdjRange2Input m_StowYRotSlider;
    SliderAdjRange2Input m_StowZRotSlider;

    //==== Attachments
    ToggleButton m_StowTransNoneButton;
    ToggleButton m_StowTransCompButton;
    ToggleButton m_StowTransUVButton;
    ToggleButton m_StowTransRSTButton;
    ToggleButton m_StowTransLMNButton;
    ToggleButton m_StowTransEtaMNButton;
    ToggleRadioGroup m_StowTransToggleGroup;

    ToggleButton m_StowRotNoneButton;
    ToggleButton m_StowRotCompButton;
    ToggleButton m_StowRotUVButton;
    ToggleButton m_StowRotRSTButton;
    ToggleButton m_StowRotLMNButton;
    ToggleButton m_StowRotEtaMNButton;
    ToggleRadioGroup m_StowRotToggleGroup;

    ToggleButton m_StowU01Toggle;
    ToggleButton m_StowU0NToggle;
    ToggleRadioGroup m_StowUScaleToggleGroup;

    ToggleButton m_StowR01Toggle;
    ToggleButton m_StowR0NToggle;
    ToggleRadioGroup m_StowRScaleToggleGroup;

    ToggleButton m_StowL01Toggle;
    ToggleButton m_StowL0LenToggle;
    ToggleRadioGroup m_StowLScaleToggleGroup;

    SliderAdjRange2Input m_StowAttachUSlider;
    SliderAdjRangeInput m_StowAttachVSlider;
    SliderAdjRange2Input m_StowAttachRSlider;
    SliderAdjRangeInput m_StowAttachSSlider;
    SliderAdjRangeInput m_StowAttachTSlider;
    SliderAdjRange2Input m_StowAttachLSlider;
    SliderAdjRangeInput m_StowAttachMSlider;
    SliderAdjRangeInput m_StowAttachNSlider;
    SliderAdjRangeInput m_StowAttachEtaSlider;

    GeomPicker m_MechGeomPicker;
    Choice m_MechSurfChoice;

    ToggleButton m_MechXFormAbsoluteToggle;
    ToggleButton m_MechXFormRelativeToggle;
    ToggleRadioGroup m_MechXFormAbsRelToggle;

    SliderAdjRange2Input m_MechXLocSlider;
    SliderAdjRange2Input m_MechYLocSlider;
    SliderAdjRange2Input m_MechZLocSlider;

    SliderAdjRangeInput m_MechXAxisSlider;
    SliderAdjRangeInput m_MechYAxisSlider;
    SliderAdjRangeInput m_MechZAxisSlider;

    //==== Attachments
    ToggleButton m_MechTransNoneButton;
    ToggleButton m_MechTransCompButton;
    ToggleButton m_MechTransUVButton;
    ToggleButton m_MechTransRSTButton;
    ToggleButton m_MechTransLMNButton;
    ToggleButton m_MechTransEtaMNButton;
    ToggleRadioGroup m_MechTransToggleGroup;

    ToggleButton m_MechU01Toggle;
    ToggleButton m_MechU0NToggle;
    ToggleRadioGroup m_MechUScaleToggleGroup;

    ToggleButton m_MechR01Toggle;
    ToggleButton m_MechR0NToggle;
    ToggleRadioGroup m_MechRScaleToggleGroup;

    ToggleButton m_MechL01Toggle;
    ToggleButton m_MechL0LenToggle;
    ToggleRadioGroup m_MechLScaleToggleGroup;

    SliderAdjRange2Input m_MechAttachUSlider;
    SliderAdjRangeInput m_MechAttachVSlider;
    SliderAdjRange2Input m_MechAttachRSlider;
    SliderAdjRangeInput m_MechAttachSSlider;
    SliderAdjRangeInput m_MechAttachTSlider;
    SliderAdjRange2Input m_MechAttachLSlider;
    SliderAdjRangeInput m_MechAttachMSlider;
    SliderAdjRangeInput m_MechAttachNSlider;
    SliderAdjRangeInput m_MechAttachEtaSlider;

    SliderAdjRangeInput m_MechKRetractSlider;
    SliderAdjRangeInput m_Mech2KRetractSlider;

    SliderAdjRangeInput m_MechKneePosSlider;
    SliderAdjRangeInput m_MechKneeAngleSlider;
    SliderAdjRangeInput m_MechKneeClockAngleSlider;
    SliderAdjRangeInput m_MechKneeElevationAngleSlider;
    SliderAdjRangeInput m_MechKneeDownAngleSlider;

    SliderAdjRangeInput m_MechRetAngleSlider;
    SliderAdjRangeInput m_MechTwistAngleSlider;
    SliderAdjRangeInput m_MechRollAngleSlider;
    SliderAdjRangeInput m_MechBogieAngleSlider;

    SliderAdjRangeInput m_MechStrutDLSlider;
};


#endif // !defined(GEARSCREEN__INCLUDED_)
