//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//


#ifndef __MATRIX_H_
#define __MATRIX_H_

#include <math.h>
#include <string.h>
#include "Defines.h"
#include "Vec3d.h"


class Matrix4d
{
public:

    Matrix4d();

    void loadIdentity();
    void setIdentity( double* m ) const;
    void translatef( const double &x, const double &y, const double & );
    void rotateX( const double &ang );
    void rotateY( const double &ang );
    void rotateZ( const double &ang );
    void rotate( const double angle, const vec3d & axis );
    void affineInverse();
    void scale( const double &scale );


    void getMat( double* m );
    void matMult( double* m );
    void postMult( double* m );


    void initMat( double* m );
    void mult( const double in[4], double out[4] ) const;
    double * data()
    {
        return mat;
    }

    void loadXZRef();
    void loadXYRef();
    void loadYZRef();

    vec3d xform( const vec3d & in ) const;
    vec3d getAngles() const;

private:

    double mat[16];
};

#endif

