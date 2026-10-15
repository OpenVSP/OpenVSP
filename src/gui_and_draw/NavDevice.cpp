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
#include <mutex>
#include <vector>

#include <FL/Fl.H>

#include <openaxis/connection_manager.hpp>
#include <openaxis/diagnostics.hpp>

#include "main.h"
#include "MainGLWindow.h"
#include "NavController.h"

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
    }

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
    std::unique_ptr < openaxis::OpenAxisClient > m_Client;
    std::unique_ptr < openaxis::OpenAxisConnectionManager > m_Manager;
    bool m_Focused;
    bool m_Failed;              // The SDK could not start.
    std::string m_LastStatus;   // As last reported to the status callback.

    void ( *m_StatusCB )( void * );
    void * m_StatusData;

    // Whether an OpenVSP window has the keyboard focus, checked twice a second.
    static void PollFocus( void * data )
    {
        Impl * impl = ( Impl * )data;
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
    m_Impl->m_Failed = false;
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
    try
    {
        m_Impl->m_Listener = std::make_shared < AxisListener >( m_Impl->m_Win );
        m_Impl->m_Client.reset( new openaxis::OpenAxisClient( options, m_Impl->m_Listener ) );

        Impl * impl = m_Impl.get();
        openaxis::OpenAxisConnectionManagerOptions manager_options;
        manager_options.metadata = [impl]()
        {
            openaxis::ConnectionMetadata metadata;
            metadata.tags = { "app.openvsp" };
            metadata.axes = std::vector < std::string > { "tx", "ty", "tz", "rx", "ry", "rz" };
            metadata.focused = impl->m_Focused;
            return metadata;
        };
        m_Impl->m_Manager.reset( new openaxis::OpenAxisConnectionManager( *m_Impl->m_Client, manager_options ) );
        m_Impl->m_Manager->on_state = [this]( const openaxis::ConnectionStatus & )
        {
            StatusChanged();
        };
        m_Impl->m_Manager->start();
    }
    catch ( ... )
    {
        m_Impl->m_Manager.reset();
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

    try
    {
        m_Impl->m_Manager->stop();
        m_Impl->m_Manager.reset();
        m_Impl->m_Client->disconnect();
    }
    catch ( ... )
    {
    }
    m_Impl->m_Manager.reset();
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
        return "Connected";
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

std::string NavDevice::GetStatus() const
{
    return "Not built in";
}

void NavDevice::SetStatusCallback( void ( * )( void * ), void * )
{
}

}

#endif
