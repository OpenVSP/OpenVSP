//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#ifndef PRIVATEVSP_SCREENSHOTSCREEN_H
#define PRIVATEVSP_SCREENSHOTSCREEN_H

#include "ScreenBase.h"
#include "Parm.h"

using namespace std;

//class ScreenMgr;
class ScreenshotScreen : public BasicScreen
{
public:
    ScreenshotScreen( ScreenMgr * mgr );
    virtual ~ScreenshotScreen();

    void Show();
    void Hide();
    bool Update();

    virtual void GuiDeviceCallBack( GuiDevice* device );

    virtual void CallBack( Fl_Widget* w );
    virtual void CloseCallBack( Fl_Widget *w );
    static void staticScreenCB( Fl_Widget *w, void* data )
    {
        ( ( ScreenshotScreen* )data )->CallBack( w );
    }

protected:

    GroupLayout m_MainLayout;
    GroupLayout m_BorderLayout;
    GroupLayout m_CurrentViewportSizeLayout;
    GroupLayout m_ViewportSizeLayout;
    GroupLayout m_TransparentLayout;

    StringOutput m_CurrentWidth;
    StringOutput m_CurrentHeight;

    RadioButton m_SelectRatio;
    RadioButton m_SelectWidth;
    RadioButton m_SelectHeight;
    SliderAdjRangeInput m_NewRatio;
    SliderAdjRangeInput m_NewWidth;
    SliderAdjRangeInput m_NewHeight;

    TriggerButton m_SetToCurrentSize;

    ToggleButton m_TransparentBG;

    TriggerButton m_CapturePNG;

    bool m_framebufferSupported;
    bool m_showedOpenGLErrorMessage;

};

#endif //PRIVATEVSP_SCREENSHOTSCREEN_H
