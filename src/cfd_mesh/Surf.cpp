//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// Surf
//
//////////////////////////////////////////////////////////////////////

#include "Surf.h"
#include "SCurve.h"
#include "ICurve.h"
#include "ISegChain.h"
#include "tri_tri_intersect.h"
#include "CfdMeshMgr.h"
#include "SubSurfaceMgr.h"
#include "IntersectPatch.h"
#include "VspUtil.h"
#include <cfloat>  //For DBL_EPSILON
#include "Vec3d.h"

#ifdef DEBUG_CFD_MESH
#include "WriteMatlab.h"
#endif

// Keep this after all other #includes.
//
// OpenABF declares a variable PI that conflicts with a preexisting #define PI.  This stores the current value,
// undefines PI, includes OpenABF, and then restores the stored value.  At the current time, Surf.cpp does not
// use PI, so this should all be un-necesacary.  If this becomes a problem in the future, the only use of
// OpenABF is limited to Surf::BuildDistMap(), which could be moved to a separate *.cpp file for compilation.
//
#ifdef PI
#define PI_SURF_CPP_TEMP
#undef PI
#endif

#include "OpenABF/OpenABF.hpp"

#ifdef PI_SURF_CPP_TEMP
#define PI PI_SURF_CPP_TEMP
#endif

Surf::Surf()
{
    m_GridDensityPtr = 0;
    m_CompID = -1;
    m_UnmergedCompID = -1;
    m_SurfID = -1;
    m_FlipFlag = false;
    m_WakeFlag = false;
    m_SurfCfdType = vsp::CFD_NORMAL;
    m_ThickSurf = true;
    m_PlateNum = -1;
    m_SymPlaneFlag = false;
    m_FarFlag = false;
    m_WakeParentSurfID = -1;
    m_Mesh.SetSurfPtr( this );
    m_NumMap = 11;
    m_BaseTag = 1;
    m_MainSurfID = 0;
    m_FeaPartIndex = -1;
    m_IgnoreSurfFlag = false;
}

Surf::~Surf()
{
    int i;
    //==== Delete Patches ====//
    for ( i = 0 ; i < ( int )m_PatchVec.size() ; i++ )
    {
        delete m_PatchVec[i];
    }

    //==== Delete SCurves ====//
    for ( i = 0 ; i < ( int )m_SCurveVec.size() ; i++ )
    {
        delete m_SCurveVec[i];
    }
}

void Surf::BuildClean()
{
    int i;
    //==== Delete SCurves ====//
    for ( i = 0 ; i < ( int )m_SCurveVec.size() ; i++ )
    {
        delete m_SCurveVec[i];
    }

    m_SCurveVec.clear();
}

void Surf::GetBorderCurve( const vec3d &uw0, const vec3d &uw1, Bezier_curve & crv ) const
{
    m_SurfCore.GetBorderCurve( uw0, uw1, crv );
}

int Surf::UWPointOnBorder( double u, double w, double tol ) const
{
    return m_SurfCore.UWPointOnBorder( u, w, tol );
}

double Surf::TargetLen( double u, double w, double gap, double radfrac, int &reason )
{
    double k1, k2, ka, kg;

    double tol = 1e-6;
    double len = numeric_limits<double>::max( );
    double r = -1.0;

    double glen = numeric_limits<double>::max( );
    double nlen = numeric_limits<double>::max( );

    double umin = m_SurfCore.GetMinU();
    double wmin = m_SurfCore.GetMinW();

    m_SurfCore.CompCurvature( u, w, k1, k2, ka, kg );

    if( std::abs( k1 ) < tol ) // If zero curvature
    {
        double du = -tol;
        if( u <= umin + tol )
        {
            du = tol;
        }
        double dw = -tol;
        if( w <= wmin + tol )
        {
            dw = tol;
        }
        // Check point offset inside the surface.
        m_SurfCore.CompCurvature( u + du, w + dw, k1, k2, ka, kg );
    }

    if( std::abs( k1 ) > tol )
    {
        // Tightest radius of curvature
        r = 1.0 / std::abs( k1 );

        if( r > gap )
        {
            // Pythagorean thm. to calculate edge length to match gap given radius.
            glen = 2.0 * sqrt( 2.0 * r * gap - gap * gap );
        }
        else
        {
            glen = 2.0 * gap;
        }

        // Radius fraction calculated elsewhere based on desired number of circle segments.
        // This calculation can give unboundedly small edge lengths.  The minimum edge length
        // is a required control to prevent this.
        nlen = r * radfrac;

        if ( glen < nlen )
        {
            reason = vsp::CURV_GAP;
        }
        else
        {
            reason = vsp::CURV_NCIRCSEG;
        }

        len = min( glen, nlen );
    }
    return len;
}

void Surf::BuildTargetMap( vector< MapSource* > &sources, int sid )
{
    int npatchu = m_SurfCore.GetNumUPatches();
    int npatchw = m_SurfCore.GetNumWPatches();

    unsigned int nmapu = npatchu * ( m_NumMap - 1 ) + 1;
    unsigned int nmapw = npatchw * ( m_NumMap - 1 ) + 1;

    double umin = m_SurfCore.GetMinU();
    double du = m_SurfCore.GetMaxU() - umin;
    double wmin = m_SurfCore.GetMinW();
    double dw = m_SurfCore.GetMaxW() - wmin;

    // Initialize map matrix dimensions
    m_SrcMap.resize( nmapu );
    for( int i = 0; i < nmapu ; i++ )
    {
        m_SrcMap[i].resize( nmapw );
    }

    bool limitFlag = false;
    if ( m_FarFlag )
    {
        limitFlag = true;
    }
    if ( m_SymPlaneFlag )
    {
        limitFlag = true;
    }

    // Loop over surface evaluating source strength and curvature
    for( int i = 0; i < nmapu ; i++ )
    {
        double u = umin + du * ( 1.0 * i ) / ( nmapu - 1 );
        for( int j = 0; j < nmapw ; j++ )
        {
            double w = wmin + dw * ( 1.0 * j ) / ( nmapw - 1 );

            double len = numeric_limits<double>::max( );

            int reason = vsp::NO_REASON;
            // apply curvature based limits
            double curv_len = TargetLen( u, w, m_GridDensityPtr->GetMaxGap( limitFlag ), m_GridDensityPtr->GetRadFrac( limitFlag ), reason );
            len = min( len, curv_len );

            // apply minimum edge length as safety on curvature
            if ( len <= m_GridDensityPtr->m_MinLen )
            {
                // Assign MIN_LEN_CONSTRAINT, MIN_LEN_CONSTRAINT_CURV_GAP, MIN_LEN_CONSTRAINT_CURV_NCIRCSEG as appropriate.
                reason += vsp::MIN_LEN_INCREMENT;

                if ( reason >= vsp::NUM_MESH_REASON ) // Should be imposible, just as a safety check.
                {
                    reason = vsp::MIN_LEN_CONSTRAINT;
                }
            }
            len = max( len, m_GridDensityPtr->m_MinLen );

            // apply sources
            vec3d p = m_SurfCore.CompPnt( u, w );

            // The last four parameters passed here (m_GeomID, m_MainSurfID, u, w)
            // represent a significant layering violation.  This is needed to allow
            // constant U/W line sources to do some evaluation in u,w space instead
            // of just x,y,z space.
            double grid_len = m_GridDensityPtr->GetTargetLen( p, limitFlag, m_GeomID, m_MainSurfID, u, w );
            if ( grid_len < len )
            {
                printf("%g %g\n", grid_len, len );
                reason = vsp::SOURCES;
            }
            len = min( len, grid_len );

            // finally check max size
            if ( len >= m_GridDensityPtr->GetBaseLen( limitFlag ) )
            {
                reason = vsp::MAX_LEN_CONSTRAINT;
            }
            len = min( len, m_GridDensityPtr->GetBaseLen( limitFlag ) );

            m_SrcMap[i][j] = MapSource( p, len, sid, reason );
            sources.push_back( &( m_SrcMap[i][j] ) );
        }
    }
}

