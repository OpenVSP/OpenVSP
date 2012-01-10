//******************************************************************************
//    
//   Bi-Cubic Bezier Surface Class
//  
// 
//   J.R. Gloudemans - 12/10/93
//   Sterling Software
//
//******************************************************************************

#include "bezier_surf.h"
#include "bezier_patch.h"
#include "bicubic_surf.h"
#include "int_curve.h"

//===== Constructor  =====//
bezier_surf::bezier_surf()
{
  group_id = 0;
  num_pnts_u = 0;
  num_pnts_w = 0;

}

//===== Destructor  =====//
bezier_surf::~bezier_surf()
{
  int_curve_ptr_list.reset(); 
  while ( !int_curve_ptr_list.end_of_list() )
    {
      int_curve* icp = int_curve_ptr_list.get_curr();
      delete icp;
      int_curve_ptr_list.inc();
    }
 
}

//===== Init  =====//
void bezier_surf::init(int num_u, int num_w)
{

  //==== Initialize Array Sizes ====//
  num_pnts_u = num_u;
  num_pnts_w = num_w;
  pnts.init(num_u, num_w);

}

//===== From BiCubic Surface  =====//
void bezier_surf::compute_bnd_boxes()
{
  int u_ind, w_ind;
  surf_bbox.init();

  int num_patches_u = (num_pnts_u-1)/3;
  int num_patches_w = (num_pnts_w-1)/3;
//  int num_patches_u = (num_pnts_u-1)/4 + 1;
//  int num_patches_w = (num_pnts_w-1)/4 + 1;

  patch_bbox.init(num_patches_u, num_patches_w);

  for ( int iu = 0 ; iu < num_patches_u ; iu++ )
    {
      u_ind = iu*3; 
      for ( int iw = 0 ; iw < num_patches_w ; iw++ )
        {
          w_ind = iw*3;
          patch_bbox(iu, iw).init();
          for ( int i = 0 ; i < 4 ; i++ )
            {
              for ( int j = 0 ; j < 4 ; j++ )
                {
                  patch_bbox(iu, iw).update(pnts(u_ind+i, w_ind+j)); 
                }
            }
          surf_bbox.update(patch_bbox(iu, iw));
        }
    }

 
}

//===== Translate Bezier Surface  =====//
void bezier_surf::translate(const vec3d& trans_vec)
{
  for ( int iu = 0 ; iu < num_pnts_u ; iu++ )
    {
      for ( int iw = 0 ; iw < num_pnts_w ; iw++ )
        {
          pnts(iu, iw) = pnts(iu, iw) + trans_vec;
        }
    }
}

//===== Scale Bezier Surface  =====//
void bezier_surf::scale(const vec3d& scale_vec)
{
  for ( int iu = 0 ; iu < num_pnts_u ; iu++ )
    {
      for ( int iw = 0 ; iw < num_pnts_w ; iw++ )
        {
          pnts(iu, iw) = pnts(iu, iw) * scale_vec;
        }
    }
}

//===== Scale Bezier Surface About Point  =====//
void bezier_surf::scale_about_pnt(const vec3d& pnt, float val)
{
  vec3d offset;

  for ( int iu = 0 ; iu < num_pnts_u ; iu++ )
    {
      for ( int iw = 0 ; iw < num_pnts_w ; iw++ )
        {
          offset = pnts(iu, iw) - pnt;
          offset.normalize();
          pnts(iu, iw) = pnts(iu, iw) + offset*val;
        }
    }
}

//===== Scale Bezier Surface About Point  =====//
void bezier_surf::scale_about_line(const vec3d& pnt1, const vec3d& pnt2, float val)
{
  vec3d offset;
  vec3d proj_pnt;

  for ( int iu = 0 ; iu < num_pnts_u ; iu++ )
    {
      for ( int iw = 0 ; iw < num_pnts_w ; iw++ )
        {
          proj_pnt = proj_pnt_on_line_seg(pnt1, pnt2, pnts(iu, iw));
          offset = pnts(iu, iw) - proj_pnt;
          offset.normalize();
          pnts(iu, iw) = pnts(iu, iw) + offset*val;
        }
    }
}

