#ifndef _VSP_ELEMENT_BUFFER_H
#define _VSP_ELEMENT_BUFFER_H

#include "VBO.h"

namespace VSPGraphic
{
/*!
* Element Buffer class.  Derived from VBO.
*/
class ElementBuffer : public VBO
{
public:
    /*!
    * Constructor.
    */
    ElementBuffer();
    /*!
    * Destructor.
    */
    virtual ~ElementBuffer();

public:
    /*!
    * Return the number of indices in Element Buffer.
    */
    unsigned int getElemSize();
};
}
#endif
