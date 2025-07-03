//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// VarPresetEditorScreen.cpp: implementation of the VarPresetEditorScreen class.
//
//////////////////////////////////////////////////////////////////////

#include "VarPresetEditorScreen.h"
#include "ParmMgr.h"
#include "StlHelper.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

VarPresetEditorScreen::VarPresetEditorScreen( ScreenMgr* mgr ) : TabScreen( mgr, 400, 750, "Variable Presets", "" )
{
    m_FLTK_Window->callback( staticCloseCB, this );

    Fl_Group* apply_tab = AddTab( "Apply" );
    apply_tab->callback( staticScreenCB, this );
    Fl_Group* apply_group = AddSubGroup( apply_tab, 5 );

    Fl_Group* group_tab = AddTab( "Group" );
    ( (Vsp_Group*) group_tab )->SetAllowDrop( true );
    group_tab->callback( staticScreenCB, this );
    Fl_Group* groups_group = AddSubGroup( group_tab, 5 );

    Fl_Group* setting_tab = AddTab( "Settings" );
    setting_tab->callback( staticScreenCB, this );
    Fl_Group* setting_group = AddSubGroup( setting_tab, 5 );


    // Set up change state persistent area at bottom/top? of GUI.
    m_ChangeStateLayout.SetGroupAndScreen( apply_group, this );

    m_ChangeStateLayout.SetChoiceButtonWidth( m_ChangeStateLayout.GetButtonWidth() );

    m_ChangeStateLayout.AddDividerBox( "Select Preset" );
    m_ChangeStateLayout.AddChoice( m_GroupChoice, "Group" );
    m_ChangeStateLayout.AddChoice( m_SettingChoice, "Setting");
    m_ChangeStateLayout.AddButton( m_ApplyButton, "Apply" );
    m_ChangeStateLayout.AddYGap();

    // Everything Relevant to "Crete" Tab
    m_GroupsLayout.SetGroupAndScreen( groups_group, this );

    m_GroupsLayout.AddDividerBox( "Groups" );

    int browser_h = 100;

    m_GroupBrowser = m_GroupsLayout.AddFlBrowser( browser_h );
    m_GroupBrowser->callback( staticScreenCB, this );
    m_GroupsLayout.AddInput( m_GroupInput, "Group" );

    m_GroupsLayout.SetSameLineFlag( true );
    m_GroupsLayout.SetFitWidthFlag( false );
    m_GroupsLayout.SetButtonWidth( m_GroupsLayout.GetW() / 2.0 );

    m_GroupsLayout.AddButton( m_AddGroupButton, "Add" );
    m_GroupsLayout.AddButton( m_DeleteGroupButton, "Delete" );
    m_GroupsLayout.ForceNewLine();

    m_GroupsLayout.AddButton( m_RenameGroupButton, "Rename" );
    m_GroupsLayout.AddButton( m_ClearGroupsButton, "Delete All" );
    m_GroupsLayout.ForceNewLine();

    m_GroupsLayout.SetSameLineFlag( false );
    m_GroupsLayout.SetFitWidthFlag( true );



    m_GroupsLayout.AddYGap();

    m_GroupsLayout.AddDividerBox( "Variables" );

    m_GroupsLayout.AddParmPicker( m_ParmPicker );

    m_GroupsLayout.SetSameLineFlag( true );
    m_GroupsLayout.SetFitWidthFlag( false );
    m_GroupsLayout.SetButtonWidth( m_GroupsLayout.GetW() / 2.0 );

    m_GroupsLayout.AddButton( m_AddVarButton, "Add" );
    m_GroupsLayout.AddButton( m_DelVarButton, "Delete" );
    m_GroupsLayout.AddButton( m_ClearVarsButton, "Delete All" );

    m_GroupsLayout.ForceNewLine();

    m_GroupsLayout.SetSameLineFlag( false );
    m_GroupsLayout.SetFitWidthFlag( true );

    // Pointer for the widths of each column in the browser to support resizing
    // Last column width must be 0
    static int col_widths[] = { 100, 100, 90, 0 }; // widths for each column

    m_VarBrowser = m_GroupsLayout.AddColResizeBrowser( col_widths, 3, browser_h );
    m_VarBrowser->callback( staticScreenCB, this );

    m_GroupsLayout.AddYGap();
    m_GroupAttrEditor.Init( &m_GroupsLayout, m_GroupsLayout.GetGroup(), this, staticScreenCB, false, 0, 140);

    // Everything Relevant to "Setting" Tab
    m_SettingLayout.SetGroupAndScreen( setting_group, this );

    m_SettingLayout.AddDividerBox( "Settings" );

    m_SettingBrowser = m_SettingLayout.AddFlBrowser( browser_h );
    m_SettingBrowser->callback( staticScreenCB, this );


    m_SettingLayout.AddInput( m_SettingInput, "Setting" );

    m_SettingLayout.SetSameLineFlag( true );
    m_SettingLayout.SetFitWidthFlag( false );
    m_SettingLayout.SetButtonWidth( m_SettingLayout.GetW() / 2.0 );

    m_SettingLayout.AddButton( m_AddSettingButton, "Add" );
    m_SettingLayout.AddButton( m_DeleteSettingButton, "Delete" );
    m_SettingLayout.ForceNewLine();

    m_SettingLayout.AddButton( m_RenameSettingButton, "Rename" );
    m_SettingLayout.AddButton( m_ClearSettingsButton, "Delete All" );
    m_SettingLayout.ForceNewLine();

    m_SettingLayout.AddButton( m_Apply2Button, "Apply" );
    m_SettingLayout.AddButton( m_UpdateSettingButton , "Update" );
    m_SettingLayout.ForceNewLine();

    m_SettingLayout.SetSameLineFlag( false );
    m_SettingLayout.SetFitWidthFlag( true );




    m_SettingLayout.ForceNewLine();
    m_SettingLayout.AddDividerBox( "Variables" );
    m_SettingLayout.AddYGap();


    m_AdjustScrollGroup = AddSubScroll( m_SettingLayout.GetGroup(), 5, 165, m_SettingLayout.GetY() - m_SettingLayout.GetStartY() );
    m_AdjustScrollGroup->type( Fl_Scroll::VERTICAL_ALWAYS );

    m_AdjustLayout.SetGroupAndScreen( m_AdjustScrollGroup, this );

    m_SettingLayout.SetY( m_SettingLayout.GetY() + m_AdjustLayout.GetY() );
    m_SettingLayout.AddYGap();
    m_SettingAttrEditor.Init( &m_SettingLayout, m_SettingLayout.GetGroup(), this, staticScreenCB, false, 0, 140);

    apply_tab->show();

    m_GroupIndex = 0;
    m_SettingIndex = 0;
    m_VarIndex = 0;

    m_ParmListHash = 0;
}

