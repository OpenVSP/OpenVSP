//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// Surf
//
//////////////////////////////////////////////////////////////////////

#include "Surf.h"
#include "SCurve.h"
#include "ISegChain.h"
#include "tritri.h"

Surf::Surf()
{
	m_NumU = m_NumW = 0;
	m_GridDensityPtr = 0;
	m_CompID = -1;
	m_SurfID = -1;
	m_Mesh.SetSurfPtr( this );
}

Surf::~Surf()
{
	int i;
	//==== Delete Patches ====//
	for ( i = 0 ; i < (int)m_PatchVec.size() ; i++ )
		delete m_PatchVec[i];

	//==== Delete SCurves ====//
	for ( i = 0 ; i < (int)m_SCurveVec.size() ; i++ )
		delete m_SCurveVec[i];
}

void Surf::BuildClean()
{
	int i;
	//==== Delete SCurves ====//
	for ( i = 0 ; i < (int)m_SCurveVec.size() ; i++ )
		delete m_SCurveVec[i];

	m_SCurveVec.clear();
}
	
//===== Read Surf From File  =====//
void Surf::ReadSurf( FILE* file_id )
{
	int i, j;
	if ( file_id )
	{
		char buff[256];

		int num_u, num_w;
		double x, y, z;
		fgets( buff, 256, file_id );
		sscanf( buff, "%d %d\n", &num_u, &num_w );

		vector< vector< vec3d > > pnts;
		pnts.resize( num_u );
		for ( i = 0 ; i < num_u ; i++ )
			pnts[i].resize( num_w );

		for ( i = 0 ; i < num_u ; i++ )
			for ( j = 0 ; j < num_w ; j++ )
			{
				fgets( buff, 256, file_id );
				sscanf( buff, "%lf %lf %lf\n", &x, &y, &z );
				pnts[i][j].set_xyz( x, y, z );
			}
		LoadControlPnts( pnts );
	}
}

void Surf::LoadControlPnts( vector< vector< vec3d > > & control_pnts )
{
	int i, j;
	assert( control_pnts.size() >= 4 );
	assert( control_pnts[0].size() >= 4);
	m_Pnts = control_pnts;

	m_NumU = m_Pnts.size();
	m_NumW = m_Pnts[0].size();
	m_MaxU = (m_NumU-1)/3;
	m_MaxW = (m_NumW-1)/3;

	//==== Load Patch Vec ====//
	m_BBox.init();
	for ( i = 0 ; i < (int)m_PatchVec.size() ; i++ )
		delete m_PatchVec[i];
	m_PatchVec.clear();

	for ( i = 0 ; i < m_NumU-1 ; i+=3 )
	{
		for ( j = 0 ; j < m_NumW-1 ; j+=3 )
		{
			SurfPatch* patch = new SurfPatch();
			for ( int pi = 0 ; pi < 4 ; pi++ )
				for ( int pj = 0 ; pj < 4 ; pj++ )
				{
					m_BBox.update( m_Pnts[pi+i][pj+j] );
					patch->put_pnt( pi, pj, m_Pnts[pi+i][pj+j] );

					patch->set_u_min_max( i/3, i/3 + 1.0 );
					patch->set_w_min_max( j/3, j/3 + 1.0 );
				}
			patch->set_surf_ptr( this );
			patch->compute_bnd_box();
			m_PatchVec.push_back( patch );
		}
	}
}

//===== Compute Blending Functions  =====//
void Surf::BlendFuncs(double u, double& F1, double& F2, double& F3, double& F4)
{
  //==== Compute All Blending Functions ====//
  double uu = u*u;
  double one_u = 1.0 - u;
  double one_u_sqr = one_u*one_u;

  F1 = one_u*one_u_sqr;
  F2 = 3.0*u*one_u_sqr;
  F3 = 3.0*uu*one_u;
  F4 = uu*u;
}

void Surf::BlendDerivFuncs(double u, double& F1, double& F2, double& F3, double& F4)
{
	double uu    = u*u;
	double one_u = 1.0 - u;
	F1 = -3.0*one_u*one_u;
	F2 = 3.0 - 12.0*u + 9.0*uu;
	F3 = 6.0*u - 9.0*uu;
	F4 = 3.0*uu;
}

//===== Compute Point On Surf Given  U W (Between 0 1 ) =====//
vec3d Surf::CompPnt01( double u, double w )
{
	return CompPnt( u*m_MaxU, w*m_MaxW );
}


