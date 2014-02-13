//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// geomScreen.cpp: implementation of the geomScreen class.
//
//////////////////////////////////////////////////////////////////////

#include "ParmLinkScreen.h"
#include "ParmMgr.h"
#include "LinkMgr.h"
#include "ScreenMgr.h"
#include <FL/Fl_File_Chooser.H>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

ParmLinkScreen::ParmLinkScreen( ScreenMgr* mgr ) : VspScreen( mgr )
{
    ParmLinkUI* ui = parmLinkUI = new ParmLinkUI();
    VspScreen::SetFlWindow( ui->UIWindow );

    parmLinkUI->UIWindow->position( 780, 30 );

    ui->compAChoice->callback( staticScreenCB, this );
    ui->groupAChoice->callback( staticScreenCB, this );
    ui->parmAChoice->callback( staticScreenCB, this );
    ui->compBChoice->callback( staticScreenCB, this );
    ui->groupBChoice->callback( staticScreenCB, this );
    ui->parmBChoice->callback( staticScreenCB, this );

    ui->offsetButton->callback( staticScreenCB, this );
    ui->scaleButton->callback( staticScreenCB, this );
    ui->lowerLimitButton->callback( staticScreenCB, this );
    ui->upperLimitButton->callback( staticScreenCB, this );
    ui->addLinkButton->callback( staticScreenCB, this );
    ui->deleteLinkButton->callback( staticScreenCB, this );
    ui->deleteAllLinksButton->callback( staticScreenCB, this );
    ui->linkBrowser->callback( staticScreenCB, this );

    ui->linkAllCompButton->callback( staticScreenCB, this );
    ui->linkAllGroupButton->callback( staticScreenCB, this );

    m_OffsetSlider.Init( this, ui->offsetSlider, ui->offsetInput, 100, " %7.3f" );
    m_ScaleSlider.Init( this, ui->scaleSlider, ui->scaleInput, 1.0, " %7.5f" );
    m_LowerLimitSlider.Init( this, ui->lowerLimitSlider, ui->lowerLimitInput, 10.0, " %7.1f" );
    m_UpperLimitSlider.Init( this, ui->upperLimitSlider, ui->upperLimitInput, 10.0, " %7.1f" );

    //==== User Parms ====//
    m_User1Group = new GroupLayout( this, ui->userParmGroup1 );
    m_User2Group = new GroupLayout( this, ui->userParmGroup2 );

    m_User1Group->AddSlider( m_UserSlider[0], "User_0", 1, "%7.3f" );
    m_User1Group->AddSlider( m_UserSlider[1], "User_1", 1, "%7.3f" );
    m_User1Group->AddSlider( m_UserSlider[2], "User_2", 1, "%7.3f" );
    m_User1Group->AddSlider( m_UserSlider[3], "User_3", 1, "%7.3f" );
    m_User1Group->AddSlider( m_UserSlider[4], "User_4", 1, "%7.3f" );

    m_User2Group->AddSlider( m_UserSlider[5], "User_5", 1, "%7.3f" );
    m_User2Group->AddSlider( m_UserSlider[6], "User_6", 1, "%7.3f" );
    m_User2Group->AddSlider( m_UserSlider[7], "User_7", 1, "%7.3f" );
    m_User2Group->AddSlider( m_UserSlider[8], "User_8", 1, "%7.3f" );
    m_User2Group->AddSlider( m_UserSlider[9], "User_9", 1, "%7.3f" );

}

ParmLinkScreen::~ParmLinkScreen()
{

}

void ParmLinkScreen::Show()
{
    //Show( aircraftPtr->getUserGeom() );
    Update();
    parmLinkUI->UIWindow->show();
}

