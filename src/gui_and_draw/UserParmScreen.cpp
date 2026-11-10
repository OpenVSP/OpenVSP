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
    ((VSP_Window*)m_FLTK_Window)->SetResizeCallback( staticResizeCB, this );

    //==== Variables =====//
    m_NumParmsLast = 0;
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
    m_PredefGroup.SetButtonWidth( 150 );
    m_PredefGroup.SetInputWidth( 100 );

    unsigned int num_sliders = LinkMgr.GetNumPredefinedUserParms();
    m_PredefSliderVec.resize( num_sliders );

    for ( int i = 0 ; i < num_sliders ; i++ )
    {
        m_PredefGroup.AddSlider( m_PredefSliderVec[i], "AUTO_UPDATE", 10, " % #.5g" );
    }
    m_PredefGroup.AddYGap();

    m_PredefGroup.AddSubGroupLayout( m_PredefResizeGroup,
        m_PredefGroup.GetW(),
        m_PredefGroup.GetRemainY() );

    //===== Create ====//
    m_CreateGroup.SetGroupAndScreen( create_group, this );

    m_CreateGroup.AddDividerBox( "Create Parm" );
    m_ParmTypeChoice.AddItem( "    Double" );
    m_ParmTypeChoice.AddItem( "    Int" );
    m_CreateGroup.AddChoice( m_ParmTypeChoice, "Type:" );
    m_CreateGroup.AddYGap();
    m_CreateGroup.AddButton( m_CreateParm, "Create" );
    m_CreateGroup.AddYGap();
    m_CreateGroup.AddDividerBox( "User Defined Parms" );
    m_UserDefinedBrowser = m_CreateGroup.AddVspBrowser( 150 );
    m_CreateGroup.AddYGap();

    m_CreateGroup.SetFitWidthFlag( false );
    m_CreateGroup.SetSameLineFlag( true );

    m_CreateGroup.SetButtonWidth( 170 );
    m_CreateGroup.AddX( 95 );
    m_CreateGroup.AddButton( m_DeleteParm, "DeleteSelected" );
    m_CreateGroup.AddX( 40 );
    m_CreateGroup.AddButton( m_DeleteAllParm, "DeleteAll" );

    m_CreateGroup.SetFitWidthFlag( true );
    m_CreateGroup.SetSameLineFlag( false );
    m_CreateGroup.ForceNewLine();
    m_CreateGroup.AddYGap();

    m_CreateGroup.SetButtonWidth( 70 );

    m_CreateGroup.AddDividerBox( "Edit User Defined Parms" );
    m_CreateGroup.AddOutput( m_EditParmTypeOutput, "Type:" );
    m_CreateGroup.AddInput( m_EditParmNameInput, "Name:" );
    m_CreateGroup.AddInput( m_EditParmGroupInput, "Group:" );
    m_CreateGroup.AddInput( m_EditParmDescInput, "Desc:" );
    m_CreateGroup.AddInput( m_EditParmMinInput, "Min:" );
    m_CreateGroup.AddInput( m_EditParmMaxInput, "Max:" );

    m_UserDefinedBrowser->Init( this, m_CreateGroup.GetGroup() );
    m_UserDefinedBrowser->type( FL_MULTI_BROWSER );

    //==== Adjust ====//
    m_AdjustScroll = AddSubScroll( adj_tab, 5 );
    
    m_AdjustLayout.SetGroupAndScreen( m_AdjustScroll, this );

    predef_tab->show();

    predef_group->resizable( m_PredefGroup.GetGroup() );
    m_PredefGroup.GetGroup()->resizable( m_PredefResizeGroup.GetGroup() );

    create_group->resizable( m_CreateGroup.GetGroup() );
    m_CreateGroup.GetGroup()->resizable( m_UserDefinedBrowser );

    adj_tab->resizable( m_AdjustScroll );
}

