#ifndef TOOL_BOX_VEC3_HPP__
#define TOOL_BOX_VEC3_HPP__

#include <cassert>
#include <cmath>
#include <iostream>
#include <stdlib.h>
#include <stdio.h>

// Dev note: Please try to maintain the other vectors when making changes

// =============================================================================
namespace Tbx
{
  // =============================================================================

  struct Point3;

  /** @brief Vector type compatible GCC and NVCC

    Vec3 can be used at your convenience with either NVCC or GCC.

    @note: the overloading operator '*' between vectors is the component wise
    multiplication. You may use dot() to do a scalar product, cross() for cross
    product and possibly mult() for component wise multiplication.

    @note We forbid implicit conversion to other vector types. This ensure a
    stronger type check. For instance it will avoid converting floats to integers
    by mistake.

    @see Vec2 Vec2i Vec3i Vec4 Vec4i

  */
  struct Vec3
  {

    float x, y, z;

    // -------------------------------------------------------------------------
    /// @name Constructors
    // -------------------------------------------------------------------------

    Vec3() { x = 0.f; y = 0.f; z = 0.f; }

    Vec3(float x_, float y_, float z_) { x = x_; y = y_; z = z_; }

    Vec3(float v) { x = v; y = v; z = v; }

    /// @note implemented in point3.h because of cross definitions
    /// @note explicit constructor to avoid unwanted automatic conversions
    inline explicit Vec3(const Point3& p3);

    static inline Vec3 unit_x()    { return Vec3(1.f, 0.f, 0.f); }
    static inline Vec3 unit_y()    { return Vec3(0.f, 1.f, 0.f); }
    static inline Vec3 unit_z()    { return Vec3(0.f, 0.f, 1.f); }
    static inline Vec3 zero()      { return Vec3(0.f, 0.f, 0.f); }
    static inline Vec3 unit_scale(){ return Vec3(1.f, 1.f, 1.f); }

    inline void set(float x_, float y_, float z_) { x = x_; y = y_; z = z_; }

    static Vec3 random(float r)
    {
      float r2 = 2.f * r;
      float x_ = rand() * 1.f /RAND_MAX;
      float y_ = rand() * 1.f /RAND_MAX;
      float z_ = rand() * 1.f /RAND_MAX;
      return Vec3(x_ * r2 - r, y_ * r2 - r, z_ * r2 - r);
    }

    // -------------------------------------------------------------------------
    /// @name Overload operators
    // -------------------------------------------------------------------------

    // ----------
    // Additions
    // ----------

    Vec3 operator+(const Vec3 &v_) const { return Vec3(x+v_.x, y+v_.y, z+v_.z); }

    Vec3& operator+= (const Vec3 &v_)
    {
      x += v_.x;
      y += v_.y;
      z += v_.z;
      return *this;
    }

    Vec3 operator+(float f_) const { return Vec3(x+f_, y+f_, z+f_); }

    /// lhs scalar cwise addition
    inline friend
      Vec3 operator+(const float d_, const Vec3& vec) { return Vec3(d_+vec.x, d_+vec.y, d_+vec.z); }

    Vec3& operator+= (float f_)
    {
      x += f_;
      y += f_;
      z += f_;
      return *this;
    }

    // -------------
    // Substractions
    // -------------

    Vec3 operator-(const Vec3 &v_) const
    {
      return Vec3(x-v_.x, y-v_.y, z-v_.z);
    }

    Vec3& operator-= (const Vec3& v_)
    {
      x -= v_.x;
      y -= v_.y;
      z -= v_.z;
      return *this;
    }

    /// opposite vector
    Vec3 operator-() const
    {
      return Vec3(-x, -y, -z);
    }

    Vec3 operator-(float f_) const { return Vec3(x-f_, y-f_, z-f_); }

    /// lhs scalar cwise substraction
    inline friend
      Vec3 operator-(const float d_, const Vec3& vec) { return Vec3(d_-vec.x, d_-vec.y, d_-vec.z); }

    Vec3& operator-= (float f_)
    {
      x -= f_;
      y -= f_;
      z -= f_;
      return *this;
    }

    // -------------
    // Comparisons
    // -------------

    bool operator!= (const Vec3 &v_) const
    {
      return (x != v_.x) |  (y != v_.y) | (z != v_.z);
    }

    bool operator==(const Vec3& d_) const
    {
      return (x == d_.x) && (y == d_.y) && (z == d_.z);
    }

    // TODO operator "<" for maps

    // -------------
    // Divisions
    // -------------

    Vec3 operator/(const float d_) const
    {
      return Vec3(x/d_, y/d_, z/d_);
    }

