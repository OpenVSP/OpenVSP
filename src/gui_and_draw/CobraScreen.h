//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// CobraScreen.h: UI for Cobra Geom
// Rob McDonald
//
//////////////////////////////////////////////////////////////////////

#if !defined(COBRA_SCREEN__INCLUDED_)
#define COBRA_SCREEN__INCLUDED_

#include "ScreenBase.h"
#include "GuiDevice.h"

#include <FL/Fl.H>

class CobraScreen : public GeomScreen
{
public:
    CobraScreen( ScreenMgr* mgr );
    virtual ~CobraScreen()                            {}

    virtual void Show();
    virtual bool Update();

    virtual void CallBack( Fl_Widget *w );

protected:

    GroupLayout m_DesignLayout;
    SliderAdjRangeInput m_LengthSlider;
    SliderAdjRangeInput m_FineSlider;
};


#endif // !defined(COBRA_SCREEN__INCLUDED_)
