//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// PreferencesScreen.cpp: implementation of the PreferencesScreen class.
//
//////////////////////////////////////////////////////////////////////

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>

#include <FL/Fl_Box.H>
#include <FL/fl_ask.H>
#include <FL/fl_draw.H>
#include <FL/filename.H>

#include "PreferencesScreen.h"
#include "MainVSPScreen.h"
#include "ScreenMgr.h"
#include "NavController.h"
#include "NavDevice.h"
#include "SpaceMouse.h"

// Values offered for the numeric 3D interaction preferences.
static const double BALL_SIZES[] = { 0.6, 0.75, 0.9, 1.0 };
static const double BALL_SPEED_MIN = 0.1;
static const double BALL_SPEED_MAX = 2.0;
static const double TURNTABLE_GAINS[] = { 0.2, 0.3, 0.4, 0.6 };
static const double ZOOM_SPEEDS[] = { 0.5, 1.0, 2.0 };
static const int CLICK_DISTANCES[] = { 0, 2, 4, 8 };

// The buttons a binding can need: one, and another held with it.
static const int BUTTONS[] = { FL_BUTTON1, FL_BUTTON2, FL_BUTTON3, FL_BUTTON4, FL_BUTTON5 };
static const char * const BUTTON_NAMES[] = { "Left", "Middle", "Right", "Back", "Forward" };
static const int NUM_BUTTONS = 5;

static const int MODIFIER_BITS[] = { FL_SHIFT, FL_CTRL, FL_ALT, FL_META };

// What a modifier must be for a binding to act.
enum
{
    MOD_UP,         // Must not be down.
    MOD_DOWN,       // Must be down.
    MOD_EITHER      // Either.
};

static const int ALL_BUTTONS = FL_BUTTON1 | FL_BUTTON2 | FL_BUTTON3 | FL_BUTTON4 | FL_BUTTON5;
static const int ALL_MODIFIERS = FL_SHIFT | FL_CTRL | FL_ALT | FL_META;

static const char * ActionName( int action )
{
    switch ( action )
    {
    case VSPGUI::NAV_ROTATE:
        return "Rotate";
    case VSPGUI::NAV_ROLL:
        return "Roll";
    case VSPGUI::NAV_PAN:
        return "Pan";
    case VSPGUI::NAV_ZOOM:
        return "Zoom";
    case VSPGUI::NAV_ZOOM_BOX:
        return "Zoom box";
    case VSPGUI::NAV_SET_PIVOT:
        return "Rotation center";
    case VSPGUI::NAV_SELECT:
        return "Select Geom";
    case VSPGUI::NAV_ZOOM_IN:
        return "Zoom in";
    case VSPGUI::NAV_ZOOM_OUT:
        return "Zoom out";
    default:
        return "Tool picks only";
    }
}

// What each of a six-axis device's axes may do, as m_AxisChoice offers them.
static const char * const AXIS_ACTION_NAMES[] = { "Nothing", "Pan right", "Pan up", "Zoom in", "Turn top toward you",
                                                  "Turn right side away", "Roll counterclockwise" };

// The device's axes, in its order, by the motion of the cap that drives each.
static const char * const AXIS_LABELS[] = { "Left / Right", "Forward / Back", "Up / Down", "Tilt Fwd / Back",
                                            "Tilt Left / Right", "Twist" };

// One axis of a six-axis device: a bar from the middle, as far as the cap is pushed, over a
// shaded dead zone.
class NavAxisMeter : public Fl_Widget
{
public:
    NavAxisMeter( int x, int y, int w, int h ) : Fl_Widget( x, y, w, h )
    {
        m_Counts = 0;
    }

    void SetCounts( int counts )
    {
        if ( counts != m_Counts )
        {
            m_Counts = counts;
            redraw();
        }
    }

    void draw()
    {
        fl_draw_box( FL_DOWN_BOX, x(), y(), w(), h(), FL_WHITE );
        int inner_x = x() + 2;
        int inner_w = w() - 4;
        int mid = inner_x + inner_w / 2;
        double full = VSPGUI::SpaceMouse::FullCounts();

        // At least a little, so the dead zone shows.
        int dead = std::max( 2, ( int )( 0.5 * inner_w * VSPGUI::SpaceMouse::DeadCounts() / full ) );
        fl_color( fl_rgb_color( 220, 220, 232 ) );
        fl_rectf( mid - dead, y() + 2, 2 * dead + 1, h() - 4 );

        double f = m_Counts / full;
        if ( f > 1.0 )
        {
            f = 1.0;
        }
        if ( f < -1.0 )
        {
            f = -1.0;
        }
        int len = ( int )( 0.5 * inner_w * f );
        fl_color( fl_rgb_color( 76, 76, 255 ) );
        if ( len > 0 )
        {
            fl_rectf( mid, y() + 4, len, h() - 8 );
        }
        else if ( len < 0 )
        {
            fl_rectf( mid + len, y() + 4, -len, h() - 8 );
        }

        fl_color( fl_rgb_color( 64, 64, 64 ) );
        fl_line( mid, y() + 2, mid, y() + h() - 3 );
    }

    int m_Counts;
};

// Index of the offered value nearest a stored one.
static int NearestIndex( const double * vals, int n, double v )
{
    int best = 0;
    for ( int i = 1; i < n; i++ )
    {
        if ( std::abs( vals[i] - v ) < std::abs( vals[best] - v ) )
        {
            best = i;
        }
    }
    return best;
}

// A box that remembers the buttons, modifiers and letter or digit keys down at a press in it.
class NavPressBox : public Fl_Box
{
public:
    NavPressBox( int x, int y, int w, int h ) : Fl_Box( x, y, w, h )
    {
        box( FL_DOWN_BOX );
        color( FL_WHITE );
        labelsize( 12 );
        align( FL_ALIGN_CENTER | FL_ALIGN_INSIDE | FL_ALIGN_WRAP );
        m_HasPress = false;
    }

    int handle( int event )
    {
        if ( event == FL_PUSH )
        {
            m_Press = VSPGUI::NavPress();
            m_Press.m_Buttons = Fl::event_state() & ALL_BUTTONS;
            m_Press.m_Modifiers = Fl::event_state() & ALL_MODIFIERS;

            int b = Fl::event_button();
            if ( b == FL_LEFT_MOUSE )
            {
                m_Press.m_PressedButton = FL_BUTTON1;
            }
            else if ( b == FL_MIDDLE_MOUSE )
            {
                m_Press.m_PressedButton = FL_BUTTON2;
            }
            else if ( b == FL_RIGHT_MOUSE )
            {
                m_Press.m_PressedButton = FL_BUTTON3;
            }
            else if ( b == FL_BACK_MOUSE )
            {
                m_Press.m_PressedButton = FL_BUTTON4;
            }
            else if ( b == FL_FORWARD_MOUSE )
            {
                m_Press.m_PressedButton = FL_BUTTON5;
            }

            for ( int k = 'a'; k <= 'z'; k++ )
            {
                if ( Fl::event_key( k ) )
                {
                    m_Press.m_HeldKeys.insert( k );
                }
            }
            for ( int k = '0'; k <= '9'; k++ )
            {
                if ( Fl::event_key( k ) )
                {
                    m_Press.m_HeldKeys.insert( k );
                }
            }

            // Buttons beyond the third (back, forward) cannot be bound.
            if ( m_Press.m_Buttons == 0 )
            {
                return 1;
            }

            m_HasPress = true;
            do_callback();
            return 1;
        }
        if ( event == FL_RELEASE || event == FL_DRAG || event == FL_ENTER || event == FL_LEAVE )
        {
            return 1;
        }
        return Fl_Box::handle( event );
    }

    VSPGUI::NavPress m_Press;
    bool m_HasPress;
};

//==== Preferences File ====//

// A preferences file is a copy of FLTK's own, which is text the same on every platform and holds
// every preference, including ones added later.

// The first line of FLTK's preferences file.
static const char * PREFS_HEADER = "; FLTK preferences file format 1.0";

// The path of the preferences file, written out first so it holds every change.
static string PrefsFileName()
{
    Fl_Preferences prefs( Fl_Preferences::USER, "openvsp.org", "VSP" );
    prefs.flush();

    char buf[ FL_PATH_MAX ];
    buf[0] = '\0';
    prefs.filename( buf, sizeof( buf ) );
    return string( buf );
}

// Copy the preferences file to fname.  False if it was not written.
static bool SavePrefsFile( const string & fname )
{
    std::error_code ec;
    std::filesystem::copy_file( PrefsFileName(), fname, std::filesystem::copy_options::overwrite_existing, ec );
    return !ec;
}

