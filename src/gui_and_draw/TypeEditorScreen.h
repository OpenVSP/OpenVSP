//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

//TypeEditorScreen: View and Edit Types Of Geom
// J.R Gloudemans
//
//////////////////////////////////////////////////////////////////////

#if !defined(VSPTYPEEDITORSCREEN__INCLUDED_)
#define VSPTYPEEDITORSCREEN__INCLUDED_

#include "ScreenBase.h"
#include "GuiDevice.h"

#include <FL/Fl.H>
#include "typeEditorFlScreen.h"

using std::string;
using std::vector;

class TypeEditorScreen : public VspScreen
{
public:
    TypeEditorScreen( ScreenMgr* mgr );
    virtual ~TypeEditorScreen()                         {}
    void Show();
    void Hide();
    bool Update();

    void CallBack( Fl_Widget *w );
    static void staticScreenCB( Fl_Widget *w, void* data )
    {
        ( ( TypeEditorScreen* )data )->CallBack( w );
    }

protected:

    int m_SelectedIndex;
    int m_GeomIndex;
    TypeEditorUI* m_TypeEditorUI;

};


#endif
