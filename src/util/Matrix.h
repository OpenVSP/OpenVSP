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


class Matrix
{
public:

    Matrix();

    void loadIdentity();
    void setIdentity( float* m );
    void translatef( float x, float y, float z );
    void rotateX( float ang );
    void rotateY( float ang );
    void rotateZ( float ang );

    void getMat( float* m );
    void matMult( float* m );
    void postMult( float* m );

    void initMat( float* m );
    void mult( float in[4], float out[4] );
    float * data()
    {
        return mat;
    }

private:

    float mat[16];


};

class Matrix4d
{
public:

    Matrix4d();

    void loadIdentity();
    void setIdentity( double* m );
    void translatef( double x, double y, double z );
    void rotateX( double ang );
    void rotateY( double ang );
    void rotateZ( double ang );
    void rotate( double angle, vec3d & axis );
    void affineInverse();
    void scale( double scale );


    void getMat( double* m );
    void matMult( double* m );
    void postMult( double* m );


    void initMat( double* m );
    void mult( double in[4], double out[4] );
    double * data()
    {
        return mat;
    }

    void loadXZRef();
    void loadXYRef();
    void loadYZRef();

    vec3d xform( vec3d & in );
    vec3d getAngles();

private:

    double mat[16];


};

#endif

