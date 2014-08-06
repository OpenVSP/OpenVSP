//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// Tri
//
//////////////////////////////////////////////////////////////////////

#include "Tri.h"
#include "Surf.h"

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
Node::~Node()
{

}

void Node::AddConnectEdge( Edge* e )
{
    for ( int i = 0 ; i < ( int )edgeVec.size() ; i++ ) //jrg not sure I need this check???
    {
        if ( e == edgeVec[i] )
        {
            return;
        }
    }
    edgeVec.push_back( e );
}

void Node::RemoveConnectEdge( Edge* e )
{
    for ( int i = 0 ; i < ( int )edgeVec.size() ; i++ )
    {
        Edge* eptr = edgeVec[i];
        if ( eptr == e )
        {
            edgeVec.erase( edgeVec.begin() + i );
            break;
        }
    }
}

void Node::GetConnectNodes( vector< Node* > & cnVec )
{
    cnVec.resize( edgeVec.size() );

    for ( int i = 0 ; i < ( int )edgeVec.size() ; i++ )
    {
        cnVec[i] = edgeVec[i]->OtherNode( this );
    }
}

void Node::GetConnectTris( vector< Tri* > & ctVec )
{
//jrg speed this up!!!!
    ctVec.clear();
    for ( int i = 0 ; i < ( int )edgeVec.size() ; i++ )
    {
        Tri* t0 = edgeVec[i]->t0;
        if ( t0 && find( ctVec.begin(), ctVec.end(), t0 ) == ctVec.end() )
        {
            ctVec.push_back( t0 );
        }

        Tri* t1 = edgeVec[i]->t1;
        if ( t1 && find( ctVec.begin(), ctVec.end(), t1 ) == ctVec.end() )
        {
            ctVec.push_back( t1 );
        }
    }

}

bool Node::AllInteriorConnectedTris()
{
    vector< Tri* > tvec;
    GetConnectTris( tvec );
    for ( int i = 0 ; i < ( int )tvec.size() ; i++ )
    {
        if ( !tvec[i]->interiorFlag )
        {
            return false;
        }
    }
    return true;
}


void Node::LaplacianSmooth( Surf* surfPtr )
{

    vector< Node* > connectNodes;
    GetConnectNodes( connectNodes );

    if ( ( int )connectNodes.size() < 2 )
    {
        return;
    }

    double bigLen = 0.0;
    double smallLen = 1.0e12;
    for ( int i = 0 ; i < ( int )connectNodes.size() ; i++ )
    {
        double len = dist( pnt, connectNodes[i]->pnt );
        if ( len > bigLen )
        {
            bigLen = len;
        }
        if ( len < smallLen )
        {
            smallLen = len;
        }
    }

    if ( smallLen < 1.0e-12 )
    {
        return;
    }

    double lenRatio = bigLen / smallLen;
    if ( lenRatio > 100.0 )
    {
        return;
    }



    vec2d moveUW;
    vec3d movePnt;
    for ( int i = 0 ; i < ( int )connectNodes.size() ; i++ )
    {
        moveUW = moveUW + connectNodes[i]->uw;
        movePnt = movePnt + connectNodes[i]->pnt;
    }

    moveUW = moveUW * ( 1.0 / ( double )connectNodes.size() );
    movePnt = movePnt * ( 1.0 / ( double )connectNodes.size() );

    vec2d close_uw = surfPtr->ClosestUW( movePnt, moveUW.x(),  moveUW.y(), 0.001, 0.001, 0.0001 );

    vec2d old_uw = uw;
    vec3d old_pnt = pnt;

    uw = uw + ( close_uw - uw ) * 0.1;
    pnt = surfPtr->CompPnt( uw.x(), uw.y() );

}