//==== Update Screen ====//
bool UserParmScreen::Update()
{
    TabScreen::Update();

    //===== Predefined Parms ====//
    for ( int i = 0 ; i < (int)m_PredefSliderVec.size() ; i++ )
    {
        m_PredefSliderVec[i].Update( LinkMgr.GetUserParmId( i ) );
    }

    Vehicle *veh = m_ScreenMgr->GetVehiclePtr();

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

    // Re-point selection at the tracked parm, since renames can re-sort the list
    vector < string >::iterator track_it = find( m_UserParmBrowserVec.begin(), m_UserParmBrowserVec.end(), m_TrackParmID );
    if ( !m_TrackParmID.empty() && track_it != m_UserParmBrowserVec.end() )
    {
        m_UserBrowserVec = { (int)( track_it - m_UserParmBrowserVec.begin() ) };
    }

    //==== Load User Parms Into Browser ====//
    // Only rebuild when contents change; clear() resets the scroll position
    bool rebuild = m_UserDefinedBrowser->size() != (int)m_UserParmBrowserVec.size();
    for ( int i = 0 ; !rebuild && i < (int)m_UserParmBrowserVec.size() ; i++ )
    {
        Parm* p = ParmMgr.FindParm( m_UserParmBrowserVec[i] );
        rebuild = !p || p->GetName() != m_UserDefinedBrowser->text( i + 1 );
    }
    int v_pos = m_UserDefinedBrowser->vposition();
    if ( rebuild )
    {
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
    }
    for ( int i = 0; i < m_UserBrowserVec.size(); i++ )
    {
        // offset by +1 to convert to browser indexing
        m_UserDefinedBrowser->select( m_UserBrowserVec[i] + 1 );
    }
    if ( rebuild && !m_TrackParmID.empty() && !m_UserBrowserVec.empty() )
    {
        // keep the prior view, then scroll only as far as needed to show the tracked item
        m_UserDefinedBrowser->vposition( v_pos );
        m_UserDefinedBrowser->ScrollToLine( m_UserBrowserVec[0] + 1 );
    }
    m_TrackParmID.clear();

    //==== Set Inputs ====//
    Parm* user_parm_ptr = nullptr;
    int index = -1;
    if ( m_UserBrowserVec.size() == 1 )
    {
        index = m_UserBrowserVec[0];
        m_EditParmNameInput.Activate();
    }
    else
    {
        m_EditParmNameInput.Deactivate();
    }
    if ( index >= 0 && index < (int)m_UserParmBrowserVec.size() )
    {
        user_parm_ptr = ParmMgr.FindParm(  m_UserParmBrowserVec[index] );
    }
    if ( user_parm_ptr )
    {
        string ptype = "";
        switch ( user_parm_ptr->GetType() )
        {
            case vsp::PARM_INT_TYPE:
                ptype = "Int";
                break;
            case vsp::PARM_DOUBLE_TYPE:
                ptype = "Double";
                break;
        }
        m_EditParmTypeOutput.Update( ptype );
        m_EditParmNameInput.Update( user_parm_ptr->GetName() );
        m_EditParmGroupInput.Update( user_parm_ptr->GetGroupName() );
        m_EditParmDescInput.Update( user_parm_ptr->GetDescript() );

        char str[255];
        snprintf( str, sizeof( str ), " %7.5f", user_parm_ptr->GetLowerLimit() );
        m_EditParmMinInput.Update( str );
        snprintf( str, sizeof( str ), " %7.5f", user_parm_ptr->GetUpperLimit() );
        m_EditParmMaxInput.Update( str );
    }
    else
    {
        m_EditParmTypeOutput.Update( "" );
        m_EditParmNameInput.Update( "" );
        m_EditParmGroupInput.Update( "" );
        m_EditParmDescInput.Update( "" );
        m_EditParmMinInput.Update( "" );
        m_EditParmMaxInput.Update( "" );
    }

    // Activate/Disable Delete Buttons depending on selection and parm vector lengths.
    // Group, description and limits apply to every selected parm; a name only to one.
    if ( !m_UserBrowserVec.empty() )
    {
        m_DeleteParm.Activate();
        m_EditParmGroupInput.Activate();
        m_EditParmDescInput.Activate();
        m_EditParmMinInput.Activate();
        m_EditParmMaxInput.Activate();
    }
    else
    {
        m_DeleteParm.Deactivate();
        m_EditParmNameInput.Deactivate();
        m_EditParmGroupInput.Deactivate();
        m_EditParmDescInput.Deactivate();
        m_EditParmMinInput.Deactivate();
        m_EditParmMaxInput.Deactivate();
    }
    if ( !m_UserParmBrowserVec.empty() )
    {
        m_DeleteAllParm.Activate();
    }
    else
    {
        m_DeleteAllParm.Deactivate();
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
    m_AdjustLayout.SetButtonWidth( 300 );
    m_AdjustLayout.SetInputWidth( 100 );

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

        if ( p->GetType() == vsp::PARM_INT_TYPE ||
             p->GetType() == vsp::PARM_BOOL_TYPE ||
             p->GetType() == vsp::PARM_LIMITED_INT_TYPE ||
             p->GetType() == vsp::PARM_POWER_INT_TYPE )
        {
            m_AdjustLayout.AddSlider( m_ParmSliderVec[i], "AUTO_UPDATE", 10, " % -15.0f" );
        }
        else
        {
            m_AdjustLayout.AddSlider( m_ParmSliderVec[i], "AUTO_UPDATE", 10, " % #.5g" );
        }

        m_ParmSliderVec[i].Update( pID );
    }

    m_NumParmsLast = num_vars;
}