//===== From BiCubic Surface  =====//
void bezier_surf::load_bicubic(bicubic_surf* bc)
{
  int u_ind, w_ind;
  vec3d tanu_00_3, tanw_00_3, tanu_10_3, tanw_10_3;
  vec3d tanu_11_3, tanw_11_3, tanu_01_3, tanw_01_3;

  id_number = bc->get_surface_number() + 1;
  u_render = bc->get_u_render();
  w_render = bc->get_w_render();
  num_pnts_u = (bc->get_num_u() - 1)*3 + 1;
  num_pnts_w = (bc->get_num_w() - 1)*3 + 1;

  pnts.init(num_pnts_u, num_pnts_w);

  for ( int iu = 0 ; iu < (bc->get_num_u() - 1) ; iu++ )
    {
      u_ind = iu*3; 
      for ( int iw = 0 ; iw < (bc->get_num_w() - 1) ; iw++ )
        {
          w_ind = iw*3;
          tanu_00_3 = bc->get_tanu(iu,  iw)/3.0;   tanw_00_3 = bc->get_tanw(iu,  iw)/3.0;
          tanu_10_3 = bc->get_tanu(iu+1,iw)/3.0;   tanw_10_3 = bc->get_tanw(iu+1,iw)/3.0;
          tanu_11_3 = bc->get_tanu(iu+1,iw+1)/3.0; tanw_11_3 = bc->get_tanw(iu+1,iw+1)/3.0;
          tanu_01_3 = bc->get_tanu(iu,  iw+1)/3.0; tanw_01_3 = bc->get_tanw(iu,  iw+1)/3.0;

          pnts(u_ind,   w_ind) = bc->get_pnt(iu,   iw);
          pnts(u_ind+1, w_ind) = bc->get_pnt(iu,   iw) + tanu_00_3;
          pnts(u_ind+2, w_ind) = bc->get_pnt(iu+1, iw) - tanu_10_3;
          pnts(u_ind+3, w_ind) = bc->get_pnt(iu+1, iw);

          pnts(u_ind,   w_ind+1) = bc->get_pnt(iu,   iw) + tanw_00_3;
          pnts(u_ind+1, w_ind+1) = bc->get_twist(iu, iw)/9.0 + 
                                   bc->get_pnt(iu,   iw) + tanu_00_3 + tanw_00_3;
          pnts(u_ind+2, w_ind+1) = bc->get_twist(iu+1, iw)/-9.0 + 
                                   bc->get_pnt(iu+1, iw) - tanu_10_3 + tanw_10_3;
          pnts(u_ind+3, w_ind+1) = bc->get_pnt(iu+1, iw) + tanw_10_3;

          pnts(u_ind,   w_ind+2) = bc->get_pnt(iu, iw+1) - tanw_01_3;
          pnts(u_ind+1, w_ind+2) = bc->get_twist(iu, iw+1)/9.0 + 
                                   bc->get_pnt(iu, iw+1) + tanu_01_3 - tanw_01_3;
          pnts(u_ind+2, w_ind+2) = bc->get_twist(iu+1, iw+1)/9.0 + 
                                   bc->get_pnt(iu+1, iw+1) - tanu_11_3 - tanw_11_3;
          pnts(u_ind+3, w_ind+2) = bc->get_pnt(iu+1, iw+1) - tanw_11_3;

          pnts(u_ind,   w_ind+3) = bc->get_pnt(iu,   iw+1);
          pnts(u_ind+1, w_ind+3) = bc->get_pnt(iu,   iw+1) + tanu_01_3;
          pnts(u_ind+2, w_ind+3) = bc->get_pnt(iu+1, iw+1) - tanu_11_3;
          pnts(u_ind+3, w_ind+3) = bc->get_pnt(iu+1, iw+1);
        }
    }


}

//===== Compute Blending Functions  =====//
void bezier_surf::blend_funcs(double u, double& F1, double& F2, double& F3, double& F4)
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

//===== Load A Point  =====//
void bezier_surf::put_pnt(int ind_u, int ind_w, const vec3d& pnt_in)
{
  pnts(ind_u,ind_w) = pnt_in;
}    



//===== Compute Point On Curve Given  U =====//
vec3d bezier_surf::comp_pnt(double u, double w)
{

  double F1u, F2u, F3u, F4u;
  double F1w, F2w, F3w, F4w;

  int trunc_u = (int)u;
  int u_index = trunc_u*3;
  if (u_index >= num_pnts_u-1) 
    {
      F1u = F2u = F3u = 0.0;
      F4u = 1.0;
      u_index = num_pnts_u-4;
    }
  else
    {
      blend_funcs(u-(double)trunc_u, F1u, F2u, F3u, F4u);
    }

  int trunc_w = (int)w;
  int w_index = trunc_w*3;
  if (w_index >= num_pnts_w-1) 
    {
      F1w = F2w = F3w = 0.0;
      F4w = 1.0;
      w_index = num_pnts_w-4;
    }
  else
    {
      blend_funcs(w-(double)trunc_w, F1w, F2w, F3w, F4w);
    }

   vec3d new_pnt;

   new_pnt = 
     (( pnts(u_index,  w_index)*F1u   + pnts(u_index+1,w_index)*F2u +         
        pnts(u_index+2,w_index)*F3u   + pnts(u_index+3,w_index)*F4u)   * F1w) +
     (( pnts(u_index,  w_index+1)*F1u + pnts(u_index+1,w_index+1)*F2u +         
        pnts(u_index+2,w_index+1)*F3u + pnts(u_index+3,w_index+1)*F4u) * F2w) +
     (( pnts(u_index,  w_index+2)*F1u + pnts(u_index+1,w_index+2)*F2u +         
        pnts(u_index+2,w_index+2)*F3u + pnts(u_index+3,w_index+2)*F4u) * F3w) +
     (( pnts(u_index,  w_index+3)*F1u + pnts(u_index+1,w_index+3)*F2u +         
        pnts(u_index+2,w_index+3)*F3u + pnts(u_index+3,w_index+3)*F4u) * F4w);

  return(new_pnt);
}

//===== Get Patch and Load Patch Obj =====//
void bezier_surf::load_patch(int ind_u, int ind_w, bezier_patch& bp)
{
  bp.int_curve_ptr_list.clear();

  bp.u_min = (double)ind_u;  bp.u_max = (double)ind_u+1.0;
  bp.w_min = (double)ind_w;  bp.w_max = (double)ind_w+1.0;

  int u_big_ind = ind_u*3;
  int w_big_ind = ind_w*3;

  for ( int iu = 0 ; iu < 4 ; iu++ )
    {
      for ( int iw = 0 ; iw < 4 ; iw++ )
        {
          bp.pnts[iu][iw] = pnts(u_big_ind+iu,w_big_ind+iw); 
        }
    }

  bp.bnd_box = patch_bbox(ind_u, ind_w);

}

