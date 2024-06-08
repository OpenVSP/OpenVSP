//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#include "AdvLinkScreen.h"
#include "AdvLinkVarRenameScreen.h"
#include "AdvLinkMgr.h"
#include "ScreenMgr.h"
#include "ParmMgr.h"
#include <FL/fl_ask.H>



//==== Constructor ====//
AdvLinkScreen::AdvLinkScreen( ScreenMgr* mgr ) : BasicScreen( mgr, 829, 645, "Advanced Parameter Links", "AdvLink.html" )
{
    m_InputBrowserSelect = -1;
    m_OutputBrowserSelect = -1;

    m_Lt_Red = fl_rgb_color( 250, 150, 150 );

    m_BigGroup.SetGroupAndScreen( m_FLTK_Window, this );
    m_BigGroup.AddY( 25 );

    m_BigGroup.AddSubGroupLayout( m_LinkAddDelGroup, 100 - 2, 80 );
    m_BigGroup.AddX( 100 + 2 );
    m_BigGroup.AddSubGroupLayout( m_LinkBrowserGroup, m_BigGroup.GetW() - 100 - 2, 80 );

    m_LinkAddDelGroup.AddButton( m_AddLink, "Add" );
    m_LinkAddDelGroup.AddYGap();
    m_LinkAddDelGroup.AddButton( m_DelLink, "Del" );
    m_LinkAddDelGroup.AddYGap();
    m_LinkAddDelGroup.AddButton( m_DelAllLink, "Del All" );
    m_LinkAddDelGroup.AddYGap();

    m_LinkBrowser = m_LinkBrowserGroup.AddFlBrowser( 70 );
    m_LinkBrowser->callback( staticScreenCB, this );

    m_LinkBrowserGroup.AddInput( m_NameInput, "Name:" );

    m_BigGroup.ForceNewLine();
    m_BigGroup.AddY( 70 );

    m_BigGroup.AddYGap();
    m_BigGroup.AddDividerBox( "Parm Picker" );

    m_BigGroup.AddParmPicker( m_ParmPicker );

    int gap = 4;

    m_BigGroup.SetButtonWidth(110);
    m_BigGroup.SetInputWidth( m_BigGroup.GetW() - ( 4 * m_BigGroup.GetW() / 10.0 ) - m_BigGroup.GetButtonWidth() - gap );
    m_BigGroup.AddYGap();

    m_BigGroup.SetFitWidthFlag( false );
    m_BigGroup.SetSameLineFlag( true );

    m_BigGroup.AddInput( m_VarNameInput, "Var Name:" );
    m_BigGroup.AddX( gap );

    m_BigGroup.SetButtonWidth( ( m_BigGroup.GetW() - gap ) / 10.0 );

    m_BigGroup.AddButton( m_GenDefNameToggle, "Auto" );
    m_BigGroup.AddButton( m_DefNameContainerToggle, "Container" );
    m_BigGroup.AddButton( m_DefNameGroupToggle, "Group" );
    m_BigGroup.AddButton( m_DefNameParmDummy, "Parm" );
    m_BigGroup.ForceNewLine();

    m_BigGroup.AddYGap();

    m_BigGroup.SetButtonWidth( m_BigGroup.GetRemainX()/2 - gap/2 );
    m_BigGroup.AddButton( m_PickInput, "Add Input Var" );
    m_BigGroup.AddX( gap );
    m_BigGroup.AddButton( m_PickOutput, "Add Output Var" );
    m_BigGroup.SetFitWidthFlag( true );
    m_BigGroup.SetSameLineFlag( false );
    m_BigGroup.ForceNewLine();
    m_BigGroup.AddYGap();

    m_BigGroup.AddSubGroupLayout( m_InputGroup, m_BigGroup.GetW() / 2 - 2, 150 );
    m_BigGroup.AddX( m_BigGroup.GetW() / 2 + 2 );
    m_BigGroup.AddSubGroupLayout( m_OutputGroup, m_BigGroup.GetW() / 2 - 2, 150 );

    ( static_cast <Vsp_Group*> ( m_InputGroup.GetGroup() ) )->SetAllowDrop( true );
    m_InputGroup.GetGroup()->callback( staticScreenCB, this );

    ( static_cast <Vsp_Group*> ( m_OutputGroup.GetGroup() ) )->SetAllowDrop( true );
    m_OutputGroup.GetGroup()->callback( staticScreenCB, this );

    m_InputGroup.AddDividerBox("Input Parms");

    int start_x = m_InputGroup.GetX();
    int start_y = m_InputGroup.GetY();
    int browser_h = 100;

    m_InputGroup.AddSubGroupLayout( m_MoveInputButtonLayout, 20, browser_h );

    m_MoveInputButtonLayout.SetSameLineFlag( false );
    m_MoveInputButtonLayout.SetFitWidthFlag( false );

    m_MoveInputButtonLayout.SetStdHeight( 20 );
    m_MoveInputButtonLayout.SetButtonWidth( 20 );
    m_MoveInputButtonLayout.AddButton( m_MoveInputTopButton, "@2<<" );
    m_MoveInputButtonLayout.AddYGap();
    m_MoveInputButtonLayout.AddButton( m_MoveInputUpButton, "@2<" );
    m_MoveInputButtonLayout.AddY( browser_h - 75 - m_InputGroup.GetStdHeight() );
    m_MoveInputButtonLayout.AddButton( m_MoveInputDownButton, "@2>" );
    m_MoveInputButtonLayout.AddYGap();
    m_MoveInputButtonLayout.AddButton( m_MoveInputBotButton, "@2>>" );

    m_InputGroup.SetY( start_y );
    m_InputGroup.AddX( 20 );
    m_InputGroup.SetFitWidthFlag( true );


    m_InputGroup.AddSubGroupLayout( m_InputBrowserLayout, m_InputGroup.GetRemainX(), browser_h );
    m_InputGroup.AddY( browser_h );


    // Pointer for the widths of each column in the browser to support resizing
    // Last column width must be 0
    static int in_col_widths[] = { 160, 85, 80, 85, 0 }; // widths for each column

    m_InputBrowser = m_InputBrowserLayout.AddColResizeBrowser( in_col_widths, 4, browser_h );
    m_InputBrowser->callback( staticScreenCB, this );
    m_InputBrowser->type( FL_MULTI_BROWSER );

    m_InputGroup.SetX( start_x );

    int space_x = ( m_InputGroup.GetW() - 20 - 5 * m_InputGroup.GetButtonWidth() ) / 4;

    m_InputGroup.SetFitWidthFlag( false );
    m_InputGroup.SetSameLineFlag( true );

    m_InputGroup.AddX( 20 );
    m_InputGroup.AddButton( m_DelInput, "Del" );
    m_InputGroup.AddX( space_x );
    m_InputGroup.AddButton( m_DelAllInput, "Del All" );
    m_InputGroup.AddX( space_x );
    m_InputGroup.AddButton( m_RenInput, "Rename" );
    m_InputGroup.AddX( space_x );
    m_InputGroup.AddButton( m_SortVarInput, "Sort Var" );
    m_InputGroup.AddX( space_x );
    m_InputGroup.AddButton( m_SortCGPInput, "Sort CGP" );

    m_OutputGroup.AddDividerBox("Output Parms");

    start_x = m_OutputGroup.GetX();
    start_y = m_OutputGroup.GetY();

    m_OutputGroup.AddSubGroupLayout( m_MoveOutputButtonLayout, 20, browser_h );

    m_MoveOutputButtonLayout.SetSameLineFlag( false );
    m_MoveOutputButtonLayout.SetFitWidthFlag( false );

    m_MoveOutputButtonLayout.SetStdHeight( 20 );
    m_MoveOutputButtonLayout.SetButtonWidth( 20 );
    m_MoveOutputButtonLayout.AddButton( m_MoveOutputTopButton, "@2<<" );
    m_MoveOutputButtonLayout.AddYGap();
    m_MoveOutputButtonLayout.AddButton( m_MoveOutputUpButton, "@2<" );
    m_MoveOutputButtonLayout.AddY( browser_h - 75 - m_OutputGroup.GetStdHeight() );
    m_MoveOutputButtonLayout.AddButton( m_MoveOutputDownButton, "@2>" );
    m_MoveOutputButtonLayout.AddYGap();
    m_MoveOutputButtonLayout.AddButton( m_MoveOutputBotButton, "@2>>" );

    m_OutputGroup.SetY( start_y );
    m_OutputGroup.AddX( 20 );
    m_OutputGroup.SetFitWidthFlag( true );


    m_OutputGroup.AddSubGroupLayout( m_OutputBrowserLayout, m_OutputGroup.GetRemainX(), browser_h );
    m_OutputGroup.AddY( browser_h );


    // Pointer for the widths of each column in the browser to support resizing
    // Last column width must be 0
    static int out_col_widths[] = { 160, 85, 80, 85, 0 }; // widths for each column

    m_OutputBrowser = m_OutputBrowserLayout.AddColResizeBrowser( out_col_widths, 4, browser_h );
    m_OutputBrowser->callback( staticScreenCB, this );
    m_OutputBrowser->type( FL_MULTI_BROWSER );

    m_OutputGroup.SetX( start_x );

    m_OutputGroup.SetFitWidthFlag( false );
    m_OutputGroup.SetSameLineFlag( true );
    m_OutputGroup.AddX( 20 );
    m_OutputGroup.AddButton( m_DelOutput, "Del" );
    m_OutputGroup.AddX( space_x );
    m_OutputGroup.AddButton( m_DelAllOutput, "Del All" );
    m_OutputGroup.AddX( space_x );
    m_OutputGroup.AddButton( m_RenOutput, "Rename" );
    m_OutputGroup.AddX( space_x );
    m_OutputGroup.AddButton( m_SortVarOutput, "Sort Var" );
    m_OutputGroup.AddX( space_x );
    m_OutputGroup.AddButton( m_SortCGPOutput, "Sort CGP" );

    m_BigGroup.ForceNewLine();
    m_BigGroup.AddY( 120 );

    m_BigGroup.AddDividerBox("Code");

    m_BigGroup.SetFitWidthFlag( false );
    m_BigGroup.SetSameLineFlag( true );

    m_BigGroup.AddButton( m_CompileCode, "Compile" );
    m_BigGroup.AddX( 12 );
    m_BigGroup.SetButtonWidth(m_BigGroup.GetRemainX()/2 - 3 );
    m_BigGroup.AddButton( m_SaveCode, "File Write..." );
    m_BigGroup.AddX( 6 );
    m_BigGroup.AddButton( m_ReadCode, "File Read..." );
    m_BigGroup.ForceNewLine();
    m_BigGroup.SetFitWidthFlag( true );
    m_BigGroup.SetSameLineFlag( false );

    //==== Code Editor ====//
    m_CodeEditor = m_BigGroup.AddFlTextEditor( 210 );

    m_CodeBuffer = new Fl_Text_Buffer;
    m_CodeEditor->buffer( m_CodeBuffer );
    m_CodeEditor->textfont( FL_COURIER );

    m_CodeBuffer->add_modify_callback(staticTextCB, this);
    m_CodeBuffer->call_modify_callbacks();

    m_CodeBuffer->text( "" );
}

