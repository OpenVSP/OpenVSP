//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// ISegChain
//
//////////////////////////////////////////////////////////////////////

#include "ISegChain.h"
#include "CfdMeshMgr.h"

//////////////////////////////////////////////////////////////////////
//==== UW Point on Surface ====//
//////////////////////////////////////////////////////////////////////
Puw::Puw()
{
    m_Surf = NULL;
}

Puw::Puw( Surf* s, vec2d uw )
{
    m_Surf = s;
    m_UW = uw;
}

Puw::~Puw()
{
}

//////////////////////////////////////////////////////////////////////
//==== IPnt Bin ====//
//////////////////////////////////////////////////////////////////////
int IPntBin::ComputeID( vec3d & pos )
{
    int id =    ( int )( pos.x() * 10000.0 ) +
                ( int )( pos.y() * 10000.0 ) +
                ( int )( pos.z() * 10000.0 );

    return id;
}

IPnt* IPntBin::Match( IPnt* ip, map< int, IPntBin > & binMap )
{
    IPnt* close_ipnt = NULL;

    if ( ip->m_Puws.size() != 2 )
    {
        return close_ipnt;
    }

    vector< IPnt* > compareIPntVec;
    AddCompareIPnts( ip, compareIPntVec );                  // Load IPnts From This Bin

    for ( int b = 0 ; b < ( int )m_AdjBins.size() ; b++ )   // Load From Adjancent Bins
    {
        int id = m_AdjBins[b];
        binMap[id].AddCompareIPnts( ip, compareIPntVec );
    }

    //==== Find Closest IPnt ====//
    double tol = 1.0e-6 * 1.0e-6;
    double close_d = 1.0e12;

    for ( int i = 0 ; i < ( int )compareIPntVec.size() ; i++ )
    {
        if (  compareIPntVec[i]->m_Puws[0]->m_Surf == ip->m_Puws[0]->m_Surf &&
                compareIPntVec[i]->m_Puws[1]->m_Surf == ip->m_Puws[1]->m_Surf )
        {
            double d = dist_squared( ip->m_Pnt, compareIPntVec[i]->m_Pnt );
            if ( d < close_d && d < tol )
            {
                close_d = d;
                close_ipnt = compareIPntVec[i];
            }
        }
    }
    return close_ipnt;
}

void IPntBin::AddCompareIPnts( IPnt* ip, vector< IPnt* > & compareIPntVec )
{
    for ( int i = 0 ; i < ( int )m_IPnts.size() ; i++ )
    {
        if ( !m_IPnts[i]->m_UsedFlag && m_IPnts[i] != ip &&  m_IPnts[i]->m_Puws.size() == 2 )
        {
            compareIPntVec.push_back( m_IPnts[i] );
        }
    }
}



//////////////////////////////////////////////////////////////////////
//==== Shared Intersection Point ====//
//////////////////////////////////////////////////////////////////////
IPnt::IPnt()
{
    m_UsedFlag = false;
}

IPnt::IPnt( Puw* p0, Puw* p1 )
{
    m_UsedFlag = false;
    m_Puws.push_back( p0 );
    m_Puws.push_back( p1 );
}

IPnt::~IPnt()
{
}

void IPnt::CompPnt()
{
    vec3d psum;
    for ( int i = 0 ; i < ( int )m_Puws.size() ; i++ )
    {
        vec2d uw = m_Puws[i]->m_UW;
        vec3d p  = m_Puws[i]->m_Surf->CompPnt( uw[0], uw[1] );
        psum = psum + p;
    }
    if ( m_Puws.size() )
    {
        psum = psum * ( 1.0 / ( double )m_Puws.size() );
    }

    m_Pnt = psum;
}

Puw* IPnt::GetPuw( Surf* surf )
{
    for ( int i = 0 ; i < ( int )m_Puws.size() ; i++ )
    {
        if ( surf == m_Puws[i]->m_Surf )
        {
            return m_Puws[i];
        }
    }
    return NULL;
}

void IPnt::AddPuws( IPnt* ip )
{
    for ( int i = 0 ; i < ( int )ip->m_Puws.size() ; i++ )
    {
        m_Puws.push_back( ip->m_Puws[i] );
    }
}


void IPnt::AddSegRef( ISeg* seg )
{
    for ( int i = 0 ; i < ( int )m_Segs.size() ; i++ )
    {
        if ( m_Segs[i] == seg )
        {
            return;
        }
    }
    m_Segs.push_back( seg );

}
void IPnt::RemoveSegRef( ISeg* seg )
{
    deque< ISeg* > svec;
    for ( int i = 0 ; i < ( int )m_Segs.size() ; i++ )
    {
        if ( m_Segs[i] != seg )
        {
            svec.push_back( m_Segs[i] );
        }
    }
    m_Segs = svec;
}

//////////////////////////////////////////////////////////////////////
//==== IPnt Group ====//
//////////////////////////////////////////////////////////////////////
double IPntGroup::GroupDist( IPntGroup* g )
{
    vec3d p0 = m_IPntVec[0]->m_Pnt;
    vec3d p1 = g->m_IPntVec[0]->m_Pnt;

    return dist( p0, p1 );
}

// This function is un-used, but it is left here in case it may be used at a later time.
// At such time, it should be updated to use the surface density map rather than only the
// mesh density sources.
double IPntGroup::GroupDistFract( IPntGroup* g )
{
    double d = GroupDist( g );
    vec3d pnt = m_IPntVec[0]->m_Pnt;

    GridDensity* gdp = CfdMeshMgr.GetGridDensityPtr();

    double target_dist = gdp->GetTargetLen( pnt );

    return ( d / target_dist );


}


void IPntGroup::AddGroup( IPntGroup* g )
{
    for ( int i = 0 ; i < ( int )g->m_IPntVec.size() ; i++ )
    {
        m_IPntVec.push_back( g->m_IPntVec[i] );
    }
}


