//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// IsectAdapt.cpp
//
//////////////////////////////////////////////////////////////////////

#include "IsectAdapt.h"
#include "IntersectPatch.h"
#include "Surf.h"

#include <algorithm>
#include <cmath>

// Below this sine of the angle between the two surfaces, they are too near parallel for the
// cross product of their normals to give the curve's direction.
static const double TAN_MIN_SIN = 0.05;

// Below this ratio of a surface's squared rates of change in its two parameters, it is too
// near collapsing for the rates of its parameters along the curve to be followed.
static const double TAN_MIN_SCALE = 1.0e-6;

// Parameters within the surface, as every evaluation of it needs them.
static vec3d ClampUW( const Surf &surf, const vec3d &uw )
{
    const SurfCore *core = surf.GetSurfCore();

    double u = std::min( std::max( uw.x(), core->GetMinU() ), core->GetMaxU() );
    double w = std::min( std::max( uw.y(), core->GetMinW() ), core->GetMaxW() );

    return vec3d( u, w, 0.0 );
}

static vec3d SurfPnt( const Surf &surf, const vec3d &uw )
{
    return surf.CompPnt( uw.x(), uw.y() );
}

// Parameters within the surface, give or take rounding
static bool InDomain( const Surf &surf, const vec3d &uw )
{
    const SurfCore *core = surf.GetSurfCore();

    double du = 1.0e-9 * ( core->GetMaxU() - core->GetMinU() );
    double dw = 1.0e-9 * ( core->GetMaxW() - core->GetMinW() );

    return uw.x() >= core->GetMinU() - du && uw.x() <= core->GetMaxU() + du &&
           uw.y() >= core->GetMinW() - dw && uw.y() <= core->GetMaxW() + dw;
}

// The rate of change of a surface's parameters that moves along the unit direction t, from
// its tangent plane at uw.  False where the surface is too near collapsing to follow.
static bool UWRate( const Surf &surf, const vec3d &uw, const vec3d &t, vec3d &rate )
{
    vec3d su = surf.GetSurfCore()->CompTanU( uw.x(), uw.y() );
    vec3d sw = surf.GetSurfCore()->CompTanW( uw.x(), uw.y() );

    double a = dot( su, su );
    double b = dot( su, sw );
    double c = dot( sw, sw );
    double det = a * c - b * b;

    if ( !( det > 1.0e-12 * a * c ) || a < TAN_MIN_SCALE * c || c < TAN_MIN_SCALE * a )
    {
        return false;
    }

    double ru = dot( su, t );
    double rw = dot( sw, t );

    rate = vec3d( ( c * ru - b * rw ) / det, ( a * rw - b * ru ) / det, 0.0 );
    return true;
}

// uw moved the smallest step there is in each parameter, with the rate dir or against it, and
// kept within the surface
static vec3d StepUW( const Surf &surf, const vec3d &uw, const vec3d &dir, double sign )
{
    double u = uw.x();
    double w = uw.y();

    if ( sign * dir.x() > 0.0 )
    {
        u = std::nextafter( u, HUGE_VAL );
    }
    else if ( sign * dir.x() < 0.0 )
    {
        u = std::nextafter( u, -HUGE_VAL );
    }

    if ( sign * dir.y() > 0.0 )
    {
        w = std::nextafter( w, HUGE_VAL );
    }
    else if ( sign * dir.y() < 0.0 )
    {
        w = std::nextafter( w, -HUGE_VAL );
    }

    return ClampUW( surf, vec3d( u, w, 0.0 ) );
}

vec3d IsectAdaptCurve::RawDir( double t ) const
{
    double t0 = m_T0;
    double t1 = m_T1;

    vec3d dir;
    for ( double dt = 1.0e-3 * ( t1 - t0 ); dt < 2.0 * ( t1 - t0 ); dt *= 2.0 )
    {
        double ta = std::max( t - dt, t0 );
        double tb = std::min( t + dt, t1 );

        dir = SurfPnt( *m_SurfA, m_CrvA->CompPnt( tb ) ) - SurfPnt( *m_SurfA, m_CrvA->CompPnt( ta ) );

        if ( dir.mag() > m_DirTol )
        {
            break;
        }
    }
    return dir;
}

