//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

//******************************************************************************
//    
//   Intersection Curve Class
//  
// 
//   J.R. Gloudemans - 2/3/94
//   Sterling Software
//
//******************************************************************************


#ifndef INT_CURVE_H
#define INT_CURVE_H

#include "dl_list.h"
#include "vec3d.h"
#include "vec2d.h"
#include "dyn_array.h"
#include <stdio.h>

#define FIRST_FIRST 0
#define FIRST_LAST  1
#define LAST_FIRST  2
#define LAST_LAST   3

struct int_node {
  vec3d pnt;
  double u1;
  double w1;
  double u2;
  double w2;
};

class bezier_surf;

class int_curve
{
   int valid_flag;
   int id_number;
   bezier_surf* surf1;
   bezier_surf* surf2;
   int_node* first_node;
   int_node* last_node;

   dl_list < int_node* > node_list;

   dyn_array < int_node* > split_list;

    int  duplicate_second_nodes( int_curve& in_crv, int join_case );

public:
   
   int_curve();
   ~int_curve();

private:
    //Explicitly disallow use of implicity generated methods 
	//Not implemented because not needed
	int_curve(const int_curve&);
	int_curve& operator= (const int_curve*);

public:
   void clear_space();
   int get_valid_flag()			{ return(valid_flag); }
   void set_id_number(int num_in)	{ id_number = num_in; }
   int  get_id_number()			{ return(id_number); }
   void load_all_pnts_into_array( dyn_array<vec3d>& pnt_list );

   void set_first_last_nodes();
   vec3d& get_first_pnt()	{ return(first_node->pnt); }
   vec3d& get_last_pnt()	{ return(last_node->pnt); }
   bezier_surf* get_surf1()	{ return(surf1); }
   bezier_surf* get_surf2()	{ return(surf2); }
   void set_surf_ptrs( bezier_surf* in_surf1, bezier_surf* in_surf2 ) 
     { surf1 = in_surf1;  surf2 = in_surf2; }
   void start( vec3d in_pnt[2], double u1[2], double w1[2], double u2[2], double w2[2] );
   double min_dist_ends( int_curve& in_crv );
   int case_min_dist_ends( int_curve& in_crv );

   void join( int_curve& in_crv);  
   void dump( );  
   void dump_even_spaced_pnts( );  
   void dump_even_spaced_pnts_felisa(FILE* file_id );

   int_curve* split_if_loop(  );
   void load_node( int_node* new_node );

   void intersect_int_curve( int_curve& in_crv );
   void dump_uw(int surf_id);

   friend void intersect( int_curve* ic1, int_curve* ic2 );
   friend void intersect_nodes( int_curve* ic_A, int_curve* ic_B, int A_flag, int B_flag, 
        int_node* A_nde1, int_node* A_nde2, int_node* B_nde1, int_node* B_nde2);
   friend void intersect_end_nodes( int_curve* ic_A, int A_flag, int B_flag, 
        int_node* A_nde1, int_node* A_nde2, int_node* B_end);


   void clear_split_list();
   void split();
   void insert_int_node(int_node* new_nde);
   void check_in_out_surfs(dyn_array<bezier_surf*>& surf_list);

   void snap_to_edges();
   friend void snap_int_curve_end_points(dyn_array<int_curve*>& all_int_curves);
  
};

   
#endif
