//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

//******************************************************************************
//    
//   BiCubic Surface Class
//  
// 
//   J.R. Gloudemans - 12/10/93
//   Sterling Software
//
//******************************************************************************

#include <string.h>
#include "bicubic_surf.h"

//===== Constructor  =====//
bicubic_surf::bicubic_surf()
{
  num_pnts_u = 0;
  num_pnts_w = 0;
  open_closed_flag_u = OPEN;
  open_closed_flag_w = OPEN;

  twist = vec3d(0.0, 0.0, 0.0);


}

//===== Destructor  =====//
bicubic_surf::~bicubic_surf()
{
}

//===== Init  =====//
void bicubic_surf::init(int num_u, int num_w)
{

  //==== Initialize Array Sizes ====//
  num_pnts_u = num_u;
  num_pnts_w = num_w;
  pnts.init(num_u, num_w);
  tanu.init(num_u, num_w);
  tanw.init(num_u, num_w);

  tanu_k.init(num_u, num_w);
  tanw_k.init(num_u, num_w);
  for ( int i = 0 ; i < num_u ; i++)
    {
      for ( int j = 0 ; j < num_w ; j++)
        {
          tanu_k(i,j) = 0.5;
          tanw_k(i,j) = 0.5;
        }
    }  
}


//===== Read File  =====//
void bicubic_surf::read(FILE* file_id)
{
  float x, y, z;

  char buff[255];

  fgets(buff, 80, file_id);
  fscanf(file_id, "%d",&surface_number); 	fgets(buff, 80, file_id);
  fgets(buff, 80, file_id);
  fscanf(file_id, "%d",&u_render); 		fgets(buff, 80, file_id);
  fscanf(file_id, "%d",&w_render); 		fgets(buff, 80, file_id);
  fscanf(file_id, "%d",&num_pnts_u); 		fgets(buff, 80, file_id);
  fscanf(file_id, "%d",&num_pnts_w); 		fgets(buff, 80, file_id);
  
  this->init(num_pnts_u, num_pnts_w);

  for ( int i = 0 ; i < num_pnts_u ; i++)
    {
      for ( int j = 0 ; j < num_pnts_w ; j++)
        {
          fscanf(file_id, "%f %f %f",&x, &y, &z);  fgets(buff, 80, file_id);
          pnts(i,j) = vec3d(x,y,z);
        }
    }
  this->comp_tans();
            
}


//===== Compute Blending Functions  =====//
void bicubic_surf::blend_funcs(double u, double& F1, double& F2, double& F3, double& F4)
{

  //==== Compute All Blending Functions ====//
  double uu = u*u;
  F1 = 2.0*uu*u - 3.0*uu + 1.0;
  F2 = 1.0 - F1;
  F3 = uu*u - 2.0*uu + u;
  F4 = uu*u - uu;

}

//===== Load A Point  =====//
void bicubic_surf::put_pnt(int ind_u, int ind_w, const vec3d& pnt_in)
{
  pnts(ind_u,ind_w) = pnt_in;
}    

//===== Load A Tangent  =====//
void bicubic_surf::put_tanu(int ind_u, int ind_w, const vec3d& tan_in)
{
  tanu(ind_u,ind_w) = tan_in;
}    

//===== Load A Tangent  =====//
void bicubic_surf::put_tanw(int ind_u, int ind_w, const vec3d& tan_in)
{
  tanw(ind_u,ind_w) = tan_in;
}    

//===== Load A U Tangent Factor  =====//
void bicubic_surf::put_tanu_k(int ind_u, int ind_w, double factor_in)
{
  tanu_k(ind_u,ind_w) = factor_in;
  tanu(ind_u,ind_w)   = tanu(ind_u,ind_w)*factor_in;
}

//===== Load A W Tangent Factor  =====//
void bicubic_surf::put_tanw_k(int ind_u, int ind_w, double factor_in)
{
   tanw_k(ind_u,ind_w) = factor_in;
   tanw(ind_u,ind_w)   = tanw(ind_u,ind_w)*factor_in;
}