AdvLinkScreen::~AdvLinkScreen()
{
    m_CodeEditor->buffer( NULL );
    delete m_CodeBuffer;
}

//==== Update Screen ====//
bool AdvLinkScreen::Update()
{
    char str[512];

    BasicScreen::Update();

    Vehicle *veh = VehicleMgr.GetVehicle();

    AdvLinkMgr.CheckLinks();

    int edit_link_index = AdvLinkMgr.GetEditLinkIndex();
    AdvLink* edit_link = AdvLinkMgr.GetLink(edit_link_index);

    if ( edit_link )
    {
        m_NameInput.Update( edit_link->GetName() );
        m_NameInput.Activate();
        m_ParmPicker.Activate();
        m_NameInput.Activate();
        m_DelLink.Activate();
        m_DelAllLink.Activate();
        m_VarNameInput.Activate();
        m_PickInput.Activate();
        m_PickOutput.Activate();
        m_DelInput.Activate();
        m_DelAllInput.Activate();
        m_RenInput.Activate();
        m_DelOutput.Activate();
        m_DelAllOutput.Activate();
        m_RenOutput.Activate();
        m_CompileCode.Activate();
        m_SaveCode.Activate();
        m_ReadCode.Activate();
        m_MoveInputUpButton.Activate();
        m_MoveInputDownButton.Activate();
        m_MoveInputTopButton.Activate();
        m_MoveInputBotButton.Activate();
        m_MoveOutputUpButton.Activate();
        m_MoveOutputDownButton.Activate();
        m_MoveOutputTopButton.Activate();
        m_MoveOutputBotButton.Activate();
        m_SortVarInput.Activate();
        m_SortCGPInput.Activate();
        m_SortVarOutput.Activate();
        m_SortCGPOutput.Activate();
        m_GenDefNameToggle.Activate();
        m_DefNameContainerToggle.Activate();
        m_DefNameGroupToggle.Activate();
        m_DefNameParmDummy.Activate();
    }
    else
    {
        m_NameInput.Update( "" );
        m_NameInput.Deactivate();
        m_ParmPicker.Deactivate();
        m_NameInput.Deactivate();
        m_DelLink.Deactivate();
        m_DelAllLink.Deactivate();
        m_VarNameInput.Deactivate();
        m_PickInput.Deactivate();
        m_PickOutput.Deactivate();
        m_DelInput.Deactivate();
        m_DelAllInput.Deactivate();
        m_RenInput.Deactivate();
        m_DelOutput.Deactivate();
        m_DelAllOutput.Deactivate();
        m_RenOutput.Deactivate();
        m_CompileCode.Deactivate();
        m_SaveCode.Deactivate();
        m_ReadCode.Deactivate();
        m_MoveInputUpButton.Deactivate();
        m_MoveInputDownButton.Deactivate();
        m_MoveInputTopButton.Deactivate();
        m_MoveInputBotButton.Deactivate();
        m_MoveOutputUpButton.Deactivate();
        m_MoveOutputDownButton.Deactivate();
        m_MoveOutputTopButton.Deactivate();
        m_MoveOutputBotButton.Deactivate();
        m_SortVarInput.Deactivate();
        m_SortCGPInput.Deactivate();
        m_SortVarOutput.Deactivate();
        m_SortCGPOutput.Deactivate();
        m_GenDefNameToggle.Deactivate();
        m_DefNameContainerToggle.Deactivate();
        m_DefNameGroupToggle.Deactivate();
        m_DefNameParmDummy.Deactivate();
    }

    //==== Update Parm Picker ====//
    m_ParmPicker.Update();

    //==== Update Link Browser ====//
    m_LinkBrowser->clear();
    vector< AdvLink* > link_vec = AdvLinkMgr.GetLinks();
    for ( int i = 0 ; i < (int)link_vec.size() ; i++ )
    {
        snprintf( str, sizeof( str ),  "  %s", link_vec[i]->GetName().c_str() );
        m_LinkBrowser->add( str );
    }

    if( veh )
    {
        m_DefNameContainerToggle.Update( veh->m_AdvLinkDefNameContainer.GetID() );
        m_DefNameGroupToggle.Update( veh->m_AdvLinkDefNameGroup.GetID() );
        m_GenDefNameToggle.Update( veh->m_AdvLinkGenDefName.GetID() );

        if ( veh->m_AdvLinkGenDefName() )
        {
            m_DefNameContainerToggle.Activate();
            m_DefNameGroupToggle.Activate();
            m_DefNameParmDummy.Activate();
        }
        else
        {
            m_DefNameContainerToggle.Deactivate();
            m_DefNameGroupToggle.Deactivate();
            m_DefNameParmDummy.Deactivate();
        }
    }

    //==== Set Current Link ====//
    if ( edit_link_index >= 0 && edit_link_index < (int)link_vec.size() )
    {
        m_LinkBrowser->select( edit_link_index + 1 );
    }

    //==== Input and Output Browsers ====//
    int input_h_pos = m_InputBrowser->hposition();
    int output_h_pos = m_OutputBrowser->hposition();
    int input_v_pos = m_InputBrowser->vposition();
    int output_v_pos = m_OutputBrowser->vposition();
    m_InputBrowser->clear();
    m_OutputBrowser->clear();
    if ( edit_link )
    {
        m_InputBrowser->column_char( ':' );

        snprintf( str, sizeof( str ),  "@b@.VAR_NAME:@b@.PARM:@b@.GROUP:@b@.CONTAINER" );
        m_InputBrowser->add( str );

        vector< VarDef > input_vars = edit_link->GetInputVars();
        for ( int i = 0 ; i < (int)input_vars.size() ; i++ )
        {
            string c_name, g_name, p_name;
            ParmMgr.GetNames( input_vars[i].m_ParmID, c_name, g_name, p_name );
            snprintf( str, sizeof( str ),  "%s:%s:%s:%s\n", input_vars[i].m_VarName.c_str(), p_name.c_str(), g_name.c_str(), c_name.c_str() );
            m_InputBrowser->add( str );
        }
        if ( m_InputBrowserSelect >= 0 && m_InputBrowserSelect < (int)input_vars.size() )
        {
            m_InputBrowser->select( m_InputBrowserSelect + 2 );
        }

        m_OutputBrowser->column_char( ':' );

        snprintf( str, sizeof( str ),  "@b@.VAR_NAME:@b@.PARM:@b@.GROUP:@b@.CONTAINER" );
        m_OutputBrowser->add( str );

        vector< VarDef > output_vars = edit_link->GetOutputVars();
        for ( int i = 0 ; i < (int)output_vars.size() ; i++ )
        {
            string c_name, g_name, p_name;
            ParmMgr.GetNames( output_vars[i].m_ParmID, c_name, g_name, p_name );
            snprintf( str, sizeof( str ),  "%s:%s:%s:%s\n", output_vars[i].m_VarName.c_str(), p_name.c_str(), g_name.c_str(), c_name.c_str() );
            m_OutputBrowser->add( str );
        }
        if ( m_OutputBrowserSelect >= 0 && m_OutputBrowserSelect < (int)output_vars.size() )
        {
            m_OutputBrowser->select( m_OutputBrowserSelect + 2 );
        }
    }

    m_InputBrowser->hposition( input_h_pos );
    m_OutputBrowser->hposition( output_h_pos );
    m_InputBrowser->vposition( input_v_pos );
    m_OutputBrowser->vposition( output_v_pos );

    if ( edit_link )
    {
        if ( edit_link->GetScriptCode() != m_CodeBuffer->text() )
        {
            m_CodeBuffer->text( edit_link->GetScriptCode().c_str() );
        }

        if ( edit_link->ValidScript() )
            m_CompileCode.SetColor( FL_GRAY );
        else
            m_CompileCode.SetColor( m_Lt_Red );
    }
    else
    {
        m_CompileCode.SetColor( FL_GRAY );
        m_CodeBuffer->text( "" );
    }


    m_FLTK_Window->redraw();
    return true;
}

