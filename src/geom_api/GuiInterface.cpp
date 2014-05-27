//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// VehicleMgr.cpp: implementation of the Vehicle Class and Vehicle Mgr Singleton.
//
//////////////////////////////////////////////////////////////////////

#include "GuiInterface.h"

#ifdef WIN32
#define NOMINMAX
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
    m_Vehicle = NULL;
}

//==== Destructor ====//
GuiInterface::~GuiInterface()
{
#ifdef VSP_USE_FLTK
    if( m_ScreenMgr )
    {
        delete m_ScreenMgr;
    }
#endif
}

void GuiInterface::InitGui( Vehicle* vPtr )
{
    m_Vehicle = vPtr;

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

void GuiInterface::InitGuiAPI( Vehicle* vPtr )
{
    m_Vehicle = vPtr;

#ifdef VSP_USE_FLTK
    if( !m_ScreenMgr )
    {
        m_ScreenMgr = new ScreenMgr( vPtr );
    }

    for( int i = 0; i < ScreenMgr::VSP_NUM_SCREENS; i++ )
    {
        m_ScreenMgr->HideScreen( i );
    }

#endif

}

void GuiInterface::StartGuiAPI( )
{
#ifdef VSP_USE_FLTK
    m_ScreenMgr->SetRunGui( true );
    m_ScreenMgr->ShowReturnToAPI();
    m_ScreenMgr->ShowScreen( ScreenMgr::VSP_MAIN_SCREEN );
    while( m_ScreenMgr->CheckRunGui() && Fl::wait() );
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




