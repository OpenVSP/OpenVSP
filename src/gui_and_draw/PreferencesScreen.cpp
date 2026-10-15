//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// PreferencesScreen.cpp: implementation of the PreferencesScreen class.
//
//////////////////////////////////////////////////////////////////////

#include "PreferencesScreen.h"

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
    }

    m_FCTypeChoice.SetVal( GetFCType() );

    m_ScreenMgr->SetUpdateFlag( true );
    TabScreen::Show();
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
    }
    else if ( device == &m_RevertButton )
    {
        SetFCType( m_FCTypeAtShow );
        m_FCTypeChoice.SetVal( m_FCTypeAtShow );
    }
    else if ( device == &m_CloseButton )
    {
        Hide();
    }

    m_ScreenMgr->SetUpdateFlag( true );
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
