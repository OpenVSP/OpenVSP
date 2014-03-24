#include "Scene.h"
#include "OpenGLHeaders.h"
#include "Renderable.h"
#include "Pickable.h"
#include "SceneObject.h"
#include "Entity.h"
#include "Marker.h"
#include "Ruler.h"
#include "PickablePnts.h"
#include "PickableGeom.h"
#include "PickableLoc.h"
#include "SelectedPnt.h"
#include "SelectedLoc.h"
#include "Lighting.h"
#include "ByteOperationUtil.h"

#include <assert.h>

namespace VSPGraphic
{
Scene::Scene()
{
    _lights = new Lighting();

    _toPick = false;
    _toSelectLoc = false;

    _highlighted = NULL;
}
Scene::~Scene()
{
    delete _lights;

    // Clean Scene Objects.
    for(int i = 0; i < (int)_sceneList.size(); i++)
    {
        delete _sceneList[i];
    }

    // Clean all selections.
    for(int i = 0; i < (int)_selections.size(); i++)
    {
        delete _selections[i];
    }
}

void Scene::createObject(Common::VSPenum objectType, unsigned int * id_out, unsigned int sourceId)
{
    SceneObject * object = NULL;

    switch(objectType)
    {
    case Common::VSP_OBJECT_MARKER:
        object = new Marker();
        break;

    case Common::VSP_OBJECT_ENTITY:
        object = new Entity(_lights);
        break;

    case Common::VSP_OBJECT_RULER:
        object = new Ruler();
        break;

    case Common::VSP_OBJECT_PICK_GEOM:
        if(sourceId != 0xFFFFFFFF)
        {
            Renderable * sourceObj = dynamic_cast<Renderable*>(getObject(sourceId));
            if(sourceObj)
            {
                object = new PickableGeom(sourceObj);
            }
        }
        break;

    case Common::VSP_OBJECT_PICK_VERTEX:
        if(sourceId != 0xFFFFFFFF)
        {
            Renderable * sourceObj = dynamic_cast<Renderable*>(getObject(sourceId));
            if(sourceObj)
            {
                object = new PickablePnts(sourceObj);
            }
        }
        break;

    case Common::VSP_OBJECT_PICK_LOCATION:
        object = new PickableLoc();
        break;
    }

    assert(object);

    if(object)
    {
        _generateUniqueId(id_out);

        // Store object and cache id.
        object->setID(*id_out);
        _sceneList.push_back(object);
    }

    // Check if picking still needed.
    _updateFlags();
}

void Scene::removeObject(unsigned int id)
{
    for(int i = 0; i < (int)_sceneList.size(); i++)
    {
        if(_sceneList[i]->getID() == id)
        {
            // Recycle id.
            _recycleBin.push_back(_sceneList[i]->getID());

            // Remove selections on this object.
            Renderable * rObj = dynamic_cast<Renderable*>(_sceneList[i]);
            if(rObj)
            {
                _removeSelections(rObj);
            }

            // Clean up.
            delete _sceneList[i];
            _sceneList.erase(_sceneList.begin() + i);
            break;
        }
    }
    // Check picking still needed.
    _updateFlags();
}

SceneObject * Scene::getObject(unsigned int id)
{
    for(int i = 0; i < (int)_sceneList.size(); i++)
    {
        if(_sceneList[i]->getID() == id)
        {
            return _sceneList[i];
        }
    }
    return NULL;
}

Lighting * Scene::getLights()
{
    return _lights;
}

std::vector<unsigned int> Scene::getIds()
{
    std::vector<unsigned int> ids;

    for(int i = 0; i < (int)_sceneList.size(); i++)
    {
        ids.push_back(_sceneList[i]->getID());
    }
    return ids;
}

void Scene::activatePicking(int x, int y)
{		
    _highlighted = NULL;

    unsigned char index[4] = {0x00, 0x00, 0x00, 0x00};
    unsigned int id = 0;

    glReadPixels(x, y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, &index);

    bytesToUInt(index, &id);

    if(id)
    {
        for(int i = 0; i < (int)_sceneList.size(); i++)
        {
            Pickable * pickable = dynamic_cast<Pickable*>(_sceneList[i]);
            if(pickable)
            {
                if(pickable->processPickingResult(id))
                {
                    _highlighted = pickable;
                }
            }
        }
    }
}

bool Scene::selectHighlight()
{
    // If nothing is picked, return false.
    if(!_highlighted)
    {
        return false;
    }

    PickablePnts * pickPnts = dynamic_cast<PickablePnts*>(_highlighted);
    PickableGeom * pickGeom = dynamic_cast<PickableGeom*>(_highlighted);

    if(pickPnts)
    {
        unsigned int index = pickPnts->getIndex();
        if(index != 0xFFFFFFFF)
        {
            SelectedPnt * selected = new SelectedPnt(pickPnts->getSource(), index);
            selected->setGroup(pickPnts->getGroup());

            _selections.push_back(selected);
        }
    }
    else if(pickGeom)
    {
        SelectedGeom * selected = new SelectedGeom(pickGeom->getSource());
        selected->setGroup(pickGeom->getGroup());

        _selections.push_back(selected);
    }
    return true;
}

bool Scene::selectLocation(double x, double y, double z)
{
    // Ignore if no selection is needed.
    if(!_toSelectLoc)
    {
        return false;
    }

    // Set first PLoc in the scene list as the pickable target.
    PickableLoc * picked;
    for(int i = 0; i < (int)_sceneList.size(); i++)
    {
        picked = dynamic_cast<PickableLoc*>(_sceneList[i]);
        if(picked)
        {
            SelectedLoc * selected = new SelectedLoc(x, y, z);
            selected->setGroup(picked->getGroup());

            _selections.push_back(selected);
            return true;
        }
    }
    return false;
}

Selectable * Scene::getLastSelected()
{
    return _selections[_selections.size() - 1];
}

Selectable * Scene::getLastSelected(std::string group)
{
    int lastIndex = _selections.size() - 1;

    for(int i = lastIndex; i >= 0; i--)
    {
        if(_selections[i]->getGroup() == group)
        {
            return _selections[i];
        }
    }
    return NULL;
}

std::vector<Selectable*> Scene::getSelected(std::string group)
{
    std::vector<Selectable*> groupSelect;

    for(int i = 0; i < (int)_selections.size(); i++)
    {
        if(_selections[i]->getGroup() == group)
        {
            groupSelect.push_back(_selections[i]);
        }
    }
    return groupSelect;
}

bool Scene::isPickingEnabled()
{
    return _toPick;
}

void Scene::_generateUniqueId(unsigned int * id_out)
{
    static unsigned int _id_tracker = 0;
    if(_recycleBin.empty())
    {
        *id_out = _id_tracker++;
    }
    else
    {
        // Recycle used ids.
        *id_out = _recycleBin[_recycleBin.size() - 1];
        _recycleBin.pop_back();
    }
}

void Scene::_clearSelections()
{
    // Clean all selections.
    for(int i = 0; i < (int)_selections.size(); i++)
    {
        delete _selections[i];
    }
    _selections.clear();
}

void Scene::_removeSelections(Renderable * source)
{
    std::vector<Selectable*> keepers;

    for(int i = 0; i < (int)_selections.size(); i++)
    {
        SelectedGeom * matched = dynamic_cast<SelectedGeom*>(_selections[i]);
        if(matched && matched->getSource() != source)
        {
            keepers.push_back(_selections[i]);
        }
    }
    _selections = keepers;
}

void Scene::_updateFlags()
{
    _toPick = false;
    _toSelectLoc = false;

    // First to check if PLoc object exist.  If it does, enable picking
    // and location selection.
    for(int i = 0; i < (int)_sceneList.size(); i++)
    {
        PickableLoc * match = dynamic_cast<PickableLoc*>(_sceneList[i]);
        if(match)
        {
            _toPick = true;
            _toSelectLoc = true;
            return;
        }
    }

    // Check if scene list has pickable objects or not.  If there are non,
    // disable picking.
    for(int i = 0; i < (int)_sceneList.size(); i++)
    {
        Pickable * match = dynamic_cast<Pickable*>(_sceneList[i]);
        if(match)
        {
            _toPick = true;
            return;
        }
    }

    // No pickable objects in list, remove all selections.
    _clearSelections();
}

void Scene::predraw()
{
    for(int i = 0; i < (int)_sceneList.size(); i++)
    {
        _sceneList[i]->predraw();
    }
}

void Scene::draw()
{
    _lights->update();

    for(int i = 0; i < (int)_sceneList.size(); i++)
    {
        _sceneList[i]->draw();
    }

    for(int i = 0; i < (int)_selections.size(); i++)
    {
        _selections[i]->draw();
    }
}
}