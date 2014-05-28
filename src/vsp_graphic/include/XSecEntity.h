#ifndef _VSP_GRAPHIC_XSEC_ENTITY_H
#define _VSP_GRAPHIC_XSEC_ENTITY_H

#include "Entity.h"

namespace VSPGraphic
{
class Lighting;

/*!
* Cross Sections Entity.
*/
class XSecEntity : public Entity
{
public:
    /*!
    * Construct a XSecEntity.
    */
    XSecEntity();
    /*!
    * Construct a XSecEntity with lighting.
    */
    XSecEntity( Lighting * lights );
    /*!
    * Destructor.
    */
    virtual ~XSecEntity();

public:
    /*!
    * Set number of Cross Sections.
    */
    void setNumXSec( unsigned int value );
    /*!
    * Return number of Cross Sections.
    */
    unsigned int getNumXSec();

    /*!
    * Set number of Points.
    */
    void setNumPnts( unsigned int value );
    /*!
    * Return number of Points.
    */
    unsigned int getNumPnts();

private:
    unsigned int _numOfXSec;
    unsigned int _numOfPnts;
};
}
#endif