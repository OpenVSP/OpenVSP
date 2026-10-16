//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// NavDevice.cpp: a 3D input device's motion, fed to a GL window's navigation.
//
//////////////////////////////////////////////////////////////////////

#include "NavDevice.h"

#ifdef VSP_OPENAXIS

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <functional>
#include <mutex>
#include <vector>

#include <FL/Fl.H>

#include <openaxis/connection_manager.hpp>
#include <openaxis/diagnostics.hpp>
#include <openaxis/navigation.hpp>

#include "main.h"
#include "MainGLWindow.h"
#include "NavController.h"
#include "NavView.h"

namespace VSPGUI
{

// Runs the SDK's work on the FLTK main thread.  It is never destroyed, since work it queued
// can still arrive after the device stops; the SDK makes such work do nothing.
class FltkScheduler : public openaxis::Scheduler
{
public:

    static FltkScheduler * Get()
    {
        static FltkScheduler * s = new FltkScheduler();
        return s;
    }

    // Any thread.
    void post( Callback cb ) override
    {
        bool wake = false;
        {
            std::lock_guard < std::mutex > lock( m_Mutex );
            m_Queue.push_back( cb );
            if ( !m_Woken )
            {
                m_Woken = true;
                wake = true;
            }
        }
        // When FLTK's queue is full, the next post tries again.
        if ( wake && Fl::awake( Drain, this ) < 0 )
        {
            std::lock_guard < std::mutex > lock( m_Mutex );
            m_Woken = false;
        }
    }

    // Any thread.  The timer is set on the main thread.
    void post_at( double deadline, Callback cb ) override
    {
        post( [deadline, cb]()
        {
            double delay = std::max( 0.0, deadline - openaxis::diagnostic_time() );
            Fl::add_timeout( delay, RunTimed, new Callback( cb ) );
        } );
    }

protected:

    FltkScheduler()
    {
        m_Woken = false;
    }

    static void Drain( void * data )
    {
        FltkScheduler * s = ( FltkScheduler * )data;
        std::vector < Callback > work;
        {
            std::lock_guard < std::mutex > lock( s->m_Mutex );
            work.swap( s->m_Queue );
            s->m_Woken = false;
        }
        for ( int i = 0; i < ( int )work.size(); i++ )
        {
            try
            {
                work[i]();
            }
            catch ( ... )
            {
            }
        }
    }

    static void RunTimed( void * data )
    {
        Callback * cb = ( Callback * )data;
        try
        {
            ( *cb )();
        }
        catch ( ... )
        {
        }
        delete cb;
    }

    std::mutex m_Mutex;
    std::vector < Callback > m_Queue;
    bool m_Woken;
};

openaxis::Scheduler * NavScheduler()
{
    return FltkScheduler::Get();
}

// Turns the streamed axis rates into ball travel for the window's navigation.
class AxisListener : public openaxis::OpenAxisListener
{
public:

    AxisListener( VspGlWindow * win )
    {
        m_Win = win;
        Reset();
    }

    void Reset()
    {
        m_HasTime = false;
        m_LastTime = 0;
        m_LastSeq = -1;
    }

    void on_axes( const std::vector < std::string > & axes ) override
    {
        m_Axes = axes;
        Reset();
    }

    void on_motion_start( std::int64_t ) override
    {
        Reset();
    }

    void on_motion_end( std::int64_t ) override
    {
        Reset();
    }

    void on_state_change( openaxis::ConnectionState ) override
    {
        Reset();
    }

    void on_frame( const openaxis::Frame & frame ) override
    {
        if ( frame.seq <= m_LastSeq )
        {
            return;
        }
        m_LastSeq = frame.seq;

        // The first frame of a motion only starts the clock, so idle time never moves the view.
        if ( !m_HasTime )
        {
            m_HasTime = true;
            m_LastTime = frame.t_us;
            return;
        }
        double dt = ( frame.t_us - m_LastTime ) * 1e-6;
        m_LastTime = frame.t_us;
        dt = std::min( std::max( dt, 0.0 ), 0.1 );

        // Rates in radians per second, in the SDK's frame: +x right, +y up, +z toward the viewer.
        glm::dvec3 rotation( 0.0 );
        glm::dvec3 translation( 0.0 );
        const char * rnames[] = { "rx", "ry", "rz" };
        const char * tnames[] = { "tx", "ty", "tz" };
        for ( int i = 0; i < ( int )m_Axes.size() && i < ( int )frame.values.size(); i++ )
        {
            for ( int j = 0; j < 3; j++ )
            {
                if ( m_Axes[i] == rnames[j] )
                {
                    rotation[j] = frame.values[i] * dt;
                }
                if ( m_Axes[i] == tnames[j] )
                {
                    translation[j] = frame.values[i] * dt;
                }
            }
        }

        m_Win->GetNav().ApplyAxes( rotation, translation );
        if ( m_OnMotion )
        {
            m_OnMotion();
        }
    }

