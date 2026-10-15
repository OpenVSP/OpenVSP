//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// NavController.cpp: turns mouse and key events in a GL window into view changes.
//
//////////////////////////////////////////////////////////////////////

#include <FL/Fl.H>

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
    m_Profile = NavProfile::Classic();
}

NavController::NavController( VspGlWindow * win )
{
    m_Win = win;
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
}

VSPGraphic::RotationMethod * NavController::CurrentRotation()
{
    return &m_ClassicBall;
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
