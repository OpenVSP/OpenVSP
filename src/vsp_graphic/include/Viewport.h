#ifndef _VSP_GRAPHIC_VIEWPORT_H
#define _VSP_GRAPHIC_VIEWPORT_H

#include "OpenGLHeaders.h"
#include "Common.h"
#include "glm/glm.hpp"
#include <vector>

namespace VSPGraphic
{
class TextMgr;
class Camera;
class Background;
class Watermark;

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
    * Resize viewport with camera
    */
    virtual void resize( int x, int y, int width, int height );

    /*!
     * Resize viewport only (not camera)
     */
    virtual void resizeViewport( int x, int y, int width, int height, float screenSizeDiffRatio );

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
    /*!
    * Draw Watermark.
    */
    virtual void drawWatermark();

public:
    /*!
    * Get Camera Manager Object of this Viewport.
    */
    Camera* getCamera();
    /*!
    * Get Background object of this Viewport.
    */
    Background * getBackground();

    virtual void clearBackground();
    /*!
    * Get Watermark object of this Viewport.
    */
    Watermark * getWatermark();

    virtual void clearWatermark();
    virtual void clearFont();

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
    glm::vec3 screenToWorld( const glm::vec2 &screenCoord);
    /*
    * Transform screen coordinate to normalized device coordinate.
    */
    glm::vec2 screenToNDC( const glm::vec2 &screenCoord);

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

    /*
    * Show / hide watermark.
    * Hide on default.
    */
    void showWatermark( bool showFlag );

    /*
    * Define Values for Grid Overlay
    * -5 to +5 spacing on default.
    */
    void setGridVals( const std::vector < double > &xGridVec, const std::vector < double > &yGridVec )
    {
        _xGridVec = xGridVec;
        _yGridVec = yGridVec;
    }

private:
    int _x;
    int _y;
    int _vWidth;
    int _vHeight;

    bool _showBorders;
    bool _showArrows;
    bool _showGrid;
    bool _showWatermark;

    float _screenSizeDiffRatio;

    std::vector < double > _xGridVec;
    std::vector < double > _yGridVec;

    Camera* _camera;
    Background * _background;

    Watermark * _watermark;

    TextMgr * _textMgr;
};
}
#endif
