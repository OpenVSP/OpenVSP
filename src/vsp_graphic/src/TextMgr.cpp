#include "OpenGLHeaders.h"

#include "TextMgr.h"
#include "MathText.h"
#include "gl2ps.h"
#include "glm/ext.hpp"

#include <algorithm>
#include <cmath>

// A line of text at scale one, as a fraction of the viewport's larger side.
#define LINE_FRACTION ( 1.0 / 126.0 )

// The size of a vector font for the height of a line of text
#define VECTOR_FONT_SIZE 0.8

// The most rendered strings kept; more and all are rendered afresh.
#define MAX_TEXT_IMAGES 64

namespace VSPGraphic
{
    TextMgr::TextMgr()
    {
    }
    TextMgr::~TextMgr()
    {
        clearFont();
    }

    void TextMgr::clearFont()
    {
        std::map< std::string, TextImage >::iterator it;
        for ( it = _images.begin(); it != _images.end(); ++it )
        {
            glDeleteTextures( 1, &( it->second.tex ) );
        }
        _images.clear();
    }

    // The pixels an em of text at scale, in the current viewport.
    static double emPixels( float scale )
    {
        GLint vp[4];
        glGetIntegerv( GL_VIEWPORT, vp );
        const VspFont & font = VspFont::Regular();
        double line = font.GetAscent() + font.GetDescent() + font.GetLineGap();
        return scale * LINE_FRACTION * std::max( vp[2], vp[3] ) / line;
    }

    const TextMgr::TextImage & TextMgr::getImage( const std::string &str, float scale )
    {
        // Sizes within a quarter pixel an em share an image.
        double px = std::round( 4.0 * emPixels( scale ) ) / 4.0;
        std::string key = str + "|" + std::to_string( px );

        std::map< std::string, TextImage >::iterator it = _images.find( key );
        if ( it != _images.end() )
        {
            return it->second;
        }

        if ( _images.size() >= MAX_TEXT_IMAGES )
        {
            clearFont();
        }

        MathLayout layout;
        LayoutMathText( str, layout );
        std::vector< unsigned char > alpha;
        int w, h, baseline;
        layout.Rasterize( px, alpha, w, h, baseline );

        // White, with the glyphs' coverage as alpha, as the color it is drawn in modulates.
        std::vector< unsigned char > texels( 2 * alpha.size() );
        for ( int i = 0; i < (int)alpha.size(); i++ )
        {
            texels[ 2 * i ] = 255;
            texels[ 2 * i + 1 ] = alpha[i];
        }

        TextImage & img = _images[ key ];
        img.w = w;
        img.h = h;
        glGenTextures( 1, &img.tex );
        glBindTexture( GL_TEXTURE_2D, img.tex );
        glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
        glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );
        glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
        glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
        glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
        glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
        glTexImage2D( GL_TEXTURE_2D, 0, GL_LUMINANCE_ALPHA, w, h, 0, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE,
                      texels.data() );
        glPixelStorei( GL_UNPACK_ALIGNMENT, 4 );

        return img;
    }

    void TextMgr::getTextSize( float scale, const std::string &str, float &w, float &h )
    {
        w = 0;
        h = 0;
        if ( str.empty() )
        {
            return;
        }
        GLint vp[4];
        glGetIntegerv( GL_VIEWPORT, vp );
        const TextImage & img = getImage( str, scale );
        w = 2.0f * img.w / vp[2];
        h = 2.0f * img.h / vp[3];
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

        // Math is written as its source, without the dollar signs.
        std::string text = str;
        text.erase( std::remove( text.begin(), text.end(), '$' ), text.end() );

        GL2PSvertex pos;
        pos.xyz[0] = ( float ) base.x;
        pos.xyz[1] = ( float ) base.y;
        pos.xyz[2] = 0.0f;
        for ( int i = 0; i < 4; i++ )
        {
            pos.rgba[i] = color[i];
        }
        gl2psForceRasterPos( &pos );
        gl2psTextOpt( text.c_str(), "Helvetica", ( GLshort ) ( size + 0.5 ), GL2PS_TEXT_BL, ( GLfloat ) angle );
    }

    void TextMgr::drawText( float scale, const std::string &str, float x0, float y0, float xoffset, float yoffset )
    {
        if ( str.empty() )
        {
            return;
        }

        // Where the center lands on screen, in window pixels.
        GLfloat mv[16];
        GLfloat pr[16];
        GLint vp[4];
        glGetFloatv( GL_MODELVIEW_MATRIX, mv );
        glGetFloatv( GL_PROJECTION_MATRIX, pr );
        glGetIntegerv( GL_VIEWPORT, vp );
        glm::vec3 c = glm::project( glm::vec3( x0, y0, 0.0f ), glm::make_mat4( mv ), glm::make_mat4( pr ),
                                    glm::vec4( vp[0], vp[1], vp[2], vp[3] ) );

        const TextImage & img = getImage( str, scale );

        // The top left corner, on a whole pixel so the texture's pixels fall on the screen's.
        float x = std::round( c.x - img.w / 2.0f + img.w * xoffset );
        float y = std::round( c.y + img.h / 2.0f + img.h * yoffset );

        glMatrixMode( GL_PROJECTION );
        glPushMatrix();
        glLoadIdentity();
        glOrtho( vp[0], vp[0] + vp[2], vp[1], vp[1] + vp[3], -1.0, 1.0 );
        glMatrixMode( GL_MODELVIEW );
        glPushMatrix();
        glLoadIdentity();

        if ( gl2psGetFileFormat() != GL2PS_UNINITIALIZED )
        {
            writeVectorText( str, x, y, img.w, img.h );
        }
        else
        {
            glEnable( GL_TEXTURE_2D );
            glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
            glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
            glBindTexture( GL_TEXTURE_2D, img.tex );

            glBegin( GL_QUADS );
            glTexCoord2f( 0.0f, 0.0f );
            glVertex2f( x, y );
            glTexCoord2f( 0.0f, 1.0f );
            glVertex2f( x, y - img.h );
            glTexCoord2f( 1.0f, 1.0f );
            glVertex2f( x + img.w, y - img.h );
            glTexCoord2f( 1.0f, 0.0f );
            glVertex2f( x + img.w, y );
            glEnd();

            glDisable( GL_TEXTURE_2D );
        }

        glMatrixMode( GL_PROJECTION );
        glPopMatrix();
        glMatrixMode( GL_MODELVIEW );
        glPopMatrix();
    }
}
