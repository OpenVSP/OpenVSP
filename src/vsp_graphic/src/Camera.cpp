#include "Camera.h"

#include "glm/ext.hpp"

#define ORTHOGRAPHIC_Z_NEAR -2500.0f
#define ORTHOGRAPHIC_Z_FAR 2500.0f
#define PERSPECTIVE_NEAR 0.1f
#define PERSPECTIVE_FAR 5000.0f

namespace VSPGraphic
{
Camera::Camera()
{
    changeView( Common::VSP_CAM_TOP );

    _vx = _vy = 0;
    _vWidth = _vHeight = 1;

    _oZoom = ORTHOGRAPHIC_ZOOM_INIT;
    _pZoom = PERSPECTIVE_ZOOM_INIT;

    _projectionType = Common::VSP_CAM_ORTHO;

    _projectionMatrix = glm::mat4( 1.0 );
    _modelviewMatrix = glm::mat4( 1.0 );

    _viewMatrix = glm::mat4( 1.0 );
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

void Camera::resetView()
{
    _modelviewMatrix = _viewMatrix * glm::mat4( 1.0f );
}

void Camera::resize( int x, int y, int width, int height )
{
    _vx = x;
    _vy = y;
    _vWidth = width;
    _vHeight = height;

    _calculateProjection();
}

void Camera::setProjection( Common::VSPenum type )
{
    _projectionType = type;
    _calculateProjection();
}

void Camera::setZoomValue( float value )
{
    _oZoom = value * ( _vWidth < _vHeight ? 1.f / _vWidth : 1.f / _vHeight );
    _calculateProjection();
}

float Camera::getZoomValue()
{
    return _oZoom / ( _vWidth < _vHeight ? 1.f / _vWidth : 1.f / _vHeight );
}

void Camera::changeView( Common::VSPenum type )
{
    switch( type )
    {
    case Common::VSP_CAM_TOP:
        _top();
        _viewMatrix = glm::lookAt( _eye, _focus, _camUp );
        resetView();
        break;

    case Common::VSP_CAM_FRONT:
        _front();
        _viewMatrix = glm::lookAt( _eye, _focus, _camUp );
        resetView();
        break;

    case Common::VSP_CAM_LEFT:
        _left();
        _viewMatrix = glm::lookAt( _eye, _focus, _camUp );
        resetView();
        break;

    case Common::VSP_CAM_LEFT_ISO:
        _left_Iso();
        _viewMatrix = glm::lookAt( _eye, _focus, _camUp );
        resetView();
        break;

    case Common::VSP_CAM_BOTTOM:
        _bottom();
        _viewMatrix = glm::lookAt( _eye, _focus, _camUp );
        resetView();
        break;

    case Common::VSP_CAM_REAR:
        _rear();
        _viewMatrix = glm::lookAt( _eye, _focus, _camUp );
        resetView();
        break;

    case Common::VSP_CAM_RIGHT:
        _right();
        _viewMatrix = glm::lookAt( _eye, _focus, _camUp );
        resetView();
        break;

    case Common::VSP_CAM_RIGHT_ISO:
        _right_Iso();
        _viewMatrix = glm::lookAt( _eye, _focus, _camUp );
        resetView();
        break;

    case Common::VSP_CAM_CENTER:
        center();
        break;

    default:
        break;
    }
}

void Camera::_top()
{
    _eye = glm::vec3( 0.0f, 0.0f, 1.0f );
    _focus = glm::vec3( 0.0f, 0.0f, 0.0f );
    _camUp = glm::vec3( 0.0f, 1.0f, 0.0f );
}

void Camera::_front()
{
    _eye = glm::vec3( -1.0f, 0.0f, 0.0f );
    _focus = glm::vec3( 0.0f, 0.0f, 0.0f );
    _camUp = glm::vec3( 0.0f, 0.0f, 1.0f );
}

void Camera::_left()
{
    _eye = glm::vec3( 0.0f, -1.0f, 0.0f );
    _focus = glm::vec3( 0.0f, 0.0f, 0.0f );
    _camUp = glm::vec3( 0.0f, 0.0f, 1.0f );
}

void Camera::_left_Iso()
{
    _eye = glm::vec3( -1.0f, -1.0f, 1.0f );
    _focus = glm::vec3( 0.0f, 0.0f, 0.0f );
    _camUp = glm::vec3( 1.0f, 1.0f, 1.0f );
}

void Camera::_bottom()
{
    _eye = glm::vec3( 0.0f, 0.0f, -1.0f );
    _focus = glm::vec3( 0.0f, 0.0f, 0.0f );
    _camUp = glm::vec3( 0.0f, -1.0f, 0.0f );
}

void Camera::_rear()
{
    _eye = glm::vec3( 1.0f, 0.0f, 0.0f );
    _focus = glm::vec3( 0.0f, 0.0f, 0.0f );
    _camUp = glm::vec3( 0.0f, 0.0f, 1.0f );
}

void Camera::_right()
{
    _eye = glm::vec3( 0.0f, 1.0f, 0.0f );
    _focus = glm::vec3( 0.0f, 0.0f, 0.0f );
    _camUp = glm::vec3( 0.0f, 0.0f, 1.0f );
}

void Camera::_right_Iso()
{
    _eye = glm::vec3( -1.0f, 1.0f, 1.0f );
    _focus = glm::vec3( 0.0f, 0.0f, 0.0f );
    _camUp = glm::vec3( 1.0f, -1.0f, 1.0f );
}

void Camera::_calculateProjection()
{
    float range = 0;
    int w = _vWidth / 2;
    int h = _vHeight / 2;

    switch( _projectionType )
    {
    case Common::VSP_CAM_ORTHO:
        _leftB = -w * _oZoom;
        _rightB = w * _oZoom;
        _upB = -h * _oZoom;
        _downB = h * _oZoom;
        _projectionMatrix = glm::ortho( _leftB, _rightB, _upB, _downB, ORTHOGRAPHIC_Z_NEAR, ORTHOGRAPHIC_Z_FAR );
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
        _projectionMatrix = glm::ortho( _leftB, _rightB, _upB, _downB, ORTHOGRAPHIC_Z_NEAR, ORTHOGRAPHIC_Z_FAR );
        break;
    }
}
}