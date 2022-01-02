#ifndef _VSP_GRAPHIC_ARCBALL_WITH_QUATERNION_H
#define _VSP_GRAPHIC_ARCBALL_WITH_QUATERNION_H

#include <vector>

#include "Camera.h"
#include "glm/glm.hpp"
#include "glm/ext.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/euler_angles.hpp"

// Return int with sign of passed val.
template < typename T >
int sgn( T val )
{
    return ( T( 0 ) < val ) - ( val < T( 0 ) );
}

namespace VSPGraphic
{
/*!
* Arcball Camera Style with Quaternion.
*/
class ArcballCam : public Camera
{
public:
    /*!
    * Constructor.
    */
    ArcballCam();
    /*!
    * Destructor.
    */
    virtual ~ArcballCam();

public:
    /*!
    * Center Geometry.
    */
    void center();
    /*!
    * Overwrite Camera. Resize viewport.
    */
    void resize( int x, int y, int width, int height );

public:
    /*!
    * Rotate the scene.
    *
    * px, py - previous xy mouse location.
    * cx, cy - current xy mouse location.
    */
    void rotate( int px, int py, int cx, int cy );

    /*!
    * Rotate the scene given 3d sphere coordinates.
    *
    * px, py - previous xy mouse location.
    * cx, cy - current xy mouse location.
    */
    void rotateSphere( float angleX, float angleY, float angleZ );

    /*!
     * Converts Eueler to point along a sphere
     * @return vec3 point along a sphere (normalized)
     */
//    glm::vec3 eulerToSphericalPoint( glm::vec3 eulerAngles );

    /*!
     * Retrieve the euler angles from the rotation matrix.
     *
     * Returns euler angles, yitch as x, yaw as y, roll as z.
     */
    glm::vec3 getRotationEulerAngles();

    /*!
    * Panning the scene.
    *
    * px, py - previous xy mouse location.
    * cx, cy - current xy mouse location.
    */
    void pan( int px, int py, int cx, int cy );

    /*!
     * Retrieves the pan values as a glm vec2
     * x - Distance in x direction.
     * y - Distance in y direction.
     */
    glm::vec2 getPanValues();

    /*!
    * Panning the scene relative to a point.
    *
    * x, y - how much distance to pan.  The distance unit is
    * the distance from a point in world space.
    */
    void relativePan( float x, float y );

    /*!
    * Zoom in or out.
    *
    * px, py - previous xy mouse location.
    * cx, cy - current xy mouse location.
    */
    void zoom( int px, int py, int cx, int cy );

    /*!
    * Zoom in or out.
    *
    * dx, dy - mouse scroll change.
    */
    void zoom( int dx, int dy );

    /*!
    * Zoom in or out relative to a point in world space.
    *
    * zoomvalue - amount to zoom in or out.  Positive zooms out,
    * negative zooms in.
    */
    void relativeZoom( float zoomvalue );

    /*!
     * Get Relative Zoom Value
     */
    float getRelativeZoomValue();

    /*!
    * Set the Center of Rotation.
    */
    void setCOR( float x, float y, float z );

    /*!
      * Retrieves the center of rotation values.
      * @return point of rotation.
      */
    glm::vec3 getCOR();

public:
    /*
    * Save current camera system.
    *
    * index - index of the save.  Index is zero based, for now, only
    * limit to four save slots.  If index is already used, its
    * content will be overwritten with new camera system.
    */
    virtual void save( int index );

    /*
    * Load saved camera system.
    *
    * index - index to load.  If index is never used, nothing will happen.
    */
    virtual void load( int index );

private:
    static glm::vec4 _normalizeQuat( glm::vec4 quat );
    glm::vec4 _multiplyQuats( glm::vec4 quat1, glm::vec4 quat2 );
    static glm::quat _multiplyGLMQuats( glm::quat quat1, glm::quat quat2 );

    glm::vec3 _toSphere3D( int x, int y );
    static glm::vec4 _toQuat( glm::vec3 axis, float angle );
    static glm::mat4 _toMatrix( glm::vec4 quat );
    static glm::vec4 _matrixToQuat ( glm::mat4 mat );

    // Normalize to [-1, 1] on x and y axis.
    glm::vec2 _toNDC2f( float x, float y );

private:
    glm::vec4 _mapOutsideSphereToQuat( int px, int py, int cx, int cy );

private:
    float _radius;
    glm::vec4 _globalQuat;
    glm::mat4 _rMat, _tMat;
    glm::mat4 _cMat;
    glm::vec2 _pan;
    glm::vec2 _center;

private:
    struct camInfo
    {
        bool empty;
        glm::vec4 globalQuat;
        glm::vec2 pan;
        glm::mat4 cor;
        Common::VSPenum projectionType;
        float orthoZoomValue;
        float perspZoomValue;
    };
    std::vector<camInfo> _saves;
};
}
#endif
