//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#include "AeroStructScreen.h"
#include "ScreenMgr.h"
#include "ScreenBase.h"
#include "ParmMgr.h"

#include "VSPAEROScreen.h"

#include <FL/fl_ask.H>


//==== Constructor ====//
AeroStructScreen::AeroStructScreen( ScreenMgr* mgr ) : BasicScreen( mgr, 300, 600, "Aero Structure Coupled Analysis" )
{

    m_GlobalLayout.SetGroupAndScreen( m_FLTK_Window, this );
    m_GlobalLayout.AddY( 25 );

    m_GlobalLayout.AddDividerBox( "VSPAERO" );

    m_GlobalLayout.AddButton( m_ShowVSPAEROGUI, "Show VSPAERO GUI" );
    m_GlobalLayout.AddButton( m_ExecuteVSPAERO, "Execute VSPAERO" );
}

AeroStructScreen::~AeroStructScreen()
{
}

//==== Update Screen ====//
bool AeroStructScreen::Update()
{

    VSPAEROScreen * AeroScreen = dynamic_cast < VSPAEROScreen* > ( m_ScreenMgr->GetScreen( ScreenMgr::VSP_VSPAERO_SCREEN ) );

    if ( AeroScreen )
    {
        if ( AeroScreen->m_SolverThreadIsRunning )
        {
            m_ExecuteVSPAERO.SetColor( FL_RED );
        }
        else
        {
            m_ExecuteVSPAERO.SetColor( FL_BACKGROUND_COLOR );
        }
    }


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

    if ( gui_device == &m_ShowVSPAEROGUI )
    {
        m_ScreenMgr->ShowScreen( ScreenMgr::VSP_VSPAERO_SCREEN );
    }
    else if( gui_device == &m_ExecuteVSPAERO )
    {
        VSPAEROScreen * AeroScreen = dynamic_cast < VSPAEROScreen* > ( m_ScreenMgr->GetScreen( ScreenMgr::VSP_VSPAERO_SCREEN ) );
        if ( AeroScreen )
        {
            AeroScreen->LaunchVSPAERO();
        }
    }
    else
    {
        return;
    }

    m_ScreenMgr->SetUpdateFlag( true );
}