void IsectAdaptCurve::SetTangent( IsectAdaptPnt &p, const vec3d &dir ) const
{
    p.m_TanOK = false;

    double ldir = dir.mag();

    if ( m_SurfA == m_SurfB || ldir <= 0.0 )
    {
        return;
    }

    vec3d uwa = ClampUW( *m_SurfA, p.m_UW[0] );
    vec3d uwb = ClampUW( *m_SurfB, p.m_UW[1] );

    vec3d na = m_SurfA->CompNorm( uwa.x(), uwa.y() );
    vec3d nb = m_SurfB->CompNorm( uwb.x(), uwb.y() );

    double lna = na.mag();
    double lnb = nb.mag();

    if ( lna <= 0.0 || lnb <= 0.0 )
    {
        return;
    }

    vec3d t = cross( na, nb ) / ( lna * lnb );
    double len = t.mag();

    if ( len < TAN_MIN_SIN )
    {
        return;
    }

    t = t / len;

    // A direction the surfaces share that runs across the curve is not the curve's
    double cosdir = dot( t, dir ) / ldir;

    if ( std::abs( cosdir ) < TAN_MIN_SIN )
    {
        return;
    }

    if ( cosdir < 0.0 )
    {
        t = t * -1.0;
    }

    // The rate of change of each surface's parameters that moves along t, on each side of the
    // point.  Across a patch break of the surface its parameters change at another rate, so
    // leaving a point on one the rate is the next patch's, and arriving, the one before.
    const Surf *surfs[2] = { m_SurfA, m_SurfB };
    const vec3d uws[2] = { uwa, uwb };

    for ( int i = 0; i < 2; i++ )
    {
        vec3d rate;
        if ( !UWRate( *surfs[i], uws[i], t, rate ) )
        {
            return;
        }

        if ( !UWRate( *surfs[i], StepUW( *surfs[i], uws[i], rate, 1.0 ), t, p.m_UWTan[i] ) ||
             !UWRate( *surfs[i], StepUW( *surfs[i], uws[i], rate, -1.0 ), t, p.m_UWTanIn[i] ) )
        {
            return;
        }
    }

    p.m_Tan = t;
    p.m_TanOK = true;
}

void IsectAdaptCurve::SegCtrl( const IsectAdaptPnt &p0, const IsectAdaptPnt &p1, int isurf, bool straight, vec3d cp[4] ) const
{
    vec3d q0, q1, m0, m1;

    double ds = dist( p0.m_Pnt, p1.m_Pnt );

    if ( isurf < 0 )
    {
        q0 = p0.m_Pnt;
        q1 = p1.m_Pnt;
    }
    else
    {
        q0 = p0.m_UW[ isurf ];
        q1 = p1.m_UW[ isurf ];
    }

    vec3d chord;
    if ( ds > 0.0 )
    {
        chord = ( q1 - q0 ) / ds;
    }

    // An end with no tangent, or a polyline, takes the chord's
    m0 = chord;
    m1 = chord;

    if ( m_Cubic && !straight && p0.m_TanOK )
    {
        if ( isurf < 0 )
        {
            m0 = p0.m_Tan;
        }
        else
        {
            m0 = p0.m_UWTan[ isurf ];
        }
    }

    if ( m_Cubic && !straight && p1.m_TanOK )
    {
        if ( isurf < 0 )
        {
            m1 = p1.m_Tan;
        }
        else
        {
            m1 = p1.m_UWTanIn[ isurf ];
        }
    }

    cp[0] = q0;
    cp[1] = q0 + m0 * ( ds / 3.0 );
    cp[2] = q1 - m1 * ( ds / 3.0 );
    cp[3] = q1;
}

static vec3d BezierPnt( const vec3d cp[4], double t )
{
    double mt = 1.0 - t;
    return cp[0] * ( mt * mt * mt ) + cp[1] * ( 3.0 * mt * mt * t ) + cp[2] * ( 3.0 * mt * t * t ) + cp[3] * ( t * t * t );
}

