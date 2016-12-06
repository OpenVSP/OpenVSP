//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

#include <math.h>

#include "Cluster.h"

double Cluster( const double &t, const double &a, const double &b )
{
    double mt = 1.0 - t;
    return mt * mt * t * a + mt * t * t * ( 3.0 - b ) + t * t * t;
}
