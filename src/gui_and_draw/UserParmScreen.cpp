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
UserParmScreen::UserParmScreen( ScreenMgr* mgr ) : TabScreen( mgr, 400, 500, "User Parms" )
{
    //==== Variables =====//
    m_NumParmsLast = 0;
    m_NumEditSliders = 10;
    m_NameText = "Default_Name";
    m_GroupText = "Default_Group";
    m_DescText = "Default_Desc";
    m_Val.Init( "Val", "", NULL, 0.0, -1.0e12, 1.0e12, false );
    m_Min.Init( "Min", "", NULL, -1.0e5, -1.0e12, 1.0e12, false );
    m_Max.Init( "Max", "", NULL,  1.0e5, -1.0e12, 1.0e12, false );
 
    //==== Tabs And Groups ====//
    Fl_Group* predef_tab = AddTab( "Predef" );
    Fl_Group* create_tab = AddTab( "Create" );
    Fl_Group* adj_tab = AddTab( "Adjust" );

    Fl_Group* predef_group = AddSubGroup( predef_tab, 5 );
    Fl_Group* create_group = AddSubGroup( create_tab, 5 );
 
    //===== Predefined ====//
    m_PredefGroup.SetGroupAndScreen( predef_group, this );
    m_PredefGroup.AddDividerBox( "Predefined" );
    int num_sliders = LinkMgr.GetNumPredefinedUserParms();
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
    m_UserDefinedBrowser = m_CreateGroup.AddFlBrowser( 210 );
    m_CreateGroup.AddYGap();

    m_CreateGroup.SetFitWidthFlag( false );
    m_CreateGroup.SetSameLineFlag( true );

    m_CreateGroup.SetButtonWidth( 160 );
    m_CreateGroup.AddX( 20 );
    m_CreateGroup.AddButton( m_DeleteParm, "Delete" );
    m_CreateGroup.AddX( 20 );
    m_CreateGroup.AddButton( m_DeleteAllParm, "DeleteAll" );

    //==== Adjust ====//
    m_AdjustScroll = AddSubScroll( adj_tab, 5 );
    m_AdjustScroll->type( Fl_Scroll::VERTICAL_ALWAYS );
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

    m_ParmNameInput.Update( m_NameText );
    m_ParmGroupInput.Update( m_GroupText );
    m_ParmDescInput.Update( m_DescText );
    m_ParmValueInput.Update( m_Val.GetID() );
    m_ParmMinInput.Update( m_Min.GetID() );
    m_ParmMaxInput.Update( m_Max.GetID() );

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
        string pname = p->GetName();
        m_UserDefinedBrowser->add( pname.c_str() );
    }

    // Parameter GUI got out of sync.  Probably from File->New or similar.
    if ( m_NumParmsLast != num_parms )
    {
        RebuildAdjustGroup();
    }

    //==== Update Parm Adjust Tab ====//
    for ( int i = 0 ; i < (int)m_ParmSliderVec.size() ; i++ )
    {
        if ( i < (int)m_UserParmBrowserVec.size() )
            m_ParmSliderVec[i].Update( m_UserParmBrowserVec[i] );
    } 

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

    int num_predef_vars = LinkMgr.GetNumPredefinedUserParms();
    int num_vars =  LinkMgr.GetNumUserParms() - num_predef_vars;
    m_ParmSliderVec.resize( num_vars );

    string lastContID;

    for ( int i = 0 ; i < num_vars ; i++ )
    {
        string pID = LinkMgr.GetUserParmId( i + num_predef_vars );
        Parm* p = ParmMgr.FindParm( pID );
        string contID = p->GetContainerID();

        if ( contID.compare( lastContID ) != 0 )
        {
            lastContID = contID;
            m_AdjustLayout.AddDividerBox( ParmMgr.FindParmContainer( contID )->GetName() );
        }
        m_AdjustLayout.SetButtonWidth( 130 );
        m_AdjustLayout.AddSlider( m_ParmSliderVec[i], "AUTO_UPDATE", 10, "%7.3f" );
        m_ParmSliderVec[i].Update( pID );
    }

    m_NumParmsLast = num_vars;
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
        if ( pid.size() )
        {
            Parm* pptr = ParmMgr.FindParm( pid );
            if ( pptr )
            {
                pptr->SetDescript( m_DescText );
                pptr->SetLowerUpperLimits( m_Min(), m_Max() );
                pptr->Set( m_Val() );
            }
            RebuildAdjustGroup();
        }
        else
        {
            m_ScreenMgr->Alert( "Duplicate Name and Group" );
        }
     }
    else if ( gui_device == &m_DeleteParm )
    {
        int index = m_UserDefinedBrowser->value() - 1;
        int num_predef = LinkMgr.GetNumPredefinedUserParms();
        LinkMgr.DeleteUserParm( index +  num_predef );
        RebuildAdjustGroup();
    }
    else if ( gui_device == &m_DeleteAllParm )
    {
        LinkMgr.DeleteAllUserParm( );
        RebuildAdjustGroup();
    }
    else
    {
        return;
    }

    m_ScreenMgr->SetUpdateFlag( true );
}
