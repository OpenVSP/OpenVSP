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

SCurve::SCurve(Surf* s)
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
		return;
		
	int num_sects = ((int)pnts.size() - 1)/3;
	m_UWCrv.init_num_sections( num_sects );

	for ( int i = 0 ; i < (int)pnts.size() ; i++ )
		m_UWCrv.put_pnt( i, pnts[i] );
}

void SCurve::BuildBezierCurve( vector< vec3d > & pnts_to_interpolate )
{
	m_UWCrv.buildCurve( pnts_to_interpolate, 0.3, 0 );
}

	
//===== Read Surf From File  =====//
void SCurve::ReadFile( const char* file_name )
{
	FILE* file_id = fopen(file_name, "r");

	if ( file_id )
	{
		int num_u;
		float u, w;

		fscanf( file_id, "%d\n", &num_u );
		m_UWCrv.init_num_sections( num_u );

		int num_pnts = num_u*3 + 1;

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
		double u = (double)i/(double)(num_segs-1);
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
		vec3d uw = m_UWCrv.get_pnt(i);
		control_pnts.push_back( m_Surf->CompPnt( uw.x(), uw.y() ) );
	}
}

double SCurve::GetTargetLen( GridDensity* grid_den, SCurve* BCurve, vec3d p, vec3d uw, double u )
{
	double grid_len = grid_den->GetTargetLen( p );

	double curv_len = m_Surf->TargetLen( uw.x(), uw.y(), grid_den->GetMaxGap(), grid_den->GetRadFrac() );

	if(BCurve){
		vec3d uwB = BCurve->m_UWCrv.comp_pnt( u );
		double curv_lenB = BCurve->m_Surf->TargetLen( uwB.x(), uwB.y(), grid_den->GetMaxGap(), grid_den->GetRadFrac() );
		curv_len = min( curv_len, curv_lenB );
	}

	double t_len = min( grid_len, curv_len );

	double target_len = max( t_len, grid_den->GetMinLen() );
	return target_len;
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

	vec3d uwstart = m_UWCrv.get_pnt(0);
	vec3d uwend = m_UWCrv.get_pnt(ncp-1);

	double ust = uwstart[0];
	double wst = uwstart[1];

	double du = uwend[0] - ust;
	double dw = uwend[1] - wst;

	double uspan = fabs( du );
	double wspan = fabs( dw );

	double span = max( uspan, wspan );

	int ptsperseg = 5;

	int npt = ( (int) span ) * (ptsperseg - 1) + 1;

	for( int i = 0; i < npt ; i++ )
	{
		double frac = (double)i/(double)( npt - 1 );

		vec3d uw = vec3d(ust + frac * du, wst + frac * dw, 0.0);
		m_UWTess.push_back( uw );
	}
}

