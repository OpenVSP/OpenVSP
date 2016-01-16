#ifndef _VSP_GRAPHIC_PICKING_GEOMETRY_H
#define _VSP_GRAPHIC_PICKING_GEOMETRY_H

#include "Pickable.h"

namespace VSPGraphic
{
/*!
* Pickable Geometry.  Derived from Pickable.
*/
class PickableGeom : public Pickable
{
public:
    /*!
    * Constructor.
    */
	PickableGeom(Renderable * source);
    /*!
    * Destructor.
    */
	virtual ~PickableGeom();

public:
    /*!
    * Overrides from Pickable.  Update unique color ids.
    */
	virtual void update();

protected:
    /*!
    * Preprocessing render.
    */
    virtual void _predraw();
    /*!
    * Draw.
    */
	virtual void _draw();
};
}
#endif
