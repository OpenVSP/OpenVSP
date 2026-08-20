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

// Unlike vec3d and Matrix4d, vec2d is not registered with AngelScript -- ScriptMgr never mentions
// it -- so it reaches the API through the Python bindings alone.  The examples below are therefore
// Python only.  An \forcpponly block here would be generated into the AngelScript test suite and
// fail to compile, and would be documenting something a script cannot use.

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


/*!
    \ingroup vec2d
*/
/*!
    vec2d is typically used to describe coordinate points and vectors in 2D space.
    Both elements in the vector are of type double.
*/
class vec2d
{
private:

public:
    double v[2];

    // Default constructor.  Leaves the coordinates uninitialized; described with the two argument
    // constructor below rather than documented separately.
    vec2d();

    // The copy constructor, copy assignment operator, and destructor are intentionally left implicit.
    // This keeps vec2d trivially copyable, so vector<vec2d> copies can use the memmove fast path
    // instead of calling an out-of-line function per element.
#ifdef SWIG
    vec2d( const vec2d& a ); // vec2d x = y -- implicit in C++, declared so SWIG wraps it for the bindings.
#endif

/*!
    \ingroup vec2d
*/
/*!
    Construct a vec2d from its two coordinates.  The default constructor leaves the coordinates
    uninitialized, so prefer this one unless the value is about to be overwritten.
    \beginPythonOnly
    \code{.py}
    a = vec2d( 3.0, 4.0 )

    assert abs( a.x() - 3.0 ) < 1e-12, "vec2d did not store x"

    assert abs( a.y() - 4.0 ) < 1e-12, "vec2d did not store y"

    \endcode
    \endPythonOnly
    \param [in] xx double X coordinate
    \param [in] yy double Y coordinate
*/

    vec2d( double xx, double yy );

    vec2d& operator=( double a );      // x = 35.

/*!
    \ingroup vec2d
*/
/*!
    Index a vec2d by coordinate, 0 for X and 1 for Y.  An index outside that range raises an
    IndexError, which is also what lets list() and tuple() walk a vec2d.
    \beginPythonOnly
    \code{.py}
    a = vec2d( 3.0, 4.0 )

    assert abs( a[0] - 3.0 ) < 1e-12, "vec2d index 0 is not x"

    assert list( a ) == [ 3.0, 4.0 ], "vec2d did not iterate as x, y"

    a[1] = 5.0

    assert abs( a.y() - 5.0 ) < 1e-12, "vec2d index assignment did not take"

    \endcode
    \endPythonOnly
    \param [in] i int Coordinate index, 0 or 1
    \return double Coordinate value
*/

    double& operator [] ( int i )
    {
        return v[i];
    }
    const double& operator [] ( int i ) const
    {
        return v[i];
    }

    // Set Point Values
/*!
    \ingroup vec2d
*/
/*!
    Set both coordinates of the vec2d
    \beginPythonOnly
    \code{.py}
    a = vec2d()

    a.set_xy( 2.0, 4.0 )

    assert abs( a.x() - 2.0 ) < 1e-12, "set_xy did not set x"

    assert abs( a.y() - 4.0 ) < 1e-12, "set_xy did not set y"

    \endcode
    \endPythonOnly
    \sa set_x, set_y
    \param [in] xx double New X value
    \param [in] yy double New Y value
    \return vec2d Updated vec2d
*/

    vec2d& set_xy( double xx, double yy );

/*!
    \ingroup vec2d
*/
/*!
    Set the X coordinate (index 0) of the vec2d
    \beginPythonOnly
    \code{.py}
    a = vec2d( 0.0, 4.0 )

    a.set_x( 2.0 )

    assert abs( a.x() - 2.0 ) < 1e-12, "set_x did not set x"

    assert abs( a.y() - 4.0 ) < 1e-12, "set_x disturbed y"

    \endcode
    \endPythonOnly
    \sa set_xy, set_y
    \param [in] xx double New X value
    \return vec2d Updated vec2d
*/

