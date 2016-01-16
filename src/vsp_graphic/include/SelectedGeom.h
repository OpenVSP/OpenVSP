#ifndef _VSP_GRAPHIC_GEOMETRY_SELECTABLE_H
#define _VSP_GRAPHIC_GEOMETRY_SELECTABLE_H

#include "Selectable.h"

namespace VSPGraphic
{
class Renderable;

/*!
* Selected Geometry.  Derived from Selectable.  This class
* stores a selected geometry.
*/
class SelectedGeom : public Selectable
{
public:
    /*!
    * Constructor.
    * source - The source provides render info for this selectable.
    */
    SelectedGeom(Renderable * source);
    /*!
    * Destructor.
    */
    virtual ~SelectedGeom();

public:
    /*!
    * Get render source.
    */
    virtual Renderable * getSource();

public:
    /*!
    * Override from Selectable.  Draw Highlight.
    */
    virtual void _draw();

protected:
    Renderable * _rSource;
};
}
#endif
