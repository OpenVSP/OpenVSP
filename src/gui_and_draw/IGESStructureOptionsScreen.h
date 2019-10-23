//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// IGESStructureOptionsScreen.h: interface for controlling IGES export options.
//
//////////////////////////////////////////////////////////////////////

#ifndef IGESSTRUCTUREOPTIONSSCREEN_H
#define IGESSTRUCTUREOPTIONSSCREEN_H

#include "ScreenMgr.h"
#include "ScreenBase.h"
#include "Vehicle.h"
#include "GuiDevice.h"

class IGESStructureOptionsScreen : public BasicScreen
{
public:
    IGESStructureOptionsScreen( ScreenMgr* mgr );
    virtual ~IGESStructureOptionsScreen();

    void Show();
    bool Update();

    void CallBack( Fl_Widget *w );
    static void staticScreenCB( Fl_Widget *w, void* data )
    {
        ( ( IGESStructureOptionsScreen* )data )->CallBack( w );
    }
    virtual void CloseCallBack( Fl_Widget *w );
    virtual void GuiDeviceCallBack( GuiDevice* device );

    bool ShowIGESOptionsScreen();

protected:

    GroupLayout m_GenLayout;

    Choice m_StructureChoice;

    ToggleButton m_SplitSurfsToggle;
    ToggleButton m_ToCubicToggle;
    SliderAdjRangeInput m_ToCubicTolSlider;

    ToggleButton m_LabelIDToggle;
    ToggleButton m_LabelNameToggle;
    ToggleButton m_LabelSurfNoToggle;
    ToggleButton m_LabelSplitNoToggle;
    Choice m_LabelDelimChoice;

    int m_PrevStructureChoice;
    bool m_PrevSplit;
    bool m_PrevCubic;
    double m_PrevToCubicTol;

    bool m_PrevLabelID;
    bool m_PrevLabelName;
    bool m_PrevLabelSurfNo;
    bool m_PrevLabelSplitNo;
    int m_PrevLabelDelim;

    TriggerButton m_OkButton;
    TriggerButton m_CancelButton;

    bool m_OkFlag;
};

#endif  // IGESSTRUCTUREOPTIONSSCREEN_H
