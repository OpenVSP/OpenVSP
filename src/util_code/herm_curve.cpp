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

#include <math.h>
#include <float.h>  // for DBL_EPSILON
#include "herm_curve.h"

//===== Default Constructor =====//
herm_curve::herm_curve() : num_pnts(0), open_closed_flag(OPEN_CURVE)
{
  curr_flags.init(NUM_FLAGS);
  curr_flags.turn_all_on();

}

//===== Copy Constructor =====//
herm_curve::herm_curve(const herm_curve& curve) : num_pnts(curve.num_pnts),
open_closed_flag(curve.open_closed_flag)
{
  pnts.init(num_pnts);
  tans.init(num_pnts);
  tan_k.init(num_pnts);
  for (int i = 0; i < num_pnts; i++)
  {
    pnts[i] = curve.pnts[i];
    tans[i] = curve.tans[i];
    tan_k[i] = curve.tan_k[i];
  }

  curr_flags.turn_all_off();
}


//===== Destructor =====//
herm_curve::~herm_curve()
{
}

//===== Equals Operator =====//
herm_curve& herm_curve::operator=(const herm_curve& curve)
{
  if (this == &curve)
      return *this;

  num_pnts = curve.num_pnts;
  pnts.init(num_pnts);
  tans.init(num_pnts);
  tan_k.init(num_pnts);
  for (int i = 0; i < num_pnts; i++)
  {
    pnts[i] = curve.pnts[i];
    tans[i] = curve.tans[i];
    tan_k[i] = curve.tan_k[i];
  }
  open_closed_flag = curve.open_closed_flag;

  curr_flags.turn_all_off();

  return *this;
}

//===== Init =====//
void herm_curve::init(int num_cntrl_pnts)
{
  curr_flags.turn_all_off();

  //==== Initialize Array Sizes ====//
  num_pnts = num_cntrl_pnts;
  pnts.init(num_pnts);
  tans.init(num_pnts);
  tan_k.set_chunk_size(num_pnts);
  tan_k.init(num_pnts);

  for ( int i = 0 ; i < num_pnts ; i++)
    {
      tan_k(i) = 1.0;
    }

}

//===== Compute Blending Functions =====//
void herm_curve::blend_funcs(float u, float& F1, float& F2, float& F3, float& F4)
{

  //==== Compute All Blending Functions ====//
  float uu = u*u;
  F1 = 2.0f*uu*u - 3.0f*uu + 1.0f;
  F2 = 1.0f - F1;
  F3 = uu*u - 2.0f*uu + u;
  F4 = uu*u - uu;

}


//===== Compute Derivative of Blending Functions  =====//
void herm_curve::deriv_blend_funcs(float u, float& F1, float& F2, float& F3, float& F4)
{

  //==== Compute All Blending Functions ====//
  float uu = u*u;
  F1 = 6.0f*(uu - u);
  F2 = -F1;
  F3 = 3.0f*uu - 4*u + 1.0f;
  F4 = 3.0f*uu - 2.0f*u;

}

//===== Load A Point  =====//
void herm_curve::load_pnt(int index, const vec3d& pnt_in)
{
  curr_flags.turn_all_off();

  if (index >= 0 && index < num_pnts)
    {
      pnts[index] = pnt_in;
    }
  else
    cout << "ERROR - herm_curve.load_pnt" << endl;
}

//===== Load A Tangent  =====//
void herm_curve::load_tan(int index, const vec3d& tan_in)
{
  curr_flags.turn_all_off();

  if (index >= 0 && index < num_pnts)
    {
      tans[index] = tan_in;
    }
  else
    cout << "ERROR - herm_curve.load_tan" << endl;
}

//===== Compute A Tangent Given Intermediate Point and U =====//
void herm_curve::comp_tan(int ind, float u, const vec3d& pnt_in)
{
  curr_flags.turn_all_off();

  if (ind >= 1 && ind < num_pnts)
    {
      float F1, F2, F3, F4;
      blend_funcs(u, F1, F2, F3, F4);

      tans[ind] = (pnt_in - pnts[ind-1]*F1 - pnts[ind]*F2 - tans[ind-1]*F3)*(1.0/F4);
    }
  else
    cout << "ERROR - herm_curve.load_tan" << endl;
}

