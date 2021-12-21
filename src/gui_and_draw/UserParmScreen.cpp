//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#include "UserParmScreen.h"
#include "ScreenMgr.h"
#include "LinkMgr.h"
#include "ParmMgr.h"

//==== Constructor ====//
UserParmScreen::UserParmScreen( ScreenMgr* mgr ) : TabScreen( mgr, 570, 580, "User Parms" )
{
    //==== Variables =====//
    m_NumParmsLast = 0;
    m_UserBrowserSelection = 0;
    m_NameText = "Default_Name";
    m_GroupText = "Default_Group";
    m_DescText = "Default_Desc";

    //==== Tabs And Groups ====//
    Fl_Group* predef_tab = AddTab( "Predef" );
    Fl_Group* create_tab = AddTab( "Create" );
    Fl_Group* adj_tab = AddTab( "Adjust" );

    Fl_Group* predef_group = AddSubGroup( predef_tab, 5 );
    Fl_Group* create_group = AddSubGroup( create_tab, 5 );
 
    //===== Predefined ====//
    m_PredefGroup.SetGroupAndScreen( predef_group, this );
    m_PredefGroup.AddDividerBox( "Predefined" );
    unsigned int num_sliders = LinkMgr.GetNumPredefinedUserParms();
    m_PredefSliderVec.resize( num_sliders );

    for ( int i = 0 ; i < num_sliders ; i++ )
    {
        m_PredefGroup.SetButtonWidth( 150 );
        m_PredefGroup.AddSlider( m_PredefSliderVec[i], "AUTO_UPDATE", 10, "%6.5f" );
    }

    //===== Create ====//
    m_CreateGroup.SetGroupAndScreen( create_group, this );

    m_CreateGroup.AddDividerBox( "Create Parm" );
    m_ParmTypeChoice.AddItem( "    Double" );
    m_ParmTypeChoice.AddItem( "    Int" );
    m_CreateGroup.AddChoice( m_ParmTypeChoice, "Type:" );
    m_CreateGroup.AddInput( m_ParmNameInput, "Name:" );
    m_CreateGroup.AddInput( m_ParmGroupInput, "Group:" );
    m_CreateGroup.AddInput( m_ParmDescInput, "Desc:" );
    m_CreateGroup.AddInput( m_ParmMinInput, "Min:", "%6.0f" );
    m_CreateGroup.AddInput( m_ParmValueInput, "Value:", "%6.5f" );
    m_CreateGroup.AddInput( m_ParmMaxInput, "Max:", "%6.0f" );
    m_CreateGroup.AddYGap();
    m_CreateGroup.AddButton( m_CreateParm, "Create" );
    m_CreateGroup.AddYGap();
    m_CreateGroup.AddDividerBox( "User Defined Parms" );
    m_UserDefinedBrowser = m_CreateGroup.AddFlBrowser( 150 );
    m_CreateGroup.AddYGap();

    m_CreateGroup.SetFitWidthFlag( false );
    m_CreateGroup.SetSameLineFlag( true );

    m_CreateGroup.SetButtonWidth( 170 );
    m_CreateGroup.AddX( 95 );
    m_CreateGroup.AddButton( m_DeleteParm, "Delete" );
    m_CreateGroup.AddX( 40 );
    m_CreateGroup.AddButton( m_DeleteAllParm, "DeleteAll" );

    m_CreateGroup.SetFitWidthFlag( true );
    m_CreateGroup.SetSameLineFlag( false );
    m_CreateGroup.ForceNewLine();
    m_CreateGroup.AddYGap();

    m_CreateGroup.SetButtonWidth( 70 );

    m_CreateGroup.AddDividerBox( "Edit User Defined Parms" );
    m_CreateGroup.AddInput( m_EditParmNameInput, "Name:" );
    m_CreateGroup.AddInput( m_EditParmGroupInput, "Group:" );
    m_CreateGroup.AddInput( m_EditParmDescInput, "Desc:" );
    m_CreateGroup.AddInput( m_EditParmMinInput, "Min:" );
    m_CreateGroup.AddInput( m_EditParmMaxInput, "Max:" );

    m_UserDefinedBrowser->callback( staticScreenCB, this );

    //==== Adjust ====//
    m_AdjustScroll = AddSubScroll( adj_tab, 5 );
    
    m_AdjustLayout.SetGroupAndScreen( m_AdjustScroll, this );

    predef_tab->show();

}