    vec2d& set_x( double xx );

/*!
    \ingroup vec2d
*/
/*!
    Set the Y coordinate (index 1) of the vec2d
    \beginPythonOnly
    \code{.py}
    a = vec2d( 2.0, 0.0 )

    a.set_y( 4.0 )

    assert abs( a.y() - 4.0 ) < 1e-12, "set_y did not set y"

    assert abs( a.x() - 2.0 ) < 1e-12, "set_y disturbed x"

    \endcode
    \endPythonOnly
    \sa set_xy, set_x
    \param [in] yy double New Y value
    \return vec2d Updated vec2d
*/

    vec2d& set_y( double yy );

    // Get Point Values
/*!
    \ingroup vec2d
*/
/*!
    Get the X coordinate (index 0) of the vec2d
    \beginPythonOnly
    \code{.py}
    a = vec2d( 3.0, 4.0 )

    assert abs( a.x() - 3.0 ) < 1e-12, "x did not return the X coordinate"

    \endcode
    \endPythonOnly
    \sa y
    \return double X value
*/

    double x() const;

/*!
    \ingroup vec2d
*/
/*!
    Get the Y coordinate (index 1) of the vec2d
    \beginPythonOnly
    \code{.py}
    a = vec2d( 3.0, 4.0 )

    assert abs( a.y() - 4.0 ) < 1e-12, "y did not return the Y coordinate"

    \endcode
    \endPythonOnly
    \sa x
    \return double Y value
*/

    double y() const;

    // Raw pointer to the coordinates, for C++ callers handing a vec2d to something that expects a
    // double[2].  %ignore'd in the bindings -- a bare pointer is of no use from Python.
    double* data()
    {
        return( v );
    }

    // Transform Matrix.  %ignore'd in the bindings: a 3x3 C array is not something Python can build.
    vec2d transform( float mat[3][3] );
    vec2d transform( double mat[3][3] );

/*!
    \ingroup vec2d
*/
/*!
    Addition operator for two vec2d objects, performed by the addition of each corresponding component
    \beginPythonOnly
    \code{.py}
    a = vec2d( 1.0, 2.0 )
    b = vec2d( 3.0, 4.0 )

    c = a + b

    assert abs( c.x() - 4.0 ) < 1e-12, "vec2d addition is wrong in x"

    assert abs( c.y() - 6.0 ) < 1e-12, "vec2d addition is wrong in y"

    \endcode
    \endPythonOnly
    \param [in] a vec2d First vector
    \param [in] b vec2d Second vector
    \return vec2d Component-wise sum
*/

    friend vec2d operator+( const vec2d& a, const vec2d& b );

/*!
    \ingroup vec2d
*/
/*!
    Subtraction operator for two vec2d objects, performed by the subtraction of each corresponding component
    \beginPythonOnly
    \code{.py}
    a = vec2d( 3.0, 4.0 )
    b = vec2d( 1.0, 2.0 )

    c = a - b

    assert abs( c.x() - 2.0 ) < 1e-12, "vec2d subtraction is wrong in x"

    assert abs( c.y() - 2.0 ) < 1e-12, "vec2d subtraction is wrong in y"

    \endcode
    \endPythonOnly
    \param [in] a vec2d First vector
    \param [in] b vec2d Second vector
    \return vec2d Component-wise difference
*/

    friend vec2d operator-( const vec2d& a, const vec2d& b );

/*!
    \ingroup vec2d
*/
/*!
    Scalar multiplication operator for a vec2d, performed by the multiplication of each vec2d component and the scalar
    \beginPythonOnly
    \code{.py}
    a = vec2d( 1.0, 2.0 )

    c = a * 1.5

    assert abs( c.x() - 1.5 ) < 1e-12, "vec2d scaling is wrong in x"

    assert abs( c.y() - 3.0 ) < 1e-12, "vec2d scaling is wrong in y"

    \endcode
    \endPythonOnly
    \param [in] a vec2d Vector
    \param [in] b double Scalar
    \return vec2d Scaled vector
*/

