//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//MeshScreen.h: UI for MeshGeom
//////////////////////////////////////////////////////////////////////

#ifndef MESHSCREEN_H_
#define MESHSCREEN_H_

#include "ScreenBase.h"
#include "GuiDevice.h"

#include <FL/Fl.H>

class MeshScreen : public GeomScreen
{
public:
    MeshScreen( ScreenMgr* mgr );
    virtual ~MeshScreen()                           {}

    virtual void Show();
    virtual bool Update();

    virtual void CallBack( Fl_Widget *w );
    virtual void GuiDeviceCallBack( GuiDevice* d );

protected:
//  MeshUI* m_MeshUI;

    GroupLayout m_OtherLayout;
    TriggerButton m_ConvertButton;

    ToggleButton m_ViewMeshToggle;
    ToggleButton m_ViewSliceToggle;

};

#endif /* MESHSCREEN_H_ */
