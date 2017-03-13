//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// DXFOptionsScreen.h: interface for controlling DXF export options.
//
// Justin Gravett
//////////////////////////////////////////////////////////////////////

#ifndef DXFOPTIONSSCREEN_H
#define DXFOPTIONSSCREEN_H

#include "ScreenMgr.h"
#include "ScreenBase.h"
#include "Vehicle.h"
#include "GuiDevice.h"

class DXFOptionsScreen : public BasicScreen
{
public:
    DXFOptionsScreen( ScreenMgr* mgr );
    virtual ~DXFOptionsScreen();

    void Show();
    bool Update();

    void CallBack( Fl_Widget *w );
    static void staticScreenCB( Fl_Widget *w, void* data )
    {
        ( ( DXFOptionsScreen* )data )->CallBack( w );
    }
    virtual void CloseCallBack( Fl_Widget *w );
    virtual void GuiDeviceCallBack( GuiDevice* device );

    bool ShowDXFOptionsScreen();

protected:

    GroupLayout m_GenLayout;

    GroupLayout m_4LayoutTL;
    GroupLayout m_4LayoutTR;
    GroupLayout m_4LayoutBL;
    GroupLayout m_4LayoutBR;

    Choice m_LenUnitChoice;
    Choice m_2DViewType;

    ToggleButton m_ProjectionLineToggle;
    SliderAdjRangeInput m_TessSlider;

    ToggleButton m_3DToggle;
    ToggleButton m_2DToggle;
    ToggleRadioGroup m_2D3DGroup;
    ToggleButton m_XSecToggle;
    ToggleButton m_ColorToggle;

    Choice m_4ViewChoice1;
    Choice m_4ViewChoice2;
    Choice m_4ViewChoice3;
    Choice m_4ViewChoice4;
    Choice m_4RotChoice1;
    Choice m_4RotChoice2;
    Choice m_4RotChoice3;
    Choice m_4RotChoice4;

    TriggerButton m_OkButton;
    TriggerButton m_CancelButton;

    bool m_OkFlag;
};

#endif  // DXFOPTIONSSCREEN_H
