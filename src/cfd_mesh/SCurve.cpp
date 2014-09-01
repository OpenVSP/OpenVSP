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

    m_UWCrv.put_pnts( pnts );
}

void SCurve::BuildBezierCurve( vector< vec3d > & pnts_to_interpolate )
{
    m_UWCrv.buildCurve( pnts_to_interpolate );
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

void SCurve::ExtractBorderControlPnts( vector< vec3d > & control_pnts )
{
    vector< vector< vec3d > > cpnts = m_Surf->GetSurfCore()->GetControlPnts();
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
    vec3d uw0 = m_UWCrv.first_pnt();
    vec3d uw1 = m_UWCrv.last_pnt();

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

    vec3d uwstart = m_UWCrv.first_pnt();
    vec3d uwend = m_UWCrv.last_pnt();

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

    double uguess = SurfA->GetSurfCore()->GetMaxU() / 2.0;
    double wguess = SurfA->GetSurfCore()->GetMaxW() / 2.0;

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
    m_UWCrv.flipCurve();
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