//===== Intersect Two Bezier Surfaces =====//
void intersect(bezier_surf& bs1, bezier_surf& bs2) 
{


  //==== Intersect Two Surfs ====//
  int num_patches_u1, num_patches_w1;
  int num_patches_u2, num_patches_w2;
  int iu1, iw1, iu2, iw2;

  bezier_patch bp1;
  bezier_patch bp2;
 
  //==== Check Bounding Box Around Entire Comp ====//
  if (compare(bs1.surf_bbox, bs2.surf_bbox))
    {

      num_patches_u1 = (bs1.num_pnts_u-1)/3;
      num_patches_w1 = (bs1.num_pnts_w-1)/3;

      num_patches_u2 = (bs2.num_pnts_u-1)/3;
      num_patches_w2 = (bs2.num_pnts_w-1)/3;

      for ( iu1 = 0 ; iu1 < num_patches_u1 ; iu1++ )
       {
        for ( iw1 = 0 ; iw1 < num_patches_w1 ; iw1++ )
         {
           if ( compare( bs1.patch_bbox(iu1, iw1), bs2.surf_bbox))
             {
              for ( iu2 = 0 ; iu2 < num_patches_u2 ; iu2++ )
               {
                for ( iw2 = 0 ; iw2 < num_patches_w2 ; iw2++ )
                 {
                   bs1.load_patch(iu1, iw1, bp1);        
                   bs2.load_patch(iu2, iw2, bp2);
                   intersect( bp1, bp2 );
                   bs1.merge_int_curves(bp1);
                 }
               }
             }
         }
       }
    }
  bs1.assemble_int_curves();

  bs1.int_curve_ptr_list.reset();
  while ( !bs1.int_curve_ptr_list.end_of_list() )
    {
      if ( !(bs1.int_curve_ptr_list.get_curr()->get_surf1()) )
        bs1.int_curve_ptr_list.get_curr()->set_surf_ptrs(&bs1, &bs2);
      bs1.int_curve_ptr_list.inc();
    } 
  
}

//===== Intersect Two Bezier Surfaces =====//
void intersect_set_level(bezier_surf& bs1, bezier_surf& bs2, int level) 
{


  //==== Intersect Two Surfs ====//
  int num_patches_u1, num_patches_w1;
  int num_patches_u2, num_patches_w2;
  int iu1, iw1, iu2, iw2;

  bezier_patch bp1;
  bezier_patch bp2;
 
  //==== Check Bounding Box Around Entire Comp ====//
  if (compare(bs1.surf_bbox, bs2.surf_bbox))
    {

      num_patches_u1 = (bs1.num_pnts_u-1)/3;
      num_patches_w1 = (bs1.num_pnts_w-1)/3;

      num_patches_u2 = (bs2.num_pnts_u-1)/3;
      num_patches_w2 = (bs2.num_pnts_w-1)/3;

      for ( iu1 = 0 ; iu1 < num_patches_u1 ; iu1++ )
       {
        for ( iw1 = 0 ; iw1 < num_patches_w1 ; iw1++ )
         {
           if ( compare( bs1.patch_bbox(iu1, iw1), bs2.surf_bbox))
             {
              for ( iu2 = 0 ; iu2 < num_patches_u2 ; iu2++ )
               {
                for ( iw2 = 0 ; iw2 < num_patches_w2 ; iw2++ )
                 {
                   bs1.load_patch(iu1, iw1, bp1);        
                   bs2.load_patch(iu2, iw2, bp2);
                   intersect_set_level( bp1, bp2, level );
                   bs1.merge_int_curves(bp1);
                 }
               }
             }
         }
       }
    }
  bs1.assemble_int_curves();

  bs1.int_curve_ptr_list.reset();
  while ( !bs1.int_curve_ptr_list.end_of_list() )
    {
      if ( !(bs1.int_curve_ptr_list.get_curr()->get_surf1()) )
        bs1.int_curve_ptr_list.get_curr()->set_surf_ptrs(&bs1, &bs2);
      bs1.int_curve_ptr_list.inc();
    } 
  
}
                                                                                                   
                                                                                                   
          
//===== Dump Bezier Surfaces =====//
void bezier_surf::dump() 
{
  int tot_num_u = ((num_pnts_u-1)/3)*u_render + 1;
  int tot_num_w = ((num_pnts_w-1)/3)*w_render + 1;

  cout << endl;
  cout << " BEZIER_SURF " << endl;
  cout << " COMPONENT NUMBER =    1 " << endl;
  cout << " COMPONENT COLOR =     3 " << endl;
  cout << " CROSS SECTIONS =      " << tot_num_u << endl;
  cout << " PTS/CROSS SECTION =   " << tot_num_w << endl;

  vec3d new_pnt;
 
  double u_offset = (double)((num_pnts_u-1)/3)/(tot_num_u - 1);
  double w_offset = (double)((num_pnts_w-1)/3)/(tot_num_w - 1);

  for (int i = 0 ; i < tot_num_u ; i++)
    {
      for (int j = 0 ; j < tot_num_w ; j++)
        {
          new_pnt = comp_pnt(i*u_offset, j*w_offset);
          cout << new_pnt.x() << " " << new_pnt.y() << " " << new_pnt.z() << endl;
        }
    }

}
          
