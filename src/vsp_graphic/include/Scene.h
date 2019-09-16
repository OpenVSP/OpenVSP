#ifndef _VSP_GRAPHIC_SCENE_H
#define _VSP_GRAPHIC_SCENE_H

#include <vector>
#include <string>
#include <set>

#include "Common.h"

namespace VSPGraphic
{
class Lighting;
class Clipping;
class Selectable;
class Pickable;
class PickablePnts;
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

    virtual void preSelectBox(int x1, int y1, int x2, int y2);
    virtual bool selectBox();

    /*!
    * Select current picked point.  Return true if action is successful, else
    * return false.
    */
    virtual bool selectHighlight();

    /*!
    * Select a given location.  If selection is not required, return false.
    */
    virtual bool selectLocation(double x, double y, double z);

    /*!
    * Select all vertices from a pickable target.
    */
    virtual void selectAll(PickablePnts * target);

    /*!
    * Unselect all.
    */
    virtual void unselectAll();

    /*!
    * Hide all selected.
    */
    virtual void hideSelection();

    /*!
    * Display selected.
    */
    virtual void showSelection();

public:
    /*!
    * Get last selected Object.
    */
    virtual Selectable * getLastSelected();
    /*!
    * Remove last selected Object.
    */
    virtual void removeLastSelected();
    /*!
    * Get last Selectable Object with a specific group name.
    */
    virtual Selectable * getLastSelected( const std::string &group);
    /*!
    * Get all Selectable Objects with a specific group name.
    */
    virtual std::vector<Selectable*> getSelected( const std::string &group);

    virtual std::vector<Selectable*> getSelected();

    /*!
    * Remove target Selectable from selection.
    */
    virtual void removeSelected(Selectable * selected);

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
    /*!
    * Any picking operation in session?
    */
    bool isPickingEnabled();

public:
    /*!
    * Get light.
    */
    Lighting * getLights();
    Clipping * GetClipping();

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

    Clipping * _clip;

    bool _toPick;
    bool _toSelectLoc;

    bool _showSelection;

    Pickable * _highlighted;
    std::set< PickablePnts* > _preselected;
};
}
#endif
