//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

//******************************************************************************
//
//  Font Manager Class
//
//
//   J.R. Gloudemans - 8/13/04
//
//******************************************************************************



#ifndef FONT_MANAGER_H
#define FONT_MANAGER_H


#pragma warning(disable:4786)

#include "stringc.h"
#include <vector>
#include <map>
using namespace std;

#include "glfont2.h"
using namespace glfont;


#define FONT_BASE_SCALE (1.0 / (double)1024.0)


class FontMgr
{
public:	
	VSPDLL FontMgr();
	VSPDLL virtual ~FontMgr();

	GLFont * loadFont( );

private:
	GLFont * font;
};


class SingleFont {
public:
	SingleFont() {
		static FontMgr *fptr = 0;
		if (!fptr) fptr = new FontMgr();
		fontMgr = fptr;
	}
	FontMgr * fontMgr;
};

static SingleFont singleFont;

#define fontMgr (singleFont.fontMgr)

#endif


