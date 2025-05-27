//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

//******************************************************************************
//
//   3D Point Double Class
//
//   J.R. Gloudemans - 7/7/93
//******************************************************************************

#ifndef VSPVEC3D_H
#define VSPVEC3D_H


#include "Vec2d.h"

typedef Eigen::Matrix< double, 1, 3, 0x1, 1, 3 > threed_point_type;


#include <iostream>
#include <vector>

  /*!
    vec3d is typically used to describe coordinate points and vectors in 3D space.
    All 3 elements in the vector are of type double.
  */
class vec3d;
class Matrix4d;

//==== Forward declare to get around MS Compiler bug ====//
/*!
	\ingroup vec3d
*/
  /*!
    Addition operator for two vec3d objects, performed by the addition of each corresponding component
    \forcpponly
    \code{.cpp}
    vec3d a(), b();                                // Default Constructor

    a.set_xyz( 1.0, 2.0, 3.0 );
    b.set_xyz( 4.0, 5.0, 6.0 );

    vec3d c = a + b;

    Print( "a + b = ", false );

    Print( c );
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
        vec3d a(), b()                                # Default Constructor

    a.set_xyz( 1.0, 2.0, 3.0 )
    b.set_xyz( 4.0, 5.0, 6.0 )

    vec3d c = a + b

    Print( "a + b = ", False )

    Print( c )

    \endcode
    \endPythonOnly
  */

vec3d operator+( const vec3d& a, const vec3d& b );

/*!
	\ingroup vec3d
*/
  /*!
    \forcpponly
    Subtraction operator for two vec3d objects, performed by the subtraction of each corresponding component    \code{.cpp}
    \forcpponly
    \code{.cpp}
    vec3d a(), b();                                // Default Constructor

    a.set_xyz( 1.0, 2.0, 3.0 );
    b.set_xyz( 4.0, 5.0, 6.0 );

    vec3d c = a - b;

    Print( "a - b = ", false );

    Print( c );
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
        vec3d a(), b()                                # Default Constructor

    a.set_xyz( 1.0, 2.0, 3.0 )
    b.set_xyz( 4.0, 5.0, 6.0 )

    vec3d c = a - b

    Print( "a - b = ", False )

    Print( c )

    \endcode
    \endPythonOnly
  */

vec3d operator-( const vec3d& a, const vec3d& b );

/*!
	\ingroup vec3d
*/
  /*!
    Scalar multiplication operator for a vec3d, performed by the multiplication of each vec3d component and the scalar
    \forcpponly
    \code{.cpp}
    vec3d a();                                // Default Constructor

    a.set_xyz( 1.0, 2.0, 3.0 );

    double b = 1.5;

    vec3d c = a * b;

    Print( "a * b = ", false );

    Print( c );
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
        vec3d a()                                # Default Constructor

    a.set_xyz( 1.0, 2.0, 3.0 )

    b = 1.5

    vec3d c = a * b

    Print( "a * b = ", False )

    Print( c )

    \endcode
    \endPythonOnly
  */

vec3d operator*( const vec3d& a, double b );

/*!
	\ingroup vec3d
*/
  /*!
    Scalar multiplication operator for a vec3d, performed by the multiplication of each vec3d component and the scalar
    \forcpponly
    \code{.cpp}
    vec3d a();                                // Default Constructor

    a.set_xyz( 1.0, 2.0, 3.0 );

    double b = 1.5;

    vec3d c = a * b;

    Print( "a * b = ", false );

    Print( c );
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
        vec3d a()                                # Default Constructor

    a.set_xyz( 1.0, 2.0, 3.0 )

    b = 1.5

    vec3d c = a * b

    Print( "a * b = ", False )

    Print( c )

    \endcode
    \endPythonOnly
  */

vec3d operator*( const vec3d& a, const vec3d& b );

/*!
	\ingroup vec3d
*/
  /*!
    Scalar division operator for a vec3d, performed by the division of of each vec3d component by the scalar
    \forcpponly
    \code{.cpp}
    vec3d a();                                // Default Constructor

    a.set_xyz( 1.0, 2.0, 3.0 );

    double b = 1.5;

    vec3d c = a / b;

    Print( "a / b = ", false );

    Print( c );
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
        vec3d a()                                # Default Constructor

    a.set_xyz( 1.0, 2.0, 3.0 )

    b = 1.5

    vec3d c = a / b

    Print( "a / b = ", False )

    Print( c )

    \endcode
    \endPythonOnly
  */

vec3d operator/( const vec3d& a, double b );


  /*!
    vec3d is typically used to describe coordinate points and vectors in 3D space.
    All 3 elements in the vector are of type double.
  */
class vec3d
{
private:


public:
    double v[3];

    vec3d();           // vec3d x or new vec3d

    ~vec3d()  {}        // delete vec3d

    vec3d( double xx, double yy, double zz );

    vec3d( const vec3d& a ); // vec3d x = y

