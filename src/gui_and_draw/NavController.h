//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// NavController.h: turns mouse and key events in a GL window into view changes.
//
//////////////////////////////////////////////////////////////////////

#ifndef VSP_GUI_NAV_CONTROLLER_H
#define VSP_GUI_NAV_CONTROLLER_H

#include <chrono>
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
    NAV_BALL_CLASSIC,
    NAV_BALL_TUMBLER,
    NAV_BALL_TRACKBALL
};

// Where a zoom is centered: the world point there stays put on screen.
enum NavZoomAnchor
{
    NAV_ANCHOR_POINTER,     // The wheel's cursor, or a drag's press point.
    NAV_ANCHOR_CENTER       // The viewport center.
};

// How the wheel's zoom step is sized.
enum NavWheelStep
{
    NAV_WHEEL_PER_EVENT,    // A fixed step per event, whatever its size.
    NAV_WHEEL_PROPORTIONAL  // A step per unit of scroll, for trackpads.
};

// How a drag zooms.
enum NavZoomLaw
{
    NAV_ZOOM_PER_EVENT,     // 10% per mouse event, by the sign of the motion only.
    NAV_ZOOM_CONTINUOUS     // A factor that depends only on the distance moved.
};

// What a rotation turns about.
enum NavPivot
{
    NAV_PIVOT_FIXED,        // The rotation center, set by Set Rotation Center, Center, Fit, ...
    NAV_PIVOT_UNDER_CURSOR  // The surface under the press, for that drag; else the rotation center.
};

// What one of a six-axis device's axes does to the view.
enum NavAxisAction
{
    NAV_AXIS_NONE,
    NAV_AXIS_PAN_X,     // Move the model right.
    NAV_AXIS_PAN_Y,     // Move the model up.
    NAV_AXIS_ZOOM,      // Bring the model closer.
    NAV_AXIS_TURN_X,    // Turn the model about the screen's horizontal axis, its top toward you.
    NAV_AXIS_TURN_Y,    // Turn the model about the screen's vertical axis, its right side away.
    NAV_AXIS_TURN_Z,    // Turn the model counterclockwise about the screen's center.
    NAV_NUM_AXIS_ACTIONS
};

// A six-axis device's axes, in the order it reports them.
static const int NAV_NUM_AXES = 6;

// The SpaceMouse speeds offered, slowest first; Faster and Slower step through them.
static const int NAV_NUM_SPACEMOUSE_SPEEDS = 5;
extern const double NAV_SPACEMOUSE_SPEEDS[ NAV_NUM_SPACEMOUSE_SPEEDS ];

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
    double m_BallSpeed;         // Multiplier on each ball type's turn, 0.1 to 2; not the Tumbler's.
    double m_TurntableGain;     // Degrees per screen point.

    bool m_WheelReverse;        // Wheel forward zooms out instead of in.
    int m_WheelAnchor;          // NavZoomAnchor.
    int m_WheelStep;            // NavWheelStep.
    bool m_DragZoomReverse;     // Drag down zooms out instead of in.
    int m_DragZoomAnchor;       // NavZoomAnchor.
    int m_DragZoomLaw;          // NavZoomLaw.
    double m_ZoomSpeed;         // Scales the continuous drag zoom and the proportional wheel.

    bool m_ShowBallOutline;     // Draw the ball's outline while it rotates.

    int m_Pivot;                // NavPivot.

    bool m_EmulateMiddle;       // Alt + left button acts as the middle button.

    bool m_UseDevice;           // Look for a 3D input device and use it when there is one.

    bool m_UseSpaceMouse;       // Look for a SpaceMouse and use it when there is one.
    double m_SpaceMouseSpeed;   // Multiplier on the SpaceMouse's speed, 0.1 to 10.
    int m_SpaceMouseAxis[ NAV_NUM_AXES ];       // NavAxisAction of each device axis.
    bool m_SpaceMouseReverse[ NAV_NUM_AXES ];   // The axis acts the other way.
    bool m_SpaceMouseRotate;    // The SpaceMouse's turns act.
    bool m_SpaceMouseMove;      // The SpaceMouse's pans and zoom act.
    bool m_SpaceMouseDominant;  // Only the SpaceMouse axis pushed furthest acts.

    // The next SpaceMouse speed offered, faster (step 1) or slower (step -1).
    void StepSpaceMouseSpeed( int step );

    NavProfile m_Profile;       // The mouse bindings.

    // Set what a preset sets: the mouse bindings, how rotation starts and what it turns about,
    // and how the wheel and drags zoom.  Every other setting but the 3D devices' and the device
    // button bindings goes back to its default.
    void ApplyPreset( int preset );

    // The NavPreset whose settings these match, or -1 for none.
    int FindPreset() const;

    // The press as the bindings take it: Alt + left is the middle button when that is on and
    // no binding acts on the press as it is.
    NavPress Effective( const NavPress & press ) const;
};

// Follows one gesture from press to release.  The gesture is chosen at press and kept until the
// buttons change, so modifiers pressed or released during a drag do not change what it does.
class NavController
{
public:

    NavController( VspGlWindow * win );
    ~NavController();

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

    // Make the next left button drag a zoom box, whatever it is bound to.
    void ArmZoomBox()
    {
        m_ZoomBoxArmed = true;
    }

    // Called for a binding's command, a NavAction for which NavIsCommand is true.
    void SetCommandCallback( void ( *cb )( int action, void * data ), void * data );

    // A six-axis device's button went down or up.  Its binding acts on the selected viewport.
    void DeviceButton( const std::string & name, bool press );

    // Forget the device's buttons held down, as when it is closed.
    void ReleaseDeviceButtons();

