//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// Mesh
//
//////////////////////////////////////////////////////////////////////

#define _USE_MATH_DEFINES
#include "Mesh.h"
#include "Surf.h"
#include "VspUtil.h"
#include <triangle.h>
#include <triangle_api.h>

bool LongEdgePairLengthCompare( const pair< Edge*, double >& a, const pair< Edge*, double >& b )
{
    return ( b.second < a.second );
}
bool ShortEdgePairLengthCompare( const pair< Edge*, double >& a, const pair< Edge*, double >& b )
{
    return ( a.second < b.second );
}
bool ShortEdgeTargetLengthCompare( const Edge* a, const Edge* b )
{
    return ( a->target_len < b->target_len );
}



Mesh::Mesh()
{
    m_HighlightNodeIndex = 0;
    m_HighlightEdgeIndex = 2;

    m_Surf = NULL;
    m_GridDensity = NULL;

    m_NumFixPointIter = 0;
}

Mesh::~Mesh()
{
    DumpGarbage();
    Clear();
}

void Mesh::Clear()
{
    list< Tri* >::iterator t;
    for ( t = triList.begin() ; t != triList.end(); ++t )
    {
        delete ( *t );
    }

    triList.clear();

    list< Edge* >::iterator e;
    for ( e = edgeList.begin() ; e != edgeList.end(); ++e )
    {
        delete ( *e );
    }

    edgeList.clear();

    list< Node* >::iterator n;
    for ( n = nodeList.begin() ; n != nodeList.end(); ++n )
    {
        delete ( *n );
    }

    nodeList.clear();

    m_NumFixPointIter = 0;
}

void Mesh::LimitTargetEdgeLength( Node* n )
{
    for( int i = 0; i < ( int )n->edgeVec.size(); i++ )
    {
        LimitTargetEdgeLength( n->edgeVec[i], n );
    }

    list< Edge* >::iterator e;
    list< Edge* > el( n->edgeVec.begin(), n->edgeVec.end() );
    el.sort( ShortEdgeTargetLengthCompare );

    e = el.begin();
    double limitlen = ( *e )->target_len * m_GridDensity->m_GrowRatio;
    ++e;

    for ( ; e != el.end(); ++e )
    {
        if( ( *e )->target_len > limitlen )
        {
            ( *e )->target_len = limitlen;
        }
    }
}

void Mesh::LimitTargetEdgeLength( Edge* e, Node* notn )
{
    vector< Edge* >::iterator ne;
    double growratio = m_GridDensity->m_GrowRatio;

    Node *n = e->OtherNode( notn );

    for ( ne = n->edgeVec.begin() ; ne != n->edgeVec.end(); ++ne )
    {
        double limitlen = growratio * ( *ne )->target_len;
        if( e->target_len > limitlen )
        {
            e->target_len = limitlen;
        }
    }
}

void Mesh::LimitTargetEdgeLength( Edge* e )
{
    Node *n;
    vector< Edge* >::iterator ne;
    double growratio = m_GridDensity->m_GrowRatio;

    n = e->n0;
    for ( ne = n->edgeVec.begin() ; ne != n->edgeVec.end(); ++ne )
    {
        double limitlen = growratio * ( *ne )->target_len;
        if( e->target_len > limitlen )
        {
            e->target_len = limitlen;
        }
    }

    n = e->n1;
    for ( ne = n->edgeVec.begin() ; ne != n->edgeVec.end(); ++ne )
    {
        double limitlen = growratio * ( *ne )->target_len;
        if( e->target_len > limitlen )
        {
            e->target_len = limitlen;
        }
    }
}

void Mesh::LimitTargetEdgeLength()
{
    Node *n;
    list< Edge* >::iterator e;
    vector< Edge* >::iterator ne;
    double growratio = m_GridDensity->m_GrowRatio;
    double limitlen;

    edgeList.sort( ShortEdgeTargetLengthCompare );

    for ( e = edgeList.begin() ; e != edgeList.end(); ++e )
    {
        limitlen = growratio * ( *e )->target_len;

        n = ( *e )->n0;
        for ( ne = n->edgeVec.begin() ; ne != n->edgeVec.end(); ++ne )
        {
            if ( !( *ne )->border )
            {
                if( ( *ne )->target_len > limitlen )
                {
                    ( *ne )->target_len = limitlen;
                }
            }
        }

        n = ( *e )->n1;
        for ( ne = n->edgeVec.begin() ; ne != n->edgeVec.end(); ++ne )
        {
            if ( !( *ne )->border )
            {
                if( ( *ne )->target_len > limitlen )
                {
                    ( *ne )->target_len = limitlen;
                }
            }
        }
    }
}


void Mesh::Remesh()
{
    int num_split = 1;
    int num_collapse = 1;

    //==== Find Target Edge Lengths ====//
    list< Edge* >::iterator e;
    for ( e = edgeList.begin() ; e != edgeList.end(); ++e )
    {
        ( *e )->ComputeLength();
        ComputeTargetEdgeLength( *e );
    }

    LimitTargetEdgeLength();

    for ( int i = 0 ; i < 20 ; i++ )
    {
        if ( num_split )
        {
            num_split = Split( 1 );
        }
        if ( num_collapse )
        {
            num_collapse = Collapse( 1 );
        }
    }

    for ( e = edgeList.begin() ; e != edgeList.end() ; ++e )
    {
        if ( !( *e )->border )
        {
            SwapEdge( *e );
        }
    }
//printf("Smooth\n");
    LaplacianSmooth( 2 );

    //ColorTris();

}

void Mesh::LoadSimpTris()
{
    list< Tri* >::iterator t;
    simpTriVec.resize( triList.size() );
    simpPntVec.resize( triList.size() * 3 );
    simpUWPntVec.resize( triList.size() * 3 );

    int cnt = 0;
    for ( t = triList.begin() ; t != triList.end(); ++t )
    {
        simpTriVec[cnt].ind0 = cnt * 3;
        simpTriVec[cnt].ind1 = cnt * 3 + 1;
        simpTriVec[cnt].ind2 = cnt * 3 + 2;

        simpPntVec[cnt * 3]   = ( *t )->n0->pnt;
        simpPntVec[cnt * 3 + 1] = ( *t )->n1->pnt;
        simpPntVec[cnt * 3 + 2] = ( *t )->n2->pnt;

        simpUWPntVec[cnt * 3] = ( *t )->n0->uw;
        simpUWPntVec[cnt * 3 + 1] = ( *t )->n1->uw;
        simpUWPntVec[cnt * 3 + 2] = ( *t )->n2->uw;
        cnt++;
    }
}

void Mesh::CondenseSimpTris()
{
    //==== Map Coincedent Point ====//
    vector< int > reMap;
    map< int, vector< int > > indMap;
    for ( int i = 0 ; i < ( int )simpTriVec.size() ; i++ )
    {
        reMap.push_back( CheckDupOrAdd( simpTriVec[i].ind0, indMap, simpPntVec ) );
        reMap.push_back( CheckDupOrAdd( simpTriVec[i].ind1, indMap, simpPntVec ) );
        reMap.push_back( CheckDupOrAdd( simpTriVec[i].ind2, indMap, simpPntVec ) );
    }

    //==== Reduce Point and UW Vec ====//
    vector< vec3d > rePntVec;
    vector < vec2d > reUWVec;
    for ( int i = 0 ; i < ( int )reMap.size() ; i++ )
    {
        if ( i == reMap[i] )
        {
            rePntVec.push_back( simpPntVec[reMap[i]] );
            reUWVec.push_back( simpUWPntVec[reMap[i]] );
            reMap[i] = rePntVec.size() - 1;
        }
        else
        {
            int im = reMap[i];
            reMap[i] = reMap[ im ];
        }
    }

    simpPntVec = rePntVec;
    simpUWPntVec = reUWVec;
    for ( int i = 0 ; i < ( int )simpTriVec.size() ; i++ )
    {
        simpTriVec[i].ind0 = reMap[3 * i];
        simpTriVec[i].ind1 = reMap[3 * i + 1];
        simpTriVec[i].ind2 = reMap[3 * i + 2];
    }
}