    vec3d( const threed_point_type &a );

    vec3d( const double a[3] );
    vec3d( const float a[3] );
    vec3d( const std::vector<double> &a );

    vec3d& operator=( const vec3d& a ); // x = y
    vec3d& operator=( const vec2d& a );
    vec3d& operator=( double a );      // x = 35.
    vec3d& operator=( const threed_point_type &a );

    double& operator [] ( int i )
    {
        return v[i];
    }
    const double& operator [] ( int i ) const
    {
        return v[i];
    }

    vec2d as_vec2d_xy();

    // Set Point Values
  /*!
    Set all three elements of the vec3d vector
    \forcpponly
    \code{.cpp}
    //==== Test Vec3d ====//
    vec3d a();                                // Default Constructor

    a.set_xyz( 2.0, 4.0, 6.0 );
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
        #==== Test Vec3d ====//
    vec3d a()                                # Default Constructor

    a.set_xyz( 2.0, 4.0, 6.0 )

    \endcode
    \endPythonOnly
    \param [in] xx New X value
    \param [in] yy New Y value
    \param [in] zz New Z value
    \return Updated vec3d
  */

    vec3d& set_xyz( double xx, double yy, double zz );

    vec3d& set_vec( const std::vector<double> &a );
    vec3d& set_arr( const double a[] );
    vec3d& set_arr( const float a[] );
  /*!
    Set the X coordinate (index 0) of the vec3d
    \forcpponly
    \code{.cpp}
    //==== Test Vec3d ====//
    vec3d a();                                // Default Constructor

    a.set_x( 2.0 );
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
        #==== Test Vec3d ====//
    vec3d a()                                # Default Constructor

    a.set_x( 2.0 )

    \endcode
    \endPythonOnly
    \param [in] xx New X value
    \return Updated vec3d
  */

    vec3d& set_x( double xx );

  /*!
    Set the Y coordinate (index 1) of the vec3d
    \forcpponly
    \code{.cpp}
    //==== Test Vec3d ====//
    vec3d a();                                // Default Constructor

    a.set_y( 4.0 );
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
        #==== Test Vec3d ====//
    vec3d a()                                # Default Constructor

    a.set_y( 4.0 )

    \endcode
    \endPythonOnly
    \param [in] yy New Y value
    \return Updated vec3d
  */

    vec3d& set_y( double yy );

  /*!
    Set the z coordinate (index 2) of the vec3d
    \forcpponly
    \code{.cpp}
    //==== Test Vec3d ====//
    vec3d a();                                // Default Constructor

    a.set_z( 6.0 );
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
        #==== Test Vec3d ====//
    vec3d a()                                # Default Constructor

    a.set_z( 6.0 )

    \endcode
    \endPythonOnly
    \param [in] zz in double new z value
    \return vec3d result
  */

    vec3d& set_z( double zz );


    vec3d& set_refx( const vec3d &a );
    vec3d& set_refy( const vec3d &a );
    vec3d& set_refz( const vec3d &a );

    // Get Point Values
    void get_pnt( double pnt[3] ) const
    {
        pnt[0] = v[0];
        pnt[1] = v[1];
        pnt[2] = v[2];
    }
    void get_pnt( float  pnt[3] ) const
    {
        pnt[0] = ( float )v[0];
        pnt[1] = ( float )v[1];
        pnt[2] = ( float )v[2];
    }
    void get_pnt( threed_point_type &pnt ) const;

  /*!
    Get the X coordinate (index 0) of the vec3d
    \forcpponly
    \code{cpp}
    vec3d a();                                // Default Constructor

    a.set_xyz( 2.0, 4.0, 6.0 );

    Print( "a.x() = ", false );
    Print( a.x() );

    Print( "a[0]= ", false );
    Print( a[0] );
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
        vec3d a()                                # Default Constructor

    a.set_xyz( 2.0, 4.0, 6.0 )

    Print( "a.x() = ", False )
    Print( a.x() )

    Print( "a[0]= ", False )
    Print( a[0] )

    \endcode
    \endPythonOnly
    \return X value
  */

    double x() const

    {
        return( v[0] );
    }
  /*!
    Get the Y coordinate (index 1) of the vec3d
    \forcpponly
    \code{cpp}
    vec3d a();                                // Default Constructor

    a.set_xyz( 2.0, 4.0, 6.0 );

    Print( "a.y() = ", false );
    Print( a.y() );

    Print( "a[1]= ", false );
    Print( a[1] );
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
        vec3d a()                                # Default Constructor

    a.set_xyz( 2.0, 4.0, 6.0 )

    Print( "a.y() = ", False )
    Print( a.y() )

    Print( "a[1]= ", False )
    Print( a[1] )

    \endcode
    \endPythonOnly
    \return Y value
  */

    double y() const

