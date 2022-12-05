//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// labelScreen.cpp: implementation of the labelScreen class.
//
//////////////////////////////////////////////////////////////////////

#include "SelectFileScreen.h"

#ifdef WIN32
#include <direct.h>
#define GetCurrentDir _getcwd
#else
#include <unistd.h>
#define GetCurrentDir getcwd
#endif


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

SelectFileScreen::SelectFileScreen( ScreenMgr* mgr ) : BasicScreen( mgr, 440, 345, "Open VSP File (*.vsp3)")
{

    char cCurrentPath[FILENAME_MAX];
    GetCurrentDir( cCurrentPath, sizeof( cCurrentPath ) );

    m_DirString = string( cCurrentPath );

    m_HomePath = VehicleMgr.GetVehicle()->GetHomePath() + string( "/" );
    m_ExePath = VehicleMgr.GetVehicle()->GetExePath() + string( "/" );

    MassageDirString( m_DirString );
    MassageDirString( m_HomePath );
    MassageDirString( m_ExePath );

    m_FilterString = string( "*.*" );

    int yPadding = 7;
    int borderPaddingWidth = 5;

    m_FileBrowserIndex = 0;
    m_SelectFileScreenIsOpen = false;
    
    //Create GUI
    m_FL_TitleBox->resize( 2, 2, 355 , 20 );

    m_FavsMenuButton = new Fl_Menu_Button(370, 2, 65, 20, "Favs");
    m_FavsMenuButton->labelfont(1);

    m_DirInput = new Fl_File_Input(5, 30, 430, 30);

    m_FileBrowser = new Fl_File_Browser(5, 60, 430, 225);
    m_FileBrowser->labelsize(12);

    m_FLTK_Window->callback( staticCloseCB, this );

    m_MainLayout.SetGroupAndScreen( m_FLTK_Window, this );
    m_MainLayout.ForceNewLine();
    m_MainLayout.AddY( yPadding );
    m_MainLayout.AddX( borderPaddingWidth );

    m_MainLayout.AddSubGroupLayout( m_BorderLayout, m_MainLayout.GetRemainX() - borderPaddingWidth,
        m_MainLayout.GetRemainY() - borderPaddingWidth );

    m_BorderLayout.AddY( 260 );
    m_BorderLayout.AddYGap();

    m_BorderLayout.AddInput( m_FileSelectInput, "File:" );
    m_BorderLayout.AddYGap();
    m_FileSelectInput.SetTextFont( FL_HELVETICA_BOLD );

    m_BorderLayout.SetFitWidthFlag( false );
    m_BorderLayout.SetSameLineFlag( true );

    m_BorderLayout.SetButtonWidth(( m_BorderLayout.GetW() / 3) );
    m_BorderLayout.AddButton( m_AcceptButton , "Accept");

    m_BorderLayout.AddX( m_BorderLayout.GetW() / 3);
    m_BorderLayout.AddButton( m_CancelButton , "Cancel");

    //Need to give it a type or browser does not work as intended
    m_FileBrowser->type( FL_SELECT_BROWSER );

    //Need the callbacks for these objects
    m_FileBrowser->callback( staticScreenCB, this );
    m_DirInput->callback( staticScreenCB, this );
    m_FavsMenuButton->callback( staticScreenCB, this );
}

bool  SelectFileScreen::Update()
{
    LoadFavsMenu();
    m_FileBrowserIndex = 0;
    m_FLTK_Window->redraw();
    return false;
}

void SelectFileScreen::Show()
{
    m_ScreenMgr->SetUpdateFlag( true );
    m_FLTK_Window->show();
}

void SelectFileScreen::Hide()
{
    //This helps FileChooser() know screen was closed
    m_SelectFileScreenIsOpen = false;
    m_FLTK_Window->hide();
    m_ScreenMgr->SetUpdateFlag( true );
}

void SelectFileScreen::CloseCallBack( Fl_Widget *w )
{
    Hide();
}

