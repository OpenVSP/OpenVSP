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

#include <cmath>
#include <cfloat>  //For DBL_EPSILON
#include "Vec2d.h"

//****** Constructor:  vec2d x() ******//
vec2d::vec2d()
{
    v[0] = v[1] = 0.0;
}

//****** Constructor:  vec2d x(1.,0.) ******//
vec2d::vec2d( double xx, double yy )
{
    v[0] = xx;
    v[1] = yy;
}

//******* vec2d x = y ******//
vec2d::vec2d( const vec2d& a )
{
    v[0] = a.v[0];
    v[1] = a.v[1];
}

//****** Equals:  x = y ******
vec2d& vec2d::operator=( const vec2d& a )
{
    if ( this == &a )
    {
        return *this;
    }

    v[0] = a.v[0];
    v[1] = a.v[1];
    return *this;
}

//******* x = 35. ******//
vec2d& vec2d::operator=( double a )
{
    v[0] = v[1] = a;
    return *this;
}

vec2d& vec2d::operator=( const twod_point_type &a )
{
    v[0] = a.x();
    v[1] = a.y();
    return *this;
}

//******* Set Point Values *******//
vec2d& vec2d::set_xy( double xx, double yy )
{
    v[0] = xx;
    v[1] = yy;
    return *this;
}

//******* Set Point Values *******//
vec2d& vec2d::set_x( double xx )
{
    v[0] = xx;
    return *this;
}

//******* Set Point Values *******//
vec2d& vec2d::set_y( double yy )
{
    v[1] = yy;
    return *this;
}

//******* Get Point Values *******//
double vec2d::x() const
{
    return v[0];
}

//******* Get Point Values *******//
double vec2d::y() const
{
    return v[1];
}

//******* Transform *******//
vec2d vec2d::transform( float mat[3][3] )
{
    return( vec2d( ( mat[0][0] * v[0] + mat[1][0] * v[1] + mat[2][0] ),
                   ( mat[0][1] * v[0] + mat[1][1] * v[1] + mat[2][1] ) ) );

}

//******* Transform *******//
vec2d vec2d::transform( double mat[3][3] )
{
    return( vec2d( ( mat[0][0] * v[0] + mat[1][0] * v[1] + mat[2][0] ),
                   ( mat[0][1] * v[0] + mat[1][1] * v[1] + mat[2][1] ) ) );

}

//************* x = a + b ******//
vec2d operator+( const vec2d& a, const vec2d& b )
{
    vec2d  ret( a.v[0] + b.v[0],  a.v[1] + b.v[1] );
    return ret;
}

//************* x = a - b ******//
vec2d operator-( const vec2d& a, const vec2d& b )
{
    vec2d  ret( a.v[0] - b.v[0],  a.v[1] - b.v[1] );
    return ret;
}

//******* x = a * b ******//
vec2d operator*( const vec2d& a, double b )
{
    vec2d ret( a.v[0] * b, a.v[1] * b );
    return ret;
}

//******* x = a * b ******//
vec2d operator*( const vec2d& a, const vec2d& b )
{
    vec2d ret( a.v[0] * b.v[0], a.v[1] * b.v[1] );
    return ret;
}

//******* x = a / b ******//
vec2d operator/( const vec2d& a, double b )
{
    vec2d ret;
    //if (b != 0.0)
    if ( b > DBL_EPSILON || b < 0.0 )
    {
        ret.set_xy( a.v[0] / b, a.v[1] / b );
    }
    else
    {
        ret.set_xy( 0.0, 0.0 );
    }

    return ret;
}


//******* cout << a ******//
//ostream& operator<< (ostream& out, const vec2d& a)
//{
//    return ( out << "  " << a.v[0] << "  " <<
//       a.v[1] << "  " ) ;
//}

//******* distance between pnts ******//
double dist( const vec2d& a, const vec2d& b )
{
    double xx = a.v[0] - b.v[0];
    double yy = a.v[1] - b.v[1];
    return ( sqrt( xx * xx + yy * yy ) );
}

//******* distance between pnts squared ******//
double dist_squared( const vec2d& a, const vec2d& b )
{
    double xx = a.v[0] - b.v[0];
    double yy = a.v[1] - b.v[1];
    return ( xx * xx + yy * yy );
}

//******* Magnitude:   x = a.mag() ******//
double vec2d::mag() const
{
    return( sqrt( v[0] * v[0] + v[1] * v[1] ) );
}

//****** Normalize:   a.normalize()  ******//
void vec2d::normalize()
{
    double length = sqrt( v[0] * v[0] + v[1] * v[1] );
    if ( length <= 0.0 )
    {
        v[0] = 0.0;
        v[1] = 0.0;
    }
    else
    {
        v[0] /= length;
        v[1] /= length;
    }
}

