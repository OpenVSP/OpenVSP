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

#include "screenMgr.h"
#include "selectFileFlScreen.h"

using namespace std;

class SelectFileScreen
{
public:

	SelectFileScreen();
	virtual ~SelectFileScreen() {}

	void LoadFavsMenu();

	virtual void show();			

	void screenCB( Fl_Widget* w );

	SelectFileUI* selectFileUI;

	char* FileChooser( const char* title, const char* filter );

	static void staticScreenCB( Fl_Widget *w, void* data ) 
					{ ((SelectFileScreen*)data)->screenCB( w ); }


protected:

	bool m_AcceptFlag;
	Stringc m_FilterString;
	Stringc m_DirString;
	Stringc m_FileName;
	Stringc m_FullPathName;
	Stringc m_Title;

	vector< Stringc > m_FavDirVec;
	

};

#endif


