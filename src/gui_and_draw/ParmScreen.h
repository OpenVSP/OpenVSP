//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// parmScreen.h: interface for the geomScreen class.
// J.R. Gloudemans
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PARMSCREEN_H__015D1220_857A_11D7_AC31_0003473A025A__INCLUDED_)
#define AFX_PARMSCREEN_H__015D1220_857A_11D7_AC31_0003473A025A__INCLUDED_

#include "ScreenBase.h"
#include "GuiDevice.h"

#include "parmFlScreen.h"
#include "ScreenMgr.h"

using std::string;

class ParmScreen : public VspScreen
{
public:
    ParmScreen( ScreenMgr* mgr );
    virtual ~ParmScreen()           {}

    void Show();
    bool Update();

    void CallBack( Fl_Widget *w );
    static void staticScreenCB( Fl_Widget *w, void* data )
    {
        ( ( ParmScreen* )data )->CallBack( w );
    }

    void SelectToLink( int sel );
    void SelectFromLink( int sel );

protected:

    string m_ParmID;
    ParmUI* m_ParmUI;

    string m_NameString;
    string m_DescriptString;

    Fl_Text_Buffer m_TextBuffer;

};

#endif