int IsectAdaptCurve::NumPCurves() const
{
    // Only a cubic curve is written with its pcurves, and only a refined one is on the second
    // surface to measure against.  A polyline is only ever used in space.
    if ( !m_Cubic )
    {
        return 0;
    }
    if ( m_Refine )
    {
        return 2;
    }
    return 1;
}

bool IsectAdaptCurve::SegInDomain( const IsectAdaptPnt &p0, const IsectAdaptPnt &p1, bool straight ) const
{
    // A Bezier segment lies within its control points
    const Surf *surfs[2] = { m_SurfA, m_SurfB };

    for ( int i = 0; i < NumPCurves(); i++ )
    {
        vec3d uwcp[4];
        SegCtrl( p0, p1, i, straight, uwcp );

        for ( int k = 0; k < 4; k++ )
        {
            if ( !InDomain( *surfs[i], uwcp[k] ) )
            {
                return false;
            }
        }
    }
    return true;
}

double IsectAdaptCurve::SegErr( const IsectAdaptPnt &p0, const IsectAdaptPnt &p1, const IsectAdaptPnt &pm, bool straight ) const
{
    // A segment leaving a surface's parameters is too far, wherever it lands
    if ( !SegInDomain( p0, p1, straight ) )
    {
        return 1.0e300;
    }

    vec3d cp[4];
    SegCtrl( p0, p1, -1, straight, cp );

    // The segment's parameter nearest pm, searched for over its middle
    double tbest = 0.5;
    double dbest = dist( BezierPnt( cp, tbest ), pm.m_Pnt );
    double step = 0.125;
    for ( int iter = 0; iter < 30; iter++ )
    {
        bool moved = false;
        for ( int k = -1; k <= 1; k += 2 )
        {
            double t = tbest + k * step;
            if ( t < 0.0 || t > 1.0 )
            {
                continue;
            }
            double d = dist( BezierPnt( cp, t ), pm.m_Pnt );
            if ( d < dbest )
            {
                dbest = d;
                tbest = t;
                moved = true;
            }
        }
        if ( !moved )
        {
            step *= 0.5;
        }
    }

    double err = dbest;

    // Each surface's image of the segment in its own parameters, against the segment: what a
    // cubic curve is written with, as its pcurves
    const Surf *surfs[2] = { m_SurfA, m_SurfB };
    const double tchk[4] = { 0.25, 0.5, 0.75, tbest };
    for ( int i = 0; i < NumPCurves(); i++ )
    {
        vec3d uwcp[4];
        SegCtrl( p0, p1, i, straight, uwcp );
        for ( int k = 0; k < 4; k++ )
        {
            vec3d s = SurfPnt( *surfs[i], ClampUW( *surfs[i], BezierPnt( uwcp, tchk[k] ) ) );
            err = std::max( err, dist( s, BezierPnt( cp, tchk[k] ) ) );
        }
    }

    // Anything that cannot be measured is too far
    if ( !( err == err ) )
    {
        err = 1.0e300;
    }

    return err;
}

// Sine of the angle between two surfaces at a point's parameters on each; zero where a normal
// vanishes
static double SinAt( const Surf &surfA, const Surf &surfB, const vec3d &uw_a, const vec3d &uw_b )
{
    vec3d uwa = ClampUW( surfA, uw_a );
    vec3d uwb = ClampUW( surfB, uw_b );

    vec3d na = surfA.CompNorm( uwa.x(), uwa.y() );
    vec3d nb = surfB.CompNorm( uwb.x(), uwb.y() );

    double lab = na.mag() * nb.mag();
    if ( lab <= 0.0 )
    {
        return 0.0;
    }
    return cross( na, nb ).mag() / lab;
}

// Whether a solve that moved a point by moved stayed near the curve it started by.  The start is
// gap from the other surface, and the surfaces meet at an angle of sine sn, so the curve can be
// up to gap / sn away; the solve may go that far beyond reach.  The solve stays in the plane
// across the curve, so a point farther away is on another part of the intersection.
static bool InReach( double moved, double reach, double gap, double sn )
{
    if ( moved <= reach )
    {
        return true;
    }
    return sn > 0.0 && moved <= reach + gap / sn;
}

