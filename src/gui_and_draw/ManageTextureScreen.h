#ifndef _VSP_GUI_TEXTURE_MANAGER_SCREEN_H
#define _VSP_GUI_TEXTURE_MANAGER_SCREEN_H

#include "ScreenBase.h"
#include "GuiDevice.h"

namespace VSPGUI
{
class VspSubGlWindow;
}

class TextureMgrUI;
class Texture;

class ManageTextureScreen : public VspScreen
{
public:
    ManageTextureScreen( ScreenMgr * mgr );
    virtual ~ManageTextureScreen();

public:
    virtual void Show();
    virtual void Hide();
    virtual bool Update();

    void CallBack( Fl_Widget * w );
    static void staticCB( Fl_Widget * w, void * data )
    {
        static_cast<ManageTextureScreen *>( data )->CallBack( w );
    }

protected:
    TextureMgrUI * m_TextureMgrUI;

    SliderInput m_UPosSlider;
    SliderInput m_WPosSlider;

    SliderInput m_UScaleSlider;
    SliderInput m_WScaleSlider;

    SliderInput m_TransparencySlider;

    ToggleButton m_FlipUButton;
    ToggleButton m_FlipWButton;

private:
    void UpdateCurrentSelected();
    void ResetCurrentSelected();

private:
    struct CompDropDownItem
    {
        std::string GeomName;
        std::string GeomID;
        int GUIIndex;
    };

    struct TexDropDownItem
    {
        Texture * TexInfo;
        int GUIIndex;
    };

    TexDropDownItem * m_SelectedTexItem;

    VSPGUI::VspSubGlWindow * m_GlWin;

    std::vector<CompDropDownItem> m_CompDropDownList;
    std::vector<TexDropDownItem> m_TexDropDownList;
};
#endif