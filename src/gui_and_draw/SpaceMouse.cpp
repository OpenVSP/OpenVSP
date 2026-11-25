//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// SpaceMouse.cpp: a six-axis device read straight from USB, moving a GL window's view.
//
//////////////////////////////////////////////////////////////////////

#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>

#include <FL/Fl.H>

#include "SpaceMouse.h"
#include "MainGLWindow.h"
#include "NavController.h"

#ifdef VSP_SPNAVDEV
#include "spnavdev.h"
#endif

namespace VSPGUI
{

// Counts at full deflection, and the counts about rest that do nothing.
static const double FULL_COUNTS = 350.0;
static const double DEAD_COUNTS = 15.0;

// At full deflection and Normal speed: the model turns this many radians, pans this many view
// heights, and zooms by e to this power each second.
static const double TURN_RATE = 1.5;
static const double PAN_RATE = 1.0;
static const double ZOOM_RATE = 1.0;

#ifdef VSP_SPNAVDEV
// How often the device is read while open, and looked for while not.
static const double POLL_PERIOD = 1.0 / 60.0;
static const double LOOK_PERIOD = 2.0;
#endif

// Past the dead zone, the fraction of full deflection, squared so small motions are fine.
static double Response( int counts )
{
    double a = std::fabs( ( double )counts );
    if ( a <= DEAD_COUNTS )
    {
        return 0.0;
    }
    double s = ( a - DEAD_COUNTS ) / ( FULL_COUNTS - DEAD_COUNTS );
    if ( s > 1.0 )
    {
        s = 1.0;
    }
    if ( counts < 0 )
    {
        return -s * s;
    }
    return s * s;
}

struct SpaceMouse::Impl
{
    VspGlWindow * m_Win;
    bool m_Started;
    bool m_Moving;              // The device moved the view in the last half second.
    bool m_Log;                 // VSP_SPACEMOUSE_LOG is set: write what the device sends.
    std::string m_Name;         // The open device's name.
    std::string m_LastStatus;   // As last reported to the status callback.
    int m_Counts[ 6 ];          // The axes as the device last reported them.
    std::chrono::steady_clock::time_point m_LastPoll;

#ifdef VSP_SPNAVDEV
    struct spndev * m_Dev;
#endif

    void ( *m_StatusCB )( void * );
    void * m_StatusData;

    bool IsOpen() const
    {
#ifdef VSP_SPNAVDEV
        return m_Dev != nullptr;
#else
        return false;
#endif
    }

    void Close()
    {
#ifdef VSP_SPNAVDEV
        if ( m_Dev )
        {
            spndev_close( m_Dev );
            m_Dev = nullptr;
        }
#endif
        m_Name.clear();
        for ( int i = 0; i < 6; i++ )
        {
            m_Counts[ i ] = 0;
        }

        // Its buttons can no longer be released.
        m_Win->GetNav().ReleaseDeviceButtons();
    }

    // The device moved the view: say so for a moment.
    static void ClearMoving( void * data )
    {
        SpaceMouse * dev = ( SpaceMouse * )data;
        dev->m_Impl->m_Moving = false;
        dev->StatusChanged();
    }

    void NoteMotion( SpaceMouse * dev )
    {
        Fl::remove_timeout( ClearMoving, dev );
        Fl::add_timeout( 0.5, ClearMoving, dev );
        if ( !m_Moving )
        {
            m_Moving = true;
            dev->StatusChanged();
        }
    }

    // With no device, look for one.
    static void Look( void * data );

