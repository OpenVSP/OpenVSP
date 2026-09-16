//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// CloneScreen.h: UI for Clone Geom
// Rob McDonald
//
//////////////////////////////////////////////////////////////////////

#if !defined(CLONESCREEN__INCLUDED_)
#define CLONESCREEN__INCLUDED_

#include "ScreenBase.h"
#include "GuiDevice.h"

#include <FL/Fl.H>

class CloneScreen : public GeomScreen
{
public:
    CloneScreen( ScreenMgr* mgr );
    virtual ~CloneScreen()                            {}

    virtual void Show();
    virtual bool Update();

    virtual void CallBack( Fl_Widget *w );
    virtual void GuiDeviceCallBack( GuiDevice* d );

protected:

    // The Clone tab: what it copies, naming, and joint deflection.
    GroupLayout m_CloneLayout;

    Choice m_OriginalChoice;
    vector <string> m_CompVec;

    SliderAdjRangeInput m_JointTranslateSlider;
    TriggerButton m_JointTranslateRngButton;
    SliderAdjRangeInput m_JointRotateSlider;
    TriggerButton m_JointRotateRngButton;

    ToggleButton m_CloneSetsButton;
    ToggleButton m_CloneSymButton;
    ToggleButton m_CloneXFormButton;
    ToggleButton m_CloneAttachButton;
    ToggleButton m_CloneAppearanceButton;
    ToggleButton m_CloneNegativeVolumeButton;
    ToggleButton m_CloneMassPropsButton;
    ToggleButton m_CloneSubSurfsButton;
    ToggleButton m_CloneJointButton;
    ToggleButton m_AutoNameButton;
    StringInput m_NameSuffixInput;
    TriggerButton m_ReplaceButton;
};


#endif // !defined(CLONESCREEN__INCLUDED_)
