//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// ICurve
//
//////////////////////////////////////////////////////////////////////

#include "ICurve.h"
#include "SCurve.h"
#include "Surf.h"

//////////////////////////////////////////////////////////////////////
ICurve::ICurve()
{
    m_SCurve_A = m_SCurve_B = nullptr;
    m_PlaneBorderIntersectFlag = false;
    m_GeomMatchFlag = false;
}

// Do points sampled along a lie on b, within tol?  Each is found on b's surface, moved onto the
// line of the surface's parameters b runs along, and must lie between b's ends and within tol
// of where it started.
static bool SamplesOnBorder( SCurve* a, SCurve* b, double tol )
{
    const int n = 16;

    Bezier_curve uwcrv = a->GetUWCrv();
    Surf* sa = a->GetSurf();
    Surf* sb = b->GetSurf();
    const SurfCore* cb = sb->GetSurfCore();
    double uwtol = 1.0e-6 * std::max( cb->GetDU(), cb->GetDW() );

    Bezier_curve uwb = b->GetUWCrv();
    vec3d b0 = uwb.CompPnt01( 0.0 );
    vec3d b1 = uwb.CompPnt01( 1.0 );

    // The parameter b runs along; the other is held
    bool alongu = std::abs( b1.x() - b0.x() ) >= std::abs( b1.y() - b0.y() );

    for ( int i = 1 ; i < n ; i++ )
    {
        vec3d uw = uwcrv.CompPnt01( ( double )i / ( double )n );
        vec3d p = sa->CompPnt( uw.x(), uw.y() );

        vec2d uwp = sb->ClosestUW( p );
        double run = uwp.y();
        double lo = std::min( b0.y(), b1.y() );
        double hi = std::max( b0.y(), b1.y() );
        if ( alongu )
        {
            run = uwp.x();
            lo = std::min( b0.x(), b1.x() );
            hi = std::max( b0.x(), b1.x() );
            uwp.set_xy( uwp.x(), b0.y() );
        }
        else
        {
            uwp.set_xy( b0.x(), uwp.y() );
        }

        if ( run < lo - uwtol || run > hi + uwtol || dist( sb->CompPnt( uwp.x(), uwp.y() ), p ) > tol )
        {
            return false;
        }
    }
    return true;
}

// The two curves run between the same ends, either way round, and each lies along the other.
// Set bkwd where they run opposite ways.
static bool GeomMatch( SCurve* a, SCurve* b, double tol, bool &bkwd )
{
    Bezier_curve uwa = a->GetUWCrv();
    Bezier_curve uwb = b->GetUWCrv();
    vec3d a0 = uwa.CompPnt01( 0.0 );
    vec3d a1 = uwa.CompPnt01( 1.0 );
    vec3d b0 = uwb.CompPnt01( 0.0 );
    vec3d b1 = uwb.CompPnt01( 1.0 );

    vec3d pa0 = a->GetSurf()->CompPnt( a0.x(), a0.y() );
    vec3d pa1 = a->GetSurf()->CompPnt( a1.x(), a1.y() );
    vec3d pb0 = b->GetSurf()->CompPnt( b0.x(), b0.y() );
    vec3d pb1 = b->GetSurf()->CompPnt( b1.x(), b1.y() );

    // A border collapsed to a point says nothing about where it runs.
    if ( dist( pa0, pa1 ) <= tol )
    {
        return false;
    }

    bool fwd = dist( pa0, pb0 ) <= tol && dist( pa1, pb1 ) <= tol;
    bkwd = !fwd && dist( pa0, pb1 ) <= tol && dist( pa1, pb0 ) <= tol;
    if ( !fwd && !bkwd )
    {
        return false;
    }

    return SamplesOnBorder( a, b, tol ) && SamplesOnBorder( b, a, tol );
}

ICurve::~ICurve()
{
}

bool ICurve::Match( SCurve* crv_A, SCurve* crv_B )
{
    Bezier_curve xyzcrvA = crv_A->GetUWCrv();
    xyzcrvA.UWCurveToXYZCurve( crv_A->GetSurf() );

    Bezier_curve xyzcrvB = crv_B->GetUWCrv();
    xyzcrvB.UWCurveToXYZCurve( crv_B->GetSurf() );

    return Match( crv_A, crv_B, xyzcrvA, xyzcrvB, -1.0 );
}