// Put a point on both surfaces, near where it is.  Where the solve fails, or finds a point out
// of reach, the point is left as it was, and false returned.
static bool RefinePnt( IsectAdaptPnt &p, const Surf &surfA, const Surf &surfB, double reach )
{
    // Code-Eli evaluates the surfaces at the guesses unchecked
    vec3d uwa = ClampUW( surfA, p.m_UW[0] );
    vec3d uwb = ClampUW( surfB, p.m_UW[1] );
    vec2d a( uwa.x(), uwa.y() );
    vec2d b( uwb.x(), uwb.y() );

    double gap = dist( SurfPnt( surfA, uwa ), SurfPnt( surfB, uwb ) );

    if ( !refine_intersect_pt( p.m_Pnt, const_cast < Surf* > ( &surfA ), a, const_cast < Surf* > ( &surfB ), b ) )
    {
        return false;
    }

    uwa = vec3d( a.x(), a.y(), 0.0 );
    uwb = vec3d( b.x(), b.y(), 0.0 );
    vec3d q = SurfPnt( surfA, uwa );

    if ( InReach( dist( q, p.m_Pnt ), reach, gap, SinAt( surfA, surfB, uwa, uwb ) ) )
    {
        p.m_UW[0] = uwa;
        p.m_UW[1] = vec3d( b.x(), b.y(), 0.0 );
        p.m_Pnt = q;
        return true;
    }
    return false;
}

double IsectAdaptCurve::SurfGap( const IsectAdaptPnt &p ) const
{
    return dist( SurfPnt( *m_SurfA, p.m_UW[0] ), SurfPnt( *m_SurfB, p.m_UW[1] ) );
}

// Order along the raw curve
static bool TCompare( const IsectAdaptPnt &x, const IsectAdaptPnt &y )
{
    return x.m_T < y.m_T;
}

void IsectAdaptCurve::FindBreakPnts( const IsectAdaptPnt &p0, const IsectAdaptPnt &p1, std::vector < IsectAdaptPnt > &brk ) const
{
    brk.clear();

    // The patch breaks of each parameter, uA wA uB wB, ends of the domain left out
    std::vector < double > pmap[4];
    m_SurfA->GetSurfCore()->GetSurf()->get_pmap_uv( pmap[0], pmap[1] );
    m_SurfB->GetSurfCore()->GetSurf()->get_pmap_uv( pmap[2], pmap[3] );

    const Bezier_curve *crvs[2] = { m_CrvA, m_CrvB };

    // Each raw segment's crossings of a break, put on both surfaces with that parameter held on it
    for ( int j = 1; j < ( int )m_RawT.size(); j++ )
    {
        double ta = m_RawT[j - 1];
        double tb = m_RawT[j];

        for ( int k = 0; k < 4; k++ )
        {
            int isurf = k / 2;
            int icomp = k % 2;

            double va = crvs[isurf]->CompPnt( ta )[icomp];
            double vb = crvs[isurf]->CompPnt( tb )[icomp];
            double lo = std::min( va, vb );
            double hi = std::max( va, vb );

            for ( int ib = 1; ib + 1 < ( int )pmap[k].size(); ib++ )
            {
                double b = pmap[k][ib];
                if ( !( lo < b && b < hi ) )
                {
                    continue;
                }

                IsectAdaptPnt q;
                q.m_T = ta + ( tb - ta ) * ( b - va ) / ( vb - va );
                q.m_UW[0] = ClampUW( *m_SurfA, m_CrvA->CompPnt( q.m_T ) );
                q.m_UW[1] = ClampUW( *m_SurfB, m_CrvB->CompPnt( q.m_T ) );
                q.m_UW[isurf][icomp] = b;
                q.m_Pnt = SurfPnt( *m_SurfA, q.m_UW[0] );

                double reach = dist( SurfPnt( *m_SurfA, m_CrvA->CompPnt( ta ) ), SurfPnt( *m_SurfA, m_CrvA->CompPnt( tb ) ) );
                double gap = SurfGap( q );

                vec2d a( q.m_UW[0].x(), q.m_UW[0].y() );
                vec2d c( q.m_UW[1].x(), q.m_UW[1].y() );
                if ( !refine_intersect_pt_held( q.m_Pnt, const_cast < Surf* > ( m_SurfA ), a, const_cast < Surf* > ( m_SurfB ), c, k ) )
                {
                    continue;
                }

                vec3d pnt = SurfPnt( *m_SurfA, vec3d( a.x(), a.y(), 0.0 ) );
                double sn = SinAt( *m_SurfA, *m_SurfB, vec3d( a.x(), a.y(), 0.0 ), vec3d( c.x(), c.y(), 0.0 ) );
                if ( !InReach( dist( pnt, q.m_Pnt ), reach, gap, sn ) )
                {
                    continue;
                }

                q.m_UW[0] = vec3d( a.x(), a.y(), 0.0 );
                q.m_UW[1] = vec3d( c.x(), c.y(), 0.0 );
                q.m_Pnt = pnt;
                brk.push_back( q );
            }
        }
    }

    std::sort( brk.begin(), brk.end(), TCompare );

    // Crossings of two breaks at once, or at an end, are one point
    std::vector < IsectAdaptPnt > kept;
    for ( int i = 0; i < ( int )brk.size(); i++ )
    {
        if ( dist( brk[i].m_Pnt, p0.m_Pnt ) <= m_ResTol || dist( brk[i].m_Pnt, p1.m_Pnt ) <= m_ResTol )
        {
            continue;
        }
        if ( !kept.empty() && dist( brk[i].m_Pnt, kept.back().m_Pnt ) <= m_ResTol )
        {
            continue;
        }
        kept.push_back( brk[i] );
    }
    brk = kept;

    for ( int i = 0; i < ( int )brk.size(); i++ )
    {
        SetTangent( brk[i], RawDir( brk[i].m_T ) );
    }
}

