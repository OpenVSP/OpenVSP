#ifndef _VSP_GRAPHIC_VIEWPORT_MANAGER_H
#define _VSP_GRAPHIC_VIEWPORT_MANAGER_H

#include <vector>
#include "Common.h"

namespace VSPGraphic
{
class Scene;
class CameraMgr;
class Viewport;

/*!
* LayoutMgr class.
* Manager for Single or Multi-Viewport Display.
*/
class LayoutMgr
{
public:
    /*!
    * Constructor.
    * row and column define the numbers of viewports in screen.
    */
    LayoutMgr( int row, int column );
    /*!
    * Destructor.
    */
    virtual ~LayoutMgr();

public:
    /*!
    * Perform preprocessing. Color Picking.
    * x and y are current mouse location.
    */
    virtual void predraw( Scene * scene, int x, int y );

    /*!
    * Draw Scene.
    */
    virtual void draw( Scene * scene, int x, int y );

    /*!
    * Resize Screen.
    */
    virtual void resize( int width, int height );

    /*
    * Select a Viewport base on the index of its order.
    *
    * Viewport order(left to right, up and down):
    * | 0 | 1 |
    * |-------|
    * | 2 | 3 |
    *
    * Set index to -1 to unselect viewports.
    */
    virtual void selectViewport( int index );

    /*
    * Select a Viewport base on mouse location.
    * If mouse location is outside viewports, unselect viewports.
    */
    virtual void selectViewport( int x, int y );

    /*
    * Get current selected Viewport.
    * If no viewport is selected, return NULL pointer.
    */
    Viewport * getViewport();

public:
    /*
    * Get list of viewports in Manager.
    */
    std::vector<Viewport *> getViewports();

public:
    /*!
    * Return row of viewport in screen.
    */
    int row()
    {
        return _row;
    }

    /*!
    * Return column of viewport in screen.
    */
    int column()
    {
        return _column;
    }

    /*!
    * Return screen width in pixels.
    */
    int screenWidth()
    {
        return _sWidth;
    }

    /*!
    * Return screen height in pixels.
    */
    int screenHeight()
    {
        return _sHeight;
    }

protected:
    std::vector<Viewport *> _viewportList;

private:
    int _row;
    int _column;

    int _selected;

    int _sWidth;
    int _sHeight;

    std::vector<Common::VSPenum> _vOrder;
};
}
#endif