    // Turn the view by angle, in radians, counterclockwise about the screen's center.
    void RollView( double angle );

    // Ignore the wheel for this long from now.  macOS sends wheel events for a SpaceMouse's
    // twist; they must not zoom the view as well.
    void HoldWheel( double seconds );

    void Push( int x, int y );
    void Drag( int x, int y );
    void Release( int x, int y );
    void Wheel( int dx, int dy, int x, int y );

    // Move the view by a six-axis device's motion over a frame, in eye axes (+x right, +y up,
    // +z toward the viewer): turn the model by rotation, in radians, and move it by
    // translation, in view heights across and natural-log zoom steps along z.
    void ApplyAxes( const glm::dvec3 & rotation, const glm::dvec3 & translation );

    // Turn the view about the screen's axis until world Z points up the screen.  Looking along
    // Z, first tip it a quarter turn about the screen's x axis.  False if Z is already up.
    bool LevelZUp();

    // Esc: end the gesture in progress and put the view back as it was at the press, or
    // disarm a zoom box.  Returns true if there was something to cancel.
    bool Cancel();

    // While a drag turns about a pivot of its own, the camera's center of rotation is that
    // pivot.  Returns true and the rotation center to show instead, in setCOR() terms, when
    // that is so for this camera.
    bool GetStickyCOR( VSPGraphic::Camera * camera, glm::vec3 & cor ) const;

    void KeyDown( int key );
    void KeyUp( int key );
    void ClearKeys();

private:

    // Choose what a press, or the buttons left after a release (pressed 0), will do.
    void Begin( int buttons, int pressed, int x, int y, bool allow_pick );

    // Start a drag action at a mouse location.
    void Start( NavAction action, int x, int y );

    // Do what a click binding does.
    void Click( const NavBinding * b, int x, int y );

    void DragZoom( int x, int y );

    // One wheel step of NAV_ZOOM_IN or NAV_ZOOM_OUT in the selected viewport, at x y or its
    // center, by NavZoomAnchor.
    void StepZoom( int action, int anchor, int x, int y );

    // While a Zoom in or Zoom out press is held, step again.
    static void RepeatZoom( void * data );

    // While a device button's Zoom in or Zoom out is held, step again.
    static void RepeatDeviceZoom( void * data );

    // The modifier keys down now, as FL_SHIFT, FL_CTRL, FL_ALT and FL_META bits.
    static int HeldModifiers();

    // Turn about the surface under the mouse for this drag, if there is one.
    void StartPivot( VSPGraphic::Camera * camera, int x, int y );

    // After the pan changed during a drag about a pivot, put the ball, its outline and the
    // marker back on the pivot and continue the drag from there.
    void FollowPivot( VSPGraphic::Camera * camera );

    // Go back to the rotation center without moving the image, unless restore is false
    // because the whole view is being put back anyway.
    void EndPivot( bool restore );

    // Remove the rotation method's outline from every viewport.
    void ClearOutline();

    // Hold the view of the selected viewport's camera for Cancel, once per gesture.
    void HoldCurrentCamera();

    // End whatever the gesture has on screen or in progress without applying it.
    void Abandon();

    // Remove the zoom box, and when apply is set, zoom to it.
    void EndZoomBox( bool apply, int x, int y );

    // A zoom factor reduced to keep the camera within the window's zoom limits.
    double LimitZoom( VSPGraphic::Camera * camera, double factor );

    // Zoom a camera holding still the world point under a mouse location, or at the exact
    // viewport center when anchor is NAV_ANCHOR_CENTER.
    void ZoomAt( VSPGraphic::Camera * camera, double factor, int anchor, int x, int y );

    VSPGraphic::RotationMethod * CurrentRotation();

    VspGlWindow * m_Win;
    NavSettings m_Settings;
    int m_RotateMode;

    VSPGraphic::ClassicBall m_ClassicBall;
    VSPGraphic::Trackball m_Trackball;
    VSPGraphic::RoundedArcball m_RoundedArcball;
    VSPGraphic::Tumbler m_Tumbler;
    VSPGraphic::Turntable m_Turntable;
    VSPGraphic::RotationMethod * m_Rotation;

    std::set < int > m_HeldKeys;

    // The cameras whose views were held during the gesture, for Cancel: the press's, and
    // another viewport's if a chord's press selected it.
    std::vector < VSPGraphic::Camera * > m_HeldCameras;

    // What was going on when a box selection started, so its end resumes only that.
    NavAction m_ActionBeforeBox;

    int m_PressModifiers;   // Modifiers at the press, for clicks.

    // The camera turning about a pivot of its own, and its rotation center to go back to.
    VSPGraphic::Camera * m_PivotCamera;
    glm::vec3 m_StickyCOR;

    NavAction m_Action;

    // A press waiting to become a click or a drag, and what each would do.
    bool m_Pending;
    const NavBinding * m_PendingDrag;
    const NavBinding * m_PendingClick;
    bool m_Moved;           // The gesture has had a drag event.
    bool m_Chord;           // More than one button has been down in this gesture.
    bool m_ChordEnded;      // A chord's button was released and the profile ends chords so.

    int m_PressEventX;      // The press in FLTK coordinates, for probing the surface there.
    int m_PressEventY;
    int m_PressX;
    int m_PressY;
    int m_PrevX;
    int m_PrevY;

    bool m_RotateEnabled;
    bool m_ZoomBoxArmed;

    void ( *m_CommandCB )( int, void * );
    void * m_CommandData;

    // The device button whose Zoom in or Zoom out repeats while it is held, and which.
    std::string m_DeviceZoomButton;
    int m_DeviceZoom;

    // Until when the wheel is ignored.
    std::chrono::steady_clock::time_point m_WheelHeldUntil;
};
}

#endif