//////////////////////////////////////////////////////////////////////
//==== Intersection Segment ====//
//////////////////////////////////////////////////////////////////////
ISeg::ISeg()
{
    m_SurfA = m_SurfB = NULL;
    m_IPnt[0] = m_IPnt[1] = NULL;
}

ISeg::~ISeg()
{
}

ISeg::ISeg( Surf* sA, Surf* sB, IPnt* ip0, IPnt* ip1 )
{
    m_SurfA = sA;
    m_SurfB = sB;
    m_IPnt[0] = ip0;
    m_IPnt[1] = ip1;
    ip0->AddSegRef( this );
    ip1->AddSegRef( this );
}

void ISeg::Copy( ISeg & s )
{
    m_SurfA   = s.m_SurfA;
    m_SurfB   = s.m_SurfB;
    m_IPnt[0] = s.m_IPnt[0];
    m_IPnt[1] = s.m_IPnt[1];

}

bool ISeg::Match( ISeg* seg )
{
    double tol = 0.01 * 0.01;
    Puw* s1uw0 = m_IPnt[0]->GetPuw( m_SurfA );
    Puw* s1uw1 = m_IPnt[1]->GetPuw( m_SurfA );
    Puw* s2uw0 = seg->m_IPnt[0]->GetPuw( m_SurfA );
    Puw* s2uw1 = seg->m_IPnt[1]->GetPuw( m_SurfA );

    double d00 = dist_squared( s1uw0->m_UW, s2uw0->m_UW );
    double d11 = dist_squared( s1uw1->m_UW, s2uw1->m_UW );

    double d10 = dist_squared( s1uw1->m_UW, s2uw0->m_UW );
    double d01 = dist_squared( s1uw0->m_UW, s2uw1->m_UW );

    if ( d00 < tol && d11 < tol )
    {
        return true;
    }
    else if ( d10 < tol && d01 < tol )
    {
        return true;
    }

    return false;
}


void ISeg::FlipDir()
{
    IPnt* tmp = m_IPnt[0];
    m_IPnt[0] = m_IPnt[1];
    m_IPnt[1] = tmp;
}

double ISeg::MinDist( ISeg* seg )
{
    Puw* s1uw0 = m_IPnt[0]->GetPuw( m_SurfA );
    Puw* s1uw1 = m_IPnt[1]->GetPuw( m_SurfA );
    Puw* s2uw0 = seg->m_IPnt[0]->GetPuw( m_SurfA );
    Puw* s2uw1 = seg->m_IPnt[1]->GetPuw( m_SurfA );

    double min_dist = dist_squared( s1uw0->m_UW, s2uw0->m_UW );
    min_dist = min( min_dist, dist_squared( s1uw0->m_UW, s2uw1->m_UW )  );
    min_dist = min( min_dist, dist_squared( s1uw1->m_UW, s2uw0->m_UW )  );
    min_dist = min( min_dist, dist_squared( s1uw1->m_UW, s2uw1->m_UW )  );

    return min_dist;
}

double ISeg::MinDist( IPnt* ip )
{
    Puw* uw  = ip->GetPuw( m_SurfA );
    Puw* uw0 = m_IPnt[0]->GetPuw( m_SurfA );
    Puw* uw1 = m_IPnt[1]->GetPuw( m_SurfA );

    double min_dist = dist_squared( uw->m_UW, uw0->m_UW );
    min_dist = min( min_dist, dist_squared( uw->m_UW, uw1->m_UW )  );

    return min_dist;
}

void ISeg::JoinBack( ISeg* seg )
{
    double df = dist_squared( m_IPnt[1]->m_Pnt, seg->m_IPnt[0]->m_Pnt );
    double db = dist_squared( m_IPnt[1]->m_Pnt, seg->m_IPnt[1]->m_Pnt );

    if ( db < df )
    {
        seg->FlipDir();
    }
}

void ISeg::JoinFront( ISeg* seg )
{
    double df = dist_squared( m_IPnt[0]->m_Pnt, seg->m_IPnt[0]->m_Pnt );
    double db = dist_squared( m_IPnt[0]->m_Pnt, seg->m_IPnt[1]->m_Pnt );

    if ( df < db )
    {
        seg->FlipDir();
    }
}

ISeg* ISeg::Split( Surf* sPtr, vec2d & uw )
{
    vec2d uwa, uwb;
    if ( sPtr == m_SurfA )
    {
        double denom = dist( m_IPnt[0]->GetPuw( m_SurfA )->m_UW, m_IPnt[1]->GetPuw( m_SurfA )->m_UW );
        double numer = dist( m_IPnt[0]->GetPuw( m_SurfA )->m_UW, uw );
        double fract = 0.0;
        if ( denom )
        {
            fract = numer / denom;
        }
        uwa = uw;
        uwb = m_IPnt[0]->GetPuw( m_SurfB )->m_UW +
              ( m_IPnt[1]->GetPuw( m_SurfB )->m_UW - m_IPnt[0]->GetPuw( m_SurfB )->m_UW ) * fract;
    }
    else
    {
        double denom = dist( m_IPnt[0]->GetPuw( m_SurfB )->m_UW, m_IPnt[1]->GetPuw( m_SurfB )->m_UW );
        double numer = dist( m_IPnt[0]->GetPuw( m_SurfB )->m_UW, uw );
        double fract = 0.0;
        if ( denom )
        {
            fract = numer / denom;
        }
        uwb = uw;
        uwa = m_IPnt[0]->GetPuw( m_SurfA )->m_UW +
              ( m_IPnt[1]->GetPuw( m_SurfA )->m_UW - m_IPnt[0]->GetPuw( m_SurfA )->m_UW ) * fract;
    }

    if ( m_SurfA->ValidUW( uwa ) && m_SurfB->ValidUW( uwb ) )
    {
        Puw* pwa = new Puw( m_SurfA, uwa );
        CfdMeshMgr.AddDelPuw( pwa );
        Puw* pwb = new Puw( m_SurfB, uwb );
        CfdMeshMgr.AddDelPuw( pwb );
        IPnt* ip = new IPnt( pwa, pwb );
        CfdMeshMgr.AddDelIPnt( ip );
        ip->CompPnt();
        ISeg* sseg = new ISeg( m_SurfA, m_SurfB, ip, m_IPnt[1] );
        m_IPnt[1] = ip;

        return sseg;
    }

    return NULL;

}