    friend vec2d operator*( const vec2d& a, double b );
    friend vec2d operator*( double b, const vec2d& a );

/*!
    \ingroup vec2d
*/
/*!
    Component-wise multiplication of two vec2d objects.  This is not a dot or a cross product; see
    dot and cross for those.
    \beginPythonOnly
    \code{.py}
    a = vec2d( 1.0, 2.0 )
    b = vec2d( 3.0, 4.0 )

    c = a * b

    assert abs( c.x() - 3.0 ) < 1e-12, "vec2d component-wise product is wrong in x"

    assert abs( c.y() - 8.0 ) < 1e-12, "vec2d component-wise product is wrong in y"

    \endcode
    \endPythonOnly
    \sa dot, cross
    \param [in] a vec2d First vector
    \param [in] b vec2d Second vector
    \return vec2d Component-wise product
*/

    friend vec2d operator*( const vec2d& a, const vec2d& b );

/*!
    \ingroup vec2d
*/
/*!
    Scalar division operator for a vec2d, performed by the division of each vec2d component by the scalar
    \beginPythonOnly
    \code{.py}
    a = vec2d( 3.0, 6.0 )

    c = a / 1.5

    assert abs( c.x() - 2.0 ) < 1e-12, "vec2d division is wrong in x"

    assert abs( c.y() - 4.0 ) < 1e-12, "vec2d division is wrong in y"

    \endcode
    \endPythonOnly
    \param [in] a vec2d Vector
    \param [in] b double Scalar
    \return vec2d Divided vector
*/

    friend vec2d operator/( const vec2d& a, double b );
    vec2d& operator+=( const vec2d& b );
    vec2d& operator-=( const vec2d& b );
    vec2d& operator*=( double b );

/*!
    \ingroup vec2d
*/
/*!
    Calculate the distance between two vec2d
    \beginPythonOnly
    \code{.py}
    a = vec2d( 0.0, 0.0 )
    b = vec2d( 3.0, 4.0 )

    assert abs( dist( a, b ) - 5.0 ) < 1e-12, "dist did not measure the distance"

    \endcode
    \endPythonOnly
    \sa dist_squared
    \param [in] a vec2d First point
    \param [in] b vec2d Second point
    \return double Distance
*/

    friend double dist( const vec2d& a, const vec2d& b );

/*!
    \ingroup vec2d
*/
/*!
    Calculate the square of the distance between two vec2d.  Cheaper than dist when the answer is
    only being compared against another distance.
    \beginPythonOnly
    \code{.py}
    a = vec2d( 0.0, 0.0 )
    b = vec2d( 3.0, 4.0 )

    assert abs( dist_squared( a, b ) - 25.0 ) < 1e-12, "dist_squared did not measure the squared distance"

    \endcode
    \endPythonOnly
    \sa dist
    \param [in] a vec2d First point
    \param [in] b vec2d Second point
    \return double Squared distance
*/

    friend double dist_squared( const vec2d& a, const vec2d& b );

/*!
    \ingroup vec2d
*/
/*!
    Get the magnitude of a vec2d
    \beginPythonOnly
    \code{.py}
    a = vec2d( 3.0, 4.0 )

    assert abs( a.mag() - 5.0 ) < 1e-12, "mag did not return the magnitude"

    \endcode
    \endPythonOnly
    \sa normalize
    \return double Magnitude
*/

    double mag() const;                // x = a.mag()

/*!
    \ingroup vec2d
*/
/*!
    Scale a vec2d to unit length, in place
    \beginPythonOnly
    \code{.py}
    a = vec2d( 3.0, 4.0 )

    a.normalize()

    assert abs( a.mag() - 1.0 ) < 1e-12, "normalize did not produce a unit vector"

    assert abs( a.x() - 0.6 ) < 1e-12, "normalize did not keep the direction"

    \endcode
    \endPythonOnly
    \sa mag
*/

