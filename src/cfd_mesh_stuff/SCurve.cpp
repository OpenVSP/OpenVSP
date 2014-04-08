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
}

SCurve::SCurve( Surf* s )
{
    m_Surf = s;
    m_ICurve = NULL;
}

SCurve::~SCurve()
{
}

//==== Load Bezier Control Points =====//
void SCurve::SetBezierControlPnts( vector< vec3d > & pnts )
{
    if ( pnts.size() < 4 )
    {
        return;
    }

    int num_sects = ( ( int )pnts.size() - 1 ) / 3;
    m_UWCrv.init_num_sections( num_sects );

    for ( int i = 0 ; i < ( int )pnts.size() ; i++ )
    {
        m_UWCrv.put_pnt( i, pnts[i] );
    }
}

void SCurve::BuildBezierCurve( vector< vec3d > & pnts_to_interpolate )
{
    m_UWCrv.buildCurve( pnts_to_interpolate, 0.3, 0 );
}


//===== Read Surf From File  =====//
void SCurve::ReadFile( const char* file_name )
{
    FILE* file_id = fopen( file_name, "r" );

    if ( file_id )
    {
        int num_u;
        float u, w;

        fscanf( file_id, "%d\n", &num_u );
        m_UWCrv.init_num_sections( num_u );

        int num_pnts = num_u * 3 + 1;

        for ( int i = 0 ; i < num_pnts ; i++ )
        {
            fscanf( file_id, "%f %f\n", &u, &w );
            m_UWCrv.put_pnt( i, vec3d( u, w, 0 ) );
        }

        fclose( file_id );
    }
}

double SCurve::Length( int num_segs )
{
    assert ( m_Surf );

    double total_dist = 0;
    vec3d uw = m_UWCrv.comp_pnt( 0 );
    vec3d last_p = m_Surf->CompPnt( uw.x(), uw.y() );
    for ( int i = 1 ; i < num_segs ; i++ )
    {
        double u = ( double )i / ( double )( num_segs - 1 );
        uw = m_UWCrv.comp_pnt( u );
        vec3d p = m_Surf->CompPnt( uw.x(), uw.y() );
        total_dist += dist( p, last_p );
    }

    return total_dist;
}

void SCurve::LoadControlPnts3D( vector< vec3d > & control_pnts )
{
    int num_pnts = m_UWCrv.get_num_control_pnts();

    for ( int i = 0 ; i < num_pnts ; i++ )
    {
        vec3d uw = m_UWCrv.get_pnt( i );
        control_pnts.push_back( m_Surf->CompPnt( uw.x(), uw.y() ) );
    }
}

void SCurve::ExtractBorderControlPnts( vector< vec3d > & control_pnts )
{
    int np = m_UWCrv.get_num_control_pnts();
    if ( np < 2 )
    {
        return;
    }

    vector< vector< vec3d > > cpnts = m_Surf->GetControlPnts();
    int nu = cpnts.size();
    if ( nu < 4 )
    {
        return;
    }
    int nw = cpnts[0].size();
    if ( nw < 4 )
    {
        return;
    }

    double tol = 1.0e-12;
    vec3d uw0 = m_UWCrv.get_pnt( 0 );
    vec3d uw1 = m_UWCrv.get_pnt( np - 1 );

    if ( fabs( uw0.x() - uw1.x() ) < tol )
    {
        int ind = ( int )( uw0.x() + 0.5 ) * 3;
        if ( ind > nu - 1 )
        {
            ind = nu - 1;
        }

        for ( int w = 0 ; w < nw ; w++ )
        {
            control_pnts.push_back( cpnts[ind][w] );
        }
    }
    else if ( fabs( uw0.y() - uw1.y() ) < tol )
    {
        int ind = ( int )( uw0.y() + 0.5 ) * 3;
        if ( ind > nw - 1 )
        {
            ind = nw - 1;
        }

        for ( int u = 0 ; u < nu ; u++ )
        {
            control_pnts.push_back( cpnts[u][ind] );
        }
    }

}

