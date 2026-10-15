//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// NavProfile.h: which mouse buttons, modifiers and keys do what in a 3D view.
//
//////////////////////////////////////////////////////////////////////

#ifndef VSP_GUI_NAV_PROFILE_H
#define VSP_GUI_NAV_PROFILE_H

#include <set>
#include <string>
#include <vector>

namespace VSPGUI
{

// What a mouse gesture does to the view.
enum NavAction
{
    NAV_NONE,
    NAV_ROTATE,
    NAV_ROLL,
    NAV_PAN,
    NAV_ZOOM,
    NAV_ZOOM_BOX,       // Drag a rectangle; on release it fills the view.
    NAV_BOX_SELECT,     // Drag a rectangle; on release the Fit Model points in it are selected.
    NAV_SET_PIVOT,      // Click: the surface point there becomes the rotation center, centered.
};

// When a binding acts.
enum NavTrigger
{
    NAV_ON_DRAG,        // At the press, or once the mouse moves past the profile's click distance.
    NAV_ON_CLICK        // At a release before the mouse has moved that far.
};

// Conditions a binding can require, as bits.
enum NavCondition
{
    NAV_COND_FIT_MODEL = 1,     // The Fit Model screen is shown.
    NAV_COND_SELECT_BOX = 2     // Its box selection mode is on.
};

// One row of a profile: the press that starts a gesture, and what it does.
struct NavBinding
{
    int m_Buttons;          // FL_BUTTON1, FL_BUTTON2 and FL_BUTTON3 bits that must all be down.
    int m_OtherButtons;     // Further button bits that may also be down.  Any others may not.
    int m_Modifiers;        // FL_SHIFT, FL_CTRL, FL_ALT and FL_META bits that must all be down.
    int m_OtherModifiers;   // Further modifier bits that may also be down.  Any others may not.
    int m_HeldKey;          // Lower case key that must be held, or 0.
    int m_PressedButton;    // Button bit whose press must start it, or 0 for any press or release.
    int m_Trigger;          // NavTrigger.
    int m_Conditions;       // NavCondition bits that must all hold.
    NavAction m_Action;
    bool m_PickFirst;       // Offer the press to the scene's picking before starting the gesture.
};

// A binding for exactly these buttons and modifiers, acting on drag, anywhere.
NavBinding NavBind( int buttons, int modifiers, NavAction action );

// A binding for at least these buttons and modifiers: any others may also be down.
NavBinding NavBindAtLeast( int buttons, int modifiers, NavAction action );

// A press, or the buttons left down after a release, as bindings see it.
struct NavPress
{
    NavPress();

    int m_Buttons;              // Button bits down.
    int m_Modifiers;            // Modifier bits down.
    std::set < int > m_HeldKeys;
    int m_PressedButton;        // Button bit just pressed, or 0 after a release.
    int m_Conditions;           // NavCondition bits that hold.
};

// An ordered list of bindings and how presses become drags and clicks.
class NavProfile
{
public:

    NavProfile();

    // OpenVSP's bindings as of 3.55.
    static NavProfile Classic();

    // The first binding with this trigger that matches the press, or null.
    const NavBinding * Match( const NavPress & press, int trigger ) const;

    std::vector < NavBinding > m_Bindings;

    // Screen points a press must move before it is a drag.  At 0 a drag starts at the press,
    // and a click is a release with no motion at all.
    int m_ClickDistance;

    // Releasing one button of a chord ends the gesture.  Otherwise the buttons still down start
    // whatever they are bound to, from there.
    bool m_ChordEnds;
};
}

#endif
