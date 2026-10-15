//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// NavProfile.cpp: which mouse buttons, modifiers and keys do what in a 3D view.
//
//////////////////////////////////////////////////////////////////////

#include <FL/Enumerations.H>

#include "NavProfile.h"

namespace VSPGUI
{

static const int NAV_ALL_BUTTONS = FL_BUTTON1 | FL_BUTTON2 | FL_BUTTON3;
static const int NAV_ALL_MODIFIERS = FL_SHIFT | FL_CTRL | FL_ALT | FL_META;

NavBinding NavBind( int buttons, int modifiers, NavAction action )
{
    NavBinding b;
    b.m_Buttons = buttons;
    b.m_OtherButtons = 0;
    b.m_Modifiers = modifiers;
    b.m_OtherModifiers = 0;
    b.m_HeldKey = 0;
    b.m_PressedButton = 0;
    b.m_Trigger = NAV_ON_DRAG;
    b.m_Conditions = 0;
    b.m_Action = action;
    b.m_PickFirst = false;
    return b;
}

NavBinding NavBindAtLeast( int buttons, int modifiers, NavAction action )
{
    NavBinding b = NavBind( buttons, modifiers, action );
    b.m_OtherButtons = NAV_ALL_BUTTONS & ~buttons;
    b.m_OtherModifiers = NAV_ALL_MODIFIERS & ~modifiers;
    return b;
}

NavPress::NavPress()
{
    m_Buttons = 0;
    m_Modifiers = 0;
    m_PressedButton = 0;
    m_Conditions = 0;
}

NavProfile::NavProfile()
{
    m_ClickDistance = 0;
    m_ChordEnds = false;
}

// Fit Model's box selection takes the left button when it is the one pressed: with Shift, or
// always in box mode, while Fit Model is shown.  3.55 gave left + right to zoom first, so the
// right button may not be down.
static void AddBoxSelect( NavProfile & p )
{
    NavBinding box = NavBindAtLeast( FL_BUTTON1, FL_SHIFT, NAV_BOX_SELECT );
    box.m_OtherButtons = FL_BUTTON2;
    box.m_PressedButton = FL_BUTTON1;
    box.m_Conditions = NAV_COND_FIT_MODEL;
    p.m_Bindings.push_back( box );

    box = NavBindAtLeast( FL_BUTTON1, 0, NAV_BOX_SELECT );
    box.m_OtherButtons = FL_BUTTON2;
    box.m_PressedButton = FL_BUTTON1;
    box.m_Conditions = NAV_COND_FIT_MODEL | NAV_COND_SELECT_BOX;
    p.m_Bindings.push_back( box );
}

NavProfile NavProfile::Classic()
{
    NavProfile p;

    // 3.55 tested each binding's buttons and modifiers as a subset of those down, so every
    // binding here allows any others.  Order matters: the first match wins.
    AddBoxSelect( p );

    p.m_Bindings.push_back( NavBindAtLeast( FL_BUTTON1 | FL_BUTTON3, 0, NAV_ZOOM ) );

    NavBinding roll = NavBindAtLeast( FL_BUTTON1, 0, NAV_ROLL );
    roll.m_HeldKey = 'z';
    p.m_Bindings.push_back( roll );

    p.m_Bindings.push_back( NavBindAtLeast( FL_BUTTON1, FL_ALT, NAV_PAN ) );
    p.m_Bindings.push_back( NavBindAtLeast( FL_BUTTON1, FL_CTRL, NAV_ZOOM ) );
    p.m_Bindings.push_back( NavBindAtLeast( FL_BUTTON1, FL_META, NAV_ZOOM ) );

    NavBinding rotate = NavBindAtLeast( FL_BUTTON1, 0, NAV_ROTATE );
    rotate.m_PickFirst = true;
    p.m_Bindings.push_back( rotate );

    p.m_Bindings.push_back( NavBindAtLeast( FL_BUTTON2, 0, NAV_ZOOM ) );
    p.m_Bindings.push_back( NavBindAtLeast( FL_BUTTON3, 0, NAV_PAN ) );

    return p;
}

// True if the binding matches the press, whatever its trigger.
static bool Matches( const NavBinding & b, const NavPress & press )
{
    if ( ( press.m_Buttons & ~b.m_OtherButtons ) != b.m_Buttons )
    {
        return false;
    }

    if ( ( press.m_Modifiers & ~b.m_OtherModifiers ) != b.m_Modifiers )
    {
        return false;
    }

    if ( b.m_HeldKey != 0 && press.m_HeldKeys.count( b.m_HeldKey ) == 0 )
    {
        return false;
    }

    if ( b.m_PressedButton != 0 && press.m_PressedButton != b.m_PressedButton )
    {
        return false;
    }

    if ( ( press.m_Conditions & b.m_Conditions ) != b.m_Conditions )
    {
        return false;
    }

    return true;
}

const NavBinding * NavProfile::Match( const NavPress & press, int trigger ) const
{
    for ( int i = 0; i < ( int )m_Bindings.size(); i++ )
    {
        const NavBinding & b = m_Bindings[i];

        if ( b.m_Trigger == trigger && Matches( b, press ) )
        {
            return &b;
        }
    }
    return nullptr;
}

}
