//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//////////////////////////////////////////////////////////////////////

#if !defined(VSPIMPORTSCREEN__INCLUDED_)
#define VSPIMPORTSCREEN__INCLUDED_

#include "ScreenBase.h"
#include "GuiDevice.h"

#include <FL/Fl.H>
#include "importFileScreen.h"

using std::string;
using std::vector;

class ImportScreen : public VspScreen
{
public:

//  enum {  STEREOLITH, NASCART, CART3D_TRI, XSEC_SURF, XSEC_MESH };

    ImportScreen( ScreenMgr* mgr );
    virtual ~ImportScreen();

    void ImportFile( string &in_file, int type );

    void CallBack( Fl_Widget *w );
    static void staticScreenCB( Fl_Widget *w, void* data )
    {
        ( ( ImportScreen* )data )->CallBack( w );
    }

protected:
    ImportFileUI* m_ImportUI;

};


#endif
