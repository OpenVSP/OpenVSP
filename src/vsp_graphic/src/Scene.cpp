#include "Scene.h"
#include "OpenGLHeaders.h"
#include "Renderable.h"
#include "Pickable.h"
#include "Entity.h"
#include "Marker.h"
#include "Protractor.h"
#include "Ruler.h"
#include "Probe.h"
#include "Routing.h"
#include "PickablePnts.h"
#include "PickableGeom.h"
#include "PickableLoc.h"
#include "SelectedPnt.h"
#include "SelectedLoc.h"
#include "Lighting.h"
#include "Clipping.h"
#include "ByteOperationUtil.h"

namespace VSPGraphic
{
Scene::Scene()
{
    _lights = new Lighting();

    _clip = new Clipping();

    _toPick = false;
    _toSelectLoc = false;

    _showSelection = true;

    _highlighted = NULL;
}
Scene::~Scene()
{
    delete _lights;
    delete _clip;

    clearScene();
}

void Scene::clearScene()
{
    // Clean Scene Objects.
    for ( auto it = _sceneMap.begin(); it != _sceneMap.end(); ++it )
    {
        _recycleBin.push_back( it->first );
        delete it->second;
    }
    _sceneMap.clear();

    // Clean all selections.
    for(int i = 0; i < (int)_selections.size(); i++)
    {
        delete _selections[i];
    }
    _selections.clear();
}

void Scene::createObject(Common::VSPenum objectType, unsigned int * id_out)
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

    case Common::VSP_OBJECT_CFD_ENTITY:
        object = new Entity(_lights);
        break;

    case Common::VSP_OBJECT_PROTRACTOR:
        object = new Protractor();
        break;

    case Common::VSP_OBJECT_RULER:
        object = new Ruler();
        break;

    case Common::VSP_OBJECT_PROBE:
        object = new Probe();
        break;

    case Common::VSP_OBJECT_ROUTING:
        object = new Routing();
        break;

        default:
        break;
    }

    assert(object);

    if(object)
    {
        _generateUniqueId(id_out);

        // Store object and cache id.
        object->setID(*id_out);
        _sceneMap[ object->getID() ] = object;
    }
}

void Scene::createObject(Common::VSPenum objectType, unsigned int * id_out, unsigned int sourceId)
{
    SceneObject * object = NULL;
    Renderable * sourceObj = NULL;

    switch(objectType)
    {
    case Common::VSP_OBJECT_PICK_GEOM:
        sourceObj = dynamic_cast<Renderable*>(getObject(sourceId));
        if(sourceObj)
        {
            object = new PickableGeom(sourceObj);
        }
        break;

    case Common::VSP_OBJECT_PICK_VERTEX:
        sourceObj = dynamic_cast<Renderable*>(getObject(sourceId));
        if(sourceObj)
        {
            object = new PickablePnts(sourceObj);
        }
        break;

    case Common::VSP_OBJECT_PICK_LOCATION:
        object = new PickableLoc();
        break;

    default:
        break;
    }

    assert(object);

    if(object)
    {
        _generateUniqueId(id_out);

        // Store object and cache id.
        object->setID(*id_out);
        _sceneMap[ object->getID() ] = object;
    }

    // Check if picking still needed.
    _updateFlags();
}

void Scene::removeObject(unsigned int id)
{
    auto it = _sceneMap.find( id );
    if ( it != _sceneMap.end() )
    {
        // Recycle id.
        _recycleBin.push_back( id );

        // Remove selections on this object.
        Renderable * rObj = dynamic_cast<Renderable*>( it->second );
        if(rObj)
        {
            _removeSelections(rObj);
        }

        // Clean up.
        delete it->second;
        _sceneMap.erase( id );
    }
    // Check picking still needed.
    _updateFlags();
}

SceneObject * Scene::getObject(unsigned int id)
{
    auto it = _sceneMap.find( id );
    if( it == _sceneMap.end() )
    {
        return nullptr;
    }
    return it->second;
}

Lighting * Scene::getLights()
{
    return _lights;
}

Clipping * Scene::GetClipping()
{
    return _clip;
}

std::vector<unsigned int> Scene::getIds()
{
    std::vector<unsigned int> ids( _sceneMap.size() );

    int i = 0;
    for ( auto it = _sceneMap.begin(); it != _sceneMap.end(); ++it )
    {
        ids[ i ] = it->first;
        i++;
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
        for ( auto it = _sceneMap.begin(); it != _sceneMap.end(); ++it )
        {
            Pickable * pickable = dynamic_cast<Pickable*>( it->second );
            if(pickable)
            {
                PickablePnts * pkpts = dynamic_cast<PickablePnts*>(pickable);
                if(pkpts)
                {
                    pkpts->reset();
                }

                if(pickable->processPickingResult(id))
                {
                    _highlighted = pickable;
                }
            }
        }
    }
}

