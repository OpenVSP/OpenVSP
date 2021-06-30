//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// parmScreen.cpp: implementation of the geomScreen class.
//
//////////////////////////////////////////////////////////////////////

#include "ParmScreen.h"
#include "ParmMgr.h"
#include "LinkMgr.h"
#include "AdvLinkMgr.h"

#include <FL/Fl_File_Chooser.H>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

ParmScreen::ParmScreen( ScreenMgr* mgr )  : TabScreen( mgr, 380, 250, "Parm" )
{
    //==== Tabs And Groups ====//
    Fl_Group* info_tab = AddTab( "Info" );
    Fl_Group* info_group = AddSubGroup( info_tab, 5 );
    Fl_Group* link_tab = AddTab( "Links" );
    Fl_Group* link_group = AddSubGroup( link_tab, 5 );
    Fl_Group* adv_link_tab = AddTab( "Adv Links" );
    Fl_Group* adv_link_group = AddSubGroup( adv_link_tab, 5 );

    //===== Info ====//
    m_InfoLayout.SetGroupAndScreen( info_group, this );
    m_InfoLayout.AddDividerBox( "Parm Info" );

    m_InfoLayout.AddOutput( m_NameString, "Name:" );
    m_InfoLayout.AddOutput( m_GroupString, "Group:" );
    m_InfoLayout.AddOutput( m_DescString, "Desc:" );

    m_InfoLayout.AddYGap();

    m_InfoLayout.AddOutput( m_MinValString, "Min Val:" );
    m_InfoLayout.AddOutput( m_CurrValString, "Curr Val:" );
    m_InfoLayout.AddOutput( m_MaxValString, "Max Val:" );

    //===== Link ====//
    m_LinkLayout.SetGroupAndScreen( link_group, this );
    m_LinkLayout.AddDividerBox( "Link To" );

    int width = ( m_LinkLayout.GetRemainX() - 10 ) / 3;

    // Pointer for the widths of each column in the browser to support resizing
    // Last column width must be 0
    static int to_col_widths[] = { width, width, width, 0 }; // widths for each column
    static int from_col_widths[] = { width, width, width, 0 }; // widths for each column

    m_LinkToBrowser = m_LinkLayout.AddColResizeBrowser( to_col_widths, 3, 75 );
    m_LinkToBrowser->callback( staticScreenCB, this );

    m_LinkLayout.AddYGap();
    m_LinkLayout.AddDividerBox( "Link From" );

    m_LinkFromBrowser = m_LinkLayout.AddColResizeBrowser( from_col_widths, 3, 75 );
    m_LinkFromBrowser->callback( staticScreenCB, this );

    //===== Adv Link ====//
    m_AdvLinkLayout.SetGroupAndScreen( adv_link_group, this );
    m_AdvLinkLayout.AddDividerBox( "Adv Link Input" );
    m_AdvLinkInputBrowser = m_AdvLinkLayout.AddFlBrowser( 75 );
    m_AdvLinkLayout.AddYGap();
    m_AdvLinkLayout.AddDividerBox( "Adv Link Output" );
    m_AdvLinkOutputBrowser = m_AdvLinkLayout.AddFlBrowser( 75 );



    info_tab->show();
}

//==== Update Screen ====//
bool ParmScreen::Update()
{
    Parm* parm_ptr = ParmMgr.GetActiveParm();

    if ( !parm_ptr )
    {
        Hide();
        return false;
    }
    string pid = parm_ptr->GetID();

    m_NameString.Update( parm_ptr->GetName() );
    m_GroupString.Update( parm_ptr->GetDisplayGroupName() );
    m_DescString.Update( parm_ptr->GetDescript() );

    m_MinValString.Update( StringUtil::double_to_string(parm_ptr->GetLowerLimit(), "%6.5f") );
    m_CurrValString.Update( StringUtil::double_to_string(parm_ptr->Get(), "%6.5f") );
    m_MaxValString.Update( StringUtil::double_to_string(parm_ptr->GetUpperLimit(), "%6.5f") );

    vector< string > a_link_vec;
    vector< string > b_link_vec;
    int num_links = LinkMgr.GetNumLinks();
    for ( int i = 0 ; i < num_links ; i++ )
    {
        Link* lptr = LinkMgr.GetLink( i );
        string pa = lptr->GetParmA();
        string pb = lptr->GetParmB();

       if ( pa == pid )
       {
           b_link_vec.push_back( pb );
       }
       else if ( pb == pid )
       {
           a_link_vec.push_back( pa );
       }
    }

    //==== Display Link Parms ====//
    int to_h_pos = m_LinkToBrowser->hposition();
    int from_h_pos = m_LinkFromBrowser->hposition();
    int to_v_pos = m_LinkToBrowser->position();
    int from_v_pos = m_LinkFromBrowser->position();
    m_LinkToBrowser->clear();
    m_LinkFromBrowser->clear();

    m_LinkToBrowser->column_char( ':' );        // use : as the column character
    m_LinkFromBrowser->column_char( ':' );        // use : as the column character

    char str[256];
    sprintf( str, "@b@.COMP:@b@.GROUP:@b@.PARM:" );
    m_LinkToBrowser->add( str );
    m_LinkFromBrowser->add( str );

    for ( int i = 0 ; i < (int)b_link_vec.size() ; i++ )
    {
        Parm* p = ParmMgr.FindParm( b_link_vec[i] );
        if ( p )
        {
            sprintf( str, "%s:%s:%s:",
                     p->GetLinkContainer()->GetName().c_str(), 
                     p->GetGroupName().c_str(), p->GetName().c_str() );

            m_LinkToBrowser->add( str );
        }
    }

    for ( int i = 0 ; i < (int)a_link_vec.size() ; i++ )
    {
        Parm* p = ParmMgr.FindParm( a_link_vec[i] );
        if ( p )
        {
            sprintf( str, "%s:%s:%s:",
                     p->GetLinkContainer()->GetName().c_str(), 
                     p->GetGroupName().c_str(), p->GetName().c_str() );

            m_LinkFromBrowser->add( str );
        }
    }

    m_LinkToBrowser->hposition( to_h_pos );
    m_LinkFromBrowser->hposition( from_h_pos );
    m_LinkToBrowser->position( to_v_pos );
    m_LinkFromBrowser->position( from_v_pos );

    //==== Show Names of Adv Links ====//
    vector< string > input_links;
    vector< string > output_links;
    vector< AdvLink* > adv_link_vec = AdvLinkMgr.GetLinks();
    for ( int i = 0 ; i < (int)adv_link_vec.size() ; i++ )
    {
        vector< VarDef > inp_vec = adv_link_vec[i]->GetInputVars();
        for ( int j = 0 ; j < (int)inp_vec.size() ; j++ )
        {
            if ( inp_vec[j].m_ParmID == pid )
            {
                input_links.push_back( adv_link_vec[i]->GetName() );
                break;
            }
        }
        vector< VarDef > out_vec = adv_link_vec[i]->GetOutputVars();
        for ( int j = 0 ; j < (int)out_vec.size() ; j++ )
        {
            if ( out_vec[j].m_ParmID == pid )
            {
                output_links.push_back( adv_link_vec[i]->GetName() );
                break;
            }
        }
    }

    //==== Display Adv Link Input Names ====//
    m_AdvLinkInputBrowser->clear();
    for ( int i = 0 ; i < (int)input_links.size() ; i++ )
    {
        m_AdvLinkInputBrowser->add( input_links[i].c_str() );

    }

    //==== Display Adv Link Output Names ====//
    m_AdvLinkOutputBrowser->clear();
    for ( int i = 0 ; i < (int)output_links.size() ; i++ )
    {
        m_AdvLinkOutputBrowser->add( output_links[i].c_str() );

    }

    return true;
}

