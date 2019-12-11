//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

//******************************************************************************
//
//   Select File Screen Class
//
//   J.R. Gloudemans - 1/27/11
//******************************************************************************

#ifndef SELECTFILESCREEN_H
#define SELECTFILESCREEN_H

#include "selectFileFlScreen.h"

#include <vector>
#include <string>
using std::string;
using std::vector;

class SelectFileScreen
{
public:

    SelectFileScreen();
    virtual ~SelectFileScreen() {}

    void LoadFavsMenu();

    virtual void show();

    void screenCB( Fl_Widget* w );

    SelectFileUI* selectFileUI;

    string FileChooser( const char* title, const char* filter, bool forceext = true );
    string FileChooser( const char* title, const char* filter, const char* dir, bool forceext = true );

    static void staticScreenCB( Fl_Widget *w, void* data )
    {
        ( static_cast <SelectFileScreen*> ( data ) )->screenCB( w );
    }


protected:

    void MassageDirString( string &in ) const;

    void EnforceFilter( string &in ) const;

    bool m_AcceptFlag;
    string m_FilterString;
    string m_DirString;
    string m_FileName;
    string m_FullPathName;
    string m_Title;

    vector< string > m_FavDirVec;

    string m_HomePath;
    string m_ExePath;

};

#endif