VarPresetEditorScreen::~VarPresetEditorScreen()
{
}

bool VarPresetEditorScreen::Update()
{
    TabScreen::Update();

    SettingGroup *sg = nullptr;
    if ( m_GroupIndex >= 0 && m_GroupIndex < m_GroupIDs.size() )
    {
        string gid = m_GroupIDs[ m_GroupIndex ];
        sg = VarPresetMgr.FindSettingGroup( gid );
    }

    Setting *s = nullptr;
    if ( m_SettingIndex >= 0 && m_SettingIndex < m_SettingIDs.size() )
    {
        string sid = m_SettingIDs[ m_SettingIndex ];

        if ( sg && sg->HasSetting( sid ) )
        {
            s = VarPresetMgr.FindSetting( sid );
        }
    }


    if ( sg && m_PrevGID != sg->GetID() )
    {
        m_GroupInput.Update( sg->GetName() );

        m_PrevGID = sg->GetID();
        m_SettingIndex = 0;
        m_PrevSID = "";
    }

    if ( s && m_PrevSID != s->GetID() )
    {
        m_SettingInput.Update( s->GetName() );
        m_PrevSID = s->GetID();
    }

    string sg_ac_id = ( sg ) ? sg->GetAttributeCollection()->GetID() : string("");
    string  s_ac_id = (  s ) ?  s->GetAttributeCollection()->GetID() : string("");

    m_GroupAttrEditor.SetEditorCollID( sg_ac_id );
    m_SettingAttrEditor.SetEditorCollID( s_ac_id );

    m_GroupAttrEditor.Update();
    m_SettingAttrEditor.Update();

    RebuildMenus();

    m_ParmPicker.Update();

    UpdateVarBrowser(); // Updates m_ParmIDs

    RebuildAdjustTab(); // Relies on up-to-date m_ParmIDs

    // ==== Check if Save Necessary ==== //
    CheckSaveStatus();

    EnableDisableWidgets();

    m_FLTK_Window->redraw();

    return false;
}