    Vec3& operator/=(const float d_)
    {
      x /= d_;
      y /= d_;
      z /= d_;
      return *this;
    }

    Vec3 operator/(const Vec3 &v_) const
    {
      return Vec3(x/v_.x, y/v_.y, z/v_.z);
    }

    // Should not be defined
    Vec3& operator/=(const Vec3& d_)
    {
      x /= d_.x;
      y /= d_.y;
      z /= d_.z;
      return *this;
    }

    // ----------------
    // Multiplication
    // ----------------

    /// rhs scalar multiplication
    Vec3 operator*(const float d_) const { return Vec3(x*d_, y*d_, z*d_); }

    /// lhs scalar multiplication
    inline friend
      Vec3 operator*(const float d_, const Vec3& vec) { return Vec3(d_*vec.x, d_*vec.y, d_*vec.z); }

    Vec3& operator*=(const float d_)
    {
      x *= d_;
      y *= d_;
      z *= d_;
      return *this;
    }

    Vec3 operator*(const Vec3 &v_) const
    {
      return Vec3(x*v_.x, y*v_.y, z*v_.z);
    }

    // Should not be defined
    Vec3& operator*=(const Vec3& d_)
    {
      x *= d_.x;
      y *= d_.y;
      z *= d_.z;
      return *this;
    }

    // -------------------------------------------------------------------------
    /// @name Operators on vector
    // -------------------------------------------------------------------------

    /// product of all components
    float product() const { return x*y*z; }

    /// sum of all components
    float sum() const { return x+y+z; }

    /// Average all components
    float average() const { return (x+y+z)/3.f; }

    /// semi dot product (component wise multiplication)
    Vec3 mult(const Vec3& v) const
    {
      return Vec3(x*v.x, y*v.y, z*v.z);
    }

    /// component wise division
    Vec3 div(const Vec3& v) const
    {
      return Vec3(x/v.x, y/v.y, z/v.z);
    }

    /// cross product
    Vec3 cross(const Vec3& v_) const
    {
      return Vec3(y*v_.z-z*v_.y, z*v_.x-x*v_.z, x*v_.y-y*v_.x);
    }

    /// dot product
    float dot(const Vec3& v_) const
    {
      return x*v_.x+y*v_.y+z*v_.z;
    }

    /// Compute the cotangente (i.e. 1./tan) between 'this' and v_
    float cotan(const Vec3& v_) const
    {
      // cot(alpha ) = dot(v1, v2) / ||cross(v1, v2)||
      // = ||v1||*||v2||*cos( angle(v1, v2) ) / ||v1||*||v2|| * sin( angle(v1, v2) )
      // = cos( angle(v1, v2) ) / sin( angle(v1, v2) )
      // = 1 / tan( angle(v1, v2) )
      // = cot( angle(v1, v2) ) = cot( alpha )
      return (this->dot(v_)) / (this->cross(v_)).norm();
    }

    /// Signed angle between 'v1' and 'v2'. Vector 'this' is the reference plane
    /// normal. Vectors 'v1' and 'v2' are projected to the reference plane
    /// in order to determine the sign of the angle. Now that we are in the
    /// reference plane if the shortest rotation <b>from</b> v1 <b>to</b> v2
    /// is counter clock wise the angle is positive.
    /// Clockwise rotation is negative
    /// @return signed angle between [-PI; PI] starting from v1 to v2
    float signed_angle(const Vec3& v1, const Vec3& v2) const
    {
      return atan2(  this->dot( v1.cross(v2) ), v1.dot(v2) );
    }

    /// absolute value of the dot product
    float abs_dot(const Vec3& v_) const
    {
      return fabsf(x * v_.x + y * v_.y + z * v_.z);
    }

    /// norm squared
    float norm_squared() const
    {
      return dot(*this);
    }

    /// normalization
    Vec3 normalized() const
    {
      return (*this) * (1.f/sqrtf(norm_squared()));
    }

    /// normalization
    float normalize()
    {
      float l = sqrtf(norm_squared());
      float f = 1.f / l;
      x *= f;
      y *= f;
      z *= f;
      return l;
    }

    /// normalization
    float safe_normalize(const float eps = 1e-10f)
    {
      float l = sqrtf(norm_squared());
      if(l > eps)
      {
        float f = 1.f / l;
        x *= f;
        y *= f;
        z *= f;
        return l;
      }
      else
      {
        x = 1.f;
        y = 0.f;
        z = 0.f;
        return 0.f;
      }
    }

    /// norm
    float norm() const
    {
      return sqrtf(norm_squared());
    }