void Mesh::StretchSimpPnts( double start_x, double end_x, double scale, double angle )
{
    double factor = scale - 1.0;
    for ( int i = 0 ; i < ( int )simpPntVec.size() ; i++ )
    {
        double x = simpPntVec[i].x();
        double z = simpPntVec[i].z();
        if ( x > start_x )
        {
            double numer = x - start_x;
            double fract = numer / ( end_x - start_x );
            double xx = start_x + numer * ( 1.0 + factor * fract * fract );
            double zz = z + ( xx - x ) * tan( DEG2RAD( angle ) );

            simpPntVec[i].set_x( xx );
            simpPntVec[i].set_z( zz );
        }
    }

}

int Mesh::CheckDupOrAdd( int ind, map< int, vector< int > > & indMap, vector< vec3d > & pntVec )
{
    double tol = 1.0e-8;
    int combind = ( int )( ( pntVec[ind].x() + pntVec[ind].y() + pntVec[ind].z() ) * 10000.0 );
    map<int, vector<int> >::const_iterator iter;
    iter = indMap.find( combind );
    if ( iter == indMap.end() ) // Add Index
    {
        indMap[combind].push_back( ind );
        return ind;
    }
    else
    {
        for ( int i = 0 ; i < ( int )iter->second.size() ; i++ )
        {
            int testind = iter->second[i];

            if ( std::abs( pntVec[ind].x() - pntVec[testind].x() ) < tol  &&
                    std::abs( pntVec[ind].y() - pntVec[testind].y() ) < tol  &&
                    std::abs( pntVec[ind].z() - pntVec[testind].z() ) < tol  )
            {
                return testind;
            }
        }
        indMap[combind].push_back( ind );
    }
    return ind;
}


int Mesh::Split( int num_iter )
{
    int num_long_edges = 0;
    list< Edge* >::iterator e;
    for ( int iter = 0 ; iter < num_iter ; iter++ )
    {
        //===== Split ====//
        vector < pair < Edge*, double > > longEdges;
        longEdges.reserve( edgeList.size() );
        for ( e = edgeList.begin() ; e != edgeList.end(); ++e )
        {
            if ( !( *e )->border )
            {
                double rat = ( *e )->GetLength() / ( *e )->target_len;
                if ( rat > 1.41 )
                {
                    longEdges.emplace_back( pair< Edge*, double >( ( *e ), rat ) );
                }
            }
        }

        //==== Sort Matches By Length ====//
        sort( longEdges.begin(), longEdges.end(), LongEdgePairLengthCompare );

        int num_split = longEdges.size() / 10;
        num_split = min( num_split, ( int )longEdges.size() );

        for ( int i = 0 ; i < num_split ; i++ )
        {
            longEdges[i].first->ComputeLength();
            SplitEdge( longEdges[i].first );
        }

        //==== Swap All Changed Edges If Needed ====//
        //for ( e = edgeList.begin() ; e != edgeList.end(); e++ )
        //{
        //  ComputeTargetEdgeLength(*e);
        //}

        num_long_edges = longEdges.size();
    }
    DumpGarbage();

    return num_long_edges;

}

int Mesh::Collapse( int num_iter )
{
    int num_short_edges = 0;
    for ( int iter = 0 ; iter < num_iter ; iter++ )
    {
        list< Edge* >::iterator e;

        //==== Collapse =====//
        vector < pair < Edge*, double > > shortEdges;
        shortEdges.reserve( edgeList.size() );
        for ( e = edgeList.begin() ; e != edgeList.end(); ++e )
        {
            if ( ValidCollapse( *e ) )
            {
                double rat = ( *e )->GetLength() / ( *e )->target_len;
                if ( rat < 0.707 )
                {
                    shortEdges.emplace_back( pair< Edge*, double >( ( *e ), rat ) );
                }
            }
        }

        //==== Sort Matches By Length ====//
        sort( shortEdges.begin(), shortEdges.end(), ShortEdgePairLengthCompare );

        int num_colapse = shortEdges.size() / 10;
        num_colapse = min( num_colapse, ( int )shortEdges.size() );

        num_short_edges = 0;
        for ( int i = 0 ; i < num_colapse ; i++ )
        {
            shortEdges[i].first->ComputeLength();
//          printf("  Collapse %f \n", dist );
            if ( ValidCollapse( shortEdges[i].first ) && !shortEdges[i].first->m_DeleteMeFlag )
            {
                num_short_edges++;
                CollapseEdge( shortEdges[i].first );
            }
        }

        ////==== Swap All Changed Edges If Needed ====//
        //for ( e = edgeList.begin() ; e != edgeList.end(); e++ )
        //{
        //      ComputeTargetEdgeLength(*e);
        //}
    }

    DumpGarbage();

    return num_short_edges;

}

int Mesh::RemoveRevTris()
{
    int badcount = 0;

    vector < Edge* > remEdges;

    list< Tri* >::iterator t;
    for ( t = triList.begin() ; t != triList.end(); ++t )
    {
        vec3d ntri = (*t)->Normal();
        vec2d avg_uw = ( (*t)->n0->uw + (*t)->n1->uw + (*t)->n2->uw ) * ( 1.0 / 3.0 );
        vec3d nsurf = m_Surf->GetSurfCore()->CompNorm( avg_uw[0], avg_uw[1] );

        double dprod = dot ( ntri, nsurf );

        if ( m_Surf->GetFlipFlag() )
        {
            dprod = -dprod;
        }

        if ( dprod < 0.0 )
        {
            Edge* e = (*t)->FindLongEdge();

            remEdges.push_back( e );

            badcount++;
        }
    }

    for ( int i = 0; i < remEdges.size(); i++ )
    {
        Edge* e = remEdges[i];

        if ( e )
        {
            if ( ValidCollapse( e ) )
            {
                CollapseEdge( e );
            }
        }

    }

    return badcount;
}

void Mesh::ColorTris()
{
    list< Tri* >::iterator t;
    for ( t = triList.begin() ; t != triList.end(); ++t )
    {
        double q = ( *t )->ComputeQual();

        if ( q > M_PI / 6.0 )                                           // > 30 Deg
        {
            ( *t )->rgb[0] = ( *t )->rgb[1] = ( *t )->rgb[2] = 255;
        }
        else if ( q > M_PI / 7.0 )
        {
            ( *t )->rgb[2] = 255;    // 25 deg
            ( *t )->rgb[0] = ( *t )->rgb[1] = 100;
        }
        else
        {
            ( *t )->rgb[0] = 255;
            ( *t )->rgb[1] = ( *t )->rgb[2] = 100;
        }
    }

//  printf( "Num Tris = %d \n", (int)triList.size() );
}

Node* Mesh::AddNode( vec3d p, vec2d uw_in )
{
    Node* nptr = new Node( p, uw_in );
    nodeList.push_back( nptr );
    nptr->list_ptr = --nodeList.end();
    return nptr;
}

void Mesh::RemoveNode( Node* nptr )
{
    garbageNodeVec.push_back( nptr );
    nodeList.erase( nptr->list_ptr );

    nptr->m_DeleteMeFlag = true;
}

Node* Mesh::FindNode( const vec3d& p )
{
    list< Node* >::iterator n;
    for ( n = nodeList.begin() ; n != nodeList.end(); ++n )
    {
        if ( !( *n )->m_DeleteMeFlag && dist_squared( ( *n )->pnt, p ) < 1.0e-7 )
        {
            return ( *n );
        }
    }
    return NULL;
}

Edge* Mesh::AddEdge( Node* n0, Node* n1 )
{
    Edge* eptr = new Edge( n0, n1 );

    edgeList.push_back( eptr );
    eptr->list_ptr = --edgeList.end();

    n0->AddConnectEdge( eptr );
    n1->AddConnectEdge( eptr );

    eptr->ComputeLength();

    return eptr;
}

void Mesh::RemoveEdge( Edge* eptr )
{
    eptr->n0->RemoveConnectEdge( eptr );
    eptr->n1->RemoveConnectEdge( eptr );

    garbageEdgeVec.push_back( eptr );
    edgeList.erase( eptr->list_ptr );

    eptr->m_DeleteMeFlag = true;
}

