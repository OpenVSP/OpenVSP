#ifndef _VSP_GUI_TEXTURE_MANAGER_SCREEN_H
#define _VSP_GUI_TEXTURE_MANAGER_SCREEN_H

#include "ScreenMgr.h"
#include "GraphicEngine.h"
#include "Display.h"
#include "Viewport.h"
#include "Background.h"
#include "GraphicSingletons.h"

namespace VSPGUI
{
class VspSubGlWindow;
}

class Texture;

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

    int m_GeomIndex;
    int m_EditIndex;
    int m_LastActiveGeomIndex;

    bool m_ActiveGeomChanged;
    bool m_ThisGuiDeviceWasCalledBack;

    VSPGUI::VspSubGlWindow * m_GlWin;

};
#endif