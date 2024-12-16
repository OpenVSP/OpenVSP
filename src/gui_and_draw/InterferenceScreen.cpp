//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#include "InterferenceScreen.h"
#include "InterferenceMgr.h"
#include "ScreenMgr.h"
#include "ParmMgr.h"
#include <FL/fl_ask.H>



//==== Constructor ====//
InterferenceScreen::InterferenceScreen( ScreenMgr* mgr ) : BasicScreen( mgr, 600, 400, "Interference Checks" )
{
    m_GenLayout.SetGroupAndScreen( m_FLTK_Window, this );

    m_GenLayout.ForceNewLine();
    m_GenLayout.AddY(7);
    m_GenLayout.AddX(5);

    m_GenLayout.AddSubGroupLayout( m_BorderLayout, m_GenLayout.GetRemainX() - 5.0,
                                   m_GenLayout.GetRemainY() - 5.0);


    // Pointer for the widths of each column in the browser to support resizing
    // Last column width must be 0
    static int out_col_widths[] = { 160, 85, 80, 85, 0 }; // widths for each column

    m_InterferenceCheckBrowser = m_BorderLayout.AddColResizeBrowser( out_col_widths, 4, 200 );
    m_InterferenceCheckBrowser->callback( staticScreenCB, this );
    m_InterferenceCheckBrowser->type( FL_MULTI_BROWSER );


    m_BorderLayout.SetButtonWidth( m_BorderLayout.GetW() / 3 );

    m_BorderLayout.SetSameLineFlag( true );
    m_BorderLayout.SetFitWidthFlag( false );
    m_BorderLayout.AddButton( m_AddInterferenceCheck, "Add" );
    m_BorderLayout.AddButton( m_DelInterferenceCheck, "Del" );
    m_BorderLayout.AddButton( m_DelAllInterferenceChecks, "Del All" );

    m_BorderLayout.ForceNewLine();
    m_BorderLayout.SetSameLineFlag( false );
    m_BorderLayout.SetFitWidthFlag( true );

    m_BorderLayout.AddYGap();
    m_BorderLayout.AddDividerBox( "Interference Check" );

    m_BorderLayout.AddInput( m_ICNameInput, "Name" );

    m_InterferenceBrowserSelect = -1;
}

InterferenceScreen::~InterferenceScreen()
{
}

//==== Update Screen ====//
bool InterferenceScreen::Update()
{
    BasicScreen::Update();

    Vehicle *veh = VehicleMgr.GetVehicle();

    InterferenceMgr.Update();

    UpdateInterferenceCheckBrowser();


    InterferenceCase* icase = InterferenceMgr.GetInterferenceCase( m_InterferenceBrowserSelect );

    if ( icase )
    {
        m_ICNameInput.Activate();

        m_ICNameInput.Update( icase->GetName() );
    }
    else
    {
        m_ICNameInput.Deactivate();
    }

    m_FLTK_Window->redraw();
    return true;
}

void InterferenceScreen::UpdateInterferenceCheckBrowser()
{
    char str[512];

    int scroll_pos = m_InterferenceCheckBrowser->vposition();
    int h_pos = m_InterferenceCheckBrowser->hposition();
    m_InterferenceCheckBrowser->clear();
    m_InterferenceCheckBrowser->column_char( ':' );

    snprintf( str, sizeof( str ),  "@b@.VAR_NAME:@b@.PARM:@b@.GROUP:@b@.CONTAINER" );
    m_InterferenceCheckBrowser->add( str );
    m_InterferenceCheckBrowser->addCopyText( "header" );

    vector < InterferenceCase* > icases = InterferenceMgr.GetAllInterferenceCases();
    for ( int i = 0 ; i < (int)icases.size() ; i++ )
    {
        snprintf( str, sizeof( str ),  "%s: : : \n", icases[i]->GetName().c_str() );
        m_InterferenceCheckBrowser->add( str );
    }
    if ( m_InterferenceBrowserSelect >= 0 && m_InterferenceBrowserSelect < (int)icases.size() )
    {
        m_InterferenceCheckBrowser->select( m_InterferenceBrowserSelect + 2 );
    }

    m_InterferenceCheckBrowser->vposition( scroll_pos );
    m_InterferenceCheckBrowser->hposition( h_pos );
}

//==== Show Screen ====//
void InterferenceScreen::Show()
{
    m_ScreenMgr->SetUpdateFlag( true );
    BasicScreen::Show();
}


//==== Hide Screen ====//
void InterferenceScreen::Hide()
{
    m_FLTK_Window->hide();
    m_ScreenMgr->SetUpdateFlag( true );
}

//==== Callbacks ====//
void InterferenceScreen::CallBack( Fl_Widget *w )
{
    Vehicle *veh = VehicleMgr.GetVehicle();

    if ( w == m_InterferenceCheckBrowser )
    {
        int sel = m_InterferenceCheckBrowser->value();
        m_InterferenceBrowserSelect = sel - 2;
    }
    else
    {
        return;
    }

    m_ScreenMgr->SetUpdateFlag( true );
}

//==== Gui Device CallBacks ====//
void InterferenceScreen::GuiDeviceCallBack( GuiDevice* gui_device )
{
    assert( m_ScreenMgr );

    Vehicle *veh = VehicleMgr.GetVehicle();

    InterferenceCase* icase = InterferenceMgr.GetInterferenceCase( m_InterferenceBrowserSelect );

    if ( gui_device == &m_AddInterferenceCheck )
    {
        string id = InterferenceMgr.AddInterferenceCase();
        m_InterferenceBrowserSelect = InterferenceMgr.GetInterferenceCaseIndex( id );
    }
    else if ( gui_device == &m_DelInterferenceCheck )
    {
        InterferenceMgr.DeleteInterferenceCase( m_InterferenceBrowserSelect );
    }
    else if ( gui_device == &m_DelAllInterferenceChecks )
    {
        InterferenceMgr.DeleteAllInterferenceCases();
    }
    else if ( gui_device == & m_ICNameInput )
    {
        if ( icase )
        {
            icase->SetName( m_ICNameInput.GetString() );
        }
    }
    else
    {
        return;
    }

    m_ScreenMgr->SetUpdateFlag( true );
}
