//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

//////////////////////////////////////////////////////////////////////
// IntersectQuads.h
// J.R. Gloudemans
//////////////////////////////////////////////////////////////////////

#if !defined(INTERSECT_QUAD__INCLUDED_)
#define INTERSECT_QUAD__INCLUDED_

#include "Vec2d.h"
#include "Vec3d.h"
#include "BndBox.h"
#include "SurfPatch.h"

#include <assert.h>
#include <cmath>
#include <float.h>

#include <vector>
#include <list>
using namespace std;


//===== Intersect Two Bezier Patches  =====//
void intersect( SurfPatch& bp1, SurfPatch& bp2, int depth );
void intersect_quads( SurfPatch& pa, SurfPatch& pb );

#endif
