//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// ICurve
//
//////////////////////////////////////////////////////////////////////

#include "ICurve.h"
#include "SCurve.h"
#include "CfdMeshMgr.h"

////////////////////////////////////////////////////////////////////////
//ISeg::ISeg()
//{
//	m_DebugFlag = false;
//	m_SharedPnt[0] = m_SharedPnt[1] = NULL;
//}
//
//ISeg::~ISeg()
//{
//
//
//}
//
//void ISeg::Copy( ISeg & s )
//{
//	m_SurfA  = s.m_SurfA;
//	m_SurfB  = s.m_SurfB;
//	m_AUW[0] = s.m_AUW[0];
//	m_AUW[1] = s.m_AUW[1];
//	m_BUW[0] = s.m_BUW[0];
//	m_BUW[1] = s.m_BUW[1];
//	m_Pnt[0] = s.m_Pnt[0];
//	m_Pnt[1] = s.m_Pnt[1];
//}
//
//void ISeg::FlipDir()
//{
//	vec2d tmp2d;
//	tmp2d = m_AUW[0]; m_AUW[0] = m_AUW[1]; m_AUW[1] = tmp2d;
//	tmp2d = m_BUW[0]; m_BUW[0] = m_BUW[1]; m_BUW[1] = tmp2d;
//
//	vec3d tmp3d;
//	tmp3d = m_Pnt[0]; m_Pnt[0] = m_Pnt[1]; m_Pnt[1] = tmp3d;
//}
//
//vec2d* ISeg::GetPnt( Surf* sPtr, int ind )
//{
//	if ( sPtr == m_SurfA )
//		return &m_AUW[ind];
//	else if ( sPtr == m_SurfB )
//		return &m_BUW[ind];
//	else
//		return NULL;
//}
////////////////////////////////////////////////////////////////////////
//bool ISegSplit::Match( ISegSplit* split )
//{
//	double tol = 0.01;
//	if ( m_Surf == split->m_Surf && dist( m_UW, split->m_UW ) < tol )
//		return true;
//
//	if ( m_Surf == split->m_OtherSurf && dist( m_UW, split->m_OtherUW ) < tol )
//		return true;
//
//	if ( m_OtherSurf == split->m_Surf && dist( m_OtherUW, split->m_UW ) < tol )
//		return true;
//
//	if ( m_OtherSurf == split->m_OtherSurf && dist( m_OtherUW, split->m_OtherUW ) < tol )
//		return true;
//
//	return false;
//}
//
////////////////////////////////////////////////////////////////////////
//void ISegBox::BuildSubDivide()
//{
//	vec2d uw0, uw1;
//	for ( int i = m_BeginInd ; i < m_EndInd ; i++ )
//	{
//		if ( m_Surf == m_ChainPtr->m_ISegDeque[i]->m_SurfA )
//		{
//			uw0 = m_ChainPtr->m_ISegDeque[i]->m_AUW[0];
//			uw1 = m_ChainPtr->m_ISegDeque[i]->m_AUW[1];
//		}
//		else if ( m_Surf == m_ChainPtr->m_ISegDeque[i]->m_SurfB )
//		{
//			uw0 = m_ChainPtr->m_ISegDeque[i]->m_BUW[0];
//			uw1 = m_ChainPtr->m_ISegDeque[i]->m_BUW[1];
//		}
//		m_Box.update( vec3d( uw0.x(), uw0.y(), 0.0 ) );
//		m_Box.update( vec3d( uw1.x(), uw1.y(), 0.0 ) );
//	}
//
//
//	if ( m_EndInd - m_BeginInd <= 8 )
//		return;
//
//	m_SubBox[0] = new ISegBox;
//	m_SubBox[1] = new ISegBox;
//
//	m_SubBox[0]->m_ChainPtr = m_ChainPtr;
//	m_SubBox[1]->m_ChainPtr = m_ChainPtr;
//
//	m_SubBox[0]->m_Surf = m_Surf;
//	m_SubBox[1]->m_Surf = m_Surf;
//
//	int split = (m_EndInd + m_BeginInd)/2;
//
//	m_SubBox[0]->m_BeginInd = m_BeginInd;
//	m_SubBox[0]->m_EndInd   = split;
//
//	m_SubBox[1]->m_BeginInd = split;
//	m_SubBox[1]->m_EndInd = m_EndInd;
//
//	m_SubBox[0]->BuildSubDivide();
//	m_SubBox[1]->BuildSubDivide();
//
//}
//	
//void ISegBox::Intersect( ISegBox* box )
//{
//	int i, j;
//	if ( !compare( m_Box, box->m_Box ) )
//		return;
//
//	if ( m_SubBox[0] && m_SubBox[1] )
//	{
//		m_SubBox[0]->Intersect( box );
//		m_SubBox[1]->Intersect( box );
//	}
//	else if ( box->m_SubBox[0] && box->m_SubBox[1] )
//	{
//		Intersect( box->m_SubBox[0] );
//		Intersect( box->m_SubBox[1] );
//	}
//	else
//	{
//		//==== Check Segments for Intersections ====//
//		for (  i = m_BeginInd ; i < m_EndInd ; i++ )
//		{
//			vec2d* p0 = m_ChainPtr->m_ISegDeque[i]->GetPnt( m_Surf, 0 );
//			vec2d* p1 = m_ChainPtr->m_ISegDeque[i]->GetPnt( m_Surf, 1 );
//			for ( j = box->m_BeginInd ; j < box->m_EndInd ; j++ )
//			{
//				vec2d int_pnt;
//				vec2d* p2 = box->m_ChainPtr->m_ISegDeque[j]->GetPnt( box->m_Surf, 0 );
//				vec2d* p3 = box->m_ChainPtr->m_ISegDeque[j]->GetPnt( box->m_Surf, 1 );
//				if ( seg_seg_intersect( *p0, *p1, *p2, *p3, int_pnt ) )
//				{
//					m_ChainPtr->GroupSplit( m_Surf, i, int_pnt );
//					box->m_ChainPtr->GroupSplit( box->m_Surf, j, int_pnt );
//
//					//m_ChainPtr->m_ISegDeque[i]->m_DebugFlag = true;
//					//box->m_ChainPtr->m_ISegDeque[j]->m_DebugFlag = true;
//				}
//			}
//		}
//	}
//}
//
//void ISegBox::Draw()
//{
//	if (!m_Surf)
//		return;
//
//	vec3d uw;
//	vec3d pnt;
//	glLineWidth( 1.0 );
//
//	glColor3ub( 0, 255, 0 );
//
//	glBegin( GL_LINE_LOOP );
//
//	uw = m_Box.get_pnt( 0 );
//	pnt = m_Surf->CompPnt( uw.x(), uw.y() );
//	glVertex3dv( pnt.data() );
//	uw = m_Box.get_pnt( 1 );
//	pnt = m_Surf->CompPnt( uw.x(), uw.y() );
//	glVertex3dv( pnt.data() );
//	uw = m_Box.get_pnt( 3 );
//	pnt = m_Surf->CompPnt( uw.x(), uw.y() );
//	glVertex3dv( pnt.data() );
//	uw = m_Box.get_pnt( 2 );
//	pnt = m_Surf->CompPnt( uw.x(), uw.y() );
//	glVertex3dv( pnt.data() );
//
//	glEnd();
//
//	if ( m_SubBox[0] )
//		m_SubBox[0]->Draw();
//
//	if ( m_SubBox[1] )
//		m_SubBox[1]->Draw();
//
//
//
//}
//
////////////////////////////////////////////////////////////////////////
//ISegChain::ISegChain()
//{
//	m_SurfA = m_SurfB = NULL;
//}
//
//ISegChain::~ISegChain()
//{
//
//}
//
//void ISegChain::FlipDir()
//{
//	deque< ISeg* > flipped;
//	for ( int i = (int)m_ISegDeque.size()-1 ; i >= 0 ; i-- )
//	{
//		flipped.push_back( m_ISegDeque[i] );
//	}
//	m_ISegDeque = flipped;
//
//	deque< ISeg* >::iterator s;
//	for ( s = m_ISegDeque.begin() ; s != m_ISegDeque.end(); s++ )
//	{
//		(*s)->FlipDir();
//	}
//}
//
//double ISegChain::MatchDist( ISeg* seg )
//{
//	ISeg* frontSeg = m_ISegDeque.front();
//
//	if ( frontSeg->m_SurfA != seg->m_SurfA || frontSeg->m_SurfB != seg->m_SurfB )
//		return 1.0e12;
//
//	ISeg* backSeg = m_ISegDeque.back();
//
//	double close_dist = dist( frontSeg->m_AUW[0], seg->m_AUW[0] );
//	close_dist = min( close_dist, dist( frontSeg->m_AUW[0], seg->m_AUW[1] ) );
//	close_dist = min( close_dist, dist( backSeg->m_AUW[1],  seg->m_AUW[0] ) );
//	close_dist = min( close_dist, dist( backSeg->m_AUW[1],  seg->m_AUW[1] ) );
//
//	return close_dist;
//}
//
//double ISegChain::ChainDist( ISegChain* B )
//{
//	ISeg* frontSegA = m_ISegDeque.front();
//	ISeg* frontSegB = B->m_ISegDeque.front();
//
//	if ( frontSegA->m_SurfA != frontSegB->m_SurfA || frontSegA->m_SurfB != frontSegB->m_SurfB )
//		return 1.0e12;
//
//	ISeg* backSegA = m_ISegDeque.back();
//	ISeg* backSegB = B->m_ISegDeque.back();
//
//	double close_dist = dist( frontSegA->m_AUW[0], frontSegB->m_AUW[0] );
//	close_dist = min( close_dist, dist( frontSegA->m_AUW[0], backSegB->m_AUW[1] ) );
//	close_dist = min( close_dist, dist( backSegA->m_AUW[1],  frontSegB->m_AUW[0] ) );
//	close_dist = min( close_dist, dist( backSegA->m_AUW[1],  backSegB->m_AUW[1] ) );
//
//	return close_dist;
//}
//
//
//void ISegChain::AddSeg( ISeg* seg )
//{
//	ISeg* backSeg  = m_ISegDeque.back();
//	ISeg* frontSeg = m_ISegDeque.front();
//
//	double dback0 = dist( backSeg->m_AUW[1], seg->m_AUW[0] );
//	double dback1 = dist( backSeg->m_AUW[1], seg->m_AUW[1] );
//	double dfront0 = dist( frontSeg->m_AUW[0], seg->m_AUW[0] );
//	double dfront1 = dist( frontSeg->m_AUW[0], seg->m_AUW[1] );
//
//	if ( dback0 < dback1 && dback0 < dfront0 && dback0 < dfront1 )
//	{
//		m_ISegDeque.push_back( seg );
//	}
//	else if ( dback1 < dfront0 && dback1 < dfront1 )
//	{
//		seg->FlipDir();
//		m_ISegDeque.push_back( seg );
//	}
//	else if ( dfront0 < dfront1 )
//	{
//		seg->FlipDir();
//		m_ISegDeque.push_front( seg );
//	}
//	else
//	{
//		m_ISegDeque.push_front( seg );
//	}
//}
//
//void ISegChain::AddChain( ISegChain* B )
//{
//	deque< ISeg* >::iterator s;
//	deque< ISeg* >::reverse_iterator r;
//	ISeg* backSegA  = m_ISegDeque.back();
//	ISeg* frontSegA = m_ISegDeque.front();
//	ISeg* backSegB  = B->m_ISegDeque.back();
//	ISeg* frontSegB = B->m_ISegDeque.front();
//
//	double dBackBack  = dist( backSegA->m_AUW[1], backSegB->m_AUW[1] );
//	double dBackFront = dist( backSegA->m_AUW[1], frontSegB->m_AUW[0] );
//	double dFrontBack  = dist( frontSegA->m_AUW[0], backSegB->m_AUW[1] );
//	double dFrontFront = dist( frontSegA->m_AUW[0], frontSegB->m_AUW[0] );
//
//	if ( dBackBack < dBackFront && dBackBack < dFrontBack && dBackBack < dFrontFront )
//	{
//		B->FlipDir();
//		for ( s = B->m_ISegDeque.begin() ; s != B->m_ISegDeque.end() ; s++ )
//		{
//			m_ISegDeque.push_back( (*s) );
//		}
//	}
//	else if ( dBackFront < dFrontBack && dBackFront < dFrontFront )
//	{
//		for ( s = B->m_ISegDeque.begin() ; s != B->m_ISegDeque.end() ; s++ )
//		{
//			m_ISegDeque.push_back( (*s) );
//		}
//	}
//	else if ( dFrontBack < dFrontFront )
//	{
//		for ( r = B->m_ISegDeque.rbegin() ; r != B->m_ISegDeque.rend() ; r++ )
//		{
//			m_ISegDeque.push_front( (*r) );
//		}
//	}
//	else
//	{
//		B->FlipDir();
//		for ( r = B->m_ISegDeque.rbegin() ; r != B->m_ISegDeque.rend() ; r++ )
//		{
//			m_ISegDeque.push_front( (*r) );
//		}
//	}
//}
//
//void ISegChain::BuildBoxes()
//{
//	if ( m_ISegDeque.size() <= 0 )
//		return;
//
//	m_SurfA = m_ISegDeque[0]->m_SurfA;
//	m_SurfB = m_ISegDeque[0]->m_SurfB;
//
//
//	m_ISegBoxA.m_ChainPtr = this;
//	m_ISegBoxA.m_Surf = m_ISegDeque[0]->m_SurfA;
//
//	m_ISegBoxA.m_BeginInd = 0;
//	m_ISegBoxA.m_EndInd   = m_ISegDeque.size();
//
//	m_ISegBoxA.BuildSubDivide();
//
//	m_ISegBoxB.m_ChainPtr = this;
//	m_ISegBoxB.m_Surf = m_ISegDeque[0]->m_SurfB;
//
//	m_ISegBoxB.m_BeginInd = 0;
//	m_ISegBoxB.m_EndInd   = m_ISegDeque.size();
//
//	m_ISegBoxB.BuildSubDivide();
//
//}
//
//void ISegChain::Intersect( Surf* surfPtr, ISegChain* B )
//{
//	ISegBox* box1;
//	if ( surfPtr == m_SurfA )		box1 = &m_ISegBoxA;
//	else							box1 = &m_ISegBoxB;
//
//	ISegBox* box2;
//	if ( surfPtr == B->m_SurfA )	box2 = &B->m_ISegBoxA;
//	else							box2 = &B->m_ISegBoxB;
//
//	box1->Intersect( box2 );
//
//}
//
//void ISegChain::GroupSplit( Surf* surfPtr, int index, vec2d int_pnt )
//{
//	//==== Check if chain allready has a point there =====//
//	double tol = 0.0001;
//	if ( surfPtr == m_SurfA )
//	{
//		if ( dist( int_pnt, m_ISegDeque[index]->m_AUW[0] ) < tol )
//			return;
//		if ( dist( int_pnt, m_ISegDeque[index]->m_AUW[1] ) < tol )
//			return;
//	}
//	else if ( surfPtr == m_SurfB )
//	{
//		if ( dist( int_pnt, m_ISegDeque[index]->m_BUW[0] ) < tol )
//			return;
//		if ( dist( int_pnt, m_ISegDeque[index]->m_BUW[1] ) < tol )
//			return;
//	}
//
//	m_SurfA->GetSurfMgr()->GroupSplit( surfPtr, this, index, int_pnt );
//}
//
//void ISegChain::Draw()
//{
//	glBegin( GL_LINES );
//	deque< ISeg* >::iterator s;
//	for ( s = m_ISegDeque.begin() ; s != m_ISegDeque.end(); s++ )
//	{
//		if ( (*s)->m_DebugFlag )
//			glColor3ub( 255, 255, 0 );
//		else
//			glColor3ub( 255, 0, 0 );
//
//		for ( int i = 0 ; i < 2 ; i++ )
//		{
//			vec2d uw = (*s)->m_AUW[i];
//			vec3d p = (*s)->m_SurfA->CompPnt( uw[0], uw[1] );
//			glVertex3dv( p.data() );
//		}
//	}
//	glEnd();
//
//	glColor3ub( 0, 255, 0 );
//	glPointSize(2.0);
//	glBegin( GL_POINTS );
//	for ( s = m_ISegDeque.begin() ; s != m_ISegDeque.end(); s++ )
//	{
//		for ( int i = 0 ; i < 2 ; i++ )
//		{
//			vec2d uw = (*s)->m_AUW[i];
//			vec3d p = (*s)->m_SurfA->CompPnt( uw[0], uw[1] );
//			glVertex3dv( p.data() );
//		}
//	}
//	glEnd();
//
//	ISeg* backSeg  = m_ISegDeque.back();
//	ISeg* frontSeg = m_ISegDeque.front();
//
//	glPointSize(4.0);
//	glBegin( GL_POINTS );
//		vec2d uw = frontSeg->m_AUW[0];
//		vec3d p = frontSeg->m_SurfA->CompPnt( uw[0], uw[1] );
//		glVertex3dv(p.data() );
//		uw = backSeg->m_AUW[1];
//		p = backSeg->m_SurfA->CompPnt( uw[0], uw[1] );
//		glVertex3dv(p.data() );
//
//	glEnd();
//
//	//glPointSize(5.0);
//	//glColor3ub( 0, 255, 255 );
//	//glBegin( GL_POINTS );
//	//for ( int i = 0 ; i < (int)m_SplitVec.size() ; i++ )
//	//{
//	//	glVertex3dv(m_SplitVec[i].m_Pnt.data() );
//	//}
//	//glEnd();
//
//
//
//}

