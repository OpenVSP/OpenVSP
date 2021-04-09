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
#include <cfloat>  //For DBL_EPSILON

Surf::Surf()
{
    m_GridDensityPtr = 0;
    m_CompID = -1;
    m_UnmergedCompID = -1;
    m_SurfID = -1;
    m_FlipFlag = false;
    m_WakeFlag = false;
    m_SurfCfdType = vsp::CFD_NORMAL;
    m_SymPlaneFlag = false;
    m_FarFlag = false;
    m_WakeParentSurfID = -1;
    m_Mesh.SetSurfPtr( this );
    m_NumMap = 11;
    m_BaseTag = 1;
    m_MainSurfID = 0;
    m_FeaPartIndex = -1;
    m_ScaleUFlag = false;
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

double Surf::TargetLen( double u, double w, double gap, double radfrac )
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

            // apply curvature based limits
            double curv_len = TargetLen( u, w, m_GridDensityPtr->GetMaxGap( limitFlag ), m_GridDensityPtr->GetRadFrac( limitFlag ) );
            len = min( len, curv_len );

            // apply minimum edge length as safety on curvature
            len = max( len, m_GridDensityPtr->m_MinLen );

            // apply sources
            vec3d p = m_SurfCore.CompPnt( u, w );

            // The last four parameters passed here (m_GeomID, m_MainSurfID, u, w)
            // represent a significant layering violation.  This is needed to allow
            // constant U/W line sources to do some evaluation in u,w space instead
            // of just x,y,z space.
            double grid_len = m_GridDensityPtr->GetTargetLen( p, limitFlag, m_GeomID, m_MainSurfID, u, w );
            len = min( len, grid_len );

            // finally check max size
            len = min( len, m_GridDensityPtr->GetBaseLen( limitFlag ) );

            MapSource ms = MapSource( p, len, sid );
            m_SrcMap[i][j] = ms;
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

        MapSource src = m_SrcMap[i][j];

        // Recursively limit from small to large (skip if dominated)
        if( !src.m_dominated )
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

            double rmax = ( t - tmin ) / grm1;
            if( rmax > 0.0 )
            {
                double r2max = rmax * rmax;

                MSTreeResults es_matches;

                int nMatches = es_tree.radiusSearch( query_pt, r2max, es_matches, params );

                for ( int k = 0; k < nMatches; k++ )
                {
                    int imatch = es_matches[k].first;
                    double r = sqrt( es_matches[k].second );

                    double str = es_cloud.sources[imatch]->m_str;

                    double ts = str + grm1 * r;
                    t = min( t, ts );
                }
                if( t < torig )
                {
                    m_SrcMap[i][j].m_str = t;
                    WalkMap( i, j );
                }
            }
        }
    }
}

