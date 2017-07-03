//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// EllipsoidScreen.h: UI for Ellipsoid Geom
// Rob McDonald
//
//////////////////////////////////////////////////////////////////////

#if !defined(ELLIPSOID_SCREEN__INCLUDED_)
#define ELLIPSOID_SCREEN__INCLUDED_

#include "ScreenBase.h"
#include "GuiDevice.h"

#include <FL/Fl.H>

class EllipsoidScreen : public GeomScreen
{
public:
    EllipsoidScreen( ScreenMgr* mgr );
    virtual ~EllipsoidScreen()                            {}

    virtual void Show();
    virtual bool Update();

    virtual void CallBack( Fl_Widget *w );

protected:

    GroupLayout m_DesignLayout;
    SliderAdjRangeInput m_ARadiusSlider;
    SliderAdjRangeInput m_BRadiusSlider;
    SliderAdjRangeInput m_CRadiusSlider;
};


#endif // !defined(ELLIPSOID_SCREEN__INCLUDED_)
