#include "Renderable.h"

#include "Common.h"

#include "VertexBuffer.h"
#include "ColorBuffer.h"
#include "ElementBuffer.h"

namespace VSPGraphic
{
Renderable::Renderable()
{
    _type = Common::VSP_QUADS;
    _style = Common::VSP_DRAW_WIRE_FRAME;

    _vBuffer = new VertexBuffer();
    _cBuffer = new ColorBuffer();
    _eBuffer = new ElementBuffer();

    _meshColor.red = _meshColor.green = _meshColor.blue = 1.0f;
    _lineColor.red = _lineColor.green = _lineColor.blue = 0.f;
    _meshColor.alpha = _lineColor.alpha = _pointColor.alpha = 1.f;
    _pointColor.red = 1.f;
    _pointColor.green = _pointColor.blue = 0.f;

    _lineWidth = _pointSize = 1.0f;

    _visible = true;
    _cBufferFlag = _eBufferFlag = false;
    _predrawFlag = _postdrawFlag = false;
}
Renderable::~Renderable()
{
    delete _vBuffer;
    delete _cBuffer;
    delete _eBuffer;
}

void Renderable::appendVBuffer( void * mem_ptr, unsigned int mem_size )
{
    _vBuffer->append( mem_ptr, mem_size );
}

void Renderable::emptyVBuffer()
{
    _vBuffer->empty();
}

void Renderable::appendEBuffer( void * mem_ptr, unsigned int mem_size )
{
    _eBuffer->append( mem_ptr, mem_size );
}

void Renderable::emptyEBuffer()
{
    _eBuffer->empty();
}

void Renderable::enableEBuffer( bool enable )
{
    _eBufferFlag = enable;
}

void Renderable::appendCBuffer( void * mem_ptr, unsigned int mem_size )
{
    _cBuffer->append( mem_ptr, mem_size );
}

void Renderable::emptyCBuffer()
{
    _cBuffer->empty();
}

void Renderable::enableCBuffer( bool enable )
{
    _cBufferFlag = enable;
}

void Renderable::setMeshColor( float r, float g, float b, float a )
{
    _meshColor.red = r;
    _meshColor.green = g;
    _meshColor.blue = b;
    _meshColor.alpha = a;
}

void Renderable::setLineColor( float r, float g, float b, float a )
{
    _lineColor.red = r;
    _lineColor.green = g;
    _lineColor.blue = b;
    _lineColor.alpha = a;
}

void Renderable::setPointColor( float r, float g, float b, float a )
{
    _pointColor.red = r;
    _pointColor.green = g;
    _pointColor.blue = b;
    _pointColor.alpha = a;
}

void Renderable::setLineWidth( float width )
{
    _lineWidth = width;
}

void Renderable::setPointSize( float size )
{
    _pointSize = size;
}

void Renderable::setGeomType( Common::VSPenum type )
{
    _type = type;
}

void Renderable::setRenderStyle( Common::VSPenum style )
{
    _style = style;
}

void Renderable::setVisibility( bool isVisible )
{
    _visible = isVisible;
}

void Renderable::enablePredraw( bool enable )
{
    _predrawFlag = enable;
}

void Renderable::enablePostdraw( bool enable )
{
    _postdrawFlag = enable;
}

void Renderable::predraw()
{
    if( _visible && _getPreDrawFlag() )
    {
        _predraw();
    }
}

void Renderable::draw()
{
    if( _visible )
    {
        _draw();
    }
}

void Renderable::postdraw()
{
    if( _visible && _getPostDrawFlag() )
    {
        _postdraw();
    }
}
}