//==== Show Screen ====//
void AdvLinkScreen::Show()
{
    m_ScreenMgr->SetUpdateFlag( true );
    BasicScreen::Show();
}


//==== Hide Screen ====//
void AdvLinkScreen::Hide()
{
    m_FLTK_Window->hide();
    m_ScreenMgr->SetUpdateFlag( true );
}

string AdvLinkScreen::MakeDefaultName( const string & parmid )
{
    string name;
    Vehicle *veh = VehicleMgr.GetVehicle();

    Parm *p = ParmMgr.FindParm( parmid );
    if ( p && veh )
    {
        string c_name, g_name, p_name;
        ParmMgr.GetNames( parmid, c_name, g_name, p_name );

        if ( veh->m_AdvLinkDefNameContainer() )
        {
            name = c_name + "_";
        }

        if ( veh->m_AdvLinkDefNameGroup() )
        {
            name += g_name + "_";
        }

        name += p_name;
    }

    return name;
}

//==== Callbacks ====//
void AdvLinkScreen::CallBack( Fl_Widget *w )
{
    Vehicle *veh = VehicleMgr.GetVehicle();

    int edit_link_index = AdvLinkMgr.GetEditLinkIndex();
    AdvLink* edit_link = AdvLinkMgr.GetLink(edit_link_index);

    if ( w == m_LinkBrowser )
    {
        int sel = m_LinkBrowser->value();
        AdvLinkMgr.SetEditLinkIndex( sel - 1 );
    }
    else if ( w == m_InputBrowser )
    {
        if ( edit_link )
        {
            if ( Fl::event_clicks() > 0 )
            {
                string vname = edit_link->GetInputVarName( m_InputBrowserSelect );
                if ( vname.size() > 0 )
                {
                    Fl::copy( vname.c_str(), vname.size(), 1 );
                }
            }
            else
            {
                int ibs = -1;
                for ( size_t i = 2; i <= m_InputBrowser->size(); i++ )
                {
                    if ( m_InputBrowser->selected( i ) )
                    {
                        ibs = i - 2;
                        break;
                    }
                }

                vector <VarDef> ivars = edit_link->GetInputVars();
                if ( ibs >= 0 && ibs < ivars.size() )
                {
                    m_InputBrowserSelect = ibs;
                    m_VarNameInput.Update( ivars[ m_InputBrowserSelect ].m_VarName );
                    m_ParmPicker.SetParmChoice( ivars[ m_InputBrowserSelect ].m_ParmID );

                    m_OutputBrowserSelect = -1;
                }
                else
                {
                    m_InputBrowserSelect = -1;
                    m_VarNameInput.Update( "" );
                    m_ParmPicker.SetParmChoice( "" );
                }
            }
        }
    }
    else if ( w == m_OutputBrowser )
    {
        if ( edit_link )
        {
            if ( Fl::event_clicks() > 0 )
            {
                string vname = edit_link->GetOutputVarName( m_OutputBrowserSelect );
                if ( vname.size() > 0 )
                {
                    Fl::copy( vname.c_str(), vname.size(), 1 );
                }
            }
            else
            {
                int obs = -1;
                for ( size_t i = 2; i <= m_OutputBrowser->size(); i++ )
                {
                    if ( m_OutputBrowser->selected( i ) )
                    {
                        obs = i - 2;
                        break;
                    }
                }

                vector < VarDef > ovars = edit_link->GetOutputVars();
                if ( obs >= 0 && obs < ovars.size())
                {
                    m_OutputBrowserSelect = obs;
                    m_VarNameInput.Update( ovars[ m_OutputBrowserSelect ].m_VarName );
                    m_ParmPicker.SetParmChoice( ovars[ m_OutputBrowserSelect ].m_ParmID );

                    m_InputBrowserSelect = -1;
                }
                else
                {
                    m_OutputBrowserSelect = -1;
                    m_VarNameInput.Update( "" );
                    m_ParmPicker.SetParmChoice( "" );
                }
            }
        }
    }
    else if ( w == m_InputGroup.GetGroup() || w == m_OutputGroup.GetGroup() )
    {
        if ( Fl::event() == FL_PASTE || Fl::event() == FL_DND_RELEASE )
        {
            string ParmID( Fl::event_text() );

            if ( veh && edit_link )
            {
                if ( veh->m_AdvLinkGenDefName() )
                {
                    Parm *p = ParmMgr.FindParm( ParmID );
                    if ( p )
                    {
                        string name = MakeDefaultName( ParmID );

                        bool cancel = false;
                        if ( edit_link->DuplicateVarName( name ) )
                        {
                            string new_name = edit_link->MakeVarNameUnique( name );

                            if ( 0 == fl_choice( "%s is not unique, use %s instead?", "Cancel", "Use Suggestion",  0, name.c_str(), new_name.c_str() ) )
                            {
                                cancel = true;
                            }
                            name = new_name;
                        }

                        if ( !cancel )
                        {
                            if ( w == m_InputGroup.GetGroup() )
                            {
                                AddInput( ParmID, name );
                            }
                            else if ( w == m_OutputGroup.GetGroup() )
                            {
                                AddOutput( ParmID, name );
                            }
                        }
                    }

                }
                else
                {
                    string name = m_VarNameInput.GetString();

                    if ( name == "" )
                    {
                        m_ScreenMgr->Alert( "Invalid Var Name" );
                    }
                    else
                    {
                        bool cancel = false;
                        if ( edit_link->DuplicateVarName( name ) )
                        {
                            string new_name = edit_link->MakeVarNameUnique( name );

                            if ( 0 == fl_choice( "%s is not unique, use %s instead?", "Cancel", "Use Suggestion",  0, name.c_str(), new_name.c_str() ) )
                            {
                                cancel = true;
                            }
                            name = new_name;
                        }

                        if ( !cancel )
                        {
                            if ( w == m_InputGroup.GetGroup() )
                            {
                                AddInput( ParmID, name );
                            }
                            else if ( w == m_OutputGroup.GetGroup() )
                            {
                                AddOutput( ParmID, name );
                            }
                        }
                    }
                }
            }
        }
    }
    else
    {
        return;
    }

    m_ScreenMgr->SetUpdateFlag( true );
}

