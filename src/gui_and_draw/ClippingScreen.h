//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// ClippingScreen.h: interface for the ClippingMgr class.
//
//////////////////////////////////////////////////////////////////////

#ifndef CLIPPINGSCREEN_H
#define CLIPPINGSCREEN_H

#include "ScreenMgr.h"
#include "ScreenBase.h"
#include "Vehicle.h"
#include "GuiDevice.h"

#include <FL/Fl_Text_Buffer.H>
#include <FL/Fl_Text_Display.H>

#define DES_FILE 0
#define XDDM_FILE 1

using namespace std;

class ClippingScreen : public BasicScreen
{
public:
	ClippingScreen( ScreenMgr* mgr );
    virtual ~ClippingScreen();

    void Show();
    void Hide();
    bool Update();

    void CallBack( Fl_Widget *w );
    static void staticScreenCB( Fl_Widget *w, void* data )
    {
        ( ( ClippingScreen* )data )->CallBack( w );
    }

    virtual void GuiDeviceCallBack( GuiDevice* device );

    void LoadDrawObjs( vector< DrawObj* > &draw_obj_vec );

protected:

    GroupLayout m_GenLayout;

    SliderAdjRangeInput m_XGTClipSlider;
    SliderAdjRangeInput m_XLTClipSlider;
    SliderAdjRangeInput m_YGTClipSlider;
    SliderAdjRangeInput m_YLTClipSlider;
    SliderAdjRangeInput m_ZGTClipSlider;
    SliderAdjRangeInput m_ZLTClipSlider;

    ToggleButton m_XGTClipToggle;
    ToggleButton m_XLTClipToggle;
    ToggleButton m_YGTClipToggle;
    ToggleButton m_YLTClipToggle;
    ToggleButton m_ZGTClipToggle;
    ToggleButton m_ZLTClipToggle;
};

#endif  // CLIPPINGSCREEN_H
