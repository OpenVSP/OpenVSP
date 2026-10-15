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

static const int NAV_BUTTONS = FL_BUTTON1 | FL_BUTTON2 | FL_BUTTON3;
static const int NAV_MODIFIERS = FL_SHIFT | FL_CTRL | FL_ALT | FL_META;

NavSettings::NavSettings()
{
    m_StartupRotateMode = NAV_ROTATE_BALL;
    m_BallType = NAV_BALL_ROUNDED_ARCBALL;
    m_BallSize = 0.9;
    m_BallBorder = 0.5;
    m_BallAngleScale = 2.0;
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

    m_Profile = NavProfile::Preset( NAV_PRESET_OPENVSP );
}

void NavSettings::ApplyPreset( int preset )
{
    NavSettings def;

    m_Profile = NavProfile::Preset( preset );
    m_StartupRotateMode = def.m_StartupRotateMode;
    m_BallType = def.m_BallType;
    m_Pivot = def.m_Pivot;

    m_WheelReverse = def.m_WheelReverse;
    m_WheelAnchor = def.m_WheelAnchor;
    m_WheelStep = def.m_WheelStep;
    m_DragZoomReverse = def.m_DragZoomReverse;
    m_DragZoomAnchor = def.m_DragZoomAnchor;
    m_DragZoomLaw = def.m_DragZoomLaw;

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

    // Maya tumbles and Rhino rotates about the world's vertical axis.
    if ( preset == NAV_PRESET_MAYA || preset == NAV_PRESET_RHINO )
    {
        m_StartupRotateMode = NAV_ROTATE_TURNTABLE;
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
             p.m_DragZoomLaw == m_DragZoomLaw )
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

    grp.get( "BallType", buf, "RoundedArcball", sizeof( buf ) );
    m_BallType = NAV_BALL_ROUNDED_ARCBALL;
    if ( strcmp( buf, "Classic" ) == 0 )
    {
        m_BallType = NAV_BALL_CLASSIC;
    }

    NavSettings def;
    grp.get( "BallSize", m_BallSize, def.m_BallSize );
    grp.get( "BallBorder", m_BallBorder, def.m_BallBorder );
    grp.get( "BallAngleScale", m_BallAngleScale, def.m_BallAngleScale );
    grp.get( "TurntableGain", m_TurntableGain, def.m_TurntableGain );

    m_BallSize = Checked( m_BallSize, 0.2, 1.5, def.m_BallSize );
    m_BallBorder = Checked( m_BallBorder, 0.0, 1.0, def.m_BallBorder );
    m_BallAngleScale = Checked( m_BallAngleScale, 0.25, 4.0, def.m_BallAngleScale );
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

    grp.get( "ClickDistance", m_Profile.m_ClickDistance, def.m_Profile.m_ClickDistance );
    if ( m_Profile.m_ClickDistance < 0 || m_Profile.m_ClickDistance > 50 )
    {
        m_Profile.m_ClickDistance = def.m_Profile.m_ClickDistance;
    }
    m_Profile.m_ChordEnds = GetFlag( grp, "ChordEnds", def.m_Profile.m_ChordEnds );

    // Bindings that do not read give the default profile whole.
    m_Profile.m_Bindings = def.m_Profile.m_Bindings;
    char * text = nullptr;
    grp.get( "Bindings", text, "" );
    if ( text && grp.entryExists( "Bindings" ) && !m_Profile.SetBindingsText( text ) )
    {
        m_Profile = def.m_Profile;
    }
    free( text );
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

    const char * ball_names[] = { "RoundedArcball", "Classic" };
    SaveName( grp, "BallType", ball_names[ m_BallType ], ball_names[ def.m_BallType ] );

    SaveDouble( grp, "BallSize", m_BallSize, def.m_BallSize );
    SaveDouble( grp, "BallBorder", m_BallBorder, def.m_BallBorder );
    SaveDouble( grp, "BallAngleScale", m_BallAngleScale, def.m_BallAngleScale );
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
    SaveName( grp, "Bindings", m_Profile.BindingsText().c_str(), def.m_Profile.BindingsText().c_str() );
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

    SetSettings( m_Settings );
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
    m_RoundedArcball.setAngleScale( m_Settings.m_BallAngleScale );
}

VSPGraphic::RotationMethod * NavController::CurrentRotation()
{
    if ( m_RotateMode == NAV_ROTATE_TURNTABLE )
    {
        // The gain is per screen point; mouse locations are in pixels.
        double gain = m_Settings.m_TurntableGain * M_PI / 180.0 / m_Win->pixels_per_unit();
        m_Turntable.setParams( gain, glm::dvec3( 0.0, 0.0, 1.0 ) );
        return &m_Turntable;
    }

    if ( m_Settings.m_BallType == NAV_BALL_CLASSIC )
    {
        return &m_ClassicBall;
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

    NavPress press;
    press.m_Buttons = buttons;
    press.m_Modifiers = Fl::event_state() & NAV_MODIFIERS;
    press.m_HeldKeys = m_HeldKeys;
    press.m_PressedButton = pressed;
    press.m_Conditions = m_Win->GetNavConditions();

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

    if ( m_Settings.m_Profile.m_ClickDistance > 0 && ( m_PendingClick || CountButtons( press.m_Buttons ) == 1 ) )
    {
        // Wait to see whether this is a click, or the first button of a chord.
        m_Pending = true;
        m_PendingDrag = drag;
        return;
    }

    if ( drag )
    {
        if ( drag->m_Action == NAV_BOX_SELECT )
        {
            m_ActionBeforeBox = before;
        }
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
