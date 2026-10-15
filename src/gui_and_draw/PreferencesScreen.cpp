//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// PreferencesScreen.cpp: implementation of the PreferencesScreen class.
//
//////////////////////////////////////////////////////////////////////

#include "PreferencesScreen.h"
#include "MainVSPScreen.h"
#include "NavController.h"

// Values offered for the numeric 3D interaction preferences.
static const double BALL_SIZES[] = { 0.6, 0.75, 0.9, 1.0 };
static const double BALL_SPEED_MIN = 0.1;
static const double BALL_SPEED_MAX = 2.0;
static const double TURNTABLE_GAINS[] = { 0.2, 0.3, 0.4, 0.6 };
static const double ZOOM_SPEEDS[] = { 0.5, 1.0, 2.0 };

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

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

PreferencesScreen::PreferencesScreen( ScreenMgr* mgr ) : TabScreen( mgr, 460, 800, "Preferences", "", 30 )
{
    m_FCTypeAtShow = vsp::FC_OPENVSP;

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

    //==== 3D View ====//
    Fl_Group * view_tab = AddTab( "3D View" );
    Fl_Group * view_group = AddSubGroup( view_tab, 5 );
    m_ViewLayout.SetGroupAndScreen( view_group, this );

    for ( int i = 0; i < VSPGUI::NAV_NUM_PRESETS; i++ )
    {
        m_ProfileChoice.AddItem( VSPGUI::NavProfile::PresetLabel( i ), i );
    }
    m_ProfileChoice.AddItem( "Custom", VSPGUI::NAV_NUM_PRESETS );
    m_ProfileChoice.UpdateItems();
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

    //==== Mouse Buttons ====//
    Fl_Group * buttons_tab = AddTab( "Mouse Buttons" );
    Fl_Group * buttons_group = AddSubGroup( buttons_tab, 5 );
    m_ButtonsLayout.SetGroupAndScreen( buttons_group, this );

    int label_w = m_ButtonsLayout.GetChoiceButtonWidth();

    m_ButtonsLayout.AddDividerBox( "All Bindings" );

    m_ButtonsLayout.SetChoiceButtonWidth( 130 );

    m_EmulateMiddleChoice.AddItem( "As bound", 0 );
    m_EmulateMiddleChoice.AddItem( "Middle button", 1 );
    m_EmulateMiddleChoice.UpdateItems();
    m_ButtonsLayout.AddChoice( m_EmulateMiddleChoice, "Alt + Left" );
    m_EmulateMiddleChoice.GetFlChoice()->tooltip( "Middle button: Alt (Option) with the left button acts as the middle button, for mice and trackpads without one.  Not where the mouse buttons bind Alt + left themselves, as OpenVSP's own do.  Some Linux desktops take Alt + drag to move windows." );

    m_ButtonsLayout.SetChoiceButtonWidth( label_w );
    m_ButtonsLayout.AddYGap();

    //==== Bottom ====//
    m_BottomLayout.SetGroupAndScreen( m_FLTK_Window, this );
    m_BottomLayout.SetY( m_FLTK_Window->h() - 27 );
    m_BottomLayout.SetSameLineFlag( true );
    m_BottomLayout.SetFitWidthFlag( false );

    int gap = ( m_BottomLayout.GetW() - 3 * m_BottomLayout.GetButtonWidth() ) / 4;

    m_BottomLayout.AddX( gap );
    m_BottomLayout.AddButton( m_DefaultsButton, "Defaults" );
    m_BottomLayout.AddX( gap );
    m_BottomLayout.AddButton( m_RevertButton, "Revert" );
    m_BottomLayout.AddX( gap );
    m_BottomLayout.AddButton( m_CloseButton, "Close" );

    m_DefaultsButton.GetFlButton()->tooltip( "Set every preference to OpenVSP's default.  Revert undoes it." );
    m_RevertButton.GetFlButton()->tooltip( "Changes take effect and are kept as they are made.  Revert puts back the preferences as they were when this window opened." );

    // Open on the first tab.
    GetTabs()->value( gen_tab );
}

PreferencesScreen::~PreferencesScreen()
{
}

bool PreferencesScreen::Update()
{
    TabScreen::Update();

    m_FLTK_Window->redraw();

    return false;
}

void PreferencesScreen::Show()
{
    // Opening the window again keeps what it was reverting to.
    if ( !IsShown() )
    {
        m_FCTypeAtShow = GetFCType();
        m_NavAtShow.Load();
    }

    m_FCTypeChoice.SetVal( GetFCType() );

    m_Nav.Load();
    ShowNav();

    m_ScreenMgr->SetUpdateFlag( true );
    TabScreen::Show();
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

    ShowActive();
    ShowScheme();
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
    else
    {
        return false;
    }
    return true;
}

void PreferencesScreen::Hide()
{
    m_FLTK_Window->hide();
    m_ScreenMgr->SetUpdateFlag( true );
}

void PreferencesScreen::CallBack( Fl_Widget* w )
{
    if ( w == m_BallSpeedSlider || w == m_BallSpeedInput )
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
    else if ( device == &m_DefaultsButton )
    {
        SetFCType( vsp::FC_OPENVSP );
        m_FCTypeChoice.SetVal( vsp::FC_OPENVSP );
        m_Nav = VSPGUI::NavSettings();
        ShowNav();
        ApplyNav();
    }
    else if ( device == &m_RevertButton )
    {
        SetFCType( m_FCTypeAtShow );
        m_FCTypeChoice.SetVal( m_FCTypeAtShow );
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
    else if ( ReadNav( device ) )
    {
        ShowActive();
        ShowScheme();
        ApplyNav();
    }

    m_ScreenMgr->SetUpdateFlag( true );
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

    m_ProfileTip = m_Nav.m_Profile.Summary();
    m_ProfileChoice.GetFlChoice()->tooltip( m_ProfileTip.c_str() );
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
