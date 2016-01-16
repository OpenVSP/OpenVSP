#include "Renderable.h"

#include "VertexBuffer.h"
#include "ColorBuffer.h"
#include "ElementBuffer.h"

namespace VSPGraphic
{
Renderable::Renderable() : SceneObject()
{
    _type = Common::VSP_QUADS;
    _style = Common::VSP_DRAW_WIRE_FRAME;

    _vBuffer = new VertexBuffer();
    _cBuffer = new ColorBuffer();
    _eBuffer = new ElementBuffer();

    _meshColor.red = _meshColor.green = _meshColor.blue = 1.f;
    _lineColor.red = _lineColor.green = _lineColor.blue = 0.f;
    _meshColor.alpha = _lineColor.alpha = _pointColor.alpha = 1.f;
    _pointColor.red = 1.f;
    _pointColor.green = _pointColor.blue = 0.f;
    _textColor.red = _textColor.green = _textColor.blue = 0.f;
    _textColor.alpha = 1.f;

    _lineWidth = _pointSize = _textSize = 1.0f;

    _cBufferFlag = _eBufferFlag = false;

    _facingCWFlag = false;
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

void Renderable::setFacingCW( bool flag )
{
    _facingCWFlag = flag;
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

void Renderable::setTextColor( float r, float g, float b, float a )
{
    _textColor.red = r;
    _textColor.green = g;
    _textColor.blue = b;
    _textColor.alpha = a;
}

void Renderable::setLineWidth( float width )
{
    _lineWidth = width;
}

void Renderable::setPointSize( float size )
{
    _pointSize = size;
}

void Renderable::setTextSize( float size )
{
    _textSize = size;
}

void Renderable::setPrimType( Common::VSPenum type )
{
    _type = type;
}

Common::VSPenum Renderable::getPrimType()
{
    return _type;
}

void Renderable::setRenderStyle( Common::VSPenum style )
{
    _style = style;
}

Common::VSPenum Renderable::getRenderStyle()
{
    return _style;
}

glm::vec3 Renderable::getVertexVec( unsigned int bufferIndex )
{
    float temp[3];

    _vBuffer->getVertex3f(bufferIndex, temp);

    return glm::vec3(temp[0], temp[1], temp[2]);
}

VertexBuffer * Renderable::getVBuffer()
{
    return _vBuffer;
}

ElementBuffer * Renderable::getEBuffer()
{
    return _eBuffer;
}

bool Renderable::getEBufferFlag()
{
    return _eBufferFlag;
}

bool Renderable::_getCBufferFlag()
{
    return _cBufferFlag;
}

bool Renderable::_getFacingCWFlag()
{
    return _facingCWFlag;
}

Renderable::Color Renderable::_getMeshColor()
{
    return _meshColor;
}

Renderable::Color Renderable::_getLineColor()
{
    return _lineColor;
}

Renderable::Color Renderable::_getPointColor()
{
    return _pointColor;
}

Renderable::Color Renderable::_getTextColor()
{
    return _textColor;
}

float Renderable::_getLineWidth()
{
    return _lineWidth;
}

float Renderable::_getPointSize()
{
    return _pointSize;
}

float Renderable::_getTextSize()
{
    return _textSize;
}
}