//===== Dump Bezier Surfaces =====//
void bezier_surf::dump_felisa(FILE* file_id) 
{
  int tot_num_u = ((num_pnts_u-1)/3)*u_render + 1;
  int tot_num_w = ((num_pnts_w-1)/3)*w_render + 1;

  fprintf(file_id, "%d	  1\n",id_number);
  fprintf(file_id, "%d    %d\n",tot_num_w, tot_num_u);
  vec3d new_pnt;
 
  double u_offset = (double)((num_pnts_u-1)/3)/(tot_num_u - 1);
  double w_offset = (double)((num_pnts_w-1)/3)/(tot_num_w - 1);

  for (int i = 0 ; i < tot_num_u ; i++)
    {
      for (int j = 0 ; j < tot_num_w ; j++)
        {
          new_pnt = comp_pnt(i*u_offset, j*w_offset);
          fprintf(file_id, "%f  %f  %f \n", new_pnt.x(), new_pnt.y(), new_pnt.z());

        }
    }

}
//===== Delete Intersection Curves  =====//
void bezier_surf::del_int_curve(int_curve* int_curve_ptr )
{
  int_curve* int_crv = 0;
  int_curve_ptr_list.reset(); 
  while ( !int_curve_ptr_list.end_of_list() )
    {
      int_crv = int_curve_ptr_list.get_curr();
      if ( int_crv == int_curve_ptr )
        {
          int_curve_ptr_list.remove_curr();
          int_curve_ptr_list.set_end();
        }
      int_curve_ptr_list.inc();
    } 
}

//===== Delete All Invalid Intersection Curves  =====//
void bezier_surf::del_all_invalid_int_curves()
{
  int_curve* int_crv = 0;
  
  int_curve_ptr_list.reset(); 
  while ( !int_curve_ptr_list.end_of_list() )
    {
       int_crv = int_curve_ptr_list.get_curr();
       if ( !int_crv->get_valid_flag() )
         {
           delete int_crv;
           int_curve_ptr_list.remove_curr(); 
         }
       else
         {
           int_curve_ptr_list.inc();
         }
     } 

}


//===== Merge Intersection Curves  =====//
void bezier_surf::merge_int_curves(bezier_patch& bp)
{
  int_curve* sub_crv = 0;

  bp.int_curve_ptr_list.reset();
  while ( !bp.int_curve_ptr_list.end_of_list() )
    {
      sub_crv = bp.int_curve_ptr_list.get_curr();
      int_curve_ptr_list.set_end();
      int_curve_ptr_list.insert_after(sub_crv);
      bp.int_curve_ptr_list.remove_curr();
   }
}


//===== Check For Duplicate Intersection Curves  =====//
int bezier_surf::duplicate_end_pts(int_curve* in_crv)
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
void bezier_surf::assemble_int_curves()
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
    
              //==== Check If Either Curve Has Surface Assigned Yet ===//
              if ( curr_crv->get_surf1() || test_crv->get_surf1())
                curr_dist = 1.0e06;
              else 
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
void bezier_surf::dump_int_curves()
{
cout << " BLOCK surface " << endl;
  int_curve_ptr_list.reset(); 
  while ( !int_curve_ptr_list.end_of_list() )
    {
      int_curve_ptr_list.get_curr()->dump_even_spaced_pnts();
//      int_curve_ptr_list.get_curr()->dump();
      int_curve_ptr_list.inc();
    } 
}

//===== Dump Intersection Curves  =====//
void bezier_surf::dump_int_curves_felisa(FILE* file_id)
{
  int_curve_ptr_list.reset(); 
  while ( !int_curve_ptr_list.end_of_list() )
    {
      if (  int_curve_ptr_list.get_curr()->get_surf1() == this )
        int_curve_ptr_list.get_curr()->dump_even_spaced_pnts_felisa(file_id);

      int_curve_ptr_list.inc();
    } 
}

//===== Add Intersection Curves  =====//
void bezier_surf::add_int_curves(bezier_surf& bs)
{
  int_curve* icp = 0;
  int_curve_ptr_list.set_end();

  bs.int_curve_ptr_list.reset();
  while ( !bs.int_curve_ptr_list.end_of_list() )
    {
      icp = bs.int_curve_ptr_list.get_curr();
      if ( icp->get_surf2() == this )
        {
          int_curve_ptr_list.insert_after(icp);
        }

      bs.int_curve_ptr_list.inc();
    } 
}

//===== Load Int Curves Into Array  =====//
void bezier_surf::load_int_curves_into_array( dyn_array<int_curve*>& int_curve_list)
{
  int_curve_ptr_list.reset(); 
  while ( !int_curve_ptr_list.end_of_list() )
    {
      int_curve* icp = int_curve_ptr_list.get_curr();
      int_curve_list.append( icp );
      int_curve_ptr_list.inc();
    }
}


//===== Number Intersection Curves  =====//
int bezier_surf::number_int_curves(int start_id)
{
  int curr_num = start_id;
  int_curve_ptr_list.reset(); 
  while ( !int_curve_ptr_list.end_of_list() )
    {
      int_curve* icp = int_curve_ptr_list.get_curr();
      if ( icp->get_id_number() < curr_num && icp->get_surf1() == this )
        {
          icp->set_id_number( curr_num );
          curr_num++;
        }
      int_curve_ptr_list.inc();
    }
  return( curr_num ); 
}



