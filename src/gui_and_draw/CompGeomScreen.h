//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#if !defined(VSPCOMPGEOMSCREEN__INCLUDED_)
#define VSPCOMPGEOMSCREEN__INCLUDED_

#include "ScreenBase.h"
#include "GuiDevice.h"

#include <FL/Fl.H>
#include "compGeomFlScreen.h"

using std::string;
using std::vector;

class CompGeomScreen : public VspScreen
{
public:

    CompGeomScreen( ScreenMgr* mgr );
    virtual ~CompGeomScreen();
    void Show();
    void Hide();
    bool Update();
    void LoadSetChoice();

    void CallBack( Fl_Widget *w );
    static void staticScreenCB( Fl_Widget *w, void* data )
    {
        ( ( CompGeomScreen* )data )->CallBack( w );
    }

protected:

    CompGeomUI* m_CompGeomUI;
    Fl_Text_Buffer* m_textBuffer;

    int m_SelectedSetIndex;
    bool m_HalfMesh;

};


#endif

