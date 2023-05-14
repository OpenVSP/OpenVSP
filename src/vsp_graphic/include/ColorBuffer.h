#ifndef _VSP_COLOR_BUFFER_H
#define _VSP_COLOR_BUFFER_H

#include "VBO.h"

namespace VSPGraphic
{
/*!
* Color Buffer class.  Derived from VBO.
*/
class ColorBuffer : public VBO
{
public:
    /*!
    * Constructor.
    */
    ColorBuffer( unsigned int t );
    /*!
    * Destructor.
    */
    virtual ~ColorBuffer();

public:
    /*!
    * Overwrite VBO, bind color buffer to current context.
    */
    void bind();
    /*!
    * Overwrite VBO, unbind color buffer.
    */
    void unbind();

    unsigned int _type;
};
}
#endif