//===== Compute A Tangent Given Intermediate Point and U =====//
void bicubic_surf::comp_tans()
{
  int i,j;
  for (  i = 0 ; i < num_pnts_u ; i++)
    {
      if (open_closed_flag_w == OPEN)
        {
          tanw(i,0) = pnts(i,1) - pnts(i,0);
          tanw(i,num_pnts_w-1) = pnts(i,num_pnts_w-1) - pnts(i,num_pnts_w-2);
        }
      else
        {
          tanw(i,0) = pnts(i,1) - pnts(i,num_pnts_w-2);
          tanw(i,num_pnts_w-1) = tanw(i,0);
        }

      for (  j = 1 ; j < num_pnts_w-1 ; j++)
        {
          tanw(i,j) = pnts(i,j+1) - pnts(i,j-1);
        }
    }

  for (  j = 0 ; j < num_pnts_w ; j++)
    {
      if (open_closed_flag_u == OPEN)
        {
          tanu(0,j) = pnts(1,j) - pnts(0,j);
          tanu(num_pnts_u-1,j) = pnts(num_pnts_u-1,j) - pnts(num_pnts_u-2,j);
        }
      else
        {
          tanu(0,j) = pnts(1,j) - pnts(num_pnts_u-2,j);
          tanu(num_pnts_u-1,j) = tanu(0,j);
        }

      for (  i = 1 ; i < num_pnts_u-1 ; i++)
        {
          tanu(i,j) = pnts(i+1,j) - pnts(i-1,j);
        }
    }

  for ( i = 0 ; i < num_pnts_u ; i++)
    {
      for (  j = 0 ; j < num_pnts_w ; j++)
        {
          tanu(i,j) = tanu(i,j)*tanu_k(i,j);
          tanw(i,j) = tanw(i,j)*tanw_k(i,j);
        }
    }  
}    


//===== Compute Point On Curve Given  U =====//
vec3d bicubic_surf::comp_pnt(double u, double w)
{

  double F1u, F2u, F3u, F4u;
  double F1w, F2w, F3w, F4w;

  int u_index = (int)u;
  if (u_index >= num_pnts_u-1) 
    {
      F1u = F3u = F4u = 0.0;
      F2u = 1.0;
      u_index--;
    }
  else
    {
      blend_funcs(u-(double)u_index, F1u, F2u, F3u, F4u);
    }

  int w_index = (int)w;
  if (w_index >= num_pnts_w-1) 
    {
      F1w = F3w = F4w = 0.0;
      F2w = 1.0;
      w_index--;
    }
  else
    {
      blend_funcs(w-(double)w_index, F1w, F2w, F3w, F4w);
    }

   vec3d new_pnt;

   new_pnt = (( pnts(u_index,w_index)*F1u   + pnts(u_index+1,w_index)*F2u +         
               tanu(u_index,w_index)*F3u   + tanu(u_index+1,w_index)*F4u) * F1w)  +
             (( pnts(u_index,w_index+1)*F1u + pnts(u_index+1,w_index+1)*F2u +         
               tanu(u_index,w_index+1)*F3u + tanu(u_index+1,w_index+1)*F4u) * F2w) +
             (( tanw(u_index,w_index)*F1u   + tanw(u_index+1,w_index)*F2u) * F3w)  +         
             (( tanw(u_index,w_index+1)*F1u + tanw(u_index+1,w_index+1)*F2u)* F4w);

  

  return(new_pnt);
}

//===== Dump Pov File =====//
void bicubic_surf::dump_pov_file()
{
  vec3d patch[4][4];

  char file_name[20];
  strcpy(file_name, "bi_cubic_pov.inc");

  /* --- Open file --- */
  FILE* dump_file = fopen(file_name, "w");


  fprintf(dump_file, "#declare Bi_Cubic_Surface =\n");
  fprintf(dump_file, "   union {\n");

  for ( int i = 0 ; i < num_pnts_u-1 ; i++)
    {
      for ( int j = 0 ; j < num_pnts_w-1 ; j++)
        {
          load_bezier_patch(i,j,patch);

          fprintf(dump_file, "     bicubic_patch {\n");
          fprintf(dump_file, "       type 1 \n");
          fprintf(dump_file, "       flatness 0.0 \n");
          fprintf(dump_file, "       u_steps 2 \n");
          fprintf(dump_file, "       w_steps 2 \n");
          for ( int ip = 0 ; ip < 4 ; ip++ )
            {
              fprintf(dump_file, "       ");
              for ( int jp = 0 ; jp < 4 ; jp++ )
                {
                  fprintf(dump_file, "<%f, %f, %f>",
                    patch[ip][jp].x(), patch[ip][jp].y(), patch[ip][jp].z());

                  if (ip != 3 && jp != 3)
                    fprintf(dump_file, ",");
                }
              fprintf(dump_file, "\n");
            }
         }
     }

}

void bicubic_surf::load_bezier_patch(int u_ind, int w_ind, vec3d patch[4][4])
{

}

