/*
void ISeg::Draw()
{
    glBegin( GL_LINES );
      glVertex3dv( m_IPnt[0]->m_Pnt.data() );
      glVertex3dv( m_IPnt[1]->m_Pnt.data() );
    glEnd();
}
*/

////////////////////////////////////////////////////////////////////////
////==== Bound Box Surrounding ISeg Chains ====//
////////////////////////////////////////////////////////////////////////
ISegBox::~ISegBox()
{
    delete m_SubBox[0];
    delete m_SubBox[1];
}

void ISegBox::BuildSubDivide()
{
    vec2d uw0, uw1;
    for ( int i = m_BeginInd ; i < m_EndInd ; i++ )
    {
        uw0 = m_ChainPtr->m_ISegDeque[i]->m_IPnt[0]->GetPuw( m_Surf )->m_UW;
        uw1 = m_ChainPtr->m_ISegDeque[i]->m_IPnt[1]->GetPuw( m_Surf )->m_UW;
        m_Box.Update( vec3d( uw0.x(), uw0.y(), 0.0 ) );
        m_Box.Update( vec3d( uw1.x(), uw1.y(), 0.0 ) );
    }

    if ( m_EndInd - m_BeginInd <= 8 )
    {
        return;
    }

    m_SubBox[0] = new ISegBox;
    m_SubBox[1] = new ISegBox;

    m_SubBox[0]->m_ChainPtr = m_ChainPtr;
    m_SubBox[1]->m_ChainPtr = m_ChainPtr;

    m_SubBox[0]->m_Surf = m_Surf;
    m_SubBox[1]->m_Surf = m_Surf;

    int split = ( m_EndInd + m_BeginInd ) / 2;

    m_SubBox[0]->m_BeginInd = m_BeginInd;
    m_SubBox[0]->m_EndInd   = split;

    m_SubBox[1]->m_BeginInd = split;
    m_SubBox[1]->m_EndInd = m_EndInd;

    m_SubBox[0]->BuildSubDivide();
    m_SubBox[1]->BuildSubDivide();

}

void ISegBox::Intersect( ISegBox* box )
{
    int i, j;
    if ( !Compare( m_Box, box->m_Box ) )
    {
        return;
    }

    if ( m_SubBox[0] && m_SubBox[1] )
    {
        m_SubBox[0]->Intersect( box );
        m_SubBox[1]->Intersect( box );
    }
    else if ( box->m_SubBox[0] && box->m_SubBox[1] )
    {
        Intersect( box->m_SubBox[0] );
        Intersect( box->m_SubBox[1] );
    }
    else
    {
        //==== Check Segments for Intersections ====//
        for (  i = m_BeginInd ; i < m_EndInd ; i++ )
        {
            vec2d p0 = m_ChainPtr->m_ISegDeque[i]->m_IPnt[0]->GetPuw( m_Surf )->m_UW;
            vec2d p1 = m_ChainPtr->m_ISegDeque[i]->m_IPnt[1]->GetPuw( m_Surf )->m_UW;
            for ( j = box->m_BeginInd ; j < box->m_EndInd ; j++ )
            {
                vec2d int_pnt;
                vec2d p2 = box->m_ChainPtr->m_ISegDeque[j]->m_IPnt[0]->GetPuw( m_Surf )->m_UW;
                vec2d p3 = box->m_ChainPtr->m_ISegDeque[j]->m_IPnt[1]->GetPuw( m_Surf )->m_UW;
                if ( seg_seg_intersect( p0, p1, p2, p3, int_pnt ) )
                {
                    m_ChainPtr->AddSplit( m_Surf, i, int_pnt );
                    box->m_ChainPtr->AddSplit( box->m_Surf, j, int_pnt );
                }
            }
        }
    }
}




//
//void ISegBox::Draw()
//{
//  if (!m_Surf)
//      return;
//
//  vec3d uw;
//  vec3d pnt;
//  glLineWidth( 1.0 );
//
//  glColor3ub( 0, 255, 0 );
//
//  glBegin( GL_LINE_LOOP );
//
//  uw = m_Box.get_pnt( 0 );
//  pnt = m_Surf->CompPnt( uw.x(), uw.y() );
//  glVertex3dv( pnt.data() );
//  uw = m_Box.get_pnt( 1 );
//  pnt = m_Surf->CompPnt( uw.x(), uw.y() );
//  glVertex3dv( pnt.data() );
//  uw = m_Box.get_pnt( 3 );
//  pnt = m_Surf->CompPnt( uw.x(), uw.y() );
//  glVertex3dv( pnt.data() );
//  uw = m_Box.get_pnt( 2 );
//  pnt = m_Surf->CompPnt( uw.x(), uw.y() );
//  glVertex3dv( pnt.data() );
//
//  glEnd();
//
//  if ( m_SubBox[0] )
//      m_SubBox[0]->Draw();
//
//  if ( m_SubBox[1] )
//      m_SubBox[1]->Draw();
//}

//////////////////////////////////////////////////////////////////////
//==== ISeg Chain - Intersection Between Two Surfaces ====//
//////////////////////////////////////////////////////////////////////
ISegChain::ISegChain()
{
    m_SurfA = m_SurfB = NULL;
    m_BorderFlag = false;
    m_WakeAttachChain = NULL;

}

