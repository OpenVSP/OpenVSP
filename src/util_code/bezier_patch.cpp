//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

//******************************************************************************
//    
//   Bi-Cubic Bezier Patch Class
//  
// 
//   J.R. Gloudemans - 1/27/94
//   Sterling Software
//
//******************************************************************************

#include <math.h>
#include "bezier_patch.h"
#include "int_curve.h"
#include <float.h> // for DBL_EPSILON

//===== Constructor  =====//
bezier_patch::bezier_patch()
{
  u_min = w_min = 0.0;
  u_max = u_max = 1.0;
  
}

//===== Destructor  =====//
bezier_patch::~bezier_patch()
{
}


//===== From BiCubic Surface  =====//
void bezier_patch::compute_bnd_box()
{
  bnd_box.init();
  for ( int iu = 0 ; iu < 4 ; iu++ )
    {
      for ( int iw = 0 ; iw < 4 ; iw++ )
        {
          bnd_box.update(pnts[iu][iw]); 
        }
    }
}

//===== Compute Blending Functions  =====//
void bezier_patch::blend_funcs(double u, double& F1, double& F2, double& F3, double& F4)
{

  //==== Compute All Blending Functions ====//
  double uu = u*u;
  double one_u = 1.0 - u;
  double one_u_sqr = one_u*one_u;

  F1 = one_u*one_u_sqr;
  F2 = 3.0*u*one_u_sqr;
  F3 = 3.0*uu*one_u;
  F4 = uu*u;

}

//===== Put A Point  =====//
void bezier_patch::put_pnt(int ind_u, int ind_w, const vec3d& pnt_in)
{
  pnts[ind_u][ind_w] = pnt_in;
}    

//===== Compute Point On Curve Given  U =====//
vec3d bezier_patch::comp_pnt(double u, double w)
{

  double F1u, F2u, F3u, F4u;
  double F1w, F2w, F3w, F4w;

  if ( u <= u_min )
    {
      F1u = 1.0;
      F2u = F3u = F4u = 0.0;
    }
  else if ( u >= u_max )
    {
      F1u = F2u = F3u = 0.0;
      F4u = 1.0;
    }
  else
    {
      blend_funcs((u - u_min)/(u_max-u_min), F1u, F2u, F3u, F4u);
    }

  if ( w <= w_min )
    {
      F1w = 1.0;
      F2w = F3w = F4w = 0.0;
    }
  else if ( w >= w_max )
    {
      F1w = F2w = F3w = 0.0;
      F4w = 1.0;
    }
  else
    {
      blend_funcs((w - w_min)/(w_max-w_min), F1w, F2w, F3w, F4w);
    }

   vec3d new_pnt;

   new_pnt = 
     (( pnts[0][0]*F1u + pnts[1][0]*F2u + pnts[2][0]*F3u + pnts[3][0]*F4u) * F1w) +
     (( pnts[0][1]*F1u + pnts[1][1]*F2u + pnts[2][1]*F3u + pnts[3][1]*F4u) * F2w) +
     (( pnts[0][2]*F1u + pnts[1][2]*F2u + pnts[2][2]*F3u + pnts[3][2]*F4u) * F3w) +
     (( pnts[0][3]*F1u + pnts[1][3]*F2u + pnts[2][3]*F3u + pnts[3][3]*F4u) * F4w);

   return(new_pnt);
}

//===== Split Patch =====//
void bezier_patch::split_patch(bezier_patch& bp00, bezier_patch& bp10, 
                               bezier_patch& bp01, bezier_patch& bp11 )
{
  vec3d tmp_hull[4][7];
  vec3d tmp0, tmp1;

  // === Hold u Constant and Insert in W Direction ===//
  for ( int iu = 0 ; iu < 4 ; iu++ )
    {
      tmp_hull[iu][0] = pnts[iu][0];            
      tmp_hull[iu][1] = (pnts[iu][0] + pnts[iu][1])*0.5;
      tmp0 = (pnts[iu][1] + pnts[iu][2])*0.5;             
      tmp1 = (pnts[iu][2] + pnts[iu][3])*0.5;

      tmp_hull[iu][2] = (tmp_hull[iu][1] + tmp0)*0.5;
      tmp0 = (tmp0 + tmp1)*0.5;

      tmp_hull[iu][3] = (tmp_hull[iu][2] + tmp0)*0.5; 
      tmp_hull[iu][4] = tmp0;
      tmp_hull[iu][5] = tmp1;
      tmp_hull[iu][6] = pnts[iu][3];
    }
        
  int iw;

  // === Hold W Constant and Insert in U Direction ===//
  for (  iw = 0 ; iw < 4 ; iw++ )
    {
      bp00.pnts[0][iw] =  tmp_hull[0][iw];
      bp00.pnts[1][iw] = (tmp_hull[0][iw] + tmp_hull[1][iw])*0.5;
      tmp0 = (tmp_hull[1][iw] + tmp_hull[2][iw])*0.5;
      tmp1 = (tmp_hull[2][iw] + tmp_hull[3][iw])*0.5;

      bp00.pnts[2][iw] = (bp00.pnts[1][iw] + tmp0)*0.5;
      tmp0 = (tmp0 + tmp1)*0.5;

      bp00.pnts[3][iw] = (bp00.pnts[2][iw] + tmp0)*0.5;
      bp10.pnts[0][iw] = bp00.pnts[3][iw];
      bp10.pnts[1][iw] = tmp0;
      bp10.pnts[2][iw] = tmp1;
      bp10.pnts[3][iw] = tmp_hull[3][iw];
    }

  for ( iw = 3 ; iw < 7 ; iw++ )
    {
      bp01.pnts[0][iw-3] =  tmp_hull[0][iw];
      bp01.pnts[1][iw-3] = (tmp_hull[0][iw] + tmp_hull[1][iw])*0.5;
      tmp0 = (tmp_hull[1][iw] + tmp_hull[2][iw])*0.5;
      tmp1 = (tmp_hull[2][iw] + tmp_hull[3][iw])*0.5;

      bp01.pnts[2][iw-3] = (bp01.pnts[1][iw-3] + tmp0)*0.5;
      tmp0 = (tmp0 + tmp1)*0.5;

      bp01.pnts[3][iw-3] = (bp01.pnts[2][iw-3] + tmp0)*0.5;
      bp11.pnts[0][iw-3] = bp01.pnts[3][iw-3];
      bp11.pnts[1][iw-3] = tmp0;
      bp11.pnts[2][iw-3] = tmp1;
      bp11.pnts[3][iw-3] = tmp_hull[3][iw];

    }

  bp00.u_min = u_min;                  bp00.w_min = w_min;
  bp00.u_max = 0.5*(u_min + u_max);    bp00.w_max = 0.5*(w_min + w_max);

  bp10.u_min = bp00.u_max;             bp10.w_min = w_min;
  bp10.u_max = u_max;                  bp10.w_max = bp00.w_max;

  bp01.u_min = u_min;                  bp01.w_min = bp00.w_max;
  bp01.u_max = bp00.u_max;             bp01.w_max = w_max;

  bp11.u_min = bp00.u_max;             bp11.w_min = bp00.w_max;
  bp11.u_max = u_max;                  bp11.w_max = w_max;

  bp00.compute_bnd_box();
  bp10.compute_bnd_box();
  bp01.compute_bnd_box();
  bp11.compute_bnd_box();
  

}


