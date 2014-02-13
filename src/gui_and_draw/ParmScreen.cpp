//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// parmScreen.cpp: implementation of the geomScreen class.
//
//////////////////////////////////////////////////////////////////////

#include "ParmScreen.h"
#include "ParmMgr.h"
#include "StringUtil.h"

#include <FL/Fl_File_Chooser.H>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

ParmScreen::ParmScreen( ScreenMgr* mgr )  : VspScreen( mgr )
{

    ParmUI* ui = m_ParmUI = new ParmUI();
    m_FLTK_Window = ui->UIWindow;

    ui->UIWindow->position( 780, 30 );
    ui->linkToBrowser->callback( staticScreenCB, this );
    ui->linkFromBrowser->callback( staticScreenCB, this );
    ui->setParmAButton->callback( staticScreenCB, this );
    ui->setParmBButton->callback( staticScreenCB, this );
    ui->parmDescription->buffer( &m_TextBuffer );
    ui->parmDescription->wrap_mode( Fl_Text_Display::WRAP_AT_BOUNDS, 0 );

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

    m_NameString = parm_ptr->GetName();
    m_ParmUI->nameBox->label( m_NameString.c_str() );

    m_DescriptString = parm_ptr->GetDescript();
    m_TextBuffer.text( m_DescriptString.c_str() );


    //char str[256];
    //sprintf( nameStr, "%s", parm_ptr->GetName().c_str() );
    //m_ParmUI->nameBox->label( nameStr );




    //parmUI->linkToBrowser->clear();
    //parmUI->linkFromBrowser->clear();

    //vector< ParmLink* > plVec = parmLinkMgrPtr->GetParmLinkVec();
    //for ( int i = 0 ; i < (int)plVec.size() ; i++ )
    //{
    //  if ( plVec[i]->GetParmA() == p )
    //      parmUI->linkToBrowser->add( plVec[i]->GetParmB()->get_name().get_char_star() );
    //  if ( plVec[i]->GetParmB() == p )
    //      parmUI->linkFromBrowser->add( plVec[i]->GetParmA()->get_name().get_char_star() );
    //}


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


//==== Callbacks ====//
void ParmScreen::CallBack( Fl_Widget *w )
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

