//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// ParasiteDragScreen.h: interface for launching VSPAERO.
//
//////////////////////////////////////////////////////////////////////

#ifndef PARASITEDRAGSCREEN_H
#define PARASITEDRAGSCREEN_H

#include "ScreenMgr.h"
#include "ScreenBase.h"
#include "Vehicle.h"

class ParasiteDragScreen;

class ParasiteDragScreen : public TabScreen
{
public:
    ParasiteDragScreen(ScreenMgr* mgr);
    virtual ~ParasiteDragScreen();

    void Show();
    void Hide();
    bool Update();

    void CallBack(Fl_Widget *w);
    static void staticScreenCB(Fl_Widget *w, void* data)
    {
        ((ParasiteDragScreen*)data)->CallBack(w);
    }

    virtual void GuiDeviceCallBack(GuiDevice* device);

    int main_XScrollPosition;
    int main_YScrollPosition;

protected:
};

#endif  // ParasiteDragScreen_H
