#ifndef _VSP_GUI_LIGHTING_MANAGER_SCREEN_H
#define _VSP_GUI_LIGHTING_MANAGER_SCREEN_H

#include "ScreenBase.h"
#include "GuiDevice.h"
#include "lightingScreen.h"

#include <vector>

class ManageLightingScreen : public VspScreen
{
public:
    ManageLightingScreen( ScreenMgr * mgr );
    virtual ~ManageLightingScreen();

public:
    virtual void Show();
    virtual void Hide();
    virtual bool Update();

    void CallBack( Fl_Widget * w );
    static void staticCB( Fl_Widget * w, void * data ) { static_cast<ManageLightingScreen *>( data )->CallBack( w ); }

public:
    void LoadDrawObjs(vector< DrawObj* > & draw_obj_vec);

protected:
    LightingUI * m_LightingUI;

    SliderInput m_XPosSlider;
    SliderInput m_YPosSlider;
    SliderInput m_ZPosSlider;

    Slider m_AmbSlider;
    Slider m_DiffSlider;
    Slider m_SpecSlider;

    ToggleButton m_LightButton0;
    ToggleButton m_LightButton1;
    ToggleButton m_LightButton2;
    ToggleButton m_LightButton3;
    ToggleButton m_LightButton4;
    ToggleButton m_LightButton5;
    ToggleButton m_LightButton6;
    ToggleButton m_LightButton7;

private:
    int m_CurrentSelected;

    DrawObj m_LightingDO;
};
#endif