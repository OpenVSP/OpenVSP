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

class QUAT {

private:

    double Vec_[4];

public:

    QUAT(void);
   ~QUAT(void);
    QUAT(const QUAT &quat);

    QUAT& operator=(const QUAT &quat);

    double& operator()(int i);

    const double& operator()(int i) const;

    friend QUAT operator+(const QUAT &quat1, const QUAT &quat2);
    friend QUAT operator-(const QUAT &quat1, const QUAT &quat2);
    friend QUAT operator*(const QUAT &quat1, const QUAT &quat2);

    void FormInverse(void);

    void FormRotationQuat(double *Vec, double Theta);
    void FormRotationQuat(float *Vec, float Theta);

};

#endif