//===== Compute Tangent In U Direction   =====//
vec3d Surf::CompTanU01(double u01, double w01)
{
 	vec3d pnt;
	if ( m_NumU < 4 || m_NumW < 4 )
		return pnt;

	double uall = u01*m_MaxU;
	double wall = w01*m_MaxW;

	double F1u, F2u, F3u, F4u;
	double F1w, F2w, F3w, F4w;

	int trunc_u = (int)uall;
	int u_ind = trunc_u*3;
	if (u_ind >= m_NumU-1 ) 
	{
		F1u = F2u = F3u = 0.0;
		F4u = 1.0;
		u_ind = m_NumU-4;
	}
	else
	{
		BlendDerivFuncs(uall-(double)trunc_u, F1u, F2u, F3u, F4u);
	}

	int trunc_w = (int)wall;
	int w_ind = trunc_w*3;
	if (w_ind >= m_NumW-1) 
	{
		F1w = F2w = F3w = 0.0;
		F4w = 1.0;
		w_ind = m_NumW-4;
	}
	else
	{
		BlendFuncs(wall-(double)trunc_w, F1w, F2w, F3w, F4w);
	}

  pnt = 
     (( m_Pnts[u_ind][w_ind]*F1u     + m_Pnts[u_ind+1][w_ind]*F2u +         
        m_Pnts[u_ind+2][w_ind]*F3u   + m_Pnts[u_ind+3][w_ind]*F4u)*F1w) +
     (( m_Pnts[u_ind][w_ind+1]*F1u   + m_Pnts[u_ind+1][w_ind+1]*F2u +         
        m_Pnts[u_ind+2][w_ind+1]*F3u + m_Pnts[u_ind+3][w_ind+1]*F4u)*F2w) +
      (( m_Pnts[u_ind][w_ind+2]*F1u  + m_Pnts[u_ind+1][w_ind+2]*F2u +         
        m_Pnts[u_ind+2][w_ind+2]*F3u + m_Pnts[u_ind+3][w_ind+2]*F4u)*F3w) +
      (( m_Pnts[u_ind][w_ind+3]*F1u  + m_Pnts[u_ind+1][w_ind+3]*F2u +         
	    m_Pnts[u_ind+2][w_ind+3]*F3u + m_Pnts[u_ind+3][w_ind+3]*F4u)*F4w);

	return pnt;
}

//===== Compute Tangent In U Direction   =====//
vec3d Surf::CompTanW01(double u01, double w01)
{
 	vec3d pnt;
	if ( m_NumU < 4 || m_NumW < 4 )
		return pnt;

	double uall = u01*m_MaxU;
	double wall = w01*m_MaxW;

	double F1u, F2u, F3u, F4u;
	double F1w, F2w, F3w, F4w;

	int trunc_u = (int)uall;
	int u_ind = trunc_u*3;
	if (u_ind >= m_NumU-1 ) 
	{
		F1u = F2u = F3u = 0.0;
		F4u = 1.0;
		u_ind = m_NumU-4;
	}
	else
	{
		BlendFuncs(uall-(double)trunc_u, F1u, F2u, F3u, F4u);
	}

	int trunc_w = (int)wall;
	int w_ind = trunc_w*3;
	if (w_ind >= m_NumW-1) 
	{
		F1w = F2w = F3w = 0.0;
		F4w = 1.0;
		w_ind = m_NumW-4;
	}
	else
	{
		BlendDerivFuncs(wall-(double)trunc_w, F1w, F2w, F3w, F4w);
	}

  pnt = 
     (( m_Pnts[u_ind][w_ind]*F1u     + m_Pnts[u_ind+1][w_ind]*F2u +         
        m_Pnts[u_ind+2][w_ind]*F3u   + m_Pnts[u_ind+3][w_ind]*F4u)*F1w) +
     (( m_Pnts[u_ind][w_ind+1]*F1u   + m_Pnts[u_ind+1][w_ind+1]*F2u +         
        m_Pnts[u_ind+2][w_ind+1]*F3u + m_Pnts[u_ind+3][w_ind+1]*F4u)*F2w) +
      (( m_Pnts[u_ind][w_ind+2]*F1u  + m_Pnts[u_ind+1][w_ind+2]*F2u +         
        m_Pnts[u_ind+2][w_ind+2]*F3u + m_Pnts[u_ind+3][w_ind+2]*F4u)*F3w) +
      (( m_Pnts[u_ind][w_ind+3]*F1u  + m_Pnts[u_ind+1][w_ind+3]*F2u +         
	    m_Pnts[u_ind+2][w_ind+3]*F3u + m_Pnts[u_ind+3][w_ind+3]*F4u)*F4w);

	return pnt;
}
//===== Compute Point On Surf Given  U W =====//
vec3d Surf::CompPnt(double u, double w)
{
	vec3d pnt;

	if ( m_NumU < 4 || m_NumW < 4 )
		return pnt;

	double F1u, F2u, F3u, F4u;
	double F1w, F2w, F3w, F4w;

	int trunc_u = (int)u;
	int u_ind = trunc_u*3;
	if (u_ind >= m_NumU-1 ) 
	{
		F1u = F2u = F3u = 0.0;
		F4u = 1.0;
		u_ind = m_NumU-4;
	}
	else
	{
		BlendFuncs(u-(double)trunc_u, F1u, F2u, F3u, F4u);
	}

	int trunc_w = (int)w;
	int w_ind = trunc_w*3;
	if (w_ind >= m_NumW-1) 
	{
		F1w = F2w = F3w = 0.0;
		F4w = 1.0;
		w_ind = m_NumW-4;
	}
	else
	{
		BlendFuncs(w-(double)trunc_w, F1w, F2w, F3w, F4w);
	}

  pnt = 
     (( m_Pnts[u_ind][w_ind]*F1u     + m_Pnts[u_ind+1][w_ind]*F2u +         
        m_Pnts[u_ind+2][w_ind]*F3u   + m_Pnts[u_ind+3][w_ind]*F4u)*F1w) +
     (( m_Pnts[u_ind][w_ind+1]*F1u   + m_Pnts[u_ind+1][w_ind+1]*F2u +         
        m_Pnts[u_ind+2][w_ind+1]*F3u + m_Pnts[u_ind+3][w_ind+1]*F4u)*F2w) +
      (( m_Pnts[u_ind][w_ind+2]*F1u  + m_Pnts[u_ind+1][w_ind+2]*F2u +         
        m_Pnts[u_ind+2][w_ind+2]*F3u + m_Pnts[u_ind+3][w_ind+2]*F4u)*F3w) +
      (( m_Pnts[u_ind][w_ind+3]*F1u  + m_Pnts[u_ind+1][w_ind+3]*F2u +         
	  m_Pnts[u_ind+2][w_ind+3]*F3u + m_Pnts[u_ind+3][w_ind+3]*F4u)*F4w);

	return pnt;
}
	
