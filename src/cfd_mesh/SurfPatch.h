//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

//////////////////////////////////////////////////////////////////////
// SurfPatch.h
// J.R. Gloudemans
//////////////////////////////////////////////////////////////////////

#if !defined(SURF_SURFPATCH__INCLUDED_)
#define SURF_SURFPATCH__INCLUDED_

#include "Vec2d.h"
#include "Vec3d.h"
#include "BndBox.h"

#include <assert.h>
#include <cmath>
#include <float.h>

#include <vector>
#include <list>
using namespace std;

#include "Vec3d.h"

#include "eli/code_eli.hpp"

#include "eli/geom/surface/bezier.hpp"
#include "eli/geom/surface/piecewise.hpp"
#include "eli/geom/curve/piecewise_creator.hpp"
#include "eli/geom/surface/piecewise_general_skinning_surface_creator.hpp"
#include "eli/geom/intersect/minimum_distance_surface.hpp"

typedef eli::geom::surface::bezier<double, 3> surface_patch_type;
typedef eli::geom::surface::piecewise<eli::geom::surface::bezier, double, 3> piecewise_surface_type;
typedef piecewise_surface_type::point_type surface_point_type;

typedef piecewise_surface_type::tolerance_type surface_tolerance_type;


class Surf;
class SurfPatch;
class SurfaceIntersectionSingleton;
class CfdMeshMgrSingleton;

//////////////////////////////////////////////////////////////////////
class SurfPatch
{
public:

    SurfPatch();
    virtual ~SurfPatch();

    void set_surf_ptr( Surf* ptr )
    {
        m_SurfPtr = ptr;
    }
    Surf* get_surf_ptr()
    {
        return m_SurfPtr;
    }

    void set_u_min_max( double min, double max )
    {
        u_min = min;
        u_max = max;
    }
    void set_w_min_max( double min, double max )
    {
        w_min = min;
        w_max = max;
    }

    void setPatch( const surface_patch_type &p )
    {
        m_Patch = p;
    }

    long degree_u()
    {
        return m_Patch.degree_u();
    }

    long degree_v()
    {
        return m_Patch.degree_v();
    }

    double get_u_min()    { return u_min; }
    double get_u_max()    { return u_max; }
    double get_w_min()    { return w_min; }
    double get_w_max()    { return w_max; }

    void compute_bnd_box();

    void split_patch( SurfPatch& bp00, SurfPatch& bp10, SurfPatch& bp01, SurfPatch& bp11 );
    bool test_planar( double tol );
    bool test_planar_rel( double reltol );

    BndBox* get_bbox()
    {
        return &bnd_box;
    }
    friend void intersect( SurfPatch& bp1, SurfPatch& bp2, int depth, SurfaceIntersectionSingleton *MeshMgr );
    void find_closest_uw( vec3d& pnt_in, double uw[2] );
    vec3d comp_pnt_01( double u, double w );
    vec3d comp_tan_u_01( double u, double w );
    vec3d comp_tan_w_01( double u, double w );


    void Draw();

    void IntersectLineSeg( vec3d & p0, vec3d & p1, BndBox & line_box, vector< double > & t_vals );
    void AddTVal( double t, vector< double > & t_vals );

    void SetSubDepth( int d )
    {
        sub_depth = d;
    }
    int  GetSubDepth()
    {
        return sub_depth;
    }

    friend void intersect_quads( SurfPatch&  bp1, SurfPatch& bp2, SurfaceIntersectionSingleton *MeshMgr );

protected:

    Surf* m_SurfPtr;

    surface_patch_type m_Patch;

    double u_min, u_max;
    double w_min, w_max;

    BndBox bnd_box;

    int sub_depth;

};


#endif
