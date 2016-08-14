#if !defined(HINGESCREEN__INCLUDED_)
#define HINGESCREEN__INCLUDED_

#include "ScreenBase.h"
#include "GuiDevice.h"

#include <FL/Fl.H>

class HingeScreen : public GeomScreen
{
public:
    HingeScreen( ScreenMgr* mgr );
    virtual ~HingeScreen()                          {}

    virtual void Show();
    virtual bool Update();

    virtual void CallBack( Fl_Widget *w );

    virtual void GuiDeviceCallBack( GuiDevice* device );

protected:

    GroupLayout m_BaseOrientationLayout;

    ToggleButton m_OrientRotToggle;
    ToggleButton m_OrientVecToggle;

    ToggleRadioGroup m_OrientTypeGroup;

    GroupLayout m_VectorLayout;

    GroupLayout m_PrimaryDirLayout;

    ToggleButton m_PrimaryXToggle;
    ToggleButton m_PrimaryYToggle;
    ToggleButton m_PrimaryZToggle;
    ToggleRadioGroup m_PrimaryDirGroup;

    GroupLayout m_SecondaryDirLayout;

    ToggleButton m_SecondaryXToggle;
    ToggleButton m_SecondaryYToggle;
    ToggleButton m_SecondaryZToggle;
    ToggleRadioGroup m_SecondaryDirGroup;

    GroupLayout m_PrimVecLayout;

    ToggleButton m_PrimVecAbsToggle;
    ToggleButton m_PrimVecRelToggle;
    ToggleRadioGroup m_PrimVecAbsRelToggle;

    SliderAdjRange2Input m_PrimXVecSlider;
    SliderAdjRange2Input m_PrimYVecSlider;
    SliderAdjRange2Input m_PrimZVecSlider;

    GroupLayout m_PrimOffLayout;

    ToggleButton m_PrimOffAbsToggle;
    ToggleButton m_PrimOffRelToggle;
    ToggleRadioGroup m_PrimOffAbsRelToggle;

    SliderAdjRange2Input m_PrimXOffSlider;
    SliderAdjRange2Input m_PrimYOffSlider;
    SliderAdjRange2Input m_PrimZOffSlider;

    GroupLayout m_PrimUWLayout;

    SliderAdjRangeInput m_PrimUSlider;
    SliderAdjRangeInput m_PrimWSlider;

    Choice m_PrimTypeChoice;

    GroupLayout m_SecVecLayout;

    ToggleButton m_SecVecAbsToggle;
    ToggleButton m_SecVecRelToggle;
    ToggleRadioGroup m_SecVecAbsRelToggle;

    ToggleButton m_SecondaryVecXToggle;
    ToggleButton m_SecondaryVecYToggle;
    ToggleButton m_SecondaryVecZToggle;
    ToggleRadioGroup m_SecondaryVecDirGroup;

    GroupLayout m_MotionLayout;

    SliderAdjRangeInput m_JointTranslateSlider;
    ToggleButton m_JointTranslateToggle;
    TriggerButton m_JointTranslateRngButton;
    SliderAdjRangeInput m_JointTransMinSlider;
    ToggleButton m_JointTransMinToggle;
    TriggerButton m_JointTransMinSetButton;
    SliderAdjRangeInput m_JointTransMaxSlider;
    ToggleButton m_JointTransMaxToggle;
    TriggerButton m_JointTransMaxSetButton;

    SliderAdjRangeInput m_JointRotateSlider;
    ToggleButton m_JointRotateToggle;
    TriggerButton m_JointRotateRngButton;
    SliderAdjRangeInput m_JointRotMinSlider;
    ToggleButton m_JointRotMinToggle;
    TriggerButton m_JointRotMinSetButton;
    SliderAdjRangeInput m_JointRotMaxSlider;
    ToggleButton m_JointRotMaxToggle;
    TriggerButton m_JointRotMaxSetButton;
};


#endif // !defined(HINGESCREEN__INCLUDED_)
