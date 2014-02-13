#include <vector>

#include "PickableMarker.h"

#include "VertexBuffer.h"
#include "ColorBuffer.h"

#include "ColorCoder.h"

#define RANGE 20.0f

namespace VSPGraphic
{
PickableMarker::PickableMarker( Common::VSPenum geomType ) : Marker( geomType )
{
    _cIndexBuffer = new ColorBuffer();

    _colorIndexRange.start = _colorIndexRange.end = 0;

    _highlightMarker.hasPicked = false;
}
PickableMarker::~PickableMarker()
{
    _delColorBlock();

    delete _cIndexBuffer;
}

void PickableMarker::pick( unsigned int colorIndex )
{
    unsigned int index = colorIndex - _colorIndexRange.start;

    if( index >= 0 && index < _vBuffer->getVertexSize() )
    {
        _highlightMarker.hasPicked = true;
        _highlightMarker.index = index;
    }
}

void PickableMarker::appendVBuffer( void * mem_ptr, unsigned int mem_size )
{
    Marker::appendVBuffer( mem_ptr, mem_size );
    _genColorBlock();
}

void PickableMarker::emptyVBuffer()
{
    Marker::emptyVBuffer();
    _delColorBlock();
}

void PickableMarker::_predraw()
{
    // Set picking range.
    glPointSize( RANGE );

    // Draw picking points with unique color ids.
    _cIndexBuffer->bind();
    _vBuffer->draw( GL_POINTS );
    _cIndexBuffer->unbind();
}

void PickableMarker::_draw()
{
    Marker::_draw();

    if( _highlightMarker.hasPicked )
    {
        _drawHighLight( _highlightMarker.index );
    }
}

void PickableMarker::_postdraw()
{
    // Reset picking.
    _highlightMarker.hasPicked = false;
}

bool PickableMarker::_getVertex3fAtColorIndex( unsigned int colorIndex, float * v_out )
{
    unsigned int index = colorIndex - _colorIndexRange.start;

    if( _vBuffer->getVertex3f( index, v_out ) )
    {
        return true;
    }
    return false;
}

void PickableMarker::_genColorBlock()
{
    // Just in case this function is spammed without _delColorBlock(),
    // free block before create.
    _delColorBlock();

    // Allocate enough space for the block (four bytes per vertex).
    std::vector<unsigned char> colorblock;
    colorblock.resize( _vBuffer->getVertexSize() * 4 );

    // Create unique block of color indices.
    _getColorCoder()->genCodeBlock( _vBuffer->getVertexSize(), &_colorIndexRange.start, &_colorIndexRange.end, colorblock.data() );

    // Bind color indices to Color Buffer.
    _cIndexBuffer->empty();
    _cIndexBuffer->append( colorblock.data(), colorblock.size() );
}

void PickableMarker::_delColorBlock()
{
    _getColorCoder()->freeCodeBlock( _colorIndexRange.start, _colorIndexRange.end );

    // Clean up
    _colorIndexRange.start = _colorIndexRange.end = 0;
    _cIndexBuffer->empty();
}

ColorCoder * PickableMarker::_getColorCoder()
{
    static ColorCoder coder = ColorCoder();
    return &coder;
}

void PickableMarker::_drawHighLight( unsigned int index )
{
    glColor3f( 0.f, 1.f, 0.f );
    glPointSize( _getPointSize() * 1.2f );

    float v[3];
    if( _vBuffer->getVertex3f( index, v ) )
    {
        glBegin( GL_POINTS );
        glVertex3fv( v );
        glEnd();
    }
}
}