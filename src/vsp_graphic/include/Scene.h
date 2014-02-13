#ifndef _VSP_GRAPHIC_SCENE_H
#define _VSP_GRAPHIC_SCENE_H

#include <vector>

#include "Common.h"

namespace VSPGraphic
{
class Renderable;
class Lighting;

/*!
* Scene class sets up and manages Renderable objects in scene.
*/
class Scene
{
public:
    /*!
    * Constructor.
    */
    Scene();
    /*!
    * Destructor.
    */
    virtual ~Scene();

public:
    /*!
    * Add an object to scene.
    * objectType - VSP_OBJECT_MARKER, VSP_OBJECT_ENTITY, VSP_OBJECT_PICKABLE_MARKER,
    * VSP_OBJECT_TEXTUREABLE_ENTITY.
    * id_out - object id out.
    */
    void createObject( Common::VSPenum objectType, unsigned int * id_out );
    /*!
    * Remove object from scene.
    */
    void removeObject( unsigned int id );

public:
    /*!
    * Get object.
    */
    Renderable * getObject( unsigned int id );
    /*!
    * Get light.
    */
    Lighting * getLights();
    /*!
    * Get ids.
    */
    std::vector<unsigned int> getIds();

public:
    /*!
    * Color Picking at mouse location.
    */
    virtual void pick( int x, int y );
    /*!
    * Preprocessing.
    */
    virtual void predraw();
    /*!
    * Draw Scene.
    */
    virtual void draw();
    /*!
    * Postprocessing.
    */
    virtual void postdraw();

public:
    /*!
    * Return if there is pickable object in scene.
    */
    bool hasPickable();

protected:
    /*!
    * Enable/disable predraw and postdraw.
    */
    virtual void _updateDrawModes();
    void _enableDrawModes( bool enablePredraw, bool enablePostdraw );

private:
    struct sceneInfo
    {
        unsigned int id;
        Renderable * object;
    };
    std::vector<sceneInfo> _sceneList;
    std::vector<unsigned int> _recycleBin;

    Lighting * _lights;

private:
    unsigned int _pCounter;
};
}
#endif