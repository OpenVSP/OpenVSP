#ifndef _VSP_GRAPHIC_VIEW_MANAGER_H
#define _VSP_GRAPHIC_VIEW_MANAGER_H

#include "Common.h"

#include <vector>
#include "glm/glm.hpp"

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
     * Saves the Default Screen size of OpenGL. Called once from MainGLWindow.cpp
     */
    void setDefaultScreenSize( int width, int height );

    /*!
     * Retrieve default OpenGL screen width.
     */
    int getDefaultGlWindowWidth( )
    {
        return _origWidth;
    }

    /*!
     * Retrieve default OpenGL screen height.
     */
    int getDefaultGlWindowHeight( )
    {
        return _origHeight;
    }

    /*!
     * Retrieve Screen Size Ratio
     */
    static float getScreenSizeDiffRatio();

    /*!
     * Resizing for screenshot
     */
    void resizeScreenshot( int width, int height );

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
    * Rotate the scene about a sphere angle and rotation axis.
    *
    * angle - Angle to rotate about an axis.
    * rotAxis - Axis of rotation
    */
    virtual void rotateSphere( float angleX, float angleY, float angleZ );

    /*!
     * Retrieves euler angles, yitch as x, yaw as y, roll as z.
     */
    virtual glm::vec3 getRotationEulerAngles();

    /*!
    * Pan the scene.
    *
    * px, py - Previous xy mouse location.
    * cx, cy - Current xy mouse location.
    */
    virtual void pan( int px, int py, int cx, int cy );

    /*!
     * Retrieves the pan values as a glm vec2
     * x - Distance in x direction.
     * y - Distance in y direction.
     */
    virtual glm::vec2 getPanValues();

    /*!
     * Pan the scene relative to a point.
     *
     * x - number of units to pan on the x-axis relative to a point.
     * y - number of units to pan on the y-axis relative to a point.
     */
    virtual void relativePan( float x, float y );

    /*!
    * Zoom in or out scene.
    *
    * px, py - Previous xy mouse location.
    * cx, cy - Current xy mouse location.
    */
    virtual void zoom( int px, int py, int cx, int cy );

    /*!
    * Zoom in or out.
    *
    * dx, dy - mouse scroll change.
    */
    virtual void zoom( int dx, int dy );

    /*!
     * Get Relative Zoom Value
     */
    virtual float getRelativeZoomValue();

    /*!
    * Zoom in or out scene relative to a point in world space.
    *
    * zoomvalue - amount to zoom in or out.  Positive zooms out,
    * negative zooms in.
    */
    virtual void relativeZoom( float zoomvalue );

    /*!
    * Set Center of Rotation.
    * x, y, z - location in world space.
    */
    virtual void setCOR( float x, float y, float z );

    /*!
      * Retrieves the center of rotation values.
      * @return point of rotation.
      */
    virtual glm::vec3 getCOR();

protected:
    std::vector<LayoutMgr *> _layoutList;

private:
    LayoutMgr * _currLayout;
    int _origWidth;
    int _origHeight;
};
}
#endif