Edge* Mesh::FindEdge( Node* n0, Node* n1 )
{
    list< Edge* >::iterator e;
    for ( e = edgeList.begin() ; e != edgeList.end(); ++e )
    {
        if ( !( *e )->m_DeleteMeFlag )
        {
            if ( ( *e )->n0 == n0 && ( *e )->n1 == n1 )
            {
                return ( *e );
            }
            if ( ( *e )->n0 == n1 && ( *e )->n1 == n0 )
            {
                return ( *e );
            }
        }
    }
    return NULL;
}

Tri* Mesh::AddTri( Node* n0, Node* n1, Node* n2, Edge* e0, Edge* e1, Edge* e2 )
{
    Tri* tptr = new Tri( n0, n1, n2, e0, e1, e2 );
    triList.push_back( tptr );
    tptr->list_ptr = --triList.end();
    return tptr;
}

void Mesh::RemoveTri( Tri* tptr )
{
    garbageTriVec.push_back( tptr );
    triList.erase( tptr->list_ptr );
    tptr->m_DeleteMeFlag = true;
}

void Mesh::DumpGarbage()
{
    //==== Delete Flagged Nodes =====//
    for ( int i = 0 ; i < ( int )garbageNodeVec.size() ; i++ )
    {
        delete garbageNodeVec[i];
    }
    garbageNodeVec.clear();

    //==== Delete Flagged Edges =====//
    for ( int i = 0 ; i < ( int )garbageEdgeVec.size() ; i++ )
    {
        delete garbageEdgeVec[i];
    }
    garbageEdgeVec.clear();

    //==== Delete Flagged Tris =====//
    for ( int i = 0 ; i < ( int )garbageTriVec.size() ; i++ )
    {
        delete garbageTriVec[i];
    }
    garbageTriVec.clear();
}

void Mesh::SetNodeFlags()
{
    list< Node* >::iterator n;
    for ( n = nodeList.begin() ; n != nodeList.end(); ++n )
    {
        ( *n )->fixed = false;
    }

    list< Edge* >::iterator e;
    for ( e = edgeList.begin() ; e != edgeList.end(); ++e )
    {
        if ( ( *e )->border || ( *e )->ridge )
        {
            ( *e )->n0->fixed = true;
            ( *e )->n1->fixed = true;
        }
    }
}

void Mesh::SplitEdge( Edge* edge )
{
    assert( m_Surf );

    if ( edge->border )                     // Dont Split Borders
    {
        return;
    }

    assert( edge->t0 || edge->t1 );

    Tri*  ta = edge->t0;
    Tri*  tb = edge->t1;

    Node* n0 = edge->n0;
    Node* n1 = edge->n1;
    if ( ta && !ta->CorrectOrder( n0, n1 ) )
    {
        n0 = edge->n1;
        n1 = edge->n0;
    }
    else if ( !ta && tb && !tb->CorrectOrder( n0, n1 ) )
    {
        n0 = edge->n1;
        n1 = edge->n0;

    }
    //else if ( tb && !tb->CorrectOrder( n0, n1 ) )
    //{
    //  n0 = edge->n1;
    //  n1 = edge->n0;
    //}

    vec3d psplit  = ( n0->pnt + n1->pnt ) * 0.5; // Split
    vec2d uwsplit = ( n0->uw  + n1->uw ) * 0.5;

    vec2d uws = m_Surf->ClosestUW( psplit, uwsplit[0], uwsplit[1] );
    vec3d ps  = m_Surf->CompPnt( uws.x(), uws.y() );

    Node* ns  = AddNode( ps, uws );
    Edge* es0 = AddEdge( n0, ns );
    Edge* es1 = AddEdge( ns, n1 );
    es0->ridge = edge->ridge;
    es1->ridge = edge->ridge;

    if ( ta )
    {
        Node* na = ta->OtherNode( n0, n1 );
        Edge* ea = AddEdge( na, ns );

        Edge* ea0 = ta->FindEdge( n0, na );
        Edge* ea1 = ta->FindEdge( na, n1 );

        Tri* ta0 = AddTri( n0, ns, na, ea0, ea, es0 );
        Tri* ta1 = AddTri( n1, na, ns, ea1, es1, ea );

        ea->t0 = ta0;
        ea->t1 = ta1;

        if ( ea0->t0 == ta )
        {
            ea0->t0 = ta0;
        }
        else if ( ea0->t1 == ta )
        {
            ea0->t1 = ta0;
        }
        else
        {
            assert( 0 );
        }

        if ( ea1->t0 == ta )
        {
            ea1->t0 = ta1;
        }
        else if ( ea1->t1 == ta )
        {
            ea1->t1 = ta1;
        }
        else
        {
            assert( 0 );
        }

        es0->t0 = ta0;
        es1->t0 = ta1;

        RemoveTri( ta );
    }

    if ( tb )
    {
        Node* nb = tb->OtherNode( n0, n1 );
        Edge* eb = AddEdge( ns, nb );

        Edge* eb0 = tb->FindEdge( n0, nb );
        Edge* eb1 = tb->FindEdge( nb, n1 );

        Tri* tb0 = AddTri( n0, nb, ns, es0, eb, eb0 );
        Tri* tb1 = AddTri( n1, ns, nb, es1, eb1, eb );

        eb->t0 = tb0;
        eb->t1 = tb1;

        if ( eb0->t0 == tb )
        {
            eb0->t0 = tb0;
        }
        else if ( eb0->t1 == tb )
        {
            eb0->t1 = tb0;
        }
        else
        {
            assert( 0 );
        }

        if ( eb1->t0 == tb )
        {
            eb1->t0 = tb1;
        }
        else if ( eb1->t1 == tb )
        {
            eb1->t1 = tb1;
        }
        else
        {
            assert( 0 );
        }

        es0->t1 = tb0;
        es1->t1 = tb1;

        RemoveTri( tb );
    }

    RemoveEdge( edge );

    ComputeTargetEdgeLength( ns );
    LimitTargetEdgeLength( ns );
}

void Mesh::SwapEdge( Edge* edge )
{
    //if ( edge->n0->fixed && edge->n1->fixed )
    //  return;
    if ( edge->border )
    {
        return;
    }

    Tri*  ta = edge->t0;
    Tri*  tb = edge->t1;

    if ( !ta || !tb )
    {
        return;
    }

    if ( ThreeEdgesThreeTris( edge ) )
    {
        return;
    }

    Node* n0 = edge->n0;
    Node* n1 = edge->n1;


    if ( !ta->CorrectOrder( n0, n1 ) )
    {
        n0 = edge->n1;
        n1 = edge->n0;
    }

    Node* na = ta->OtherNode( n0, n1 );
    Node* nb = tb->OtherNode( n0, n1 );

    assert( na != nb );

    //==== Determine Tri Quality of Existing Tris =====//
    double qa = ta->ComputeQual();
    double qb = tb->ComputeQual();
    double qc = Tri::ComputeQual( n0, nb, na );
    double qd = Tri::ComputeQual( n1, na, nb );

    if ( min( qc, qd ) <= min( qa, qb ) )
    {
        return;
    }

    vec3d norma = ta->Normal();
    vec3d normb = tb->Normal();
    vec3d normc = Tri::Normal( n0, nb, na );
    vec3d normd = Tri::Normal( n1, na, nb );

    double angab = angle( norma, normb );

    if ( angab > 0.25 * M_PI_4  )
    {
        return;
    }

    double angcd = angle( normc, normd );

    if ( angcd > 0.25 * M_PI_4  )
    {
        return;
    }

    double angac = angle( norma, normc );

    if ( angac > 0.25 * M_PI_4 )
    {
        return;
    }

    Edge* ea0 = ta->FindEdge( n0, na );
    Edge* ea1 = ta->FindEdge( na, n1 );
    Edge* eb0 = tb->FindEdge( n0, nb );
    Edge* eb1 = tb->FindEdge( nb, n1 );

    edge->n0 = na;
    edge->n1 = nb;
    edge->ComputeLength();
    ComputeTargetEdgeLength( edge );

    na->AddConnectEdge( edge );
    nb->AddConnectEdge( edge );
    n0->RemoveConnectEdge( edge );
    n1->RemoveConnectEdge( edge );

    ta->SetNodesEdges( n0, nb, na, ea0, edge, eb0 );
    tb->SetNodesEdges( n1, na, nb, eb1, edge, ea1 );

    if ( ea1->t0 == ta )
    {
        ea1->t0 = tb;
    }
    else if ( ea1->t1 == ta )
    {
        ea1->t1 = tb;
    }
    else
    {
        assert( 0 );
    }

    if ( eb0->t0 == tb )
    {
        eb0->t0 = ta;
    }
    else if ( eb0->t1 == tb )
    {
        eb0->t1 = ta;
    }
    else
    {
        assert( 0 );
    }

    LimitTargetEdgeLength( edge );

//CheckValidAllEdges();
}