ISegChain::~ISegChain()
{
    int i;
    for ( i = 0 ; i < ( int )m_ISegDeque.size() ; i++ )
    {
        delete m_ISegDeque[i];
    }

    for ( i = 0 ; i < ( int )m_SplitVec.size() ; i++ )
    {
        delete m_SplitVec[i];
    }

    for ( i = 0 ; i < ( int )m_CreatedIPnts.size() ; i++ )
    {
        delete m_CreatedIPnts[i];
    }

}

void ISegChain::FlipDir()
{
    deque< ISeg* > flipped;
    for ( int i = ( int )m_ISegDeque.size() - 1 ; i >= 0 ; i-- )
    {
        flipped.push_back( m_ISegDeque[i] );
    }
    m_ISegDeque = flipped;

    deque< ISeg* >::iterator s;
    for ( s = m_ISegDeque.begin() ; s != m_ISegDeque.end(); s++ )
    {
        ( *s )->FlipDir();
    }
}

double ISegChain::MatchDist( ISeg* seg )
{
    ISeg* frontSeg = m_ISegDeque.front();
    ISeg* backSeg = m_ISegDeque.back();

    if ( frontSeg->m_SurfA != seg->m_SurfA || frontSeg->m_SurfB != seg->m_SurfB )
    {
        return 1.0e12;
    }

    double close_dist = seg->MinDist( frontSeg->m_IPnt[0] );
    close_dist = min( close_dist, seg->MinDist( backSeg->m_IPnt[1] ) );

    return close_dist;
}

double ISegChain::ChainDist( ISegChain* B )
{
    ISeg* frontSegA = m_ISegDeque.front();
    ISeg* frontSegB = B->m_ISegDeque.front();

    if ( frontSegA->m_SurfA != frontSegB->m_SurfA || frontSegA->m_SurfB != frontSegB->m_SurfB )
    {
        return 1.0e12;
    }

    ISeg* backSegA = m_ISegDeque.back();
    ISeg* backSegB = B->m_ISegDeque.back();

    double close_dist = frontSegA->MinDist( frontSegB );
    close_dist = min( close_dist, frontSegA->MinDist( backSegB ) );
    close_dist = min( close_dist, backSegA->MinDist( frontSegB ) );
    close_dist = min( close_dist, backSegA->MinDist( backSegB ) );

    return close_dist;
}

// Test if ISegChain B matches this ISegChain.
bool ISegChain::Match( ISegChain* B )
{
    double tol = 1e-8;

    // Check that parent surfaces are the same.
    if( m_SurfA->GetSurfID() != B->m_SurfA->GetSurfID() )
    {
        return false;
    }

    // Find 3d x,y,z coordinates of each chain's end points.
    ISeg* frontSegA = m_ISegDeque.front();
    frontSegA->m_IPnt[0]->CompPnt();
    vec3d pA0 = frontSegA->m_IPnt[0]->m_Pnt;

    ISeg* frontSegB = B->m_ISegDeque.front();
    frontSegB->m_IPnt[0]->CompPnt();
    vec3d pB0 = frontSegB->m_IPnt[0]->m_Pnt;

    ISeg* backSegA = m_ISegDeque.back();
    backSegA->m_IPnt[1]->CompPnt();
    vec3d pA1 = backSegA->m_IPnt[1]->m_Pnt;

    ISeg* backSegB = B->m_ISegDeque.back();
    backSegB->m_IPnt[1]->CompPnt();
    vec3d pB1 = backSegB->m_IPnt[1]->m_Pnt;

    // Test for matching end points.
    if( dist_squared( pA0, pB0 ) < tol && dist_squared( pA1, pB1 ) < tol )
    {
        return true;
    }

    // Check for flipped matching end points.
    if( dist_squared( pA0, pB1 ) < tol && dist_squared( pA1, pB0 ) < tol )
    {
        this->FlipDir();
        printf( "Flipping\n" );
        return true;
    }

    // No match.
    return false;
}

void ISegChain::AddSeg( ISeg* seg, bool frontFlag )
{
    if ( frontFlag )
    {
        ISeg* frontSeg = m_ISegDeque.front();
        frontSeg->JoinFront( seg );
        m_ISegDeque.push_front( seg );
    }
    else
    {
        ISeg* backSeg  = m_ISegDeque.back();
        backSeg->JoinBack( seg );
        m_ISegDeque.push_back( seg );
    }
}



void ISegChain::AddSeg( ISeg* seg )
{
    ISeg* backSeg  = m_ISegDeque.back();
    ISeg* frontSeg = m_ISegDeque.front();

    double backd  = seg->MinDist( backSeg->m_IPnt[1] );
    double frontd = seg->MinDist( frontSeg->m_IPnt[0] );

    if ( backd < frontd )
    {
        if ( !backSeg->Match( seg ) )
        {
            backSeg->JoinBack( seg );
            m_ISegDeque.push_back( seg );
        }
        else
        {
            delete seg;
        }
    }
    else
    {
        if ( !frontSeg->Match( seg ) )
        {
            frontSeg->JoinFront( seg );
            m_ISegDeque.push_front( seg );
        }
        else
        {
            delete seg;
        }
    }
}

bool ISegChain::Valid()
{
    if ( m_ISegDeque.size() < 1 )
    {
        return false;
    }

    double len = 0.0;
    for ( int i = 0 ; i < ( int )m_ISegDeque.size() ; i++ )
    {
        len += dist( m_ISegDeque[i]->m_IPnt[0]->m_Pnt, m_ISegDeque[i]->m_IPnt[1]->m_Pnt );
    }

    double tol = 1.0e-4;
    if ( len < tol )
    {
        return false;
    }

    return true;

}