//void ParmLinkScreen::Show( Geom* geomPtr )
//{
//  //==== Check For Duplicate Comp Names ====//
//
//
//
//  UserGeom* currGeom = (UserGeom*)geomPtr;
//
//  m_User1Slider->set_parm_ptr( &currGeom->userParm1 );
//  m_User1Input->set_parm_ptr(  &currGeom->userParm1 );
//  m_User2Slider->set_parm_ptr( &currGeom->userParm2 );
//  m_User2Input->set_parm_ptr(  &currGeom->userParm2 );
//  m_User3Slider->set_parm_ptr( &currGeom->userParm3 );
//  m_User3Input->set_parm_ptr(  &currGeom->userParm3 );
//  m_User4Slider->set_parm_ptr( &currGeom->userParm4 );
//  m_User4Input->set_parm_ptr(  &currGeom->userParm4 );
//  m_User5Slider->set_parm_ptr( &currGeom->userParm5 );
//  m_User5Input->set_parm_ptr(  &currGeom->userParm5 );
//  m_User6Slider->set_parm_ptr( &currGeom->userParm6 );
//  m_User6Input->set_parm_ptr(  &currGeom->userParm6 );
//  m_User7Slider->set_parm_ptr( &currGeom->userParm7 );
//  m_User7Input->set_parm_ptr(  &currGeom->userParm7 );
//  m_User8Slider->set_parm_ptr( &currGeom->userParm8 );
//  m_User8Input->set_parm_ptr(  &currGeom->userParm8 );
//
//  m_User1Button->set_parm_ptr( &currGeom->userParm1 );
//  m_User2Button->set_parm_ptr( &currGeom->userParm2 );
//  m_User3Button->set_parm_ptr( &currGeom->userParm3 );
//  m_User4Button->set_parm_ptr( &currGeom->userParm4 );
//  m_User5Button->set_parm_ptr( &currGeom->userParm5 );
//  m_User6Button->set_parm_ptr( &currGeom->userParm6 );
//  m_User7Button->set_parm_ptr( &currGeom->userParm7 );
//  m_User8Button->set_parm_ptr( &currGeom->userParm8 );
//
//  parmMgrPtr->LoadAllParms();
//  update();
//  parmLinkUI->UIWindow->show();
//}

void ParmLinkScreen::Hide()
{
    parmLinkUI->UIWindow->hide();
}

//void ParmLinkScreen::RemoveAllRefs( GeomBase* gPtr )
//{
//  vector< ParmButton* > tempVec;
//
//  for ( int i = 0 ; i < (int)m_ParmButtonVec.size() ; i++ )
//  {
//      Parm* p =  m_ParmButtonVec[i]->get_parm_ptr();
//      if ( p && p->get_geom_base() != gPtr )
//          tempVec.push_back( m_ParmButtonVec[i] );
//  }
//  m_ParmButtonVec = tempVec;
//}

