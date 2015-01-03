#ifndef _VSP_GRAPHIC_VIEWPORT_H
#define _VSP_GRAPHIC_VIEWPORT_H

#include "Common.h"
#include "glm/glm.hpp"

namespace VSPGraphic
{
class TextMgr;
class FontMgr;
class CameraMgr;
class Background;

/*!
* Viewport Class.
*/
class Viewport
{
public:
    /*!
    * Constructor.
    * (x, y) - lower left corner of the viewport.
    * width and height define the size of the viewport.
    */
    Viewport( int x, int y, int width, int height );
    /*!
    * Destructor.
    */
    virtual ~Viewport();

public:
    /*!
    * Apply Projection and Modelview Matrix to current matrix.
    */
    virtual void bind();
    /*!
    * Clean up after bind().
    */
    virtual void unbind();
    /*!
    * Resize viewport.
    */
    virtual void resize( int x, int y, int width, int height );
    /*!
    * Draw border.
    * If selected, draw border in red.
    */
    virtual void drawBorder( bool selected );
    /*!
    * Draw coordinate system vector arrows.
    */
    virtual void drawXYZArrows();
    /*!
    * Draw 2D Grid Overlay.
    */
    virtual void drawGridOverlay();
    /*!
    * Draw Background.
    */
    virtual void drawBackground();

public:
    /*!
    * Get Camera Manager Object of this Viewport.
    */
    CameraMgr* getCameraMgr();
    /*!
    * Get Background object of this Viewport.
    */
    Background * getBackground();

public:
    /*!
    * x
    * Lower left corner of viewport.
    */
    int x()
    {
        return _x;
    }

    /*!
    * y
    * Lower left corner of viewport.
    */
    int y()
    {
        return _y;
    }

    /*!
    * Viewport Width.
    */
    int width()
    {
        return _vWidth;
    }

    /*!
    * Viewport Height.
    */
    int height()
    {
        return _vHeight;
    }

public:
    /*
    * Transform screen coordinate to world coordinates.
    */
    glm::vec3 screenToWorld(glm::vec2 screenCoord);
    /*
    * Transform screen coordinate to normalized device coordinate.
    */
    glm::vec2 screenToNDC(glm::vec2 screenCoord);

    void drawRectangle( int sx, int sy, int x, int y );

public:
    /*
    * Show / hide Borders.
    * Show on default.
    */
    void showBorders( bool showFlag );

    /*
    * Show / hide XYZ Arrows.
    * Show on default.
    */
    void showXYZArrows( bool showFlag );

    /*
    * Show / hide Grid Overlay.
    * Hide on default.
    */
    void showGridOverlay( bool showFlag );

private:
    int _x;
    int _y;
    int _vWidth;
    int _vHeight;

    bool _showBorders;
    bool _showArrows;
    bool _showGrid;

    CameraMgr * _cameraMgr;
    Background * _background;

    TextMgr * _textMgr;
    FontMgr * _font;
};
}
#endif