void ISegChain::AddChain( ISegChain* B )
{
    deque< ISeg* >::iterator s;
    deque< ISeg* >::reverse_iterator r;
    ISeg* backSegA  = m_ISegDeque.back();
    ISeg* frontSegA = m_ISegDeque.front();
    ISeg* backSegB  = B->m_ISegDeque.back();
    ISeg* frontSegB = B->m_ISegDeque.front();

    double dBackBack   = backSegA->MinDist( backSegB );
    double dBackFront  = backSegA->MinDist( frontSegB );
    double dFrontBack  = frontSegA->MinDist( backSegB );
    double dFrontFront = frontSegA->MinDist( frontSegB );

    if ( dBackBack < dBackFront && dBackBack < dFrontBack && dBackBack < dFrontFront )
    {
        for ( r = B->m_ISegDeque.rbegin() ; r != B->m_ISegDeque.rend() ; r++ )
        {
            AddSeg( ( *r ) );
        }
    }
    else if ( dBackFront < dFrontBack && dBackFront < dFrontFront )
    {
        for ( s = B->m_ISegDeque.begin() ; s != B->m_ISegDeque.end() ; s++ )
        {
            AddSeg( ( *s ) );
        }
    }
    else if ( dFrontBack < dFrontFront )
    {
        for ( r = B->m_ISegDeque.rbegin() ; r != B->m_ISegDeque.rend() ; r++ )
        {
            AddSeg( ( *r ) );
        }
    }
    else
    {
        for ( s = B->m_ISegDeque.begin() ; s != B->m_ISegDeque.end() ; s++ )
        {
            AddSeg( ( *s ) );
        }
    }
    B->m_ISegDeque.clear();
}

void ISegChain::BuildBoxes()
{
    if ( m_ISegDeque.size() <= 0 )
    {
        return;
    }

    m_SurfA = m_ISegDeque[0]->m_SurfA;
    m_SurfB = m_ISegDeque[0]->m_SurfB;

    m_ISegBoxA.m_ChainPtr = this;
    m_ISegBoxA.m_Surf = m_SurfA;

    m_ISegBoxA.m_BeginInd = 0;
    m_ISegBoxA.m_EndInd   = m_ISegDeque.size();

    m_ISegBoxA.BuildSubDivide();

    m_ISegBoxB.m_ChainPtr = this;
    m_ISegBoxB.m_Surf = m_SurfB;

    m_ISegBoxB.m_BeginInd = 0;
    m_ISegBoxB.m_EndInd   = m_ISegDeque.size();

    m_ISegBoxB.BuildSubDivide();

}

void ISegChain::Intersect( Surf* surfPtr, ISegChain* B )
{
    ISegBox* box1;
    if ( surfPtr == m_SurfA )
    {
        box1 = &m_ISegBoxA;
    }
    else
    {
        box1 = &m_ISegBoxB;
    }

    ISegBox* box2;
    if ( surfPtr == B->m_SurfA )
    {
        box2 = &B->m_ISegBoxA;
    }
    else
    {
        box2 = &B->m_ISegBoxB;
    }

    box1->Intersect( box2 );

}

void ISegChain::AddSplit( Surf* surfPtr, int index, vec2d int_pnt )
{
    //jrg 9/27
    //==== Check if chain allready has a point there =====//
    //double tol = 0.00001;
    //if ( surfPtr == m_SurfA )
    //{
    //  if ( dist( int_pnt, m_ISegDeque[index]->m_IPnt[0]->GetPuw(m_SurfA)->m_UW ) < tol )
    //      return;
    //  if ( dist( int_pnt, m_ISegDeque[index]->m_IPnt[1]->GetPuw(m_SurfA)->m_UW ) < tol )
    //      return;
    //}
    //else if ( surfPtr == m_SurfB )
    //{
    //  if ( dist( int_pnt, m_ISegDeque[index]->m_IPnt[0]->GetPuw(m_SurfB)->m_UW ) < tol )
    //      return;
    //  if ( dist( int_pnt, m_ISegDeque[index]->m_IPnt[1]->GetPuw(m_SurfB)->m_UW ) < tol )
    //      return;
    //}


    ISegSplit* split = new ISegSplit;
    split->m_Surf  = surfPtr;
    split->m_Index = index;
    split->m_Fract = 0.0;
    split->m_UW    = int_pnt;
    m_SplitVec.push_back( split );

}
void ISegChain::AddBorderSplit( IPnt* ip, Puw* uw )
{

//  double tol = 0.000001;
    double tol = 1.0e-3;
    Surf* surfPtr = uw->m_Surf;

    int closest_index = -1;
    double closest_fract = 0.0;
    double closest_dist = 1.0e12;
    vec2d closest_proj;

    for ( int i = 0 ; i < ( int )m_ISegDeque.size() ; i++ )
    {
        //==== Find Distance From Seg ====//
        vec2d iuw0 = m_ISegDeque[i]->m_IPnt[0]->GetPuw( surfPtr )->m_UW;
        vec2d iuw1 = m_ISegDeque[i]->m_IPnt[1]->GetPuw( surfPtr )->m_UW;
        double u = proj_pnt_on_line_u( iuw0, iuw1, uw->m_UW );
        if ( u > 0.0 && u < 1.0 )
        {
            vec2d proj = iuw0 + ( iuw1 - iuw0 ) * u;

//jrg Why 3D ????
            //==== Find Distance in 3D ====//
            vec3d proj3d = surfPtr->CompPnt( proj[0], proj[1] );
            vec3d uw3d =  surfPtr->CompPnt( uw->m_UW[0], uw->m_UW[1] );

            double d = dist( uw3d, proj3d );
            if ( d < closest_dist )
            {
                closest_dist = d;
                closest_index = i;
                closest_fract = 0.0;
                vec3d p0 = surfPtr->CompPnt( iuw0[0], iuw0[1] );
                vec3d p1 = surfPtr->CompPnt( iuw1[0], iuw1[1] );
                double denom = dist( p0, p1 );
                if ( denom > 1.0e-7 )
                {
                    closest_fract = dist( p0, proj3d ) / denom;
                }
            }
        }
    }

    if ( closest_dist < tol )
    {
        ISegSplit* split = new ISegSplit;
        split->m_Surf  = surfPtr;
        split->m_Index = closest_index;
        split->m_Fract = closest_fract;
        split->m_UW    = uw->m_UW;
        m_SplitVec.push_back( split );
    }

#ifdef DEBUG_CFD_MESH
    static bool once = true;
    if ( once )
    {
        fprintf( CfdMeshMgr.m_DebugFile, "ISegChain::AddBorderSplit \n" );
        once = false;
    }
    if ( closest_dist < 0.1 )
    {
        fprintf( CfdMeshMgr.m_DebugFile, "  Closest_Dist = %12.10f UW =  %12.10f\n",
                 closest_dist, closest_uw_dist );

    }


#endif

}

