//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// FeaPartEditScreen.h FeaPart editor screen.
// Justin Gravett
//
//////////////////////////////////////////////////////////////////////

#if !defined(FEAPARTEDITSCREEN__INCLUDED_)
#define FEAPARTEDITSCREEN__INCLUDED_

#include "ScreenMgr.h"
#include "ScreenBase.h"

class FeaPartEditScreen : public BasicScreen
{
    public:
    FeaPartEditScreen( ScreenMgr* mgr );
    virtual ~FeaPartEditScreen();
    virtual bool Update();
    virtual void GuiDeviceCallBack( GuiDevice* device );
    virtual void CloseCallBack( Fl_Widget *w );

};

#endif // !defined(FEAPARTEDITSCREEN__INCLUDED_)