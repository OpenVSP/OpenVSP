//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// ParmLinkScreen.cpp: The programmatic implementation of the Parameter Link GUI
//
//////////////////////////////////////////////////////////////////////

#include "ParmLinkScreen.h"
#include "ParmMgr.h"
#include "LinkMgr.h"

ParmLinkScreen::ParmLinkScreen( ScreenMgr* mgr ) : BasicScreen( mgr, 600, 615, "Parm Link: ( A * Scale + Offset = B )" )
{
    m_MainLayout.SetGroupAndScreen( m_FLTK_Window, this );
    m_MainLayout.AddY( 25 );
    m_MainLayout.AddX( 5 );

    m_MainLayout.AddSubGroupLayout( m_GenLayout, m_MainLayout.GetRemainX() - 5.0,
                                    m_MainLayout.GetRemainY() - 5.0);

    m_GenLayout.SetFitWidthFlag( true );
    m_GenLayout.SetSameLineFlag( false );

    // Add Parameter Picker subgroups
    m_GenLayout.AddSubGroupLayout( m_ParmAGroup, m_GenLayout.GetW() / 2 - 52, 80 );
    m_GenLayout.AddX( m_GenLayout.GetW() / 2 - 49 );
    m_GenLayout.AddSubGroupLayout( m_LinkGroup, 100, 80 );
    m_GenLayout.AddX( 102 );
    m_GenLayout.AddSubGroupLayout( m_ParmBGroup, m_GenLayout.GetW() / 2 - 52, 75 );

    // Add content to Parameter Picker subgroups
    m_ParmAGroup.AddDividerBox( "Parm A" );
    m_ParmAGroup.AddParmPicker( m_ParmAPicker );
    ( (Vsp_Group*) m_ParmAGroup.GetGroup() )->SetAllowDrop( true );
    m_ParmAGroup.GetGroup()->callback( staticScreenCB, this );

    m_ParmBGroup.AddDividerBox( "Parm B" );
    m_ParmBGroup.AddParmPicker( m_ParmBPicker );
    ( (Vsp_Group*) m_ParmBGroup.GetGroup() )->SetAllowDrop( true );
    m_ParmBGroup.GetGroup()->callback( staticScreenCB, this );


    m_LinkGroup.AddY( 17 );
    m_LinkGroup.AddButton( m_LinkConts, "Link All" );
    m_LinkGroup.AddButton( m_LinkGroups, "Link All" );

    m_GenLayout.ForceNewLine();
    m_GenLayout.AddY( 65 );

    m_GenLayout.SetButtonWidth( 62 );

    // Add 'Offfset' On/Off toggle button and slider
    m_GenLayout.SetFitWidthFlag( false );
    m_GenLayout.SetSameLineFlag( true );
    m_GenLayout.AddButton( m_OffsetTog, "Offset" );
    m_GenLayout.SetFitWidthFlag( true );
    m_GenLayout.AddSlider( m_OffsetSlider, "Offset", 100, " %7.6f" );
    m_GenLayout.ForceNewLine();
    m_GenLayout.SetSameLineFlag( false );

    m_GenLayout.AddY( 5 );

    // Add 'Scale' On/Off toggle button and slider
    m_GenLayout.SetFitWidthFlag( false );
    m_GenLayout.SetSameLineFlag( true );
    m_GenLayout.AddButton( m_ScaleTog, "Scale" );
    m_GenLayout.SetFitWidthFlag( true );
    m_GenLayout.AddSlider( m_ScaleSlider, "Scale", 2, " %7.6f" );
    m_GenLayout.ForceNewLine();
    m_GenLayout.SetSameLineFlag( false );

    m_GenLayout.AddY( 10 );
    m_GenLayout.AddDividerBox( "Constraints" );

    // Set 'Lower' On/Off toggle button and slider
    m_GenLayout.SetFitWidthFlag( false );
    m_GenLayout.SetSameLineFlag( true );
    m_GenLayout.AddButton( m_LowerTog, "Lower" );
    m_GenLayout.SetFitWidthFlag( true );
    m_GenLayout.AddSlider( m_LowerLimitSlider, "Lower", 10.0, "%7.6f" );
    m_GenLayout.ForceNewLine();
    m_GenLayout.SetSameLineFlag( false );

    m_GenLayout.AddY( 5 );

    // Set 'Upper' On/Off toggle button and slider
    m_GenLayout.SetFitWidthFlag( false );
    m_GenLayout.SetSameLineFlag( true );
    m_GenLayout.AddButton( m_UpperTog, "Upper" );
    m_GenLayout.SetFitWidthFlag( true );
    m_GenLayout.AddSlider( m_UpperLimitSlider, "Upper", 10.0, "%7.6f" );
    m_GenLayout.ForceNewLine();
    m_GenLayout.SetSameLineFlag( false );

    m_GenLayout.AddY( 10 );
    m_GenLayout.AddSubGroupLayout( m_ConstraintsButtons, m_GenLayout.GetW(), 70 );

    // Set the Add and Delete buttons under 'Constraints'
    m_ConstraintsButtons.SetFitWidthFlag( false );
    m_ConstraintsButtons.SetSameLineFlag( true );
    m_ConstraintsButtons.SetButtonWidth( 186 );
    m_ConstraintsButtons.AddButton( m_AddLink, "Add Link" );
    m_ConstraintsButtons.AddX( 15 );
    m_ConstraintsButtons.AddButton( m_DeleteLink, "Delete Link" );
    m_ConstraintsButtons.AddX( 15 );
    m_ConstraintsButtons.AddButton( m_DeleteAllLinks, "Delete All Links" );

    m_ConstraintsButtons.ForceNewLine();
    m_ConstraintsButtons.AddYGap();

    m_ConstraintsButtons.AddX( 93-15 );
    m_ConstraintsButtons.AddButton( m_ASort, "Sort by A" );
    m_ConstraintsButtons.AddX( 60 );
    m_ConstraintsButtons.AddButton( m_BSort, "Sort by B" );

    // Place the Parm Link List box
    m_GenLayout.ForceNewLine();
    m_GenLayout.AddY( 30 );

    // Add Link Browser
    m_GenLayout.AddDividerBox( "Parm Link List" );

    // Pointer for the widths of each column in the browser to support resizing
    // Last column width must be 0
    static int col_widths[] = { 90, 90, 105, 20, 90, 90, 105, 0 }; // widths for each column

    m_LinkBrowser = m_GenLayout.AddColResizeBrowser( col_widths, 7, 310 );
    m_LinkBrowser->callback( staticScreenCB, this );
}