//===== Number Intersection Curves  =====//
void bezier_surf::dump_int_curve_indexs(FILE* file_id)
{
  fprintf(file_id, " %d	  %d      1	\n", id_number, id_number, int_curve_ptr_list.num_objects());
  fprintf(file_id, "%d	 \n",int_curve_ptr_list.num_objects());
  int_curve_ptr_list.reset(); 
  while ( !int_curve_ptr_list.end_of_list() )
    {
      fprintf(file_id, "%d   ",int_curve_ptr_list.get_curr()->get_id_number()+1);
      int_curve_ptr_list.inc();
    } 
  fprintf(file_id, "\n");
}



//===== Dump Pov File =====//
void bezier_surf::dump_pov_file(FILE* file_id)
{
  bezier_patch bp;

  fprintf(file_id, "#declare Bezier_Surface_%d =\n",id_number);
  fprintf(file_id, "   union {\n");

  int num_patches_u = (num_pnts_u-1)/3;
  int num_patches_w = (num_pnts_w-1)/3;

  for ( int iu = 0 ; iu < num_patches_u ; iu++ )
    {
      for ( int iw = 0 ; iw < num_patches_w ; iw++ )
        {
          load_patch(iu, iw, bp);
          bp.dump_pov_file(file_id);
        }
    }        
  fprintf(file_id, "   }\n");

}


void bezier_surf::intersect_line( vec3d& lp1, vec3d& lp2, dyn_array<vec3d>& int_pnts)
{
  int num_patches_u, num_patches_w;
  int iu, iw;

  bezier_patch bp;

  bbox line_bbox;
  line_bbox.update(lp1);
  line_bbox.update(lp2);
 
  //==== Check Bounding Box Around Entire Comp ====//
  if (compare(surf_bbox, line_bbox))
    {

      num_patches_u = (num_pnts_u-1)/3;
      num_patches_w = (num_pnts_w-1)/3;

      for ( iu = 0 ; iu < num_patches_u ; iu++ )
       {
        for ( iw = 0 ; iw < num_patches_w ; iw++ )
         {
           if ( compare( patch_bbox(iu, iw), line_bbox))
             {
               load_patch(iu, iw, bp);        
               bp.intersect_line( lp1, lp2, int_pnts );
             }
         }
       }
    }  
    
}

void bezier_surf::intersect_line( vec3d& lp1, vec3d& lp2, 
       dyn_array<vec3d>& int_pnts, dyn_array<int>& in_out)
{
  int num_patches_u, num_patches_w;
  int iu, iw;

  bezier_patch bp;

  bbox line_bbox;
  line_bbox.update(lp1);
  line_bbox.update(lp2);
 
  //==== Check Bounding Box Around Entire Comp ====//
  if (compare(surf_bbox, line_bbox))
    {

      num_patches_u = (num_pnts_u-1)/3;
      num_patches_w = (num_pnts_w-1)/3;

      for ( iu = 0 ; iu < num_patches_u ; iu++ )
       {
        for ( iw = 0 ; iw < num_patches_w ; iw++ )
         {
           if ( compare( patch_bbox(iu, iw), line_bbox))
             {
               load_patch(iu, iw, bp);        
               bp.intersect_line( lp1, lp2, int_pnts, in_out );
             }
         }
       }
    }  
    
}

/*********************************************
//===== Split Looped Intersect Curves =====//
void bezier_surf::split_int_curves()
{
  int_curve* new_crv;

  int_curve_ptr_list.reset(); 
  while ( !int_curve_ptr_list.end_of_list() )
    {
      new_crv = int_curve_ptr_list.get_curr()->split_if_loop();
      if (new_crv)
        {
          int_curve_ptr_list.insert_after(new_crv);
          int_curve_ptr_list.inc();
        }
      int_curve_ptr_list.inc();
    } 
}
************************************************/

//===== Dump Int Curves UW =====//
void bezier_surf::dump_uw_int_curves()
{
  int_curve* int_crv = 0;
  int_curve_ptr_list.reset(); 
  while ( !int_curve_ptr_list.end_of_list() )
    {
      int_crv = int_curve_ptr_list.get_curr();
      if ( int_crv->get_surf1() == this )
        int_crv->dump_uw(0);
      else  
        int_crv->dump_uw(1);

      int_curve_ptr_list.inc();
    } 
}

//===== Check For Common Border Curves =====//
int check_for_common_border( bezier_surf& bs1, bezier_surf& bs2 ) 
{
  int i, j;
  int common_flag = FALSE;

  //==== Set Group Id ====//
  if ( !bs1.group_id )
    bs1.group_id = (long)&bs1;

  for ( i = 0 ; i < 4 ; i++ )
    {
      for ( j = 0 ; j < 4 ; j++ )
        {
          if ( add_matching_border_curve_forward(bs1, bs2, i, j) )
             common_flag = TRUE;
        }
    }

  for ( i = 0 ; i < 4 ; i++ )
    {
      for ( j = 0 ; j < 4 ; j++ )
        {
          if ( add_matching_border_curve_backward(bs1, bs2, i, j) )
             common_flag = TRUE;
        }
    }

  if (common_flag)
    bs2.group_id  = bs1.group_id;

  return( common_flag );
}