//void ISegChain::AddBorderSplit( IPnt* ip, Puw* uw )
//{
//  double tol = 0.001;
//  Surf* surfPtr = uw->m_Surf;
//  bool add_flag = true;
//  int  add_index = -1;
//  for ( int i = 0 ; i < (int)m_ISegDeque.size() ; i++ )
//  {
//      vec2d iuw0 = m_ISegDeque[i]->m_IPnt[0]->GetPuw(surfPtr)->m_UW;
//      vec2d iuw1 = m_ISegDeque[i]->m_IPnt[1]->GetPuw(surfPtr)->m_UW;
//      double del_u = fabs( iuw0[0] - uw->m_UW[0] );
//      double del_w = fabs( iuw0[1] - uw->m_UW[1] );
//      if ( del_u > tol && del_w > tol )
//      {
//          add_flag = false;
//      }
//      else if ( (del_u > del_w) && ( uw->m_UW[0] >= iuw0[0] ) && ( uw->m_UW[0] <= iuw1[0] )  )
//      {
//          add_index = i;
//      }
//      else if ( (del_u > del_w) && ( uw->m_UW[0] <= iuw0[0] ) && ( uw->m_UW[0] >= iuw1[0] )  )
//      {
//          add_index = i;
//      }
//      else if ( (del_u <= del_w) && ( uw->m_UW[1] >= iuw0[1] ) && ( uw->m_UW[1] <= iuw1[1] ) )
//      {
//          add_index = i;
//      }
//      else if ( (del_u <= del_w) && ( uw->m_UW[1] <= iuw0[1] ) && ( uw->m_UW[1] >= iuw1[1] ) )
//      {
//          add_index = i;
//      }
//  }
//
//  if ( add_flag && add_index >= 0 )
//  {
//      ISegSplit* split = new ISegSplit;
//      split->m_Surf  = surfPtr;
//      split->m_Index = add_index;
//      split->m_UW    = uw->m_UW;
//      m_SplitVec.push_back( split );
//  }
//}


void ISegChain::MergeSplits()
{
    int i, j;
    //==== Compute 3D Pnts ====//
    for (  i = 0 ; i < ( int )m_SplitVec.size() ; i++ )
    {
        vec2d uw = m_SplitVec[i]->m_UW;
        m_SplitVec[i]->m_Pnt  = m_SplitVec[i]->m_Surf->CompPnt( uw[0], uw[1] );
    }

//  double tol = 0.01;
    double tol = 1.0e-6;
    vector < vector< ISegSplit* > > mergedVec;
    for (  i = 0 ; i < ( int )m_SplitVec.size() ; i++ )
    {
        bool merge_flag = false;
        for (  j = 0 ; j < ( int )mergedVec.size() ; j++ )
        {
            double d = dist( m_SplitVec[i]->m_Pnt, mergedVec[j][0]->m_Pnt );
            if ( d < tol )
            {
                merge_flag = true;
                mergedVec[j].push_back( m_SplitVec[i] );
            }
        }
        if ( !merge_flag )
        {
            vector< ISegSplit* > v;
            v.push_back( m_SplitVec[i] );
            mergedVec.push_back( v );
        }
    }

    vector< ISegSplit* > newVec;
    for (  j = 0 ; j < ( int )mergedVec.size() ; j++ )
    {
        newVec.push_back( mergedVec[j][0] );
//9/22 jrg
        //for ( k = 1 ; k < (int)mergedVec[j].size() ; k++ )            // Remove Unused Splits
        //  delete mergedVec[j][k];
    }
    m_SplitVec = newVec;
}


void ISegChain::RemoveChainEndSplits()
{
    double tol = 1.0e-7;

    Puw* frontPuwA = m_ISegDeque.front()->m_IPnt[0]->GetPuw( m_SurfA );
    Puw* frontPuwB = m_ISegDeque.front()->m_IPnt[0]->GetPuw( m_SurfB );
    Puw* backPuwA  = m_ISegDeque.back()->m_IPnt[1]->GetPuw( m_SurfA );
    Puw* backPuwB  = m_ISegDeque.back()->m_IPnt[1]->GetPuw( m_SurfB );

    //==== Check Distance ====//
    vector< ISegSplit* > newVec;
    for (  int i = 0 ; i < ( int )m_SplitVec.size() ; i++ )
    {
        ISegSplit* split = m_SplitVec[i];
        double close_dist = 1.0e12;
        if ( split->m_Surf == m_SurfA )
        {
            double dfront = dist( split->m_UW, frontPuwA->m_UW );
            if ( dfront < close_dist )
            {
                close_dist = dfront;
            }
            double dback = dist( split->m_UW, backPuwA->m_UW );
            if ( dback < close_dist )
            {
                close_dist = dback;
            }
        }
        else if ( split->m_Surf == m_SurfB )
        {
            double dfront = dist( split->m_UW, frontPuwB->m_UW );
            if ( dfront < close_dist )
            {
                close_dist = dfront;
            }
            double dback = dist( split->m_UW, backPuwB->m_UW );
            if ( dback < close_dist )
            {
                close_dist = dback;
            }
        }
        else
        {
            assert( 0 );
        }

        if ( close_dist > tol )
        {
            newVec.push_back( split );
        }

    }

    m_SplitVec = newVec;

}