double SCurve::GetTargetLen( GridDensity* grid_den, SCurve* BCurve, vec3d p, vec3d uw, double u )
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

    len = m_Surf->InterpTargetMap( uw.x(), uw.y() );

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

        vec3d uwB = BCurve->m_UWCrv.comp_pnt( u );

        double lenB = grid_den->GetBaseLen( limitFlag );

        lenB = BCurve->m_Surf->InterpTargetMap( uwB.x(), uwB.y() );

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

    int ncp = m_UWCrv.get_num_control_pnts();

    vec3d uwstart = m_UWCrv.get_pnt( 0 );
    vec3d uwend = m_UWCrv.get_pnt( ncp - 1 );

    double ust = uwstart[0];
    double wst = uwstart[1];

    double du = uwend[0] - ust;
    double dw = uwend[1] - wst;

    double uspan = fabs( du );
    double wspan = fabs( dw );

    double span = max( uspan, wspan );

    int ptsperseg = 5;

    int npt = ( ( int ) span ) * ( ptsperseg - 1 ) + 1;

    for( int i = 0; i < npt ; i++ )
    {
        double frac = ( double )i / ( double )( npt - 1 );

        vec3d uw = vec3d( ust + frac * du, wst + frac * dw, 0.0 );
        m_UWTess.push_back( uw );
    }
}

void SCurve::CheapTesselate( )
{
    int npts = 10000;

    m_UTess.clear();
    m_UTess.resize( npts );
    for ( int i = 0; i < npts; i++ )
    {
        m_UTess[i] = ( double )i / ( double )( npts - 1 );
    }

    UWTess();
}

