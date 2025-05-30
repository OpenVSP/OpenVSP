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

#ifdef __JETBRAINS_IDE__
#define VSP_USE_FLTK 1
#endif

#ifdef VSP_USE_FLTK
#include "ScreenMgr.h"
#endif

#include "MainThreadIDMgr.h"

//==== Constructor ====//
GuiInterface::GuiInterface()
{
    // Set up MainThreadID if this is entry point.
    MainThreadIDMgr.getInstance();

    m_EventLoopRunning = false;

    m_ScreenMgr = nullptr;
}

//==== Destructor ====//
GuiInterface::~GuiInterface()
{
#ifdef VSP_USE_FLTK
    delete m_ScreenMgr;
#endif
}

// In a multi-threaded environment, this must be called from the main thread only.
void GuiInterface::InitGUI( Vehicle* vPtr )
{
#ifdef VSP_USE_FLTK
    if( !m_ScreenMgr )
    {
        m_ScreenMgr = new ScreenMgr( vPtr );
    }
#endif

}

// In a multi-threaded environment, this must be called from the main thread only.
void GuiInterface::StartGUI()
{
#ifdef VSP_USE_FLTK
    Fl::lock();
    if ( m_ScreenMgr )
    {
        m_ScreenMgr->FirstShow();
    }
    m_EventLoopRunning = true;
    Fl::run();
    m_EventLoopRunning = false;
    Fl::unlock();  // We wont' ever get here.
#endif
}

// In a multi-threaded environment, this must be called from the main thread only.
void GuiInterface::StartGUIAPI( )
{
#ifdef VSP_USE_FLTK

    static bool once = false;

    if ( m_ScreenMgr )
    {
        if ( !once )
        {
            m_ScreenMgr->FirstShow();
            once = true;
        }

        m_ScreenMgr->SetRunGui( true );
        m_ScreenMgr->DisableExitMenuItem();
        m_ScreenMgr->EnableStopGUIMenuItem();
        m_ScreenMgr->ShowScreen( vsp::VSP_MAIN_SCREEN );
        m_ScreenMgr->APIShowScreensImplementation();
        Fl::lock();
        m_EventLoopRunning = true;
        while( m_ScreenMgr->CheckRunGui() )
        {
            Fl::wait( 1e20 );
        }
        Fl::wait( 0.01 );
        m_EventLoopRunning = false;
        Fl::unlock();
    }
#endif

}

// In a multi-threaded environment, this uses Fl::awake() to make it safe to run from a secondary thread.
void GuiInterface::StopGUI()
{
#ifdef VSP_USE_FLTK
    if ( m_ScreenMgr )
    {
        m_ScreenMgr->APIHideScreens();     // Runs in main thread, is blocking.
        m_ScreenMgr->SetRunGui( false ); // Safe from secondary thread.
    }
#endif
}


// In a multi-threaded environment, this uses Fl::awake() to make it safe to run from a secondary thread.
void GuiInterface::PopupMsg( const std::string &message )
{
#ifdef VSP_USE_FLTK
    if ( m_ScreenMgr )
    {
        m_ScreenMgr->Alert( message );
    }
#endif

}

void GuiInterface::UpdateGUI()
{
#ifdef VSP_USE_FLTK
    if ( m_ScreenMgr )
    {
        m_ScreenMgr->APIUpdateGUI();
    }
#endif
}

// In a multi-threaded environment, this is safe to run from a secondary thread.
void GuiInterface::Lock()
{
#ifdef VSP_USE_FLTK
    Fl::lock();
#endif
}

// In a multi-threaded environment, this is safe to run from a secondary thread.
void GuiInterface::Unlock()
{
#ifdef VSP_USE_FLTK
    Fl::unlock();
#endif
}

bool GuiInterface::IsEventLoopRunning() const
{
    return m_EventLoopRunning;
}

void GuiInterface::ScreenGrab( const std::string &fname, int w, int h, bool transparentBG, bool autocrop )
{
#ifdef VSP_USE_FLTK
    if ( m_ScreenMgr )
    {
        m_ScreenMgr->APIScreenGrab( fname, w, h, transparentBG, autocrop );
    }
#endif
}