//////////////////////////////////////////////////////////////////////
ICurve::ICurve()
{
	m_SCurve_A = m_SCurve_B = NULL;
}

ICurve::~ICurve()
{
}


bool ICurve::Match( SCurve* crv_A, SCurve* crv_B )
{
	double tol = 0.00001;
	vector< vec3d > control_pnts_A;
	vector< vec3d > control_pnts_B;

	crv_A->LoadControlPnts3D( control_pnts_A );
	crv_B->LoadControlPnts3D( control_pnts_B );

	if ( control_pnts_A.size() == 0 )
		return false;

	if ( control_pnts_A.size() != control_pnts_B.size() )
		return false;


	bool match = true;

	int num_pnts = (int)control_pnts_A.size();
	for ( int i = 0 ; i < num_pnts ; i++ )
	{
		double dist_forward  = dist( control_pnts_A[i], control_pnts_B[i] );
		double dist_backward = dist( control_pnts_A[i], control_pnts_B[num_pnts - i - 1] );
		if ( dist_forward > tol && dist_backward > tol )
		{
			match = false;
			break;
		}
	}

	if ( match )
	{
		double dist_front  = dist( control_pnts_A[0], control_pnts_B[0] );

		if ( dist_front > tol )
		{
			//=== Flip Direction ====//
			crv_B->FlipDir();
		}

		m_SCurve_A = crv_A;
		m_SCurve_A->SetICurve( this );
		m_SCurve_B = crv_B;
		m_SCurve_B->SetICurve( this );
	}

	return match;
}

