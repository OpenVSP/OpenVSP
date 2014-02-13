//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// SWIGDefines.h: Creates functions to access some macros that swig has
// a difficult time wrapping
//
//////////////////////////////////////////////////////////////////////

#if !defined(SWIGDEFINES__INCLUDED_)
#define SWIGDEFINES__INCLUDED_

#include <stdio.h>
#include "APIErrorMgr.h"
#include <vector>
#include "Vec3d.h"

namespace vsp
{
const FILE* cstdout = stdout;
void AddVec3D( std::vector< vec3d > &INOUT, double x, double y, double z )
{
    INOUT.push_back( vec3d( x, y, z ) );
}
}

#endif