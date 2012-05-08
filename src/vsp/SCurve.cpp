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
	m_StartSearchIndex = 0;
	m_ICurve = NULL;
}

SCurve::SCurve(Surf* s)
{
	m_Surf = s;
	m_StartSearchIndex = 0;
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

void SCurve::ExtractBorderControlPnts( vector< vec3d > & control_pnts )
{
	int np = m_UWCrv.get_num_control_pnts();
	if ( np < 2 )	return;

	vector< vector< vec3d > > cpnts = m_Surf->GetControlPnts();
	int nu = cpnts.size();
	if ( nu < 4 )	return;
	int nw = cpnts[0].size();
	if ( nw < 4 )	return;

	double tol = 1.0e-12;
	vec3d uw0 = m_UWCrv.get_pnt( 0 );
	vec3d uw1 = m_UWCrv.get_pnt( np-1 );

	if ( fabs( uw0.x() - uw1.x() ) < tol )
	{
		int ind = (int)(uw0.x() + 0.5)*3;
		if ( ind > nu-1 )	ind = nu-1;

		for ( int w = 0 ; w < nw ; w++ )
		{
			control_pnts.push_back( cpnts[ind][w] );
		}
	}
	else if ( fabs( uw0.y() - uw1.y() ) < tol )
	{
		int ind = (int)(uw0.y() + 0.5)*3;
		if ( ind > nw-1 )	ind = nw-1;

		for ( int u = 0 ; u < nu ; u++ )
			control_pnts.push_back( cpnts[u][ind] );
	}

}


void SCurve::Tesselate( GridDensity* grid_den )
{
	assert( m_Surf );

	m_UTess.clear();
	m_UWTess.clear();
	m_StartSearchIndex = 0;

	vector< double > u_vec;
	vector< double > dist_vec;
	vector< vec3d >  pnt_vec;

	//==== Build U to Dist Table ====//
	int num_segs = 10000;
	double total_dist = 0.0;
	vec3d uw = m_UWCrv.comp_pnt( 0 );
	vec3d last_p = m_Surf->CompPnt( uw.x(), uw.y() );
	for ( int i = 0 ; i < num_segs ; i++ )
	{
		double u = (double)i/(double)(num_segs-1);

		uw = m_UWCrv.comp_pnt( u );
		vec3d p = m_Surf->CompPnt( uw.x(), uw.y() );
		pnt_vec.push_back( p );

		u_vec.push_back( u );

		total_dist += dist( p, last_p );
		dist_vec.push_back( total_dist );

		last_p = p;
	}


	bool stopFlag = false;

	double u = 0.0;
	uw = m_UWCrv.comp_pnt( u );
	m_UWTess.push_back( uw );

	total_dist = 0;
	m_StartSearchIndex = 0;

	m_UTess.push_back( 0.0 );
	while ( !stopFlag )
	{
		vec3d p = m_Surf->CompPnt( uw.x(), uw.y() );

		double target_len = grid_den->GetTargetLen( p );
		total_dist += target_len;

//		double u = FindU( p, target_len, u_vec, pnt_vec );
		double u = FindUDist( total_dist, u_vec, dist_vec );

		if ( u < 1.0 )
		{
			m_UTess.push_back( u );
			uw = m_UWCrv.comp_pnt( u );
			m_UWTess.push_back( uw );
		}
		else
		{
			uw = m_UWCrv.comp_pnt( u );
			double last_dist = dist( p, m_Surf->CompPnt( uw.x(), uw.y() ) );

			//bool remove_last_point = false;
			//if ( last_dist < 0.75*target_len )
			//{
			//	remove_last_point = true;
			//}

			if (  m_UTess.size() == 1 )
			{
				m_UTess.push_back( 1.0 );
				uw = m_UWCrv.comp_pnt( 1.0 );
				m_UWTess.push_back( uw );
			}
			else if ( last_dist < 0.75*target_len )
			{
				m_UTess[ m_UTess.size()-1 ]   = u;
				m_UWTess[ m_UWTess.size()-1 ] = uw;
			}
			else
			{
				m_UTess.push_back( u );
				m_UWTess.push_back( uw );
			}
			stopFlag = true;
		}
	}

	//if ( m_UTess.size() == 1 )
	//{
	//	m_UTess.push_back( 1.0 );
	//	uw = m_UWCrv.comp_pnt( 1.0 );
	//	m_UWTess.push_back( uw );
	//}


//FILE* fp = fopen("curve_dist.dat", "w");
//for ( int i = 1 ; i < m_PntVec.size() ; i++ )
//{
//	vec3d p0 = m_Surf1->CompPnt( m_PntVec[i-1].x(),  m_PntVec[i-1].y() );
//	vec3d p1 = m_Surf1->CompPnt( m_PntVec[i].x(),  m_PntVec[i].y() );
//	double d = dist( p0, p1 );
//	fprintf(fp, "Dist = %f\n", d );
//}
//fclose(fp);

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

double SCurve::FindU( vec3d & last_pnt, double target_len, vector< double > & u_vec, vector< vec3d > & pnt_vec )
{
	assert( u_vec.size() == pnt_vec.size() );

	for ( int i = m_StartSearchIndex ; i < (int)pnt_vec.size()-1 ; i++ )
	{
		double di  = dist( last_pnt, pnt_vec[i] );
		double dii = dist( last_pnt, pnt_vec[i+1] );

		if ( target_len >= di && target_len <= dii )
		{
			double denom = dii - di;
			double fract = 0.0;

			if ( denom )
				fract = (target_len - di)/denom;

			double u = u_vec[i] + fract*( u_vec[i+1] - u_vec[i] );

			if ( i > 0 )
				m_StartSearchIndex = i-1;

			return u;
		}
	}
	return 1.0;
}
	
double SCurve::FindUDist( double target_dist, vector< double > & u_vec, vector< double > & dist_vec )
{
	assert( u_vec.size() == dist_vec.size() );

	if ( m_StartSearchIndex >= (int)dist_vec.size() )
		m_StartSearchIndex = 0;

	if ( dist_vec[m_StartSearchIndex] > target_dist )
		m_StartSearchIndex = 0;

	for ( int i = m_StartSearchIndex ; i < (int)dist_vec.size()-1 ; i++ )
	{
		if ( target_dist >= dist_vec[i] && target_dist <= dist_vec[i+1] )
		{
			double denom = dist_vec[i+1] - dist_vec[i];
			double fract = 0.0;

			if ( denom )
				fract = (target_dist - dist_vec[i])/denom;

			double u = u_vec[i] + fract*( u_vec[i+1] - u_vec[i] );
			
			if ( i > 0 )
				m_StartSearchIndex = i-1;

			return u;
		}
	}
	return 1.0;
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