//===== Add Matching Common Border Curves =====//
int add_matching_border_curve_forward( bezier_surf& bs1, bezier_surf& bs2, 
                                       int cur_id1, int cur_id2 )
{
  int i,j;
  int num_pnts_u, num_pnts_w;
//jrg FIXWARN 
  dyn_array<double> u1;		
  dyn_array<double> w1;		 
  dyn_array<vec3d> pnt1;  	

  num_pnts_u = (bs1.num_pnts_u-1)/3 + 1;
  num_pnts_w = (bs1.num_pnts_w-1)/3 + 1;

  if ( cur_id1 == 0 || cur_id1 == 1 )
    {
      for (  i = 0 ; i < num_pnts_u ; i++ )
        {
           u1.append( (double) i );
           if ( cur_id1 == 0 )
             {
               pnt1.append( bs1.pnts(i*3, 0) ); 
               w1.append(0.0);
             }
           else   
             {
               pnt1.append( bs1.pnts(i*3, bs1.num_pnts_w-1) ); 
               w1.append((double)(num_pnts_w-1));
             }
         }
    }
  else
    {
      for (  i = 0 ; i < num_pnts_w ; i++ )
        {
           w1.append( (double) i );
           if ( cur_id1 == 2 )
             {
               pnt1.append( bs1.pnts(0, i*3) ); 
               u1.append(0.0);
             }
           else   
             {
               pnt1.append( bs1.pnts(bs1.num_pnts_u-1, i*3) ); 
               u1.append((double)(num_pnts_u-1));
             }
         }
    }
//jrg FIXWARN
  dyn_array<double> u2;
  dyn_array<double> w2;
  dyn_array<vec3d> pnt2; 

  num_pnts_u = (bs2.num_pnts_u-1)/3 + 1;
  num_pnts_w = (bs2.num_pnts_w-1)/3 + 1;

  if ( cur_id2 == 0 || cur_id2 == 1 )
    {
      for (  i = 0 ; i < num_pnts_u ; i++ )
        {
           u2.append( (double) i );
           if ( cur_id2 == 0 )
             {
               pnt2.append( bs2.pnts(i*3, 0) ); 
               w2.append(0.0);
             }
           else   
             {
               pnt2.append( bs2.pnts(i*3, bs2.num_pnts_w-1) ); 
               w2.append((double)(num_pnts_w-1));
             }
         }
    }
  else
    {
      for (  i = 0 ; i < num_pnts_w ; i++ )
        {
           w2.append( (double) i );
           if ( cur_id2 == 2 )
             {
               pnt2.append( bs2.pnts(0, i*3) ); 
               u2.append(0.0);
             }
           else   
             {
               pnt2.append( bs2.pnts(bs2.num_pnts_u-1, i*3) ); 
               u2.append((double)(num_pnts_u-1));
             }
         }
    }
 
  if ( pnt1.dimension() != pnt2.dimension() )
    return(0);

  double dist_tol = 0.00001;
  //==== Check For Single Point ====//
  int common_flag = FALSE;
  for ( i = 0 ; i < pnt1.dimension()-1 ; i++ )
    {
      if ( dist ( pnt1(i), pnt1(i+1) ) > dist_tol )
        common_flag = TRUE;
    }
//  int common_flag = TRUE;

  for ( i = 0 ; i < pnt1.dimension() ; i++ )
    {
      if ( dist ( pnt1(i), pnt2(i) ) > dist_tol )
        common_flag = FALSE;
    }
  
  if ( common_flag )
    {
      int_curve* new_int_crv = new int_curve;
      bs1.int_curve_ptr_list.set_end();
      bs1.int_curve_ptr_list.insert_after(new_int_crv);

      int num_pnts = pnt1.dimension();

      for (  i = 0 ; i < num_pnts-1 ; i++ )
        {
          for (  j = 0 ; j < 5 ; j++ )
            {
              double fract = (double)j/5.0f;
              int_node* new_node = new int_node;
              double uu1 = u1[i] + fract*(u1[i+1] - u1[i]);
              double ww1 = w1[i] + fract*(w1[i+1] - w1[i]);
              double uu2 = u2[i] + fract*(u2[i+1] - u2[i]);
              double ww2 = w2[i] + fract*(w2[i+1] - w2[i]);

              vec3d bp1 = bs1.comp_pnt(uu1, ww1);
              vec3d bp2 = bs2.comp_pnt(uu2, ww2);

              new_node->pnt = (bp1 + bp2)*0.5;
              new_node->u1  = uu1;
              new_node->w1  = ww1;
              new_node->u2  = uu2;
              new_node->w2  = ww2;

              new_int_crv->load_node( new_node );
            }
         }
      int_node* new_node = new int_node;

      new_node->pnt = (pnt1(num_pnts-1) + pnt2(num_pnts-1))*0.5;
      new_node->u1  = u1[num_pnts-1];
      new_node->w1  = w1[num_pnts-1];
      new_node->u2  = u2[num_pnts-1];
      new_node->w2  = w2[num_pnts-1];

      new_int_crv->load_node( new_node );

      new_int_crv->set_surf_ptrs(&bs1, &bs2);
    }
  return(common_flag);
}