void Surf::SetSymPlaneFlag( bool flag )
{
    m_SymPlaneFlag = flag;

    // Refine background map for symmetry plane.
    if( m_SymPlaneFlag )
    {
        m_NumMap = 101;
    }
    else
    {
        m_NumMap = 11;
    }
}


bool indxcompare( const pair < double, pair < int, int > > &a, const pair < double, pair < int, int > > &b )
{
    return ( a.first < b.first );
}

void Surf::WalkMap( int istart, int jstart, int kstart )
{
    static int iadd[] = { -1, 1,  0, 0 };
    static int jadd[] = {  0, 0, -1, 1 };

    vector < pair < int, int > > v;

    for( int i = 0; i < 4; i++ )
    {
        int inext = istart + iadd[i];
        int jnext = jstart + jadd[i];

        if( inext < m_SrcMap.size() && inext >= 0 && jnext < m_SrcMap[0].size() && jnext >= 0 )
        {
            v.push_back( make_pair( inext, jnext ) );
        }
    }

    while ( !v.empty() )
    {
        pair < int, int > p = v.back();
        v.pop_back();
        int icurrent = p.first;
        int jcurrent = p.second;

        if( m_SrcMap[ icurrent ][ jcurrent ].m_maxvisited < kstart )
        {
            m_SrcMap[ icurrent ][ jcurrent ].m_maxvisited = kstart;

            double targetstr = m_SrcMap[istart][jstart].m_str +
                    ( m_SrcMap[ icurrent ][ jcurrent ].m_pt - m_SrcMap[istart][jstart].m_pt ).mag() *
                    (m_GridDensityPtr->m_GrowRatio - 1.0);

            if( m_SrcMap[ icurrent ][ jcurrent ].m_str > targetstr )
            {
                // Mark dominated as progress is made
                m_SrcMap[ icurrent ][ jcurrent ].m_dominated = true;
                m_SrcMap[ icurrent ][ jcurrent ].m_str = targetstr;

                if ( m_SrcMap[istart][jstart].m_reason < vsp::MIN_GROW_LIMIT )
                {
                    m_SrcMap[ icurrent ][ jcurrent ].m_reason = m_SrcMap[istart][jstart].m_reason + vsp::GROW_LIMIT_INCREMENT;
                }
                else
                {
                    m_SrcMap[ icurrent ][ jcurrent ].m_reason = m_SrcMap[istart][jstart].m_reason;
                }

                for( int i = 0; i < 4; i++ )
                {
                    int inext = icurrent + iadd[i];
                    int jnext = jcurrent + jadd[i];

                    if( inext < m_SrcMap.size() && inext >= 0 && jnext < m_SrcMap[0].size() && jnext >= 0 )
                    {
                        v.push_back( make_pair( inext, jnext ) );
                    }
                }
            }
        }
    }
}

void Surf::WalkMap( int istart, int jstart )
{
    static const int iadd[] = { -1, 1,  0, 0 };
    static const int jadd[] = {  0, 0, -1, 1 };

    vector < pair < int, int > > v;

    for( int i = 0; i < 4; i++ )
    {
        int inext = istart + iadd[i];
        int jnext = jstart + jadd[i];

        if( inext < m_SrcMap.size() && inext >= 0 && jnext < m_SrcMap[0].size() && jnext >= 0 )
        {
            v.push_back( make_pair( inext, jnext ) );
        }
    }

    while ( !v.empty() )
    {
        pair < int, int > p = v.back();
        v.pop_back();
        int icurrent = p.first;
        int jcurrent = p.second;

        double targetstr = m_SrcMap[istart][jstart].m_str +
                ( m_SrcMap[ icurrent ][ jcurrent ].m_pt - m_SrcMap[istart][jstart].m_pt ).mag() *
                (m_GridDensityPtr->m_GrowRatio - 1.0);


        if( m_SrcMap[ icurrent ][ jcurrent ].m_str > targetstr )
        {
            m_SrcMap[ icurrent ][ jcurrent ].m_str = targetstr;

            if ( m_SrcMap[istart][jstart].m_reason < vsp::MIN_GROW_LIMIT )
            {
                m_SrcMap[ icurrent ][ jcurrent ].m_reason = m_SrcMap[istart][jstart].m_reason + vsp::GROW_LIMIT_INCREMENT;
            }
            else
            {
                m_SrcMap[ icurrent ][ jcurrent ].m_reason = m_SrcMap[istart][jstart].m_reason;
            }

            for( int i = 0; i < 4; i++ )
            {
                int inext = icurrent + iadd[i];
                int jnext = jcurrent + jadd[i];

                if( inext < m_SrcMap.size() && inext >= 0 && jnext < m_SrcMap[0].size() && jnext >= 0 )
                {
                    v.push_back( make_pair( inext, jnext ) );
                }
            }
        }
    }
}

