//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#include "AdvLinkScreen.h"
#include "AdvLinkMgr.h"
#include "ScreenMgr.h"
#include "CustomGeom.h"
#include "LinkMgr.h"
#include "ParmMgr.h"

#include <assert.h>

//==== Constructor ====//
AdvLinkScreen::AdvLinkScreen( ScreenMgr* mgr ) : VspScreen( mgr )
{
    m_InputBrowserSelect = -1;
    m_OutputBrowserSelect = -1;

    AdvLinkUI* ui = m_AdvLinkUI = new AdvLinkUI();
    VspScreen::SetFlWindow( ui->UIWindow );

    m_BigGroup.SetGroupAndScreen( ui->windowGroup, this );
    m_BigGroup.AddDividerBox( "Adv Links" );

    m_BigGroup.AddSubGroupLayout( m_LinkAddDelGroup, 100 - 2, 80 );
    m_BigGroup.AddX( 100 + 2 );
    m_BigGroup.AddSubGroupLayout( m_LinkBrowserGroup, m_BigGroup.GetW() - 100 - 2, 80 );

    m_LinkAddDelGroup.AddButton( m_AddLink, "Add" );
    m_LinkAddDelGroup.AddYGap();
    m_LinkAddDelGroup.AddButton( m_DelLink, "Del" );
    m_LinkAddDelGroup.AddYGap();
    m_LinkAddDelGroup.AddButton( m_DelAllLink, "Del_All" );
    m_LinkAddDelGroup.AddYGap();

    m_LinkBrowser = m_LinkBrowserGroup.AddFlBrowser( 70 );
    m_LinkBrowser->callback( staticScreenCB, this );

    m_LinkBrowserGroup.AddInput( m_NameInput, "Name:" );

    m_BigGroup.ForceNewLine();
    m_BigGroup.AddY( 70 );


    //m_BigGroup.SetFitWidthFlag( false );
    //m_BigGroup.SetSameLineFlag( true );
    //m_BigGroup.SetButtonWidth(90);
    //m_BigGroup.AddButton( m_AddLink, "Add" );
    //m_BigGroup.AddX( 20 );
    //m_BigGroup.SetFitWidthFlag( true );
    //m_BigGroup.AddInput( m_NameInput, "Link Name:" );
    //m_BigGroup.ForceNewLine();
    //m_BigGroup.SetSameLineFlag( false );

    //m_LinkBrowser = m_BigGroup.AddFlBrowser( 80 );
    //m_LinkBrowser->callback( staticScreenCB, this );

    //m_BigGroup.SetFitWidthFlag( false );
    //m_BigGroup.SetSameLineFlag( true );

    //int gap = 20;
    //m_BigGroup.AddButton( m_DelLink, "Del" );
    //m_BigGroup.AddX( gap );
    //m_BigGroup.AddButton( m_DelAllLink, "Del_All" );
    //m_BigGroup.AddX( gap );
    //m_BigGroup.AddButton( m_SaveLink, "Save File..." );
    //m_BigGroup.AddX( gap );
    //m_BigGroup.AddButton( m_LoadLink, "Read File..." );
    //m_BigGroup.ForceNewLine();

    //m_BigGroup.SetFitWidthFlag( true );
    //m_BigGroup.SetSameLineFlag( false );

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
    m_BigGroup.AddButton( m_PickInput, "Add_Input" );
    m_BigGroup.AddX( gap );
    m_BigGroup.AddButton( m_PickOutput, "Add_Output" );
    m_BigGroup.SetFitWidthFlag( true );
    m_BigGroup.SetSameLineFlag( false );
    m_BigGroup.ForceNewLine();
    m_BigGroup.AddYGap();

    m_BigGroup.AddSubGroupLayout( m_InputGroup, m_BigGroup.GetW() / 2 - 2, 150 );
    m_BigGroup.AddX( m_BigGroup.GetW() / 2 + 2 );
    m_BigGroup.AddSubGroupLayout( m_OutputGroup, m_BigGroup.GetW() / 2 - 2, 150 );

    m_InputGroup.AddDividerBox("Input Parms");
    m_InputBrowser = m_InputGroup.AddFlBrowser( 100 );
    m_InputBrowser->callback( staticScreenCB, this );

    m_InputGroup.SetFitWidthFlag( false );
    m_InputGroup.SetSameLineFlag( true );
    m_InputGroup.AddX( 20 );
    m_InputGroup.AddButton( m_DelInput, "Del" );
    m_InputGroup.AddX( 20 );
    m_InputGroup.AddButton( m_DelAllInput, "Del All" );

    m_OutputGroup.AddDividerBox("Output Parms");
    m_OutputBrowser = m_OutputGroup.AddFlBrowser( 100 );
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

    //==== Code Editor ====//
    m_CodeEditor = m_BigGroup.AddFlTextEditor( 190 );
    m_CodeEditor->textcolor( FL_DARK_RED );

    m_CodeBuffer = new Fl_Text_Buffer;
    m_CodeEditor->buffer( m_CodeBuffer );
    m_CodeEditor->textfont( FL_COURIER );

    m_CodeBuffer->add_modify_callback(staticTextCB, this);
    m_CodeBuffer->call_modify_callbacks();
    m_CodeBuffer->text( "" );

    m_BigGroup.SetFitWidthFlag( false );
    m_BigGroup.SetSameLineFlag( true );
    m_BigGroup.AddButton( m_SaveCode, "Save Code..." );
    m_BigGroup.AddX( 2 );
    m_BigGroup.AddButton( m_ReadCode, "Read Code..." );


}