double Surf::InterpTargetMap( double u, double w )
{
    int i, j;
    double fraci, fracj;
    UWtoTargetMapij( u, w, i, j, fraci, fracj );

    double ti = m_SrcMap[i][j].m_str + fracj * ( m_SrcMap[i][j + 1].m_str - m_SrcMap[i][j].m_str );
    double tip1 = m_SrcMap[i + 1][j].m_str + fracj * ( m_SrcMap[i + 1][j + 1].m_str - m_SrcMap[i + 1][j].m_str );

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

void Surf::ApplyES( vec3d uw, double t )
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
    scrv->BuildBezierCurve( pnts, 0.25 );

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
    scrv->BuildBezierCurve( pnts, 0.25 );

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
    scrv->BuildBezierCurve( pnts, 0.25 );

    if ( scrv->Length( 10 ) > degen_tol )
    {
        m_SCurveVec.push_back( scrv );
    }
    else
    {
        delete scrv;
    }

    pnts[0].set_xyz( min_u, max_w,   0 );           // Dec W
    pnts[1].set_xyz( min_u, min_w,       0 );

    scrv = new SCurve( this );
    scrv->BuildBezierCurve( pnts, 0.25 );

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

    m_Mesh.TriangulateBorder( uw_border );

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

void Surf::IntersectLineSegMesh( vec3d & p0, vec3d & p1, vector< double > & t_vals )
{
    BndBox line_box;
    line_box.Update( p0 );
    line_box.Update( p1 );

    if ( !Compare( line_box, m_BBox ) )
    {
        return;
    }

    double tparm, uparm, vparm;
    list< Tri* >::iterator t;
    list <Tri*> triList = m_Mesh.GetTriList();

    vec3d dir = p1 - p0;

    for ( t = triList.begin() ; t != triList.end(); ++t )
    {
        int iFlag = intersect_triangle( p0.v, dir.v,
                                        ( *t )->n0->pnt.v, ( *t )->n1->pnt.v, ( *t )->n2->pnt.v, &tparm, &uparm, &vparm );

        if ( iFlag && tparm > 0.0 )
        {
            //==== Find If T is Already Included ====//
            int dupFlag = 0;
            for ( int j = 0 ; j < ( int )t_vals.size() ; j++ )
            {
                if ( std::abs( tparm - t_vals[j] ) < 1.0e-7 )
                {
                    dupFlag = 1;
                    break;
                }
            }

            if ( !dupFlag )
            {
                t_vals.push_back( tparm );
            }
        }
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


void Surf::InitMesh( vector< ISegChain* > chains )
{

//static int name_cnt = 0;
//char str[256];
//sprintf( str, "Surf_UW%d.dat", name_cnt );
//name_cnt++;
//FILE* fp = fopen( str, "w" );
//int ccnt = 0;
//for ( int i = 0 ; i < (int)chains.size() ; i++ )
//{
//  if ( ccnt%5 == 0 ) fprintf( fp, "COLOR RED\n" );
//  else if ( ccnt % 5 == 1 ) fprintf( fp, "COLOR BLUE\n" );
//  else if ( ccnt % 5 == 2 ) fprintf( fp, "COLOR GREEN\n" );
//  else if ( ccnt % 5 == 3 ) fprintf( fp, "COLOR PURPLE\n" );
//  else if ( ccnt % 5 == 4 ) fprintf( fp, "COLOR YELLOW\n" );
//  fprintf( fp, "MOVE \n" );
//  ccnt++;
//      for ( int j = 1 ; j < (int)chains[i]->m_TessVec.size() ; j++ )
//      {
//          vec2d uw0 = chains[i]->m_TessVec[j-1]->GetPuw( this )->m_UW;
//          vec2d uw1 = chains[i]->m_TessVec[j]->GetPuw( this )->m_UW;
//          fprintf( fp, "%f %f\n", uw0[0], uw0[1] );
//          fprintf( fp, "%f %f\n", uw1[0], uw1[1] );
//      }
//}
//fclose(fp);

    //==== Store Only One Instance of each IPnt ====//
    set< IPnt* > ipntSet;
    for ( int i = 0 ; i < ( int )chains.size() ; i++ )
        for ( int j = 0 ; j < ( int )chains[i]->m_TessVec.size() ; j++ )
        {
            ipntSet.insert( chains[i]->m_TessVec[j] );
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


    MeshSeg seg;
    vector< MeshSeg > isegVec;
    for ( int i = 0 ; i < ( int )chains.size() ; i++ )
        for ( int j = 1 ; j < ( int )chains[i]->m_TessVec.size() ; j++ )
        {
            seg.m_Index[0] = chains[i]->m_TessVec[j - 1]->m_Index;
            seg.m_Index[1] = chains[i]->m_TessVec[j]->m_Index;
            isegVec.push_back( seg );
        }

//  ////jrg Check For Duplicate Segs
//  vector< MeshSeg > dupMeshSegVec;
//  for ( int i = 0 ; i < (int)isegVec.size() ; i++ )
//  {
//      int iind0 = isegVec[i].m_Index[0];
//      int iind1 = isegVec[i].m_Index[1];
//      bool dup = false;
//      for ( int j = i+1 ; j < (int)isegVec.size() ; j++ )
//      {
//          int jind0 = isegVec[j].m_Index[0];
//          int jind1 = isegVec[j].m_Index[1];
//          if ( (iind0 == jind0 && iind1 == jind1) ||
//               (iind0 == jind1 && iind1 == jind0) )
//          {
//              dup = true;
//              //printf("Surf: Duplicate Seg \n" );
//          }
//      }
//      if ( !dup )
//          dupMeshSegVec.push_back( isegVec[i] );
//  }
//  isegVec = dupMeshSegVec;


////jrg - Check For Point Close to Other Segs
//vector< MeshSeg > newMeshSegVec;
//for ( int i = 0 ; i < (int)isegVec.size() ; i++ )
//{
//  int iind0 = isegVec[i].m_Index[0];
//  int iind1 = isegVec[i].m_Index[1];
//
//  for ( int j = 0 ; j < (int)uwPntVec.size() ; j++ )
//  {
//      if ( j != iind0 && j != iind1 )
//      {
//          vec2d proj = proj_pnt_on_line_seg(uwPntVec[iind0], uwPntVec[iind1], uwPntVec[j] );
//          double d = dist( proj, uwPntVec[j] );
//
//          if ( d < 0.000001 )
//          {
//              MeshSeg addseg1, addseg2;
//              addseg1.m_Index[0] = iind0;
//              addseg1.m_Index[1] = j;
//              addseg2.m_Index[0] = j;
//              addseg2.m_Index[1] = iind1;
//              newMeshSegVec.push_back( addseg1 );
//              newMeshSegVec.push_back( addseg2 );
//
//              //printf("Surf: %d Proj Pnt Dist = %f\n", (int)this, d );
//              //printf("  Seg = %f %f   %f %f\n",
//              //  uwPntVec[iind0].x(),  uwPntVec[iind0].y(),
//              //  uwPntVec[iind1].x(),  uwPntVec[iind1].y());
//              //vec3d p = CompPnt( uwPntVec[j].x(), uwPntVec[j].y() );
//              //printf("  Pnt uv = %f %f    %f %f %f\n", uwPntVec[j].x(), uwPntVec[j].y(), p.x(), p.y(), p.z() );
//          }
//          else
//          {
//              newMeshSegVec.push_back( isegVec[i] );
//          }
//      }
//  }
//}

//vector< MeshSeg > smallMeshSegVec;
//for ( int i = 0 ; i < (int)isegVec.size() ; i++ )
//{
//  int ind0 = isegVec[i].m_Index[0];
//  int ind1 = isegVec[i].m_Index[1];
//  double d = dist( uwPntVec[ind0], uwPntVec[ind1] );
//  if ( d > 0.00000001 )
//  {
//      smallMeshSegVec.push_back( isegVec[i] );
//  }
//  else
//      printf("Surf: %d Small Seg Dist = %12.12f\n", (int)this, d );
//}
//isegVec = smallMeshSegVec;


    m_Mesh.InitMesh( uwPntVec, isegVec );
}


void Surf::BuildDistMap()
{
    int i, j;
    const unsigned int nump = 101;

    double VspMinU = m_SurfCore.GetMinU();
    double VspMinW = m_SurfCore.GetMinW();

    double VspMaxU = m_SurfCore.GetMaxU();
    double VspMaxW = m_SurfCore.GetMaxW();

    double VspdU = VspMaxU - VspMinU;
    double VspdW = VspMaxW - VspMinW;

    //==== Load Point Vec ====//
    vector< vector< vec3d > > pvec;
    pvec.resize( nump );
    for ( i = 0 ; i < nump ; i++ )
    {
        pvec[i].resize( nump );
        double u = VspMinU + VspdU * ( double )i / ( double )( nump - 1 );
        for ( j = 0 ; j < nump ; j++ )
        {
            double w = VspMinW + VspdW * ( double )j / ( double )( nump - 1 );
            pvec[i][j] = m_SurfCore.CompPnt( u, w );
        }
    }

    //==== Find U Dists ====//
    double maxUDist = 0.0;
    vector< double > uDistVec;
    for ( i = 0 ; i < nump ; i++ )
    {
        double sum_d = 0.0;
        for ( j = 1 ; j < nump ; j++  )
        {
            sum_d += dist( pvec[j - 1][i], pvec[j][i] );
        }
        uDistVec.push_back( sum_d );

        if ( sum_d > maxUDist )
        {
            maxUDist = sum_d;
        }
    }
    if ( maxUDist < DBL_EPSILON )
    {
        maxUDist = 1.0e-9;
    }

    //==== Find W Dists ====//
    double maxWDist = 0.0;
    vector< double > wDistVec;
    for ( i = 0 ; i < nump ; i++ )
    {
        double sum_d = 0.0;
        for ( j = 1 ; j < nump ; j++  )
        {
            sum_d += dist( pvec[i][j - 1], pvec[i][j] );
        }
        wDistVec.push_back( sum_d );

        if ( sum_d > maxWDist )
        {
            maxWDist = sum_d;
        }
    }

    if ( maxWDist < DBL_EPSILON )
    {
        maxWDist = 1.0e-9;
    }


    //==== Scale U Dists ====//
    double wu_ratio = VspdW / VspdU;
    m_UScaleMap.resize( uDistVec.size() );
    for ( i = 0 ; i < ( int )uDistVec.size() ; i++ )
    {
        m_UScaleMap[i] = wu_ratio * ( uDistVec[i] / maxWDist );

        if ( m_UScaleMap[i] < 1.0e-5 )
        {
            m_UScaleMap[i] = 1.0e-5;
        }
    }

    //==== Scale W Dists ====//
    double uw_ratio = VspdU / VspdW;
    m_WScaleMap.resize( wDistVec.size() );
    for ( i = 0 ; i < ( int )wDistVec.size() ; i++ )
    {
        m_WScaleMap[i] = uw_ratio * ( wDistVec[i] / maxUDist );

        if ( m_WScaleMap[i] < 1.0e-5 )
        {
            m_WScaleMap[i] = 1.0e-5;
        }
    }

    //==== Figure Out Which to Scale ====//
    double min_u_scale = 1.0e12;
    double max_u_scale = 0.0;
    for ( i = 0 ; i < ( int )m_UScaleMap.size() ; i++ )
    {
        if ( m_UScaleMap[i] < min_u_scale )
        {
            min_u_scale = m_UScaleMap[i];
        }
        if ( m_UScaleMap[i] > max_u_scale )
        {
            max_u_scale = m_UScaleMap[i];
        }
    }
    double u_ratio = max_u_scale / min_u_scale;

    double min_w_scale = 1.0e12;
    double max_w_scale = 0.0;
    for ( i = 0 ; i < ( int )m_WScaleMap.size() ; i++ )
    {
        if ( m_WScaleMap[i] < min_w_scale )
        {
            min_w_scale = m_WScaleMap[i];
        }
        if ( m_WScaleMap[i] > max_w_scale )
        {
            max_w_scale = m_WScaleMap[i];
        }
    }
    double w_ratio = max_w_scale / min_w_scale;

    m_ScaleUFlag = u_ratio > w_ratio;


//char str[256];
//static int cnt = 0;
//sprintf( str, "uwscale_%d.dat", cnt );
//cnt++;
//  FILE* fp = fopen(str, "w");
//  fprintf( fp, "ws 1 2\n" );
//  fprintf( fp, "color green\n" );
//  for ( i = 0 ; i < (int)m_WScaleMap.size() ; i++ )
//  {
//      double u = (double)i/(double)(m_WScaleMap.size()-1);
//      fprintf( fp, "%f %f \n", u, m_WScaleMap[i]  );
//  }
//  fprintf( fp, "color blue\n" );
//  for ( i = 0 ; i < 1001 ; i++ )
//  {
//      double u = (double)i/(double)(1000);
//      fprintf( fp, "%f %f \n", u, GetWScale(u)  );
//  }
//  fclose( fp );

}

double Surf::GetUScale( double w01 )      // w 0->1
{
    if ( !m_ScaleUFlag )
    {
        return 1.0;
    }

    int num = m_UScaleMap.size();
    double indd = w01 * (double) ( num - 1 );
    int ind = ( int ) indd;
    if ( ind < 0 )
    {
        ind = 0;
    }
    if ( ind > num - 2 )
    {
        ind = num - 2;
    }

    double fract = indd - ( double )ind;
    if ( fract < 0.0 )
    {
        fract = 0.0;
    }
    if ( fract > 1.0 )
    {
        fract = 1.0;
    }

    double uscale = m_UScaleMap[ind] + fract * ( m_UScaleMap[ind + 1] - m_UScaleMap[ind] );
    return uscale;
}

double Surf::GetWScale( double u01 )      // u 0->1
{
    if ( m_ScaleUFlag )
    {
        return 1.0;
    }

    int num = m_WScaleMap.size();
    double indd = u01 * (double) ( num - 1 );
    int ind = ( int ) indd;
    if ( ind < 0 )
    {
        ind = 0;
    }
    if ( ind > num - 2 )
    {
        ind = num - 2;
    }

    double fract = indd - ( double )ind;
    if ( fract < 0.0 )
    {
        fract = 0.0;
    }
    if ( fract > 1.0 )
    {
        fract = 1.0;
    }

    double wscale = m_WScaleMap[ind] + fract * ( m_WScaleMap[ind + 1] - m_WScaleMap[ind] );
    return wscale;
}

bool Surf::ValidUW( vec2d & uw )
{
    //return true;
    double slop = 1.0e-4;
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

void Surf::Subtag( bool tag_subs )
{
    vector< SimpTri >& tri_vec = m_Mesh.GetSimpTriVec();
    const vector< vec2d >& pnts = m_Mesh.GetSimpUWPntVec();
    vector< SubSurface* > s_surfs;

    if ( tag_subs ) s_surfs = SubSurfaceMgr.GetSubSurfs( m_GeomID, m_MainSurfID );

    for ( int t = 0 ; t < ( int ) tri_vec.size() ; t++ )
    {
        SimpTri& tri = tri_vec[t];
        tri.m_Tags.push_back( m_BaseTag );
        vec2d center = ( pnts[tri.ind0] + pnts[tri.ind1] + pnts[tri.ind2] ) * 1 / 3.0;
        vec2d cent2d = center;

        for ( int s = 0 ; s < ( int ) s_surfs.size() ; s++ )
        {
            if ( s_surfs[s]->Subtag( vec3d( cent2d.x(), cent2d.y(), 0 ) ) )
            {
                tri.m_Tags.push_back( s_surfs[s]->m_Tag );
            }
        }
        SubSurfaceMgr.m_TagCombos.insert( tri.m_Tags );
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