//===== Test If Patch Is Planar (within tol)  =====//
int bezier_patch::test_planar(double tol)
{
  vec3d org = pnts[0][0];
  vec3d v1 = pnts[3][0] - pnts[0][0]; 
  vec3d v2 = pnts[0][3] - pnts[0][0];

  vec3d norm = cross(v1, v2);

  norm.normalize();

  if ( dist_pnt_2_plane(org, norm, pnts[3][3]) > tol ) return(0);
  if ( dist_pnt_2_plane(org, norm, pnts[1][1]) > tol ) return(0);
  if ( dist_pnt_2_plane(org, norm, pnts[2][1]) > tol ) return(0);
  if ( dist_pnt_2_plane(org, norm, pnts[1][2]) > tol ) return(0);
  if ( dist_pnt_2_plane(org, norm, pnts[2][2]) > tol ) return(0);

  if ( dist_pnt_2_line( pnts[0][0], pnts[3][0], pnts[1][0]) > tol ) return(0);  
  if ( dist_pnt_2_line( pnts[0][0], pnts[3][0], pnts[2][0]) > tol ) return(0);  
  if ( dist_pnt_2_line( pnts[0][0], pnts[0][3], pnts[0][1]) > tol ) return(0);  
  if ( dist_pnt_2_line( pnts[0][0], pnts[0][3], pnts[0][2]) > tol ) return(0);  
  if ( dist_pnt_2_line( pnts[3][3], pnts[0][3], pnts[1][3]) > tol ) return(0);  
  if ( dist_pnt_2_line( pnts[3][3], pnts[0][3], pnts[2][3]) > tol ) return(0);  
  if ( dist_pnt_2_line( pnts[3][3], pnts[3][0], pnts[3][1]) > tol ) return(0);  
  if ( dist_pnt_2_line( pnts[3][3], pnts[3][0], pnts[3][2]) > tol ) return(0);  

  return(1);
}


//===== Intersect Two Bezier Patches  =====//
void intersect( bezier_patch& bp1, bezier_patch& bp2)
{

  if ( !compare(bp1.bnd_box, bp2.bnd_box) )
    return;

 
  bezier_patch bps[4];
 
  int pln_flag1 = bp1.test_planar( PLANE_TOL );      
  int pln_flag2 = bp2.test_planar( PLANE_TOL ); 

  if ( pln_flag1 && pln_flag2 )
    {
      //==== Plane - Plane Intersection =====//
      intersect_quads( bp1, bp2 );
    }

  else if ( pln_flag1 )
    {
      //==== Split Patch2 and Keep Subdividing =====//
      bp2.split_patch(bps[0], bps[1], bps[2], bps[3]);
 
      intersect( bp1, bps[0] );         
      intersect( bp1, bps[1] );         
      intersect( bp1, bps[2] );         
      intersect( bp1, bps[3] );

      bp1.assemble_int_curves();
    }         
                 
  else if ( pln_flag2 )
    {
      //==== Split Patch1 and Keep Subdividing =====//
      bp1.split_patch(bps[0], bps[1], bps[2], bps[3]);
 
      intersect( bps[0], bp2 );         
      intersect( bps[1], bp2 );         
      intersect( bps[2], bp2 );         
      intersect( bps[3], bp2 );

      bp1.merge_int_curves(bps);
      bp1.assemble_int_curves();
    }  
                 
  else if ( bp1.bnd_box.diag_dist() > bp2.bnd_box.diag_dist() )
    {
      //==== Split Patch1 and Keep Subdividing =====//
      bp1.split_patch(bps[0], bps[1], bps[2], bps[3]);

      intersect( bps[0], bp2 );         
      intersect( bps[1], bp2 );         
      intersect( bps[2], bp2 );         
      intersect( bps[3], bp2 );

      bp1.merge_int_curves(bps);
      bp1.assemble_int_curves();
    }
  
  else 
    {
      //==== Split Patch2 and Keep Subdividing =====//
      bp2.split_patch(bps[0], bps[1], bps[2], bps[3]);
 
      intersect( bp1, bps[0] );         
      intersect( bp1, bps[1] );         
      intersect( bp1, bps[2] );         
      intersect( bp1, bps[3] );

      bp1.assemble_int_curves();

    } 
/*
  cout << &bp1 <<  "  " << bp1.int_curve_ptr_list.num_objects() << endl;
     
  bp1.int_curve_ptr_list.reset();
  while ( !bp1.int_curve_ptr_list.end_of_list() )
    {
      cout << "     " << bp1.int_curve_ptr_list.get_curr() << endl;
      bp1.int_curve_ptr_list.inc();
    }
*/
        
}  

//===== Intersect Two Bezier Patches  =====//
void intersect_set_level( bezier_patch& bp1, bezier_patch& bp2, int level)
{

  if ( !compare(bp1.bnd_box, bp2.bnd_box) )
    return;

  level--; 

  if ( level < 0 )
    {
      //==== Plane - Plane Intersection =====//
      intersect_quads( bp1, bp2 );
    }

  else
    {
      bezier_patch bps1[4];
      bezier_patch bps2[4];

      //==== Split Patch1 and Keep Subdividing =====//
      bp1.split_patch(bps1[0], bps1[1], bps1[2], bps1[3]);

      //==== Split Patch2 and Keep Subdividing =====//
      bp2.split_patch(bps2[0], bps2[1], bps2[2], bps2[3]);
 
      for ( int i = 0 ; i < 4 ; i++ )
        {
          for ( int j = 0 ; j < 4 ; j++ )
            {
              intersect_set_level( bps1[i], bps2[j], level ); 
            }
        }
        
      bp1.merge_int_curves(bps1);
      bp1.assemble_int_curves();
    }         
                 
}  

