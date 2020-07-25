//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#include "TypeEditorScreen.h"

//==== Constructor ====//
TypeEditorScreen::TypeEditorScreen( ScreenMgr* mgr ) : BasicScreen( mgr, 210, 245, "Import Custom Script" )
{
    int borderPaddingWidth = 5;
    int yPadding = 7;
    int browserHeight = 170;

    m_ScriptIndex = 0;

    m_FLTK_Window->callback( staticCloseCB, this );

    m_MainLayout.SetGroupAndScreen( m_FLTK_Window, this );

    m_MainLayout.ForceNewLine();
    m_MainLayout.AddY( yPadding );
    m_MainLayout.AddX( borderPaddingWidth );

    m_MainLayout.AddSubGroupLayout( m_BorderLayout, m_MainLayout.GetRemainX() - borderPaddingWidth,
                                        m_MainLayout.GetRemainY() - borderPaddingWidth );

    m_BorderLayout.AddDividerBox( "Custom Scripts" );

    m_CustomScriptsBrowser = m_BorderLayout.AddFlBrowser( browserHeight );
    m_BorderLayout.AddYGap();

    m_BorderLayout.AddButton( m_CustomScriptToggle, "Save To File" );

    m_CustomScriptsBrowser->callback( staticCB, this ); 
}

//==== Update Screen ====//
bool TypeEditorScreen::Update()
{
    assert( m_ScreenMgr );
    Vehicle* veh = m_ScreenMgr->GetVehiclePtr();

    m_CustomScriptsBrowser->clear();
    vector< string > mod_name_vec = CustomGeomMgr.GetCustomScriptModuleNames();
    for ( int i = 0 ; i < ( int )mod_name_vec.size() ; i++ )
    {
        m_CustomScriptsBrowser->add( mod_name_vec[i].c_str() );
    }

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

    if (device == &m_CustomScriptToggle )
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
    if ( w == m_CustomScriptsBrowser )
    {
        m_ScriptIndex = m_CustomScriptsBrowser->value() - 1;
    }

    assert( m_ScreenMgr );
    m_ScreenMgr->SetUpdateFlag( true );
}

void TypeEditorScreen::CloseCallBack( Fl_Widget* w )
{
    Hide();
}