    // Called after each frame moves the view.
    std::function < void() > m_OnMotion;

protected:

    VspGlWindow * m_Win;
    std::vector < std::string > m_Axes;
    bool m_HasTime;
    std::int64_t m_LastTime;
    std::int64_t m_LastSeq;
};

struct NavDevice::Impl
{
    VspGlWindow * m_Win;
    std::shared_ptr < AxisListener > m_Listener;
    std::unique_ptr < NavViewAdapter > m_Adapter;
    std::unique_ptr < openaxis::NavigationSession > m_Session;
    int m_Mode;                 // NavDeviceMode it was started in.
    bool m_Moving;              // The device moved the view in the last half second.
    std::unique_ptr < openaxis::OpenAxisClient > m_Client;
    std::unique_ptr < openaxis::OpenAxisConnectionManager > m_Manager;
    bool m_Focused;
    bool m_SettledDue;          // The host is to be told the view has settled.
    bool m_PivotLocked;         // The host agreed to hold its pivot.
    std::shared_ptr < bool > m_Alive;  // False once stopped, for answers that come after.
    bool m_Failed;              // The SDK could not start.
    std::string m_LastStatus;   // As last reported to the status callback.

    void ( *m_StatusCB )( void * );
    void * m_StatusData;

    // The device moved the view: say so for a moment.
    static void ClearMoving( void * data )
    {
        NavDevice * dev = ( NavDevice * )data;
        dev->m_Impl->m_Moving = false;
        dev->StatusChanged();
    }

    void NoteMotion( NavDevice * dev )
    {
        Fl::remove_timeout( ClearMoving, dev );
        Fl::add_timeout( 0.5, ClearMoving, dev );
        if ( !m_Moving )
        {
            m_Moving = true;
            dev->StatusChanged();
        }
    }

