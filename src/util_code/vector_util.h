//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

#if !defined(VECTOR_UTIL_UTIL__INCLUDED_)
#define VECTOR_UTIL_UTIL__INCLUDED_

#include <vector>
using namespace std;

extern int FindInterval( vector< double > & vals, double val );
extern double Interpolate( vector< double > & vals, double val, int interval );

#endif

