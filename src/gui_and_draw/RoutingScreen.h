//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// RoutingScreen.h: UI for Routing Geom
// Rob McDonald
//
//////////////////////////////////////////////////////////////////////

#if !defined(ROUTINGSCREEN__INCLUDED_)
#define ROUTINGSCREEN__INCLUDED_

#include "ScreenBase.h"
#include "GuiDevice.h"

#include <FL/Fl.H>

class RoutingScreen : public GeomScreen
{
public:
    RoutingScreen( ScreenMgr* mgr );
    virtual ~RoutingScreen()                            {}

    virtual void Show();
    virtual bool Update();

    virtual void UpdateBrowser();

    virtual void CallBack( Fl_Widget *w );

    virtual void GuiDeviceCallBack( GuiDevice* gui_device );

    void Set( const vec3d &placement, const std::string &targetGeomId );

    virtual std::string getFeedbackGroupName();

    void LoadDrawObjs( vector< DrawObj* > & draw_obj_vec );

    void UpdatePickList();

protected:

    SliderAdjRangeInput m_LinearDensitySlider;

    GroupLayout m_DesignLayout;

    GroupLayout m_MovePointLayout;
    GroupLayout m_PointBrowserLayout;


    ColResizeBrowser* m_RoutingPointBrowser;
    int m_LiveIndex;


    TriggerButton m_AddRoutingPoint;
    TriggerButton m_DelRoutingPoint;
    TriggerButton m_DelAllRoutingPoints;

    TriggerButton m_SetRoutingPoint;

    TriggerButton m_AddMultipleRoutingPoints;
    TriggerButton m_StopAdding;

    TriggerButton m_InsertRoutingPoint;
    TriggerButton m_InsertMultipleRoutingPoints;


    TriggerButton m_MovePntUpButton;
    TriggerButton m_MovePntDownButton;
    TriggerButton m_MovePntTopButton;
    TriggerButton m_MovePntBotButton;

    StringInput m_PtNameInput;

    GeomPicker m_GeomPicker;
    Choice m_SurfChoice;

    Choice m_RoutePtCoordChoice;

    ToggleButton m_RoutePtCoordU01Toggle;
    ToggleButton m_RoutePtCoordU0NToggle;
    ToggleRadioGroup m_RoutePtCoordUScaleToggleGroup;

    ToggleButton m_RoutePtCoordR01Toggle;
    ToggleButton m_RoutePtCoordR0NToggle;
    ToggleRadioGroup m_RoutePtCoordRScaleToggleGroup;

    ToggleButton m_RoutePtCoordL01Toggle;
    ToggleButton m_RoutePtCoordL0LenToggle;
    ToggleRadioGroup m_RoutePtCoordLScaleToggleGroup;

    SliderAdjRange2Input m_RoutePtCoordUSlider;
    SliderAdjRangeInput m_RoutePtCoordWSlider;
    SliderAdjRange2Input m_RoutePtCoordRSlider;
    SliderAdjRangeInput m_RoutePtCoordSSlider;
    SliderAdjRangeInput m_RoutePtCoordTSlider;
    SliderAdjRange2Input m_RoutePtCoordLSlider;
    SliderAdjRangeInput m_RoutePtCoordMSlider;
    SliderAdjRangeInput m_RoutePtCoordNSlider;
    SliderAdjRangeInput m_RoutePtCoordEtaSlider;

    ToggleButton m_RoutePtDeltaAbsoluteToggle;
    ToggleButton m_RoutePtDeltaRelativeToggle;
    ToggleRadioGroup m_RoutePtDeltaAbsRelToggle;

    SliderAdjRange2Input m_RoutePtDeltaXSlider;
    SliderAdjRange2Input m_RoutePtDeltaYSlider;
    SliderAdjRange2Input m_RoutePtDeltaZSlider;

    SliderAdjRangeInput m_RoutePtRadiusSlider;

    Output m_LengthOutput;
    Output m_SymmLengthOutput;

private:

    bool m_SelectionFlag;
    bool m_AddMultipleFlag;
    bool m_InsertMultipleFlag;
    std::vector<DrawObj> m_PickList;

};


#endif // !defined(ROUTINGSCREEN__INCLUDED_)
