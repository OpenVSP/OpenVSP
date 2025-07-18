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

#ifndef VSPVEC2D_H
#define VSPVEC2D_H

#include <iostream>
#include <vector>

class vec2d;

//==== Forward declare to get around MS Compiler bug ====//
vec2d operator+( const vec2d& a, const vec2d& b );
vec2d operator-( const vec2d& a, const vec2d& b );
vec2d operator*( const vec2d& a, double b );
vec2d operator*( const vec2d& a, const vec2d& b );
vec2d operator/( const vec2d& a, double b );


class vec2d
{
private:

public:
    double v[2];

    vec2d();           // vec2d x or new vec2d

    ~vec2d()  {}        // delete vec2d

    vec2d( double xx, double yy );

    vec2d( const vec2d& a ); // vec2d x = y

    vec2d& operator=( const vec2d& a ); // x = y
    vec2d& operator=( double a );      // x = 35.

    double& operator [] ( int i )
    {
        return v[i];
    }
    const double& operator [] ( int i ) const
    {
        return v[i];
    }

    // Set Point Values
    vec2d& set_xy( double xx, double yy );
    vec2d& set_x( double xx );
    vec2d& set_y( double yy );

    // Get Point Values
    double x() const;
    double y() const;
    double* data()
    {
        return( v );
    }

    // Transform Matrix
    vec2d transform( float mat[3][3] );
    vec2d transform( double mat[3][3] );

    // x = a + b, x = a - b, a*scale, a/scale
    friend vec2d operator+( const vec2d& a, const vec2d& b );
    friend vec2d operator-( const vec2d& a, const vec2d& b );
    friend vec2d operator*( const vec2d& a, double b );
    friend vec2d operator*( double b, const vec2d& a );
    friend vec2d operator*( const vec2d& a, const vec2d& b );
    friend vec2d operator/( const vec2d& a, double b );
    vec2d& operator+=( const vec2d& b );
    vec2d& operator-=( const vec2d& b );
    vec2d& operator*=( double b );

    friend double dist( const vec2d& a, const vec2d& b );
    friend double dist_squared( const vec2d& a, const vec2d& b );

    double mag() const;                // x = a.mag()
    void normalize();           // a.normalize()

    friend double cross( const vec2d& a, const vec2d& b );
    friend double dot( const vec2d& a, const vec2d& b ); // x = dot(a,b)
    friend double angle( const vec2d& a, const vec2d& b );
    friend double cos_angle( const vec2d& a, const vec2d& b );

    friend int seg_seg_intersect( const vec2d& pnt_A, const vec2d& pnt_B, const vec2d& pnt_C, const vec2d& pnt_D, vec2d& int_pnt, double &t1, double &t2 );
    friend vec2d proj_pnt_on_line_seg( const vec2d& line_A, const vec2d& line_B, const vec2d& pnt );
    friend double proj_pnt_on_line_u( const vec2d& line_A, const vec2d& line_B, const vec2d& pnt );

    friend void encode( double x_min, double y_min, double x_max, double y_max,
                        const vec2d& pnt, int code[4] );
    friend void clip_seg_rect( double x_min, double y_min, double x_max, double y_max,
                               vec2d& pnt1, vec2d& pnt2, int& visible );

    friend bool PointInPolygon( const vec2d & R, const std::vector< vec2d > & pnts );
    friend double det( const vec2d & p0, const vec2d & p1, const vec2d & offset );
    friend double poly_area( const std::vector< vec2d > & pnt_vec );
    friend vec2d poly_centroid( const std::vector< vec2d > & pnt_vec );

    friend double orient2d( const vec2d & p0, const vec2d & p1, const vec2d & p );
    friend void bi_lin_interp( const vec2d &p0, const vec2d &p1, const vec2d &p2, vec2d const &p3, double s, double t, vec2d &p );
    friend int inverse_bi_lin_interp( const vec2d &p0, const vec2d &p1, const vec2d &p2, vec2d const &p3, const vec2d &p, double &s, double &t, double &s2, double &t2 );
};

#endif



