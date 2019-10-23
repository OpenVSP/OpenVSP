#ifndef TOOL_BOX_Transfo_HPP__
#define TOOL_BOX_Transfo_HPP__

#include <cassert>
#include "vec3.h"
#include "mat3.h"
#include "point3.h"

// =============================================================================
namespace Tbx
{
  // =============================================================================

  /**
    @name Transfo
    @brief Handling geometric transformations with a 4x4 matrix

    Read carrefully, this class is not just a 4x4 matrix but aims to efficiently
    handle affine transformations, there is special cases you must be aware of:

    Some methods will assume the 4x4 matrix represent an affine transformations,
    these methods will have undefined behavior if #Transfo isn't an affine
    transformation (For instance a persepective projection matrix is not)

    @section Memory layout

    The 4x4 matrix is stored linearly with an array of 16 floats.
    The matrix is layout with <b> rows first (i.e row major) </b>

    @section Vector multiplication

    For convenience one can multiply #Transfo against #Vec3 and #Point3.
    But be aware of their behaviors:

    A #Vec3 miss is forth homegenous component. When multiplying, the translation
    (last column) is ignored as well as the last row:
    @code
    // Multiplies against the upper 3x3 matrix
    Vec3 v = Transfo(...) * Vec3(1.f); // Valid if Transfo is affine
    @endcode

    A #Point3 miss is forth homegenous component. When multiplying, the
    translation part (last column) will be added (as points are expected to translate)
    but the last row will be ignored:
    @code
    // Multiplies against the upper 3x3 matrix and add "Transfo(...).get_translation()"
    Point3 v = Transfo(...) * Point3(1.f); // Valid if Transfo is affine
    @endcode

    A #Vec4 is not a special case and behaves as any matrix multiplication.

    @section Projection

    If one wants to project a point this can be done manually with a #Vec4 with
    the last homegenous component set to one:
    @code
    Vec3 point;
    Vec4 h_point = Transfo(projection) * Vec4(point, 1.f);
    point = Vec3( h_point.x, h_point.y, h_point.z ) / h_point.z;
    @endcode

    Which is equivalent to use #Transfo::project() with a #Point3:
    @code
    Point3 result = Transfo(projection).project( Point3(a_point) );
    @endcode

    @section Inversion
    Two methods can be used for inverting the matrix:
    @li #Transfo::fast_invert() : assume the Transfo is affine and only inverts
    the 3x3 matrix and take the opposite of the translation component.
    @li #Transfo::full_invert() : works with any matrix but is slower

    @see Mat3 Vec3
  */
  struct Transfo
  {

    /// Linear matrix storage with <b> rows first (i.e row major) </b>
    /// Using this with OpenGL can be done by transposing first:
    /// @code
    ///     Transfo tr;
    ///     // OpenGL is column major !
    ///     glMultMatrixf( (GLfloat)(tr.transpose().m) );
    /// @endcode
    float m[16];

    // -------------------------------------------------------------------------
    /// @name Constructors
    // -------------------------------------------------------------------------

    inline Transfo() {}

    inline Transfo(float a00, float a01, float a02, float a03,
      float a10, float a11, float a12, float a13,
      float a20, float a21, float a22, float a23,
      float a30, float a31, float a32, float a33)
    {
      m[ 0] = a00; m[ 1] = a01; m[ 2] = a02; m[ 3] = a03;
      m[ 4] = a10; m[ 5] = a11; m[ 6] = a12; m[ 7] = a13;
      m[ 8] = a20; m[ 9] = a21; m[10] = a22; m[11] = a23;
      m[12] = a30; m[13] = a31; m[14] = a32; m[15] = a33;
    }

    inline Transfo(const Mat3& x)
    {
      m[ 0] = x.a; m[ 1] = x.b; m[ 2] = x.c; m[ 3] = 0.f;
      m[ 4] = x.d; m[ 5] = x.e; m[ 6] = x.f; m[ 7] = 0.f;
      m[ 8] = x.g; m[ 9] = x.h; m[10] = x.i; m[11] = 0.f;
      m[12] = 0.f; m[13] = 0.f; m[14] = 0.f; m[15] = 1.f;
    }

