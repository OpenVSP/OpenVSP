#ifndef MAT3_H_18DCD394_4680_11E9_B573_10FEED04CD1C
#define MAT3_H_18DCD394_4680_11E9_B573_10FEED04CD1C

#include "vec3.h"
#include <cmath>
#include <cassert>
#include <stdlib.h>
#include <stdio.h>

// =============================================================================
namespace Tbx {
  // =============================================================================

  /**
   * @name Mat3
   * @brief Handling 3*3 matrix
   *
   * @see Transfo Vec3
   */
  struct Mat3 {

    ///< first row
    float a, b, c;
    ///< second row
    float d, e, f;
    ///< third row
    float g, h ,i;

    inline Mat3() {   }

    inline   Mat3(float a_, float b_, float c_,
      float d_, float e_, float f_,
      float g_, float h_, float i_)
    {
      a = a_; b = b_; c = c_;
      d = d_; e = e_; f = f_;
      g = g_; h = h_; i = i_;
    }

    inline   Mat3(const Vec3& x,
      const Vec3& y,
      const Vec3& z)
    {
      a = x.x; b = y.x; c = z.x;
      d = x.y; e = y.y; f = z.y;
      g = x.z; h = y.z; i = z.z;
    }

    // -------------------------------------------------------------------------
    /// @name operators overload
    // -------------------------------------------------------------------------

    //----------------
    // Multiplications
    //----------------

    Vec3 operator*(const Vec3& v) const {
      float x = v.x * a + v.y * b + v.z * c;
      float y = v.x * d + v.y * e + v.z * f;
      float z = v.x * g + v.y * h + v.z * i;
      return Vec3(x, y, z);
    }

    inline Mat3 operator*(const Mat3& m) const {
      return Mat3(a * m.a + b * m.d + c * m.g,
        a * m.b + b * m.e + c * m.h,
        a * m.c + b * m.f + c * m.i,
        d * m.a + e * m.d + f * m.g,
        d * m.b + e * m.e + f * m.h,
        d * m.c + e * m.f + f * m.i,
        g * m.a + h * m.d + i * m.g,
        g * m.b + h * m.e + i * m.h,
        g * m.c + h * m.f + i * m.i);
    }

    inline Mat3 operator*(float x) const {
      return Mat3(a * x, b * x, c * x,
        d * x, e * x, f * x,
        g * x, h * x, i * x);
    }

    inline Mat3& operator*=(float x) {
      a *= x; b *= x; c *= x;
      d *= x; e *= x; f *= x;
      g *= x; h *= x; i *= x;
      return *this;
    }

    inline friend Mat3 operator*(const float x_, const Mat3& mat) {
      return Mat3(x_ * mat.a, x_ * mat.b, x_ * mat.c,
        x_ * mat.d, x_ * mat.e, x_ * mat.f,
        x_ * mat.g, x_ * mat.h, x_ * mat.i);
    }

    //----------
    // Divisions
    //----------

    // TODO

    /*
    inline Mat2 operator/(float x) const {
        return Mat2(m[0] / x, m[1] / x,
                    m[2] / x, m[3] / x);
    }

    inline Mat2& operator/=(float x) {
        m[0] /= x; m[1] /= x;
        m[2] /= x; m[3] /= x;
        return *this;
    }

    inline friend
    Mat2 operator/(const float x_, const Mat2& mat) {
        return Mat2(x_ / mat[0], x_ / mat[1],
                    x_ / mat[2], x_ / mat[3]);
    }

    */

    //----------
    // Additions
    //----------

    inline Mat3 operator+(const Mat3& m) const {
      return Mat3(a + m.a, b + m.b, c + m.c,
        d + m.d, e + m.e, f + m.f,
        g + m.g, h + m.h, i + m.i);
    }

    /* TODO
    inline Mat2 operator+(float x) const {
        return Mat2(m[0] + x, m[1] + x,
                    m[2] + x, m[3] + x);
    }

    inline friend
    Mat2 operator+(const float x_, const Mat2& mat) {
        return Mat2(x_ + mat[0], x_ + mat[1],
                    x_ + mat[2], x_ + mat[3]);
    }

    inline Mat2& operator+=(float x) {
        m[0] += x; m[1] += x;
        m[2] += x; m[3] += x;
        return *this;
    }
    */

    //--------------
    // Substractions
    //--------------

    inline Mat3 operator-(const Mat3& m) const {
      return Mat3(a - m.a, b - m.b, c - m.c,
        d - m.d, e - m.e, f - m.f,
        g - m.g, h - m.h, i - m.i);
    }

    /*
    inline Mat2 operator-() const {
        return Mat2(-m[0], -m[1],
                    -m[2], -m[3]);
    }

    inline Mat2 operator-(float x) const {
        return Mat2(m[0] - x, m[1] - x,
                    m[2] - x, m[3] - x);
    }

    inline friend Mat2 operator-(const float x_, const Mat2& mat) {
        return Mat2(x_ - mat[0], x_ - mat[1],
                    x_ - mat[2], x_ - mat[3]);
    }

    inline Mat2& operator-=(float x) {
        m[0] -= x; m[1] -= x;
        m[2] -= x; m[3] -= x;
        return *this;
    }
    */

    // -------------------------------------------------------------------------
    /// @name Accessors
    // -------------------------------------------------------------------------

    //----------------
    // Access elements
    //----------------

