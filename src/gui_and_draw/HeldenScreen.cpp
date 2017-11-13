//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#include "HeldenScreen.h"
#include "ProjectionMgr.h"

HeldenScreen::HeldenScreen( ScreenMgr* mgr ) : BasicScreen( mgr, 300, 397, "Helden Mesh Setup" )
{
    m_FLTK_Window->callback( staticCloseCB, this );
    m_MainLayout.SetGroupAndScreen( m_FLTK_Window, this );

}

HeldenScreen::~HeldenScreen()
{
}

void HeldenScreen::LoadSetChoice( Choice & c, int & index )
{
    c.ClearItems();

    Vehicle* veh = m_ScreenMgr->GetVehiclePtr();
    vector< string > set_name_vec = veh->GetSetNameVec();

    for ( int i = 0 ; i < ( int )set_name_vec.size() ; ++i )
    {
        c.AddItem( set_name_vec[i].c_str() );
    }

    c.UpdateItems();
    c.SetVal( index );
}

void HeldenScreen::Show()
{
    m_ScreenMgr->SetUpdateFlag( true );
    m_FLTK_Window->show();
}

void HeldenScreen::Hide()
{
    m_FLTK_Window->hide();
    m_ScreenMgr->SetUpdateFlag( true );
}

bool HeldenScreen::Update()
{
    Vehicle* vehiclePtr = m_ScreenMgr->GetVehiclePtr();

    m_FLTK_Window->redraw();
    return false;
}

// Callback for Link Browser
void HeldenScreen::CallBack( Fl_Widget* w )
{
    assert( m_ScreenMgr );

    m_ScreenMgr->SetUpdateFlag( true );

}

void HeldenScreen::CloseCallBack( Fl_Widget *w )
{
    Hide();
}

// Callback for all other GUI Devices
void HeldenScreen::GuiDeviceCallBack( GuiDevice* device )
{
    Vehicle* vehiclePtr = m_ScreenMgr->GetVehiclePtr();

    assert( m_ScreenMgr );

    m_ScreenMgr->SetUpdateFlag( true );
}
