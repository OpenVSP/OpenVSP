//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//


#ifndef __MATRIX_H_
#define __MATRIX_H_

#include <cmath>

#include <cstring>
#include "Vec3d.h"


/*!
    \ingroup Matrix4d
*/
/*!
    Matrix4d is typically used to perform rotations, translations, scaling, projections, and other transformations in 3D space.
*/
class Matrix4d
{
public:
    Matrix4d();

    /*!
    \ingroup Matrix4d
    */
    /*!
        Create a 4x4 identity matrix
        \forcpponly
        \code{.cpp}
        //==== Test Matrix4d ====//
        Matrix4d m();                            // Default Constructor
        m.loadIdentity();

        \endcode
        \endforcpponly
        \beginPythonOnly
        \code{.py}
    #==== Test Matrix4d ====
    m = Matrix4d()                                # Default Constructor
    m.loadIdentity()
    \endcode
        \endPythonOnly
    */

    void loadIdentity();

/*!
    \internal
    Print the matrix to stdout, for debugging.  Writes to the console rather than returning anything,
    so there is nothing for an example to check.
*/
    void print( const std::string &header = "" );

/*!
    \internal
    Overloads that read or write a raw double[16] or double[4].  The bindings cannot hand one of those
    across, so these are the C++ spellings; loadIdentity, getMat, matMult, postMult and initMat taking
    a Matrix4d are the ones to use from a script.
*/
    static void setIdentity( double *m );

    /*!
    \ingroup Matrix4d
    */
    /*!
        Translate the Matrix4d along the given axes values
        \forcpponly
        \code{.cpp}
        //==== Test Matrix4d ====//
        Matrix4d m();                            // Default Constructor

        m.loadIdentity();

        m.translatef( 1.0, 0.0, 0.0 );
        \endcode
        \endforcpponly
        \beginPythonOnly
        \code{.py}
    #==== Test Matrix4d ====
    m = Matrix4d()                                # Default Constructor

    m.loadIdentity()

    m.translatef( 1.0, 0.0, 0.0 )
    \endcode
        \endPythonOnly
        \param [in] x Translation along the X axis
        \param [in] y Translation along the Y axis
        \param [in] z Translation along the Z axis
    */

    void translatef( const double &x, const double &y, const double &z );

/*!
    \ingroup Matrix4d
*/
/*!
    Translate the matrix by a vec3d.  The vec3d counterpart of translatef.
    \beginPythonOnly
    \code{.py}
    m = Matrix4d()

    m.loadIdentity()

    m.translatev( vec3d( 1.0, 2.0, 3.0 ) )

    p = m.xform( vec3d( 0.0, 0.0, 0.0 ) )

    assert abs( p.x() - 1.0 ) < 1e-12, "translatev did not move the origin"

    assert abs( p.z() - 3.0 ) < 1e-12, "translatev did not move in z"
    \endcode
    \endPythonOnly
    \sa translatef, getTranslation
    \param [in] v vec3d Translation to apply
*/

    void translatev( const vec3d &v );

    /*!
    \ingroup Matrix4d
    */
    /*!
        Rotate the Matrix4d about the X axis
        \forcpponly
        \code{.cpp}
        //==== Test Matrix4d ====//
        Matrix4d m();                            // Default Constructor

        m.loadIdentity();

        m.rotateX( 90.0 );
        \endcode
        \endforcpponly
        \beginPythonOnly
        \code{.py}
    #==== Test Matrix4d ====
    m = Matrix4d()                                # Default Constructor

    m.loadIdentity()

    m.rotateX( 90.0 )
    \endcode
        \endPythonOnly
        \param [in] ang Angle of rotation (degrees)
    */

    void rotateX( const double &ang );

    /*!
    \ingroup Matrix4d
    */
    /*!
        Rotate the Matrix4d about the Y axis
        \forcpponly
        \code{.cpp}
        //==== Test Matrix4d ====//
        Matrix4d m();                            // Default Constructor

        m.loadIdentity();

        m.rotateY( 90.0 );
        \endcode
        \endforcpponly
        \beginPythonOnly
        \code{.py}
    #==== Test Matrix4d ====
    m = Matrix4d()                                # Default Constructor

    m.loadIdentity()

    m.rotateY( 90.0 )
    \endcode
        \endPythonOnly
        \param [in] ang Angle of rotation (degrees)
    */

    void rotateY( const double &ang );