ParmLinkScreen::~ParmLinkScreen()
{
}

void ParmLinkScreen::Show()
{
    BasicScreen::Show();
    m_ScreenMgr->SetUpdateFlag( true );

}

bool ParmLinkScreen::Update()
{
    int i;
    char str[256];

    LinkMgr.CheckLinks();
    if ( ParmMgr.GetDirtyFlag() )
    {
        LinkMgr.BuildLinkableParmData();
    }
    Link* currLink = LinkMgr.GetCurrLink();

    m_ParmAPicker.SetParmChoice( currLink->GetParmA() );
    m_ParmBPicker.SetParmChoice( currLink->GetParmB() );

    //==== Update Parm Picker ====//
    m_ParmAPicker.Update();
    m_ParmBPicker.Update();


    //===== Update Offset ====//
    m_OffsetSlider.Update( currLink->m_Offset.GetID() );
    if ( currLink->GetOffsetFlag() )
    {
        m_OffsetTog.GetFlButton()->value( 1 );
        m_OffsetSlider.Activate();
    }
    else
    {
        m_OffsetTog.GetFlButton()->value( 0 );
        m_OffsetSlider.Deactivate();
    }

    //===== Update Scale ====//
    m_ScaleSlider.Update( currLink->m_Scale.GetID() );
    if ( currLink->GetScaleFlag() )
    {
        m_ScaleTog.GetFlButton()->value( 1 );
        m_ScaleSlider.Activate();
    }
    else
    {
        m_ScaleTog.GetFlButton()->value( 0 );
        m_ScaleSlider.Deactivate();
    }


    //===== Update Lower Limit ====//
    m_LowerLimitSlider.Update ( currLink->m_LowerLimit.GetID() );
    if ( currLink->GetLowerLimitFlag() )
    {
        m_LowerTog.GetFlButton()->value( 1 );
        m_LowerLimitSlider.Activate();
    }
    else
    {
        m_LowerTog.GetFlButton()->value( 0 );
        m_LowerLimitSlider.Deactivate();
    }

    //===== Update Upper Limit ====//
    m_UpperLimitSlider.Update( currLink->m_UpperLimit.GetID() );
    if ( currLink->GetUpperLimitFlag() )
    {
        m_UpperTog.GetFlButton()->value( 1 );
        m_UpperLimitSlider.Activate();
    }
    else
    {
        m_UpperTog.GetFlButton()->value( 0 );
        m_UpperLimitSlider.Deactivate();
    }

    //==== Update Link Browser ====//
    int h_pos = m_LinkBrowser->hposition();
    int v_pos = m_LinkBrowser->position();
    m_LinkBrowser->clear();
    m_LinkBrowser->column_char( ':' );        // use : as the column character

    sprintf( str, "@b@.COMP_A:@b@.GROUP:@b@.PARM:->:@b@.COMP_B:@b@.GROUP:@b@.PARM" );
    m_LinkBrowser->add( str );

    int num_links = LinkMgr.GetNumLinks();
    for ( i = 0 ; i < num_links ; i++ )
    {
        Link* pl = LinkMgr.GetLink( i );

        string c_name_A, g_name_A, p_name_A;
        string c_name_B, g_name_B, p_name_B;
        ParmMgr.GetNames( pl->GetParmA(), c_name_A, g_name_A, p_name_A );
        ParmMgr.GetNames( pl->GetParmB(), c_name_B, g_name_B, p_name_B );

        sprintf( str, "%s:%s:%s:->:%s:%s:%s",
                 c_name_A.c_str(), g_name_A.c_str(), p_name_A.c_str(),
                 c_name_B.c_str(), g_name_B.c_str(), p_name_B.c_str() );

        m_LinkBrowser->add( str );
    }

    int index = LinkMgr.GetCurrLinkIndex();
    if ( index >= 0 && index < num_links )
    {
        m_LinkBrowser->select( index + 2 );
    }

    m_LinkBrowser->hposition( h_pos );
    m_LinkBrowser->position( v_pos );

    m_FLTK_Window->redraw();

    return true;
}

