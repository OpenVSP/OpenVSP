//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// SCurve
//
//////////////////////////////////////////////////////////////////////

#include "SCurve.h"

#include <algorithm>

ParmLine ParmLine::Between( double ua, double wa, double ub, double wb )
{
    if ( ua == ub && wa != wb )
    {
        return ParmLine( U_CONST, ua );
    }
    if ( wa == wb && ua != ub )
    {
        return ParmLine( W_CONST, wa );
    }
    return ParmLine();
}

vec3d ParmLine::OnLine( const vec3d &uw ) const
{
    if ( m_Kind == U_CONST )
    {
        return vec3d( m_Val, uw.y(), uw.z() );
    }
    if ( m_Kind == W_CONST )
    {
        return vec3d( uw.x(), m_Val, uw.z() );
    }
    return uw;
}

double ParmLine::Along( const vec3d &uw ) const
{
    if ( m_Kind == U_CONST )
    {
        return uw.y();
    }
    return uw.x();
}

SCurve::SCurve()
{
    m_Surf = nullptr;
    m_ICurve = nullptr;
    num_segs = 0;
}

SCurve::SCurve( Surf* s )
{
    m_Surf = s;
    m_ICurve = nullptr;
    num_segs = 0;
}

SCurve::~SCurve()
{
}

void SCurve::InterpolateLinear( const vector<vec3d> &pnts_to_interpolate )
{
    if ( pnts_to_interpolate.size() >= 2 )
    {
        m_UWPolyline = pnts_to_interpolate;
        m_UWCrv = Bezier_curve();
    }
    else
    {
        vector< vec3d >().swap( m_UWPolyline );
        m_UWCrv.InterpolateLinear( pnts_to_interpolate );
    }
    CleanupDistTable();
}

void SCurve::PromoteTo( int deg )
{
    if ( !m_UWPolyline.empty() )
    {
        m_UWCrv.InterpolateLinear( m_UWPolyline );
        vector< vec3d >().swap( m_UWPolyline );
    }
    m_UWCrv.PromoteTo( deg );
    CleanupDistTable();
}

Bezier_curve SCurve::GetUWCrv() const
{
    if ( !m_UWPolyline.empty() )
    {
        Bezier_curve crv;
        crv.InterpolateLinear( m_UWPolyline );
        return crv;
    }
    return m_UWCrv;
}

// Evaluate the polyline as Code-Eli evaluates the curve InterpolateLinear builds: segment i
// spans parameter [i, i+1], and a parameter out of range lands on the last segment.
vec3d SCurve::UWPolylinePnt( double t ) const
{
    int nseg = ( int )m_UWPolyline.size() - 1;

    int i = nseg - 1;
    double tt;
    if ( t == nseg )
    {
        tt = 1.0;
    }
    else if ( t < 0.0 )
    {
        tt = -1.0;
    }
    else
    {
        if ( t < nseg )
        {
            i = ( int )t;
        }
        tt = t - i;
    }

    const vec3d &p0 = m_UWPolyline[i];
    const vec3d &p1 = m_UWPolyline[i + 1];
    double omt = 1.0 - tt;
    return vec3d( omt * p0.x() + tt * p1.x(), omt * p0.y() + tt * p1.y(), omt * p0.z() + tt * p1.z() );
}

vec3d SCurve::UWCompPnt01( double u ) const
{
    if ( m_UWPolyline.empty() )
    {
        return m_UWCrv.CompPnt01( u );
    }

    if ( u > 1.0 )
    {
        u = 1.0;
    }

    int nseg = ( int )m_UWPolyline.size() - 1;
    return UWPolylinePnt( u * nseg );
}

vec3d SCurve::UWCompPnt( double t ) const
{
    if ( m_UWPolyline.empty() )
    {
        return m_UWCrv.CompPnt( t );
    }

    int nseg = ( int )m_UWPolyline.size() - 1;
    if ( t > nseg )
    {
        t = nseg;
    }
    return UWPolylinePnt( t );
}

void SCurve::GetUWParms( vector< double > &parms ) const
{
    if ( m_UWPolyline.empty() )
    {
        m_UWCrv.GetCurve().get_pmap( parms );
        return;
    }

    parms.resize( m_UWPolyline.size() );
    for ( size_t i = 0; i < parms.size(); i++ )
    {
        parms[i] = ( double )i;
    }
}

vec3d SCurve::UWFirstPnt() const
{
    if ( m_UWPolyline.empty() )
    {
        return m_UWCrv.FirstPnt();
    }
    return m_UWPolyline.front();
}

vec3d SCurve::UWLastPnt() const
{
    if ( m_UWPolyline.empty() )
    {
        return m_UWCrv.LastPnt();
    }
    return m_UWPolyline.back();
}

int SCurve::UWNumSections() const
{
    if ( m_UWPolyline.empty() )
    {
        return m_UWCrv.GetNumSections();
    }
    return ( int )m_UWPolyline.size() - 1;
}

