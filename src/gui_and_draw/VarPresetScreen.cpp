//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// ParmDebugScreen.cpp: implementation of the ParmDebugScreen class.
//
//////////////////////////////////////////////////////////////////////

#include "VarPresetScreen.h"
#include "ParmMgr.h"
#include "StlHelper.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

VarPresetScreen::VarPresetScreen( ScreenMgr* mgr ) : TabScreen( mgr, 300, 600, "Variable Presets", "", (-6 * 20) - 25) // 6 rows * 20 StdHeight of UI objects, 25 Height of Tabs
{
    // Hidden Var Tree Tab until complete
    //Fl_Group* tree_tab = AddTab( "Var Tree" );
    //( (Vsp_Group*) tree_tab )->SetAllowDrop( true );
    //tree_tab->callback( staticScreenCB, this );

    Fl_Group* pick_tab = AddTab( "Create" );
    ( (Vsp_Group*) pick_tab )->SetAllowDrop( true );
    pick_tab->callback( staticScreenCB, this );

    Fl_Group* adj_tab = AddTab( "Adjust" );
    Fl_Group* pick_group = AddSubGroup( pick_tab, 5 );
    m_AdjustGroup = AddSubScroll( adj_tab, 5 );
    m_AdjustGroup->type( Fl_Scroll::VERTICAL_ALWAYS );

    // Set up change state persistent area at bottom/top? of GUI.
    m_ChangeStateLayout.SetGroupAndScreen( m_FLTK_Window, this );

    m_ChangeStateLayout.AddY(2 * m_ChangeStateLayout.GetGapHeight()
        + m_ChangeStateLayout.GetDividerHeight() );

    m_ChangeStateLayout.SetChoiceButtonWidth( m_ChangeStateLayout.GetButtonWidth() );

    m_ChangeStateLayout.AddDividerBox( "Select Preset" );
    m_ChangeStateLayout.AddChoice( m_GroupChoice, "Group" );
    m_ChangeStateLayout.AddChoice( m_SettingChoice, "Setting");
    m_ChangeStateLayout.AddButton( m_ApplyButton, "Apply" );
    m_ChangeStateLayout.AddYGap();

    m_ChangeStateLayout.AddDividerBox( "Edit" );

    m_ChangeStateLayout.SetButtonWidth( ( m_ChangeStateLayout.GetRemainX() ) / 3 );
    m_ChangeStateLayout.SetSameLineFlag( true );
    m_ChangeStateLayout.SetFitWidthFlag( false );
    m_ChangeStateLayout.AddButton( m_SaveButton , "Save Changes" );
    m_ChangeStateLayout.AddButton( m_DeleteGroupButton, "Delete Group" );
    m_ChangeStateLayout.AddButton( m_DeleteSettingButton, "Delete Setting" );

    // Everything Relevant to "Pick" Tab
    m_PickLayout.SetGroupAndScreen( pick_group, this );

    m_PickLayout.AddDividerBox( "Create Preset" );
    m_PickLayout.SetSameLineFlag( true );
    m_PickLayout.AddInput( m_GroupInput, "Group", m_PickLayout.GetButtonWidth() );
    m_PickLayout.SetFitWidthFlag( false );
    m_PickLayout.AddButton( m_AddGroupButton, "Add" );
    m_PickLayout.ForceNewLine();

    m_PickLayout.SetFitWidthFlag( true );
    m_PickLayout.AddInput( m_SettingInput, "Setting", m_PickLayout.GetButtonWidth() );
    m_PickLayout.SetFitWidthFlag( false );
    m_PickLayout.AddButton( m_AddSettingButton, "Add" );
    m_PickLayout.ForceNewLine();
    m_PickLayout.SetSameLineFlag( false );
    m_PickLayout.SetFitWidthFlag( true );
    m_PickLayout.AddYGap();

    m_PickLayout.AddDividerBox( "Variable" );

    m_PickLayout.SetChoiceButtonWidth( m_PickLayout.GetButtonWidth() );

    m_PickLayout.AddParmPicker( m_ParmPicker );

    m_PickLayout.SetFitWidthFlag( false );
    m_PickLayout.SetSameLineFlag( true );

    m_PickLayout.AddYGap();

    m_PickLayout.SetButtonWidth( ( m_PickLayout.GetRemainX() ) / 2 );
    m_PickLayout.AddButton( m_AddVarButton, "Add Variable" );
    m_PickLayout.AddButton( m_DelVarButton, "Delete Variable" );

    m_PickLayout.ForceNewLine();

    m_PickLayout.SetFitWidthFlag( true );
    m_PickLayout.SetSameLineFlag( false );

    m_PickLayout.AddYGap();

    m_PickLayout.AddDividerBox( "Variable List" );

    // Pointer for the widths of each column in the browser to support resizing
    // Last column width must be 0
    static int col_widths[] = { 100, 100, 90, 0 }; // widths for each column

    int browser_h = 210;
    m_VarBrowser = m_PickLayout.AddColResizeBrowser( col_widths, 3, browser_h );
    m_VarBrowser->callback( staticScreenCB, this );

    pick_tab->show();

    // Everything Relevant to "Adjust" Tab
    m_AdjustLayout.SetGroupAndScreen( m_AdjustGroup, this );

    m_GroupChoiceIndex = 0;
    m_SettingChoiceIndex = 0;
    m_SelectedVarIndex = 0;

    m_ParmListHash = 0;
}

