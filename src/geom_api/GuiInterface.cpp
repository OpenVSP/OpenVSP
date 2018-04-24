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

#include <stddef.h>

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
        m_ScreenMgr->ShowReturnToAPI();
        m_ScreenMgr->ShowScreen( ScreenMgr::VSP_MAIN_SCREEN );
        while( m_ScreenMgr->CheckRunGui() && Fl::wait() );
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

void GuiInterface::PopupMsg( const char* message, bool lock_out )
{
#ifdef VSP_USE_FLTK
    if ( m_ScreenMgr )
    {
        m_ScreenMgr->Alert( message );
    }
#endif

}

void GuiInterface::ScreenGrab( const std::string &fname, int w, int h )
{
#ifdef VSP_USE_FLTK
    if ( m_ScreenMgr )
    {
        m_ScreenMgr->ForceUpdate();
        MainVSPScreen* main = dynamic_cast < MainVSPScreen * >( m_ScreenMgr->GetScreen( m_ScreenMgr->VSP_MAIN_SCREEN ) );
        if( main )
        {
            main->ScreenGrab( fname, w, h );
        }
    }
#endif
}
