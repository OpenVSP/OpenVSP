//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#ifndef PROJECTIONSCREEN_H
#define PROJECTIONSCREEN_H

#include "ScreenBase.h"
#include "ScreenMgr.h"
#include "GuiDevice.h"

using namespace std;

class ProjectionScreen : public BasicScreen
{
public:
    ProjectionScreen( ScreenMgr * mgr );
    virtual ~ProjectionScreen();

    void Show();
    void Hide();
    bool Update();

    virtual void GuiDeviceCallBack( GuiDevice* device );

    virtual void CallBack( Fl_Widget* w );
    virtual void CloseCallBack( Fl_Widget *w );
    static void staticScreenCB( Fl_Widget *w, void* data )
    {
        ( ( ProjectionScreen* )data )->CallBack( w );
    }

    void LoadSetChoice( Choice & c, int & index );

protected:

    GroupLayout m_MainLayout;
    GroupLayout m_BorderLayout;

    Choice m_TargetSet;
    GeomPicker m_TargetGeom;

    ToggleRadioGroup m_TargetTypeGroup;
    ToggleButton m_TargetTypeSet;
    ToggleButton m_TargetTypeGeom;

    Choice m_BoundarySet;
    GeomPicker m_BoundaryGeom;

    ToggleRadioGroup m_BoundaryTypeGroup;
    ToggleButton m_BoundaryTypeNone;
    ToggleButton m_BoundaryTypeSet;
    ToggleButton m_BoundaryTypeGeom;



    ToggleRadioGroup m_DirectionTypeGroup;

    ToggleButton m_DirectionTypeX;
    ToggleButton m_DirectionTypeY;
    ToggleButton m_DirectionTypeZ;
    ToggleButton m_DirectionTypeGeom;
    ToggleButton m_DirectionTypeVector;

    SliderInput m_XSlider;
    SliderInput m_YSlider;
    SliderInput m_ZSlider;

    GeomPicker m_DirectionGeom;


    StringOutput m_FileSelect;
    TriggerButton m_FileTrigger;

    StringOutput m_AreaOutput;


    TriggerButton m_Execute;

};
#endif //PROJECTIONSCREEN_H
