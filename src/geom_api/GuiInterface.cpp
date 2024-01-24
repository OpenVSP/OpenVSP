//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// VehicleMgr.cpp: implementation of the Vehicle Class and Vehicle Mgr Singleton.
//
//////////////////////////////////////////////////////////////////////

#include "GuiInterface.h"

#ifdef WIN32
#include <windows.h>
#endif

#ifdef VSP_USE_FLTK
#include "ScreenMgr.h"
#endif

//==== Constructor ====//
GuiInterface::GuiInterface()
{
    m_ScreenMgr = NULL;
}

//==== Destructor ====//
GuiInterface::~GuiInterface()
{
#ifdef VSP_USE_FLTK
    delete m_ScreenMgr;
#endif
}

void GuiInterface::InitGui( Vehicle* vPtr )
{
#ifdef VSP_USE_FLTK
    if( !m_ScreenMgr )
    {
        m_ScreenMgr = new ScreenMgr( vPtr );
    }
#endif

}

void GuiInterface::StartGui()
{
#ifdef VSP_USE_FLTK
    Fl::lock();
    Fl::run();
#endif
}

void GuiInterface::StartGuiAPI( )
{
#ifdef VSP_USE_FLTK
    if ( m_ScreenMgr )
    {
        m_ScreenMgr->SetRunGui( true );
        m_ScreenMgr->HideExit();
        m_ScreenMgr->ShowScreen( vsp::VSP_MAIN_SCREEN );
        m_ScreenMgr->APIShowScreens();
        while( m_ScreenMgr->CheckRunGui() && Fl::wait() );
    }
#endif

}

void GuiInterface::StopGui()
{
#ifdef VSP_USE_FLTK
    if ( m_ScreenMgr )
    {
        m_ScreenMgr->ReturnToAPI();
    }
#endif
}

void GuiInterface::HideScreens()
{
#ifdef VSP_USE_FLTK
    if (m_ScreenMgr)
    {
        m_ScreenMgr->APIHideScreens();
    }
#endif
}

void GuiInterface::ShowScreens()
{
#ifdef VSP_USE_FLTK
    if (m_ScreenMgr)
    {
        m_ScreenMgr->APIShowScreens();
    }
#endif
}

void GuiInterface::UpdateGui( )
{
#ifdef VSP_USE_FLTK
    if ( m_ScreenMgr )
    {
        m_ScreenMgr->SetUpdateFlag( true );
//      m_ScreenMgr->UpdateAllScreens();
    }
#endif

}

void GuiInterface::PopupMsg( const std::string &message )
{
#ifdef VSP_USE_FLTK
    if ( m_ScreenMgr )
    {
        m_ScreenMgr->Alert( message );
    }
#endif

}

void GuiInterface::Lock()
{
#ifdef VSP_USE_FLTK
    Fl::lock();
#endif
}

void GuiInterface::Unlock()
{
#ifdef VSP_USE_FLTK
    Fl::unlock();
#endif
}

void GuiInterface::ScreenGrab( const std::string &fname, int w, int h, bool transparentBG, bool autocrop )
{
#ifdef VSP_USE_FLTK
    if ( m_ScreenMgr )
    {
        m_ScreenMgr->ForceUpdate();
        MainVSPScreen* main = dynamic_cast < MainVSPScreen * >( m_ScreenMgr->GetScreen( vsp::VSP_MAIN_SCREEN ) );
        if( main )
        {
            main->ScreenGrab( fname, w, h, transparentBG, autocrop );
        }
    }
#endif
}

void GuiInterface::SetViewAxis( bool vaxis )
{
#ifdef VSP_USE_FLTK
    if ( m_ScreenMgr )
    {
        MainVSPScreen* main = dynamic_cast < MainVSPScreen * >( m_ScreenMgr->GetScreen( vsp::VSP_MAIN_SCREEN ) );
        if( main )
        {
            main->SetViewAxis( vaxis );
        }
    }
#endif
}

void GuiInterface::SetShowBorders( bool brdr )
{
#ifdef VSP_USE_FLTK
    if ( m_ScreenMgr )
    {
        MainVSPScreen* main = dynamic_cast < MainVSPScreen * >( m_ScreenMgr->GetScreen( vsp::VSP_MAIN_SCREEN ) );
        if( main )
        {
            main->SetShowBorders( brdr );
        }
    }
#endif
}

void GuiInterface::SetBackground( double r, double g, double b )
{
#ifdef VSP_USE_FLTK
    if ( m_ScreenMgr )
    {
        MainVSPScreen* main = dynamic_cast < MainVSPScreen * >( m_ScreenMgr->GetScreen( vsp::VSP_MAIN_SCREEN ) );
        if( main )
        {
            main->SetBackground( r, g, b );
        }
    }
#endif
}

void GuiInterface::EnableReturnToAPI()
{
#ifdef VSP_USE_FLTK
    if ( m_ScreenMgr )
    {
        m_ScreenMgr->ShowReturnToAPI();
    }
#endif
}

void GuiInterface::SetGUIElementDisable( int e, bool state )
{
#ifdef VSP_USE_FLTK
    if ( m_ScreenMgr )
    {
        m_ScreenMgr->SetGUIElementDisable( e, state );
    }
#endif
}

void GuiInterface::SetGUIScreenDisable( int s, bool state )
{
#ifdef VSP_USE_FLTK
    if ( m_ScreenMgr )
    {
        m_ScreenMgr->SetGUIScreenDisable( s, state );
    }
#endif
}

void GuiInterface::SetGeomScreenDisable( int s, bool state )
{
#ifdef VSP_USE_FLTK
    if ( m_ScreenMgr )
    {
        m_ScreenMgr->SetGeomScreenDisable( s, state );
    }
#endif
}
