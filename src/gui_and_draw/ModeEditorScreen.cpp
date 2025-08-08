//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#include "ModeEditorScreen.h"

#include "ModeMgr.h"
#include "StlHelper.h"
#include "VarPresetMgr.h"

using namespace vsp;

ModeEditorScreen::ModeEditorScreen(ScreenMgr* mgr ) : BasicScreen( mgr, 600, 775, "Mode Editor" )
{
    int browserHeight = 150;
    int borderPaddingWidth = 5;
    int yPadding = 7;

    m_SelectedModeIndex = 0;
    m_SelectedSettingIndex = 0;

    m_FLTK_Window->callback( staticCloseCB, this );

    m_MainLayout.SetGroupAndScreen( m_FLTK_Window, this );

    m_MainLayout.ForceNewLine();
    m_MainLayout.AddY( yPadding );
    m_MainLayout.AddX( borderPaddingWidth );

    m_MainLayout.AddSubGroupLayout( m_GenLayout, m_MainLayout.GetRemainX() - borderPaddingWidth, m_MainLayout.GetRemainY() - borderPaddingWidth);

    m_GenLayout.AddDividerBox( "Mode" );

    // Pointer for the widths of each column in the browser to support resizing
    // Last column width must be 0
    static int mode_widths[] = { 120, 120, 120, 600 - ( 3 * 120 ), 0 }; // widths for each column

    m_ModeBrowser = m_GenLayout.AddColResizeBrowser( mode_widths, 4, browserHeight );
    m_ModeBrowser->callback( staticScreenCB, this );

    m_GenLayout.SetFitWidthFlag( false );
    m_GenLayout.AddX( m_GenLayout.GetW() * 0.25 );
    m_GenLayout.SetButtonWidth( m_GenLayout.GetW() * 0.5 );
    m_GenLayout.AddButton( m_ApplyAndShowOnlyMode, "Apply and Show Only" );

    m_GenLayout.SetFitWidthFlag( true );

    int bw = 100;
    m_GenLayout.SetChoiceButtonWidth( bw );

    m_GenLayout.SetButtonWidth( bw );
    m_GenLayout.AddInput( m_ModeNameInput, "Name:" );

    m_GenLayout.SetButtonWidth( m_GenLayout.GetW() * 0.5 );

    m_GenLayout.SetSameLineFlag( true );
    m_GenLayout.SetFitWidthFlag( false );
    m_GenLayout.AddButton( m_AddMode, "Create" );
    m_GenLayout.AddButton( m_DeleteMode, "Delete" );
    m_GenLayout.ForceNewLine();

    m_GenLayout.AddButton( m_RenameMode, "Rename" );
    m_GenLayout.AddButton( m_DeleteAllModes, "Delete All" );
    m_GenLayout.ForceNewLine();

    m_GenLayout.SetSameLineFlag( false );
    m_GenLayout.SetFitWidthFlag( true );



    m_GenLayout.AddYGap();



    m_GenLayout.AddDividerBox( "Sets" );

    m_GenLayout.AddChoice( m_NormalSetChoice, "Normal Set:" );
    m_GenLayout.AddChoice( m_DegenSetChoice, "Degen Set:" );
    m_GenLayout.AddYGap();


    m_GenLayout.AddDividerBox( "Variable Presets" );

    // Pointer for the widths of each column in the browser to support resizing
    // Last column width must be 0
    static int setting_widths[] = { 160, 85, 0 }; // widths for each column

    m_SettingBrowser = m_GenLayout.AddColResizeBrowser( setting_widths, 2, browserHeight );
    m_SettingBrowser->callback( staticScreenCB, this );

    m_GenLayout.SetSameLineFlag( true );
    m_GenLayout.SetFitWidthFlag( true );
    m_GenLayout.AddChoice( m_GroupChoice, "Setting Group:", m_GenLayout.GetW() * 0.5 );
    m_GenLayout.AddChoice( m_SettingChoice, "Setting:", m_GenLayout.GetW() * 0.5  );
    m_GenLayout.ForceNewLine();

    m_GenLayout.SetButtonWidth( m_GenLayout.GetW() / 3.0 );

    m_GenLayout.SetSameLineFlag( true );
    m_GenLayout.SetFitWidthFlag( false );
    m_GenLayout.AddButton( m_AddSetting, "Add" );
    m_GenLayout.AddButton( m_DeleteSetting, "Remove Setting" );
    m_GenLayout.AddButton( m_DeleteAllSettings, "Remove All Settings" );
    m_GenLayout.ForceNewLine();

    m_GenLayout.SetSameLineFlag( false );
    m_GenLayout.SetFitWidthFlag( true );
    m_GenLayout.AddYGap();
    m_AttributeEditor.Init( &m_GenLayout , m_GenLayout.GetGroup() , this , staticScreenCB, false, 0, 190 );

}

