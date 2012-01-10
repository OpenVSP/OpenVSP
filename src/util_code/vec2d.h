//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

//******************************************************************************
//    
//   2D Point Double Class
//   
//   Paul C. Davis & J.R. Gloudemans
//   Date - 6/13/94
//
//******************************************************************************

#ifndef VEC2D_H
#define VEC2D_H

#define FALSE 0
#define TRUE 1

#include <iostream>
using namespace std;

class vec2d;

//==== Forward declare to get around MS Compiler bug ====//
vec2d operator+(const vec2d& a, const vec2d& b);
vec2d operator-(const vec2d& a, const vec2d& b);
vec2d operator*(const vec2d& a, double b);
vec2d operator*(const vec2d& a, const vec2d& b);
vec2d operator/(const vec2d& a, double b); 


class vec2d
{
private:

public:
    double v[2];
    friend void encode(double x_min, double y_min, double x_max, double y_max, 
            const vec2d& pnt, int code[4]);

     vec2d();			// vec2d x or new vec2d

    ~vec2d()  {}		// delete vec2d

     vec2d(double xx, double yy);

     vec2d(const vec2d& a);	// vec2d x = y

     vec2d& operator=(const vec2d& a);	// x = y
     vec2d& operator=(double a);		// x = 35.

	double& operator [] (int i)					{ return v[i]; }
	const double& operator [] (int i) const		{ return v[i]; }

    // Set Point Values
     vec2d& set_xy(double xx, double yy);
     vec2d& set_x(double xx);
     vec2d& set_y(double yy);

    // Get Point Values
     double* get_pnt()		{ return( v ); }
     double x();
     double y();
	 double* data()				{ return( v ); }


    // Transform Matrix
    vec2d transform(float mat[3][3]);
    vec2d transform(double mat[3][3]);

    // x = a + b, x = a - b, a*scale, a/scale
    friend vec2d operator+(const vec2d& a, const vec2d& b);
    friend vec2d operator-(const vec2d& a, const vec2d& b);
    friend vec2d operator*(const vec2d& a, double b);
    friend vec2d operator*(const vec2d& a, const vec2d& b);
    friend vec2d operator/(const vec2d& a, double b);

    // cout << a			
//    friend ostream& operator<< (ostream& out, const vec2d& a);
  
    friend double dist(const vec2d& a, const vec2d& b);
    friend double dist_squared(const vec2d& a, const vec2d& b);

    double mag() const;                // x = a.mag()
    void normalize();           // a.normalize()
 
    friend double dot(const vec2d& a, const vec2d& b);   // x = dot(a,b)
    friend double angle(const vec2d& a, const vec2d& b); 
    friend double cos_angle(const vec2d& a, const vec2d& b);
//    friend void line_line_intersect(vec2d& line1_pnt1, vec2d& line1_pnt2, 
//                   vec2d& line2_pnt1, vec2d& line2_pnt2, int intersected, vec2d& int_pnt);
//    friend void line_seg_intersect(vec2d& line1_pnt1, vec2d& line1_pnt2, 
//                   vec2d& line2_pnt1, vec2d& line2_pnt2, int intersected, vec2d& int_pnt);
    friend int seg_seg_intersect(vec2d& pnt_A, vec2d& pnt_B, vec2d& pnt_C, vec2d& pnt_D, vec2d& int_pnt);
    friend void clip_seg_rect(double x_min, double y_min, double x_max, double y_max, 
                   vec2d& pnt1, vec2d& pnt2, int& visable);
    friend vec2d proj_pnt_on_line_seg(const vec2d& line_A, const vec2d& line_B, const vec2d& pnt);
 
	friend double proj_pnt_on_line_u(const vec2d& line_A, const vec2d& line_B, const vec2d& pnt);

};

#endif


		
