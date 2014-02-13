//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#include "TypeEditorScreen.h"
#include "ScreenMgr.h"

#include <assert.h>

//==== Constructor ====//
TypeEditorScreen::TypeEditorScreen( ScreenMgr* mgr ) : VspScreen( mgr )
{
    TypeEditorUI* ui = m_TypeEditorUI = new TypeEditorUI();
    VspScreen::SetFlWindow( ui->UIWindow );

    ui->addButton->callback( staticScreenCB, this );
    ui->deleteType->callback( staticScreenCB, this );
    ui->geomChoice->callback( staticScreenCB, this );
    ui->typeBrowser->callback( staticScreenCB, this );
    ui->typeNameInput->callback( staticScreenCB, this );

    m_SelectedIndex = 0;
    m_GeomIndex = 0;
}

//==== Update Screen ====//
bool TypeEditorScreen::Update()
{
    Vehicle* veh = m_ScreenMgr->GetVehiclePtr();

    int num_type = veh->GetNumGeomTypes();

    //==== Only Display Non Fixed Types =====//
    vector< GeomType* > type_vec;
    for ( int i = 0 ; i < num_type ; i++ )
    {
        GeomType* type_ptr = veh->GetGeomType( i );
        if ( type_ptr && type_ptr->m_FixedFlag == false )
        {
            type_vec.push_back( type_ptr );
        }
    }

    if ( m_SelectedIndex >= ( int )type_vec.size() )
    {
        m_SelectedIndex = -1;
    }


    //==== Load Set Names and Values ====//
    m_TypeEditorUI->typeBrowser->clear();
    for ( int i = 0 ; i < ( int )type_vec.size() ; i++ )
    {
        m_TypeEditorUI->typeBrowser->add( type_vec[i]->m_Name.c_str() );
    }

    if ( m_SelectedIndex >= 0 )
    {
        m_TypeEditorUI->typeBrowser->select( m_SelectedIndex + 1 );
        m_TypeEditorUI->typeNameInput->value( type_vec[m_SelectedIndex]->m_Name.c_str() );
    }
    else
    {
        m_TypeEditorUI->typeNameInput->value( "" );
    }

    //==== Load Geometry ====//
    char str[256];
    m_TypeEditorUI->geomChoice->clear();
    vector< string > geom_id_vec = veh->GetGeomVec();
    vector< Geom* > geom_vec = veh->FindGeomVec( geom_id_vec );
    for ( int i = 0 ; i < ( int )geom_vec.size() ; i++ )
    {
        sprintf( str, "%d.  %s", i + 1, geom_vec[i]->GetName().c_str() );

        m_TypeEditorUI->geomChoice->add( str );
    }

    if ( m_GeomIndex >= 0 )
    {
        m_TypeEditorUI->geomChoice->value( m_GeomIndex );
    }


    return true;
}

//==== Show Screen ====//
void TypeEditorScreen::Show()
{
    Update();
    m_FLTK_Window->show();
}


//==== Hide Screen ====//
void TypeEditorScreen::Hide()
{
    m_FLTK_Window->hide();
}

//==== Callbacks ====//
void TypeEditorScreen::CallBack( Fl_Widget *w )
{
    Vehicle* veh = m_ScreenMgr->GetVehiclePtr();
    if ( w == m_TypeEditorUI->addButton )
    {
        vector< string > geom_id_vec = veh->GetGeomVec();
        if ( m_GeomIndex >= 0 && m_GeomIndex < ( int )geom_id_vec.size() )
        {
            veh->AddType( geom_id_vec[m_GeomIndex] );
        }
    }
    else if ( w == m_TypeEditorUI->typeBrowser )
    {
        m_SelectedIndex = m_TypeEditorUI->typeBrowser->value() - 1;
    }
    else if ( w == m_TypeEditorUI->geomChoice )
    {
        m_GeomIndex = m_TypeEditorUI->geomChoice->value();
    }
    else if ( w == m_TypeEditorUI->typeNameInput )
    {
        string name = string( m_TypeEditorUI->typeNameInput->value() );

        int offset = veh->GetNumFixedGeomTypes();
        GeomType* type_ptr = veh->GetGeomType( m_SelectedIndex + offset );
        if ( type_ptr && type_ptr->m_FixedFlag == false )
        {
            type_ptr->m_Name = name;
        }
    }
    else if ( w == m_TypeEditorUI->deleteType )
    {
        int offset = veh->GetNumFixedGeomTypes();
        veh->DeleteType( m_SelectedIndex + offset );
        m_SelectedIndex = -1;
    }

    m_ScreenMgr->SetUpdateFlag( true );
//  m_ScreenMgr->UpdateAllScreens();
}


