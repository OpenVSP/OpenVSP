#ifndef PARMLINKSCREENPROG_H
#define PARMLINKSCREENPROG_H

#include "ScreenBase.h"
#include "GuiDevice.h"
#include "ScreenMgr.h"
#include "Vehicle.h"
#include "DesignVarMgr.h"
#include <vector>

#include <FL/Fl.H>
#include <FL/Fl_File_Chooser.H>

class ParmLinkScreenProg : public BasicScreen
{
public:
    ParmLinkScreenProg( ScreenMgr * mgr );
    virtual ~ParmLinkScreenProg();

    virtual void Show();
    virtual bool Update();

    virtual void GuiDeviceCallBack( GuiDevice* device );
    virtual void CallBack( Fl_Widget* w );
    static void staticScreenCB( Fl_Widget *w, void* data )
    {
        ( ( ParmLinkScreenProg* )data )->CallBack( w );
    }

    void LoadDrawObjs(vector< DrawObj* > & draw_obj_vec);

protected:

    // Main group
    GroupLayout m_GenLayout;

    // Subgroups
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

    // Used to activate and deactivate sliders
    ToggleButton m_OffsetTog;
    ToggleButton m_ScaleTog;
    ToggleButton m_LowerTog;
    ToggleButton m_UpperTog;

    SliderInput m_OffsetSlider;
    SliderInput m_ScaleSlider;
    SliderInput m_LowerLimitSlider;
    SliderInput m_UpperLimitSlider;

    Fl_Browser* m_LinkBrowser;

private:
    int m_CurrentSelected;

    DrawObj m_LightingDO;
};
#endif
