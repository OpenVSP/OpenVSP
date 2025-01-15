//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//


#ifndef __MATRIX_H_
#define __MATRIX_H_

#include "Defines.h"
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
            #==== Test Matrix4d ====//
        Matrix4d m()                            # Default Constructor
        m.loadIdentity()


        \endcode
        \endPythonOnly
        \return Identity Matrix4d
    */

    void loadIdentity();

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
            #==== Test Matrix4d ====//
        Matrix4d m()                            # Default Constructor

        m.loadIdentity()

        m.translatef( 1.0, 0.0, 0.0 )

        \endcode
        \endPythonOnly
        \param [in] x Translation along the X axis
        \param [in] y Translation along the Y axis
        \param [in] z Translation along the Z axis
        \return Translated Matrix4d
    */

    void translatef( const double &x, const double &y, const double &z );

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
            #==== Test Matrix4d ====//
        Matrix4d m()                            # Default Constructor

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
            #==== Test Matrix4d ====//
        Matrix4d m()                            # Default Constructor

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
            #==== Test Matrix4d ====//
        Matrix4d m()                            # Default Constructor

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
            #==== Test Matrix4d ====//
        Matrix4d m()                            # Default Constructor
        PI = 3.14

        m.loadIdentity()

        m.rotate( PI / 4, vec3d( 0.0, 0.0, 1.0 ) )      # Radians

        \endcode
        \endPythonOnly
        \param [in] angle Angle of rotation (rad)
        \param [in] axis Vector to rotate about
    */

    void rotate( const double &angle, const vec3d &axis );


    void rotatealongX( const vec3d &dir1 );

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
            #==== Test Matrix4d ====//
        Matrix4d m()                            # Default Constructor

        m.loadIdentity()

        m.rotateY( 10.0 )
        m.rotateX( 20.0 )
        m.rotateZ( 30.0 )

        vec3d c = m.xform( vec3d( 1.0, 1.0, 1.0 ) )

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
            #==== Test Matrix4d ====//
        Matrix4d m()                            # Default Constructor

        m.loadXZRef()

        m.scale( 10.0 )

        \endcode
        \endPythonOnly
        \param [in] scale Value to scale by
    */

    void scale( const double &scale );

    void scalex( const double &scalex );

    void scaley( const double &scaley );

    void scalez( const double &scalez );

    void mirrory();

    void flipx();

    void getMat( double *m ) const;

    void matMult( const double *m );

    void postMult( const double *m );

    void matMult( const Matrix4d &m );

    void postMult( const Matrix4d &m );


    void initMat( const double *m );

    void initMat( const Matrix4d &m );

    void mult( const double in[ 4 ], double out[ 4 ] ) const;

    double *data()
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
            #==== Test Matrix4d ====//
        Matrix4d m()                            # Default Constructor

        m.loadXZRef()

        vec3d b = m.xform( vec3d( 1, 2, 3 ) )

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
            #==== Test Matrix4d ====//
        Matrix4d m()                            # Default Constructor

        m.loadXYRef()

        vec3d b = m.xform( vec3d( 1, 2, 3 ) )

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
            #==== Test Matrix4d ====//
        Matrix4d m()                            # Default Constructor

        m.loadYZRef()

        vec3d b = m.xform( vec3d( 1, 2, 3 ) )

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
            #==== Test Matrix4d ====//
        Matrix4d m()                            # Default Constructor

        m.loadIdentity()

        vec3d a = m.xform( vec3d( 1.0, 2.0, 3.0 ) )

        \endcode
        \endPythonOnly
        \param [in] in Transformation vector
    */

    vec3d xform( const vec3d &in ) const;

    void xformvec( std::vector < vec3d > &in ) const;

    void xformmat( std::vector < std::vector < vec3d > > &in ) const;

    vec3d xformnorm( const vec3d &in ) const;

    void xformnormvec( std::vector < vec3d > &in ) const;

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

        m.rotate( PI / 4, vec3d( 0.0, 0.0, 1.0 ) );      // Radians

        vec3d angles = mat.getAngles();
        \endcode
        \endforcpponly
        \beginPythonOnly
        \code{.py}
            Matrix4d mat
        PI = 3.14

        mat.loadIdentity()

        m.rotate( PI / 4, vec3d( 0.0, 0.0, 1.0 ) )      # Radians

        vec3d angles = mat.getAngles()

        \endcode
        \endPythonOnly
        \return Angle measurement between each axis (degrees)
    */

    vec3d getAngles() const;

    vec3d getTranslation() const;

    /*!
    \ingroup Matrix4d
    */
    /*!
        Translate the Matrix4d to a given position and rotate it a about a given center of rotation
        \param [in] pos Position to translate to
        \param [in] rot Angle of rotation (degrees)
        \param [in] cent_rot Center of rotation
    */

    void buildXForm( const vec3d &pos, const vec3d &rot, const vec3d &cent_rot );


    void getBasis( vec3d &xdir, vec3d &ydir, vec3d &zdir );

    void setBasis( const vec3d &xdir, const vec3d &ydir, const vec3d &zdir );

    void toQuat( double &qw, double &qx, double &qy, double &qz, double &tx, double &ty, double &tz ) const;

private:
    double mat[ 16 ];
};

#endif