void Surf::LimitTargetMap()
{
    int nmapu = m_SrcMap.size();
    int nmapw = m_SrcMap[0].size();

    int nmap = nmapu * nmapw;

    // Create size sortable index of array i,j coordinates
    vector< pair < double, pair < int, int > > > index;
    index.resize( nmap );

    int k = 0;
    for( int i = 0; i < nmapu ; i++ )
    {
        for( int j = 0; j < nmapw ; j++ )
        {
            pair< int, int > ij( i, j );
            pair < double, pair < int, int > > id( m_SrcMap[i][j].m_str, ij );
            index[k] = id;
            k++;
            m_SrcMap[i][j].m_maxvisited = -1;  // Reset traversal limiter.
        }
    }

    // Sort index
    std::sort( index.begin(), index.end(), indxcompare );

    // Start from smallest
    for( k = 0; k < nmap; k++ )
    {
        pair< int, int > ij = index[k].second;
        int i = ij.first;
        int j = ij.second;

        // Recursively limit from small to large (skip if dominated)
        if( !( m_SrcMap[i][j].m_dominated ) )
        {
            WalkMap( i, j, k );
        }
    }
}

void Surf::LimitTargetMap( const MSCloud &es_cloud, MSTree &es_tree, double minmap )
{
    double grm1 = m_GridDensityPtr->m_GrowRatio - 1.0;

    double tmin = min( minmap, es_cloud.sources[0]->m_str );

    SearchParams params;
    params.sorted = false;

    int nmapu = m_SrcMap.size();
    int nmapw = m_SrcMap[0].size();

    // Loop over surface evaluating source strength and curvature
    for( int i = 0; i < nmapu ; i++ )
    {
//      double u = ( 1.0 * i ) / ( m_NumMap - 1 );
        for( int j = 0; j < nmapw ; j++ )
        {
//          double w = ( 1.0 * j ) / ( m_NumMap - 1 );

            double *query_pt = m_SrcMap[i][j].m_pt.v;

            double t = m_SrcMap[i][j].m_str;
            double torig = t;
            int reason = m_SrcMap[i][j].m_reason;

            double rmax = ( t - tmin ) / grm1;
            if( rmax > 0.0 )
            {
                double r2max = rmax * rmax;

                MSTreeResults es_matches;

                unsigned int nMatches = es_tree.radiusSearch( query_pt, r2max, es_matches, params );

                for ( int k = 0; k < nMatches; k++ )
                {
                    unsigned int imatch = es_matches[k].first;
                    double r = sqrt( es_matches[k].second );

                    double str = es_cloud.sources[imatch]->m_str;

                    double ts = str + grm1 * r;

                    if ( ts < t )
                    {
                        reason = es_cloud.sources[imatch]->m_reason;
                    }

                    t = min( t, ts );
                }
                if( t < torig )
                {
                    m_SrcMap[i][j].m_str = t;

                    if ( reason < vsp::MIN_GROW_LIMIT )
                    {
                        m_SrcMap[i][j].m_reason = reason + vsp::GROW_LIMIT_INCREMENT;
                    }
                    else
                    {
                        m_SrcMap[i][j].m_reason = reason;
                    }

                    WalkMap( i, j );
                }
            }
        }
    }
}

double Surf::InterpTargetMap( double u, double w, int &reason )
{
    int i, j;
    double fraci, fracj;
    UWtoTargetMapij( u, w, i, j, fraci, fracj );

    double ti = m_SrcMap[i][j].m_str + fracj * ( m_SrcMap[i][j + 1].m_str - m_SrcMap[i][j].m_str );
    double tip1 = m_SrcMap[i + 1][j].m_str + fracj * ( m_SrcMap[i + 1][j + 1].m_str - m_SrcMap[i + 1][j].m_str );

    // Assign reason based on nearest map source point.
    reason = m_SrcMap[ round( i + fraci ) ][ round( j + fracj ) ].m_reason;

    double t = ti + fraci * ( tip1 - ti );
    return t;
}

void Surf::UWtoTargetMapij( double u, double w, int &i, int &j, double &fraci, double &fracj )
{
    int npatchu = m_SurfCore.GetNumUPatches();
    int npatchw = m_SurfCore.GetNumWPatches();

    // int nmapu = npatchu * ( m_NumMap - 1 ) + 1;
    // int nmapw = npatchw * ( m_NumMap - 1 ) + 1;

    double umin = m_SurfCore.GetMinU();
    double du = m_SurfCore.GetMaxU() - umin;
    double wmin = m_SurfCore.GetMinW();
    double dw = m_SurfCore.GetMaxW() - wmin;

    int imax = m_SrcMap.size() - 1;
    double di = ( u - umin ) * ( m_NumMap - 1 ) * npatchu / du;
    i = ( int ) di;
    fraci = di - i;
    if( i >= imax )
    {
        i = imax - 1;
        fraci = 1.0;
    }

    int jmax = m_SrcMap[0].size() - 1;
    double dj = ( w - wmin ) * ( m_NumMap - 1 ) * npatchw / dw;
    j = ( int ) dj;
    fracj = dj - j;
    if( j >= jmax )
    {
        j = jmax - 1;
        fracj = 1.0;
    }
}

void Surf::UWtoTargetMapij( double u, double w, int &i, int &j )
{
    double fraci, fracj;
    UWtoTargetMapij( u, w, i, j, fraci, fracj );
}

void Surf::ApplyES( vec3d uw, double t, int reason )
{
    double grm1 = m_GridDensityPtr->m_GrowRatio - 1.0;
    int nmapu = m_SrcMap.size();
    int nmapw = m_SrcMap[0].size();

    int ibase, jbase;
    double u = uw.x();
    double w = uw.y();
    UWtoTargetMapij( u, w, ibase, jbase );

    vec3d p = m_SurfCore.CompPnt( u, w );

    int iadd[] = { 0, 1, 0, 1 };
    int jadd[] = { 0, 0, 1, 1 };

    for( int i = 0; i < 4; i++ )
    {
        int itarget = ibase + iadd[i];
        int jtarget = jbase + jadd[i];

        if( itarget < nmapu && itarget >= 0 && jtarget < nmapw && jtarget >= 0 )
        {
            vec3d p2 = m_SrcMap[ itarget ][ jtarget ].m_pt;
            double r = ( p2 - p ).mag();
            double targetstr = t + r * grm1;
            if( m_SrcMap[ itarget ][ jtarget ].m_str > targetstr )
            {
                m_SrcMap[ itarget ][ jtarget ].m_str = targetstr;

                if ( reason < vsp::MIN_GROW_LIMIT )
                {
                    m_SrcMap[ itarget ][ jtarget ].m_reason = reason + vsp::GROW_LIMIT_INCREMENT;
                }
                else
                {
                    m_SrcMap[ itarget ][ jtarget ].m_reason = reason;
                }

                WalkMap( itarget, jtarget );
            }
        }
    }
}

vec2d Surf::ClosestUW( const vec3d & pnt_in, double guess_u, double guess_w ) const
{
    double u, w;
    m_SurfCore.FindNearest( u, w, pnt_in, guess_u, guess_w );
    return vec2d( u, w );
}

