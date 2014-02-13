//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//////////////////////////////////////////////////////////////////////

#ifndef MASSPROPSCREEN_H_
#define MASSPROPSCREEN_H_

#include "ScreenBase.h"
#include "GuiDevice.h"

#include "massPropFlScreen.h"

using std::string;
using std::vector;

class MassPropScreen : public VspScreen
{
public:

    MassPropScreen( ScreenMgr* mgr );
    virtual ~MassPropScreen();
    void Show();
    void Hide();
    bool Update();
    void LoadSetChoice();
    static void staticScreenCB( Fl_Widget *w, void* data )
    {
        ( ( MassPropScreen* )data )->CallBack( w );
    }
    void CallBack( Fl_Widget *w );


protected:

    MassPropUI* m_MassPropUI;
    int m_SelectedSetIndex;

};

#endif /* MASSPROPSCREEN_H_ */
