//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// ClearanceScreen.h: UI for Clearance Geom
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

protected:

    GroupLayout m_DesignLayout;

    ColResizeBrowser* m_RoutingPointBrowser;
    int m_RoutingPointBrowserSelect;


    TriggerButton m_AddRoutingPoint;
    TriggerButton m_DelRoutingPoint;
    TriggerButton m_DelAllRoutingPoints;

    StringInput m_PtNameInput;

    GeomPicker m_GeomPicker;

    SliderAdjRangeInput m_USlider;
    SliderAdjRangeInput m_WSlider;

};


#endif // !defined(ROUTINGSCREEN__INCLUDED_)