    /*!
    \ingroup Matrix4d
    */
    /*!
        Rotate the Matrix4d about the Z axis
        \forcpponly
        \code{.cpp}
        //==== Test Matrix4d ====//
        Matrix4d m();                            // Default Constructor

        m.loadIdentity();

        m.rotateZ( 90.0 );
        \endcode
        \endforcpponly
        \beginPythonOnly
        \code{.py}
    #==== Test Matrix4d ====
    m = Matrix4d()                                # Default Constructor

    m.loadIdentity()

    m.rotateZ( 90.0 )
    \endcode
        \endPythonOnly
        \param [in] ang Angle of rotation (degrees)
    */

    void rotateZ( const double &ang );

    /*!
    \ingroup Matrix4d
    */
    /*!
        Rotate the Matrix4d about an arbitrary axis
        \forcpponly
        \code{.cpp}
        //==== Test Matrix4d ====//
        Matrix4d m();                            // Default Constructor
        float PI = 3.14;

        m.loadIdentity();

        m.rotate( PI / 4, vec3d( 0.0, 0.0, 1.0 ) );      // Radians
        \endcode
        \endforcpponly
        \beginPythonOnly
        \code{.py}
    #==== Test Matrix4d ====
    m = Matrix4d()                                # Default Constructor
    PI = 3.14

    m.loadIdentity()

    m.rotate( PI / 4, vec3d( 0.0, 0.0, 1.0 ) )                                # Radians
    \endcode
        \endPythonOnly
        \param [in] angle Angle of rotation (rad)
        \param [in] axis Vector to rotate about
    */

    void rotate( const double &angle, const vec3d &axis );


/*!
    \ingroup Matrix4d
*/
/*!
    Rotate the matrix so that its X axis points along a given direction.
    \beginPythonOnly
    \code{.py}
    m = Matrix4d()

    m.loadIdentity()

    m.rotatealongX( vec3d( 0.0, 1.0, 0.0 ) )

    p = m.xform( vec3d( 1.0, 0.0, 0.0 ) )

    assert abs( p.y() - 1.0 ) < 1e-12, "rotatealongX did not take X onto the given direction"
    \endcode
    \endPythonOnly
    \param [in] dir1 vec3d Direction the X axis should point along
*/

    void rotatealongX( const vec3d &dir1 );

/*!
    \ingroup Matrix4d
*/
/*!
    Clear the translation part of the matrix, leaving the rotation and scaling alone.
    \beginPythonOnly
    \code{.py}
    m = Matrix4d()

    m.loadIdentity()

    m.translatev( vec3d( 1.0, 2.0, 3.0 ) )

    m.zeroTranslations()

    t = m.getTranslation()

    assert abs( t.x() ) < 1e-12 and abs( t.z() ) < 1e-12, "zeroTranslations left a translation behind"
    \endcode
    \endPythonOnly
    \sa getTranslation, translatev
*/

    void zeroTranslations();

    /*!
    \ingroup Matrix4d
    */
    /*!
        Perform an affine transform on the Matrix4d
        \forcpponly
        \code{.cpp}
        //==== Test Matrix4d ====//
        Matrix4d m();                            // Default Constructor

        m.loadIdentity();

        m.rotateY( 10.0 );
        m.rotateX( 20.0 );
        m.rotateZ( 30.0 );

        vec3d c = m.xform( vec3d( 1.0, 1.0, 1.0 ) );

        m.affineInverse();
        \endcode
        \endforcpponly
        \beginPythonOnly
        \code{.py}
    #==== Test Matrix4d ====
    m = Matrix4d()                                # Default Constructor

    m.loadIdentity()

    m.rotateY( 10.0 )
    m.rotateX( 20.0 )
    m.rotateZ( 30.0 )

    c = m.xform( vec3d( 1.0, 1.0, 1.0 ) )

    m.affineInverse()
    \endcode
        \endPythonOnly
    */

    void affineInverse();

    /*!
    \ingroup Matrix4d
    */
    /*!
        Multiply the Matrix4d by a scalar value
        \forcpponly
        \code{.cpp}
        //==== Test Matrix4d ====//
        Matrix4d m();                            // Default Constructor

        m.loadXZRef();

        m.scale( 10.0 );
        \endcode
        \endforcpponly
        \beginPythonOnly
        \code{.py}
    #==== Test Matrix4d ====
    m = Matrix4d()                                # Default Constructor

    m.loadXZRef()

    m.scale( 10.0 )
    \endcode
        \endPythonOnly
        \param [in] scale Value to scale by
    */

