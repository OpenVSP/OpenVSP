#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <string>

#include "VBO.h"

#define BUFFER_INITIAL      (1024)        // initial buffer size
#define BUFFER_INCREMENT    (1024)        // buffer increment size

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

    _id = 0;
    // Generate a buffer ID for vertex buffer.
    glGenBuffers( 1, &_id );

    // Initialize buffer.
    glBindBuffer( _buffer_Type, _id );
    glBufferData( _buffer_Type, BUFFER_INITIAL, nullptr, GL_DYNAMIC_DRAW );
    glBindBuffer( _buffer_Type, 0 );

    _start = _end = 0;
    _size = BUFFER_INITIAL;
    _overfull = false;
}
VBO::~VBO()
{
    if ( !_support )
    {
        return;
    }

    glDeleteBuffers( 1, &_id );
}

void VBO::append( void * mem_ptr, size_t mem_size )
{
    if ( !_support )
    {
        return;
    }

    size_t end = ( size_t )_end + mem_size;
    if ( end > MAX_BYTES )
    {
        if ( !_overfull )
        {
            std::printf( "Not drawing %.0f MB of graphics data: one buffer can hold at most %.0f MB.\n",
                         end / 1.0e6, MAX_BYTES / 1.0e6 );
        }
        _overfull = true;
        return;
    }

    _start = _end;
    _end = ( unsigned int )end;

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
    _overfull = false;
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

bool VBO::_extend( size_t mem_size )
{
    void * temp = malloc( _size );

    if( temp == nullptr )
    {
        assert( false ); // Failed to allocate memory.
        return false;
    }

    glBindBuffer( _buffer_Type, _id );
    glGetBufferSubData( _buffer_Type, 0, _size, temp );
    glBindBuffer( _buffer_Type, 0 );
    glDeleteBuffers( 1, &_id );

    unsigned int oldSize = _size;

    // Whole increments, enough for the new data.  The caller has kept _end within MAX_BYTES; the
    // capacity rounds up from there and is held to the same limit.
    size_t nincr = mem_size / BUFFER_INCREMENT;
    if ( mem_size % BUFFER_INCREMENT != 0 )
    {
        nincr++;
    }
    size_t newsize = ( size_t )_size + BUFFER_INCREMENT * nincr;
    if ( newsize > MAX_BYTES )
    {
        newsize = _end;
    }
    _size = ( unsigned int )newsize;

    glGenBuffers( 1, &_id );
    glBindBuffer( _buffer_Type, _id );
    glBufferData( _buffer_Type, _size, nullptr, GL_DYNAMIC_DRAW );
    glBufferSubData( _buffer_Type, 0, oldSize, temp );
    glBindBuffer( _buffer_Type, 0 );
    free( temp );

    return true;
}
}
