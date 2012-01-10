//******************************************************************************
//    
//   Bezier Surface Class
//  
// 
//   J.R. Gloudemans - 12/10/93
//   Sterling Software
//
//******************************************************************************

#ifndef BEZIER_SURF_H
#define BEZIER_SURF_H

#include <stdio.h>

#include "defines.h"

#include "bbox.h"
#include "array_2d.h"
#include "vec3d.h"
#include "dl_list.h"
#include "dyn_array.h"

//class bicubic_surf;
//class int_curve;

#include "bicubic_surf.h"
#include "int_curve.h"

class bezier_patch;

class bezier_surf
{
  int id_number;
  int group_id;
  int u_render;
  int w_render;
  int num_pnts_u;
  int num_pnts_w;
  array_2d<vec3d> pnts;
  bbox surf_bbox;
  array_2d<bbox> patch_bbox;

  dl_list<int_curve*> int_curve_ptr_list;

  void blend_funcs(double u, double& F1, double& F2, double& F3, double& F4);

public:

  bezier_surf();
  ~bezier_surf();

private:
	//Explicitly disallow use of implicity generated methods 
	//Not implemented because not needed
	bezier_surf(const bezier_surf&);
	bezier_surf& operator= (const bezier_surf*);

public:
  void init(int num_u, int num_w);
  void put_pnt(int ind_u, int ind_w, const vec3d& pnt_in);
  void load_bicubic(bicubic_surf* bc);
  void compute_bnd_boxes();
  void translate(const vec3d& trans_vec);
  void scale(const vec3d& scale_vec);
  void scale_about_pnt(const vec3d& pnt, float val);
  void scale_about_line(const vec3d& pnt1, const vec3d& pnt2, float val);
  int  get_num_int_curves()		{ return( int_curve_ptr_list.num_objects() ); }
  int  number_int_curves( int start_id );
  void set_id_num( int id_in )		{ id_number = id_in; }
  
  void add_int_curve(int_curve* int_curve_ptr )	
      { int_curve_ptr_list.insert_after( int_curve_ptr ); }
  void del_int_curve(int_curve* int_curve_ptr );
  void del_all_invalid_int_curves();	

  void load_int_curves_into_array( dyn_array<int_curve*>& int_curve_list);

  int  get_id_number()			{ return(id_number); }
  int  get_group_id()			{ return(group_id); }
  int  get_u_max()			{ return((num_pnts_u-1)/3); }
  int  get_w_max()			{ return((num_pnts_w-1)/3); }

  int get_num_pnts_u()		{ return num_pnts_u; }
  int get_num_pnts_w()		{ return num_pnts_w; }

  vec3d get_control_pnt( int uid, int wid )		{ return pnts(uid, wid); }


  void load_patch(int ind_u, int ind_w, bezier_patch& bp); 
  vec3d comp_pnt(double u, double w);
  friend void intersect(bezier_surf& bs1, bezier_surf& bs2);
  friend void intersect_set_level(bezier_surf& bs1, bezier_surf& bs2, int level);
  void intersect_line(vec3d& lp1, vec3d& lp2, dyn_array<vec3d>& int_pnts);
  void intersect_line(vec3d& lp1, vec3d& lp2, dyn_array<vec3d>& int_pnts, dyn_array<int>& in_out);
  void merge_int_curves(bezier_patch& bp);
  int  duplicate_end_pts(int_curve* in_crv);
  void assemble_int_curves();
  void dump_int_curves();
  void dump();
  void dump_int_curves_felisa(FILE* file_id);
  void dump_felisa(FILE* file_id);
  void add_int_curves(bezier_surf& bs);
  void dump_int_curve_indexs(FILE* file_id);
  void dump_pov_file(FILE* file_id);

  void dump_uw_int_curves();

  friend int check_for_common_border( bezier_surf& bs1, bezier_surf& bs2 );
  friend int add_matching_border_curve_forward( bezier_surf& bs1, bezier_surf& bs2,int cur_id1, int cur_id2 );
  friend int add_matching_border_curve_backward( bezier_surf& bs1, bezier_surf& bs2,int cur_id1, int cur_id2 );


//  friend int find_matching_border_curve(bezier_surf& bs1, bezier_surf& bs2, double u1[2], double w1[2], double u2[2], double w2[2]);
//  friend int border_center_pnts_match(bezier_surf& bs1, bezier_surf& bs2, double u1[2], double w1[2], double u2[2], double w2[2]);
//  void add_border_int_curve(bezier_surf& bs2, double u1[2], double w1[2], double u2[2], double w2[2]);
 
  

};

#endif
  
  