//===== Add Matching Common Border Curves =====//
int add_matching_border_curve_backward( bezier_surf& bs1, bezier_surf& bs2, 
                                        int cur_id1, int cur_id2 )
{
  int i,j;
  int num_pnts_u, num_pnts_w;

//jrg FIXWARN
  dyn_array<double> u1;		
  dyn_array<double> w1;		 
  dyn_array<vec3d> pnt1;  	

  num_pnts_u = (bs1.num_pnts_u-1)/3 + 1;
  num_pnts_w = (bs1.num_pnts_w-1)/3 + 1;

  if ( cur_id1 == 0 || cur_id1 == 1 )
    {
      for (  i = 0 ; i < num_pnts_u ; i++ )
        {
           u1.append( (double) i );
           if ( cur_id1 == 0 )
             {
               pnt1.append( bs1.pnts(i*3, 0) ); 
               w1.append(0.0);
             }
           else   
             {
               pnt1.append( bs1.pnts(i*3, bs1.num_pnts_w-1) ); 
               w1.append((double)(num_pnts_w-1));
             }
         }
    }
  else
    {
      for (  i = 0 ; i < num_pnts_w ; i++ )
        {
           w1.append( (double) i );
           if ( cur_id1 == 2 )
             {
               pnt1.append( bs1.pnts(0, i*3) ); 
               u1.append(0.0);
             }
           else   
             {
               pnt1.append( bs1.pnts(bs1.num_pnts_u-1, i*3) ); 
               u1.append((double)(num_pnts_u-1));
             }
         }
    }
//jrg FIXWARN
  dyn_array<double> u2;
  dyn_array<double> w2;
  dyn_array<vec3d> pnt2; 

  num_pnts_u = (bs2.num_pnts_u-1)/3 + 1;
  num_pnts_w = (bs2.num_pnts_w-1)/3 + 1;

  if ( cur_id2 == 0 || cur_id2 == 1 )
    {
      for (  i = num_pnts_u-1 ; i >= 0  ; i-- )
        {
           u2.append( (double) i );
           if ( cur_id2 == 0 )
             {
               pnt2.append( bs2.pnts(i*3, 0) ); 
               w2.append(0.0);
             }
           else   
             {
               pnt2.append( bs2.pnts(i*3, bs2.num_pnts_w-1) ); 
               w2.append((double)(num_pnts_w-1));
             }
         }
    }
  else
    {
      for (  i = num_pnts_w-1 ; i >= 0  ; i-- )
        {
           w2.append( (double) i );
           if ( cur_id2 == 2 )
             {
               pnt2.append( bs2.pnts(0, i*3) ); 
               u2.append(0.0);
             }
           else   
             {
               pnt2.append( bs2.pnts(bs2.num_pnts_u-1, i*3) ); 
               u2.append((double)(num_pnts_u-1));
             }
         }
    }
 
  if ( pnt1.dimension() != pnt2.dimension() )
    return(0);

  double dist_tol = 0.00001;
  //==== Check For Single Point ====//
  int common_flag = FALSE;
  for ( i = 0 ; i < pnt1.dimension()-1 ; i++ )
    {
      if ( dist ( pnt1(i), pnt1(i+1) ) > dist_tol )
        common_flag = TRUE;
    }
//  int common_flag = TRUE;

  for ( i = 0 ; i < pnt1.dimension() ; i++ )
    {
      if ( dist ( pnt1(i), pnt2(i) ) > dist_tol )
        common_flag = FALSE;
    }
  
  if ( common_flag )
    {
      int_curve* new_int_crv = new int_curve;
      bs1.int_curve_ptr_list.set_end();
      bs1.int_curve_ptr_list.insert_after(new_int_crv);

      int num_pnts = pnt1.dimension();

      for (  i = 0 ; i < num_pnts-1 ; i++ )
        {
          for (  j = 0 ; j < 5 ; j++ )
            {
              double fract = (double)j/5.0;
              int_node* new_node = new int_node;
              double uu1 = u1[i] + fract*(u1[i+1] - u1[i]);
              double ww1 = w1[i] + fract*(w1[i+1] - w1[i]);
              double uu2 = u2[i] + fract*(u2[i+1] - u2[i]);
              double ww2 = w2[i] + fract*(w2[i+1] - w2[i]);

              vec3d bp1 = bs1.comp_pnt(uu1, ww1);
              vec3d bp2 = bs2.comp_pnt(uu2, ww2);

              new_node->pnt = (bp1 + bp2)*0.5;
              new_node->u1  = uu1;
              new_node->w1  = ww1;
              new_node->u2  = uu2;
              new_node->w2  = ww2;

              new_int_crv->load_node( new_node );
            }
         }
      int_node* new_node = new int_node;

      new_node->pnt = (pnt1(num_pnts-1) + pnt2(num_pnts-1))*0.5;
      new_node->u1  = u1[num_pnts-1];
      new_node->w1  = w1[num_pnts-1];
      new_node->u2  = u2[num_pnts-1];
      new_node->w2  = w2[num_pnts-1];

      new_int_crv->load_node( new_node );


      new_int_crv->set_surf_ptrs(&bs1, &bs2);
    }
  return(common_flag);
}

