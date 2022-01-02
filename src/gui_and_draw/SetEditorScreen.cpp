//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#include "SetEditorScreen.h"

using namespace vsp;

SetEditorScreen::SetEditorScreen(ScreenMgr* mgr ) : BasicScreen( mgr, 300, 370, "Set Editor" )
{
    //Variables to help get locations of widgets to look nice and clean
    int browserHeight = 200;
    int borderPaddingWidth = 5;
    int yPadding = 7;
    
    //Fl_Window/Fl_Group objects inherited from the Fl_Widget class
    //M-FLTK_Window is a member of the VSPScreen Class, inherited from BasicScreen class, included with ScreenBase.h
    //StaticCloseCB is a callback function ptr, defined in Fl_Widget.h, it sets up the windows close callback funtcions
    m_FLTK_Window->callback( staticCloseCB, this );

    //This helps init m_MainLayouts group and screen functionality
    m_MainLayout.SetGroupAndScreen( m_FLTK_Window, this );
    
    //Initialize variables
    m_CopyIndex = -1;
    m_PasteIndex = -1;
    m_SelectedSetIndex = DEFAULT_SET;

    //This sets position below heading with position at far left 
    m_MainLayout.ForceNewLine();
    //Adds padding on left and top of position
    m_MainLayout.AddY( yPadding );
    m_MainLayout.AddX( borderPaddingWidth );

    //Inserting the m_BorderLayout into m_MainLayout, using remains to get correct alignment
    m_MainLayout.AddSubGroupLayout( m_BorderLayout, m_MainLayout.GetRemainX() - borderPaddingWidth, m_MainLayout.GetRemainY() - borderPaddingWidth);
    //Length of button is half the width of the border layout
    m_BorderLayout.SetButtonWidth(( m_BorderLayout.GetW() / 2)- borderPaddingWidth );
    //Adding a input functionality needs a string object type, and a label name to set it
    m_BorderLayout.AddInput( m_SetNameInput, "Set Name:" );

    //Uses a pre-set value to increase y padding on m_BorderLayout
    m_BorderLayout.AddYGap();

    //Adding 2 subgroups: m_LeftLayout, m_RightLayout, to attach our browsers objects
    //Pass in the new subgroup and its width and height
    m_BorderLayout.AddSubGroupLayout( m_LeftLayout, ( m_BorderLayout.GetW() / 2)- borderPaddingWidth, m_BorderLayout.GetRemainY() );
    //Adds the divider box/label
    m_LeftLayout.AddDividerBox( "Sets" );
    //Adds padding to get correct alignment
    m_BorderLayout.AddX(( m_BorderLayout.GetW() / 2 ) + borderPaddingWidth );
    //Adding the right side subgroup using width and height
    m_BorderLayout.AddSubGroupLayout( m_RightLayout, ( m_BorderLayout.GetW() / 2 ) - borderPaddingWidth, m_BorderLayout.GetRemainY() );
    //Adds the divider box/label
    m_RightLayout.AddDividerBox( "Geom In Set" );

    //==== Adding Browsers to the Layouts ====//
    //To get into position, move y down below dividers
    m_BorderLayout.AddY( 15 );

    //We add the m_setBrowser to the m_LeftLayout
    m_SetBrowser = m_LeftLayout.AddFlBrowser( browserHeight );
    //Need to give it a type
    m_SetBrowser->type( FL_SELECT_BROWSER );

    //We add the m_SetSelectBrowser to the m_RightLayout
    m_SetSelectBrowser = m_RightLayout.AddCheckBrowser( browserHeight );
    //This sets the m_BorderLayout y position to line up with bottom of m_SetSelectBrowser
    m_BorderLayout.AddY( browserHeight );
    //Adds small gap at bottom of the browser areas
    m_BorderLayout.AddYGap();
    //Sets x back to the left side with offset
    m_BorderLayout.SetX( borderPaddingWidth );

    //Use this so we can have 2 buttons on same line
    m_BorderLayout.SetSameLineFlag( true );

    //Add in our copy and select all buttons -- these are unrelated,
    //but this GUI is set up column-wise, so this seems to make more sense.
    m_BorderLayout.AddButton(m_CopySet, "Copy Set Members", m_BorderLayout.GetW() / 2);
    m_BorderLayout.AddButton(m_SelectAll, "Add All Geoms", m_BorderLayout.GetW() / 2);

    //starts a new line
    m_BorderLayout.ForceNewLine();

    //Complete the columns of buttons with paste and unselect
    m_BorderLayout.AddButton(m_PasteSet, "Paste Set Members", m_BorderLayout.GetW() / 2);
    m_BorderLayout.AddButton(m_UnselectAll, "Remove All Geoms", m_BorderLayout.GetW() / 2);

    //add highlight button on the bottom of screen
    m_BorderLayout.ForceNewLine();
    m_BorderLayout.AddButton(m_HighlightSet, "Select Set" );

    m_BorderLayout.AddYGap();

    //add our toggle for copying geoms with their sets
    m_BorderLayout.ForceNewLine();
    m_BorderLayout.AddButton(m_CopySetToggle, "Copy Set Membership When Copying Geoms");

    //Browser objects need to have there static callbacks set in SetEditorScreen's constructor
    m_SetBrowser->callback( staticScreenCB, this );
    m_SetSelectBrowser->callback( staticScreenCB, this );
}

