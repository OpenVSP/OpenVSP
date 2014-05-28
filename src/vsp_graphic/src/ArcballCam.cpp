#include "ArcballCam.h"
#include "glm/ext.hpp"

#include <iostream>

#define TOLERANCE 0.00001
#define R_SENSITIVITY 1.0f  // Rotation Sensitivity
#define P_SENSITIVITY 15.0f // Panning Sensitivity

#define ALLOWABLE_SAVES 4

namespace VSPGraphic
{
ArcballCam::ArcballCam() : Camera()
{
    _globalQuat = glm::vec4( 0.0, 0.0, 0.0, 1.0 );
    _rMat = _tMat = _cMat = glm::mat4( 1.0 );
    _pan = glm::vec2( 0.0f );
    _center = glm::vec2( _vx + _vWidth / 2, _vy + _vHeight / 2 );
    _radius = _vHeight < _vWidth ? _vHeight / 2.0f : _vWidth / 2.0f;

    _prevLB = _prevMB = _prevRB = _prevAltLB = glm::vec2( 0xffffffff );

    camInfo initSave;
    initSave.empty = true;
    initSave.globalQuat = _globalQuat;
    initSave.pan = _pan;
    initSave.cor = _cMat;
    initSave.orthoZoomValue = _oZoom;
    initSave.perspZoomValue = _pZoom;
    initSave.projectionType = _projectionType;

    for( int i = 0; i < ALLOWABLE_SAVES; i++ )
    {
        _saves.push_back( initSave );
    }
}
ArcballCam::~ArcballCam()
{
}

void ArcballCam::save( int index )
{
    assert( index < ( int )_saves.size() );
    if( index >= ( int )_saves.size() )
    {
        return;
    }

    _saves[index].empty = false;
    _saves[index].globalQuat = _globalQuat;
    _saves[index].pan = _pan;
    _saves[index].cor = _cMat;
    _saves[index].projectionType = _projectionType;
    _saves[index].orthoZoomValue = _oZoom;
    _saves[index].perspZoomValue = _pZoom;
}

void ArcballCam::load( int index )
{
    assert( index < ( int )_saves.size() );
    if( index >= ( int )_saves.size() )
    {
        return;
    }

    // If slot is empty, do nothing.
    if( _saves[index].empty )
    {
        return;
    }

    // Load Saved values.
    _globalQuat = _saves[index].globalQuat;
    _pan = _saves[index].pan;
    _projectionType = _saves[index].projectionType;
    _oZoom = _saves[index].orthoZoomValue;
    _pZoom = _saves[index].perspZoomValue;

    // Apply saved values.
    _calculateProjection();
    _rMat = _toMatrix( _globalQuat );
    _tMat = glm::translate( _pan.x, _pan.y, 0.0f );
    _cMat = _saves[index].cor;
    _modelviewMatrix = _tMat * _viewMatrix * _rMat * _cMat;
}

void ArcballCam::resetView()
{
    // Reset global quaternion and transformation matrices.
    _globalQuat = glm::vec4( 0.0, 0.0, 0.0, 1.0 );
    _rMat = _tMat = glm::mat4( 1.0f );

    // Reset panning.
    _pan = glm::vec2( 0.0f, 0.0f );

    // Reset center of rotation.
    _cMat = glm::mat4( 1.0f );

    // Reset modelview matrix.
    Camera::resetView();
}

void ArcballCam::center()
{
    // Reset Panning.
    _pan = glm::vec2( 0.0f, 0.0f );
    _tMat = glm::mat4( 1.0f );

    // Keep View and Rotation.
    _modelviewMatrix = _tMat * _viewMatrix * _rMat * _cMat;
}

void ArcballCam::resize( int x, int y, int width, int height )
{
    Camera::resize( x, y, width, height );

    // Adjust center and radius.
    _center = glm::vec2( _vx + _vWidth / 2, _vy + _vHeight / 2 );
    _radius = _vHeight < _vWidth ? _vHeight / 2.0f : _vWidth / 2.0f;
}

void ArcballCam::rotate( int px, int py, int cx, int cy )
{
    if( px == cx && py == cy )
    {
        return;
    }

    glm::vec3 pvec = _toSphere3D( px, py );
    glm::vec3 cvec = _toSphere3D( cx, cy );

    // If pvec or cvec is on the edge of the sphere, Re-map sphere.
    // This occur when mouse is dragged outside of the sphere.
    if( pvec.z == 0.f || cvec.z == 0.f )
    {
        _globalQuat = _multiplyQuats( _mapOutsideSphereToQuat( px, py, cx, cy ), _globalQuat );
    }
    else
    {
        // Calculate Rotation Axis and Angle.
        glm::vec3 rotAxis = - glm::vec3( glm::inverse( _modelviewMatrix ) * glm::vec4( glm::cross( pvec, cvec ), 0.0 ) );

        float dot = glm::dot( pvec, cvec );
        float angle = R_SENSITIVITY * glm::acos( dot > 1.0f ? 1.0f : dot < -1.0f ? -1.0f : dot );

        // Quaternion
        _globalQuat = _multiplyQuats( _toQuat( rotAxis, angle ), _globalQuat );
    }

    _rMat = _toMatrix( _globalQuat );
    _modelviewMatrix = _tMat * _viewMatrix * _rMat * _cMat;
}

void ArcballCam::pan( int px, int py, int cx, int cy )
{
    if( px == cx && py == cy )
    {
        return;
    }

    _pan.x += P_SENSITIVITY * ( float )( cx - px ) / _vWidth;
    _pan.y += P_SENSITIVITY * ( float )( cy - py ) / _vHeight;

    _tMat = glm::translate( _pan.x, _pan.y, 0.0f );

    _modelviewMatrix = _tMat * _viewMatrix * _rMat * _cMat;
}

void ArcballCam::pan( float x, float y )
{
    _pan.x += x;
    _pan.y += y;

    _tMat = glm::translate( _pan.x, _pan.y, 0.0f );

    _modelviewMatrix = _tMat * _viewMatrix * _rMat * _cMat;
}

void ArcballCam::zoom( int px, int py, int cx, int cy )
{
    if( px == cx && py == cy )
    {
        return;
    }

    // Perform zoom on Y-axis mouse movement.  Zoom in when
    // mouse moves down, zoom out when mouse moves up.

    if( cy - py > 0 )
    {
        _pZoom += 0.5f;
        _oZoom += 0.001f;
    }
    else if( cy - py < 0 )
    {
        _pZoom -= 0.5f;
        _oZoom -= 0.001f;
    }

    if( _oZoom < 0.001f )
    {
        _oZoom = 0.001f;
    }

    if( _pZoom < 10.f )
    {
        _pZoom = 10.f;
    }

    if( _pZoom > 360.f )
    {
        _pZoom = 360.f;
    }

    _calculateProjection();
}

void ArcballCam::zoom( float zoomvalue )
{
    _oZoom += zoomvalue;
    _pZoom += zoomvalue;

    if( _oZoom < 0.001f )
    {
        _oZoom = 0.001f;
    }

    if( _pZoom < 10.f )
    {
        _pZoom = 10.f;
    }

    if( _pZoom > 360.f )
    {
        _pZoom = 360.f;
    }

    _calculateProjection();
}

void ArcballCam::setCOR( float x, float y, float z )
{
    _cMat = glm::translate( glm::vec3( x, y, z ) );
    _modelviewMatrix = _tMat * _viewMatrix * _rMat * _cMat;
}

glm::vec3 ArcballCam::_toSphere3D( int x, int y )
{
    glm::vec3 result = glm::vec3( _toNDC2f( ( float )x, ( float )y ), 0.f );

    float xySq = result.x * result.x + result.y * result.y;

    if( xySq < 1.0 )
    {
        /*
        * z equals to sqrt(r^2 - sqrt(xySq)^2)
        */
        result.z = ( float )glm::sqrt( 1.0 - xySq );
    }
    else
    {
        result = glm::normalize( result );
    }
    return result;
}

glm::vec4 ArcballCam::_toQuat( glm::vec3 rotAxis, float angleRad )
{
    glm::vec3 normalizedAxis = glm::normalize( rotAxis );

    float half_angle = angleRad * 0.5f;

    /*
    * result = (v * sin(angle / 2), cos(angle / 2))
    */
    glm::vec4 result = glm::vec4( normalizedAxis * glm::sin( half_angle ), glm::cos( half_angle ) );

    return result;
}

glm::mat4 ArcballCam::_toMatrix( glm::vec4 quat )
{
    glm::mat4 result = glm::mat4( 1.0 );

    float xx = quat.x * quat.x;
    float xy = quat.x * quat.y;
    float xz = quat.x * quat.z;
    float yy = quat.y * quat.y;
    float yz = quat.y * quat.z;
    float zz = quat.z * quat.z;
    float wx = quat.w * quat.x;
    float wy = quat.w * quat.y;
    float wz = quat.w * quat.z;

    result[0][0] = 1.0f - 2.0f * ( yy + zz );
    result[0][1] = 2.0f * ( xy - wz );
    result[0][2] = 2.0f * ( xz + wy );

    result[1][0] = 2.0f * ( xy + wz );
    result[1][1] = 1.0f - 2.0f * ( xx + zz );
    result[1][2] = 2.0f * ( yz - wx );

    result[2][0] = 2.0f * ( xz - wy );
    result[2][1] = 2.0f * ( yz + wx );
    result[2][2] = 1.0f - 2.0f * ( yy + xx );

    return result;
}

glm::vec4 ArcballCam::_normalizeQuat( glm::vec4 quat )
{
    glm::vec4 result = quat;

    // Normalize Quaternion if needed.
    float sq = result.x * result.x + result.y * result.y + result.z * result.z + result.w * result.w;

    if( glm::abs( sq - 1.0 ) > TOLERANCE && glm::abs( sq ) > TOLERANCE )
    {
        float magnitude = glm::sqrt( sq );
        result /= magnitude;
    }
    return result;
}

glm::vec4 ArcballCam::_multiplyQuats( glm::vec4 quat1, glm::vec4 quat2 )
{
    glm::vec3 v1 = glm::vec3( quat1 );
    glm::vec3 v2 = glm::vec3( quat2 );

    float w1 = quat1.w;
    float w2 = quat2.w;

    glm::vec4 result;
    /*
    * result = (v, w);
    * v = w1 * v2 + w2 * v1 + v1 cross v2
    * w = w1 * w2 - v1 dot v2;
    */
    result = glm::vec4( w1 * v2 + w2 * v1 + glm::cross( v1, v2 ), w1 * w2 - glm::dot( v1, v2 ) );

    // Normalize if needed.
    result = _normalizeQuat( result );

    return result;
}

glm::vec2 ArcballCam::_toNDC2f( float x, float y )
{
    glm::vec2 result;

    result.x = 2.f * ( ( float )( x - _vx ) / _vWidth ) - 1.f;
    result.y = 2.f * ( ( float )( y - _vy ) / _vHeight ) - 1.f;

    return result;
}

glm::vec4 ArcballCam::_mapOutsideSphereToQuat( int px, int py, int cx, int cy )
{
    glm::vec4 result = glm::vec4( 0.0f, 0.0f, 0.0f, 1.0f );

    float pRadius = glm::sqrt( ( px - _center.x ) * ( px - _center.x ) + ( py - _center.y ) * ( py - _center.y ) );
    float cRadius = glm::sqrt( ( cx - _center.x ) * ( cx - _center.x ) + ( cy - _center.y ) * ( cy - _center.y ) );

    if( pRadius == cRadius )
    {
        result = glm::vec4( 0.0f, 0.0f, 0.0f, 1.0f );
    }

    // Mouse is Moving Inward.
    if( pRadius > cRadius )
    {
        float pAngle = glm::atan( ( float )( py - _center.y ), ( float )( px - _center.x ) );
        float cAngle = glm::atan( ( float )( cy - _center.y ), ( float )( cx - _center.x ) );

        float difference = pRadius - cRadius;
        float releative_radius = _radius - difference;

        cx = ( int )( _center.x + releative_radius * glm::cos( cAngle ) );
        cy = ( int )( _center.y + releative_radius * glm::sin( cAngle ) );

        px = ( int )( _center.x + _radius * glm::cos( pAngle ) );
        py = ( int )( _center.y + _radius * glm::sin( pAngle ) );
    }

    // Mouse is Moving Outward.
    if( pRadius < cRadius )
    {
        float pAngle = glm::atan( ( float )( py - _center.y ), ( float )( px - _center.x ) );
        float cAngle = glm::atan( ( float )( cy - _center.y ), ( float )( cx - _center.x ) );

        float difference = cRadius - pRadius;
        float releative_radius = _radius - difference;

        cx = ( int )( _center.x + _radius * glm::cos( cAngle ) );
        cy = ( int )( _center.y + _radius * glm::sin( cAngle ) );

        px = ( int )( _center.x + releative_radius * glm::cos( pAngle ) );
        py = ( int )( _center.y + releative_radius * glm::sin( pAngle ) );
    }

    if( px != cx || py != cy )
    {
        glm::vec3 pvec = _toSphere3D( px, py );
        glm::vec3 cvec = _toSphere3D( cx, cy );

        glm::vec3 rotAxis = - glm::vec3( glm::inverse( _modelviewMatrix ) * glm::vec4( glm::cross( pvec, cvec ), 0.0f ) );

        float dot = glm::dot( pvec, cvec );
        float angle = R_SENSITIVITY * glm::acos( dot > 1.0f ? 1.0f : dot < -1.0f ? -1.0f : dot );

        result = _toQuat( rotAxis, angle );
    }
    else
    {
        result = glm::vec4( 0.0f, 0.0f, 0.0f, 1.0f );
    }
    return result;
}
}