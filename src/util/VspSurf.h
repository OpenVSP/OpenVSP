//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// VspSurf.h:
// J.R Gloudemans
//
//////////////////////////////////////////////////////////////////////

#ifndef VSP_SURF_H
#define VSP_SURF_H

#include "Vec3d.h"
#include "Matrix.h"
#include "VspCurve.h"
#include "BndBox.h"

#include "eli/code_eli.hpp"

#include "eli/geom/surface/bezier.hpp"
#include "eli/geom/surface/piecewise.hpp"

typedef eli::geom::surface::bezier<double, 3> surface_patch_type;
typedef eli::geom::surface::piecewise<eli::geom::surface::bezier, double, 3> piecewise_surface_type;

#include <vector>
#include <string>
using std::vector;

struct VspJointInfo
{
public:
    enum {FULL, C0, C1_AUTO, C1, G1, C2_AUTO, C2, G2, NUM_CONTINUITY_TYPES};
    enum {TOP_SIDE, RIGHT_SIDE, BOTTOM_SIDE, LEFT_SIDE, NUM_SIDES};

public:
    VspJointInfo();
    VspJointInfo( const VspJointInfo &ji );
    ~VspJointInfo();

    void SetLeftParams( int side, const double &angle, const double &strength, const double &curvature );
    double GetLeftAngle( int side ) const;
    double GetLeftStrength( int side ) const;
    double GetLeftCurvature( int side ) const;

    void SetRightParams( int side, const double &angle, const double &strength, const double &curvature );
    double GetRightAngle( int side ) const;
    double GetRightStrength( int side ) const;
    double GetRightCurvature( int side ) const;

    void SetState( int state );
    int GetState() const;
    bool C1Continuous() const;
    bool G1Continuous() const;
    bool C2Continuous() const;
    bool G2Continuous() const;
    int DegreesOfFreedom() const;
    bool FiniteDifferenceFp() const;
    bool FiniteDifferenceFpp() const;

private:
    double m_LeftAngle[4];
    double m_LeftStrength[4];
    double m_LeftCurvature[4];
    double m_RightAngle[4];
    double m_RightStrength[4];
    double m_RightCurvature[4];
    int m_State;
};

class VspSurf
{
public:

    VspSurf();
    virtual ~VspSurf();

    // create surface as a body of revolution using the specified curve
    void CreateBodyRevolution( const VspCurve &input_crv );

    // creates piecewise polynomial lofting of curves with user control over slopes and curvatures
    void InterpolateManual( const std::vector<VspCurve> &input_crv_vec, const vector<VspJointInfo> &joint_info_vec, bool closed_flag );

    // creates C0 continuous piecewise line
    void InterpolateLinear( const vector< VspCurve > &input_crv_vec, bool closed_flag );

    // creates C1 continuous Piecewise Cubic Hermite Interpolating Polynomial
    void InterpolatePCHIP( const vector< VspCurve > &input_crv_vec, bool closed_flag );

    // creates C2 continuous piecewise cubic spline polynomial with not-a-knot or closed end conditions
    void InterpolateCSpline( const vector< VspCurve > &input_crv_vec, bool closed_flag );

    int GetNumSectU() const;
    int GetNumSectW() const;

    void ReverseUDirection();
    void ReverseWDirection();
    void SwapUWDirections();
    void Transform( Matrix4d & mat );
    void GetBoundingBox( BndBox &bb ) const;
    bool IsClosedU() const;
    bool IsClosedW() const;

    bool GetFlipNormal() { return m_FlipNormal; }
    void FlipNormal() { m_FlipNormal = !m_FlipNormal; }
    void ResetFlipNormal( ) { m_FlipNormal = false; }

    double FindNearest( double &u, double &w, const vec3d &pt ) const;
    double FindNearest( double &u, double &w, const vec3d &pt, const double &u0, const double &w0 ) const;

    double FindNearest01( double &u, double &w, const vec3d &pt ) const;
    double FindNearest01( double &u, double &w, const vec3d &pt, const double &u0, const double &w0 ) const;

    void GetUConstCurve( VspCurve &c, const double &u ) const;
    void GetWConstCurve( VspCurve &c, const double &w ) const;

    Matrix4d CompRotCoordSys( const double &u, const double &w );
    Matrix4d CompTransCoordSys( const double &u, const double &w );

    void CompJointParams( int joint, VspJointInfo &jointInfo ) const;

    //===== Bezier Funcs ====//
    vec3d CompPnt( double u, double v ) const;
    vec3d CompTanU( double u, double v ) const;
    vec3d CompTanW( double u, double v ) const;
    vec3d CompTanUU( double u, double v ) const;
    vec3d CompTanWW( double u, double v ) const;
    vec3d CompTanUW( double u, double v ) const;

    vec3d CompPnt01( double u, double v ) const;
    vec3d CompTanU01( double u, double v ) const;
    vec3d CompTanW01( double u, double v ) const;
    vec3d CompTanUU01( double u, double v ) const;
    vec3d CompTanWW01( double u, double v ) const;
    vec3d CompTanUW01( double u, double v ) const;

    vec3d CompNorm( double u, double v ) const;
    vec3d CompNorm01( double u, double v ) const;

    void WriteBezFile( FILE* id, const std::string &geom_id );

    //===== Tesselate ====//
    void Tesselate( int num_u, int num_v, std::vector< vector< vec3d > > & pnts,  std::vector< vector< vec3d > > & norms ) const;
    void Tesselate( const vector<int> &num_u, int num_v, std::vector< vector< vec3d > > & pnts,  std::vector< vector< vec3d > > & norms ) const;

protected:
    void PrepairCrossSections( vector<piecewise_curve_type> &pc, const vector<VspCurve> &crv_in ) const;
    void DegreeReduceSections( const vector<VspCurve> &input_crv_vec, bool closed_flag );

protected:

    bool m_FlipNormal;
    piecewise_surface_type m_Surface;
};
#endif
