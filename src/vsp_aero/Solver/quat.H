//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//////////////////////////////////////////////////////////////////////

#ifndef QUAT_H
#define QUAT_H

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "utils.H"

#include "START_NAME_SPACE.H"

class QUAT {

private:

    double Vec_[4]; // Vec_[0 - 2] are the vector components, Vec_[3] is the scalar

public:

    QUAT(void);
   ~QUAT(void);
    QUAT(const QUAT &quat);

    QUAT& operator=(const QUAT &quat);
    
    /** Access to the quaternion vector... 0,1,2 are the vector components .. 3 is the scalar **/

    double& operator()(int i);

    /** Access to the quaternion vector... 0,1,2 are the vector components .. 3 is the scalar **/
    
    const double& operator()(int i) const;

    friend QUAT operator+(const QUAT &quat1, const QUAT &quat2);
    friend QUAT operator-(const QUAT &quat1, const QUAT &quat2);
    friend QUAT operator*(const QUAT &quat1, const QUAT &quat2);
    friend QUAT operator/(const QUAT &quat1, const QUAT &quat2);

    /** Form inverse of quaternion vector **/
    
    void FormInverse(void);

    /** Form rotation quaternion given rotation vector and rotation angle (in radians) **/
    
    void FormRotationQuat(double *Vec, double Theta);

    /** Init quat vector to 0, and scalar to A **/
    
    void Init(double A) { Vec_[0] = 0. ; Vec_[1] = 0. ; Vec_[2] = 0. ; Vec_[3] = A; };
    
    /** Init quat vector to (A,B,C) and scalar to D **/
    
    void Init(double A, double B, double C, double D) { Vec_[0] = A ; Vec_[1] = B ; Vec_[2] = C ; Vec_[3] = D; };
    
    /** Init quat vector using input vector **/
    
    void Init(double Vec[4]) { Vec_[0] = Vec[0] ; Vec_[1] = Vec[1] ; Vec_[2] = Vec[2] ; Vec_[3] = Vec[3]; };
    
    /** Return magnitude of quaternion vector **/
    
    double Magnitude(void);
    
    /** Print out quaternion vector **/

    void print(char *name);
    
};

#include "END_NAME_SPACE.H"

#endif
