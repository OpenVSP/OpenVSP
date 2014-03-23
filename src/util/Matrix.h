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