    {
        return( v[1] );
    }
  /*!
    Get the Z coordinate (index 2) of the vec3d
    \forcpponly
    \code{cpp}
    vec3d a();                                // Default Constructor

    a.set_xyz( 2.0, 4.0, 6.0 );

    Print( "a.z() = ", false );
    Print( a.z() );

    Print( "a[2]= ", false );
    Print( a[2] );
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
        vec3d a()                                # Default Constructor

    a.set_xyz( 2.0, 4.0, 6.0 )

    Print( "a.z() = ", False )
    Print( a.z() )

    Print( "a[2]= ", False )
    Print( a[2] )

    \endcode
    \endPythonOnly
    \return Z value
  */

    double z() const

    {
        return( v[2] );
    }

    double* data()
    {
        return( v );
    }

    void Transform( const Matrix4d &m );
    void FlipNormal()                          {};

    // Rotate About Axis --> Change Internal Values
  /*!
    Rotate the vec3d about the X axis. 
    \forcpponly
    \code{.cpp}
    //==== Test Vec3d ====//
    vec3d a();                                // Default Constructor
    float PI = 3.14;

    a.set_xyz( 1.0, 0.0, 0.0 );

    a.rotate_x( 0.5 * PI );
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    import math
    #==== Test Vec3d ====//
    vec3d a()                                # Default Constructor
    PI = 3.14

    a.set_xyz( 1.0, 0.0, 0.0 )

    a.rotate_x( 0.5 * PI )

    \endcode
    \endPythonOnly
    \param [in] theta double Rotation angle in radians
  */

    void rotate_x( double theta );

  /*!
    Rotate the vec3d about the Y axis. 
    \forcpponly
    \code{.cpp}
    //==== Test Vec3d ====//
    vec3d a();                                // Default Constructor
    float PI = 3.14;

    a.set_xyz( 1.0, 0.0, 0.0 );

    a.rotate_y( 0.5 * PI );
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    import math
    #==== Test Vec3d ====//
    vec3d a()                                # Default Constructor
    PI = 3.14

    a.set_xyz( 1.0, 0.0, 0.0 )

    a.rotate_y( 0.5 * PI )

    \endcode
    \endPythonOnly
    \param [in] theta double Rotation angle in radians
  */

    void rotate_y( double theta );

  /*!
    Rotate the vec3d about the Z axis.
    \forcpponly
    \code{.cpp}
    //==== Test Vec3d ====//
    vec3d a();                                // Default Constructor
    float PI = 3.14;

    a.set_xyz( 1.0, 0.0, 0.0 );

    a.rotate_z( 0.5 * PI );
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    import math
    #==== Test Vec3d ====//
    vec3d a()                                # Default Constructor
    PI = 3.14

    a.set_xyz( 1.0, 0.0, 0.0 )

    a.rotate_z( 0.5 * PI )

    \endcode
    \endPythonOnly
    \param [in] theta double Rotation angle in radians
  */

    void rotate_z( double theta );

    // Scale Up Single Coord  --> Change Internal Values
/*!
    Scale the X coordinate of the vec3d
    \forcpponly
    \code{.cpp}
    //==== Test Vec3d ====//
    vec3d a();                                // Default Constructor

    //===== Test Scale ====//
    a.set_xyz( 2.0, 2.0, 2.0 );

    a.scale_x( 2.0 );
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
        #==== Test Vec3d ====//
    vec3d a()                                # Default Constructor

    #===== Test Scale ====//
    a.set_xyz( 2.0, 2.0, 2.0 )

    a.scale_x( 2.0 )

    \endcode
    \endPythonOnly
    \param [in] scale Scaling factor for the X value
*/

    void scale_x( double scale )

    {
        v[0] *= scale;
    };
/*!
    Scale the Y coordinate of the vec3d
    \forcpponly
    \code{.cpp}
    //==== Test Vec3d ====//
    vec3d a();                                // Default Constructor

    //===== Test Scale ====//
    a.set_xyz( 2.0, 2.0, 2.0 );

    a.scale_y( 2.0 );
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
        #==== Test Vec3d ====//
    vec3d a()                                # Default Constructor

    #===== Test Scale ====//
    a.set_xyz( 2.0, 2.0, 2.0 )

    a.scale_y( 2.0 )

    \endcode
    \endPythonOnly
    \param [in] scale Scaling factor for the Y value
*/

    void scale_y( double scale )

    {
        v[1] *= scale;
    };
/*!
    Scale the Z coordinate of the vec3d
    \forcpponly
    \code{.cpp}
    //==== Test Vec3d ====//
    vec3d a();                                // Default Constructor

    //===== Test Scale ====//
    a.set_xyz( 2.0, 2.0, 2.0 );

    a.scale_z( 2.0 );
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
        #==== Test Vec3d ====//
    vec3d a()                                # Default Constructor

    #===== Test Scale ====//
    a.set_xyz( 2.0, 2.0, 2.0 )

    a.scale_z( 2.0 )

    \endcode
    \endPythonOnly
    \param [in] scale Scaling factor for the Z value
*/

