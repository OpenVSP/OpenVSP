//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// NavController.cpp: turns mouse and key events in a GL window into view changes.
//
//////////////////////////////////////////////////////////////////////

#include <cmath>
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

    m_Profile = NavProfile::Classic();
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

    prefs.flush();
}

NavController::NavController( VspGlWindow * win )
{
    m_Win = win;
    m_RotateMode = m_Settings.m_StartupRotateMode;
    m_Rotation = nullptr;
    m_Action = NAV_NONE;
    m_PressX = 0;
    m_PressY = 0;
    m_PrevX = 0;
    m_PrevY = 0;
    m_RotateEnabled = true;
    m_ActionBeforeBox = NAV_NONE;

    SetSettings( m_Settings );
}

void NavController::SetSettings( const NavSettings & settings )
{
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
        Begin( buttons, pressed, x, y, false );
    }
    else
    {
        Begin( buttons, pressed, x, y, true );
    }
}

void NavController::Begin( int buttons, int pressed, int x, int y, bool allow_pick )
{
    NavAction before = m_Action;

    m_Action = NAV_NONE;
    m_PressX = x;
    m_PressY = y;
    m_PrevX = x;
    m_PrevY = y;

    NavPress press;
    press.m_Buttons = buttons;
    press.m_Modifiers = Fl::event_state() & NAV_MODIFIERS;
    press.m_HeldKeys = m_HeldKeys;
    press.m_PressedButton = pressed;
    press.m_Conditions = m_Win->GetNavConditions();

    const NavBinding * drag = m_Settings.m_Profile.Match( press, NAV_ON_DRAG );

    if ( drag && drag->m_PickFirst && allow_pick && m_Win->PickAtPress() )
    {
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

void NavController::Start( NavAction action, int x, int y )
{
    m_Action = action;

    if ( m_Action == NAV_ROTATE && m_RotateEnabled )
    {
        VSPGraphic::Camera * camera = m_Win->getGraphicEngine()->getDisplay()->getCamera();

        m_Rotation = CurrentRotation();
        if ( camera )
        {
            m_Rotation->begin( camera, x, y );
        }
    }
    else if ( m_Action == NAV_BOX_SELECT )
    {
        m_Win->BeginBoxSelect( x, y );
    }
}

void NavController::Drag( int x, int y )
{
    VSPGraphic::Display * display = m_Win->getGraphicEngine()->getDisplay();

    switch ( m_Action )
    {
    case NAV_ROTATE:
        if ( m_RotateEnabled && m_Rotation && display->getCamera() )
        {
            m_Rotation->drag( display->getCamera(), x, y );
            m_Win->UpdateRotationParms();
        }
        break;

    case NAV_ROLL:
        display->rotateZ( m_PrevX, m_PrevY, x, y );
        m_Win->UpdateRotationParms();
        break;

    case NAV_PAN:
        display->pan( m_PrevX, m_PrevY, x, y );
        m_Win->UpdatePanParms();
        break;

    case NAV_ZOOM:
        display->zoom( m_PrevX, m_PrevY, x, y );
        m_Win->UpdateZoomParms();
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

    if ( buttons != 0 && m_Action != NAV_NONE )
    {
        // Buttons are still down; carry on with whatever they now mean, from here.
        Begin( buttons, 0, x, y, false );
    }
    else
    {
        m_Action = NAV_NONE;
    }
}

void NavController::Wheel( int dx, int dy, int x, int y )
{
    float zoomChange = 1;
    float zoomSpeed = 1.15; // size increase when zooming in

    if ( dy > 0 ) // wheel pulled toward user
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

    VSPGraphic::Camera * camera = m_Win->getGraphicEngine()->getDisplay()->getCamera();
    if ( !camera )
    {
        return;
    }

    camera->zoomToward( zoomChange, x, y );
    m_Win->UpdateZoomParms();
    m_Win->UpdatePanParms();
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
