#ifndef _VSP_GRAPHIC_SELECTABLE_OBJECT_H
#define _VSP_GRAPHIC_SELECTABLE_OBJECT_H

#include "SceneObject.h"

#include <string>

namespace VSPGraphic
{
class Selectable : public SceneObject
{
public:
    Selectable();
    virtual ~Selectable();

public:
    void setGroup(std::string group);
    std::string getGroup();

protected:
    virtual void _predraw() {}

protected:
    std::string _groupName;
};
}
#endif