void SelectFileScreen::CallBack( Fl_Widget* w )
{
    if ( w == m_FileBrowser )
    {
        m_FileBrowserIndex = m_FileBrowser->value();
        
        // Check for NULL Char pointer
        const char * text = m_FileBrowser->text( m_FileBrowserIndex );
        if ( text == NULL )
        {
            return;
        }

        string selText = string( text );

        if ( selText == "../" )
        {
            if ( StringUtil::count_char_matches( m_DirString, '/' ) > 1 )
            {
                unsigned int dirLen = m_DirString.size();
                m_DirString.erase( dirLen - 1, 1 );

                unsigned int slashLoc = m_DirString.find_last_of( '/' );

                if ( slashLoc + 1 <= ( dirLen - 1 ) )
                {
                    m_DirString.erase( slashLoc + 1, dirLen - slashLoc );
                }

                m_FileBrowser->load( m_DirString.c_str() );
                m_DirInput->value( m_DirString.c_str() );

            }
        }
        else if ( StringUtil::count_char_matches( selText, '/' ) >= 1 )
        {
            m_DirString.append( selText );
            m_FileBrowser->load( m_DirString.c_str() );
            m_DirInput->value( m_DirString.c_str() );
        }
        else
        {
            m_FileName = selText;
            m_FileSelectInput.Update( m_FileName.c_str() );
        }
        
    }
    else if ( w == m_DirInput )
    {
        m_DirString = string( m_DirInput->value() );
        char forwardSlash = '\\';
        StringUtil::change_from_to( m_DirString, forwardSlash, '/' );
        int dirSize = m_DirString.size();
        if ( m_DirString[dirSize - 1] != '/' )
        {
            m_DirString.append( "/" );
        }
        m_FileBrowser->load( m_DirString.c_str() );

        //Clears file name when directory is changed
        m_FileSelectInput.Update( "" );  
    }
    else if ( w == m_FavsMenuButton )
    {
        int val = m_FavsMenuButton->value();

        if ( val == ADD_FAV )
        {
            Fl_Preferences prefs( Fl_Preferences::USER, "openvsp.org", "VSP" );
            char favstr[256];
            snprintf( favstr, sizeof( favstr ), "fav%d", static_cast<int>( m_FavDirVec.size() ) );

            //If m_FavDirVec has m_DirString already, we don't need to set pref again
            if ( !std::count( m_FavDirVec.begin(), m_FavDirVec.end(), m_DirString.c_str() ) )
            {
                prefs.set( favstr, m_DirString.c_str() );
                prefs.flush();
            }        
        }
        else if ( val == DELETE_FAV )
        {
            m_FavDirVec.clear();
            Fl_Preferences prefs( Fl_Preferences::USER, "openvsp.org", "VSP" );

            //This helps handle getting rid of all prefs entries
            while ( ( int )prefs.entries() > 0 )
            {
                for ( int i = 0 ; i < ( int )prefs.entries() ; i++ )
                {
                    prefs.deleteEntry( prefs.entry( i ) );
                    m_FavsMenuButton->clear();
                }
            }
            prefs.flush();
        }
        else if ( val == HOME )
        {
            m_DirString = m_HomePath;
            m_FileBrowser->load( m_DirString.c_str() );
            m_DirInput->value( m_DirString.c_str() );

            //Clears file name when directory is changed
            m_FileSelectInput.Update( "" ); 
        }
        else if ( val == VSP )
        {
            m_DirString = m_ExePath;
            m_FileBrowser->load( m_DirString.c_str() );
            m_DirInput->value( m_DirString.c_str() );

            //Clears file name when directory is changed
            m_FileSelectInput.Update( "" ); 
        }
        else
        {
            //==== Select Favorite Dir ====//
            int ind = val - FAV;
            if ( ind >= 0 && ind < ( int )m_FavDirVec.size() )
            {
                m_DirString = m_FavDirVec[ind];
                m_FileBrowser->load( m_DirString.c_str() );
                m_DirInput->value( m_DirString.c_str() );
            }

            //Clears file name when directory is changed
            m_FileSelectInput.Update( "" ); 
        }
    }

    //To update values, we can utilize Update Function by setting flag to true
    m_ScreenMgr->SetUpdateFlag( true );
}