//==== Update Screen ====//
bool UserParmScreen::Update()
{
    //===== Predefined Parms ====//
    for ( int i = 0 ; i < (int)m_PredefSliderVec.size() ; i++ )
    {
        m_PredefSliderVec[i].Update( LinkMgr.GetUserParmId( i ) );
    }

    Vehicle *veh = m_ScreenMgr->GetVehiclePtr();

    m_ParmNameInput.Update( m_NameText );
    m_ParmGroupInput.Update( m_GroupText );
    m_ParmDescInput.Update( m_DescText );
    m_ParmValueInput.Update( veh->m_UserParmVal.GetID() );
    m_ParmMinInput.Update( veh->m_UserParmMin.GetID() );
    m_ParmMaxInput.Update( veh->m_UserParmMax.GetID() );

    //==== Load User Created Parms ====//
    int num_predef_parms = LinkMgr.GetNumPredefinedUserParms();
    int num_parms =  LinkMgr.GetNumUserParms() - num_predef_parms;
    m_UserParmBrowserVec.clear();
    for ( int i = 0 ; i < num_parms ; i++ )
    {
        string pid = LinkMgr.GetUserParmId( i +  num_predef_parms );
        if ( pid.size() )
            m_UserParmBrowserVec.push_back( pid );
    }

    //==== Load User Parms Into Browser ====//
    m_UserDefinedBrowser->clear();
    for ( int i = 0 ; i < (int)m_UserParmBrowserVec.size() ; i++ )
    {
        Parm* p = ParmMgr.FindParm( m_UserParmBrowserVec[i] );
        if ( p )
        {
            string pname = p->GetName();
            m_UserDefinedBrowser->add( pname.c_str() );
        }
    }
    m_UserDefinedBrowser->select( m_UserBrowserSelection );

    //==== Set Inputs ====//
    Parm* user_parm_ptr = NULL;
    int index = m_UserBrowserSelection - 1;
    if ( index >= 0 && index < (int)m_UserParmBrowserVec.size() )
    {
        user_parm_ptr = ParmMgr.FindParm(  m_UserParmBrowserVec[index] );
    }
    if ( user_parm_ptr )
    {
        m_EditParmNameInput.Update( user_parm_ptr->GetName() );
        m_EditParmGroupInput.Update( user_parm_ptr->GetGroupName() );
        m_EditParmDescInput.Update( user_parm_ptr->GetDescript() );

        char str[255];
        sprintf( str, " %7.5f", user_parm_ptr->GetLowerLimit() );
        m_EditParmMinInput.Update( str );
        sprintf( str, " %7.5f", user_parm_ptr->GetUpperLimit() );
        m_EditParmMaxInput.Update( str );
    }
    else
    {
        m_EditParmNameInput.Update( "" );
        m_EditParmGroupInput.Update( "" );
        m_EditParmDescInput.Update( "" );
        m_EditParmMinInput.Update( "" );
        m_EditParmMaxInput.Update( "" );
    }

    // Parameter GUI got out of sync.  Probably from File->New or similar.
    UserParmContainer * upc = LinkMgr.GetUserParmContainer();
    if ( upc )
    {
        if ( !upc->SortVars() || m_NumParmsLast != num_parms )
        {
            RebuildAdjustGroup();
        }
    }


    //==== Update Parm Adjust Tab ====//
    for ( int i = 0 ; i < (int)m_ParmSliderVec.size() ; i++ )
    {
        if ( i < (int)m_UserParmBrowserVec.size() )
            m_ParmSliderVec[i].Update( m_UserParmBrowserVec[i] );
    }

    m_FLTK_Window->redraw();
    return true;
}

void UserParmScreen::RebuildAdjustGroup()
{
    if ( !m_AdjustScroll )
        return;

    m_AdjustScroll->clear();
    m_AdjustLayout.SetGroup( m_AdjustScroll );
    m_AdjustLayout.InitWidthHeightVals();

    m_ParmSliderVec.clear();

    unsigned int num_predef_vars = LinkMgr.GetNumPredefinedUserParms();
    unsigned int num_vars =  LinkMgr.GetNumUserParms() - num_predef_vars;
    m_ParmSliderVec.resize( num_vars );

    string lastGroupName;

    for ( int i = 0 ; i < num_vars ; i++ )
    {
        string pID = LinkMgr.GetUserParmId( i + num_predef_vars );
        Parm* p = ParmMgr.FindParm( pID );

        string groupName = p->GetDisplayGroupName();

        if ( groupName.compare( lastGroupName ) != 0 )
        {
            lastGroupName = groupName;
            m_AdjustLayout.AddDividerBox( groupName );
        }
        m_AdjustLayout.SetButtonWidth( 300 );
        m_AdjustLayout.AddSlider( m_ParmSliderVec[i], "AUTO_UPDATE", 10, "%7.3f" );
        m_ParmSliderVec[i].Update( pID );
    }

    m_NumParmsLast = num_vars;
}

