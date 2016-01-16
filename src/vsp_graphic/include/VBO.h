#ifndef _VSP_VERTEX_BUFFER_OBJECT_H
#define _VSP_VERTEX_BUFFER_OBJECT_H

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
    */
    virtual void append( void * mem_ptr, unsigned int mem_size );
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
    virtual bool _extend( unsigned int mem_size );

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

private:
    GLuint _id;
    GLenum _buffer_Type;

    unsigned int _start;
    unsigned int _end;
    unsigned int _size;

    bool _support;
};
}
#endif
