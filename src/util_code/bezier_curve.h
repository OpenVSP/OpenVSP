//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

//******************************************************************************
//    
//   Bezier Curve Class  (Cubic)
//  
// 
//   J.R. Gloudemans - 7/20/94
//   Sterling Software
//
//******************************************************************************


#ifndef BEZIER_CURVE_H
#define BEZIER_CURVE_H

#include "vec3d.h"
#include "sdyn_array.h"

#include <vector>				//jrg windows??	
using namespace std;			//jrg windows??

class Bezier_curve
{
  int num_sections;
  vector <vec3d> pnts;

  void blend_funcs(double u, double& F1, double& F2, double& F3, double& F4);
  void der_blend_funcs(double u, double& F1, double& F2, double& F3, double& F4);


public:

  Bezier_curve();
  ~Bezier_curve();

  void init_num_sections(int num_sections_in);
  int  get_num_sections()						{ return(num_sections); }
  int  get_num_control_pnts()					{ return( num_sections*3 + 1 ); }
  void put_pnt(int ind, const vec3d& pnt_in)	{ pnts[ind] = pnt_in; }
  vec3d& get_pnt(int ind)						{ return(pnts[ind]); }

  vec3d comp_pnt(int sec_num, double u);		// Section and u between 0 and 1
  vec3d comp_tan(int sec_num, double u);

  vec3d comp_pnt( double u );					// u between 0 and 1
 
  double get_length();

  void buildCurve( const vector< vec3d > & pVec, double tanStr, int closeFlag );

  enum { NADA, SHARP, ZERO, ZERO_X, ZERO_Y, ZERO_Z, ONLY_BACK, ONLY_FORWARD, PREDICT, MIRROR_END_CAP, };
  void buildCurve( const vector< vec3d > & pVec, const vector< int > & pFlagVec, double tanStr, int closeFlag );
  void buildCurve2( const vector< vec3d > & pVec, const vector< int > & pFlagVec, double tanStr, int closeFlag );

  void buildCurve( const vector< vec3d > & pVec, const vector< vec3d > & spineVec, 
				   const vector< int > & pFlagVec, const vector< double > & tAngVec, 
	               const vector< double > & tStr1Vec, const vector< double > & tStr2Vec, int closeFlag );
  void buildCurve( const vector< vec3d > & pVec, vec3d & axis, const vector< double > & tAngVec,  
				   const vector< double > & tStr1Vec, const vector< double > & tStr2Vec );

  void setTan( vec3d & tan, const vector< vec3d > & pVec, int ind, int type, int forward, int close  ); 
};

#endif
  
  