bool Mesh::ThreeEdgesThreeTris( Edge* edge )
{
    Node* n0 = edge->n0;
    Node* n1 = edge->n1;

    vector< Tri* > tvec0;
    n0->GetConnectTris( tvec0 );
    if ( tvec0.size() == 3 && n0->edgeVec.size() == 3 )
    {
        return true;
    }

    vector< Tri* > tvec1;
    n1->GetConnectTris( tvec1 );

    return tvec1.size() == 3 && n1->edgeVec.size() == 3;
}


bool Mesh::ValidCollapse( Edge* edge )
{
    if ( edge->m_DeleteMeFlag )
    {
        return false;
    }

    if ( edge->border || edge->ridge )
    {
        return false;
    }

    //////if ( edge->n0->fixed || edge->n1->fixed )
    //////  return false;
    if ( edge->n0->fixed && edge->n1->fixed )
    {
        return false;
    }

    if ( !edge->t0 || !edge->t1 )
    {
        return false;
    }

    if ( edge->t0->m_DeleteMeFlag || edge->t1->m_DeleteMeFlag )
    {
        return false;
    }

    Node* n0 = edge->n0;
    Node* n1 = edge->n1;
    Tri*  ta = edge->t0;
    Tri*  tb = edge->t1;
    Node* na = ta->OtherNode( n0, n1 );
    Node* nb = tb->OtherNode( n0, n1 );

    //==== Check 3 Tris in a Tri Case =====//
    Edge* e0a = ta->FindEdge( n0, na );
    Edge* e1a = ta->FindEdge( n1, na );

    if ( !e0a || ! e1a )
    {
        return false;
    }

    Tri* ta0 = e0a->OtherTri( ta );
    Tri* ta1 = e1a->OtherTri( ta );

    if ( ta0 && ta1 )
    {
        Node* na0 = ta0->OtherNode( n0, na );
        Node* na1 = ta1->OtherNode( n1, na );

        if ( na0 == na1 )
        {
            return false;
        }
    }

    Edge* e0b = tb->FindEdge( n0, nb );
    Edge* e1b = tb->FindEdge( n1, nb );

    Tri* tb0 = e0b->OtherTri( tb );
    Tri* tb1 = e1b->OtherTri( tb );

    if ( tb0 && tb1 )
    {
        Node* nb0 = tb0->OtherNode( n0, nb );
        Node* nb1 = tb1->OtherNode( n1, nb );

        if ( nb0 == nb1 )
        {
            return false;
        }
    }


    //if ( na->edgeVec.size() <= 3 )            // 3-Division Config
    //  return false;
    //if ( nb->edgeVec.size() <= 3 )            // 3-Division Config
    //  return false;

    return true;
}

bool Mesh::ValidNodeMove( Node* nptr, const vec3d & move_to, Tri* ignoreTri )
{
    int i;
    bool valid_flag = true;
    vector < Tri* > triVec;
    nptr->GetConnectTris( triVec );

    vector < vec3d > normals;
    for ( i = 0 ; i < ( int )triVec.size() ; i++ )
    {
        if ( triVec[i] != ignoreTri )
        {
            normals.push_back( triVec[i]->Normal() );
        }
    }

    vec3d save_pos = nptr->pnt;
    nptr->pnt = move_to;

    vector < vec3d > move_normals;
    for ( i = 0 ; i < ( int )triVec.size() ; i++ )
    {
        if ( triVec[i] != ignoreTri )
        {
            move_normals.push_back( triVec[i]->Normal() );
        }
    }

    for ( i = 0 ; i < ( int )normals.size() ; i++ )
    {
        if ( angle( normals[i], move_normals[i] ) >= 0.5 * M_PI_4 )
        {
            valid_flag = false;
            break;
        }
    }

    nptr->pnt = save_pos;


    return valid_flag;
}

void Mesh::CollapseHighlightEdge()
{
    Edge* hedge = NULL;
    int cnt = 0;
    list< Edge* >::iterator e;
    for ( e = edgeList.begin() ; e != edgeList.end(); ++e )
    {
        if ( cnt == m_HighlightEdgeIndex )
        {
            hedge = ( *e );
        }
        cnt++;
    }

    if ( hedge && ValidCollapse( hedge ) )
    {
        CollapseEdge( hedge );
        DumpGarbage();
    }


}

void Mesh::CollapseEdge( Edge* edge )
{
    Node* n0 = edge->n0;
    Node* n1 = edge->n1;

    Tri*  ta = edge->t0;
    Tri*  tb = edge->t1;
    Node* na = ta->OtherNode( n0, n1 );
    Node* nb = tb->OtherNode( n0, n1 );

    assert( na != nb );

    Edge* ea0 = ta->FindEdge( na, n0 );
    Edge* ea1 = ta->FindEdge( na, n1 );
    Edge* eb0 = tb->FindEdge( nb, n0 );
    Edge* eb1 = tb->FindEdge( nb, n1 );
    Tri*  ta0 = ea0->OtherTri( ta );
    Tri*  ta1 = ea1->OtherTri( ta );
    Tri*  tb0 = eb0->OtherTri( tb );
    Tri*  tb1 = eb1->OtherTri( tb );


    if ( ta0 && ta1 )
    {
        Node* other_ta0 = ta0->OtherNode( na, n0 );
        Node* other_ta1 = ta1->OtherNode( na, n1 );
        assert ( other_ta0 != other_ta1 );
    }

    if ( tb0 && tb1 )
    {
        Node* other_tb0 = tb0->OtherNode( nb, n0 );
        Node* other_tb1 = tb1->OtherNode( nb, n1 );
        assert ( other_tb0 != other_tb1 );
    }




    vec3d pc;
    vec2d uwc;
    if ( n0->fixed )
    {
        pc = n0->pnt;
        uwc = n0->uw;
    }
    else if ( n1->fixed )
    {
        pc = n1->pnt;
        uwc = n1->uw;
    }
    else
    {
//      pc  = (n0->pnt + n1->pnt)*0.5;
        uwc = ( n0->uw + n1->uw ) * 0.5;
        pc  = m_Surf->CompPnt( uwc.x(), uwc.y() );
    }

    if ( !ValidNodeMove( n0, pc, ta ) )
    {
        return;
    }
    if ( !ValidNodeMove( n1, pc, tb ) )
    {
        return;
    }

    Node* nc  = AddNode( pc, uwc );
    if ( n0->fixed || n1->fixed )
    {
        nc->fixed = true;
    }

    Edge* eca = AddEdge( na, nc );
    Edge* ecb = AddEdge( nb, nc );

    if ( ea0->border || ea1->border )
    {
        eca->border = true;
    }
    if ( eb0->border || eb1->border )
    {
        ecb->border = true;
    }
    if ( ea0->ridge  || ea1->ridge )
    {
        eca->ridge = true;
    }
    if ( eb0->ridge  || eb1->ridge )
    {
        ecb->ridge = true;
    }

//jrg Check for invalid tris and improved qual

    eca->t0 = ta0;
    eca->t1 = ta1;
    ecb->t0 = tb0;
    ecb->t1 = tb1;

    if ( ta0 )
    {
        ta0->ReplaceEdge( ea0, eca );
    }
    if ( ta1 )
    {
        ta1->ReplaceEdge( ea1, eca );
    }
    if ( tb0 )
    {
        tb0->ReplaceEdge( eb0, ecb );
    }
    if ( tb1 )
    {
        tb1->ReplaceEdge( eb1, ecb );
    }


//CheckValidEdge(eca);
//CheckValidEdge(ecb);

    //==== Change Any Tris That Point to n0 ====//
    vector< Tri* > tVec;
    n0->GetConnectTris( tVec );
    for ( int i = 0 ; i < ( int )tVec.size() ; i++ )
    {
        tVec[i]->ReplaceNode( n0, nc );
    }
    n1->GetConnectTris( tVec );
    for ( int i = 0 ; i < ( int )tVec.size() ; i++ )
    {
        tVec[i]->ReplaceNode( n1, nc );
    }

    //==== Change Edges That Point To n0 ====//
    for ( int i = 0 ; i < ( int )n0->edgeVec.size() ; i++ )
    {
        Edge* e = n0->edgeVec[i];
        if ( e != edge && e != ea0 && e != ea1 && e != eb0 && e != eb1 )
        {
            e->ReplaceNode( n0, nc );
//CheckValidEdge(e);
            nc->AddConnectEdge( e );
            CheckValidEdge( e );
        }
    }
    //==== Change Edges That Point To n1 ====//
    for ( int i = 0 ; i < ( int )n1->edgeVec.size() ; i++ )
    {
        Edge* e = n1->edgeVec[i];
        if ( e != edge && e != ea0 && e != ea1 && e != eb0 && e != eb1 )
        {
            e->ReplaceNode( n1, nc );
//CheckValidEdge(e);
            nc->AddConnectEdge( e );
            CheckValidEdge( e );
        }
    }
//if ( ecb->t0 )
//{
//assert( ecb->t0->Contains( ecb ) );
//assert( ecb->t0->Contains( ecb->n0, ecb->n1 ) );
//}

    RemoveEdge( edge );
    RemoveNode( n0 );
    RemoveNode( n1 );
    RemoveTri( ta );
    RemoveTri( tb );
    RemoveEdge( ea0 );
    RemoveEdge( ea1 );
    RemoveEdge( eb0 );
    RemoveEdge( eb1 );

    ComputeTargetEdgeLength( nc );
    LimitTargetEdgeLength( nc );

//CheckValidAllEdges( );

}

