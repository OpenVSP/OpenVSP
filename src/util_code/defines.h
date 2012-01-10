//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

#ifdef __VSP_PLUGIN__ 
//#define VSPDLL __declspec(dllexport)
#define PLUGIN 1
#else
#define PLUGIN 0
#endif
#define VSPDLL

#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE 1
#endif


#ifndef OFF
#define OFF   0
#endif
#ifndef ON
#define ON    1
#endif

#ifndef PI
#define PI (3.14159265358979323846)
#endif

#ifndef DEG_2_RAD
#define DEG_2_RAD   (0.017453293)
#endif

#ifndef RAD_2_DEG
#define RAD_2_DEG   (57.29578)
#endif

#ifndef SMALL_POS_FLOAT
#define SMALL_POS_FLOAT   0.000001
#endif

#ifndef SMALL_NEG_FLOAT
#define SMALL_NEG_FLOAT  -0.000001
#endif

#ifdef NO_BOOL
typedef int bool;
#ifndef false
#define false 0
#endif
#ifndef true
#define true 1
#endif



#endif

