#ifndef _VSP_GUI_TEXTURE_MANAGER_SCREEN_H
#define _VSP_GUI_TEXTURE_MANAGER_SCREEN_H

#include "ScreenMgr.h"
#include "ScreenBase.h"
#include "GuiDevice.h"

#include "ManageTextureScreen.h"
#include "textureMgrFlScreen.h"

#include "GraphicEngine.h"
#include "Display.h"
#include "Viewport.h"
#include "Background.h"
#include "GraphicSingletons.h"

namespace VSPGUI
{
class VspSubGlWindow;
}

//class TextureMgrUI;
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
    Choice m_SurfaceChoice;

    TriggerButton m_AddButton;
    TriggerButton m_DeleteButton;

    StringInput m_NameInput;

    Fl_Group *texGLGroup;

    SliderAdjRangeInput m_UPosSlider;
    SliderAdjRangeInput m_WPosSlider;

    SliderAdjRangeInput m_UScaleSlider;
    SliderAdjRangeInput m_WScaleSlider;

    SliderAdjRangeInput m_BrightnessSlider;
    SliderAdjRangeInput m_TransparencySlider;

    ToggleButton m_RepeatTextureToggle;

    ToggleButton m_FlipUToggle;
    ToggleButton m_FlipWToggle;

    ToggleButton m_FlipUReflectedToggle;
    ToggleButton m_FlipWReflectedToggle;

private:

private:

    int m_GeomIndex;
    int m_EditIndex;
    int m_SurfaceIndex;
    int m_LastActiveGeomIndex;

    bool m_ActiveGeomChanged;
    bool m_ThisGuiDeviceWasCalledBack;

    VSPGUI::VspSubGlWindow * m_GlWin;

};
#endif