void  SelectFileScreen::GuiDeviceCallBack(GuiDevice* device)
{
    if ( device == &m_AcceptButton )
    {
        m_AcceptFlag = true;
        Hide();
    }
    else if ( device == &m_CancelButton )
    {
        m_AcceptFlag = false;
        Hide();
    }
    else if ( device == &m_FileSelectInput )
    {
        m_FileName = m_FileSelectInput.GetString();
    }

    //Tells m_ScreenMgr to exacute Update() function
    m_ScreenMgr->SetUpdateFlag( true );
}

void SelectFileScreen::MassageDirString( string &in ) const
{
    char forwardSlash = '\\';
    StringUtil::change_from_to( in, forwardSlash, '/' );

    int dirSize = ( int )in.size();
    if ( in[dirSize - 1] != '/' )
    {
        in.append( "/" );
    }
}

void SelectFileScreen::EnforceFilter( string &in ) const
{
    string desiredext = m_FilterString.substr( 1, string::npos ); // Skip leading character probably '*'.
    string desiredlower = desiredext;
    std::transform( desiredlower.begin(), desiredlower.end(), desiredlower.begin(), ::tolower );

    std::string::size_type extIndex = in.find_last_of( '.' );

    if( extIndex == std::string::npos )
    {
        in += desiredext;
    }
    else
    {
        std::string ext = in.substr( extIndex, in.size() - extIndex );
        std::transform( ext.begin(), ext.end(), ext.begin(), ::tolower );
        if( ext != desiredlower ) // compare in lower case
        {
            in += desiredext;
        }
    }
}

void SelectFileScreen::LoadFavsMenu()
{
    m_FavDirVec.clear();
    m_FavsMenuButton->add( "Add to Favorites" );
    m_FavsMenuButton->add( "Delete All Favorites", 0, NULL, ( void* )0, FL_MENU_DIVIDER );
    m_FavsMenuButton->add( "Home" );
    m_FavsMenuButton->add( "VSP Path", 0, NULL, ( void* )0, FL_MENU_DIVIDER );

    //==== Preferences ====//
    Fl_Preferences prefs( Fl_Preferences::USER, "openvsp.org", "VSP" );

    //==== Load All Favorites From Preferences ====//
    char tag[256], str[256];
    bool keep_looking = true;
    while ( keep_looking )
    {
        keep_looking = false;
        snprintf( tag, sizeof( tag ), "fav%d", ( int )m_FavDirVec.size() );
        if ( prefs.get( tag, str, "", 256 ) )
        {
            keep_looking = true;

            m_FavDirVec.push_back( string( str ) );
            
            string menu_label( str );
            menu_label.insert( 0, "/" );
            m_FavsMenuButton->add( menu_label.c_str() );
        }
    }
}

string SelectFileScreen::FileChooser( const char* title, const char* filter, bool forceext )
{
    string file_name;
    m_AcceptFlag = false;
    m_SelectFileScreenIsOpen = true;

    m_FileName = string();

    char filter_str[256];
    snprintf( filter_str, sizeof( filter_str ), "   (%s)", filter );

    SetTitle( string( title ).append( filter_str ) );

    m_FilterString = filter;

    m_FileSelectInput.Update( m_FileName.c_str() );
    m_FileBrowser->filter( m_FilterString.c_str() );
    m_FileBrowser->load( m_DirString.c_str() );
    m_DirInput->value( m_DirString.c_str() );
    Show();

    while ( m_SelectFileScreenIsOpen )
    {
        Fl::wait();
    }

    if ( m_AcceptFlag )
    {
        m_FullPathName = m_DirString;
        m_FullPathName.append( m_FileName );

        if ( forceext )
        {
            EnforceFilter( m_FullPathName );
        }

        file_name = m_FullPathName;
    }

    return file_name;
}

string SelectFileScreen::FileChooser( const char* title, const char* filter, const char* dir, bool forceext )
{
    m_DirString = dir;
    return FileChooser( title, filter, forceext );
}