void VarPresetEditorScreen::RebuildAdjustTab()
{
    // Serialize m_ParmIDs into single long string.
    string str = string_vec_serialize( m_ParmIDs );
    // Calculate hash to detect changes in m_ParmIDs
    std::size_t str_hash = std::hash < std::string >{}( str );

    // Relies on currency of m_ParmIDs by UpdateVarBrowser()
    if ( str_hash != m_ParmListHash )
    {
        m_AdjustScrollGroup->clear();
        m_AdjustLayout.SetGroup( m_AdjustScrollGroup );
        m_AdjustLayout.InitWidthHeightVals();
        m_AdjustLayout.SetButtonWidth( 100 );

        m_ParmSliderVec.clear();

        m_ParmSliderVec.resize( m_ParmIDs.size() );

        string lastContID;
        for ( int i = 0; i < m_ParmIDs.size(); i++ )
        {
            Parm *p = ParmMgr.FindParm( m_ParmIDs[i] );

            if ( p )
            {
                string contID = p->GetContainerID();
                if ( contID.compare( lastContID ) != 0 )
                {
                    lastContID = contID;
                    ParmContainer *pc = ParmMgr.FindParmContainer( contID );
                    if ( pc )
                    {
                        m_AdjustLayout.AddDividerBox( pc->GetName() );
                    }
                }

                m_AdjustLayout.AddSlider( m_ParmSliderVec[i], "AUTO_UPDATE", 10, "%7.3f" );
            }
        }
        m_ParmListHash = str_hash;
    }

    for ( int i = 0; i < m_ParmIDs.size(); i++ )
    {
        Parm *p = ParmMgr.FindParm( m_ParmIDs[i] );

        if ( p )
        {
            m_ParmSliderVec[i].Update( m_ParmIDs[i] );
        }
    }
}

void VarPresetEditorScreen::RebuildMenus()
{
    char str[256];

    int h_pos = m_GroupBrowser->hposition();
    int v_pos = m_GroupBrowser->vposition();
    m_GroupBrowser->clear();

    m_GroupChoice.ClearItems();
    m_GroupIDs.clear();

    m_GroupIDs = VarPresetMgr.GetAllSettingGroups();
    for ( int i = 0; i < m_GroupIDs.size(); i++ )
    {
        SettingGroup *sg = VarPresetMgr.FindSettingGroup( m_GroupIDs[i] );
        if ( sg )
        {
            m_GroupChoice.AddItem( sg->GetName(), i );

            snprintf( str, sizeof( str ), "%s", sg->GetName().c_str() );
            m_GroupBrowser->add( str );
        }
    }
    m_GroupChoice.UpdateItems();

    m_GroupChoice.SetVal( m_GroupIndex );
    m_GroupBrowser->select( m_GroupIndex + 1 );

    m_GroupBrowser->hposition( h_pos );
    m_GroupBrowser->vposition( v_pos );



    h_pos = m_SettingBrowser->hposition();
    v_pos = m_SettingBrowser->vposition();
    m_SettingBrowser->clear();


    m_SettingChoice.ClearItems();
    m_SettingIDs.clear();

    if ( m_GroupIndex >= 0 && m_GroupIndex < m_GroupIDs.size() )
    {
        string gid = m_GroupIDs[ m_GroupIndex ];

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

                    snprintf( str, sizeof( str ), "%s", s->GetName().c_str() );
                    m_SettingBrowser->add( str );
                }
            }
        }
    }
    m_SettingChoice.UpdateItems();

    m_SettingChoice.SetVal( m_SettingIndex );
    m_SettingBrowser->select( m_SettingIndex + 1 );

    m_SettingBrowser->hposition( h_pos );
    m_SettingBrowser->vposition( v_pos );

}