    void scale( const double &scale );

/*!
    \ingroup Matrix4d
*/
/*!
    Scale the matrix along X alone.  Unlike scale, which scales all three axes together.
    \beginPythonOnly
    \code{.py}
    m = Matrix4d()

    m.loadIdentity()

    m.scalex( 2.0 )

    p = m.xform( vec3d( 1.0, 1.0, 1.0 ) )

    assert abs( p.x() - 2.0 ) < 1e-12, "scalex did not scale X"

    assert abs( p.y() - 1.0 ) < 1e-12, "scalex scaled an axis it should not have"
    \endcode
    \endPythonOnly
    \sa scale
    \param [in] scalex double Scale factor for X
*/

    void scalex( const double &scalex );

/*!
    \ingroup Matrix4d
*/
/*!
    Scale the matrix along Y alone.  Unlike scale, which scales all three axes together.
    \beginPythonOnly
    \code{.py}
    m = Matrix4d()

    m.loadIdentity()

    m.scaley( 2.0 )

    p = m.xform( vec3d( 1.0, 1.0, 1.0 ) )

    assert abs( p.y() - 2.0 ) < 1e-12, "scaley did not scale Y"

    assert abs( p.z() - 1.0 ) < 1e-12, "scaley scaled an axis it should not have"
    \endcode
    \endPythonOnly
    \sa scale
    \param [in] scaley double Scale factor for Y
*/

    void scaley( const double &scaley );

/*!
    \ingroup Matrix4d
*/
/*!
    Scale the matrix along Z alone.  Unlike scale, which scales all three axes together.
    \beginPythonOnly
    \code{.py}
    m = Matrix4d()

    m.loadIdentity()

    m.scalez( 2.0 )

    p = m.xform( vec3d( 1.0, 1.0, 1.0 ) )

    assert abs( p.z() - 2.0 ) < 1e-12, "scalez did not scale Z"

    assert abs( p.x() - 1.0 ) < 1e-12, "scalez scaled an axis it should not have"
    \endcode
    \endPythonOnly
    \sa scale
    \param [in] scalez double Scale factor for Z
*/

    void scalez( const double &scalez );

/*!
    \ingroup Matrix4d
*/
/*!
    Flip the matrix about the YZ plane, negating X.
    \beginPythonOnly
    \code{.py}
    m = Matrix4d()

    m.loadIdentity()

    m.flipx()

    p = m.xform( vec3d( 1.0, 2.0, 3.0 ) )

    assert abs( p.x() + 1.0 ) < 1e-12, "flipx did not negate x"

    assert abs( p.y() - 2.0 ) < 1e-12, "flipx disturbed y"
    \endcode
    \endPythonOnly
    \sa loadYZRef
*/

    void flipx();

/*!
    \internal
*/
    void getMat( double *m ) const;

/*!
    \internal
*/
    void matMult( const double *m );

/*!
    \internal
*/
    void postMult( const double *m );

/*!
    \ingroup Matrix4d
*/
/*!
    Multiply this matrix by another, this * m, and keep the result.
    \beginPythonOnly
    \code{.py}
    a = Matrix4d()
    b = Matrix4d()

    a.loadIdentity()
    a.translatev( vec3d( 1.0, 0.0, 0.0 ) )

    b.loadIdentity()
    b.scale( 2.0 )

    a.matMult( b )

    p = a.xform( vec3d( 1.0, 0.0, 0.0 ) )

    assert abs( p.x() - 3.0 ) < 1e-12, "matMult did not apply both transformations"
    \endcode
    \endPythonOnly
    \sa postMult, initMat
    \param [in] m Matrix4d Matrix to multiply by
*/

    void matMult( const Matrix4d &m );

/*!
    \ingroup Matrix4d
*/
/*!
    Multiply another matrix by this one, m * this, and keep the result.  The other order from matMult.
    \beginPythonOnly
    \code{.py}
    a = Matrix4d()
    b = Matrix4d()

    a.loadIdentity()
    a.translatev( vec3d( 1.0, 0.0, 0.0 ) )

    b.loadIdentity()
    b.scale( 2.0 )

    a.postMult( b )

    p = a.xform( vec3d( 1.0, 0.0, 0.0 ) )

    assert abs( p.x() - 4.0 ) < 1e-12, "postMult did not apply the transformations in the other order"
    \endcode
    \endPythonOnly
    \sa matMult, initMat
    \param [in] m Matrix4d Matrix to multiply by
*/

