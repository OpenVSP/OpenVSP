//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#include "PickSetScreen.h"
#include "ScreenMgr.h"
#include "EventMgr.h"
#include "Vehicle.h"
#include "VehicleMgr.h"
#include "StlHelper.h"

#include <assert.h>

//==== Constructor ====//
PickSetScreen::PickSetScreen()
{
    PickSetUI* ui = m_PickSetUI = new PickSetUI();
    m_SelectedSetIndex = 0;

    ui->setChoice->callback( staticScreenCB, this );

    ui->okButton->callback( staticScreenCB, this );
    ui->cancelButton->callback( staticScreenCB, this );

}

int PickSetScreen::PickSet( string title )
{
    m_AcceptFlag = false;

    m_PickSetUI->pickTitleBox->copy_label( title.c_str() );

    Show();

    while( m_PickSetUI->UIWindow->shown() )
    {
        Fl::wait();
    }

    if ( m_AcceptFlag )
    {
        return m_SelectedSetIndex;
    }

    return -1;
}

//==== Update Screen ====//
bool PickSetScreen::Update()
{
    LoadSetChoice();

    return true;
}

//==== Show Screen ====//
void PickSetScreen::Show()
{
    Update();
    m_PickSetUI->UIWindow->show();
}

//==== Hide Screen ====//
void PickSetScreen::Hide()
{
    m_PickSetUI->UIWindow->hide();
}

//==== Load Type Choice ====//
void PickSetScreen::LoadSetChoice()
{
    m_PickSetUI->setChoice->clear();

    Vehicle* veh = VehicleMgr::getInstance().GetVehicle();
    vector< string > set_name_vec = veh->GetSetNameVec();

    for ( int i = 0 ; i < ( int )set_name_vec.size() ; i++ )
    {
        m_PickSetUI->setChoice->add( set_name_vec[i].c_str() );
    }

    m_PickSetUI->setChoice->value( m_SelectedSetIndex );

}

//==== Callbacks ====//
void PickSetScreen::CallBack( Fl_Widget *w )
{
    Vehicle* veh = VehicleMgr::getInstance().GetVehicle();
    string newfile;

    if ( w ==   m_PickSetUI->okButton )
    {
        m_AcceptFlag = true;
        Hide();
    }
    else if ( w == m_PickSetUI->cancelButton )
    {
        m_AcceptFlag = false;
        Hide();
    }
    else if ( w == m_PickSetUI->setChoice )
    {
        m_SelectedSetIndex = m_PickSetUI->setChoice->value();
    }
}


