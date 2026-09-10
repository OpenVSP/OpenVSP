//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// CloneScreen.h: UI for Clone Geom
// Rob McDonald
//
//////////////////////////////////////////////////////////////////////

#if !defined(CLONESCREEN__INCLUDED_)
#define CLONESCREEN__INCLUDED_

#include "ScreenBase.h"
#include "GuiDevice.h"

#include <FL/Fl.H>

class CloneScreen : public GeomScreen
{
public:
    CloneScreen( ScreenMgr* mgr );
    virtual ~CloneScreen()                            {}

    virtual void Show();
    virtual bool Update();

    virtual void CallBack( Fl_Widget *w );
    virtual void GuiDeviceCallBack( GuiDevice* d );

protected:

    // The Clone tab: what it copies, naming, and joint deflection.
    GroupLayout m_CloneLayout;

    Choice m_OriginalChoice;
    vector <string> m_CompVec;
};


#endif // !defined(CLONESCREEN__INCLUDED_)
