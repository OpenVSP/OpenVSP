//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// labelScreen.cpp: implementation of the labelScreen class.
//
//////////////////////////////////////////////////////////////////////

#include <FL/Fl_Preferences.H>
#include "SelectFileScreen.h"
#include "ScreenMgr.h"
#include "StringUtil.h"

#include <stdio.h>  /* defines FILENAME_MAX */
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

SelectFileScreen::SelectFileScreen()
{
    char cCurrentPath[FILENAME_MAX];
    GetCurrentDir( cCurrentPath, sizeof( cCurrentPath ) );

    m_DirString = string( cCurrentPath );

    m_HomePath = VehicleMgr.GetVehicle()->GetHomePath();
    m_ExePath = VehicleMgr.GetVehicle()->GetExePath();

    char forwardSlash = '\\';
    StringUtil::change_from_to( m_DirString, forwardSlash, '/' );

    int dirSize = ( int )m_DirString.size();
    if ( m_DirString[dirSize - 1] != '/' )
    {
        m_DirString.append( "/" );
    }

    m_FilterString = string( "*.*" );

    selectFileUI = new SelectFileUI();
    selectFileUI->UIWindow->position( 30, 30 );
    selectFileUI->fileBrowser->type( FL_SELECT_BROWSER );
    selectFileUI->fileBrowser->callback( staticScreenCB, this );
    selectFileUI->dirInput->callback( staticScreenCB, this );
    selectFileUI->dirInput->when( FL_WHEN_CHANGED );

    selectFileUI->fileInput->callback( staticScreenCB, this );
    selectFileUI->acceptButton->callback( staticScreenCB, this );
    selectFileUI->cancelButton->callback( staticScreenCB, this );

    selectFileUI->favsMenuButton->callback( staticScreenCB, this );

    LoadFavsMenu();
}

void SelectFileScreen::LoadFavsMenu()
{
    m_FavDirVec.clear();
    selectFileUI->favsMenuButton->clear();
    selectFileUI->favsMenuButton->add( "Add to Favorites" );
    selectFileUI->favsMenuButton->add( "Delete All Favorites", 0, NULL, ( void* )0, FL_MENU_DIVIDER );
    selectFileUI->favsMenuButton->add( "Home" );
    selectFileUI->favsMenuButton->add( "VSP Path", 0, NULL, ( void* )0, FL_MENU_DIVIDER );

    //==== Preferences ====//
    Fl_Preferences prefs( Fl_Preferences::USER, "openvsp.org", "VSP" );

    //==== Load All Favorites From Preferences ====//
    char tag[256], str[256];
    bool keep_looking = true;
    while ( keep_looking )
    {
        keep_looking = false;
        sprintf( tag, "fav%d", ( int )m_FavDirVec.size() );
        if ( prefs.get( tag, str, "", 256 ) )
        {
            keep_looking = true;

            m_FavDirVec.push_back( string( str ) );
            string menu_label( str );
            menu_label.insert( 0, "/" );
            selectFileUI->favsMenuButton->add( menu_label.c_str() );
        }
    }
}

void SelectFileScreen::show()
{
    if ( selectFileUI )
    {
        selectFileUI->UIWindow->show();
    }
}

string SelectFileScreen::FileChooser( const char* title, const char* filter )
{
    string file_name;
    m_AcceptFlag = false;

    m_FileName = string();

    char filter_str[256];
    sprintf( filter_str, "   (%s)", filter );

    m_Title = string( title );
    m_Title.append( filter_str );
    selectFileUI->titleBox->label( m_Title.c_str() );

    m_FilterString = filter;

    selectFileUI->fileInput->value( m_FileName.c_str() );
    selectFileUI->fileBrowser->filter( m_FilterString.c_str() );
    selectFileUI->fileBrowser->load( m_DirString.c_str() );
    selectFileUI->dirInput->value( m_DirString.c_str() );
    show();

    while( selectFileUI->UIWindow->shown() )
    {
        Fl::wait();
    }

    if ( m_AcceptFlag )
    {
        m_FullPathName = m_DirString;
        m_FullPathName.append( m_FileName );
        file_name = m_FullPathName;
    }

    return file_name;
}