//===== Intersect Two Close To Planar Quadrilaterals  =====//
void intersect_quads( bezier_patch& bp1, bezier_patch& bp2)
{
  vec3d temp_pnt;
  vec3d int_pnt[2];
  double u1[2], w1[2];
  double u2[2], w2[2];

  double r, s, t;
  r = s = t = 0.0; 

  //==== Form Triangle and Segment Vectors ====//
  vec3d OA1 = bp1.pnts[0][0];
  vec3d OB1 = bp1.pnts[3][3];
  vec3d A1 = bp1.pnts[3][3] - bp1.pnts[0][0];
  vec3d B1 = bp1.pnts[3][0] - bp1.pnts[0][0];
  vec3d C1 = bp1.pnts[0][3] - bp1.pnts[0][0];
  vec3d D1 = bp1.pnts[3][0] - bp1.pnts[3][3];
  vec3d E1 = bp1.pnts[0][3] - bp1.pnts[3][3];
  double del_u1 = bp1.u_max - bp1.u_min;
  double del_w1 = bp1.w_max - bp1.w_min;

  vec3d OA2 = bp2.pnts[0][0];
  vec3d OB2 = bp2.pnts[3][3];
  vec3d A2 = bp2.pnts[3][3] - bp2.pnts[0][0];
  vec3d B2 = bp2.pnts[3][0] - bp2.pnts[0][0];
  vec3d C2 = bp2.pnts[0][3] - bp2.pnts[0][0];
  vec3d D2 = bp2.pnts[3][0] - bp2.pnts[3][3];
  vec3d E2 = bp2.pnts[0][3] - bp2.pnts[3][3];
  double del_u2 = bp2.u_max - bp2.u_min;
  double del_w2 = bp2.w_max - bp2.w_min;

  int ni = 0;

  //==== Perform Intersections - First Triangle====//
  if ( tri_seg_intersect(OA1, A1, B1, OA2, B2, r, s, t) ) 
    {
      u1[ni] = bp1.u_min + (r + s)*del_u1;
      w1[ni] = bp1.w_min + r*del_w1;
      u2[ni] = bp2.u_min + t*del_u2;
      w2[ni] = bp2.w_min;
      int_pnt[ni] = OA2 + B2*t;  ni++;
    }
  if ( tri_seg_intersect(OA1, A1, B1, OA2, C2, r, s, t) ) 
    {
      temp_pnt = OA2 + C2*t;
      if ( !is_dup_pnt(ni, int_pnt, temp_pnt) ) 
        {
          u1[ni] = bp1.u_min + (r + s)*del_u1;
          w1[ni] = bp1.w_min + r*del_w1;
          u2[ni] = bp2.u_min;
          w2[ni] = bp2.w_min + t*del_w2;
          int_pnt[ni] = temp_pnt; ni++;
        }
    }
  if ( (ni < 2)  && tri_seg_intersect(OA1, A1, B1, OB2, D2, r, s, t) ) 
    {
      temp_pnt = OB2 + D2*t;
      if ( !is_dup_pnt(ni, int_pnt, temp_pnt) ) 
        {
          u1[ni] = bp1.u_min + (r + s)*del_u1;
          w1[ni] = bp1.w_min + r*del_w1;
          u2[ni] = bp2.u_max;
          w2[ni] = bp2.w_max - t*del_w2;
          int_pnt[ni] = temp_pnt; ni++;
        }
    }
  if ( (ni < 2)  && tri_seg_intersect(OA1, A1, B1, OB2, E2, r, s, t) ) 
    {
      temp_pnt = OB2 + E2*t;
      if ( !is_dup_pnt(ni, int_pnt, temp_pnt) ) 
        {
          u1[ni] = bp1.u_min + (r + s)*del_u1;
          w1[ni] = bp1.w_min + r*del_w1;
          u2[ni] = bp2.u_max - t*del_u2;
          w2[ni] = bp2.w_max;
          int_pnt[ni] = temp_pnt; ni++;
        }
    }

  //==== Perform Intersections - Second Triangle====//
  if ( (ni < 2)  && tri_seg_intersect(OA1, A1, C1, OA2, B2, r, s, t) ) 
    {
      temp_pnt =  OA2 + B2*t;
      if ( !is_dup_pnt(ni, int_pnt, temp_pnt) ) 
        {
          u1[ni] = bp1.u_min + r*del_u1;
          w1[ni] = bp1.w_min + (r + s)*del_w1;
          u2[ni] = bp2.u_min + t*del_u2;
          w2[ni] = bp2.w_min;
          int_pnt[ni] = temp_pnt; ni++;
        }
    }
  if ( (ni < 2)  && tri_seg_intersect(OA1, A1, C1, OA2, C2, r, s, t) ) 
    {
      temp_pnt =  OA2 + C2*t;
      if ( !is_dup_pnt(ni, int_pnt, temp_pnt) ) 
        {
          u1[ni] = bp1.u_min + r*del_u1;
          w1[ni] = bp1.w_min + (r + s)*del_w1;
          u2[ni] = bp2.u_min;
          w2[ni] = bp2.w_min + t*del_w2;
          int_pnt[ni] = temp_pnt; ni++;
        }
     }
  if ( (ni < 2)  && tri_seg_intersect(OA1, A1, C1, OB2, D2, r, s, t) ) 
    {
      temp_pnt = OB2 + D2*t;
      if ( !is_dup_pnt(ni, int_pnt, temp_pnt) ) 
        {
          u1[ni] = bp1.u_min + r*del_u1;
          w1[ni] = bp1.w_min + (r + s)*del_w1;
          u2[ni] = bp2.u_max;
          w2[ni] = bp2.w_max - t*del_w2;
          int_pnt[ni] = temp_pnt; ni++;
        }
    }
  if ( (ni < 2)  && tri_seg_intersect(OA1, A1, C1, OB2, E2, r, s, t) ) 
    {
      temp_pnt = OB2 + E2*t;
      if ( !is_dup_pnt(ni, int_pnt, temp_pnt) ) 
        {
          u1[ni] = bp1.u_min + r*del_u1;
          w1[ni] = bp1.w_min + (r + s)*del_w1;
          u2[ni] = bp2.u_max - t*del_u2;
          w2[ni] = bp2.w_max;
          int_pnt[ni] = temp_pnt; ni++;
        }
    }

  //==== Perform Intersections - Third Triangle====//
  if ( (ni < 2)  && tri_seg_intersect(OA2, A2, B2, OA1, B1, r, s, t) ) 
    {
      temp_pnt = OA1 + B1*t;
      if ( !is_dup_pnt(ni, int_pnt, temp_pnt) ) 
        {
          u2[ni] = bp2.u_min + (r + s)*del_u2;
          w2[ni] = bp2.w_min + r*del_w2;
          u1[ni] = bp1.u_min + t*del_u1;
          w1[ni] = bp1.w_min;
          int_pnt[ni] = temp_pnt; ni++;
        }
    }
  if ( (ni < 2)  && tri_seg_intersect(OA2, A2, B2, OA1, C1, r, s, t) ) 
    {
      temp_pnt = OA1 + C1*t;
      if ( !is_dup_pnt(ni, int_pnt, temp_pnt) ) 
        {
          u2[ni] = bp2.u_min + (r + s)*del_u2;
          w2[ni] = bp2.w_min + r*del_w2;
          u1[ni] = bp1.u_min;
          w1[ni] = bp1.w_min + t*del_w1;
          int_pnt[ni] = temp_pnt; ni++;
        }
    }
  if ( (ni < 2)  && tri_seg_intersect(OA2, A2, B2, OB1, D1, r, s, t) ) 
    {
      temp_pnt = OB1 + D1*t;
      if ( !is_dup_pnt(ni, int_pnt, temp_pnt) ) 
        {
          u2[ni] = bp2.u_min + (r + s)*del_u2;
          w2[ni] = bp2.w_min + r*del_w2;
          u1[ni] = bp1.u_max;
          w1[ni] = bp1.w_max - t*del_w1;
          int_pnt[ni] = temp_pnt; ni++;
        }
    }
  if ( (ni < 2)  && tri_seg_intersect(OA2, A2, B2, OB1, E1, r, s, t) ) 
    {
      temp_pnt = OB1 + E1*t;
      if ( !is_dup_pnt(ni, int_pnt, temp_pnt) ) 
        {
          u2[ni] = bp2.u_min + (r + s)*del_u2;
          w2[ni] = bp2.w_min + r*del_w2;
          u1[ni] = bp1.u_max - t*del_u1;
          w1[ni] = bp1.w_max;
          int_pnt[ni] = temp_pnt; ni++;
        }
    }

  //==== Perform Intersections - Fourth Triangle====//
  if ( (ni < 2)  && tri_seg_intersect(OA2, A2, C2, OA1, B1, r, s, t) ) 
    {
      temp_pnt = OA1 + B1*t;
      if ( !is_dup_pnt(ni, int_pnt, temp_pnt) ) 
        {
          u2[ni] = bp2.u_min + r*del_u2;
          w2[ni] = bp2.w_min + (r + s)*del_w2;
          u1[ni] = bp1.u_min + t*del_u1;
          w1[ni] = bp1.w_min;
          int_pnt[ni] = temp_pnt; ni++;
        }
    }
  if ( (ni < 2)  && tri_seg_intersect(OA2, A2, C2, OA1, C1, r, s, t) ) 
    {
      temp_pnt = OA1 + C1*t;
      if ( !is_dup_pnt(ni, int_pnt, temp_pnt) ) 
        {
          u2[ni] = bp2.u_min + r*del_u2;
          w2[ni] = bp2.w_min + (r + s)*del_w2;
          u1[ni] = bp1.u_min;
          w1[ni] = bp1.w_min + t*del_w1;
          int_pnt[ni] = temp_pnt; ni++;
        }
    }
  if ( (ni < 2)  && tri_seg_intersect(OA2, A2, C2, OB1, D1, r, s, t) ) 
    {
      temp_pnt = OB1 + D1*t;
      if ( !is_dup_pnt(ni, int_pnt, temp_pnt) ) 
        {
          u2[ni] = bp2.u_min + r*del_u2;
          w2[ni] = bp2.w_min + (r + s)*del_w2;
          u1[ni] = bp1.u_max;
          w1[ni] = bp1.w_max - t*del_w1;
          int_pnt[ni] = temp_pnt; ni++;
        }
    }
  if ( (ni < 2)  && tri_seg_intersect(OA2, A2, C2, OB1, E1, r, s, t) ) 
    {
      temp_pnt = OB1 + E1*t;
      if ( !is_dup_pnt(ni, int_pnt, temp_pnt) ) 
        {
          u2[ni] = bp2.u_min + r*del_u2;
          w2[ni] = bp2.w_min + (r + s)*del_w2;
          u1[ni] = bp1.u_max - t*del_u1;
          w1[ni] = bp1.w_max;
          int_pnt[ni] = temp_pnt; ni++;
        }
    }
//if (ni == 1 )
// cout << " ONLY ONE PNT " << endl;

  if ( ni > 1)
    {
      double real_uw[2];
      double guess_uw[2];
 
//      for ( int i = 0 ; i < 3 ; i++ )
//        {
      guess_uw[0] = u1[0];  guess_uw[1] = w1[0];
      bp1.find_closest_uw( int_pnt[0], guess_uw, real_uw );
      u1[0] = real_uw[0];   w1[0] = real_uw[1];

      guess_uw[0] = u1[1];  guess_uw[1] = w1[1];
      bp1.find_closest_uw( int_pnt[1], guess_uw, real_uw );
      u1[1] = real_uw[0];   w1[1] = real_uw[1];

      guess_uw[0] = u2[0];  guess_uw[1] = w2[0];
      bp2.find_closest_uw( int_pnt[0], guess_uw, real_uw );
      u2[0] = real_uw[0];   w2[0] = real_uw[1];

      guess_uw[0] = u2[1];  guess_uw[1] = w2[1];
      bp2.find_closest_uw( int_pnt[1], guess_uw, real_uw );
      u2[1] = real_uw[0];   w2[1] = real_uw[1];

//      int_pnt[0] = ( bp1.comp_pnt( u1[0], w1[0]) + bp2.comp_pnt( u2[0], w2[0]) )* 0.5;
//      int_pnt[1] = ( bp1.comp_pnt( u1[1], w1[1]) + bp2.comp_pnt( u2[1], w2[1]) )* 0.5;
//      int_pnt[0] = bp1.comp_pnt( u1[0], w1[0]);
//      int_pnt[1] = bp1.comp_pnt( u1[1], w1[1]);
//        }

      int_curve* new_int_curve = new int_curve;

      bp1.int_curve_ptr_list.set_end();
      bp1.int_curve_ptr_list.insert_after(new_int_curve);

      new_int_curve->start(int_pnt, u1, w1, u2, w2);


/*
  cout << endl;
  cout << " INT_CURVE " << endl;
  cout << " COMPONENT NUMBER =    1 " << endl;
  cout << " COMPONENT COLOR =     3 " << endl;
  cout << " CROSS SECTIONS =      1" << endl;
  cout << " PTS/CROSS SECTION =   2" <<  endl;
*/
//      cout << int_pnt[0] << endl;
     
//      cout << int_pnt[1] << endl;
    }
}


