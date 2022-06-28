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

    VSPAERO_DOUBLE Vec_[4]; // Vec_[0 - 2] are the vector components, Vec_[3] is the scalar

public:

    QUAT(void);
   ~QUAT(void);
    QUAT(const QUAT &quat);

    QUAT& operator=(const QUAT &quat);

    VSPAERO_DOUBLE& operator()(int i);

    const VSPAERO_DOUBLE& operator()(int i) const;

    friend QUAT operator+(const QUAT &quat1, const QUAT &quat2);
    friend QUAT operator-(const QUAT &quat1, const QUAT &quat2);
    friend QUAT operator*(const QUAT &quat1, const QUAT &quat2);

    void FormInverse(void);

    void FormRotationQuat(VSPAERO_DOUBLE *Vec, VSPAERO_DOUBLE Theta);
    
    void Init(VSPAERO_DOUBLE A) { Vec_[0] = 0. ; Vec_[1] = 0. ; Vec_[2] = 0. ; Vec_[3] = A; };
    void Init(VSPAERO_DOUBLE A, VSPAERO_DOUBLE B, VSPAERO_DOUBLE C, VSPAERO_DOUBLE D) { Vec_[0] = A ; Vec_[1] = B ; Vec_[2] = C ; Vec_[3] = D; };
    void Init(VSPAERO_DOUBLE Vec[4]) { Vec_[0] = Vec[0] ; Vec_[1] = Vec[1] ; Vec_[2] = Vec[2] ; Vec_[3] = Vec[3]; };

    void print(char *name);
    
};

#include "END_NAME_SPACE.H"

#endif
