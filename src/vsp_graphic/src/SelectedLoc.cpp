#include "SelectedLoc.h"

namespace VSPGraphic
{
SelectedLoc::SelectedLoc(double x, double y, double z) : Selectable()
{
    _loc = glm::vec3(x, y, z);
}
SelectedLoc::~SelectedLoc()
{
}

glm::vec3 SelectedLoc::getLoc()
{
    return _loc;
}

void SelectedLoc::_draw()
{
}
}
