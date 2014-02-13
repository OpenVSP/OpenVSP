// Quaternion class for X3D rotations //

#ifndef QUAT_H
#define QUAT_H

#include "Defines.h"

#include "Vec3d.h"

#include <iostream>
using namespace std;

class quat;


class VSPDLL quat
{
private:

public:
    double q[4];

    quat();  //new quaternion
    ~quat() {} // delete quaternion
    quat( double w, double x, double y, double z );
    quat( const quat& a );  // quat x = y
    quat& operator=( const quat& a ); // x = y

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
    void quat2axisangle( vec3d& axis, double& angle );
    quat( vec3d axis, double angle );


};

#endif