// Copy fname over the preferences file if it is one.  Returns why it was not, or an empty string.
static string LoadPrefsFile( const string & fname )
{
    std::ifstream in( fname );
    string line;
    if ( !in || !std::getline( in, line ) )
    {
        return "could not be read";
    }
    in.close();

    if ( !line.empty() && line.back() == '\r' )
    {
        line.pop_back();
    }
    if ( line != PREFS_HEADER )
    {
        return "is not a preferences file";
    }

    std::filesystem::path dest = PrefsFileName();
    std::error_code ec;
    std::filesystem::create_directories( dest.parent_path(), ec );
    std::filesystem::copy_file( fname, dest, std::filesystem::copy_options::overwrite_existing, ec );
    if ( ec )
    {
        return "could not be copied to the preferences";
    }
    return string();
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

PreferencesScreen::PreferencesScreen( ScreenMgr* mgr ) : TabScreen( mgr, 460, 800, "Preferences", "MouseKeyboard.html", 30 )
{
    m_SelBinding = -1;
    m_FCTypeAtShow = vsp::FC_OPENVSP;
    m_ToolbarAtShow = TOOLBAR_MEDIUM;
    m_WindowModeAtShow = vsp::WINDOW_MODE_FREE;
    m_BrowserPlaceAtShow = BROWSER_FLOATING;
    m_ModeChanging = false;

    //==== General ====//
    Fl_Group * gen_tab = AddTab( "General" );
    Fl_Group * gen_group = AddSubGroup( gen_tab, 5 );
    m_GenLayout.SetGroupAndScreen( gen_group, this );

    m_GenLayout.AddDividerBox( "File Chooser GUI" );
    m_GenLayout.AddYGap();

    m_FCTypeChoice.AddItem( "OpenVSP Default", vsp::FC_OPENVSP );
    m_FCTypeChoice.AddItem( "Native", vsp::FC_NATIVE );
    m_FCTypeChoice.UpdateItems();

    m_GenLayout.AddChoice( m_FCTypeChoice, "File Chooser" );

    m_GenLayout.AddYGap();
    m_GenLayout.AddDividerBox( "Main Window" );

    m_ToolbarChoice.AddItem( "Hidden", TOOLBAR_HIDDEN );
    m_ToolbarChoice.AddItem( "Small", TOOLBAR_SMALL );
    m_ToolbarChoice.AddItem( "Medium", TOOLBAR_MEDIUM );
    m_ToolbarChoice.AddItem( "Large", TOOLBAR_LARGE );
    m_ToolbarChoice.UpdateItems();
    m_GenLayout.AddChoice( m_ToolbarChoice, "Toolbar" );
    m_ToolbarChoice.GetFlChoice()->tooltip( "The row of buttons under the menu bar.  Window > Toolbar also shows or hides it." );

    m_GenLayout.AddYGap();
    m_GenLayout.AddDividerBox( "Windows" );

    m_WindowModeChoice.AddItem( "Free Windows", vsp::WINDOW_MODE_FREE );
    m_WindowModeChoice.AddItem( "Captive Windows", vsp::WINDOW_MODE_CAPTIVE );
    m_WindowModeChoice.UpdateItems();
    m_GenLayout.AddChoice( m_WindowModeChoice, "Screens" );
    m_WindowModeChoice.GetFlChoice()->tooltip( "Each screen a window of its own, or inside the main window over the 3D view." );

    m_BrowserPlaceChoice.AddItem( "Floating", BROWSER_FLOATING );
    m_BrowserPlaceChoice.AddItem( "Docked", BROWSER_DOCKED );
    m_BrowserPlaceChoice.UpdateItems();
    m_GenLayout.AddChoice( m_BrowserPlaceChoice, "Geom Browser" );
    m_BrowserPlaceChoice.GetFlChoice()->tooltip( "The Geom Browser over the right of the 3D view, or beside it, the view narrower while the browser is shown." );

    //==== 3D View ====//
    Fl_Group * view_tab = AddTab( "3D View" );
    Fl_Group * view_group = AddSubGroup( view_tab, 5 );
    m_ViewLayout.SetGroupAndScreen( view_group, this );

    for ( int i = 0; i < VSPGUI::NAV_NUM_PRESETS; i++ )
    {
        m_ProfileChoice.AddItem( VSPGUI::NavProfile::PresetLabel( i ), i );
        m_ButtonsProfileChoice.AddItem( VSPGUI::NavProfile::PresetLabel( i ), i );
    }
    m_ProfileChoice.AddItem( "Custom", VSPGUI::NAV_NUM_PRESETS );
    m_ButtonsProfileChoice.AddItem( "Custom", VSPGUI::NAV_NUM_PRESETS );
    m_ProfileChoice.UpdateItems();
    m_ButtonsProfileChoice.UpdateItems();
    m_ViewLayout.AddChoice( m_ProfileChoice, "Scheme" );
    m_ViewLayout.AddYGap();

    m_ViewLayout.AddDividerBox( "Rotation" );

    m_RotateModeChoice.AddItem( "Ball", VSPGUI::NAV_ROTATE_BALL );
    m_RotateModeChoice.AddItem( "Turntable", VSPGUI::NAV_ROTATE_TURNTABLE );
    m_RotateModeChoice.UpdateItems();
    m_ViewLayout.AddChoice( m_RotateModeChoice, "Start With" );

    m_PivotChoice.AddItem( "Rotation center", VSPGUI::NAV_PIVOT_FIXED );
    m_PivotChoice.AddItem( "Point under cursor", VSPGUI::NAV_PIVOT_UNDER_CURSOR );
    m_PivotChoice.UpdateItems();
    m_ViewLayout.AddChoice( m_PivotChoice, "Rotate About" );
    m_PivotChoice.GetFlChoice()->tooltip( "Point under cursor: each drag turns about the surface under the press, or the rotation center if there is none.  The rotation center is kept." );
    m_ViewLayout.AddYGap();

    m_ViewLayout.AddDividerBox( "Ball" );

    m_BallTypeChoice.AddItem( "Rounded Arcball", VSPGUI::NAV_BALL_ROUNDED_ARCBALL );
    m_BallTypeChoice.AddItem( "OpenVSP Classic", VSPGUI::NAV_BALL_CLASSIC );
    m_BallTypeChoice.AddItem( "Trackball", VSPGUI::NAV_BALL_TRACKBALL );
    m_BallTypeChoice.AddItem( "Tumbler", VSPGUI::NAV_BALL_TUMBLER );
    m_BallTypeChoice.UpdateItems();
    m_ViewLayout.AddChoice( m_BallTypeChoice, "Type" );

    // A SliderInput's widgets, as Ball Speed is a preference rather than a Parm; the label as wide as
    // the choices' on this tab.
    int ball_button_w = m_ViewLayout.GetButtonWidth();
    m_ViewLayout.SetButtonWidth( m_ViewLayout.GetChoiceButtonWidth() );
    m_ViewLayout.AddSliderInputWidgets( "Speed", &m_BallSpeedLabel, &m_BallSpeedSlider, &m_BallSpeedInput );
    m_ViewLayout.SetButtonWidth( ball_button_w );
    m_BallSpeedSlider->bounds( BALL_SPEED_MIN, BALL_SPEED_MAX );
    m_BallSpeedSlider->callback( staticScreenCB, this );
    m_BallSpeedInput->callback( staticScreenCB, this );
    m_BallSpeedSlider->tooltip( "How fast the ball turns, as a multiple.  Not the Tumbler, which has its own speed." );
    m_BallSpeedInput->tooltip( "How fast the ball turns, as a multiple.  Not the Tumbler, which has its own speed." );

    m_BallSizeChoice.AddItem( "60% of view", 0 );
    m_BallSizeChoice.AddItem( "75% of view", 1 );
    m_BallSizeChoice.AddItem( "90% of view", 2 );
    m_BallSizeChoice.AddItem( "100% of view", 3 );
    m_BallSizeChoice.UpdateItems();
    m_ViewLayout.AddChoice( m_BallSizeChoice, "Size" );
    m_BallSizeChoice.GetFlChoice()->tooltip( "Rounded Arcball only" );

    m_BallOutlineChoice.AddItem( "Show while rotating", 1 );
    m_BallOutlineChoice.AddItem( "Hide", 0 );
    m_BallOutlineChoice.UpdateItems();
    m_ViewLayout.AddChoice( m_BallOutlineChoice, "Outline" );
    m_BallOutlineChoice.GetFlChoice()->tooltip( "Rounded Arcball only" );
    m_ViewLayout.AddYGap();

    m_ViewLayout.AddDividerBox( "Turntable and Tumbler" );

    m_TurntableSpeedChoice.AddItem( "0.2 deg/point", 0 );
    m_TurntableSpeedChoice.AddItem( "0.3 deg/point", 1 );
    m_TurntableSpeedChoice.AddItem( "0.4 deg/point", 2 );
    m_TurntableSpeedChoice.AddItem( "0.6 deg/point", 3 );
    m_TurntableSpeedChoice.UpdateItems();
    m_ViewLayout.AddChoice( m_TurntableSpeedChoice, "Speed" );
    m_TurntableSpeedChoice.GetFlChoice()->tooltip( "Degrees turned per point of mouse motion" );
    m_ViewLayout.AddYGap();

    m_ViewLayout.AddDividerBox( "Zoom" );

    m_WheelDirChoice.AddItem( "Forward zooms in", 0 );
    m_WheelDirChoice.AddItem( "Forward zooms out", 1 );
    m_WheelDirChoice.UpdateItems();
    m_ViewLayout.AddChoice( m_WheelDirChoice, "Wheel Direction" );

    m_WheelAnchorChoice.AddItem( "At pointer", VSPGUI::NAV_ANCHOR_POINTER );
    m_WheelAnchorChoice.AddItem( "At view center", VSPGUI::NAV_ANCHOR_CENTER );
    m_WheelAnchorChoice.UpdateItems();
    m_ViewLayout.AddChoice( m_WheelAnchorChoice, "Wheel Zooms" );

    m_WheelStepChoice.AddItem( "Fixed per notch", VSPGUI::NAV_WHEEL_PER_EVENT );
    m_WheelStepChoice.AddItem( "By scroll amount", VSPGUI::NAV_WHEEL_PROPORTIONAL );
    m_WheelStepChoice.UpdateItems();
    m_ViewLayout.AddChoice( m_WheelStepChoice, "Wheel Step" );
    m_ViewLayout.AddYGap();

    m_DragZoomDirChoice.AddItem( "Down zooms in", 0 );
    m_DragZoomDirChoice.AddItem( "Down zooms out", 1 );
    m_DragZoomDirChoice.UpdateItems();
    m_ViewLayout.AddChoice( m_DragZoomDirChoice, "Drag Direction" );

    m_DragZoomAnchorChoice.AddItem( "At view center", VSPGUI::NAV_ANCHOR_CENTER );
    m_DragZoomAnchorChoice.AddItem( "At press point", VSPGUI::NAV_ANCHOR_POINTER );
    m_DragZoomAnchorChoice.UpdateItems();
    m_ViewLayout.AddChoice( m_DragZoomAnchorChoice, "Drag Zooms" );

    m_DragZoomLawChoice.AddItem( "Classic, per event", VSPGUI::NAV_ZOOM_PER_EVENT );
    m_DragZoomLawChoice.AddItem( "By distance", VSPGUI::NAV_ZOOM_CONTINUOUS );
    m_DragZoomLawChoice.UpdateItems();
    m_ViewLayout.AddChoice( m_DragZoomLawChoice, "Drag Step" );
    m_ViewLayout.AddYGap();

    m_ZoomSpeedChoice.AddItem( "Slow", 0 );
    m_ZoomSpeedChoice.AddItem( "Normal", 1 );
    m_ZoomSpeedChoice.AddItem( "Fast", 2 );
    m_ZoomSpeedChoice.UpdateItems();
    m_ViewLayout.AddChoice( m_ZoomSpeedChoice, "Zoom Speed" );
    m_ZoomSpeedChoice.GetFlChoice()->tooltip( "Speed of the wheel by scroll amount and of the drag by distance" );

    if ( VSPGUI::NavDevice::IsBuilt() )
    {
        m_ViewLayout.AddYGap();
        m_ViewLayout.AddDividerBox( "3D Device" );

        m_DeviceChoice.AddItem( "Use when present", 1 );
        m_DeviceChoice.AddItem( "Off", 0 );
        m_DeviceChoice.UpdateItems();
        m_ViewLayout.AddChoice( m_DeviceChoice, "Rotatrix" );
        m_DeviceChoice.GetFlChoice()->tooltip( "The Rotatrix 3D trackball, through its host application (version 1.6 or later).  OpenVSP uses it once the host is verified." );

        int view_button_w = m_ViewLayout.GetButtonWidth();
        m_ViewLayout.SetButtonWidth( m_ViewLayout.GetChoiceButtonWidth() );
        m_ViewLayout.AddOutput( m_DeviceStatus, "Status" );
        m_ViewLayout.SetButtonWidth( view_button_w );
    }

    //==== Mouse Buttons ====//
    Fl_Group * buttons_tab = AddTab( "Mouse Buttons" );
    Fl_Group * buttons_group = AddSubGroup( buttons_tab, 5 );
    m_ButtonsLayout.SetGroupAndScreen( buttons_group, this );

    m_ButtonsLayout.AddChoice( m_ButtonsProfileChoice, "Scheme" );
    m_ButtonsLayout.AddYGap();

    int label_w = m_ButtonsLayout.GetChoiceButtonWidth();

    m_ButtonsLayout.AddDividerBox( "All Bindings" );

    // What the first two settings mean, in the window rather than only in tooltips.
    int about_h = 3 * m_ButtonsLayout.GetStdHeight();
    Fl_Box * about = new Fl_Box( m_ButtonsLayout.GetX(), m_ButtonsLayout.GetY(), m_ButtonsLayout.GetW(), about_h );
    about->box( FL_NO_BOX );
    about->labelsize( 12 );
    about->align( FL_ALIGN_LEFT | FL_ALIGN_TOP | FL_ALIGN_INSIDE | FL_ALIGN_WRAP );
    about->label( "A press may be a click until the mouse moves the distance below; a middle click can then set "
                  "the rotation center while a middle drag rotates.  With two buttons down, letting go of one "
                  "goes on with the other, as OpenVSP always has, or stops, as CAD programs do." );
    buttons_group->add( about );
    m_ButtonsLayout.AddY( about_h );

    m_ButtonsLayout.SetChoiceButtonWidth( 130 );

    m_ClickDistanceChoice.AddItem( "At once (no clicks while moving)", 0 );
    m_ClickDistanceChoice.AddItem( "After 2 points", 1 );
    m_ClickDistanceChoice.AddItem( "After 4 points", 2 );
    m_ClickDistanceChoice.AddItem( "After 8 points", 3 );
    m_ClickDistanceChoice.UpdateItems();
    m_ButtonsLayout.AddChoice( m_ClickDistanceChoice, "Drag Starts" );
    m_ClickDistanceChoice.GetFlChoice()->tooltip( "How far a press must move before it is a drag rather than a click" );

    m_ChordChoice.AddItem( "Go on with the other button", 0 );
    m_ChordChoice.AddItem( "Stop", 1 );
    m_ChordChoice.UpdateItems();
    m_ButtonsLayout.AddChoice( m_ChordChoice, "Release One of Two" );
    m_ChordChoice.GetFlChoice()->tooltip( "What letting go of one of two buttons held together does" );

    m_EmulateMiddleChoice.AddItem( "As bound", 0 );
    m_EmulateMiddleChoice.AddItem( "Middle button", 1 );
    m_EmulateMiddleChoice.UpdateItems();
    m_ButtonsLayout.AddChoice( m_EmulateMiddleChoice, "Alt + Left" );
    m_EmulateMiddleChoice.GetFlChoice()->tooltip( "Middle button: Alt (Option) with the left button acts as the middle button, for mice and trackpads without one.  Not where the mouse buttons bind Alt + left themselves, as OpenVSP's own do.  Some Linux desktops take Alt + drag to move windows." );

    m_ButtonsLayout.SetChoiceButtonWidth( label_w );
    m_ButtonsLayout.AddYGap();

    m_ButtonsLayout.AddDividerBox( "Bindings: the first that fits a press acts" );

    static int binding_widths[] = { 120, 190, 110, 0 };
    m_BindingBrowser = m_ButtonsLayout.AddColResizeBrowser( binding_widths, 3, 160 );
    m_BindingBrowser->callback( staticScreenCB, this );

    int bw = m_ButtonsLayout.GetW() / 4;
    m_ButtonsLayout.SetSameLineFlag( true );
    m_ButtonsLayout.SetFitWidthFlag( false );
    m_ButtonsLayout.SetButtonWidth( bw );
    m_ButtonsLayout.AddButton( m_AddBindingButton, "Add" );
    m_ButtonsLayout.AddButton( m_DeleteBindingButton, "Delete" );
    m_ButtonsLayout.AddButton( m_UpBindingButton, "Up" );
    m_ButtonsLayout.AddButton( m_DownBindingButton, "Down" );
    m_ButtonsLayout.ForceNewLine();
    m_ButtonsLayout.SetSameLineFlag( false );
    m_ButtonsLayout.SetFitWidthFlag( true );
    m_AddBindingButton.GetFlButton()->tooltip( "Add a binding above the selected one, like it" );

    m_ButtonsLayout.AddYGap();
    m_ButtonsLayout.AddDividerBox( "Selected Binding" );

    const int actions[] = { VSPGUI::NAV_ROTATE, VSPGUI::NAV_ROLL, VSPGUI::NAV_PAN, VSPGUI::NAV_ZOOM, VSPGUI::NAV_ZOOM_IN,
                            VSPGUI::NAV_ZOOM_OUT, VSPGUI::NAV_ZOOM_BOX, VSPGUI::NAV_SET_PIVOT, VSPGUI::NAV_SELECT };
    for ( int i = 0; i < ( int )( sizeof( actions ) / sizeof( actions[0] ) ); i++ )
    {
        m_ActionChoice.AddItem( ActionName( actions[i] ), actions[i] );
    }
    m_ActionChoice.AddItem( ActionName( VSPGUI::NAV_NONE ), VSPGUI::NAV_NONE );
    m_ActionChoice.UpdateItems();

    int half = m_ButtonsLayout.GetW() / 2;

    m_TriggerChoice.AddItem( "Drag", VSPGUI::NAV_ON_DRAG );
    m_TriggerChoice.AddItem( "Click", VSPGUI::NAV_ON_CLICK );
    m_TriggerChoice.UpdateItems();

    m_WithButtonChoice.AddItem( "None", 0 );
    for ( int i = 0; i < NUM_BUTTONS; i++ )
    {
        m_ButtonChoice.AddItem( BUTTON_NAMES[i], i );
        m_WithButtonChoice.AddItem( BUTTON_NAMES[i], i + 1 );
    }
    m_ButtonChoice.UpdateItems();
    m_WithButtonChoice.UpdateItems();

    m_ButtonsLayout.SetSameLineFlag( true );
    m_ButtonsLayout.SetChoiceButtonWidth( 90 );
    m_ButtonsLayout.AddChoice( m_ActionChoice, "Action", half );
    m_ButtonsLayout.AddChoice( m_TriggerChoice, "On", half );
    m_ButtonsLayout.ForceNewLine();
    m_ActionChoice.GetFlChoice()->tooltip( "Tool picks only: the press goes to the tool picks (rotation center, rulers, probes, ...) and nothing else" );
    m_ButtonsLayout.AddChoice( m_ButtonChoice, "Button", half );
    m_ButtonsLayout.AddChoice( m_WithButtonChoice, "With", half );
    m_ButtonsLayout.ForceNewLine();
    m_ButtonChoice.GetFlChoice()->tooltip( "The button that must be down; Back and Forward are a mouse's side buttons" );
    m_WithButtonChoice.GetFlChoice()->tooltip( "A second button that must be down with it, for a chord, or none" );

    for ( int i = 0; i < 4; i++ )
    {
        m_ModChoice[i].AddItem( "Up", MOD_UP );
        m_ModChoice[i].AddItem( "Down", MOD_DOWN );
        m_ModChoice[i].AddItem( "Either", MOD_EITHER );
        m_ModChoice[i].UpdateItems();
    }
    m_ButtonsLayout.AddChoice( m_ModChoice[0], VSPGUI::NavModifierName( FL_SHIFT ), half );
    m_ButtonsLayout.AddChoice( m_ModChoice[1], VSPGUI::NavModifierName( FL_CTRL ), half );
    m_ButtonsLayout.ForceNewLine();
    m_ButtonsLayout.AddChoice( m_ModChoice[2], VSPGUI::NavModifierName( FL_ALT ), half );
    m_ButtonsLayout.AddChoice( m_ModChoice[3], VSPGUI::NavModifierName( FL_META ), half );
    m_ButtonsLayout.ForceNewLine();

    m_OtherButtonsChoice.AddItem( "Up", 0 );
    m_OtherButtonsChoice.AddItem( "Either", 1 );
    m_OtherButtonsChoice.AddItem( "Some", 2 );
    m_OtherButtonsChoice.UpdateItems();
    m_ButtonsLayout.AddChoice( m_OtherButtonsChoice, "Other Buttons", half );
    m_OtherButtonsChoice.GetFlChoice()->tooltip( "Whether buttons the binding does not need may also be down" );
    int button_w = m_ButtonsLayout.GetButtonWidth();
    m_ButtonsLayout.SetButtonWidth( 90 );
    m_ButtonsLayout.AddInput( m_HeldKeyInput, "Held Key", half );
    m_ButtonsLayout.SetButtonWidth( button_w );
    m_ButtonsLayout.ForceNewLine();

    m_WhereChoice.AddItem( "Anywhere", VSPGUI::NAV_ANYWHERE );
    m_WhereChoice.AddItem( "On geometry", VSPGUI::NAV_ON_GEOMETRY );
    m_WhereChoice.AddItem( "On background", VSPGUI::NAV_ON_BACKGROUND );
    m_WhereChoice.UpdateItems();
    m_PickFirstChoice.AddItem( "Not offered", 0 );
    m_PickFirstChoice.AddItem( "Offered first", 1 );
    m_PickFirstChoice.UpdateItems();
    m_ButtonsLayout.AddChoice( m_WhereChoice, "Where", half );
    m_ButtonsLayout.AddChoice( m_PickFirstChoice, "Tool Picks", half );
    m_PickFirstChoice.GetFlChoice()->tooltip( "Offered first: a tool pick in progress (rotation center, rulers, probes, ...) takes the press" );
    m_ButtonsLayout.ForceNewLine();
    m_ButtonsLayout.SetSameLineFlag( false );
    m_ButtonsLayout.SetChoiceButtonWidth( label_w );

    m_ButtonsLayout.AddYGap();
    m_ButtonsLayout.AddDividerBox( "Try a Press" );

    int box_h = 44;
    m_PressBox = new NavPressBox( m_ButtonsLayout.GetX(), m_ButtonsLayout.GetY(), m_ButtonsLayout.GetW(), box_h );
    m_PressBox->callback( staticScreenCB, this );
    buttons_group->add( m_PressBox );
    m_ButtonsLayout.AddY( box_h );
    m_ButtonsLayout.AddButton( m_UsePressButton, "Use This Press for the Selected Binding" );

    m_ButtonsLayout.AddYGap();
    int notes_h = 4 * m_ButtonsLayout.GetStdHeight();
    m_NotesOutput = new Fl_Multiline_Output( m_ButtonsLayout.GetX(), m_ButtonsLayout.GetY(), m_ButtonsLayout.GetW(), notes_h );
    m_NotesOutput->wrap( 1 );
    m_NotesOutput->textsize( 12 );
    m_NotesOutput->color( ( Fl_Color )23 );
    buttons_group->add( m_NotesOutput );
    m_ButtonsLayout.AddY( notes_h );

    //==== SpaceMouse ====//
    for ( int i = 0; i < VSPGUI::NAV_NUM_AXES; i++ )
    {
        m_AxisMeter[i] = nullptr;
    }
    if ( VSPGUI::SpaceMouse::IsBuilt() )
    {
        Fl_Group * sm_tab = AddTab( "SpaceMouse" );
        Fl_Group * sm_group = AddSubGroup( sm_tab, 5 );
        m_SpaceMouseLayout.SetGroupAndScreen( sm_group, this );

        m_SpaceMouseLayout.AddDividerBox( "Device" );

        m_SpaceMouseChoice.AddItem( "Use when present", 1 );
        m_SpaceMouseChoice.AddItem( "Off", 0 );
        m_SpaceMouseChoice.UpdateItems();
        m_SpaceMouseLayout.AddChoice( m_SpaceMouseChoice, "SpaceMouse" );
        m_SpaceMouseChoice.GetFlChoice()->tooltip( "A 3Dconnexion SpaceMouse or other six-axis device on USB, read with no vendor driver.  It moves the view while OpenVSP has the keyboard focus." );

        int sm_button_w = m_SpaceMouseLayout.GetButtonWidth();
        m_SpaceMouseLayout.SetButtonWidth( m_SpaceMouseLayout.GetChoiceButtonWidth() );
        m_SpaceMouseLayout.AddOutput( m_SpaceMouseStatus, "Status" );
        m_SpaceMouseLayout.SetButtonWidth( sm_button_w );
        m_SpaceMouseLayout.AddYGap();

        m_SpaceMouseLayout.AddDividerBox( "Motion" );

        const char * speed_names[] = { "Slowest", "Slow", "Normal", "Fast", "Fastest" };
        for ( int i = 0; i < VSPGUI::NAV_NUM_SPACEMOUSE_SPEEDS; i++ )
        {
            m_SpaceMouseSpeedChoice.AddItem( speed_names[i], i );
        }
        m_SpaceMouseSpeedChoice.UpdateItems();
        m_SpaceMouseLayout.AddChoice( m_SpaceMouseSpeedChoice, "Speed" );
        m_SpaceMouseSpeedChoice.GetFlChoice()->tooltip( "How fast the SpaceMouse turns, pans and zooms the view" );

        m_SpaceMouseRotateChoice.AddItem( "On", 1 );
        m_SpaceMouseRotateChoice.AddItem( "Off", 0 );
        m_SpaceMouseRotateChoice.UpdateItems();
        m_SpaceMouseLayout.AddChoice( m_SpaceMouseRotateChoice, "Turns" );
        m_SpaceMouseRotateChoice.GetFlChoice()->tooltip( "Off: the axes that turn the model do nothing" );

        m_SpaceMouseMoveChoice.AddItem( "On", 1 );
        m_SpaceMouseMoveChoice.AddItem( "Off", 0 );
        m_SpaceMouseMoveChoice.UpdateItems();
        m_SpaceMouseLayout.AddChoice( m_SpaceMouseMoveChoice, "Pan and Zoom" );
        m_SpaceMouseMoveChoice.GetFlChoice()->tooltip( "Off: the axes that pan and zoom do nothing" );

        m_SpaceMouseDominantChoice.AddItem( "Every axis", 0 );
        m_SpaceMouseDominantChoice.AddItem( "Only the axis pushed furthest", 1 );
        m_SpaceMouseDominantChoice.UpdateItems();
        m_SpaceMouseLayout.AddChoice( m_SpaceMouseDominantChoice, "Acting Axes" );
        m_SpaceMouseDominantChoice.GetFlChoice()->tooltip( "Only the axis pushed furthest: one motion at a time, so twisting the cap does not also pan or zoom.  3Dconnexion calls this Dominant." );
        m_SpaceMouseLayout.AddYGap();

        m_SpaceMouseLayout.AddDividerBox( "Axes" );

        int about_h = 2 * m_SpaceMouseLayout.GetStdHeight();
        Fl_Box * about = new Fl_Box( m_SpaceMouseLayout.GetX(), m_SpaceMouseLayout.GetY(), m_SpaceMouseLayout.GetW(), about_h );
        about->box( FL_NO_BOX );
        about->labelsize( 12 );
        about->align( FL_ALIGN_LEFT | FL_ALIGN_TOP | FL_ALIGN_INSIDE | FL_ALIGN_WRAP );
        about->label( "What each motion of the cap does.  Move the cap: each bar shows how far its axis is pushed; "
                      "inside the shaded middle an axis does nothing." );
        sm_group->add( about );
        m_SpaceMouseLayout.AddY( about_h );

        // Each row: the motion and what it does, whether reversed, and how far it is pushed.
        int label_w = m_SpaceMouseLayout.GetChoiceButtonWidth();
        int row_w = m_SpaceMouseLayout.GetW();
        int motion_w = 110;
        int action_w = 170;
        int reverse_w = 80;
        int meter_w = row_w - motion_w - action_w - reverse_w;

        // A choice's width is what the rest of the line leaves it.
        m_SpaceMouseLayout.SetSameLineFlag( true );
        for ( int i = 0; i < VSPGUI::NAV_NUM_AXES; i++ )
        {
            for ( int a = 0; a < VSPGUI::NAV_NUM_AXIS_ACTIONS; a++ )
            {
                m_AxisChoice[i].AddItem( AXIS_ACTION_NAMES[a], a );
            }
            m_AxisChoice[i].UpdateItems();
            m_AxisReverseChoice[i].AddItem( "Normal", 0 );
            m_AxisReverseChoice[i].AddItem( "Reverse", 1 );
            m_AxisReverseChoice[i].UpdateItems();

            m_SpaceMouseLayout.SetChoiceButtonWidth( motion_w );
            m_SpaceMouseLayout.AddChoice( m_AxisChoice[i], AXIS_LABELS[i], reverse_w + meter_w );
            m_SpaceMouseLayout.SetChoiceButtonWidth( 0 );
            m_SpaceMouseLayout.AddChoice( m_AxisReverseChoice[i], "", row_w - reverse_w );
            m_AxisReverseChoice[i].GetFlChoice()->tooltip( "Reversed: the axis acts the other way" );

            m_AxisMeter[i] = new NavAxisMeter( m_SpaceMouseLayout.GetX(), m_SpaceMouseLayout.GetY(), meter_w,
                                               m_SpaceMouseLayout.GetStdHeight() );
            m_AxisMeter[i]->tooltip( "How far the cap is pushed along this axis" );
            sm_group->add( m_AxisMeter[i] );
            m_SpaceMouseLayout.ForceNewLine();
        }
        m_SpaceMouseLayout.SetSameLineFlag( false );
        m_SpaceMouseLayout.SetChoiceButtonWidth( label_w );

        m_SpaceMouseLayout.AddYGap();
        m_SpaceMouseLayout.AddButton( m_AxisDefaultsButton, "Default Axes" );
        m_AxisDefaultsButton.GetFlButton()->tooltip( "Put every axis back as OpenVSP starts, with the model moving as the cap does" );
    }

    //==== Bottom ====//
    m_BottomLayout.SetGroupAndScreen( m_FLTK_Window, this );
    m_BottomLayout.SetY( m_FLTK_Window->h() - 27 );
    m_BottomLayout.SetSameLineFlag( true );
    m_BottomLayout.SetFitWidthFlag( false );

    int gap = ( m_BottomLayout.GetW() - 5 * m_BottomLayout.GetButtonWidth() ) / 6;

    m_BottomLayout.AddX( gap );
    m_BottomLayout.AddButton( m_SaveButton, "Save..." );
    m_BottomLayout.AddX( gap );
    m_BottomLayout.AddButton( m_LoadButton, "Load..." );
    m_BottomLayout.AddX( gap );
    m_BottomLayout.AddButton( m_DefaultsButton, "Defaults" );
    m_BottomLayout.AddX( gap );
    m_BottomLayout.AddButton( m_RevertButton, "Revert" );
    m_BottomLayout.AddX( gap );
    m_BottomLayout.AddButton( m_CloseButton, "Close" );

    m_SaveButton.GetFlButton()->tooltip( "Save the preferences to a file, to load on this or another computer." );
    m_LoadButton.GetFlButton()->tooltip( "Load preferences saved to a file.  Revert undoes it." );
    m_DefaultsButton.GetFlButton()->tooltip( "Set every preference to OpenVSP's default.  Revert undoes it." );
    m_RevertButton.GetFlButton()->tooltip( "Changes take effect and are kept as they are made.  Revert puts back the preferences as they were when this window opened." );

    // Open on the first tab.
    GetTabs()->value( gen_tab );
}

PreferencesScreen::~PreferencesScreen()
{
    Fl::remove_timeout( StaticMeterCB, this );
}

bool PreferencesScreen::Update()
{
    TabScreen::Update();

    MainVSPScreen * main = dynamic_cast < MainVSPScreen * > ( m_ScreenMgr->GetScreen( vsp::VSP_MAIN_SCREEN ) );
    if ( main && VSPGUI::NavDevice::IsBuilt() )
    {
        m_DeviceStatus.Update( main->GetNavDevice()->GetStatus() );
    }
    if ( main && VSPGUI::SpaceMouse::IsBuilt() )
    {
        m_SpaceMouseStatus.Update( main->GetSpaceMouse()->GetStatus() );
    }

    m_FLTK_Window->redraw();

    return false;
}

void PreferencesScreen::Show()
{
    // Opening the window again keeps what it was reverting to.
    if ( !IsShown() && !m_ModeChanging )
    {
        m_FCTypeAtShow = GetFCType();
        m_NavAtShow.Load();
        m_ToolbarAtShow = GetToolbarSize();
        m_WindowModeAtShow = m_ScreenMgr->GetWindowMode();
        m_BrowserPlaceAtShow = GetBrowserPlace();
    }

    m_FCTypeChoice.SetVal( GetFCType() );
    m_ToolbarChoice.SetVal( GetToolbarSize() );
    m_WindowModeChoice.SetVal( m_ScreenMgr->GetWindowMode() );
    m_BrowserPlaceChoice.SetVal( GetBrowserPlace() );

    m_Nav.Load();
    ShowNav();

    Fl::remove_timeout( StaticMeterCB, this );
    if ( VSPGUI::SpaceMouse::IsBuilt() )
    {
        Fl::add_timeout( 0.05, StaticMeterCB, this );
    }

    m_ScreenMgr->SetUpdateFlag( true );
    TabScreen::Show();
}

void PreferencesScreen::StaticMeterCB( void * data )
{
    PreferencesScreen * screen = ( PreferencesScreen * )data;
    screen->ShowMeters();
    Fl::repeat_timeout( 0.05, StaticMeterCB, data );
}

void PreferencesScreen::ShowMeters()
{
    MainVSPScreen * main = dynamic_cast < MainVSPScreen * > ( m_ScreenMgr->GetScreen( vsp::VSP_MAIN_SCREEN ) );
    if ( !main )
    {
        return;
    }
    int counts[ VSPGUI::NAV_NUM_AXES ];
    main->GetSpaceMouse()->GetCounts( counts );
    for ( int i = 0; i < VSPGUI::NAV_NUM_AXES; i++ )
    {
        if ( m_AxisMeter[i] )
        {
            m_AxisMeter[i]->SetCounts( counts[i] );
        }
    }
}

void PreferencesScreen::ShowNav()
{
    m_RotateModeChoice.SetVal( m_Nav.m_StartupRotateMode );
    m_BallTypeChoice.SetVal( m_Nav.m_BallType );
    m_BallSizeChoice.SetVal( NearestIndex( BALL_SIZES, 4, m_Nav.m_BallSize ) );
    ShowBallSpeed();
    m_BallOutlineChoice.SetVal( ( int )m_Nav.m_ShowBallOutline );
    m_PivotChoice.SetVal( m_Nav.m_Pivot );
    m_EmulateMiddleChoice.SetVal( ( int )m_Nav.m_EmulateMiddle );
    m_TurntableSpeedChoice.SetVal( NearestIndex( TURNTABLE_GAINS, 4, m_Nav.m_TurntableGain ) );

    m_WheelDirChoice.SetVal( ( int )m_Nav.m_WheelReverse );
    m_WheelAnchorChoice.SetVal( m_Nav.m_WheelAnchor );
    m_WheelStepChoice.SetVal( m_Nav.m_WheelStep );
    m_DragZoomDirChoice.SetVal( ( int )m_Nav.m_DragZoomReverse );
    m_DragZoomAnchorChoice.SetVal( m_Nav.m_DragZoomAnchor );
    m_DragZoomLawChoice.SetVal( m_Nav.m_DragZoomLaw );
    m_ZoomSpeedChoice.SetVal( NearestIndex( ZOOM_SPEEDS, 3, m_Nav.m_ZoomSpeed ) );
    if ( VSPGUI::NavDevice::IsBuilt() )
    {
        m_DeviceChoice.SetVal( ( int )m_Nav.m_UseDevice );
    }
    if ( VSPGUI::SpaceMouse::IsBuilt() )
    {
        m_SpaceMouseChoice.SetVal( ( int )m_Nav.m_UseSpaceMouse );
        m_SpaceMouseSpeedChoice.SetVal( NearestIndex( VSPGUI::NAV_SPACEMOUSE_SPEEDS, VSPGUI::NAV_NUM_SPACEMOUSE_SPEEDS,
                                                      m_Nav.m_SpaceMouseSpeed ) );
        m_SpaceMouseRotateChoice.SetVal( ( int )m_Nav.m_SpaceMouseRotate );
        m_SpaceMouseMoveChoice.SetVal( ( int )m_Nav.m_SpaceMouseMove );
        m_SpaceMouseDominantChoice.SetVal( ( int )m_Nav.m_SpaceMouseDominant );
        for ( int i = 0; i < VSPGUI::NAV_NUM_AXES; i++ )
        {
            m_AxisChoice[i].SetVal( m_Nav.m_SpaceMouseAxis[i] );
            m_AxisReverseChoice[i].SetVal( ( int )m_Nav.m_SpaceMouseReverse[i] );
        }
    }

    ShowActive();
    ShowScheme();
    ShowBindings();
}

void PreferencesScreen::ShowBallSpeed()
{
    m_BallSpeedSlider->value( m_Nav.m_BallSpeed );
    char str[32];
    snprintf( str, sizeof( str ), "%5.3f", m_Nav.m_BallSpeed );
    m_BallSpeedInput->value( str );
}

void PreferencesScreen::ShowActive()
{
    // The ball's other settings shape the rounded arcball alone.
    if ( m_Nav.m_BallType == VSPGUI::NAV_BALL_ROUNDED_ARCBALL )
    {
        m_BallSizeChoice.Activate();
        m_BallOutlineChoice.Activate();
    }
    else
    {
        m_BallSizeChoice.Deactivate();
        m_BallOutlineChoice.Deactivate();
    }

    // The Tumbler's speed is the one it shares with the Turntable.
    if ( m_Nav.m_BallType == VSPGUI::NAV_BALL_TUMBLER )
    {
        m_BallSpeedLabel->deactivate();
        m_BallSpeedSlider->deactivate();
        m_BallSpeedInput->deactivate();
    }
    else
    {
        m_BallSpeedLabel->activate();
        m_BallSpeedSlider->activate();
        m_BallSpeedInput->activate();
    }

    // The zoom speed scales only the wheel by scroll amount and the drag by distance.
    if ( m_Nav.m_WheelStep == VSPGUI::NAV_WHEEL_PROPORTIONAL || m_Nav.m_DragZoomLaw == VSPGUI::NAV_ZOOM_CONTINUOUS )
    {
        m_ZoomSpeedChoice.Activate();
    }
    else
    {
        m_ZoomSpeedChoice.Deactivate();
    }
}

bool PreferencesScreen::ReadNav( GuiDevice * device )
{
    if ( device == &m_BallTypeChoice )
    {
        m_Nav.m_BallType = m_BallTypeChoice.GetVal();
    }
    else if ( device == &m_BallSizeChoice )
    {
        m_Nav.m_BallSize = BALL_SIZES[ m_BallSizeChoice.GetVal() ];
    }
    else if ( device == &m_RotateModeChoice )
    {
        m_Nav.m_StartupRotateMode = m_RotateModeChoice.GetVal();
    }
    else if ( device == &m_BallOutlineChoice )
    {
        m_Nav.m_ShowBallOutline = ( m_BallOutlineChoice.GetVal() != 0 );
    }
    else if ( device == &m_PivotChoice )
    {
        m_Nav.m_Pivot = m_PivotChoice.GetVal();
    }
    else if ( device == &m_EmulateMiddleChoice )
    {
        m_Nav.m_EmulateMiddle = ( m_EmulateMiddleChoice.GetVal() != 0 );
    }
    else if ( device == &m_TurntableSpeedChoice )
    {
        m_Nav.m_TurntableGain = TURNTABLE_GAINS[ m_TurntableSpeedChoice.GetVal() ];
    }
    else if ( device == &m_WheelDirChoice )
    {
        m_Nav.m_WheelReverse = ( m_WheelDirChoice.GetVal() != 0 );
    }
    else if ( device == &m_WheelAnchorChoice )
    {
        m_Nav.m_WheelAnchor = m_WheelAnchorChoice.GetVal();
    }
    else if ( device == &m_WheelStepChoice )
    {
        m_Nav.m_WheelStep = m_WheelStepChoice.GetVal();
    }
    else if ( device == &m_DragZoomDirChoice )
    {
        m_Nav.m_DragZoomReverse = ( m_DragZoomDirChoice.GetVal() != 0 );
    }
    else if ( device == &m_DragZoomAnchorChoice )
    {
        m_Nav.m_DragZoomAnchor = m_DragZoomAnchorChoice.GetVal();
    }
    else if ( device == &m_DragZoomLawChoice )
    {
        m_Nav.m_DragZoomLaw = m_DragZoomLawChoice.GetVal();
    }
    else if ( device == &m_ZoomSpeedChoice )
    {
        m_Nav.m_ZoomSpeed = ZOOM_SPEEDS[ m_ZoomSpeedChoice.GetVal() ];
    }
    else if ( device == &m_DeviceChoice )
    {
        m_Nav.m_UseDevice = ( m_DeviceChoice.GetVal() != 0 );
    }
    else if ( device == &m_SpaceMouseChoice )
    {
        m_Nav.m_UseSpaceMouse = ( m_SpaceMouseChoice.GetVal() != 0 );
    }
    else if ( device == &m_SpaceMouseSpeedChoice )
    {
        m_Nav.m_SpaceMouseSpeed = VSPGUI::NAV_SPACEMOUSE_SPEEDS[ m_SpaceMouseSpeedChoice.GetVal() ];
    }
    else if ( device == &m_SpaceMouseRotateChoice )
    {
        m_Nav.m_SpaceMouseRotate = ( m_SpaceMouseRotateChoice.GetVal() != 0 );
    }
    else if ( device == &m_SpaceMouseMoveChoice )
    {
        m_Nav.m_SpaceMouseMove = ( m_SpaceMouseMoveChoice.GetVal() != 0 );
    }
    else if ( device == &m_SpaceMouseDominantChoice )
    {
        m_Nav.m_SpaceMouseDominant = ( m_SpaceMouseDominantChoice.GetVal() != 0 );
    }
    else if ( device == &m_AxisDefaultsButton )
    {
        VSPGUI::NavSettings def;
        for ( int i = 0; i < VSPGUI::NAV_NUM_AXES; i++ )
        {
            m_Nav.m_SpaceMouseAxis[i] = def.m_SpaceMouseAxis[i];
            m_Nav.m_SpaceMouseReverse[i] = def.m_SpaceMouseReverse[i];
        }
        ShowNav();
    }
    else
    {
        for ( int i = 0; i < VSPGUI::NAV_NUM_AXES; i++ )
        {
            if ( device == &m_AxisChoice[i] )
            {
                m_Nav.m_SpaceMouseAxis[i] = m_AxisChoice[i].GetVal();
                return true;
            }
            if ( device == &m_AxisReverseChoice[i] )
            {
                m_Nav.m_SpaceMouseReverse[i] = ( m_AxisReverseChoice[i].GetVal() != 0 );
                return true;
            }
        }
        return false;
    }
    return true;
}

bool PreferencesScreen::ReadBinding( GuiDevice * device )
{
    std::vector < VSPGUI::NavBinding > & bindings = m_Nav.m_Profile.m_Bindings;

    if ( device == &m_ClickDistanceChoice )
    {
        m_Nav.m_Profile.m_ClickDistance = CLICK_DISTANCES[ m_ClickDistanceChoice.GetVal() ];
        return true;
    }
    if ( device == &m_ChordChoice )
    {
        m_Nav.m_Profile.m_ChordEnds = ( m_ChordChoice.GetVal() != 0 );
        return true;
    }

    if ( device == &m_AddBindingButton )
    {
        VSPGUI::NavBinding b = VSPGUI::NavBind( FL_BUTTON1, 0, VSPGUI::NAV_ROTATE );
        int at = 0;
        if ( m_SelBinding >= 0 && m_SelBinding < ( int )bindings.size() )
        {
            b = bindings[ m_SelBinding ];
            at = m_SelBinding;
        }
        bindings.insert( bindings.begin() + at, b );
        m_SelBinding = at;
        return true;
    }

    if ( m_SelBinding < 0 || m_SelBinding >= ( int )bindings.size() )
    {
        return false;
    }
    VSPGUI::NavBinding & b = bindings[ m_SelBinding ];

    if ( device == &m_DeleteBindingButton )
    {
        bindings.erase( bindings.begin() + m_SelBinding );
        if ( m_SelBinding >= ( int )bindings.size() )
        {
            m_SelBinding = ( int )bindings.size() - 1;
        }
    }
    else if ( device == &m_UpBindingButton )
    {
        if ( m_SelBinding > 0 )
        {
            std::swap( bindings[ m_SelBinding ], bindings[ m_SelBinding - 1 ] );
            m_SelBinding--;
        }
    }
    else if ( device == &m_DownBindingButton )
    {
        if ( m_SelBinding + 1 < ( int )bindings.size() )
        {
            std::swap( bindings[ m_SelBinding ], bindings[ m_SelBinding + 1 ] );
            m_SelBinding++;
        }
    }
    else if ( device == &m_ActionChoice )
    {
        b.m_Action = ( VSPGUI::NavAction )m_ActionChoice.GetVal();
        if ( b.m_Action == VSPGUI::NAV_ZOOM_IN || b.m_Action == VSPGUI::NAV_ZOOM_OUT )
        {
            b.m_Trigger = VSPGUI::NAV_ON_DRAG;
        }
    }
    else if ( device == &m_TriggerChoice )
    {
        b.m_Trigger = m_TriggerChoice.GetVal();
    }
    else if ( device == &m_ButtonChoice || device == &m_WithButtonChoice )
    {
        // Other buttons allowed before stay allowed, except those now needed.
        bool any_other = ( b.m_OtherButtons == ( ALL_BUTTONS & ~b.m_Buttons ) );
        b.m_Buttons = BUTTONS[ m_ButtonChoice.GetVal() ];
        int with = m_WithButtonChoice.GetVal();
        if ( with > 0 )
        {
            b.m_Buttons |= BUTTONS[ with - 1 ];
        }
        b.m_OtherButtons &= ~b.m_Buttons;
        if ( any_other )
        {
            b.m_OtherButtons = ALL_BUTTONS & ~b.m_Buttons;
        }
        if ( ( b.m_PressedButton & b.m_Buttons ) == 0 )
        {
            b.m_PressedButton = 0;
        }
    }
    else if ( device == &m_OtherButtonsChoice )
    {
        // Some only shows a partial set; choosing it changes nothing.
        if ( m_OtherButtonsChoice.GetVal() == 0 )
        {
            b.m_OtherButtons = 0;
        }
        else if ( m_OtherButtonsChoice.GetVal() == 1 )
        {
            b.m_OtherButtons = ALL_BUTTONS & ~b.m_Buttons;
        }
    }
    else if ( device == &m_WhereChoice )
    {
        b.m_Where = m_WhereChoice.GetVal();
    }
    else if ( device == &m_PickFirstChoice )
    {
        b.m_PickFirst = ( m_PickFirstChoice.GetVal() != 0 );
    }
    else if ( device == &m_HeldKeyInput )
    {
        std::string key = m_HeldKeyInput.GetString();
        b.m_HeldKey = 0;
        if ( !key.empty() && std::isalnum( ( unsigned char )key[0] ) )
        {
            b.m_HeldKey = std::tolower( ( unsigned char )key[0] );
        }
    }
    else if ( device == &m_UsePressButton )
    {
        if ( !m_PressBox->m_HasPress )
        {
            return false;
        }
        // Exactly this press: the buttons and keys down, and no others.
        const VSPGUI::NavPress & p = m_PressBox->m_Press;
        b.m_Buttons = p.m_Buttons;
        b.m_OtherButtons = 0;
        b.m_Modifiers = p.m_Modifiers;
        b.m_OtherModifiers = 0;
        if ( ( b.m_PressedButton & b.m_Buttons ) == 0 )
        {
            b.m_PressedButton = 0;
        }
        b.m_HeldKey = 0;
        if ( !p.m_HeldKeys.empty() )
        {
            b.m_HeldKey = *p.m_HeldKeys.begin();
        }
    }
    else
    {
        bool found = false;
        for ( int i = 0; i < 4; i++ )
        {
            if ( device == &m_ModChoice[i] )
            {
                int bit = MODIFIER_BITS[i];
                b.m_Modifiers &= ~bit;
                b.m_OtherModifiers &= ~bit;
                if ( m_ModChoice[i].GetVal() == MOD_DOWN )
                {
                    b.m_Modifiers |= bit;
                }
                else if ( m_ModChoice[i].GetVal() == MOD_EITHER )
                {
                    b.m_OtherModifiers |= bit;
                }
                found = true;
            }
        }
        if ( !found )
        {
            return false;
        }
    }
    return true;
}

void PreferencesScreen::ShowBindings()
{
    const std::vector < VSPGUI::NavBinding > & bindings = m_Nav.m_Profile.m_Bindings;
    std::vector < int > shadowed = m_Nav.m_Profile.Shadowed();

    if ( m_SelBinding >= ( int )bindings.size() )
    {
        m_SelBinding = ( int )bindings.size() - 1;
    }

    int vpos = m_BindingBrowser->vposition();
    m_BindingBrowser->clear();
    m_BindingBrowser->column_char( ':' );
    m_BindingBrowser->add( "@b@.ACTION:@b@.PRESS:@b@.NOTE" );

    for ( int i = 0; i < ( int )bindings.size(); i++ )
    {
        const VSPGUI::NavBinding & b = bindings[i];

        std::string note;
        if ( std::find( shadowed.begin(), shadowed.end(), i ) != shadowed.end() )
        {
            note = "@C1@.never acts";
        }
        else if ( b.m_Where == VSPGUI::NAV_ON_GEOMETRY )
        {
            note = "@.on geometry";
        }
        else if ( b.m_Where == VSPGUI::NAV_ON_BACKGROUND )
        {
            note = "@.on background";
        }
        else if ( b.m_OtherModifiers != 0 )
        {
            note = "@.other keys ok";
        }

        std::string row = std::string( "@." ) + ActionName( b.m_Action ) + ":@." + VSPGUI::NavDescribe( b ) + ":" + note;
        m_BindingBrowser->add( row.c_str() );
    }
    if ( m_SelBinding >= 0 )
    {
        m_BindingBrowser->select( m_SelBinding + 2 );
    }
    m_BindingBrowser->vposition( vpos );

    // The selected binding.
    bool has_sel = ( m_SelBinding >= 0 );
    GuiDevice * edits[] = { &m_DeleteBindingButton, &m_UpBindingButton, &m_DownBindingButton, &m_ActionChoice,
                            &m_TriggerChoice, &m_ButtonChoice, &m_WithButtonChoice, &m_ModChoice[0], &m_ModChoice[1],
                            &m_ModChoice[2], &m_ModChoice[3], &m_OtherButtonsChoice, &m_HeldKeyInput, &m_WhereChoice,
                            &m_PickFirstChoice };
    for ( int i = 0; i < ( int )( sizeof( edits ) / sizeof( edits[0] ) ); i++ )
    {
        if ( has_sel )
        {
            edits[i]->Activate();
        }
        else
        {
            edits[i]->Deactivate();
        }
    }
    // Zoom in and Zoom out act at the press; On does not apply.
    if ( has_sel && ( bindings[ m_SelBinding ].m_Action == VSPGUI::NAV_ZOOM_IN ||
                      bindings[ m_SelBinding ].m_Action == VSPGUI::NAV_ZOOM_OUT ) )
    {
        m_TriggerChoice.Deactivate();
    }
    if ( has_sel && m_PressBox->m_HasPress )
    {
        m_UsePressButton.Activate();
    }
    else
    {
        m_UsePressButton.Deactivate();
    }

    if ( has_sel )
    {
        const VSPGUI::NavBinding & b = bindings[ m_SelBinding ];
        m_ActionChoice.SetVal( b.m_Action );
        m_TriggerChoice.SetVal( b.m_Trigger );
        // The lowest button needed is the button, and the next the one with it.
        int button = -1;
        int with = 0;
        for ( int i = 0; i < NUM_BUTTONS; i++ )
        {
            if ( b.m_Buttons & BUTTONS[i] )
            {
                if ( button < 0 )
                {
                    button = i;
                }
                else if ( with == 0 )
                {
                    with = i + 1;
                }
            }
        }
        if ( button < 0 )
        {
            button = 0;
        }
        m_ButtonChoice.SetVal( button );
        m_WithButtonChoice.SetVal( with );
        for ( int i = 0; i < 4; i++ )
        {
            int state = MOD_UP;
            if ( b.m_Modifiers & MODIFIER_BITS[i] )
            {
                state = MOD_DOWN;
            }
            else if ( b.m_OtherModifiers & MODIFIER_BITS[i] )
            {
                state = MOD_EITHER;
            }
            m_ModChoice[i].SetVal( state );
        }
        int others = 2;
        if ( b.m_OtherButtons == 0 )
        {
            others = 0;
        }
        else if ( b.m_OtherButtons == ( ALL_BUTTONS & ~b.m_Buttons ) )
        {
            others = 1;
        }
        m_OtherButtonsChoice.SetVal( others );
        std::string key;
        if ( b.m_HeldKey != 0 )
        {
            key += ( char )b.m_HeldKey;
        }
        m_HeldKeyInput.Update( key );
        m_WhereChoice.SetVal( b.m_Where );
        int pick = 0;
        if ( b.m_PickFirst )
        {
            pick = 1;
        }
        m_PickFirstChoice.SetVal( pick );
    }

    int slop = 0;
    for ( int i = 1; i < 4; i++ )
    {
        if ( std::abs( CLICK_DISTANCES[i] - m_Nav.m_Profile.m_ClickDistance ) < std::abs( CLICK_DISTANCES[slop] - m_Nav.m_Profile.m_ClickDistance ) )
        {
            slop = i;
        }
    }
    m_ClickDistanceChoice.SetVal( slop );
    int chord = 0;
    if ( m_Nav.m_Profile.m_ChordEnds )
    {
        chord = 1;
    }
    m_ChordChoice.SetVal( chord );

    // Notes: bindings that can never act, and gestures a one-button mouse cannot make.
    std::string notes;
    if ( !shadowed.empty() )
    {
        notes += "Red rows never act: rows above take every press they fit.  Move them up, or make the rows above need more.\n";
    }

    const VSPGUI::NavAction moves[] = { VSPGUI::NAV_ROTATE, VSPGUI::NAV_PAN, VSPGUI::NAV_ZOOM };
    std::string missing;
    for ( int a = 0; a < 3; a++ )
    {
        bool left_only = false;
        for ( int i = 0; i < ( int )bindings.size(); i++ )
        {
            bool usable = ( std::find( shadowed.begin(), shadowed.end(), i ) == shadowed.end() );
            if ( usable && bindings[i].m_Action == moves[a] && bindings[i].m_Trigger == VSPGUI::NAV_ON_DRAG )
            {
                int buttons = bindings[i].m_Buttons;
                // Alt + left stands in for the middle button when that is on and free.
                if ( buttons == FL_BUTTON2 && m_Nav.m_EmulateMiddle && ( bindings[i].m_Modifiers & FL_ALT ) == 0 )
                {
                    VSPGUI::NavPress alt_left;
                    alt_left.m_Buttons = FL_BUTTON1;
                    alt_left.m_Modifiers = FL_ALT | bindings[i].m_Modifiers;
                    alt_left.m_PressedButton = FL_BUTTON1;
                    VSPGUI::NavPress p = m_Nav.Effective( alt_left );
                    if ( m_Nav.m_Profile.Match( p, VSPGUI::NAV_ON_DRAG ) == &bindings[i] )
                    {
                        buttons = FL_BUTTON1;
                    }
                }
                if ( buttons == FL_BUTTON1 )
                {
                    left_only = true;
                }
            }
        }
        if ( !left_only )
        {
            if ( !missing.empty() )
            {
                missing += ", ";
            }
            missing += ActionName( moves[a] );
        }
    }
    if ( !missing.empty() )
    {
        notes += "With the left button alone (one-button mouse, trackpad) there is no " + missing + ".  Add a left button binding with a key, or set Alt + Left to the middle button on the 3D View tab.\n";
    }

#if !defined( __APPLE__ ) && !defined( _WIN32 )
    for ( int i = 0; i < ( int )bindings.size(); i++ )
    {
        if ( bindings[i].m_Modifiers & FL_ALT )
        {
            notes += "Some Linux desktops take Alt + drag to move windows; such bindings may never reach OpenVSP.\n";
            break;
        }
    }
#endif

    if ( notes.empty() )
    {
        notes = "Select a row to edit it.  Press in the box above to see what a press does.";
    }
    m_NotesOutput->value( notes.c_str() );

    ShowTriedPress();
}

void PreferencesScreen::ShowTriedPress()
{
    static std::string label;

    if ( !m_PressBox->m_HasPress )
    {
        label = "Press here with the buttons and keys to try";
    }
    else
    {
        VSPGUI::NavPress press = m_Nav.Effective( m_PressBox->m_Press );
        label = VSPGUI::NavDescribe( m_PressBox->m_Press ) + ":  ";

        const std::vector < VSPGUI::NavBinding > & bindings = m_Nav.m_Profile.m_Bindings;
        const VSPGUI::NavBinding * drag = m_Nav.m_Profile.Match( press, VSPGUI::NAV_ON_DRAG );
        const VSPGUI::NavBinding * click = m_Nav.m_Profile.Match( press, VSPGUI::NAV_ON_CLICK );

        if ( drag )
        {
            label += std::string( "drag " ) + ActionName( drag->m_Action ) + " (row " + std::to_string( ( int )( drag - &bindings[0] ) + 1 ) + ")";
        }
        else
        {
            label += "drag does nothing";
        }
        if ( click )
        {
            label += std::string( ", click " ) + ActionName( click->m_Action ) + " (row " + std::to_string( ( int )( click - &bindings[0] ) + 1 ) + ")";
        }
    }
    m_PressBox->label( label.c_str() );
    m_PressBox->redraw();
}

void PreferencesScreen::Hide()
{
    Fl::remove_timeout( StaticMeterCB, this );
    VspScreen::Hide();
    m_ScreenMgr->SetUpdateFlag( true );
}

void PreferencesScreen::CallBack( Fl_Widget* w )
{
    if ( w == m_BindingBrowser )
    {
        m_SelBinding = m_BindingBrowser->value() - 2;
        ShowBindings();
    }
    else if ( w == m_PressBox )
    {
        ShowBindings();
    }
    else if ( w == m_BallSpeedSlider || w == m_BallSpeedInput )
    {
        double speed = m_BallSpeedSlider->value();
        if ( w == m_BallSpeedInput )
        {
            speed = atof( m_BallSpeedInput->value() );
        }
        m_Nav.m_BallSpeed = std::max( BALL_SPEED_MIN, std::min( BALL_SPEED_MAX, speed ) );
        ShowBallSpeed();
        ShowScheme();
        ApplyNav();
    }
    m_ScreenMgr->SetUpdateFlag( true );
}

void PreferencesScreen::GuiDeviceCallBack( GuiDevice* device )
{
    assert( m_ScreenMgr );

    if ( device == &m_FCTypeChoice )
    {
        SetFCType( m_FCTypeChoice.GetVal() );
    }
    else if ( device == &m_ToolbarChoice )
    {
        SetToolbarSize( m_ToolbarChoice.GetVal() );
    }
    else if ( device == &m_WindowModeChoice )
    {
        SetWindowMode( m_WindowModeChoice.GetVal() );
    }
    else if ( device == &m_BrowserPlaceChoice )
    {
        SetBrowserPlace( m_BrowserPlaceChoice.GetVal() );
    }
    else if ( device == &m_SaveButton )
    {
        string fname = m_ScreenMgr->FileChooser( "Save Preferences", "*.vspprefs", vsp::SAVE );
        if ( !fname.empty() && !SavePrefsFile( fname ) )
        {
            fl_alert( "Preferences not written to %s", fname.c_str() );
        }
    }
    else if ( device == &m_LoadButton )
    {
        string fname = m_ScreenMgr->FileChooser( "Load Preferences", "*.vspprefs" );
        if ( !fname.empty() )
        {
            string err = LoadPrefsFile( fname );
            if ( !err.empty() )
            {
                fl_alert( "%s %s.  No preferences were changed.", fname.c_str(), err.c_str() );
            }
            else
            {
                ApplyStored();
            }
        }
    }
    else if ( device == &m_DefaultsButton )
    {
        SetFCType( vsp::FC_OPENVSP );
        m_FCTypeChoice.SetVal( vsp::FC_OPENVSP );
        SetToolbarSize( TOOLBAR_MEDIUM );
        m_ToolbarChoice.SetVal( TOOLBAR_MEDIUM );
        SetWindowMode( vsp::WINDOW_MODE_FREE );
        m_WindowModeChoice.SetVal( vsp::WINDOW_MODE_FREE );
        SetBrowserPlace( BROWSER_FLOATING );
        m_BrowserPlaceChoice.SetVal( BROWSER_FLOATING );
        m_Nav = VSPGUI::NavSettings();
        ShowNav();
        ApplyNav();
    }
    else if ( device == &m_RevertButton )
    {
        SetFCType( m_FCTypeAtShow );
        m_FCTypeChoice.SetVal( m_FCTypeAtShow );
        SetToolbarSize( m_ToolbarAtShow );
        m_ToolbarChoice.SetVal( m_ToolbarAtShow );
        SetWindowMode( m_WindowModeAtShow );
        m_WindowModeChoice.SetVal( m_WindowModeAtShow );
        SetBrowserPlace( m_BrowserPlaceAtShow );
        m_BrowserPlaceChoice.SetVal( m_BrowserPlaceAtShow );
        m_Nav = m_NavAtShow;
        ShowNav();
        ApplyNav();
    }
    else if ( device == &m_CloseButton )
    {
        Hide();
    }
    else if ( device == &m_ProfileChoice )
    {
        ApplyScheme( m_ProfileChoice.GetVal() );
    }
    else if ( device == &m_ButtonsProfileChoice )
    {
        ApplyScheme( m_ButtonsProfileChoice.GetVal() );
    }
    else if ( ReadNav( device ) )
    {
        ShowActive();
        ShowScheme();
        ShowBindings();
        ApplyNav();
    }
    else if ( ReadBinding( device ) )
    {
        ShowScheme();
        ShowBindings();
        ApplyNav();
    }

    m_ScreenMgr->SetUpdateFlag( true );
}

int PreferencesScreen::GetToolbarSize()
{
    MainVSPScreen * main = dynamic_cast < MainVSPScreen * > ( m_ScreenMgr->GetScreen( vsp::VSP_MAIN_SCREEN ) );
    if ( main )
    {
        return main->GetToolbarSize();
    }
    return TOOLBAR_MEDIUM;
}

void PreferencesScreen::SetToolbarSize( int size )
{
    MainVSPScreen * main = dynamic_cast < MainVSPScreen * > ( m_ScreenMgr->GetScreen( vsp::VSP_MAIN_SCREEN ) );
    if ( main )
    {
        main->SetToolbarSize( size );
    }
}

int PreferencesScreen::GetBrowserPlace()
{
    MainVSPScreen * main = dynamic_cast < MainVSPScreen * > ( m_ScreenMgr->GetScreen( vsp::VSP_MAIN_SCREEN ) );
    if ( main )
    {
        return main->GetBrowserPlace();
    }
    return BROWSER_FLOATING;
}

void PreferencesScreen::SetBrowserPlace( int place )
{
    MainVSPScreen * main = dynamic_cast < MainVSPScreen * > ( m_ScreenMgr->GetScreen( vsp::VSP_MAIN_SCREEN ) );
    if ( main )
    {
        main->SetBrowserPlace( place );
    }
}

void PreferencesScreen::SetWindowMode( int mode )
{
    ScreenMgr::SetWindowModePref( mode );

    // After this control's callback, as its window moves
    if ( mode != m_ScreenMgr->GetWindowMode() )
    {
        Fl::add_timeout( 0.0, StaticWindowModeCB, this );
    }
}

void PreferencesScreen::StaticWindowModeCB( void* data )
{
    PreferencesScreen* ps = static_cast< PreferencesScreen* >( data );
    ps->m_ModeChanging = true;
    ps->m_ScreenMgr->SetWindowMode( ScreenMgr::GetWindowModePref() );
    ps->m_ModeChanging = false;
}

void PreferencesScreen::ApplyNav()
{
    m_Nav.Save();

    MainVSPScreen * main = dynamic_cast < MainVSPScreen * > ( m_ScreenMgr->GetScreen( vsp::VSP_MAIN_SCREEN ) );
    if ( main )
    {
        main->SetNavSettings( m_Nav );
    }
}

void PreferencesScreen::ApplyStored()
{
    m_FCTypeChoice.SetVal( GetFCType() );

    int size = TOOLBAR_MEDIUM;
    int place = BROWSER_FLOATING;
    {
        Fl_Preferences prefs( Fl_Preferences::USER, "openvsp.org", "VSP" );
        Fl_Preferences app( prefs, "Application" );
        app.get( "Toolbar_Size", size, TOOLBAR_MEDIUM );
        app.get( "Geom_Browser_Place", place, BROWSER_FLOATING );
    }
    SetToolbarSize( size );
    m_ToolbarChoice.SetVal( GetToolbarSize() );

    SetBrowserPlace( place );
    m_BrowserPlaceChoice.SetVal( GetBrowserPlace() );

    int mode = ScreenMgr::GetWindowModePref();
    SetWindowMode( mode );
    m_WindowModeChoice.SetVal( mode );

    m_Nav.Load();
    ShowNav();
    ApplyNav();
}

void PreferencesScreen::ApplyScheme( int scheme )
{
    // A preset sets everything it covers; Custom keeps the settings as they are.
    if ( scheme < VSPGUI::NAV_NUM_PRESETS )
    {
        m_Nav.ApplyPreset( scheme );
    }
    ShowNav();
    ApplyNav();
}

void PreferencesScreen::ShowScheme()
{
    int preset = m_Nav.FindPreset();
    if ( preset < 0 )
    {
        preset = VSPGUI::NAV_NUM_PRESETS;
    }
    m_ProfileChoice.SetVal( preset );
    m_ButtonsProfileChoice.SetVal( preset );

    m_ProfileTip = m_Nav.m_Profile.Summary();
    m_ProfileChoice.GetFlChoice()->tooltip( m_ProfileTip.c_str() );
    m_ButtonsProfileChoice.GetFlChoice()->tooltip( m_ProfileTip.c_str() );
}

int PreferencesScreen::GetFCType()
{
    Fl_Preferences prefs( Fl_Preferences::USER, "openvsp.org", "VSP" );
    Fl_Preferences app( prefs, "Application" );

    int fc_type = vsp::FC_OPENVSP;
    app.get( "File_Chooser", fc_type, vsp::FC_OPENVSP );

    return fc_type;
}

void PreferencesScreen::SetFCType( int fc )
{
    Fl_Preferences prefs( Fl_Preferences::USER, "openvsp.org", "VSP" );
    Fl_Preferences app( prefs, "Application" );

    app.set( "File_Chooser", fc );

    prefs.flush();
}