void Mesh::LaplacianSmooth( int num_iter )
{
    for ( int i = 0 ; i < num_iter ; i++ )
    {
        list< Node* >::iterator n;
        for ( n = nodeList.begin() ; n != nodeList.end(); ++n )
        {
            if ( !( *n )->m_DeleteMeFlag && !( *n )->fixed )
            {
                ////(*n)->LaplacianSmoothUW();
                ////(*n)->pnt = m_Surf->CompPnt((*n)->uw.x(), (*n)->uw.y());
                //(*n)->LaplacianSmooth();
                //vec2d uw = m_Surf->ClosestUW( (*n)->pnt, (*n)->uw.x(), (*n)->uw.y(), 0.001, 0.001 );
                //(*n)->pnt = m_Surf->CompPnt( uw.x(), uw.y());
                //(*n)->uw = uw;
//              (*n)->LaplacianSmooth( m_Surf );
                ( *n )->AreaWeightedLaplacianSmooth( m_Surf );
            }
        }
    }
}

void Mesh::OptSmooth( int num_iter )
{
    for ( int i = 0 ; i < num_iter ; i++ )
    {
        list< Node* >::iterator n;
        for ( n = nodeList.begin() ; n != nodeList.end(); ++n )
        {
            if ( !( *n )->m_DeleteMeFlag && !( *n )->fixed )
            {
                ( *n )->OptSmooth();
            }
        }
    }
}

bool Mesh::SetFixPoint( const vec3d &fix_pnt, vec2d fix_uw )
{
    double min_dist = FLT_MAX;
    Node* closest_node = NULL;
    double tol = m_GridDensity->m_MinLen;

    m_NumFixPointIter++;

    list< Node* >::iterator n;
    for ( n = nodeList.begin(); n != nodeList.end(); ++n )
    {
        if ( !( *n )->fixed )
        {
            double space = dist( fix_pnt, ( *n )->pnt );
            if ( space < min_dist )
            {
                min_dist = space;
                closest_node = ( *n );
            }
        }
    }

    if ( closest_node && m_Surf->ValidUW( fix_uw ) )
    {
        // Check lengths of connected edges and split if longer than tolerance
        vector < Edge* > check_edge_vec = closest_node->edgeVec;
        bool long_edge = false;

        for ( size_t i = 0; i < check_edge_vec.size(); i++ )
        {
            if ( !check_edge_vec[i]->border && check_edge_vec[i]->ComputeLength() > tol )
            {
                long_edge = true;
                break;
            }
        }

        if ( !long_edge )
        {
            // Move closest node to fixed point location
            closest_node->uw = m_Surf->ClosestUW( fix_pnt, fix_uw.x(), fix_uw.y() );
            closest_node->pnt = m_Surf->CompPnt( closest_node->uw.x(), closest_node->uw.y() );
            closest_node->fixed = true;
            return true;
        }
    }

    list< Edge* >::iterator e;

    //===== Split if no nodes found ====//
    vector < Edge* > split_edge_vec;
    split_edge_vec.reserve( edgeList.size() );
    for ( e = edgeList.begin(); e != edgeList.end(); ++e )
    {
        if ( !( *e )->border )
        {
            split_edge_vec.push_back( ( *e ) );
        }
    }

    int num_split = split_edge_vec.size();

    for ( int i = 0; i < num_split; i++ )
    {
        SplitEdge( split_edge_vec[i] );
    }

    DumpGarbage();

    if ( num_split > 0 )
    {
        return SetFixPoint( fix_pnt, fix_uw );
    }

    return false; // Indicates no closest node was found
}

void Mesh::AdjustEdgeLengths()
{
    //==== Find Avg Edge Length ====//
    double avg_length = 0.0;
    list< Edge* >::iterator e;
    for ( e = edgeList.begin() ; e != edgeList.end(); ++e )
    {
        avg_length += dist( ( *e )->n0->pnt, ( *e )->n1->pnt );
    }

    avg_length /= ( double )edgeList.size();

    for ( e = edgeList.begin() ; e != edgeList.end(); ++e )
    {
        if ( !( *e )->n0->fixed && !( *e )->n1->fixed )
        {
            vec3d  dir = ( *e )->n0->pnt - ( *e )->n1->pnt;
            double len = dir.mag();
            double scale = 1.0 + 0.25 * ( ( avg_length / len ) - 1.0 );

            ( *e )->n0->pnt = ( *e )->n1->pnt + dir * scale;
            ( *e )->n1->pnt = ( *e )->n0->pnt - dir * scale;
        }

    }
}

void Mesh::ComputeTargetEdgeLength( Node* n )
{
    for( int i = 0; i < ( int )n->edgeVec.size(); i++ )
    {
        ComputeTargetEdgeLength( n->edgeVec[i] );
    }
}

void Mesh::ComputeTargetEdgeLength( Edge* edge )
{
    assert( m_GridDensity );

    if( edge->border && edge->m_Length > m_GridDensity->m_MinLen )
    {
        edge->target_len = edge->m_Length;
    }
    else
    {
        vec3d cent = ( edge->n0->pnt + edge->n1->pnt ) * 0.5;
        vec2d uwcent = ( edge->n0->uw  + edge->n1->uw ) * 0.5;

        edge->target_len = m_Surf->InterpTargetMap( uwcent.x(), uwcent.y() );
    }
}


void Mesh::CheckValidAllEdges()
{
    list< Edge* >::iterator e;
    for ( e = edgeList.begin() ; e != edgeList.end(); ++e )
    {
        if ( !( *e )->m_DeleteMeFlag )
        {
            CheckValidEdge( ( *e ) );
        }
    }
}

void Mesh::CheckValidEdge( Edge* edge )
{
    Node* n0 = edge->n0;
    Node* n1 = edge->n1;

    assert( n0 );
    assert( n1 );

    Tri* t0 = edge->t0;
    Tri* t1 = edge->t1;

    assert ( t0 || t1 );

    if ( t0 )
    {
        assert ( t0->Contains( ( edge ) ) );
        assert ( t0->Contains( n0, n1 ) );
        if ( !t0->Contains( edge ) )
        {
            t0->debugFlag = true;
        }
    }
    if ( t1 )
    {
        assert ( t1->Contains( ( edge ) ) );
        assert ( t1->Contains( n0, n1 ) );
        if ( !t1->Contains( edge ) )
        {
            t1->debugFlag = true;
        }

    }
    if ( t0 && t1 )
    {
        Node* na = t0->OtherNode( n0, n1 );
        Node* nb = t1->OtherNode( n0, n1 );
        assert( na != nb );

        vec3d norm0 = t0->Normal();
        vec3d norm1 = t1->Normal();

//      assert( angle( norm0, norm1 ) < M_PI_2 );
        if ( angle( norm0, norm1 ) >= M_PI_2 )
        {
            t0->debugFlag = true;
            t1->debugFlag = true;
        }
    }
}