//===== Find Closest UW On Patch to Given Point  =====//
void bezier_patch::find_closest_uw(vec3d& pnt_in, double guess_uw[2], double uw[2])
{
  int Max_Iter  = 5;
  double UW_Tol = 0.00000001;
  double norm_uw[2];
  double del_uw[2];
  vec3d guess_pnt;

  //==== Normalize uw Values Between 0 and 1 ====//
  norm_uw[0] = ( guess_uw[0] - u_min)/(u_max - u_min);
  norm_uw[1] = ( guess_uw[1] - w_min)/(w_max - w_min);

  //===== Loop Until U and W Stops Changing or Max Iterations is Hit  =====//
  int cnt = 0;
  int stop_flag = FALSE;
  while ( !stop_flag )
    {
      guess_pnt = comp_pnt_01( norm_uw[0], norm_uw[1] );

      //==== Find Delta U and W Values ====//
      comp_delta_uw( pnt_in, guess_pnt, norm_uw, del_uw );

      norm_uw[0] += del_uw[0];
      norm_uw[1] -= del_uw[1];

      //==== Test To Stop Iteration ====//
      if ( (fabs(del_uw[0]) + fabs(del_uw[1])) < UW_Tol )
        stop_flag = TRUE;

      else if ( cnt > Max_Iter )
        stop_flag = TRUE;

      else
        cnt++;
     }

  //==== Convert uw Values Back to Original Space ====//
  uw[0] = norm_uw[0]*(u_max - u_min) + u_min;             
  uw[1] = norm_uw[1]*(w_max - w_min) + w_min;             
  
  //==== Clamp Values At Bounds  ====//
  if      ( uw[0] < u_min ) uw[0] = u_min;
  else if ( uw[0] > u_max ) uw[0] = u_max;
      
  if      ( uw[1] < w_min ) uw[1] = w_min;
  else if ( uw[1] > w_max ) uw[1] = w_max;
 
}


