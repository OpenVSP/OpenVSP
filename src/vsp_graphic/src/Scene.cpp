#include <assert.h>

#include "OpenGLHeaders.h"

#include "Scene.h"

#include "Renderable.h"

#include "Entity.h"
#include "Marker.h"

#include "PickableMarker.h"

#include "Lighting.h"

#include "ByteOperationUtil.h"

namespace VSPGraphic
{
Scene::Scene()
{
    _lights = new Lighting();
    _pCounter = 0;
}
Scene::~Scene()
{
    delete _lights;

    for( int i = 0; i < ( int )_sceneList.size(); i++ )
    {
        delete _sceneList[i].object;
    }
}

void Scene::createObject( Common::VSPenum objectType, unsigned int * id_out )
{
    Renderable * object = NULL;
    switch( objectType )
    {
    case Common::VSP_OBJECT_MARKER:
        object = new Marker( Common::VSP_POINTS );
        break;

    case Common::VSP_OBJECT_ENTITY:
        object = new Entity( Common::VSP_QUADS, _lights );
        break;

    case Common::VSP_OBJECT_PICKABLE_MARKER:
        object = new PickableMarker( Common::VSP_POINTS );
        _pCounter++;
        break;
    }

    static unsigned int _id_tracker = 0;
    sceneInfo objectInfo;

    if( object )
    {
        // Generate an unique id for object.
        if( _recycleBin.empty() )
        {
            *id_out = _id_tracker++;
        }
        else
        {
            // Recycle used ids.
            *id_out = _recycleBin[_recycleBin.size() - 1];
            _recycleBin.pop_back();
        }

        // Store object and cache id.
        objectInfo.id = *id_out;
        objectInfo.object = object;

        _sceneList.push_back( objectInfo );
    }
    _updateDrawModes();
}

void Scene::removeObject( unsigned int id )
{
    for( int i = 0; i < ( int )_sceneList.size(); i++ )
    {
        if( _sceneList[i].id == id )
        {
            // Update pickable counter.
            if( dynamic_cast<PickableMarker*>( _sceneList[i].object ) )
            {
                _pCounter--;
            }

            // Recycle id.
            _recycleBin.push_back( _sceneList[i].id );

            // Clean up.
            delete _sceneList[i].object;
            _sceneList.erase( _sceneList.begin() + i );
            break;
        }
    }
    _updateDrawModes();
}

Renderable * Scene::getObject( unsigned int id )
{
    for( int i = 0; i < ( int )_sceneList.size(); i++ )
    {
        if( _sceneList[i].id == id )
        {
            return _sceneList[i].object;
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

    for( int i = 0; i < ( int )_sceneList.size(); i++ )
    {
        ids.push_back( _sceneList[i].id );
    }
    return ids;
}

bool Scene::hasPickable()
{
    if( _pCounter )
    {
        return true;
    }
    return false;
}

void Scene::pick( int x, int y )
{
    unsigned char index[4] = {0x00, 0x00, 0x00, 0x00};
    unsigned int id = 0;

    glReadPixels( x, y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, &index );

    bytesToUInt( index, &id );

    if( id )
    {
        for( int i = 0; i < ( int )_sceneList.size(); i++ )
        {
            PickableMarker * pickable = dynamic_cast<PickableMarker *>( _sceneList[i].object );
            if( pickable )
            {
                pickable->pick( id );
            }
        }
    }
}

void Scene::predraw()
{
    for( int i = 0; i < ( int )_sceneList.size(); i++ )
    {
        _sceneList[i].object->predraw();
    }
}

void Scene::draw()
{
    _lights->update();

    for( int i = 0; i < ( int )_sceneList.size(); i++ )
    {
        _sceneList[i].object->draw();
    }
}

void Scene::postdraw()
{
    for( int i = 0; i < ( int )_sceneList.size(); i++ )
    {
        _sceneList[i].object->postdraw();
    }
}

void Scene::_updateDrawModes()
{
    if( hasPickable() )
    {
        _enableDrawModes( true, true );
    }
    else
    {
        _enableDrawModes( false, false );
    }
}

void Scene::_enableDrawModes( bool enablePredraw, bool enablePostdraw )
{
    for( int i = 0; i < ( int )_sceneList.size(); i++ )
    {
        _sceneList[i].object->enablePredraw( enablePredraw );
        _sceneList[i].object->enablePostdraw( enablePostdraw );
    }
}
}