//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// ClearanceScreen.h: UI for Clearance Geom
// Rob McDonald
//
//////////////////////////////////////////////////////////////////////

#if !defined(CLEARANCESCREEN__INCLUDED_)
#define CLEARANCESCREEN__INCLUDED_

#include "ScreenBase.h"
#include "GuiDevice.h"

#include <FL/Fl.H>

class ClearanceScreen : public GeomScreen
{
public:
    ClearanceScreen( ScreenMgr* mgr );
    virtual ~ClearanceScreen()                            {}

    virtual void Show();
    virtual bool Update();

    virtual void CallBack( Fl_Widget *w );

protected:

    GroupLayout m_DesignLayout;

};


#endif // !defined(CLEARANCESCREEN__INCLUDED_)