// In a multi-threaded environment, this is safe to run from a secondary thread.
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

// In a multi-threaded environment, this is safe to run from a secondary thread.
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

// In a multi-threaded environment, this is safe to run from a secondary thread.
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

void GuiInterface::SetView( int viewport, int view )
{
#ifdef VSP_USE_FLTK
    if ( m_ScreenMgr )
    {
        MainVSPScreen* main = dynamic_cast < MainVSPScreen * >( m_ScreenMgr->GetScreen( vsp::VSP_MAIN_SCREEN ) );
        if( main )
        {
            main->SetView( viewport, main->ConvertViewEnums( view ) );
        }
    }
#endif
}

void GuiInterface::SetAllViews( int view )
{
#ifdef VSP_USE_FLTK
    if ( m_ScreenMgr )
    {
        MainVSPScreen* main = dynamic_cast < MainVSPScreen * >( m_ScreenMgr->GetScreen( vsp::VSP_MAIN_SCREEN ) );
        if( main )
        {
            main->SetAllViews( main->ConvertViewEnums( view ) );
        }
    }
#endif
}

void GuiInterface::FitAllViews()
{
#ifdef VSP_USE_FLTK
    if ( m_ScreenMgr )
    {
        MainVSPScreen* main = dynamic_cast < MainVSPScreen * >( m_ScreenMgr->GetScreen( vsp::VSP_MAIN_SCREEN ) );
        if( main )
        {
            main->FitView( true );
        }
    }
#endif
}

void GuiInterface::ResetViews()
{
#ifdef VSP_USE_FLTK
    if ( m_ScreenMgr )
    {
        MainVSPScreen* main = dynamic_cast < MainVSPScreen * >( m_ScreenMgr->GetScreen( vsp::VSP_MAIN_SCREEN ) );
        if( main )
        {
            main->ResetViews();
        }
    }
#endif
}

void GuiInterface::SetWindowLayout( int r, int c )
{
#ifdef VSP_USE_FLTK
    if ( m_ScreenMgr )
    {
        MainVSPScreen* main = dynamic_cast < MainVSPScreen * >( m_ScreenMgr->GetScreen( vsp::VSP_MAIN_SCREEN ) );
        if( main )
        {
            main->SetWindowLayout( r, c );
        }
    }
#endif
}

// In a multi-threaded environment, this uses Fl::awake() to make it safe to run from a secondary thread.
void GuiInterface::EnableStopGUIMenuItem()
{
#ifdef VSP_USE_FLTK
    if ( m_ScreenMgr )
    {
        m_ScreenMgr->EnableStopGUIMenuItem();
    }
#endif
}

// In a multi-threaded environment, this uses Fl::awake() to make it safe to run from a secondary thread.
void GuiInterface::DisableStopGUIMenuItem()
{
#ifdef VSP_USE_FLTK
    if ( m_ScreenMgr )
    {
        m_ScreenMgr->DisableStopGUIMenuItem();
    }
#endif
}

// In a multi-threaded environment, this is safe to run from a secondary thread.
void GuiInterface::SetGUIElementDisable( int e, bool state )
{
#ifdef VSP_USE_FLTK
    if ( m_ScreenMgr )
    {
        m_ScreenMgr->SetGUIElementDisable( e, state );
    }
#endif
}

// In a multi-threaded environment, this is safe to run from a secondary thread.
void GuiInterface::SetGUIScreenDisable( int s, bool state )
{
#ifdef VSP_USE_FLTK
    if ( m_ScreenMgr )
    {
        m_ScreenMgr->SetGUIScreenDisable( s, state );
    }
#endif
}

// In a multi-threaded environment, this is safe to run from a secondary thread.
void GuiInterface::SetGeomScreenDisable( int s, bool state )
{
#ifdef VSP_USE_FLTK
    if ( m_ScreenMgr )
    {
        m_ScreenMgr->SetGeomScreenDisable( s, state );
    }
#endif
}
