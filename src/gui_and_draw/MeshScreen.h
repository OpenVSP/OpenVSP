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

protected:
//  MeshUI* m_MeshUI;

};

#endif /* MESHSCREEN_H_ */