bool ModeEditorScreen::Update()
{
    BasicScreen::Update();

    assert(m_ScreenMgr);

    Mode *mod = ModeMgr.GetMode( m_SelectedModeIndex );

    if ( mod )
    {
        m_ScreenMgr->LoadSetChoice( {&m_NormalSetChoice, &m_DegenSetChoice}, {mod->m_NormalSet.GetID(), mod->m_DegenSet.GetID()}, true );

        if ( m_PrevMID != mod->GetID() )
        {
            m_ModeNameInput.Update( mod->GetName() );
            m_PrevMID = mod->GetID();
        }

        AttributeCollection* ac = mod->GetAttrCollection();
        string ac_id = ( ac )? ac->GetID() : "NONE";

        m_AttributeEditor.SetEditorCollID( ac_id );
        m_AttributeEditor.Update();
    }

    UpdateModeBrowser();

    UpdateSettingBrowser();

    UpdateVarPresetChoices();


    if ( m_ModeIDs.empty() )
    {
        m_DeleteAllModes.Deactivate();
    }
    else
    {
        m_DeleteAllModes.Activate();
    }

    if ( mod )
    {
        m_ApplyAndShowOnlyMode.Activate();
        m_DeleteMode.Activate();
        m_RenameMode.Activate();
        m_NormalSetChoice.Activate();
        m_DegenSetChoice.Activate();
        m_GroupChoice.Activate();
        m_SettingChoice.Activate();
        m_AddSetting.Activate();
    }
    else
    {
        m_ApplyAndShowOnlyMode.Deactivate();
        m_DeleteMode.Deactivate();
        m_RenameMode.Deactivate();
        m_NormalSetChoice.Deactivate();
        m_DegenSetChoice.Deactivate();
        m_GroupChoice.Deactivate();
        m_SettingChoice.Deactivate();
        m_AddSetting.Deactivate();
    }

    if ( m_NumSetting <= 0 )
    {
        m_DeleteAllSettings.Deactivate();
    }
    else
    {
        m_DeleteAllSettings.Activate();
    }

    if ( mod && m_SelectedSettingIndex >= 0 && m_SelectedSettingIndex < m_NumSetting )
    {
        m_DeleteSetting.Activate();
    }
    else
    {
        m_DeleteSetting.Deactivate();
    }


    m_FLTK_Window->redraw();
    return false;
}

void ModeEditorScreen::UpdateModeBrowser()
{
    char str[255];
    Vehicle* veh = m_ScreenMgr->GetVehiclePtr();

    //==== Input and Output Browsers ====//
    int input_h_pos = m_ModeBrowser->hposition();
    int input_v_pos = m_ModeBrowser->vposition();

    m_ModeBrowser->clear();
    m_ModeIDs.clear();

    m_ModeBrowser->column_char( ':' );

    snprintf( str, sizeof( str ),  "@b@.NAME:@b@.NORMAL SET:@b@.DEGEN SET:@b@.SETTINGS" );
    m_ModeBrowser->add( str );


    int nmode = ModeMgr.GetNumModes();

    for ( int i = 0 ; i < nmode; i++ )
    {
        Mode *mod = ModeMgr.GetMode( i );
        if ( mod )
        {
            string mname = mod->GetName();
            string sname = veh->GetSetNameVec( true )[ mod->m_NormalSet() + 1 ];
            string dname = veh->GetSetNameVec( true )[ mod->m_DegenSet() + 1 ];

            string settingstr;

            vector < pair < string, string > > settingvec = mod->GetAllGroupSettings();
            m_NumSetting = settingvec.size();

            for ( int j = 0 ; j < m_NumSetting; j++ )
            {
                string sid = settingvec[j].second;

                Setting *s = VarPresetMgr.FindSetting( sid );
                if ( s )
                {
                    string settingname = s->GetName();
                    settingstr.append( settingname );

                    if ( j < m_NumSetting - 1 )
                    {
                        settingstr.append( ", " );
                    }
                }
            }

            snprintf( str, sizeof( str ),  "%s:%s:%s:%s\n", mname.c_str(), sname.c_str(), dname.c_str(), settingstr.c_str() );
            m_ModeBrowser->add( str );
            m_ModeIDs.push_back( mod->GetID() );
        }
    }

    if ( m_SelectedModeIndex >= 0 && m_SelectedModeIndex < (int)m_ModeIDs.size() )
    {
        m_ModeBrowser->select( m_SelectedModeIndex + 2 );
    }

    m_ModeBrowser->hposition( input_h_pos );
    m_ModeBrowser->vposition( input_v_pos );
}

