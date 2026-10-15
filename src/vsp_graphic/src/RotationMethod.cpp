#include "RotationMethod.h"
#include "Camera.h"

#include <algorithm>
#include <cmath>

namespace VSPGraphic
{
void RotationMethod::rebaseIfTurned( Camera * cam, int x, int y )
{
    if ( cam->rotationVersion() != _version )
    {
        begin( cam, x, y );
    }
}

ClassicBall::ClassicBall()
{
    _px = 0;
    _py = 0;
    _speed = 1.0;
}

void ClassicBall::setSpeed( double speed )
{
    _speed = speed;
}

void ClassicBall::begin( Camera * cam, int x, int y )
{
    _px = x;
    _py = y;
}

void ClassicBall::drag( Camera * cam, int x, int y )
{
    cam->rotate( _px, _py, x, y, ( float )_speed );

    _px = x;
    _py = y;
}

Trackball::Trackball()
{
    _speed = 1.0;
    _p0 = glm::dvec3( 0.0, 0.0, 1.0 );
    _rot0 = glm::dquat( 1.0, 0.0, 0.0, 0.0 );
}

void Trackball::setSpeed( double speed )
{
    _speed = speed;
}

glm::dvec3 Trackball::ballPoint( Camera * cam, int x, int y )
{
    double w = cam->viewportWidth();
    double h = cam->viewportHeight();
    double rb = 0.5 * std::min( w, h );
    if ( rb <= 0.0 )
    {
        return glm::dvec3( 0.0, 0.0, 1.0 );
    }

    double u = ( x - ( cam->viewportX() + 0.5 * w ) ) / rb;
    double v = ( y - ( cam->viewportY() + 0.5 * h ) ) / rb;
    double rsq = u * u + v * v;

    // Bell's: the unit sphere inside a radius of 1 / sqrt(2), the sheet z = 1 / ( 2 r )
    // outside, meeting it there with the same height and slope.
    double z = 0.0;
    if ( rsq < 0.5 )
    {
        z = std::sqrt( 1.0 - rsq );
    }
    else
    {
        z = 0.5 / std::sqrt( rsq );
    }
    return glm::normalize( glm::dvec3( u, v, z ) );
}

void Trackball::begin( Camera * cam, int x, int y )
{
    _p0 = ballPoint( cam, x, y );
    _rot0 = glm::dquat( cam->getViewRotation() );
    _version = cam->rotationVersion();
}

void Trackball::drag( Camera * cam, int x, int y )
{
    rebaseIfTurned( cam, x, y );

    glm::dvec3 p = ballPoint( cam, x, y );

    // Turn by the angle between the two points, times the speed, about their common
    // normal, in eye coordinates.  Every point lies in front of the sphere's center, so
    // no two are opposite and the axis is always defined.
    glm::dvec3 c = glm::cross( _p0, p );
    double s = glm::length( c );

    glm::dquat q( 1.0, 0.0, 0.0, 0.0 );
    if ( s > 0.0 )
    {
        double between = std::atan2( s, glm::dot( _p0, p ) );
        q = glm::angleAxis( _speed * between, c / s );
    }

    cam->setViewRotation( glm::quat( q * _rot0 ) );
    _version = cam->rotationVersion();
}

RoundedArcball::RoundedArcball()
{
    _size = 0.9;
    _border = 0.5;
    _angleScale = 2.0;
    _p0 = glm::dvec3( 0.0, 0.0, 1.0 );
    _rot0 = glm::dquat( 1.0, 0.0, 0.0, 0.0 );
}

void RoundedArcball::setShape( double size, double border )
{
    _size = std::max( 0.1, std::min( 1.5, size ) );
    _border = std::max( 0.0, std::min( 1.0, border ) );
}

void RoundedArcball::setAngleScale( double scale )
{
    _angleScale = std::max( 0.25, std::min( 4.0, scale ) );
}

glm::dvec3 RoundedArcball::ballPoint( Camera * cam, int x, int y )
{
    double w = cam->viewportWidth();
    double h = cam->viewportHeight();
    double cx = cam->viewportX() + 0.5 * w;
    double cy = cam->viewportY() + 0.5 * h;

    // The ball, fillet included, spans _size of the smaller viewport dimension.  The
    // sphere has unit radius and the fillet ends at radius ra.
    double rb = 0.5 * _size * std::min( w, h );
    if ( rb <= 0.0 )
    {
        return glm::dvec3( 0.0, 0.0, 1.0 );
    }

    double ra = 1.0 + _border;
    double a = _border * ( 1.0 + 0.5 * _border );
    double ri = 2.0 * ra / ( 1.0 + ra * ra );

    glm::dvec2 u( ( x - cx ) * ra / rb, ( y - cy ) * ra / rb );
    double r = glm::length( u );

    double z = 0.0;
    if ( r < ri )
    {
        z = std::sqrt( 1.0 - r * r );
    }
    else if ( r < ra )
    {
        z = a - std::sqrt( a * a - ( ra - r ) * ( ra - r ) );
    }

    glm::dvec3 p( u.x, u.y, z );
    double len = glm::length( p );
    if ( len <= 0.0 )
    {
        return glm::dvec3( 0.0, 0.0, 1.0 );
    }
    return p / len;
}

void RoundedArcball::outline( Camera * cam, float & cx, float & cy, std::vector < float > & radii )
{
    double w = cam->viewportWidth();
    double h = cam->viewportHeight();
    cx = cam->viewportX() + 0.5 * w;
    cy = cam->viewportY() + 0.5 * h;

    // As in ballPoint().
    double rb = 0.5 * _size * std::min( w, h );
    double ra = 1.0 + _border;
    double ri = 2.0 * ra / ( 1.0 + ra * ra );

    radii.clear();
    radii.push_back( ri * rb / ra );
    if ( _border > 0.0 )
    {
        radii.push_back( rb );
    }
}

void RoundedArcball::begin( Camera * cam, int x, int y )
{
    _p0 = ballPoint( cam, x, y );
    _rot0 = glm::dquat( cam->getViewRotation() );
    _version = cam->rotationVersion();
}

void RoundedArcball::drag( Camera * cam, int x, int y )
{
    rebaseIfTurned( cam, x, y );

    glm::dvec3 p = ballPoint( cam, x, y );

    // Turn about the common normal of the two points, in eye coordinates.
    glm::dvec3 c = glm::cross( _p0, p );
    double s = glm::length( c );

    glm::dquat q( 1.0, 0.0, 0.0, 0.0 );
    if ( s > 0.0 )
    {
        double between = std::atan2( s, glm::dot( _p0, p ) );
        q = glm::angleAxis( _angleScale * between, c / s );

        // At any scale but 2 the rotation does not return to the identity as the points
        // become opposite, so restart from here before the axis becomes undefined.
        if ( _angleScale != 2.0 && between > 150.0 * M_PI / 180.0 )
        {
            _rot0 = q * _rot0;
            _p0 = p;
            q = glm::dquat( 1.0, 0.0, 0.0, 0.0 );
        }
    }

    cam->setViewRotation( glm::quat( q * _rot0 ) );
    _version = cam->rotationVersion();
}

Tumbler::Tumbler()
{
    _gain = 0.4 * M_PI / 180.0;
    _px = 0;
    _py = 0;
}

void Tumbler::setGain( double gain )
{
    _gain = gain;
}

void Tumbler::begin( Camera * cam, int x, int y )
{
    _px = x;
    _py = y;
}

void Tumbler::drag( Camera * cam, int x, int y )
{
    double dx = x - _px;
    double dy = y - _py;
    double len = std::sqrt( dx * dx + dy * dy );

    _px = x;
    _py = y;

    if ( len <= 0.0 )
    {
        return;
    }

    // Turn about the screen axis perpendicular to the move, in eye coordinates, so the near
    // side follows the mouse.
    glm::dvec3 axis( -dy / len, dx / len, 0.0 );
    glm::dquat q = glm::angleAxis( _gain * len, axis );

    glm::dquat rot = q * glm::dquat( cam->getViewRotation() );
    cam->setViewRotation( glm::quat( glm::normalize( rot ) ) );
}

Turntable::Turntable()
{
    _gain = 0.4 * M_PI / 180.0;
    _up = glm::dvec3( 0.0, 0.0, 1.0 );
    _x0 = 0;
    _y0 = 0;
    _reverse = 1.0;
    _tipAxis = glm::dvec3( 1.0, 0.0, 0.0 );
    _rot0 = glm::dquat( 1.0, 0.0, 0.0, 0.0 );
}

void Turntable::setParams( double gain, const glm::dvec3 & up )
{
    _gain = gain;
    _up = glm::normalize( up );
}

void Turntable::begin( Camera * cam, int x, int y )
{
    _x0 = x;
    _y0 = y;
    _rot0 = glm::dquat( cam->getViewRotation() );

    glm::dquat inv = glm::conjugate( _rot0 );

    // Reverse horizontal motion for this drag when the up axis points down the screen.
    glm::dvec3 upEye = _rot0 * _up;
    _reverse = 1.0;
    if ( upEye.y < 0.0 )
    {
        _reverse = -1.0;
    }

    // Tip about the screen's horizontal axis, blended toward the world's horizon when the
    // view is rolled (Blender's turntable).  Looking along the up axis the screen axis is
    // kept; looking level the horizon is used.
    glm::dvec3 viewX = inv * glm::dvec3( 1.0, 0.0, 0.0 );
    glm::dvec3 viewZ = inv * glm::dvec3( 0.0, 0.0, 1.0 );
    glm::dvec3 horizon = glm::cross( _up, viewZ );

    glm::dvec3 tip = viewX;
    double hlen = glm::length( horizon );
    if ( hlen > 1.0e-9 )
    {
        horizon = horizon / hlen;

        // Point the horizon along the screen's x.  When the view is rolled 90 degrees they
        // are perpendicular, and the screen's y decides instead of rounding noise.
        glm::dvec3 viewY = inv * glm::dvec3( 0.0, 1.0, 0.0 );
        double side = glm::dot( horizon, viewX );
        if ( std::fabs( side ) < 1.0e-6 )
        {
            side = glm::dot( horizon, viewY );
        }
        if ( side < 0.0 )
        {
            horizon = -horizon;
        }

        double cosang = std::max( -1.0, std::min( 1.0, glm::dot( _up, viewZ ) ) );
        double fac = std::fabs( std::acos( cosang ) / M_PI - 0.5 ) * 2.0;
        fac = fac * fac;

        tip = glm::normalize( horizon * ( 1.0 - fac ) + viewX * fac );
    }

    _tipAxis = _rot0 * tip;
    _version = cam->rotationVersion();
}

void Turntable::drag( Camera * cam, int x, int y )
{
    rebaseIfTurned( cam, x, y );

    double turn = _reverse * _gain * ( x - _x0 );
    double tip = -_gain * ( y - _y0 );

    glm::dquat rot = glm::angleAxis( tip, _tipAxis ) * _rot0 * glm::angleAxis( turn, _up );

    cam->setViewRotation( glm::quat( rot ) );
    _version = cam->rotationVersion();
}
}