void Node::AreaWeightedLaplacianSmooth( Surf* surfPtr )
{
    vector< Tri* > connectTris;
    GetConnectTris( connectTris );

    vector< double > fracts;
    fracts.resize( connectTris.size() );

    double sum_area = 0.0;
    for ( int i = 0 ; i < ( int )connectTris.size() ; i++ )
    {
        fracts[i] = connectTris[i]->Area();
        sum_area += fracts[i];
    }

    if ( sum_area < 1.0e-12 )
    {
        return;
    }

    vec3d movePnt = vec3d( 0, 0, 0 );
    vec2d moveUW  = vec2d( 0, 0 );
    for ( int i = 0 ; i < ( int )connectTris.size() ; i++ )
    {
        movePnt = movePnt + connectTris[i]->n0->pnt * ( 1.0 / 3.0 ) * ( fracts[i] / sum_area );
        moveUW = moveUW + connectTris[i]->n0->uw * ( 1.0 / 3.0 ) * ( fracts[i] / sum_area );
        movePnt = movePnt + connectTris[i]->n1->pnt * ( 1.0 / 3.0 ) * ( fracts[i] / sum_area );
        moveUW = moveUW + connectTris[i]->n1->uw * ( 1.0 / 3.0 ) * ( fracts[i] / sum_area );
        movePnt = movePnt + connectTris[i]->n2->pnt * ( 1.0 / 3.0 ) * ( fracts[i] / sum_area );
        moveUW = moveUW + connectTris[i]->n2->uw * ( 1.0 / 3.0 ) * ( fracts[i] / sum_area );
    }

//  vec2d close_uw = surfPtr->ClosestUW( movePnt, moveUW.x(),  moveUW.y(), 0.001, 0.001, 0.00001 );
    vec2d close_uw = surfPtr->ClosestUW( movePnt, moveUW.x(),  moveUW.y() );

    vec2d old_uw = uw;
    vec3d old_pnt = pnt;

    uw = uw + ( close_uw - uw ) * 0.1;
    pnt = surfPtr->CompPnt( uw.x(), uw.y() );

}

void Node::LaplacianSmooth()
{
    vector< Node* > connectNodes;
    GetConnectNodes( connectNodes );

    if ( ( int )connectNodes.size() < 2 )
    {
        return;
    }

    vec3d movePnt;
    for ( int i = 0 ; i < ( int )connectNodes.size() ; i++ )
    {
        movePnt = movePnt + connectNodes[i]->pnt;
    }
    movePnt = movePnt * ( 1.0 / ( double )connectNodes.size() );

    pnt = pnt + ( movePnt - pnt ) * 0.1;
}


void Node::LaplacianSmoothUW()
{
    vector< Node* > connectNodes;
    GetConnectNodes( connectNodes );

    if ( ( int )connectNodes.size() < 2 )
    {
        return;
    }

    vec2d moveUW;
    for ( int i = 0 ; i < ( int )connectNodes.size() ; i++ )
    {
        moveUW = moveUW + connectNodes[i]->uw;
    }
    moveUW = moveUW * ( 1.0 / ( double )connectNodes.size() );

    uw = uw + ( moveUW - uw ) * 0.02;

}

void Node::OptSmooth()
{
    vector< Tri* > connectTris;
    GetConnectTris( connectTris );

    if ( ( int )connectTris.size() < 3 )
    {
        return;
    }

    double worst_qual = 0.0;
    Tri* worst_tri = NULL;
    for ( int i = 0 ; i < ( int )connectTris.size() ; i++ )
    {
        double q = connectTris[i]->ComputeCosSmallAng();
        if ( q > worst_qual )
        {
            worst_qual = q;
            worst_tri  = connectTris[i];
        }
    }

    //==== Good Tris -> Don't Bother ====//
    //if ( worst_qual < 0.707 )
    //  return;

    vec3d orig_pos = pnt;
    Edge* far_edge = worst_tri->FindEdgeWithout( this );

    //==== Find Target Pos ====//
    vec3d proj = proj_pnt_on_ray( far_edge->n0->pnt, far_edge->n1->pnt, orig_pos );
    vec3d dir = orig_pos - proj;
    dir.normalize();

    double len = 0.866 * dist( far_edge->n0->pnt, far_edge->n1->pnt );
    vec3d target_pos = ( far_edge->n0->pnt + far_edge->n1->pnt ) * 0.5 + dir * len;

    pnt = pnt + ( target_pos - pnt ) * 0.02;            // Move 1% Towards Target

    bool move_back = false;
    for ( int i = 0 ; i < ( int )connectTris.size() ; i++ )
    {
        double q = connectTris[i]->ComputeCosSmallAng();
        if ( q > worst_qual )
        {
            move_back = true;
            break;
        }
    }

    //==== Restore Pos ====//
    if ( move_back )
    {
        pnt = orig_pos;
    }

}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

