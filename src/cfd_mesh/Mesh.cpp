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
#include "SurfaceIntersectionMgr.h"

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
}

Mesh::~Mesh()
{
    DumpGarbage();
    Clear();
}

void Mesh::Clear()
{
    list< Face* >::iterator f;
    for ( f = faceList.begin() ; f != faceList.end(); ++f )
    {
        delete ( *f );
    }

    faceList.clear();

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

void Mesh::LoadSimpFaces()
{
    list< Face* >::iterator f;
    simpFaceVec.resize( faceList.size() );
    simpPntVec.resize( faceList.size() * 4 );
    simpUWPntVec.resize( faceList.size() * 4 );

    int cnt = 0;
    int ncnt = 0;
    for ( f = faceList.begin() ; f != faceList.end(); ++f )
    {
        simpFaceVec[cnt].ind0 = ncnt;
        simpPntVec[ncnt]   = ( *f )->n0->pnt;
        simpUWPntVec[ncnt] = ( *f )->n0->uw;
        ncnt++;

        simpFaceVec[cnt].ind1 = ncnt;
        simpPntVec[ncnt] = ( *f )->n1->pnt;
        simpUWPntVec[ncnt] = ( *f )->n1->uw;
        ncnt++;

        simpFaceVec[cnt].ind2 = ncnt;
        simpPntVec[ncnt] = ( *f )->n2->pnt;
        simpUWPntVec[ncnt] = ( *f )->n2->uw;
        ncnt++;

        if ( ( *f )->IsQuad() )
        {
            simpFaceVec[cnt].m_isQuad = true;
            simpFaceVec[cnt].ind3 = ncnt;
            simpPntVec[ncnt] = ( *f )->n3->pnt;
            simpUWPntVec[ncnt] = ( *f )->n3->uw;
            ncnt++;
        }

        cnt++;
    }

    simpPntVec.resize( ncnt );
    simpUWPntVec.resize( ncnt );
}

void Mesh::CondenseSimpFaces()
{
    //==== Map Coincedent Point ====//
    vector< int > reMap;
    reMap.reserve( simpFaceVec.size() * 4 );
    map< int, vector< int > > indMap;
    for ( int i = 0 ; i < ( int )simpFaceVec.size() ; i++ )
    {
        reMap.push_back( CheckDupOrAdd( simpFaceVec[i].ind0, indMap, simpPntVec ) );
        reMap.push_back( CheckDupOrAdd( simpFaceVec[i].ind1, indMap, simpPntVec ) );
        reMap.push_back( CheckDupOrAdd( simpFaceVec[i].ind2, indMap, simpPntVec ) );
        if ( simpFaceVec[i].m_isQuad )
        {
            reMap.push_back( CheckDupOrAdd( simpFaceVec[i].ind3, indMap, simpPntVec ) );
        }
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
    int iremap = 0;
    for ( int i = 0 ; i < ( int )simpFaceVec.size() ; i++ )
    {
        simpFaceVec[i].ind0 = reMap[ iremap ];
        iremap++;
        simpFaceVec[i].ind1 = reMap[ iremap ];
        iremap++;
        simpFaceVec[i].ind2 = reMap[ iremap ];
        iremap++;

        if ( simpFaceVec[i].m_isQuad )
        {
            simpFaceVec[i].ind3 = reMap[ iremap ];
            iremap++;
        }
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

// TODO: Re-write with nanoflann
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

int Mesh::RemoveRevFaces()
{
    int badcount = 0;

    vector < Edge* > remEdges;

    list< Face* >::iterator f;
    for ( f = faceList.begin() ; f != faceList.end(); ++f )
    {
        vec3d nface = (*f)->Normal();
        vec3d nsurf = (*f)->ComputeCenterNormal( m_Surf );

        double dprod = dot ( nface, nsurf );

        if ( m_Surf->GetFlipFlag() )
        {
            dprod = -dprod;
        }

        if ( dprod < 0.0 )
        {
            Edge* e = (*f)->FindLongEdge();

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
    list< Face* >::iterator f;
    for ( f = faceList.begin() ; f != faceList.end(); ++f )
    {
        double q = ( *f )->ComputeTriQual();

        if ( q > M_PI / 6.0 )                                           // > 30 Deg
        {
            ( *f )->rgb[0] = ( *f )->rgb[1] = ( *f )->rgb[2] = 255;
        }
        else if ( q > M_PI / 7.0 )
        {
            ( *f )->rgb[2] = 255;    // 25 deg
            ( *f )->rgb[0] = ( *f )->rgb[1] = 100;
        }
        else
        {
            ( *f )->rgb[0] = 255;
            ( *f )->rgb[1] = ( *f )->rgb[2] = 100;
        }
    }

//  printf( "Num Faces = %d \n", (int)faceList.size() );
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

Face* Mesh::AddFace( Node* nn0, Node* nn1, Node* nn2, Edge* ee0, Edge* ee1, Edge* ee2 )
{
    Face* fptr = new Face( nn0, nn1, nn2, ee0, ee1, ee2 );
    faceList.push_back( fptr );
    fptr->list_ptr = --faceList.end();

    ee0->SetFace( fptr );
    ee1->SetFace( fptr );
    ee2->SetFace( fptr );

    return fptr;
}

Face* Mesh::AddFace( Node* nn0, Node* nn1, Node* nn2, Node* nn3, Edge* ee0, Edge* ee1, Edge* ee2, Edge* ee3 )
{
    Face* fptr = new Face( nn0, nn1, nn2, nn3, ee0, ee1, ee2, ee3 );
    faceList.push_back( fptr );
    fptr->list_ptr = --faceList.end();

    ee0->SetFace( fptr );
    ee1->SetFace( fptr );
    ee2->SetFace( fptr );
    ee3->SetFace( fptr );

    return fptr;
}

void Mesh::RemoveFace( Face* fptr )
{
    garbageFaceVec.push_back( fptr );
    faceList.erase( fptr->list_ptr );
    fptr->m_DeleteMeFlag = true;
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

    //==== Delete Flagged Faces =====//
    for ( int i = 0 ; i < ( int )garbageFaceVec.size() ; i++ )
    {
        delete garbageFaceVec[i];
    }
    garbageFaceVec.clear();
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

    assert( edge->f0 || edge->f1 );

    Face* fa = edge->f0;
    Face* fb = edge->f1;

    Node* n0 = edge->n0;
    Node* n1 = edge->n1;
    if ( fa && !fa->CorrectOrder( n0, n1 ) )
    {
        n0 = edge->n1;
        n1 = edge->n0;
    }
    else if ( !fa && fb && !fb->CorrectOrder( n0, n1 ) )
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
    es0->border = edge->border; // Should be impossible.
    es1->border = edge->border; // Should be impossible.

    if ( fa )
    {
        Node* na = fa->OtherNodeTri( n0, n1 );
        Edge* ea = AddEdge( na, ns );

        Edge* ea0 = fa->FindEdge( n0, na );
        Edge* ea1 = fa->FindEdge( na, n1 );

        ea0->RemoveFace( fa );
        ea1->RemoveFace( fa );

        Face* fa0 = AddFace( n0, ns, na, ea0, ea, es0 );
        Face* fa1 = AddFace( n1, na, ns, ea1, es1, ea );

        RemoveFace( fa );
    }

    if ( fb )
    {
        Node* nb = fb->OtherNodeTri( n0, n1 );
        Edge* eb = AddEdge( ns, nb );

        Edge* eb0 = fb->FindEdge( n0, nb );
        Edge* eb1 = fb->FindEdge( nb, n1 );

        eb0->RemoveFace( fb );
        eb1->RemoveFace( fb );

        Face* fb0 = AddFace( n0, nb, ns, es0, eb, eb0 );
        Face* fb1 = AddFace( n1, ns, nb, es1, eb1, eb );

        RemoveFace( fb );
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

    Face*  fa = edge->f0;
    Face*  fb = edge->f1;

    if ( !fa || !fb )
    {
        return;
    }

    if ( ThreeEdgesThreeFaces( edge ) )
    {
        return;
    }

    Node* n0 = edge->n0;
    Node* n1 = edge->n1;


    if ( !fa->CorrectOrder( n0, n1 ) )
    {
        n0 = edge->n1;
        n1 = edge->n0;
    }

    Node* na = fa->OtherNodeTri( n0, n1 );
    Node* nb = fb->OtherNodeTri( n0, n1 );

    assert( na != nb );

    //==== Determine Face Quality of Existing Faces =====//
    double qa = fa->ComputeTriQual();
    double qb = fb->ComputeTriQual();
    double qc = Face::ComputeTriQual( n0, nb, na );
    double qd = Face::ComputeTriQual( n1, na, nb );

    if ( min( qc, qd ) <= min( qa, qb ) )
    {
        return;
    }

    vec3d norma = fa->Normal();
    vec3d normb = fb->Normal();
    vec3d normc = Face::Normal( n0, nb, na );
    vec3d normd = Face::Normal( n1, na, nb );

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

    Edge* ea0 = fa->FindEdge( n0, na );
    Edge* ea1 = fa->FindEdge( na, n1 );
    Edge* eb0 = fb->FindEdge( n0, nb );
    Edge* eb1 = fb->FindEdge( nb, n1 );

    edge->n0 = na;
    edge->n1 = nb;
    edge->ComputeLength();
    ComputeTargetEdgeLength( edge );

    na->AddConnectEdge( edge );
    nb->AddConnectEdge( edge );
    n0->RemoveConnectEdge( edge );
    n1->RemoveConnectEdge( edge );

    fa->SetNodesEdges( n0, nb, na, ea0, edge, eb0 );
    fb->SetNodesEdges( n1, na, nb, eb1, edge, ea1 );

    if ( ea1->f0 == fa )
    {
        ea1->f0 = fb;
    }
    else if ( ea1->f1 == fa )
    {
        ea1->f1 = fb;
    }
    else
    {
        assert( 0 );
    }

    if ( eb0->f0 == fb )
    {
        eb0->f0 = fa;
    }
    else if ( eb0->f1 == fb )
    {
        eb0->f1 = fa;
    }
    else
    {
        assert( 0 );
    }

    LimitTargetEdgeLength( edge );

//CheckValidAllEdges();
}

bool Mesh::ThreeEdgesThreeFaces( Edge* edge )
{
    Node* n0 = edge->n0;
    Node* n1 = edge->n1;

    vector< Face* > f;
    n0->GetConnectFaces( f );
    if ( f.size() == 3 && n0->edgeVec.size() == 3 )
    {
        return true;
    }

    vector< Face* > fvec1;
    n1->GetConnectFaces( fvec1 );

    return fvec1.size() == 3 && n1->edgeVec.size() == 3;
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

    if ( !edge->f0 || !edge->f1 )
    {
        return false;
    }

    if ( edge->f0->m_DeleteMeFlag || edge->f1->m_DeleteMeFlag )
    {
        return false;
    }

    Node* n0 = edge->n0;
    Node* n1 = edge->n1;
    Face* fa = edge->f0;
    Face* fb = edge->f1;
    Node* na = fa->OtherNodeTri( n0, n1 );
    Node* nb = fb->OtherNodeTri( n0, n1 );

    //==== Check 3 Faces in a Face Case =====//
    Edge* e0a = fa->FindEdge( n0, na );
    Edge* e1a = fa->FindEdge( n1, na );

    if ( !e0a || ! e1a )
    {
        return false;
    }

    Face* fa0 = e0a->OtherFace( fa );
    Face* fa1 = e1a->OtherFace( fa );

    if ( fa0 && fa1 )
    {
        Node* na0 = fa0->OtherNodeTri( n0, na );
        Node* na1 = fa1->OtherNodeTri( n1, na );

        if ( na0 == na1 )
        {
            return false;
        }
    }

    Edge* e0b = fb->FindEdge( n0, nb );
    Edge* e1b = fb->FindEdge( n1, nb );

    Face* fb0 = e0b->OtherFace( fb );
    Face* fb1 = e1b->OtherFace( fb );

    if ( fb0 && fb1 )
    {
        Node* nb0 = fb0->OtherNodeTri( n0, nb );
        Node* nb1 = fb1->OtherNodeTri( n1, nb );

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

bool Mesh::ValidNodeMove( Node* nptr, const vec3d & move_to, Face* ignoreFace )
{
    int i;
    bool valid_flag = true;
    vector < Face* > faceVec;
    nptr->GetConnectFaces( faceVec );

    vector < vec3d > normals;
    for ( i = 0 ; i < ( int )faceVec.size() ; i++ )
    {
        if ( faceVec[i] != ignoreFace )
        {
            normals.push_back( faceVec[i]->Normal() );
        }
    }

    vec3d save_pos = nptr->pnt;
    nptr->pnt = move_to;

    vector < vec3d > move_normals;
    for ( i = 0 ; i < ( int )faceVec.size() ; i++ )
    {
        if ( faceVec[i] != ignoreFace )
        {
            move_normals.push_back( faceVec[i]->Normal() );
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

    Face* fa = edge->f0;
    Face* fb = edge->f1;
    Node* na = fa->OtherNodeTri( n0, n1 );
    Node* nb = fb->OtherNodeTri( n0, n1 );

    assert( na != nb );

    Edge* ea0 = fa->FindEdge( na, n0 );
    Edge* ea1 = fa->FindEdge( na, n1 );
    Edge* eb0 = fb->FindEdge( nb, n0 );
    Edge* eb1 = fb->FindEdge( nb, n1 );
    Face* fa0 = ea0->OtherFace( fa );
    Face* fa1 = ea1->OtherFace( fa );
    Face* fb0 = eb0->OtherFace( fb );
    Face* fb1 = eb1->OtherFace( fb );


    if ( fa0 && fa1 )
    {
        Node* other_ta0 = fa0->OtherNodeTri( na, n0 );
        Node* other_ta1 = fa1->OtherNodeTri( na, n1 );
        assert ( other_ta0 != other_ta1 );
    }

    if ( fb0 && fb1 )
    {
        Node* other_tb0 = fb0->OtherNodeTri( nb, n0 );
        Node* other_tb1 = fb1->OtherNodeTri( nb, n1 );
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
        vec3d psplit = ( n0->pnt + n1->pnt ) * 0.5;
        vec2d uwsplit = ( n0->uw + n1->uw ) * 0.5;
        uwc = m_Surf->ClosestUW( psplit, uwsplit[0], uwsplit[1] );
        pc  = m_Surf->CompPnt( uwc.x(), uwc.y() );
    }

    if ( !ValidNodeMove( n0, pc, fa ) )
    {
        return;
    }
    if ( !ValidNodeMove( n1, pc, fb ) )
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

        std::map< Edge *, Node * >::iterator it;
        it = m_BorderEdgeSplitNode.find( ea0 );

        if( it != m_BorderEdgeSplitNode.end() )
        {
            Node* ns = it->second;
            m_BorderEdgeSplitNode.erase( it );
            m_BorderEdgeSplitNode[ eca ] = ns;
        }

        it = m_BorderEdgeSplitNode.find( ea1 );

        if( it != m_BorderEdgeSplitNode.end() )
        {
            Node* ns = it->second;
            m_BorderEdgeSplitNode.erase( it );
            m_BorderEdgeSplitNode[ eca ] = ns;
        }
    }
    if ( eb0->border || eb1->border )
    {
        ecb->border = true;

        std::map< Edge *, Node * >::iterator it;
        it = m_BorderEdgeSplitNode.find( eb0 );

        if( it != m_BorderEdgeSplitNode.end() )
        {
            Node* ns = it->second;
            m_BorderEdgeSplitNode.erase( it );
            m_BorderEdgeSplitNode[ ecb ] = ns;
        }

        it = m_BorderEdgeSplitNode.find( eb1 );

        if( it != m_BorderEdgeSplitNode.end() )
        {
            Node* ns = it->second;
            m_BorderEdgeSplitNode.erase( it );
            m_BorderEdgeSplitNode[ ecb ] = ns;
        }
    }
    if ( ea0->ridge  || ea1->ridge )
    {
        eca->ridge = true;
    }
    if ( eb0->ridge  || eb1->ridge )
    {
        ecb->ridge = true;
    }

//jrg Check for invalid faces and improved qual

    eca->f0 = fa0;
    eca->f1 = fa1;
    ecb->f0 = fb0;
    ecb->f1 = fb1;

    if ( fa0 )
    {
        fa0->ReplaceEdge( ea0, eca );
    }
    if ( fa1 )
    {
        fa1->ReplaceEdge( ea1, eca );
    }
    if ( fb0 )
    {
        fb0->ReplaceEdge( eb0, ecb );
    }
    if ( fb1 )
    {
        fb1->ReplaceEdge( eb1, ecb );
    }


//CheckValidEdge(eca);
//CheckValidEdge(ecb);

    //==== Change Any Faces That Point to n0 ====//
    vector< Face* > fVec;
    n0->GetConnectFaces( fVec );
    for ( int i = 0 ; i < ( int )fVec.size() ; i++ )
    {
        fVec[i]->ReplaceNode( n0, nc );
    }
    n1->GetConnectFaces( fVec );
    for ( int i = 0 ; i < ( int )fVec.size() ; i++ )
    {
        fVec[i]->ReplaceNode( n1, nc );
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
    RemoveFace( fa );
    RemoveFace( fb );
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
    double min_dist = DBL_MAX;
    Node* closest_node = NULL;

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
        // Move closest node to fixed point location
        closest_node->uw = m_Surf->ClosestUW( fix_pnt, fix_uw.x(), fix_uw.y() );
        closest_node->pnt = m_Surf->CompPnt( closest_node->uw.x(), closest_node->uw.y() );
        closest_node->fixed = true;

        // Check for any error.  Should always be 0.0.
        // However, projecting point and computing is cheap, so no harm in keeping the above code.
        // vec2d duw = closest_node->uw - fix_uw;
        // vec3d dpt = closest_node->pnt - fix_pnt;
        // printf( "duw %e %e dpt %e %e %e\n", duw.x(), duw.y(), dpt.x(), dpt.y(), dpt.z() );

        return true;
    }

    return false;
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

    Face* f0 = edge->f0;
    Face* f1 = edge->f1;

    assert ( f0 || f1 );

    if ( f0 )
    {
        assert ( f0->Contains(( edge ) ) );
        assert ( f0->Contains( n0, n1 ) );
        if ( !f0->Contains( edge ) )
        {
            f0->debugFlag = true;
        }
    }
    if ( f1 )
    {
        assert ( f1->Contains(( edge ) ) );
        assert ( f1->Contains( n0, n1 ) );
        if ( !f1->Contains( edge ) )
        {
            f1->debugFlag = true;
        }

    }
    if ( f0 && f1 )
    {
        Node* na = f0->OtherNodeTri( n0, n1 );
        Node* nb = f1->OtherNodeTri( n0, n1 );
        assert( na != nb );

        vec3d norm0 = f0->Normal();
        vec3d norm1 = f1->Normal();

//      assert( angle( norm0, norm1 ) < M_PI_2 );
        if ( angle( norm0, norm1 ) >= M_PI_2 )
        {
            f0->debugFlag = true;
            f1->debugFlag = true;
        }
    }
}

bool vec2dCompare( const vec2d &a, const vec2d &b )
{
    if ( a.x() == b.x() )
        return a.y() < b.y();
    return a.x() < b.x();
}

void Mesh::InitMesh( vector< vec2d > & uw_points, vector< MeshSeg > & segs_indexes, SurfaceIntersectionSingleton *MeshMgr )
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

#ifdef DEBUG_CFD_MESH
    static int namecnt = 0;
    FILE* fp;

    vector< vec2d > sorted = uw_points;
    sort( sorted.begin(), sorted.end(), vec2dCompare );

    sprintf( str, "%sSortedUnscaledMesh_UW%d.m", MeshMgr->m_DebugDir.c_str(), namecnt );
    fp = fopen( str, "w" );

    fprintf( fp, "u = [" );
    for ( i = 0 ; i < sorted.size() ; i++ )
    {
        fprintf( fp, "%.19e", sorted[i].x() );

        if ( i < sorted.size() - 1 )
        {
            fprintf( fp, ";\n" );
        }
        else
        {
            fprintf( fp, "];\n" );
        }
    }
    fprintf( fp, "v = [" );
    for ( i = 0 ; i < sorted.size() ; i++ )
    {
        fprintf( fp, "%.19e", sorted[i].y() );

        if ( i < sorted.size() - 1 )
        {
            fprintf( fp, ";\n" );
        }
        else
        {
            fprintf( fp, "];\n" );
        }
    }
    fprintf( fp, "figure ( 1 );\n" );
    fprintf( fp, "plot( u', v', 'x' );\n" );
    fprintf( fp, "axis equal;\n" );

    fclose( fp );
#endif

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
    sprintf( str, "%sMesh_UW%d.m", MeshMgr->m_DebugDir.c_str(), namecnt );
    fp = fopen( str, "w" );
    fprintf( fp, "u = [" );
    for ( i = 0 ; i < num_edges ; i++ )
    {
        int ind0 = segs_indexes[i].m_Index[0];
        int ind1 = segs_indexes[i].m_Index[1];
        fprintf( fp, "%.19e %.19e", uw_points[ind0].x(), uw_points[ind1].x() );

        if ( i < num_edges - 1 )
        {
            fprintf( fp, ";\n" );
        }
        else
        {
            fprintf( fp, "];\n" );
        }
    }
    fprintf( fp, "v = [" );
    for ( i = 0 ; i < num_edges ; i++ )
    {
        int ind0 = segs_indexes[i].m_Index[0];
        int ind1 = segs_indexes[i].m_Index[1];
        fprintf( fp, "%.19e %.19e", uw_points[ind0].y(), uw_points[ind1].y() );

        if ( i < num_edges - 1 )
        {
            fprintf( fp, ";\n" );
        }
        else
        {
            fprintf( fp, "];\n" );
        }
    }
    fprintf( fp, "figure ( 1 );\n" );
    fprintf( fp, "plot( u', v', 'x-' );\n" );
    fprintf( fp, "axis equal;\n" );

    fclose( fp );
#endif



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

            Edge* e0 = n0->FindEdge( n1 );
            if ( !e0 )
            {
                e0 = AddEdge( n0, n1 );
            }

            Edge* e1 = n1->FindEdge( n2 );
            if ( !e1 )
            {
                e1 = AddEdge( n1, n2 );
            }

            Edge* e2 = n2->FindEdge( n0 );
            if ( !e2 )
            {
                e2 = AddEdge( n2, n0 );
            }

            Face* face = AddFace( n0, n1, n2, e0, e1, e2 );
        }


        for ( j = 0; j < (int)segs_indexes.size(); j++ )
        {
            Node* n0 = nodeVec[segs_indexes[j].m_Index[0]];
            Node* n1 = nodeVec[segs_indexes[j].m_Index[1]];

            Edge *e = n0->FindEdge( n1 );

            if ( e )
            {
                e->border = true;

                n0->fixed = true;
                n1->fixed = true;

                vec2d uw = segs_indexes[j].m_UWmid;
                vec3d pnt = m_Surf->CompPnt( uw.v[0], uw.v[1] );
                Node* nsplit = AddNode( pnt, uw );
                nsplit->fixed = true;

                m_BorderEdgeSplitNode[ e ] = nsplit;
            }
        }
    }

#ifdef DEBUG_CFD_MESH
        sprintf( str, "%sUWTriMeshOut%d.m", MeshMgr->m_DebugDir.c_str(), namecnt );
        fp = fopen( str, "w" );
        fprintf( fp, "clear all\nformat compact\n" );
        fprintf( fp, "t = [" );
        for ( i = 0 ; i < out.numberoftriangles ; i++ )
        {
            int ind0 = out.trianglelist[i * 3] + 1;
            int ind1 = out.trianglelist[i * 3 + 1] + 1;
            int ind2 = out.trianglelist[i * 3 + 2] + 1;

            fprintf( fp, "%d, %d, %d", ind0, ind1, ind2 );

            if ( i < out.numberoftriangles - 1 )
                fprintf( fp, ";\n" );
            else
                fprintf( fp, "];\n" );
        }

        fprintf( fp, "uprm = [" );
        for ( i = 0; i < out.numberofpoints; i++ )
        {
            fprintf( fp, "%f", out.pointlist[i * 2] );

            if ( i < out.numberofpoints - 1 )
                fprintf( fp, ";\n" );
            else
                fprintf( fp, "];\n" );

        }

        fprintf( fp, "wprm = [" );
        for ( i = 0; i < out.numberofpoints; i++ )
        {
            fprintf( fp, "%f", out.pointlist[i * 2 + 1] );

            if ( i < out.numberofpoints - 1 )
                fprintf( fp, ";\n" );
            else
                fprintf( fp, "];\n" );

        }

        fprintf( fp, "u = [" );
        for ( i = 0; i < out.numberofpoints; i++ )
        {
            double u = out.pointlist[i * 2];
            double w = out.pointlist[i * 2 + 1];
            double su = 1.0 / m_Surf->GetUScale( w / VspdW );
            double sw = 1.0 / m_Surf->GetWScale( u / VspdU );
            double uu = su * u + VspMinU;

            fprintf( fp, "%f", uu );

            if ( i < out.numberofpoints - 1 )
                fprintf( fp, ";\n" );
            else
                fprintf( fp, "];\n" );
        }

        fprintf( fp, "w = [" );
        for ( i = 0; i < out.numberofpoints; i++ )
        {
            double u = out.pointlist[i * 2];
            double w = out.pointlist[i * 2 + 1];
            double su = 1.0 / m_Surf->GetUScale( w / VspdW );
            double sw = 1.0 / m_Surf->GetWScale( u / VspdU );
            double ww = sw * w + VspMinW;

            fprintf( fp, "%f", ww );

            if ( i < out.numberofpoints - 1 )
                fprintf( fp, ";\n" );
            else
                fprintf( fp, "];\n" );
        }

        fprintf( fp, "x = [" );
        for ( i = 0; i < out.numberofpoints; i++ )
        {
            double u = out.pointlist[i * 2];
            double w = out.pointlist[i * 2 + 1];
            double su = 1.0 / m_Surf->GetUScale( w / VspdW );
            double sw = 1.0 / m_Surf->GetWScale( u / VspdU );
            vec2d uw = vec2d( su * u + VspMinU, sw * w + VspMinW );
            vec3d pnt = m_Surf->CompPnt( uw.v[0], uw.v[1] );

            fprintf( fp, "%f", pnt.x() );

            if ( i < out.numberofpoints - 1 )
                fprintf( fp, ";\n" );
            else
                fprintf( fp, "];\n" );
        }

        fprintf( fp, "y = [" );
        for ( i = 0; i < out.numberofpoints; i++ )
        {
            double u = out.pointlist[i * 2];
            double w = out.pointlist[i * 2 + 1];
            double su = 1.0 / m_Surf->GetUScale( w / VspdW );
            double sw = 1.0 / m_Surf->GetWScale( u / VspdU );
            vec2d uw = vec2d( su * u + VspMinU, sw * w + VspMinW );
            vec3d pnt = m_Surf->CompPnt( uw.v[0], uw.v[1] );

            fprintf( fp, "%f", pnt.y() );

            if ( i < out.numberofpoints - 1 )
                fprintf( fp, ";\n" );
            else
                fprintf( fp, "];\n" );
        }

        fprintf( fp, "z = [" );
        for ( i = 0; i < out.numberofpoints; i++ )
        {
            double u = out.pointlist[i * 2];
            double w = out.pointlist[i * 2 + 1];
            double su = 1.0 / m_Surf->GetUScale( w / VspdW );
            double sw = 1.0 / m_Surf->GetWScale( u / VspdU );
            vec2d uw = vec2d( su * u + VspMinU, sw * w + VspMinW );
            vec3d pnt = m_Surf->CompPnt( uw.v[0], uw.v[1] );

            fprintf( fp, "%f", pnt.z() );

            if ( i < out.numberofpoints - 1 )
                fprintf( fp, ";\n" );
            else
                fprintf( fp, "];\n" );
        }

        fprintf( fp, "figure( 2 )\n" );
        fprintf( fp, "triplot( t, uprm, wprm )\n" );
        fprintf( fp, "axis equal\n" );

        fprintf( fp, "figure( 3 )\n" );
        fprintf( fp, "triplot( t, u, w )\n" );
        fprintf( fp, "axis equal\n" );

        fprintf( fp, "figure( 4 )\n" );
        fprintf( fp, "trimesh( t, x, y, z )\n" );
        fprintf( fp, "axis equal\n" );

        fclose( fp );
        namecnt++;
#endif



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

void Mesh::RemoveInteriorFacesEdgesNodes()
{
    set < Face* > remFaces;
    set < Edge* > remEdges;
    set < Node* > remNodes;

    list< Face* >::iterator f;
    for ( f = faceList.begin() ; f != faceList.end(); ++f )
    {
        //==== Check Surrounding Faces =====//
        if ( ( *f )->deleteFlag )
        {
            ( *f )->BuildRemovalSet( remFaces, remEdges, remNodes );
        }
    }

    //==== Remove References to Deleted Faces =====//
    set< Face* >::iterator sf;
    for ( sf = remFaces.begin() ; sf != remFaces.end(); ++sf )
    {
        ( *sf )->EdgeForgetFace();
    }
    set< Edge* >::iterator se;
    for ( se = remEdges.begin() ; se != remEdges.end(); ++se )
    {
        ( *se )->NodeForgetEdge();
    }

    //==== Remove Node Edges and Faces =====//
    set< Node* >::iterator sn;
    for ( sn = remNodes.begin() ; sn != remNodes.end(); ++sn )
    {
        RemoveNode( ( *sn ) );
    }
    for ( se = remEdges.begin() ; se != remEdges.end(); ++se )
    {
        RemoveEdge( ( *se ) );
    }
    for ( sf = remFaces.begin() ; sf != remFaces.end(); ++sf )
    {
        RemoveFace(( *sf ));
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

            Face* face = AddFace( n0, n1, n2, e0, e1, e2 );
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
        if (( *e )->f0 == NULL || ( *e )->f1 == NULL )
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
    for ( int i = 0 ; i < ( int )simpFaceVec.size() ; i++ )
    {
        SimpFace* f = &simpFaceVec[i];

        vec3d& p0 = simpPntVec[f->ind0];
        vec3d& p1 = simpPntVec[f->ind1];
        vec3d& p2 = simpPntVec[f->ind2];
        vec3d v01 = p1 - p0;
        vec3d v12 = p2 - p1;
        vec3d norm = cross( v01, v12 );
        norm.normalize();

        fprintf( file_id, " facet normal  %2.10le %2.10le %2.10le\n",  norm.x(), norm.y(), norm.z() );
        fprintf( file_id, "   outer loop\n" );

        fprintf( file_id, "     vertex %2.10le %2.10le %2.10le\n", p0.x(), p0.y(), p0.z() );
        fprintf( file_id, "     vertex %2.10le %2.10le %2.10le\n", p1.x(), p1.y(), p1.z() );
        fprintf( file_id, "     vertex %2.10le %2.10le %2.10le\n", p2.x(), p2.y(), p2.z() );

        fprintf( file_id, "   endloop\n" );
        fprintf( file_id, " endfacet\n" );

        if ( f->m_isQuad ) // Split quad and write additional tri.
        {
            vec3d& p3 = simpPntVec[f->ind3];
            vec3d v23 = p3 - p2;
            vec3d v30 = p0 - p3;
            norm = cross( v23, v30 );
            norm.normalize();

            fprintf( file_id, " facet normal  %2.10le %2.10le %2.10le\n",  norm.x(), norm.y(), norm.z() );
            fprintf( file_id, "   outer loop\n" );

            fprintf( file_id, "     vertex %2.10le %2.10le %2.10le\n", p0.x(), p0.y(), p0.z() );
            fprintf( file_id, "     vertex %2.10le %2.10le %2.10le\n", p2.x(), p2.y(), p2.z() );
            fprintf( file_id, "     vertex %2.10le %2.10le %2.10le\n", p3.x(), p3.y(), p3.z() );

            fprintf( file_id, "   endloop\n" );
            fprintf( file_id, " endfacet\n" );
        }
    }
}

// Edge split data structure.
class splitData
{
public:
    splitData() : ns{nullptr}, es0{nullptr}, es1{nullptr} {}
    splitData( Node* ns, Edge* es0, Edge* es1 ) : ns{ns}, es0{es0}, es1{es1} {}

    Node* ns;
    Edge* es0;
    Edge* es1;
};

void Mesh::ConvertToQuads()
{
    // Store copies of original edge and face lists.
    // Working from a copy allows us to traverse the list as we add edges/faces without traversing the new edges/faces.
    list < Edge* > origEdgeList = edgeList;
    list < Face* > origFaceList = faceList;

    // Map containing information about each edge split -- keyed by the edge.  This allows us to recall this information
    // each time the edge is used.
    map< Edge*, splitData > splitEdgeMap;

    // Loop over all edges.
    for ( list< Edge* >::iterator e = origEdgeList.begin() ; e != origEdgeList.end(); ++e )
    {
        Node* n0 = ( *e )->n0;
        Node* n1 = ( *e )->n1;

        Node* ns = NULL;

        std::map< Edge *, Node * >::iterator it;
        it = m_BorderEdgeSplitNode.find( *e );

        if( it != m_BorderEdgeSplitNode.end() )
        {
            ns = it->second;
        }
        else
        {
            // Approximate edge midpoint.
            // Should perhaps be weighted by relative target edge lengths.
            vec3d psplit  = ( n0->pnt + n1->pnt ) * 0.5;
            vec2d uwsplit = ( n0->uw  + n1->uw ) * 0.5;

            // Project approximate midpoint to surface, determine true UW and XYZ.
            vec2d uws = m_Surf->ClosestUW( psplit, uwsplit[0], uwsplit[1] );
            vec3d ps  = m_Surf->CompPnt( uws.x(), uws.y() );

            // Create midpoint node.
            ns  = AddNode( ps, uws );

            // Node will be fixed if both endpoints are fixed (i.e. edge is a border edge).
            ns->fixed = n0->fixed && n1->fixed;
        }

        // Create split edges.
        Edge* es0 = AddEdge( n0, ns );
        Edge* es1 = AddEdge( ns, n1 );

        // Copy parent properties to split edges.
        es0->ridge = ( *e )->ridge;
        es1->ridge = ( *e )->ridge;
        es0->border = ( *e )->border;
        es1->border = ( *e )->border;

        // Add split data to map.
        splitEdgeMap[ *e ] = splitData( ns, es0, es1 );

        // Compute edge length for new node.
        ComputeTargetEdgeLength( ns );
        LimitTargetEdgeLength( ns );
    }

    // Loop over all faces
    for ( list< Face* >::iterator f = origFaceList.begin() ; f != origFaceList.end(); ++f )
    {
        // Skip any quads - should be impossible.
        if( ( *f )->IsQuad() )
        {
            continue;
        }

        // Construct triangle center point and add node.  Center point could possibly be weighted based on target
        // edge lengths.
        vec3d pcen;
        vec2d uwcen;
        ( *f )->ComputeCenterPnt( m_Surf, pcen, uwcen );
        Node* ncen  = AddNode( pcen, uwcen );

        // Get existing triangle nodes.  These are in cw order.
        Node* n0 = ( *f )->n0;
        Node* n1 = ( *f )->n1;
        Node* n2 = ( *f )->n2;

        // Get existing triangle edges.  Lookup edges by nodes because they aren't stored in any particular order.
        Edge* e0 = ( *f )->FindEdge( n0, n1 );
        Edge* e1 = ( *f )->FindEdge( n1, n2 );
        Edge* e2 = ( *f )->FindEdge( n2, n0 );

        // Get split data for existing edges.
        splitData sd0 = splitEdgeMap[ e0 ];
        splitData sd1 = splitEdgeMap[ e1 ];
        splitData sd2 = splitEdgeMap[ e2 ];

        // Construct edges from edge split to tri center.
        Edge* em0 = AddEdge( sd0.ns, ncen );
        Edge* em1 = AddEdge( sd1.ns, ncen );
        Edge* em2 = AddEdge( sd2.ns, ncen );

        // Determine which half of split edge is used with node 0
        Edge *ea, *eb;
        ea = sd0.es0;
        if ( !ea->ContainsNode( n0 ) )
            ea = sd0.es1;

        eb = sd2.es0;
        if ( !eb->ContainsNode( n0 ) )
            eb = sd2.es1;

        // Add quad starting at node 0
        AddFace( n0, sd0.ns, ncen, sd2.ns, ea, em0, em2, eb );

        // Determine which half of split edge is used with node 1
        ea = sd1.es0;
        if ( !ea->ContainsNode( n1 ) )
            ea = sd1.es1;

        eb = sd0.es0;
        if ( !eb->ContainsNode( n1 ) )
            eb = sd0.es1;

        // Add quad starting at node 1
        AddFace( n1, sd1.ns, ncen, sd0.ns, ea, em1, em0, eb );

        // Determine which half of split edge is used with node 2
        ea = sd2.es0;
        if ( !ea->ContainsNode( n2 ) )
            ea = sd2.es1;

        eb = sd1.es0;
        if ( !eb->ContainsNode( n2 ) )
            eb = sd1.es1;

        // Add quad starting at node 2
        AddFace( n2, sd2.ns, ncen, sd1.ns, ea, em2, em1, eb );
    }

    // Clean up edges and tris.
    for ( list< Edge* >::iterator e = origEdgeList.begin() ; e != origEdgeList.end(); ++e )
    {
        RemoveEdge( *e );
    }

    for ( list< Face* >::iterator f = origFaceList.begin() ; f != origFaceList.end(); ++f )
    {
        RemoveFace( *f );
    }

    DumpGarbage();
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