vec2d Surf::ClosestUW( const vec3d & pnt_in ) const
{
    double u, w;
    m_SurfCore.FindNearest( u, w, pnt_in );
    return vec2d( u, w );
}

void Surf::FindBorderCurves()
{
    double degen_tol = 1.0e-6;

    //==== Load 4 Border Curves if Not Degenerate ====//
    SCurve* scrv;
    double min_u = m_SurfCore.GetMinU();
    double min_w = m_SurfCore.GetMinW();
    double max_u = m_SurfCore.GetMaxU();
    double max_w = m_SurfCore.GetMaxW();

    vector< vec3d > pnts;
    pnts.resize( 2 );

    pnts[0].set_xyz( min_u, min_w, 0 );         // Inc U
    pnts[1].set_xyz( max_u, min_w, 0 );

    scrv = new SCurve( this );
    scrv->InterpolateLinear( pnts );
    scrv->PromoteTo( 3 );  // Need to be cubic as intermediate points are checked for degeneracy.

    if ( scrv->Length( 10 ) > degen_tol )
    {
        m_SCurveVec.push_back( scrv );
    }
    else
    {
        delete scrv;
    }

    pnts[0].set_xyz( max_u, min_w, 0 );       // Inc W
    pnts[1].set_xyz( max_u, max_w, 0 );

    scrv = new SCurve( this );
    scrv->InterpolateLinear( pnts );
    scrv->PromoteTo( 3 );  // Need to be cubic as intermediate points are checked for degeneracy.

    if ( scrv->Length( 10 ) > degen_tol )
    {
        m_SCurveVec.push_back( scrv );
    }
    else
    {
        delete scrv;
    }

    pnts[0].set_xyz( max_u, max_w, 0 );         // Dec U
    pnts[1].set_xyz( min_u, max_w, 0 );

    scrv = new SCurve( this );
    scrv->InterpolateLinear( pnts );
    scrv->PromoteTo( 3 );  // Need to be cubic as intermediate points are checked for degeneracy.

    if ( scrv->Length( 10 ) > degen_tol )
    {
        m_SCurveVec.push_back( scrv );
    }
    else
    {
        delete scrv;
    }

    pnts[0].set_xyz( min_u, max_w,   0 );           // Dec W
    pnts[1].set_xyz( min_u, min_w,   0 );

    scrv = new SCurve( this );
    scrv->InterpolateLinear( pnts );
    scrv->PromoteTo( 3 );  // Need to be cubic as intermediate points are checked for degeneracy.

    if ( scrv->Length( 10 ) > degen_tol )
    {
        m_SCurveVec.push_back( scrv );
    }
    else
    {
        delete scrv;
    }
}

void Surf::LoadSCurves( vector< SCurve* > & scurve_vec )
{
    for ( int i = 0 ; i < ( int )m_SCurveVec.size() ; i++ )
    {
        scurve_vec.push_back( m_SCurveVec[i] );
    }
}

void Surf::BuildGrid()
{
    int i;
    vector< vec3d > uw_border;

    for ( i = 0 ; i < ( int )m_SCurveVec.size() ; i++ )
    {
        vector< vec3d > suw_vec = m_SCurveVec[i]->GetUWTessPnts();

        for ( int j = 0 ; j < ( int )suw_vec.size() ; j++ )
        {
            if ( uw_border.size() )         // Check For Duplicate Points
            {
                double d = dist( uw_border.back(), suw_vec[j] );
                if ( d > 1.0e-7 )
                {
                    uw_border.push_back( suw_vec[j] );
                }
            }
            else
            {
                uw_border.push_back( suw_vec[j] );
            }
        }
    }

    //for ( i = 0 ; i < 10 ; i++ )
    //{
    //  m_Mesh.Remesh(100);

    //}

}

void Surf::WriteSTL( const char* filename )
{
    m_Mesh.WriteSTL( filename );
}

void Surf::Intersect( Surf* surfPtr, SurfaceIntersectionSingleton *MeshMgr )
{
    int i;

    if ( surfPtr->GetCompID() == m_CompID )
    {
        return;
    }

    if ( !Compare( m_BBox, surfPtr->GetBBox() ) )
    {
        return;
    }
    if ( BorderCurveOnSurface( surfPtr, MeshMgr ) )
    {
        return;
    }
    if ( surfPtr->BorderCurveOnSurface( this, MeshMgr ) )
    {
        return;
    }

    vector< SurfPatch* > otherPatchVec = surfPtr->GetPatchVec();
    for ( i = 0 ; i < ( int )m_PatchVec.size() ; i++ )
        if ( Compare( *m_PatchVec[i]->get_bbox(), surfPtr->GetBBox() ) )
        {
            for ( int j = 0 ; j < ( int )otherPatchVec.size() ; j++ )
            {
                if ( Compare( *m_PatchVec[i]->get_bbox(), *otherPatchVec[j]->get_bbox() ) )
                {
                    intersect( *m_PatchVec[i], *otherPatchVec[j], MeshMgr );
                }
            }
        }
}

void Surf::IntersectLineSeg( vec3d & p0, vec3d & p1, vector< double > & t_vals )
{
    BndBox line_box;
    line_box.Update( p0 );
    line_box.Update( p1 );

    if ( !Compare( line_box, m_BBox ) )
    {
        return;
    }

    for ( int i = 0 ; i < ( int )m_PatchVec.size() ; i++ )
    {
        m_PatchVec[i]->IntersectLineSeg( p0, p1, line_box, t_vals );
    }
}

bool Surf::BorderCurveOnSurface( Surf* surfPtr, SurfaceIntersectionSingleton *MeshMgr )
{
    bool retFlag = false;
    double tol = 1.0e-05;

    if ( this->GetSurfaceCfdType() == vsp::CFD_STRUCTURE )
    {
        return retFlag;
    }

    vector< SCurve* > border_curves;
    surfPtr->LoadSCurves( border_curves );

    for ( int i = 0 ; i < ( int )border_curves.size() ; i++ )
    {
        Bezier_curve crv;
        border_curves[i]->GetBorderCurve( crv );

        BndBox crvbox;
        crv.GetBBox( crvbox );

        if ( Compare( m_BBox, crvbox ) )
        {
            Bezier_curve projcrv = crv;
            projcrv.XYZCurveToUWCurve( this );
            projcrv.UWCurveToXYZCurve( this );

            int num_pnts_on_surf = crv.CountMatch( projcrv, tol );

            if ( num_pnts_on_surf > 2 || ( num_pnts_on_surf == 2 && crv.SingleLinear() ) )
            {
                retFlag = true;
                //==== If Surface Add To List ====//
                MeshMgr->AddPossCoPlanarSurf( this, surfPtr );
                PlaneBorderCurveIntersect( surfPtr, border_curves[i], MeshMgr );
            }
        }
    }

    return retFlag;
}

