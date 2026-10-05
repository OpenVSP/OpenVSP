//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// CloneNameSuffixScreen.cpp: implementation of the CloneNameSuffixScreen class.
//
//////////////////////////////////////////////////////////////////////

#include "CloneNameSuffixScreen.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CloneNameSuffixScreen::CloneNameSuffixScreen( ScreenMgr* mgr ) : BasicScreen( mgr, 400, 110, "Clone" )
{
    m_GenLayout.SetGroupAndScreen( m_FLTK_Window, this );

    m_GenLayout.ForceNewLine();
    m_GenLayout.AddY( 5 );
    m_GenLayout.AddX( 5 );

    m_GenLayout.AddSubGroupLayout( m_BorderLayout, m_GenLayout.GetRemainX() - 5.0,
                                   m_GenLayout.GetRemainY() - 5.0 );

    m_BorderLayout.SetButtonWidth( m_BorderLayout.GetW() / 3.5 );
    int spaceX = ( m_BorderLayout.GetW() - ( 2 * m_BorderLayout.GetButtonWidth() ) ) / 3;

    m_BorderLayout.AddYGap();

    m_BorderLayout.AddInput( m_NameSuffixInput, "Name Suffix" );

    m_BorderLayout.AddY( 25 );

    m_BorderLayout.SetSameLineFlag( true );
    m_BorderLayout.SetFitWidthFlag( false );

    m_BorderLayout.AddX( spaceX );
    m_BorderLayout.AddButton( m_OK, "Clone" );
    m_BorderLayout.AddX( spaceX );
    m_BorderLayout.AddButton( m_Cancel, "Cancel" );
}

CloneNameSuffixScreen::~CloneNameSuffixScreen()
{
}

bool CloneNameSuffixScreen::Update()
{
    BasicScreen::Update();

    m_FLTK_Window->redraw();

    return false;
}

void CloneNameSuffixScreen::Show()
{
    m_ScreenMgr->SetUpdateFlag( true );
    BasicScreen::Show();
}

void CloneNameSuffixScreen::Hide()
{
    m_FLTK_Window->hide();
    m_ScreenMgr->SetUpdateFlag( true );
}

void CloneNameSuffixScreen::SetupAndShow( const vector < string > &geom_id_vec )
{
    m_GeomIDVec = geom_id_vec;

    // Same default suffix a Clone made any other way gets.
    m_NameSuffixInput.Update( "_Clone" );

    Show();
}

void CloneNameSuffixScreen::CallBack( Fl_Widget* w )
{
    m_ScreenMgr->SetUpdateFlag( true );
}

void CloneNameSuffixScreen::GuiDeviceCallBack( GuiDevice* device )
{
    assert( m_ScreenMgr );

    if ( device == &m_OK )
    {
        Vehicle* veh = VehicleMgr.GetVehicle();

        if ( veh && !m_GeomIDVec.empty() )
        {
            // Geoms may have been deleted while the window was open.  CloneGeomVec silently
            // skips unknown IDs, so drop them here and report how many are missing.
            vector< string > live;
            for ( int i = 0; i < ( int )m_GeomIDVec.size(); i++ )
            {
                if ( veh->FindGeom( m_GeomIDVec[i] ) )
                {
                    live.push_back( m_GeomIDVec[i] );
                }
            }

            if ( live.size() != m_GeomIDVec.size() )
            {
                MessageData errMsgData;
                errMsgData.m_String = "Error";
                errMsgData.m_IntVec.push_back( vsp::VSP_INVALID_ID );
                char str[256];
                snprintf( str, sizeof( str ),
                          "Error:  %d of the %d Geoms chosen are no longer there.",
                          ( int )( m_GeomIDVec.size() - live.size() ), ( int )m_GeomIDVec.size() );
                errMsgData.m_StringVec.push_back( string( str ) );
                MessageMgr::getInstance().SendAll( errMsgData );
            }

            // One suffix for the whole selection.
            if ( !live.empty() )
            {
                veh->CloneGeomVec( live, m_NameSuffixInput.GetString() );
            }
        }

        m_GeomIDVec.clear();
        Hide();
    }
    else if ( device == &m_Cancel )
    {
        m_GeomIDVec.clear();
        Hide();
    }

    m_ScreenMgr->SetUpdateFlag( true );
}
