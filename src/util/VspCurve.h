//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// VspCurve.h:
// J.R Gloudemans
//
//////////////////////////////////////////////////////////////////////

#ifndef VSP_CURVE_H
#define VSP_CURVE_H

#include "Matrix.h"
#include "Vec3d.h"
#include "BndBox.h"

#include <vector>
using std::vector;

#include "eli/code_eli.hpp"

#include "eli/geom/curve/bezier.hpp"
#include "eli/geom/curve/piecewise.hpp"

typedef eli::geom::curve::bezier<double, 3> curve_segment_type;
typedef eli::geom::curve::piecewise<eli::geom::curve::bezier, double, 3> piecewise_curve_type;

class VspCurve
{
public:

    VspCurve();
    virtual ~VspCurve();

    void Copy( VspCurve & input_crv );
    void Split( double u );
    void Append( VspCurve & input_crv ); // Append Curve

    bool IsClosed() const;

    void RoundJoint( double rad, int i );
    void RoundAllJoints( double rad );

    // creates C0 continuous piecewise line
    void InterpolateLinear( vector< vec3d > & input_pnt_vec, const vector<double> &param, bool closed_flag );

    // creates C1 continuous Piecewise Cubic Hermite Interpolating Polynomial
    void InterpolatePCHIP( vector< vec3d > & input_pnt_vec, const vector<double> &param, bool closed_flag );

    // creates C2 continuous piecewise cubic spline polynomial with not-a-knot or closed end conditions
    void InterpolateCSpline( vector< vec3d > & input_pnt_vec, const vector<double> &param, bool closed_flag );

    // creates C2 continuous piecewise cubic spline polynomial with clamped end slopes
    void InterpolateCSpline( vector< vec3d > & input_pnt_vec, const vec3d &start_slope, const vec3d &end_slope, const vector<double> &param );

    void SetCubicControlPoints( const vector< vec3d > & cntrl_pts, bool closed_flag );

    const piecewise_curve_type & GetCurve() const;
    void SetCurve( const piecewise_curve_type &c );
    void AppendCurveSegment( curve_segment_type &c );

    double FindDistant( double &u, const vec3d &pt, const double &d, const double &u0 ) const;

    double FindNearest( double &u, const vec3d &pt ) const;
    double FindNearest( double &u, const vec3d &pt, const double &u0 ) const;

    double FindNearest01( double &u, const vec3d &pt ) const;
    double FindNearest01( double &u, const vec3d &pt, const double &u0 ) const;

    //===== Bezier Funcs ====//
    vec3d CompPnt( double u );
    vec3d CompTan( double u );

    vec3d CompPnt01( double u );
    vec3d CompTan01( double u );

    double CompLength( double tol = 1e-6 );

    //===== Tesselate ====//
    void Tesselate( int num_pnts_u, vector< vec3d > & output );

    void Offset( vec3d offvec );
    void OffsetX( double x );
    void OffsetY( double y );
    void OffsetZ( double Z );

    void RotateX( double ang );
    void RotateY( double ang );
    void RotateZ( double ang );

    void Transform( Matrix4d & mat );

    void ReflectXY();
    void ReflectXZ();
    void ReflectYZ();
    void Reflect( vec3d axis );
    void Reflect( vec3d axis, double d );

    void Reverse();

    bool IsEqual( const VspCurve & crv );

    void GetBoundingBox( BndBox &bb ) const;

protected:
    piecewise_curve_type m_Curve;

    int GetNumSections() const;
    void GetCurveSegment( curve_segment_type &c, int i ) const;
    double GetCurveDt( int i ) const;
};


#endif