    void normalize();           // a.normalize()

/*!
    \ingroup vec2d
*/
/*!
    Calculate the 2D cross product of two vec2d.  In two dimensions the cross product is the single
    scalar a.x * b.y - a.y * b.x, which is the signed area of the parallelogram they span.
    \beginPythonOnly
    \code{.py}
    a = vec2d( 1.0, 0.0 )
    b = vec2d( 0.0, 1.0 )

    assert abs( cross( a, b ) - 1.0 ) < 1e-12, "cross did not return the signed area"

    assert abs( cross( b, a ) + 1.0 ) < 1e-12, "cross did not change sign with the order"

    \endcode
    \endPythonOnly
    \sa dot
    \param [in] a vec2d First vector
    \param [in] b vec2d Second vector
    \return double Signed area spanned by the two vectors
*/

    friend double cross( const vec2d& a, const vec2d& b );

/*!
    \ingroup vec2d
*/
/*!
    Calculate the dot product of two vec2d
    \beginPythonOnly
    \code{.py}
    a = vec2d( 1.0, 2.0 )
    b = vec2d( 3.0, 4.0 )

    assert abs( dot( a, b ) - 11.0 ) < 1e-12, "dot did not return the dot product"

    \endcode
    \endPythonOnly
    \sa cross
    \param [in] a vec2d First vector
    \param [in] b vec2d Second vector
    \return double Dot product
*/

    friend double dot( const vec2d& a, const vec2d& b ); // x = dot(a,b)

/*!
    \ingroup vec2d
*/
/*!
    Calculate the angle between two vec2d, in radians.  The result is unsigned, in [0, pi].
    \beginPythonOnly
    \code{.py}
    import math

    a = vec2d( 1.0, 0.0 )
    b = vec2d( 0.0, 1.0 )

    assert abs( angle( a, b ) - 0.5 * math.pi ) < 1e-12, "angle did not measure a right angle"

    \endcode
    \endPythonOnly
    \sa cos_angle
    \param [in] a vec2d First vector
    \param [in] b vec2d Second vector
    \return double Angle in radians
*/

    friend double angle( const vec2d& a, const vec2d& b );

/*!
    \ingroup vec2d
*/
/*!
    Calculate the cosine of the angle between two vec2d.  Cheaper than angle, which has to take an
    arc cosine, and enough on its own when the angle is only being compared.
    \beginPythonOnly
    \code{.py}
    a = vec2d( 1.0, 0.0 )
    b = vec2d( 0.0, 1.0 )

    assert abs( cos_angle( a, b ) ) < 1e-12, "cos_angle of a right angle should be zero"

    assert abs( cos_angle( a, a ) - 1.0 ) < 1e-12, "cos_angle of a vector with itself should be one"

    \endcode
    \endPythonOnly
    \sa angle
    \param [in] a vec2d First vector
    \param [in] b vec2d Second vector
    \return double Cosine of the angle between the vectors
*/

    friend double cos_angle( const vec2d& a, const vec2d& b );

/*!
    \ingroup vec2d
*/
/*!
    Intersect two line segments, AB and CD.  Reports whether they cross, and where.  The parameters
    t1 and t2 locate the intersection along each segment, running 0 at the first point to 1 at the
    second.
    \beginPythonOnly
    \code{.py}
    hit, pnt, t1, t2 = seg_seg_intersect( vec2d( 0.0, 0.0 ), vec2d( 2.0, 0.0 ), vec2d( 1.0, -1.0 ), vec2d( 1.0, 1.0 ) )

    assert hit != 0, "seg_seg_intersect missed a crossing"

    assert abs( pnt.x() - 1.0 ) < 1e-12, "seg_seg_intersect put the crossing in the wrong place"

    assert abs( t1 - 0.5 ) < 1e-12, "seg_seg_intersect did not locate the crossing along AB"

    assert abs( t2 - 0.5 ) < 1e-12, "seg_seg_intersect did not locate the crossing along CD"

    \endcode
    \endPythonOnly
    \param [in] pnt_A vec2d First point of the first segment
    \param [in] pnt_B vec2d Second point of the first segment
    \param [in] pnt_C vec2d First point of the second segment
    \param [in] pnt_D vec2d Second point of the second segment
    \param [out] int_pnt vec2d Intersection point
    \param [out] t1 double Parameter of the intersection along AB, in [0, 1]
    \param [out] t2 double Parameter of the intersection along CD, in [0, 1]
    \return int Nonzero if the segments intersect
*/

