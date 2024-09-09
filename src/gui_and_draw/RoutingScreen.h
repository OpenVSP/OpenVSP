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

    virtual void CallBack( Fl_Widget *w );

protected:

    GroupLayout m_DesignLayout;

};


#endif // !defined(ROUTINGSCREEN__INCLUDED_)