    inline Transfo(const Mat3& x, const Vec3& v)
    {
      m[ 0] = x.a; m[ 1] = x.b; m[ 2] = x.c; m[ 3] = v.x;
      m[ 4] = x.d; m[ 5] = x.e; m[ 6] = x.f; m[ 7] = v.y;
      m[ 8] = x.g; m[ 9] = x.h; m[10] = x.i; m[11] = v.z;
      m[12] = 0.f; m[13] = 0.f; m[14] = 0.f; m[15] = 1.f;
    }

    inline Transfo(const Vec3& v)
    {
      m[ 0] = 1.f; m[ 1] = 0.f; m[ 2] = 0.f; m[ 3] = v.x;
      m[ 4] = 0.f; m[ 5] = 1.f; m[ 6] = 0.f; m[ 7] = v.y;
      m[ 8] = 0.f; m[ 9] = 0.f; m[10] = 1.f; m[11] = v.z;
      m[12] = 0.f; m[13] = 0.f; m[14] = 0.f; m[15] = 1.f;
    }

    // -------------------------------------------------------------------------
    /// @name Accessors
    // -------------------------------------------------------------------------

    inline Vec3 x() const{ return Vec3( m[0], m[4], m[ 8] ); }
    inline Vec3 y() const{ return Vec3( m[1], m[5], m[ 9] ); }
    inline Vec3 z() const{ return Vec3( m[2], m[6], m[10] ); }

    inline Mat3 get_mat3() const
    {
      return Mat3(m[0], m[1], m[2],
        m[4], m[5], m[6],
        m[8], m[9], m[10]);
    }

    /// get translation part of the matrix
    /// @note same as get_org()
    inline Vec3 get_translation() const { return Vec3(m[3], m[7], m[11]); }

    /// get origine of the frame represented by the matrix
    /// @note same as get_translation()
    inline Vec3 get_org() const { return Vec3(m[3], m[7], m[11]); }

    inline void set_x(const Vec3& x){ m[0] = x.x; m[4] = x.y; m[ 8] = x.z; }
    inline void set_y(const Vec3& y){ m[1] = y.x; m[5] = y.y; m[ 9] = y.z; }
    inline void set_z(const Vec3& z){ m[2] = z.x; m[6] = z.y; m[10] = z.z; }

    inline void set_translation(const Vec3& tr){ m[3] = tr.x; m[7] = tr.y; m[11] = tr.z; }
    inline void set_org        (const Vec3& tr){ m[3] = tr.x; m[7] = tr.y; m[11] = tr.z; }

    inline void set_translation(const Transfo& tr)
    {
      const Vec3 trans = tr.get_translation();
      m[3] = trans.x; m[7] = trans.y; m[11] = trans.z;
    }

    inline void set_mat3(const Mat3& x)
    {
      m[ 0] = x.a; m[ 1] = x.b; m[ 2] = x.c;
      m[ 4] = x.d; m[ 5] = x.e; m[ 6] = x.f;
      m[ 8] = x.g; m[ 9] = x.h; m[10] = x.i;
    }

    // -------------------------------------------------------------------------
    /// @name Operators
    /// @note A special attention has to be made regarding the multiplication
    /// operators. The operator is overloaded differently wether you use
    /// Vec3 or Point3. Vec3 does not need to be multiplied against the
    /// matrix translation part, whereas Point3 does. This is because the
    /// homogenous part is not represented. Yet when projecting a point you will
    /// need the fourth component. In this case projection can't be done through
    /// matrix multiplication use instead the method 'project()' or Vec4
    // -------------------------------------------------------------------------

    /// The translation part of the transformation is ignored (as expected
    /// for a vector) you may use #Vec4 for a full multiplication.
    /// @warning undefined behavior if 'this' is not an affine transformation
    inline Vec3 operator*(const Vec3& v) const
    {
      return Vec3(
        m[0] * v.x + m[1] * v.y + m[ 2] * v.z,
        m[4] * v.x + m[5] * v.y + m[ 6] * v.z,
        m[8] * v.x + m[9] * v.y + m[10] * v.z);
    }

    #if 0
    /// Full matrix multiplication against the Vec4
    inline Vec4 operator*(const Vec4& v) const
    {
      return Vec4(m[ 0] * v.x + m[ 1] * v.y + m[ 2] * v.z + m[ 3] * v.w,
        m[ 4] * v.x + m[ 5] * v.y + m[ 6] * v.z + m[ 7] * v.w,
        m[ 8] * v.x + m[ 9] * v.y + m[10] * v.z + m[11] * v.w,
        m[12] * v.x + m[13] * v.y + m[14] * v.z + m[15] * v.w);
    }
    #endif