//******* Dot Product:  x = a.dot(b) ******//
double dot( const vec2d& a, const vec2d& b )
{
    return ( a.v[0] * b.v[0] + a.v[1] * b.v[1] ) ;
}

//******* Angle Between Vectors ******//
double angle( const vec2d& a, const vec2d& b )
{
    double angle = dot( a, b ) / ( a.mag() * b.mag() );

    if ( angle >= -1.0 && angle <= 1.0 )
    {
        return( acos( angle ) );
    }
    else
    {
        return( 0.0 );
    }
}


//******* Cosine of Angle Between Vectors ******//
double cos_angle( const vec2d& a, const vec2d& b )
{
    double angle = dot( a, b ) / ( a.mag() * b.mag() );

    if ( angle >= -1.0 && angle <= 1.0 )
    {
        return( angle );
    }
    else
    {
        return( 0.0 );
    }
}

//******* Encode Segment Points To Rectangle  Boundaries ******//
// Adapted From : Hill, "Computer Graphics", Macmillan Pub., New York, 1990. //
void encode( double x_min, double y_min, double x_max, double y_max, const vec2d& pnt, int code[4] )
{
    code[0] = code[1] = code[2] = code[3] = 0;

    if ( pnt.v[0] < x_min )
    {
        code[0] = 1;    // Left Bound
    }
    if ( pnt.v[0] > x_max )
    {
        code[1] = 1;    // Right Bound
    }
    if ( pnt.v[1] < y_min )
    {
        code[2] = 1;    // Bottom Bound
    }
    if ( pnt.v[1] > y_max )
    {
        code[3] = 1;    // Top Bound
    }
}

//******* Clip Line Segment To Rectangle ******//
// Adapted From : Hill, "Computer Graphics", Macmillan Pub., New York, 1990. //
void clip_seg_rect( double x_min, double y_min, double x_max, double y_max,
                    vec2d& pnt1, vec2d& pnt2, int& visible )
{
    int c1[4];
    int c2[4];
    int tmp_c[4];
    vec2d tmp_pnt;
    double m = 0.0;
    int done = 0;
    int swap = 0;

    while ( !done )
    {
        encode( x_min, y_min, x_max, y_max, pnt1, c1 );
        encode( x_min, y_min, x_max, y_max, pnt2, c2 );

        // Segment Lies Entirely Inside Of Rectangle //
        if ( ( !c1[0] && !c1[1] && !c1[2] && !c1[3] ) && ( !c2[0] && !c2[1] && !c2[2] && !c2[3] ) )
        {
            visible = 1;
            done = 1;
        }

        // Segment Lies Entirely Outside Of Rectangle //
        else if ( ( c1[0] && c2[0] ) || ( c1[1] && c2[1] ) || ( c1[2] && c2[2] ) || ( c1[3] && c2[3] ) )
        {
            visible = 0;
            done = 1;
        }

        // Segment Has At Least One Point Outside Of Rectangle //
        else
        {
            // Insure pnt1 is Not Outside By Swapping Points //
            if ( !c1[0] && !c1[1] && !c1[2] && !c1[3] )
            {
                for ( int i = 0; i < 4; i++ )
                {
                    tmp_c[i] = c1[i];
                    c1[i] = c2[i];
                    c2[i] = tmp_c[i];
                }
                tmp_pnt = pnt1;
                pnt1 = pnt2;
                pnt2 = tmp_pnt;
                swap = 1;
            }

            // Vertical Line //
            //if ( pnt2.v[0] == pnt1.v[0] )
            if( std::abs( pnt2.v[0] - pnt1.v[0] ) <= DBL_EPSILON )
            {
                if ( c1[3] )
                {
                    pnt1.v[1] = y_max;
                }
                else if ( c1[2] )
                {
                    pnt1.v[1] = y_min;
                }
            }

            // Not A Vertical Line //
            else
            {
                m = ( pnt2.v[1] - pnt1.v[1] ) / ( pnt2.v[0] - pnt1.v[0] );
                if ( c1[0] )
                {
                    pnt1.v[1] += ( x_min - pnt1.v[0] ) * m;
                    pnt1.v[0] = x_min;
                }
                else if ( c1[1] )
                {
                    pnt1.v[1] += ( x_max - pnt1.v[0] ) * m;
                    pnt1.v[0] = x_max;
                }
                else if ( c1[2] )
                {
                    pnt1.v[0] += ( y_min - pnt1.v[1] ) / m;
                    pnt1.v[1] = y_min;
                }
                else if ( c1[3] )
                {
                    pnt1.v[0] += ( y_max - pnt1.v[1] ) / m;
                    pnt1.v[1] = y_max;
                }
            }
        }
        if ( swap )
        {
            for ( int i = 0; i < 4; i++ )
            {
                tmp_c[i] = c1[i];
                c1[i] = c2[i];
                c2[i] = tmp_c[i];
            }
            tmp_pnt = pnt1;
            pnt1 = pnt2;
            pnt2 = tmp_pnt;
            swap = 0;
        }
    }

}


