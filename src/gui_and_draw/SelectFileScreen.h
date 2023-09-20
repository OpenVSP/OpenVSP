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

#include <FL/Fl_Preferences.H>
#include <FL/Fl_File_Browser.H>
#include <FL/Fl_File_Input.H>
#include <FL/Fl_Menu_Button.H>
#include "ScreenMgr.h"
#include "StringUtil.h"
#include "ScreenBase.h"

#include <vector>
#include <string>

using std::string;
using std::vector;

enum FavsMode { ADD_FAV = 0, DELETE_FAV = 1, HOME = 2, VSP = 3, FAV = 4 };

class SelectFileScreen : public BasicScreen
{
public:

    SelectFileScreen( ScreenMgr* mgr );
    virtual ~SelectFileScreen() {}

    void LoadFavsMenu();

    virtual void Show();
    virtual void Hide();
    virtual bool Update();

    void CallBack( Fl_Widget* w );
    static void staticScreenCB( Fl_Widget *w, void* data )
    {
        ( ( SelectFileScreen* )data )->CallBack( w );
    }
    virtual void CloseCallBack( Fl_Widget* w );
    virtual void GuiDeviceCallBack( GuiDevice* device );

    string FileChooser( const char* title, const char* filter, bool forceext = true, const string &dir = string() );

    void MassageDirString( string &in ) const;

    void EnforceFilter( string &in ) const;

protected:

    GroupLayout m_MainLayout;
    GroupLayout m_BorderLayout;

    TriggerButton m_AcceptButton;
    TriggerButton m_CancelButton;

    Fl_File_Input * m_DirInput;
    Fl_File_Browser* m_FileBrowser;

    Fl_Menu_Button *m_FavsMenuButton;

    StringInput m_FileSelectInput;

private:

    bool m_SelectFileScreenIsOpen;
    bool m_AcceptFlag;

    int m_FileBrowserIndex;

    string m_FilterString;
    string m_DirString;
    string m_FileName;
    string m_FullPathName;
    string m_HomePath;
    string m_ExePath;

    vector< string > m_FavDirVec;

};

#endif