void SCurve::BuildDistTable( GridDensity* grid_den, SCurve* BCurve )
{
	assert( m_Surf );

	//==== Build U to Dist Table ====//
	num_segs = 10000;
	double total_dist = 0.0;
	vec3d uw = m_UWCrv.comp_pnt( 0 );
	vec3d last_p = m_Surf->CompPnt( uw.x(), uw.y() );
	for ( int i = 0 ; i < num_segs ; i++ )
	{
		double u = (double)i/(double)(num_segs-1);

		uw = m_UWCrv.comp_pnt( u );
		vec3d p = m_Surf->CompPnt( uw.x(), uw.y() );

		double t = GetTargetLen( grid_den, BCurve, p, uw, u);

		u_vec.push_back( u );
		target_vec.push_back( t );
		pnt_vec.push_back( p );

		total_dist += dist( p, last_p );
		dist_vec.push_back( total_dist );

		last_p = p;
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
	double growratio = grid_den->GetGrowRatio();

	for ( int i = 1 ; i < num_segs ; i++ )
	{
		double dt = target_vec[i]-target_vec[i-1];
		double ds = dist_vec[i]-dist_vec[i-1];

		double dtlim = ( growratio - 1.0 ) * ds;
		if( dt > dtlim )
		{
			target_vec[i] = target_vec[i-1] + dtlim;
		}
	}

	// Walk the curve backward limiting target length.
	for ( int i = num_segs-2 ; i > -1  ; i-- )
	{
		double dt = target_vec[i]-target_vec[i+1];
		double ds = dist_vec[i]-dist_vec[i+1];

		double dtlim = -1.0 * ( growratio - 1.0 ) * ds;
		if( dt > dtlim )
		{
			target_vec[i] = target_vec[i+1] + dtlim;
		}
	}
}

void SCurve::TessIntegrate()
{
	m_UTess.clear();
	double nprev = 0.0;
	double uprev = 0.0;

	m_UTess.push_back( 0.0 );

	int nlast = 0;
	double n = 0.0;
	double dn;

	// Start at i = 1 because ds for the first step is zero anyway.
	for ( int i = 1 ; i < num_segs ; i++ )
	{
		double t = target_vec[i];
		double ds = dist_vec[i] - dist_vec[i-1];
		double u = u_vec[i];

		dn = ds/t;
		n += dn;

		if( nlast != (int) n )
		{
			double denom = n-nprev;
			double frac = 0.0;
			if(denom)
				frac = (u-uprev)/denom;

			double ut = uprev + frac * (u-uprev);

			m_UTess.push_back( ut );
			nlast = (int) n;
		}

		uprev = u;
		nprev = n;
	}

	double ufinal = m_UTess.back();

	if( ufinal < 1.0 )
	{
		double du = 1-ufinal;
		double dutarget = 1.0/((1.0*num_segs) * dn);

		if( du > 0.5 * dutarget )
		{
			m_UTess.push_back( 1.0 );
		}
		else
		{
			int ifinal = m_UTess.size()-1;
			if( ifinal > 0 )
				m_UTess[ifinal] = ( m_UTess[ifinal-1]+1.0 ) / 2.0;
			m_UTess.push_back( 1.0 );
		}
	}
}

void SCurve::UWTess()
{
	m_UWTess.clear();

	vec3d uw;
	for ( int i = 0 ; i < m_UTess.size() ; i++ )
	{
		double u = m_UTess[i];
		uw = m_UWCrv.comp_pnt( u );
		m_UWTess.push_back( uw );
	}
}

void SCurve::CalcDensity( GridDensity* grid_den, SCurve* BCurve )
{
	BuildDistTable( grid_den, BCurve );

	LimitTarget( grid_den );
}

void SCurve::Tesselate( )
{
	TessIntegrate();

	UWTess();

	CleanupDistTable();
}

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
		double u = (double)i/(double)(num_segs-1);

		uw = m_UWCrv.comp_pnt( u );
		vec3d p = m_Surf->CompPnt( uw.x(), uw.y() );
		pnt_vec.push_back( p );
		u_vec.push_back( u );
		last_p = p;
	}

	//===== Look For Closest U For Each Target Point ====//
	for ( int i = 0 ; i < (int)target_pnts.size() ; i++ )
	{
		int close_ind = 0;
		double close_d2 = 1.0e12;

		for ( int j = 1 ; j < (int)pnt_vec.size()-1 ; j++ )
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
		double d0 = pointSegDistSquared( target_pnts[i], pnt_vec[close_ind], pnt_vec[close_ind-1], &f0 );
		double d1 = pointSegDistSquared( target_pnts[i], pnt_vec[close_ind], pnt_vec[close_ind+1], &f1 );
		double u;
		if ( d0 < d1 )
			u = u_vec[close_ind] + (u_vec[close_ind-1] - u_vec[close_ind])*f0;
		else
			u = u_vec[close_ind] + (u_vec[close_ind+1] - u_vec[close_ind])*f1;

		m_UTess.push_back( u );
		m_UWTess.push_back( m_UWCrv.comp_pnt( u ) );

//vec3d uw = m_UWCrv.comp_pnt( u );
//double d = dist(  target_pnts[i], m_Surf->CompPnt( uw[0], uw[1] ) );
//if ( d > 0.01 )
//	printf( "SCurve Tess Target %f %f \n", f0, f1 );
	}


//	if ( m_UTess.size() == 1 )
//	{
//int junk = 23;
//
//	}

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
	for ( int i = 0 ; i < (int)m_UTess.size() ; i++ )
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
		pnts.push_back( m_UWCrv.get_pnt(i) );
	}

	for (  i = 0 ; i < num_pnts ; i++ )
	{
		m_UWCrv.put_pnt( i, pnts[num_pnts-1-i] );
	}
}

vec3d SCurve::CompPntUW( double u )
{
	return m_UWCrv.comp_pnt( u );
}


void SCurve::Draw()
{
	int i;
	glLineWidth( 1.0 );
//	glColor3ub( 255, 0, 255 );
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
	//	vec3d uw = m_UWCrv.get_pnt( i );
	//	vec3d p = m_Surf->CompPnt(  uw.x(),  uw.y() );
	//	glVertex3dv( p.data() );
	//}
	//glEnd();

	//glBegin( GL_LINE_STRIP );

	//for ( i = 0 ; i < 100 ; i++ )
	//{
	//	double u = (double)i/99.0;
	//	vec3d uw = m_UWCrv.comp_pnt(u);
	//	vec3d p = m_Surf->CompPnt(  uw.x(),  uw.y() );
	//	glVertex3dv( p.data() );
	//}
	//glEnd();

	//glPointSize( 4.0f );
	//glColor3ub( 255, 255, 0 );
	//glBegin( GL_POINTS );

	//for ( i = 0 ; i < 100 ; i++ )
	//{
	//	double u = (double)i/99.0;
	//	vec3d uw = m_UWCrv.comp_pnt(u);
	//	vec3d p = m_Surf->CompPnt(  uw.x(),  uw.y() );
	//	glVertex3dv( p.data() );
	//}
	//glEnd();
		


}