void Mesh::TriangulateBorder( const vector< vec3d > &uw_border )
{

}

void Mesh::CheckValidTriInput( vector< vec2d > & uw_points, vector< MeshSeg > & segs_indexes )
{
    //int min_i;
    //int min_j;
    double min_dist = 1.0e12;
    for ( int i = 0 ; i < ( int )uw_points.size() ; i++ )
    {
        for ( int j = i + 1 ; j < ( int )uw_points.size() ; j++ )
        {
            double d = dist( uw_points[i], uw_points[j] );
            if ( d < min_dist )
            {
                min_dist = d;
            }
        }
    }
}


void Mesh::InitMesh( vector< vec2d > & uw_points, vector< MeshSeg > & segs_indexes )
{
    assert( m_Surf );

    int i, j;
    char str[256];

    int num_pnts  = uw_points.size();
    int num_edges = segs_indexes.size();

    if ( num_pnts < 3 )
    {
        return;
    }

    vec2d VspMinUW = vec2d( m_Surf->GetSurfCore()->GetMinU(), m_Surf->GetSurfCore()->GetMinW() );
    double VspMinU = VspMinUW.v[0];
    double VspMinW = VspMinUW.v[1];

    vec2d VspMaxUW = vec2d( m_Surf->GetSurfCore()->GetMaxU(), m_Surf->GetSurfCore()->GetMaxW() );
    double VspMaxU = VspMaxUW.v[0];
    double VspMaxW = VspMaxUW.v[1];

    double VspdU = VspMaxU - VspMinU;
    double VspdW = VspMaxW - VspMinW;

    for ( i = 0 ; i < num_pnts ; i++ )
    {
        uw_points[i] = uw_points[i] - VspMinUW;
    }


    //==== Scale UW Pnts ====//
    for ( i = 0 ; i < ( int )uw_points.size() ; i++ )
    {
        double su = m_Surf->GetUScale( uw_points[i].y() / VspdW );
        double sw = m_Surf->GetWScale( uw_points[i].x() / VspdU );
        uw_points[i].set_x( su * uw_points[i].x() );
        uw_points[i].set_y( sw * uw_points[i].y() );
    }

#ifdef DEBUG_CFD_MESH
    static int namecnt = 0;
    sprintf( str, "%sMesh_UW%d.dat", cfdMeshMgrPtr->m_DebugDir.get_char_star(), namecnt );

    FILE* fp = fopen( str, "w" );
    for ( i = 0 ; i < num_edges ; i++ )
    {
        int ind0 = segs_indexes[i].m_Index[0];
        int ind1 = segs_indexes[i].m_Index[1];

        fprintf( fp, "MOVE \n" );
        fprintf( fp, "%f %f\n", uw_points[ind0].x(), uw_points[ind0].y() );
        fprintf( fp, "%f %f\n", uw_points[ind1].x(), uw_points[ind1].y() );
    }
    fclose( fp );
#endif



//CheckValidTriInput( uw_points, segs_indexes );


    //==== Dump Into Triangle ====//
    context* ctx;
    triangleio in, out;
    int tristatus = TRI_NULL;

    // init
    ctx = triangle_context_create();

    memset( &in, 0, sizeof( in ) ); // Load Zeros
    memset( &out, 0, sizeof( out ) );

    //==== PreAllocate Data For In/Out ====//
    in.pointlist    = ( REAL * ) malloc( num_pnts * 2 * sizeof( REAL ) );
    out.pointlist   = NULL;

    in.segmentlist  = ( int * ) malloc( num_edges * 2 * sizeof( int ) );
    out.segmentlist  = NULL;
    out.trianglelist  = NULL;

    in.numberofpointattributes = 0;
    in.pointattributelist = NULL;
    in.pointmarkerlist = NULL;
    in.numberofholes = 0;
    in.numberoftriangles = 0;
    in.numberofpointattributes = 0;
    in.numberofedges = 0;
    in.trianglelist = NULL;
    in.trianglearealist = NULL;
    in.edgelist = NULL;
    in.edgemarkerlist = NULL;
    in.segmentmarkerlist = NULL;

    //==== Load Points into Triangle Struct ====//
    in.numberofpoints = num_pnts;

    int cnt = 0;
    for ( j = 0 ; j < num_pnts ; j++ )
    {
        in.pointlist[cnt] = uw_points[j].x();
        cnt++;
        in.pointlist[cnt] = uw_points[j].y();
        cnt++;
    }

    in.numberofsegments = num_edges;
    cnt = 0;
    for ( j = 0 ; j < ( int )segs_indexes.size() ; j++ )
    {
        assert ( in.segmentlist[cnt] < num_pnts );
        in.segmentlist[cnt] = segs_indexes[j].m_Index[0];
        cnt++;
        assert ( in.segmentlist[cnt] < num_pnts );
        in.segmentlist[cnt] = segs_indexes[j].m_Index[1];
        cnt++;
    }

    //==== Constrained Delaunay Trianglulation ====//
    double est_num_tris = ( uw_points.size() / 4 ) * ( uw_points.size() / 4 );
    if ( est_num_tris < 1 )
    {
        est_num_tris = 1;
    }
    if ( est_num_tris > 10000 )
    {
        est_num_tris = 10000;
    }

    BndBox box;
    for ( i = 0 ; i < ( int )uw_points.size() ; i++ )
    {
        vec3d uwpnt( uw_points[i].x(), uw_points[i].y(), 0 );
        box.Update( uwpnt );
    }

    double uw_area = ( box.GetMax( 0 ) - box.GetMin( 0 ) ) * ( box.GetMax( 1 ) - box.GetMin( 1 ) );
    double fudgefactor = 4.0;
    double uw_tri_area = fudgefactor * uw_area / est_num_tris;

    if ( uw_tri_area < 1.0e-4 )
    {
        uw_tri_area = 1.0e-4;
    }

    sprintf( str, "zpYYQa%8.6fq20", uw_tri_area );

    //==== Constrained Delaunay Trianglulation ====//
    tristatus = triangle_context_options( ctx, str );
    if ( tristatus != TRI_OK ) printf( "triangle_context_options Error\n" );

    // Triangulate the polygon
    tristatus = triangle_mesh_create( ctx, &in );
    if ( tristatus != TRI_OK ) printf( "triangle_mesh_create Error\n" );

#ifdef DEBUG_CFD_MESH
    sprintf( str, "%sUWTriMeshOut%d.dat", cfdMeshMgrPtr->m_DebugDir.get_char_star(), namecnt );
    fp = fopen( str, "w" );
    for ( i = 0 ; i < out.numberoftriangles ; i++ )
    {
        int ind0 = out.trianglelist[i * 3];
        int ind1 = out.trianglelist[i * 3 + 1];
        int ind2 = out.trianglelist[i * 3 + 2];

        fprintf( fp, "MOVE \n" );
        fprintf( fp, "%f %f\n", out.pointlist[ind0 * 2], out.pointlist[ind0 * 2 + 1] );
        fprintf( fp, "%f %f\n", out.pointlist[ind1 * 2], out.pointlist[ind1 * 2 + 1] );
        fprintf( fp, "%f %f\n", out.pointlist[ind2 * 2], out.pointlist[ind2 * 2 + 1] );
        fprintf( fp, "%f %f\n", out.pointlist[ind0 * 2], out.pointlist[ind0 * 2 + 1] );

    }
    fclose( fp );
    namecnt++;
#endif

//    static int namecnt = 0;
//
//    sprintf( str, "UWTriMeshOut%d.m", namecnt );
//    FILE *fp = fopen( str, "w" );
//    fprintf( fp, "clear all\nformat compact\n" );
//    fprintf( fp, "t=[" );
//    for ( i = 0 ; i < out.numberoftriangles ; i++ )
//    {
//        int ind0 = out.trianglelist[i * 3] + 1;
//        int ind1 = out.trianglelist[i * 3 + 1] + 1;
//        int ind2 = out.trianglelist[i * 3 + 2] + 1;
//
//        fprintf( fp, "%d, %d, %d", ind0, ind1, ind2 );
//
//        if ( i < out.numberoftriangles - 1 )
//            fprintf( fp, ";\n" );
//        else
//            fprintf( fp, "];\n" );
//    }
//
//    fprintf( fp, "x=[" );
//    for ( i = 0; i < out.numberofpoints; i++ )
//    {
//        fprintf( fp, "%f", out.pointlist[i * 2] );
//
//        if ( i < out.numberofpoints - 1 )
//            fprintf( fp, ";\n" );
//        else
//            fprintf( fp, "];\n" );
//
//    }
//
//    fprintf( fp, "y=[" );
//    for ( i = 0; i < out.numberofpoints; i++ )
//    {
//        fprintf( fp, "%f", out.pointlist[i * 2 + 1] );
//
//        if ( i < out.numberofpoints - 1 )
//            fprintf( fp, ";\n" );
//        else
//            fprintf( fp, "];\n" );
//
//    }
//
//    fprintf( fp, "triplot(t,x,y)\n" );
//
//    fclose( fp );
//    namecnt++;

    //==== Clear All Node, Edge, Tri Data ====//
    Clear();

    if ( tristatus == TRI_OK )
    {
        triangle_mesh_copy( ctx, &out, 1, 1 );

        //==== Create Nodes ====//
        vector< Node* > nodeVec;
        for ( i = 0; i < out.numberofpoints; i++ )
        {
            double u = out.pointlist[i * 2];
            double w = out.pointlist[i * 2 + 1];
            double su = 1.0 / m_Surf->GetUScale( w / VspdW );
            double sw = 1.0 / m_Surf->GetWScale( u / VspdU );
            vec2d uw = vec2d( su * u + VspMinU, sw * w + VspMinW );
            vec3d pnt = m_Surf->CompPnt( uw.v[0], uw.v[1] );
            nodeVec.push_back( AddNode( pnt, uw ) );
        }

        //==== Load Triangles if No New Point Created ====//
        cnt = 0;
        for ( i = 0; i < out.numberoftriangles; i++ )
        {
            Node* n0 = nodeVec[out.trianglelist[cnt]];
            Node* n1;
            Node* n2;

            if ( !m_Surf->GetFlipFlag() )
            {
                n1 = nodeVec[out.trianglelist[cnt + 1]];
                n2 = nodeVec[out.trianglelist[cnt + 2]];
            }
            else
            {
                n1 = nodeVec[out.trianglelist[cnt + 2]];
                n2 = nodeVec[out.trianglelist[cnt + 1]];
            }

            cnt += 3;

            Edge* e0 = FindEdge( n0, n1 );
            if ( !e0 )
            {
                e0 = AddEdge( n0, n1 );
            }

            Edge* e1 = FindEdge( n1, n2 );
            if ( !e1 )
            {
                e1 = AddEdge( n1, n2 );
            }

            Edge* e2 = FindEdge( n2, n0 );
            if ( !e2 )
            {
                e2 = AddEdge( n2, n0 );
            }

            Tri* tri = AddTri( n0, n1, n2, e0, e1, e2 );

            if ( e0->t0 == NULL )
            {
                e0->t0 = tri;
            }
            else if ( e0->t1 == NULL )
            {
                e0->t1 = tri;
            }
            else
            {
                assert( 0 );
            }

            if ( e1->t0 == NULL )
            {
                e1->t0 = tri;
            }
            else if ( e1->t1 == NULL )
            {
                e1->t1 = tri;
            }
            else
            {
                assert( 0 );
            }

            if ( e2->t0 == NULL )
            {
                e2->t0 = tri;
            }
            else if ( e2->t1 == NULL )
            {
                e2->t1 = tri;
            }
            else
            {
                assert( 0 );
            }

        }

        //==== Fix The Exterior Edges ====//
        list< Edge* >::iterator e;
        for ( e = edgeList.begin(); e != edgeList.end(); ++e )
        {
            if ( ( *e )->t0 == NULL || ( *e )->t1 == NULL )
            {
                //          (*e)->ridge = true;
                ( *e )->border = true;
                ( *e )->n0->fixed = true;
                ( *e )->n1->fixed = true;
            }
        }

        for ( j = 0; j < (int)segs_indexes.size(); j++ )
        {
            Node* n0 = nodeVec[segs_indexes[j].m_Index[0]];
            Node* n1 = nodeVec[segs_indexes[j].m_Index[1]];

            for ( int k = 0; k < (int)n0->edgeVec.size(); k++ )
            {
                Node* ne0 = n0->edgeVec[k]->n0;
                Node* ne1 = n0->edgeVec[k]->n1;

                if ( ( ne0 == n0 && ne1 == n1 ) || ( ne0 == n1 && ne1 == n0 ) )
                {
                    n0->edgeVec[k]->border = true;
                    n0->fixed = true;
                    n1->fixed = true;
                }
            }
        }
    }

    //==== Free Local Memory ====//
    if ( in.pointlist )
    {
        free( in.pointlist );
    }
    if ( in.segmentlist )
    {
        free( in.segmentlist );
    }
    if ( out.pointlist )
    {
        free( out.pointlist );
    }
    if ( out.pointmarkerlist )
    {
        free( out.pointmarkerlist );
    }
    if ( out.trianglelist )
    {
        free( out.trianglelist );
    }
    if ( out.segmentlist )
    {
        free( out.segmentlist );
    }
    if ( out.segmentmarkerlist )
    {
        free( out.segmentmarkerlist );
    }

    // cleanup
    triangle_context_destroy( ctx );
}

