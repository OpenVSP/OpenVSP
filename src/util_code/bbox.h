//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

//******************************************************************************
//    
//   Bounding Box Class
//  
// 
//   J.R. Gloudemans - 1/25/93
//   Sterling Software
//
//******************************************************************************
#include "defines.h"
#ifndef BBOX_H
#define BBOX_H

#define BBOX_TOL 1.0e-06

class vec3d;

class VSPDLL bbox
{
   double min[3];
   double max[3];
   int set_flag;

public:

   bbox();
   ~bbox()  {}			// Destructor
   bbox(vec3d& mn, vec3d& mx);
   bbox& operator=(const bbox& bb);
   void   init()			{ set_flag = 0; }
   void   set_max(int i, double value);
   void   set_min(int i, double value);
   double get_max(int i);
   double get_min(int i);
   void update( const vec3d& pnt);
   void update( const bbox& bb);
   double diag_dist();
   double get_largest_dim();
   double get_est_area();
   void   expand( double val );
   void   scale( vec3d & scale_xyz );
   vec3d get_center();
   vec3d get_pnt( int i );
   int check_pnt(double x, double y, double z);
   friend int compare(bbox& bb1, bbox& bb2);
};

#endif
















