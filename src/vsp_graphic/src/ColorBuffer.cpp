#include "ColorBuffer.h"

#define COLOR_SIZE      4       // r, g, b, a

namespace VSPGraphic
{
ColorBuffer::ColorBuffer() : VBO( GL_ARRAY_BUFFER )
{
}
ColorBuffer::~ColorBuffer()
{
}

void ColorBuffer::bind()
{
    // Bind Color Buffer
    VBO::bind();
    glColorPointer( COLOR_SIZE, GL_UNSIGNED_BYTE, 0, ( void * )0 );
    VBO::unbind();

    // Enable Color Buffer
    glEnableClientState( GL_COLOR_ARRAY );
}

void ColorBuffer::unbind()
{
    // Disable Color Buffer
    glDisableClientState( GL_COLOR_ARRAY );
}
}