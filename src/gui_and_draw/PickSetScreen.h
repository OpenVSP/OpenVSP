//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// SetEditorScreen: Export Files from VSP
// J.R Gloudemans
//
//////////////////////////////////////////////////////////////////////

#if !defined(VSPPICKSETSCREEN__INCLUDED_)
#define VSPPICKSETSCREEN__INCLUDED_

#include "ScreenBase.h"
#include "GuiDevice.h"

#include <FL/Fl.H>
#include "pickSetFlScreen.h"

using std::string;
using std::vector;

class PickSetScreen
{
public:

    PickSetScreen( );
    virtual ~PickSetScreen()                            {}
    void Show();
    void Hide();
    bool Update();

    void LoadSetChoice();

    int PickSet( string title );

    void CallBack( Fl_Widget *w );
    static void staticScreenCB( Fl_Widget *w, void* data )
    {
        ( ( PickSetScreen* )data )->CallBack( w );
    }

protected:

    int m_SelectedSetIndex;
    PickSetUI* m_PickSetUI;

    bool m_AcceptFlag;

};


#endif