//===== Compute Point On Patch  =====//
vec3d bezier_patch::comp_pnt_01(double u, double w)
{
  double uu    = u*u;
  double one_u = 1.0 - u;
  double F1u   = one_u*one_u*one_u;
  double F2u   = 3.0*one_u*one_u*u;
  double F3u   = 3.0*one_u*uu;
  double F4u   = uu*u;

  double ww    = w*w;
  double one_w = 1.0 - w;
  double F1w   = one_w*one_w*one_w;
  double F2w   = 3.0*one_w*one_w*w;
  double F3w   = 3.0*one_w*ww;
  double F4w   = ww*w;
  
  //==== Compute Point On Surface ====//
  vec3d new_pnt = 
     (( pnts[0][0]*F1u + pnts[1][0]*F2u + pnts[2][0]*F3u + pnts[3][0]*F4u) * F1w) +
     (( pnts[0][1]*F1u + pnts[1][1]*F2u + pnts[2][1]*F3u + pnts[3][1]*F4u) * F2w) +
     (( pnts[0][2]*F1u + pnts[1][2]*F2u + pnts[2][2]*F3u + pnts[3][2]*F4u) * F3w) +
     (( pnts[0][3]*F1u + pnts[1][3]*F2u + pnts[2][3]*F3u + pnts[3][3]*F4u) * F4w);

  return( new_pnt );  

}

//===== Compute Tangent In U Direction   =====//
vec3d bezier_patch::comp_tan_u_01(double u, double w)
{
  double uu    = u*u;
  double one_u = 1.0 - u;
  double F1u   = -3.0*one_u*one_u;
  double F2u   = 3.0 - 12.0*u + 9.0*uu;
  double F3u   = 6.0*u - 9.0*uu;
  double F4u   = 3.0*uu;

  double ww    = w*w;
  double one_w = 1.0 - w;
  double F1w   = one_w*one_w*one_w;
  double F2w   = 3.0*one_w*one_w*w;
  double F3w   = 3.0*one_w*ww;
  double F4w   = ww*w;
  
  //==== Compute Tangent In U Direction ====//
  vec3d new_pnt = 
     (( pnts[0][0]*F1u + pnts[1][0]*F2u + pnts[2][0]*F3u + pnts[3][0]*F4u) * F1w) +
     (( pnts[0][1]*F1u + pnts[1][1]*F2u + pnts[2][1]*F3u + pnts[3][1]*F4u) * F2w) +
     (( pnts[0][2]*F1u + pnts[1][2]*F2u + pnts[2][2]*F3u + pnts[3][2]*F4u) * F3w) +
     (( pnts[0][3]*F1u + pnts[1][3]*F2u + pnts[2][3]*F3u + pnts[3][3]*F4u) * F4w);

  return( new_pnt );  

}

