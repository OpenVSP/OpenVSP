//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#include "TypeEditorScreen.h"

//==== Constructor ====//
TypeEditorScreen::TypeEditorScreen( ScreenMgr* mgr ) : BasicScreen( mgr, 210, 450, "Geom Type Editor" )
{
    int borderPaddingWidth = 5;
    int yPadding = 7;
    int smallButtonWidth = 100;
    int smallGap = 25;
    int browserHeight = 127;

    m_TypeBrowserIndex = 0;
    m_GeomIndex = 0;
    m_ScriptIndex = 0;

    m_FLTK_Window->callback( staticCloseCB, this );

    m_MainLayout.SetGroupAndScreen( m_FLTK_Window, this );

    m_MainLayout.ForceNewLine();
    m_MainLayout.AddY( yPadding );
    m_MainLayout.AddX( borderPaddingWidth );

    m_MainLayout.AddSubGroupLayout( m_BorderLayout, m_MainLayout.GetRemainX() - borderPaddingWidth,
                                        m_MainLayout.GetRemainY() - borderPaddingWidth );

    m_BorderLayout.SetChoiceButtonWidth(0);
    m_BorderLayout.AddChoice( m_GeomChoice, "" );
    m_BorderLayout.AddYGap();

    m_BorderLayout.SetFitWidthFlag( true );

    m_BorderLayout.AddButton( m_AddToggle, "Add Geom" );
    m_BorderLayout.AddYGap();

    m_BorderLayout.AddDividerBox( "Geom Types" );

    m_TypeBrowser = m_BorderLayout.AddFlBrowser( browserHeight );

    m_BorderLayout.AddInput( m_TypeNameInput, "Name:", 0 );
    m_BorderLayout.AddYGap();

    m_BorderLayout.AddButton( m_DeleteToggle, "Delete" );
    m_BorderLayout.AddYGap();

    m_BorderLayout.AddDividerBox( "Custom Scripts" );

    m_CustomScriptsBrowser = m_BorderLayout.AddFlBrowser( browserHeight );
    m_BorderLayout.AddYGap();

    m_BorderLayout.AddButton( m_CustomScriptToggle, "Save To File" );

    m_TypeBrowser->callback( staticCB, this );
    m_CustomScriptsBrowser->callback( staticCB, this ); 
}

//==== Update Screen ====//
bool TypeEditorScreen::Update()
{
    assert( m_ScreenMgr );
    Vehicle* veh = m_ScreenMgr->GetVehiclePtr();

    char str[256];
    m_GeomChoice.ClearItems();
    vector < string > geom_id_vec = veh->GetValidTypeGeoms();
    vector< Geom* > geom_vec = veh->FindGeomVec( geom_id_vec );
    for ( int i = 0 ; i < ( int )geom_vec.size() ; i++ )
    {
        sprintf( str, "%d.  %s", i + 1, geom_vec[i]->GetName().c_str() );
        m_GeomChoice.AddItem( str );
    }

    m_GeomChoice.UpdateItems();
    m_GeomChoice.SetVal( m_GeomIndex );

    m_TypeBrowser->clear();
    vector< GeomType > type_vec = veh->GetEditableGeomTypes();
    for ( int i = 0; i < ( int )type_vec.size(); i++ )
    {
        printf( "HOW MANY %d ", ( int )type_vec.size() );
        m_TypeBrowser->add( type_vec[i].m_Name.c_str() );
    }

    m_CustomScriptsBrowser->clear();
    vector< string > mod_name_vec = CustomGeomMgr.GetCustomScriptModuleNames();
    for ( int i = 0 ; i < ( int )mod_name_vec.size() ; i++ )
    {
        m_CustomScriptsBrowser->add( mod_name_vec[i].c_str() );
    }

    m_TypeBrowser->select( m_TypeBrowserIndex );
    m_CustomScriptsBrowser->select( m_ScriptIndex );

    return true;
}

//==== Show Screen ====//
void TypeEditorScreen::Show()
{
    m_ScreenMgr->SetUpdateFlag( true );
    m_FLTK_Window->show();
}

//==== Hide Screen ====//
void TypeEditorScreen::Hide()
{
    m_FLTK_Window->hide();
    m_ScreenMgr->SetUpdateFlag( true );
}

void TypeEditorScreen::GuiDeviceCallBack( GuiDevice* device )
{
    assert( m_ScreenMgr );
    Vehicle* veh = m_ScreenMgr->GetVehiclePtr();
    vector < string > geom_id_vec = veh->GetValidTypeGeoms();

    if ( device == &m_AddToggle )
    {
        if ( m_GeomIndex > 0 || m_GeomIndex < ( int )geom_id_vec.size() )
        {
            veh->AddType( geom_id_vec[m_GeomIndex] );
        }
    }
    else if ( device == &m_DeleteToggle )
    {
        int offset = veh->GetNumFixedGeomTypes();
        veh->DeleteType( m_TypeBrowserIndex + offset );
    }
    else if ( device == &m_GeomChoice )
    {
        m_GeomIndex = m_GeomChoice.GetVal();
    }
    else if ( device == &m_TypeNameInput )
    {
        string name = string( m_TypeNameInput.GetString() );
        if ( name != "" )
        {
            int offset = veh->GetNumFixedGeomTypes();
            GeomType type = veh->GetGeomType( m_TypeBrowserIndex + offset );
            if ( ! type.m_FixedFlag )
            {
                type.m_Name = name;
                veh->SetGeomType( m_TypeBrowserIndex + offset, type );
            }
        }
        
    }
    else if (device == &m_CustomScriptToggle )
    {
        vector< string > mod_name_vec = CustomGeomMgr.GetCustomScriptModuleNames();
        if ( m_ScriptIndex >= 0 && m_ScriptIndex < (int)mod_name_vec.size() )
        {
            string module_name = mod_name_vec[m_ScriptIndex];

            string dir = veh->GetWriteScriptDir();
            string savefile = m_ScreenMgr->GetSelectFileScreen()->FileChooser( "Save Custom Geom Script", "*.vsppart",  dir.c_str() );

            CustomGeomMgr.SaveScriptContentToFile( module_name, savefile );
        }
    }

    m_ScreenMgr->SetUpdateFlag( true );
}

//==== Callbacks ====//
void TypeEditorScreen::CallBack( Fl_Widget *w )
{
    if ( w == m_TypeBrowser )
    {
        m_TypeBrowserIndex = m_TypeBrowser->value() ;
    }
    else if ( w == m_CustomScriptsBrowser )
    {
        m_ScriptIndex = m_CustomScriptsBrowser->value() ;
    }

    assert( m_ScreenMgr );
    m_ScreenMgr->SetUpdateFlag( true );
}

void TypeEditorScreen::CloseCallBack( Fl_Widget* w )
{
    Hide();
}


