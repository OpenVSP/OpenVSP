#ifndef OPENVSP_MULTTRANSSCREEN_H
#define OPENVSP_MULTTRANSSCREEN_H

#include "ScreenBase.h"
#include "GuiDevice.h"

#include <vector>

class MultTransScreen : public BasicScreen
{
public:
    MultTransScreen( ScreenMgr * mgr );
    virtual ~MultTransScreen();

    virtual void Show();
    virtual bool Update();

    virtual void GuiDeviceCallBack( GuiDevice* device );

    void LoadDrawObjs(vector< DrawObj* > & draw_obj_vec);

protected:

    GroupLayout m_GenLayout;

    SliderAdjRangeInput m_XLoc;
    SliderAdjRangeInput m_YLoc;
    SliderAdjRangeInput m_ZLoc;

    SliderAdjRangeInput m_XRot;
    SliderAdjRangeInput m_YRot;
    SliderAdjRangeInput m_ZRot;

    SliderInput m_Scale;

private:
};

#endif //OPENVSP_MULTTRANSSCREEN_H