//===== Compute Tangent In W Direction   =====//
vec3d bezier_patch::comp_tan_w_01(double u, double w)
{
  double uu    = u*u;
  double one_u = 1.0 - u;
  double F1u   = one_u*one_u*one_u;
  double F2u   = 3.0*one_u*one_u*u;
  double F3u   = 3.0*one_u*uu;
  double F4u   = uu*u;

  double ww    = w*w;
  double one_w = 1.0 - w;
  double F1w   = -3.0*one_w*one_w;
  double F2w   = 3.0 - 12.0*w + 9.0*ww;
  double F3w   = 6.0*w - 9.0*ww;
  double F4w   = 3.0*ww;
  
  //==== Compute Point On Surface ====//
  vec3d new_pnt = 
     (( pnts[0][0]*F1u + pnts[1][0]*F2u + pnts[2][0]*F3u + pnts[3][0]*F4u) * F1w) +
     (( pnts[0][1]*F1u + pnts[1][1]*F2u + pnts[2][1]*F3u + pnts[3][1]*F4u) * F2w) +
     (( pnts[0][2]*F1u + pnts[1][2]*F2u + pnts[2][2]*F3u + pnts[3][2]*F4u) * F3w) +
     (( pnts[0][3]*F1u + pnts[1][3]*F2u + pnts[2][3]*F3u + pnts[3][3]*F4u) * F4w);

  return( new_pnt );  

}

//===== Compute Point On Patch  =====//
void bezier_patch::comp_delta_uw( vec3d& pnt_in, vec3d& guess_pnt, double norm_uw[2], double delta_uw[2] )
{
  vec3d tan_u = comp_tan_u_01( norm_uw[0], norm_uw[1] );
  vec3d tan_w = comp_tan_w_01( norm_uw[0], norm_uw[1] );

  vec3d dist_vec = guess_pnt - pnt_in;

  vec3d A = cross(tan_w, dist_vec);
  vec3d B = cross(tan_u, dist_vec);
  vec3d norm = cross(tan_u, tan_w);

  double N = dot(norm, norm);

  //if ( N != 0.0 )
  if( fabs(N) > DBL_EPSILON )
    {
      delta_uw[0] = dot(A, norm)/N;
      delta_uw[1] = dot(B, norm)/N;
    }
  else
    {
      delta_uw[0] = 0.0;
      delta_uw[1] = 0.0;
    }
}



//===== Check If New Intersection Point Is A Duplicate  =====//
int is_dup_pnt(int ni, vec3d int_pnt[2], vec3d& temp_pnt)
{
  double Dist_Tol = 0.00000001;

  if ( ni == 0 ) return(0);

  if ( dist_squared( int_pnt[0], temp_pnt) < Dist_Tol ) return(1);

  return(0);
      
}


//===== Merge Intersection Curves  =====//
void bezier_patch::merge_int_curves(bezier_patch bps[4])
{
  
  int_curve* sub_crv=0;

  //==== Loop Thru Sub-Patch ===//
  for ( int isub = 0 ; isub < 4 ; isub++ )
    {
      bps[isub].int_curve_ptr_list.reset();
      while ( !bps[isub].int_curve_ptr_list.end_of_list() )
        {
          sub_crv = bps[isub].int_curve_ptr_list.get_curr();
//          if ( !duplicate_end_pts(sub_crv))
//            {
              int_curve_ptr_list.set_end();
              int_curve_ptr_list.insert_after(sub_crv);
//            }

          bps[isub].int_curve_ptr_list.remove_curr();

        }
    }
}

//===== Assemble Intersection Curves  =====//
int bezier_patch::duplicate_end_pts(int_curve* in_crv)
{
  double Dist_tol = 0.0000001;

  int duplicate_flag = 0;

  int_curve* curr_crv = 0;
  int_curve_ptr_list.reset();
  while ( !duplicate_flag  && !int_curve_ptr_list.end_of_list() )
    {
      curr_crv =  int_curve_ptr_list.get_curr();
      
      if ( (dist_squared(curr_crv->get_first_pnt(), in_crv->get_first_pnt()) +     
            dist_squared(curr_crv->get_last_pnt(), in_crv->get_last_pnt())) < Dist_tol)
        duplicate_flag = 1;
      else if ( (dist_squared(curr_crv->get_first_pnt(), in_crv->get_last_pnt()) +     
                 dist_squared(curr_crv->get_last_pnt(), in_crv->get_first_pnt())) < Dist_tol)
        duplicate_flag = 1;

      int_curve_ptr_list.inc();
    }

if (duplicate_flag) cout << "DUPLICATE !!!" << endl;

  return(duplicate_flag);


}

//===== Assemble Intersection Curves  =====//
void bezier_patch::assemble_int_curves()
{
  double Dist_tol = 0.0001;
  double min_dist, curr_dist;

  int still_condensing_curves = 1;   

  int_curve* curr_crv = 0; int_curve** curr_crv_ptr = 0;
  int_curve* test_crv = 0; int_curve** test_crv_ptr = 0;
  int_curve* base_crv = 0; int_curve** base_crv_ptr = 0;
  int_curve* join_crv = 0; int_curve** join_crv_ptr = 0;

  while ( still_condensing_curves ) 
    {  
      //===== Find Int Curve Pair With Min Dist Between End Points =====//
      min_dist = Dist_tol;
      base_crv_ptr = join_crv_ptr = 0;   
      int_curve_ptr_list.reset(); 
      while ( !int_curve_ptr_list.end_of_list() )
        {
          curr_crv = int_curve_ptr_list.get_curr();
          curr_crv_ptr = int_curve_ptr_list.get_curr_ptr();
          int_curve_ptr_list.inc();
          while (!int_curve_ptr_list.end_of_list() )
            { 
              test_crv = int_curve_ptr_list.get_curr();
              test_crv_ptr = int_curve_ptr_list.get_curr_ptr();
              curr_dist = curr_crv->min_dist_ends(*test_crv);
              if ( curr_dist < min_dist )
                {
                  min_dist = curr_dist;
                  base_crv_ptr = curr_crv_ptr;
                  join_crv_ptr = test_crv_ptr;
                }
              int_curve_ptr_list.inc();
            }
          int_curve_ptr_list.set_curr_ptr(curr_crv_ptr);
          int_curve_ptr_list.inc();
        }
      if ( base_crv_ptr )
        {
          int_curve_ptr_list.set_curr_ptr(base_crv_ptr);
          base_crv = int_curve_ptr_list.get_curr();
          int_curve_ptr_list.set_curr_ptr(join_crv_ptr);
          join_crv = int_curve_ptr_list.get_curr();

          base_crv->join(*join_crv);
          delete join_crv;
          int_curve_ptr_list.remove_curr();
          still_condensing_curves = 1;
        }
      else
        {
          still_condensing_curves = 0;
        }
    }
          
  
}


