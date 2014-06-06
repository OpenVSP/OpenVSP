//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#if !defined(VSPDEGENGEOMSCREEN__INCLUDED_)
#define VSPDEGENGEOMSCREEN__INCLUDED_

#include "ScreenBase.h"
#include "GuiDevice.h"

#include <FL/Fl.H>
#include "degenGeomFlScreen.h"

using std::string;
using std::vector;

class DegenGeomScreen : public VspScreen
{
public:

    DegenGeomScreen( ScreenMgr* mgr );
    virtual ~DegenGeomScreen();
    void Show();
    void Hide();
    bool Update();
    void LoadSetChoice();

    void CallBack( Fl_Widget *w );
    static void staticScreenCB( Fl_Widget *w, void* data )
    {
        ( ( DegenGeomScreen* )data )->CallBack( w );
    }

protected:

    DegenGeomUI* m_DegenGeomUI;
    Fl_Text_Buffer* m_textBuffer;

    int m_SelectedSetIndex;
};


#endif