//==== Show Screen ====//
void UserParmScreen::Show()
{
    m_ScreenMgr->SetUpdateFlag( true );
    m_FLTK_Window->show();
}


//==== Hide Screen ====//
void UserParmScreen::Hide()
{
    m_FLTK_Window->hide();
    m_ScreenMgr->SetUpdateFlag( true );
}

//==== Callbacks ====//
void UserParmScreen::CallBack( Fl_Widget *w )
{
    if ( w == m_UserDefinedBrowser )
    {
        //==== Load User Parm Edit Names/Desc ====//
        m_UserBrowserSelection = m_UserDefinedBrowser->value();
    }

    m_ScreenMgr->SetUpdateFlag( true );
}

void UserParmScreen::GuiDeviceCallBack( GuiDevice* gui_device )
{
    Vehicle *veh = m_ScreenMgr->GetVehiclePtr();
    if ( gui_device == &m_CreateParm )
    {
        int type = vsp::PARM_DOUBLE_TYPE;
        if ( m_ParmTypeChoice.GetVal() == 1 )
            type = vsp::PARM_INT_TYPE;

        string pid = LinkMgr.AddUserParm( type, m_NameText, m_GroupText );
        if ( pid.size() )
        {
            Parm* pptr = ParmMgr.FindParm( pid );
            if ( pptr )
            {
                pptr->SetDescript( m_DescText );
                pptr->SetLowerUpperLimits( veh->m_UserParmMin(), veh->m_UserParmMax() );
                pptr->Set( veh->m_UserParmVal() );
            }
            RebuildAdjustGroup();
        }
        else
        {
            m_ScreenMgr->Alert( "Duplicate Name and Group" );
        }
        m_UserBrowserSelection = -1;
     }
    else if ( gui_device == &m_DeleteParm )
    {
        int index = m_UserDefinedBrowser->value() - 1;
        int num_predef = LinkMgr.GetNumPredefinedUserParms();
        LinkMgr.DeleteUserParm( index +  num_predef );
        RebuildAdjustGroup();
        m_UserBrowserSelection = -1;
    }
    else if ( gui_device == &m_DeleteAllParm )
    {
        LinkMgr.DeleteAllUserParm( );
        RebuildAdjustGroup();
        m_UserBrowserSelection = -1;
    }
    else if ( gui_device == &m_ParmNameInput)
    {
        m_NameText = m_ParmNameInput.GetString();
    }
    else if ( gui_device == &m_ParmGroupInput)
    {
        m_GroupText = m_ParmGroupInput.GetString();
    }
    else if ( gui_device == &m_ParmDescInput)
    {
        m_DescText = m_ParmDescInput.GetString();
    }
    else if ( gui_device == &m_EditParmNameInput ||
              gui_device == &m_EditParmGroupInput ||
              gui_device == &m_EditParmDescInput ||
              gui_device == &m_EditParmMinInput ||
              gui_device == &m_EditParmMaxInput )
    {
        int index = m_UserDefinedBrowser->value() - 1;
        if ( index >= 0 && index < (int)m_UserParmBrowserVec.size() )
        {
            Parm* p = ParmMgr.FindParm(  m_UserParmBrowserVec[index] );
            if ( p )
            {
                if ( gui_device == &m_EditParmNameInput )
                    p->SetName( m_EditParmNameInput.GetString() );
                else if ( gui_device == &m_EditParmGroupInput )
                    p->SetGroupName( m_EditParmGroupInput.GetString() );
                else if ( gui_device == &m_EditParmDescInput )
                    p->SetDescript( m_EditParmDescInput.GetString() );
                else if ( gui_device == &m_EditParmMinInput )
                    p->SetLowerLimit( atof( m_EditParmMinInput.GetString().c_str() ) );
                else if ( gui_device == &m_EditParmMaxInput )
                    p->SetUpperLimit( atof( m_EditParmMaxInput.GetString().c_str() ) );

                //==== Load User Parms Into Browser ====//
                m_UserDefinedBrowser->clear();
                for ( int i = 0 ; i < (int)m_UserParmBrowserVec.size() ; i++ )
                {
                    Parm* p2 = ParmMgr.FindParm( m_UserParmBrowserVec[i] );
                    if ( p2 )
                    {
                        string pname = p2->GetName();
                        m_UserDefinedBrowser->add( pname.c_str() );
                    }
                }
            }
        }
    }
    else
    {
        return;
    }

    m_ScreenMgr->SetUpdateFlag( true );
}