//==== Show Screen ====//
void UserParmScreen::Show()
{
    m_ScreenMgr->SetUpdateFlag( true );
    TabScreen::Show();
}


//==== Hide Screen ====//
void UserParmScreen::Hide()
{
    VspScreen::Hide();
    m_ScreenMgr->SetUpdateFlag( true );
}

//==== Callbacks ====//
void UserParmScreen::CallBack( Fl_Widget *w )
{
    m_UserDefinedBrowser->HidePopupInput();

    if ( w == m_UserDefinedBrowser )
    {
        //==== Apply popup rename to prev selection ====//
        if ( m_UserDefinedBrowser->GetCBReason() == BROWSER_CALLBACK_POPUP_ENTER && m_UserBrowserVec.size() == 1 )
        {
            Parm* p = ParmMgr.FindParm(  m_UserParmBrowserVec[ m_UserBrowserVec[0] ] );
            if ( p )
            {
                string pname = m_UserDefinedBrowser->GetPopupValue();
                p->SetName( pname.c_str() );
                m_TrackParmID = p->GetID();
                UserParmContainer* upc = LinkMgr.GetUserParmContainer();
                if ( upc && !upc->SortVars() )
                {
                    RebuildAdjustGroup();
                }
            }
        }

        //==== Load User Parm Edit Names/Desc ====//
        m_UserDefinedBrowser->GetSelected( m_UserBrowserVec );

        //==== Open up popup over new selection ====//
        if ( m_UserDefinedBrowser->GetCBReason() == BROWSER_CALLBACK_POPUP_OPEN && m_UserBrowserVec.size() == 1 )
        {
            Parm* p = ParmMgr.FindParm(  m_UserParmBrowserVec[ m_UserBrowserVec[0] ] );
            // double click open the popup input
            if ( p )
            {
                m_UserDefinedBrowser->InsertPopupInput( p->GetName(), m_UserDefinedBrowser->value() );
            }
        }
    }

    m_ScreenMgr->SetUpdateFlag( true );
}

