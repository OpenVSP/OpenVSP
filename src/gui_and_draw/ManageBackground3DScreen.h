#ifndef _VSP_GUI_BACKGROUND3D_MANAGER_SCREEN_H
#define _VSP_GUI_BACKGROUND3D_MANAGER_SCREEN_H

#include "ScreenBase.h"
#include "DrawObj.h"
#include "GuiDevice.h"

class ScreenMgr;

class ManageBackground3DScreen : public BasicScreen
{
public:
    ManageBackground3DScreen( ScreenMgr * mgr );
    virtual ~ManageBackground3DScreen();

    virtual void Show();
    virtual void Hide();
    virtual bool Update();

    virtual void GuiDeviceCallBack( GuiDevice* device );

    virtual void CallBack( Fl_Widget *w );
    virtual void CloseCallBack( Fl_Widget *w );
    static void staticScreenCB( Fl_Widget *w, void* data )
    {
        ( ( ManageBackground3DScreen* )data )->CallBack( w );
    }

    void LoadDrawObjs( vector< DrawObj* > & draw_obj_vec );

protected:

    GroupLayout m_MainLayout;
    GroupLayout m_BorderLayout;
    GroupLayout m_BgLayout;

    ColResizeBrowser* m_Background3DBrowser;

    TriggerButton m_AddBackground3DButton;
    TriggerButton m_RemoveBackground3DButton;
    TriggerButton m_RemoveAllBackground3DsButton;
    TriggerButton m_ShowAllBackground3DsButton;
    TriggerButton m_HideAllBackground3DsButton;

    StringInput m_Background3DNameInput;

    ToggleButton m_VisibleBackground3DButton;
    ToggleButton m_RearVisibleBackground3DButton;
    ToggleButton m_AlignVisibleBackground3DButton;
    SliderAdjRangeInput m_VisTolSlider;

    StringOutput m_FileOutput;
    TriggerButton m_FileSelect;

    Choice m_DirectionChoice;

    SliderAdjRangeInput m_NXSlider;
    SliderAdjRangeInput m_NYSlider;
    SliderAdjRangeInput m_NZSlider;

    SliderAdjRangeInput m_UpXSlider;
    SliderAdjRangeInput m_UpYSlider;
    SliderAdjRangeInput m_UpZSlider;

    ToggleRadioGroup m_ScaleGroup;

    ToggleButton m_WScaleToggleButton;
    ToggleButton m_HScaleToggleButton;
    ToggleButton m_ResolutionScaleToggleButton;

    SliderAdjRangeInput m_WSlider;
    SliderAdjRangeInput m_HSlider;
    SliderAdjRangeInput m_ResolutionSlider;

    Choice m_HAlignChoice;
    Choice m_VAlignChoice;
    SliderAdjRangeInput m_ImageXSlider;
    SliderAdjRangeInput m_ImageYSlider;

    Output m_ImageWOutput;
    Output m_ImageHOutput;

    ToggleButton m_ImageFlipLRToggleButton;
    ToggleButton m_ImageFlipUDToggleButton;
    ToggleButton m_ImageAutoTransparentToggleButton;

    Choice m_ImageRotChoice;

    SliderAdjRangeInput m_XSlider;
    SliderAdjRangeInput m_YSlider;
    SliderAdjRangeInput m_ZSlider;

    Choice m_DepthChoice;

};
#endif
