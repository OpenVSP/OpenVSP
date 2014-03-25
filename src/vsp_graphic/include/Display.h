#ifndef _VSP_GRAPHIC_VIEW_MANAGER_H
#define _VSP_GRAPHIC_VIEW_MANAGER_H

#include "Common.h"

#include <vector>

namespace VSPGraphic
{
class Scene;
class LayoutMgr;
class Camera;
class CameraMgr;
class Viewport;

/*!
* Handle multiple layouts of Display.
*/
class Display
{
public:
    /*!
    * Constructor.
    */
    Display();
    /*!
    * Destructor.
    */
    virtual ~Display();

public:
    /*!
    * Perform preprocessing on the scene. Color Picking.
    * x and y are current mouse location.
    */
    void predraw( Scene * scene, int x, int y );

    /*!
    * Draw Scene.
    * x and y are current mouse location.
    */
    void draw( Scene * scene, int x, int y );

    /*!
    * Set Display Layout.
    * row and column define the numbers of viewports in screen.
    */
    void setDisplayLayout( int row, int column );

    /*!
    * Resize Screen.
    */
    void resize( int width, int height );

    /*!
    * Select a Viewport base on the index of its order and current selected
    * viewportMgr.
    *
    * Viewport order(left to right, up and down):
    * | 0 | 1 |
    * |-------|
    * | 2 | 3 |
    *
    * Set index to -1 to unselect viewports.
    */
    virtual void selectViewport( int index );

    /*!
    * Change view type of current selected viewport on current selected ViewStyle.
    * If no viewport is selected, do nothing.
    *
    * Acceptable values are VSP_CAM_TOP, VSP_CAM_FRONT, VSP_CAM_LEFT, VSP_CAM_LEFT_ISO,
    * VSP_CAM_BOTTOM, VSP_CAM_REAR, VSP_CAM_RIGHT, VSP_CAM_RIGHT_ISO, VSP_CAM_CENTER.
    */
    virtual void changeView( Common::VSPenum type );

    /*!
    * Reset Geometry View.
    */
    virtual void resetView();

    /*!
    * Center Geometry.
    */
    virtual void center();

    /*!
    * Get current selected viewport.
    * Return NULL if no viewport is selected.
    */
    Viewport * getViewport();

    /*!
    * Get camera of selected viewport.
    * Return NULL if no viewport is selected.
    */
    Camera * getCamera();

    /*!
    * Get camera manager of selected viewport.
    * Return NULL if no viewport is selected.
    */
    CameraMgr * getCameraMgr();

    /*!
    * Get current active ViewLayoutMgr.
    */
    LayoutMgr * getLayoutMgr();

public:
    /*!
    * Select Viewport at x, y mouse location.
    */
    void selectViewport( int x, int y );

public:
    /*
    * Save current camera system.
    */
    void save( int index );

    /*
    * Load saved camera system.
    */
    void load( int index );

public:
    /*!
    * Rotate the scene.
    *
    * px, py - Previous xy mouse location.
    * cx, cy - Current xy mouse location.
    */
    virtual void rotate( int px, int py, int cx, int cy );

    /*!
    * Pan the scene.
    *
    * px, py - Previous xy mouse location.
    * cx, cy - Current xy mouse location.
    */
    virtual void pan( int px, int py, int cx, int cy );

    /*!
    * Pan the scene.
    *
    * x - number of units to pan on x-axis.
    * y - number of units to pan on y-axis.
    */
    virtual void pan( float x, float y );

    /*!
    * Zoom in or out scene.
    *
    * px, py - Previous xy mouse location.
    * cx, cy - Current xy mouse location.
    */
    virtual void zoom( int px, int py, int cx, int cy );

    /*!
    * Zoom in or out scene.
    *
    * zoomvalue - amount to zoom in or out.  Positive zooms out,
    * negative zooms in.
    */
    virtual void zoom( float zoomvalue );

protected:
    std::vector<LayoutMgr *> _layoutList;

private:
    LayoutMgr * _currLayout;
};
}
#endif