//===== Dump Intersection Curves  =====//
void bezier_patch::dump_int_curves()
{
  int_curve_ptr_list.reset(); 
  while ( !int_curve_ptr_list.end_of_list() )
    {
      int_curve_ptr_list.get_curr()->dump();
      int_curve_ptr_list.inc();
    } 
}



//===== Dump Intersection Curves  =====//
void bezier_patch::dump_pov_file(FILE* file_id)
{
  fprintf(file_id, "     bicubic_patch {\n");
  fprintf(file_id, "       type 1 \n");
  fprintf(file_id, "       flatness 0.0 \n");
  fprintf(file_id, "       u_steps 2 \n");
  fprintf(file_id, "       v_steps 2 \n");
  for ( int ip = 0 ; ip < 4 ; ip++ )
    {
      fprintf(file_id, "       ");
      for ( int jp = 0 ; jp < 4 ; jp++ )
        {
          fprintf(file_id, "<%f, %f, %f>",
                  pnts[ip][jp].x(),  pnts[ip][jp].y(),  pnts[ip][jp].z());
          if (ip != 3 || jp != 3)
            fprintf(file_id, ",");
        }
      fprintf(file_id, "\n");
    }
  fprintf(file_id, "        }\n");
}

//===== Intersect Patch With Line  =====//
void bezier_patch::intersect_line(vec3d& lp1, vec3d& lp2, dyn_array<vec3d>& int_pnts)
{
  bbox line_bbox;
  line_bbox.update(lp1);
  line_bbox.update(lp2);
  line_bbox.update(lp1);
  line_bbox.update(lp2);

  if ( !compare(bnd_box, line_bbox) )
    return;
 
  bezier_patch bps[4];

  int pln_flag = test_planar( PLANE_TOL );      

  if ( pln_flag )
    {
      //==== Plane - Line Intersection =====//
      intersect_line_quad( lp1, lp2, int_pnts );
    }
                 
  else if ( bnd_box.diag_dist() > line_bbox.diag_dist() )
    {
      //==== Split Patch and Keep Subdividing =====//
      split_patch(bps[0], bps[1], bps[2], bps[3]);

      bps[0].intersect_line( lp1, lp2, int_pnts );         
      bps[1].intersect_line( lp1, lp2, int_pnts );         
      bps[2].intersect_line( lp1, lp2, int_pnts );         
      bps[3].intersect_line( lp1, lp2, int_pnts );         
    }
  else 
    {
      //==== Split Line and Keep Subdividing =====//
      vec3d half_pnt = (lp1 + lp2)*0.5;

      intersect_line( lp1, half_pnt, int_pnts);
      intersect_line( half_pnt, lp2, int_pnts);
    } 
}  

//===== Intersect Two Close To Planar Quadrilaterals  =====//
void bezier_patch::intersect_line_quad( vec3d& lp1, vec3d& lp2, dyn_array<vec3d>& int_pnts)
{
  double u, w, t;
  u = w = t = 0.0; 

  //==== Form Triangle  ====//
  vec3d A = pnts[3][3] - pnts[0][0];
  vec3d B = pnts[3][0] - pnts[0][0];
  vec3d C = pnts[0][3] - pnts[0][0];


  //==== Form Seg Vector  ====//
  vec3d svec = lp2 - lp1;

  //==== Perform Intersections - First Triangle====//
  if ( tri_seg_intersect(pnts[0][0], B, A, lp1, svec, u, w, t) ) 
    {
      int_pnts.append(lp1 + svec*t);
    }

  else if ( tri_seg_intersect(pnts[0][0], A, C, lp1, svec, u, w, t) ) 
    {
      int_pnts.append(lp1 + svec*t);
    }
/*******************
else 
 {
  cout << "No Intersection " << endl;
  cout << " lp1 = " << lp1 << " lp2 = " << lp2 << endl;
  cout << " p00 = " << pnts[0][0]<< endl;
  cout << " p30 = " << pnts[3][0]<< endl;
  cout << " p03 = " << pnts[0][3]<< endl;
  cout << " p33 = " << pnts[3][3]<< endl;
 }
*************************/
}

//===== Intersect Patch With Line  =====//
void bezier_patch::intersect_line(vec3d& lp1, vec3d& lp2, 
       dyn_array<vec3d>& int_pnts, dyn_array<int>& in_out)
{
  bbox line_bbox;
  line_bbox.update(lp1);
  line_bbox.update(lp2);
  line_bbox.update(lp1);
  line_bbox.update(lp2);

  if ( !compare(bnd_box, line_bbox) )
    return;

 
  bezier_patch bps[4];
 
  int pln_flag = test_planar( PLANE_TOL );      

  if ( pln_flag )
    {
      //==== Plane - Line Intersection =====//
      intersect_line_quad( lp1, lp2, int_pnts, in_out );
    }
                 
  else if ( bnd_box.diag_dist() > line_bbox.diag_dist() )
    {
      //==== Split Patch and Keep Subdividing =====//
      split_patch(bps[0], bps[1], bps[2], bps[3]);

      bps[0].intersect_line( lp1, lp2, int_pnts, in_out );         
      bps[1].intersect_line( lp1, lp2, int_pnts, in_out );         
      bps[2].intersect_line( lp1, lp2, int_pnts, in_out );         
      bps[3].intersect_line( lp1, lp2, int_pnts, in_out );         
    }
  else 
    {
      //==== Split Line and Keep Subdividing =====//
      vec3d half_pnt = (lp1 + lp2)*0.5;

      intersect_line( lp1, half_pnt, int_pnts, in_out);
      intersect_line( half_pnt, lp2, int_pnts, in_out);
    } 
}  

