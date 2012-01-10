//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

//******************************************************************************
//    
//   Hermite Curve Class
//  
// 
//   J.R. Gloudemans - 10/10/93
//   Sterling Software
//
//******************************************************************************

#include "state.h"

#ifndef HERM_CURVE_H
#define HERM_CURVE_H

#include "vec3d.h"
#include "sarray.h"
#include "dyn_array_dbl.h"


//===== Define Axes =====//
#define X_AXIS 0
#define Y_AXIS 1
#define Z_AXIS 2

//===== Define Curve Types =====//
#define OPEN_CURVE   1
#define CLOSED_CURVE 0


#define NUM_SAMPLES 10

//===== State Flags =====//
#define NUM_FLAGS   	6
#define SAMPLE_FLAG 	0
#define LENGTH_FLAG 	1
#define U_FUNC_LENGTH 	2
#define U_FUNC_X 	3
#define U_FUNC_Y 	4
#define U_FUNC_Z 	5

class herm_curve
{
  //===== DATA MEMBERS =====//
  private:
  	int		num_pnts;
  	struct_array<vec3d>	pnts;
  	struct_array<vec3d>	tans;
  	Dyn_array_dbl	tan_k;
  	int		open_closed_flag;

 	state		curr_flags;
  	float		length;
  	struct_array<vec3d>	samp_pnts;
  	Dyn_array_dbl	u_func_length;
  	Dyn_array_dbl	u_func_xyz;
  


  //===== MEMBER FUNCTIONS =====//
  private:
  	void		blend_funcs(float u, float& F1, float& F2, float& F3, float& F4);
  	void		deriv_blend_funcs(float u, float& F1, float& F2, float& F3, float& F4);
  	void		load_sample_pnts();
  	float		sum_sample_length();
	float		compute_length();
  	void		load_u_func_length();
	void		load_u_func_length_xyz(int xyz_id);

  public:
	herm_curve();
    herm_curve(const herm_curve& curve);
	~herm_curve();

	//===== Operators =====//
	herm_curve& operator=(const herm_curve& curve);

	//===== Computational Functions =====//
	void		init(int num_cntrl_pnts);
	void		comp_tan(int index, float u, const vec3d& pnt_in);
	void		comp_tans();

	//===== Accessor Functions =====//
	int		    get_num_pnts() const				{ return( pnts.dimension() ); }
	void		load_pnt(int index, const vec3d& pnt_in);
	vec3d&		get_pnt(int index) const			{ return( pnts[index] ); }
	void		load_tan(int index, const vec3d& tan_in);
	vec3d&		get_tan(int index) const			{ return( tans[index] ); }
	float		get_length();
	void		set_open_closed_flag(int flag)		{ open_closed_flag = flag; }

	vec3d		comp_pnt(float u);
	vec3d		comp_pnt_per_xyz(int xyz_id, double per); 
	vec3d		comp_pnt_per_length(float per); 

	float		get_integral(int func_id, int var_id, int num_divs);
	vec3d		get_derivative(float u);

};

#endif
  
  
