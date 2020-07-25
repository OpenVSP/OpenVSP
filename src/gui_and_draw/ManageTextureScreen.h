#ifndef _VSP_GUI_TEXTURE_MANAGER_SCREEN_H
#define _VSP_GUI_TEXTURE_MANAGER_SCREEN_H

#include "ScreenMgr.h"

class ManageTextureScreen : public BasicScreen
{
public:
    ManageTextureScreen( ScreenMgr * mgr );
    virtual ~ManageTextureScreen();

public:
    virtual void Show();
    virtual void Hide();
    virtual bool Update();

    virtual void GuiDeviceCallBack( GuiDevice* device );
    void CallBack( Fl_Widget * w );
    virtual void CloseCallBack( Fl_Widget* w );

protected:

    GroupLayout m_MainLayout;
    GroupLayout m_BorderLayout;
    GroupLayout m_ActiveTextureLayout;

    Choice m_GeomChoice;
    Choice m_EditChoice;

    TriggerButton m_AddButton;
    TriggerButton m_DeleteButton;

    StringInput m_NameInput;

    Fl_Group *texGLGroup;

    SliderAdjRangeInput m_UPosSlider;
    SliderAdjRangeInput m_WPosSlider;

    SliderAdjRangeInput m_UScaleSlider;
    SliderAdjRangeInput m_WScaleSlider;

    SliderAdjRangeInput m_TransparencySlider;

    ToggleButton m_FlipUToggle;
    ToggleButton m_FlipWToggle;

private:

    string m_SelectedGeomID;
    vector < string > m_GeomIDVec; // Does not include Geoms that don't support textures
    int m_SelectedTextureIndex;

    VSPGUI::VspSubGlWindow * m_GlWin;

};
#endif