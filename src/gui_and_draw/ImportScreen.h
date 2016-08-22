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

using std::string;
using std::vector;

class ImportScreen : public BasicScreen
{
public:

    ImportScreen( ScreenMgr* mgr );
    virtual ~ImportScreen();

    void ImportFile( string &in_file, int type );

    void CallBack( Fl_Widget *w );
    static void staticScreenCB( Fl_Widget *w, void* data )
    {
        ( ( ImportScreen* )data )->CallBack( w );
    }

    virtual void GuiDeviceCallBack( GuiDevice* device );

protected:

    GroupLayout m_MainLayout;
    GroupLayout m_GenLayout;

    TriggerButton m_TRIButton;
    TriggerButton m_STLButton;
    TriggerButton m_NASCARTButton;
    TriggerButton m_XSecButton;
    TriggerButton m_PTSButton;
    TriggerButton m_V2Button;
    TriggerButton m_BEMButton;

};


#endif
