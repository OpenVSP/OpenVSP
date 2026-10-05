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
#include <set>
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

// Include this file to ensure alternative operators are available on all platforms.
#include <iso646.h>
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
    m_CopyIndex = -1;
    m_SymPlaneFlag = false;
    m_FarFlag = false;
    m_WakeParentSurfID = -1;
    m_Mesh.SetSurfPtr( this );
    m_NumMap = 11;
    m_WalkVisitID = 0;
    m_BaseTag = 1;
    m_MainSurfID = 0;
    m_SplitNum = 0;
    m_FeaPartIndex = -1;
    m_FeaPartSurfNum = -1;
    m_FeaSymmIndex = -1;
    m_IgnoreSurfFlag = false;
    m_PlanarUWAspect = -1;
    m_DistMapBuilt = false;
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

// A point can sit exactly on the seam between two regions, and the parameters carry rounding
// from the splits that produced them, so the containment tests are given a little room.
static const double uwregion_tol = 1.0e-8;

int Surf::FindRegionPatchUW( double u, double w ) const
{
    for ( int i = 0; i < ( int )m_UWRegions.size(); i++ )
    {
        double ulo, uhi, wlo, whi;
        m_UWRegions[i].PatchExtentU( ulo, uhi );
        m_UWRegions[i].PatchExtentW( wlo, whi );

        if ( u >= ulo - uwregion_tol && u <= uhi + uwregion_tol &&
             w >= wlo - uwregion_tol && w <= whi + uwregion_tol )
        {
            return i;
        }
    }
    return -1;
}

vector < UWRegion > Surf::GetUWRegionsOrWhole() const
{
    if ( !m_UWRegions.empty() )
    {
        return m_UWRegions;
    }

    vector < UWRegion > rv( 1 );
    rv[0].m_UMin = m_SurfCore.GetMinU();
    rv[0].m_UMax = m_SurfCore.GetMaxU();
    rv[0].m_WMin = m_SurfCore.GetMinW();
    rv[0].m_WMax = m_SurfCore.GetMaxW();
    return rv;
}

bool Surf::ToOriginalUW( double u, double w, double &uo, double &wo ) const
{
    if ( m_UWRegions.empty() )
    {
        uo = u;
        wo = w;
        return true;
    }


    int i = FindRegionPatchUW( u, w );

    if ( i < 0 )
    {
        return false;
    }

    uo = m_UWRegions[i].ToOrigU( u );
    wo = m_UWRegions[i].ToOrigW( w );
    return true;
}

