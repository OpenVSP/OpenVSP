//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
#ifndef VSPDEFINES_H
#define VSPDEFINES_H

#define _USE_MATH_DEFINES
#include <math.h>

#ifdef __VSP_PLUGIN__
//#define VSPDLL __declspec(dllexport)
#define PLUGIN 1
#else
#define PLUGIN 0
#endif
#define VSPDLL

#ifndef PI
#define PI (M_PI)
#endif

#ifndef DEG_2_RAD
#define DEG_2_RAD   (M_PI/180.0)
#endif

#ifndef RAD_2_DEG
#define RAD_2_DEG   (180.0/M_PI)
#endif


#endif



