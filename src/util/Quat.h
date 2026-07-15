// Quaternion class for X3D rotations //

#ifndef QUAT_H
#define QUAT_H



#include "Vec3d.h"

class quat;


class quat
{
private:

public:
    double q[4];

    quat();  //new quaternion
    // The copy constructor, copy assignment operator, and destructor are intentionally left implicit.
    // This keeps quat trivially copyable.
    quat( const double &w, const double &x, const double &y, const double &z );

    friend quat hamilton( const quat& a, const quat& b ); // c = hamilton(a,b)

    // get point values //
    double w() const
    {
        return( q[0] );
    }
    double x() const
    {
        return( q[1] );
    }
    double y() const
    {
        return( q[2] );
    }
    double z() const
    {
        return( q[3] );
    }

    // convert to and from axis angle //
    void quat2axisangle( vec3d& axis, double& angle ) const;
    quat( const vec3d &axis, const double &angle );


};

#endif
