#include "Marker.h"

#include "VertexBuffer.h"
#include "ColorBuffer.h"
#include "ElementBuffer.h"

#include "Display.h"

namespace VSPGraphic
{
Marker::Marker() : Renderable()
{
}
Marker::~Marker()
{
}

void Marker::_predraw()
{
}

void Marker::_draw()
{
    switch( getPrimType() )
    {
    case Common::VSP_POINTS:
        _draw_Points();
        break;

    case Common::VSP_LINES:
        _draw_Lines();
        break;

    case Common::VSP_LINE_LOOP:
        _draw_Lines();
        break;

    case Common::VSP_LINE_STRIP:
        _draw_Lines();
        break;

    default:
        break;
    }
}

void Marker::_draw_Points()
{
    bool cBufferEnabled = _getCBufferFlag();
    Color color = _getPointColor();

    if( cBufferEnabled )
    {
        _cBuffer->bind();
    }

    _draw_Points( color.red, color.green, color.blue, color.alpha );

    if( cBufferEnabled )
    {
        _cBuffer->unbind();
    }
}

void Marker::_draw_Points( float r, float g, float b, float a, float size )
{
    bool eBufferEnabled = getEBufferFlag();

    glColor4f( r, g, b, a );
    if( size == 0.f )
    {
        glPointSize( _getPointSize() );
    }
    else
    {
        glLineWidth( size * Display::getScreenSizeDiffRatio() );
    }

    if( eBufferEnabled )
    {
        _draw_EBuffer();
    }
    else
    {
        _draw_VBuffer();
    }
}

void Marker::_draw_Lines()
{
    bool cBufferEnabled = _getCBufferFlag();
    Color color = _getLineColor();

    if( cBufferEnabled )
    {
        _cBuffer->bind();
    }

    _draw_Lines( color.red, color.green, color.blue, color.alpha );

    if( cBufferEnabled )
    {
        _cBuffer->unbind();
    }
}

void Marker::_draw_Lines( float r, float g, float b, float a, float width )
{
    bool eBufferEnabled = getEBufferFlag();

    glColor4f( r, g, b, a );
    if( width == 0.f )
    {
        glLineWidth( _getLineWidth() * Display::getScreenSizeDiffRatio() );
    }
    else
    {
        glLineWidth( width * Display::getScreenSizeDiffRatio() );
    }

    if( eBufferEnabled )
    {
        _draw_EBuffer();
    }
    else
    {
        _draw_VBuffer();
    }
}

void Marker::_draw_VBuffer()
{
    switch( getPrimType() )
    {
    case Common::VSP_POINTS:
        _vBuffer->draw( GL_POINTS );
        break;

    case Common::VSP_LINES:
        _vBuffer->draw( GL_LINES );
        break;

    case Common::VSP_LINE_LOOP:
        _vBuffer->draw( GL_LINE_LOOP );
        break;

    case Common::VSP_LINE_STRIP:
        _vBuffer->draw( GL_LINE_STRIP );
        break;

    default:
        break;
    }
}

void Marker::_draw_EBuffer()
{
    _eBuffer->bind();
    switch( getPrimType() )
    {
    case Common::VSP_POINTS:
        _vBuffer->drawElem( GL_POINTS, _eBuffer->getElemSize(), ( void* )0 );
        break;

    case Common::VSP_LINES:
        _vBuffer->drawElem( GL_LINES, _eBuffer->getElemSize(), ( void* )0 );
        break;

    case Common::VSP_LINE_LOOP:
        _vBuffer->drawElem( GL_LINE_LOOP, _eBuffer->getElemSize(), ( void* )0 );
        break;

    case Common::VSP_LINE_STRIP:
        _vBuffer->drawElem( GL_LINE_STRIP, _eBuffer->getElemSize(), ( void* )0 );
        break;

    default:
        break;
    }
    _eBuffer->unbind();
}
}