VarPresetScreen::~VarPresetScreen()
{
}

bool VarPresetScreen::Update()
{
    TabScreen::Update();

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

void VarPresetScreen::RebuildAdjustTab()
{
    // Serialize m_ParmIDs into single long string.
    string str = string_vec_serialize( m_ParmIDs );
    // Calculate hash to detect changes in m_ParmIDs
    std::size_t str_hash = std::hash < std::string >{}( str );

    // Relies on currency of m_ParmIDs by UpdateVarBrowser()
    if ( str_hash != m_ParmListHash )
    {
        m_AdjustGroup->clear();
        m_AdjustLayout.SetGroup( m_AdjustGroup );
        m_AdjustLayout.InitWidthHeightVals();

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

void VarPresetScreen::RebuildMenus()
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

void VarPresetScreen::UpdateVarBrowser()
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

    if ( m_GroupChoiceIndex >= 0 && m_GroupChoiceIndex < m_GroupIDs.size() )
    {
        string gid = m_GroupIDs[ m_GroupChoiceIndex ];

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

    if ( m_SelectedVarIndex >= 0 && m_SelectedVarIndex < m_ParmIDs.size() )
    {
        m_VarBrowser->select( m_SelectedVarIndex + 2 );
    }

    m_VarBrowser->hposition( h_pos );
    m_VarBrowser->vposition( v_pos );
}

void VarPresetScreen::CheckSaveStatus()
{
    m_SaveButton.Deactivate();
    m_SaveButton.GetFlButton()->label( "Save Changes" );

    if ( m_GroupChoiceIndex >= 0 && m_GroupChoiceIndex < m_GroupIDs.size() &&
         m_SettingChoiceIndex >= 0 && m_SettingChoiceIndex < m_SettingIDs.size() )
    {
        string gid = m_GroupIDs[ m_GroupChoiceIndex ];
        string sid = m_SettingIDs[ m_SettingChoiceIndex ];

        SettingGroup *sg = VarPresetMgr.FindSettingGroup( gid );

        if ( sg )
        {
            if ( !sg->CheckSetting( sid ) )
            {
                m_SaveButton.GetFlButton()->label( "*SAVE CHANGES*" );
                m_SaveButton.Activate();
            }

        }
    }
}

void VarPresetScreen::EnableDisableWidgets()
{
    SettingGroup *sg = NULL;
    Setting *s = NULL;
    Parm *p = NULL;

    if ( m_GroupChoiceIndex >= 0 && m_GroupChoiceIndex < m_GroupIDs.size() )
    {
        string gid = m_GroupIDs[ m_GroupChoiceIndex ];
        sg = VarPresetMgr.FindSettingGroup( gid );
    }

    if ( m_SettingChoiceIndex >= 0 && m_SettingChoiceIndex < m_SettingIDs.size() )
    {
        string sid = m_SettingIDs[ m_SettingChoiceIndex ];
        s = VarPresetMgr.FindSetting( sid );
    }

    if ( m_SelectedVarIndex >= 0 && m_SelectedVarIndex < m_ParmIDs.size() )
    {
        string pid = m_ParmIDs[ m_SelectedVarIndex ];
        p = ParmMgr.FindParm( pid );
    }


    if ( m_GroupIDs.empty() )
    {
        m_GroupChoice.Deactivate();
    }
    else
    {
        m_GroupChoice.Activate();
    }

    if ( m_SettingIDs.empty() )
    {
        m_SettingChoice.Deactivate();
    }
    else
    {
        m_SettingChoice.Activate();
    }

    if ( sg && s )
    {
        m_AddVarButton.Activate();
        m_ParmPicker.Activate();
        m_VarBrowser->activate();
    }
    else
    {
        m_AddVarButton.Deactivate();
        m_ParmPicker.Deactivate();
        m_VarBrowser->deactivate();
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
    }
    else
    {
        m_DeleteGroupButton.Deactivate();
    }

    if ( s )
    {
        m_DeleteSettingButton.Activate();
    }
    else
    {
        m_DeleteSettingButton.Deactivate();
    }
}

void VarPresetScreen::Show()
{
    Update();
    TabScreen::Show();
}

void VarPresetScreen::Hide()
{
    m_FLTK_Window->hide();
}

void VarPresetScreen::CallBack( Fl_Widget* w )
{
    assert( m_ScreenMgr );

    if( Fl::event() == FL_PASTE || Fl::event() == FL_DND_RELEASE )
    {
        string pid( Fl::event_text() );
        m_ParmPicker.SetParmChoice( pid );

        if ( m_GroupChoiceIndex >= 0 && m_GroupChoiceIndex < m_GroupIDs.size() )
        {
            string gid = m_GroupIDs[ m_GroupChoiceIndex ];

            SettingGroup *sg = VarPresetMgr.FindSettingGroup( gid );

            if ( sg )
            {
                sg->AddParm( pid );
            }
        }
    }
    else if (  w == m_VarBrowser )
    {
        m_SelectedVarIndex = m_VarBrowser->value() - 2;

        if ( m_SelectedVarIndex >= 0 && m_SelectedVarIndex < m_ParmIDs.size() )
        {
            m_ParmPicker.SetParmChoice( m_ParmIDs[ m_SelectedVarIndex ] );
        }
    }

    m_ScreenMgr->SetUpdateFlag( true );
}

void VarPresetScreen::GuiDeviceCallBack( GuiDevice* device )
{
    assert( m_ScreenMgr );

    if ( device == &m_AddVarButton )
    {
        if ( m_GroupChoiceIndex >= 0 && m_GroupChoiceIndex < m_GroupIDs.size() )
        {
            string gid = m_GroupIDs[ m_GroupChoiceIndex ];

            SettingGroup *sg = VarPresetMgr.FindSettingGroup( gid );

            if ( sg )
            {
                if ( sg->AddParm( m_ParmPicker.GetParmChoice() ) )
                {
                    m_SelectedVarIndex = m_ParmIDs.size();
                }
            }
        }
    }
    else if ( device == &m_DelVarButton )
    {
        if ( m_GroupChoiceIndex >= 0 && m_GroupChoiceIndex < m_GroupIDs.size() )
        {
            string gid = m_GroupIDs[ m_GroupChoiceIndex ];

            SettingGroup *sg = VarPresetMgr.FindSettingGroup( gid );

            if ( sg )
            {
                if ( m_SelectedVarIndex >= 0 && m_SelectedVarIndex < m_ParmIDs.size() )
                {
                    sg->RemoveParm( m_ParmIDs[ m_SelectedVarIndex ] );
                    m_SelectedVarIndex = 0;
                }
            }
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
    else if ( device == &m_AddGroupButton )
    {
        string gname = m_GroupInput.GetString();
        if ( !gname.empty() )
        {
            SettingGroup *sg = new SettingGroup();
            sg->SetName( gname );

            if ( VarPresetMgr.AddSettingGroup( sg ) )
            {
                m_GroupChoiceIndex = m_GroupIDs.size();
                m_GroupInput.Update( "" );
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
            if ( m_GroupChoiceIndex >= 0 && m_GroupChoiceIndex < m_GroupIDs.size() )
            {
                string gid = m_GroupIDs[ m_GroupChoiceIndex ];

                SettingGroup *sg = VarPresetMgr.FindSettingGroup( gid );

                if ( sg )
                {
                    Setting *s = new Setting();
                    s->SetName( m_SettingInput.GetString() );

                    if ( sg->AddSetting( s, true ) )
                    {
                        m_SettingChoiceIndex = m_SettingIDs.size();
                        m_SettingInput.Update( "" );
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
        if ( m_GroupChoiceIndex >= 0 && m_GroupChoiceIndex < m_GroupIDs.size() )
        {
            string gid = m_GroupIDs[ m_GroupChoiceIndex ];

            SettingGroup *sg = VarPresetMgr.FindSettingGroup( gid );

            if ( sg )
            {
                VarPresetMgr.RemoveSettingGroup( sg );
                vector < string > settings = sg->GetSettingIDVec();
                for ( int i = 0; i < settings.size(); i++ )
                {
                    Setting *s = VarPresetMgr.FindSetting( settings[i] );
                    if ( s )
                    {
                        VarPresetMgr.RemoveSetting( s );

                        delete s;
                    }
                }
                delete sg;
                m_GroupChoiceIndex = 0;
            }
        }
    }
    else if ( device == &m_DeleteSettingButton )
    {
        if ( m_GroupChoiceIndex >= 0 && m_GroupChoiceIndex < m_GroupIDs.size() &&
             m_SettingChoiceIndex >= 0 && m_SettingChoiceIndex < m_SettingIDs.size() )
        {
            string gid = m_GroupIDs[ m_GroupChoiceIndex ];
            string sid = m_SettingIDs[ m_SettingChoiceIndex ];

            SettingGroup *sg = VarPresetMgr.FindSettingGroup( gid );

            if ( sg )
            {
                Setting *s = VarPresetMgr.FindSetting( sid );
                if ( s )
                {
                    sg->RemoveSetting( s );

                    delete s;
                    m_SettingChoiceIndex = 0;
                }
            }
        }
    }
    else if ( device == &m_ApplyButton )
    {
        if ( m_GroupChoiceIndex >= 0 && m_GroupChoiceIndex < m_GroupIDs.size() &&
             m_SettingChoiceIndex >= 0 && m_SettingChoiceIndex < m_SettingIDs.size() )
        {
            string gid = m_GroupIDs[ m_GroupChoiceIndex ];
            string sid = m_SettingIDs[ m_SettingChoiceIndex ];

            SettingGroup *sg = VarPresetMgr.FindSettingGroup( gid );

            if ( sg )
            {
                sg->ApplySetting( sid );
            }
        }
    }
    else if ( device == &m_SaveButton )
    {
        if ( m_GroupChoiceIndex >= 0 && m_GroupChoiceIndex < m_GroupIDs.size() &&
             m_SettingChoiceIndex >= 0 && m_SettingChoiceIndex < m_SettingIDs.size() )
        {
            string gid = m_GroupIDs[ m_GroupChoiceIndex ];
            string sid = m_SettingIDs[ m_SettingChoiceIndex ];

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

    m_ScreenMgr->SetUpdateFlag( true );
}
