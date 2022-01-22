#ifndef _VSP_GUI_MEASURE_MANAGER_SCREEN_H
#define _VSP_GUI_MEASURE_MANAGER_SCREEN_H

#include "ScreenBase.h"
#include "DrawObj.h"
#include "GuiDevice.h"

class Ruler;
class ScreenMgr;

class ManageMeasureScreen : public TabScreen
{
public:
    ManageMeasureScreen( ScreenMgr * mgr );
    virtual ~ManageMeasureScreen();

    virtual void Show();
    virtual void Hide();
    virtual bool Update();

    void CallBack( Fl_Widget *w );
    static void staticScreenCB( Fl_Widget *w, void* data )
    {
        ( ( ManageMeasureScreen* )data )->CallBack( w );
    }

    virtual void GuiDeviceCallBack( GuiDevice* device );



    static void Set( vec3d placement, std::string targetGeomId = "" );

    virtual std::string getFeedbackGroupName();

    void LoadDrawObjs( vector< DrawObj* > & draw_obj_vec );

    void UpdatePickList();

protected:


    GroupLayout m_RulerLayout;
    GroupLayout m_ProbeLayout;

    Choice m_RulerLengthUnitChoice;
    Choice m_RulerDirectionChoice;

    SliderAdjRangeInput m_PrecisionSlider;

    TriggerButton m_AddRulerButton;
    TriggerButton m_RemoveRulerButton;
    TriggerButton m_RemoveAllRulersButton;

    TriggerButton m_ShowAllRulersButton;
    TriggerButton m_HideAllRulersButton;

    StringInput m_RulerNameInput;

    TriggerButton m_AttachRulerButton;

    ToggleButton m_VisibleRulerButton;

    Fl_Browser* m_RulerBrowser;

    GeomPicker m_StartGeom;
    Choice m_StartSurfChoice;

    GeomPicker m_EndGeom;
    Choice m_EndSurfChoice;

    SliderAdjRangeInput m_StartUSlider;
    SliderAdjRangeInput m_StartWSlider;
    SliderAdjRangeInput m_EndUSlider;
    SliderAdjRangeInput m_EndWSlider;

    SliderAdjRangeInput m_XOffsetSlider;
    SliderAdjRangeInput m_YOffsetSlider;
    SliderAdjRangeInput m_ZOffsetSlider;

    Output m_DeltaXOutput;
    Output m_DeltaYOutput;
    Output m_DeltaZOutput;

    Output m_DistanceOutput;

    TriggerButton m_AddProbeButton;
    TriggerButton m_RemoveProbeButton;
    TriggerButton m_RemoveAllProbesButton;
    TriggerButton m_ShowAllProbesButton;
    TriggerButton m_HideAllProbesButton;

    Choice m_ProbeLengthUnitChoice;

    StringInput m_ProbeNameInput;

    TriggerButton m_AttachProbeButton;

    ToggleButton m_VisibleProbeButton;

    Fl_Browser* m_ProbeBrowser;

    GeomPicker m_ProbeGeom;
    Choice m_ProbeSurfChoice;

    SliderAdjRangeInput m_ProbeUSlider;
    SliderAdjRangeInput m_ProbeWSlider;

    SliderAdjRangeInput m_ProbeLenSlider;

    SliderAdjRangeInput m_ProbePrecisionSlider;

    Output m_XOutput;
    Output m_YOutput;
    Output m_ZOutput;

    Output m_NXOutput;
    Output m_NYOutput;
    Output m_NZOutput;

    Output m_K1Output;
    Output m_K2Output;
    Output m_KaOutput;
    Output m_KgOutput;

private:

    std::vector<DrawObj> m_PickList;

};
#endif