    void postMult( const Matrix4d &m );


/*!
    \internal
*/
    void initMat( const double *m );

/*!
    \ingroup Matrix4d
*/
/*!
    Copy another matrix into this one, replacing whatever was here.
    \beginPythonOnly
    \code{.py}
    a = Matrix4d()
    b = Matrix4d()

    b.loadIdentity()
    b.translatev( vec3d( 5.0, 0.0, 0.0 ) )

    a.initMat( b )

    p = a.xform( vec3d( 0.0, 0.0, 0.0 ) )

    assert abs( p.x() - 5.0 ) < 1e-12, "initMat did not copy the matrix"
    \endcode
    \endPythonOnly
    \sa matMult, postMult
    \param [in] m Matrix4d Matrix to copy
*/

    void initMat( const Matrix4d &m );

/*!
    \internal
*/
    void mult( const double in[ 4 ], double out[ 4 ] ) const;

/*!
    \internal
    Raw pointer to the 16 doubles, for C++ callers handing the matrix to something that expects an
    OpenGL style array.  Of no use from a script.
*/
    double *data()
    {
        return mat;
    }

    const double *const_data() const
    {
        return mat;
    }

    /*!
    \ingroup Matrix4d
    */
    /*!
        Load an identy Matrix4d and set the Y coordinate of the diagonal (index 5) to -1
        \forcpponly
        \code{.cpp}
        //==== Test Matrix4d ====//
        Matrix4d m();                            // Default Constructor

        m.loadXZRef();

        vec3d b = m.xform( vec3d( 1, 2, 3 ) );
        \endcode
        \endforcpponly
        \beginPythonOnly
        \code{.py}
    #==== Test Matrix4d ====
    m = Matrix4d()                                # Default Constructor

    m.loadXZRef()

    b = m.xform( vec3d( 1, 2, 3 ) )
    \endcode
        \endPythonOnly
    */

    void loadXZRef();

    /*!
    \ingroup Matrix4d
    */
    /*!
        Load an identy Matrix4d and set the Z coordinate of the diagonal (index 10) to -1
        \forcpponly
        \code{.cpp}
        //==== Test Matrix4d ====//
        Matrix4d m();                            // Default Constructor

        m.loadXYRef();

        vec3d b = m.xform( vec3d( 1, 2, 3 ) );
        \endcode
        \endforcpponly
        \beginPythonOnly
        \code{.py}
    #==== Test Matrix4d ====
    m = Matrix4d()                                # Default Constructor

    m.loadXYRef()

    b = m.xform( vec3d( 1, 2, 3 ) )
    \endcode
        \endPythonOnly
    */

    void loadXYRef();

    /*!
    \ingroup Matrix4d
    */
    /*!
        Load an identy Matrix4d and set the X coordinate of the diagonal (index 0) to -1
        \forcpponly
        \code{.cpp}
        //==== Test Matrix4d ====//
        Matrix4d m();                            // Default Constructor

        m.loadYZRef();

        vec3d b = m.xform( vec3d( 1, 2, 3 ) );
        \endcode
        \endforcpponly
        \beginPythonOnly
        \code{.py}
    #==== Test Matrix4d ====
    m = Matrix4d()                                # Default Constructor

    m.loadYZRef()

    b = m.xform( vec3d( 1, 2, 3 ) )
    \endcode
        \endPythonOnly
    */

    void loadYZRef();


    /*!
    \ingroup Matrix4d
    */
    /*!
        Transform the Matrix4d by the given vector
        \forcpponly
        \code{.cpp}
        //==== Test Matrix4d ====//
        Matrix4d m();                            // Default Constructor

        m.loadIdentity();

        vec3d a = m.xform( vec3d( 1.0, 2.0, 3.0 ) );
        \endcode
        \endforcpponly
        \beginPythonOnly
        \code{.py}
    #==== Test Matrix4d ====
    m = Matrix4d()                                # Default Constructor

    m.loadIdentity()

    a = m.xform( vec3d( 1.0, 2.0, 3.0 ) )
    \endcode
        \endPythonOnly
    */

