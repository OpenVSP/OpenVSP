#include "ElementBuffer.h"

namespace VSPGraphic
{
ElementBuffer::ElementBuffer() : VBO( GL_ELEMENT_ARRAY_BUFFER )
{
}
ElementBuffer::~ElementBuffer()
{
}

unsigned int ElementBuffer::getElemSize()
{
    return _getDataSize() / sizeof( unsigned int );
}
}