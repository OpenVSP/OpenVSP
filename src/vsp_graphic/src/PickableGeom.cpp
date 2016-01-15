#include "PickableGeom.h"
#include "Renderable.h"
#include "VertexBuffer.h"
#include "ElementBuffer.h"
#include "ColorBuffer.h"
#include "Entity.h"

namespace VSPGraphic
{
PickableGeom::PickableGeom(Renderable * source) : Pickable(source)
{
}
PickableGeom::~PickableGeom()
{
	_delColorBlock();
}

void PickableGeom::update()
{
	_genColorBlock(true);
}

void PickableGeom::_predraw()
{
    // FIXME: Only Cross Section Geometries are selectable for now.
    if(dynamic_cast<Entity*>(_rSource))
    {
        ElementBuffer * eBuffer = _rSource->getEBuffer();
        VertexBuffer * vBuffer = _rSource->getVBuffer();

        _cIndexBuffer->bind();
        eBuffer->bind();
        vBuffer->drawElem(GL_QUADS, eBuffer->getElemSize(), (void*)0);
        eBuffer->unbind();
        _cIndexBuffer->unbind();
    }
}

void PickableGeom::_draw()
{
	if(_rSource->getPrimType() == Common::VSP_QUADS &&_rSource->getEBufferFlag())
	{
		ElementBuffer * eBuffer = _rSource->getEBuffer();
		VertexBuffer * vBuffer = _rSource->getVBuffer();

		if(_highlighted)
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

            glColor3f(1.0f, 0.0f, 0.0f);

			eBuffer->bind();
			vBuffer->drawElem(GL_QUADS, eBuffer->getElemSize(), (void*)0);
			eBuffer->unbind();
		}

		// Reset highlights so highlighted point turns off
		// after mouse moved away.
		_highlighted = false;
	}
}
}