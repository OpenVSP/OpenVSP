#ifndef _VSP_GUI_LIGHTING_MANAGER_SCREEN_H
#define _VSP_GUI_LIGHTING_MANAGER_SCREEN_H

#include "ScreenBase.h"
#include "GuiDevice.h"

#include <vector>

class ManageLightingScreen : public BasicScreen
{
public:
    ManageLightingScreen( ScreenMgr * mgr );
    virtual ~ManageLightingScreen();

    virtual void Show();
    virtual bool Update();

    virtual void GuiDeviceCallBack( GuiDevice* device );

    void LoadDrawObjs(vector< DrawObj* > & draw_obj_vec);

protected:

    GroupLayout m_GenLayout;

    SliderAdjRangeInput m_XPosSlider;
    SliderAdjRangeInput m_YPosSlider;
    SliderAdjRangeInput m_ZPosSlider;

    SliderInput m_AmbSlider;
    SliderInput m_DiffSlider;
    SliderInput m_SpecSlider;

    Choice m_LightChoice;

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
