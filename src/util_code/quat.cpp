#include <math.h>
#include <stdio.h>
#include "quat.h"

// new quat with out any initial values //
quat::quat()
{
	q[0] = q[1] = q[2] = q[3] = 0.0;
}

// new quat with initial values //

quat::quat(double w, double x, double y, double z)
{
	q[0] = w;
	q[1] = x;
	q[2] = y;
	q[3] = z;
}

quat::quat(const quat& a)		
{
	q[0] = a.q[0];
	q[1] = a.q[1];
	q[2] = a.q[2];
	q[3] = a.q[3];
}

quat& quat::operator=(const quat& a)  
{
	if (this == &a)
	   return *this;

	q[0] = a.q[0];
	q[1] = a.q[1];
	q[2] = a.q[2];
	q[3] = a.q[3];
	return *this;
}

// Quaternion Hamilton Product //
quat hamilton(const quat& a, const quat& b)
{
	return quat((a.q[0]*b.q[0])-(a.q[1]*b.q[1])-(a.q[2]*b.q[2])-(a.q[3]*b.q[3]),
		(a.q[0]*b.q[1])+(a.q[1]*b.q[0])+(a.q[2]*b.q[3])-(a.q[3]*b.q[2]),
		(a.q[0]*b.q[2])-(a.q[1]*b.q[3])+(a.q[2]*b.q[0])+(a.q[3]*b.q[1]),
		(a.q[0]*b.q[3])+(a.q[1]*b.q[2])-(a.q[2]*b.q[1])+(a.q[3]*b.q[0]));
}

// Quaternion to Axis Angle //
// Axis is where the rotation axis will be stored, and angle is where the angle will be stored //
void quat::quat2axisangle(vec3d& axis, double& angle)
{
	double denom = sqrt(1-q[0]*q[0]);
	angle = 2*acos(q[0]);
	axis = vec3d(q[1]/denom, q[2]/denom, q[3]/denom);
}