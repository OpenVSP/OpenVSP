#ifndef _VSP_GRAPHIC_LOCATION_SELECTABLE_H
#define _VSP_GRAPHIC_LOCATION_SELECTABLE_H

#include "Selectable.h"
#include "glm/glm.hpp"

namespace VSPGraphic
{
/*!
* Selected Location.
*/
class SelectedLoc : public Selectable
{
public:
    /*!
    * Constructor.
    * xyz - location to store.
    */
    SelectedLoc(double x, double y, double z);
    /*!
    * Destructor.
    */
    virtual ~SelectedLoc();

public:
    /*!
    * Return stored location.
    */
    glm::vec3 getLoc();

protected:
    /*!
    * Overrides SceneObject.
    */
    virtual void _draw();

private:
    glm::vec3 _loc;
};
}
#endif