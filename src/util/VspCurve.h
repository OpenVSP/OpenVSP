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

#include "Matrix4d.h"
#include "Vec3d.h"
#include "BndBox.h"

#include <vector>
using std::vector;

#include "eli/code_eli.hpp"

#include "eli/geom/curve/bezier.hpp"
#include "eli/geom/curve/piecewise.hpp"

typedef eli::geom::curve::bezier<double, 3> curve_segment_type;
typedef eli::geom::curve::piecewise<eli::geom::curve::bezier, double, 3> piecewise_curve_type;

#define TMAGIC 0.004

struct BezierSegment
{
    int order;
    double t0;
    double tmax;
    vector < vec3d > control_pnt_vec;
};

class VspCurve
{
public:

    VspCurve();
    virtual ~VspCurve();

    void Copy( VspCurve & input_crv );
    void Split( double u );
    void Append( VspCurve & input_crv ); // Append Curve

    void Spin( double u );
    void Spin01( double u );

    bool IsClosed() const;

    bool RoundJoint( double rad, int i );
    bool RoundJoint( double rad, double u );
    bool RoundJoints( double rad, vector < double > uvec );

    void Modify( int type, bool le, double len, double off, double str );

    // creates C0 continuous piecewise line
    void InterpolateLinear( vector< vec3d > & input_pnt_vec, const vector<double> &param, bool closed_flag );

    // creates C1 continuous Piecewise Cubic Hermite Interpolating Polynomial
    void InterpolatePCHIP( vector< vec3d > & input_pnt_vec, const vector<double> &param, bool closed_flag );
    void InterpolateEqArcLenPCHIP( const piecewise_curve_type &c );

    // creates C2 continuous piecewise cubic spline polynomial with not-a-knot or closed end conditions
    void InterpolateCSpline( vector< vec3d > & input_pnt_vec, const vector<double> &param, bool closed_flag );

    // creates C2 continuous piecewise cubic spline polynomial with clamped end slopes
    void InterpolateCSpline( vector< vec3d > & input_pnt_vec, const vec3d &start_slope, const vec3d &end_slope, const vector<double> &param );

    void ToBinaryCubic( bool wingtype, double ttol = 1e-6, double atol = 0.01, int dmin = 2, int dmax = 12 );

    void SetCubicControlPoints( const vector< vec3d > & cntrl_pts ); // Automatic curve parameterization
    void SetCubicControlPoints( const vector < vec3d > & cntrl_pts, const vector < double > & param ); // Specify curve parameterization
    void GetCubicControlPoints( vector< vec3d >& cntrl_pts, vector< double >& param );

    void GetLinearControlPoints( vector< vec3d >& cntrl_pts, vector< double >& param );

    const piecewise_curve_type & GetCurve() const;
    void SetCurve( const piecewise_curve_type &c );
    void AppendCurveSegment( curve_segment_type &c );

    double FindDistant( double &u, const vec3d &pt, const double &d, const double &u0 ) const;
    double FindDistant( double &u, const vec3d &pt, const double &d ) const;
    double FindDistant( double &u, const vec3d &pt, const double &d, const double &umin, const double &umax ) const;
    double FindThickness( double &u1, double &u2, const vec3d &pt, const double &thick, const double &u10, const double &u20 ) const;

    double FindNearest( double &u, const vec3d &pt ) const;
    double FindNearest( double &u, const vec3d &pt, const double &u0 ) const;

    double FindNearest01( double &u, const vec3d &pt ) const;
    double FindNearest01( double &u, const vec3d &pt, const double &u0 ) const;

    double FindMinimumDimension( double &u, const int &idim, const double &u0 ) const;
    double FindMinimumDimension( double &u, const int &idim ) const;

    //===== Bezier Funcs ====//
    vec3d CompPnt( double u ) const;
    vec3d CompTan( double u ) const;
    vec3d CompTan( double u, int sideflag ) const;
    vec3d CompNorm( double u ) const;
    vec3d CompNorm( double u, int sideflag ) const;

    double CompCurve( double u ) const;
    double CompCurve( double u, int sideflag ) const;

    enum { BEFORE, AFTER };

    vec3d CompPnt01( double u ) const;
    vec3d CompTan01( double u ) const;

    double CompLength( double tol = 1e-6 ) const;

    double CompArea( int idir, int jdir ) const;

    //===== Tesselate ====//
    void TesselateNoCorner( int num_pnts_u, double umin, double umax, vector< vec3d > & output, vector< double > &uout );
    void Tesselate( const vector< double > &u, vector< vec3d > & output );

    void TessAdapt( vector< vec3d > & output, double tol, int Nlimit );
    void TessAdapt( double umin, double umax, std::vector< vec3d > & pnts, double tol, int Nlimit );
    void TessAdapt( double umin, double umax, const vec3d & pmin, const vec3d & pmax, std::vector< vec3d > & pnts, double tol, int Nlimit, int Nadapt = 0 );

    void Offset( const vec3d &offvec );
    void OffsetX( double x );
    void OffsetY( double y );
    void OffsetZ( double Z );

    void ProjectOntoCylinder( double r, bool wingtype, double ttol = 1e-6, double atol = 0.01, int dmin = 2, int dmax =  15 );

    void RotateX( double ang );
    void RotateY( double ang );
    void RotateZ( double ang );

    void Transform( Matrix4d & mat );

    void Scale( double s );

    void ScaleX( double s );
    void ScaleY( double s );
    void ScaleZ( double s );

    void ZeroI( int i );

    void ReflectXY();
    void ReflectXZ();
    void ReflectYZ();
    void Reflect( const vec3d &axis );
    void Reflect( const vec3d &axis, double d );

    void Reverse();

    bool IsEqual( const VspCurve & crv );

    void GetBoundingBox( BndBox &bb ) const;

    double CalculateThick( double &loc ) const;
    void MatchThick( const double & ttarget );

    double Angle( const double & u1, const int &dir1, const double & u2, const int &dir2, const bool & flipflag ) const;

    vector < BezierSegment > GetBezierSegments();

    void CreateRoundedRectangle( double w, double h, double k, double sk, double vsk, const double & r1, const double & r2, const double & r3, const double & r4, bool keycorner = true );

    void ToCubic( double tol = 0.1 );

protected:
    piecewise_curve_type m_Curve;

    int GetNumSections() const;
    void GetCurveSegment( curve_segment_type &c, int i ) const;
    double GetCurveDt( int i ) const;
};


#endif


