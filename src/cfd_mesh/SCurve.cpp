//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// SCurve
//
//////////////////////////////////////////////////////////////////////

#include "SCurve.h"

SCurve::SCurve()
{
    m_Surf = NULL;
    m_ICurve = NULL;
    num_segs = 0;
}

SCurve::SCurve( Surf* s )
{
    m_Surf = s;
    m_ICurve = NULL;
    num_segs = 0;
}

SCurve::~SCurve()
{
}

void SCurve::InterpolateLinear(vector<vec3d> &pnts_to_interpolate)
{
    m_UWCrv.InterpolateLinear( pnts_to_interpolate );
}

void SCurve::PromoteTo( int deg )
{
    m_UWCrv.PromoteTo( deg );
}

double SCurve::Length( int num_segs )
{
    assert ( m_Surf );

    double total_dist = 0;
    vec3d uw = m_UWCrv.CompPnt01( 0 );
    vec3d last_p = m_Surf->CompPnt( uw.x(), uw.y() );
    for ( int i = 1 ; i < num_segs ; i++ )
    {
        double u = ( double )i / ( double )( num_segs - 1 );
        uw = m_UWCrv.CompPnt01( u );
        vec3d p = m_Surf->CompPnt( uw.x(), uw.y() );
        total_dist += dist( p, last_p );
    }

    return total_dist;
}


void SCurve::GetBorderCurve( Bezier_curve & crv ) const
{
    vec3d uw0 = m_UWCrv.FirstPnt();
    vec3d uw1 = m_UWCrv.LastPnt();
    m_Surf->GetBorderCurve( uw0, uw1, crv );
}

double SCurve::GetTargetLen( SimpleGridDensity* grid_den, SCurve* BCurve, vec3d p, vec3d uw, double u )
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

    len = min( len, m_Surf->InterpTargetMap( uw.x(), uw.y() ) );

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

        vec3d uwB = BCurve->m_UWCrv.CompPnt01( u );

        double lenB = grid_den->GetBaseLen( limitFlag );

        lenB = min( lenB, BCurve->m_Surf->InterpTargetMap( uwB.x(), uwB.y() ) );

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

    vec3d uwstart = m_UWCrv.FirstPnt();
    vec3d uwend = m_UWCrv.LastPnt();

    double ust = uwstart[0];
    double wst = uwstart[1];

    double du = uwend[0] - ust;
    double dw = uwend[1] - wst;

    double uspan = std::abs( du );
    double wspan = std::abs( dw );

    double span = max( uspan, wspan );

    int ptsperseg = 5;

    int npt = ( ceil( span ) ) * ( ptsperseg - 1 ) + 1;

    for( int i = 0; i < npt ; i++ )
    {
        double frac = ( double )i / ( double )( npt - 1 );

        vec3d uw = vec3d( ust + frac * du, wst + frac * dw, 0.0 );
        m_UWTess.push_back( uw );
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

void SCurve::InterpDistTable( double idouble, double &t, double &u, double &s, double &dsdi )
{
    int imax = target_vec.size() - 1;

    if ( idouble > imax )
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

}

void SCurve::BuildDistTable( SimpleGridDensity* grid_den, SCurve* BCurve, list< MapSource* > & splitSources )
{
    assert( m_Surf );

    CleanupDistTable();
    vector< vec3d >  pnt_vec;

    //==== Build U to Dist Table ====//
    int nref = 10;
    int nseglim = 10000;
    num_segs = nref * m_UWCrv.GetNumSections() + 1;
    if( num_segs > nseglim )
    {
        nref = nseglim / m_UWCrv.GetNumSections();
        if( nref < 1 )
        {
            nref = 1;
        }
        num_segs = nref * m_UWCrv.GetNumSections() + 1;
    }
    double total_dist = 0.0;
    vec3d uw = m_UWCrv.CompPnt01( 0 );
    vec3d last_p = m_Surf->CompPnt( uw.x(), uw.y() );
    for ( int i = 0 ; i < num_segs ; i++ )
    {
        double u = ( double )i / ( double )( num_segs - 1 );

        uw = m_UWCrv.CompPnt01( u );
        vec3d p = m_Surf->CompPnt( uw.x(), uw.y() );

        double t = GetTargetLen( grid_den, BCurve, p, uw, u );

        u_vec.push_back( u );
        target_vec.push_back( t );
        pnt_vec.push_back( p );

        total_dist += dist( p, last_p );
        dist_vec.push_back( total_dist );

        last_p = p;
    }

    double grm1 = grid_den->m_GrowRatio - 1.0;

    // Indices of first and last points in table.
    int indx[2] = { 0, num_segs - 1 };

    list< MapSource* >::iterator ss;
    for ( ss = splitSources.begin(); ss != splitSources.end(); ++ss )
    {
        vec3d pt = ( *ss )->m_pt;
        double str = ( *ss )->m_str;

        for ( int i = 0; i < 2; i++ ) // Loop over first and last points.
        {
            double r = dist( pt, pnt_vec[indx[i]] );
            double targetstr = str + r * grm1;

            if ( targetstr < target_vec[indx[i]] )
            {
                target_vec[indx[i]] = targetstr;
            }
            else
            {
                double targetrev = target_vec[indx[i]] + r * grm1;
                if ( targetrev < str )
                {
                    ( *ss )->m_str = targetrev;
                }
            }
        }
    }
}

void SCurve::CleanupDistTable()
{
    u_vec.clear();
    dist_vec.clear();
    target_vec.clear();
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

bool SCurve::BisectFind( double starget, double &s, double &ireal, double &t, double &dsdi, double &u, int direction )
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
        InterpDistTable( imid, tmid, umid, smid, dsdimid );

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
    InterpDistTable( ireal, t, u, s, dsdi );

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

bool SCurve::NewtonFind( double starget, double &s, double &ireal, double &t, double &dsdi, double &u )
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

        InterpDistTable( ireal, t, u, s, dsdi );

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

            InterpDistTable( ireal, t, u, s, dsdi );
        }

        iter = iter + 1;
    }

    if( std::abs( s - starget ) > tol ) // Failed to converge.  Reset to start point and return failure.
    {
        ireal = irorig;
        InterpDistTable( ireal, t, u, s, dsdi );
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
    InterpDistTable( ireal, t, u, s, dsdi );

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
            if( !NewtonFind( starget, s, ireal, t, dsdi, u ) )
            {
                BisectFind( starget, s, ireal, t, dsdi, u, direction );
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
        uw = m_UWCrv.CompPnt01( u );
        m_UWTess[i] = uw;
    }
}

void SCurve::SpreadDensity( SCurve* BCurve )
{
    for ( int i = 0 ; i < num_segs ; i++ )
    {
        double u = u_vec[i];
        double t = target_vec[i];
        ApplyESSurface( u, t );
        BCurve->ApplyESSurface( u, t );
    }
}

void SCurve::CalcDensity( SimpleGridDensity* grid_den, SCurve* BCurve, list< MapSource* > & splitSources )
{
    BuildDistTable( grid_den, BCurve, splitSources );

    LimitTarget( grid_den );
}

void SCurve::ApplyESSurface( double u, double t )
{
    vec3d uw = m_UWCrv.CompPnt01( u );
    m_Surf->ApplyES( uw, t );
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
    m_UWCrv.FlipCurve();
}

vec3d SCurve::CompPntUW( double u )
{
    return m_UWCrv.CompPnt01( u );
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