//==== Update Screen ====//

//If we have events from callbacks this is were we update logic based on any changes
bool SetEditorScreen::Update()
{
    assert(m_ScreenMgr);

    Vehicle* vehiclePtr = m_ScreenMgr->GetVehiclePtr();

    int setBrowserScrollPosition = m_SetBrowser->position();
    int setSelectBrowserScrollPosition = m_SetSelectBrowser->position();

    //In case browsers has leftover items, clear them out
    m_SetBrowser->clear();
    m_SetSelectBrowser->clear();

    //When index is pointing to geom sets that are not editable, we deactivate the buttons and input
    if ( m_SelectedSetIndex <= SET_NOT_SHOWN )
    {
        m_SetNameInput.Deactivate();
    }
    else
    {
        m_SetNameInput.Activate();
    }
    
    //===== Update Copy Set Toggle =====//
    m_CopySetToggle.Update(vehiclePtr->m_CopySetsWithGeomsFlag.GetID());

    //This helps keep the PasteSet button deactivated when user should not be able to paste sets
    if ( m_CopyIndex > SET_ALL && m_SelectedSetIndex > SET_ALL )
    {
        m_PasteSet.Activate();
    }
    else
    {
        m_PasteSet.Deactivate();
    }

    //==== Load Set Names and Values ====//

    //This is a vector that has all the names of the sets
    vector< string > set_name_vec = vehiclePtr->GetSetNameVec();
    //We iterate thru sets to be checked using ENUM
    for ( int i = SET_SHOWN ; i < ( int )set_name_vec.size() ; i++ )
    {
        //Adds the name to browser list after casting string to char* 
        m_SetBrowser->add( set_name_vec[i].c_str() );
    }

    //Updates what should be selected by utilizing m_SelectedSetIndex
    m_SetBrowser->select( m_SelectedSetIndex );
    //Updating the text in the input field by utilizing m_SelectedSetIndex
    m_SetNameInput.Update( set_name_vec[m_SelectedSetIndex] );

    ////==== Load Geometry ====//

    //geom_id_vec is filled with all possible geom's children an ID's
    vector< string > geom_id_vec = vehiclePtr->GetGeomVec();
    //geom_vec is filled with geom ptrs using ID's from geom_id_vec
    vector< Geom* > geom_vec = vehiclePtr->FindGeomVec( geom_id_vec );
    for ( int i = 0 ; i < ( int )geom_vec.size() ; i++ )
    {
        //We need to extract name of geom to display
        string g_name = geom_vec[i]->GetName();
        //We need the flag so we can display geom's "state" in browser
        bool flag = geom_vec[i]->GetSetFlag( m_SelectedSetIndex );
        //Adding the geom after Char * cast with flag
        m_SetSelectBrowser->add( g_name.c_str(), flag );
    }
    //This helps scroll position to stay in place after user clicks on items
    m_SetBrowser->position(setBrowserScrollPosition);
    m_SetSelectBrowser->position(setSelectBrowserScrollPosition);

    m_FLTK_Window->redraw();
    return false;
}

//==== Show Screen ====//
void SetEditorScreen::Show()
{
    m_ScreenMgr->SetUpdateFlag( true );
    m_FLTK_Window->show();
}

//==== Hide Screen ====//
void SetEditorScreen::Hide()
{
    m_FLTK_Window->hide();
    m_ScreenMgr->SetUpdateFlag( true );
}

void SetEditorScreen::CloseCallBack( Fl_Widget *w )
{
    Hide();
}