vec2d Surf::ClosestUW( vec3d & pnt, double guess_u, double guess_w, double guess_del_u, double guess_del_w, double tol )
{
	double u = guess_u;
	double w = guess_w;
	double del_u = guess_del_u;
	double del_w = guess_del_w;
	double dist = dist_squared( pnt, CompPnt(u, w) );

	if ( dist < tol )
		return vec2d( u, w );

	int iter = 20;
	while ( iter > 0 )
	{
		double u_plus  = u + del_u;
		double u_minus = u - del_u;
		if ( u_plus  > m_MaxU )
		{
			u_plus  = m_MaxU;
			del_u *= 0.25;
		}
		if ( u_minus < 0 )
		{
			u_minus = 0.0;
			del_u *= 0.25;
		}

		double dist_plus_u  = dist_squared( pnt, CompPnt(u_plus, w) );
		double dist_minus_u = dist_squared( pnt, CompPnt(u_minus, w) );
		if ( dist_plus_u < dist )
		{
			u = u_plus;
			del_u *= 2.0;
			dist = dist_plus_u;
		}
		else if ( dist_minus_u < dist )
		{
			u = u_minus;
			del_u *= 2.0;
			dist = dist_minus_u;
		}
		else
		{
			del_u *= 0.5;
			iter--;
		}

		double w_plus = w + del_w;
		double w_minus = w - del_w;
		if ( w_plus > m_MaxW )
		{
			w_plus = m_MaxW;
			del_w *= 0.25;
		}
		if ( w_minus < 0 )
		{
			w_minus = 0;
			del_w *= 0.25;
		}

		double dist_plus_w  = dist_squared( pnt, CompPnt(u, w_plus ) );
		double dist_minus_w = dist_squared( pnt, CompPnt(u, w_minus ) );
		if ( dist_plus_w < dist )
		{
			w = w_plus;
			del_w *= 2.0;
			dist = dist_plus_w;
		}
		else if ( dist_minus_w < dist )
		{
			w = w_minus;
			del_w *= 2.0;
			dist = dist_minus_w;
		}
		else
		{
			del_w *= 0.5;
			iter--;
		}
	}

	return vec2d( u, w );
}

vec2d Surf::ClosestUW( vec3d & pnt_in, double guess_u, double guess_w )
{
	int Max_Iter  = 10;
	double UW_Tol = 1.0e-14;
	double norm_uw[2];
	double del_uw[2];
	vec3d guess_pnt;

//	double orig_d = dist_squared( pnt_in, CompPnt( guess_u, guess_w ) );

	//==== Normalize uw Values Between 0 and 1 ====//
	norm_uw[0] = guess_u/m_MaxU;
	norm_uw[1] = guess_w/m_MaxW;

	//===== Loop Until U and W Stops Changing or Max Iterations is Hit  =====//
	int cnt = 0;
	int stop_flag = FALSE;
	while ( !stop_flag )
	{
		guess_pnt = CompPnt01( norm_uw[0], norm_uw[1] );

		//==== Find Delta U and W Values ====//
		CompDeltaUW( pnt_in, guess_pnt, norm_uw, del_uw );

		norm_uw[0] += del_uw[0]/m_MaxU;
		norm_uw[1] -= del_uw[1]/m_MaxW;

		if ( norm_uw[0] < 0.0 ) norm_uw[0] = 0.0;
		else if ( norm_uw[0] > 1.0 ) norm_uw[0] = 1.0;
		if ( norm_uw[1] < 0.0 ) norm_uw[1] = 0.0;
		else if ( norm_uw[1] > 1.0 ) norm_uw[1] = 1.0;

		//==== Test To Stop Iteration ====//
		if ( (fabs(del_uw[0]) + fabs(del_uw[1])) < UW_Tol )
			stop_flag = TRUE;
		else if ( cnt > Max_Iter )
			stop_flag = TRUE;
		else
			cnt++;
	}

	//==== Convert uw Values Back to Original Space ====//
	vec2d uw;
	uw[0] = norm_uw[0]*m_MaxU;             
	uw[1] = norm_uw[1]*m_MaxW;  
//
//	double new_d = dist_squared( pnt_in, guess_pnt );
//	if ( new_d > orig_d )
//	{
//		//===== Brute Force ====//
//		uw = ClosestUW( pnt_in, guess_u, guess_w, 0.001, 0.001, 0.00001 );
//	}
  
	//==== Clamp Values At Bounds  ====//
	if      ( uw[0] < 0.0 )		uw[0] = 0.0;
	else if ( uw[0] > m_MaxU )  uw[0] = m_MaxU;
      
	if      ( uw[1] < 0.0 )		uw[1] = 0.0;
	else if ( uw[1] > m_MaxW )	uw[1] = m_MaxW;

	return uw;
}


