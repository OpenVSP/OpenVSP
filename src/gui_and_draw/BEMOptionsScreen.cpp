//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// BEMOptionsScreen.cpp: implementation of the BEMOptionsScreen class.
//
//////////////////////////////////////////////////////////////////////

#include "BEMOptionsScreen.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

BEMOptionsScreen::BEMOptionsScreen( ScreenMgr* mgr ) : BasicScreen( mgr, 250, 174, "BEM Options" )
{
    m_OkFlag = false;

    m_GenLayout.SetGroupAndScreen( m_FLTK_Window, this );
    m_GenLayout.AddY( 25 );

    m_GenLayout.AddYGap();

    m_PropGeomPicker.AddIncludeType( PROP_GEOM_TYPE );

    m_GenLayout.AddGeomPicker( m_PropGeomPicker );
    m_GenLayout.AddYGap();

    m_GenLayout.AddY( 25 );
    m_GenLayout.SetFitWidthFlag( false );
    m_GenLayout.SetSameLineFlag( true );
    m_GenLayout.SetButtonWidth( 100 );

    m_GenLayout.AddX( 20 );
    m_GenLayout.AddButton( m_OkButton, "OK" );
    m_GenLayout.AddX( 10 );
    m_GenLayout.AddButton( m_CancelButton, "Cancel" );
    m_GenLayout.ForceNewLine();
}

BEMOptionsScreen::~BEMOptionsScreen()
{
}

bool BEMOptionsScreen::Update()
{
    m_PropGeomPicker.Update();

    m_FLTK_Window->redraw();

    return false;
}


void BEMOptionsScreen::Show()
{
    Update();
    m_FLTK_Window->show();
}

void BEMOptionsScreen::CallBack( Fl_Widget* w )
{
    m_ScreenMgr->SetUpdateFlag( true );
}

void BEMOptionsScreen::GuiDeviceCallBack( GuiDevice* device )
{
    assert( m_ScreenMgr );

    if ( device == &m_OkButton )
    {
        m_OkFlag = true;
        Vehicle *veh = VehicleMgr.GetVehicle();

        if( veh )
        {
            veh->m_BEMPropID = m_PropGeomPicker.GetGeomChoice();
        }
        Hide();
    }
    else if ( device == &m_CancelButton )
    {
        Hide();
    }

    m_ScreenMgr->SetUpdateFlag( true );
}

bool BEMOptionsScreen::ShowBEMOptionsScreen()
{
    Show();

    m_OkFlag = false;

    while( m_FLTK_Window->shown() )
    {
        Fl::wait();
    }

    return m_OkFlag;
}