//==== Callbacks ====//

//This Main Callback is responding to events for windows, groups, and widgets attached to this object.
//m_ScreenMgr is another BasicScreen member inherated from VspScreen (included from "ScreenBase.h")
void SetEditorScreen::CallBack( Fl_Widget *w )
{
    //We get a vehiclePtr to help work with events
    assert( m_ScreenMgr ); 
    Vehicle* vehiclePtr = m_ScreenMgr->GetVehiclePtr();

    if ( w == m_SetSelectBrowser )
    {
        //We get the index of the item user clicked
        int geom_index = m_SetSelectBrowser->value() - 1;
        //geom_id_vec is filled with all possible geom's children an ID's 
        vector< string > geom_id_vec = vehiclePtr->GetGeomVec();
        if ( geom_index >= 0 && geom_index < ( int )geom_id_vec.size() )
        {
            int flag = m_SetSelectBrowser->checked( geom_index + 1 );
            //We use geom_index to go thru possible geoms and get selected geom's ptr
            Geom* gptr = vehiclePtr->FindGeom( geom_id_vec[geom_index] );
            if ( gptr )
            {
                //If index points to geoms that can be selected (based on enums), we then set the flag passed in
                gptr->SetSetFlag( m_SelectedSetIndex, flag );
            }
        }
    }
    else if ( w == m_SetBrowser)
    {
        //We update m_SelectedSetIndex with selected value
        m_SelectedSetIndex = m_SetBrowser->value();
    }

    //To update values, we can utilize Update Function by setting flag to true
    m_ScreenMgr->SetUpdateFlag( true );
}

//Callback for GUI Devices related events like buttons or input fields
void SetEditorScreen::GuiDeviceCallBack( GuiDevice* device )
{
    //We get a vehiclePtr to help work with events
    assert( m_ScreenMgr );
    Vehicle* vehiclePtr = m_ScreenMgr->GetVehiclePtr();
    vector< string > geom_id_vec = vehiclePtr->GetGeomVec();

    //This checks if user entered input in the SetNameInput field
    //It uses m_SelectedSetIndex to select and name correct set
    if ( device == &m_SetNameInput )
    {
        if ( m_SelectedSetIndex > SET_NOT_SHOWN )
        {
            string name = m_SetNameInput.GetString();
            vehiclePtr->SetSetName( m_SelectedSetIndex, name );
        }
    }
    //This is for setting m_CopyIndex with the index of the set we want to copy 
    else if ( device == &m_CopySet )
    {
        m_CopyIndex = m_SelectedSetIndex;
    }
    //This is for setting m_PasteIndex with the index of the set we want to paste into 
    else if ( device == &m_PasteSet )
    {
        m_PasteIndex = m_SelectedSetIndex;
        vehiclePtr->CopyPasteSet(m_CopyIndex, m_PasteIndex);
    }
    //This is for selecting all Geoms in a certain set
    else if ( device == &m_SelectAll )
    {
        if ( m_SelectedSetIndex > SET_ALL )
        {
            vector< string > geom_id_vec = vehiclePtr->GetGeomVec();
            for (int i = 0; i < ( int )geom_id_vec.size(); i++ )
            {
                Geom* gptr = vehiclePtr->FindGeom( geom_id_vec[i] );
                if ( gptr )
                {
                    gptr->SetSetFlag( m_SelectedSetIndex, true );
                }
            }
        } 
    }
    //This is for unselecting all Geoms in a certain set
    else if ( device == &m_UnselectAll )
    {
        if ( m_SelectedSetIndex > SET_ALL )
        {
            vector< string > geom_id_vec = vehiclePtr->GetGeomVec();
            for (int i = 0; i < ( int )geom_id_vec.size(); i++)
            {
                Geom* gptr = vehiclePtr->FindGeom( geom_id_vec[i] );
                if ( gptr )
                {
                    gptr->SetSetFlag( m_SelectedSetIndex, false );
                }
            }
        }
    }
    //This is for Highliting Geoms selected in a set
    else if ( device == &m_HighlightSet )
    {
        vector < string > activate_geom_vec = vehiclePtr->GetGeomSet( m_SelectedSetIndex );
        vehiclePtr->SetActiveGeomVec( activate_geom_vec );
    }

    //Tells m_ScreenMgr to exacute Update() function
    m_ScreenMgr->SetUpdateFlag( true );
}