//===== Compute Point On Patch  =====//
void Surf::CompDeltaUW( vec3d& pnt_in, vec3d& guess_pnt, double norm_uw[2], double delta_uw[2] )
{
	vec3d tan_u = CompTanU01( norm_uw[0], norm_uw[1] );
	vec3d tan_w = CompTanW01( norm_uw[0], norm_uw[1] );

	vec3d dist_vec = guess_pnt - pnt_in;

	vec3d A = cross(tan_w, dist_vec);
	vec3d B = cross(tan_u, dist_vec);
	vec3d norm = cross(tan_u, tan_w);

	double N = dot(norm, norm);

	if( fabs(N) > DBL_EPSILON )
	{
		delta_uw[0] = dot(A, norm)/N;
		delta_uw[1] = dot(B, norm)/N;
	}
	else
	{
		delta_uw[0] = 0.0;
		delta_uw[1] = 0.0;
	}
}

bool Surf::LessThanY( double val )
{
	for ( int i = 0 ; i < m_NumU ; i++ )
		for ( int j = 0 ; j < m_NumW ; j++ )
		{
			if ( m_Pnts[i][j][1] > val )
				return false;
		}
	return true;


}

bool Surf::OnYZeroPlane()
{
	double tol = 0.0000001;
	bool onPlaneFlag = true;
	for ( int i = 0 ; i < m_NumU ; i++ )				// Border Curve 1 w = 0
	{
		if ( fabs( m_Pnts[i][0][1] ) > tol )		
			onPlaneFlag = false;
	}
	if ( onPlaneFlag ) return onPlaneFlag;

	onPlaneFlag = true;
	for ( int i = 0 ; i < m_NumU ; i++ )				// Border Curve 2 w = max
	{
		if ( fabs( m_Pnts[i][m_NumW-1][1] ) > tol )		
			onPlaneFlag = false;
	}
	if ( onPlaneFlag ) return onPlaneFlag;

	onPlaneFlag = true;
	for ( int i = 0 ; i < m_NumW ; i++ )				// Border Curve 3 u = 0
	{
		if ( fabs( m_Pnts[0][i][1] ) > tol )	
			onPlaneFlag = false;
	}
	if ( onPlaneFlag ) return onPlaneFlag;

	onPlaneFlag = true;
	for ( int i = 0 ; i < m_NumW ; i++ )				// Border Curve 4 u = max
	{
		if ( fabs( m_Pnts[m_NumU-1][i][1] ) > tol )	
			onPlaneFlag = false;
	}
	if ( onPlaneFlag ) return onPlaneFlag;

	return false;
}

bool Surf::PlaneAtYZero()
{
	double tol = 0.000001;
	for ( int i = 0 ; i < m_NumU ; i++ )
		for ( int j = 0 ; j < m_NumW ; j++ )
		{
			double yval = m_Pnts[i][j][1];
			if ( fabs( yval ) > tol )
				return false;
		}
	return true;
}

void Surf::FindBorderCurves()
{
	double degen_tol = 0.000001;

	//==== Load 4 Border Curves if Not Degenerate ====//
	SCurve* scrv;
	double max_u = (m_NumU-1)/3.0;
	double max_w = (m_NumW-1)/3.0;

	vector< vec3d > pnts;
	pnts.resize( 4 );

	pnts[0].set_xyz( 0,		    0, 0 );			// Inc U
	pnts[1].set_xyz( max_u/2,   0, 0 );
	pnts[2].set_xyz( max_u/2, 0, 0 );
	pnts[3].set_xyz( max_u,     0, 0 );

	scrv = new SCurve( this );
	scrv->SetBezierControlPnts( pnts );

	if ( scrv->Length(10) > degen_tol )
		m_SCurveVec.push_back( scrv );
	else
		delete scrv;

	pnts[0].set_xyz( max_u,		  0, 0 );		// Inc W
	pnts[1].set_xyz( max_u,   max_w/2, 0 );
	pnts[2].set_xyz( max_u,   max_w/2, 0 );
	pnts[3].set_xyz( max_u,     max_w, 0 );

	scrv = new SCurve( this );
	scrv->SetBezierControlPnts( pnts );

	if ( scrv->Length(10) > degen_tol )
		m_SCurveVec.push_back( scrv );
	else
		delete scrv;

	pnts[0].set_xyz( max_u,	    max_w, 0 );			// Dec U
	pnts[1].set_xyz( max_u/2,	max_w, 0 );
	pnts[2].set_xyz( max_u/2,	max_w, 0 );
	pnts[3].set_xyz( 0,	        max_w, 0 );

	scrv = new SCurve( this );
	scrv->SetBezierControlPnts( pnts );

	if ( scrv->Length(10) > degen_tol )
		m_SCurveVec.push_back( scrv );
	else
		delete scrv;

	pnts[0].set_xyz( 0,	max_w,   0 );			// Dec W
	pnts[1].set_xyz( 0,	max_w/2, 0 );
	pnts[2].set_xyz( 0,	max_w/2, 0 );
	pnts[3].set_xyz( 0,	0,       0 );

	scrv = new SCurve( this );
	scrv->SetBezierControlPnts( pnts );

	if ( scrv->Length(10) > degen_tol )
		m_SCurveVec.push_back( scrv );
	else
		delete scrv;
}