void Surf::PlaneBorderCurveIntersect( Surf* surfPtr, SCurve* brdPtr, SurfaceIntersectionSingleton *MeshMgr )
{
    bool repeat_curve = false;
    bool null_ICurve = false;
    if ( brdPtr->GetICurve() != NULL )
    {
        for ( int j = 0 ; j < (int)m_SCurveVec.size() ; j++ )
        {
            if ( brdPtr->GetICurve() == m_SCurveVec[j]->GetICurve() )
            {
                repeat_curve = true;
            }
        }
    }
    else
    {
        null_ICurve = true;
    }

    if ( !repeat_curve )
    {
        SCurve* pSCurve = new SCurve;
        SCurve* bSCurve = new SCurve;
        ICurve* pICurve = new ICurve;

        ICurve* bICurve = pICurve;
        ICurve* obICurve = brdPtr->GetICurve();

        vector< ICurve* > ICurves = MeshMgr->GetICurveVec();
        int ICurveVecIndex;

        Bezier_curve crv = brdPtr->GetUWCrv();
        crv.UWCurveToXYZCurve( surfPtr );
        crv.XYZCurveToUWCurve( this );
        pSCurve->SetUWCrv( crv );

        pICurve->m_SCurve_A = brdPtr;
        pICurve->m_SCurve_B = pSCurve;
        pICurve->m_PlaneBorderIntersectFlag = true;
        pSCurve->SetSurf( this );
        pSCurve->SetICurve( pICurve );

        bICurve->m_SCurve_A = pSCurve;
        bICurve->m_SCurve_B = brdPtr;
        bICurve->m_PlaneBorderIntersectFlag = true;
        bSCurve->SetSurf( surfPtr );
        bSCurve->SetICurve( bICurve );

        brdPtr->SetICurve( bICurve );

        if ( !null_ICurve )
        {
            ICurveVecIndex = distance( ICurves.begin(), find( ICurves.begin(), ICurves.end(), obICurve ) );
            if ( ICurveVecIndex < (int)ICurves.size() )
            {
                MeshMgr->SetICurveVec( brdPtr->GetICurve(), ICurveVecIndex );
            }
        }
        else
        {
            for ( int i = 0 ; i < (int)ICurves.size() ; i++ )
            {
                if ( ICurves[i]->m_SCurve_A == brdPtr && ICurves[i]->m_SCurve_B == NULL )
                {
                    ICurves[i]->m_SCurve_B = pSCurve;
                    ICurves[i]->m_PlaneBorderIntersectFlag = true;
                }
            }
        }

        m_SCurveVec.push_back( pSCurve );
    }
}

void Surf::SetBBox( const vec3d &pmin, const vec3d &pmax )
{
    m_BBox.Reset();
    m_BBox.Update( pmin );
    m_BBox.Update( pmax );
}


void Surf::InitMesh( vector< ISegChain* > chains, const vector < vec2d > &adduw, SurfaceIntersectionSingleton *MeshMgr )
{
    //==== Store Only One Instance of each IPnt ====//
    set< IPnt* > ipntSet;
    for ( int i = 0 ; i < ( int )chains.size() ; i++ )
    {
        for ( int j = 0 ; j < ( int )chains[i]->m_TessVec.size() ; j += 2 )  // Note every other point fed.
        {
            ipntSet.insert( chains[i]->m_TessVec[j] );
        }
    }

    vector < vec2d > uwPntVec;

    set< IPnt* >::iterator ip;
    for ( ip = ipntSet.begin() ; ip != ipntSet.end() ; ++ip )
    {
        vec2d uw = ( *ip )->GetPuw( this )->m_UW;

        int min_id = -1;
        double min_dist = 1.0;
        for ( int i = 0 ; i < ( int )uwPntVec.size() ; i++ )
        {
            double d = dist( uwPntVec[i], uw );
            if ( d < min_dist )
            {
                min_dist = d;
                min_id = i;
            }
        }

        if ( min_dist < 1.0e-4 )
        {
            ( *ip )->m_Index = min_id;
        }
        else
        {
            uwPntVec.push_back( uw );
            ( *ip )->m_Index = uwPntVec.size() - 1;
        }
    }

    // Add additional points for Triangle -- these are structures Fix Points.
    for ( int i = 0; i < adduw.size(); i++ )
    {
        uwPntVec.push_back( adduw[i] );
    }

    MeshSeg seg;
    vector< MeshSeg > isegVec;
    for ( int i = 0 ; i < ( int )chains.size() ; i++ )
    {
        int n = chains[i]->m_TessVec.size();
        int nhalf = 0.5 * ( n - 1 )  + 1;
        for ( int j = 0 ; j < nhalf - 1; j++ )
        {
            seg.m_Index[0] = chains[i]->m_TessVec[2 * j]->m_Index;
            seg.m_Index[1] = chains[i]->m_TessVec[2 * (j + 1)]->m_Index;
            seg.m_UWmid = chains[i]->m_TessVec[2 * j + 1]->GetPuw( this )->m_UW;
            isegVec.push_back( seg );
        }
    }

    BuildDistMap();

    m_Mesh.InitMesh( uwPntVec, isegVec, MeshMgr );

    CleanupDistMap();
}


