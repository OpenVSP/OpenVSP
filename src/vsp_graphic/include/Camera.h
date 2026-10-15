#ifndef _VSP_GRAPHIC_CAMERA_BASE_H
#define _VSP_GRAPHIC_CAMERA_BASE_H

#include <cmath>

#include "glm/glm.hpp"
#include "glm/gtc/quaternion.hpp"
#include "Common.h"

#define PERSPECTIVE_ZOOM_INIT 60.0f
#define ORTHOGRAPHIC_ZOOM_INIT 0.018f

namespace VSPGraphic
{
/*!
* A camera pose in world terms, for navigation devices.  The view is orthographic, so the eye
* point is synthetic: a point some distance behind the center of the view.
*/
struct CameraPose
{
    glm::dquat m_Rotation;      // Camera to world.  The camera looks along its -z with +y up.
    glm::dvec3 m_Eye;           // On the line through the center of the view, behind it.
    double m_OrthoExtent;       // Visible height of the view in world units.
};

/*!
* Camera Base Class.
*/
class Camera
{
public:
    /*!
    * Constructor.
    */
    Camera();
    /*!
    * Destructor.
    */
    virtual ~Camera();

public:
    /*!
    * Return current ProjectionMatrix.
    */
    virtual glm::mat4 getProjectionMatrix();
    /*!
    * Return current ModelViewMatrix.
    */
    virtual glm::mat4 getModelViewMatrix();
    /*!
    * Resize viewport.
    */
    virtual void resize( int x, int y, int width, int height );
    /*!
    * Set model extents.
    */
    virtual void setZNearFar( float zn, float zf );
    /*!
    * Set Projection type. Possible type are VSP_CAM_ORTHO and VSP_CAM_PERSPECTIVE.
    * Default: VSP_CAM_ORTHO
    */
    virtual void setProjection( Common::VSPenum type );

    /*!
     * Get Relative Zoom Value
     */
    virtual float getRelativeZoomValue() = 0;

    /*!
    * Change view type.
    * Acceptable values are VSP_CAM_TOP, VSP_CAM_FRONT, VSP_CAM_LEFT, VSP_CAM_LEFT_ISO,
    * VSP_CAM_BOTTOM, VSP_CAM_REAR, VSP_CAM_RIGHT, VSP_CAM_RIGHT_ISO, VSP_CAM_CENTER.
    */
    virtual void changeView( Common::VSPenum type );

public:
    /*
    * Save current camera system.
    *
    * index - index of the save.  Index is zero based.
    * If index is already used, its content will be
    * overwritten with new camera system.
    */
    virtual void save( int index ) = 0;

    /*
    * Load saved camera system.
    *
    * index - index to load.  If index is never used,
    * nothing will happen.
    */
    virtual void load( int index ) = 0;

    /*!
    * Remember the view -- rotation, pan, center of rotation and zoom -- and put it back
    * exactly.  One view is held; holding again replaces it.
    */
    virtual void holdView() = 0;
    virtual void restoreView() = 0;

public:
    /*!
    * Rotate the scene.
    *
    * px, py - previous xy mouse location.
    * cx, cy - current xy mouse location.
    */
    virtual void rotate( int px, int py, int cx, int cy ) = 0;

    /*!
    * Rotate the scene about camera's look axis.
    *
    * px, py - previous xy mouse location.
    * cx, cy - current xy mouse location.
    */
    virtual void rotateZ( int px, int py, int cx, int cy ) = 0;

    /*!
    * Rotate the scene given 3d sphere coordinates.
    *
    * px, py - previous xy mouse location.
    * cx, cy - current xy mouse location.
    */
    virtual void rotateSphere( float angleX, float angleY, float angleZ ) = 0;

    virtual glm::vec3 getRotationEulerAngles() = 0;

    /*!
    * Rotation from world to eye coordinates, as a unit quaternion.
    */
    virtual glm::quat getViewRotation() = 0;

    /*!
    * Set the rotation from world to eye coordinates.  The rotation is about the center of rotation.
    */
    virtual void setViewRotation( const glm::quat & rot ) = 0;