    void scale_z( double scale )

    {
        v[2] *= scale;
    };

/*!
    Offset the X coordinate of the vec3d
    \forcpponly
    \code{.cpp}
    //==== Test Vec3d ====//
    vec3d a();                                // Default Constructor

    //===== Test Offset ====//
    a.set_xyz( 2.0, 2.0, 2.0 );

    a.offset_x( 10.0 );
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
        #==== Test Vec3d ====//
    vec3d a()                                # Default Constructor

    #===== Test Offset ====//
    a.set_xyz( 2.0, 2.0, 2.0 )

    a.offset_x( 10.0 )

    \endcode
    \endPythonOnly
    \param [in] offset Offset for the X value
*/

    void offset_x( double offset )

    {
        v[0] += offset;
    };
/*!
    Offset the Y coordinate of the vec3d
    \forcpponly
    \code{.cpp}
    //==== Test Vec3d ====//
    vec3d a();                                // Default Constructor

    //===== Test Offset ====//
    a.set_xyz( 2.0, 2.0, 2.0 );

    a.offset_y( 10.0 );
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
        #==== Test Vec3d ====//
    vec3d a()                                # Default Constructor

    #===== Test Offset ====//
    a.set_xyz( 2.0, 2.0, 2.0 )

    a.offset_y( 10.0 )

    \endcode
    \endPythonOnly
    \param [in] offset Offset for the Y value
*/

    void offset_y( double offset )

    {
        v[1] += offset;
    };
/*!
    Offset the Z coordinate of the vec3d
    \forcpponly
    \code{.cpp}
    //==== Test Vec3d ====//
    vec3d a();                                // Default Constructor

    //===== Test Offset ====//
    a.set_xyz( 2.0, 2.0, 2.0 );

    a.offset_z( 10.0 );
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
        #==== Test Vec3d ====//
    vec3d a()                                # Default Constructor

    #===== Test Offset ====//
    a.set_xyz( 2.0, 2.0, 2.0 )

    a.offset_z( 10.0 )

    \endcode
    \endPythonOnly
    \param [in] offset Offset for the Z value
*/

    void offset_z( double offset )

    {
        v[2] += offset;
    };
    void offset_i( double offset, int idir )
    {
        v[idir] += offset;
    };

/*!
    Reflect the vec3d across the XY plane
    \forcpponly
    \code{.cpp}
    //==== Test Vec3d ====//
    vec3d a(), b();                                // Default Constructor

    //===== Test Reflect ====//
    a.set_xyz( 1.0, 2.0, 3.0 );

    b = a.reflect_xy();
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
        #==== Test Vec3d ====//
    vec3d a(), b()                                # Default Constructor

    #===== Test Reflect ====//
    a.set_xyz( 1.0, 2.0, 3.0 )

    b = a.reflect_xy()

    \endcode
    \endPythonOnly
    \return Reflected vec3d
*/

    vec3d reflect_xy() const

    {
        return( vec3d( v[0],  v[1], -v[2] ) );
    }
/*!
    Reflect the vec3d across the XZ plane
    \forcpponly
    \code{.cpp}
    //==== Test Vec3d ====//
    vec3d a(), b();                                // Default Constructor

    //===== Test Reflect ====//
    a.set_xyz( 1.0, 2.0, 3.0 );

    b = a.reflect_xz();
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
        #==== Test Vec3d ====//
    vec3d a(), b()                                # Default Constructor

    #===== Test Reflect ====//
    a.set_xyz( 1.0, 2.0, 3.0 )

    b = a.reflect_xz()

    \endcode
    \endPythonOnly
    \return Reflected vec3d
*/

    vec3d reflect_xz() const

    {
        return( vec3d( v[0], -v[1],  v[2] ) );
    }
/*!
    Reflect the vec3d across the YZ plane
    \forcpponly
    \code{.cpp}
    //==== Test Vec3d ====//
    vec3d a(), b();                                // Default Constructor

    //===== Test Reflect ====//
    a.set_xyz( 1.0, 2.0, 3.0 );

    b = a.reflect_yz();
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
        #==== Test Vec3d ====//
    vec3d a(), b()                                # Default Constructor

    #===== Test Reflect ====//
    a.set_xyz( 1.0, 2.0, 3.0 )

    b = a.reflect_yz()

    \endcode
    \endPythonOnly
    \return Reflected vec3d
*/

    vec3d reflect_yz() const

    {
        return( vec3d( -v[0],  v[1],  v[2] ) );
    }

    vec3d swap_xy() const
    {
        return( vec3d( v[1],  v[0],  v[2] ) );
    }
    vec3d swap_xz() const
    {
        return( vec3d( v[2],  v[1],  v[0] ) );
    }
    vec3d swap_yz() const
    {
        return( vec3d( v[0],  v[2],  v[1] ) );
    }

