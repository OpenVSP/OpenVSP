//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// STLOptionsScreen.cpp: implementation of the STLOptionsScreen class.
//
//////////////////////////////////////////////////////////////////////

#include "STLOptionsScreen.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

STLOptionsScreen::STLOptionsScreen( ScreenMgr* mgr ) : BasicScreen( mgr, 250, 170, "STL Options" )
{
    m_FLTK_Window->callback( staticCloseCB, this );

    m_OkFlag = false;
    m_PrevMultiSolid = false;

    m_GenLayout.SetGroupAndScreen( m_FLTK_Window, this );
    m_GenLayout.AddY( 25 );

    m_GenLayout.AddYGap();

    m_GenLayout.AddButton( m_MultiSolidToggle, "Tagged Multi Solid File (Non-Standard)" );

    m_GenLayout.AddY( 95 );
    m_GenLayout.SetFitWidthFlag( false );
    m_GenLayout.SetSameLineFlag( true );
    m_GenLayout.SetButtonWidth( 100 );

    m_GenLayout.AddX( 20 );
    m_GenLayout.AddButton( m_OkButton, "OK" );
    m_GenLayout.AddX( 10 );
    m_GenLayout.AddButton( m_CancelButton, "Cancel" );
    m_GenLayout.ForceNewLine();
}

STLOptionsScreen::~STLOptionsScreen()
{
}

bool STLOptionsScreen::Update()
{
    Vehicle *veh = VehicleMgr.GetVehicle();

    if( veh )
    {
    	m_MultiSolidToggle.Update( veh->m_STLMultiSolid.GetID() );
    }

    m_FLTK_Window->redraw();

    return false;
}


void STLOptionsScreen::Show()
{
    Update();
    m_FLTK_Window->show();
}

void STLOptionsScreen::CallBack( Fl_Widget* w )
{
    m_ScreenMgr->SetUpdateFlag( true );
}

void STLOptionsScreen::GuiDeviceCallBack( GuiDevice* device )
{
    assert( m_ScreenMgr );

    if ( device == &m_OkButton )
    {
        m_OkFlag = true;
        Hide();
    }
    else if ( device == &m_CancelButton )
    {
        Vehicle *veh = VehicleMgr.GetVehicle();

        if( veh )
        {
            veh->m_STLMultiSolid.Set( m_PrevMultiSolid );
        }
        Hide();
    }

    m_ScreenMgr->SetUpdateFlag( true );
}

bool STLOptionsScreen::ShowSTLOptionsScreen()
{
    Show();

    m_OkFlag = false;

    Vehicle *veh = VehicleMgr.GetVehicle();

    if( veh )
    {
        m_PrevMultiSolid = veh->m_STLMultiSolid();
    }

    while( m_FLTK_Window->shown() )
    {
        Fl::wait();
    }

    return m_OkFlag;
}

void STLOptionsScreen::CloseCallBack( Fl_Widget *w )
{
    assert( m_ScreenMgr );

    Vehicle *veh = VehicleMgr.GetVehicle();

    if( veh )
    {
        veh->m_STLMultiSolid.Set( m_PrevMultiSolid );
    }

    Hide();
}
