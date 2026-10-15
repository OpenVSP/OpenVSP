#include "Camera.h"

#include "glm/ext.hpp"

#define PERSPECTIVE_NEAR 0.1f
#define PERSPECTIVE_FAR 5000.0f

namespace VSPGraphic
{
Camera::Camera()
{

    _vx = _vy = 0;
    _vWidth = _vHeight = 1;

    _znear = -2500.0f;
    _zfar = 2500.0f;

    _oZoom = ORTHOGRAPHIC_ZOOM_INIT;
    _pZoom = PERSPECTIVE_ZOOM_INIT;

    _projectionType = Common::VSP_CAM_ORTHO;

    _rotationVersion = 0;

    _projectionMatrix = glm::mat4( 1.0 );
    _modelviewMatrix = glm::mat4( 1.0 );
}
Camera::~Camera()
{
}

glm::mat4 Camera::getModelViewMatrix()
{
    return _modelviewMatrix;
}

glm::mat4 Camera::getProjectionMatrix()
{
    return _projectionMatrix;
}

void Camera::resize( int x, int y, int width, int height )
{
    _vx = x;
    _vy = y;
    _vWidth = width;
    _vHeight = height;

    _calculateProjection();
}

void Camera::setZNearFar( float zn, float zf )
{
    _znear = zn;
    _zfar = zf;
}

void Camera::setProjection( Common::VSPenum type )
{
    _projectionType = type;
    _calculateProjection();
}

void Camera::changeView( Common::VSPenum type )
{
    switch( type )
    {
    case Common::VSP_CAM_TOP:
        setCOR( 0.0, 0.0, 0.0 );
        relativePan( 0.0, 0.0 );
        rotateSphere( 0, 0, 0 );
        break;

    case Common::VSP_CAM_FRONT:
        setCOR( 0.0, 0.0, 0.0 );
        relativePan( 0.0, 0.0 );
        rotateSphere( 0.0, - 90.0 * ( M_PI / 180.0 ), - 90.0 * ( M_PI / 180.0 ) );
        break;

    case Common::VSP_CAM_FRONT_YUP:
        setCOR( 0.0, 0.0, 0.0 );
        relativePan( 0.0, 0.0 );
        rotateSphere( 0.0, - 90.0 * ( M_PI / 180.0 ), 0.0 );
        break;

    case Common::VSP_CAM_LEFT:
        setCOR( 0.0, 0.0, 0.0 );
        relativePan( 0.0, 0.0 );
        rotateSphere( 90.0 * ( M_PI / 180.0 ), 0.0, 0.0 );
        break;

    case Common::VSP_CAM_LEFT_ISO:
        setCOR( 0.0, 0.0, 0.0 );
        relativePan( 0.0, 0.0 );
        rotateSphere( 0.1959132757121161 * M_PI, -0.25 * M_PI, -M_PI / 3.0 );
        break;

    case Common::VSP_CAM_BOTTOM:
        setCOR( 0.0, 0.0, 0.0 );
        relativePan( 0.0, 0.0 );
        rotateSphere( 0.0, 180.0 * ( M_PI / 180.0 ), 180.0 * ( M_PI / 180.0 ) );
        break;

    case Common::VSP_CAM_REAR:
        setCOR( 0.0, 0.0, 0.0 );
        relativePan( 0.0, 0.0 );
        rotateSphere( 0.0, 90.0 * ( M_PI / 180.0 ), 90.0 * ( M_PI / 180.0 ) );
        break;

    case Common::VSP_CAM_RIGHT:
        setCOR( 0.0, 0.0, 0.0 );
        relativePan( 0.0, 0.0 );
        rotateSphere( -90.0 * ( M_PI / 180.0 ), -180.0 * ( M_PI / 180.0 ), 0.0 );
        break;

    case Common::VSP_CAM_RIGHT_ISO:
        setCOR( 0.0, 0.0, 0.0 );
        relativePan( 0.0, 0.0 );
        rotateSphere( -0.1959132757121161 * M_PI, -0.25 * M_PI, -2.0 * M_PI / 3.0 );
        break;

    case Common::VSP_CAM_CENTER:
        center();
        break;

    default:
        break;
    }
}

void Camera::zoomToward( float zoomChange, int x, int y )
{
    float width = _vWidth;
    float height = _vHeight;
    int mx = x - _vx;
    int my = y - _vy;

    float zoom = getRelativeZoomValue();
    glm::vec2 pan = getPanValues();

    float mousePosBeforeX, mousePosBeforeY;
    mousePosBeforeX = ( ( ( zoom * width ) * ( ( mx / width ) - 0.5 ) - pan.x ) );
    mousePosBeforeY = ( ( ( zoom * height ) * ( ( my / height ) - 0.5 ) - pan.y ) );

    float target = zoom * zoomChange;
    relativeZoom( target );

    // Scale the pan by the zoom actually applied, which the limit may have changed.
    if ( getRelativeZoomValue() != target )
    {
        zoomChange = getRelativeZoomValue() / zoom;
    }

    zoom *= zoomChange;
    pan *= zoomChange;

    float mousePosAfterX, mousePosAfterY;
    mousePosAfterX = ( ( ( zoom * width ) * ( ( mx / width ) - 0.5 ) - pan.x ) );
    mousePosAfterY = ( ( ( zoom * height ) * ( ( my / height ) - 0.5 ) - pan.y )  );

    // Cancel out any effect the zoom had on pan position, so that the same location remains under the mouse.
    relativePan( pan.x + mousePosAfterX - mousePosBeforeX , pan.y + mousePosAfterY - mousePosBeforeY );
}

void Camera::centerAt( float x, float y )
{
    // Offset from the viewport center in eye units.
    float zoom = getRelativeZoomValue();
    float ex = zoom * ( x - _vx - 0.5f * _vWidth );
    float ey = zoom * ( y - _vy - 0.5f * _vHeight );

    glm::vec2 pan = getPanValues();
    relativePan( pan.x - ex, pan.y - ey );
}

void Camera::_calculateProjection()
{
    float range = 0;
    float w = _vWidth / 2.0;
    float h = _vHeight / 2.0;

    switch( _projectionType )
    {
    case Common::VSP_CAM_ORTHO:
        _leftB = -w * _oZoom;
        _rightB = w * _oZoom;
        _upB = -h * _oZoom;
        _downB = h * _oZoom;
        _projectionMatrix = glm::ortho( _leftB, _rightB, _upB, _downB, _znear, _zfar );
        break;

    case Common::VSP_CAM_PERSPECTIVE:
        _projectionMatrix = glm::perspective( _pZoom, ( float )_vWidth / ( float )_vHeight, PERSPECTIVE_NEAR, PERSPECTIVE_FAR );

        // Update bounds.
        range = glm::tan( glm::radians( _pZoom / 2.f ) ) * PERSPECTIVE_NEAR;
        _leftB = -range * ( ( float )_vWidth / ( float )_vHeight );
        _rightB = range * ( ( float )_vWidth / ( float )_vHeight );
        _downB = -range;
        _upB = range;
        break;

    default:
        assert( false ); // Unknown Projection Type
        _leftB = -w * _oZoom;
        _rightB = w * _oZoom;
        _upB = -h * _oZoom;
        _downB = h * _oZoom;
        _projectionMatrix = glm::ortho( _leftB, _rightB, _upB, _downB, _znear, _zfar );
        break;
    }
}
}
