//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#include "TypeEditorScreen.h"
#include "ScreenMgr.h"
#include "CustomGeom.h"

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
    ui->customScriptsBrowser->callback( staticScreenCB, this );
    ui->customScriptFileButton ->callback( staticScreenCB, this );

    m_SelectedIndex = 0;
    m_GeomIndex = 0;
    m_ScriptIndex = 0;
}

//==== Update Screen ====//
bool TypeEditorScreen::Update()
{
    Vehicle* veh = m_ScreenMgr->GetVehiclePtr();

    //==== Only Display Editable Types =====//
    vector< GeomType > type_vec = veh->GetEditableGeomTypes();
    if ( m_SelectedIndex >= ( int )type_vec.size() )
    {
        m_SelectedIndex = -1;
    }

    //==== Load Type Names and Values ====//
    m_TypeEditorUI->typeBrowser->clear();
    for ( int i = 0 ; i < ( int )type_vec.size() ; i++ )
    {
        m_TypeEditorUI->typeBrowser->add( type_vec[i].m_Name.c_str() );
    }

    if ( m_SelectedIndex >= 0 )
    {
        m_TypeEditorUI->typeBrowser->select( m_SelectedIndex + 1 );
        m_TypeEditorUI->typeNameInput->value( type_vec[m_SelectedIndex].m_Name.c_str() );
    }
    else
    {
        m_TypeEditorUI->typeNameInput->value( "" );
    }

    //==== Load Geometry ====//
    char str[256];
    m_TypeEditorUI->geomChoice->clear();
    vector< string > geom_id_vec = veh->GetValidTypeGeoms();
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

    //==== Load Custom Script Module Names ====//
    m_TypeEditorUI->customScriptsBrowser->clear();
    vector< string > mod_name_vec = CustomGeomMgr.GetCustomScriptModuleNames();
    for ( int i = 0 ; i < ( int )mod_name_vec.size() ; i++ )
    {
        m_TypeEditorUI->customScriptsBrowser->add( mod_name_vec[i].c_str() );
    }

    if ( m_ScriptIndex >= ( int )mod_name_vec.size() )
        m_ScriptIndex = -1;

    if ( m_ScriptIndex >= 0 )
        m_TypeEditorUI->customScriptsBrowser->select( m_ScriptIndex + 1 );

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
        vector< string > geom_id_vec = veh->GetValidTypeGeoms();
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
        GeomType type = veh->GetGeomType( m_SelectedIndex + offset );
        if ( type.m_FixedFlag == false )
        {
            type.m_Name = name;
            veh->SetGeomType( m_SelectedIndex + offset, type );
        }
    }
    else if ( w == m_TypeEditorUI->deleteType )
    {
        int offset = veh->GetNumFixedGeomTypes();
        veh->DeleteType( m_SelectedIndex + offset );
        m_SelectedIndex = -1;
    }
    else if ( w == m_TypeEditorUI->customScriptsBrowser )
    {
        m_ScriptIndex = m_TypeEditorUI->customScriptsBrowser->value() - 1;
    }
    else if ( w == m_TypeEditorUI->customScriptFileButton )
    {
        vector< string > mod_name_vec = CustomGeomMgr.GetCustomScriptModuleNames();
        if ( m_ScriptIndex >= 0 && m_ScriptIndex < (int)mod_name_vec.size() )
        {
            string module_name = mod_name_vec[m_ScriptIndex];

            string dir = CustomGeomMgr.GetScriptDir();
            string savefile = m_ScreenMgr->GetSelectFileScreen()->FileChooser( "Save Custom Geom Script", "*.as",  dir.c_str() );

            CustomGeomMgr.SaveScriptContentToFile( module_name, savefile );
        }
    }

    m_ScreenMgr->SetUpdateFlag( true );

}


