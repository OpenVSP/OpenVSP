//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// SVGOptionsScreen.h: interface for controlling SVG export options.
//
// Justin Gravett
//////////////////////////////////////////////////////////////////////

#ifndef SVGOPTIONSSCREEN_H
#define SVGOPTIONSSCREEN_H

#include "ScreenMgr.h"
#include "ScreenBase.h"
#include "Vehicle.h"
#include "GuiDevice.h"
#include "SVGUtil.h"

class SVGOptionsScreen : public BasicScreen
{
public:
    SVGOptionsScreen( ScreenMgr* mgr );
    virtual ~SVGOptionsScreen();

    void Show();
    bool Update();

    void CallBack( Fl_Widget *w );
    static void staticScreenCB( Fl_Widget *w, void* data )
    {
        ( ( SVGOptionsScreen* )data )->CallBack( w );
    }
    virtual void CloseCallBack( Fl_Widget *w );
    virtual void GuiDeviceCallBack( GuiDevice* device );

    bool ShowSVGOptionsScreen( int write_set );

    virtual void GetScale( int write_set );

protected:

    GroupLayout m_GenLayout;

    Choice m_LenUnitChoice;
    Choice m_2DViewType;
    SliderAdjRangeInput m_ScaleSlider;

    ToggleButton m_GeomProjectionLineToggle;
    ToggleButton m_TotalProjectionLineToggle;
    SliderAdjRangeInput m_TessSlider;

    ToggleButton m_ManualToggle;
    ToggleButton m_ModelToggle;
    ToggleButton m_NoneToggle;
    ToggleRadioGroup m_ScaleToggle;

    ToggleButton m_XSecToggle;

    GroupLayout m_4LayoutTL;
    GroupLayout m_4LayoutTR;
    GroupLayout m_4LayoutBL;
    GroupLayout m_4LayoutBR;

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
    bool m_ScaleFlag;
};

#endif  // SVGOPTIONSSCREEN_H