void IsectAdaptCurve::AdaptSeg( const IsectAdaptPnt &p0_in, const IsectAdaptPnt &p1, int Nlimit )
{
    IsectAdaptPnt p0 = p0_in;

    // The raw intersection halfway along, moved onto both surfaces
    IsectAdaptPnt pm;
    pm.m_T = 0.5 * ( p0.m_T + p1.m_T );
    pm.m_UW[0] = m_CrvA->CompPnt( pm.m_T );
    pm.m_UW[1] = m_CrvB->CompPnt( pm.m_T );
    pm.m_Pnt = SurfPnt( *m_SurfA, pm.m_UW[0] );

    double chord = dist( p0.m_Pnt, p1.m_Pnt );

    // Every segment longer than an eighth of the curve is divided whatever it measures, so the
    // curve is looked at in at least that many places
    bool coarse = ( p1.m_T - p0.m_T ) > 0.125 * ( 1.0 + 1.0e-9 ) * ( m_T1 - m_T0 );

    bool pm_on_both = true;
    if ( m_Refine && m_SurfA != m_SurfB )
    {
        pm_on_both = RefinePnt( pm, *m_SurfA, *m_SurfB, std::max( chord, m_AbsTol ) );
    }

    // A cubic is measured at a few points only, so is held to half what it must meet
    double tol = m_RelTol * chord;
    if ( m_AbsTol > 0.0 )
    {
        double abs_tol = m_AbsTol;
        if ( m_Cubic )
        {
            abs_tol = 0.5 * m_AbsTol;
        }

        if ( m_RelTol > 0.0 )
        {
            tol = std::min( tol, abs_tol );
        }
        else
        {
            tol = abs_tol;
        }
    }

    // Nothing finer than the surfaces resolve
    tol = std::max( tol, m_ResTol );

    // Where the solve cannot put pm on both surfaces, its raw point may be on both to within
    // the tolerance already.  Where it is not, the surfaces do not meet there to within it,
    // and no division of the segment finds a point that does.
    bool miss = false;
    if ( !pm_on_both && SurfGap( pm ) > tol )
    {
        miss = true;
    }

    double err = SegErr( p0, p1, pm, false );

    // A coarse segment is divided anyway, so its middle point is kept off the surfaces
    if ( miss )
    {
        m_NumMiss++;
        m_MaxMiss = std::max( m_MaxMiss, SurfGap( pm ) );
        if ( !coarse )
        {
            err = 0.0;
        }
    }

    if ( m_Cubic && err > tol && !coarse && SegErr( p0, p1, pm, true ) <= tol )
    {
        p0.m_Straight = true;
        err = 0.0;
    }

    SetTangent( pm, RawDir( pm.m_T ) );

    if ( ( err > tol && Nlimit > 0 ) || coarse )
    {
        AdaptSeg( p0, pm, Nlimit - 1 );
        AdaptSeg( pm, p1, Nlimit - 1 );
    }
    else
    {
        // Out of halvings, a segment leaving a surface's parameters follows its chord, which
        // stays within them
        if ( m_Cubic && !SegInDomain( p0, p1, p0.m_Straight ) )
        {
            p0.m_Straight = true;
        }

        // Count a segment left over the tolerance, and how far over it is as written
        if ( err > tol )
        {
            m_NumLimit++;
            m_MaxLimitErr = std::max( m_MaxLimitErr, SegErr( p0, p1, pm, p0.m_Straight ) );
        }

        m_Pnts.push_back( p0 );

        // A polyline keeps the point it was measured at, halving its segments again.  A
        // cubic's segments are the ones measured.
        if ( !m_Cubic )
        {
            m_Pnts.push_back( pm );
        }
    }
}

