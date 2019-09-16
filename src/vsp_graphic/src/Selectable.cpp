#include "Selectable.h"
#include "Renderable.h"

namespace VSPGraphic
{
Selectable::Selectable() : SceneObject()
{
    _groupName = "";
}
Selectable::~Selectable()
{
}

void Selectable::setGroup( const std::string &group)
{
    _groupName = group;
}

std::string Selectable::getGroup()
{
    return _groupName;
}
}