    /*!
    * Panning the scene.
    *
    * px, py - previous xy mouse location.
    * cx, cy - current xy mouse location.
    */
    virtual void pan( int px, int py, int cx, int cy ) = 0;

    /*!
     * Retrieves the pan values as a glm vec2
     * x - Distance in x direction.
     * y - Distance in y direction.
     */
    virtual glm::vec2 getPanValues() = 0;

    /*!
    * Panning the scene relative to a point in world space.
    *
    * x, y - how much distance to pan.
    */
    virtual void relativePan( float x, float y ) = 0;

    /*!
    * Zoom in or out.
    *
    * px, py - previous xy mouse location.
    * cx, cy - current xy mouse location.
    */
    virtual void zoom( int px, int py, int cx, int cy ) = 0;

    /*!
    * Zoom in or out.
    *
    * dx, dy - mouse scroll change.
    */
    virtual void zoom( int dx, int dy ) = 0;

    /*!
    * Zoom in or out relative to a point in world space.
    *
    * zoomvalue - amount to zoom in or out.  Positive zooms out,
    * negative zooms in.
    */
    virtual void relativeZoom( float zoomvalue ) = 0;

    /*!
    * Zoom by a factor, keeping the world point under a mouse location fixed on screen.
    *
    * zoomChange - factor applied to the zoom value.  Greater than one zooms out.
    * x, y - mouse location in window pixels.
    */
    void zoomToward( float zoomChange, int x, int y );

    /*!
    * Pan so that the world point under a window location moves to the viewport center.
    *
    * x, y - location in window pixels.
    */
    void centerAt( float x, float y );

    /*!
    * Set the center of rotation without moving the image: the pan changes to make up for
    * it.  setCOR() leaves the pan, so the model moves on screen.  x, y, z are as for setCOR().
    */
    void setCORKeepView( float x, float y, float z );

    /*!
    * The pose, with the eye eyeDistance behind the world point at the center of the view.
    */
    CameraPose getPose( double eyeDistance );

    /*!
    * Set rotation, pan and zoom from a pose, and return the pose that results (the camera's
    * own zoom floor may limit it; a caller with tighter limits applies them first).  The
    * center of rotation is kept, and the eye's distance along the view direction has no
    * effect, as the view is orthographic.
    */
    CameraPose setPose( const CameraPose & pose );

    /*!
    * Turn the view about an axis in eye coordinates (+x right, +y up, +z toward the viewer),
    * through the center of rotation, by the axis vector's length in radians.
    */
    void rotateEye( const glm::dvec3 & rotation );

    /*!
    * Move Geometry to center.
    */
    virtual void center() = 0;

    /*!
    * Set Center of Rotation.  XYZ are in world space.
    */
    virtual void setCOR( float x, float y, float z ) = 0;

    /*!
     * Retrieves the center of rotation values.
     * @return point of rotation.
     */
    virtual glm::vec3 getCOR() = 0;

public:
    /*!
    * Viewport lower left corner, width and height in window pixels.
    */
    int viewportX()
    {
        return _vx;
    }
    int viewportY()
    {
        return _vy;
    }
    int viewportWidth()
    {
        return _vWidth;
    }
    int viewportHeight()
    {
        return _vHeight;
    }

    /*!
    * Count of changes to the view rotation, by any means.  A drag measured from its press
    * compares it to tell when something else turned the view.
    */
    int rotationVersion()
    {
        return _rotationVersion;
    }

protected:
    virtual void _calculateProjection();

protected:
    // Transformation matrices.
    glm::mat4 _modelviewMatrix;
    glm::mat4 _projectionMatrix;

    // Viewport x, y, width and height.
    int _vx;
    int _vy;
    int _vWidth;
    int _vHeight;

    // Representative length for z-clipping.
    float _znear;
    float _zfar;

    // Zoom Value
    float _oZoom;
    float _pZoom;

    // Borders
    float _leftB;
    float _rightB;
    float _upB;
    float _downB;

    // Type of Projection.
    Common::VSPenum _projectionType;

    int _rotationVersion;
};
}
#endif