    friend int seg_seg_intersect( const vec2d& pnt_A, const vec2d& pnt_B, const vec2d& pnt_C, const vec2d& pnt_D, vec2d& int_pnt, double &t1, double &t2 );

/*!
    \ingroup vec2d
*/
/*!
    Project a point onto a line segment.  The result is clamped to the segment, so a point that
    projects past either end comes back as that end point.
    \beginPythonOnly
    \code{.py}
    p = proj_pnt_on_line_seg( vec2d( 0.0, 0.0 ), vec2d( 2.0, 0.0 ), vec2d( 1.0, 1.0 ) )

    assert abs( p.x() - 1.0 ) < 1e-12, "proj_pnt_on_line_seg projected to the wrong place"

    assert abs( p.y() ) < 1e-12, "proj_pnt_on_line_seg did not land on the segment"

    q = proj_pnt_on_line_seg( vec2d( 0.0, 0.0 ), vec2d( 2.0, 0.0 ), vec2d( 5.0, 1.0 ) )

    assert abs( q.x() - 2.0 ) < 1e-12, "proj_pnt_on_line_seg did not clamp to the end of the segment"

    \endcode
    \endPythonOnly
    \sa proj_pnt_on_line_u
    \param [in] line_A vec2d First point of the segment
    \param [in] line_B vec2d Second point of the segment
    \param [in] pnt vec2d Point to project
    \return vec2d Projected point on the segment
*/

    friend vec2d proj_pnt_on_line_seg( const vec2d& line_A, const vec2d& line_B, const vec2d& pnt );

/*!
    \ingroup vec2d
*/
/*!
    Project a point onto a line and return where along it the projection falls, running 0 at the
    first point to 1 at the second.  Unlike proj_pnt_on_line_seg the result is not clamped, so a
    point beyond the segment gives a parameter outside [0, 1].
    \beginPythonOnly
    \code{.py}
    u = proj_pnt_on_line_u( vec2d( 0.0, 0.0 ), vec2d( 2.0, 0.0 ), vec2d( 1.0, 1.0 ) )

    assert abs( u - 0.5 ) < 1e-12, "proj_pnt_on_line_u did not find the midpoint"

    \endcode
    \endPythonOnly
    \sa proj_pnt_on_line_seg
    \param [in] line_A vec2d First point of the line
    \param [in] line_B vec2d Second point of the line
    \param [in] pnt vec2d Point to project
    \return double Parameter of the projection along the line
*/

    friend double proj_pnt_on_line_u( const vec2d& line_A, const vec2d& line_B, const vec2d& pnt );