//==== Gui Device CallBacks ====//
void AdvLinkScreen::GuiDeviceCallBack( GuiDevice* gui_device )
{
    assert( m_ScreenMgr );

    Vehicle *veh = VehicleMgr.GetVehicle();

    int edit_link_index = AdvLinkMgr.GetEditLinkIndex();
    AdvLink* edit_link = AdvLinkMgr.GetLink(edit_link_index);

    if ( gui_device == &m_PickInput ||  gui_device == &m_PickOutput )
    {
        if ( veh && edit_link )
        {
            string ParmID( m_ParmPicker.GetParmChoice() );

            if ( veh->m_AdvLinkGenDefName() )
            {
                Parm *p = ParmMgr.FindParm( ParmID );
                if ( p )
                {
                    string name = MakeDefaultName( ParmID );

                    bool cancel = false;
                    if ( edit_link->DuplicateVarName( name ) )
                    {
                        string new_name = edit_link->MakeVarNameUnique( name );

                        if ( 0 == fl_choice( "%s is not unique, use %s instead?", "Cancel", "Use Suggestion",  0, name.c_str(), new_name.c_str() ) )
                        {
                            cancel = true;
                        }
                        name = new_name;
                    }

                    if ( !cancel )
                    {
                        if ( gui_device == &m_PickInput )
                        {
                            AddInput( ParmID, name );
                        }
                        else
                        {
                            AddOutput( ParmID, name );
                        }
                    }
                }
            }
            else
            {
                string name = m_VarNameInput.GetString();

                if ( name == "" )
                {
                    m_ScreenMgr->Alert( "Invalid Var Name" );
                }
                else
                {
                    bool cancel = false;
                    if ( edit_link->DuplicateVarName( name ) )
                    {
                        string new_name = edit_link->MakeVarNameUnique( name );

                        if ( 0 == fl_choice( "%s is not unique, use %s instead?", "Cancel", "Use Suggestion",  0, name.c_str(), new_name.c_str() ) )
                        {
                            cancel = true;
                        }
                        name = new_name;
                    }

                    if ( !cancel )
                    {
                        if ( gui_device == &m_PickInput )
                        {
                            AddInput( ParmID, name );
                        }
                        else
                        {
                            AddOutput( ParmID, name );
                        }
                    }
                }
            }
        }
    }
    else if ( gui_device == &m_NameInput )
    {
        if ( edit_link )
        {
            string name = m_NameInput.GetString();
            if ( !AdvLinkMgr.DuplicateLinkName( name ) )
            {
                edit_link->SetName( m_NameInput.GetString() );
            }
        }
    }
    else if ( gui_device == &m_AddLink )
    {
         AdvLinkMgr.AddLink( "" );
    }
    else if ( gui_device == &m_DelLink )
    {
        AdvLinkMgr.DelLink( edit_link );
    }
    else if ( gui_device == &m_DelAllLink )
    {
        AdvLinkMgr.DelAllLinks();
    }
    else if ( gui_device == &m_DelInput )
    {
        if ( edit_link )
        {
            edit_link->DeleteVar( m_InputBrowserSelect, true );
        }
    }
    else if ( gui_device == &m_DelAllInput )
    {
        if ( edit_link )
        {
            edit_link->DeleteAllVars( true );
        }
    }
    else if ( gui_device == &m_RenInput )
    {
        AdvLinkVarRenameScreen * vrScreen = dynamic_cast< AdvLinkVarRenameScreen* >
        ( m_ScreenMgr->GetScreen( vsp::VSP_ADV_LINK_VAR_RENAME_SCREEN ) );

        if ( vrScreen && edit_link )
        {
            vector < VarDef > ivars = edit_link->GetInputVars();
            if ( m_InputBrowserSelect >= 0 && m_InputBrowserSelect < ivars.size() )
            {
                vrScreen->SetupAndShow( m_InputBrowserSelect, ivars[ m_InputBrowserSelect ].m_VarName, true );
            }
        }
    }
    else if ( gui_device == &m_MoveInputUpButton )
    {
        m_InputBrowserSelect = edit_link->ReorderInputVar( m_InputBrowserSelect, vsp::REORDER_MOVE_UP );
    }
    else if ( gui_device == &m_MoveInputDownButton )
    {
        m_InputBrowserSelect = edit_link->ReorderInputVar( m_InputBrowserSelect, vsp::REORDER_MOVE_DOWN );
    }
    else if ( gui_device == &m_MoveInputTopButton )
    {
        m_InputBrowserSelect = edit_link->ReorderInputVar( m_InputBrowserSelect, vsp::REORDER_MOVE_TOP );
    }
    else if ( gui_device == &m_MoveInputBotButton )
    {
        m_InputBrowserSelect = edit_link->ReorderInputVar( m_InputBrowserSelect, vsp::REORDER_MOVE_BOTTOM );
    }
    else if ( gui_device == &m_SortVarInput )
    {
        m_InputBrowserSelect = edit_link->SortInputsVar( m_InputBrowserSelect );
    }
    else if ( gui_device == &m_SortCGPInput )
    {
        m_InputBrowserSelect = edit_link->SortInputsCGP( m_InputBrowserSelect );
    }
    else if ( gui_device == &m_DelOutput )
    {
        if ( edit_link )
        {
            edit_link->DeleteVar( m_OutputBrowserSelect, false );
        }
    }
    else if ( gui_device == &m_DelAllOutput )
    {
        if ( edit_link )
        {
            edit_link->DeleteAllVars( false );
        }
    }
    else if ( gui_device == &m_RenOutput )
    {
        AdvLinkVarRenameScreen * vrScreen = dynamic_cast< AdvLinkVarRenameScreen* >
        ( m_ScreenMgr->GetScreen( vsp::VSP_ADV_LINK_VAR_RENAME_SCREEN ) );

        if ( vrScreen && edit_link )
        {
            vector < VarDef > ovars = edit_link->GetOutputVars();
            if ( m_OutputBrowserSelect >= 0 && m_OutputBrowserSelect < ovars.size() )
            {
                vrScreen->SetupAndShow( m_OutputBrowserSelect, ovars[ m_OutputBrowserSelect ].m_VarName, false );
            }
        }
    }
    else if ( gui_device == &m_MoveOutputUpButton )
    {
        m_OutputBrowserSelect = edit_link->ReorderOutputVar( m_OutputBrowserSelect, vsp::REORDER_MOVE_UP );
    }
    else if ( gui_device == &m_MoveOutputDownButton )
    {
        m_OutputBrowserSelect = edit_link->ReorderOutputVar( m_OutputBrowserSelect, vsp::REORDER_MOVE_DOWN );
    }
    else if ( gui_device == &m_MoveOutputTopButton )
    {
        m_OutputBrowserSelect = edit_link->ReorderOutputVar( m_OutputBrowserSelect, vsp::REORDER_MOVE_TOP );
    }
    else if ( gui_device == &m_MoveOutputBotButton )
    {
        m_OutputBrowserSelect = edit_link->ReorderOutputVar( m_OutputBrowserSelect, vsp::REORDER_MOVE_BOTTOM );
    }
    else if ( gui_device == &m_SortVarOutput )
    {
        m_OutputBrowserSelect = edit_link->SortOutputsVar( m_OutputBrowserSelect );
    }
    else if ( gui_device == &m_SortCGPOutput )
    {
        m_OutputBrowserSelect = edit_link->SortOutputsCGP( m_OutputBrowserSelect );
    }
    else if ( gui_device == &m_SaveCode )
    {
        if ( edit_link )
        {
            string newfile = m_ScreenMgr->FileChooser( "Select Code File", "*.vspcode", vsp::SAVE );
            if ( newfile.size() > 0 )
            {
                edit_link->SaveCode( newfile );
            }
        }
    }
    else if ( gui_device == &m_ReadCode )
    {
        if ( edit_link )
        {
            string newfile = m_ScreenMgr->FileChooser( "Select Code File", "*.vspcode" );
            if ( newfile.size() > 0 )
            {
                edit_link->ReadCode( newfile );
            }
        }
    }

    else if ( gui_device == &m_CompileCode )
    {
        if ( edit_link )
        {
            edit_link->SetScriptCode( m_CodeBuffer->text() );
            bool valid_script = edit_link->BuildScript();
            if ( valid_script )
            {
                edit_link->ForceUpdate();
            }
        }
    }
    else
    {
        return;
    }

    m_ScreenMgr->SetUpdateFlag( true );
}

