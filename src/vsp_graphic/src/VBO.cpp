#include <assert.h>
#include <vector>
#include <string>

#include "VBO.h"

#define BUFFER_INITIAL      (1024 * 1024)       // initial buffer size
#define BUFFER_INCREMENT    (1024 * 1024)       // buffer increment size

namespace VSPGraphic
{
VBO::VBO( GLenum type )
{
    // Make sure current driver supports Vertex Buffer Object
    _support = true;
    if ( !GL_ARB_vertex_buffer_object )
    {
        _support = false;
        const GLubyte* str = glGetString( GL_VERSION );
        std::printf("Current driver does not support Vertex Buffer Object.\n");
        std::printf("OpenGL version reports: %s\n", str );
        assert( false ); // Current driver does not support Vertex Buffer Object.
        return;
    }

    if ( !glGenBuffers )
    {
        const GLubyte* str = glGetString( GL_VERSION );
        std::printf("Current driver does not support Vertex Buffer Object.\n");
        std::printf("OpenGL version reports: %s\n", str );
        assert( false ); // Current driver does not support Vertex Buffer Object.
        return;
    }

    _buffer_Type = type;

    // Generate a buffer ID for vertex buffer.
    glGenBuffers( 1, &_id );

    // Initialize buffer.
    glBindBuffer( _buffer_Type, _id );
    glBufferData( _buffer_Type, BUFFER_INITIAL, NULL, GL_DYNAMIC_DRAW );
    glBindBuffer( _buffer_Type, 0 );

    _start = _end = 0;
    _size = BUFFER_INITIAL;
}
VBO::~VBO()
{
    if ( !_support )
    {
        return;
    }

    glDeleteBuffers( 1, &_id );
}

void VBO::append( void * mem_ptr, unsigned int mem_size )
{
    if ( !_support )
    {
        return;
    }

    _start = _end;
    _end = _end + mem_size;

    if( _end > _size )
    {
        // Increase Buffer Size
        if( !_extend( mem_size ) )
        {
            assert( false ); // Extend buffer failed.
            return;
        }
    }
    glBindBuffer( _buffer_Type, _id );
    glBufferSubData( _buffer_Type, _start, _end - _start, mem_ptr );
    glBindBuffer( _buffer_Type, 0 );
}

void VBO::empty()
{
    if( !_support )
    {
        return;
    }

    _start = _end = 0;
}

void VBO::bind()
{
    if ( !_support )
    {
        return;
    }

    glBindBuffer( _buffer_Type, _id );
}

void VBO::unbind()
{
    if ( !_support )
    {
        return;
    }

    glBindBuffer( _buffer_Type, 0 );
}

bool VBO::_extend( unsigned int mem_size )
{
    void * temp = malloc( _size );

    if( temp == NULL )
    {
        assert( false ); // Failed to allocate memory.
        return false;
    }

    glBindBuffer( _buffer_Type, _id );
    glGetBufferSubData( _buffer_Type, 0, _size, temp );
    glBindBuffer( _buffer_Type, 0 );
    glDeleteBuffers( 1, &_id );

    unsigned int oldSize = _size;
    _size = _size + BUFFER_INCREMENT * ( ( int )( mem_size / BUFFER_INCREMENT ) + ( mem_size % BUFFER_INCREMENT == 0 ? 0 : 1 ) );

    glGenBuffers( 1, &_id );
    glBindBuffer( _buffer_Type, _id );
    glBufferData( _buffer_Type, _size, NULL, GL_DYNAMIC_DRAW );
    glBufferSubData( _buffer_Type, 0, oldSize, temp );
    glBindBuffer( _buffer_Type, 0 );
    free( temp );

    return true;
}
}