/*****************************************************************
//===== Check For Common Border Curves =====//
int check_for_common_border( bezier_surf& bs1, bezier_surf& bs2 ) 
{
  //==== Set Group Id ====//
  if ( !bs1.group_id )
    bs1.group_id = (int)&bs1;

  int common_flag = FALSE; 
  double u1[2], u2[2];
  double w1[2], w2[2];
  vec3d pnt0, pnt1;

  int tot_num_u = ((bs1.num_pnts_u-1)/3);
  int tot_num_w = ((bs1.num_pnts_w-1)/3);

  //==== Side One ====//
  u1[0] = 0.0;		u1[1] = tot_num_u;
  w1[0] = 0.0;		w1[1] = 0.0;

  if ( find_matching_border_curve(bs1, bs2, u1, w1, u2, w2) )
    {
      common_flag = TRUE;
      bs1.add_border_int_curve(bs2, u1, w1, u2, w2);
    }

  //==== Side Two ====//
  u1[0] = tot_num_u;	u1[1] = tot_num_u;
  w1[0] = 0.0;		w1[1] = tot_num_w;

  if ( find_matching_border_curve(bs1, bs2, u1, w1, u2, w2) )
    {
      common_flag = TRUE;
      bs1.add_border_int_curve(bs2, u1, w1, u2, w2);
    }

  //==== Side Three ====//
  u1[0] = tot_num_u;	u1[1] = 0.0;
  w1[0] = tot_num_w;	w1[1] = tot_num_w;

  if ( find_matching_border_curve(bs1, bs2, u1, w1, u2, w2) )
    {
      common_flag = TRUE;
      bs1.add_border_int_curve(bs2, u1, w1, u2, w2);
    }

  //==== Side Four ====//
  u1[0] = 0.0;		u1[1] = 0.0;
  w1[0] = tot_num_w;	w1[1] = 0.0;

  if ( find_matching_border_curve(bs1, bs2, u1, w1, u2, w2) )
    {
      common_flag = TRUE;
      bs1.add_border_int_curve(bs2, u1, w1, u2, w2);
    }

  if (common_flag)
    bs2.group_id  = bs1.group_id;

  return(common_flag);

}

//===== Find Matching Border Curve =====//
int find_matching_border_curve(bezier_surf& bs1, bezier_surf& bs2,  
               double u1[2], double w1[2], double u2[2], double w2[2])
{
  vec3d pnt0  = bs1.comp_pnt(u1[0], w1[0]);
  vec3d pnt1  = bs1.comp_pnt(u1[1], w1[1]);

  int i;
  int match_flag = FALSE;

  double dist_tol = 0.000001;

  //==== Compute Corner Points ====//
  double cu[4];
  cu[0] = cu[3] = 0.0;
  cu[1] = cu[2] = ((bs2.num_pnts_u-1)/3);

  double cw[4];
  cw[0] = cw[1] = 0.0;
  cw[2] = cw[3] = ((bs2.num_pnts_w-1)/3);

  vec3d cp[4];
  for (  i = 0 ; i < 4 ; i++ )
    {
      cp[i] = bs2.comp_pnt( cu[i], cw[i] );
    }

  //==== Check For Match With First Point ====//
  int id1 = -1;
  for (  i = 0 ; i < 4 ; i++)  
    {
      if ( dist( pnt0, cp[i]) < dist_tol )
        {
          id1 = i;
        }
    }

  //==== Check For Match With Second Point ====//
  int id2 = -1;
  for (  i = 0 ; i < 4 ; i++)  
    {
      if ( dist( pnt1, cp[i]) < dist_tol )
        {
          id2 = i;
        }
    }

  //==== Check For Valid Indexs ====//
  if ( id1 >= 0 && id2 >= 0 && id1 != id2 )
    {
      if ( id1 == (id2+1)%4 || id2 == (id1+1)%4 )
        {
          u2[0] = cu[id1];	u2[1] = cu[id2];
          w2[0] = cw[id1];	w2[1] = cw[id2];
          if ( border_center_pnts_match( bs1, bs2, u1, w1, u2, w2) )
            match_flag = TRUE;
        }
    }
  return( match_flag );
}

//===== Find Matching Border Curve =====//
int border_center_pnts_match(bezier_surf& bs1, bezier_surf& bs2,  
          double u1[2], double w1[2], double u2[2], double w2[2])
{
  
  vec3d pnt0  = bs1.comp_pnt(u1[0], w1[0]);
  vec3d pnt1  = bs1.comp_pnt(u1[1], w1[1]);
  double dist_tol = 0.000001*dist( pnt0,  pnt1 );

  int match_flag = FALSE;

  vec3d cpnt1 = bs1.comp_pnt( 0.5*(u1[0] + u1[1]), 0.5*(w1[0] + w1[1]));

  vec3d cpnt2 = bs2.comp_pnt( 0.5*(u2[0] + u2[1]), 0.5*(w2[0] + w2[1]));

  if ( dist(cpnt1, cpnt2) < dist_tol ) match_flag = TRUE;

  return( match_flag ); 

}

//===== Create and Add Border Intersection Curves =====//
void bezier_surf::add_border_int_curve(bezier_surf& bs2, 
                  double u1[2], double w1[2], double u2[2], double w2[2])
{
  int_curve* new_int_crv = new int_curve;
  int_curve_ptr_list.set_end();
  int_curve_ptr_list.insert_after(new_int_crv);

  int num_pnts = 25;

  vec3d pnt;
  double uu1, ww1, uu2, ww2; 
  double u1_inc = (u1[1]-u1[0])/(double)(num_pnts - 1);
  double w1_inc = (w1[1]-w1[0])/(double)(num_pnts - 1);
  double u2_inc = (u2[1]-u2[0])/(double)(num_pnts - 1);
  double w2_inc = (w2[1]-w2[0])/(double)(num_pnts - 1);

  for ( int i = 0 ; i < num_pnts ; i++ )
    {
      uu1 = u1[0] + (double)i*u1_inc;
      ww1 = w1[0] + (double)i*w1_inc;
      uu2 = u2[0] + (double)i*u2_inc;
      ww2 = w2[0] + (double)i*w2_inc;
      pnt = comp_pnt( uu1, ww1 );

      int_node* new_node = new int_node;

      new_node->pnt = pnt;
      new_node->u1  = uu1;
      new_node->w1  = ww1;
      new_node->u2  = uu2;
      new_node->w2  = ww2;

      new_int_crv->load_node( new_node );

    }
  new_int_crv->set_surf_ptrs(this, &bs2);

}
***************************************************************/