double SCurve::Length( int num_segs )
{
    assert ( m_Surf );

    double total_dist = 0;
    vec3d uw = UWCompPnt01( 0 );
    vec3d last_p = m_Surf->CompPnt( uw.x(), uw.y() );
    for ( int i = 1 ; i < num_segs ; i++ )
    {
        double u = ( double )i / ( double )( num_segs - 1 );
        uw = UWCompPnt01( u );
        vec3d p = m_Surf->CompPnt( uw.x(), uw.y() );
        total_dist += dist( p, last_p );
    }

    return total_dist;
}


void SCurve::GetBorderCurve( Bezier_curve & crv ) const
{
    vec3d uw0 = UWFirstPnt();
    vec3d uw1 = UWLastPnt();
    m_Surf->GetBorderCurve( uw0, uw1, crv );
}

double SCurve::GetTargetLen( SimpleGridDensity *grid_den, SCurve *BCurve, const vec3d &p, const vec3d &uw, const vec3d &uwB, int &reason )
{
    bool limitFlag = false;
    if ( m_Surf->GetFarFlag() )
    {
        limitFlag = true;
    }
    if ( m_Surf->GetSymPlaneFlag() )
    {
        limitFlag = true;
    }

    double len = grid_den->GetBaseLen( limitFlag );

    double maplen = m_Surf->InterpTargetMap( uw.x(), uw.y(), reason );
    if ( len < maplen )
    {
        reason = vsp::MAX_LEN_CONSTRAINT;
    }
    len = min( len, maplen );

    if( BCurve )
    {
        limitFlag = false;
        if ( BCurve->m_Surf->GetFarFlag() )
        {
            limitFlag = true;
        }
        if ( BCurve->m_Surf->GetSymPlaneFlag() )
        {
            limitFlag = true;
        }

        double lenB = grid_den->GetBaseLen( limitFlag );

        int reasonB = -1;
        maplen = BCurve->m_Surf->InterpTargetMap( uwB.x(), uwB.y(), reasonB );
        if ( lenB < maplen )
        {
            reasonB = vsp::MAX_LEN_CONSTRAINT;
        }
        lenB = min( lenB, maplen );

        if ( lenB < len)
        {
            reason = reasonB;
        }
        len = min( len, lenB );
    }

    return len;
}

void SCurve::BorderTesselate( )
{
    // Cheap curve Tesselate intended for boundary curves.  These curves
    // run from 0 to max of one surface parameter, and are constant in the
    // other parameter (either 0 or max).
    //
    // First, figure out the span of variation in the running parameter.
    // That span is the number of segments in the surface along that parameter.
    //
    // Then, generate uniformly separated points (in the surface parameters)
    // from the start to finish.
    //
    // This routine only works for border curves.
    //
    // This routine does not populate the m_UTess curve.  It skips straight to
    // the m_UWTess. curve.

    m_UTess.clear();
    m_UWTess.clear();

    vec3d uwstart = UWFirstPnt();
    vec3d uwend = UWLastPnt();

    double ust = uwstart[0];
    double wst = uwstart[1];

    double du = uwend[0] - ust;
    double dw = uwend[1] - wst;

    double uspan = std::abs( du );
    double wspan = std::abs( dw );

    double span = max( uspan, wspan );

    int ptsperseg = 5;

    int npt = ( ceil( span ) ) * ( ptsperseg - 1 ) + 1;

    BorderTesselate( npt );
}

// Same, with the number of points named rather than worked out from this curve's own span.
// Two curves that are the same edge of two patches have to be cut the same number of times,
// and the spans they are cut from need not agree -- a patch made by joining two others
// covers twice the parameter its neighbour does along the same edge.
void SCurve::BorderTesselate( int npt )
{
    m_UTess.clear();
    m_UWTess.clear();

    vec3d uwstart = UWFirstPnt();
    vec3d uwend = UWLastPnt();

    double ust = uwstart[0];
    double wst = uwstart[1];

    double du = uwend[0] - ust;
    double dw = uwend[1] - wst;

    if ( npt < 2 )
    {
        npt = 2;
    }

    m_UWTess.resize( npt );

    for( int i = 0; i < npt ; i++ )
    {
        double frac = ( double )i / ( double )( npt - 1 );

        m_UWTess[i] = vec3d( ust + frac * du, wst + frac * dw, 0.0 );
    }
}

void SCurve::CheapTesselate( )
{
    const unsigned int npts = 10000;

    m_UTess.clear();
    m_UTess.resize( npts );
    for ( int i = 0; i < npts; i++ )
    {
        m_UTess[i] = ( double )i / ( double )( npts - 1 );
    }

    UWTess();
}

void SCurve::TessAdapt( const Surf& srf, double tol, int Nlimit )
{
    m_UTess.clear();
    vector < vec3d > pnt_vec;

    // Binary Adaptive refinement - ignore 3D points
    GetUWCrv().TessAdaptXYZ( srf, pnt_vec, tol, Nlimit, m_UTess );

    // Where the surface collapses, a run of parameter lands on one place.  The 3D points are
    // dropped here, but the parameters that named them are what the mesh is built from, and
    // several of them naming the same place is several mesh nodes there.  The points decide
    // which parameters are worth keeping, so they earn their keep before being thrown away.
    RemoveRepeatedPnts( pnt_vec, m_UTess );

    UWTess();
}

