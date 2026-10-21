#include <cassert>

#include "OpenGLHeaders.h"

#include "TextMgr.h"
#include "glfont2.h"
#include "gl2ps.h"
#include "glm/ext.hpp"

#include <cmath>

using glfont::GLFont;

#define FONT_BASE_SCALE (1.0 / (double)1024.0)

// The size of a vector font for the height of a line of glyphs of the texture font
#define VECTOR_FONT_SIZE 0.8

namespace VSPGraphic
{
    TextMgr::TextMgr()
    {
        _font = nullptr;
        texID = 0;
    }
    TextMgr::~TextMgr()
    {
        clearFont();
    }

    void TextMgr::clearFont()
    {
        if( _font )
        {
            glDeleteTextures(1, &(this->texID) );
            delete _font;
            _font = nullptr;
            texID = 0;
        }
    }

    double TextMgr::getBaseScale()
    {
        return FONT_BASE_SCALE;
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

    // Writes a string as text to the open gl2ps page, where its glyphs, w by h below the top left
    // corner (x, y), would be drawn: as large and as turned, on the baseline a font of that size
    // has, and in front of everything, as the overlays it labels are drawn on screen.
    static void writeVectorText( const std::string &str, double x, double y, double w, double h )
    {
        GLdouble mv[16];
        GLdouble pr[16];
        GLint vp[4];
        GLfloat color[4];
        glGetDoublev( GL_MODELVIEW_MATRIX, mv );
        glGetDoublev( GL_PROJECTION_MATRIX, pr );
        glGetIntegerv( GL_VIEWPORT, vp );
        glGetFloatv( GL_CURRENT_COLOR, color );

        glm::dmat4 m = glm::make_mat4( mv );
        glm::dmat4 p = glm::make_mat4( pr );
        glm::dvec4 v( vp[0], vp[1], vp[2], vp[3] );
        glm::dvec3 corner = glm::project( glm::dvec3( x, y, 0.0 ), m, p, v );
        glm::dvec3 right = glm::project( glm::dvec3( x + w, y, 0.0 ), m, p, v );
        glm::dvec3 down = glm::project( glm::dvec3( x, y - h, 0.0 ), m, p, v );

        double size = glm::length( glm::dvec2( down - corner ) ) * VECTOR_FONT_SIZE;
        double angle = std::atan2( right.y - corner.y, right.x - corner.x ) * 180.0 / M_PI;
        glm::dvec3 base = corner + ( down - corner ) * VECTOR_FONT_SIZE;

        GL2PSvertex pos;
        pos.xyz[0] = ( float ) base.x;
        pos.xyz[1] = ( float ) base.y;
        pos.xyz[2] = 0.0f;
        for ( int i = 0; i < 4; i++ )
        {
            pos.rgba[i] = color[i];
        }
        gl2psForceRasterPos( &pos );
        gl2psTextOpt( str.c_str(), "Helvetica", ( GLshort ) ( size + 0.5 ), GL2PS_TEXT_BL, ( GLfloat ) angle );
    }

    void TextMgr::drawText( GLFont * font, float scale, const std::string &str, float x0, float y0, float xoffset, float yoffset, float arW, float arH )
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
            w = ( ( float ) dimension.first ) * FONT_BASE_SCALE * scale * arH;
            h = ( ( float ) dimension.second ) * FONT_BASE_SCALE * scale * arW;
            double x = x0 - w / 2 + w * xoffset;
            double y = y0 + h / 2 + h * yoffset;
            if ( gl2psGetFileFormat() != GL2PS_UNINITIALIZED )
            {
                writeVectorText( str, x, y, w, h );
            }
            else
            {
                font->Begin();
                font->DrawString( str.data(), ( float )( FONT_BASE_SCALE * scale ), ( float )arW, ( float )arH, ( float )x, ( float )y );
            }
        }
        glDisable( GL_TEXTURE_2D );
    }
}
