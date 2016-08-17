#ifndef OPENVSP_MULTTRANSSCREEN_H
#define OPENVSP_MULTTRANSSCREEN_H

#include "ScreenBase.h"
#include "GuiDevice.h"

#include <vector>

class MultTransScreen : public TabScreen
{
public:
    MultTransScreen( ScreenMgr * mgr );
    virtual ~MultTransScreen();

    virtual void Show();
    virtual bool Update();

    virtual void GuiDeviceCallBack( GuiDevice* device );

protected:

    GroupLayout m_TransTabMainLayout;
    GroupLayout m_TransTabGenLayout;

    SliderAdjRangeInput m_XLoc;
    SliderAdjRangeInput m_YLoc;
    SliderAdjRangeInput m_ZLoc;

    SliderAdjRangeInput m_XRot;
    SliderAdjRangeInput m_YRot;
    SliderAdjRangeInput m_ZRot;

    SliderInput m_Scale;
    ToggleButton m_ApplyScaleToTranslations;

    TriggerButton m_ResetButton;
    TriggerButton m_AcceptButton;

    // Material Tab members
    GroupLayout m_MaterialTabMainLayout;
    GroupLayout m_MaterialTabGenLayout;
    Choice m_MaterialChoice;
    TriggerButton m_CustomMaterialButton;
    ColorPicker m_ColorPicker;

    // Methods for materials
    virtual void UpdateMaterialNames();

private:
};

#endif //OPENVSP_MULTTRANSSCREEN_H