    /// This ignores the homogenous coordinates (Since Point3 doesn't have any)
    /// use project() to virtually consider the fourth component
    /// and do the perspective division for projection matrices
    /// @warning undefined behavior if 'this' is not an affine transformation
    inline Point3 operator*(const Point3& v) const
    {
      return Point3(
        m[0] * v.x + m[1] * v.y + m[ 2] * v.z + m[ 3],
        m[4] * v.x + m[5] * v.y + m[ 6] * v.z + m[ 7],
        m[8] * v.x + m[9] * v.y + m[10] * v.z + m[11]);
    }

    /// Multiply 'v' by the matrix and do the perspective division
    inline Point3 project(const Point3& v) const
    {
      Point3 tmp =  Point3(
        m[0] * v.x + m[1] * v.y + m[ 2] * v.z + m[ 3],
        m[4] * v.x + m[5] * v.y + m[ 6] * v.z + m[ 7],
        m[8] * v.x + m[9] * v.y + m[10] * v.z + m[11]);

      return tmp / (m[12] * v.x + m[13] * v.y + m[14] * v.z + m[15]);
    }

    inline Transfo operator*(const Transfo& t) const
    {
      Transfo res;
      for(int i = 0; i < 4; i++)
      {
        int j = i*4;
        res[j+0] = m[j] * t.m[0] + m[j+1] * t.m[4] + m[j+2] * t.m[ 8] + m[j+3] * t.m[12];
        res[j+1] = m[j] * t.m[1] + m[j+1] * t.m[5] + m[j+2] * t.m[ 9] + m[j+3] * t.m[13];
        res[j+2] = m[j] * t.m[2] + m[j+1] * t.m[6] + m[j+2] * t.m[10] + m[j+3] * t.m[14];
        res[j+3] = m[j] * t.m[3] + m[j+1] * t.m[7] + m[j+2] * t.m[11] + m[j+3] * t.m[15];
      }
      return res;
    }

    inline Transfo& operator*=(const Transfo& t)
    {
      (*this) = (*this) * t;
      return (*this);
    }

    /// The translation part of the transformation is ignored (as expected
    /// for a vector) you may use #Vec4 for a full multiplication.
    /// @warning undefined behavior if 'this' is not an affine transformation
    inline Vec3 vec_prod(const Vec3& v) const
    {
      return Vec3(
        m[0] * v.x + m[1] * v.y + m[ 2] * v.z,
        m[4] * v.x + m[5] * v.y + m[ 6] * v.z,
        m[8] * v.x + m[9] * v.y + m[10] * v.z);
    }

    inline Transfo operator*(float x) const
    {
      Transfo res;
      for(int i = 0; i < 16; i++)
      {
        res[i] = m[i] * x;
      }
      return res;
    }

    inline Transfo operator+(const Transfo& t) const
    {
      Transfo res;
      for(int i = 0; i < 16; i++)
      {
        res[i] = m[i] + t.m[i];
      }
      return res;
    }

    inline Transfo& operator+=(const Transfo& t)
    {
      (*this) = (*this) + t;
      return (*this);
    }

    inline Transfo operator-(const Transfo& t) const
    {
      Transfo res;
      for(int i = 0; i < 16; i++)
      {
        res[i] = m[i] - t.m[i];
      }
      return res;
    }

    inline Transfo& operator-=(const Transfo& t)
    {
      (*this) = (*this) - t;
      return (*this);
    }

    inline float& operator[](int i)
    {
      return m[i];
    }

    inline const float& operator[](int i) const
    {
      return m[i];
    }

    // -------------------------------------------------------------------------
    /// @name Getters
    // -------------------------------------------------------------------------

    inline Transfo transpose() const
    {
      return Transfo(m[0], m[4], m[ 8], m[12],
        m[1], m[5], m[ 9], m[13],
        m[2], m[6], m[10], m[14],
        m[3], m[7], m[11], m[15]);
    }