//===== Intersect Two Close To Planar Quadrilaterals  =====//
void bezier_patch::intersect_line_quad( vec3d& lp1, vec3d& lp2, 
       dyn_array<vec3d>& int_pnts, dyn_array<int>& in_out)
{
  double u, w, t;
  u = w = t = 0.0; 

  //==== Form Triangle  ====//
  vec3d A = pnts[3][3] - pnts[0][0];
  vec3d B = pnts[3][0] - pnts[0][0];
  vec3d C = pnts[0][3] - pnts[0][0];


  //==== Form Seg Vector  ====//
  vec3d svec = lp2 - lp1;

  //==== Perform Intersections - First Triangle====//
  if ( tri_seg_intersect(pnts[0][0], B, A, lp1, svec, u, w, t) ) 
    {
      if ( angle( svec, cross(A,B))  > PI*0.5 )
        in_out.append(1);
      else
        in_out.append(0);
        
      int_pnts.append(lp1 + svec*t);
    }

  else if ( tri_seg_intersect(pnts[0][0], A, C, lp1, svec, u, w, t) ) 
    {
      if ( angle( svec, cross(A,C)) > PI*0.5  )
        in_out.append(1);
      else
        in_out.append(0);

      int_pnts.append(lp1 + svec*t);
    }
}

/********************************************************************

BOUNDING BOX DUMP


  cout << endl;
  cout << " BBOX_PNT " << endl;
  cout << " COMPONENT NUMBER =    1 " << endl;
  cout << " COMPONENT COLOR =     3 " << endl;
  cout << " CROSS SECTIONS =      1" << endl;
  cout << " PTS/CROSS SECTION =   2" <<  endl;
  cout << bp1.bnd_box.get_min(0) << " " << bp1.bnd_box.get_min(1) << " " << bp1.bnd_box.get_min(2) << " " << endl;
  cout << bp1.bnd_box.get_min(0) << " " << bp1.bnd_box.get_min(1) << " " << bp1.bnd_box.get_min(2) << " " << endl;

  cout << endl;
  cout << " BBOX_PNT " << endl;
  cout << " COMPONENT NUMBER =    1 " << endl;
  cout << " COMPONENT COLOR =     3 " << endl;
  cout << " CROSS SECTIONS =      1" << endl;
  cout << " PTS/CROSS SECTION =   2" <<  endl;
  cout << bp1.bnd_box.get_max(0) << " " << bp1.bnd_box.get_min(1) << " " << bp1.bnd_box.get_min(2) << " " << endl;
  cout << bp1.bnd_box.get_max(0) << " " << bp1.bnd_box.get_min(1) << " " << bp1.bnd_box.get_min(2) << " " << endl;

  cout << endl;
  cout << " BBOX_PNT " << endl;
  cout << " COMPONENT NUMBER =    1 " << endl;
  cout << " COMPONENT COLOR =     3 " << endl;
  cout << " CROSS SECTIONS =      1" << endl;
  cout << " PTS/CROSS SECTION =   2" <<  endl;
  cout << bp1.bnd_box.get_min(0) << " " << bp1.bnd_box.get_max(1) << " " << bp1.bnd_box.get_min(2) << " " << endl;
  cout << bp1.bnd_box.get_min(0) << " " << bp1.bnd_box.get_max(1) << " " << bp1.bnd_box.get_min(2) << " " << endl;

  cout << endl;
  cout << " BBOX_PNT " << endl;
  cout << " COMPONENT NUMBER =    1 " << endl;
  cout << " COMPONENT COLOR =     3 " << endl;
  cout << " CROSS SECTIONS =      1" << endl;
  cout << " PTS/CROSS SECTION =   2" <<  endl;
  cout << bp1.bnd_box.get_min(0) << " " << bp1.bnd_box.get_min(1) << " " << bp1.bnd_box.get_max(2) << " " << endl;
  cout << bp1.bnd_box.get_min(0) << " " << bp1.bnd_box.get_min(1) << " " << bp1.bnd_box.get_max(2) << " " << endl;

  cout << endl;
  cout << " BBOX_PNT " << endl;
  cout << " COMPONENT NUMBER =    1 " << endl;
  cout << " COMPONENT COLOR =     3 " << endl;
  cout << " CROSS SECTIONS =      1" << endl;
  cout << " PTS/CROSS SECTION =   2" <<  endl;
  cout << bp1.bnd_box.get_max(0) << " " << bp1.bnd_box.get_max(1) << " " << bp1.bnd_box.get_min(2) << " " << endl;
  cout << bp1.bnd_box.get_max(0) << " " << bp1.bnd_box.get_max(1) << " " << bp1.bnd_box.get_min(2) << " " << endl;

  cout << endl;
  cout << " BBOX_PNT " << endl;
  cout << " COMPONENT NUMBER =    1 " << endl;
  cout << " COMPONENT COLOR =     3 " << endl;
  cout << " CROSS SECTIONS =      1" << endl;
  cout << " PTS/CROSS SECTION =   2" <<  endl;
  cout << bp1.bnd_box.get_max(0) << " " << bp1.bnd_box.get_min(1) << " " << bp1.bnd_box.get_max(2) << " " << endl;
  cout << bp1.bnd_box.get_max(0) << " " << bp1.bnd_box.get_min(1) << " " << bp1.bnd_box.get_max(2) << " " << endl;

  cout << endl;
  cout << " BBOX_PNT " << endl;
  cout << " COMPONENT NUMBER =    1 " << endl;
  cout << " COMPONENT COLOR =     3 " << endl;
  cout << " CROSS SECTIONS =      1" << endl;
  cout << " PTS/CROSS SECTION =   2" <<  endl;
  cout << bp1.bnd_box.get_min(0) << " " << bp1.bnd_box.get_max(1) << " " << bp1.bnd_box.get_max(2) << " " << endl;
  cout << bp1.bnd_box.get_min(0) << " " << bp1.bnd_box.get_max(1) << " " << bp1.bnd_box.get_max(2) << " " << endl;

  cout << endl;
  cout << " BBOX_PNT " << endl;
  cout << " COMPONENT NUMBER =    1 " << endl;
  cout << " COMPONENT COLOR =     3 " << endl;
  cout << " CROSS SECTIONS =      1" << endl;
  cout << " PTS/CROSS SECTION =   2" <<  endl;
  cout << bp1.bnd_box.get_max(0) << " " << bp1.bnd_box.get_max(1) << " " << bp1.bnd_box.get_max(2) << " " << endl;
  cout << bp1.bnd_box.get_max(0) << " " << bp1.bnd_box.get_max(1) << " " << bp1.bnd_box.get_max(2) << " " << endl;

********************************************************************/