    void mirrory();

/*!
    \ingroup Matrix4d
*/
/*!
    Transform a point by the matrix, applying the rotation, the scaling and the translation.
    \forcpponly
    \code{.cpp}
    Matrix4d m;

    m.loadIdentity();

    m.translatef( 1.0, 2.0, 3.0 );

    vec3d p = m.xform( vec3d( 0.0, 0.0, 0.0 ) );

    if ( abs( p.x() - 1.0 ) > 1e-12 )                    { Print( "ERROR: xform" ); __failure++; }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    m = Matrix4d()

    m.loadIdentity()

    m.translatev( vec3d( 1.0, 2.0, 3.0 ) )

    p = m.xform( vec3d( 0.0, 0.0, 0.0 ) )

    assert abs( p.x() - 1.0 ) < 1e-12, "xform did not apply the translation"
    \endcode
    \endPythonOnly
    \sa xformnorm, xformvec
    \param [in] in vec3d Point to transform
    \return vec3d Transformed point
*/

    vec3d xform( const vec3d &in ) const;

/*!
    \ingroup Matrix4d
*/
/*!
    Transform a whole vector of points by the matrix, in place.
    \beginPythonOnly
    \code{.py}
    m = Matrix4d()

    m.loadIdentity()

    m.translatev( vec3d( 1.0, 0.0, 0.0 ) )

    pts = Vec3dVec( [ vec3d( 0.0, 0.0, 0.0 ), vec3d( 1.0, 0.0, 0.0 ) ] )

    m.xformvec( pts )

    assert abs( pts[0].x() - 1.0 ) < 1e-12, "xformvec did not transform the first point"

    assert abs( pts[1].x() - 2.0 ) < 1e-12, "xformvec did not transform the second point"
    \endcode
    \endPythonOnly
    \sa xform, xformmat
    \param [in,out] in vector<vec3d> Points to transform, replaced by the result
*/

    void xformvec( std::vector < vec3d > &in ) const;

/*!
    \ingroup Matrix4d
*/
/*!
    Transform a grid of points by the matrix, in place.  The two dimensional counterpart of xformvec.
    \beginPythonOnly
    \code{.py}
    m = Matrix4d()

    m.loadIdentity()

    m.translatev( vec3d( 1.0, 0.0, 0.0 ) )

    grid = Vec3dVecVec( [ Vec3dVec( [ vec3d( 0.0, 0.0, 0.0 ) ] ), Vec3dVec( [ vec3d( 1.0, 0.0, 0.0 ) ] ) ] )

    m.xformmat( grid )

    assert abs( grid[0][0].x() - 1.0 ) < 1e-12, "xformmat did not transform the first row"

    assert abs( grid[1][0].x() - 2.0 ) < 1e-12, "xformmat did not transform the second row"
    \endcode
    \endPythonOnly
    \sa xformvec
    \param [in,out] in vector<vector<vec3d>> Grid of points to transform, replaced by the result
*/

    void xformmat( std::vector < std::vector < vec3d > > &in ) const;

/*!
    \ingroup Matrix4d
*/
/*!
    Transform a normal vector by the matrix.  Unlike xform this leaves the translation out, since a direction has no position.
    \beginPythonOnly
    \code{.py}
    m = Matrix4d()

    m.loadIdentity()

    m.translatev( vec3d( 1.0, 2.0, 3.0 ) )

    n = m.xformnorm( vec3d( 1.0, 0.0, 0.0 ) )

    assert abs( n.x() - 1.0 ) < 1e-12, "xformnorm changed the direction"

    assert abs( n.y() ) < 1e-12, "xformnorm applied the translation to a direction"
    \endcode
    \endPythonOnly
    \sa xform, xformnormvec
    \param [in] in vec3d Direction to transform
    \return vec3d Transformed direction
*/

    vec3d xformnorm( const vec3d &in ) const;

/*!
    \ingroup Matrix4d
*/
/*!
    Transform a whole vector of normals by the matrix, in place, leaving the translation out.
    \beginPythonOnly
    \code{.py}
    m = Matrix4d()

    m.loadIdentity()

    m.translatev( vec3d( 1.0, 2.0, 3.0 ) )

    norms = Vec3dVec( [ vec3d( 1.0, 0.0, 0.0 ) ] )

    m.xformnormvec( norms )

    assert abs( norms[0].y() ) < 1e-12, "xformnormvec applied the translation to a direction"
    \endcode
    \endPythonOnly
    \sa xformnorm, xformnormmat
    \param [in,out] in vector<vec3d> Directions to transform, replaced by the result
*/