void VarPresetEditorScreen::UpdateVarBrowser()
{
    char str[256];

    //==== Update Parm Browser ====//
    int h_pos = m_VarBrowser->hposition();
    int v_pos = m_VarBrowser->vposition();
    m_VarBrowser->clear();
    m_ParmIDs.clear();

    m_VarBrowser->column_char( ':' );         // use : as the column character

    snprintf( str, sizeof( str ), "@b@.COMP:@b@.GROUP:@b@.PARM" );
    m_VarBrowser->add( str );

    if ( m_GroupIndex >= 0 && m_GroupIndex < m_GroupIDs.size() )
    {
        string gid = m_GroupIDs[ m_GroupIndex ];

        SettingGroup *sg = VarPresetMgr.FindSettingGroup( gid );

        if ( sg )
        {
            m_ParmIDs = sg->GetParmIDVec();
        }
    }

    for ( int i = 0 ; i < m_ParmIDs.size() ; i++ )
    {
        string parmid = m_ParmIDs[ i ];

        string c_name, g_name, p_name;
        ParmMgr.GetNames( parmid, c_name, g_name, p_name );

        snprintf( str, sizeof( str ), "%s:%s:%s", c_name.c_str(), g_name.c_str(), p_name.c_str() );
        m_VarBrowser->add( str );
    }

    if ( m_VarIndex >= 0 && m_VarIndex < m_ParmIDs.size() )
    {
        m_VarBrowser->select( m_VarIndex + 2 );
    }

    m_VarBrowser->hposition( h_pos );
    m_VarBrowser->vposition( v_pos );
}

void VarPresetEditorScreen::CheckSaveStatus()
{
    m_UpdateSettingButton.Deactivate();
    m_UpdateSettingButton.GetFlButton()->label( "Update" );

    if ( m_GroupIndex >= 0 && m_GroupIndex < m_GroupIDs.size() &&
         m_SettingIndex >= 0 && m_SettingIndex < m_SettingIDs.size() )
    {
        string gid = m_GroupIDs[ m_GroupIndex ];
        string sid = m_SettingIDs[ m_SettingIndex ];

        SettingGroup *sg = VarPresetMgr.FindSettingGroup( gid );
        Setting *s = VarPresetMgr.FindSetting( sid );

        if ( sg && s )
        {
            if ( !sg->CheckSetting( sid ) )
            {
                m_UpdateSettingButton.GetFlButton()->label( "*UPDATE*" );
                m_UpdateSettingButton.Activate();
            }

        }
    }
}

void VarPresetEditorScreen::EnableDisableWidgets()
{
    SettingGroup *sg = nullptr;
    Setting *s = nullptr;
    Parm *p = nullptr;

    if ( m_GroupIndex >= 0 && m_GroupIndex < m_GroupIDs.size() )
    {
        string gid = m_GroupIDs[ m_GroupIndex ];
        sg = VarPresetMgr.FindSettingGroup( gid );
    }

    if ( m_SettingIndex >= 0 && m_SettingIndex < m_SettingIDs.size() )
    {
        string sid = m_SettingIDs[ m_SettingIndex ];
        s = VarPresetMgr.FindSetting( sid );
    }

    if ( m_VarIndex >= 0 && m_VarIndex < m_ParmIDs.size() )
    {
        string pid = m_ParmIDs[ m_VarIndex ];
        p = ParmMgr.FindParm( pid );
    }


    if ( m_GroupIDs.empty() )
    {
        m_GroupChoice.Deactivate();
        m_ClearGroupsButton.Deactivate();
    }
    else
    {
        m_GroupChoice.Activate();
        m_ClearGroupsButton.Activate();
    }

    if ( m_SettingIDs.empty() )
    {
        m_SettingChoice.Deactivate();
        m_ClearSettingsButton.Deactivate();
    }
    else
    {
        m_SettingChoice.Activate();
        m_ClearSettingsButton.Activate();
    }

    if ( m_ParmIDs.empty() )
    {
        m_ClearVarsButton.Deactivate();
    }
    else
    {
        m_ClearVarsButton.Activate();
    }

    if ( sg && s )
    {
        m_AddVarButton.Activate();
        m_ParmPicker.Activate();
        m_VarBrowser->activate();
        m_ApplyButton.Activate();
        m_Apply2Button.Activate();
    }
    else
    {
        m_AddVarButton.Deactivate();
        m_ParmPicker.Deactivate();
        m_VarBrowser->deactivate();
        m_ApplyButton.Deactivate();
        m_Apply2Button.Deactivate();
    }

    if ( p )
    {
        m_DelVarButton.Activate();
    }
    else
    {
        m_DelVarButton.Deactivate();
    }

    if ( sg )
    {
        m_DeleteGroupButton.Activate();
        m_RenameGroupButton.Activate();
        m_AddSettingButton.Activate();
    }
    else
    {
        m_DeleteGroupButton.Deactivate();
        m_RenameGroupButton.Deactivate();
        m_AddSettingButton.Deactivate();
    }

    if ( s )
    {
        m_DeleteSettingButton.Activate();
        m_RenameSettingButton.Activate();
    }
    else
    {
        m_DeleteSettingButton.Deactivate();
        m_RenameSettingButton.Deactivate();
    }
}

