#include "VehicleGuiDraw.h"
#include "Labels.h"
#include "Lights.h"

VehicleGuiDraw::VehicleGuiDraw()
{
}
VehicleGuiDraw::~VehicleGuiDraw()
{
}

Labels * VehicleGuiDraw::getLabels()
{
    static Labels labels;
    return &labels;
}

Lights * VehicleGuiDraw::getLights()
{
    static Lights lights;
    return &lights;
}

