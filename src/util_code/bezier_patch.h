//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

//******************************************************************************
//    
//   Bezier Patch Class
//  
// 
//   J.R. Gloudemans - 12/10/93
//   Sterling Software
//
//******************************************************************************


#ifndef BEZIER_PATCH_H
#define BEZIER_PATCH_H

#include "vec3d.h"
#include "bbox.h"
#include "dl_list.h"

#include "bezier_surf.h"

#define PLANE_TOL 0.01

//class int_curve;

#include "int_curve.h"

void intersect_quads( bezier_patch& bp1, bezier_patch& bp2);
int is_dup_pnt(int ni, vec3d int_pnt[2], vec3d& temp_pnt);

class bezier_patch
{
  double u_min, u_max;
  double w_min, w_max;
  vec3d pnts[4][4];
  bbox bnd_box;

  dl_list <int_curve*> int_curve_ptr_list;
  

  void blend_funcs(double u, double& F1, double& F2, double& F3, double& F4);

public:

  bezier_patch();
  ~bezier_patch();

  void put_pnt(int ind_u, int ind_w, const vec3d& pnt_in);
  void compute_bnd_box();

  vec3d comp_pnt(double u, double w);
  void  split_patch(bezier_patch& bp00, bezier_patch& bp10, 
                    bezier_patch& bp01, bezier_patch& bp11 );
  friend void bezier_surf::load_patch(int ind_u, int ind_w, bezier_patch& bp); 
  friend void bezier_surf::merge_int_curves(bezier_patch& bp); 

  int test_planar(double tol);

  friend void intersect( bezier_patch& bp1, bezier_patch& bp2);
  friend void intersect_set_level( bezier_patch& bp1, bezier_patch& bp2, int level);
  void find_closest_uw( vec3d& pnt_in, double guess_uw[2], double uw[2]);
  vec3d comp_pnt_01(double u, double w);
  vec3d comp_tan_u_01(double u, double w);
  vec3d comp_tan_w_01(double u, double w);
  void comp_delta_uw(vec3d& pnt_in, vec3d& guess_pnt, double norm_uw[2], double delta_uw[2]);

  void intersect_line(vec3d& lp1, vec3d& lp2, dyn_array<vec3d>& int_pnts);
  void intersect_line_quad( vec3d& lp1, vec3d& lp2, dyn_array<vec3d>& int_pnts);
  void intersect_line(vec3d& lp1, vec3d& lp2, dyn_array<vec3d>& int_pnts, dyn_array<int>& in_out);
  void intersect_line_quad(vec3d& lp1, vec3d& lp2, dyn_array<vec3d>& int_pnts, dyn_array<int>& in_out);
  friend void intersect_quads( bezier_patch& bp1, bezier_patch& bp2);
  friend int is_dup_pnt(int ni, vec3d int_pnt[2], vec3d& temp_pnt);
  void merge_int_curves(bezier_patch bps[4]);
  int duplicate_end_pts(int_curve* in_crv);
  void assemble_int_curves();
  void dump_int_curves();
  void dump_pov_file(FILE* file_id);
 
};

#endif
  
  
