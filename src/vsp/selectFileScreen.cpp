// labelScreen.cpp: implementation of the labelScreen class.
//
//////////////////////////////////////////////////////////////////////

#include <FL/Fl_Preferences.H>
#include "selectFileScreen.h"
#include "screenMgr.h"

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
	GetCurrentDir(cCurrentPath, sizeof(cCurrentPath));

	m_DirString = Stringc(cCurrentPath);

	char forwardSlash = '\\';
	m_DirString.change_from_to(forwardSlash, '/');

	int dirSize = m_DirString.get_length();
	if ( m_DirString[dirSize-1] != '/' )
		m_DirString.concatenate("/");

	m_FilterString = Stringc("*.*");

	selectFileUI = new SelectFileUI();
	selectFileUI->UIWindow->position( 30, 30 );
	selectFileUI->fileBrowser->type(FL_SELECT_BROWSER);
	selectFileUI->fileBrowser->callback( staticScreenCB, this );
	selectFileUI->dirInput->callback( staticScreenCB, this );
	selectFileUI->dirInput->when(FL_WHEN_CHANGED);

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
	selectFileUI->favsMenuButton->add("Add to Favorites");
	selectFileUI->favsMenuButton->add("Delete All Favorites", 0, NULL, (void*)0, FL_MENU_DIVIDER );

	//==== Preferences ====//
	Fl_Preferences prefs( Fl_Preferences::USER, "NASA", "VSP");

	//==== Load All Favorites From Preferences ====//
	char tag[256], str[256];
	bool keep_looking = true;
	while ( keep_looking )
	{
		keep_looking = false;
		sprintf( tag, "fav%d", (int)m_FavDirVec.size() );
		if ( prefs.get( tag, str, "", 256 ) )
		{
			keep_looking = true;

			m_FavDirVec.push_back( Stringc(str) );
			Stringc menu_label(str);
			menu_label.insert_string_at( 0, "/" );
			selectFileUI->favsMenuButton->add( menu_label );
		}
	}
}

void SelectFileScreen::show()
{
	if (selectFileUI) 
		selectFileUI->UIWindow->show(); 
}

char* SelectFileScreen::FileChooser( const char* title, const char* filter )
{
	char* file_name = NULL;
	m_AcceptFlag = false;

	m_FileName = Stringc();

	char filter_str[256];
	sprintf( filter_str, "   (%s)", filter );

	m_Title = Stringc( title );
	m_Title.concatenate( filter_str );
	selectFileUI->titleBox->label( m_Title );

	m_FilterString = filter;

	selectFileUI->fileInput->value( m_FileName );
	selectFileUI->fileBrowser->filter( m_FilterString );
	selectFileUI->fileBrowser->load( m_DirString );
	selectFileUI->dirInput->value( m_DirString );
	show();

    while(selectFileUI->UIWindow->shown())
        { Fl::wait(); }

	if ( m_AcceptFlag )
	{
		m_FullPathName = m_DirString;
		m_FullPathName.concatenate( m_FileName );
		file_name = m_FullPathName.get_char_star();
	}

	return file_name;
}


void SelectFileScreen::screenCB( Fl_Widget* w )
{
	if ( w == selectFileUI->fileBrowser )
	{
		int sid = selectFileUI->fileBrowser->value();
		selectFileUI->fileBrowser->select( sid );

		Stringc selText = Stringc(selectFileUI->fileBrowser->text(sid));

		if ( selText == "../" )
		{
			if ( m_DirString.count_substrings( "/" ) > 1 )
			{
				int dirLen = m_DirString.get_length();
				int slashLoc = dirLen-2;

				while ( slashLoc > 0 )
				{
					if ( m_DirString[slashLoc] == '/' )
						break;
					slashLoc--;
				}
				if ( slashLoc+1 <= dirLen-1 )
					m_DirString.delete_range( slashLoc+1, dirLen-1 );

				selectFileUI->fileBrowser->load( m_DirString.get_char_star() );
				selectFileUI->dirInput->value( m_DirString );
			}
		}
		else if ( selText.count_substrings( "/" ) >= 1 )
		{
			m_DirString.concatenate( selText );
			selectFileUI->fileBrowser->load( m_DirString.get_char_star() );
			selectFileUI->dirInput->value( m_DirString );	
		}
		else
		{
			m_FileName = selText;
			selectFileUI->fileInput->value( m_FileName );
		}
	}
	else if ( w == selectFileUI->dirInput )
	{
		m_DirString = Stringc(selectFileUI->dirInput->value());

		char forwardSlash = '\\';
		m_DirString.change_from_to(forwardSlash, '/');

		int dirSize = m_DirString.get_length();
		if ( m_DirString[dirSize-1] != '/' )
			m_DirString.concatenate("/");

		selectFileUI->fileBrowser->load( m_DirString.get_char_star() );
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

		if ( val == 0 )				// Add To Favorites
		{
			Fl_Preferences prefs( Fl_Preferences::USER, "NASA", "VSP");
			char favstr[256];
			sprintf( favstr, "fav%d", m_FavDirVec.size() );
			prefs.set( favstr, m_DirString );
			prefs.flush();
			LoadFavsMenu();
		}
		else if ( val == 1 )	
		{
			m_FavDirVec.clear();
			Fl_Preferences prefs( Fl_Preferences::USER, "NASA", "VSP");
			for ( int i = 0 ; i < (int)prefs.entries() ; i++ )
			{
				prefs.deleteEntry( prefs.entry( i ) );
			}
			prefs.flush();
			LoadFavsMenu();	
		}
		else
		{
			//==== Select Favorite Dir ====//
			int ind = val - 2;
			if ( ind >= 0 && ind < (int)m_FavDirVec.size() )
			{
				m_DirString = m_FavDirVec[ind];
//				m_DirString.delete_range( 0, 0 );
//				m_DirString.remove_leading('/');
				selectFileUI->fileBrowser->load( m_DirString );
				selectFileUI->dirInput->value( m_DirString );
			}
		}
	}
}