    // x = a + b, x = a - b, a*scale, a/scale
    friend vec3d operator+( const vec3d& a, const vec3d& b );
    friend vec3d operator-( const vec3d& a, const vec3d& b );
    friend vec3d operator*( const vec3d& a, double b );
    friend vec3d operator*( double b, const vec3d& a );
    friend vec3d operator*( const vec3d& a, const vec3d& b );
    friend vec3d operator/( const vec3d& a, double b );
    vec3d& operator+=( const vec3d& b );
    vec3d& operator-=( const vec3d& b );
    vec3d& operator*=( double b );
    vec3d& operator+=( double b[] );
    vec3d& operator-=( double b[] );
    vec3d& operator+=( float b[] );
    vec3d& operator-=( float b[] );

    friend vec3d operator-( const vec3d & in );

    friend bool operator==( const vec3d &a, const vec3d& b );
    friend bool operator!=( const vec3d &a, const vec3d& b );

    // cout << a
//    friend ostream& operator<< (ostream& out, const vec3d& a);

    friend double dist( const vec3d& a, const vec3d& b );
    friend double dist_squared( const vec3d& a, const vec3d& b );

/*!
    Get the magnitude of a vec3d
    \forcpponly
    \code{.cpp}
    //==== Test Vec3d ====//
    vec3d a();                                // Default Constructor

    //==== Test Mag ====//
    a.set_xyz( 1.0, 2.0, 3.0 );

    if ( abs( a.mag() - sqrt( 14 ) ) > 1e-6 )                        { Print( "---> Error: Vec3d Mag " ); }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    import math
    #==== Test Vec3d ====//
    vec3d a()                                # Default Constructor

    #==== Test Mag ====//
    a.set_xyz( 1.0, 2.0, 3.0 )

    if  abs( a.mag() - math.sqrt( 14 ) ) > 1e-6 : Print( "---> Error: Vec3d Mag " ); }

    \endcode
    \endPythonOnly
    \return Magnitude
*/

    double mag() const;                // x = a.mag()

    double magsq() const;

/*!
    Normalize the vec3d
    \forcpponly
    \code{.cpp}
    //==== Test Vec3d ====//
    vec3d a(), b(), c();                                // Default Constructor

    //==== Test Cross ====//
    a.set_xyz( 4.0, 0.0, 0.0 );
    b.set_xyz( 0.0, 3.0, 0.0 );

    c = cross( a, b );

    c.normalize();
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
        #==== Test Vec3d ====//
    vec3d a(), b(), c()                                # Default Constructor

    #==== Test Cross ====//
    a.set_xyz( 4.0, 0.0, 0.0 )
    b.set_xyz( 0.0, 3.0, 0.0 )

    c = cross( a, b )

    c.normalize()

    \endcode
    \endPythonOnly
*/

    void normalize();           // a.normalize()


    int major_comp() const;
    int minor_comp() const;

    bool isnan() const;
    bool isinf() const;
    bool isfinite() const;

    void print( const char* label = "" ) const;