//===== Compute Tangents Using Forward and Reverse Difference Method =====//
void herm_curve::comp_tans()
{
  int i = 0;
  float temp_dist1 = 0.0, temp_dist2 = 0.0;
  Dyn_array_dbl dist_mag;

  dist_mag.set_chunk_size(num_pnts);
  dist_mag.init(num_pnts);

  for (  i = 1 ; i < num_pnts-1 ; i++)
    {
      temp_dist1  = (float) (pnts(i) - pnts(i+1)).mag();
      temp_dist2  = (float) (pnts(i-1) - pnts(i)).mag();
      if ( temp_dist1 < temp_dist2)
        dist_mag(i) = temp_dist1;
      else
        dist_mag(i) = temp_dist2;
    }
  temp_dist1 = (float) (pnts(1) - pnts(0)).mag();
  temp_dist2 = (float) (pnts(num_pnts-2) - pnts(0)).mag();

  if ( temp_dist1 < temp_dist2)
    dist_mag(0) = temp_dist1;
  else
    dist_mag(0) = temp_dist2;
  dist_mag(num_pnts-1) = dist_mag(0);


  if (open_closed_flag == OPEN_CURVE)
    {
      tans(0) = pnts(1) - pnts(0);
      tans(num_pnts-1) = pnts(num_pnts-1) - pnts(num_pnts-2);
    }
  else
    {
      tans(0) = pnts(1) - pnts(num_pnts-2);
      tans(0).normalize();
      tans(0) = tans(0) * dist_mag(0);
      tans(num_pnts-1) = tans(0);
    }

  for ( i = 1 ; i < num_pnts-1 ; i++)
    {
      tans(i) = pnts(i+1) - pnts(i-1);
      tans(i).normalize();
      tans(i) = tans(i) * dist_mag(i);
    }

  for ( i = 0 ; i < num_pnts ; i++)
    {
      tans(i) = tans(i)*tan_k(i);
    }
}


//===== Compute Length If Curve Has Changed =====//
float herm_curve::get_length()
{

  if (curr_flags.is_on(LENGTH_FLAG)) return( length );

  curr_flags.turn_on(LENGTH_FLAG);

  if (curr_flags.is_on(SAMPLE_FLAG))
    {
      length = this->sum_sample_length();
      return(length);
    }
  else
    {
      length = this->compute_length();
      return(length);
    }
}

//===== Compute Length By Summing Sampled Curve =====//
float herm_curve::sum_sample_length()
{
  length = 0.0;
  for (int i = 0 ; i < samp_pnts.dimension()-1 ; i++)
    {
      length += (float) dist(samp_pnts[i], samp_pnts[i+1]);
    }

  return(length);
}

//===== Compute Length By Summing Sampled Curve =====//
float herm_curve::compute_length()
{
  float u;
  float F1, F2, F3, F4;
  float G1, G2, G3, G4;
  vec3d pnt1, pnt2;

  length = 0.0;
  for ( int i = 0 ; i < NUM_SAMPLES-1 ; i++ )
    {
      u = (float) i / (float) (NUM_SAMPLES-1);
      blend_funcs(u, F1, F2, F3, F4);
      u = (float) (i+1) / (float) (NUM_SAMPLES-1);
      blend_funcs(u, G1, G2, G3, G4);

      for ( int j = 0 ; j < num_pnts-1 ; j++ )
        {
          pnt1 = pnts[j]*F1 + pnts[j+1]*F2 + tans[j]*F3 + tans[j+1]*F4;

          pnt2 = pnts[j]*G1 + pnts[j+1]*G2 + tans[j]*G3 + tans[j+1]*G4;

          length += (float) dist(pnt1, pnt2);
        }
    }
  return(length);
}

//===== Sample And Store Points Along Curve =====//
void herm_curve::load_sample_pnts()
{
  if (curr_flags.is_on(SAMPLE_FLAG)) return;

  curr_flags.turn_on(SAMPLE_FLAG);

  int index;
  float u, F1, F2, F3, F4;

  int tot_num_sample_pnts = (num_pnts-1)*(NUM_SAMPLES) + 1;

  samp_pnts.init(tot_num_sample_pnts);

  //==== Fill Intermediate Points ====//
  for ( int i = 1 ; i < NUM_SAMPLES ; i++ )
    {
      u = (float) i / (float) (NUM_SAMPLES);
      blend_funcs(u, F1, F2, F3, F4);

      for ( int j = 0 ; j < num_pnts-1 ; j++ )
        {
          index = j*NUM_SAMPLES + i;
          samp_pnts[index] = pnts[j]*F1 + pnts[j+1]*F2 + tans[j]*F3 + tans[j+1]*F4;
        }
     }

   //==== Fill Interpolated Points ====//
   for ( int j = 0 ; j < num_pnts ; j++ )
     {
       index = j*NUM_SAMPLES;
       samp_pnts[index] = pnts[j];
     }
}

