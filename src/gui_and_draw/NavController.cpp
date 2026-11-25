//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// NavController.cpp: turns mouse and key events in a GL window into view changes.
//
//////////////////////////////////////////////////////////////////////

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <cstring>

#include <FL/Fl.H>
#include <FL/Fl_Preferences.H>

#include "NavController.h"
#include "MainGLWindow.h"
#include "GraphicEngine.h"
#include "Display.h"
#include "LayoutMgr.h"
#include "Viewport.h"
#include "Camera.h"

#include "glm/ext.hpp"

namespace VSPGUI
{

static const int NAV_BUTTONS = FL_BUTTON1 | FL_BUTTON2 | FL_BUTTON3 | FL_BUTTON4 | FL_BUTTON5;
static const int NAV_MODIFIERS = FL_SHIFT | FL_CTRL | FL_ALT | FL_META;

// Zoom in and Zoom out: a wheel notch's factor, and how a held press repeats it, in seconds.
static const double NAV_STEP_ZOOM = 1.15;
static const double NAV_STEP_DELAY = 0.4;
static const double NAV_STEP_INTERVAL = 0.1;

const double NAV_SPACEMOUSE_SPEEDS[ NAV_NUM_SPACEMOUSE_SPEEDS ] = { 0.25, 0.5, 1.0, 2.0, 4.0 };

// As 3Dconnexion's driver and Blender start: push right and up to move the model so, pull
// toward you to bring it closer, tilt and twist to turn it as the cap turns.
static const int SPACEMOUSE_AXES[ NAV_NUM_AXES ] = { NAV_AXIS_PAN_X, NAV_AXIS_ZOOM, NAV_AXIS_PAN_Y, NAV_AXIS_TURN_X,
                                                     NAV_AXIS_TURN_Z, NAV_AXIS_TURN_Y };
static const bool SPACEMOUSE_REVERSE[ NAV_NUM_AXES ] = { false, false, true, false, false, true };

static bool IsStepZoom( int action )
{
    return action == NAV_ZOOM_IN || action == NAV_ZOOM_OUT;
}

NavSettings::NavSettings()
{
    m_StartupRotateMode = NAV_ROTATE_BALL;
    m_BallType = NAV_BALL_TRACKBALL;
    m_BallSize = 0.9;
    m_BallBorder = 0.5;
    m_BallSpeed = 0.5;
    m_TurntableGain = 0.4;

    m_WheelReverse = false;
    m_WheelAnchor = NAV_ANCHOR_POINTER;
    m_WheelStep = NAV_WHEEL_PER_EVENT;
    m_DragZoomReverse = false;
    m_DragZoomAnchor = NAV_ANCHOR_CENTER;
    m_DragZoomLaw = NAV_ZOOM_PER_EVENT;
    m_ZoomSpeed = 1.0;

    m_ShowBallOutline = true;

    m_Pivot = NAV_PIVOT_FIXED;

    m_EmulateMiddle = false;

    m_UseDevice = true;

    m_UseSpaceMouse = true;
    m_SpaceMouseSpeed = 1.0;
    for ( int i = 0; i < NAV_NUM_AXES; i++ )
    {
        m_SpaceMouseAxis[i] = SPACEMOUSE_AXES[i];
        m_SpaceMouseReverse[i] = SPACEMOUSE_REVERSE[i];
    }
    m_SpaceMouseRotate = true;
    m_SpaceMouseMove = true;
    m_SpaceMouseDominant = false;

    m_Profile = NavProfile::Preset( NAV_PRESET_OPENVSP );
    std::vector < NavBinding > device = NavProfile::DeviceDefaults();
    m_Profile.m_Bindings.insert( m_Profile.m_Bindings.end(), device.begin(), device.end() );
}

void NavSettings::StepSpaceMouseSpeed( int step )
{
    int now = 0;
    for ( int i = 1; i < NAV_NUM_SPACEMOUSE_SPEEDS; i++ )
    {
        if ( std::abs( NAV_SPACEMOUSE_SPEEDS[i] - m_SpaceMouseSpeed ) < std::abs( NAV_SPACEMOUSE_SPEEDS[now] - m_SpaceMouseSpeed ) )
        {
            now = i;
        }
    }
    now += step;
    if ( now < 0 )
    {
        now = 0;
    }
    if ( now >= NAV_NUM_SPACEMOUSE_SPEEDS )
    {
        now = NAV_NUM_SPACEMOUSE_SPEEDS - 1;
    }
    m_SpaceMouseSpeed = NAV_SPACEMOUSE_SPEEDS[ now ];
}

void NavSettings::ApplyPreset( int preset )
{
    NavSettings def;

    std::vector < NavBinding > device = m_Profile.DeviceBindings();
    m_Profile = NavProfile::Preset( preset );
    m_Profile.m_Bindings.insert( m_Profile.m_Bindings.end(), device.begin(), device.end() );
    m_StartupRotateMode = def.m_StartupRotateMode;
    m_BallType = def.m_BallType;
    m_Pivot = def.m_Pivot;

    m_WheelReverse = def.m_WheelReverse;
    m_WheelAnchor = def.m_WheelAnchor;
    m_WheelStep = def.m_WheelStep;
    m_DragZoomReverse = def.m_DragZoomReverse;
    m_DragZoomAnchor = def.m_DragZoomAnchor;
    m_DragZoomLaw = def.m_DragZoomLaw;

    m_BallSize = def.m_BallSize;
    m_BallBorder = def.m_BallBorder;
    m_BallSpeed = def.m_BallSpeed;
    m_TurntableGain = def.m_TurntableGain;
    m_ZoomSpeed = def.m_ZoomSpeed;
    m_ShowBallOutline = def.m_ShowBallOutline;
    m_EmulateMiddle = def.m_EmulateMiddle;

    if ( preset == NAV_PRESET_OPENVSP )
    {
        return;
    }

    // As the CAD programs do: the wheel and drags zoom by how far they go.
    m_WheelStep = NAV_WHEEL_PROPORTIONAL;
    m_DragZoomLaw = NAV_ZOOM_CONTINUOUS;

    if ( preset == NAV_PRESET_SOLIDWORKS || preset == NAV_PRESET_INVENTOR_FUSION || preset == NAV_PRESET_HYPERWORKS )
    {
        m_WheelReverse = true;
    }

    // ParaView's wheel zooms toward the middle of the view.
    if ( preset == NAV_PRESET_PARAVIEW )
    {
        m_WheelAnchor = NAV_ANCHOR_CENTER;
    }

    // Maya and Rhino turn about the world's vertical axis.
    if ( preset == NAV_PRESET_MAYA || preset == NAV_PRESET_RHINO )
    {
        m_StartupRotateMode = NAV_ROTATE_TURNTABLE;
    }

    // ParaView, EnSight, Pointwise and STAR-CCM+ turn about the screen's axes with each move.
    if ( preset == NAV_PRESET_PARAVIEW || preset == NAV_PRESET_ENSIGHT || preset == NAV_PRESET_POINTWISE ||
         preset == NAV_PRESET_STARCCM )
    {
        m_BallType = NAV_BALL_TUMBLER;
    }

    // Inventor and Fusion, 3ds Max and Abaqus turn the view inside a circle and roll it outside.
    if ( preset == NAV_PRESET_INVENTOR_FUSION || preset == NAV_PRESET_3DS_MAX || preset == NAV_PRESET_ABAQUS )
    {
        m_BallType = NAV_BALL_ROUNDED_ARCBALL;
    }

    // Onshape and HyperWorks rotate about the point under the cursor.
    if ( preset == NAV_PRESET_ONSHAPE || preset == NAV_PRESET_HYPERWORKS )
    {
        m_Pivot = NAV_PIVOT_UNDER_CURSOR;
    }
}

NavPress NavSettings::Effective( const NavPress & press ) const
{
    // A press that only reaches the tool picks counts as free.
    NavPress p = press;
    const NavBinding * drag = m_Profile.Match( p, NAV_ON_DRAG );
    if ( ( drag && drag->m_Action != NAV_NONE ) || m_Profile.Match( p, NAV_ON_CLICK ) )
    {
        return p;
    }

    if ( m_EmulateMiddle && ( p.m_Buttons & FL_BUTTON1 ) && !( p.m_Buttons & FL_BUTTON2 ) && ( p.m_Modifiers & FL_ALT ) )
    {
        p.m_Buttons = ( p.m_Buttons & ~FL_BUTTON1 ) | FL_BUTTON2;
        p.m_Modifiers &= ~FL_ALT;
        if ( p.m_PressedButton == FL_BUTTON1 )
        {
            p.m_PressedButton = FL_BUTTON2;
        }
    }
    return p;
}

int NavSettings::FindPreset() const
{
    for ( int i = 0; i < NAV_NUM_PRESETS; i++ )
    {
        NavSettings p = *this;
        p.ApplyPreset( i );

        if ( p.m_Profile.SameBehavior( m_Profile ) &&
             p.m_StartupRotateMode == m_StartupRotateMode &&
             p.m_BallType == m_BallType &&
             p.m_Pivot == m_Pivot &&
             p.m_WheelReverse == m_WheelReverse &&
             p.m_WheelAnchor == m_WheelAnchor &&
             p.m_WheelStep == m_WheelStep &&
             p.m_DragZoomReverse == m_DragZoomReverse &&
             p.m_DragZoomAnchor == m_DragZoomAnchor &&
             p.m_DragZoomLaw == m_DragZoomLaw &&
             p.m_BallSize == m_BallSize &&
             p.m_BallBorder == m_BallBorder &&
             p.m_BallSpeed == m_BallSpeed &&
             p.m_TurntableGain == m_TurntableGain &&
             p.m_ZoomSpeed == m_ZoomSpeed &&
             p.m_ShowBallOutline == m_ShowBallOutline &&
             p.m_EmulateMiddle == m_EmulateMiddle )
        {
            return i;
        }
    }
    return -1;
}

// Choices are stored by name so that reordering the enums never reinterprets a saved value.
// Replace a stored number that is not finite or is out of range with the default.
static double Checked( double v, double lo, double hi, double def )
{
    if ( !std::isfinite( v ) || v < lo || v > hi )
    {
        return def;
    }
    return v;
}

// A choice stored by name.  names[i] is the name of value i; an unknown name gives def.
static int GetChoice( Fl_Preferences & grp, const char * key, const char * const * names, int n, int def )
{
    char buf[64];
    grp.get( key, buf, names[def], sizeof( buf ) );
    for ( int i = 0; i < n; i++ )
    {
        if ( strcmp( buf, names[i] ) == 0 )
        {
            return i;
        }
    }
    return def;
}

static bool GetFlag( Fl_Preferences & grp, const char * key, bool def )
{
    int defval = 0;
    if ( def )
    {
        defval = 1;
    }
    int v = defval;
    grp.get( key, v, defval );
    return v != 0;
}

static const char * const ANCHOR_NAMES[] = { "Pointer", "Center" };
static const char * const WHEEL_STEP_NAMES[] = { "PerEvent", "Proportional" };
static const char * const ZOOM_LAW_NAMES[] = { "PerEvent", "Continuous" };
static const char * const PIVOT_NAMES[] = { "Fixed", "UnderCursor" };
static const char * const AXIS_NAMES[] = { "None", "PanX", "PanY", "Zoom", "TurnX", "TurnY", "TurnZ" };

// The axes as text: "PanX Zoom -PanY ...", a minus for a reversed axis.
static std::string AxesText( const int * axis, const bool * reverse )
{
    std::string t;
    for ( int i = 0; i < NAV_NUM_AXES; i++ )
    {
        if ( i > 0 )
        {
            t += " ";
        }
        if ( reverse[i] )
        {
            t += "-";
        }
        t += AXIS_NAMES[ axis[i] ];
    }
    return t;
}

// Read AxesText's text.  On any error, return false and leave the axes as they were.
static bool ReadAxesText( const std::string & text, int * axis, bool * reverse )
{
    int a[ NAV_NUM_AXES ];
    bool r[ NAV_NUM_AXES ];
    size_t start = 0;
    for ( int i = 0; i < NAV_NUM_AXES; i++ )
    {
        if ( start > text.size() )
        {
            return false;
        }
        size_t end = text.find( ' ', start );
        if ( end == std::string::npos )
        {
            end = text.size();
        }
        std::string name = text.substr( start, end - start );
        start = end + 1;

        r[i] = ( !name.empty() && name[0] == '-' );
        if ( r[i] )
        {
            name = name.substr( 1 );
        }
        a[i] = -1;
        for ( int j = 0; j < NAV_NUM_AXIS_ACTIONS; j++ )
        {
            if ( name == AXIS_NAMES[j] )
            {
                a[i] = j;
            }
        }
        if ( a[i] < 0 )
        {
            return false;
        }
    }
    for ( int i = 0; i < NAV_NUM_AXES; i++ )
    {
        axis[i] = a[i];
        reverse[i] = r[i];
    }
    return true;
}

// The bindings' text, as a profile's.
static std::string BindingsText( const std::vector < NavBinding > & bindings )
{
    NavProfile p;
    p.m_Bindings = bindings;
    return p.BindingsText();
}

// USER_L writes and reads numbers in the C locale, so a decimal comma never gets into the file.
void NavSettings::Load()
{
    Fl_Preferences prefs( Fl_Preferences::USER_L, "openvsp.org", "VSP" );
    Fl_Preferences grp( prefs, "Interaction" );

    char buf[64];

    grp.get( "StartupRotate", buf, "Ball", sizeof( buf ) );
    m_StartupRotateMode = NAV_ROTATE_BALL;
    if ( strcmp( buf, "Turntable" ) == 0 )
    {
        m_StartupRotateMode = NAV_ROTATE_TURNTABLE;
    }

    grp.get( "BallType", buf, "Trackball", sizeof( buf ) );
    m_BallType = NAV_BALL_TRACKBALL;
    if ( strcmp( buf, "RoundedArcball" ) == 0 )
    {
        m_BallType = NAV_BALL_ROUNDED_ARCBALL;
    }
    else if ( strcmp( buf, "Classic" ) == 0 )
    {
        m_BallType = NAV_BALL_CLASSIC;
    }
    else if ( strcmp( buf, "Tumbler" ) == 0 )
    {
        m_BallType = NAV_BALL_TUMBLER;
    }

    NavSettings def;
    grp.get( "BallSize", m_BallSize, def.m_BallSize );
    grp.get( "BallBorder", m_BallBorder, def.m_BallBorder );
    // Before Ball Speed, the Rounded Arcball's speed was an angle scale: 2 is a speed of 1.
    double angle_scale = 2.0 * def.m_BallSpeed;
    grp.get( "BallAngleScale", angle_scale, angle_scale );
    grp.get( "BallSpeed", m_BallSpeed, 0.5 * angle_scale );
    grp.get( "TurntableGain", m_TurntableGain, def.m_TurntableGain );

    m_BallSize = Checked( m_BallSize, 0.2, 1.5, def.m_BallSize );
    m_BallBorder = Checked( m_BallBorder, 0.0, 1.0, def.m_BallBorder );
    m_BallSpeed = Checked( m_BallSpeed, 0.1, 2.0, def.m_BallSpeed );
    m_TurntableGain = Checked( m_TurntableGain, 0.01, 5.0, def.m_TurntableGain );

    m_WheelReverse = GetFlag( grp, "WheelReverse", def.m_WheelReverse );
    m_WheelAnchor = GetChoice( grp, "WheelAnchor", ANCHOR_NAMES, 2, def.m_WheelAnchor );
    m_WheelStep = GetChoice( grp, "WheelStep", WHEEL_STEP_NAMES, 2, def.m_WheelStep );
    m_DragZoomReverse = GetFlag( grp, "DragZoomReverse", def.m_DragZoomReverse );
    m_DragZoomAnchor = GetChoice( grp, "DragZoomAnchor", ANCHOR_NAMES, 2, def.m_DragZoomAnchor );
    m_DragZoomLaw = GetChoice( grp, "DragZoomLaw", ZOOM_LAW_NAMES, 2, def.m_DragZoomLaw );
    grp.get( "ZoomSpeed", m_ZoomSpeed, def.m_ZoomSpeed );
    m_ZoomSpeed = Checked( m_ZoomSpeed, 0.1, 10.0, def.m_ZoomSpeed );

    m_ShowBallOutline = GetFlag( grp, "ShowBallOutline", def.m_ShowBallOutline );

    m_Pivot = GetChoice( grp, "Pivot", PIVOT_NAMES, 2, def.m_Pivot );

    m_EmulateMiddle = GetFlag( grp, "EmulateMiddle", def.m_EmulateMiddle );
    m_UseDevice = GetFlag( grp, "UseDevice", def.m_UseDevice );
    m_UseSpaceMouse = GetFlag( grp, "UseSpaceMouse", def.m_UseSpaceMouse );
    grp.get( "SpaceMouseSpeed", m_SpaceMouseSpeed, def.m_SpaceMouseSpeed );
    m_SpaceMouseSpeed = Checked( m_SpaceMouseSpeed, 0.1, 10.0, def.m_SpaceMouseSpeed );
    char axes[ 256 ];
    grp.get( "SpaceMouseAxes", axes, "", sizeof( axes ) );
    for ( int i = 0; i < NAV_NUM_AXES; i++ )
    {
        m_SpaceMouseAxis[i] = def.m_SpaceMouseAxis[i];
        m_SpaceMouseReverse[i] = def.m_SpaceMouseReverse[i];
    }
    ReadAxesText( axes, m_SpaceMouseAxis, m_SpaceMouseReverse );
    m_SpaceMouseRotate = GetFlag( grp, "SpaceMouseRotate", def.m_SpaceMouseRotate );
    m_SpaceMouseMove = GetFlag( grp, "SpaceMouseMove", def.m_SpaceMouseMove );
    m_SpaceMouseDominant = GetFlag( grp, "SpaceMouseDominant", def.m_SpaceMouseDominant );

    grp.get( "ClickDistance", m_Profile.m_ClickDistance, def.m_Profile.m_ClickDistance );
    if ( m_Profile.m_ClickDistance < 0 || m_Profile.m_ClickDistance > 50 )
    {
        m_Profile.m_ClickDistance = def.m_Profile.m_ClickDistance;
    }
    m_Profile.m_ChordEnds = GetFlag( grp, "ChordEnds", def.m_Profile.m_ChordEnds );

    // Mouse bindings that do not read give the default profile whole; device button bindings
    // that do not read give the default device bindings.
    std::vector < NavBinding > mouse = def.m_Profile.MouseBindings();
    std::vector < NavBinding > device = def.m_Profile.DeviceBindings();
    NavProfile read;
    char * text = nullptr;
    grp.get( "Bindings", text, "" );
    if ( text && grp.entryExists( "Bindings" ) )
    {
        if ( read.SetBindingsText( text ) )
        {
            mouse = read.MouseBindings();
        }
        else
        {
            m_Profile.m_ClickDistance = def.m_Profile.m_ClickDistance;
            m_Profile.m_ChordEnds = def.m_Profile.m_ChordEnds;
        }
    }
    free( text );
    text = nullptr;
    grp.get( "DeviceBindings", text, "" );
    if ( text && grp.entryExists( "DeviceBindings" ) && read.SetBindingsText( text ) )
    {
        device = read.DeviceBindings();
    }
    free( text );
    m_Profile.m_Bindings = mouse;
    m_Profile.m_Bindings.insert( m_Profile.m_Bindings.end(), device.begin(), device.end() );
}

// Only values that differ from the defaults are stored, so a later change of a default
// reaches everyone who never changed that setting.
static void SaveName( Fl_Preferences & grp, const char * key, const char * val, const char * def )
{
    if ( strcmp( val, def ) == 0 )
    {
        grp.deleteEntry( key );
    }
    else
    {
        grp.set( key, val );
    }
}

static void SaveDouble( Fl_Preferences & grp, const char * key, double val, double def )
{
    if ( val == def )
    {
        grp.deleteEntry( key );
    }
    else
    {
        grp.set( key, val );
    }
}

static void SaveInt( Fl_Preferences & grp, const char * key, int val, int def )
{
    if ( val == def )
    {
        grp.deleteEntry( key );
    }
    else
    {
        grp.set( key, val );
    }
}

static void SaveFlag( Fl_Preferences & grp, const char * key, bool val, bool def )
{
    if ( val == def )
    {
        grp.deleteEntry( key );
    }
    else
    {
        grp.set( key, ( int )val );
    }
}

void NavSettings::Save() const
{
    Fl_Preferences prefs( Fl_Preferences::USER_L, "openvsp.org", "VSP" );
    Fl_Preferences grp( prefs, "Interaction" );
    NavSettings def;

    grp.set( "FormatVersion", 1 );

    const char * rotate_names[] = { "Ball", "Turntable" };
    SaveName( grp, "StartupRotate", rotate_names[ m_StartupRotateMode ], rotate_names[ def.m_StartupRotateMode ] );

    const char * ball_names[] = { "RoundedArcball", "Classic", "Tumbler", "Trackball" };
    SaveName( grp, "BallType", ball_names[ m_BallType ], ball_names[ def.m_BallType ] );

    SaveDouble( grp, "BallSize", m_BallSize, def.m_BallSize );
    SaveDouble( grp, "BallBorder", m_BallBorder, def.m_BallBorder );
    SaveDouble( grp, "BallSpeed", m_BallSpeed, def.m_BallSpeed );
    grp.deleteEntry( "BallAngleScale" );
    SaveDouble( grp, "TurntableGain", m_TurntableGain, def.m_TurntableGain );

    SaveFlag( grp, "WheelReverse", m_WheelReverse, def.m_WheelReverse );
    SaveName( grp, "WheelAnchor", ANCHOR_NAMES[ m_WheelAnchor ], ANCHOR_NAMES[ def.m_WheelAnchor ] );
    SaveName( grp, "WheelStep", WHEEL_STEP_NAMES[ m_WheelStep ], WHEEL_STEP_NAMES[ def.m_WheelStep ] );
    SaveFlag( grp, "DragZoomReverse", m_DragZoomReverse, def.m_DragZoomReverse );
    SaveName( grp, "DragZoomAnchor", ANCHOR_NAMES[ m_DragZoomAnchor ], ANCHOR_NAMES[ def.m_DragZoomAnchor ] );
    SaveName( grp, "DragZoomLaw", ZOOM_LAW_NAMES[ m_DragZoomLaw ], ZOOM_LAW_NAMES[ def.m_DragZoomLaw ] );
    SaveDouble( grp, "ZoomSpeed", m_ZoomSpeed, def.m_ZoomSpeed );

    SaveFlag( grp, "ShowBallOutline", m_ShowBallOutline, def.m_ShowBallOutline );
    SaveName( grp, "Pivot", PIVOT_NAMES[ m_Pivot ], PIVOT_NAMES[ def.m_Pivot ] );
    SaveFlag( grp, "EmulateMiddle", m_EmulateMiddle, def.m_EmulateMiddle );
    SaveFlag( grp, "UseDevice", m_UseDevice, def.m_UseDevice );
    SaveFlag( grp, "UseSpaceMouse", m_UseSpaceMouse, def.m_UseSpaceMouse );
    SaveDouble( grp, "SpaceMouseSpeed", m_SpaceMouseSpeed, def.m_SpaceMouseSpeed );
    SaveName( grp, "SpaceMouseAxes", AxesText( m_SpaceMouseAxis, m_SpaceMouseReverse ).c_str(),
              AxesText( def.m_SpaceMouseAxis, def.m_SpaceMouseReverse ).c_str() );
    SaveFlag( grp, "SpaceMouseRotate", m_SpaceMouseRotate, def.m_SpaceMouseRotate );
    SaveFlag( grp, "SpaceMouseMove", m_SpaceMouseMove, def.m_SpaceMouseMove );
    SaveFlag( grp, "SpaceMouseDominant", m_SpaceMouseDominant, def.m_SpaceMouseDominant );
    SaveName( grp, "Bindings", BindingsText( m_Profile.MouseBindings() ).c_str(),
              BindingsText( def.m_Profile.MouseBindings() ).c_str() );
    SaveName( grp, "DeviceBindings", BindingsText( m_Profile.DeviceBindings() ).c_str(),
              BindingsText( def.m_Profile.DeviceBindings() ).c_str() );
    SaveInt( grp, "ClickDistance", m_Profile.m_ClickDistance, def.m_Profile.m_ClickDistance );
    SaveFlag( grp, "ChordEnds", m_Profile.m_ChordEnds, def.m_Profile.m_ChordEnds );

    prefs.flush();
}

NavController::NavController( VspGlWindow * win )
{
    m_Win = win;
    m_RotateMode = m_Settings.m_StartupRotateMode;
    m_Rotation = nullptr;
    m_Action = NAV_NONE;
    m_PressEventX = 0;
    m_PressEventY = 0;
    m_PressX = 0;
    m_PressY = 0;
    m_PrevX = 0;
    m_PrevY = 0;
    m_RotateEnabled = true;
    m_ZoomBoxArmed = false;

    m_Pending = false;
    m_PendingDrag = nullptr;
    m_PendingClick = nullptr;
    m_Moved = false;
    m_Chord = false;
    m_ChordEnded = false;

    m_ActionBeforeBox = NAV_NONE;
    m_PressModifiers = 0;

    m_PivotCamera = nullptr;
    m_StickyCOR = glm::vec3( 0.0f );

    m_CommandCB = nullptr;
    m_CommandData = nullptr;
    m_DeviceZoom = NAV_NONE;

    SetSettings( m_Settings );
}

NavController::~NavController()
{
    Fl::remove_timeout( RepeatZoom, this );
    Fl::remove_timeout( RepeatDeviceZoom, this );
}

void NavController::SetCommandCallback( void ( *cb )( int action, void * data ), void * data )
{
    m_CommandCB = cb;
    m_CommandData = data;
}

void NavController::DeviceButton( const std::string & name, bool press )
{
    if ( !press )
    {
        if ( NavSameDeviceButton( name, m_DeviceZoomButton ) )
        {
            Fl::remove_timeout( RepeatDeviceZoom, this );
            m_DeviceZoomButton.clear();
            m_DeviceZoom = NAV_NONE;
        }
        return;
    }

    NavPress p;
    p.m_DeviceButton = name;
    p.m_Modifiers = HeldModifiers();
    p.m_HeldKeys = m_HeldKeys;
    const NavBinding * b = m_Settings.m_Profile.Match( p, NAV_ON_DRAG );
    if ( !b )
    {
        return;
    }

    if ( IsStepZoom( b->m_Action ) )
    {
        // No pointer goes with the device: zoom at the view's center.
        Fl::remove_timeout( RepeatDeviceZoom, this );
        m_DeviceZoomButton = name;
        m_DeviceZoom = b->m_Action;
        StepZoom( m_DeviceZoom, NAV_ANCHOR_CENTER, 0, 0 );
        Fl::add_timeout( NAV_STEP_DELAY, RepeatDeviceZoom, this );
    }
    else if ( NavIsCommand( b->m_Action ) && m_CommandCB )
    {
        m_CommandCB( b->m_Action, m_CommandData );
    }
}

void NavController::ReleaseDeviceButtons()
{
    Fl::remove_timeout( RepeatDeviceZoom, this );
    m_DeviceZoomButton.clear();
    m_DeviceZoom = NAV_NONE;
}

int NavController::HeldModifiers()
{
    // No event comes with a device button, and the last event's state may be old: after
    // switching programs it can still show the key that switched.
    const int keys[][3] = { { FL_Shift_L, FL_Shift_R, FL_SHIFT }, { FL_Control_L, FL_Control_R, FL_CTRL },
                            { FL_Alt_L, FL_Alt_R, FL_ALT }, { FL_Meta_L, FL_Meta_R, FL_META } };
    int mods = 0;
    for ( int i = 0; i < 4; i++ )
    {
        if ( Fl::get_key( keys[i][0] ) || Fl::get_key( keys[i][1] ) )
        {
            mods |= keys[i][2];
        }
    }
    return mods;
}

void NavController::RepeatDeviceZoom( void * data )
{
    NavController * nav = ( NavController * )data;
    if ( IsStepZoom( nav->m_DeviceZoom ) )
    {
        nav->StepZoom( nav->m_DeviceZoom, NAV_ANCHOR_CENTER, 0, 0 );
        Fl::repeat_timeout( NAV_STEP_INTERVAL, RepeatDeviceZoom, data );
    }
}

void NavController::RollView( double angle )
{
    VSPGraphic::Camera * camera = m_Win->getGraphicEngine()->getDisplay()->getCamera();
    if ( !camera )
    {
        return;
    }
    camera->rotateEye( glm::dvec3( 0.0, 0.0, angle ) );
    m_Win->UpdateRotationParms();
    m_Win->UpdatePanParms();
    m_Win->ViewChanged();
    m_Win->redraw();
}

void NavController::SetSettings( const NavSettings & settings )
{
    // New bindings end any gesture.
    if ( !settings.m_Profile.SameBehavior( m_Settings.m_Profile ) )
    {
        Abandon();
    }

    m_Settings = settings;

    m_RoundedArcball.setShape( m_Settings.m_BallSize, m_Settings.m_BallBorder );
    m_RoundedArcball.setAngleScale( 2.0 * m_Settings.m_BallSpeed );
    // At a Ball Speed of 1 the classic ball and the trackball turn twice as fast as OpenVSP's
    // classic ball always has, about as fast as the Rounded Arcball; 0.5 is the classic speed.
    m_ClassicBall.setSpeed( 2.0 * m_Settings.m_BallSpeed );
    m_Trackball.setSpeed( 2.0 * m_Settings.m_BallSpeed );
}

VSPGraphic::RotationMethod * NavController::CurrentRotation()
{
    // The gain is per screen point; mouse locations are in pixels.
    double gain = m_Settings.m_TurntableGain * M_PI / 180.0 / m_Win->pixels_per_unit();

    if ( m_RotateMode == NAV_ROTATE_TURNTABLE )
    {
        m_Turntable.setParams( gain, glm::dvec3( 0.0, 0.0, 1.0 ) );
        return &m_Turntable;
    }

    if ( m_Settings.m_BallType == NAV_BALL_CLASSIC )
    {
        return &m_ClassicBall;
    }
    if ( m_Settings.m_BallType == NAV_BALL_TRACKBALL )
    {
        return &m_Trackball;
    }
    if ( m_Settings.m_BallType == NAV_BALL_TUMBLER )
    {
        m_Tumbler.setGain( gain );
        return &m_Tumbler;
    }
    return &m_RoundedArcball;
}

static int CountButtons( int buttons )
{
    int n = 0;
    if ( buttons & FL_BUTTON1 )
    {
        n++;
    }
    if ( buttons & FL_BUTTON2 )
    {
        n++;
    }
    if ( buttons & FL_BUTTON3 )
    {
        n++;
    }
    if ( buttons & FL_BUTTON4 )
    {
        n++;
    }
    if ( buttons & FL_BUTTON5 )
    {
        n++;
    }
    return n;
}

// The mouse button bit for the button of the current event.
static int EventButtonBit()
{
    switch ( Fl::event_button() )
    {
    case FL_LEFT_MOUSE:
        return FL_BUTTON1;
    case FL_MIDDLE_MOUSE:
        return FL_BUTTON2;
    case FL_RIGHT_MOUSE:
        return FL_BUTTON3;
    case FL_BACK_MOUSE:
        return FL_BUTTON4;
    case FL_FORWARD_MOUSE:
        return FL_BUTTON5;
    default:
        return 0;
    }
}

void NavController::Push( int x, int y )
{
    int buttons = Fl::event_state() & NAV_BUTTONS;
    int pressed = EventButtonBit();

    // Other buttons do nothing during a box selection.
    if ( m_Action == NAV_BOX_SELECT )
    {
        return;
    }

    if ( ( buttons & ~pressed ) != 0 )
    {
        // Another button joined buttons already down; choose again from the new set.  The
        // press may have selected another viewport.
        HoldCurrentCamera();
        Begin( buttons, pressed, x, y, false );
    }
    else
    {
        m_HeldCameras.clear();
        HoldCurrentCamera();
        m_ChordEnded = false;

        Begin( buttons, pressed, x, y, true );
    }
}

void NavController::Begin( int buttons, int pressed, int x, int y, bool allow_pick )
{
    NavAction before = m_Action;

    Fl::remove_timeout( RepeatZoom, this );
    if ( m_Action == NAV_ZOOM_BOX )
    {
        EndZoomBox( false, x, y );
    }
    EndPivot( true );
    ClearOutline();

    m_Action = NAV_NONE;
    m_Pending = false;
    m_PendingDrag = nullptr;
    m_PendingClick = nullptr;
    m_Moved = false;
    m_PressEventX = Fl::event_x();
    m_PressEventY = Fl::event_y();
    m_PressX = x;
    m_PressY = y;
    m_PrevX = x;
    m_PrevY = y;

    m_PressModifiers = Fl::event_state() & NAV_MODIFIERS;

    if ( m_ZoomBoxArmed && pressed != 0 && buttons != FL_BUTTON1 )
    {
        // Any other press gives up the zoom box.
        m_ZoomBoxArmed = false;
    }

    if ( m_ZoomBoxArmed && pressed != 0 && buttons == FL_BUTTON1 )
    {
        m_ZoomBoxArmed = false;
        m_Action = NAV_ZOOM_BOX;
        return;
    }

    // Fit Model's box selection takes the left button before any binding, as in 3.55: with
    // Shift, or always in box mode, while Fit Model is shown.  3.55 gave left + right to zoom
    // first, so the right button may not be down.
    if ( pressed == FL_BUTTON1 && ( buttons & FL_BUTTON3 ) == 0 &&
         m_Win->StartsBoxSelect( Fl::event_state() & NAV_MODIFIERS ) )
    {
        m_ActionBeforeBox = before;
        Start( NAV_BOX_SELECT, x, y );
        return;
    }

    NavPress press;
    press.m_Buttons = buttons;
    press.m_Modifiers = Fl::event_state() & NAV_MODIFIERS;
    press.m_HeldKeys = m_HeldKeys;
    press.m_PressedButton = pressed;

    // Where the press is, first: a binding for it may take Alt + left before the middle button does.
    if ( m_Settings.m_Profile.NeedsWhere() )
    {
        glm::vec3 hit;
        press.m_Where = NAV_ON_BACKGROUND;
        if ( m_Win->ProbeSurface( m_PressEventX, m_PressEventY, hit ) )
        {
            press.m_Where = NAV_ON_GEOMETRY;
        }
    }
    press = m_Settings.Effective( press );

    if ( CountButtons( press.m_Buttons ) > 1 )
    {
        m_Chord = true;
    }

    const NavBinding * drag = m_Settings.m_Profile.Match( press, NAV_ON_DRAG );
    m_PendingClick = m_Settings.m_Profile.Match( press, NAV_ON_CLICK );

    if ( drag && drag->m_PickFirst && allow_pick && m_Win->PickAtPress() )
    {
        m_PendingClick = nullptr;
        return;
    }

    if ( drag && IsStepZoom( drag->m_Action ) )
    {
        // A step zoom acts at the press, without waiting to see whether it is a click.
        m_PendingClick = nullptr;
        Start( drag->m_Action, x, y );
        return;
    }

    if ( drag && NavIsCommand( drag->m_Action ) )
    {
        // So does a command, once: only at a press, not when a chord's other button lets go.
        m_PendingClick = nullptr;
        if ( m_CommandCB && pressed != 0 )
        {
            m_CommandCB( drag->m_Action, m_CommandData );
        }
        return;
    }

    if ( m_Settings.m_Profile.m_ClickDistance > 0 && ( m_PendingClick || CountButtons( press.m_Buttons ) == 1 ) )
    {
        // Wait to see whether this is a click, or the first button of a chord.
        m_Pending = true;
        m_PendingDrag = drag;
        return;
    }

    if ( drag )
    {
        Start( drag->m_Action, x, y );
    }
}

void NavController::HoldCurrentCamera()
{
    VSPGraphic::Camera * camera = m_Win->getGraphicEngine()->getDisplay()->getCamera();
    if ( camera && std::find( m_HeldCameras.begin(), m_HeldCameras.end(), camera ) == m_HeldCameras.end() )
    {
        camera->holdView();
        m_HeldCameras.push_back( camera );
    }
}

void NavController::Start( NavAction action, int x, int y )
{
    m_Action = action;

    if ( m_Action == NAV_ROTATE && m_RotateEnabled )
    {
        VSPGraphic::Camera * camera = m_Win->getGraphicEngine()->getDisplay()->getCamera();

        m_Rotation = CurrentRotation();
        if ( camera )
        {
            StartPivot( camera, x, y );
            m_Rotation->begin( camera, x, y );

            VSPGraphic::Viewport * viewport = m_Win->getGraphicEngine()->getDisplay()->getViewport();
            if ( m_Settings.m_ShowBallOutline && viewport )
            {
                float cx, cy;
                std::vector < float > radii;
                m_Rotation->outline( camera, cx, cy, radii );
                viewport->setOverlayCircles( cx, cy, radii );
            }
        }
    }
    else if ( m_Action == NAV_BOX_SELECT )
    {
        m_Win->BeginBoxSelect( x, y );
    }
    else if ( IsStepZoom( m_Action ) )
    {
        StepZoom( m_Action, m_Settings.m_WheelAnchor, x, y );
        Fl::add_timeout( NAV_STEP_DELAY, RepeatZoom, this );
    }
}

void NavController::Click( const NavBinding * b, int x, int y )
{
    if ( !b )
    {
        return;
    }

    if ( b->m_Action == NAV_SET_PIVOT )
    {
        VSPGraphic::Camera * camera = m_Win->getGraphicEngine()->getDisplay()->getCamera();
        glm::vec3 hit;
        if ( camera && m_Win->ProbeSurface( m_PressEventX, m_PressEventY, hit ) )
        {
            // As Set Rotation Center does.
            camera->setCOR( -hit.x, -hit.y, -hit.z );
            camera->center();
            m_Win->UpdateCORParms();
            m_Win->UpdatePanParms();
        }
    }
    else if ( b->m_Action == NAV_SELECT )
    {
        glm::vec3 hit;
        std::string geom;
        m_Win->ProbeSurface( m_PressEventX, m_PressEventY, hit, &geom );

        int modifiers = m_PressModifiers;
        bool toggle = ( modifiers & ( FL_CTRL | FL_META ) ) != 0;
        bool add = ( modifiers & FL_SHIFT ) != 0 && !toggle;
        m_Win->ClickSelect( geom, add, toggle );
    }
}

void NavController::Drag( int x, int y )
{
    VSPGraphic::Display * display = m_Win->getGraphicEngine()->getDisplay();

    if ( m_Pending )
    {
        double dx = x - m_PressX;
        double dy = y - m_PressY;
        double limit = m_Settings.m_Profile.m_ClickDistance * m_Win->pixels_per_unit();
        if ( dx * dx + dy * dy <= limit * limit )
        {
            return;
        }

        // A drag after all: start it at the press, then follow the mouse from there.
        m_Pending = false;
        m_PendingClick = nullptr;
        if ( m_PendingDrag )
        {
            Start( m_PendingDrag->m_Action, m_PressX, m_PressY );
        }
    }
    m_Moved = true;

    switch ( m_Action )
    {
    case NAV_ROTATE:
        if ( m_RotateEnabled && m_Rotation && display->getCamera() )
        {
            m_Rotation->drag( display->getCamera(), x, y );
            m_Win->UpdateRotationParms();
            if ( m_PivotCamera )
            {
                m_Win->UpdatePanParms();
            }
        }
        break;

    case NAV_ROLL:
        display->rotateZ( m_PrevX, m_PrevY, x, y );
        m_Win->UpdateRotationParms();
        if ( m_PivotCamera )
        {
            m_Win->UpdatePanParms();
        }
        break;

    case NAV_PAN:
        display->pan( m_PrevX, m_PrevY, x, y );
        m_Win->UpdatePanParms();
        break;

    case NAV_ZOOM:
        DragZoom( x, y );
        break;

    case NAV_ZOOM_BOX:
        if ( display->getViewport() )
        {
            display->getViewport()->setOverlayBox( true, m_PressX, m_PressY, x, y );
        }
        break;

    default:
        break;
    }

    m_PrevX = x;
    m_PrevY = y;
}

void NavController::Release( int x, int y )
{
    int released = EventButtonBit();
    int buttons = Fl::event_state() & NAV_BUTTONS & ~released;

    Fl::remove_timeout( RepeatZoom, this );

    if ( m_Action == NAV_BOX_SELECT )
    {
        // Only the left button ends a box selection.
        if ( released != FL_BUTTON1 )
        {
            return;
        }
        m_Win->EndBoxSelect( true );

        // Buttons pressed during the box did nothing; resume only a gesture from before it.
        m_Action = m_ActionBeforeBox;
        m_ActionBeforeBox = NAV_NONE;
    }

    if ( m_Action == NAV_ZOOM_BOX )
    {
        EndZoomBox( true, x, y );
        m_Action = NAV_NONE;
    }
    EndPivot( true );
    ClearOutline();

    // A release before the press became a drag is a click.
    if ( m_Pending || ( !m_Moved && m_Settings.m_Profile.m_ClickDistance == 0 ) )
    {
        const NavBinding * click = m_PendingClick;
        m_Pending = false;
        m_PendingClick = nullptr;
        Click( click, x, y );
    }
    m_PendingClick = nullptr;

    if ( m_Chord && m_Settings.m_Profile.m_ChordEnds )
    {
        // Releasing a button of a chord ends it; the rest of the buttons do nothing.
        m_ChordEnded = true;
    }

    if ( buttons != 0 && m_Action != NAV_NONE && !m_ChordEnded )
    {
        // Buttons are still down; carry on with whatever they now mean, from here.
        Begin( buttons, 0, x, y, false );
    }
    else
    {
        m_Action = NAV_NONE;
    }

    if ( buttons == 0 )
    {
        m_HeldCameras.clear();
        m_Chord = false;
        m_ChordEnded = false;
    }
}

bool NavController::Cancel()
{
    bool armed = m_ZoomBoxArmed;
    m_ZoomBoxArmed = false;

    if ( m_Pending )
    {
        // Nothing has moved yet; just forget the press.
        m_Pending = false;
        m_PendingDrag = nullptr;
        m_PendingClick = nullptr;
        return true;
    }

    if ( m_Action == NAV_NONE )
    {
        return armed;
    }

    Abandon();

    for ( int i = 0; i < ( int )m_HeldCameras.size(); i++ )
    {
        m_HeldCameras[i]->restoreView();
    }
    return true;
}

void NavController::Abandon()
{
    Fl::remove_timeout( RepeatZoom, this );
    if ( m_Action == NAV_ZOOM_BOX )
    {
        EndZoomBox( false, m_PressX, m_PressY );
    }
    else if ( m_Action == NAV_BOX_SELECT )
    {
        m_Win->EndBoxSelect( false );
    }

    // Cancel puts the whole view back, the pivot's center of rotation included.
    EndPivot( false );
    ClearOutline();

    // Ignore the rest of the drag; releasing the buttons then ends it.
    m_Action = NAV_NONE;
    m_ActionBeforeBox = NAV_NONE;
    m_Pending = false;
    m_PendingDrag = nullptr;
    m_PendingClick = nullptr;
}

void NavController::DragZoom( int x, int y )
{
    VSPGraphic::Display * display = m_Win->getGraphicEngine()->getDisplay();
    VSPGraphic::Camera * camera = display->getCamera();
    if ( !camera )
    {
        return;
    }

    int dy = y - m_PrevY;
    if ( m_Settings.m_DragZoomReverse )
    {
        dy = -dy;
    }

    if ( m_Settings.m_DragZoomLaw == NAV_ZOOM_PER_EVENT && m_Settings.m_DragZoomAnchor == NAV_ANCHOR_CENTER )
    {
        // OpenVSP's drag zoom as it was.
        display->zoom( m_PrevX, m_PrevY, x, m_PrevY + dy );
        m_Win->UpdateZoomParms();
        return;
    }

    // Moving up zooms out, as above.
    double factor = 1.0;
    if ( m_Settings.m_DragZoomLaw == NAV_ZOOM_CONTINUOUS )
    {
        factor = std::exp( 0.02 * m_Settings.m_ZoomSpeed * dy / m_Win->pixels_per_unit() );
    }
    else if ( dy > 0 )
    {
        factor = 1.1;
    }
    else if ( dy < 0 )
    {
        factor = 0.9;
    }

    ZoomAt( camera, factor, m_Settings.m_DragZoomAnchor, m_PressX, m_PressY );
    m_Win->UpdateZoomParms();
    m_Win->UpdatePanParms();
}

void NavController::StartPivot( VSPGraphic::Camera * camera, int x, int y )
{
    m_RoundedArcball.setCenter( false, 0.0f, 0.0f );

    VSPGraphic::Display * display = m_Win->getGraphicEngine()->getDisplay();
    VSPGraphic::Viewport * viewport = display->getViewport();
    if ( m_Settings.m_Pivot != NAV_PIVOT_UNDER_CURSOR || !viewport )
    {
        return;
    }

    // The mouse can be over another viewport when a gesture starts on a button release.
    int ivp = display->getLayoutMgr()->findViewport( x, y );
    if ( ivp < 0 || display->getLayoutMgr()->getViewports()[ ivp ] != viewport )
    {
        return;
    }

    glm::vec3 hit;
    if ( !m_Win->ProbeSurface( m_PressEventX, m_PressEventY, hit ) )
    {
        return;
    }

    m_PivotCamera = camera;
    m_StickyCOR = camera->getCOR();
    camera->setCORKeepView( -hit.x, -hit.y, -hit.z );

    // The ball turns about the pivot, so center it there.
    glm::vec4 rect( camera->viewportX(), camera->viewportY(), camera->viewportWidth(), camera->viewportHeight() );
    glm::vec3 s = glm::project( hit, camera->getModelViewMatrix(), camera->getProjectionMatrix(), rect );
    m_RoundedArcball.setCenter( true, s.x, s.y );
    viewport->setOverlayMarker( true, s.x, s.y );

    m_Win->UpdatePanParms();
}

void NavController::FollowPivot( VSPGraphic::Camera * camera )
{
    if ( m_Action != NAV_ROTATE || !m_Rotation || camera != m_PivotCamera )
    {
        return;
    }

    VSPGraphic::Viewport * viewport = m_Win->getGraphicEngine()->getDisplay()->getViewport();
    if ( !viewport )
    {
        return;
    }

    // The pivot is the center of rotation, in setCOR() terms the negative of the world point.
    glm::vec3 pivot = -camera->getCOR();
    glm::vec4 rect( camera->viewportX(), camera->viewportY(), camera->viewportWidth(), camera->viewportHeight() );
    glm::vec3 s = glm::project( pivot, camera->getModelViewMatrix(), camera->getProjectionMatrix(), rect );

    m_RoundedArcball.setCenter( true, s.x, s.y );
    viewport->setOverlayMarker( true, s.x, s.y );
    if ( m_Settings.m_ShowBallOutline )
    {
        float cx, cy;
        std::vector < float > radii;
        m_Rotation->outline( camera, cx, cy, radii );
        viewport->setOverlayCircles( cx, cy, radii );
    }

    m_Rotation->begin( camera, m_PrevX, m_PrevY );
}

void NavController::EndPivot( bool restore )
{
    if ( !m_PivotCamera )
    {
        return;
    }

    if ( restore )
    {
        m_PivotCamera->setCORKeepView( m_StickyCOR.x, m_StickyCOR.y, m_StickyCOR.z );
    }
    m_PivotCamera = nullptr;
    m_RoundedArcball.setCenter( false, 0.0f, 0.0f );

    std::vector < VSPGraphic::Viewport * > viewports = m_Win->getGraphicEngine()->getDisplay()->getLayoutMgr()->getViewports();
    for ( int i = 0; i < ( int )viewports.size(); i++ )
    {
        viewports[i]->setOverlayMarker( false, 0.0f, 0.0f );
    }

    m_Win->UpdateCORParms();
    m_Win->UpdatePanParms();
}

bool NavController::GetStickyCOR( VSPGraphic::Camera * camera, glm::vec3 & cor ) const
{
    if ( !m_PivotCamera || camera != m_PivotCamera )
    {
        return false;
    }
    cor = m_StickyCOR;
    return true;
}

void NavController::ClearOutline()
{
    std::vector < VSPGraphic::Viewport * > viewports = m_Win->getGraphicEngine()->getDisplay()->getLayoutMgr()->getViewports();
    std::vector < float > none;
    for ( int i = 0; i < ( int )viewports.size(); i++ )
    {
        viewports[i]->setOverlayCircles( 0.0f, 0.0f, none );
    }
}

void NavController::EndZoomBox( bool apply, int x, int y )
{
    VSPGraphic::Display * display = m_Win->getGraphicEngine()->getDisplay();

    // A chord's press may have selected another viewport since the box began.
    std::vector < VSPGraphic::Viewport * > viewports = display->getLayoutMgr()->getViewports();
    for ( int i = 0; i < ( int )viewports.size(); i++ )
    {
        viewports[i]->setOverlayBox( false, 0, 0, 0, 0 );
    }

    VSPGraphic::Viewport * viewport = display->getViewport();
    if ( !viewport )
    {
        return;
    }

    // A box smaller than this, in screen points, is taken as a click and ignored.
    double minsize = 4.0 * m_Win->pixels_per_unit();
    double bw = std::abs( x - m_PressX );
    double bh = std::abs( y - m_PressY );
    if ( !apply || ( bw < minsize && bh < minsize ) )
    {
        return;
    }

    VSPGraphic::Camera * camera = viewport->getCamera();

    // The whole box fits in the view.
    double ratio = std::max( std::max( bw, 1.0 ) / camera->viewportWidth(), std::max( bh, 1.0 ) / camera->viewportHeight() );

    camera->centerAt( 0.5f * ( m_PressX + x ), 0.5f * ( m_PressY + y ) );
    ZoomAt( camera, ratio, NAV_ANCHOR_CENTER, 0, 0 );

    m_Win->UpdateZoomParms();
    m_Win->UpdatePanParms();
}

double NavController::LimitZoom( VSPGraphic::Camera * camera, double factor )
{
    // Stop at the limits rather than let a clamp put the zoom back after the pan has moved.
    double lo, hi;
    m_Win->GetZoomLimits( lo, hi );
    double zoom = camera->getRelativeZoomValue();
    if ( zoom * factor > hi )
    {
        factor = hi / zoom;
    }
    else if ( zoom * factor < lo )
    {
        factor = lo / zoom;
    }
    return factor;
}

void NavController::ZoomAt( VSPGraphic::Camera * camera, double factor, int anchor, int x, int y )
{
    factor = LimitZoom( camera, factor );

    if ( anchor == NAV_ANCHOR_CENTER )
    {
        // The pan is measured from the viewport center, so it stays as it is.  A center pixel
        // would be half a pixel off in a viewport of odd size.
        camera->relativeZoom( camera->getRelativeZoomValue() * factor );
    }
    else
    {
        camera->zoomToward( factor, x, y );
    }
}

void NavController::StepZoom( int action, int anchor, int x, int y )
{
    VSPGraphic::Camera * camera = m_Win->getGraphicEngine()->getDisplay()->getCamera();
    if ( !camera )
    {
        return;
    }

    double factor = NAV_STEP_ZOOM;
    if ( action == NAV_ZOOM_IN )
    {
        factor = 1.0 / NAV_STEP_ZOOM;
    }
    ZoomAt( camera, factor, anchor, x, y );
    FollowPivot( camera );

    m_Win->UpdateZoomParms();
    m_Win->UpdatePanParms();
    m_Win->ViewChanged();
    m_Win->redraw();
}

void NavController::RepeatZoom( void * data )
{
    NavController * nav = ( NavController * )data;
    if ( IsStepZoom( nav->m_Action ) )
    {
        nav->StepZoom( nav->m_Action, nav->m_Settings.m_WheelAnchor, nav->m_PressX, nav->m_PressY );
        Fl::repeat_timeout( NAV_STEP_INTERVAL, RepeatZoom, data );
    }
}

void NavController::Wheel( int dx, int dy, int x, int y )
{
    if ( m_Settings.m_WheelReverse )
    {
        dy = -dy;
    }

    float zoomChange = 1;
    float zoomSpeed = 1.15; // size increase when zooming in

    if ( m_Settings.m_WheelStep == NAV_WHEEL_PROPORTIONAL )
    {
        zoomChange = std::pow( ( double )zoomSpeed, m_Settings.m_ZoomSpeed * dy );
    }
    else if ( dy > 0 ) // wheel pulled toward user
    {
        zoomChange = zoomSpeed;
    }
    else if ( dy < 0 ) // wheel pushed away from user
    {
        zoomChange = 1 / zoomSpeed;
    }

    if ( zoomChange == 1 )
    {
        return;
    }

    // Zoom whichever viewport is under the mouse, selected or not.
    VSPGraphic::Display * display = m_Win->getGraphicEngine()->getDisplay();
    VSPGraphic::LayoutMgr * layout = display->getLayoutMgr();

    int ivp = layout->findViewport( x, y );
    if ( ivp < 0 )
    {
        return;
    }

    VSPGraphic::Viewport * viewport = layout->getViewports()[ ivp ];
    VSPGraphic::Camera * camera = viewport->getCamera();

    ZoomAt( camera, zoomChange, m_Settings.m_WheelAnchor, x, y );
    FollowPivot( camera );

    if ( viewport == display->getViewport() )
    {
        m_Win->UpdateZoomParms();
        m_Win->UpdatePanParms();
    }
}

void NavController::ApplyAxes( const glm::dvec3 & rotation, const glm::dvec3 & translation )
{
    VSPGraphic::Camera * camera = m_Win->getGraphicEngine()->getDisplay()->getCamera();
    if ( !camera )
    {
        return;
    }

    if ( m_RotateMode == NAV_ROTATE_TURNTABLE )
    {
        // Tip about the screen's horizontal axis, then turn about world up, the way that moves
        // the near side with the device, as the mouse turntable does.  There is no roll.
        glm::dquat r0( camera->getViewRotation() );
        glm::dvec3 up( 0.0, 0.0, 1.0 );
        double yaw = rotation.y;
        if ( ( r0 * up ).y < 0.0 )
        {
            yaw = -yaw;
        }
        glm::dquat r = glm::angleAxis( rotation.x, glm::dvec3( 1.0, 0.0, 0.0 ) ) * r0 * glm::angleAxis( yaw, up );
        camera->setViewRotation( glm::quat( r ) );
    }
    else
    {
        camera->rotateEye( rotation );
    }

    double height = camera->viewportHeight() * camera->getRelativeZoomValue();
    glm::vec2 pan = camera->getPanValues();
    camera->relativePan( pan.x + height * translation.x, pan.y + height * translation.y );

    if ( translation.z != 0.0 )
    {
        // Toward the viewer brings the model closer.
        ZoomAt( camera, std::exp( -translation.z ), NAV_ANCHOR_CENTER, 0, 0 );
    }

    m_Win->UpdateRotationParms();
    m_Win->UpdatePanParms();
    m_Win->UpdateZoomParms();
    m_Win->ViewChanged();
    m_Win->redraw();
}

bool NavController::LevelZUp()
{
    VSPGraphic::Camera * camera = m_Win->getGraphicEngine()->getDisplay()->getCamera();
    if ( !camera )
    {
        return false;
    }

    // World Z in eye axes.
    glm::dvec3 z = glm::dquat( camera->getViewRotation() ) * glm::dvec3( 0.0, 0.0, 1.0 );
    bool moved = false;

    // Looking along Z, tip the view a quarter turn about the screen's x axis, the way that
    // brings Z up the screen.
    if ( std::hypot( z.x, z.y ) < 1e-6 )
    {
        double tip = -0.5 * M_PI;
        if ( z.z < 0.0 )
        {
            tip = 0.5 * M_PI;
        }
        camera->rotateEye( glm::dvec3( tip, 0.0, 0.0 ) );
        z = glm::dquat( camera->getViewRotation() ) * glm::dvec3( 0.0, 0.0, 1.0 );
        moved = true;
    }

    // Turning about the screen's axis by Z's angle from screen up brings it upright.
    double angle = std::atan2( z.x, z.y );
    if ( std::abs( angle ) > 1e-9 )
    {
        camera->rotateEye( glm::dvec3( 0.0, 0.0, angle ) );
        moved = true;
    }
    if ( !moved )
    {
        return false;
    }

    m_Win->UpdateRotationParms();
    m_Win->UpdatePanParms();
    m_Win->redraw();
    return true;
}

void NavController::KeyDown( int key )
{
    m_HeldKeys.insert( key );
}

void NavController::KeyUp( int key )
{
    m_HeldKeys.erase( key );
}

void NavController::ClearKeys()
{
    m_HeldKeys.clear();
}

}