bool SplitCompare( const ISegSplit* a, const ISegSplit* b )
{
    if ( a->m_Index == b->m_Index )
    {
        return ( a->m_Fract > b->m_Fract );
    }
    return ( a->m_Index > b->m_Index );
}


vector< ISegChain* > ISegChain::SortAndSplit()
{
//for ( int i = 0 ; i < (int)m_SplitVec.size() ; i++ )
//  printf("Split Index = %d %f\n", m_SplitVec[i]->m_Index,  m_SplitVec[i]->m_Fract );

    sort( m_SplitVec.begin(), m_SplitVec.end(), SplitCompare );

//for ( int i = 0 ; i < (int)m_SplitVec.size() ; i++ )
//  printf("Sort Split Index = %d %f\n", m_SplitVec[i]->m_Index,  m_SplitVec[i]->m_Fract );

    vector< ISegChain* > new_chains;

    for ( int i = 0 ; i < ( int )m_SplitVec.size() ; i++ )
    {
        ISegSplit* s = m_SplitVec[i];
        ISeg* new_seg = m_ISegDeque[s->m_Index]->Split( s->m_Surf, s->m_UW );

        if ( new_seg )
        {
            ISegChain* nc = new ISegChain();
            nc->m_SurfA = m_SurfA;
            nc->m_SurfB = m_SurfB;
            nc->m_ISegDeque.push_back( new_seg );
            for ( int j = s->m_Index + 1 ; j < ( int )m_ISegDeque.size() ; j++ )
            {
                nc->m_ISegDeque.push_back( m_ISegDeque[j] );
            }

            int num_remove = ( int )m_ISegDeque.size() - s->m_Index - 1;
            for ( int j = 0 ; j < num_remove ; j++ )
            {
                m_ISegDeque.pop_back();
            }

            new_chains.push_back( nc );
        }
    }

    return new_chains;
}

vector< ISegChain* > ISegChain::FindCoPlanarChains( Surf* sPtr, Surf* adjSurf )
{
    vector< ISegChain* > new_chains;

    vector< IPnt* > ipnt_vec;
    for ( int i = 0 ; i < ( int )m_TessVec.size() ; i++ )
    {
        IPnt* ip = m_TessVec[i];
        vec3d p = ip->m_Pnt;

        //==== See if Point Is On Surface ====//
        double tol = 1.0e-04;
        vec2d uw = sPtr->ClosestUW( p, sPtr->GetSurfCore()->GetMidU(), sPtr->GetSurfCore()->GetMidW() );

        vec3d sp = sPtr->CompPnt( uw[0], uw[1] );

        if ( dist( p, sp ) < tol )
        {
            Puw* puwa = new Puw( sPtr, vec2d( uw[0], uw[1] ) );
            CfdMeshMgr.AddDelPuw( puwa );

            Puw* puwb = new Puw( sPtr, vec2d( uw[0], uw[1] ) );
            CfdMeshMgr.AddDelPuw( puwb );

            IPnt* newip  = new IPnt( puwa, puwb );
            m_CreatedIPnts.push_back( newip );

            ipnt_vec.push_back( newip );
        }
    }

    if ( ipnt_vec.size() > 1 )
    {
        ISegChain* nc = new ISegChain();
        nc->m_SurfA = sPtr;
        nc->m_SurfB = sPtr;
        nc->m_BorderFlag = true;
        new_chains.push_back( nc );

        for ( int i = 0 ; i < ( int )ipnt_vec.size() ; i++ )
        {
            nc->m_TessVec.push_back( ipnt_vec[i] );
        }
    }

    return new_chains;
}


void ISegChain::MergeInteriorIPnts()
{
    for ( int i = 1 ; i < ( int )m_ISegDeque.size() ; i++ )
    {
        IPnt* ipe = m_ISegDeque[i - 1]->m_IPnt[1];

#ifdef DEBUG_CFD_MESH
        IPnt* ipb = m_ISegDeque[i]->m_IPnt[0];
        static double max_dist = 0.0;
        double d = dist( ipe->m_Pnt, ipb->m_Pnt );

        if ( d > max_dist )
        {
            max_dist = d;
            fprintf( CfdMeshMgr.m_DebugFile, "ISegChain::MergeInteriorIPnts Max Dist = %10.10f \n", max_dist );
        }
#endif

        m_ISegDeque[i]->m_IPnt[0] = ipe;
        ipe->AddSegRef( m_ISegDeque[i] );

//      delete ipb;
    }

}

