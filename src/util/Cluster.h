//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

#ifndef __CLUSTER_H_
#define __CLUSTER_H_

double Cluster( const double &t, const double &a, const double &b );

double Cubic_Stretch( const double &t, const double &ds0, const double &ds1 );
double HypTan_Stretch( const double &t, const double &ds0, const double &ds1 );

double asinhc_approx( const double &y );
double asinhc( const double &y );

double asinc_approx( const double &y );
double asinc( const double &y );

#endif
