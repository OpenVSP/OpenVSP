#ifndef _VSP_GRAPHIC_PICKING_LOCATION_H
#define _VSP_GRAPHIC_PICKING_LOCATION_H

#include "Pickable.h"

namespace VSPGraphic
{
/*!
* Pickable Location.  Derived from Pickable.  This class is
* used to pick locations on screen without vertices.
*/
class PickableLoc : public Pickable
{
public:
    /*!
    * Constructor.
    */
	PickableLoc();
    /*!
    * Destructor.
    */
	virtual ~PickableLoc();

public:
    /*!
    * Override from Pickable.
    */
	virtual void update();

protected:
    /*!
    * Render preprocessing.
    */
	virtual void _predraw();
    /*!
    * Draw.
    */
	virtual void _draw();
};
}
#endif