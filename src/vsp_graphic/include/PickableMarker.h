#ifndef _VSP_GRAPHIC_PICKABLE_MARKER_H
#define _VSP_GRAPHIC_PICKABLE_MARKER_H

#include "Marker.h"
#include "Common.h"

namespace VSPGraphic
{
class ColorBuffer;
class ColorCoder;

/*!
* This class enable all vertices in marker to be pickable.
*/
class PickableMarker : public Marker
{
public:
    /*!
    * Constructor.
    */
    PickableMarker();
    /*!
    * Destructor.
    */
    virtual ~PickableMarker();

public:
    /*!
    * Test if vertex is picked.
    */
    virtual void pick( unsigned int colorIndex );

public:
    /*!
    * Override from Renderable.
    */
    virtual void appendVBuffer( void * mem_ptr, unsigned int mem_size );
    /*!
    * Override from Renderable.
    */
    virtual void emptyVBuffer();

protected:
    /*!
    * Preprocessing for color picking.
    */
    virtual void _predraw();
    /*!
    * Draw Marker.
    */
    virtual void _draw();
    /*!
    * Postprocessing.
    */
    virtual void _postdraw();

protected:
    /*!
    * Get vertex (x, y, z) at color index.
    * If color index is not valid, return false.
    */
    bool _getVertex3fAtColorIndex( unsigned int colorIndex, float * v_out );
    /*!
    * Get color coder.
    */
    ColorCoder * _getColorCoder();
    /*!
    * Generate a block of unique color ids.
    */
    virtual void _genColorBlock();
    /*!
    * Free the block of unique color ids.
    */
    virtual void _delColorBlock();

private:
    /*!
    * Draw highlight.
    */
    void _drawHighLight( unsigned int index );

protected:
    struct
    {
        unsigned int start;
        unsigned int end;
    } _colorIndexRange;

    ColorBuffer * _cIndexBuffer;

    struct
    {
        bool hasPicked;
        unsigned int index;
    } _highlightMarker;
};
}
#endif