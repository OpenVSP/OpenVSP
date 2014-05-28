#ifndef _VSP_GRAPHIC_ARCBALL_WITH_QUATERNION_H
#define _VSP_GRAPHIC_ARCBALL_WITH_QUATERNION_H

#include <vector>

#include "Camera.h"
#include "glm/glm.hpp"

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
    * Overwrite Camera. Reset camera to default.
    */
    void resetView();
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
    * Panning the scene.
    *
    * px, py - previous xy mouse location.
    * cx, cy - current xy mouse location.
    */
    void pan( int px, int py, int cx, int cy );

    /*!
    * Panning the scene.
    *
    * x, y - how much distance to pan.  The distance unit is
    * in world space.
    */
    void pan( float x, float y );

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
    * zoomvalue - amount to zoom in or out.  Positive zooms out,
    * negative zooms in.
    */
    void zoom( float zoomvalue );

    /*!
    * Set the Center of Rotation.
    */
    void setCOR( float x, float y, float z );

public:
    /*
    * Save current camera system.
    *
    * index - index of the save.  Index is zero based, for now, only
    * limit to four save slots.  If index is already used, its
    * content will be overwrited with new camera system.
    */
    virtual void save( int index );

    /*
    * Load saved camera system.
    *
    * index - index to load.  If index is never used, nothing will happen.
    */
    virtual void load( int index );

private:
    glm::vec4 _normalizeQuat( glm::vec4 quat );
    glm::vec4 _multiplyQuats( glm::vec4 quat1, glm::vec4 quat2 );

    glm::vec3 _toSphere3D( int x, int y );
    glm::vec4 _toQuat( glm::vec3 axis, float angle );
    glm::mat4 _toMatrix( glm::vec4 quat );

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
    glm::vec2 _prevLB;
    glm::vec2 _prevMB;
    glm::vec2 _prevRB;
    glm::vec2 _prevAltLB;
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