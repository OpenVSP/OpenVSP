//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//////////////////////////////////////////////////////////////////////

#ifndef _VSP_GUI_VIEW_MANAGER_SCREEN_H
#define _VSP_GUI_VIEW_MANAGER_SCREEN_H

#define _USE_MATH_DEFINES
#include <cmath>

#include "ScreenBase.h"
#include "Parm.h"

using namespace std;

//class ScreenMgr;
class ManageViewScreen : public BasicScreen
{
public:
    ManageViewScreen( ScreenMgr * mgr );
    virtual ~ManageViewScreen();

    void Show();
    void Hide();
    bool Update();

    virtual void GuiDeviceCallBack( GuiDevice* device );

    virtual void CallBack( Fl_Widget* w );
    virtual void CloseCallBack( Fl_Widget *w );
    static void staticScreenCB( Fl_Widget *w, void* data )
    {
        ( ( ManageViewScreen* )data )->CallBack( w );
    }

protected:

    GroupLayout m_MainLayout;
    GroupLayout m_BorderLayout;

    SliderAdjRangeInput m_ViewportSizeX;
    SliderAdjRangeInput m_ViewportSizeY;

    TriggerButton m_SetDefaultViewportSize;

    SliderAdjRangeInput m_CORX;
    SliderAdjRangeInput m_CORY;
    SliderAdjRangeInput m_CORZ;

    TriggerButton m_PickLookAtBtn;
    TriggerButton m_ResetOriginLookAtBtn;

    SliderAdjRangeInput m_PanXPos;
    SliderAdjRangeInput m_PanYPos;
    SliderAdjRangeInput m_Zoom;

    //===== Attempt at Euler Angle Rotation =====//
    SliderAdjRangeInput m_XRotation;
    SliderAdjRangeInput m_YRotation;
    SliderAdjRangeInput m_ZRotation;

    SliderAdjRangeInput m_AxisLenSlider;
    SliderAdjRangeInput m_TextSizeSlider;

    TriggerButton m_ResetCamera;

};

#endif