void VarPresetEditorScreen::Show()
{
    Update();
    m_GroupAttrEditor.Show();
    m_SettingAttrEditor.Show();
    TabScreen::Show();
}

void VarPresetEditorScreen::Hide()
{
    m_FLTK_Window->hide();
}

void VarPresetEditorScreen::CallBack( Fl_Widget* w )
{
    assert( m_ScreenMgr );

    if( Fl::event() == FL_PASTE || Fl::event() == FL_DND_RELEASE )
    {
        string pid( Fl::event_text() );
        m_ParmPicker.SetParmChoice( pid );

        if ( m_GroupIndex >= 0 && m_GroupIndex < m_GroupIDs.size() )
        {
            string gid = m_GroupIDs[ m_GroupIndex ];

            SettingGroup *sg = VarPresetMgr.FindSettingGroup( gid );

            if ( sg )
            {
                sg->AddParm( pid );
            }
        }
    }
    else if (  w == m_VarBrowser )
    {
        m_VarIndex = m_VarBrowser->value() - 2;

        if ( m_VarIndex >= 0 && m_VarIndex < m_ParmIDs.size() )
        {
            m_ParmPicker.SetParmChoice( m_ParmIDs[ m_VarIndex ] );
        }
    }
    else if (  w == m_GroupBrowser )
    {
        m_GroupIndex = m_GroupBrowser->value() - 1;
    }
    else if (  w == m_SettingBrowser )
    {
        m_SettingIndex = m_SettingBrowser->value() - 1;
    }

    m_GroupAttrEditor.DeviceCB( w );
    m_SettingAttrEditor.DeviceCB( w );

    m_ScreenMgr->SetUpdateFlag( true );
}

void VarPresetEditorScreen::CloseCallBack( Fl_Widget *w )
{
    m_ScreenMgr->SetUpdateFlag( true );
    Hide();
}

