//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#include "AdvLinkScreen.h"
#include "AdvLinkMgr.h"
#include "ScreenMgr.h"
#include "ParmMgr.h"

#include <FL/fl_ask.H>


//==== Constructor ====//
AdvLinkScreen::AdvLinkScreen( ScreenMgr* mgr ) : BasicScreen( mgr, 829, 645, "Advanced Parameter Links" )
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

    m_BigGroup.SetButtonWidth(110);
    m_BigGroup.AddYGap();
    m_BigGroup.AddInput( m_VarNameInput, "Var Name:" );
    m_BigGroup.AddYGap();

    int gap = 4;
    m_BigGroup.SetFitWidthFlag( false );
    m_BigGroup.SetSameLineFlag( true );
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

    // Pointer for the widths of each column in the browser to support resizing
    // Last column width must be 0
    static int in_col_widths[] = { 160, 85, 80, 85, 0 }; // widths for each column

    m_InputBrowser = m_InputGroup.AddColResizeBrowser( in_col_widths, 4, 100 );
    m_InputBrowser->callback( staticScreenCB, this );

    m_InputGroup.SetFitWidthFlag( false );
    m_InputGroup.SetSameLineFlag( true );
    m_InputGroup.AddX( 20 );
    m_InputGroup.AddButton( m_DelInput, "Del" );
    m_InputGroup.AddX( 20 );
    m_InputGroup.AddButton( m_DelAllInput, "Del All" );

    m_OutputGroup.AddDividerBox("Output Parms");

    // Pointer for the widths of each column in the browser to support resizing
    // Last column width must be 0
    static int out_col_widths[] = { 160, 85, 80, 85, 0 }; // widths for each column

    m_OutputBrowser = m_OutputGroup.AddColResizeBrowser( out_col_widths, 4, 100 );
    m_OutputBrowser->callback( staticScreenCB, this );

    m_OutputGroup.SetFitWidthFlag( false );
    m_OutputGroup.SetSameLineFlag( true );
    m_OutputGroup.AddX( 20 );
    m_OutputGroup.AddButton( m_DelOutput, "Del" );
    m_OutputGroup.AddX( 20 );
    m_OutputGroup.AddButton( m_DelAllOutput, "Del All" );

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
        m_DelOutput.Activate();
        m_DelAllOutput.Activate();
        m_CompileCode.Activate();
        m_SaveCode.Activate();
        m_ReadCode.Activate();
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
        m_DelOutput.Deactivate();
        m_DelAllOutput.Deactivate();
        m_CompileCode.Deactivate();
        m_SaveCode.Deactivate();
        m_ReadCode.Deactivate();
    }

    //==== Update Parm Picker ====//
    m_ParmPicker.Update();

    //==== Update Link Browser ====//
    m_LinkBrowser->clear();
    vector< AdvLink* > link_vec = AdvLinkMgr.GetLinks();
    for ( int i = 0 ; i < (int)link_vec.size() ; i++ )
    {
        sprintf( str, "  %s", link_vec[i]->GetName().c_str() );
        m_LinkBrowser->add( str );
    }

    //==== Set Current Link ====//
    if ( edit_link_index >= 0 && edit_link_index < (int)link_vec.size() )
    {
        m_LinkBrowser->select( edit_link_index + 1 );
    }

    //==== Input and Output Browsers ====//
    int input_h_pos = m_InputBrowser->hposition();
    int output_h_pos = m_OutputBrowser->hposition();
    int input_v_pos = m_InputBrowser->position();
    int output_v_pos = m_OutputBrowser->position();
    m_InputBrowser->clear();
    m_OutputBrowser->clear();
    if ( edit_link )
    {
        m_InputBrowser->column_char( ':' );

        sprintf( str, "@b@.VAR_NAME:@b@.PARM:@b@.GROUP:@b@.CONTAINER" );
        m_InputBrowser->add( str );

        vector< VarDef > input_vars = edit_link->GetInputVars();
        for ( int i = 0 ; i < (int)input_vars.size() ; i++ )
        {
            string c_name, g_name, p_name;
            ParmMgr.GetNames( input_vars[i].m_ParmID, c_name, g_name, p_name );
            sprintf( str, "%s:%s:%s:%s\n", input_vars[i].m_VarName.c_str(), p_name.c_str(), g_name.c_str(), c_name.c_str() );
            m_InputBrowser->add( str );
        }
        if ( m_InputBrowserSelect >= 0 && m_InputBrowserSelect < (int)input_vars.size() )
        {
            m_InputBrowser->select( m_InputBrowserSelect + 2 );
        }

        m_OutputBrowser->column_char( ':' );

        sprintf( str, "@b@.VAR_NAME:@b@.PARM:@b@.GROUP:@b@.CONTAINER" );
        m_OutputBrowser->add( str );

        vector< VarDef > output_vars = edit_link->GetOutputVars();
        for ( int i = 0 ; i < (int)output_vars.size() ; i++ )
        {
            string c_name, g_name, p_name;
            ParmMgr.GetNames( output_vars[i].m_ParmID, c_name, g_name, p_name );
            sprintf( str, "%s:%s:%s:%s\n", output_vars[i].m_VarName.c_str(), p_name.c_str(), g_name.c_str(), c_name.c_str() );
            m_OutputBrowser->add( str );
        }
        if ( m_OutputBrowserSelect >= 0 && m_OutputBrowserSelect < (int)output_vars.size() )
        {
            m_OutputBrowser->select( m_OutputBrowserSelect + 2 );
        }
    }

    m_InputBrowser->hposition( input_h_pos );
    m_OutputBrowser->hposition( output_h_pos );
    m_InputBrowser->position( input_v_pos );
    m_OutputBrowser->position( output_v_pos );

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
    m_FLTK_Window->show();
}