    friend double dot( const vec3d& a, const vec3d& b ); // x = dot(a,b)
    friend vec3d cross( const vec3d& a, const vec3d& b ); // a = cross(b,c)
    friend double angle( const vec3d& a, const vec3d& b );
    friend double signed_angle( const vec3d& a, const vec3d& b, const vec3d& ref );
    friend double cos_angle( const vec3d& a, const vec3d& b );
    friend double radius_of_circle( const vec3d& p1, const vec3d& p2, const vec3d& p3 );
    friend void center_of_circle( const vec3d& p1, const vec3d& p2, const vec3d& p3, vec3d& center );
    friend bool triangle_plane_intersect_test( const vec3d& org, const vec3d& norm, const vec3d& p1, const vec3d& p2, const vec3d& p3 );
    friend double triangle_plane_minimum_dist( const vec3d& org, const vec3d& norm, const vec3d& p1, const vec3d& p2, const vec3d& p3, vec3d &pa, vec3d &pb );
    friend double triangle_plane_maximum_dist( const vec3d& org, const vec3d& norm, const vec3d& p1, const vec3d& p2, const vec3d& p3, vec3d &pa, vec3d &pb );
    friend bool plane_plane_intersection( const vec3d &p0, const vec3d &n0, const vec3d &p1, const vec3d &n1, vec3d &p, vec3d &v );
    friend double angle_pnt_2_plane( const vec3d& ptplane, vec3d norm, const vec3d& ptaxis, vec3d axis, const vec3d &pt, int ccw, vec3d &prot );
    friend double signed_dist_pnt_2_plane( const vec3d& org, const vec3d& norm, const vec3d& pnt );
    friend double dist_pnt_2_plane( const vec3d& org, const vec3d& norm, const vec3d& pnt );
    friend double dist_pnt_2_line( const vec3d& line_pt1, const vec3d& line_pt2, const vec3d& pnt );
    friend double dist_pnt_2_ray( const vec3d& line_pt1, const vec3d& dir_unit_vec, const vec3d& pnt );
//   friend double dist_pnt_2_line_seg( vec3d& line_pt1,  vec3d& line_pt2,  vec3d& pnt);
    friend vec3d proj_u_on_v( const vec3d& u, const vec3d& v );
    friend vec3d proj_pnt_on_line_seg( const vec3d& line_pt1, const vec3d& line_pt2, const vec3d& pnt );
    friend vec3d proj_pnt_on_ray( const vec3d& line_pt1, const vec3d& line_dir, const vec3d& pnt );
    friend vec3d proj_pnt_on_line( const vec3d& line_pt1, const vec3d& line_pt2, const vec3d& pnt );
    friend vec3d proj_pnt_to_plane( const vec3d& org, const vec3d& plane_ln1, const vec3d& plane_ln2, const vec3d& pnt );
    friend vec3d proj_vec_to_plane( const vec3d& vec, const vec3d& norm );
    friend int tri_seg_intersect( const vec3d& A, const vec3d& B, const vec3d& C, const vec3d& D, const vec3d& E,
                                  double& u, double& w, double& t );
    friend int tri_ray_intersect( const vec3d& A, const vec3d& B, const vec3d& C, const vec3d& D, const vec3d& E,
                                  double& u, double& w, double& t );
    friend int plane_ray_intersect( const vec3d& A, const vec3d& B, const vec3d& C, const vec3d& D, const vec3d& E, double& t );
    friend int plane_ray_intersect( const vec3d& orig, const vec3d& norm, const vec3d& D, const vec3d& E, double& t );
    friend int ray_ray_intersect( const vec3d& A, const vec3d& B, const vec3d& C, const vec3d& D, vec3d& int_pnt1, vec3d& int_pnt2 );
    friend double tetra_volume( const vec3d& A, const vec3d& B, const vec3d& C );
    friend double area( const vec3d& A, const vec3d& B, const vec3d& C );
    friend double poly_area( const std::vector< vec3d > & pnt_vec );
    friend double dist3D_Segment_to_Segment( const vec3d& S1P0, const vec3d& S1P1, const vec3d& S2P0, const vec3d& S2P1 );
    friend double dist3D_Segment_to_Segment( const vec3d& S1P0, const vec3d& S1P1, const vec3d& S2P0, const vec3d& S2P1,
            double* Lt, vec3d* Ln, double* St, vec3d* Sn );
    friend double nearSegSeg( const vec3d& L0, const vec3d& L1, const vec3d& S0, const vec3d& S1, double* Lt, vec3d* Ln, double* St,  vec3d* Sn );
    friend double pointLineDistSquared( const vec3d & p, const vec3d& lp0, const vec3d& lp1, double &t, vec3d &pon );
    friend double pointSegDistSquared( const vec3d& p, const vec3d& sp0, const vec3d& sp1, double &t, vec3d &pon );
    friend vec3d  point_on_line( const vec3d & lp0, const vec3d & lp1, const double & t );

    friend vec2d MapToPlane( const vec3d & p, const vec3d & planeOrig, const vec3d & planeVec1, const vec3d & planeVec2 );
    friend vec3d MapFromPlane( const vec2d & uw, const vec3d & planeOrig, const vec3d & planeVec1, const vec3d & planeVec2 );

    friend int plane_half_space( const vec3d & planeOrig, const vec3d & planeNorm, const vec3d & pnt );

    friend bool line_line_intersect( const vec3d & p1, const vec3d & p2, const vec3d & p3, const vec3d & p4, double* s, double* t );

    friend vec3d RotateArbAxis( const vec3d & p, double theta, const vec3d & r );

    friend bool PtInTri( const vec3d & v0, const vec3d & v1, const vec3d & v2, const vec3d & p );
    friend vec3d BarycentricWeights( const vec3d & v0, const vec3d & v1, const vec3d & v2, const vec3d & p );
    friend void BilinearWeights( const vec3d & p0, const vec3d & p1, const vec3d & p, std::vector< double > & weights );
    friend double tri_tri_min_dist( const vec3d & v0, const vec3d & v1, const vec3d & v2, const vec3d & v3, const vec3d & v4, const vec3d & v5, vec3d &p1, vec3d &p2 );
    friend double pnt_tri_min_dist( const vec3d & v0, const vec3d & v1, const vec3d & v2, const vec3d & pnt, vec3d &pnearest );

    friend vec3d slerp( const vec3d& a, const vec3d& b, const double &t );
    friend void printpt( const vec3d & v );
    friend vec3d ToSpherical( const vec3d & v );
    friend vec3d ToSpherical2( const vec3d & v, const vec3d & vdet );
    friend vec3d ToCartesian( const vec3d & v );

    friend void FitPlane( const std::vector < vec3d > & pts, vec3d & cen, vec3d & norm );
};