// Callback for Link Browser and Drag & Drop
void ParmLinkScreen::CallBack( Fl_Widget* w )
{
    assert( m_ScreenMgr );

    if ( w == m_LinkBrowser )
    {
        int sel = m_LinkBrowser->value();
        LinkMgr.SetCurrLinkIndex( sel - 2 );
    }
    // Check that Parm is dropped on top of ParmPicker A
    else if ( w == ( m_ParmAGroup.GetGroup() ) && Fl::event_inside( w ) )
    {
        if( Fl::event() == FL_PASTE || Fl::event() == FL_DND_RELEASE )
        {
            string ParmID( Fl::event_text() );
            LinkMgr.SetCurrLinkIndex( -1 );
            LinkMgr.SetParm( true, ParmID );
        }
    }
    // Check that Parm is dropped on top of ParmPicker B
    else if ( w == ( m_ParmBGroup.GetGroup() ) )
    {
        if( Fl::event() == FL_PASTE || Fl::event() == FL_DND_RELEASE )
        {
            string ParmID( Fl::event_text() );
            LinkMgr.SetCurrLinkIndex( -1 );
            LinkMgr.SetParm( false, ParmID );
        }
    }

    m_ScreenMgr->SetUpdateFlag( true );
}

// Callback for all other GUI Devices
void ParmLinkScreen::GuiDeviceCallBack( GuiDevice* device )
{
    assert( m_ScreenMgr );
    Link* currLink = LinkMgr.GetCurrLink();

    if( device == &m_ParmAPicker || device == &m_ParmBPicker )
    {
        LinkMgr.SetParm( true, m_ParmAPicker.GetParmChoice() );
        LinkMgr.SetParm( false, m_ParmBPicker.GetParmChoice() );
        LinkMgr.SetCurrLinkIndex( -1 );
    }
    else if( device == &m_AddLink )
    {
        bool success = LinkMgr.AddCurrLink();
        if ( !success )
        {
            fl_alert( "Error: Identical Parms or Already Linked" );
        }
    }
    else if ( device == &m_DeleteLink )
    {
        LinkMgr.DelCurrLink();
    }
    else if ( device == &m_DeleteAllLinks )
    {
        LinkMgr.DelAllLinks();
    }
    else if ( device == &m_LinkConts )
    {
        bool success = LinkMgr.LinkAllComp();
        if ( !success )
        {
            fl_alert( "Error: Identical Comps" );
        }
    }
    else if ( device == &m_LinkGroups )
    {
        bool success = LinkMgr.LinkAllGroup();
        if ( !success )
        {
            fl_alert( "Error: Identical Group" );
        }
    }
    else if ( device == &m_OffsetTog )
    {
        if ( m_OffsetTog.GetFlButton()->value() )
        {
            currLink->SetOffsetFlag( true );
        }
        else
        {
            currLink->SetOffsetFlag( false );
        }

        m_OffsetSlider.Activate();
        if ( !currLink->GetOffsetFlag() )
        {
            m_OffsetSlider.Deactivate();
        }

        LinkMgr.ParmChanged( currLink->GetParmA(), true );
    }
    else if ( device == &m_ScaleTog )
    {
        if ( m_ScaleTog.GetFlButton()->value() )
        {
            currLink->SetScaleFlag( true );
        }
        else
        {
            currLink->SetScaleFlag( false );
        }

        m_ScaleSlider.Activate();
        if ( !currLink->GetScaleFlag() )
        {
            m_ScaleSlider.Deactivate();
        }
        LinkMgr.ParmChanged( currLink->GetParmA(), true );
    }
    else if ( device == &m_LowerTog )
    {
        if ( m_LowerTog.GetFlButton()->value() )
        {
            currLink->SetLowerLimitFlag( true );
        }
        else
        {
            currLink->SetLowerLimitFlag( false );
        }

        m_LowerLimitSlider.Activate();
        if ( !currLink->GetLowerLimitFlag() )
        {
            m_LowerLimitSlider.Deactivate();
        }
        LinkMgr.ParmChanged( currLink->GetParmA(), true );
    }
    else if ( device == &m_UpperTog )
    {
        if ( m_UpperTog.GetFlButton()->value() )
        {
            currLink->SetUpperLimitFlag( true );
        }
        else
        {
            currLink->SetUpperLimitFlag( false );
        }

        m_UpperLimitSlider.Activate();
        if ( !currLink->GetUpperLimitFlag() )
        {
            m_UpperLimitSlider.Deactivate();
        }
        LinkMgr.ParmChanged( currLink->GetParmA(), true );
    }
    else if ( device == &m_ASort )
    {
        LinkMgr.SortLinksByA();
    }
    else if ( device == &m_BSort )
    {
        LinkMgr.SortLinksByB();
    }

    m_ScreenMgr->SetUpdateFlag( true );
}
