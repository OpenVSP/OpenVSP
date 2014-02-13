#include "VertexBuffer.h"

#define VERTEX_SIZE     3       // x, y, z 
#define NORMAL_SIZE     3       // Nx, Ny, Nz
#define TEXCOORD_SIZE   2       // u, v
#define PADDING_SIZE        0       // padding, no use

#define VERTEX_DATA_SIZE ((VERTEX_SIZE + NORMAL_SIZE + TEXCOORD_SIZE + PADDING_SIZE) * sizeof(float))  // bytes per vertex data

#define VERTEX_MEM_POS      (0 * sizeof(float))     // vertex memory position
#define NORMAL_MEM_POS      (3 * sizeof(float))     // normal
#define TEXCOORD_MEM_POS    (6 * sizeof(float))     // texture coordinate

namespace VSPGraphic
{
VertexBuffer::VertexBuffer() : VBO( GL_ARRAY_BUFFER )
{
}
VertexBuffer::~VertexBuffer()
{
}

void VertexBuffer::draw( GLenum primitive )
{
    if ( !_isSupport() )
    {
        return;
    }

    _setPointers();

    glEnableClientState( GL_VERTEX_ARRAY );
    glEnableClientState( GL_NORMAL_ARRAY );

    glDrawArrays( primitive, 0, _getDataSize() / VERTEX_DATA_SIZE );

    glDisableClientState( GL_NORMAL_ARRAY );
    glDisableClientState( GL_VERTEX_ARRAY );
}

void VertexBuffer::drawElem( GLenum primitive, unsigned int index_size, void * indices )
{
    if ( !_isSupport() )
    {
        return;
    }

    _setPointers();

    glEnableClientState( GL_VERTEX_ARRAY );
    glEnableClientState( GL_NORMAL_ARRAY );

    glDrawElements( primitive, index_size, GL_UNSIGNED_INT, indices );

    glDisableClientState( GL_NORMAL_ARRAY );
    glDisableClientState( GL_VERTEX_ARRAY );
}

unsigned int VertexBuffer::getVertexSize()
{
    return _getDataSize() / VERTEX_DATA_SIZE;
}

int VertexBuffer::getVertex3f( unsigned int index, float * v_out )
{
    if( index < 0 || index >= getVertexSize() )
    {
        return 0;
    }
    glBindBuffer( GL_ARRAY_BUFFER, _getID() );
    glGetBufferSubData( GL_ARRAY_BUFFER, index * VERTEX_DATA_SIZE, VERTEX_SIZE * sizeof( float ), v_out );
    glBindBuffer( GL_ARRAY_BUFFER, 0 );
    return 1;
}

void VertexBuffer::_setPointers()
{
    glBindBuffer( GL_ARRAY_BUFFER, _getID() );
    glVertexPointer( VERTEX_SIZE, GL_FLOAT, VERTEX_DATA_SIZE, ( const GLvoid* )VERTEX_MEM_POS );
    glNormalPointer( GL_FLOAT, VERTEX_DATA_SIZE, ( const GLvoid* )NORMAL_MEM_POS );

    // Set Texture Coordinates to Texture Unit 0.
    glClientActiveTexture( GL_TEXTURE0 );
    glTexCoordPointer( TEXCOORD_SIZE, GL_FLOAT, VERTEX_DATA_SIZE, ( const GLvoid* )TEXCOORD_MEM_POS );
    glEnableClientState( GL_TEXTURE_COORD_ARRAY );
}
}