//==== Hide Screen ====//
void AdvLinkScreen::Hide()
{
    m_FLTK_Window->hide();
    m_ScreenMgr->SetUpdateFlag( true );
}

//==== Callbacks ====//
void AdvLinkScreen::CallBack( Fl_Widget *w )
{

    int edit_link_index = AdvLinkMgr.GetEditLinkIndex();
    AdvLink* edit_link = AdvLinkMgr.GetLink(edit_link_index);

    if ( w == m_LinkBrowser )
    {
        int sel = m_LinkBrowser->value();
        AdvLinkMgr.SetEditLinkIndex( sel - 1 );
    }
    else if ( w == m_InputBrowser )
    {
        m_InputBrowserSelect = m_InputBrowser->value() - 2;

        vector <VarDef> ivars = edit_link->GetInputVars();
        m_VarNameInput.Update( ivars[ m_InputBrowserSelect ].m_VarName );
        m_ParmPicker.SetParmChoice( ivars[ m_InputBrowserSelect ].m_ParmID );
    }
    else if ( w == m_OutputBrowser )
    {
        m_OutputBrowserSelect = m_OutputBrowser->value() - 2;

        vector <VarDef> ovars = edit_link->GetOutputVars();
        m_VarNameInput.Update(ovars[ m_OutputBrowserSelect ].m_VarName );
        m_ParmPicker.SetParmChoice(ovars[ m_OutputBrowserSelect ].m_ParmID );
    }
    else if ( w == m_InputGroup.GetGroup() || w == m_OutputGroup.GetGroup() )
    {
        if ( Fl::event() == FL_PASTE || Fl::event() == FL_DND_RELEASE )
        {
            if ( edit_link )
            {
                if ( edit_link->DuplicateVarName( m_VarNameInput.GetString() ) )
                {
                    m_ScreenMgr->Alert( "Duplicate Var Name" );
                }
                else if ( m_VarNameInput.GetString() == "" )
                {
                    m_ScreenMgr->Alert( "Invalid Var Name" );
                }
                else if ( w == m_InputGroup.GetGroup() )
                {
                    string ParmID( Fl::event_text() );
                    AdvLinkMgr.AddInput( ParmID, m_VarNameInput.GetString() );
                }
                else if ( w == m_OutputGroup.GetGroup() )
                {
                    string ParmID( Fl::event_text() );
                    AdvLinkMgr.AddOutput( ParmID, m_VarNameInput.GetString() );
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

    int edit_link_index = AdvLinkMgr.GetEditLinkIndex();
    AdvLink* edit_link = AdvLinkMgr.GetLink(edit_link_index);

    if ( gui_device == &m_PickInput ||  gui_device == &m_PickOutput )
    {
        if ( edit_link )
        {
            if ( edit_link->DuplicateVarName( m_VarNameInput.GetString() ) )
            {
                m_ScreenMgr->Alert( "Duplicate Var Name" );
            }
            else if ( m_VarNameInput.GetString() == "" )
            {
                m_ScreenMgr->Alert( "Invalid Var Name" );
            }
            else if ( gui_device == &m_PickInput )
            {
                AdvLinkMgr.AddInput(  m_ParmPicker.GetParmChoice(), m_VarNameInput.GetString() );
            }
            else
            {
                AdvLinkMgr.AddOutput(  m_ParmPicker.GetParmChoice(), m_VarNameInput.GetString() );
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
    else if ( gui_device == &m_SaveCode )
    {
        if ( edit_link )
        {
            string newfile = m_ScreenMgr->GetSelectFileScreen()->FileChooser( "Select Code File", "*.vspcode" );
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
            string newfile = m_ScreenMgr->GetSelectFileScreen()->FileChooser( "Select Code File", "*.vspcode" );
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
