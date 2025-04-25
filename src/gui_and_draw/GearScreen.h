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

class Gearcreen : public GeomScreen
{
public:
	Gearcreen( ScreenMgr* mgr );
    virtual ~Gearcreen()                            {}

    virtual void Show();
    virtual bool Update();

    virtual void CallBack( Fl_Widget *w );

protected:

    GroupLayout m_DesignLayout;
    SliderInput m_NumPntsXSecSlider;
    SliderAdjRangeInput m_LengthSlider;
    SliderAdjRangeInput m_FineSlider;

    SliderInput m_NumBaseSlider;
};


#endif // !defined(GEARSCREEN__INCLUDED_)
