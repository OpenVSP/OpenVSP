//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
// PickSetScreen.cpp
//
//////////////////////////////////////////////////////////////////////

#include "PickSetScreen.h"
#include "ScreenMgr.h"

//==== Constructor ====//
PickSetScreen::PickSetScreen( ScreenMgr* mgr ) : BasicScreen( mgr, 150, 120, "Pick Set" )
{
    // Init member variables
    m_SelectedSetIndex = DEFAULT_SET;
    m_AcceptFlag = false;

    // Identify variables for spacing group layouts
    const int borderPaddingWidth = 5;
    int yPadding = 7;

    m_FLTK_Window->callback( staticCloseCB, this );

    //This helps init m_MainLayouts group and screen functionality
    m_MainLayout.SetGroupAndScreen( m_FLTK_Window, this );

    //Adds padding on left and top of position
    m_MainLayout.ForceNewLine();
    m_MainLayout.AddY( yPadding );
    m_MainLayout.AddX( borderPaddingWidth );

    m_MainLayout.AddSubGroupLayout( m_BorderLayout, m_MainLayout.GetRemainX() - borderPaddingWidth, m_MainLayout.GetRemainY() - borderPaddingWidth );

    m_BorderLayout.AddDividerBox( "Sets" );
    m_BorderLayout.SetChoiceButtonWidth( 0 );

    m_BorderLayout.AddChoice( m_PickSetChoice, "" );
    m_BorderLayout.AddYGap();

    m_BorderLayout.AddButton( m_AcceptButton, "OK" );
    m_BorderLayout.AddYGap();

    m_BorderLayout.AddButton( m_CancelButton, "Cancel" );

}

int PickSetScreen::PickSet( const string & title )
{
    m_AcceptFlag = false;

    SetTitle( title.c_str() );

    Show();

    while( IsShown() )
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
    m_FLTK_Window->show();
}

//==== Hide Screen ====//
void PickSetScreen::Hide()
{
    m_FLTK_Window->hide();
}

//==== Load Type Choice ====//
void PickSetScreen::LoadSetChoice()
{
    m_PickSetChoice.ClearItems();

    Vehicle* veh = VehicleMgr.GetVehicle();
    vector< string > set_name_vec = veh->GetSetNameVec();

    for ( int i = 0 ; i < ( int )set_name_vec.size() ; i++ )
    {
        m_PickSetChoice.AddItem( set_name_vec[i].c_str() );
    }

    m_PickSetChoice.UpdateItems();
    m_PickSetChoice.SetVal( m_SelectedSetIndex );

}

//==== Callbacks ====//
void PickSetScreen::GuiDeviceCallBack( GuiDevice* device )
{
    assert( m_ScreenMgr );

    if ( device == &m_AcceptButton )
    {
        m_AcceptFlag = true;
        Hide();
    }
    else if ( device == &m_CancelButton )
    {
        m_AcceptFlag = false;
        Hide();
    }
    else if ( device == &m_PickSetChoice )
    {
        m_SelectedSetIndex = m_PickSetChoice.GetVal();
    }

    // Tell m_ScreenMgr to update all screens
    m_ScreenMgr->SetUpdateFlag( true );
}
