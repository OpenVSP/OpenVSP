#include "SceneObject.h"

namespace VSPGraphic
{
SceneObject::SceneObject()
{
	_id = 0xFFFFFFFF;

    _visible = true;
    _predrawFlag = true;
}
SceneObject::~SceneObject()
{
}

void SceneObject::predraw()
{
	if(_visible && _predrawFlag)
	{
		_predraw();
	}
}

void SceneObject::draw()
{
	if(_visible)
	{
		_draw();
	}
}

void SceneObject::setVisibility(bool visible)
{
	_visible = visible;
}

void SceneObject::enablePredraw(bool enable)
{
	_predrawFlag = enable;
}

unsigned int SceneObject::getID()
{
	return _id;
}

void SceneObject::setID(unsigned int id)
{
	_id = id;
}
}