void SCurve::ProjectTessToSurf( SCurve* othercurve )
{
    vector< vec3d > UWTessB = othercurve->GetUWTessPnts();

    Surf* SurfA = GetSurf();
    Surf* SurfB = othercurve->GetSurf();

    double uguess = SurfA->GetSurfCore()->GetMidU();
    double wguess = SurfA->GetSurfCore()->GetMidW();

    unsigned int npts = UWTessB.size();
    m_UWTess.clear();
    m_UWTess.resize( npts );
    for ( int i = 0 ; i < npts ; i++ )
    {
        vec3d ptOther = SurfB->CompPnt( UWTessB[i].x(), UWTessB[i].y() );

        vec2d uw = SurfA->ClosestUW( ptOther, uguess, wguess );

        m_UWTess[i] = vec3d( uw.x(), uw.y(), 0 );

        uguess = uw.x();
        wguess = uw.y();
    }
}

void SCurve::ProjectBorderTess( SCurve* othercurve )
{
    const vector< vec3d > &uwother = othercurve->GetUWTessPnts();
    Surf* other = othercurve->GetSurf();

    int npts = ( int )m_UWTess.size();
    if ( npts < 2 || npts != ( int )uwother.size() )
    {
        return;
    }

    // The border runs in one parameter and holds the other fixed.
    bool alongu = std::abs( m_UWTess.back().x() - m_UWTess.front().x() ) >= std::abs( m_UWTess.back().y() - m_UWTess.front().y() );

    for ( int i = 1 ; i < npts - 1 ; i++ )
    {
        vec3d p = other->CompPnt( uwother[i].x(), uwother[i].y() );
        vec2d uw = m_Surf->ClosestUW( p, m_UWTess[i].x(), m_UWTess[i].y() );

        if ( alongu )
        {
            m_UWTess[i] = vec3d( uw.x(), m_UWTess[i].y(), 0.0 );
        }
        else
        {
            m_UWTess[i] = vec3d( m_UWTess[i].x(), uw.y(), 0.0 );
        }
    }
}

void SCurve::InterpDistTable( double idouble, double &t, double &u, double &s, double &dsdi, int &reason )
{
    int imax = target_vec.size() - 1;

    if ( idouble > imax )
    {
        idouble = 0.0;
    }

    // A station before the table, infinite or not a number, is the first one.
    if ( !( idouble >= 0.0 ) )
    {
        idouble = 0.0;
    }

    int ifloor = (int)floor( idouble );
    int iceil = (int)ceil( idouble );

    if( iceil == ifloor )
    {
        iceil++;
    }

    double ifrac = idouble - ifloor;

    if( iceil > imax )  // Should be equivalent to idouble > imax
    {
        iceil = imax;
        ifloor = imax - 1;
        ifrac = 1.0;
    }

    if( ifloor < 0 )
    {
        ifloor = 0;
        iceil = 1;
        ifrac = 0.0;
    }


    double tf = target_vec[ifloor];
    double tc = target_vec[iceil];
    t = tf + ifrac * ( tc - tf );

    double uf = u_vec[ifloor];
    double uc = u_vec[iceil];
    u = uf + ifrac * ( uc - uf );

    double sf = dist_vec[ifloor];
    double sc = dist_vec[iceil];
    dsdi = sc - sf;
    s = sf + ifrac * dsdi;

    reason = reason_vec[ round( ifloor + ifrac ) ];
}

// Where the distance table's stations are and the points they land on.  The curve does not move
// while density is spread along it, so this is worked out once.  Each entry costs a curve
// evaluation and a surface evaluation, which is most of what BuildDistTable does.
void SCurve::BuildDistTableGeom()
{
    assert( m_Surf );

    //==== Build U to Dist Table ====//
    int nref = 10;
    int nseglim = 10000;
    num_segs = nref * UWNumSections() + 1;
    if( num_segs > nseglim )
    {
        nref = nseglim / UWNumSections();
        if( nref < 1 )
        {
            nref = 1;
        }
        num_segs = nref * UWNumSections() + 1;
    }

    if ( ( int )m_TablePnt.size() != num_segs )
    {
        CleanupDistTable();

        u_vec.reserve( num_segs );
        dist_vec.reserve( num_segs );
        m_TableUW.reserve( num_segs );
        m_TablePnt.reserve( num_segs );

        double total_dist = 0.0;
        vec3d uw0 = UWCompPnt01( 0 );
        vec3d last_p = m_Surf->CompPnt( uw0.x(), uw0.y() );

        for ( int i = 0 ; i < num_segs ; i++ )
        {
            double u = ( double )i / ( double )( num_segs - 1 );

            vec3d uw = UWCompPnt01( u );
            vec3d p = m_Surf->CompPnt( uw.x(), uw.y() );

            u_vec.push_back( u );
            m_TableUW.push_back( uw );
            m_TablePnt.push_back( p );

            total_dist += dist( p, last_p );
            dist_vec.push_back( total_dist );

            last_p = p;
        }
    }
}

