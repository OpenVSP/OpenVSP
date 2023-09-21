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

PreferencesScreen::PreferencesScreen( ScreenMgr* mgr ) : BasicScreen( mgr, 300, 170, "Preferences" )
{
    m_GenLayout.SetGroupAndScreen( m_FLTK_Window, this );

    m_GenLayout.ForceNewLine();
    m_GenLayout.AddY(7);
    m_GenLayout.AddX(5);

    m_GenLayout.AddSubGroupLayout( m_BorderLayout, m_GenLayout.GetRemainX() - 5.0,
                                   m_GenLayout.GetRemainY() - 5.0);

    m_BorderLayout.AddDividerBox( "File Chooser GUI" );
    m_BorderLayout.AddYGap();

    m_FCTypeChoice.AddItem( "OpenVSP Default", vsp::FC_OPENVSP );
    m_FCTypeChoice.AddItem( "Native", vsp::FC_NATIVE );
    m_FCTypeChoice.UpdateItems();

    m_BorderLayout.AddChoice( m_FCTypeChoice, "File Chooser" );

    m_BorderLayout.SetSameLineFlag( true );
    m_BorderLayout.SetFitWidthFlag( false );

    m_BorderLayout.SetY( m_BorderLayout.GetH() );

    int gap = ( m_BorderLayout.GetW() - 2 * m_BorderLayout.GetButtonWidth() ) / 3;

    m_BorderLayout.AddX( gap );
    m_BorderLayout.AddButton( m_AcceptButton, "Accept" );
    m_BorderLayout.AddX( gap );
    m_BorderLayout.AddButton( m_CancelButton, "Cancel" );
}

PreferencesScreen::~PreferencesScreen()
{
}

bool PreferencesScreen::Update()
{
    BasicScreen::Update();

    Vehicle *veh = VehicleMgr.GetVehicle();

    if( veh )
    {
    }

    m_FLTK_Window->redraw();

    return false;
}


void PreferencesScreen::Show()
{
    int fc = GetFCType();

    m_FCTypeChoice.SetVal( fc );

    m_ScreenMgr->SetUpdateFlag( true );
    BasicScreen::Show();
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

    if ( device == &m_AcceptButton )
    {
        int fc = m_FCTypeChoice.GetVal();

        SetFCType( fc );
        Hide();
    }
    else if ( device == &m_CancelButton )
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