    inline const float& operator()(int row, int column) const {
      assert(row >= 0 && row < 3);
      assert(column >= 0 && column < 3);
      return ((float*)(this))[column + row*3];
    }

    inline float& operator()(int row, int column) {
      assert(row >= 0 && row < 3);
      assert(column >= 0 && column < 3);
      return ((float*)(this))[column + row*3];
    }

    inline float& operator[](int idx) {
      assert(idx >= 0 && idx < 9);
      return ((float*)(this))[idx];
    }

    inline const float& operator[](int idx) const {
      assert(idx >= 0 && idx < 9);
      return ((float*)(this))[idx];
    }

    // -------------------------------------------------------------------------
    /// @name operations
    // -------------------------------------------------------------------------

    inline float det() const {
      return a * ( e * i - f * h) - b * (d * i - f * g) + c * (d * h - e * g);
    }

    /// @return the matrix with normalized x, y, z column vectors
    inline Mat3 normalized() const {
      return Mat3(x().normalized(), y().normalized(), z().normalized());
    }

    inline Mat3 inverse() const {
      float c0 = e * i - f * h;
      float c1 = f * g - d * i;
      float c2 = d * h - e * g;
      float idet = 1.f / (a * c0 + b * c1 + c * c2);
      return Mat3(c0 , c * h - b * i, b * f - c * e,
        c1 , a * i - c * g, c * d - a * f,
        c2 , b * g - a * h, a * e - b * d) * idet;
    }

    inline Mat3 transpose() const {
      return Mat3(a, d, g, b, e, h, c, f, i);
    }

    inline void set_abs() {
      a = fabs(a); b = fabs(b); c = fabs(c);
      d = fabs(d); e = fabs(e); f = fabs(f);
      g = fabs(g); h = fabs(h); i = fabs(i);
    }

    inline float max_elt() const {
      return fmaxf(i, fmaxf(fmaxf(fmaxf(a,b),fmaxf(c,d)),
        fmaxf(fmaxf(e,f),fmaxf(g,h))));
    }

    inline float min_elt() const {
      return fminf(i, fminf(fminf(fminf(a,b),fminf(c,d)),
        fminf(fminf(e,f),fminf(g,h))));
    }

    Mat3 get_ortho() const {
      Mat3 h0 = (*this);
      Mat3 h1 = h0;
      h1.set_abs();
      float eps =(1.f +  h1.min_elt()) * 1e-5f;
      for(int i = 0; i < 500/* to avoid infinite loop */; i++){
      h0 = (h0 + (h0.inverse()).transpose()) * 0.5f;
      h1 = h1 - h0;
      h1.set_abs();
      if(h1.max_elt() <= eps) {
        break;
      }
      h1 = h0;
    }
    return h0;
  }

  float get_rotation_axis_angle(Vec3& axis) const {
    axis.x = h - f + 1e-5f;
    axis.y = c - g;
    axis.z = d - b;
    float sin_angle = axis.safe_normalize();
    float cos_angle = a + e + i - 1.f;
    return atan2(sin_angle, cos_angle);
  }

  inline Vec3 x() const { return Vec3(a, d, g); }

  inline Vec3 y() const { return Vec3(b, e, h); }

  inline Vec3 z() const { return Vec3(c, f, i); }

  //--------------------------------------------------------------------------
  /// @name Static constructors
  //--------------------------------------------------------------------------

  static inline Mat3 identity() {
    return Mat3(1.f, 0.f, 0.f,
      0.f, 1.f, 0.f,
      0.f, 0.f, 1.f);

  }

  /// @return the rotation matrix given 'axis' and 'angle' in radian
  static inline Mat3 rotate(const Vec3& axis, float angle) {
    Vec3 n = axis;
    n.normalize();
    float cp = cosf(angle);
    float sp = sinf(angle);
    float acp = 1.f - cp;
    float nxz = n.x * n.z;
    float nxy = n.x * n.y;
    float nyz = n.y * n.z;
    return Mat3(cp + acp * n.x * n.x,
      acp * nxy - sp * n.z,
      acp * nxz + sp * n.y,

      acp * nxy + sp * n.z,
      cp + acp * n.y * n.y,
      acp * nyz - sp * n.x,

      acp * nxz - sp * n.y,
      acp * nyz + sp * n.x,
      cp + acp * n.z * n.z);
  }

  /// @return a orthogonal/normalized frame with its x axis aligned to x_axis
  static inline Mat3 coordinate_system(const Vec3& x_axis) {
    Vec3 fx, fy, fz;
    fx = x_axis.normalized();
    fx.coordinate_system(fy, fz);
    return Mat3(fx, fy, fz);
  }

  //--------------------------------------------------------------------------
  /// @name Print matrix
  //--------------------------------------------------------------------------

  inline void print() const {
    printf("%f %f %f \n", a, b, c );
    printf("%f %f %f \n", d, e, f );
    printf("%f %f %f \n", g, h, i );
  }

  inline friend std::ostream& operator<< ( std::ostream& ofs, const Mat3& tr ) {
    ofs << tr.a << ", " << tr.b << ", " << tr.c << "\n";
    ofs << tr.d << ", " << tr.e << ", " << tr.f << "\n";
    ofs << tr.g << ", " << tr.h << ", " << tr.i;
    return ofs;
  }

};

// END Tbx NAMESPACE ==========================================================
}

#endif // MAT3_H_18DCD394_4680_11E9_B573_10FEED04CD1C
