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

using std::string;
using std::vector;

class ExportScreen : public BasicScreen
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

    virtual void GuiDeviceCallBack( GuiDevice* device );

protected:

    GroupLayout m_MainLayout;
    GroupLayout m_GenLayout;

    Choice m_ExportSetChoice;

    TriggerButton m_XSecButton;
    TriggerButton m_Plot3DButton;
    TriggerButton m_STLButton;
    TriggerButton m_NASCARTButton;
    TriggerButton m_TRIButton;
    TriggerButton m_GMSHButton;
    TriggerButton m_POVButton;
    TriggerButton m_X3DButton;
    TriggerButton m_STEPButton;
    TriggerButton m_IGESButton;
    TriggerButton m_BEMButton;
    TriggerButton m_DXFButton;
    TriggerButton m_FacetButton;

    int m_SelectedSetIndex;
};


#endif
