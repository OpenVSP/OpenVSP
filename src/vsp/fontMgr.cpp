//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

#include "fontMgr.h"

FontMgr::FontMgr()
{
	font = NULL;
}

FontMgr::~FontMgr() 
{
	delete font;
}

GLFont * FontMgr::loadFont( )
{
	if( font )
		return font;

	// create font
	glClearColor(1.0, 1.0, 1.0, 1.0);

	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	GLFont * texfont = new GLFont();
	GLuint texID;
	glGenTextures(1, &texID);
	if ( !texfont->Create(texID) )
	{
		delete texfont;
		texfont = 0;
	}
	else
	{
		font = texfont;
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_BLEND);
	}
	return texfont;
}