void IsectAdaptCurve::Adapt( const Bezier_curve &uwcrvA, const Bezier_curve &uwcrvB, const Surf &surfA, const Surf &surfB,
                             bool refine, bool cubic, double rel_tol, double abs_tol, int Nlimit )
{
    m_CrvA = &uwcrvA;
    m_CrvB = &uwcrvB;
    m_SurfA = &surfA;
    m_SurfB = &surfB;
    m_Refine = refine;
    m_Cubic = cubic;
    m_RelTol = rel_tol;
    m_AbsTol = abs_tol;

    m_Pnts.clear();
    m_NumMiss = 0;
    m_MaxMiss = 0.0;
    m_NumLimit = 0;
    m_MaxLimitErr = 0.0;

    // A copy of the raw curve, made once
    piecewise_curve_type raw_crv = uwcrvA.GetCurve();
    m_T0 = raw_crv.get_t0();
    m_T1 = raw_crv.get_tmax();
    raw_crv.get_pmap( m_RawT );

    // Points nearer each other than this are the same place, rounded, however small the curve.
    // Raw points are only that close by rounding, so directions between them are measured over
    // ten times as far.
    m_ResTol = 1.0e-10 * std::max( surfA.GetBBox().DiagDist(), surfB.GetBBox().DiagDist() );
    m_DirTol = 10.0 * m_ResTol;

    // The ends stay where the intersection put them, which is where the curves they meet end
    IsectAdaptPnt p0, p1;
    p0.m_T = m_T0;
    p1.m_T = m_T1;

    const IsectAdaptPnt *ends[2] = { &p0, &p1 };
    IsectAdaptPnt *pends[2] = { &p0, &p1 };
    for ( int k = 0; k < 2; k++ )
    {
        pends[k]->m_UW[0] = uwcrvA.CompPnt( ends[k]->m_T );
        pends[k]->m_UW[1] = uwcrvB.CompPnt( ends[k]->m_T );
        pends[k]->m_Pnt = SurfPnt( surfA, pends[k]->m_UW[0] );
    }

    // The ends only close the small gap the raw intersection leaves between the surfaces
    if ( refine && &surfA != &surfB )
    {
        double reach = std::max( 1.0e-3 * dist( p0.m_Pnt, p1.m_Pnt ), 100.0 * abs_tol );
        double endtol = std::max( abs_tol, m_ResTol );
        for ( int k = 0; k < 2; k++ )
        {
            if ( !RefinePnt( *pends[k], surfA, surfB, reach ) && SurfGap( *pends[k] ) > endtol )
            {
                m_NumMiss++;
                m_MaxMiss = std::max( m_MaxMiss, SurfGap( *pends[k] ) );
            }
        }
    }

    SetTangent( p0, RawDir( p0.m_T ) );
    SetTangent( p1, RawDir( p1.m_T ) );

    // A pcurve is smooth only within a patch of its surface, so a cubic curve is adapted
    // piece by piece between the points where it crosses one's edge
    std::vector < IsectAdaptPnt > brk;
    if ( cubic && refine && &surfA != &surfB )
    {
        FindBreakPnts( p0, p1, brk );
    }

    IsectAdaptPnt prev = p0;
    for ( int i = 0; i < ( int )brk.size(); i++ )
    {
        AdaptSeg( prev, brk[i], Nlimit );
        prev = brk[i];
    }
    AdaptSeg( prev, p1, Nlimit );
    m_Pnts.push_back( p1 );

    // Where a surface collapses, the same place comes up more than once, give or take the
    // rounding of putting each on both surfaces
    std::vector < vec3d > pnt_vec( m_Pnts.size() );
    std::vector < double > index_vec( m_Pnts.size() );
    for ( int i = 0; i < ( int )m_Pnts.size(); i++ )
    {
        pnt_vec[i] = m_Pnts[i].m_Pnt;
        index_vec[i] = i;
    }

    RemoveRepeatedPnts( pnt_vec, index_vec, m_ResTol );

    // A segment spanning points dropped between its ends is straight where any it replaces was
    std::vector < IsectAdaptPnt > kept( index_vec.size() );
    for ( int i = 0; i < ( int )index_vec.size(); i++ )
    {
        int j = ( int )index_vec[i];
        kept[i] = m_Pnts[j];

        if ( i + 1 < ( int )index_vec.size() )
        {
            for ( int k = j + 1; k < ( int )index_vec[i + 1]; k++ )
            {
                if ( m_Pnts[k].m_Straight )
                {
                    kept[i].m_Straight = true;
                }
            }
        }
    }
    m_Pnts = kept;
}

