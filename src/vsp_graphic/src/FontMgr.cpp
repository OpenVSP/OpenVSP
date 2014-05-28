//******************************************************************************
//
//  Slightly Modified Font Manager Class
//
//  J.R. Gloudemans - 8/13/04
//   Phoenix Integration - 2012
//
//******************************************************************************
#include <assert.h>

#include "OpenGLHeaders.h"

#include "FontMgr.h"
#include "glfont2.h"

using glfont::GLFont;

namespace VSPGraphic
{
FontMgr::FontMgr()
{
    _font = NULL;
}
FontMgr::~FontMgr()
{
    if( _font )
    {
        delete _font;
    }
}

GLFont * FontMgr::loadFont()
{
    if( _font )
    {
        return _font;
    }

    glClearColor( 1.0, 1.0, 1.0, 1.0 );

    glEnable( GL_TEXTURE_2D );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

    GLFont * texfont = new GLFont();
    GLuint texID;
    glGenTextures( 1, &texID );

    if ( !texfont->Create( texID ) )
    {
        // Font failed to load.
        assert( false );
        delete texfont;
        texfont = 0;
    }
    else
    {
        _font = texfont;
    }
    glDisable( GL_TEXTURE_2D );

    return texfont;
}
}
