//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// NavController.h: turns mouse and key events in a GL window into view changes.
//
//////////////////////////////////////////////////////////////////////

#ifndef VSP_GUI_NAV_CONTROLLER_H
#define VSP_GUI_NAV_CONTROLLER_H

#include <set>
#include <string>
#include <vector>

#include "NavProfile.h"
#include "RotationMethod.h"

namespace VSPGraphic
{
class Camera;
}

namespace VSPGUI
{
class VspGlWindow;

// Which rotation a rotate gesture uses.  The quick switch on the main window changes it.
enum NavRotateMode
{
    NAV_ROTATE_BALL,
    NAV_ROTATE_TURNTABLE
};

// What the Ball does.
enum NavBallType
{
    NAV_BALL_ROUNDED_ARCBALL,
    NAV_BALL_CLASSIC
};

// The user's 3D interaction preferences.
struct NavSettings
{
    NavSettings();

    // Read from and write to the user's FLTK preferences, group "Interaction".
    void Load();
    void Save() const;

    int m_StartupRotateMode;    // NavRotateMode the main window starts in.
    int m_BallType;             // NavBallType.
    double m_BallSize;          // Ball diameter, fraction of the smaller viewport dimension.
    double m_BallBorder;        // Fillet width, fraction of the ball radius.
    double m_BallAngleScale;    // 2 turns 360 degrees per crossing of the ball, 1 turns 180.
    double m_TurntableGain;     // Degrees per screen point.

    NavProfile m_Profile;       // The mouse bindings.
};

// Follows one gesture from press to release.  The gesture is chosen at press and kept until the
// buttons change, so modifiers pressed or released during a drag do not change what it does.
class NavController
{
public:

    NavController( VspGlWindow * win );

    void SetSettings( const NavSettings & settings );
    const NavSettings & GetSettings() const
    {
        return m_Settings;
    }

    void SetRotateMode( int mode )
    {
        m_RotateMode = mode;
    }
    int GetRotateMode() const
    {
        return m_RotateMode;
    }

    NavAction GetAction() const
    {
        return m_Action;
    }

    void SetRotateEnabled( bool flag )
    {
        m_RotateEnabled = flag;
    }

    void Push( int x, int y );
    void Drag( int x, int y );
    void Release( int x, int y );
    void Wheel( int dx, int dy, int x, int y );

    void KeyDown( int key );
    void KeyUp( int key );
    void ClearKeys();

private:

    // Choose what a press, or the buttons left after a release (pressed 0), will do.
    void Begin( int buttons, int pressed, int x, int y, bool allow_pick );

    // Start a drag action at a mouse location.
    void Start( NavAction action, int x, int y );

    // A zoom factor reduced to keep the camera within the window's zoom limits.
    double LimitZoom( VSPGraphic::Camera * camera, double factor );

    VSPGraphic::RotationMethod * CurrentRotation();

    VspGlWindow * m_Win;
    NavSettings m_Settings;
    int m_RotateMode;

    VSPGraphic::ClassicBall m_ClassicBall;
    VSPGraphic::RoundedArcball m_RoundedArcball;
    VSPGraphic::Turntable m_Turntable;
    VSPGraphic::RotationMethod * m_Rotation;

    std::set < int > m_HeldKeys;

    // What was going on when a box selection started, so its end resumes only that.
    NavAction m_ActionBeforeBox;

    NavAction m_Action;

    int m_PressX;
    int m_PressY;
    int m_PrevX;
    int m_PrevY;

    bool m_RotateEnabled;
};
}

#endif