bool Edge::SetTri( Tri* t )
{
    if ( t0 && t1 )
    {
        printf( "Edge: More Than 2 Tris %16.10f %16.10f %16.10f\n",
//          t0->ComputeQual(), t1->ComputeQual(), t->ComputeQual() );
                t0->Area(), t1->Area(), t->Area() );
        Tri* badtri = t;
        if ( t0->Area() < badtri->Area() )
        {
            t0 = badtri;
        }
        if ( t1->Area() < badtri->Area() )
        {
            t1 = badtri;
        }

        printf( "     vertex %f %f %f\n", t0->n0->pnt.x(), t0->n0->pnt.y(), t0->n0->pnt.z() );
        printf( "     vertex %f %f %f\n", t0->n1->pnt.x(), t0->n1->pnt.y(), t0->n1->pnt.z() );
        printf( "     vertex %f %f %f\n", t0->n2->pnt.x(), t0->n2->pnt.y(), t0->n2->pnt.z() );

        printf( "     vertex %f %f %f\n", t1->n0->pnt.x(), t1->n0->pnt.y(), t1->n0->pnt.z() );
        printf( "     vertex %f %f %f\n", t1->n1->pnt.x(), t1->n1->pnt.y(), t1->n1->pnt.z() );
        printf( "     vertex %f %f %f\n", t1->n2->pnt.x(), t1->n2->pnt.y(), t1->n2->pnt.z() );

        printf( "     vertex %f %f %f\n", t->n0->pnt.x(), t->n0->pnt.y(), t->n0->pnt.z() );
        printf( "     vertex %f %f %f\n", t->n1->pnt.x(), t->n1->pnt.y(), t->n1->pnt.z() );
        printf( "     vertex %f %f %f\n", t->n2->pnt.x(), t->n2->pnt.y(), t->n2->pnt.z() );

        printf( "   Tri = %f %f %f\n        %f %f %f\n        %f %f %f \n",
                badtri->n0->pnt.x(), badtri->n0->pnt.y(), badtri->n0->pnt.z(),
                badtri->n1->pnt.x(), badtri->n1->pnt.y(), badtri->n1->pnt.z(),
                badtri->n2->pnt.x(), badtri->n2->pnt.y(), badtri->n2->pnt.z() );

        printf( "   Border %d %d %d\n", badtri->e0->border,  badtri->e1->border,  badtri->e2->border );


        return false;
    }
    if ( t0 )
    {
        t1 = t;
    }
    else
    {
        t0 = t;
    }

    return true;
}

bool Edge::ContainsNodes( Node* in0, Node* in1 )
{
    if ( in0 == n0 && in1 == n1 )
    {
        return true;
    }
    else if ( in0 == n1 && in1 == n0 )
    {
        return true;
    }

    return false;
}

Tri* Edge::OtherTri( Tri* t )
{
    if ( t == t0 )
    {
        return t1;
    }
    else if ( t == t1 )
    {
        return t0;
    }
    else
    {
        assert( 0 );
    }

    return NULL;
}

Node* Edge::OtherNode( Node* n )
{
    if ( n == n0 )
    {
        return n1;
    }
    else if ( n == n1 )
    {
        return n0;
    }
    else
    {
        assert( 0 );
    }

    return NULL;
}

void Edge::ReplaceNode( Node* curr_node, Node* replace_node )
{
    if ( n0 == curr_node )
    {
        n0 = replace_node;
    }
    else if ( n1 == curr_node )
    {
        n1 = replace_node;
    }
    else
    {
        assert( 0 );
    }
}

void Edge::ReplaceTri( Tri* t, Tri* replace_t )
{
    if ( t0 == t )
    {
        t0 = replace_t;
    }
    else if ( t1 == t )
    {
        t1 = replace_t;
    }
}

bool Edge::BothAdjoiningTrisInterior()
{
    if ( ( t0 && t0->interiorFlag ) || ( t0 == NULL ) )
        if ( ( t1 && t1->interiorFlag ) || ( t1 == NULL ) )
        {
            return true;
        }
    return false;
}


//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
Tri::Tri()
{
    m_DeleteMeFlag = false;
    debugFlag = false;
    n0 = n1 = n2 = NULL;
    e0 = e1 = e2 = NULL;
    interiorFlag = false;
    intExtCount = 0;
}

Tri::Tri( Node* nn0, Node* nn1, Node* nn2, Edge* ee0, Edge* ee1, Edge* ee2 )
{
    m_DeleteMeFlag = false;
    debugFlag = false;
    SetNodesEdges( nn0, nn1, nn2, ee0, ee1, ee2 );
    interiorFlag = false;
    intExtCount = 0;
}