void UserParmScreen::GuiDeviceCallBack( GuiDevice* gui_device )
{
    m_UserDefinedBrowser->HidePopupInput();

    Vehicle *veh = m_ScreenMgr->GetVehiclePtr();
    if ( gui_device == &m_CreateParm )
    {
        int type = vsp::PARM_DOUBLE_TYPE;
        string ptype = "DOUBLE_";
        if ( m_ParmTypeChoice.GetVal() == 1 )
        {
            type = vsp::PARM_INT_TYPE;
            ptype = "INT_";
        }

        string pname;
        string pid;
        int index = 0;
        while ( !pid.size() )
        {
            pname = "USERPARM_" + ptype + to_string( index );
            pid = LinkMgr.AddUserParm( type, pname, m_GroupText );
            index++;
        }

        Parm* pptr = ParmMgr.FindParm( pid );
        if ( pptr )
        {
            pptr->SetDescript( m_DescText );
            pptr->SetLowerUpperLimits( veh->m_UserParmMin(), veh->m_UserParmMax() );
            pptr->Set( veh->m_UserParmVal() );
        }
        RebuildAdjustGroup();
        m_UserBrowserVec.clear();
        m_TrackParmID = pid; // select and scroll to the new parm
     }
    else if ( gui_device == &m_DeleteParm )
    {
        int num_predef = LinkMgr.GetNumPredefinedUserParms();
        vector < int > delete_vec;
        for ( int i = 0; i < m_UserBrowserVec.size(); i++ )
        {
            delete_vec.push_back( m_UserBrowserVec[i] + num_predef );
        }
        LinkMgr.DeleteUserParm( delete_vec );
        RebuildAdjustGroup();

        int old_index = *min_element( m_UserBrowserVec.begin(), m_UserBrowserVec.end() );
        int max_index = LinkMgr.GetNumUserParms() - num_predef - 1;

        m_UserBrowserVec.clear();
        if ( max_index >= 0 )
        {
            m_UserBrowserVec.push_back( min( old_index , max_index ) );
            m_TrackParmID = LinkMgr.GetUserParmId( m_UserBrowserVec[0] + num_predef );
        }
    }
    else if ( gui_device == &m_DeleteAllParm )
    {
        LinkMgr.DeleteAllUserParm( );
        RebuildAdjustGroup();
        m_UserBrowserVec.clear();
    }
    else if ( gui_device == &m_EditParmNameInput ||
              gui_device == &m_EditParmGroupInput ||
              gui_device == &m_EditParmDescInput ||
              gui_device == &m_EditParmMinInput ||
              gui_device == &m_EditParmMaxInput )
    {
        for ( int i = 0; i < m_UserBrowserVec.size(); i++ )
        {
            int index = m_UserBrowserVec[i];
            if ( index >= 0 && index < (int)m_UserParmBrowserVec.size() )
            {
                Parm* p = ParmMgr.FindParm(  m_UserParmBrowserVec[index] );
                if ( p )
                {
                    if ( gui_device == &m_EditParmNameInput && m_UserBrowserVec.size() == 1 )
                    {
                        p->SetName( m_EditParmNameInput.GetString() );
                        m_TrackParmID = p->GetID();
                    }
                    else if ( gui_device == &m_EditParmGroupInput )
                        p->SetGroupName( m_EditParmGroupInput.GetString() );
                    else if ( gui_device == &m_EditParmDescInput )
                        p->SetDescript( m_EditParmDescInput.GetString() );
                    else if ( gui_device == &m_EditParmMinInput )
                        p->SetLowerLimit( atof( m_EditParmMinInput.GetString().c_str() ) );
                    else if ( gui_device == &m_EditParmMaxInput )
                        p->SetUpperLimit( atof( m_EditParmMaxInput.GetString().c_str() ) );
                }
                RebuildAdjustGroup();
            }
        }
        // sort now so the browser rebuilds once in final order
        UserParmContainer* upc = LinkMgr.GetUserParmContainer();
        if ( upc && !upc->SortVars() )
        {
            RebuildAdjustGroup();
        }
    }
    else
    {
        return;
    }

    m_ScreenMgr->SetUpdateFlag( true );
}

void UserParmScreen::ResizeCallBack( Fl_Widget *w )
{
    RebuildAdjustGroup();
    m_ScreenMgr->SetUpdateFlag( true );
}
