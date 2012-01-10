#include "fontMgr.h"


FontMgr::~FontMgr() 
{
	for (int i = 0; i < (int)fontVec.size(); i++)
	{
		delete fontVec[i].second;
	}
}

GLFont * FontMgr::loadFont(const char * file) 
{

	// get font
	for (int i = 0; i < (int)fontVec.size(); i++)
	{
		pair< Stringc, GLFont * > fontPair = fontVec[i];

		if ((char*)fontPair.first == file)
		{
			return fontPair.second;
		}
	}


	// create font
	glClearColor(1.0, 1.0, 1.0, 1.0);

	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	GLFont * texfont = new GLFont();
	GLuint texID;
	glGenTextures(1, &texID);
	if ( !texfont->Create(file, texID) )
	{
		delete texfont;
		texfont = 0;
	}
	else
	{
		fontVec.push_back( pair< Stringc, GLFont * >(Stringc(file), texfont) );
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_BLEND);
	}
	return texfont;
}
