//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// ICurve
//
//////////////////////////////////////////////////////////////////////

#include "ICurve.h"
#include "SCurve.h"

//////////////////////////////////////////////////////////////////////
ICurve::ICurve()
{
    m_SCurve_A = m_SCurve_B = NULL;
    m_PlaneBorderIntersectFlag = false;
}

ICurve::~ICurve()
{
}


bool ICurve::Match( SCurve* crv_A, SCurve* crv_B )
{
    double tol = 0.00001;

    Bezier_curve xyzcrvA = crv_A->GetUWCrv();
    xyzcrvA.UWCurveToXYZCurve( crv_A->GetSurf() );

    Bezier_curve xyzcrvB = crv_B->GetUWCrv();
    xyzcrvB.UWCurveToXYZCurve( crv_B->GetSurf() );

    bool fmatch = xyzcrvA.MatchFwd( xyzcrvB, tol );
    bool bmatch = xyzcrvA.MatchBkwd( xyzcrvB, tol );

    bool match = false;
    if ( (fmatch == true) || (bmatch == true) )
    {
        match = true;
    }

    if( (fmatch == true) && (bmatch == true) )
    {
    }

    if ( match )
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
    }

    return match;
}

void ICurve::BorderTesselate( )
{
    m_SCurve_A->BorderTesselate( );
    if ( !m_SCurve_B )
    {
        return;
    }

    m_SCurve_B->BorderTesselate( );
}

void ICurve::PlaneBorderTesselate( SCurve* crv_A, SCurve* crv_B )
{
    crv_A->CheapTesselate();
    crv_B->ProjectTessToSurf( crv_A );
}

void ICurve::Draw()
{

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