bool ParmLinkScreen::Update()
{
    int i;
    char str[256];

    LinkMgr.CheckLinks();
    LinkMgr.BuildLinkableParmData();
    Link* currLink = LinkMgr.GetCurrLink();

    //==== Geom Names A ====//
    parmLinkUI->compAChoice->clear();
    vector< string > containVecA;
    int indA = LinkMgr.GetCurrContainerVec( currLink->GetParmA(), containVecA );
    for ( i = 0 ; i < ( int )containVecA.size() ; i++ )
    {
        sprintf( str, "%d-%s", i,  containVecA[i].c_str() );
        parmLinkUI->compAChoice->add( str );
    }
    parmLinkUI->compAChoice->value( indA );

    //==== Group Names A ====//
    parmLinkUI->groupAChoice->clear();
    vector< string > groupNameVecA;
    indA = LinkMgr.GetCurrGroupNameVec( currLink->GetParmA(), groupNameVecA );
    for ( i = 0 ; i < ( int )groupNameVecA.size() ; i++ )
    {
        parmLinkUI->groupAChoice->add( groupNameVecA[i].c_str() );
    }
    parmLinkUI->groupAChoice->value( indA );

    //==== Parm Names A =====//
    parmLinkUI->parmAChoice->clear();
    vector< string > parmIDVecA;
    indA = LinkMgr.GetCurrParmIDVec( currLink->GetParmA(), parmIDVecA );
    vector< string > parmNameVecA = FindParmNames( true, parmIDVecA );
    for ( i = 0 ; i < ( int )parmNameVecA.size() ; i++ )
    {
        parmLinkUI->parmAChoice->add( parmNameVecA[i].c_str() );
    }
    parmLinkUI->parmAChoice->value( indA );

    //==== Geom Names B ====//
    parmLinkUI->compBChoice->clear();
    vector< string > containVecB;
    int indB = LinkMgr.GetCurrContainerVec( currLink->GetParmB(), containVecB );
    for ( i = 0 ; i < ( int )containVecB.size() ; i++ )
    {
        sprintf( str, "%d-%s", i,  containVecB[i].c_str() );
        parmLinkUI->compBChoice->add( str );
    }
    parmLinkUI->compBChoice->value( indB );

    //==== Group Names B ====//
    parmLinkUI->groupBChoice->clear();
    vector< string > groupNameVecB;
    indB = LinkMgr.GetCurrGroupNameVec( currLink->GetParmB(), groupNameVecB );
    for ( i = 0 ; i < ( int )groupNameVecB.size() ; i++ )
    {
        parmLinkUI->groupBChoice->add( groupNameVecB[i].c_str() );
    }
    parmLinkUI->groupBChoice->value( indB );

    //==== Parm Names B =====//
    parmLinkUI->parmBChoice->clear();
    vector< string > parmIDVecB;
    indB = LinkMgr.GetCurrParmIDVec( currLink->GetParmB(), parmIDVecB );
    vector< string > parmNameVecB = FindParmNames( false, parmIDVecB );
    for ( i = 0 ; i < ( int )parmNameVecB.size() ; i++ )
    {
        parmLinkUI->parmBChoice->add( parmNameVecB[i].c_str() );
    }
    parmLinkUI->parmBChoice->value( indB );

    //===== Update Offset ====//
    m_OffsetSlider.Update( currLink->m_Offset.GetID() );
    if ( currLink->GetOffsetFlag() )
    {
        parmLinkUI->offsetButton->value( 1 );
        m_OffsetSlider.Activate();
    }
    else
    {
        parmLinkUI->offsetButton->value( 0 );
        m_OffsetSlider.Deactivate();
    }

    //===== Update Scale ====//
    m_ScaleSlider.Update( currLink->m_Scale.GetID() );
    if ( currLink->GetScaleFlag() )
    {
        parmLinkUI->scaleButton->value( 1 );
        m_ScaleSlider.Activate();
    }
    else
    {
        parmLinkUI->scaleButton->value( 0 );
        m_ScaleSlider.Deactivate();
    }


    //===== Update Lower Limit ====//
    m_LowerLimitSlider.Update ( currLink->m_LowerLimit.GetID() );
    if ( currLink->GetLowerLimitFlag() )
    {
        parmLinkUI->lowerLimitButton->value( 1 );
        m_LowerLimitSlider.Activate();
    }
    else
    {
        parmLinkUI->lowerLimitButton->value( 0 );
        m_LowerLimitSlider.Deactivate();
    }

    //===== Update Upper Limit ====//
    m_UpperLimitSlider.Update( currLink->m_UpperLimit.GetID() );
    if ( currLink->GetUpperLimitFlag() )
    {
        parmLinkUI->upperLimitButton->value( 1 );
        m_UpperLimitSlider.Activate();
    }
    else
    {
        parmLinkUI->upperLimitButton->value( 0 );
        m_UpperLimitSlider.Deactivate();
    }

    //==== Update Link Browser ====//
    parmLinkUI->linkBrowser->clear();

    static int widths[] = { 75, 75, 90, 20, 75, 75, 80, 0 }; // widths for each column
    parmLinkUI->linkBrowser->column_widths( widths );   // assign array to widget
    parmLinkUI->linkBrowser->column_char( ':' );        // use : as the column character

    sprintf( str, "@b@.COMP_A:@b@.GROUP:@b@.PARM:->:@b@.COMP_B:@b@.GROUP:@b@.PARM" );
    parmLinkUI->linkBrowser->add( str );

    int num_links = LinkMgr.GetNumLinks();
    for ( i = 0 ; i < num_links ; i++ )
    {
        Link* pl = LinkMgr.GetLink( i );

        string c_name_A, g_name_A, p_name_A;
        string c_name_B, g_name_B, p_name_B;
        ParmMgr.GetNames( pl->GetParmA(), c_name_A, g_name_A, p_name_A );
        ParmMgr.GetNames( pl->GetParmB(), c_name_B, g_name_B, p_name_B );

        sprintf( str, "%s:%s:%s:->:%s:%s:%s",
                 c_name_A.c_str(), g_name_A.c_str(), p_name_A.c_str(),
                 c_name_B.c_str(), g_name_B.c_str(), p_name_B.c_str() );

        parmLinkUI->linkBrowser->add( str );
    }

    int index = LinkMgr.GetCurrLinkIndex();
    if ( index >= 0 && index < num_links )
    {
        parmLinkUI->linkBrowser->select( index + 2 );
    }

    for ( int i = 0 ; i < NUM_USER_SLIDERS ; i++ )
    {
        m_UserSlider[i].Update( LinkMgr.m_UserParms.GetUserParmId( i ) );
    }
//
//  parmLinkUI->UIWindow->redraw();
//
//  for ( int i = 0 ; i < (int)m_ParmButtonVec.size() ; i++ )
//  {
//      m_ParmButtonVec[i]->update();
//  }
    return false;

}