//==== Line Seg - Seg Intersection ====//
int seg_seg_intersect( const vec2d& pnt_A, const vec2d& pnt_B, const vec2d& pnt_C, const vec2d& pnt_D, vec2d& int_pnt )
{

    double denom = ( ( pnt_D[1] - pnt_C[1] ) * ( pnt_B[0] - pnt_A[0] ) ) -
                   ( ( pnt_D[0] - pnt_C[0] ) * ( pnt_B[1] - pnt_A[1] ) );

    if ( denom < DBL_EPSILON && denom > -DBL_EPSILON )
    {
        return 0;
    }

    double nume_a = ( ( pnt_D[0] - pnt_C[0] ) * ( pnt_A[1] - pnt_C[1] ) ) -
                    ( ( pnt_D[1] - pnt_C[1] ) * ( pnt_A[0] - pnt_C[0] ) );

    double nume_b = ( ( pnt_B[0] - pnt_A[0] ) * ( pnt_A[1] - pnt_C[1] ) ) -
                    ( ( pnt_B[1] - pnt_A[1] ) * ( pnt_A[0] - pnt_C[0] ) );

    double ua = nume_a / denom;
    double ub = nume_b / denom;

    double zero = -0.0000000000001;
    double one =   1.0000000000001;
    if( ua >= zero && ua <= one && ub >= zero && ub <= one )
    {
        // Get the intersection point.
        int_pnt = pnt_A + ( pnt_B - pnt_A ) * ua;
        return 1;
    }

    return 0;
}



//==== Project Point Onto Line Seg ====//
vec2d proj_pnt_on_line_seg( const vec2d& line_A, const vec2d& line_B, const vec2d& pnt )
{
    vec2d AB = line_B - line_A;

    //if ( AB.mag() == 0.0 )
    if (  AB.mag() <= DBL_EPSILON )
    {
        return( line_A );
    }

    double t = dot( ( pnt - line_A ), AB ) / dot( AB, AB );

    if ( t < 0.0 )
    {
        return( line_A );
    }

    if ( t > 1.0 )
    {
        return( line_B );
    }

    return( line_A + AB * t );

}

//==== Project Point Onto Line Seg ====//
double proj_pnt_on_line_u( const vec2d& line_A, const vec2d& line_B, const vec2d& pnt )
{
    vec2d AB = line_B - line_A;

    if (  AB.mag() <= DBL_EPSILON )
    {
        return 0.0;
    }

    double t = dot( ( pnt - line_A ), AB ) / dot( AB, AB );

    return t;

}

bool PointInPolygon( const vec2d & R, const std::vector< vec2d > & pnts )
{
    // Implementation of Algorithm 6 from "The Point in Polygon Problem
    // for Arbitrary Polygons" by Hormann and Agathos.
    //
    // R: the point in question
    // pnts: vector of points defining polygon, first and last point should be the same
    //
    // Note: This algorithm does not test to see if the test point lies on the
    //       an edge of the polygon

    int w = 0; // winding number
    bool modify_w = false;

    for ( int i = 0; i < ( int )pnts.size() - 1; i++ )
    {
        if ( ( pnts[i].y() < R.y() ) != ( pnts[i + 1].y() < R.y() ) ) // if crossing
        {
            if ( pnts[i].x() >= R.x() )
            {
                if ( pnts[i + 1].x() > R.x() )
                {
                    modify_w = true;
                }
                else if ( ( det( pnts[i], pnts[i + 1], R ) > 0 ) == ( pnts[i + 1].y() > pnts[i].y() ) ) // right crossing
                {
                    modify_w = true;
                }
            }
            else if ( pnts[i + 1].x() > R.x() )
            {
                if ( ( det( pnts[i], pnts[i + 1], R ) > 0 ) == ( pnts[i + 1].y() > pnts[i].y() ) ) // right crossing
                {
                    modify_w = true;
                }
            }
        }

        if ( modify_w )
        {
            w = w + 2 * ( pnts[i + 1].y() > pnts[i].y() ) - 1;    // modify w
        }

        modify_w = false;
    }

    return std::abs(w % 2) != 0;
}

double det( const vec2d & p0, const vec2d & p1, const vec2d & offset )
{
    double d = ( p0[0] - offset[0] ) * ( p1[1] - offset[1] ) - ( p1[0] - offset[0] ) * ( p0[1] - offset[1] );
    return d;
}