void ModeEditorScreen::UpdateSettingBrowser()
{
    char str[255];
    Vehicle* veh = m_ScreenMgr->GetVehiclePtr();

    //==== Input and Output Browsers ====//
    int input_h_pos = m_SettingBrowser->hposition();
    int input_v_pos = m_SettingBrowser->vposition();

    m_SettingBrowser->clear();
    m_NumSetting = 0;

    m_SettingBrowser->column_char( ':' );

    snprintf( str, sizeof( str ),  "@b@.SETTING GROUP:@b@.SETTING" );
    m_SettingBrowser->add( str );

    Mode *mod = ModeMgr.GetMode( m_SelectedModeIndex );

    if ( mod )
    {
        vector < pair < string, string > > settingvec = mod->GetAllGroupSettings();
        m_NumSetting = settingvec.size();

        for ( int i = 0 ; i < m_NumSetting; i++ )
        {
            string gid = settingvec[i].first;
            string sid = settingvec[i].second;

            SettingGroup *sg = VarPresetMgr.FindSettingGroup( gid );
            Setting *s = VarPresetMgr.FindSetting( sid );

            if ( sg && s )
            {
                string gname = sg->GetName();
                string sname = s->GetName();

                snprintf( str, sizeof( str ),  "%s:%s\n", gname.c_str(), sname.c_str() );
                m_SettingBrowser->add( str );
            }
        }

        if ( m_SelectedSettingIndex >= 0 && m_SelectedSettingIndex < m_NumSetting )
        {
            m_SettingBrowser->select( m_SelectedSettingIndex + 2 );
        }
    }


    m_SettingBrowser->hposition( input_h_pos );
    m_SettingBrowser->vposition( input_v_pos );
}

void ModeEditorScreen::UpdateVarPresetChoices()
{
    m_GroupChoice.ClearItems();
    m_GroupIDs.clear();

    m_GroupIDs = VarPresetMgr.GetAllSettingGroups();
    for ( int i = 0; i < m_GroupIDs.size(); i++ )
    {
        SettingGroup *sg = VarPresetMgr.FindSettingGroup( m_GroupIDs[i] );
        if ( sg )
        {
            m_GroupChoice.AddItem( sg->GetName(), i );
        }
    }
    m_GroupChoice.UpdateItems();

    m_GroupChoice.SetVal( m_GroupChoiceIndex );

    m_SettingChoice.ClearItems();
    m_SettingIDs.clear();

    if ( m_GroupChoiceIndex >= 0 && m_GroupChoiceIndex < m_GroupIDs.size() )
    {
        string gid = m_GroupIDs[ m_GroupChoiceIndex ];

        SettingGroup *sg = VarPresetMgr.FindSettingGroup( gid );

        if ( sg )
        {
            m_SettingIDs = sg->GetSettingIDVec();

            for ( int i = 0; i < m_SettingIDs.size(); i++ )
            {
                Setting *s = VarPresetMgr.FindSetting( m_SettingIDs[i] );
                if ( s )
                {
                    m_SettingChoice.AddItem( s->GetName(), i );
                }
            }
        }
    }
    m_SettingChoice.UpdateItems();

    m_SettingChoice.SetVal( m_SettingChoiceIndex );
}

//==== Show Screen ====//
void ModeEditorScreen::Show()
{
    m_ScreenMgr->SetUpdateFlag( true );
    m_AttributeEditor.Show();
    BasicScreen::Show();
}

//==== Hide Screen ====//
void ModeEditorScreen::Hide()
{
    m_FLTK_Window->hide();
    m_ScreenMgr->SetUpdateFlag( true );
}

void ModeEditorScreen::CloseCallBack( Fl_Widget *w )
{
    Hide();
}