//==== Show Screen ====//
void ParmScreen::Show()
{
    if ( Update() )
    {
        m_FLTK_Window->show();
    }
}

//==== Hide Screen ====//
void ParmScreen::Hide()
{
    m_FLTK_Window->hide();
}


//==== Callbacks ====//
void ParmScreen::CallBack( Fl_Widget *w )
{


}

void ParmScreen::GuiDeviceCallBack( GuiDevice* gui_device )
{

}

//void ParmScreen::update( Parm* p )
//{
//  currParm = p;
//  sprintf( nameStr, "%s", p->get_name().get_char_star() );
//  parmUI->nameBox->label( nameStr );
//
//  parmUI->linkToBrowser->clear();
//  parmUI->linkFromBrowser->clear();
//
//  vector< ParmLink* > plVec = parmLinkMgrPtr->GetParmLinkVec();
//  for ( int i = 0 ; i < (int)plVec.size() ; i++ )
//  {
//      if ( plVec[i]->GetParmA() == p )
//          parmUI->linkToBrowser->add( plVec[i]->GetParmB()->get_name().get_char_star() );
//      if ( plVec[i]->GetParmB() == p )
//          parmUI->linkFromBrowser->add( plVec[i]->GetParmA()->get_name().get_char_star() );
//  }
//
//
//}
//
//
//void ParmScreen::show(Parm* p)
//{
//  update(p);
//  parmUI->UIWindow->show();
//}
//
//void ParmScreen::screenCB( Fl_Widget* w )
//{
//  if (  w == parmUI->linkToBrowser )
//  {
//      int sel = parmUI->linkToBrowser->value();
//      SelectToLink( sel );
//  }
//  else if (  w == parmUI->linkFromBrowser )
//  {
//      int sel = parmUI->linkFromBrowser->value();
//      SelectFromLink( sel );
//  }
//  else if ( w == parmUI->setParmAButton )
//  {
//      parmLinkMgrPtr->SetCurrParmLinkIndex(-1);
//      parmLinkMgrPtr->SetParmA( currParm );
//  }
//  else if ( w == parmUI->setParmBButton )
//  {
//      parmLinkMgrPtr->SetCurrParmLinkIndex(-1);
//      parmLinkMgrPtr->SetParmB( currParm );
//  }
//
//
//}
//
//void ParmScreen::SelectToLink( int sel )
//{
//  vector< ParmLink* > plVec = parmLinkMgrPtr->GetParmLinkVec();
//  int count = 0;
//  for ( int i = 0 ; i < (int)plVec.size() ; i++ )
//  {
//      if ( plVec[i]->GetParmA() == currParm )
//      {
//          count++;
//          if ( count == sel )
//          {
//              parmLinkMgrPtr->SetCurrParmLinkIndex( i );
//              parmLinkMgrPtr->RebuildAllLink();
//          }
//      }
//  }
//}
//
//void ParmScreen::SelectFromLink( int sel )
//{
//  vector< ParmLink* > plVec = parmLinkMgrPtr->GetParmLinkVec();
//  int count = 0;
//  for ( int i = 0 ; i < (int)plVec.size() ; i++ )
//  {
//      if ( plVec[i]->GetParmB() == currParm )
//      {
//          count++;
//          if ( count == sel )
//          {
//              parmLinkMgrPtr->SetCurrParmLinkIndex( i );
//              parmLinkMgrPtr->RebuildAllLink();
//          }
//      }
//  }
//}

