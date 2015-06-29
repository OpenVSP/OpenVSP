//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// PodScreen.h: UI for Pod Geom
// J.R Gloudemans
//
//////////////////////////////////////////////////////////////////////

#if !defined(PODSCREEN__INCLUDED_)
#define PODSCREEN__INCLUDED_

#include "ScreenBase.h"
#include "GuiDevice.h"

#include <FL/Fl.H>

class PodScreen : public GeomScreen
{
public:
    PodScreen( ScreenMgr* mgr );
    virtual ~PodScreen()                            {}

    virtual void Show();
    virtual bool Update();

    virtual void CallBack( Fl_Widget *w );

protected:

    GroupLayout m_DesignLayout;
    SliderAdjRangeInput m_LengthSlider;
    SliderAdjRangeInput m_FineSlider;
};


#endif // !defined(PODSCREEN__INCLUDED_)
