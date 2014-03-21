#include "PickableGeom.h"
#include "Renderable.h"
#include "VertexBuffer.h"
#include "ElementBuffer.h"
#include "ColorBuffer.h"

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
	// Fix Me: Only Quad Geometries with Element Buffer are selectable. This takes the 
	// assumption of Quad Geometries with Element Buffer is a Cross Section Geometry.
	// The reason is to keep Render Engine as generic as possible, the engine
	// does not track which renderable belongs to which geometry. In example, CFD
	// Mesh contains multiple renderables to render a complete mesh. The engine is
	// not able to id renderables that belong to this CFD Mesh.

	if(_rSource->getPrimType() == Common::VSP_QUADS &&_rSource->getEBufferFlag())
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