//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//////////////////////////////////////////////////////////////////////

#ifndef PSLICESCREEN_H_
#define PSLICESCREEN_H_

#include "ScreenBase.h"
#include "GuiDevice.h"

#include "StlHelper.h"

#include "Vec3d.h"

using std::string;
using std::vector;

class PSliceScreen : public BasicScreen
{
public:

    PSliceScreen( ScreenMgr* mgr );
    virtual ~PSliceScreen();

    void Show();
    void Hide();
    bool Update();

    virtual void GuiDeviceCallBack( GuiDevice* device );
    void CallBack( Fl_Widget *w );
    virtual void CloseCallBack( Fl_Widget* w );
    static void staticScreenCB( Fl_Widget *w, void* data )
    {
        ( ( PSliceScreen* )data )->CallBack( w );
    }

protected:

    GroupLayout m_MainLayout;
    GroupLayout m_BorderLayout;

    Choice m_AxisChoice;

    ToggleButton m_SetToggle;
    Choice m_SetChoice;

    ToggleRadioGroup m_ModeSetToggleGroup;
    ToggleButton m_ModeToggle;
    Choice m_ModeChoice;

    SliderAdjRangeInput m_NumSlicesInput;
    SliderAdjRangeInput m_StartLocSlider;
    SliderAdjRangeInput m_EndLocSlider;

    StringOutput m_FileSelect;

    ToggleButton m_AutoButton;
    ToggleButton m_MeasureDuctButton;
    TriggerButton m_FileTrigger;
    TriggerButton m_StartSlicingTrigger;

    Fl_Text_Display* m_TextDisplay;
    Fl_Text_Buffer* m_TextBuffer;

    vec3d m_Norm;

private :

    int m_SelectedSetIndex;
    int m_SelectedModeChoice;
    vector < string > m_ModeIDs;
};

#endif /* PSLICESCREEN_H_ */
