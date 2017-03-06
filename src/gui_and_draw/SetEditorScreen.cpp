//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#include "SetEditorScreen.h"
#include "ScreenMgr.h"
#include "StlHelper.h"

using namespace vsp;

//==== Constructor ====//
SetEditorScreen::SetEditorScreen( ScreenMgr* mgr ) : VspScreen( mgr )
{
    SetEditorUI* ui = m_SetEditorUI = new SetEditorUI();
    VspScreen::SetFlWindow( ui->UIWindow );

    m_SelectedSetIndex = SET_ALL;

    ui->geomInSetBrowser->callback( staticScreenCB, this );
    ui->setBrowser->callback( staticScreenCB, this );
    ui->setNameInput->callback( staticScreenCB, this );
    ui->highlightSetButton->callback( staticScreenCB, this );
}

//==== Update Screen ====//
bool SetEditorScreen::Update()
{
    Vehicle* veh = m_ScreenMgr->GetVehiclePtr();

    vector< string > set_name_vec = veh->GetSetNameVec();

    //==== Load Set Names and Values ====//
    m_SetEditorUI->setBrowser->clear();
    for ( int i = SET_SHOWN ; i < ( int )set_name_vec.size() ; i++ )
    {
        m_SetEditorUI->setBrowser->add( set_name_vec[i].c_str() );
    }
    m_SetEditorUI->setBrowser->select( m_SelectedSetIndex );
    m_SetEditorUI->setNameInput->value( set_name_vec[m_SelectedSetIndex].c_str() );

    if ( m_SelectedSetIndex <= SET_NOT_SHOWN )
    {
        m_SetEditorUI->setNameInput->deactivate();
    }
    else
    {
        m_SetEditorUI->setNameInput->activate();
    }

    //==== Load Geometry ====//
    m_SetEditorUI->geomInSetBrowser->clear();
    vector< string > geom_id_vec = veh->GetGeomVec();
    vector< Geom* > geom_vec = veh->FindGeomVec( geom_id_vec );
    for ( int i = 0 ; i < ( int )geom_vec.size() ; i++ )
    {
        string gname = geom_vec[i]->GetName();
        bool flag = geom_vec[i]->GetSetFlag( m_SelectedSetIndex );
        m_SetEditorUI->geomInSetBrowser->add( gname.c_str(), !!flag );

    }

    return true;
}

//==== Show Screen ====//
void SetEditorScreen::Show()
{
    Update();
    m_FLTK_Window->show();
}


//==== Hide Screen ====//
void SetEditorScreen::Hide()
{
    m_FLTK_Window->hide();
}

//==== Callbacks ====//
void SetEditorScreen::CallBack( Fl_Widget *w )
{
    Vehicle* veh = m_ScreenMgr->GetVehiclePtr();
    if ( w == m_SetEditorUI->geomInSetBrowser )
    {
        int geom_index = m_SetEditorUI->geomInSetBrowser->value() - 1;
        vector< string > geom_id_vec = veh->GetGeomVec();
        if ( geom_index >= 0 && geom_index < ( int )geom_id_vec.size() )
        {
            int flag = m_SetEditorUI->geomInSetBrowser->checked( geom_index + 1 );
            Geom* gptr = veh->FindGeom( geom_id_vec[ geom_index ] );
            if ( gptr )
            {
                gptr->SetSetFlag( m_SelectedSetIndex, !!flag );
            }
        }
    }
    else if ( w == m_SetEditorUI->setBrowser )
    {
        m_SelectedSetIndex = m_SetEditorUI->setBrowser->value();
    }
    else if ( w == m_SetEditorUI->setNameInput )
    {
        string name = string( m_SetEditorUI->setNameInput->value() );
        veh->SetSetName( m_SelectedSetIndex, name );
    }
    else if ( w == m_SetEditorUI->highlightSetButton )
    {
        vector < string > activate_geom_vec = veh->GetGeomSet( m_SelectedSetIndex );
        veh->SetActiveGeomVec( activate_geom_vec );
    }

    m_ScreenMgr->SetUpdateFlag( true );
//  m_ScreenMgr->UpdateAllScreens();
}