void ISegChain::BuildCurves( )
{
    //==== A SCurve ====//
    vector< vec3d > auw_pnts;
    Puw* uw = m_ISegDeque.front()->m_IPnt[0]->GetPuw( m_SurfA );
    auw_pnts.push_back( vec3d( uw->m_UW[0], uw->m_UW[1], 0.0 ) );
    for ( int i = 0 ; i < ( int )m_ISegDeque.size() ; i++ )
    {
        uw = m_ISegDeque[i]->m_IPnt[1]->GetPuw( m_SurfA );
        auw_pnts.push_back( vec3d( uw->m_UW[0], uw->m_UW[1], 0.0 ) );
    }

    m_ACurve.SetSurf( m_SurfA );
    m_ACurve.BuildBezierCurve( auw_pnts, 0.3 );

    //==== B SCurve ====//
    vector< vec3d > buw_pnts;
    uw = m_ISegDeque.front()->m_IPnt[0]->GetPuw( m_SurfB );
    buw_pnts.push_back( vec3d( uw->m_UW[0], uw->m_UW[1], 0.0 ) );
    for ( int i = 0 ; i < ( int )m_ISegDeque.size() ; i++ )
    {
        uw = m_ISegDeque[i]->m_IPnt[1]->GetPuw( m_SurfB );
        buw_pnts.push_back( vec3d( uw->m_UW[0], uw->m_UW[1], 0.0 ) );
    }

    m_BCurve.SetSurf( m_SurfB );
    m_BCurve.BuildBezierCurve( buw_pnts, 0.3 );
}

void ISegChain::TransferTess( )
{
    vector< double > autess = m_ACurve.GetUTessPnts();
    m_BCurve.Tesselate( autess );
}

void ISegChain::ApplyTess( )
{
    //==== Clear Old Tess ====//
    m_TessVec.clear();

    vector< vec3d > tuwa = m_ACurve.GetUWTessPnts();
    vector< vec3d > tuwb = m_BCurve.GetUWTessPnts();
    assert( tuwa.size() == tuwb.size() );

    //==== Add Other IPnts ====//
    for ( int i = 0 ; i < ( int )tuwa.size() ; i++ )
    {
        Puw* puwa = new Puw( m_SurfA, vec2d( tuwa[i][0], tuwa[i][1] ) );
        CfdMeshMgr.AddDelPuw( puwa );
        Puw* puwb = new Puw( m_SurfB, vec2d( tuwb[i][0], tuwb[i][1] ) );
        CfdMeshMgr.AddDelPuw( puwb );

        IPnt* ip  = new IPnt( puwa, puwb );

        m_CreatedIPnts.push_back( ip );
        //m_CreatedPuws.push_back( puwa );
        //m_CreatedPuws.push_back( puwb );

        //vec3d pA = m_SurfA->CompPnt( tuwa[i][0], tuwa[i][1] );
        //vec3d pB = m_SurfB->CompPnt( tuwb[i][0], tuwb[i][1] );

        //double d = dist( pA, pB );
        //  printf( "%d Big D = %f \n",i, d );

        ip->CompPnt();
        m_TessVec.push_back( ip );
    }

//double d = dist( m_TessVec.front()->m_Pnt,  m_TessVec.back()->m_Pnt );
//printf("Tess Chain Size = %d %f\n", m_TessVec.size(), d );
}

void ISegChain::SpreadDensity( )
{
    m_ACurve.SpreadDensity( &m_BCurve );
}

void ISegChain::CalcDensity( GridDensity* grid_den, list< MapSource* > & splitSources )
{
    m_ACurve.CalcDensity( grid_den, &m_BCurve, splitSources );
}

void ISegChain::Tessellate()
{
    m_ACurve.Tesselate();
    m_ACurve.CleanupDistTable();
}

void ISegChain::TessEndPts()
{
    m_ACurve.TessEndPts();
}

/*
void ISegChain::Draw()
{
    //glBegin( GL_LINES );
    //deque< ISeg* >::iterator s;
    //for ( s = m_ISegDeque.begin() ; s != m_ISegDeque.end(); s++ )
    //{
    //  (*s)->Draw();
    //}
    //glEnd();

    //ISeg* backSeg  = m_ISegDeque.back();
    //ISeg* frontSeg = m_ISegDeque.front();


    //glBegin( GL_POINTS );
    //  glVertex3dv( backSeg->m_IPnt[1]->m_Pnt.data() );
    //  glVertex3dv( frontSeg->m_IPnt[0]->m_Pnt.data() );
    //glEnd();

    //glColor3ub( 255, 255, 0 );
    //m_ACurve.Draw();
    //glColor3ub( 0, 255, 255 );
    //m_BCurve.Draw();

//  glColor3ub( 255, 0, 255 );
    glLineWidth(1.0);
    glBegin( GL_LINE_STRIP );
    for ( int i = 0 ; i < (int)m_TessVec.size() ; i++ )
    {
        IPnt* ip = m_TessVec[i];
        vec2d uw = ip->GetPuw( m_SurfA )->m_UW;
        vec3d p = m_SurfA->CompPnt( uw[0], uw[1] );
        glVertex3dv( p.data() );
    }
    glEnd();
    glPointSize(4.0);
    glBegin( GL_POINTS );
    for ( int i = 0 ; i < (int)m_TessVec.size() ; i++ )
    {
        IPnt* ip = m_TessVec[i];
        vec2d uw = ip->GetPuw( m_SurfA )->m_UW;
        vec3d p = m_SurfA->CompPnt( uw[0], uw[1] );
        glVertex3dv( p.data() );
    }
    glEnd();

    //ISeg* backSeg  = m_ISegDeque.back();
    //ISeg* frontSeg = m_ISegDeque.front();

    //glPointSize(4.0);
    //glBegin( GL_POINTS );
    //  vec2d uw = frontSeg->m_AUW[0];
    //  vec3d p = frontSeg->m_SurfA->CompPnt( uw[0], uw[1] );
    //  glVertex3dv(p.data() );
    //  uw = backSeg->m_AUW[1];
    //  p = backSeg->m_SurfA->CompPnt( uw[0], uw[1] );
    //  glVertex3dv(p.data() );

    //glEnd();

    ////glPointSize(5.0);
    ////glColor3ub( 0, 255, 255 );
    ////glBegin( GL_POINTS );
    ////for ( int i = 0 ; i < (int)m_SplitVec.size() ; i++ )
    ////{
    ////    glVertex3dv(m_SplitVec[i].m_Pnt.data() );
    ////}
    ////glEnd();



}
*/
