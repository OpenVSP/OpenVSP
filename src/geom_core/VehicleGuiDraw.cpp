#include "VehicleGuiDraw.h"
#include "Labels.h"
#include "Lights.h"

VehicleGuiDraw::VehicleGuiDraw()
{
    m_Labels = new Labels();
    m_Lights = new Lights();
}
VehicleGuiDraw::~VehicleGuiDraw()
{
    delete m_Labels;
    delete m_Lights;
}

Labels * VehicleGuiDraw::getLabels()
{
    return m_Labels;
}

Lights * VehicleGuiDraw::getLights()
{
    return m_Lights;
}