    // Whether an OpenVSP window has the keyboard focus, and whether the view moved under a
    // still device, checked twice a second.  Whether the view has settled is told here too.
    static void PollFocus( void * data )
    {
        Impl * impl = ( Impl * )data;
        if ( impl->m_Session )
        {
            try
            {
                impl->m_Session->check_context();
                if ( impl->m_SettledDue )
                {
                    impl->m_SettledDue = false;
                    impl->m_Session->viewport_settled();
                }
            }
            catch ( ... )
            {
            }
        }
        bool focused = ( Fl::focus() != nullptr );
        if ( focused != impl->m_Focused && impl->m_Manager )
        {
            impl->m_Focused = focused;
            try
            {
                impl->m_Manager->refresh_metadata();
            }
            catch ( ... )
            {
            }
        }
        Fl::repeat_timeout( 0.5, PollFocus, data );
    }
};

NavDevice::NavDevice( VspGlWindow * win ) : m_Impl( new Impl() )
{
    m_Impl->m_Win = win;
    m_Impl->m_Focused = true;
    m_Impl->m_SettledDue = false;
    m_Impl->m_PivotLocked = false;
    m_Impl->m_Failed = false;
    m_Impl->m_Mode = NAV_DEVICE_NAVIGATION;
    m_Impl->m_Moving = false;
    m_Impl->m_StatusCB = nullptr;
    m_Impl->m_StatusData = nullptr;
}

NavDevice::~NavDevice()
{
    // Whoever owns the callback may be going away too.
    m_Impl->m_StatusCB = nullptr;
    Stop();
}

bool NavDevice::IsBuilt()
{
    return true;
}

void NavDevice::Start()
{
    if ( m_Impl->m_Manager )
    {
        return;
    }

    // Lets the network thread wake the main loop through Fl::awake.
    Fl::lock();
    Fl::unlock();

    openaxis::OpenAxisClientOptions options;
    options.client_name = "OpenVSP";
    options.client_version = VSPVERSION4;
    options.scheduler = FltkScheduler::Get();
    try
    {
        // The host sends input to the client whose process is in front.
        options.target = { { "pid", openaxis::current_process_id() } };
    }
    catch ( ... )
    {
    }

    // Any failure to start leaves the device unavailable.
    m_Impl->m_Failed = false;
    m_Impl->m_Alive = std::make_shared < bool >( true );
    m_Impl->m_Mode = m_Impl->m_Win->GetNav().GetSettings().m_DeviceMode;
    try
    {
        Impl * impl = m_Impl.get();
        NavDevice * dev = this;

        m_Impl->m_Listener = std::make_shared < AxisListener >( m_Impl->m_Win );
        m_Impl->m_Listener->m_OnMotion = [impl, dev]()
        {
            impl->NoteMotion( dev );
        };
        m_Impl->m_Client.reset( new openaxis::OpenAxisClient( options, m_Impl->m_Listener ) );

        // Navigation: the host computes the motion from what the adapter tells it of the scene.
        if ( m_Impl->m_Mode == NAV_DEVICE_NAVIGATION )
        {
            m_Impl->m_Adapter.reset( new NavViewAdapter( m_Impl->m_Win ) );
            m_Impl->m_Adapter->m_OnMotion = [impl, dev]()
            {
                impl->NoteMotion( dev );
            };
            // The view settles as each gesture ends.
            openaxis::NavigationOptions nav_options = NavOptions( m_Impl->m_Adapter.get() );
            nav_options.on_event = [impl]( const openaxis::NavigationEvent & e )
            {
                if ( e.event == "gesture_finished" )
                {
                    impl->m_SettledDue = true;
                }
            };
            m_Impl->m_Session.reset( new openaxis::NavigationSession( *m_Impl->m_Client, *m_Impl->m_Adapter, nullptr,
                                                                      nav_options ) );

            // For finding out why a device does nothing: the session's events, to a file.
            const char * log = getenv( "VSP_NAV_LOG" );
            if ( log )
            {
                FILE * f = fopen( log, "a" );
                if ( f )
                {
                    m_Impl->m_Session->diagnostics = [f]( const openaxis::Diagnostic & d )
                    {
                        fprintf( f, "%.3f %s %s %s\n", openaxis::diagnostic_time(), d.event.c_str(), d.target.c_str(), d.detail.c_str() );
                        fflush( f );
                    };
                }
            }
        }

        openaxis::OpenAxisConnectionManagerOptions manager_options;
        manager_options.metadata = [impl]()
        {
            openaxis::ConnectionMetadata metadata;
            // OpenVSP models, and its camera orbits; it has no eye to fly.
            metadata.tags = { "app.openvsp", "workspace.modeling", "navigation.hint.orbit" };
            if ( impl->m_Mode == NAV_DEVICE_NAVIGATION )
            {
                metadata.capabilities = { "navigation" };
            }
            else
            {
                metadata.axes = std::vector < std::string > { "tx", "ty", "tz", "rx", "ry", "rz" };
            }
            metadata.focused = impl->m_Focused;
            return metadata;
        };
        m_Impl->m_Manager.reset( new openaxis::OpenAxisConnectionManager( *m_Impl->m_Client, manager_options ) );
        m_Impl->m_Manager->on_state = [this, impl]( const openaxis::ConnectionStatus & status )
        {
            // A host just connected finds the view settled.  One that went away holds no pivot.
            if ( status.state == "ready" )
            {
                impl->m_SettledDue = true;
            }
            else
            {
                impl->m_PivotLocked = false;
            }
            StatusChanged();
        };
        m_Impl->m_Manager->start();
    }
    catch ( ... )
    {
        m_Impl->m_Manager.reset();
        if ( m_Impl->m_Session )
        {
            m_Impl->m_Session->close();
        }
        m_Impl->m_Session.reset();
        m_Impl->m_Adapter.reset();
        m_Impl->m_Client.reset();
        m_Impl->m_Listener.reset();
        m_Impl->m_Failed = true;
        StatusChanged();
        return;
    }

    Fl::add_timeout( 0.5, Impl::PollFocus, m_Impl.get() );
}

void NavDevice::Stop()
{
    if ( !m_Impl->m_Manager )
    {
        return;
    }

    Fl::remove_timeout( Impl::PollFocus, m_Impl.get() );
    Fl::remove_timeout( Impl::ClearMoving, this );
    m_Impl->m_Moving = false;
    m_Impl->m_PivotLocked = false;
    *m_Impl->m_Alive = false;

    try
    {
        m_Impl->m_Manager->stop();
        m_Impl->m_Manager.reset();
        if ( m_Impl->m_Session )
        {
            m_Impl->m_Session->close();
        }
        m_Impl->m_Client->disconnect();
    }
    catch ( ... )
    {
    }
    m_Impl->m_Manager.reset();
    m_Impl->m_Session.reset();
    m_Impl->m_Adapter.reset();
    m_Impl->m_Client.reset();
    m_Impl->m_Listener.reset();

    StatusChanged();
}

bool NavDevice::IsStarted() const
{
    return ( bool )m_Impl->m_Manager;
}

bool NavDevice::IsAvailable() const
{
    return m_Impl->m_Client && m_Impl->m_Client->connected();
}

int NavDevice::GetMode() const
{
    return m_Impl->m_Mode;
}

void NavDevice::ViewChanged()
{
    if ( m_Impl->m_Session )
    {
        try
        {
            m_Impl->m_Adapter->NoteView();
            m_Impl->m_Session->native_camera_changed();
        }
        catch ( ... )
        {
        }
    }
}

void NavDevice::DocumentChanged()
{
    NavNewDocument();
    m_Impl->m_SettledDue = true;
    if ( m_Impl->m_PivotLocked )
    {
        m_Impl->m_PivotLocked = false;
        StatusChanged();
    }
    if ( m_Impl->m_Session )
    {
        try
        {
            m_Impl->m_Session->context_changed();
        }
        catch ( ... )
        {
        }
    }
}

void NavDevice::SetPivotLocked( bool lock )
{
    if ( !m_Impl->m_Session || !IsAvailable() )
    {
        return;
    }

    std::string command = "navigation.pivot.clear";
    if ( lock )
    {
        command = "navigation.pivot.lock";
    }

    Impl * impl = m_Impl.get();
    NavDevice * dev = this;
    std::shared_ptr < bool > alive = m_Impl->m_Alive;
    try
    {
        // Refused when the host has not chosen a pivot yet; then nothing changes.
        m_Impl->m_Client->execute_command( command, openaxis::Value::object(),
                                           [impl, dev, alive, lock]( const openaxis::Value & result, const openaxis::Value & )
        {
            if ( !*alive || !result.is_object() || !result.value( "accepted", false ) )
            {
                return;
            }
            impl->m_PivotLocked = lock;
            dev->StatusChanged();
        } );
    }
    catch ( ... )
    {
    }
}

bool NavDevice::IsPivotLocked() const
{
    return m_Impl->m_PivotLocked;
}

std::string NavDevice::GetStatus() const
{
    if ( m_Impl->m_Failed )
    {
        return "Unavailable";
    }
    if ( !m_Impl->m_Manager )
    {
        return "Off";
    }
    if ( IsAvailable() )
    {
        std::string status = "Connected";
        if ( m_Impl->m_Moving )
        {
            status += ", moving the view";
        }
        if ( m_Impl->m_PivotLocked )
        {
            status += ", pivot locked";
        }
        return status;
    }
    // An older host refuses the protocol; an unverified one is closed by the SDK.
    return "Looking for Rotatrix 1.6 or later";
}

void NavDevice::SetStatusCallback( void ( *cb )( void * ), void * data )
{
    m_Impl->m_StatusCB = cb;
    m_Impl->m_StatusData = data;
}

void NavDevice::StatusChanged()
{
    // The manager reports every retry; only a change people can see is passed on.
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

}

#else

namespace VSPGUI
{

struct NavDevice::Impl
{
};

NavDevice::NavDevice( VspGlWindow * )
{
}

NavDevice::~NavDevice()
{
}

bool NavDevice::IsBuilt()
{
    return false;
}

void NavDevice::Start()
{
}

void NavDevice::Stop()
{
}

bool NavDevice::IsStarted() const
{
    return false;
}

bool NavDevice::IsAvailable() const
{
    return false;
}

int NavDevice::GetMode() const
{
    return 0;
}

void NavDevice::ViewChanged()
{
}

void NavDevice::DocumentChanged()
{
}

void NavDevice::SetPivotLocked( bool )
{
}

bool NavDevice::IsPivotLocked() const
{
    return false;
}

std::string NavDevice::GetStatus() const
{
    return "Not built in";
}

void NavDevice::SetStatusCallback( void ( * )( void * ), void * )
{
}

}

#endif
