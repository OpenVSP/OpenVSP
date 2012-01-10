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

#include "vec2d.h"
#include "vec3d.h"
#include "bbox.h"

//Original
//#define DEFAULT_PLANE_TOL (0.0002)
//jrg 8/25/2010
//#define DEFAULT_PLANE_TOL (0.0002)
#define DEFAULT_PLANE_TOL (0.00001)

#include <assert.h>
#include <math.h>
#include <float.h>

#include <vector>
#include <list>
using namespace std;

class Surf;
class SurfPatch;


//void intersect_quads( SurfPatch& bp1, SurfPatch& bp2);

int is_dup_int_pnt(int ni, vec3d int_pnt[2], vec3d& temp_pnt);

//////////////////////////////////////////////////////////////////////
class SurfPatch
{
public:

	SurfPatch();
	virtual ~SurfPatch();

	void set_surf_ptr( Surf* ptr )								{ m_SurfPtr = ptr; }
	Surf* get_surf_ptr()										{ return m_SurfPtr; }

	void set_plane_tol( double t )								{ plane_tol = t; }

	void set_u_min_max( double min, double max )				{ u_min = min; u_max = max; }
	void set_w_min_max( double min, double max )				{ w_min = min; w_max = max; }


	void compute_bnd_box();
	void put_pnt(int iu, int iw, const vec3d& pnt_in)			{ pnts[iu][iw] = pnt_in; }
	vec3d comp_pnt(double u, double w);

	void split_patch(SurfPatch& bp00, SurfPatch& bp10, SurfPatch& bp01, SurfPatch& bp11 );
	bool test_planar(double tol);

	bbox* get_bbox()											{ return &bnd_box; }
	friend void intersect( SurfPatch& bp1, SurfPatch& bp2 );
	friend void intersect( SurfPatch& bp1, SurfPatch& bp2, int depth);
	void find_closest_uw( vec3d& pnt_in, double guess_uw[2], double uw[2]);
	void find_closest_uw( vec3d& pnt_in, double uw[2]);
	vec3d comp_pnt_01(double u, double w);
	vec3d comp_tan_u_01(double u, double w);
	vec3d comp_tan_w_01(double u, double w);


	void comp_delta_uw(vec3d& pnt_in, vec3d& guess_pnt, double norm_uw[2], double delta_uw[2]);

	void Draw();

	void IntersectLineSeg( vec3d & p0, vec3d & p1, bbox & line_box, vector< double > & t_vals );
	void AddTVal( double t, vector< double > & t_vals );

	void SetSubDepth( int d )							{ sub_depth = d; }
	int  GetSubDepth()									{ return sub_depth; }

	friend void intersect_quads( SurfPatch&  bp1, SurfPatch& bp2);


bool draw_flag;

protected:

	Surf* m_SurfPtr;

	double plane_tol;
	double u_min, u_max;
	double w_min, w_max;
	vec3d pnts[4][4];
	bbox bnd_box;

	int sub_depth;

//	list <int_curve*> int_curve_ptr_list;
	void blend_funcs(double u, double& F1, double& F2, double& F3, double& F4);

};


#endif 