void SCurve::ProjectTessToSurf( SCurve* othercurve )
{
    vector< vec3d > UWTessB = othercurve->GetUWTessPnts();

    Surf* SurfA = GetSurf();
    Surf* SurfB = othercurve->GetSurf();

    double uguess = SurfA->GetMaxU() / 2.0;
    double wguess = SurfA->GetMaxW() / 2.0;

    int npts = UWTessB.size();
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

    int ifloor = floor( idouble );
    int iceil = ceil( idouble );

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

void SCurve::BuildDistTable( GridDensity* grid_den, SCurve* BCurve, list< MapSource* > & splitSources )
{
    assert( m_Surf );

    CleanupDistTable();

    //==== Build U to Dist Table ====//
    int nref = 10;
    int nseglim = 10000;
    num_segs = nref * m_UWCrv.get_num_sections() + 1;
    if( num_segs > nseglim )
    {
        nref = nseglim / m_UWCrv.get_num_sections();
        if( nref < 1 )
        {
            nref = 1;
        }
        num_segs = nref * m_UWCrv.get_num_sections() + 1;
    }
    double total_dist = 0.0;
    vec3d uw = m_UWCrv.comp_pnt( 0 );
    vec3d last_p = m_Surf->CompPnt( uw.x(), uw.y() );
    for ( int i = 0 ; i < num_segs ; i++ )
    {
        double u = ( double )i / ( double )( num_segs - 1 );

        uw = m_UWCrv.comp_pnt( u );
        vec3d p = m_Surf->CompPnt( uw.x(), uw.y() );

        double t = GetTargetLen( grid_den, BCurve, p, uw, u );

        u_vec.push_back( u );
        target_vec.push_back( t );
        pnt_vec.push_back( p );

        total_dist += dist( p, last_p );
        dist_vec.push_back( total_dist );

        last_p = p;
    }

    double grm1 = grid_den->m_GrowRatio() - 1.0;

    // Indices of first and last points in table.
    int indx[2] = { 0, num_segs - 1 };

    list< MapSource* >::iterator ss;
    for ( ss = splitSources.begin(); ss != splitSources.end(); ss++ )
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
    pnt_vec.clear();
}

void SCurve::LimitTarget( GridDensity* grid_den )
{
    // Walk the curve forward limiting target length.
    double growratio = grid_den->m_GrowRatio();

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

void SCurve::TessIntegrate()
{
    vector<double> utess;
    TessIntegrate( 1, utess );
    m_UTess.swap( utess );
}

void SCurve::TessRevIntegrate( vector< double > &utess )
{
    TessIntegrate( -1, utess );
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
    double irorig = ireal;

    double tol = 1e-3;
    double ds = fabs( starget - sold );

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
    while( fabs( supper - slower ) / ds > tol )
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

bool SCurve::NewtonFind( double starget, double &s, double &ireal, double &t, double &dsdi, double &u, int direction )
{
    double sold = s;
    double irorig = ireal;

    double ds = fabs( starget - sold );

    int itermax = 10;
    double tol = 1e-3;

    int iter = 0;
    while( fabs( s - starget ) / ds > tol && iter < itermax )
    {
        double irold = ireal;
        double di = - ( s - starget ) / dsdi;

        ireal = ireal + di;

        InterpDistTable( ireal, t, u, s, dsdi );

        // Check to keep Newton's method from exploding.  If the solution is
        // diverging, just move one segment in the indicated direction and
        // continue with Newton's method from there.
        if( fabs( s - starget ) > fabs( sold - starget ) )
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

    if( fabs( s - starget ) > tol ) // Failed to converge.  Reset to start point and return failure.
    {
        ireal = irorig;
        InterpDistTable( ireal, t, u, s, dsdi );
        return false;
    }

    return true;
}


void SCurve::TessIntegrate( int direction, vector< double > &utess )
{
    utess.clear();

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

    int itermax = 10;
    double tol = 1e-3;

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

    utess.push_back( u );

    while( ireal <= imax && ireal >= 0.0 )
    {
        double ds = t * dn;
        double starget = s + ds;
        double sold = s;

        if( starget < 0.0 || starget > smax ) // Reached end of integration, break out and force final point.
        {
            break;
        }
        else
        {
            if( !NewtonFind( starget, s, ireal, t, dsdi, u, direction ) )
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
            utess.push_back( u );
            isub = 0;
        }
    }

    // Just in case, double check the final point.
    if( direction < 0 )
    {
        if( utess.back() > 0.0 )
        {
            utess.push_back( 0.0 );
        }
    }
    else
    {
        if( utess.back() < 1.0 )
        {
            utess.push_back( 1.0 );
        }
    }
}

void SCurve::SmoothTess()
{

    vector< double > UTessRev;
    TessRevIntegrate( UTessRev );

    int nfwd = m_UTess.size();
    int nrev = UTessRev.size();
    int n;

    if( nfwd > nrev )
    {
        n = nrev;
        m_UTess.pop_back();
        m_UTess[ n - 1 ] = 1.0;
    }
    else if( nrev > nfwd )
    {
        n = nfwd;
        UTessRev.pop_back();
        UTessRev[ n - 1 ] = 0.0;
    }
    else
    {
        n = nfwd;
    }

    for( int i = 1; i < n - 1; i++ )
    {
        double u = m_UTess[ i ];
        double ur = UTessRev[ n - i - 1 ];
        double uave = ( 2.0 * u - u * u + ur * ur ) / 2.0;

        m_UTess[ i ] = uave;
    }
    m_UTess[ 0 ] = 0.0;
    m_UTess[ n - 1 ] = 1.0;
}

void SCurve::UWTess()
{
    m_UWTess.clear();

    vec3d uw;
    for ( int i = 0 ; i < ( int )m_UTess.size() ; i++ )
    {
        double u = m_UTess[i];
        uw = m_UWCrv.comp_pnt( u );
        m_UWTess.push_back( uw );
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

void SCurve::CalcDensity( GridDensity* grid_den, SCurve* BCurve, list< MapSource* > & splitSources )
{
    BuildDistTable( grid_den, BCurve, splitSources );

    LimitTarget( grid_den );
}

void SCurve::ApplyESSurface( double u, double t )
{
    vec3d uw = m_UWCrv.comp_pnt( u );
    m_Surf->ApplyES( uw, t );
}


void SCurve::Tesselate()
{
    TessIntegrate();
    SmoothTess();
    UWTess();
}

// This routine is currently unused.
void SCurve::Tesselate( vector< vec3d > & target_pnts )
{
    assert( m_Surf );

    m_UTess.clear();
    m_UWTess.clear();

    vector< double > u_vec;
    vector< vec3d >  pnt_vec;

    //==== Build U to Pnt Table ====//
    int num_segs = 10000;
    vec3d uw = m_UWCrv.comp_pnt( 0 );
    vec3d last_p = m_Surf->CompPnt( uw.x(), uw.y() );
    for ( int i = 0 ; i < num_segs ; i++ )
    {
        double u = ( double )i / ( double )( num_segs - 1 );

        uw = m_UWCrv.comp_pnt( u );
        vec3d p = m_Surf->CompPnt( uw.x(), uw.y() );
        pnt_vec.push_back( p );
        u_vec.push_back( u );
        last_p = p;
    }

    //===== Look For Closest U For Each Target Point ====//
    for ( int i = 0 ; i < ( int )target_pnts.size() ; i++ )
    {
        int close_ind = 0;
        double close_d2 = 1.0e12;

        for ( int j = 1 ; j < ( int )pnt_vec.size() - 1 ; j++ )
        {
            double d2 = dist_squared( target_pnts[i], pnt_vec[j] );
            if ( d2 < close_d2 )
            {
                close_d2 = d2;
                close_ind = j;
            }
        }

        //==== Make Sure Ind is Valid ====//
        double f0, f1;
        double d0 = pointSegDistSquared( target_pnts[i], pnt_vec[close_ind], pnt_vec[close_ind - 1], &f0 );
        double d1 = pointSegDistSquared( target_pnts[i], pnt_vec[close_ind], pnt_vec[close_ind + 1], &f1 );
        double u;
        if ( d0 < d1 )
        {
            u = u_vec[close_ind] + ( u_vec[close_ind - 1] - u_vec[close_ind] ) * f0;
        }
        else
        {
            u = u_vec[close_ind] + ( u_vec[close_ind + 1] - u_vec[close_ind] ) * f1;
        }

        m_UTess.push_back( u );
        m_UWTess.push_back( m_UWCrv.comp_pnt( u ) );

//vec3d uw = m_UWCrv.comp_pnt( u );
//double d = dist(  target_pnts[i], m_Surf->CompPnt( uw[0], uw[1] ) );
//if ( d > 0.01 )
//  printf( "SCurve Tess Target %f %f \n", f0, f1 );
    }

    //==== Reset Begin and End Points ====//
    m_UTess.front()  = 0.0;
    m_UWTess.front() = m_UWCrv.comp_pnt( 0.0 );
    m_UTess.back()   = 1.0;
    m_UWTess.back()  = m_UWCrv.comp_pnt( 1.0 );







}

void SCurve::Tesselate( vector< double > & u_tess )
{

    m_UTess = u_tess;
    m_UWTess.clear();
    for ( int i = 0 ; i < ( int )m_UTess.size() ; i++ )
    {
        double u = m_UTess[i];
        vec3d uw = m_UWCrv.comp_pnt( u );
        m_UWTess.push_back( uw );
    }
}

void SCurve::FlipDir()
{
    int i;
    vector <vec3d> pnts;
    int num_pnts = m_UWCrv.get_num_control_pnts();
    for (  i = 0 ; i < num_pnts ; i++ )
    {
        pnts.push_back( m_UWCrv.get_pnt( i ) );
    }

    for (  i = 0 ; i < num_pnts ; i++ )
    {
        m_UWCrv.put_pnt( i, pnts[num_pnts - 1 - i] );
    }
}

vec3d SCurve::CompPntUW( double u )
{
    return m_UWCrv.comp_pnt( u );
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
