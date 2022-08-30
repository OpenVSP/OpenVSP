//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#include "AeroStructScreen.h"
#include "ScreenMgr.h"
#include "ParmMgr.h"

#include <FL/fl_ask.H>


//==== Constructor ====//
AeroStructScreen::AeroStructScreen( ScreenMgr* mgr ) : BasicScreen( mgr, 800, 600, "Aero Structure Coupled Analysis" )
{
}

AeroStructScreen::~AeroStructScreen()
{
}

//==== Update Screen ====//
bool AeroStructScreen::Update()
{

    m_FLTK_Window->redraw();
    return true;
}

//==== Show Screen ====//
void AeroStructScreen::Show()
{
    m_ScreenMgr->SetUpdateFlag( true );
    m_FLTK_Window->show();
}


//==== Hide Screen ====//
void AeroStructScreen::Hide()
{
    m_FLTK_Window->hide();
    m_ScreenMgr->SetUpdateFlag( true );
}

//==== Callbacks ====//
void AeroStructScreen::CallBack( Fl_Widget *w )
{

//    if ( w == m_Foo )
    {

    }
    // else
    {
        return;
    }

    m_ScreenMgr->SetUpdateFlag( true );
}

//==== Gui Device CallBacks ====//
void AeroStructScreen::GuiDeviceCallBack( GuiDevice* gui_device )
{
    assert( m_ScreenMgr );

    // if ( gui_device == &m_Foo )
    {

    }
    // else
    {
        return;
    }

    m_ScreenMgr->SetUpdateFlag( true );
}
