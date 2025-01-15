#include <assert.h>

#include "OpenGLHeaders.h"

#include "Watermark.h"

#include "Texture2D.h"
#include "TextureMgr.h"
#include "VertexBuffer.h"

#include "TextMgr.h"
#include "glfont2.h"

namespace VSPGraphic
{
Watermark::Watermark() : Renderable()
{
    _textScale = 2.0f;

    _text = "Watermark";

    setFillColor( 1.0f, 1.0f, 1.0f, 1.0f );
    setLineColor( 0.0f, 0.0f, 0.0f, 1.0f );
    setTextColor( 0.0f, 0.0f, 0.0f, 1.0f );

    _vWidth = -1;
    _vHeight = -1;
    _screenSizeDiffRatio = 1.0f;
}

Watermark::~Watermark()
{
}

void Watermark::setWidthHeight( int w, int h, const float & screenSizeDiffRatio )
{
    if ( w == _vWidth && h == _vHeight )
    {
        return;
    }

    _vWidth = w;
    _vHeight = h;
    _screenSizeDiffRatio = screenSizeDiffRatio;
}

void Watermark::reset()
{
    _textScale = 2.0f;

    _text = "Watermark";

    setFillColor( 1.0f, 1.0f, 1.0f, 1.0f );
    setLineColor( 0.0f, 0.0f, 0.0f, 1.0f );
    setTextColor( 0.0f, 0.0f, 0.0f, 1.0f );
}

void Watermark::setText( const std::string &str )
{
    _text = str;
}

void Watermark::setTextScale( const float & s )
{
    _textScale = s;
}

void Watermark::setFillColor( float r, float g, float b, float a )
{
    setMeshColor( r, g, b, a );
}

void Watermark::_predraw()
{
}

void Watermark::_draw()
{
}

void Watermark::draw_watermark( TextMgr * textMgr )
{
    // Calculate screen aspect ratio.
    const float aspectRatioH = _vHeight > _vWidth ? _vHeight / ( float )_vWidth : 1.0f;
    const float aspectRatioW = _vWidth > _vHeight ? _vWidth / ( float )_vHeight : 1.0f;

    // Position in terms of screen (top, center)
    constexpr float x0 = 0;
    const float y0 = 1.0f - 0.036f / aspectRatioH; // magic value based on BORDER_OFFSET

    // Position in terms of text box size (top, center)
    constexpr float xoffset = 0;
    constexpr float yoffset = -0.5f;

    // Margin around text in fraction of screen width
    constexpr float textMargin = 0.01f;

    // Box line width
    constexpr float boxLineWidth = 3.0f;

    const Color fillcolor = _getMeshColor();
    const Color linecolor = _getLineColor();
    const Color textcolor = _getTextColor();


    // Calculate size of watermark box.  Duplicate from TextMgr::drawText().
    std::pair< int, int > dimension;
    textMgr->loadFont()->GetStringSize( _text.data(), &dimension );
    float w = ( ( float ) dimension.first ) * TextMgr::getBaseScale() * _textScale * aspectRatioH;
    float h = ( ( float ) dimension.second ) * TextMgr::getBaseScale() * _textScale * aspectRatioW;
    double x = x0 - w / 2 + w * xoffset;
    double y = y0 + h / 2 + h * yoffset;




    // Set Watermark box fill color.
    glColor4f( fillcolor.red, fillcolor.green, fillcolor.blue, fillcolor.alpha );

    // Draw filled box around watermark.
    glBegin( GL_QUADS );
    glVertex2f( x - textMargin * aspectRatioW, y + textMargin * aspectRatioH );
    glVertex2f( x + w + textMargin * aspectRatioW, y + textMargin * aspectRatioH );
    glVertex2f( x + w + textMargin * aspectRatioW, y - h - textMargin * aspectRatioH );
    glVertex2f( x - textMargin * aspectRatioW, y - h - textMargin * aspectRatioH );
    glEnd();

    // Set Watermark box color.
    glColor4f( linecolor.red, linecolor.green, linecolor.blue, linecolor.alpha );

    // Set Watermark box line thickness.
    glLineWidth( boxLineWidth * _screenSizeDiffRatio );

    // Draw box around watermark.
    glBegin( GL_LINE_LOOP );
    glVertex2f( x - textMargin * aspectRatioW, y + textMargin * aspectRatioH );
    glVertex2f( x + w + textMargin * aspectRatioW, y + textMargin * aspectRatioH );
    glVertex2f( x + w + textMargin * aspectRatioW, y - h - textMargin * aspectRatioH );
    glVertex2f( x - textMargin * aspectRatioW, y - h - textMargin * aspectRatioH );
    glEnd();

    // Set Watermark text color.
    glColor4f( textcolor.red, textcolor.green, textcolor.blue, textcolor.alpha );

    // Active Texture Image Unit 0.
    glActiveTexture( GL_TEXTURE0 );

    // Draw text.
    TextMgr::drawText( textMgr->loadFont(), _textScale, _text, x0, y0, xoffset, yoffset, aspectRatioW, aspectRatioH );
}


}
