#ifndef _VSP_VERTEX_BUFFER_OBJECT_H
#define _VSP_VERTEX_BUFFER_OBJECT_H

#include <cstddef>

#include "OpenGLHeaders.h"

namespace VSPGraphic
{
/*!
* Vertex Buffer Object class.
*/
class VBO
{
public:
    /*!
    * Constructor.
    * type - Type of Buffer.
    * GL_ARRAY_BUFFER, GL_COLOR_BUFFER, GL_ELEMENT_ARRAY_BUFFER, etc.
    */
    VBO( GLenum type );
    /*!
    * Destructor.
    */
    virtual ~VBO();

public:
    /*!
    * Push to the back of the buffer.
    * mem_ptr - data pointer.
    * mem_size - data size in bytes.
    * Data that would take the buffer past MAX_BYTES is not taken, and the buffer draws nothing
    * until it is emptied.
    */
    virtual void append( void * mem_ptr, size_t mem_size );
    /*!
    * Reset append location.  Does not dispose buffer.
    */
    virtual void empty();
    /*!
    * Bind buffer to current context.
    */
    virtual void bind();
    /*!
    * Unbind buffer.
    */
    virtual void unbind();

protected:
    virtual bool _extend( size_t mem_size );

protected:
    virtual GLuint _getID()
    {
        return _id;
    }
    virtual GLenum _getBufferType()
    {
        return _buffer_Type;
    }
    virtual unsigned int _getDataSize()
    {
        return _end;
    }
    virtual unsigned int _getCapacity()
    {
        return _size;
    }
    virtual bool _isSupport()
    {
        return _support;
    }
    // Some data was not taken because it would not fit; see append.
    virtual bool _isOverfull()
    {
        return _overfull;
    }

    // The largest buffer the graphics driver is trusted with.  The macOS driver accepts a larger
    // one without an error and then faults when it is filled.
    static const size_t MAX_BYTES = 0x7FFFFFFF;

private:
    GLuint _id;
    GLenum _buffer_Type;

    unsigned int _start;
    unsigned int _end;
    unsigned int _size;

    bool _support;
    bool _overfull;
};
}
#endif
