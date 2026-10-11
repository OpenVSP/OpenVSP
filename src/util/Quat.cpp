#include <cmath>
#include <type_traits>

#include "Quat.h"

static_assert( std::is_trivially_copyable< quat >::value, "quat must remain trivially copyable" );

// New quat with out any initial values //
quat::quat()
{
    q[0] = q[1] = q[2] = q[3] = 0.0;
}

// New quat with initial values //

quat::quat( const double &w, const double &x, const double &y, const double &z )
{
    q[0] = w;
    q[1] = x;
    q[2] = y;
    q[3] = z;
}
// Construct quat from axis angle //
quat::quat( const vec3d &axis, const double &angle )
{
    q[0] = cos( angle / 2 );
    q[1] = axis.x() * sin( angle / 2 );
    q[2] = axis.y() * sin( angle / 2 );
    q[3] = axis.z() * sin( angle / 2 );
}

// Quaternion Hamilton Product //
quat hamilton( const quat& a, const quat& b )
{
    return quat( ( a.q[0] * b.q[0] ) - ( a.q[1] * b.q[1] ) - ( a.q[2] * b.q[2] ) - ( a.q[3] * b.q[3] ),
                 ( a.q[0] * b.q[1] ) + ( a.q[1] * b.q[0] ) + ( a.q[2] * b.q[3] ) - ( a.q[3] * b.q[2] ),
                 ( a.q[0] * b.q[2] ) - ( a.q[1] * b.q[3] ) + ( a.q[2] * b.q[0] ) + ( a.q[3] * b.q[1] ),
                 ( a.q[0] * b.q[3] ) + ( a.q[1] * b.q[2] ) - ( a.q[2] * b.q[1] ) + ( a.q[3] * b.q[0] ) );
}

// Quaternion to Axis Angle //
// Axis is where the rotation axis will be stored, and angle is where the angle will be stored //
void quat::quat2axisangle( vec3d& axis, double& angle ) const
{
    // The vector part is sin( angle / 2 ) axis and the scalar part cos( angle / 2 ); atan2 of the two
    // keeps its digits near 0 and 2 pi, where acos and sqrt( 1 - w^2 ) lose them.
    double s = sqrt( q[1] * q[1] + q[2] * q[2] + q[3] * q[3] );
    angle = 2 * atan2( s, q[0] );

    if ( s > 0.0 )
    {
        axis = vec3d( q[1] / s, q[2] / s, q[3] / s );
    }
    else
    {
        // No rotation
        axis = vec3d( 0, 0, 1 );
    }
}