    /// Fast inversion of the Transformation matrix. To accelerate computation
    /// we consider that the matrix only represents affine Transformations
    /// such as rotation, scaling, translation, shear... Basically you can
    /// use this procedure only if the last row is equal to (0, 0, 0, 1)
    /// @warning undefined behavior if 'this' is not an affine transformation
    /// don't use this procedure to invert a projection matrix or
    /// anything that is not an affine transformation. Use #full_invert() instead.
    /// @see full_invert()
    inline Transfo fast_invert() const
    {
      Mat3 a(m[0], m[1], m[ 2],
        m[4], m[5], m[ 6],
        m[8], m[9], m[10]);

      Vec3 b(m[3], m[7], m[11]);
      Mat3 x = a.inverse();
      Vec3 y = x * b;
      return Transfo(x.a, x.b, x.c, -y.x,
        x.d, x.e, x.f, -y.y,
        x.g, x.h, x.i, -y.z,
        0.f, 0.f, 0.f,  1.f);
    }

    static inline
      float MINOR(const Transfo& m, const int r0, const int r1, const int r2, const int c0, const int c1, const int c2)
    {
      return m[4*r0+c0] * (m[4*r1+c1] * m[4*r2+c2] - m[4*r2+c1] * m[4*r1+c2]) -
        m[4*r0+c1] * (m[4*r1+c0] * m[4*r2+c2] - m[4*r2+c0] * m[4*r1+c2]) +
        m[4*r0+c2] * (m[4*r1+c0] * m[4*r2+c1] - m[4*r2+c0] * m[4*r1+c1]);
    }

    inline
      Transfo adjoint() const
    {
      return Transfo( MINOR(*this,1,2,3,1,2,3), -MINOR(*this,0,2,3,1,2,3),  MINOR(*this,0,1,3,1,2,3), -MINOR(*this,0,1,2,1,2,3),
        -MINOR(*this,1,2,3,0,2,3),  MINOR(*this,0,2,3,0,2,3), -MINOR(*this,0,1,3,0,2,3),  MINOR(*this,0,1,2,0,2,3),
        MINOR(*this,1,2,3,0,1,3), -MINOR(*this,0,2,3,0,1,3),  MINOR(*this,0,1,3,0,1,3), -MINOR(*this,0,1,2,0,1,3),
        -MINOR(*this,1,2,3,0,1,2),  MINOR(*this,0,2,3,0,1,2), -MINOR(*this,0,1,3,0,1,2),  MINOR(*this,0,1,2,0,1,2));
    }

    inline float det() const
    {
      return m[0] * MINOR(*this, 1, 2, 3, 1, 2, 3) -
        m[1] * MINOR(*this, 1, 2, 3, 0, 2, 3) +
        m[2] * MINOR(*this, 1, 2, 3, 0, 1, 3) -
        m[3] * MINOR(*this, 1, 2, 3, 0, 1, 2);
    }

    /// Full inversion of the Transformation matrix. No assumption is made about
    /// the 4x4 matrix to optimize inversion. if the Transformation is
    /// not affine you MUST use this procedure to invert the matrix. For
    /// instance perspective projection can't use the fast_invert() procedure
    /// @see fast_invert()
    inline Transfo full_invert() const
    {
      return adjoint() * (1.0f / det());
    }

    /// @return the Transformation with normalized x, y, z column vectors
    inline Transfo normalized() const
    {
      return Transfo(get_mat3().normalized(), get_translation());
    }

    /// Check if the vectors representing the frame are orthogonals.
    /// @warning Don't mix up this with orthogonal matrices.
    inline bool is_frame_ortho(float eps = 0.0001f) const
    {
      return fabsf( x().dot( y() ) ) < eps &&
        fabsf( x().dot( z() ) ) < eps &&
        fabsf( y().dot( z() ) ) < eps;
    }

    inline
      void print() const
    {
      printf("%f %f %f %f\n", m[0 ], m[1 ], m[2 ], m[3 ] );
      printf("%f %f %f %f\n", m[4 ], m[5 ], m[6 ], m[7 ] );
      printf("%f %f %f %f\n", m[8 ], m[9 ], m[10], m[11] );
      printf("%f %f %f %f\n", m[12], m[13], m[14], m[15] );
    }

