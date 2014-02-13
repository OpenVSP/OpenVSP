#ifndef _VSP_VERTEX_BUFFER_H
#define _VSP_VERTEX_BUFFER_H

#include "VBO.h"

namespace VSPGraphic
{
/*!
* Vertex Buffer class.  This class is derived from VBO.
*/
class VertexBuffer : public VBO
{
public:
    /*!
    * Constructor.
    */
    VertexBuffer();
    /*!
    * Destructor.
    */
    virtual ~VertexBuffer();

public:
    /*!
    * Draw Data in Vertex Buffer.
    * primitive - GL_LINES, GL_TRIANGLES, GL_QUADS, etc...
    */
    void draw( GLenum primitive );
    /*!
    * Draw Data using glDrawElements.
    */
    void drawElem( GLenum primitive, unsigned int index_size, void * indices );

public:
    /*!
    * Return number of vertices.
    */
    unsigned int getVertexSize();
    /*!
    * Get Vertex info at index.
    * Return 0 if operation failed.
    */
    int getVertex3f( unsigned int index, float * v_out );

private:
    void _setPointers();
};
}
#endif