void Surf::BuildDistMap()
{
    int i, j;
    const unsigned int nump = 11;

    double VspMinU = m_SurfCore.GetMinU();
    double VspMinW = m_SurfCore.GetMinW();

    double VspMaxU = m_SurfCore.GetMaxU();
    double VspMaxW = m_SurfCore.GetMaxW();

    double VspdU = VspMaxU - VspMinU;
    double VspdW = VspMaxW - VspMinW;

    using ABF = OpenABF::ABFPlusPlus < double >;
    using LSCM = OpenABF::AngleBasedLSCM < double, ABF::Mesh >;

    //auto mesh = vspMesh< double >::New();
    auto mesh = ABF::Mesh::New();

    //==== Load Point Vec ====//

    vector < double > uvec, wvec;

    uvec.resize( nump );
    for ( i = 0 ; i < nump ; i++ )
    {
        double u = VspMinU + VspdU * ( double ) i / ( double ) ( nump - 1 );
        uvec[i] = u;
    }

    wvec.resize( nump );
    for ( j = 0 ; j < nump ; j++ )
    {
        double w = VspMinW + VspdW * ( double ) j / ( double ) ( nump - 1 );
        wvec[j] = w;
    }

    vector < vec3d > pvec( nump * nump );
    vector< vector< int > > ptindx( nump, vector< int > ( nump, 0 ) );
    vector< int > ki( nump * nump );
    vector< int > kj( nump * nump );
    int k = 0;
    for ( i = 0 ; i < nump ; i++ )
    {
        double u = uvec[i];
        for ( j = 0 ; j < nump ; j++ )
        {
            double w = wvec[j];
            pvec[k] = m_SurfCore.CompPnt( u, w );
            ki[k] = i;
            kj[k] = j;
            ptindx[i][j] = k;
            k++;
        }
    }

    PntNodeCloud pnCloud;
    //==== Build Map ====//
    pnCloud.AddPntNodes( pvec );

    //==== Use NanoFlann to Find Close Points and Group ====//
    IndexPntNodes( pnCloud, 1e-6 );

    //==== Load Used Points ====//
    vector < vec3d > usedPts;
    vector < int > kused;
    for ( i = 0; i < (int)pvec.size(); i++ )
    {
        if ( pnCloud.UsedNode( i ) )
        {
            mesh->insert_vertex( pvec[i].x(), pvec[i].y(), pvec[i].z() );
            kused.push_back( i );
        }
    }

    for ( i = 0 ; i < nump - 1; i++ )
    {
        for ( j = 0; j < nump - 1; j++ )
        {
            int i0, i1, i2, i3;

            i0 = pnCloud.GetNodeUsedIndex( ptindx[ i ][ j ] );
            i1 = pnCloud.GetNodeUsedIndex( ptindx[ i + 1 ][ j ] );
            i2 = pnCloud.GetNodeUsedIndex( ptindx[ i ][ j + 1 ] );
            i3 = pnCloud.GetNodeUsedIndex( ptindx[ i + 1 ][ j + 1 ] );

            if ( (i0 != i1) && (i0 != i2) && (i1 != i2) )
            {
                mesh->insert_face( i0, i1, i2 );
            }

            if ( (i1 != i3) && (i1 != i2) && (i3 != i2) )
            {
                mesh->insert_face( i1, i3, i2 );
            }
        }
    }


    ABF::Compute( mesh );
    LSCM::Compute( mesh );

    vector< vector< double > > smat( nump, vector< double > ( nump, -1.0 ) );
    vector< vector< double > > tmat( nump, vector< double > ( nump, -1.0 ) );

    vector < ABF::Mesh::VertPtr > verts = mesh->vertices();
    for ( i = 0; i < verts.size(); i++ )
    {
        double r = verts[ i ]->pos[ 0 ];
        double t = verts[ i ]->pos[ 1 ];

        vector < long long int > match = pnCloud.GetMatches( kused[ i ] );

        for ( j = 0; j < match.size(); j++ )
        {
            smat[ ki[ match[ j ] ] ][ kj[ match[ j ] ] ] = r;
            tmat[ ki[ match[ j ] ] ][ kj[ match[ j ] ] ] = t;
        }
    }


    m_STMap.resize( nump );
    for ( i = 0 ; i < nump ; i++ )
    {
        m_STMap[i].resize( nump );
        for ( j = 0 ; j < nump ; j++ )
        {
            m_STMap[i][j] = vec2d( smat[i][j], tmat[i][j] );
        }
    }

    m_UWMap.AddPntNodes( m_STMap );
    m_UWMap.BuildIndex();

#ifdef DEBUG_CFD_MESH
    static int cnt = 0;

    if ( true )
    {
        char str[256];
        snprintf( str, sizeof( str ), "stmat_%d.m", cnt );

        FILE *fp = fopen( str, "w" );

        WriteMatDoubleM writeMatDouble;

        writeMatDouble.write( fp, smat, string( "smat" ), nump, nump );

        writeMatDouble.write( fp, tmat, string( "tmat" ), nump, nump );

        fprintf( fp, "figure(2)\n" );
        fprintf( fp, "plot( smat, tmat, smat', tmat' );\n" );

        fclose( fp );
    }

    cnt++;
#endif
}

void Surf::UtoIndexFrac( const double &u, int &indx, double &frac )
{
    int num = m_STMap.size();

    double indd = u * (double) ( num - 1 );

    indx = ( int ) indd;
    indx = clamp( indx, 0, num - 2 );

    frac = indd - ( double )indx;
    frac = clamp( frac, 0.0, 1.0 );
}

vec2d Surf::GetST( const vec2d &uw )
{
    double VspMinU = m_SurfCore.GetMinU();
    double VspMinW = m_SurfCore.GetMinW();

    double VspMaxU = m_SurfCore.GetMaxU();
    double VspMaxW = m_SurfCore.GetMaxW();

    double VspdU = VspMaxU - VspMinU;
    double VspdW = VspMaxW - VspMinW;

    double u = ( uw.x() - VspMinU ) / VspdU;
    double w = ( uw.y() - VspMinW ) / VspdW;

    int iu, iw;
    double fu, fw;
    UtoIndexFrac( u, iu, fu );
    UtoIndexFrac( w, iw, fw );

    vec2d st;
    bi_lin_interp( m_STMap[ iu ][ iw ], m_STMap[ iu + 1 ][ iw ], m_STMap[ iu ][ iw + 1 ], m_STMap[ iu + 1 ][ iw + 1 ], fu, fw, st );

    return st;
}

