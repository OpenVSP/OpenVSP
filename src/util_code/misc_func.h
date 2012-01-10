//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

#ifndef MISCFUNC_H
#define MISCFUNC_H

#include <math.h>

float super_ellipse(float x_over_A, float n)
{
	if ( n <= 0 ) n = 0.000001f;

	float temp = 1.0f - powf(x_over_A, n);
	return( powf( temp, 1.0f/n ) );
}

#endif
