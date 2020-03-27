#ifndef PARMLINKSCREEN_H
#define PARMLINKSCREEN_H

#include "ScreenBase.h"
#include "GuiDevice.h"
#include "ScreenMgr.h"
#include "Vehicle.h"
#include "DesignVarMgr.h"
#include <vector>

#include <FL/Fl.H>
#include <FL/Fl_File_Chooser.H>

class ParmLinkScreen : public BasicScreen
{
public:
    ParmLinkScreen( ScreenMgr * mgr );
    virtual ~ParmLinkScreen();

    virtual void Show();
    virtual bool Update();

    virtual void GuiDeviceCallBack( GuiDevice* device );
    virtual void CallBack( Fl_Widget* w );
    static void staticScreenCB( Fl_Widget *w, void* data )
    {
        ( ( ParmLinkScreen* )data )->CallBack( w );
    }

protected:

    // Main group
    GroupLayout m_MainLayout;

    // Subgroups
    GroupLayout m_GenLayout;
    GroupLayout m_ParmAGroup;
    GroupLayout m_ParmBGroup;
    GroupLayout m_LinkGroup;
    GroupLayout m_ConstraintsButtons;

    ParmPicker m_ParmAPicker;
    ParmPicker m_ParmBPicker;

    TriggerButton m_LinkConts;
    TriggerButton m_LinkGroups;
    TriggerButton m_AddLink;
    TriggerButton m_DeleteLink;
    TriggerButton m_DeleteAllLinks;

    TriggerButton m_ASort;
    TriggerButton m_BSort;

    // Used to activate and deactivate sliders
    ToggleButton m_OffsetTog;
    ToggleButton m_ScaleTog;
    ToggleButton m_LowerTog;
    ToggleButton m_UpperTog;

    SliderAdjRangeInput m_OffsetSlider;
    SliderAdjRangeInput m_ScaleSlider;
    SliderAdjRangeInput m_LowerLimitSlider;
    SliderAdjRangeInput m_UpperLimitSlider;

    // Pointer for the widths of each column in the browser to support resizing
    int* m_ColWidths;

    Fl_Browser* m_LinkBrowser;
};
#endif