void Surf::FindSTBox( const vec2d &st, int &i_match, int &j_match )
{
    bool debugprint = false;

    i_match = 0;
    j_match = 0;

    int res = m_UWMap.LookupPnt( st );

    if ( res >= 0 )
    {
        i_match = m_UWMap.m_PntNodes[ res ].m_iU;
        j_match = m_UWMap.m_PntNodes[ res ].m_iV;

        int ni = m_STMap.size();
        int nj = m_STMap[ 0 ].size();

        // Clamp to two less than number of entries.
        // This handles both zero indexing and also guarantees that i+1 and j+1 are valid indices.
        i_match = clamp( i_match, 0, ni - 2 );
        j_match = clamp( j_match, 0, nj - 2 );

        if ( debugprint )
        {
            vector < vec2d > poly = { m_STMap[ i_match ][ j_match ], m_STMap[ i_match + 1 ][ j_match ], m_STMap[ i_match + 1 ][ j_match + 1 ], m_STMap[ i_match ][ j_match + 1 ], m_STMap[ i_match ][ j_match ] };

            printf( "\n\nplot([" );
            for ( int ipoly = 0; ipoly < poly.size(); ipoly++ )
            {
                printf( "%f ", poly[ ipoly ].x());
            }
            printf( "],[" );
            for ( int ipoly = 0; ipoly < poly.size(); ipoly++ )
            {
                printf( "%f ", poly[ ipoly ].y());
            }
            printf( "],%f, %f,'x');\n", st.x(), st.y());

            if ( PointInPolygon( st, poly ) )
            {
                printf( "%% In polygon.\n" );
            }
            else
            {
                printf( "%% Outside polygon.\n" );
            }
        }

        bool stop = false;

        int n = 0;
        while ( !stop )
        {
            int di = 0;
            int dj = 0;

            int i_old = i_match;
            int j_old = j_match;

            if ( orient2d( m_STMap[ i_match ][ j_match ], m_STMap[ i_match + 1 ][ j_match ], st ) < 0 )
            {
                dj -= 1;
            }

            if ( orient2d( m_STMap[ i_match + 1 ][ j_match ], m_STMap[ i_match + 1 ][ j_match + 1 ], st ) < 0 )
            {
                di += 1;
            }

            if ( orient2d( m_STMap[ i_match + 1 ][ j_match + 1 ], m_STMap[ i_match ][ j_match + 1 ], st ) < 0 )
            {
                dj += 1;
            }

            if ( orient2d( m_STMap[ i_match ][ j_match + 1 ], m_STMap[ i_match ][ j_match ], st ) < 0 )
            {
                di -= 1;
            }

            i_match += di;
            j_match += dj;

            i_match = clamp( i_match, 0, ni - 2 );
            j_match = clamp( j_match, 0, nj - 2 );

            di = i_old - i_match;
            dj = j_old - j_match;

            n++;

            // di and dj == 0 can result from either the point lying in the polygon, or from clamp enforcing boundaries.
            if ( di == 0 && dj == 0 )
            {
                stop = true;
            }
            else if ( n > 10 ) // Abundance of caution.
            {
                stop = true;
            }
        }

        if ( debugprint )
        {
            vector < vec2d > poly = { m_STMap[ i_match ][ j_match ], m_STMap[ i_match + 1 ][ j_match ], m_STMap[ i_match + 1 ][ j_match + 1 ], m_STMap[ i_match ][ j_match + 1 ], m_STMap[ i_match ][ j_match ] };

            printf( "hold on; plot([" );
            for ( int ipoly = 0; ipoly < poly.size(); ipoly++ )
            {
                printf( "%f ", poly[ ipoly ].x());
            }
            printf( "],[" );
            for ( int ipoly = 0; ipoly < poly.size(); ipoly++ )
            {
                printf( "%f ", poly[ ipoly ].y());
            }
            printf( "],%f, %f,'x'); hold off;\n", st.x(), st.y());

            if ( PointInPolygon( st, poly ) )
            {

                printf( "%% Success after one iteration.\n" );
            }
            else
            {
                printf( "%% Still failing.\n" );
            }
        }
    }
}

vec2d Surf::GetUW( const vec2d &st )
{
    int num = m_STMap.size();

    int i, j;

    FindSTBox( st, i, j );

    double VspMinU = m_SurfCore.GetMinU();
    double VspMinW = m_SurfCore.GetMinW();

    double VspMaxU = m_SurfCore.GetMaxU();
    double VspMaxW = m_SurfCore.GetMaxW();

    double VspdU = VspMaxU - VspMinU;
    double VspdW = VspMaxW - VspMinW;

    double fu, fw, u2, w2;
    inverse_bi_lin_interp( m_STMap[ i ][ j ], m_STMap[ i + 1 ][ j ], m_STMap[ i ][ j + 1 ], m_STMap[ i + 1 ][ j + 1 ], st, fu, fw, u2, w2 );

    double iud = i + fu;
    double iwd = j + fw;

    double u01 = clamp( iud / ( double )( num - 1 ), 0.0, 1.0 );
    double w01 = clamp( iwd / ( double )( num - 1 ), 0.0, 1.0 );

    double u = VspMinU + u01 * VspdU;
    double w = VspMinW + w01 * VspdW;

    return vec2d( u, w );
}

void Surf::CleanupDistMap()
{
    m_UWMap.Cleanup();
    m_STMap.clear();
}

bool Surf::ValidUW( vec2d & uw, double slop ) const
{
    //return true;
    if ( uw[0] < m_SurfCore.GetMinU() - slop )
    {
        return false;
    }
    if ( uw[1] < m_SurfCore.GetMinW() - slop )
    {
        return false;
    }
    if ( uw[0] > m_SurfCore.GetMaxU() + slop )
    {
        return false;
    }
    if ( uw[1] > m_SurfCore.GetMaxW() + slop )
    {
        return false;
    }

    if ( uw[0] < m_SurfCore.GetMinU() )
    {
        uw[0] = m_SurfCore.GetMinU();
    }
    if ( uw[1] < m_SurfCore.GetMinW() )
    {
        uw[1] = m_SurfCore.GetMinW();
    }
    if ( uw[0] > m_SurfCore.GetMaxU() )
    {
        uw[0] = m_SurfCore.GetMaxU();
    }
    if ( uw[1] > m_SurfCore.GetMaxW() )
    {
        uw[1] = m_SurfCore.GetMaxW();
    }

    return true;
}

bool Surf::BorderMatch( Surf* otherSurf )
{
    double tol = 1e-4;

    vector < Bezier_curve > borderCurvesA;
    m_SurfCore.LoadBorderCurves( borderCurvesA );

    vector < Bezier_curve > borderCurvesB;
    otherSurf->GetSurfCore()->LoadBorderCurves( borderCurvesB );

    for ( int i = 0 ; i < ( int )borderCurvesA.size() ; i++ )
    {
        for ( int j = 0 ; j < ( int )borderCurvesB.size() ; j++ )
        {
            if ( borderCurvesA[i].Match( borderCurvesB[j], tol ) )
            {
                return true;
            }
        }
    }
    return false;
}

bool Surf::BorderMatch( int iborder, Surf* otherSurf )
{
    double tol = 1e-4;

    Bezier_curve borderA = m_SurfCore.GetBorderCurve( iborder );

    vector < Bezier_curve > borderCurvesB;
    otherSurf->GetSurfCore()->LoadBorderCurves( borderCurvesB );

    for ( int j = 0 ; j < ( int )borderCurvesB.size() ; j++ )
    {
        if ( borderA.Match( borderCurvesB[j], tol ) )
        {
            return true;
        }
    }

    return false;
}


