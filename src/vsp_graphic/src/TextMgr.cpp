#include "OpenGLHeaders.h"

#include <map>

#include "TextMgr.h"
#include "glfont2.h"

using glfont::GLFont;

#define FONT_BASE_SCALE (1.0 / (double)1024.0)

namespace VSPGraphic
{
TextMgr::TextMgr()
{
}
TextMgr::~TextMgr()
{
}

void TextMgr::drawText( GLFont * font, float scale, std::string str, float x0, float y0, float xoffset, float yoffset )
{
    glEnable( GL_TEXTURE_2D );
    glEnable( GL_BLEND );
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
    glDisable( GL_BLEND );
}
}