//===== Compute Point Given Percentage Length Along Axis =====//
vec3d herm_curve::comp_pnt_per_xyz(int xyz_id, double per)
{
  vec3d new_pnt;
  float u = 0.0;

  if ( per <= 0.0 )
    new_pnt = pnts[0];
  else if ( per >= 1.0 )
    new_pnt = pnts[num_pnts-1];
  else
    {
      this->load_u_func_length_xyz(xyz_id);

      //==== Find U Interval For Given Length Percentage ====//
      int guess = (int) ( per*(num_pnts-1) );
      int low_ind = u_func_xyz.find_interval(per,guess);

      if ( low_ind < 0)
        low_ind = 0;
      else if (low_ind > u_func_xyz.dimension()-2)
        low_ind = u_func_xyz.dimension()-2;

      float fract = ((float) (per - u_func_xyz[low_ind]) )/
                     ( (float) (u_func_xyz[low_ind+1] - u_func_xyz[low_ind]) );
      u = ((float)low_ind + fract)/(float)NUM_SAMPLES;

      new_pnt = this->comp_pnt(u);
    }
  return(new_pnt);

}

//===== Compute U as a Function Of Percentage Length =====//
void herm_curve::load_u_func_length()
{
  //==== Check if U as a Function of Length is Current ====//
  if (curr_flags.is_on(U_FUNC_LENGTH)) return;

  //==== Turn On Current Flag ====//
  curr_flags.turn_on(U_FUNC_LENGTH);

  this->load_sample_pnts();
  int num_samp_pnts = samp_pnts.dimension();

  u_func_length.set_chunk_size(num_samp_pnts);
  u_func_length.init(num_samp_pnts);

  u_func_length[0] = 0.0;

  length = 0.0;
  for (int i = 0 ; i < u_func_length.dimension()-1 ; i++)
    {
      length += (float) dist(samp_pnts[i], samp_pnts[i+1]);
      u_func_length[i+1] = length;
    }

  if (length > 0.0)
    {
      for (int i = 1 ; i < u_func_length.dimension() ; i++)
        {
          u_func_length[i] /= length;
        }
    }
  else
    {
      for (int i = 1 ; i < u_func_length.dimension() ; i++)
        {
          u_func_length[i] = (float)i/(float)(u_func_length.dimension()-1);
        }
    }
}

//===== Compute Point On Curve Given  U =====//
vec3d herm_curve::comp_pnt(float u)
{
//  if ( u <= 0.0 ) return(pnts[0]);

//  if ( u >= 1.0 ) return(pnts[num_pnts-1]);

  vec3d new_pnt;

//  u = u*(float)(num_pnts-1);

  if ( u >= 0.0 && u < num_pnts-1 )
    {
      int index = (int)u;

      float new_u = u - (float)index;

      float F1, F2, F3, F4;
      blend_funcs(new_u, F1, F2, F3, F4);

      new_pnt = pnts[index]*F1 + pnts[index+1]*F2 +
                tans[index]*F3 + tans[index+1]*F4;
    }
//  else if ( u == (float)num_pnts-1)
  else if( fabs(u - ((float)num_pnts-1)) <= DBL_EPSILON)
    {
      new_pnt = pnts[num_pnts-1];
    }
  else
    cout << "ERROR - herm_curve.comp_pnt: u = " << u << endl;

  return(new_pnt);
}


//===== Compute Point Given  Percentage Length =====//
vec3d herm_curve::comp_pnt_per_length(float per)
{
	if (pnts.dimension() == 0 )
		return vec3d();

  vec3d new_pnt;
  float u = 0.0;

  if ( per <= 0.0 )
    new_pnt = pnts[0];
  else if ( per >= 1.0 )
    new_pnt = pnts[num_pnts-1];
  else
    {
      this->load_u_func_length();

      //==== Find U Interval For Given Length Percentage ====//
      int guess = (int) ( per*(num_pnts-1) );
      int low_ind = u_func_length.find_interval(per,guess);

      if ( low_ind < 0)
        low_ind = 0;
      else if (low_ind > u_func_length.dimension()-2)
        low_ind = u_func_length.dimension()-2;

      float fract = ( (float) (per - u_func_length[low_ind]) )/
                    ( (float) (u_func_length[low_ind+1] - u_func_length[low_ind]) );
      u = ((float)low_ind + fract)/(float)NUM_SAMPLES;

      new_pnt = this->comp_pnt(u);
    }
  return(new_pnt);

}

