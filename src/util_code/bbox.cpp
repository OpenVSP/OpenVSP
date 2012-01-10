//******************************************************************************
//    
//   Bounding Box Class
//  
// 
//   J.R. Gloudemans - 1/25/93
//   Sterling Software
//
//******************************************************************************

//#include "includes.h"
#include "vec3d.h"
#include "bbox.h"

//===== Constructor: bbox b();  =====//
 bbox::bbox()
{
   min[0] = min[1] = min[2] =  1.0e12;
   max[0] = max[1] = max[2] = -1.0e12;
   set_flag = 0;
}

//===== Constructor: bbox b(minpt, maxpt);  =====//
 bbox::bbox(vec3d& mn, vec3d& mx)
{
   min[0] = mn.x();  min[1] = mn.y();  min[2] = mn.z();
   max[0] = mx.x();  max[1] = mx.y();  max[2] = mx.z();
   set_flag = 1;
}

//==== Equals: x = y ====//
 bbox& bbox::operator=(const bbox& bb)		
{
   if (this == &bb)
	   return *this;

   min[0] = bb.min[0];   min[1] = bb.min[1];   min[2] = bb.min[2];
   max[0] = bb.max[0];   max[1] = bb.max[1];   max[2] = bb.max[2];
   set_flag = bb.set_flag;
   return *this;
}

//====== max[i] = value ======//
 void bbox::set_max(int i, double value)
{
    if ((i >= 0) && (i < 3)) 
      max[i] = value;
    else
      cout << "bbox.set_max: array bounds error\n";
}

//====== min[i] = value ======//
 void bbox::set_min(int i, double value)
{
    if ((i >= 0) && (i < 3)) 
      min[i] = value;
    else
      cout << "bbox.set_min: array bounds error\n";
}

//====== f = max[i] ======//
 double bbox::get_max(int i)
{
    return max[(i >= 0) && (i < 3) ? i : 0];
}

//====== f = min[i] ======//
 double bbox::get_min(int i)
{
    return min[(i >= 0) && (i < 3) ? i : 0];
}

//==== Update Bounding Box - Pnt3f ====//
 void bbox::update( const vec3d& pnt)
{
   if (set_flag)
     {
       if (pnt.x() < min[0]) min[0] = pnt.x();
       else if (pnt.x() > max[0]) max[0] = pnt.x();

       if (pnt.y() < min[1]) min[1] = pnt.y();
       else if (pnt.y() > max[1]) max[1] = pnt.y();

       if (pnt.z() < min[2]) min[2] = pnt.z();
       else if (pnt.z() > max[2]) max[2] = pnt.z();
     }
   else
     {
       min[0] = max[0] = pnt.x();
       min[1] = max[1] = pnt.y();
       min[2] = max[2] = pnt.z();
       set_flag = 1;
     }
}

//==== Update Bounding Box - Bbox ====//
 void bbox::update( const bbox& bb)
{

   if (bb.min[0] < min[0]) min[0] = bb.min[0];
   if (bb.min[1] < min[1]) min[1] = bb.min[1];
   if (bb.min[2] < min[2]) min[2] = bb.min[2];

   if (bb.max[0] > max[0]) max[0] = bb.max[0];
   if (bb.max[1] > max[1]) max[1] = bb.max[1];
   if (bb.max[2] > max[2]) max[2] = bb.max[2];

   set_flag = bb.set_flag;
}


//==== Get Diagonal Dist ====//
double bbox::diag_dist()
{
   vec3d temp = vec3d( max[0] - min[0], max[1] - min[1], max[2] - min[2]);
   return (temp.mag());
}


//==== Get Largest Dimension ====//
double bbox::get_largest_dim()
{
   double del_x = max[0] - min[0];
   double del_y = max[1] - min[1];
   double del_z = max[2] - min[2];

   if ( del_x > del_y && del_x > del_z ) 
     return( del_x );
   else if ( del_y > del_z )
     return( del_y );
   else
     return( del_z );
}

//==== Get Estimated Area ====//
double bbox::get_est_area()
{
   double del_x = max[0] - min[0];
   double del_y = max[1] - min[1];
   double del_z = max[2] - min[2];

   if ( del_x >= del_z && del_y >= del_z ) 
     return( del_x*del_y );
   else if ( del_x >= del_y && del_z >= del_y )
     return( del_x*del_z );
   else
     return( del_y*del_z );
}


//==== Get Center f Bounding Box ====//
vec3d bbox::get_center()
{ 
  return( vec3d( (min[0] + max[0])*0.5, 
                 (min[1] + max[1])*0.5, 
                 (min[2] + max[2])*0.5 )); 
}

vec3d  bbox::get_pnt( int ind )
{
	switch ( ind )
	{
		case 0:
			return ( vec3d ( min[0], min[1], min[2] ) );

		case 1:
			return ( vec3d ( max[0], min[1], min[2] ) );

		case 2:
			return ( vec3d ( min[0], max[1], min[2] ) );

		case 3:
			return ( vec3d ( max[0], max[1], min[2] ) );

		case 4:
			return ( vec3d ( min[0], min[1], max[2] ) );

		case 5:
			return ( vec3d ( max[0], min[1], max[2] ) );

		case 6:
			return ( vec3d ( min[0], max[1], max[2] ) );

		default:
			return ( vec3d ( max[0], max[1], max[2] ) );
	}
	
	return vec3d( max[0], max[1], max[2] );
}


//==== Expland Bounding Box ====//
void bbox::expand( double val )
{
	for ( int i = 0 ; i < 3 ; i++ )
	{
		min[i] -= val;
		max[i] += val;
	}
}

//==== Scale Bounding Box ====//
void bbox::scale( vec3d & scale_xyz )
{
	vec3d center = get_center();
	for ( int i = 0 ; i < 3 ; i++ )
	{
		min[i] = center[i] + (min[i] - center[i])*scale_xyz[i];
		max[i] = center[i] + (max[i] - center[i])*scale_xyz[i];
	}
}

//==== Compare Bounding Box ====//
int compare(bbox& bb1, bbox& bb2)
{
   if ((bb2.min[0]-bb1.max[0]) > BBOX_TOL) return(0);
   if ((bb1.min[0]-bb2.max[0]) > BBOX_TOL) return(0);
   if ((bb2.min[1]-bb1.max[1]) > BBOX_TOL) return(0);
   if ((bb1.min[1]-bb2.max[1]) > BBOX_TOL) return(0);
   if ((bb2.min[2]-bb1.max[2]) > BBOX_TOL) return(0);
   if ((bb1.min[2]-bb2.max[2]) > BBOX_TOL) return(0);
   return(1);
}

//==== Compare Bounding Box ====//
int bbox::check_pnt(double x, double y, double z)
{
	if ( x < min[0] ) return 0;
	if ( x > max[0] ) return 0;
	if ( y < min[1] ) return 0;
	if ( y > max[1] ) return 0;
	if ( z < min[2] ) return 0;
	if ( z > max[2] ) return 0;

	return 1;
}


