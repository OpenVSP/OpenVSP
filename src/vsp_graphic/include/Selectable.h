#ifndef _VSP_GRAPHIC_SELECTABLE_OBJECT_H
#define _VSP_GRAPHIC_SELECTABLE_OBJECT_H

#include "SceneObject.h"

#include <string>

namespace VSPGraphic
{
/*!
* Base class for all Selectable objects.
*/
class Selectable : public SceneObject
{
public:
    /*!
    * Constructor.
    */
    Selectable();
    /*!
    * Destructor.
    */
    virtual ~Selectable();

public:
    /*!
    * Set this Selectable to a group. Group identifies who
    * the feedback is for.
    */
    void setGroup(std::string group);
    /*!
    * Get group name.
    */
    std::string getGroup();

protected:
    virtual void _predraw() {}

protected:
    std::string _groupName;
};
}
#endif