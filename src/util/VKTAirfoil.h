//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// VKTAirfoil.h: Karman-Trefftz airfoil routines
// Rob McDonald
//
//////////////////////////////////////////////////////////////////////

#if !defined(VKTAIRFOIL__INCLUDED_)
#define VKTAIRFOIL__INCLUDED_

#include "Vec3d.h"

typedef std::complex< double > doublec;

vec3d vkt_airfoil_point( double theta, double epsilon, double kappa, double tau );

doublec cmplx_potential( doublec zeta, double alpha, double beta, double a, doublec mu );
doublec cmplx_velocity( doublec zeta, double alpha, double beta, double a, doublec mu );
doublec derivative( doublec zeta, double ell, double n );

#endif
