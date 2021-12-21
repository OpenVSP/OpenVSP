//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#ifndef MANAGE_BACKGROUND_SCREEN_H
#define MANAGE_BACKGROUND_SCREEN_H

#include "ScreenBase.h"
#include "Parm.h"

using namespace std;

//class ScreenMgr;
class ManageBackgroundScreen : public BasicScreen
{
public:
    ManageBackgroundScreen( ScreenMgr * mgr );
    virtual ~ManageBackgroundScreen();

    void Show();
    void Hide();
    bool Update();

    virtual void GuiDeviceCallBack( GuiDevice* device );

    virtual void CallBack( Fl_Widget* w );
    virtual void CloseCallBack( Fl_Widget *w );
    static void staticScreenCB( Fl_Widget *w, void* data )
    {
        ( ( ManageBackgroundScreen* )data )->CallBack( w );
    }

protected:

    GroupLayout m_MainLayout;
    GroupLayout m_BorderLayout;
    GroupLayout m_ColorLayout;
    GroupLayout m_ImageLayout;
    GroupLayout m_PreserveAspectLayout;

    Fl_Color_Chooser* colorChooser;

    ToggleButton m_Color;
    ToggleButton m_Image;

    StringOutput m_FileOutput;
    TriggerButton m_FileSelect;

    SliderAdjRangeInput m_WScale;
    SliderAdjRangeInput m_HScale;
    ToggleButton m_PreserveAspect;

    SliderAdjRangeInput m_XOffset;
    SliderAdjRangeInput m_YOffset;

    TriggerButton m_ResetDefaults;

    string m_ImageFile;

};
#endif //MANAGE_BACKGROUND_SCREEN_H