//===== Compute U as a Function Of Percentage Length Along Axis=====//
void herm_curve::load_u_func_length_xyz(int xyz_id)
{
  //==== Check if U as a Function of Length is Current ====//
  if (curr_flags.is_on(xyz_id + 3)) return;

  //==== Turn Off All Flags ====//
  curr_flags.turn_off(U_FUNC_X);
  curr_flags.turn_off(U_FUNC_Y);
  curr_flags.turn_off(U_FUNC_Z);

  //==== Turn On Current Flag ====//
  curr_flags.turn_on(xyz_id + 3);

  this->load_sample_pnts();
  int num_samp_pnts = samp_pnts.dimension();

  u_func_xyz.set_chunk_size(num_samp_pnts);
  u_func_xyz.init(num_samp_pnts);

  u_func_xyz[0] = 0.0;

  float len_xyz = 0.0;
  for (int i = 0 ; i < u_func_xyz.dimension()-1 ; i++)
    {
      if ( xyz_id == X_AXIS )
        len_xyz += (float) fabs(samp_pnts[i].x() - samp_pnts[i+1].x());
      else if ( xyz_id == Y_AXIS )
        len_xyz += (float) fabs(samp_pnts[i].y() - samp_pnts[i+1].y());
      else if ( xyz_id == Z_AXIS )
        len_xyz += (float) fabs(samp_pnts[i].z() - samp_pnts[i+1].z());

      u_func_xyz[i+1] = len_xyz;
    }

  if (len_xyz > 0.0)
    {
      for (int i = 1 ; i < u_func_xyz.dimension() ; i++)
        {
          u_func_xyz[i] /= len_xyz;
        }
    }
  else
    {
      for (int i = 1 ; i < u_func_xyz.dimension() ; i++)
        {
          u_func_xyz[i] = (float)i/(float)(u_func_xyz.dimension()-1);
        }
    }
}

//===== Get Integral via Trapazoid Rule Note: Assumes Planar Curve =====//
// function = f(variable),  i.e   y = f(x)
float herm_curve::get_integral(int func_id, int var_id, int num_divs)
{
  float area = 0.0;
  float delta = 0.0;
  vec3d f_last;
  vec3d f_curr;

  //===== Check The Function & Variable Id's =====//
  if (func_id == var_id)
    cout << "ERROR - herm_curve.comp_integral: func_id = var_id" << endl;
  else if (func_id < X_AXIS || func_id > Z_AXIS)
    cout << "ERROR - herm_curve.comp_integral: out of bounds func_id = " << func_id << endl;
  else if (var_id < X_AXIS || var_id > Z_AXIS)
    cout << "ERROR - herm_curve.comp_integral: out of bounds var_id = " << var_id << endl;

  else
  {
    //===== Check Number of Divisions =====//
    if (num_divs < 0) num_divs = 1;

    f_last = comp_pnt(0.0);
    for (int i = 1; i < num_divs; i++)
    {
      float u = (float) ( i * (num_pnts-1) )/ (float) num_divs ;
      f_curr = comp_pnt(u);

      //===== Determine Variable Spacing =====//
      if (var_id == X_AXIS)
        delta = (float) ( f_curr.x() - f_last.x());
      else if (var_id == Y_AXIS)
        delta = (float) ( f_curr.y() - f_last.y());
      else if (var_id == Z_AXIS)
        delta = (float) ( f_curr.z() - f_last.z());

      //===== Sum Areas of Trapazoids =====//
      if (func_id == X_AXIS)
        area += (float) (.5 * delta * ( f_last.x() + f_curr.x() ));
      else if (func_id == Y_AXIS)
        area += (float) (.5 * delta * ( f_last.y() + f_curr.y() ));
      else if (func_id == Z_AXIS)
        area += (float) (.5 * delta * ( f_last.z() + f_curr.z() ));

      f_last = f_curr;
    }
  }
  return(area);

}

//===== Get Derivative =====//
vec3d herm_curve::get_derivative(float u)
{
//  if ( u <= 0.0 ) return(pnts[0]);

//  if ( u >= 1.0 ) return(pnts[num_pnts-1]);

  vec3d new_pnt;

//  u = u*(float)(num_pnts-1);

  if ( u >= 0.0 && u < num_pnts-1 )
    {
      int index = (int)u;

      float new_u = u - (float)index;

      float F1, F2, F3, F4;
      deriv_blend_funcs(new_u, F1, F2, F3, F4);

      new_pnt = pnts[index]*F1 + pnts[index+1]*F2 +
                tans[index]*F3 + tans[index+1]*F4;
    }
  //else if ( u == (float)num_pnts-1)
  else if ( fabs(u - ((float)num_pnts-1)) <= DBL_EPSILON)
    {
      new_pnt = pnts[num_pnts-1];
    }
  else
    cout << "ERROR - herm_curve.get_deriv: u = " << u << endl;

  return(new_pnt);
}
