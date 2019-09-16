#include <cassert>

#include "OpenGLHeaders.h"

#include "TextMgr.h"
#include "glfont2.h"

using glfont::GLFont;

#define FONT_BASE_SCALE (1.0 / (double)1024.0)

namespace VSPGraphic
{
    TextMgr::TextMgr()
    {
        _font = NULL;
        texID = 0;
    }
    TextMgr::~TextMgr()
    {
        if( _font )
        {
            glDeleteTextures(1, &(this->texID) );
            delete _font;
        }
    }

    GLFont * TextMgr::loadFont()
    {
        // We only want to generate a font (texture) once.
        if( _font )
        {
            return _font;
        }

        glClearColor( 1.0, 1.0, 1.0, 1.0 );

        glEnable( GL_TEXTURE_2D );
        glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

        GLFont * texfont = new GLFont();
        glGenTextures( 1, &(this->texID) );

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

    void TextMgr::drawText( GLFont * font, float scale, const std::string &str, float x0, float y0, float xoffset, float yoffset )
    {
        glEnable( GL_TEXTURE_2D );
        glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
        glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );

        double w = 0;
        double h = 0;
        if ( !str.empty() )
        {
            std::pair< int, int > dimension;
            font->GetStringSize( str.data(), &dimension );
            w = ( ( float ) dimension.first ) * FONT_BASE_SCALE * scale;
            h = ( ( float ) dimension.second ) * FONT_BASE_SCALE * scale;
            double x = x0 - w / 2 + w * xoffset;
            double y = y0 + h / 2 + h * yoffset;
            font->Begin();
            font->DrawString( str.data(), ( float )( FONT_BASE_SCALE * scale ), ( float )x, ( float )y );
        }
        glDisable( GL_TEXTURE_2D );
    }
}