void Scene::preSelectBox(int x1, int y1, int x2, int y2)
{
    _preselected.clear();

    int w = std::abs( x2 - x1 );
    int h = std::abs( y2 - y1 );

    int x, y;

    if ( x1 < x2 )
        x = x1;
    else
        x = x2;

    if ( y1 < y2 )
        y = y1;
    else
        y = y2;

    int bpp = 4;
    int scanLen = bpp * ( w + 1 );

    unsigned char * index = new unsigned char[( h + 1 ) * scanLen];
    for ( int i = 0; i < ( h + 1 ) * scanLen; i++ )
    {
        index[i] = 0x00;
    }

    glReadPixels( x, y, w, h, GL_RGBA, GL_UNSIGNED_BYTE, index );


    std::set< unsigned int > ids;

    for ( int i = 0; i < h + 1; i++ )
    {
        for ( int j = 0; j < w + 1; j++ )
        {
            unsigned char * idx = index + (i * scanLen + j * bpp);
            unsigned int id = 0;
            bytesToUInt(idx, &id);

            if(id)
            {
                ids.insert(id);
            }
        }
    }

    for ( auto it = _sceneMap.begin(); it != _sceneMap.end(); ++it )
    {
        PickablePnts * pickable = dynamic_cast<PickablePnts*>( it->second );
        if(pickable)
        {
            pickable->reset();
        }
    }


    for ( auto idit = ids.begin(); idit != ids.end(); ++idit )
    {
        unsigned int id = (*idit);

        for ( auto it = _sceneMap.begin(); it != _sceneMap.end(); ++it )
        {
            PickablePnts * pickable = dynamic_cast<PickablePnts*>( it->second );
            if(pickable)
            {
                if( pickable->processPickingResult(id) )
                {
                    _preselected.insert( pickable );
                }
            }
        }
    }
    delete [] index;
}

bool Scene::selectBox()
{
    if ( _preselected.size() == 0 )
    {
        return false;
    }

    std::set< PickablePnts* >::iterator it;

    for(it = _preselected.begin(); it != _preselected.end(); ++it)
    {
        PickablePnts * pickable = dynamic_cast<PickablePnts*>( (*it) );
        if(pickable)
        {
            std::vector< int > index = pickable->getIndex();
            for ( int i = 0; i < index.size(); ++i )
            {
                SelectedPnt * selected = new SelectedPnt(pickable->getSource(), index[i]);
                selected->setGroup(pickable->getGroup());

                _selections.push_back(selected);
            }
        }
    }
    return true;
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
        std::vector< int > index = pickPnts->getIndex();
        for ( int i = 0; i < index.size(); ++i )
        {
            SelectedPnt * selected = new SelectedPnt(pickPnts->getSource(), index[i]);
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
    for ( auto it = _sceneMap.begin(); it != _sceneMap.end(); ++it )
    {
        picked = dynamic_cast<PickableLoc*>( it->second );
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

void Scene::selectAll(PickablePnts * target)
{
    // Select all points on target.
    std::vector<glm::vec3> pnts = target->getAllPnts();
    for( unsigned int i = 0; i < (int)pnts.size(); i++)
    {
        SelectedPnt * selected = new SelectedPnt(target->getSource(), i);
        selected->setGroup(target->getGroup());

        _selections.push_back(selected);
    }
}

void Scene::unselectAll()
{
    for(int i = 0; i < (int)_selections.size(); i++)
    {
        delete _selections[i];
    }
    _selections.clear();
}

Selectable * Scene::getLastSelected()
{
    return _selections[_selections.size() - 1];
}

void Scene::removeLastSelected()
{
    if ( !_selections.empty() )
    {
        delete _selections[_selections.size() - 1];
        _selections.pop_back();
    }
}

void Scene::removeSelected(Selectable* selected)
{
    for(int i = 0; i < (int)_selections.size(); i++)
    {
        if(_selections[i] == selected)
        {
            delete _selections[i];
            _selections.erase(_selections.begin() + i);
            return;
        }
    }
}

Selectable * Scene::getLastSelected( const std::string &group)
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

std::vector<Selectable*> Scene::getSelected( const std::string &group)
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


std::vector<Selectable*> Scene::getSelected()
{
    return _selections;
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
    for ( auto it = _sceneMap.begin(); it != _sceneMap.end(); ++it )
    {
        PickableLoc * match = dynamic_cast<PickableLoc*>( it->second );
        if(match)
        {
            _toPick = true;
            _toSelectLoc = true;
            return;
        }
    }

    // Check if scene list has pickable objects or not.  If there are none,
    // disable picking.
    for ( auto it = _sceneMap.begin(); it != _sceneMap.end(); ++it )
    {
        Pickable * match = dynamic_cast<Pickable*>( it->second );
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
    for ( auto it = _sceneMap.begin(); it != _sceneMap.end(); ++it )
    {
        it->second->predraw();
    }

    for(int i = 0; i < (int)_selections.size(); i++)
    {
        _selections[i]->predraw();
    }
}

void Scene::draw()
{
    std::vector<SceneObject*> alphaList;

    _lights->update();

    _clip->predraw();

    // Draw markers and entities that are not transparent.  Store transparent entities to render later.
    for ( auto it = _sceneMap.begin(); it != _sceneMap.end(); ++it )
    {
        Entity * entity = dynamic_cast<Entity*>( it->second );
        if( entity && entity->isTransparent() && 
            ( entity->getRenderStyle() == Common::VSP_DRAW_SHADED ||
              entity->getRenderStyle() == Common::VSP_DRAW_TEXTURED ||
              entity->getRenderStyle() == Common::VSP_DRAW_TEXTURED_TRANSPARENT_BACK ))
        {
            alphaList.push_back( it->second );
        }
        else
        {
            it->second->draw();
        }
    }

    // Draw selection points.
    if(_showSelection)
    {
        for(int i = 0; i < (int)_selections.size(); i++)
        {
            _selections[i]->draw();
        }
    }

    // Draw transparent entities.
    glDepthMask( GL_FALSE );
    glEnable( GL_CULL_FACE );
    glCullFace( GL_FRONT );
    for(int i = 0; i < (int)alphaList.size(); i++)
    {
        alphaList[i]->draw();
    }
    glCullFace( GL_BACK );
    for(int i = 0; i < (int)alphaList.size(); i++)
    {
        alphaList[i]->draw();
    }
    glDisable( GL_CULL_FACE );
    glDepthMask( GL_TRUE );

    _clip->postdraw();
}

void Scene::showSelection()
{
    _showSelection = true;
}

void Scene::hideSelection()
{
    _showSelection = false;
}
}