bool Surf::ToPatchUW( double uo, double wo, double &u, double &w ) const
{
    if ( m_UWRegions.empty() )
    {
        u = uo;
        w = wo;
        return true;
    }

    for ( int i = 0; i < ( int )m_UWRegions.size(); i++ )
    {
        const UWRegion &r = m_UWRegions[i];

        if ( uo >= r.m_UMin - uwregion_tol && uo <= r.m_UMax + uwregion_tol &&
             wo >= r.m_WMin - uwregion_tol && wo <= r.m_WMax + uwregion_tol )
        {
            u = r.ToPatchU( uo );
            w = r.ToPatchW( wo );
            return true;
        }
    }
    return false;
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

// The number of points BuildTargetMap will lay on this surface.
int Surf::GetTargetMapSize() const
{
    int nmapu = m_SurfCore.GetNumUPatches() * ( m_NumMap - 1 ) + 1;
    int nmapw = m_SurfCore.GetNumWPatches() * ( m_NumMap - 1 ) + 1;
    return nmapu * nmapw;
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

    // Scratch stamps for WalkMap.  Zero is reserved as "never visited" so the
    // first walk can use ID one without clearing.
    m_WalkVisited.assign( ( size_t )nmapu * ( size_t )nmapw, 0 );
    m_WalkVisitID = 0;

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

                if ( reason >= vsp::NUM_MESH_REASON ) // Should be impossible, just as a safety check.
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
    static const int iadd[] = { -1, 1,  0, 0 };
    static const int jadd[] = {  0, 0, -1, 1 };

    const int nmapu = ( int )m_SrcMap.size();
    const int nmapw = ( int )m_SrcMap[0].size();

    // The seed cannot improve itself, so what is measured from it holds for the whole walk.
    const vec3d pstart = m_SrcMap[istart][jstart].m_pt;
    const double strstart = m_SrcMap[istart][jstart].m_str;
    const double grm1 = m_GridDensityPtr->m_GrowRatio - 1.0;

    int reason = m_SrcMap[istart][jstart].m_reason;
    if ( reason < vsp::MIN_GROW_LIMIT )
    {
        reason += vsp::GROW_LIMIT_INCREMENT;
    }

    vector < pair < int, int > > &v = m_WalkStack;
    v.clear();

    for( int i = 0; i < 4; i++ )
    {
        int inext = istart + iadd[i];
        int jnext = jstart + jadd[i];

        if( inext < nmapu && inext >= 0 && jnext < nmapw && jnext >= 0 )
        {
            v.push_back( make_pair( inext, jnext ) );
        }
    }

    while ( !v.empty() )
    {
        int icurrent = v.back().first;
        int jcurrent = v.back().second;
        v.pop_back();

        MapSource &cell = m_SrcMap[ icurrent ][ jcurrent ];

        if( cell.m_maxvisited < kstart )
        {
            cell.m_maxvisited = kstart;

            double targetstr = strstart + ( cell.m_pt - pstart ).mag() * grm1;

            if( cell.m_str > targetstr )
            {
                // Mark dominated as progress is made
                cell.m_dominated = true;
                cell.m_str = targetstr;
                cell.m_reason = reason;

                for( int i = 0; i < 4; i++ )
                {
                    int inext = icurrent + iadd[i];
                    int jnext = jcurrent + jadd[i];

                    if( inext < nmapu && inext >= 0 && jnext < nmapw && jnext >= 0 )
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

    // A cell only ever needs to be looked at once per walk.  targetstr below is
    // measured from the seed cell, which cannot improve itself, so it is fixed
    // for the whole walk; m_str only ever decreases.  A cell that fails the
    // comparison can therefore never pass it later in the same walk, and one
    // that passes is written with its final value straight away.  Without this
    // the four neighbours of every improved cell were pushed unconditionally,
    // so cells were popped and re-measured many times over.  The overload that
    // LimitTargetMap uses has always had this guard by way of m_maxvisited.
    const int nmapu = ( int )m_SrcMap.size();
    const int nmapw = ( int )m_SrcMap[0].size();
    const size_t ncell = ( size_t )nmapu * ( size_t )nmapw;

    if( m_WalkVisited.size() != ncell )
    {
        m_WalkVisited.assign( ncell, 0 );
        m_WalkVisitID = 0;
    }

    m_WalkVisitID++;
    if( m_WalkVisitID == 0 )    // Wrapped.  Zero means never visited, so start over.
    {
        m_WalkVisited.assign( ncell, 0 );
        m_WalkVisitID = 1;
    }

    const unsigned int visitid = m_WalkVisitID;
    unsigned int *visited = m_WalkVisited.data();

    const vec3d pstart = m_SrcMap[istart][jstart].m_pt;
    const double strstart = m_SrcMap[istart][jstart].m_str;
    const double grm1 = m_GridDensityPtr->m_GrowRatio - 1.0;

    int reason = m_SrcMap[istart][jstart].m_reason;
    if ( reason < vsp::MIN_GROW_LIMIT )
    {
        reason += vsp::GROW_LIMIT_INCREMENT;
    }

    vector < pair < int, int > > &v = m_WalkStack;
    v.clear();

    for( int i = 0; i < 4; i++ )
    {
        int inext = istart + iadd[i];
        int jnext = jstart + jadd[i];

        if( inext < nmapu && inext >= 0 && jnext < nmapw && jnext >= 0 )
        {
            visited[ ( size_t )inext * nmapw + jnext ] = visitid;
            v.push_back( make_pair( inext, jnext ) );
        }
    }

    while ( !v.empty() )
    {
        int icurrent = v.back().first;
        int jcurrent = v.back().second;
        v.pop_back();

        MapSource &cell = m_SrcMap[ icurrent ][ jcurrent ];

        double targetstr = strstart + ( cell.m_pt - pstart ).mag() * grm1;

        if( cell.m_str > targetstr )
        {
            cell.m_str = targetstr;
            cell.m_reason = reason;

            for( int i = 0; i < 4; i++ )
            {
                int inext = icurrent + iadd[i];
                int jnext = jcurrent + jadd[i];

                if( inext < nmapu && inext >= 0 && jnext < nmapw && jnext >= 0 )
                {
                    if( visited[ ( size_t )inext * nmapw + jnext ] != visitid )
                    {
                        visited[ ( size_t )inext * nmapw + jnext ] = visitid;
                        v.push_back( make_pair( inext, jnext ) );
                    }
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

void Surf::LimitTargetMap( const MSCloud &es_cloud, const MSTree &es_tree, double minmap )
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

    if ( i < 0 )
        i = 0;
    if ( j < 0 )
        j = 0;
    if ( fraci < 0 )
        fraci = 0;
    if ( fracj < 0 )
        fracj = 0;
}

void Surf::UWtoTargetMapij( double u, double w, int &i, int &j )
{
    double fraci, fracj;
    UWtoTargetMapij( u, w, i, j, fraci, fracj );
}

void Surf::ApplyES( const vec3d &uw, double t, int reason )
{
    ApplyESAtPnt( uw, m_SurfCore.CompPnt( uw.x(), uw.y() ), t, reason );
}

void Surf::ApplyESAtPnt( const vec3d &uw, const vec3d &p, double t, int reason )
{
    double grm1 = m_GridDensityPtr->m_GrowRatio - 1.0;
    int nmapu = m_SrcMap.size();
    int nmapw = m_SrcMap[0].size();

    int ibase, jbase;
    double u = uw.x();
    double w = uw.y();
    UWtoTargetMapij( u, w, ibase, jbase );

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

// A pattern search for the least of the larger of the two distances, over a box of the
// surface's parameters.  Derivative free, because the objective creases along the set where
// the two distances are equal, which is where the answer lives.  Slower than solving for it,
// but it cannot converge to the wrong stationary point.
double Surf::SplitSearch( const vec3d & p0, const vec3d & p1,
                          double ulo, double uhi, double wlo, double whi,
                          double &u, double &w ) const
{
    vec3d p = m_SurfCore.CompPnt( u, w );
    double best = max( dist( p, p0 ), dist( p, p1 ) );

    double du = 0.25 * ( uhi - ulo );
    double dw = 0.25 * ( whi - wlo );

    for ( int iter = 0; iter < 40; iter++ )
    {
        bool moved = false;

        for ( int k = 0; k < 4; k++ )
        {
            double tu = u;
            double tw = w;

            if ( k == 0 ) { tu = u + du; }
            if ( k == 1 ) { tu = u - du; }
            if ( k == 2 ) { tw = w + dw; }
            if ( k == 3 ) { tw = w - dw; }

            tu = clamp( tu, ulo, uhi );
            tw = clamp( tw, wlo, whi );

            vec3d tp = m_SurfCore.CompPnt( tu, tw );
            double f = max( dist( tp, p0 ), dist( tp, p1 ) );

            if ( f < best )
            {
                best = f;
                u = tu;
                w = tw;
                moved = true;
            }
        }

        if ( !moved )
        {
            du *= 0.5;
            dw *= 0.5;

            if ( du < 1.0e-7 && dw < 1.0e-7 )
            {
                break;
            }
        }
    }

    return best;
}

// Where to put the new point when an edge is split.
//
// Taking the midpoint of the two ends and projecting it to the surface assumes the midpoint
// lies near the surface.  Across a wing tip, where an edge runs from the lower surface round
// to the upper, it does not: the midpoint is inside the wing, and the nearest surface point to
// it is back on the side the edge came from.  The split then produces a child edge nearly as
// long as its parent, which is split again, and the mesher fills the tip with a hairball until
// something gives way.
//
// What halves the edge is the point equidistant from its two ends, and of those the nearest.
// Solving for it directly is quick -- a two by two system in u and w, see
// Solving for it directly is quick -- a two by two system in u and w, see
// eli/geom/intersect/equidistant_surface.hpp -- but it finds a stationary point of the
// constrained problem, which is not always the one wanted.
//
// So solve first and check the answer.  A split that halves its edge is kept, and only one
// Both are measured against the projection, which is kept where it beats them.
vec2d Surf::SplitUW( const vec3d & p0, const vec3d & p1, const vec2d & uw0, const vec2d & uw1 ) const
{
    double u0 = 0.5 * ( uw0.x() + uw1.x() );
    double w0 = 0.5 * ( uw0.y() + uw1.y() );

    double ulo = min( uw0.x(), uw1.x() );
    double uhi = max( uw0.x(), uw1.x() );
    double wlo = min( uw0.y(), uw1.y() );
    double whi = max( uw0.y(), uw1.y() );

    double upad = 0.25 * ( uhi - ulo );
    double wpad = 0.25 * ( whi - wlo );

    ulo = max( ulo - upad, m_SurfCore.GetMinU() );
    uhi = min( uhi + upad, m_SurfCore.GetMaxU() );
    wlo = max( wlo - wpad, m_SurfCore.GetMinW() );
    whi = min( whi + wpad, m_SurfCore.GetMaxW() );

    // Seed on the straight line between the two ends in parameter space.  The equidistant
    // point along that line always exists and is always found, and unlike the parametric
    // midpoint it is already equidistant, which is most of what the solve is looking for.
    double su = u0;
    double sw = w0;
    m_SurfCore.FindEquidistantOnLine( su, sw, p0, p1, uw0.x(), uw0.y(), uw1.x(), uw1.y() );

    su = clamp( su, ulo, uhi );
    sw = clamp( sw, wlo, whi );

    double u = su;
    double w = sw;
    m_SurfCore.FindEquidistant( u, w, p0, p1, su, sw, ulo, uhi, wlo, whi );

    vec3d pe = m_SurfCore.CompPnt( u, w );
    double fe = max( dist( pe, p0 ), dist( pe, p1 ) );

    double half = 0.5 * dist( p0, p1 );

    // Did it halve the edge?  If it did, that is the answer.
    //
    // Both fallbacks below -- the search, and projecting the midpoint onto the surface -- are
    // for the case where the solve did not.  Projecting is a second Newton solve as costly as
    // the first, and the most it can win once the solve is already within five percent of a
    // perfect halving is those five percent, which is not worth a solve to chase.
    if ( fe > 1.05 * half )
    {
        double su = u0;
        double sw = w0;
        double fs = SplitSearch( p0, p1, ulo, uhi, wlo, whi, su, sw );

        if ( fs < fe )
        {
            u = su;
            w = sw;
            fe = fs;
        }

        // Fall back on the projection where it does better.
        vec3d pmid = ( p0 + p1 ) * 0.5;
        double pu, pw;
        m_SurfCore.FindNearest( pu, pw, pmid, u0, w0 );
        vec3d pp = m_SurfCore.CompPnt( pu, pw );

        if ( max( dist( pp, p0 ), dist( pp, p1 ) ) < fe )
        {
            return vec2d( pu, pw );
        }
    }

    return vec2d( u, w );
}

// One side of the patch, from one corner of its parameter domain to the next.
//
// Where a patch has been put back together out of two pieces, the side along the join can come
// back on itself: the surface reaches the same place from either half, so the side runs out and
// back over one curve in space.  Cut in two at the turn, the halves are that curve from either
// end and pair with each other, which is one edge of topology rather than the two an ordinary
// cut would make.
void Surf::AddBorderCurve( double ua, double wa, double ub, double wb )
{
    double degen_tol = 1.0e-6;

    bool folded = true;
    int nchk = 8;

    for ( int i = 1; i < nchk; i++ )
    {
        double f = ( double )i / ( double )( 2 * nchk );

        vec3d p0 = m_SurfCore.CompPnt( ua + f * ( ub - ua ), wa + f * ( wb - wa ) );
        vec3d p1 = m_SurfCore.CompPnt( ub - f * ( ub - ua ), wb - f * ( wb - wa ) );

        if ( dist( p0, p1 ) > degen_tol )
        {
            folded = false;
            break;
        }
    }

    int npiece = 1;
    if ( folded )
    {
        npiece = 2;
    }

    vector< vec3d > pnts( 2 );

    for ( int k = 0; k < npiece; k++ )
    {
        double f0 = ( double )k / ( double )npiece;
        double f1 = ( double )( k + 1 ) / ( double )npiece;

        pnts[0].set_xyz( ua + f0 * ( ub - ua ), wa + f0 * ( wb - wa ), 0 );
        pnts[1].set_xyz( ua + f1 * ( ub - ua ), wa + f1 * ( wb - wa ), 0 );

        SCurve* scrv = new SCurve( this );
        scrv->InterpolateLinear( pnts );
        scrv->SetParmLine( ParmLine::Between( ua, wa, ub, wb ) );
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
}

void Surf::FindBorderCurves()
{
    double min_u = m_SurfCore.GetMinU();
    double min_w = m_SurfCore.GetMinW();
    double max_u = m_SurfCore.GetMaxU();
    double max_w = m_SurfCore.GetMaxW();

    AddBorderCurve( min_u, min_w, max_u, min_w );       // Inc U
    AddBorderCurve( max_u, min_w, max_u, max_w );       // Inc W
    AddBorderCurve( max_u, max_w, min_u, max_w );       // Dec U
    AddBorderCurve( min_u, max_w, min_u, min_w );       // Dec W
}

string Surf::GetDisplayName()
{
    char buf[255];
    snprintf( buf, sizeof( buf ), "%s_%d",  m_Name.c_str(), m_SplitNum );

    return string( buf );
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
    m_Mesh.WriteSimpleSTL( filename );
}

// Everything that has to happen before two surfaces' patch trees are worth intersecting, and
// the answer to whether they are.  Kept apart from the patch work because it projects curves
// onto the surface -- an evaluation, which writes the surface's own scratch buffers, so two
// threads must never do it to the same surface at once.  It also adds curves of its own.
bool Surf::IntersectPrepare( Surf* surfPtr, SurfaceIntersectionSingleton *MeshMgr )
{
    if ( surfPtr->GetCompID() == m_CompID )
    {
        return false;
    }

    if ( m_FeaSymmIndex >= 0 && surfPtr->GetFeaSymmIndex() >= 0 &&
         surfPtr->GetFeaSymmIndex() != m_FeaSymmIndex )
    {
        return false;
    }

    if ( !Compare( m_BBox, surfPtr->GetBBox() ) )
    {
        return false;
    }
    if ( BorderCurveOnSurface( surfPtr, MeshMgr ) )
    {
        return false;
    }
    if ( surfPtr->BorderCurveOnSurface( this, MeshMgr ) )
    {
        return false;
    }

    return true;
}

void Surf::FindIntersectPatches( Surf* surfPtr, vector < int > &patch_vec )
{
    patch_vec.clear();

    for ( int i = 0 ; i < ( int )m_PatchVec.size() ; i++ )
    {
        if ( Compare( *m_PatchVec[i]->get_bbox(), surfPtr->GetBBox() ) )
        {
            patch_vec.push_back( i );
        }
    }
}

// Walk one patch's tree against the patches of surfPtr it meets.  This reaches the surfaces only
// through their control points, and every patch it makes along the way comes from a pool that
// belongs to the calling thread, so two threads may do this to the same surface at the same time.
void Surf::IntersectPatch( int ipatch, Surf* surfPtr, SurfaceIntersectionSingleton *MeshMgr )
{
    const vector< SurfPatch* > &otherPatchVec = surfPtr->GetPatchVec();

    for ( int j = 0 ; j < ( int )otherPatchVec.size() ; j++ )
    {
        if ( Compare( *m_PatchVec[ipatch]->get_bbox(), *otherPatchVec[j]->get_bbox() ) )
        {
            intersect( *m_PatchVec[ipatch], *otherPatchVec[j], MeshMgr );
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
    if ( brdPtr->GetICurve() != nullptr )
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
                if ( ICurves[i]->m_SCurve_A == brdPtr && ICurves[i]->m_SCurve_B == nullptr )
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


// Order two mesh input points by where they are, not by where they happen to live in memory.
// See the note in Surf::InitMesh.
static bool UWPntIndexCompare( const pair< vec2d, IPnt* > &a, const pair< vec2d, IPnt* > &b )
{
    if ( a.first.x() != b.first.x() )
    {
        return a.first.x() < b.first.x();
    }
    return a.first.y() < b.first.y();
}

void Surf::InitMesh( const vector< ISegChain* > &chains, const vector < vec2d > &adduw, SurfaceIntersectionSingleton *MeshMgr )
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

    // A set of pointers iterates in heap address order, and that moves from one run to the
    // next.  This loop hands each point its index, and the constraint segments below are
    // written in terms of those indices, so the entire input to the triangulator would change
    // shape between two runs of the same model.  That alone makes a run impossible to reproduce;
    // it also matters because both triangulators are sensitive to the order they are fed, to the
    // point of occasionally failing on one ordering and not another.
    //
    // Order the points by where they are instead.  The result is the same set of points,
    // always in the same sequence, for the same geometry.
    vector < pair < vec2d, IPnt* > > ipnts;
    ipnts.reserve( ipntSet.size() );

    for ( set< IPnt* >::iterator ip = ipntSet.begin() ; ip != ipntSet.end() ; ++ip )
    {
        ipnts.push_back( pair< vec2d, IPnt* >( ( *ip )->GetPuw( this )->m_UW, *ip ) );
    }

    sort( ipnts.begin(), ipnts.end(), UWPntIndexCompare );

    // Where each intersection point sits in uwPntVec.  A point belongs to a chain, and a chain
    // is handed to both the surfaces it lies between, so this numbering is the surface's own
    // and is kept here rather than on the point.
    unordered_map< IPnt*, int > pntindex;
    pntindex.reserve( ipnts.size() );

    // A point is taken to be one already numbered only where the two are close in 3D as well as
    // in u,w, to the tolerance MergeBorderEndPoints joins chain ends with.
    double tol3d = -1.0;
    SimpleGridDensity* gd = MeshMgr->GetGridDensityPtr();
    if ( gd )
    {
        tol3d = gd->m_MinLen / 100.0;
    }
    vector < vec3d > pntVec;

    for ( int k = 0 ; k < ( int )ipnts.size() ; k++ )
    {
        vec2d uw = ipnts[k].first;
        IPnt *ipt = ipnts[k].second;
        vec3d p = CompPnt( uw[0], uw[1] );

        int min_id = -1;
        double min_dist = 1.0;
        for ( int i = 0 ; i < ( int )uwPntVec.size() ; i++ )
        {
            double d = dist( uwPntVec[i], uw );
            if ( d < min_dist && ( tol3d < 0.0 || dist( pntVec[i], p ) < tol3d ) )
            {
                min_dist = d;
                min_id = i;
            }
        }

        if ( min_dist < 1.0e-4 )
        {
            pntindex[ ipt ] = min_id;
        }
        else
        {
            uwPntVec.push_back( uw );
            pntVec.push_back( p );
            pntindex[ ipt ] = ( int )uwPntVec.size() - 1;
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
            seg.m_Index[0] = pntindex[ chains[i]->m_TessVec[2 * j] ];
            seg.m_Index[1] = pntindex[ chains[i]->m_TessVec[2 * (j + 1)] ];
            seg.m_UWmid = chains[i]->m_TessVec[2 * j + 1]->GetPuw( this )->m_UW;

            seg.m_P[0] = chains[i]->m_TessVec[2 * j]->m_Pnt;
            seg.m_P[1] = chains[i]->m_TessVec[2 * (j + 1)]->m_Pnt;
            seg.m_Pmid = chains[i]->m_TessVec[2 * j + 1]->m_Pnt;

            isegVec.push_back( seg );
        }
    }

    // Remove duplicate constraint segments -- same pair of point indices in either order.
    // Duplicates arise when two chains share a boundary and both contribute the same edge.
    // Duplicates appear to be harmless, but removing them is cheap insurance.
    {
        set< pair< int, int > > seen;
        vector< MeshSeg > unique;
        unique.reserve( isegVec.size() );
        for ( int i = 0; i < (int)isegVec.size(); i++ )
        {
            int a = isegVec[i].m_Index[0];
            int b = isegVec[i].m_Index[1];
            pair< int, int > key( min( a, b ), max( a, b ) );
            if ( seen.insert( key ).second )
            {
                unique.push_back( isegVec[i] );
            }
        }
#ifdef DEBUG_CFD_MESH
        int ndelta = isegVec.size() - unique.size();
        if ( ndelta != 0 )
        {
            printf( "%d duplicate constraint segments removed.\n", ndelta );
        }
#endif
        isegVec.swap( unique );
    }

    BuildDistMap();

    m_Mesh.InitMesh( uwPntVec, isegVec, MeshMgr );

    CleanupDistMap();
}


// OpenABF builds a half edge mesh, and that needs every directed edge to belong to at most one
// face.  A grid whose rows collapse -- a patch that runs to a point, or one made by laying a
// cap against itself -- can offer the same directed edge twice.
//
// OpenABF answers some of those with an exception, which is caught below.  It does not answer
// this one: where it decides a face has to be wound the other way, it swaps the edges it has
// collected for their pairs but leaves the face's head pointing at the edge it abandoned.
// That edge is never given a next, so walking the face reads through a null pointer instead of
// throwing, and the program goes down.  The check has to happen here, before the face is
// offered.
//
// Returns false, and records nothing, for a face that would reuse a directed edge.
static bool ManifoldFace( std::set< std::pair< int, int > > &used, int a, int b, int c )
{
    std::pair< int, int > e0( a, b ), e1( b, c ), e2( c, a );

    if ( used.count( e0 ) > 0 || used.count( e1 ) > 0 || used.count( e2 ) > 0 )
    {
        return false;
    }

    used.insert( e0 );
    used.insert( e1 );
    used.insert( e2 );

    return true;
}

void Surf::BuildDistMap()
{
#ifdef DEBUG_CFD_MESH
    static int cnt = 0;
#endif

    if ( m_DistMapBuilt )
    {
        return;
    }
    m_DistMapBuilt = true;

    if ( m_PlanarUWAspect > 0 )
    {
#ifdef DEBUG_CFD_MESH
        cnt++;
#endif
        return;
    }

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
    IndexPntNodes( pnCloud, PT_MERGE_TOL );

    //==== Load Used Points ====//
    vector < int > kused;
    for ( i = 0; i < (int)pvec.size(); i++ )
    {
        if ( pnCloud.UsedNode( i ) )
        {
            mesh->insert_vertex( pvec[i].x(), pvec[i].y(), pvec[i].z() );
            kused.push_back( i );
        }
    }

    // A patch that runs to a point at one end -- a wing that closes on its spine -- samples
    // to a grid whose rows collapse, and the triangles built from it can ask for a third face
    // along an edge.  There is no flattening one of those, and asking for it walks off the
    // end of the mesh, so notice and give up on the map instead.  Without it the mesher
    // spaces points by the surface's own parameter, as it does for a planar patch.
    bool badface = false;
    int nrefuse = 0;
    std::set< std::pair< int, int > > usededge;

    for ( i = 0 ; i < nump - 1; i++ )
    {
        for ( j = 0; j < nump - 1; j++ )
        {
            int i0, i1, i2, i3;

            i0 = pnCloud.GetNodeUsedIndex( ptindx[ i ][ j ] );
            i1 = pnCloud.GetNodeUsedIndex( ptindx[ i + 1 ][ j ] );
            i2 = pnCloud.GetNodeUsedIndex( ptindx[ i ][ j + 1 ] );
            i3 = pnCloud.GetNodeUsedIndex( ptindx[ i + 1 ][ j + 1 ] );

            // A patch that runs to a point at one end -- a wing that closes on its spine --
            // samples to a grid whose rows collapse, and the triangles built from it can ask
            // for a third face along an edge.  Leave those out rather than let the throw take
            // the program down; what is left still stands in for the surface well enough to
            // space points on it.
            if ( (i0 != i1) && (i0 != i2) && (i1 != i2) )
            {
                if ( ManifoldFace( usededge, i0, i1, i2 ) )
                {
                    try { mesh->insert_face( i0, i1, i2 ); }
                    catch ( const std::exception & ) { badface = true; }
                }
                else
                {
                    nrefuse++;
                }
            }

            if ( (i1 != i3) && (i1 != i2) && (i3 != i2) )
            {
                if ( ManifoldFace( usededge, i1, i3, i2 ) )
                {
                    try { mesh->insert_face( i1, i3, i2 ); }
                    catch ( const std::exception & ) { badface = true; }
                }
                else
                {
                    nrefuse++;
                }
            }
        }
    }

    // Flattening can fail outright on a grid like that.  Without the map the mesher spaces
    // points by the surface's own parameter, which is what it does for a planar patch, so
    // give up on the map rather than on the surface.
    // Faces left out are not on their own a reason to give up.  A grid that repeats points
    // loses the faces built on them, and what remains often still flattens; only a flattener
    // that actually fails costs the map.
    if ( nrefuse > 0 )
    {
        printf( "Surface %s: %d of the distance map grid's faces are not 2-manifold and were "
                "left out.\n", GetDisplayName().c_str(), nrefuse );
    }

    if ( badface )
    {
        printf( "Surface %s: the flattener refused a face; spacing points by surface "
                "parameter instead.\n", GetDisplayName().c_str() );
        return;
    }

    try
    {
        ABF::Compute( mesh );
        LSCM::Compute( mesh );
    }
    catch ( const std::exception &e )
    {
        printf( "Surface %s: could not flatten the distance map grid (%s); spacing points by "
                "surface parameter instead.\n", GetDisplayName().c_str(), e.what() );
        return;
    }

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

    if ( true )
    {
        char str[256];
        snprintf( str, sizeof( str ), "stmat_%d.m", cnt );

        FILE *fp = fopen( str, "w" );

        if ( fp )
        {
            WriteMatDoubleM writeMatDouble;

            writeMatDouble.write( fp, smat, string( "smat" ), nump, nump );

            writeMatDouble.write( fp, tmat, string( "tmat" ), nump, nump );

            fprintf( fp, "figure(2)\n" );
            fprintf( fp, "plot( smat, tmat, smat', tmat' );\n" );

            fclose( fp );
        }
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

// The aspect to map U and W through when there is no ST map to use.
//
// A planar patch never builds one and says what its aspect is.  A patch whose map had to be
// abandoned -- a grid whose rows collapse, or one OpenABF could not flatten -- has no aspect
// of its own, so it falls back on its parameter extents.  Either way the mesher ends up
// spacing points by the surface's own parameter, which is what giving up on the map means.
//
// Returns a negative number when the map is there and should be used instead.
double Surf::LinearSTAspect() const
{
    if ( m_PlanarUWAspect > 0 )
    {
        return m_PlanarUWAspect;
    }

    if ( !m_STMap.empty() )
    {
        return -1.0;
    }

    double dw = m_SurfCore.GetMaxW() - m_SurfCore.GetMinW();

    if ( dw > 0.0 )
    {
        return ( m_SurfCore.GetMaxU() - m_SurfCore.GetMinU() ) / dw;
    }

    return 1.0;
}

vec2d Surf::GetST( const vec2d &uw )
{
    double asp = LinearSTAspect();

    if ( asp > 0 )
    {
        vec2d st;
        st.set_xy( asp * uw.x(), uw.y() );
        return st;
    }

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
    double asp = LinearSTAspect();

    if ( asp > 0 )
    {
        vec2d uw;
        uw.set_xy( st.x() / asp, st.y() );
        return uw;
    }

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
    m_DistMapBuilt = false;

    if ( m_PlanarUWAspect > 0 )
    {
        return;
    }

    m_UWMap.Cleanup();
    m_STMap.clear();
}

// Twice the signed area of a, b, c.
static double Orient2D( const vec2d &a, const vec2d &b, const vec2d &c )
{
    return ( b.x() - a.x() ) * ( c.y() - a.y() ) - ( b.y() - a.y() ) * ( c.x() - a.x() );
}

void Surf::FindCrossingTessSegs( const vector< ISegChain* > &chains, vector< pair< ISegChain*, int > > &segs )
{
    BuildDistMap();

    struct TSeg
    {
        vec2d m_A, m_B;          // In the triangulator's parameters
        vec2d m_UWA, m_UWB;      // In this surface's, where InitMesh merges points
        double m_XMin, m_XMax, m_YMin, m_YMax;
        ISegChain* m_Chain;
        int m_Index;
    };

    vector< TSeg > tsegs;
    for ( int i = 0 ; i < ( int )chains.size() ; i++ )
    {
        const deque< IPnt* > &tv = chains[i]->m_TessVec;
        int n = tv.size();
        int nhalf = 0.5 * ( n - 1 ) + 1;
        for ( int j = 0 ; j < nhalf - 1; j++ )
        {
            TSeg s;
            s.m_UWA = tv[ 2 * j ]->GetPuw( this )->m_UW;
            s.m_UWB = tv[ 2 * ( j + 1 ) ]->GetPuw( this )->m_UW;
            s.m_A = GetST( s.m_UWA );
            s.m_B = GetST( s.m_UWB );
            s.m_XMin = min( s.m_A.x(), s.m_B.x() );
            s.m_XMax = max( s.m_A.x(), s.m_B.x() );
            s.m_YMin = min( s.m_A.y(), s.m_B.y() );
            s.m_YMax = max( s.m_A.y(), s.m_B.y() );
            s.m_Chain = chains[i];
            s.m_Index = j;
            tsegs.push_back( s );
        }
    }

    sort( tsegs.begin(), tsegs.end(), []( const TSeg &a, const TSeg &b ) { return a.m_XMin < b.m_XMin; } );

    // InitMesh treats two points this close in u,w as one.
    const double mergetol = 1.0e-4;

    set< pair< ISegChain*, int > > found;

    for ( int i = 0 ; i < ( int )tsegs.size() ; i++ )
    {
        const TSeg &p = tsegs[i];
        for ( int k = i + 1 ; k < ( int )tsegs.size() && tsegs[k].m_XMin <= p.m_XMax ; k++ )
        {
            const TSeg &q = tsegs[k];
            if ( q.m_YMin > p.m_YMax || q.m_YMax < p.m_YMin )
            {
                continue;
            }

            // Which ends the two segments share, as InitMesh will number them.
            bool aa = dist( p.m_UWA, q.m_UWA ) < mergetol;
            bool ab = dist( p.m_UWA, q.m_UWB ) < mergetol;
            bool ba = dist( p.m_UWB, q.m_UWA ) < mergetol;
            bool bb = dist( p.m_UWB, q.m_UWB ) < mergetol;
            int nshared = aa + ab + ba + bb;

            // Only a proper crossing: the triangulator handles a segment that touches or runs
            // along another, by splitting it at the vertex it meets.
            bool bad = false;

            if ( nshared == 0 )
            {
                double d1 = Orient2D( p.m_A, p.m_B, q.m_A );
                double d2 = Orient2D( p.m_A, p.m_B, q.m_B );
                double d3 = Orient2D( q.m_A, q.m_B, p.m_A );
                double d4 = Orient2D( q.m_A, q.m_B, p.m_B );

                if ( d1 * d2 < 0.0 && d3 * d4 < 0.0 )
                {
                    bad = true;
                }
            }

            if ( bad )
            {
                found.insert( pair< ISegChain*, int >( p.m_Chain, p.m_Index ) );
                found.insert( pair< ISegChain*, int >( q.m_Chain, q.m_Index ) );
            }
        }
    }

    segs.assign( found.begin(), found.end() );
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

        // As in CfdMeshMgrSingleton::Subtag -- the centre is in this patch's parameters
        // and the subsurface was drawn in the Geom's.
        double uo, wo;
        bool onsurf = ToOriginalUW( center.x(), center.y(), uo, wo );

        for ( int s = 0 ; s < ( int ) s_surfs.size() ; s++ )
        {
            if ( onsurf && s_surfs[s]->Subtag( vec3d( uo, wo, 0 ) ) )
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
