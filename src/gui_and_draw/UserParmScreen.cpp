//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#include "UserParmScreen.h"
#include "ScreenMgr.h"
#include "CustomGeom.h"
#include "LinkMgr.h"
#include "ParmMgr.h"

#include <assert.h>

//==== Constructor ====//
UserParmScreen::UserParmScreen( ScreenMgr* mgr ) : VspScreen( mgr )
{
    UserParmUI* ui = m_UserParmUI = new UserParmUI();
    VspScreen::SetFlWindow( ui->UIWindow );

    m_NumEditSliders = 10;
    m_NameText = "Default_Name";
    m_GroupText = "Default_Group";
    m_DescText = "Default_Desc";
    m_Val.Init( "Val", "", NULL, 0.0, -1.0e12, 1.0e12, false );
    m_Min.Init( "Min", "", NULL, -1.0e5, -1.0e12, 1.0e12, false );
    m_Max.Init( "Max", "", NULL,  1.0e5, -1.0e12, 1.0e12, false );
 
    m_BigGroup.SetGroupAndScreen( ui->windowGroup, this );

    m_BigGroup.AddDividerBox( "Predefined" );

    int num_sliders = LinkMgr.GetNumPredefinedUserParms();
    m_PredefSliderVec.resize( num_sliders );

    for ( int i = 0 ; i < num_sliders ; i++ )
    {
        m_BigGroup.AddSlider( m_PredefSliderVec[i], "AUTO_UPDATE", 10, "%6.5f" );
    }

    m_BigGroup.AddYGap();
    m_BigGroup.AddDividerBox( "Create Parm" );
    m_ParmTypeChoice.AddItem( "    Double" );
    m_ParmTypeChoice.AddItem( "    Int" );
    m_BigGroup.AddChoice( m_ParmTypeChoice, "Type:" );
    m_BigGroup.AddInput( m_ParmNameInput, "Name:" );
    m_BigGroup.AddInput( m_ParmGroupInput, "Group:" );
    m_BigGroup.AddInput( m_ParmDescInput, "Desc:" );
    m_BigGroup.SetFitWidthFlag( false );
    m_BigGroup.SetSameLineFlag( true );
    m_BigGroup.AddInput( m_ParmMinInput, "Min:", "%6.0f" );
    m_BigGroup.AddX( 6 );
    m_BigGroup.AddInput( m_ParmValueInput, "Value:", "%6.5f" );
    m_BigGroup.AddX( 6 );
    m_BigGroup.AddInput( m_ParmMaxInput, "Max:", "%6.0f" );
    m_BigGroup.ForceNewLine();
    m_BigGroup.SetFitWidthFlag( true );
    m_BigGroup.SetSameLineFlag( false );


    m_BigGroup.AddYGap();
    m_BigGroup.AddButton( m_CreateParm, "Create" );

    m_BigGroup.AddYGap();
    m_BigGroup.AddDividerBox( "Edit Parms" );

    m_EditLayout.SetGroupAndScreen( ui->editGroup, this );
    m_EditSliderVec.resize( m_NumEditSliders );

    m_EditLayout.SetButtonWidth( 100 );
    for ( int i = 0 ; i < (int)m_EditSliderVec.size() ; i++ )
    {
        m_EditLayout.AddSlider( m_EditSliderVec[i], "", 10, "%6.5f" );
    }
 
    ui->parmBrowser->callback( staticScreenCB, this );

}

//==== Update Screen ====//
bool UserParmScreen::Update()
{
    m_ParmNameInput.Update( m_NameText );
    m_ParmGroupInput.Update( m_GroupText );
    m_ParmDescInput.Update( m_DescText );
    m_ParmValueInput.Update( m_Val.GetID() );
    m_ParmMinInput.Update( m_Min.GetID() );
    m_ParmMaxInput.Update( m_Max.GetID() );


    //===== Predefined Parms ====//
    for ( int i = 0 ; i < (int)m_PredefSliderVec.size() ; i++ )
    {
        m_PredefSliderVec[i].Update( LinkMgr.GetUserParmId( i ) );
    }

    //==== Load Current Parms ====//
    m_UserParmBrowserVec.clear();
    int num_user_parms = LinkMgr.GetNumUserParms();

    for ( int i = 0 ; i < num_user_parms ; i++ )
    {
        string pid = LinkMgr.GetUserParmId( i );
        if ( pid.size() )
            m_UserParmBrowserVec.push_back( pid );
    }

    //==== Load User Parms Into Browser ====//
    m_UserParmUI->parmBrowser->clear();
    for ( int i = 0 ; i < (int)m_UserParmBrowserVec.size() ; i++ )
    {
        Parm* p = ParmMgr.FindParm( m_UserParmBrowserVec[i] );
        string pname = p->GetName();
        m_UserParmUI->parmBrowser->add( pname.c_str() );
    }

    //==== Update Sliders ====//
    for ( int i = 0 ; i < (int)m_EditSliderVec.size() ; i++ )
    {
        if ( i < (int)m_EditParmVec.size() )
        {
            m_EditSliderVec[i].Update( m_EditParmVec[i] );
        }
    }
 

    return true;
}

//==== Show Screen ====//
void UserParmScreen::Show()
{
    Update();
    m_FLTK_Window->show();
}


//==== Hide Screen ====//
void UserParmScreen::Hide()
{
    m_FLTK_Window->hide();
}

//==== Callbacks ====//
void UserParmScreen::CallBack( Fl_Widget *w )
{
    if ( w == m_UserParmUI->parmBrowser )
    {
        int index = m_UserParmUI->parmBrowser->value() - 1;
        if ( index >= 0 && index < (int)m_UserParmBrowserVec.size() )
        {
            m_EditParmVec.push_front( m_UserParmBrowserVec[index] );
        }
        m_ScreenMgr->SetUpdateFlag( true );
    }

}


void UserParmScreen::GuiDeviceCallBack( GuiDevice* gui_device )
{
    if ( gui_device == &m_CreateParm )
    {
        int type = PARM_DOUBLE_TYPE;
        if ( m_ParmTypeChoice.GetVal() == 1 )
            type = PARM_INT_TYPE;

        m_NameText = m_ParmNameInput.GetString();
        m_GroupText = m_ParmGroupInput.GetString();
        m_DescText = m_ParmDescInput.GetString();

        string pid = LinkMgr.AddUserParm( type, m_NameText, m_GroupText );
        Parm* pptr = ParmMgr.FindParm( pid );
        if ( pptr )
        {
            pptr->SetDescript( m_DescText );
            pptr->SetLowerUpperLimits( m_Min(), m_Max() );
            pptr->Set( m_Val() );
        }
        m_ScreenMgr->SetUpdateFlag( true );

    }
    else if ( gui_device == &m_ParmNameInput )
    {
        m_NameText = m_ParmNameInput.GetString();
    }

}