    // Cohen-Sutherland clipping helpers.  Both hand their answer back through a C array or through
    // arguments that are read as well as written, so neither is exposed in the bindings.
    friend void encode( double x_min, double y_min, double x_max, double y_max,
                        const vec2d& pnt, int code[4] );
    friend void clip_seg_rect( double x_min, double y_min, double x_max, double y_max,
                               vec2d& pnt1, vec2d& pnt2, int& visible );

/*!
    \ingroup vec2d
*/
/*!
    Test whether a point lies inside a polygon.  The polygon is given as its vertices in order; it
    does not have to be convex and does not have to repeat its first point at the end.
    \beginPythonOnly
    \code{.py}
    square = Vec2dVec( [ vec2d( 0.0, 0.0 ), vec2d( 1.0, 0.0 ), vec2d( 1.0, 1.0 ), vec2d( 0.0, 1.0 ) ] )

    assert PointInPolygon( vec2d( 0.5, 0.5 ), square ), "PointInPolygon missed an interior point"

    assert not PointInPolygon( vec2d( 1.5, 0.5 ), square ), "PointInPolygon accepted an exterior point"

    \endcode
    \endPythonOnly
    \sa poly_area, poly_centroid
    \param [in] R vec2d Point to test
    \param [in] pnts vector<vec2d> Vertices of the polygon, in order
    \return bool True if the point is inside the polygon
*/

    friend bool PointInPolygon( const vec2d & R, const std::vector< vec2d > & pnts );

/*!
    \ingroup vec2d
*/
/*!
    Twice the signed area of the triangle p0, p1, offset.  Positive when the three points turn
    counter-clockwise, so the sign says which side of the line p0-p1 the third point is on.
    \beginPythonOnly
    \code{.py}
    d = det( vec2d( 0.0, 0.0 ), vec2d( 1.0, 0.0 ), vec2d( 0.0, 1.0 ) )

    assert d > 0.0, "det did not report a counter-clockwise turn as positive"

    \endcode
    \endPythonOnly
    \sa orient2d
    \param [in] p0 vec2d First point
    \param [in] p1 vec2d Second point
    \param [in] offset vec2d Third point
    \return double Twice the signed area of the triangle
*/

    friend double det( const vec2d & p0, const vec2d & p1, const vec2d & offset );

/*!
    \ingroup vec2d
*/
/*!
    Calculate the area enclosed by a polygon, given as its vertices in order.  The result is
    unsigned, so the winding direction does not matter.  Repeating the first point at the end is
    allowed but not required.
    \beginPythonOnly
    \code{.py}
    square = Vec2dVec( [ vec2d( 0.0, 0.0 ), vec2d( 1.0, 0.0 ), vec2d( 1.0, 1.0 ), vec2d( 0.0, 1.0 ) ] )

    assert abs( poly_area( square ) - 1.0 ) < 1e-12, "poly_area did not measure the unit square"

    \endcode
    \endPythonOnly
    \sa poly_centroid, PointInPolygon
    \param [in] pnt_vec vector<vec2d> Vertices of the polygon, in order
    \return double Enclosed area
*/

    friend double poly_area( const std::vector< vec2d > & pnt_vec );

/*!
    \ingroup vec2d
*/
/*!
    Calculate the centroid of a polygon, given as its vertices in order.  This is the centroid of
    the enclosed area, not the average of the vertices.
    \beginPythonOnly
    \code{.py}
    square = Vec2dVec( [ vec2d( 0.0, 0.0 ), vec2d( 1.0, 0.0 ), vec2d( 1.0, 1.0 ), vec2d( 0.0, 1.0 ) ] )

    c = poly_centroid( square )

    assert abs( c.x() - 0.5 ) < 1e-12, "poly_centroid is wrong in x"

    assert abs( c.y() - 0.5 ) < 1e-12, "poly_centroid is wrong in y"

    \endcode
    \endPythonOnly
    \sa poly_area
    \param [in] pnt_vec vector<vec2d> Vertices of the polygon, in order
    \return vec2d Centroid of the enclosed area
*/

    friend vec2d poly_centroid( const std::vector< vec2d > & pnt_vec );

/*!
    \ingroup vec2d
*/
/*!
    Report which side of the directed line p0-p1 the point p falls on.  Positive when p is to the
    left, negative to the right, and zero when the three are collinear.
    \beginPythonOnly
    \code{.py}
    assert orient2d( vec2d( 0.0, 0.0 ), vec2d( 1.0, 0.0 ), vec2d( 0.0, 1.0 ) ) > 0.0, "orient2d put a left turn on the right"

    assert orient2d( vec2d( 0.0, 0.0 ), vec2d( 1.0, 0.0 ), vec2d( 0.0, -1.0 ) ) < 0.0, "orient2d put a right turn on the left"

    \endcode
    \endPythonOnly
    \sa det
    \param [in] p0 vec2d First point of the line
    \param [in] p1 vec2d Second point of the line
    \param [in] p vec2d Point to test
    \return double Positive to the left of the line, negative to the right, zero when collinear
*/

