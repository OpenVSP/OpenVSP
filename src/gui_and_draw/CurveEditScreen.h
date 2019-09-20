#pragma once
//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// CurveEditScreen.h: Based on OpenVSP v2 Curve Edit GUI and v3 propeller blade editor
// Justin Gravett, ESAero, 7/10/19
//////////////////////////////////////////////////////////////////////

#ifndef CURVE_EDIT_H
#define CURVE_EDIT_H

#include "ScreenMgr.h"
#include "ScreenBase.h"

class CurveEditScreen : public BasicScreen
{
public:
    CurveEditScreen( ScreenMgr* mgr );
    virtual ~CurveEditScreen();

    virtual void Show();
    virtual bool Update();
    virtual void CloseCallBack( Fl_Widget* w );
    virtual void GuiDeviceCallBack( GuiDevice* d );

private:

};

#endif // !defined(CURVE_EDIT_H)