bool ICurve::Match( SCurve* crv_A, SCurve* crv_B, const Bezier_curve &xyzcrvA, const Bezier_curve &xyzcrvB, double geomtol )
{
    double tol = 1.0e-5;

    bool fmatch = xyzcrvA.MatchFwd( xyzcrvB, tol );
    bool bmatch = xyzcrvA.MatchBkwd( xyzcrvB, tol );

    // Around a removed face, two curves match only where they lie along each other.  A border
    // whose u,w runs straight has only its ends for control points, so the two halves of a rim
    // match by them; and two bodies may give one rim by different control polygons.
    if ( geomtol > 0.0 )
    {
        bool bkwd = false;
        if ( !GeomMatch( crv_A, crv_B, geomtol, bkwd ) )
        {
            fmatch = false;
            bmatch = false;
        }
        else if ( !fmatch && !bmatch )
        {
            fmatch = !bkwd;
            bmatch = bkwd;
            m_GeomMatchFlag = true;
        }
    }

    if ( fmatch || bmatch )
    {
        if ( bmatch )
        {
            //=== Flip Direction ====//
            crv_B->FlipDir();
        }

        m_SCurve_A = crv_A;
        m_SCurve_A->SetICurve( this );
        m_SCurve_B = crv_B;
        m_SCurve_B->SetICurve( this );
        return true;
    }

    return false;
}

void ICurve::BorderTesselate( )
{
    m_SCurve_A->BorderTesselate( );
    if ( !m_SCurve_B )
    {
        return;
    }

    // Cut to the same count as A.  The two are the same edge seen from either side, and the
    // chain built from them walks both with one index; if B comes back shorter, that walk
    // reads past its end.
    m_SCurve_B->BorderTesselate( ( int )m_SCurve_A->GetUWTessPnts().size() );

    // Parameterized differently, B's points are moved to where A's lie.
    if ( m_GeomMatchFlag )
    {
        m_SCurve_B->ProjectBorderTess( m_SCurve_A );
    }
}

void ICurve::PlaneBorderTesselate( SCurve* crv_A, SCurve* crv_B )
{
    crv_A->TessAdapt( *crv_A->GetSurf(), 5e-4, 16 );

    crv_B->ProjectTessToSurf( crv_A );
}

void ICurve::DebugEdgeMatching( FILE* fp )
{
    if ( !m_SCurve_A || !m_SCurve_B )
    {
        fprintf( fp, "  ICurve: %d Unmatched Curve \n",  ( int )( ( size_t )this ) );
        return;
    }

    Surf* surfA = m_SCurve_A->GetSurf();
    Surf* surfB = m_SCurve_B->GetSurf();

    fprintf( fp, "  ICurve: %d   Surf A = %d, Surf B = %d \n",  ( int )( ( size_t )this ),  ( int )( ( size_t )surfA ),  ( int )( ( size_t )surfB ) );

    double total_dist = 0.0;
    for ( int i = 0 ; i < 21 ; i++ )
    {
        double u = ( double )i / 20.0;
        vec3d uwpntA = m_SCurve_A->CompPntUW( u );

//      fprintf( fp, "u = %f  uwpntA = %f %f %f \n",  u, uwpntA.x(), uwpntA.y(), uwpntA.z() );

        vec3d uwpntB = m_SCurve_B->CompPntUW( u );

        vec3d pntA = surfA->CompPnt( uwpntA.x(), uwpntA.y() );
        vec3d pntB = surfB->CompPnt( uwpntB.x(), uwpntB.y() );

        double d = dist( pntA, pntB );
        total_dist += d;

        //fprintf( fp, "  Dist PntA %f %f %f PntB %f %f %f = %f \n",
        //  pntA.x(), pntA.y(), pntA.z(),  pntB.x(), pntB.y(), pntB.z(), d );

    }
    fprintf( fp, "  Total Dist PntA - PntB = %f \n", total_dist );


}