/*!
	\ingroup vec3d
*/
/*!
    Calculate the distance between two vec3d inputs
    \forcpponly
    \code{.cpp}
    //==== Test Vec3d ====//
    vec3d a(), b();                                // Default Constructor

    //==== Test Dist ====//
    a.set_xyz( 2.0, 2.0, 2.0 );
    b.set_xyz( 3.0, 4.0, 5.0 );

    double d = dist( a, b );

    if ( abs( d - sqrt( 14 ) ) > 1e-6 )    { Print( "---> Error: Vec3d Dist " ); }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    import math
    #==== Test Vec3d ====//
    vec3d a(), b()                                # Default Constructor

    #==== Test Dist ====//
    a.set_xyz( 2.0, 2.0, 2.0 )
    b.set_xyz( 3.0, 4.0, 5.0 )

    d = dist( a; b )

    if  abs( d - math.sqrt( 14 ) ) > 1e-6 : Print( "---> Error: Vec3d Dist " ); }

    \endcode
    \endPythonOnly
    \sa dist_squared
    \param [in] a First vec3d
    \param [in] b Second vec3d
    \return Distance
*/

double dist( const vec3d& a, const vec3d& b );

/*!
	\ingroup vec3d
*/
/*!
    Calculate distance squared between two vec3d inputs
    \forcpponly
    \code{.cpp}
    //==== Test Vec3d ====//
    vec3d a(), b();                                // Default Constructor

    //==== Test Dist ====//
    a.set_xyz( 2.0, 2.0, 2.0 );
    b.set_xyz( 3.0, 4.0, 5.0 );

    double d2 = dist_squared( a, b );

    if ( abs( d2 - 14 ) > 1e-6 )    { Print( "---> Error: Vec3d Dist " ); }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
        #==== Test Vec3d ====//
    vec3d a(), b()                                # Default Constructor

    #==== Test Dist ====//
    a.set_xyz( 2.0, 2.0, 2.0 )
    b.set_xyz( 3.0, 4.0, 5.0 )

    d2 = dist_squared( a; b )

    if  abs( d2 - 14 ) > 1e-6 : Print( "---> Error: Vec3d Dist " ); }

    \endcode
    \endPythonOnly
    \sa dist
    \param [in] a First vec3d
    \param [in] b Second vec3d
    \return Distance squared
*/

double dist_squared( const vec3d& a, const vec3d& b );

/*!
	\ingroup vec3d
*/
/*!
    Calculate the dot product between two vec3d inputs
    \forcpponly
    \code{.cpp}
    //==== Test Vec3d ====//
    vec3d a(), b();                                // Default Constructor

    //==== Test Dot ====//
    a.set_xyz( 1.0, 2.0, 3.0 );
    b.set_xyz( 2.0, 3.0, 4.0 );

    if ( abs( dot( a, b ) - 20 ) > 1e-6 )                            { Print( "---> Error: Vec3d Dot " ); }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
        #==== Test Vec3d ====//
    vec3d a(), b()                                # Default Constructor

    #==== Test Dot ====//
    a.set_xyz( 1.0, 2.0, 3.0 )
    b.set_xyz( 2.0, 3.0, 4.0 )

    if  abs( dot( a, b ) - 20 ) > 1e-6 : Print( "---> Error: Vec3d Dot " ); }

    \endcode
    \endPythonOnly
    \param [in] a First vec3d
    \param [in] b Second vec3d
    \return Dot product
*/

double dot( const vec3d& a, const vec3d& b );

/*!
	\ingroup vec3d
*/
/*!
    Calculate the cross product between two vec3d inputs
    \forcpponly
    \code{.cpp}
    //==== Test Vec3d ====//
    vec3d a(), b(), c();                                // Default Constructor

    //==== Test Cross ====//
    a.set_xyz( 4.0, 0.0, 0.0 );
    b.set_xyz( 0.0, 3.0, 0.0 );

    c = cross( a, b );

    c.normalize();
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
        #==== Test Vec3d ====//
    vec3d a(), b(), c()                                # Default Constructor

    #==== Test Cross ====//
    a.set_xyz( 4.0, 0.0, 0.0 )
    b.set_xyz( 0.0, 3.0, 0.0 )

    c = cross( a, b )

    c.normalize()

    \endcode
    \endPythonOnly
    \param [in] a First vec3d
    \param [in] b Second vec3d
    \return Cross product
*/

vec3d cross( const vec3d& a, const vec3d& b );

/*!
	\ingroup vec3d
*/
/*!
    Calculate the angle between two vec3d inputs (dot product divided by their magnitudes multiplied)
    \forcpponly
    \code{.cpp}
    //==== Test Vec3d ====//
    vec3d a(), b();                                // Default Constructor
    float PI = 3.14159265359;

    //==== Test Angle ====//
    a.set_xyz( 1.0, 1.0, 0.0 );
    b.set_xyz( 1.0, 0.0, 0.0 );

    if ( abs( angle( a, b ) - PI / 4 ) > 1e-6 )                    { Print( "---> Error: Vec3d Angle " ); }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
        #==== Test Vec3d ====//
    vec3d a(), b()                                # Default Constructor
    PI = 3.14159265359

    #==== Test Angle ====//
    a.set_xyz( 1.0, 1.0, 0.0 )
    b.set_xyz( 1.0, 0.0, 0.0 )

    if  abs( angle( a, b ) - PI / 4 ) > 1e-6 : Print( "---> Error: Vec3d Angle " ); }

    \endcode
    \endPythonOnly
    \param [in] a First vec3d
    \param [in] b Second vec3d
    \return Angle in Radians
*/

