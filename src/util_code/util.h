
#if !defined(UTIL_UTIL__INCLUDED_)
#define UTIL_UTIL__INCLUDED_

#include <vector>
#include "vec3d.h"
using namespace std;


#define MAX(x,y) (((x) < (y)) ? (y):(x))
#define MIN(x,y) (((x) < (y)) ? (x):(y))
#define SIGN(x,y) (((x)*(y) > 0.0) ? (x):(-x))
#define DEG2RAD(x) ( x*0.017453293 )
#define RAD2DEG(x) ( x*57.29578 )

//#define FRACT(v,min,max) ( ((max-min) == 0.0 ) ? (0.0):((v-min)/(max-min)))

#include <float.h>  /* for DBL_EPSILON */
#define FRACT(v,min,max) ( (fabs(max-min) <= DBL_EPSILON ) ? (0.0):((v-min)/(max-min)))

template <class T> bool ContainsVal( vector< T > & vec, T const & val );

#endif

