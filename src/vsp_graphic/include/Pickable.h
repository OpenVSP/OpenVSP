#ifndef _VSP_GRAPHIC_PICKABLE_H
#define _VSP_GRAPHIC_PICKABLE_H

#include "SceneObject.h"

#include <string>

namespace VSPGraphic
{
class ColorBuffer;
class Renderable;

/*!
* Base class for all Pickable objects.
*/
class Pickable : public SceneObject
{
public:
    /*!
    * Constructor.
    * source - The source provides render info for this pickable.
    */
    Pickable(Renderable * source);
    /*!
    * Destructor.
    */
    virtual ~Pickable();

public:
    /*!
    * Process picked Id. If current object is picked, return true.
    */
    virtual bool processPickingResult(unsigned int pickedId);

public:
    /*!
    * Get source.
    */
    Renderable * getSource();

public:
    /*!
    * Set this Pickable to specific group. Group identifies who
    * the feedback is for.
    */
    void setGroup(std::string group);
    /*!
    * Return the group name.
    */
    std::string getGroup();

public:
    /*!
    * Update pickable.  Must implement.
    */
    virtual void update() = 0;

protected:
    /*!
    * Generate a block of unique color ids for this Pickable.
    * If geomPicking, only generate one color for the whole
    * Pickable.  Else generate one color per vertex.
    */
    virtual void _genColorBlock(bool geomPicking);
    /*!
    * Clear all color ids.
    */
    virtual void _delColorBlock();

protected:
    Renderable * _rSource;
    ColorBuffer * _cIndexBuffer;

    bool _highlighted;

    std::string _groupName;

    struct
    {
        unsigned int start;
        unsigned int end;
    } _colorIndexRange;
};
}
#endif