    // Read everything the device has sent, then move the view by the axes as they now are.
    static void Poll( void * data );
};

void SpaceMouse::Impl::Look( void * data )
{
#ifdef VSP_SPNAVDEV
    SpaceMouse * dev = ( SpaceMouse * )data;
    Impl * impl = dev->m_Impl.get();

    impl->m_Dev = spndev_open( nullptr );
    if ( !impl->m_Dev )
    {
        Fl::repeat_timeout( LOOK_PERIOD, Look, data );
        return;
    }

    const char * name = spndev_name( impl->m_Dev );
    if ( name )
    {
        impl->m_Name = name;
    }
    impl->m_LastPoll = std::chrono::steady_clock::now();
    dev->StatusChanged();
    Fl::add_timeout( POLL_PERIOD, Poll, data );
#else
    ( void )data;
#endif
}

void SpaceMouse::Impl::Poll( void * data )
{
#ifdef VSP_SPNAVDEV
    SpaceMouse * dev = ( SpaceMouse * )data;
    Impl * impl = dev->m_Impl.get();

    union spndev_event ev;
    int type = spndev_process( impl->m_Dev, &ev );
    while ( type != SPNDEV_NONE )
    {
        if ( type < 0 )
        {
            // Gone, e.g. unplugged: look for it again.
            impl->Close();
            Fl::remove_timeout( ClearMoving, dev );
            impl->m_Moving = false;
            dev->StatusChanged();
            Fl::add_timeout( LOOK_PERIOD, Look, data );
            return;
        }

        if ( type == SPNDEV_MOTION )
        {
            for ( int i = 0; i < 6; i++ )
            {
                impl->m_Counts[ i ] = ev.mot.v[ i ];
            }
            if ( impl->m_Log )
            {
                printf( "spacemouse motion %d %d %d %d %d %d\n", ev.mot.v[ 0 ], ev.mot.v[ 1 ], ev.mot.v[ 2 ],
                        ev.mot.v[ 3 ], ev.mot.v[ 4 ], ev.mot.v[ 5 ] );
            }
        }
        else if ( type == SPNDEV_BUTTON )
        {
            const char * bname = spndev_button_name( impl->m_Dev, ev.bn.num );
            std::string name;
            if ( bname )
            {
                name = bname;
            }
            if ( impl->m_Log )
            {
                printf( "spacemouse button %d %s %d\n", ev.bn.num, name.c_str(), ev.bn.press );
            }

            // A release always counts, so a held Zoom in stops wherever the focus went.
            if ( !name.empty() && ( Fl::focus() || !ev.bn.press ) )
            {
                impl->m_Win->GetNav().DeviceButton( name, ev.bn.press != 0 );
            }
        }
        type = spndev_process( impl->m_Dev, &ev );
    }

    std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
    double dt = std::chrono::duration < double > ( now - impl->m_LastPoll ).count();
    impl->m_LastPoll = now;
    if ( dt > 0.1 )
    {
        dt = 0.1;
    }

    // Only the program with the keyboard focus follows the device.
    if ( Fl::focus() )
    {
        dev->Move( impl->m_Counts, dt );
    }

    Fl::repeat_timeout( POLL_PERIOD, Poll, data );
#else
    ( void )data;
#endif
}

SpaceMouse::SpaceMouse( VspGlWindow * win ) : m_Impl( new Impl() )
{
    m_Impl->m_Win = win;
    m_Impl->m_Started = false;
    m_Impl->m_Moving = false;
    m_Impl->m_Log = ( getenv( "VSP_SPACEMOUSE_LOG" ) != nullptr );
    for ( int i = 0; i < 6; i++ )
    {
        m_Impl->m_Counts[ i ] = 0;
    }
#ifdef VSP_SPNAVDEV
    m_Impl->m_Dev = nullptr;
#endif
    m_Impl->m_StatusCB = nullptr;
    m_Impl->m_StatusData = nullptr;
}

SpaceMouse::~SpaceMouse()
{
    // Whoever is called back may be going away too.
    m_Impl->m_StatusCB = nullptr;
    Stop();
}

bool SpaceMouse::IsBuilt()
{
#ifdef VSP_SPNAVDEV
    return true;
#else
    return false;
#endif
}

void SpaceMouse::Start()
{
    if ( !IsBuilt() || m_Impl->m_Started )
    {
        return;
    }
    m_Impl->m_Started = true;
    Fl::add_timeout( 0.0, Impl::Look, this );
    StatusChanged();
}

void SpaceMouse::Stop()
{
    if ( !m_Impl->m_Started )
    {
        return;
    }
    m_Impl->m_Started = false;
    Fl::remove_timeout( Impl::Look, this );
    Fl::remove_timeout( Impl::Poll, this );
    Fl::remove_timeout( Impl::ClearMoving, this );
    m_Impl->m_Moving = false;
    m_Impl->Close();
    StatusChanged();
}

bool SpaceMouse::IsStarted() const
{
    return m_Impl->m_Started;
}

bool SpaceMouse::IsAvailable() const
{
    return m_Impl->IsOpen();
}

std::string SpaceMouse::GetStatus() const
{
    if ( !IsBuilt() )
    {
        return "Not built";
    }
    if ( !m_Impl->m_Started )
    {
        return "Off";
    }
    if ( !m_Impl->IsOpen() )
    {
        return "Looking for a device";
    }
    if ( m_Impl->m_Moving )
    {
        return "Moving the view";
    }
    return "Connected, " + m_Impl->m_Name;
}

void SpaceMouse::SetStatusCallback( void ( *cb )( void * ), void * data )
{
    m_Impl->m_StatusCB = cb;
    m_Impl->m_StatusData = data;
}

void SpaceMouse::StatusChanged()
{
    std::string status = GetStatus();
    if ( status == m_Impl->m_LastStatus )
    {
        return;
    }
    m_Impl->m_LastStatus = status;
    if ( m_Impl->m_StatusCB )
    {
        m_Impl->m_StatusCB( m_Impl->m_StatusData );
    }
}

std::vector < std::string > SpaceMouse::GetButtonNames() const
{
    std::vector < std::string > names;
#ifdef VSP_SPNAVDEV
    if ( m_Impl->m_Dev )
    {
        int n = spndev_num_buttons( m_Impl->m_Dev );
        for ( int i = 0; i < n; i++ )
        {
            const char * name = spndev_button_name( m_Impl->m_Dev, i );
            if ( name && name[0] != '\0' )
            {
                names.push_back( name );
            }
        }
    }
#endif
    return names;
}

void SpaceMouse::GetCounts( int counts[6] ) const
{
    for ( int i = 0; i < 6; i++ )
    {
        counts[i] = m_Impl->m_Counts[i];
    }
}

double SpaceMouse::FullCounts()
{
    return FULL_COUNTS;
}

double SpaceMouse::DeadCounts()
{
    return DEAD_COUNTS;
}

void SpaceMouse::Move( const int counts[6], double dt )
{
    const NavSettings & settings = m_Impl->m_Win->GetNav().GetSettings();

    // With Dominant, only the axis pushed furthest of those that can act acts.
    int only = -1;
    if ( settings.m_SpaceMouseDominant )
    {
        int most = 0;
        for ( int i = 0; i < NAV_NUM_AXES; i++ )
        {
            int a = settings.m_SpaceMouseAxis[i];
            bool turns = ( a == NAV_AXIS_TURN_X || a == NAV_AXIS_TURN_Y || a == NAV_AXIS_TURN_Z );
            bool acts = ( a != NAV_AXIS_NONE && ( ( turns && settings.m_SpaceMouseRotate ) ||
                                                  ( !turns && settings.m_SpaceMouseMove ) ) );
            if ( acts && std::abs( counts[i] ) > most )
            {
                most = std::abs( counts[i] );
                only = i;
            }
        }
    }

    // Eye axes: x right, y up, z toward you.  Translation z is the zoom.
    glm::dvec3 turn( 0.0 );
    glm::dvec3 move( 0.0 );
    for ( int i = 0; i < NAV_NUM_AXES; i++ )
    {
        if ( only >= 0 && i != only )
        {
            continue;
        }
        double r = Response( counts[i] );
        if ( settings.m_SpaceMouseReverse[i] )
        {
            r = -r;
        }

        switch ( settings.m_SpaceMouseAxis[i] )
        {
        case NAV_AXIS_PAN_X:
            move.x += r * PAN_RATE;
            break;
        case NAV_AXIS_PAN_Y:
            move.y += r * PAN_RATE;
            break;
        case NAV_AXIS_ZOOM:
            move.z += r * ZOOM_RATE;
            break;
        case NAV_AXIS_TURN_X:
            turn.x += r * TURN_RATE;
            break;
        case NAV_AXIS_TURN_Y:
            turn.y += r * TURN_RATE;
            break;
        case NAV_AXIS_TURN_Z:
            turn.z += r * TURN_RATE;
            break;
        default:
            break;
        }
    }
    if ( !settings.m_SpaceMouseRotate )
    {
        turn = glm::dvec3( 0.0 );
    }
    if ( !settings.m_SpaceMouseMove )
    {
        move = glm::dvec3( 0.0 );
    }

    if ( turn == glm::dvec3( 0.0 ) && move == glm::dvec3( 0.0 ) )
    {
        return;
    }

    double speed = settings.m_SpaceMouseSpeed * dt;
    m_Impl->m_Win->GetNav().ApplyAxes( turn * speed, move * speed );

    m_Impl->NoteMotion( this );
}

}
