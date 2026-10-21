//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#ifndef PRIVATEVSP_VECTORSCREENSHOTSCREEN_H
#define PRIVATEVSP_VECTORSCREENSHOTSCREEN_H

#include "ScreenBase.h"
#include "Parm.h"

using namespace std;

class VectorScreenshotScreen : public BasicScreen
{
public:
    VectorScreenshotScreen( ScreenMgr * mgr );
    virtual ~VectorScreenshotScreen();

    void Show();
    bool Update();

    virtual void GuiDeviceCallBack( GuiDevice* device );

    virtual void CallBack( Fl_Widget* w );
    virtual void CloseCallBack( Fl_Widget *w );
    static void staticScreenCB( Fl_Widget *w, void* data )
    {
        ( ( VectorScreenshotScreen* )data )->CallBack( w );
    }

protected:

    GroupLayout m_MainLayout;
    GroupLayout m_BorderLayout;

    StringOutput m_WindowWidth;
    StringOutput m_WindowHeight;

    Choice m_FileTypeChoice;
    ToggleButton m_AutoCropToggle;

    ToggleButton m_ScaleToPageToggle;
    SliderAdjRangeInput m_PageWidthSlider;
    SliderAdjRangeInput m_PageHeightSlider;
    Choice m_PageUnitsChoice;
    StringOutput m_DrawingSize;

    TriggerButton m_CaptureButton;
};

#endif //PRIVATEVSP_VECTORSCREENSHOTSCREEN_H