double angle( const vec3d& a, const vec3d& b );

/*!
	\ingroup vec3d
*/
/*!
    Calculate the signed angle between two vec3d inputs (dot product divided by their magnitudes multiplied) and an input reference axis
    \forcpponly
    \code{.cpp}
    //==== Test Vec3d ====//
    vec3d a(), b(), c();                                // Default Constructor
    float PI = 3.14159265359;

    //==== Test Angle ====//
    a.set_xyz( 1.0, 1.0, 0.0 );
    b.set_xyz( 1.0, 0.0, 0.0 );
    c.set_xyz( 0.0, 0.0, 1.0 );

    if ( abs( signed_angle( a, b, c ) - -PI / 4 ) > 1e-6 )            { Print( "---> Error: Vec3d SignedAngle " ); }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
        #==== Test Vec3d ====//
    vec3d a(), b(), c()                                # Default Constructor
    PI = 3.14159265359

    #==== Test Angle ====//
    a.set_xyz( 1.0, 1.0, 0.0 )
    b.set_xyz( 1.0, 0.0, 0.0 )
    c.set_xyz( 0.0, 0.0, 1.0 )

    if  abs( signed_angle( a, b, c ) - -PI / 4 ) > 1e-6 : Print( "---> Error: Vec3d SignedAngle " ); }

    \endcode
    \endPythonOnly
    \param [in] a First vec3d
    \param [in] b Second vec3d
    \param [in] ref Reference axis
    \return Angle in Radians
*/

double signed_angle( const vec3d& a, const vec3d& b, const vec3d& ref );

/*!
	\ingroup vec3d
*/
/*!
    Calculate the cosine of angle between two vec3d inputs
    \forcpponly
    \code{.cpp}
    vec3d pnt = vec3d( 2, 4, 6);

    vec3d line_pt1(), line_pt2();

    line_pt1.set_z( 4 );
    line_pt2.set_y( 3 );

    vec3d p_ln1 = pnt - line_pt1;

    vec3d ln2_ln1 = line_pt2 - line_pt1;

    double numer =  cos_angle( p_ln1, ln2_ln1 ) * p_ln1.mag();
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
        vec3d pnt = vec3d( 2, 4, 6)

    vec3d line_pt1(), line_pt2()

    line_pt1.set_z( 4 )
    line_pt2.set_y( 3 )

    vec3d p_ln1 = pnt - line_pt1

    vec3d ln2_ln1 = line_pt2 - line_pt1

    numer =  cos_angle( p_ln1; ln2_ln1 ) * p_ln1.mag()

    \endcode
    \endPythonOnly
    \sa angle
    \param [in] a First vec3d
    \param [in] b Second vec3d
    \return Angle in Radians
*/ // TODO: verify description

double cos_angle( const vec3d& a, const vec3d& b );

/*!
	\ingroup vec3d
*/
/*!
    Rotate a input point by specified angle around an arbitrary axis. Assume right hand coordinate system
    \forcpponly
    \code{.cpp}
    //==== Test Vec3d ====//
    vec3d a(), b(), c();                                // Default Constructor
    float PI = 3.14;

    //==== Test Rotate ====//
    a.set_xyz( 1.0, 1.0, 0.0 );
    b.set_xyz( 1.0, 0.0, 0.0 );
    c.set_xyz( 0.0, 0.0, 1.0 );

    c = RotateArbAxis( b, PI, a );
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
        #==== Test Vec3d ====//
    vec3d a(), b(), c()                                # Default Constructor
    PI = 3.14

    #==== Test Rotate ====//
    a.set_xyz( 1.0, 1.0, 0.0 )
    b.set_xyz( 1.0, 0.0, 0.0 )
    c.set_xyz( 0.0, 0.0, 1.0 )

    c = RotateArbAxis( b, PI, a )

    \endcode
    \endPythonOnly
    \param [in] p Coordinate point to rotate
    \param [in] theta Angle of rotation in Radians
    \param [in] r Reference axis for rotation
    \return Coordinates of rotated point
*/

vec3d RotateArbAxis( const vec3d & p, double theta, const vec3d & r );


namespace std
{
string to_string( const vec3d &v);
}

vec3d slerp( const vec3d& a, const vec3d& b, const double &t );

void FitPlane( const std::vector < vec3d > & pts, vec3d & cen, vec3d & norm );

#endif



