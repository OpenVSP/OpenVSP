//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// GuiDeviceEnums.h:
// J.R Gloudemans
//
//////////////////////////////////////////////////////////////////////

#ifndef VSP_GUIDEVICEENUMS_H
#define VSP_GUIDEVICEENUMS_H


//  ParmButton      (Parm)          Button with label.
//  Input           (Parm/IntParm)  Number input.
//  Slider          (Parm/IntParm)  Normal slider
//  SliderAdjRange  (Parm/IntParm)  Slider with additional buttons to adjust range
//  Check Button    (BoolParm)      Toggle type button with check mark
//  CheckButtonBit  (IntParm)       Check button with val (0 or val)
//  RadioButton     (IntParm)       Round radio button
//  Toggle Button   (BoolParm)      Toggle type button with light
//  ToggleRadioGroup (IntParm)      Group of toggle buttons that act like radio buttons.
//  TriggerButton   (None)          Button that triggers a GuiDeviceCallBack to VspScreen (no Parm)
//  Counter         (Parm)          Fltk counter widget
//  Choice          (IntParm)       Fltk choice widget
//  SliderInput     (Parm)          Combo of Slider (or LogSlider) and Input and optional Parm Button
//  SliderAdjRangeInput (Parm)      Combo of SliderAdjRange and Input and optional Parm Button
//  SliderAdjRange2Input (2 Parms)  Combo of SliderAdjRange and two Inputs and optional Parm Button.
//                                  Display two different parms in each window (only one active at a time)
//  FractParmSlider (FractParm)     Combo of SliderAdjRange and two Inputs and optional Parm Button.
//                                      The second input display the fractional value.
//  StringInput     (None)          Set and display text (string) triggers a GuiDeviceCallBack to VspScreen
//  IndexSelector   (IntParm)       Display and set integers. Composed of input and 5 arrow buttons.
//  ColorPicker     (None)          Display, edit and pick colors.  Composed of buttons and sliders.



enum GDEV
{
    GDEV_TAB,
    GDEV_SCROLL_TAB,
    GDEV_GROUP,
    GDEV_PARM_BUTTON,
    GDEV_INPUT,
    GDEV_OUTPUT,
    GDEV_SLIDER,
    GDEV_SLIDER_ADJ_RANGE,
    GDEV_CHECK_BUTTON,
    GDEV_CHECK_BUTTON_BIT,
    GDEV_RADIO_BUTTON,
    GDEV_TOGGLE_BUTTON,
    GDEV_TOGGLE_RADIO_GROUP,
    GDEV_TRIGGER_BUTTON,
    GDEV_COUNTER,
    GDEV_CHOICE,
    GDEV_ADD_CHOICE_ITEM,
    GDEV_SLIDER_INPUT,
    GDEV_SLIDER_ADJ_RANGE_INPUT,
    GDEV_SLIDER_ADJ_RANGE_TWO_INPUT,
    GDEV_FRACT_PARM_SLIDER,
    GDEV_STRING_INPUT,
    GDEV_INDEX_SELECTOR,
    GDEV_COLOR_PICKER,
    GDEV_YGAP,
    GDEV_DIVIDER_BOX,
    GDEV_BEGIN_SAME_LINE,
    GDEV_END_SAME_LINE,
    GDEV_FORCE_WIDTH,
    GDEV_SET_FORMAT,
};

class GuiDef
{
public:

    int m_Type;
    string m_Label;
    string m_ParmName;
    string m_GroupName;
    double m_Range;

};

class GuiUpdate
{
public:

    int m_GuiID;
    string m_ParmID;
};


#endif