// Where the partner curve sits at each of this table's stations.  Spreading walks the two
// curves in step, so every pass asked the partner for the same points and asked its
// surface where they land; neither moves between passes.  Needs BuildDistTableGeom first.
void SCurve::BuildDistTableGeomB( SCurve* BCurve )
{
    if ( BCurve && ( m_TableBCurve != BCurve || ( int )m_TableUWB.size() != num_segs ) )
    {
        m_TableBCurve = BCurve;
        m_TableUWB.clear();
        m_TablePntB.clear();
        m_TableUWB.reserve( num_segs );
        m_TablePntB.reserve( num_segs );

        for ( int i = 0 ; i < num_segs ; i++ )
        {
            vec3d uwB = BCurve->UWCompPnt01( u_vec[i] );
            m_TableUWB.push_back( uwB );
            m_TablePntB.push_back( BCurve->m_Surf->CompPnt( uwB.x(), uwB.y() ) );
        }
    }
}

SplitSourceSet::SplitSourceSet( const list< MapSource* > &src ) : m_Src( src.begin(), src.end() ), m_Tree( nullptr )
{
    m_Searches = 0;
    m_Overs = 0;

    m_StrMax = 0.0;
    m_NonNegative = true;
    for ( size_t i = 0 ; i < m_Src.size() ; i++ )
    {
        double str = m_Src[i]->m_str;
        if ( !( str >= 0.0 ) )
        {
            m_NonNegative = false;
        }
        else if ( str > m_StrMax )
        {
            m_StrMax = str;
        }
    }

    for ( size_t i = 0 ; i < m_Src.size() ; i++ )
    {
        m_Box.Update( m_Src[i]->m_pt );
    }

    if ( !m_Src.empty() )
    {
        m_Cloud.sources = m_Src;
        m_Tree = new MSTree( 3, m_Cloud, KDTreeSingleIndexAdaptorParams( 10 ) );
        m_Tree->buildIndex();
    }
}

SplitSourceSet::~SplitSourceSet()
{
    delete m_Tree;
}

// Split sources against the two ends of a curve's table: those listed in near in that order, or
// every one where near is null.  Each end's target falls to what a source allows there, or the
// source's strength to what the end allows at the source.
static void ApplySplitSources( const vector< MapSource* > &src, const vector< size_t > *near, const vector< vec3d > &tablepnt,
                               const int indx[2], double grm1, vector< double > &target_vec, vector< double > &reason_vec )
{
    size_t n = src.size();
    if ( near )
    {
        n = near->size();
    }

    for ( size_t k = 0 ; k < n ; k++ )
    {
        MapSource* ss = src[k];
        if ( near )
        {
            ss = src[ ( *near )[k] ];
        }

        vec3d pt = ss->m_pt;
        double str = ss->m_str;

        for ( int i = 0; i < 2; i++ ) // Loop over first and last points.
        {
            double r = dist( pt, tablepnt[indx[i]] );
            double targetstr = str + r * grm1;

            if ( targetstr < target_vec[indx[i]] )
            {
                target_vec[indx[i]] = targetstr;

                if ( ss->m_reason < vsp::MIN_GROW_LIMIT )
                {
                    reason_vec[indx[i]] = ss->m_reason + vsp::GROW_LIMIT_INCREMENT;
                }
                else
                {
                    reason_vec[indx[i]] = ss->m_reason;
                }
            }
            else
            {
                double targetrev = target_vec[indx[i]] + r * grm1;
                if ( targetrev < str )
                {
                    ss->m_str = targetrev;

                    if ( reason_vec[indx[i]] < vsp::MIN_GROW_LIMIT )
                    {
                        ss->m_reason = reason_vec[indx[i]] + vsp::GROW_LIMIT_INCREMENT;
                    }
                    else
                    {
                        ss->m_reason = reason_vec[indx[i]];
                    }
                }
            }
        }
    }
}

// The sources a tree search finds within a radius, until there are more than a cap; then the
// search stops and m_Over is set.
class NearSources
{
public:
    typedef double DistanceType;
    typedef size_t IndexType;

    NearSources( size_t cap, vector< size_t > &found ) : m_Found( found )
    {
        m_Cap = cap;
        m_RadiusSq = 0.0;
        m_Over = false;
    }

    size_t size() const
    {
        return m_Found.size();
    }

    bool full() const
    {
        return true;
    }

    double worstDist() const
    {
        return m_RadiusSq;
    }

    bool addPoint( double dsq, size_t index )
    {
        if ( dsq < m_RadiusSq )
        {
            if ( m_Found.size() >= m_Cap )
            {
                m_Over = true;
                return false;
            }
            m_Found.push_back( index );
        }
        return true;
    }

    void sort()
    {
    }

    vector< size_t > &m_Found;
    size_t m_Cap;
    double m_RadiusSq;
    bool m_Over;
};