void VarPresetEditorScreen::GuiDeviceCallBack( GuiDevice* device )
{
    assert( m_ScreenMgr );

    if ( device == &m_AddVarButton )
    {
        if ( m_GroupIndex >= 0 && m_GroupIndex < m_GroupIDs.size() )
        {
            string gid = m_GroupIDs[ m_GroupIndex ];

            SettingGroup *sg = VarPresetMgr.FindSettingGroup( gid );

            if ( sg )
            {
                if ( sg->AddParm( m_ParmPicker.GetParmChoice() ) )
                {
                    m_VarIndex = m_ParmIDs.size();
                }
            }
        }
    }
    else if ( device == &m_DelVarButton )
    {
        if ( m_GroupIndex >= 0 && m_GroupIndex < m_GroupIDs.size() )
        {
            string gid = m_GroupIDs[ m_GroupIndex ];

            SettingGroup *sg = VarPresetMgr.FindSettingGroup( gid );

            if ( sg )
            {
                if ( m_VarIndex >= 0 && m_VarIndex < m_ParmIDs.size() )
                {
                    sg->RemoveParm( m_ParmIDs[ m_VarIndex ] );
                    m_VarIndex = 0;
                }
            }
        }
    }
    else if ( device == &m_ClearVarsButton )
    {
        if ( m_GroupIndex >= 0 && m_GroupIndex < m_GroupIDs.size() )
        {
            string gid = m_GroupIDs[ m_GroupIndex ];

            SettingGroup *sg = VarPresetMgr.FindSettingGroup( gid );

            if ( sg )
            {
                sg->RemoveAllParms();
                m_VarIndex = 0;
            }
        }
    }
    else if ( device == &m_GroupChoice )
    {
        m_GroupIndex = m_GroupChoice.GetVal();
    }
    else if ( device == &m_SettingChoice )
    {
        m_SettingIndex = m_SettingChoice.GetVal();
    }
    else if ( device == &m_AddGroupButton )
    {
        string gname = m_GroupInput.GetString();
        if ( !gname.empty() )
        {
            SettingGroup *sg = new SettingGroup();
            sg->SetName( gname );

            if ( VarPresetMgr.AddSettingGroup( sg ) )
            {
                m_GroupIndex = m_GroupIDs.size();
            }
            else
            {
                delete sg;
            }
        }
    }
    else if ( device == &m_AddSettingButton )
    {
        if ( !m_SettingInput.GetString().empty() )
        {
            if ( m_GroupIndex >= 0 && m_GroupIndex < m_GroupIDs.size() )
            {
                string gid = m_GroupIDs[ m_GroupIndex ];

                SettingGroup *sg = VarPresetMgr.FindSettingGroup( gid );

                if ( sg )
                {
                    Setting *s = new Setting();
                    s->SetName( m_SettingInput.GetString() );

                    if ( sg->AddSetting( s, true ) )
                    {
                        m_SettingIndex = m_SettingIDs.size();
                    }
                    else
                    {
                        delete s;
                    }
                }
            }
        }
    }
    else if ( device == &m_DeleteGroupButton )
    {
        if ( m_GroupIndex >= 0 && m_GroupIndex < m_GroupIDs.size() )
        {
            string gid = m_GroupIDs[ m_GroupIndex ];

            VarPresetMgr.DeleteSettingGroup( gid );
            m_GroupIndex = 0;
        }
    }
    else if ( device == &m_ClearGroupsButton )
    {
        VarPresetMgr.DeleteAllSettingGroups();
    }
    else if ( device == &m_RenameGroupButton )
    {
        if ( m_GroupIndex >= 0 && m_GroupIndex < m_GroupIDs.size() )
        {
            string gid = m_GroupIDs[ m_GroupIndex ];

            SettingGroup *sg = VarPresetMgr.FindSettingGroup( gid );

            if ( sg )
            {
                sg->SetName( m_GroupInput.GetString() );
            }
        }
    }
    else if ( device == &m_DeleteSettingButton )
    {
        if ( m_GroupIndex >= 0 && m_GroupIndex < m_GroupIDs.size() &&
             m_SettingIndex >= 0 && m_SettingIndex < m_SettingIDs.size() )
        {
            string gid = m_GroupIDs[ m_GroupIndex ];
            string sid = m_SettingIDs[ m_SettingIndex ];

            VarPresetMgr.DeleteSetting( gid, sid );
            m_SettingIndex = 0;
        }
    }
    else if ( device == &m_ClearSettingsButton )
    {
        if ( m_GroupIndex >= 0 && m_GroupIndex < m_GroupIDs.size() )
        {
            string gid = m_GroupIDs[ m_GroupIndex ];

            VarPresetMgr.DeleteAllSettingsInGroup( gid );
        }
    }
    else if ( device == &m_RenameSettingButton )
    {
        if ( m_SettingIndex >= 0 && m_SettingIndex < m_SettingIDs.size() )
        {
            string sid = m_SettingIDs[ m_SettingIndex ];
            Setting *s = VarPresetMgr.FindSetting( sid );
            if ( s )
            {
                s->SetName( m_SettingInput.GetString() );
            }
        }
    }
    else if ( device == &m_ApplyButton ||
              device == &m_Apply2Button )
    {
        if ( m_GroupIndex >= 0 && m_GroupIndex < m_GroupIDs.size() &&
             m_SettingIndex >= 0 && m_SettingIndex < m_SettingIDs.size() )
        {
            string gid = m_GroupIDs[ m_GroupIndex ];
            string sid = m_SettingIDs[ m_SettingIndex ];

            SettingGroup *sg = VarPresetMgr.FindSettingGroup( gid );

            if ( sg )
            {
                sg->ApplySetting( sid );

                Vehicle* veh = VehicleMgr.GetVehicle();
                if ( veh )
                {
                    veh->Update();
                }
            }
        }
    }
    else if ( device == &m_UpdateSettingButton )
    {
        if ( m_GroupIndex >= 0 && m_GroupIndex < m_GroupIDs.size() &&
             m_SettingIndex >= 0 && m_SettingIndex < m_SettingIDs.size() )
        {
            string gid = m_GroupIDs[ m_GroupIndex ];
            string sid = m_SettingIDs[ m_SettingIndex ];

            SettingGroup *sg = VarPresetMgr.FindSettingGroup( gid );

            if ( sg )
            {
                sg->SaveSetting( sid );
            }
        }
    }
    else
    {
        return;
    }

    m_GroupAttrEditor.GuiDeviceCallBack( device );
    m_SettingAttrEditor.GuiDeviceCallBack( device );

    m_ScreenMgr->SetUpdateFlag( true );
}

void VarPresetEditorScreen::GetCollIDs( vector < string > &collIDVec )
{
    m_GroupAttrEditor.GetCollIDs( collIDVec );
    m_SettingAttrEditor.GetCollIDs( collIDVec );
}
