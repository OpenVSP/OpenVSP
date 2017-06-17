//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//////////////////////////////////////////////////////////////////////

#include "quat.H"

/*##############################################################################
#                                                                              #
#                               QUAT Constructor                               #
#                                                                              #
##############################################################################*/

QUAT::QUAT(void)
{

    // Just zero the quat

    Vec_[0] = Vec_[1] = Vec_[2] = Vec_[3] = 0.;

}

/*##############################################################################
#                                                                              #
#                               QUAT Destructor                                #
#                                                                              #
##############################################################################*/

QUAT::~QUAT(void)
{

    // Just zero the matrix

    Vec_[0] = Vec_[1] = Vec_[2] = Vec_[3] = 0.;

}

/*##############################################################################
#                                                                              #
#                               QUAT Copy                                      #
#                                                                              #
##############################################################################*/

QUAT::QUAT(const QUAT &quat)
{

    int i;

    // Copy contents of quat

    for ( i = 0 ; i <= 3 ; i++ ) {

       (*this)(i) = quat(i);

    }

}

/*##############################################################################
#                                                                              #
#                               QUAT Operator =                                #
#                                                                              #
##############################################################################*/

QUAT &QUAT::operator=(const QUAT &quat)
{

    int i;

    // Copy contents of quat

    for ( i = 0 ; i <= 3 ; i++ ) {

       (*this)(i) = quat(i);

    }

    return *this;

}

/*##############################################################################
#                                                                              #
#                               QUAT Operator ()                               #
#                                                                              #
##############################################################################*/

double &QUAT::operator()(int i)
{

    return (Vec_[i]);

}

/*##############################################################################
#                                                                              #
#                           QUAT Operator () const                             #
#                                                                              #
##############################################################################*/

const double& QUAT::operator()(int i) const
{

    return (Vec_[i]);

}

/*##############################################################################
#                                                                              #
#                            QUAT Operator +                                   #
#                                                                              #
##############################################################################*/

QUAT operator+(const QUAT &quat1, const QUAT &quat2)
{

    int i;
    QUAT Quat;

    // Add two quats

    for ( i = 0 ; i <= 3 ; i++ ) {

       Quat(i) = quat1(i) + quat2(i);

    }

    return Quat;

}

/*##############################################################################
#                                                                              #
#                            QUAT Operator -                                   #
#                                                                              #
##############################################################################*/

QUAT operator-(const QUAT &quat1, const QUAT &quat2)
{

    int i;
    QUAT Quat;

    // Subtract two quats

    for ( i = 0 ; i <= 3 ; i++ ) {

       Quat(i) = quat1(i) - quat2(i);

    }

    return Quat;

}

/*##############################################################################
#                                                                              #
#                               QUAT Operator *                                #
#                                                                              #
##############################################################################*/

QUAT operator*(const QUAT &quat1, const QUAT &quat2)
{

    int i;
    double Vec1[3], Vec2[3], Vec3[3], a1, a2, dot;
    QUAT Quat;

    for ( i = 0 ; i <= 2 ; i++ ) {

      Vec1[i] = quat1(i);
      Vec2[i] = quat2(i);

    }

    a1 = quat1(3);

    a2 = quat2(3);

    dot = vector_dot(Vec1,Vec2);

    vector_cross(Vec1,Vec2,Vec3);

    for ( i = 0 ; i <= 2 ; i++ ) {

       Quat(i) = a1 * quat2(i) + a2 * quat1(i) + Vec3[i];

    }

    Quat(3) = a1 * a2 - dot;

    return Quat;

}

/*##############################################################################
#                                                                              #
#                              QUAT FormInverse                                #
#                                                                              #
##############################################################################*/

void QUAT::FormInverse(void)
{

    int i;

    // Invert quaternion

    for ( i = 0 ; i <= 2 ; i++ ) {

       (*this)(i) *= -1;

    }

}

/*##############################################################################
#                                                                              #
#                         QUAT FormRotationQuat                                #
#                                                                              #
##############################################################################*/

void QUAT::FormRotationQuat(double *Vec, double Theta)
{

    int i;
    double Dot, SinHalfTheta;
    QUAT Quat;

    // Create the pure part along the normalized vector

    Dot = sqrt(vector_dot(Vec,Vec));

    SinHalfTheta = sin(0.5*Theta);

    for ( i = 0 ; i <= 2 ; i++ ) {

       (*this)(i) = Vec[i] * SinHalfTheta / Dot;

    }

    (*this)(3) = cos(0.5*Theta);

}


/*##############################################################################
#                                                                              #
#                         QUAT FormRotationQuat                                #
#                                                                              #
##############################################################################*/

void QUAT::FormRotationQuat(float *Vec, float Theta)
{

    int i;
    float Dot, SinHalfTheta;
    QUAT Quat;

    // Create the pure part along the normalized vector

    Dot = sqrt(vector_dot(Vec,Vec));

    SinHalfTheta = sin(0.5*Theta);

    for ( i = 0 ; i <= 2 ; i++ ) {

       (*this)(i) = Vec[i] * SinHalfTheta / Dot;

    }

    (*this)(3) = cos(0.5*Theta);

}