void SCurve::BuildDistTable( SimpleGridDensity* grid_den, SCurve* BCurve, SplitSourceSet & splitSources )
{
    BuildDistTableGeom();
    BuildDistTableGeomB( BCurve );

    target_vec.clear();
    reason_vec.clear();
    target_vec.reserve( num_segs );
    reason_vec.reserve( num_segs );

    for ( int i = 0 ; i < num_segs ; i++ )
    {
        int reason = -1;
        vec3d uwB;

        if ( BCurve )
        {
            uwB = m_TableUWB[i];
        }

        double t = GetTargetLen( grid_den, BCurve, m_TablePnt[i], m_TableUW[i], uwB, reason );

        target_vec.push_back( t );
        reason_vec.push_back( reason );
    }

    double grm1 = grid_den->m_GrowRatio - 1.0;

    // Indices of first and last points in table.
    int indx[2] = { 0, num_segs - 1 };

    const vector< MapSource* > &src = splitSources.m_Src;

    // A source changes an end, or the end changes it, only where it lies nearer than
    // | target - strength | / grm1.  Targets and strengths here only fall and never below zero,
    // so the larger of the end's target and the strongest source bounds that, and the sources
    // beyond it are passed over.  Those taken are taken in their order, as the full loop would.
    // Where that reaches past every source, or takes in more than one in sixty-four of them, they
    // are all taken: searching for more would cost more than it saves.  Where most searches find
    // that many, the searching stops.
    double t0 = target_vec[indx[0]];
    double t1 = target_vec[indx[1]];
    bool crowded = splitSources.m_Searches >= 256 && splitSources.m_Overs * 2 > splitSources.m_Searches;
    if ( splitSources.m_Tree && splitSources.m_NonNegative && grm1 > 0.0 && t0 >= 0.0 && t1 >= 0.0 && !crowded )
    {
        double reach[2];
        bool all = false;
        for ( int i = 0; i < 2; i++ )
        {
            reach[i] = 1.01 * std::max( target_vec[indx[i]], splitSources.m_StrMax ) / grm1;

            const BndBox &box = splitSources.m_Box;
            const vec3d &p = m_TablePnt[indx[i]];
            double far2 = 0.0;
            for ( int j = 0; j < 3; j++ )
            {
                double d = std::max( std::abs( p[j] - box.GetMin( j ) ), std::abs( p[j] - box.GetMax( j ) ) );
                far2 += d * d;
            }
            if ( far2 <= reach[i] * reach[i] )
            {
                all = true;
            }
        }

        if ( !all )
        {
            vector< size_t > near;
            NearSources found( src.size() / 64, near );
            nanoflann::SearchParameters params;
            params.sorted = false;

            for ( int i = 0; i < 2 && !found.m_Over; i++ )
            {
                vec3d p = m_TablePnt[indx[i]];
                found.m_RadiusSq = reach[i] * reach[i];
                splitSources.m_Tree->findNeighbors( found, &p[0], params );
            }

            splitSources.m_Searches++;
            if ( found.m_Over )
            {
                splitSources.m_Overs++;
            }

            if ( !found.m_Over )
            {
                std::sort( near.begin(), near.end() );
                near.erase( std::unique( near.begin(), near.end() ), near.end() );

                ApplySplitSources( src, &near, m_TablePnt, indx, grm1, target_vec, reason_vec );
                return;
            }
        }
    }

    ApplySplitSources( src, nullptr, m_TablePnt, indx, grm1, target_vec, reason_vec );
}

// Swapping with an empty vector gives the memory back, where clear() would keep it.
void SCurve::CleanupDistTable()
{
    vector< double >().swap( u_vec );
    vector< double >().swap( dist_vec );
    vector< double >().swap( target_vec );
    vector< double >().swap( reason_vec );
    vector< vec3d >().swap( m_TableUW );
    vector< vec3d >().swap( m_TablePnt );
    m_TableBCurve = nullptr;
    vector< vec3d >().swap( m_TableUWB );
    vector< vec3d >().swap( m_TablePntB );
}

void SCurve::LimitTarget( SimpleGridDensity* grid_den )
{
    // Walk the curve forward limiting target length.
    double growratio = grid_den->m_GrowRatio;

    for ( int i = 1 ; i < num_segs ; i++ )
    {
        double dt = target_vec[i] - target_vec[i - 1];
        double ds = dist_vec[i] - dist_vec[i - 1];

        double dtlim = ( growratio - 1.0 ) * ds;
        if( dt > dtlim )
        {
            target_vec[i] = target_vec[i - 1] + dtlim;

            if ( reason_vec[i - 1] < vsp::MIN_GROW_LIMIT )
            {
                reason_vec[i] = reason_vec[i - 1] + vsp::GROW_LIMIT_INCREMENT;
            }
            else
            {
                reason_vec[i] = reason_vec[i - 1];
            }
        }
    }

    // Walk the curve backward limiting target length.
    for ( int i = num_segs - 2 ; i > -1  ; i-- )
    {
        double dt = target_vec[i] - target_vec[i + 1];
        double ds = dist_vec[i] - dist_vec[i + 1];

        double dtlim = -1.0 * ( growratio - 1.0 ) * ds;
        if( dt > dtlim )
        {
            target_vec[i] = target_vec[i + 1] + dtlim;
            if ( reason_vec[i + 1] < vsp::MIN_GROW_LIMIT )
            {
                reason_vec[i] = reason_vec[i + 1] + vsp::GROW_LIMIT_INCREMENT;
            }
            else
            {
                reason_vec[i] = reason_vec[i + 1];
            }
        }
    }
}

