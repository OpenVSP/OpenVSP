//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// MaterialEditScreen.h Material editor screen.
// Rob McDonald
//
//////////////////////////////////////////////////////////////////////

#if !defined(MATERIALEDITSCREEN__INCLUDED_)
#define MATERIALEDITSCREEN__INCLUDED_

#include "ScreenMgr.h"
#include "ScreenBase.h"

class MaterialEditScreen : public BasicScreen
{
public:
    MaterialEditScreen( ScreenMgr* mgr );
    virtual ~MaterialEditScreen();
    virtual bool Update();
    virtual void GuiDeviceCallBack( GuiDevice* device );
    virtual void CloseCallBack( Fl_Widget *w );

    GroupLayout m_GenLayout;

    ColorPicker m_AmbientColorPicker;
    ColorPicker m_DiffuseColorPicker;
    ColorPicker m_SpecularColorPicker;
    ColorPicker m_EmissiveColorPicker;

    SliderInput m_AlphaSlider;
    SliderInput m_ShininessSlider;

    StringInput m_MaterialNameInput;
    TriggerButton m_SaveApplyButton;
    TriggerButton m_CancelButton;

    string m_OrigColor;
};

#endif // !defined(MATERIALEDITSCREEN__INCLUDED_)
