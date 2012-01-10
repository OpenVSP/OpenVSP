//******************************************************************************
//    
//   BiCubic Surface Class
//  
// 
//   J.R. Gloudemans - 12/10/93
//   Sterling Software
//
//******************************************************************************


#ifndef BICUBIC_SURF_H
#define BICUBIC_SURF_H

#include <stdio.h>

#include "array_2d.h"
#include "vec3d.h"

#define CLOSED 0
#define OPEN   1

class bicubic_surf
{
  int surface_number;
  int u_render;
  int w_render;
  int open_closed_flag_u;
  int open_closed_flag_w;
  int num_pnts_u;
  int num_pnts_w;
  array_2d<vec3d> pnts;
  array_2d<vec3d> tanu;
  array_2d<vec3d> tanw;
  array_2d<double> tanu_k;
  array_2d<double> tanw_k;

  vec3d twist;
 
  void blend_funcs(double u, double& F1, double& F2, double& F3, double& F4);

public:

  bicubic_surf();
  ~bicubic_surf();

  void init(int num_u, int num_w);
  void put_pnt(int ind_u, int ind_w, const vec3d& pnt_in);

  int get_num_u()			 { return(num_pnts_u); }
  int get_num_w()			 { return(num_pnts_w); }
  int get_surface_number()		 { return(surface_number); }
  int get_u_render()			 { return(u_render); }
  int get_w_render()			 { return(w_render); }
  vec3d& get_pnt(int ind_u, int ind_w)	 { return(pnts(ind_u, ind_w)); }
  vec3d& get_tanu(int ind_u, int ind_w)  { return(tanu(ind_u, ind_w)); }
  vec3d& get_tanw(int ind_u, int ind_w)  { return(tanw(ind_u, ind_w)); }
  vec3d& get_twist(int, int ) 		 { return(twist); }

  void put_tanu(int ind_u, int ind_w, const vec3d& tan_in);
  void put_tanw(int ind_u, int ind_w, const vec3d& tan_in);
  void put_tanu_k(int ind_u, int ind_w, double factor_in);
  void put_tanw_k(int ind_u, int ind_w, double factor_in);
  void comp_tans();

  void set_open_closed_flag_u( int flag )		{ open_closed_flag_u = flag; }
  void set_open_closed_flag_w( int flag )		{ open_closed_flag_w = flag; }

  vec3d comp_pnt(double u, double w);

  void dump_pov_file();
  void load_bezier_patch(int u_ind, int w_ind, vec3d patch[4][4]);
  void read(FILE* file_id);

};

#endif
  
  
