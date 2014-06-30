#include "SelectedPnt.h"
#include "Renderable.h"
#include "OpenGLHeaders.h"
#include "VertexBuffer.h"

#include <assert.h>

namespace VSPGraphic
{
SelectedPnt::SelectedPnt(Renderable * source, unsigned int index) : SelectedGeom(source)
{
    _index = index;
}
SelectedPnt::~SelectedPnt()
{
}

unsigned int SelectedPnt::getIndex()
{
    return _index;
}

void SelectedPnt::_predraw()
{
    glPointSize(20);
    glColor4f(1, 1, 1, 1);
    glm::vec3 selected = _rSource->getVertexVec(_index);
    glBegin(GL_POINTS);
    glVertex3f(selected[0], selected[1], selected[2]);
    glEnd();
}

void SelectedPnt::_draw()
{
    glPointSize(12);
    glColor3f(0, 0, 1);
    glm::vec3 selected = _rSource->getVertexVec(_index);
    glBegin(GL_POINTS);
    glVertex3f(selected[0], selected[1], selected[2]);
    glEnd();
}
}