void SCurve::TessEndPts()
{
    m_UTess.clear();
    m_UTess.push_back( 0.0 );
    m_UTess.push_back( 1.0 );

    UWTess();
}

// Bisection method solver to find i,u corresponding to starget.
//
// starget  i       Target s input.
// s        i/o     Initial s in, final s out.
// ireal    i/o     Initial i in, final i out.
// t        i/o     t at current point.  Passed in/out as optimization.
// dsdi     i/o     ds/di at current point.  Passed in/out as optimization.
// u        o       Final u out.

bool SCurve::BisectFind( double starget, double &s, double &ireal, double &t, double &dsdi, double &u, int &reason, int direction )
{
    double sold = s;

    double tol = 1e-3;
    double ds = std::abs( starget - sold );

    double slower, supper;
    double ilower, iupper;

    if( direction < 0 ) // Descending, current point upper bound.
    {
        slower = 0.0;
        ilower = 0.0;
        supper = s;
        iupper = ireal;
    }
    else // Ascending, current point lower bound.
    {
        slower = s;
        ilower = ireal;
        supper = dist_vec.back();
        iupper = ( ( double ) dist_vec.size() ) - 1.0;
    }

    double imid, tmid, umid, smid, dsdimid;

    imid = ( ilower + iupper ) / 2.0;

    int iter = 0;
    while( std::abs( supper - slower ) / ds > tol )
    {
        InterpDistTable( imid, tmid, umid, smid, dsdimid, reason );

        if( smid < starget )
        {
            slower = smid;
            ilower = imid;
        }
        else
        {
            supper = smid;
            iupper = imid;
        }
        imid = ( ilower + iupper ) / 2.0;
        iter++;
    }

    ireal = imid;
    InterpDistTable( ireal, t, u, s, dsdi, reason );

    return true; // Failure no option for bisection.
}

// Newton's method solver to find i,u corresponding to starget.
//
// starget  i       Target s input.
// s        i/o     Initial s in, final s out.
// ireal    i/o     Initial i in, final i out.
// t        i/o     t at current point.  Passed in/out as optimization.
// dsdi     i/o     ds/di at current point.  Passed in/out as optimization.
// u        o       Final u out.

bool SCurve::NewtonFind( double starget, double &s, double &ireal, double &t, double &dsdi, double &u, int &reason )
{
    double sold = s;
    double irorig = ireal;

    double ds = std::abs( starget - sold );

    int itermax = 10;
    double tol = 1e-3;

    int iter = 0;
    while( std::abs( s - starget ) / ds > tol && iter < itermax )
    {
        double irold = ireal;
        double di = - ( s - starget ) / dsdi;

        ireal = ireal + di;

        InterpDistTable( ireal, t, u, s, dsdi, reason );

        // Check to keep Newton's method from exploding.  If the solution is
        // diverging, just move one segment in the indicated direction and
        // continue with Newton's method from there.
        if( std::abs( s - starget ) > std::abs( sold - starget ) )
        {
            if( di < 0 )
            {
                di = -1.0;
            }
            else
            {
                di = 1.0;
            }

            ireal = irold + di;

            InterpDistTable( ireal, t, u, s, dsdi, reason );
        }

        iter = iter + 1;
    }

    if( std::abs( s - starget ) / ds > tol ) // Failed to converge.  Reset to start point and return failure.
    {
        ireal = irorig;
        InterpDistTable( ireal, t, u, s, dsdi, reason );
        return false;
    }

    return true;
}


void SCurve::TessIntegrate( int direction, vector< double > &stess )
{
    stess.clear();

    int nsubstep = 5;

    double dn;

    if( direction < 0 )
    {
        dn = -1.0 / ( ( double ) nsubstep );
    }
    else
    {
        dn = 1.0 / ( ( double ) nsubstep );
    }

    int isub = 0;

    double imax = ( ( double ) dist_vec.size() ) - 1.0;

    double smax = dist_vec.back();

    double ireal;

    if( direction < 0 )
    {
        ireal = imax;
    }
    else
    {
        ireal = 0.0;
    }

    double t, u, s, dsdi;
    int reason = -1;
    InterpDistTable( ireal, t, u, s, dsdi, reason );

    stess.push_back( s );

    while( ireal <= imax && ireal >= 0.0 )
    {
        double ds = t * dn;
        double starget = s + ds;

        if( starget < 0.0 || starget > smax ) // Reached end of integration, break out and force final point.
        {
            break;
        }
        else
        {
            if( !NewtonFind( starget, s, ireal, t, dsdi, u, reason ) )
            {
                BisectFind( starget, s, ireal, t, dsdi, u, reason, direction );
            }
        }
        isub = isub + 1;

        // If we're at or beyond the last substep, add the point to the list.
        // >= is used instead of == through an abundance of caution.  There is no
        // reason that isub should ever be greater than nsubstep.
        if( isub >= nsubstep )
        {
            stess.push_back( s );
            isub = 0;
        }
    }

    // Just in case, double check the final point.
    if( direction < 0 )
    {
        if( stess.back() > 0.0 )
        {
            stess.push_back( 0.0 );
        }
    }
    else
    {
        if( stess.back() < smax )
        {
            stess.push_back( smax );
        }
    }
}

