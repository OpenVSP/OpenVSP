//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// BORScreen.h: UI for BOR Geom
// Rob McDonald
//
//////////////////////////////////////////////////////////////////////

#if !defined(BOR_SCREEN__INCLUDED_)
#define BOR_SCREEN__INCLUDED_

#include "ScreenBase.h"
#include "GuiDevice.h"

#include <FL/Fl.H>

class BORScreen : public GeomScreen
{
public:
    BORScreen( ScreenMgr* mgr );
    virtual ~BORScreen()                            {}

    virtual void Show();
    virtual bool Update();

    virtual void CallBack( Fl_Widget *w );

protected:

    GroupLayout m_DesignLayout;
    SliderAdjRangeInput m_ARadiusSlider;
    SliderAdjRangeInput m_BRadiusSlider;
    SliderAdjRangeInput m_CRadiusSlider;
};


#endif // !defined(BOR_SCREEN__INCLUDED_)
