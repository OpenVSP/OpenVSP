#ifndef _VSP_GRAPHIC_POINTCLOUD_CONTROLPOINTS_H
#define _VSP_GRAPHIC_POINTCLOUD_CONTROLPOINTS_H

#include "Pickable.h"
#include "glm/glm.hpp"

#include <vector>
#include <set>

namespace VSPGraphic
{
class Renderable;

/*!
* Pickable Points.  Derived from Pickable.  This class
* makes all vertices on geometry pickable.
*/
class PickablePnts : public Pickable
{
public:
    /*!
    * Constructor.
    * source - Render source.
    */
	PickablePnts(Renderable * source);
    /*!
    * Destructor.
    */
	virtual ~PickablePnts();

public:
    /*!
    * Process picked id.  If id is picked, return true.
    */
	virtual bool processPickingResult(unsigned int pickedId);

public:
	/*!
	* Get highlighted buffer index.  If nothing is highlighted, return 0xffffffff.
	*/
	std::vector< int > getIndex();

	virtual void reset();

public:
    /*!
    * Overrides Pickable.  Update unique color ids.
    */
	virtual void update();

public:
    /*!
    * Adjust the range of picking.
    */
	void setPickRange(float range);
    /*!
    * Adjust the size of point.
    */
    void setPointSize(float size);

public:
    /*!
    * Get all pickable vertices.
    */
    std::vector<glm::vec3> getAllPnts();

protected:
    /*!
    * Overrides Pickable. Render preprocessing.
    */
	virtual void _predraw();
    /*!
    * Overrides Pickable. Draw highlighted point.
    */
	virtual void _draw();

private:
    float _pickRange;
    float _pointSize;
    std::set< int > _highlightedId;
};
}
#endif
