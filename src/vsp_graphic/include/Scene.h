#ifndef _VSP_GRAPHIC_SCENE_H
#define _VSP_GRAPHIC_SCENE_H

#include <vector>
#include <string>

#include "Common.h"

namespace VSPGraphic
{
class Lighting;
class Selectable;
class Pickable;
class Renderable;
class SceneObject;

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
    * Add an object to scene. ( Normal Entity )
    * objectType - VSP_OBJECT_MARKER, VSP_OBJECT_ENTITY, VSP_OBJECT_XSEC_ENTITY, VSP_OBJECT_RULER.
    * id_out - Object id out.
    */
    virtual void createObject( Common::VSPenum objectType, unsigned int * id_out );

    /*!
    * Add an object to scene. ( Picking )
    * objectType - VSP_OBJECT_PICK_VERTEX, VSP_OBJECT_PICK_GEOM, VSP_OBJECT_PICK_LOCATION.
    * id_out - Object id out.
    * sourceId - Picking / Selecting source id.  It's used to link pickable or selectable object to
    * a specific renderable object.  The renderable object provides render information for both.
    */
    virtual void createObject( Common::VSPenum objectType, unsigned int * id_out, unsigned int sourceId );

    /*!
    * Remove object from scene.
    */
    virtual void removeObject(unsigned int id);

public:
    /*!
    * Get object.
    */
    virtual SceneObject * getObject(unsigned int id);

    /*!
    * Get ids of all objects in scene.
    */
    std::vector<unsigned int> getIds();

public:
    /*!
    * Color Picking at mouse location.
    */
    virtual void activatePicking(int x, int y);

    /*!
    * Select current picked point.  Return true if action is successful, else
    * return false.
    */
    virtual bool selectHighlight();

    /*!
    * Select a given location.  If selection is not required, return false.
    */
    virtual bool selectLocation(double x, double y, double z);

public:
    /*!
    * Get last selected Object.
    */
    virtual Selectable * getLastSelected();
    /*!
    * Get last Selectable Object with a specific group name.
    */
    virtual Selectable * getLastSelected(std::string group);
    /*!
    * Get all Selectable Objects with a specific group name.
    */
    virtual std::vector<Selectable*> getSelected(std::string group);

public:
    /*!
    * Preprocessing.
    */
    virtual void predraw();
    /*!
    * Draw Scene.
    */
    virtual void draw();

public:
    bool isPickingEnabled();

public:
    /*!
    * Get light.
    */
    Lighting * getLights();

private:
    void _generateUniqueId(unsigned int * id_out);
    void _updateFlags();
    void _clearSelections();
    void _removeSelections(Renderable * source);

private:
    std::vector<SceneObject*> _sceneList;
    std::vector<Selectable*> _selections;
    std::vector<unsigned int> _recycleBin;

    Lighting * _lights;

    bool _toPick;
    bool _toSelectLoc;

    Pickable * _highlighted;
};
}
#endif