void Mesh::RemoveInteriorTrisEdgesNodes()
{
    set < Tri* > remTris;
    set < Edge* > remEdges;
    set < Node* > remNodes;

    list< Tri* >::iterator t;
    for ( t = triList.begin() ; t != triList.end(); ++t )
    {
        //==== Check Surrounding Tris =====//
        if ( ( *t )->deleteFlag )
        {
            //==== Check Edges ====//
            if ( ( *t )->e0->BothAdjoiningTrisInterior() )
            {
                remEdges.insert( ( *t )->e0 );
            }
            if ( ( *t )->e1->BothAdjoiningTrisInterior() )
            {
                remEdges.insert( ( *t )->e1 );
            }
            if ( ( *t )->e2->BothAdjoiningTrisInterior() )
            {
                remEdges.insert( ( *t )->e2 );
            }

            //==== Check Nodes ====//
            if ( ( *t )->n0->AllInteriorConnectedTris() )
            {
                remNodes.insert( ( *t )->n0 );
            }
            if ( ( *t )->n1->AllInteriorConnectedTris() )
            {
                remNodes.insert( ( *t )->n1 );
            }
            if ( ( *t )->n2->AllInteriorConnectedTris() )
            {
                remNodes.insert( ( *t )->n2 );
            }

            remTris.insert( ( *t ) );
        }
    }

    //==== Remove References to Deleted Tris =====//
    set< Tri* >::iterator st;
    for ( st = remTris.begin() ; st != remTris.end(); ++st )
    {
        ( *st )->e0->ReplaceTri( ( *st ), NULL );
        ( *st )->e1->ReplaceTri( ( *st ), NULL );
        ( *st )->e2->ReplaceTri( ( *st ), NULL );
    }
    set< Edge* >::iterator se;
    for ( se = remEdges.begin() ; se != remEdges.end(); ++se )
    {
        ( *se )->n0->RemoveConnectEdge( ( *se ) );
        ( *se )->n1->RemoveConnectEdge( ( *se ) );
    }

    //==== Remove Node Edges and Tris =====//
    set< Node* >::iterator sn;
    for ( sn = remNodes.begin() ; sn != remNodes.end(); ++sn )
    {
        RemoveNode( ( *sn ) );
    }
    for ( se = remEdges.begin() ; se != remEdges.end(); ++se )
    {
        RemoveEdge( ( *se ) );
    }
    for ( st = remTris.begin() ; st != remTris.end(); ++st )
    {
        RemoveTri( ( *st ) );
    }

    DumpGarbage();
}