Tri::~Tri()
{
}

void Tri::SetNodesEdges( Node* nn0, Node* nn1, Node* nn2, Edge* ee0, Edge* ee1, Edge* ee2 )
{
    n0 = nn0;
    n1 = nn1;
    n2 = nn2;
    e0 = ee0;
    e1 = ee1;
    e2 = ee2;
}

Edge* Tri::FindEdge( Node* nn0, Node* nn1 )
{
    if ( e0->n0 == nn0 && e0->n1 == nn1 )
    {
        return e0;
    }
    if ( e0->n0 == nn1 && e0->n1 == nn0 )
    {
        return e0;
    }
    if ( e1->n0 == nn0 && e1->n1 == nn1 )
    {
        return e1;
    }
    if ( e1->n0 == nn1 && e1->n1 == nn0 )
    {
        return e1;
    }
    if ( e2->n0 == nn0 && e2->n1 == nn1 )
    {
        return e2;
    }
    if ( e2->n0 == nn1 && e2->n1 == nn0 )
    {
        return e2;
    }

    return NULL;
}

Edge* Tri::FindEdgeWithout( Node* node_ptr )
{
    if ( e0->n0 != node_ptr && e0->n1 != node_ptr )
    {
        return e0;
    }
    if ( e1->n0 != node_ptr && e1->n1 != node_ptr )
    {
        return e1;
    }
    if ( e2->n0 != node_ptr && e2->n1 != node_ptr )
    {
        return e2;
    }

    return NULL;
}

void Tri::ReplaceNode( Node* curr_node, Node* replace_node )
{
    if ( n0 == curr_node )
    {
        n0 = replace_node;
    }
    else if ( n1 == curr_node )
    {
        n1 = replace_node;
    }
    else if ( n2 == curr_node )
    {
        n2 = replace_node;
    }
    else
    {
        assert( 0 );
    }
}

void Tri::ReplaceEdge( Edge* curr_edge, Edge* replace_edge )
{
    if ( e0 == curr_edge )
    {
        e0 = replace_edge;
    }
    else if ( e1 == curr_edge )
    {
        e1 = replace_edge;
    }
    else if ( e2 == curr_edge )
    {
        e2 = replace_edge;
    }
    else
    {
        assert( 0 );
    }
}

double Tri::ComputeQual()
{
    return ComputeQual( n0, n1, n2 );
}

double Tri::ComputeQual( Node* n0, Node* n1, Node* n2 )
{
    double ang0, ang1, ang2;

    ComputeCosAngles( n0, n1, n2, &ang0, &ang1, &ang2 );

    double minang = max( ang0, max( ang1, ang2 ) );

    if ( minang > 1.0 )
    {
        return 0.0;
    }
    else if ( minang < -1.0 )
    {
        return M_PI;
    }

    return acos( minang );


    //double A = dist( n0->pnt, n1->pnt );
    //double B = dist( n1->pnt, n2->pnt );
    //double C = dist( n2->pnt, n0->pnt );

    //double qual = 1.0;
    //if ( A > B && A > B )     qual = ((B+C)-A)/A;
    //else if ( B > C  )            qual = ((A+C)-B)/B;
    //else                      qual = ((A+B)-C)/C;

    //return qual;


    //double l0 = dist_squared(n0->pnt, n1->pnt );
    //double l1 = dist_squared(n1->pnt, n2->pnt );
    //double l2 = dist_squared(n2->pnt, n0->pnt );
    //double a = area( n0->pnt, n1->pnt, n2->pnt );
    //double qual = 6.9282*a/(l0 + l1 + l2);
    //return qual;
}

double Tri::ComputeCosSmallAng()
{
    double ang0, ang1, ang2;
    ComputeCosAngles( n0, n1, n2, &ang0, &ang1, &ang2 );

    double minang = max( ang0, max( ang1, ang2 ) );

    if ( minang > 1.0 )
    {
        return 1.0;
    }
    else if ( minang < -1.0 )
    {
        return -1.0;
    }

    return minang;
}

vec3d Tri::Normal()
{
    return Normal( n0, n1, n2 );
}

vec3d Tri::Normal( Node* n0, Node* n1, Node* n2 )
{
    return cross( n1->pnt - n0->pnt, n2->pnt - n0->pnt );
}



