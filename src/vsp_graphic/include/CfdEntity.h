#ifndef _VSP_GRAPHIC_CFD_ENTITY_H
#define _VSP_GRAPHIC_CFD_ENTITY_H

#include "Entity.h"

namespace VSPGraphic
{
class Lighting;

/*!
* CFD Entity.
*/
class CfdEntity : public Entity
{
public:
    /*!
    * Construct a CfdEntity.
    */
    CfdEntity();
    /*!
    * Construct a CfdEntity with lighting.
    */
    CfdEntity( Lighting * lights );
    /*!
    * Destructor.
    */
    virtual ~CfdEntity();

public:
    /*!
    * Override Entity.  Make rendering appear 'opaque' from inside.
    */
    virtual void _draw_Wire_Frame_Solid();
};
}
#endif