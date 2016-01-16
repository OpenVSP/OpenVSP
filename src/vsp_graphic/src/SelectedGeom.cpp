#include "SelectedGeom.h"
#include "Renderable.h"

namespace VSPGraphic
{
SelectedGeom::SelectedGeom(Renderable * source) : Selectable()
{
    _rSource = source;
}
SelectedGeom::~SelectedGeom()
{
}

Renderable * SelectedGeom::getSource()
{
    return _rSource;
}

void SelectedGeom::_draw()
{
}
}