void Surf::Subtag( bool tag_subs )
{
    vector< SimpFace >& face_vec = m_Mesh.GetSimpFaceVec();
    const vector< vec2d >& pnts = m_Mesh.GetSimpUWPntVec();
    vector< SubSurface* > s_surfs;

    if ( tag_subs ) s_surfs = SubSurfaceMgr.GetSubSurfs( m_GeomID, m_MainSurfID );

    for ( int f = 0 ; f < ( int ) face_vec.size() ; f++ )
    {
        SimpFace& face = face_vec[f];
        face.m_Tags.push_back( m_BaseTag );
        vec2d center;
        if ( face.m_isQuad )
        {
            center = ( pnts[face.ind0] + pnts[face.ind1] + pnts[face.ind2] + pnts[face.ind3] ) * 1 / 4.0;
        }
        else
        {
            center = ( pnts[face.ind0] + pnts[face.ind1] + pnts[face.ind2] ) * 1 / 3.0;
        }

        for ( int s = 0 ; s < ( int ) s_surfs.size() ; s++ )
        {
            if ( s_surfs[s]->Subtag( vec3d( center.x(), center.y(), 0 ) ) )
            {
                face.m_Tags.push_back( s_surfs[s]->m_Tag );
            }
        }
        SubSurfaceMgr.m_TagCombos.insert( face.m_Tags );
    }
}

/*
void Surf::Draw()
{
    ////==== Draw Control Hull ====//
    //glLineWidth( 2.0 );
    //glColor3ub( 0, 0, 255 );

    //for ( int i = 0 ; i < (int)m_Pnts.size() ; i++ )
    //{
    //  glBegin( GL_LINE_STRIP );
    //  for ( int j = 0 ; j < (int)m_Pnts[i].size() ; j++ )
    //  {
    //      glVertex3dv( m_Pnts[i][j].data() );
    //  }
    //  glEnd();
    //}
    //glPointSize( 3.0 );
    //glColor3ub( 255, 255, 255 );
    //glBegin( GL_POINTS );
    //for ( int i = 0 ; i < (int)m_Pnts.size() ; i++ )
    //{
    //  for ( int j = 0 ; j < (int)m_Pnts[i].size() ; j++ )
    //  {
    //      glVertex3dv( m_Pnts[i][j].data() );
    //  }
    //}
    //glEnd();



    //==== Draw Surface ====//
    int max_u = (m_NumU-1)/3;
    int max_w = (m_NumW-1)/3;

    glLineWidth( 1.0 );
    glColor3ub( 0, 255, 0 );

    int num_xsec = 10;
    int num_tess = 20;
    for ( int i = 0 ; i < num_xsec ; i++ )
    {
        double u = max_u*(double)i/(double)(num_xsec-1);
        glBegin( GL_LINE_STRIP );
        for ( int j = 0 ; j < num_tess ; j++ )
        {
            double w = max_w*(double)j/(double)(num_tess-1);
            vec3d p = CompPnt( u, w );
            glVertex3dv( p.data() );
        }
        glEnd();
    }

    for ( int j = 0 ; j < num_xsec ; j++ )
    {
        double w = max_w*(double)j/(double)(num_xsec-1);
        glBegin( GL_LINE_STRIP );
        for ( int i = 0 ; i < num_tess ; i++ )
        {
            double u = max_u*(double)i/(double)(num_tess-1);
            vec3d p = CompPnt( u, w );
            glVertex3dv( p.data() );

        }
        glEnd();
    }

    //for ( int i = 0 ; i < (int)m_SCurveVec.size() ; i++ )
    //{
    //  m_SCurveVec[i]->Draw();
    //}

    m_Mesh.Draw();

    //for ( int i = 0 ; i < (int)m_PatchVec.size() ; i++ )
    //{
    //  m_PatchVec[i]->Draw();
    //}

    //glLineWidth( 2.0 );
    //glColor3ub( 255, 0, 0 );
    //glBegin( GL_LINES );
    //for ( int i = 0 ; i < (int)ipnts.size() ; i++ )
    //{
    //  if ( i%4 > 1 )
    //      glColor3ub( 255, 0, 0 );
    //  else
    //      glColor3ub( 255, 255, 0 );


    //  vec3d uw = ipnts[i];
    //  vec3d p = CompPnt( uw[0], uw[1] );
    //  glVertex3dv( p.data() );
    //}
    //glEnd();



}
*/

vec3d Surf::CompPnt( double u, double w ) const
{
    return m_SurfCore.CompPnt( u, w );
}

vec3d Surf::CompPnt01( double u, double w ) const
{
    return m_SurfCore.CompPnt01( u, w );
}

vec3d Surf::CompNorm( double u, double w ) const
{
    return m_SurfCore.CompNorm( u, w );
}

// Compute the individual element material orientation after mesh has been created.  Consequently, we
// know the U, V coordinates of element centers required to find the local directions used by NASTRAN in some cases.
vec3d Surf::GetFeaElementOrientation( double u, double w )
{
    return GetFeaElementOrientation( u, w, m_FeaOrientationType, m_FeaOrientation );
}

vec3d Surf::GetFeaElementOrientation( double u, double w, int type, const vec3d & defaultorientation )
{
    // All COMP_XYZ, OML_UVRST and cases with invalid u, w
    vec3d orient = defaultorientation;
    if ( type == vsp::FEA_ORIENT_GLOBAL_X )
    {
        orient = vec3d( 1.0, 0, 0 );
    }
    else if ( type == vsp::FEA_ORIENT_GLOBAL_Y )
    {
        orient = vec3d( 0, 1.0, 0 );
    }
    else if ( type == vsp::FEA_ORIENT_GLOBAL_Z )
    {
        orient = vec3d( 0, 0, 1.0 );
    }
    else if ( type == vsp::FEA_ORIENT_PART_U )
    {
        vec2d uw = vec2d( u, w );
        if ( ValidUW( uw ) )
        {
            orient = m_SurfCore.CompTanU( u, w );
        }
    }
    else if ( type == vsp::FEA_ORIENT_PART_V )
    {
        vec2d uw = vec2d( u, w );
        if ( ValidUW( uw ) )
        {
            orient = m_SurfCore.CompTanW( u, w );
        }
    }

    return orient;
}

// Compute the per-surface material orientation for CalculiX.  Since no per-element information is available,
// the per-surface stored orientation is used.
vec3d Surf::GetFeaElementOrientation()
{
    // All COMP_XYZ, OML_UVRST, PART_UV
    vec3d orient = m_FeaOrientation;

    // Global XYZ are done here as they are independent of the transformations applied to the other orientations.
    if ( m_FeaOrientationType == vsp::FEA_ORIENT_GLOBAL_X )
    {
        orient = vec3d( 1.0, 0, 0 );
    }
    else if ( m_FeaOrientationType == vsp::FEA_ORIENT_GLOBAL_Y )
    {
        orient = vec3d( 0, 1.0, 0 );
    }
    else if ( m_FeaOrientationType == vsp::FEA_ORIENT_GLOBAL_Z )
    {
        orient = vec3d( 0, 0, 1.0 );
    }

    return orient;
}