void Surf::LoadSCurves( vector< SCurve* > & scurve_vec )
{
	for ( int i = 0 ; i < (int)m_SCurveVec.size() ; i++ )
	{
		scurve_vec.push_back( m_SCurveVec[i] );
	}
}

void Surf::TesselateCurves()
{
	for ( int i = 0 ; i < (int)m_SCurveVec.size() ; i++ )
	{
		m_SCurveVec[i]->Tesselate(m_GridDensityPtr);
	}
}

void Surf::BuildGrid()
{
	int i;
	vector< vec3d > uw_border;

	for ( i = 0 ; i < (int)m_SCurveVec.size() ; i++ )
	{
		vector< vec3d > suw_vec = m_SCurveVec[i]->GetUWTessPnts();

		for ( int j = 0 ; j < (int)suw_vec.size() ; j++ )
		{
			if ( uw_border.size() )			// Check For Duplicate Points
			{
				double d = dist( uw_border.back(), suw_vec[j] );
				if ( d > 0.0000001 )
					uw_border.push_back( suw_vec[j] );
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
	//	m_Mesh.Remesh(100);

	//}

}

void Surf::WriteSTL( const char* filename )
{
	m_Mesh.WriteSTL( filename );
}

void Surf::Intersect( Surf* surfPtr )
{
	int i;
	if ( surfPtr->GetCompID() == m_CompID )
		return;

	if ( !compare( m_BBox, surfPtr->GetBBox() ) )
		return;

	vector< SurfPatch* > otherPatchVec = surfPtr->GetPatchVec();
	for ( i = 0 ; i < (int)m_PatchVec.size() ; i++ )
		if ( compare( *m_PatchVec[i]->get_bbox(), surfPtr->GetBBox() ) ) 
		{
			for ( int j = 0 ; j < (int)otherPatchVec.size() ; j++ )
			{
				if ( compare( *m_PatchVec[i]->get_bbox(), *otherPatchVec[j]->get_bbox() ) )
				{
					intersect( *m_PatchVec[i], *otherPatchVec[j], 0 );
					m_PatchVec[i]->draw_flag = true;
					otherPatchVec[j]->draw_flag = true;
				}
			}
		}
}

void Surf::IntersectLineSeg( vec3d & p0, vec3d & p1, vector< double > & t_vals )
{
	bbox line_box;
	line_box.update( p0 );
	line_box.update( p1 );

	if ( !compare( line_box, m_BBox ) )
		return;

	for ( int i = 0 ; i < (int)m_PatchVec.size() ; i++ )
	{
		m_PatchVec[i]->IntersectLineSeg( p0, p1, line_box, t_vals );
	}
}

void Surf::IntersectLineSegMesh( vec3d & p0, vec3d & p1, vector< double > & t_vals )
{
	bbox line_box;
	line_box.update( p0 );
	line_box.update( p1 );

	if ( !compare( line_box, m_BBox ) )
		return;

	double tparm, uparm, vparm;
	list< Tri* >::iterator t;
	list <Tri*> triList = m_Mesh.GetTriList(); 

	vec3d dir = p1 - p0;

	for ( t = triList.begin() ; t != triList.end(); t++ )
	{
		int iFlag = intersect_triangle( p0.v, dir.v, 
			(*t)->n0->pnt.v, (*t)->n1->pnt.v, (*t)->n2->pnt.v, &tparm, &uparm, &vparm );

		if ( iFlag && tparm > 0.0 )
		{
			//==== Find If T is Already Included ====//
			int dupFlag = 0;
			for ( int j = 0 ; j < (int)t_vals.size() ; j++ )
			{
				if ( fabs(tparm-t_vals[j]) < 0.0000001 )
				{
					dupFlag = 1;
					break;
				}
			}

			if ( !dupFlag )
				t_vals.push_back( tparm );
		}
	}
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
//	if ( ccnt%5 == 0 ) fprintf( fp, "COLOR RED\n" );
//	else if ( ccnt % 5 == 1 ) fprintf( fp, "COLOR BLUE\n" );
//	else if ( ccnt % 5 == 2 ) fprintf( fp, "COLOR GREEN\n" );
//	else if ( ccnt % 5 == 3 ) fprintf( fp, "COLOR PURPLE\n" );
//	else if ( ccnt % 5 == 4 ) fprintf( fp, "COLOR YELLOW\n" );
//	fprintf( fp, "MOVE \n" );
//	ccnt++;
//		for ( int j = 1 ; j < (int)chains[i]->m_TessVec.size() ; j++ )
//		{
//			vec2d uw0 = chains[i]->m_TessVec[j-1]->GetPuw( this )->m_UW;
//			vec2d uw1 = chains[i]->m_TessVec[j]->GetPuw( this )->m_UW;
//			fprintf( fp, "%f %f\n", uw0[0], uw0[1] );
//			fprintf( fp, "%f %f\n", uw1[0], uw1[1] );
//		}
//}
//fclose(fp);

	//==== Store Only One Instance of each IPnt ====//
	set< IPnt* > ipntSet;
	for ( int i = 0 ; i < (int)chains.size() ; i++ )
		for ( int j = 0 ; j < (int)chains[i]->m_TessVec.size() ; j++ )
		{
			ipntSet.insert( chains[i]->m_TessVec[j] );
		}

	vector < vec2d > uwPntVec;

	set< IPnt* >::iterator ip;
	for ( ip = ipntSet.begin() ; ip != ipntSet.end() ; ip++ )
	{
		vec2d uw = (*ip)->GetPuw( this )->m_UW;

		int min_id;
		double min_dist = 1.0;
		for ( int i = 0 ; i < (int)uwPntVec.size() ; i++ )
		{
			double d = dist( uwPntVec[i], uw );
			if ( d < min_dist )
			{
				min_dist = d;
				min_id = i;
			}
		}

		if ( min_dist < 0.0001 )
		{
			(*ip)->m_Index = min_id;
		}
		else
		{
			uwPntVec.push_back( uw );
			(*ip)->m_Index = uwPntVec.size()-1;
		}
	}


	MeshSeg seg;
	vector< MeshSeg > isegVec;
	for ( int i = 0 ; i < (int)chains.size() ; i++ )
		for ( int j = 1 ; j < (int)chains[i]->m_TessVec.size() ; j++ )
		{
			seg.m_Index[0] = chains[i]->m_TessVec[j-1]->m_Index;
			seg.m_Index[1] = chains[i]->m_TessVec[j]->m_Index;
			isegVec.push_back( seg );
		}

//	////jrg Check For Duplicate Segs
//	vector< MeshSeg > dupMeshSegVec;
//	for ( int i = 0 ; i < (int)isegVec.size() ; i++ )
//	{
//		int iind0 = isegVec[i].m_Index[0];
//		int iind1 = isegVec[i].m_Index[1];
//		bool dup = false;
//		for ( int j = i+1 ; j < (int)isegVec.size() ; j++ )
//		{
//			int jind0 = isegVec[j].m_Index[0];
//			int jind1 = isegVec[j].m_Index[1];
//			if ( (iind0 == jind0 && iind1 == jind1) || 
//				 (iind0 == jind1 && iind1 == jind0) )
//			{
//				dup = true;
//				//printf("Surf: Duplicate Seg \n" );
//			}
//		}
//		if ( !dup )
//			dupMeshSegVec.push_back( isegVec[i] );
//	}
//	isegVec = dupMeshSegVec;


////jrg - Check For Point Close to Other Segs
//vector< MeshSeg > newMeshSegVec;
//for ( int i = 0 ; i < (int)isegVec.size() ; i++ )
//{
//	int iind0 = isegVec[i].m_Index[0];
//	int iind1 = isegVec[i].m_Index[1];
//
//	for ( int j = 0 ; j < (int)uwPntVec.size() ; j++ )
//	{
//		if ( j != iind0 && j != iind1 )
//		{
//			vec2d proj = proj_pnt_on_line_seg(uwPntVec[iind0], uwPntVec[iind1], uwPntVec[j] );
//			double d = dist( proj, uwPntVec[j] );
//
//			if ( d < 0.000001 )
//			{
//				MeshSeg addseg1, addseg2;
//				addseg1.m_Index[0] = iind0;
//				addseg1.m_Index[1] = j;
//				addseg2.m_Index[0] = j;
//				addseg2.m_Index[1] = iind1;
//				newMeshSegVec.push_back( addseg1 );
//				newMeshSegVec.push_back( addseg2 );
//
//				//printf("Surf: %d Proj Pnt Dist = %f\n", (int)this, d );
//				//printf("  Seg = %f %f   %f %f\n", 
//				//	uwPntVec[iind0].x(),  uwPntVec[iind0].y(), 
//				//	uwPntVec[iind1].x(),  uwPntVec[iind1].y());
//				//vec3d p = CompPnt( uwPntVec[j].x(), uwPntVec[j].y() );
//				//printf("  Pnt uv = %f %f    %f %f %f\n", uwPntVec[j].x(), uwPntVec[j].y(), p.x(), p.y(), p.z() );
//			}
//			else
//			{
//				newMeshSegVec.push_back( isegVec[i] );
//			}
//		}
//	}
//}

//vector< MeshSeg > smallMeshSegVec;
//for ( int i = 0 ; i < (int)isegVec.size() ; i++ )
//{
//	int ind0 = isegVec[i].m_Index[0];
//	int ind1 = isegVec[i].m_Index[1];
//	double d = dist( uwPntVec[ind0], uwPntVec[ind1] );
//	if ( d > 0.00000001 )
//	{
//		smallMeshSegVec.push_back( isegVec[i] );
//	}
//	else
//		printf("Surf: %d Small Seg Dist = %12.12f\n", (int)this, d );
//}
//isegVec = smallMeshSegVec;


	m_Mesh.InitMesh( uwPntVec, isegVec );
}


void Surf::BuildDistMap()
{
	int i, j;
	int nump = 101;

	//==== Load Point Vec ====//
	vector< vector< vec3d > > pvec;
	pvec.resize( nump );
	for ( i = 0 ; i < nump ; i++ )
	{
		pvec[i].resize( nump );
		double u = (double)i/(double)(nump-1);
		for ( j = 0 ; j < nump ; j++ )
		{
			double w = (double)j/(double)(nump-1);
			pvec[i][j] = CompPnt01( u, w );
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
			sum_d += dist( pvec[j-1][i], pvec[j][i] );
		}
		uDistVec.push_back( sum_d );

		if ( sum_d > maxUDist )
			maxUDist = sum_d;
	}
	if ( maxUDist < DBL_EPSILON )
		maxUDist = 0.000000001;

	//==== Find W Dists ====//
	double maxWDist = 0.0;
	vector< double > wDistVec;
	for ( i = 0 ; i < nump ; i++ )
	{
		double sum_d = 0.0;
		for ( j = 1 ; j < nump ; j++  )
		{
			sum_d += dist( pvec[i][j-1], pvec[i][j] );
		}
		wDistVec.push_back( sum_d );

		if ( sum_d > maxWDist )
			maxWDist = sum_d;
	}

	if ( maxWDist < DBL_EPSILON )
		maxWDist = 0.000000001;


	//==== Scale U Dists ====//
	double wu_ratio = m_MaxW/m_MaxU;
	m_UScaleMap.resize( uDistVec.size() );
	for ( i = 0 ; i < (int)uDistVec.size() ; i++ )
	{
		m_UScaleMap[i] = wu_ratio*(uDistVec[i]/maxWDist);

		if ( m_UScaleMap[i] < 0.00001 )
			m_UScaleMap[i] = 0.00001;
	}

	//==== Scale W Dists ====//
	double uw_ratio = m_MaxU/m_MaxW;
	m_WScaleMap.resize( wDistVec.size() );
	for ( i = 0 ; i < (int)wDistVec.size() ; i++ )
	{
		m_WScaleMap[i] = uw_ratio*(wDistVec[i]/maxUDist);

		if ( m_WScaleMap[i] < 0.00001 )
			m_WScaleMap[i] = 0.00001;
	}

	//==== Figure Out Which to Scale ====//
	double min_u_scale = 1.0e12;
	double max_u_scale = 0.0;
	for ( i = 0 ; i < (int)m_UScaleMap.size() ; i++ )
	{	
		if ( m_UScaleMap[i] < min_u_scale ) min_u_scale = m_UScaleMap[i];
		if ( m_UScaleMap[i] > max_u_scale ) max_u_scale = m_UScaleMap[i];
	}
	double u_ratio = max_u_scale/min_u_scale;

	double min_w_scale = 1.0e12;
	double max_w_scale = 0.0;
	for ( i = 0 ; i < (int)m_WScaleMap.size() ; i++ )
	{	
		if ( m_WScaleMap[i] < min_w_scale ) min_w_scale = m_WScaleMap[i];
		if ( m_WScaleMap[i] > max_w_scale ) max_w_scale = m_WScaleMap[i];
	}
	double w_ratio = max_w_scale/min_w_scale;

	if ( u_ratio > w_ratio )
		m_ScaleUFlag = true;
	else
		m_ScaleUFlag = false;
	


//char str[256];
//static int cnt = 0;
//sprintf( str, "uwscale_%d.dat", cnt );
//cnt++;
//	FILE* fp = fopen(str, "w");
//	fprintf( fp, "ws 1 2\n" );
//	fprintf( fp, "color green\n" );
//	for ( i = 0 ; i < (int)m_WScaleMap.size() ; i++ )
//	{
//		double u = (double)i/(double)(m_WScaleMap.size()-1);
//		fprintf( fp, "%f %f \n", u, m_WScaleMap[i]  );
//	}
//	fprintf( fp, "color blue\n" );
//	for ( i = 0 ; i < 1001 ; i++ )
//	{
//		double u = (double)i/(double)(1000);
//		fprintf( fp, "%f %f \n", u, GetWScale(u)  );
//	}
//	fclose( fp );

}

double Surf::GetUScale( double w )		// w 0->1
{
	if ( !m_ScaleUFlag )
		return 1.0;

	int num = m_UScaleMap.size();
	int ind = (int)(w*(double)(num-1));
	if ( ind < 0 ) ind = 0;
	if ( ind > num-2 )ind = num-2;

	double fract = w*(double)(num-1) - (double)ind;
	if ( fract < 0.0 ) fract = 0.0;
	if ( fract > 1.0 ) fract = 1.0;

	double uscale = m_UScaleMap[ind] + fract*(m_UScaleMap[ind+1] - m_UScaleMap[ind]);
	return uscale;
}

double Surf::GetWScale( double u )		// u 0->1
{
	if ( m_ScaleUFlag )
		return 1.0;

	int num = m_WScaleMap.size();
	int ind = (int)(u*(double)(num-1));
	if ( ind < 0 ) ind = 0;
	if ( ind > num-2 )ind = num-2;

	double fract = u*(double)(num-1) - (double)ind;
	if ( fract < 0.0 ) fract = 0.0;
	if ( fract > 1.0 ) fract = 1.0;

	double wscale = m_WScaleMap[ind] + fract*(m_WScaleMap[ind+1] - m_WScaleMap[ind]);
	return wscale;
}

bool Surf::ValidUW( vec2d & uw )
{
return true;
	double slop = 0.0001;
	if ( uw[0] < -slop )			return false;
	if ( uw[1] < -slop )			return false;
	if ( uw[0] > m_MaxU + slop )	return false;
	if ( uw[1] > m_MaxW + slop)		return false;

	if ( uw[0] < 0.0 )		uw[0] = 0.0;
	if ( uw[1] < 0.0 )		uw[1] = 0.0;
	if ( uw[0] > m_MaxU )	uw[0] = m_MaxU;
	if ( uw[1] > m_MaxW )	uw[1] = m_MaxW;

	return true;
}
	
void Surf::LoadBorderCurves( vector< vector <vec3d> > & borderCurves )
{
	vector< vec3d > borderPnts;

	borderPnts.clear();
	for ( int i = 0 ; i < m_NumU ; i++ )				// Border Curve w = 0
		borderPnts.push_back( m_Pnts[i][0] );
	borderCurves.push_back( borderPnts );

	borderPnts.clear();
	for ( int i = 0 ; i < m_NumU ; i++ )				// Border Curve w = max
		borderPnts.push_back( m_Pnts[i][m_NumW-1] );
	borderCurves.push_back( borderPnts );

	borderPnts.clear();
	for ( int i = 0 ; i < m_NumW ; i++ )				// Border Curve u = 0
		borderPnts.push_back( m_Pnts[0][i] );
	borderCurves.push_back( borderPnts );

	borderPnts.clear();
	for ( int i = 0 ; i < m_NumW ; i++ )				// Border Curve u = max
		borderPnts.push_back( m_Pnts[m_NumU-1][i] );
	borderCurves.push_back( borderPnts );
}
	
bool Surf::BorderCurveMatch( vector< vec3d > & curveA, vector< vec3d > & curveB )
{
	double tol = 0.00000001;

	if ( curveA.size() != curveB.size() )
		return false;

	bool match = true;

	//==== Check Same Way ====//
	for ( int i = 0 ; i < (int)curveA.size() ; i++ )
	{
		if ( dist_squared( curveA[i], curveB[i] ) > tol )
		{
			match = false;
			break;
		}
	}

	if ( match )	
		return true;

	match = true;
	int max_ind = (int)curveB.size() - 1;
	//==== Check Opposite Way ====//
	for ( int i = 0 ; i < (int)curveA.size() ; i++ )
	{
		if ( dist_squared( curveA[i], curveB[max_ind-i] ) > tol )
		{
			match = false;
			break;
		}
	}

	if ( match )	
		return true;

	return false;
}

bool Surf::BorderMatch( Surf* otherSurf )
{
	vector< vector< vec3d > > borderCurvesA;
	LoadBorderCurves( borderCurvesA );

	vector< vector< vec3d > > borderCurvesB;
	otherSurf->LoadBorderCurves( borderCurvesB );

	for ( int i = 0 ; i < (int)borderCurvesA.size() ; i++ )
	{
		for ( int j = 0 ; j < (int)borderCurvesB.size() ; j++ )
		{
			if ( BorderCurveMatch( borderCurvesA[i], borderCurvesB[j] ) )
				return true;
		}
	}
	return false;
}


void Surf::Draw()
{
	////==== Draw Control Hull ====//
	//glLineWidth( 2.0 );
	//glColor3ub( 0, 0, 255 );

	//for ( int i = 0 ; i < (int)m_Pnts.size() ; i++ )
	//{
	//	glBegin( GL_LINE_STRIP );
	//	for ( int j = 0 ; j < (int)m_Pnts[i].size() ; j++ )
	//	{
	//		glVertex3dv( m_Pnts[i][j].data() );
	//	}
	//	glEnd();
	//}
	//glPointSize( 3.0 );
	//glColor3ub( 255, 255, 255 );
	//glBegin( GL_POINTS );
	//for ( int i = 0 ; i < (int)m_Pnts.size() ; i++ )
	//{
	//	for ( int j = 0 ; j < (int)m_Pnts[i].size() ; j++ )
	//	{
	//		glVertex3dv( m_Pnts[i][j].data() );
	//	}
	//}
	//glEnd();


/************************************************/

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
/**********************************/

	//for ( int i = 0 ; i < (int)m_SCurveVec.size() ; i++ )
	//{
	//	m_SCurveVec[i]->Draw();
	//}

	m_Mesh.Draw();

	//for ( int i = 0 ; i < (int)m_PatchVec.size() ; i++ )
	//{
	//	m_PatchVec[i]->Draw();
	//}

	//glLineWidth( 2.0 );
	//glColor3ub( 255, 0, 0 );
	//glBegin( GL_LINES );
	//for ( int i = 0 ; i < (int)ipnts.size() ; i++ )
	//{
	//	if ( i%4 > 1 )
	//		glColor3ub( 255, 0, 0 );
	//	else
	//		glColor3ub( 255, 255, 0 );


	//	vec3d uw = ipnts[i];
	//	vec3d p = CompPnt( uw[0], uw[1] );
	//	glVertex3dv( p.data() );
	//}
	//glEnd();



}