//==== Update Screen ====//
bool AdvLinkScreen::Update()
{
    char str[512];

    int edit_link_index = AdvLinkMgr.GetEditLinkIndex();
    AdvLink* edit_link = AdvLinkMgr.GetLink(edit_link_index);

    if ( edit_link )
    {
        m_NameInput.Update( edit_link->GetName() );
    }
    else
    {
        m_NameInput.Update( "" );
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
    m_InputBrowser->clear();
    m_OutputBrowser->clear();
    if ( edit_link )
    {
        static int widths[] = { 75, 75, 75, 75, 0 };
        m_InputBrowser->column_widths( widths );
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

        m_OutputBrowser->column_widths( widths );
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

    if ( edit_link )
    {
        if ( edit_link->GetScriptCode() != m_CodeBuffer->text() )
        {
            m_CodeBuffer->text( edit_link->GetScriptCode().c_str() );

            if ( edit_link->ValidScript() )
                m_CodeEditor->textcolor( FL_BLACK );
            else
                m_CodeEditor->textcolor( FL_DARK_RED );
        }
    }
    else
    {
        m_CodeEditor->textcolor( FL_DARK_RED );
        if ( m_CodeBuffer->text() != "" )
            m_CodeBuffer->text( "" );
    }


    return true;
}

//==== Show Screen ====//
void AdvLinkScreen::Show()
{
    Update();
    m_FLTK_Window->show();
}


//==== Hide Screen ====//
void AdvLinkScreen::Hide()
{
    m_FLTK_Window->hide();
}

//==== Callbacks ====//
void AdvLinkScreen::CallBack( Fl_Widget *w )
{
    if ( w == m_LinkBrowser )
    {
        int sel = m_LinkBrowser->value();
        AdvLinkMgr.SetEditLinkIndex( sel - 1 );
    }
    else if ( w == m_InputBrowser )
    {
        m_InputBrowserSelect = m_InputBrowser->value() - 2;
    }
    else if ( w == m_OutputBrowser )
    {
        m_OutputBrowserSelect = m_OutputBrowser->value() - 2;
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
            string newfile = m_ScreenMgr->GetSelectFileScreen()->FileChooser( "Select (.as) File", "*.as" );
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
            string newfile = m_ScreenMgr->GetSelectFileScreen()->FileChooser( "Select (.as) File", "*.as" );
            if ( newfile.size() > 0 )
            {
                edit_link->ReadCode( newfile );
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
    m_CodeEditor->textcolor( FL_DARK_RED );
    if ( nInserted > 0 )
    {
        char c = m_CodeBuffer->char_at( pos );
        if ( c == 10 )                              // Return
        {
            if ( edit_link )
            {
                edit_link->SetScriptCode( m_CodeBuffer->text() );
                bool valid_script = edit_link->BuildScript();
                if ( valid_script )
                    m_CodeEditor->textcolor( FL_BLACK );
            }
            m_ScreenMgr->SetUpdateFlag( true );
        }
    }

}
