//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// SetEditorScreen: View and Edit Geom Sets
// J.R Gloudemans
//
//////////////////////////////////////////////////////////////////////

#if !defined(VSPSETEDITORSCREEN__INCLUDED_)
#define VSPSETEDITORSCREEN__INCLUDED_

#include "ScreenBase.h"
#include "GuiDevice.h"

#include <FL/Fl.H>
#include "setEditorFlScreen.h"

using std::string;
using std::vector;

class SetEditorScreen : public VspScreen
{
public:
    SetEditorScreen( ScreenMgr* mgr );
    virtual ~SetEditorScreen()                          {}
    void Show();
    void Hide();
    bool Update();

    void CallBack( Fl_Widget *w );
    static void staticScreenCB( Fl_Widget *w, void* data )
    {
        ( ( SetEditorScreen* )data )->CallBack( w );
    }

protected:

    int m_SelectedSetIndex;
    SetEditorUI* m_SetEditorUI;

};


#endif