    friend double orient2d( const vec2d & p0, const vec2d & p1, const vec2d & p );

/*!
    \ingroup vec2d
*/
/*!
    Interpolate a point inside the quadrilateral p0, p1, p2, p3.  The parameter s runs from the
    p0-p3 edge to the p1-p2 edge and t runs from the p0-p1 edge to the p3-p2 edge, both over [0, 1].
    \beginPythonOnly
    \code{.py}
    p0 = vec2d( 0.0, 0.0 )
    p1 = vec2d( 1.0, 0.0 )
    p2 = vec2d( 1.0, 1.0 )
    p3 = vec2d( 0.0, 1.0 )

    p = bi_lin_interp( p0, p1, p2, p3, 0.25, 0.75 )

    assert abs( p.x() - 0.625 ) < 1e-12, "bi_lin_interp is wrong in x"

    assert abs( p.y() - 0.75 ) < 1e-12, "bi_lin_interp is wrong in y"

    \endcode
    \endPythonOnly
    \sa inverse_bi_lin_interp
    \param [in] p0 vec2d First corner
    \param [in] p1 vec2d Second corner
    \param [in] p2 vec2d Third corner
    \param [in] p3 vec2d Fourth corner
    \param [in] s double First parameter, in [0, 1]
    \param [in] t double Second parameter, in [0, 1]
    \param [out] p_out vec2d Interpolated point
*/

    friend void bi_lin_interp( const vec2d &p0, const vec2d &p1, const vec2d &p2, vec2d const &p3, double s, double t, vec2d &p_out );

/*!
    \ingroup vec2d
*/
/*!
    Recover the parameters s and t that place a point inside the quadrilateral p0, p1, p2, p3 --
    the inverse of bi_lin_interp.  The problem is quadratic, so it can have two answers, returned
    as (s, t) and (s2, t2); the return value says how many were found.  A configuration that
    degenerates for the point being asked about can return none.
    \beginPythonOnly
    \code{.py}
    p0 = vec2d( 0.0, 0.0 )
    p1 = vec2d( 1.0, 0.0 )
    p2 = vec2d( 1.0, 1.0 )
    p3 = vec2d( 0.0, 1.0 )

    p = bi_lin_interp( p0, p1, p2, p3, 0.25, 0.75 )

    n, s, t, s2, t2 = inverse_bi_lin_interp( p0, p1, p2, p3, p )

    assert n > 0, "inverse_bi_lin_interp found no solution"

    assert abs( s - 0.25 ) < 1e-9, "inverse_bi_lin_interp did not recover s"

    assert abs( t - 0.75 ) < 1e-9, "inverse_bi_lin_interp did not recover t"

    \endcode
    \endPythonOnly
    \sa bi_lin_interp
    \param [in] p0 vec2d First corner
    \param [in] p1 vec2d Second corner
    \param [in] p2 vec2d Third corner
    \param [in] p3 vec2d Fourth corner
    \param [in] p vec2d Point to locate
    \param [out] s double First parameter of the first solution
    \param [out] t double Second parameter of the first solution
    \param [out] s2 double First parameter of the second solution
    \param [out] t2 double Second parameter of the second solution
    \return int Number of solutions found
*/

    friend int inverse_bi_lin_interp( const vec2d &p0, const vec2d &p1, const vec2d &p2, vec2d const &p3, const vec2d &p, double &s, double &t, double &s2, double &t2 );
};

#endif
