//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

//////////////////////////////////////////////////////////////////////
// IsectAdapt.h
//
// Binary adaptation of an intersection curve between two surfaces, from the raw points
// the intersection found.  The adapted points lie on both surfaces, and the curve through
// them is either a polyline or piecewise cubic, each cubic segment built from the points
// and tangents at its ends.  The tangent at a point is the direction both surfaces share
// there: the cross product of their normals.
//
//////////////////////////////////////////////////////////////////////

#if !defined(ISECTADAPT__INCLUDED_)
#define ISECTADAPT__INCLUDED_

#include "Vec3d.h"
#include "BezierCurve.h"

#include <vector>

class Surf;

// One point of an adapted intersection curve.
struct IsectAdaptPnt
{
    IsectAdaptPnt()
    {
        m_T = 0.0;
        m_TanOK = false;
        m_Straight = false;
    }

    // Parameter along the raw intersection
    double m_T;

    vec3d m_Pnt;

    // Parameters on each surface, u and w in x and y
    vec3d m_UW[2];

    // Unit tangent in space and the matching rates of change of each surface's parameters,
    // all per unit length along the curve, leaving the point and arriving at it.  The two
    // differ where the point is on a patch break of a surface.  Only meaningful where m_TanOK.
    vec3d m_Tan;
    vec3d m_UWTan[2];
    vec3d m_UWTanIn[2];
    bool m_TanOK;

    // The segment from this point to the next is straight, in space and in each surface's
    // parameters.  Where the curve turns a corner no cubic fits, but a short chord does.
    bool m_Straight;
};

class IsectAdaptCurve
{
public:

    // Adapt the curve the raw points uwcrvA and uwcrvB describe on surfA and surfB.
    //
    // refine     put every point the adaptation adds on both surfaces
    // cubic      make each segment a cubic from the points and tangents at its ends
    // rel_tol    bound on how far the curve may stray, as a fraction of a segment's length;
    //            zero for none
    // abs_tol    bound on how far it may stray outright; zero for none
    //
    // How far the curve strays is measured along each segment from the intersection, and for
    // a cubic curve, which is written with its pcurves, also from each surface's image of the
    // segment in its own parameters.  Unrefined, the points are on the first surface only,
    // and it alone is measured.
    void Adapt( const Bezier_curve &uwcrvA, const Bezier_curve &uwcrvB, const Surf &surfA, const Surf &surfB,
                bool refine, bool cubic, double rel_tol, double abs_tol, int Nlimit = 16 );

    // The adapted curve as a piecewise Bezier curve of degree 1 or 3: control points in
    // space, and in each surface's parameters, sharing the points where segments meet, with
    // the parameter at each segment end.
    void GetBezier( std::vector < vec3d > &pnts, std::vector < vec3d > &uw_a, std::vector < vec3d > &uw_b,
                    std::vector < double > &breaks, int &deg ) const;

    std::vector < IsectAdaptPnt > m_Pnts;

    // Where the surfaces do not meet to within the tolerance, so no point of the curve was
    // found on both: how many segments were kept that way, and the widest gap among them
    int m_NumMiss;
    double m_MaxMiss;

    // Segments kept over the tolerance because they ran out of halvings, and the worst of them
    int m_NumLimit;
    double m_MaxLimitErr;

protected:

    void AdaptSeg( const IsectAdaptPnt &p0, const IsectAdaptPnt &p1, int Nlimit );

    // The points between p0 and p1 where the curve crosses a patch break of either surface, on
    // both surfaces with the parameter exactly on the break, in order along the curve
    void FindBreakPnts( const IsectAdaptPnt &p0, const IsectAdaptPnt &p1, std::vector < IsectAdaptPnt > &brk ) const;

    // Which way the raw intersection runs at parameter t, from the nearest raw points either
    // side of it more than m_DirTol apart
    vec3d RawDir( double t ) const;

    // Fill in a point's tangents from the surfaces' normals, oriented along dir
    void SetTangent( IsectAdaptPnt &p, const vec3d &dir ) const;

    // Hermite control points of a segment from p0 to p1: in space, and in surface isurf's
    // parameters when isurf is 0 or 1.  A straight segment follows its chord.
    void SegCtrl( const IsectAdaptPnt &p0, const IsectAdaptPnt &p1, int isurf, bool straight, vec3d cp[4] ) const;

    // How many surfaces' parameters a segment is written in, and so measured in
    int NumPCurves() const;

    // Whether each surface's image of a segment in its own parameters stays within the surface
    bool SegInDomain( const IsectAdaptPnt &p0, const IsectAdaptPnt &p1, bool straight ) const;

    // How far a segment strays from pm, the curve's point nearest its middle, and each
    // surface's image of it from it
    double SegErr( const IsectAdaptPnt &p0, const IsectAdaptPnt &p1, const IsectAdaptPnt &pm, bool straight ) const;

    // How far apart the two surfaces are at a point's parameters on each
    double SurfGap( const IsectAdaptPnt &p ) const;

    const Bezier_curve *m_CrvA;
    const Bezier_curve *m_CrvB;

    // Parameter range of the raw curves, and the ends of their segments
    double m_T0;
    double m_T1;
    std::vector < double > m_RawT;
    const Surf *m_SurfA;
    const Surf *m_SurfB;

    bool m_Refine;
    bool m_Cubic;
    double m_RelTol;
    double m_AbsTol;
    double m_DirTol;
    double m_ResTol;
};

#endif