string SelectFileScreen::FileChooser( const char* title, const char* filter, const char* dir )
{
    m_DirString = dir;
    return FileChooser( title, filter );
}

void SelectFileScreen::screenCB( Fl_Widget* w )
{
    if ( w == selectFileUI->fileBrowser )
    {
        int sid = selectFileUI->fileBrowser->value();
        selectFileUI->fileBrowser->select( sid );

        // Check for NULL Char pointer
        const char * text = selectFileUI->fileBrowser->text( sid );
        if ( text == NULL )
        {
            return;
        }

        string selText = string( text );

        if ( selText == "../" )
        {
            if ( StringUtil::count_char_matches( m_DirString, '/' ) > 1 )
            {
                int dirLen = m_DirString.size();
                m_DirString.erase( dirLen - 1, 1 );

                int slashLoc = m_DirString.find_last_of( '/' );

                if ( slashLoc + 1 <= ( dirLen - 1 ) )
                {
                    m_DirString.erase( slashLoc + 1, dirLen - slashLoc );
                }

                selectFileUI->fileBrowser->load( m_DirString.c_str() );
                selectFileUI->dirInput->value( m_DirString.c_str() );

            }
        }
        else if ( StringUtil::count_char_matches( selText, '/' ) >= 1 )
        {
            m_DirString.append( selText );
            selectFileUI->fileBrowser->load( m_DirString.c_str() );
            selectFileUI->dirInput->value( m_DirString.c_str() );
        }
        else
        {
            m_FileName = selText;
            selectFileUI->fileInput->value( m_FileName.c_str() );
        }
    }
    else if ( w == selectFileUI->dirInput )
    {
        m_DirString = string( selectFileUI->dirInput->value() );
        char forwardSlash = '\\';
        StringUtil::change_from_to( m_DirString, forwardSlash, '/' );
        int dirSize = m_DirString.size();
        if ( m_DirString[dirSize - 1] != '/' )
        {
            m_DirString.append( "/" );
        }
        selectFileUI->fileBrowser->load( m_DirString.c_str() );

    }
    else if ( w == selectFileUI->fileInput )
    {
        m_FileName = selectFileUI->fileInput->value();
    }
    else if ( w == selectFileUI->acceptButton )
    {
        m_AcceptFlag = true;
        selectFileUI->UIWindow->hide();
    }
    else if ( w == selectFileUI->cancelButton )
    {
        m_AcceptFlag = false;
        selectFileUI->UIWindow->hide();
    }
    else if ( w == selectFileUI->favsMenuButton )
    {
        int val = selectFileUI->favsMenuButton->value();

        if ( val == 0 )             // Add To Favorites
        {
            Fl_Preferences prefs( Fl_Preferences::USER, "openvsp.org", "VSP" );
            char favstr[256];
            sprintf( favstr, "fav%d", static_cast<int>( m_FavDirVec.size() ) );
            prefs.set( favstr, m_DirString.c_str() );
            prefs.flush();
            LoadFavsMenu();
        }
        else if ( val == 1 )
        {
            m_FavDirVec.clear();
            Fl_Preferences prefs( Fl_Preferences::USER, "openvsp.org", "VSP" );
            for ( int i = 0 ; i < ( int )prefs.entries() ; i++ )
            {
                prefs.deleteEntry( prefs.entry( i ) );
            }
            prefs.flush();
            LoadFavsMenu();
        }
        else if ( val == 2 )
        {
            m_DirString = m_HomePath;
            selectFileUI->fileBrowser->load( m_DirString.c_str() );
            selectFileUI->dirInput->value( m_DirString.c_str() );
        }
        else if ( val == 3 )
        {
            m_DirString = m_ExePath;
            selectFileUI->fileBrowser->load( m_DirString.c_str() );
            selectFileUI->dirInput->value( m_DirString.c_str() );
        }
        else
        {
            //==== Select Favorite Dir ====//
            int ind = val - 4;
            if ( ind >= 0 && ind < ( int )m_FavDirVec.size() )
            {
                m_DirString = m_FavDirVec[ind];
//              m_DirString.delete_range( 0, 0 );
//              m_DirString.remove_leading('/');
                selectFileUI->fileBrowser->load( m_DirString.c_str() );
                selectFileUI->dirInput->value( m_DirString.c_str() );
            }
        }
    }
}