void SCurve::STessToUTess()
{
    STessToUTess( m_STess, m_UTess );
}

void SCurve::STessToUTess( const vector< double > &stess, vector< double > &utess )
{
    int ntess = stess.size();
    utess.clear();
    utess.resize( ntess, -1.0 );

    int ntable = dist_vec.size();
    int ilow = 0;
    int iup = 1;
    utess[ 0 ] = 0;

    for ( int i = 1; i < ntess - 1; i++ )
    {
        while( iup < ntable && stess[i] > dist_vec[ iup ] )
        {
            ilow++;
            iup++;
        }

        if ( iup >= ntable )
        {
            iup = ntable - 1;
            ilow = iup - 1;
        }

        double slow = dist_vec[ ilow ];
        double sup = dist_vec[ iup ];
        double frac = ( stess[i] - slow )/( sup - slow );

        double ulow = u_vec[ ilow ];
        double uup = u_vec[ iup ];

        double u = ulow + frac * ( uup - ulow );
        utess[ i ] = u;
    }

    utess[ ntess - 1 ] = 1.0;
}

void SCurve::TessIntegrateAndSmooth()
{
    TessIntegrate( 1, m_STess );

    vector< double > STessRev;
    TessIntegrate( -1, STessRev );

    double smax = dist_vec.back();

    int nfwd = m_STess.size();
    int nrev = STessRev.size();
    int n;

    if( nfwd > nrev )
    {
        n = nrev;
        m_STess.pop_back();
        m_STess[ n - 1 ] = smax;
    }
    else if( nrev > nfwd )
    {
        n = nfwd;
        STessRev.pop_back();
        STessRev[ n - 1 ] = 0.0;
    }
    else
    {
        n = nfwd;
    }

    for( int i = 1; i < n - 1; i++ )
    {
        double s = m_STess[ i ];
        double sr = STessRev[ n - i - 1 ];

        // Average of two weighted sums.
        // One weighted by (s/smax), the other by (sr/smax).
        // The weighting ensures the 'head' of each curve (forward or reverse) is taken as truth, with no
        // contribution from the 'tail' of the opposing curve.  However, in the middle the two are averaged.
        double save = ( 2.0 * s + ( (sr * sr) - (s * s) ) / smax ) / 2.0;

        m_STess[ i ] = save;
    }
    m_STess[ 0 ] = 0.0;
    m_STess[ n - 1 ] = smax;
}

void SCurve::UWTess()
{
    int n = m_UTess.size();
    m_UWTess.clear();
    m_UWTess.resize( n );

    vec3d uw;
    for ( int i = 0 ; i < n ; i++ )
    {
        double u = m_UTess[i];
        uw = UWCompPnt01( u );
        m_UWTess[i] = uw;
    }
}

void SCurve::SpreadDensity( SCurve* BCurve )
{
    // The stations and the points they land on were worked out when the table was built.
    if ( ( int )m_TablePnt.size() != num_segs || ( int )m_TablePntB.size() != num_segs )
    {
        return;
    }

    for ( int i = 0 ; i < num_segs ; i++ )
    {
        double t = target_vec[i];
        int reason = reason_vec[i];
        ApplyESAtUW( m_TableUW[i], m_TablePnt[i], t, reason );
        BCurve->ApplyESAtUW( m_TableUWB[i], m_TablePntB[i], t, reason );
    }
}

// The half of SpreadDensity that lands on this curve's surface.  Where the two curves lie on
// different surfaces the halves touch different maps, and give what SpreadDensity does.
void SCurve::SpreadDensityA()
{
    if ( ( int )m_TablePnt.size() != num_segs || ( int )m_TablePntB.size() != num_segs )
    {
        return;
    }

    for ( int i = 0 ; i < num_segs ; i++ )
    {
        ApplyESAtUW( m_TableUW[i], m_TablePnt[i], target_vec[i], reason_vec[i] );
    }
}

// The half of SpreadDensity that lands on the partner curve's surface.
void SCurve::SpreadDensityB( SCurve* BCurve )
{
    if ( ( int )m_TablePnt.size() != num_segs || ( int )m_TablePntB.size() != num_segs )
    {
        return;
    }

    for ( int i = 0 ; i < num_segs ; i++ )
    {
        BCurve->ApplyESAtUW( m_TableUWB[i], m_TablePntB[i], target_vec[i], reason_vec[i] );
    }
}