void AdvLinkScreen::AddInput( const string & parmid, const string & name )
{
    int edit_link_index = AdvLinkMgr.GetEditLinkIndex();
    AdvLink* edit_link = AdvLinkMgr.GetLink(edit_link_index);

    if ( edit_link )
    {
        vector < VarDef > ivars = edit_link->GetInputVars();
        int nbefore = ivars.size();

        AdvLinkMgr.AddInput( parmid, name );

        ivars = edit_link->GetInputVars();
        int nafter = ivars.size();

        if ( nafter > nbefore )
        {
            m_InputBrowserSelect = nafter - 1;
            m_VarNameInput.Update( name );
            m_ParmPicker.SetParmChoice( parmid );

            m_OutputBrowserSelect = -1;
        }
    }
}

void AdvLinkScreen::AddOutput( const string & parmid, const string & name )
{
    int edit_link_index = AdvLinkMgr.GetEditLinkIndex();
    AdvLink* edit_link = AdvLinkMgr.GetLink(edit_link_index);

    if ( edit_link )
    {
        vector < VarDef > ovars = edit_link->GetOutputVars();
        int nbefore = ovars.size();

        AdvLinkMgr.AddOutput(  parmid, name );

        ovars = edit_link->GetOutputVars();
        int nafter = ovars.size();

        if ( nafter > nbefore )
        {
            m_OutputBrowserSelect = nafter - 1;
            m_VarNameInput.Update( name );
            m_ParmPicker.SetParmChoice( parmid );

            m_InputBrowserSelect = -1;
        }
    }
}

void AdvLinkScreen::TextCallBack( int pos, int nInserted, int nDeleted, int nRestyled, const char* deletedText )
{
    AdvLink* edit_link = AdvLinkMgr.GetLink( AdvLinkMgr.GetEditLinkIndex() );

    if ( !edit_link )
        return;

    if ( nInserted || nDeleted || nRestyled )
    {
        if ( edit_link->GetScriptCode() != m_CodeBuffer->text() )
        {
            edit_link->SetScriptCode( m_CodeBuffer->text() );
            edit_link->SetValidScriptFlag( false );
            m_CompileCode.SetColor( m_Lt_Red );
        }
    }
}