void Tri::ComputeCosAngles( double* ang0, double* ang1, double* ang2 )
{
    ComputeCosAngles( n0, n1, n2, ang0, ang1, ang2 );
}

void Tri::ComputeCosAngles( Node* n0, Node* n1, Node* n2, double* ang0, double* ang1, double* ang2 )
{
    double dsqr01 = dist_squared( n0->pnt, n1->pnt );
    double dsqr12 = dist_squared( n1->pnt, n2->pnt );
    double dsqr20 = dist_squared( n2->pnt, n0->pnt );

    double d01 = sqrt( dsqr01 );
    double d12 = sqrt( dsqr12 );
    double d20 = sqrt( dsqr20 );

    *ang0 = ( -dsqr12 + dsqr01 + dsqr20 ) / ( 2.0 * d01 * d20 );
    *ang1 = ( -dsqr20 + dsqr01 + dsqr12 ) / ( 2.0 * d01 * d12 );
    *ang2 = ( -dsqr01 + dsqr12 + dsqr20 ) / ( 2.0 * d12 * d20 );
}

Node* Tri::OtherNode( Node* a, Node* b )
{
    if      ( n0 == a && n1 == b )
    {
        return n2;
    }
    else if ( n0 == b && n1 == a )
    {
        return n2;
    }
    else if ( n1 == a && n2 == b )
    {
        return n0;
    }
    else if ( n1 == b && n2 == a )
    {
        return n0;
    }
    else if ( n0 == a && n2 == b )
    {
        return n1;
    }
    else if ( n0 == b && n2 == a )
    {
        return n1;
    }
    else
    {
        assert( 0 );
    }

    return NULL;
}

bool Tri::Contains( Node* a, Node* b )
{
    if ( a == b )
    {
        return false;
    }

    if ( a == n0 || a == n1 || a == n2 )
        if ( b == n0 || b == n1 || b == n2 )
        {
            return true;
        }

    return false;
}

bool Tri::Contains( Edge* e )
{
    if ( e == e0 || e == e1 || e == e2 )
    {
        return true;
    }
    return false;
}


bool Tri::CorrectOrder( Node* en0, Node* en1 )
{
    if ( en0 == n0 && en1 == n1 )
    {
        return true;
    }
    if ( en0 == n1 && en1 == n2 )
    {
        return true;
    }
    if ( en0 == n2 && en1 == n0 )
    {
        return true;
    }

    return false;

}

double Tri::Area()
{
    return area( n0->pnt, n1->pnt, n2->pnt );
}

vec3d Tri::ComputeCenterPnt( Surf* surfPtr )
{
    //vec3d avg_p = (n0->pnt + n1->pnt)*0.5;
    //avg_p = (avg_p + n2->pnt)*0.5;
    //vec2d avg_uw = (n0->uw + n1->uw)*0.5;
    //avg_uw = (avg_uw + n2->uw)*0.5;
    //vec2d uw = surfPtr->ClosestUW( avg_p, avg_uw[0], avg_uw[1] );
    //return surfPtr->CompPnt( uw[0], uw[1] );

    //return avg_p;

    //vec2d avg_uw = (n0->uw + n1->uw)*0.5;
    //avg_uw = (avg_uw + n2->uw)*0.5;
    //return surfPtr->CompPnt( avg_uw[0], avg_uw[1] );

    vec2d avg_uw = ( n0->uw + n1->uw + n2->uw ) * ( 1.0 / 3.0 );
    vec3d avg_p  = ( n0->pnt + n1->pnt + n2->pnt ) * ( 1.0 / 3.0 );
    vec2d uw = surfPtr->ClosestUW( avg_p, avg_uw[0], avg_uw[1] );
    return surfPtr->CompPnt( uw[0], uw[1] );
}

void Tri::LoadAdjTris( int num_levels, set< Tri* > & triSet )
{
    Tri* t;
    triSet.insert( this );

    num_levels--;
    if ( num_levels <= 0 )
    {
        return;
    }

    if ( !e0->border )
    {
        t = e0->OtherTri( this );
        if ( t )
        {
            t->LoadAdjTris( num_levels, triSet );
        }
    }
    if ( !e1->border )
    {
        t = e1->OtherTri( this );
        if ( t )
        {
            t->LoadAdjTris( num_levels, triSet );
        }
    }
    if ( !e2->border )
    {
        t = e2->OtherTri( this );
        if ( t )
        {
            t->LoadAdjTris( num_levels, triSet );
        }
    }
}








