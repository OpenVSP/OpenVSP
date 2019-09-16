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

#include <cassert>
#include <cmath>
#include <cfloat>

#include <vector>
#include <list>
using namespace std;

class SurfaceIntersectionSingleton;
class CfdMeshMgrSingleton;

//===== Intersect Two Bezier Patches  =====//
void intersect( const SurfPatch& bp1, const SurfPatch& bp2, SurfaceIntersectionSingleton *MeshMgr );
void intersect_quads( const SurfPatch& pa, const SurfPatch& pb, SurfaceIntersectionSingleton *MeshMgr );
void refine_intersect_pt( const vec3d& pt, const SurfPatch &pA, double uwA[2], const SurfPatch &pB, double uwB[2] );
double refine_intersect_pt( const vec3d& pt, Surf *sA, vec2d &uwA, Surf *sB, vec2d &uwB );

#endif
