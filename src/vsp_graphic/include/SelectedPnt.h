#ifndef _VSP_GRAPHIC_VERTEX_SELECTABLE_H
#define _VSP_GRAPHIC_VERTEX_SELECTABLE_H

#include "SelectedGeom.h"

namespace VSPGraphic
{
class Renderable;

/*!
* Selected Point.
*/
class SelectedPnt : public SelectedGeom
{
public:
    /*!
    * Constructor.
    * source - Render info.
    * index - Index of selected point in buffer.
    */
    SelectedPnt(Renderable * source, unsigned int index);
    /*!
    * Destructor.
    */
    virtual ~SelectedPnt();

public:
    /*!
    * Return selected index.
    */
    unsigned int getIndex();

protected:
    /*!
    * Overrides SelectedGeom.
    */
    virtual void _draw();

protected:
    unsigned int _index;
};
}
#endif