void ICurve::BorderTesselate( )
{
	m_SCurve_A->BorderTesselate( );
	if ( !m_SCurve_B )
	{
		return;
	}

	m_SCurve_B->BorderTesselate( );
}

void ICurve::Draw()
{
	
}

void ICurve::DebugEdgeMatching(FILE* fp)
{
	if ( !m_SCurve_A || !m_SCurve_B )
	{
		fprintf( fp, "  ICurve: % Unmatched Curve \n", this );
		return;
	}

	Surf* surfA = m_SCurve_A->GetSurf();
	Surf* surfB = m_SCurve_B->GetSurf();

	fprintf( fp, "  ICurve: %d   Surf A = %d, Surf B = %d \n", this, surfA, surfB );

	double total_dist = 0.0;
	for ( int i = 0 ; i < 21 ; i++ )
	{
		double u = (double)i/20.0;
		vec3d uwpntA = m_SCurve_A->CompPntUW(u);

//	    fprintf( fp, "u = %f  uwpntA = %f %f %f \n",  u, uwpntA.x(), uwpntA.y(), uwpntA.z() ); 

		vec3d uwpntB = m_SCurve_B->CompPntUW(u);

		vec3d pntA = surfA->CompPnt( uwpntA.x(), uwpntA.y() );
		vec3d pntB = surfB->CompPnt( uwpntB.x(), uwpntB.y() );

		double d = dist( pntA, pntB );
		total_dist += d;

		//fprintf( fp, "  Dist PntA %f %f %f PntB %f %f %f = %f \n", 
		//	pntA.x(), pntA.y(), pntA.z(),  pntB.x(), pntB.y(), pntB.z(), d );

	}
	fprintf( fp, "  Total Dist PntA - PntB = %f \n", total_dist );


}

void ICurve::AddCoplanerSurface( Surf* sPtr )
{
	for ( int i = 0 ; i < (int)m_CoPlanerSurfVec.size() ; i++ )
	{
		//==== Check If Surface is Already Added ====//
		if ( sPtr == m_CoPlanerSurfVec[i] )
			return;
	}
	m_CoPlanerSurfVec.push_back( sPtr );
}