    inline friend
      std::ostream& operator<< ( std::ostream& ofs, const Transfo& tr )
    {
      ofs << tr.m[0 ] << ", " << tr.m[1 ] << ", " << tr.m[2 ] << ", " << tr.m[3 ] << "\n";
      ofs << tr.m[4 ] << ", " << tr.m[5 ] << ", " << tr.m[6 ] << ", " << tr.m[7 ] << "\n";
      ofs << tr.m[8 ] << ", " << tr.m[9 ] << ", " << tr.m[10] << ", " << tr.m[11] << "\n";
      ofs << tr.m[12] << ", " << tr.m[13] << ", " << tr.m[14] << ", " << tr.m[15];
      return ofs;
    }

    // -------------------------------------------------------------------------
    /// @name Static transformation constructors (translation/rotation/scale)
    // -------------------------------------------------------------------------

    static inline Transfo translate(float dx, float dy, float dz)
    {
      return Transfo(1.f, 0.f, 0.f, dx,
        0.f, 1.f, 0.f, dy,
        0.f, 0.f, 1.f, dz,
        0.f, 0.f, 0.f, 1.f);
    }

    static inline Transfo translate(const Vec3& v)
    {
      return Transfo::translate(v.x, v.y, v.z);
    }

    static inline Transfo scale(float sx, float sy, float sz)
    {
      return Transfo( sx,  0.f, 0.f, 0.f,
        0.f,  sy, 0.f, 0.f,
        0.f, 0.f,  sz, 0.f,
        0.f, 0.f, 0.f, 1.f);
    }

    static inline Transfo scale(const Vec3& v)
    {
      return Transfo::scale(v.x, v.y, v.z);
    }

    /// Build a uniform scaling matrix on x,y,z.
    static inline Transfo scale(float s)
    {
      return Transfo::scale(s, s, s);
    }

    /// @return the scale matrix given scale factors in 'v' and
    /// scale origin 'center'
    static inline Transfo scale(const Vec3& center, const Vec3& v)
    {
      Transfo sc = Transfo::scale( v );
      return Transfo::translate(center) * sc * Transfo::translate(-center);
    }

    /// @return the uniform scale matrix given a scale factor 's' and
    /// scale origin 'center'
    static inline Transfo scale(const Vec3& center, float s)
    {
      Transfo sc = Transfo::scale(s);
      return Transfo::translate(center) * sc * Transfo::translate(-center);
    }

    static inline Transfo rotate(const Vec3& center,
      const Vec3& axis,
      float angle,
      const Mat3& frame)
    {
      Transfo r(frame * Mat3::rotate(axis, angle) * frame.inverse());
      return Transfo::translate(center) * r * Transfo::translate(-center);
    }

    /// @return the rotation matrix given an 'axis' rotation 'center' and
    /// 'angle' in radian
    static inline Transfo rotate(const Vec3& center,
      const Vec3& axis,
      float angle)
    {
      Transfo r(Mat3::rotate(axis, angle));
      return Transfo::translate(center) * r * Transfo::translate(-center);
    }

    /// build a rotation matrix around the origin.
    /// @param axis : the <b> normalized </b> axis of rotation
    /// @param angle : rotation's angle in radian
    static inline Transfo rotate(const Vec3& axis, float angle)
    {
      return Transfo(Mat3::rotate(axis, angle));
    }

    static inline Transfo identity()
    {
      return Transfo(1.f, 0.f, 0.f, 0.f,
        0.f, 1.f, 0.f, 0.f,
        0.f, 0.f, 1.f, 0.f,
        0.f, 0.f, 0.f, 1.f);
    }

    static inline Transfo empty()
    {
      return Transfo(0.f, 0.f, 0.f, 0.f,
        0.f, 0.f, 0.f, 0.f,
        0.f, 0.f, 0.f, 0.f,
        0.f, 0.f, 0.f, 0.f);
    }

    /// Given a origin 'org' and a axis 'x_axis' generate the corresponding
    /// 3D frame. Meaning the columns for z and y axis will be
    /// computed to be orthogonal to 'x_axis'.
    static inline Transfo coordinate_system(const Vec3& org, const Vec3& x_axis)
    {
      return Transfo(Mat3::coordinate_system( x_axis), org);
    }

  };

  // END Tbx NAMESPACE ==========================================================
}


// TOOL_BOX_Transfo_HPP__
#endif