    void xformnormvec( std::vector < vec3d > &in ) const;

/*!
    \ingroup Matrix4d
*/
/*!
    Transform a grid of normals by the matrix, in place.  The two dimensional counterpart of xformnormvec.
    \beginPythonOnly
    \code{.py}
    m = Matrix4d()

    m.loadIdentity()

    m.translatev( vec3d( 1.0, 2.0, 3.0 ) )

    grid = Vec3dVecVec( [ Vec3dVec( [ vec3d( 1.0, 0.0, 0.0 ) ] ) ] )

    m.xformnormmat( grid )

    assert abs( grid[0][0].y() ) < 1e-12, "xformnormmat applied the translation to a direction"
    \endcode
    \endPythonOnly
    \sa xformnormvec
    \param [in,out] in vector<vector<vec3d>> Grid of directions to transform, replaced by the result
*/

    void xformnormmat( std::vector < std::vector < vec3d > > &in ) const;

    /*!
    \ingroup Matrix4d
    */
    /*!
        Calculate the Matrix4d's angles between the X, Y and Z axes
        \forcpponly
        \code{.cpp}
        Matrix4d mat;
        float PI = 3.14;

        mat.loadIdentity();

        mat.rotate( PI / 4, vec3d( 0.0, 0.0, 1.0 ) );      // Radians

        vec3d angles = mat.getAngles();
        \endcode
        \endforcpponly
        \beginPythonOnly
        \code{.py}
    mat = Matrix4d()
    PI = 3.14

    mat.loadIdentity()

    mat.rotate( PI / 4, vec3d( 0.0, 0.0, 1.0 ) )                                # Radians

    angles = mat.getAngles()
    \endcode
        \endPythonOnly
        \return Angle measurement between each axis (degrees)
    */

    vec3d getAngles() const;

/*!
    \ingroup Matrix4d
*/
/*!
    Get the rotation as the arcball angles the GUI trackball uses.
    \beginPythonOnly
    \code{.py}
    m = Matrix4d()

    m.loadIdentity()

    a = m.getArcballAngles()

    assert abs( a.x() ) < 1e-12, "getArcballAngles found a rotation in an identity matrix"
    \endcode
    \endPythonOnly
    \sa getAngles
    \return vec3d Arcball angles
*/

    vec3d getArcballAngles() const;

/*!
    \ingroup Matrix4d
*/
/*!
    Get the translation part of the matrix.
    \beginPythonOnly
    \code{.py}
    m = Matrix4d()

    m.loadIdentity()

    m.translatev( vec3d( 1.0, 2.0, 3.0 ) )

    t = m.getTranslation()

    assert abs( t.x() - 1.0 ) < 1e-12, "getTranslation is wrong in x"

    assert abs( t.z() - 3.0 ) < 1e-12, "getTranslation is wrong in z"
    \endcode
    \endPythonOnly
    \sa translatev, zeroTranslations
    \return vec3d Translation
*/

    vec3d getTranslation() const;

    /*!
    \ingroup Matrix4d
    */
    /*!
        Translate the Matrix4d to a given position and rotate it a about a given center of rotation
        \forcpponly
        \code{.cpp}
        Matrix4d m;

        m.loadIdentity();

        m.buildXForm( vec3d( 1.0, 0.0, 0.0 ), vec3d( 0.0, 0.0, 90.0 ), vec3d( 0.0, 0.0, 0.0 ) );

        vec3d p = m.xform( vec3d( 1.0, 0.0, 0.0 ) );

        if ( abs( p.y() - 1.0 ) > 1e-9 )                     { Print( "ERROR: buildXForm" ); __failure++; }

        if ( abs( p.x() - 1.0 ) > 1e-9 )                     { Print( "ERROR: buildXForm" ); __failure++; }
        \endcode
        \endforcpponly
        \beginPythonOnly
        \code{.py}
        m = Matrix4d()

        m.loadIdentity()

        m.buildXForm( vec3d( 1.0, 0.0, 0.0 ), vec3d( 0.0, 0.0, 90.0 ), vec3d( 0.0, 0.0, 0.0 ) )

        p = m.xform( vec3d( 1.0, 0.0, 0.0 ) )

        assert abs( p.y() - 1.0 ) < 1e-9, "buildXForm did not rotate about the given center"

        assert abs( p.x() - 1.0 ) < 1e-9, "buildXForm did not translate to the given position"

        \endcode
        \endPythonOnly
        \sa translatev, rotate
        \param [in] pos Position to translate to
        \param [in] rot Angle of rotation (degrees)
        \param [in] cent_rot Center of rotation
    */

