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
static const double BALL_ANGLE_SCALES[] = { 2.0, 1.0 };
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

    m_ViewLayout.AddDividerBox( "Rotation" );
    m_ViewLayout.AddYGap();

    m_RotateModeChoice.AddItem( "Ball", VSPGUI::NAV_ROTATE_BALL );
    m_RotateModeChoice.AddItem( "Turntable", VSPGUI::NAV_ROTATE_TURNTABLE );
    m_RotateModeChoice.UpdateItems();
    m_ViewLayout.AddChoice( m_RotateModeChoice, "Start With" );

    m_BallTypeChoice.AddItem( "Rounded Arcball", VSPGUI::NAV_BALL_ROUNDED_ARCBALL );
    m_BallTypeChoice.AddItem( "OpenVSP Classic", VSPGUI::NAV_BALL_CLASSIC );
    m_BallTypeChoice.UpdateItems();
    m_ViewLayout.AddChoice( m_BallTypeChoice, "Ball" );

    m_BallSizeChoice.AddItem( "60% of view", 0 );
    m_BallSizeChoice.AddItem( "75% of view", 1 );
    m_BallSizeChoice.AddItem( "90% of view", 2 );
    m_BallSizeChoice.AddItem( "100% of view", 3 );
    m_BallSizeChoice.UpdateItems();
    m_ViewLayout.AddChoice( m_BallSizeChoice, "Ball Size" );

    m_BallAngleChoice.AddItem( "360 deg across", 0 );
    m_BallAngleChoice.AddItem( "180 deg across", 1 );
    m_BallAngleChoice.UpdateItems();
    m_ViewLayout.AddChoice( m_BallAngleChoice, "Ball Speed" );

    m_BallOutlineChoice.AddItem( "Show while rotating", 1 );
    m_BallOutlineChoice.AddItem( "Hide", 0 );
    m_BallOutlineChoice.UpdateItems();
    m_ViewLayout.AddChoice( m_BallOutlineChoice, "Ball Outline" );

    m_TurntableSpeedChoice.AddItem( "0.2 deg/point", 0 );
    m_TurntableSpeedChoice.AddItem( "0.3 deg/point", 1 );
    m_TurntableSpeedChoice.AddItem( "0.4 deg/point", 2 );
    m_TurntableSpeedChoice.AddItem( "0.6 deg/point", 3 );
    m_TurntableSpeedChoice.UpdateItems();
    m_ViewLayout.AddChoice( m_TurntableSpeedChoice, "Turntable Speed" );

    m_ViewLayout.AddYGap();
    m_ViewLayout.AddDividerBox( "Zoom" );
    m_ViewLayout.AddYGap();

    m_WheelDirChoice.AddItem( "Forward zooms in", 0 );
    m_WheelDirChoice.AddItem( "Forward zooms out", 1 );
    m_WheelDirChoice.UpdateItems();
    m_ViewLayout.AddChoice( m_WheelDirChoice, "Wheel" );

    m_WheelAnchorChoice.AddItem( "At pointer", VSPGUI::NAV_ANCHOR_POINTER );
    m_WheelAnchorChoice.AddItem( "At view center", VSPGUI::NAV_ANCHOR_CENTER );
    m_WheelAnchorChoice.UpdateItems();
    m_ViewLayout.AddChoice( m_WheelAnchorChoice, "Wheel Zooms" );

    m_WheelStepChoice.AddItem( "Fixed per notch", VSPGUI::NAV_WHEEL_PER_EVENT );
    m_WheelStepChoice.AddItem( "By scroll amount", VSPGUI::NAV_WHEEL_PROPORTIONAL );
    m_WheelStepChoice.UpdateItems();
    m_ViewLayout.AddChoice( m_WheelStepChoice, "Wheel Step" );

    m_DragZoomDirChoice.AddItem( "Down zooms in", 0 );
    m_DragZoomDirChoice.AddItem( "Down zooms out", 1 );
    m_DragZoomDirChoice.UpdateItems();
    m_ViewLayout.AddChoice( m_DragZoomDirChoice, "Drag" );

    m_DragZoomAnchorChoice.AddItem( "At view center", VSPGUI::NAV_ANCHOR_CENTER );
    m_DragZoomAnchorChoice.AddItem( "At press point", VSPGUI::NAV_ANCHOR_POINTER );
    m_DragZoomAnchorChoice.UpdateItems();
    m_ViewLayout.AddChoice( m_DragZoomAnchorChoice, "Drag Zooms" );

    m_DragZoomLawChoice.AddItem( "Classic, per event", VSPGUI::NAV_ZOOM_PER_EVENT );
    m_DragZoomLawChoice.AddItem( "By distance", VSPGUI::NAV_ZOOM_CONTINUOUS );
    m_DragZoomLawChoice.UpdateItems();
    m_ViewLayout.AddChoice( m_DragZoomLawChoice, "Drag Rate" );

    m_ZoomSpeedChoice.AddItem( "Slow", 0 );
    m_ZoomSpeedChoice.AddItem( "Normal", 1 );
    m_ZoomSpeedChoice.AddItem( "Fast", 2 );
    m_ZoomSpeedChoice.UpdateItems();
    m_ViewLayout.AddChoice( m_ZoomSpeedChoice, "Zoom Speed" );
    m_ZoomSpeedChoice.GetFlChoice()->tooltip( "Speed of drag zoom by distance and of the wheel by scroll amount" );

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

    view_tab->show();
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
    m_BallAngleChoice.SetVal( NearestIndex( BALL_ANGLE_SCALES, 2, m_Nav.m_BallAngleScale ) );
    m_BallOutlineChoice.SetVal( ( int )m_Nav.m_ShowBallOutline );
    m_TurntableSpeedChoice.SetVal( NearestIndex( TURNTABLE_GAINS, 4, m_Nav.m_TurntableGain ) );

    m_WheelDirChoice.SetVal( ( int )m_Nav.m_WheelReverse );
    m_WheelAnchorChoice.SetVal( m_Nav.m_WheelAnchor );
    m_WheelStepChoice.SetVal( m_Nav.m_WheelStep );
    m_DragZoomDirChoice.SetVal( ( int )m_Nav.m_DragZoomReverse );
    m_DragZoomAnchorChoice.SetVal( m_Nav.m_DragZoomAnchor );
    m_DragZoomLawChoice.SetVal( m_Nav.m_DragZoomLaw );
    m_ZoomSpeedChoice.SetVal( NearestIndex( ZOOM_SPEEDS, 3, m_Nav.m_ZoomSpeed ) );
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
    else if ( device == &m_BallAngleChoice )
    {
        m_Nav.m_BallAngleScale = BALL_ANGLE_SCALES[ m_BallAngleChoice.GetVal() ];
    }
    else if ( device == &m_RotateModeChoice )
    {
        m_Nav.m_StartupRotateMode = m_RotateModeChoice.GetVal();
    }
    else if ( device == &m_BallOutlineChoice )
    {
        m_Nav.m_ShowBallOutline = ( m_BallOutlineChoice.GetVal() != 0 );
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
    else if ( ReadNav( device ) )
    {
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