void SCurve::CalcDensity( SimpleGridDensity* grid_den, SCurve* BCurve, SplitSourceSet & splitSources )
{
    BuildDistTable( grid_den, BCurve, splitSources );

    LimitTarget( grid_den );
}

void SCurve::ApplyESAtUW( const vec3d &uw, const vec3d &p, double t, int reason )
{
    m_Surf->ApplyESAtPnt( uw, p, t, reason );
}

void SCurve::DoubleTess()
{
    int n = m_STess.size();

    // Build up tvec corresponding to points m_STess
    vector < double > tvec( n );
    tvec[0] = target_vec[0];
    int ilow = 0;
    int iup = 1;
    int ntable = dist_vec.size();
    for( int i = 1; i < n - 1; i++ )
    {
        while( iup < ntable && m_STess[i] > dist_vec[ iup ] )
        {
            ilow++;
            iup++;
        }

        if ( iup >= ntable )
        {
            iup = ntable - 1;
            ilow = iup - 1;
        }

        double slow = dist_vec[ ilow ];
        double sup = dist_vec[ iup ];
        double frac = ( m_STess[i] - slow )/( sup - slow );

        tvec[i] = target_vec[ ilow ] + frac * ( target_vec[ iup ] - target_vec[ ilow ] );
    }
    tvec[ n - 1 ] = target_vec[ ntable - 1 ];

    // Build up doubleSTess with 'mid'-points inserted.  Points are inserted at a point adjusted
    // for the target lengths at the start and end of the segment.
    vector< double > doubleSTess( 2 * ( n - 1 ) + 1 );
    for( int i = 0; i < n - 1; i++ )
    {
        doubleSTess[ 2 * i ] = m_STess[i];

        double k = 1.0 / ( 1.0 + ( tvec[ i + 1 ] / tvec[ i ] ) );
        doubleSTess[ 2 * i + 1 ] = m_STess[ i ] + k * ( m_STess[ i + 1 ] - m_STess[ i ] );
    }
    doubleSTess[ 2 * ( n - 1 ) ] = m_STess[ n - 1 ];

    m_STess.swap( doubleSTess );
}

void SCurve::Tesselate()
{
    TessIntegrateAndSmooth();
    DoubleTess();
    STessToUTess();
    UWTess();
}

void SCurve::Tesselate( const vector< double > & u_tess )
{
    m_UTess = u_tess;
    UWTess();
}

void SCurve::FlipDir()
{
    if ( m_UWPolyline.empty() )
    {
        m_UWCrv.FlipCurve();
    }
    else
    {
        std::reverse( m_UWPolyline.begin(), m_UWPolyline.end() );
    }
    CleanupDistTable();
}

vec3d SCurve::CompPntUW( double u )
{
    return UWCompPnt01( u );
}

/*
void SCurve::Draw()
{
    int i;
    glLineWidth( 1.0 );
//  glColor3ub( 255, 0, 255 );
    glBegin( GL_LINE_STRIP );

    for ( i = 0 ; i < (int)m_UWTess.size() ; i++ )
    {
        vec3d p = m_Surf->CompPnt(  m_UWTess[i].x(),  m_UWTess[i].y() );
        glVertex3dv( p.data() );
    }
    glEnd();

    glPointSize( 2.0f );
    glColor3ub( 255, 255, 0 );
    glBegin( GL_POINTS );

    for ( i = 0 ; i < (int)m_UWTess.size() ; i++ )
    {
        vec3d p = m_Surf->CompPnt(  m_UWTess[i].x(),  m_UWTess[i].y() );
        glVertex3dv( p.data() );
    }
    glEnd();

    //glPointSize( 8.0f );
    //glColor3ub( 0, 255, 0 );
    //glBegin( GL_POINTS );

    //int num_sec = m_UWCrv.get_num_sections();
    //int num_ctl = 3*(num_sec) + 1;

    //for ( i = 0 ; i < num_ctl ; i++ )
    //{
    //  vec3d uw = m_UWCrv.get_pnt( i );
    //  vec3d p = m_Surf->CompPnt(  uw.x(),  uw.y() );
    //  glVertex3dv( p.data() );
    //}
    //glEnd();

    //glBegin( GL_LINE_STRIP );

    //for ( i = 0 ; i < 100 ; i++ )
    //{
    //  double u = (double)i/99.0;
    //  vec3d uw = m_UWCrv.comp_pnt(u);
    //  vec3d p = m_Surf->CompPnt(  uw.x(),  uw.y() );
    //  glVertex3dv( p.data() );
    //}
    //glEnd();

    //glPointSize( 4.0f );
    //glColor3ub( 255, 255, 0 );
    //glBegin( GL_POINTS );

    //for ( i = 0 ; i < 100 ; i++ )
    //{
    //  double u = (double)i/99.0;
    //  vec3d uw = m_UWCrv.comp_pnt(u);
    //  vec3d p = m_Surf->CompPnt(  uw.x(),  uw.y() );
    //  glVertex3dv( p.data() );
    //}
    //glEnd();



}
*/