void ModeEditorScreen::CallBack( Fl_Widget *w )
{

    bool changed = false;

    assert( m_ScreenMgr ); 
    Vehicle* vehiclePtr = m_ScreenMgr->GetVehiclePtr();

    if ( w == m_ModeBrowser )
    {
        m_SelectedModeIndex = m_ModeBrowser->value() - 2;

        Mode *mod = ModeMgr.GetMode( m_SelectedModeIndex );

        if ( mod )
        {
            m_ModeNameInput.Update( mod->GetName() );
        }
        changed = true;
    }
    else if ( w == m_SettingBrowser )
    {
        m_SelectedSettingIndex = m_SettingBrowser->value() - 2;
        changed = true;
    }

    if ( changed )
    {
        Mode *mod = ModeMgr.GetMode( m_SelectedModeIndex );

        if ( mod )
        {
            vector < pair < string, string > > settingvec = mod->GetAllGroupSettings();

            if ( m_SelectedSettingIndex >= 0 && m_SelectedSettingIndex < settingvec.size() )
            {
                const pair < string, string >& setting = settingvec[ m_SelectedSettingIndex ];
                string gid = setting.first;
                m_GroupChoiceIndex = vector_find_val( m_GroupIDs, gid );

                // Updating m_SettingIDs here (vs. UpdateVarPresetChoices()) is needed so this
                // vector_find_val will search the right vector.
                SettingGroup *sg = VarPresetMgr.FindSettingGroup( gid );
                if ( sg )
                {
                    m_SettingIDs = sg->GetSettingIDVec();
                }

                string sid = setting.second;
                m_SettingChoiceIndex = vector_find_val( m_SettingIDs, sid );
            }
        }
    }

    m_AttributeEditor.DeviceCB( w );

    m_ScreenMgr->SetUpdateFlag( true );
}

//Callback for GUI Devices related events like buttons or input fields
void ModeEditorScreen::GuiDeviceCallBack( GuiDevice* device )
{

    assert( m_ScreenMgr );
    Vehicle* vehiclePtr = m_ScreenMgr->GetVehiclePtr();

    Mode *mod = ModeMgr.GetMode( m_SelectedModeIndex );


    if ( device == &m_AddMode )
    {
        string mname = m_ModeNameInput.GetString();
        if ( !mname.empty() )
        {
            ModeMgr.CreateAndAddMode( mname );
            m_SelectedModeIndex = ModeMgr.GetNumModes() - 1;
        }
    }
    else if ( device == &m_DeleteMode )
    {
        if ( mod )
        {
            ModeMgr.DelMode( m_SelectedModeIndex );
        }
    }
    else if ( device == &m_DeleteAllModes )
    {
        ModeMgr.DelAllModes();
    }
    else if ( device == &m_RenameMode )
    {
        if ( mod )
        {
            mod->SetName( m_ModeNameInput.GetString() );
        }
    }
    else if ( device == &m_AddSetting )
    {
        if ( mod )
        {
            if ( m_GroupChoiceIndex >= 0 && m_GroupChoiceIndex < m_GroupIDs.size() )
            {
                string gid = m_GroupIDs[ m_GroupChoiceIndex ];

                if ( m_SettingChoiceIndex >= 0 && m_SettingChoiceIndex < m_SettingIDs.size() )
                {
                    string sid = m_SettingIDs[ m_SettingChoiceIndex ];

                    pair < string, string > setting( gid, sid );
                    mod->AddSetting( setting );
                }
            }
        }
    }
    else if ( device == &m_DeleteSetting )
    {
        if ( mod && m_SelectedSettingIndex >= 0 && m_SelectedSettingIndex < m_NumSetting )
        {
            mod->RemoveSetting( m_SelectedSettingIndex );
        }
    }
    else if ( device == &m_DeleteAllSettings )
    {
        if ( mod )
        {
            mod->RemoveAllSettings();
        }
    }
    else if ( device == &m_ApplyAndShowOnlyMode )
    {
        if ( mod )
        {
            mod->ApplySettings();

            mod->ShowOnly();
        }
    }
    else if ( device == &m_GroupChoice )
    {
        m_GroupChoiceIndex = m_GroupChoice.GetVal();
    }
    else if ( device == &m_SettingChoice )
    {
        m_SettingChoiceIndex = m_SettingChoice.GetVal();
    }

    m_AttributeEditor.GuiDeviceCallBack( device );

    m_ScreenMgr->SetUpdateFlag( true );
}

void ModeEditorScreen::GetCollIDs( vector < string > &collIDVec )
{
    m_AttributeEditor.GetCollIDs( collIDVec );
}