//void ParmLinkScreen::ClearButtonParms()
//{
//  for ( int i = 0 ; i < (int)m_ParmButtonVec.size() ; i++ )
//  {
//      m_ParmButtonVec[i]->set_parm_ptr( 0 );
//  }
//
//}



//void ParmLinkScreen::SetTitle( const char* name )
//{
//  //title = "PARMLINK : ";
//  //title.concatenate( name );
//
//  //parmLinkUI->TitleBox->label( title );
//}

////==== Close Callbacks =====//
//void ParmLinkScreen::CloseCB( Fl_Widget* w)
//{
//  parmLinkUI->UIWindow->hide();
//}


void ParmLinkScreen::CallBack( Fl_Widget* w )
{
    Link* currLink = LinkMgr.GetCurrLink();

    if ( w == parmLinkUI->compAChoice  || w == parmLinkUI->compBChoice ||
            w == parmLinkUI->groupAChoice || w == parmLinkUI->groupBChoice ||
            w == parmLinkUI->parmAChoice  || w == parmLinkUI->parmBChoice )
    {
        CompGroupLinkChange();
    }
    else if (  w == parmLinkUI->offsetButton )
    {
        if ( parmLinkUI->offsetButton->value() )
        {
            currLink->SetOffsetFlag( true );
        }
        else
        {
            currLink->SetOffsetFlag( false );
        }

        m_OffsetSlider.Activate();
        if ( !currLink->GetOffsetFlag() )
        {
            m_OffsetSlider.Deactivate();
        }

        LinkMgr.ParmChanged( currLink->GetParmA(), true );
    }
    else if (  w == parmLinkUI->scaleButton )
    {
        if ( parmLinkUI->scaleButton->value() )
        {
            currLink->SetScaleFlag( true );
        }
        else
        {
            currLink->SetScaleFlag( false );
        }

        m_ScaleSlider.Activate();
        if ( !currLink->GetScaleFlag() )
        {
            m_ScaleSlider.Deactivate();
        }
        LinkMgr.ParmChanged( currLink->GetParmA(), true );
    }
    else if (  w == parmLinkUI->lowerLimitButton )
    {
        if ( parmLinkUI->lowerLimitButton->value() )
        {
            currLink->SetLowerLimitFlag( true );
        }
        else
        {
            currLink->SetLowerLimitFlag( false );
        }

        m_LowerLimitSlider.Activate();
        if ( !currLink->GetLowerLimitFlag() )
        {
            m_LowerLimitSlider.Deactivate();
        }
        LinkMgr.ParmChanged( currLink->GetParmA(), true );
    }
    else if (  w == parmLinkUI->upperLimitButton )
    {
        if ( parmLinkUI->upperLimitButton->value() )
        {
            currLink->SetUpperLimitFlag( true );
        }
        else
        {
            currLink->SetUpperLimitFlag( false );
        }

        m_UpperLimitSlider.Activate();
        if ( !currLink->GetUpperLimitFlag() )
        {
            m_UpperLimitSlider.Deactivate();
        }
        LinkMgr.ParmChanged( currLink->GetParmA(), true );
    }
    else if (  w == parmLinkUI->addLinkButton )
    {
        bool success = LinkMgr.AddCurrLink();
        if ( !success )
        {
            fl_alert( "Error: Identical Parms or Already Linked" );
        }
        Update();
    }
    else if (  w == parmLinkUI->deleteLinkButton )
    {
        LinkMgr.DelCurrLink();
        Update();
    }
    else if (  w == parmLinkUI->deleteAllLinksButton )
    {
        LinkMgr.DelAllLinks();
        Update();
    }
    else if (  w == parmLinkUI->linkAllCompButton )
    {
        bool success = LinkMgr.LinkAllComp();
        if ( !success )
        {
            fl_alert( "Error: Identical Comps" );
        }
        Update();
    }
    else if (  w == parmLinkUI->linkAllGroupButton )
    {
        bool success = LinkMgr.LinkAllGroup();
        if ( !success )
        {
            fl_alert( "Error: Identical Group" );
        }
        Update();
    }
    else if (  w == parmLinkUI->linkBrowser )
    {
        int sel = parmLinkUI->linkBrowser->value();
        LinkMgr.SetCurrLinkIndex( sel - 2 );
        Update();
    }
    //else if ( m_OffsetSlider->GuiChanged( w ) )
    //{
    //  currLink->SetOffset( m_OffsetSlider->GetVal() );
    //  parmLinkMgrPtr->ParmChanged( currLink->GetParmA(), true );
    //}
    //else if ( m_ScaleSlider->GuiChanged( w ) )
    //{
    //  currLink->SetScale( m_ScaleSlider->GetVal() );
    //  parmLinkMgrPtr->ParmChanged( currLink->GetParmA(), true );
    //}
    //else if ( m_LowerLimitSlider->GuiChanged( w ) )
    //{
    //  currLink->SetLowerLimit( m_LowerLimitSlider->GetVal() );
    //  parmLinkMgrPtr->ParmChanged( currLink->GetParmA(), true );
    //}
    //else if ( m_UpperLimitSlider->GuiChanged( w ) )
    //{
    //  currLink->SetUpperLimit( m_UpperLimitSlider->GetVal() );
    //  parmLinkMgrPtr->ParmChanged( currLink->GetParmA(), true );
    //}

    //aircraftPtr->triggerDraw();
    m_ScreenMgr->SetUpdateFlag( true );
//  m_ScreenMgr->UpdateAllScreens();
}



void ParmLinkScreen::CompGroupLinkChange()
{
    ParmLinkUI* ui = parmLinkUI;
    LinkMgr.SetCurrLinkIndex( -1 );

    LinkMgr.SetParm( true, ui->compAChoice->value(), ui->groupAChoice->value(), ui->parmAChoice->value() );
    LinkMgr.SetParm( false, ui->compBChoice->value(), ui->groupBChoice->value(), ui->parmBChoice->value() );

    Update();

    //parmLinkMgrPtr->SetParm( true,
    //  ui->compAChoice->value(), ui->groupAChoice->value(), ui->parmAChoice->value() );
    //parmLinkMgrPtr->SetParm( false,
    //  ui->compBChoice->value(), ui->groupBChoice->value(), ui->parmBChoice->value() );
    //Update();
}


vector< string > ParmLinkScreen::FindParmNames( bool A_flag, vector< string > & parm_id_vec )
{
    vector< string > name_vec;
    for ( int i = 0 ; i < ( int )parm_id_vec.size() ; i++ )
    {
        string name;

        Parm* p = ParmMgr.FindParm( parm_id_vec[i] );
        if ( p )
        {
            name = p->GetName();
        }

        name_vec.push_back( name );
    }


    return name_vec;
}