void IsectAdaptCurve::GetBezier( std::vector < vec3d > &pnts, std::vector < vec3d > &uw_a, std::vector < vec3d > &uw_b,
                                 std::vector < double > &breaks, int &deg ) const
{
    pnts.clear();
    uw_a.clear();
    uw_b.clear();
    breaks.clear();

    if ( m_Pnts.empty() )
    {
        deg = 1;
        return;
    }

    pnts.push_back( m_Pnts[0].m_Pnt );
    uw_a.push_back( m_Pnts[0].m_UW[0] );
    uw_b.push_back( m_Pnts[0].m_UW[1] );
    breaks.push_back( 0.0 );

    if ( !m_Cubic )
    {
        deg = 1;
        for ( int i = 1; i < ( int )m_Pnts.size(); i++ )
        {
            pnts.push_back( m_Pnts[i].m_Pnt );
            uw_a.push_back( m_Pnts[i].m_UW[0] );
            uw_b.push_back( m_Pnts[i].m_UW[1] );
            breaks.push_back( breaks.back() + dist( m_Pnts[i - 1].m_Pnt, m_Pnts[i].m_Pnt ) );
        }
        return;
    }

    deg = 3;
    for ( int i = 1; i < ( int )m_Pnts.size(); i++ )
    {
        bool straight = m_Pnts[i - 1].m_Straight;
        vec3d cp[4], cpa[4], cpb[4];
        SegCtrl( m_Pnts[i - 1], m_Pnts[i], -1, straight, cp );
        SegCtrl( m_Pnts[i - 1], m_Pnts[i], 0, straight, cpa );
        SegCtrl( m_Pnts[i - 1], m_Pnts[i], 1, straight, cpb );

        for ( int j = 1; j < 4; j++ )
        {
            pnts.push_back( cp[j] );
            uw_a.push_back( cpa[j] );
            uw_b.push_back( cpb[j] );
        }
        breaks.push_back( breaks.back() + dist( m_Pnts[i - 1].m_Pnt, m_Pnts[i].m_Pnt ) );
    }
}