void Mesh::ReadSTL( const char* file_name )
{
    FILE* file_id = fopen( file_name, "r" );

    char str[256];
    float nx, ny, nz;
    float v0[3];
    float v1[3];
    float v2[3];

    if ( file_id )
    {
        fgets( str, 255, file_id );
        int stopFlag = 0;
        while ( !stopFlag )
        {
            if ( EOF == fscanf( file_id, "%*s %*s %f %f %f\n", &nx, &ny, &nz ) )
            {
                break;
            }
            if ( EOF == fscanf( file_id, "%*s %*s" ) )
            {
                break;
            }
            if ( EOF == fscanf( file_id, "%*s %f %f %f\n", &v0[0], &v0[1], &v0[2] ) )
            {
                break;
            }
            if ( EOF == fscanf( file_id, "%*s %f %f %f\n", &v1[0], &v1[1], &v1[2] ) )
            {
                break;
            }
            if ( EOF == fscanf( file_id, "%*s %f %f %f\n", &v2[0], &v2[1], &v2[2] ) )
            {
                break;
            }
            if ( EOF == fscanf( file_id, "%*s" ) )
            {
                break;
            }
            if ( EOF == fscanf( file_id, "%*s" ) )
            {
                break;
            }

            //==== Add Nodes ====//
            Node* n0 = FindNode( vec3d( v0[0], v0[1], v0[2] ) );
            if ( !n0 )
            {
                n0 = AddNode( vec3d( v0[0], v0[1], v0[2] ), vec2d( 0, 0 ) );
            }

            Node* n1 = FindNode( vec3d( v1[0], v1[1], v1[2] ) );
            if ( !n1 )
            {
                n1 = AddNode( vec3d( v1[0], v1[1], v1[2] ), vec2d( 0, 0 ) );
            }

            Node* n2 = FindNode( vec3d( v2[0], v2[1], v2[2] ) );
            if ( !n2 )
            {
                n2 = AddNode( vec3d( v2[0], v2[1], v2[2] ), vec2d( 0, 0 ) );
            }

            Edge* e0 = FindEdge( n0, n1 );
            if ( !e0 )
            {
                e0 = AddEdge( n0, n1 );
            }

            Edge* e1 = FindEdge( n1, n2 );
            if ( !e1 )
            {
                e1 = AddEdge( n1, n2 );
            }

            Edge* e2 = FindEdge( n2, n0 );
            if ( !e2 )
            {
                e2 = AddEdge( n2, n0 );
            }

            Tri* tri = AddTri( n0, n1, n2, e0, e1, e2 );

            if      ( e0->t0 == NULL )
            {
                e0->t0 = tri;
            }
            else if ( e0->t1 == NULL )
            {
                e0->t1 = tri;
            }
            else
            {
                assert( 0 );
            }

            if      ( e1->t0 == NULL )
            {
                e1->t0 = tri;
            }
            else if ( e1->t1 == NULL )
            {
                e1->t1 = tri;
            }
            else
            {
                assert( 0 );
            }

            if      ( e2->t0 == NULL )
            {
                e2->t0 = tri;
            }
            else if ( e2->t1 == NULL )
            {
                e2->t1 = tri;
            }
            else
            {
                assert( 0 );
            }
        }
    }
    if ( file_id )
    {
        fclose( file_id );
    }

    //==== Fix The Exterior Edges ====//
    list< Edge* >::iterator e;
    for ( e = edgeList.begin() ; e != edgeList.end(); ++e )
    {
        if ( ( *e )->t0 == NULL || ( *e )->t1 == NULL )
        {
            ( *e )->ridge = true;
            ( *e )->n0->fixed = true;
            ( *e )->n1->fixed = true;
        }
    }


}

void Mesh::WriteSTL( const char* file_name )
{
    FILE* file_id = fopen( file_name, "w" );
    if ( file_id )
    {
        fprintf( file_id, "solid\n" );

        WriteSTL( file_id );

        fprintf( file_id, "endsolid\n" );
        fclose( file_id );
    }
}



void Mesh::WriteSTL( FILE* file_id )
{
    for ( int i = 0 ; i < ( int )simpTriVec.size() ; i++ )
    {
        SimpTri* t = &simpTriVec[i];

        vec3d& p0 = simpPntVec[t->ind0];
        vec3d& p1 = simpPntVec[t->ind1];
        vec3d& p2 = simpPntVec[t->ind2];
        vec3d v10 = p1 - p0;
        vec3d v20 = p2 - p1;
        vec3d norm = cross( v10, v20 );
        norm.normalize();

        fprintf( file_id, " facet normal  %2.10le %2.10le %2.10le\n",  norm.x(), norm.y(), norm.z() );
        fprintf( file_id, "   outer loop\n" );

        fprintf( file_id, "     vertex %2.10le %2.10le %2.10le\n", p0.x(), p0.y(), p0.z() );
        fprintf( file_id, "     vertex %2.10le %2.10le %2.10le\n", p1.x(), p1.y(), p1.z() );
        fprintf( file_id, "     vertex %2.10le %2.10le %2.10le\n", p2.x(), p2.y(), p2.z() );

        fprintf( file_id, "   endloop\n" );
        fprintf( file_id, " endfacet\n" );
    }
}

/*
void Mesh::Draw()
{

    //==== Debug ====//
    //list< Tri* >::iterator t;
    //for ( t = triList.begin() ; t != triList.end(); t++ )
    //{
    //  glColor3ubv( (*t)->rgb );
    //  glBegin( GL_POLYGON );

    //  glVertex3dv( (*t)->n0->pnt.data() );
    //  glVertex3dv( (*t)->n1->pnt.data() );
    //  glVertex3dv( (*t)->n2->pnt.data() );

    //  glEnd();
    //}

    //==== Edges ====//
    glLineWidth( 1.0 );
    glBegin( GL_LINES );

    Edge* hl_edge = NULL;
    int edge_cnt = 0;
    list< Edge* >::iterator e;
    for ( e = edgeList.begin() ; e != edgeList.end(); e++ )
    {
        glLineWidth( 1.0 );
        glColor3ub( 0, 0, 255 );
        if ( !(*e)->debugFlag )
        {
            glVertex3dv( (*e)->n0->pnt.data() );
            glVertex3dv( (*e)->n1->pnt.data() );
        }

        edge_cnt++;
    }
    glEnd();

    glLineWidth( 3.0 );
    glColor3ub( 255, 0, 0 );
    glBegin( GL_LINES );
    for ( e = edgeList.begin() ; e != edgeList.end(); e++ )
    {
        if ( (*e)->debugFlag )
        {
            glVertex3dv( (*e)->n0->pnt.data() );
            glVertex3dv( (*e)->n1->pnt.data() );
        }
    }
    glEnd();



    //Node* hl_node = 0;
    //glPointSize( 3.0f );
    //glBegin( GL_POINTS );
    //int cnt = 0;
    //list< Node* >::iterator n;
    //for ( n = nodeList.begin() ; n != nodeList.end(); n++ )
    //{
    //  glColor3ub( 255, 0, 0 );

    //  if ( (*n)->fixed )
    //      glColor3ub( 255, 255, 0 );

    //  glVertex3dv( (*n)->pnt.data() );

    //  if ( cnt == m_HighlightNodeIndex )
    //  {
    //      hl_node = (*n);
    //  }
    //  cnt++;

    //}
    //glEnd();

    ////==== Highlight ====//
    //if ( hl_node )
    //{
    //  glPointSize( 10.0f );
    //  glBegin( GL_POINTS );
    //  glColor3ub( 0, 255, 0 );
    //  glVertex3dv( hl_node->pnt.data() );
    //  glEnd();

    //  glBegin( GL_LINES );
    //  glColor3ub( 0, 255, 0 );

    //  for ( int i = 0 ; i < (int)hl_node->edgeVec.size() ; i++ )
    //  {
    //      Edge* eptr = hl_node->edgeVec[i];
    //      glVertex3dv( eptr->n0->pnt.data() );
    //      glVertex3dv( eptr->n1->pnt.data() );
    //  }
    //  glEnd();

    //}


}
*/











