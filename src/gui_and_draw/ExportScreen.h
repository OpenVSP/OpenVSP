//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// SetEditorScreen: Export Files from VSP
// J.R Gloudemans
//
//////////////////////////////////////////////////////////////////////

#if !defined(VSPEXPORTSCREEN__INCLUDED_)
#define VSPEXPORTSCREEN__INCLUDED_

#include "ScreenBase.h"
#include "GuiDevice.h"

#include <FL/Fl.H>
#include "exportFileScreen.h"

using std::string;
using std::vector;

class ExportScreen : public VspScreen
{
public:

    ExportScreen( ScreenMgr* mgr );
    virtual ~ExportScreen()                         {}
    void Show();
    void Hide();
    bool Update();

    void LoadSetChoice();

    void ExportFile( string &newfile, int write_set, int type );

    void CallBack( Fl_Widget *w );
    static void staticScreenCB( Fl_Widget *w, void* data )
    {
        ( ( ExportScreen* )data )->CallBack( w );
    }

protected:

    int m_SelectedSetIndex;
    ExportFileUI* m_ExportFileUI;

};


#endif