    /// value of the min coordinate
    float get_min() const
    {
      return fminf(fminf(x,y),z);
    }

    /// value of the max coordinate
    float get_max() const
    {
      return fmaxf(fmaxf(x,y),z);
    }

    /// clamp each vector values
    Vec3 clamp(float min_v, float max_v) const
    {
      return Vec3( fminf( fmaxf(x, min_v), max_v),
        fminf( fmaxf(y, min_v), max_v),
        fminf( fmaxf(z, min_v), max_v));
    }

    /// clamp each vector values
    Vec3 clamp(const Vec3& min_v, const Vec3& max_v) const
    {
      return Vec3( fminf( fmaxf(x, min_v.x), max_v.x),
        fminf( fmaxf(y, min_v.y), max_v.y),
        fminf( fmaxf(z, min_v.z), max_v.z));
    }

    /// floorf every components
    Vec3 floor() const
    {
      return Vec3( floorf(x), floorf(y), floorf(z) );
    }

    /// rotate of 0 step to the left (present for symmetry)
    Vec3 perm_x() const
    {
      return Vec3(x, y, z);
    }

    /// rotate of 1 step to the left (so that y is the first coordinate)
    Vec3 perm_y() const
    {
      return Vec3(y, z, x);
    }

    /// rotate of 2 steps to the left (so that z is the first coordinate)
    Vec3 perm_z() const
    {
      return Vec3(z, x, y);
    }

    /// Given the vector '*this' generate a 3d frame where vectors y_axis and
    /// z_axis are orthogonal to '*this'.
    void coordinate_system (Vec3& y_axis_, Vec3& z_axis) const
    {
      //for numerical stability, and seen that z will
      //always be present, take the greatest component between
      //x and y.
      if( fabsf(x) > fabsf(y) )
      {
        float inv_len = 1.f / sqrtf(x * x + z * z);
        Vec3 tmp(-z * inv_len, 0.f, x * inv_len);
        y_axis_ = tmp;
      }
      else
      {
        float inv_len = 1.f / sqrtf (y * y + z * z);
        Vec3 tmp(0.f, z * inv_len, -y * inv_len);
        y_axis_ = tmp;
      }
      z_axis = (*this).cross (y_axis_);
    }

    /// Get a random orthogonal vector
    Vec3 get_ortho() const
    {
      Vec3 ortho = this->cross(Vec3(1.f, 0.f, 0.f));

      if (ortho.norm_squared() < 1e-06f * 1e-06f)
        ortho = this->cross( Vec3(0.f, 1.f, 0.f) );

      return ortho;
    }

    /// @return the vector to_project projected on the plane defined by the
    /// normal '*this'
    /// @warning don't forget to normalize the vector before calling
    /// proj_on_plane() !
    Vec3 proj_on_plane(const Vec3& to_project) const
    {
      return ( (*this).cross(to_project) ).cross( (*this) );
    }

    /// @return the point to_project projected on the plane defined by the
    /// normal '*this' and passing through pos_plane
    /// @warning don't forget to normalize the vector before calling
    /// proj_on_plane() !
    /// @note implemented in Point3.h because of cross definitions
    inline Point3 proj_on_plane(const Point3& pos_plane,
      const Point3& to_project) const;

    // -------------------------------------------------------------------------
    /// @name Accessors
    // -------------------------------------------------------------------------

    /// @note implemented in Point3.h because of cross definitions
    inline Point3 to_point3() const;

    inline const float& operator[](int i) const
    {
      assert( i < 3);
      return ((float*)this)[i];
    }

    inline float& operator[](int i)
    {
      assert( i < 3);
      return ((float*)this)[i];
    }

    /// Conversion returns the memory address of the vector.
    /// Very convenient to pass a Vec pointer as a parameter to OpenGL:
    /// @code
    /// Vec3 pos, normal;
    /// glNormal3fv(normal);
    /// glVertex3fv(pos);
    /// @endcode
    operator const float*() const { return &x; }

    /// Conversion returns the memory address of the vector. (Non const version)
    operator float*() { return &x; }

    // -------------------------------------------------------------------------
    /// @name Print vector
    // -------------------------------------------------------------------------

    void print() const
    {
      printf("%f, %f, %f\n", x, y, z);
    }

    inline friend
      std::ostream& operator<< ( std::ostream& ofs, const Vec3& v3 )
    {
      ofs << v3.x << ", " << v3.y << ", " << v3.z << "; ";
      return ofs;
    }

  };

  // END Tbx NAMESPACE ==========================================================
}


// TOOL_BOX_VEC3_HPP__
#endif
