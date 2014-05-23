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

#include <vector>
using std::vector;

#include "eli/code_eli.hpp"

#include "eli/geom/curve/bezier.hpp"
#include "eli/geom/curve/piecewise.hpp"

typedef eli::geom::curve::bezier<double, 3> curve_segment_type;
typedef eli::geom::curve::piecewise<eli::geom::curve::bezier, double, 3> piecewise_curve_type;

class VspPntData
{
public:
    VspPntData();
    VspPntData( int type );

    enum { NORMAL, SHARP, ZERO, ZERO_X, ZERO_Y, ZERO_Z, ONLY_BACK,
           ONLY_FORWARD, PREDICT
         };

    int m_Type;
    bool m_UseTan1;
    vec3d m_Tan1;
    bool m_UseTan2;
    vec3d m_Tan2;

    bool m_UseTanStr1;
    double m_TanStr1;
    bool m_UseTanStr2;
    double m_TanStr2;

    void SetType( int t )
    {
        m_Type = t;
    }
    void SetTan1( const vec3d & t )
    {
        m_UseTan1 = true;
        m_Tan1 = t;
    }
    void SetTan2( const vec3d & t )
    {
        m_UseTan2 = true;
        m_Tan2 = t;
    }
    void SetTanStr1( double & s )
    {
        m_UseTanStr1 = true;
        m_TanStr1 = s;
    }
    void SetTanStr2( double & s )
    {
        m_UseTanStr2 = true;
        m_TanStr2 = s;
    }
};



class VspCurveInfo
{
public:
    enum {SET_POINT, SET_1ST_DERIVATIVE, SET_2ND_DERIVATIVE, NUM_SETS};

public:
    VspCurveInfo();
    VspCurveInfo( const VspCurveInfo &ci );
    ~VspCurveInfo();

    void SetEndType( int t );
    int GetEndType() const;
    bool UseEnd1stDerivative() const;
    bool UseEnd2ndDerivative() const;

    void SetStartType( int t );
    int GetStartType() const;
    bool UseStart1stDerivative() const;
    bool UseStart2ndDerivative() const;

    void SetPoint( const vec3d &p );
    vec3d GetPoint() const;

    void SetEndData( const vec3d &lp, const vec3d &lpp );
    void SetEnd1stDerivative( const vec3d &lp );
    vec3d GetEnd1stDerivative() const;
    void SetEnd2ndDerivative( const vec3d &lpp );
    vec3d GetEnd2ndDerivative() const;

    void SetStartData( const vec3d &rp, const vec3d &rpp );
    void SetStart1stDerivative( const vec3d &rp );
    vec3d GetStart1stDerivative() const;
    void SetStart2ndDerivative( const vec3d &rpp );
    vec3d GetStart2ndDerivative() const;

    void SetParameter( double u );
    double GetParameter() const;

private:
    double m_U;
    int m_EndType, m_StartType;
    vec3d m_Point, m_End1stDerivative, m_End2ndDerivative, m_Start1stDerivative, m_Start2ndDerivative;
};

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

    // creates C1 continuous piecewise polynomial with user control over slopes
    void InterpolateManual( const vector< VspCurveInfo > & curve_info, bool closed_flag );

    // creates C0 continuous piecewise line
    void InterpolateLinear( vector< vec3d > & input_pnt_vec, const vector<double> &param, bool closed_flag );

    // creates C1 continuous Piecewise Cubic Hermite Interpolating Polynomial
    void InterpolatePCHIP( vector< vec3d > & input_pnt_vec, const vector<double> &param, bool closed_flag );

    // creates C2 continuous piecewise cubic spline polynomial with not-a-knot or closed end conditions
    void InterpolateCSpline( vector< vec3d > & input_pnt_vec, const vector<double> &param, bool closed_flag );

    // creates C2 continuous piecewise cubic spline polynomial with clamped end slopes
    void InterpolateCSpline( vector< vec3d > & input_pnt_vec, const vec3d &start_slope, const vec3d &end_slope, const vector<double> &param );

    void SetCubicControlPoints( const vector< vec3d > & cntrl_pts, bool closed_flag );

    int GetNumSections() const;

    const piecewise_curve_type & GetCurve() const;
    void SetCurve( const piecewise_curve_type &c );
    void GetCurveSegment( curve_segment_type &c, int i ) const;
    double GetCurveDt( int i ) const;
    void AppendCurveSegment( curve_segment_type &c );

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

protected:
    piecewise_curve_type m_Curve;
};


#endif