    void buildXForm( const vec3d &pos, const vec3d &rot, const vec3d &cent_rot );


/*!
    \ingroup Matrix4d
*/
/*!
    Get the three axis directions of the matrix.  The vec3d passed in are filled in rather than returned.
    \beginPythonOnly
    \code{.py}
    m = Matrix4d()

    m.loadIdentity()

    xdir = vec3d()
    ydir = vec3d()
    zdir = vec3d()

    m.getBasis( xdir, ydir, zdir )

    assert abs( xdir.x() - 1.0 ) < 1e-12, "getBasis did not report the X axis"

    assert abs( zdir.z() - 1.0 ) < 1e-12, "getBasis did not report the Z axis"
    \endcode
    \endPythonOnly
    \sa setBasis
    \param [out] xdir vec3d X axis direction
    \param [out] ydir vec3d Y axis direction
    \param [out] zdir vec3d Z axis direction
*/

    void getBasis( vec3d &xdir, vec3d &ydir, vec3d &zdir ) const;

/*!
    \ingroup Matrix4d
*/
/*!
    Set the rotation of the matrix from three axis directions.
    \beginPythonOnly
    \code{.py}
    m = Matrix4d()

    m.loadIdentity()

    m.setBasis( vec3d( 0.0, 1.0, 0.0 ), vec3d( -1.0, 0.0, 0.0 ), vec3d( 0.0, 0.0, 1.0 ) )

    p = m.xform( vec3d( 1.0, 0.0, 0.0 ) )

    assert abs( p.y() - 1.0 ) < 1e-12, "setBasis did not take X onto the given direction"
    \endcode
    \endPythonOnly
    \sa getBasis
    \param [in] xdir vec3d X axis direction
    \param [in] ydir vec3d Y axis direction
    \param [in] zdir vec3d Z axis direction
*/

    void setBasis( const vec3d &xdir, const vec3d &ydir, const vec3d &zdir );

/*!
    \ingroup Matrix4d
*/
/*!
    Reduce the matrix to a single rotation about an axis: the direction of the axis, a point on it, and the angle turned through in radians.
    \beginPythonOnly
    \code{.py}
    import math

    m = Matrix4d()

    m.loadIdentity()

    m.rotateZ( 90.0 )

    axis_dir = vec3d()
    axis_pnt = vec3d()

    angle = m.getRotationAxis( axis_dir, axis_pnt )

    assert abs( abs( axis_dir.z() ) - 1.0 ) < 1e-9, "getRotationAxis did not find the Z axis"

    assert abs( abs( angle ) - 0.5 * math.pi ) < 1e-9, "getRotationAxis did not measure the angle"
    \endcode
    \endPythonOnly
    \sa toQuat, getAngles
    \param [out] axis_dir vec3d Direction of the rotation axis
    \param [out] axis_pnt vec3d A point on the rotation axis
    \param [out] angle_out double Angle turned through, in radians
*/

    void getRotationAxis( vec3d &axis_dir, vec3d &axis_pnt, double &angle_out ) const;

/*!
    \ingroup Matrix4d
*/
/*!
    Decompose the matrix into a rotation quaternion and a translation.
    \beginPythonOnly
    \code{.py}
    m = Matrix4d()

    m.loadIdentity()

    qw, qx, qy, qz, tx, ty, tz = m.toQuat()

    assert abs( qw - 1.0 ) < 1e-12, "toQuat did not give the identity quaternion"

    assert abs( qx ) + abs( qy ) + abs( qz ) < 1e-12, "toQuat found a rotation in an identity matrix"

    assert abs( tx ) + abs( ty ) + abs( tz ) < 1e-12, "toQuat found a translation in an identity matrix"
    \endcode
    \endPythonOnly
    \sa getRotationAxis
    \param [out] qw_out double Scalar part of the rotation quaternion
    \param [out] qx_out double X part of the rotation quaternion
    \param [out] qy_out double Y part of the rotation quaternion
    \param [out] qz_out double Z part of the rotation quaternion
    \param [out] tx_out double X translation
    \param [out] ty_out double Y translation
    \param [out] tz_out double Z translation
*/

    void toQuat( double &qw_out, double &qx_out, double &qy_out, double &qz_out, double &tx_out, double &ty_out, double &tz_out ) const;

private:
    double mat[ 16 ];
};

#endif
