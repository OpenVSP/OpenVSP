//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// Face
//
//////////////////////////////////////////////////////////////////////

#include "Face.h"
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

void Node::GetConnectFaces( vector< Face* > & cfVec )
{
//jrg speed this up!!!!
    cfVec.clear();
    for ( int i = 0 ; i < ( int )edgeVec.size() ; i++ )
    {
        Face* f0 = edgeVec[i]->f0;
        if ( f0 && find( cfVec.begin(), cfVec.end(), f0 ) == cfVec.end() )
        {
            cfVec.push_back( f0 );
        }

        Face* f1 = edgeVec[i]->f1;
        if ( f1 && find( cfVec.begin(), cfVec.end(), f1 ) == cfVec.end() )
        {
            cfVec.push_back( f1 );
        }
    }

}

Edge * Node::FindEdge( Node* n )
{
    for ( int k = 0; k < (int)edgeVec.size(); k++ )
    {
        Node* ne0 = edgeVec[k]->n0;
        Node* ne1 = edgeVec[k]->n1;

        if ( ( ne0 == this && ne1 == n ) || ( ne0 == n && ne1 == this ) )
        {
            return edgeVec[k];
        }
    }
    return NULL;
}

bool Node::AllInteriorConnectedFaces()
{
    vector< Face* > fvec;
    GetConnectFaces( fvec );
    for ( int i = 0 ; i < ( int )fvec.size() ; i++ )
    {
        if ( !fvec[i]->deleteFlag )
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

    vec2d close_uw = surfPtr->ClosestUW( movePnt, moveUW.x(),  moveUW.y() );

    uw = uw + ( close_uw - uw ) * 0.1;
    pnt = surfPtr->CompPnt( uw.x(), uw.y() );

}

void Node::AreaWeightedLaplacianSmooth( Surf* surfPtr )
{
    vector< Face* > connectFaces;
    GetConnectFaces( connectFaces );

    vector< double > areas;
    areas.resize( connectFaces.size() );

    double sum_area = 0.0;
    for ( int i = 0 ; i < ( int )connectFaces.size() ; i++ )
    {
        areas[i] = connectFaces[i]->Area();
        sum_area += areas[i];
    }

    if ( sum_area < 1.0e-12 )
    {
        return;
    }

    vec3d movePnt = vec3d( 0, 0, 0 );
    vec2d moveUW  = vec2d( 0, 0 );
    double k2 = 1.0 / ( 3.0 * sum_area );
    for ( int i = 0 ; i < ( int )connectFaces.size() ; i++ )
    {
        double k = k2 * areas[i];
        movePnt = movePnt + ( connectFaces[i]->n0->pnt + connectFaces[i]->n1->pnt + connectFaces[i]->n2->pnt ) * k;
        moveUW = moveUW + ( connectFaces[i]->n0->uw + connectFaces[i]->n1->uw + connectFaces[i]->n2->uw ) * k;
    }

    // TODO:  This routine calculates an area weighted smoothed point to high precision.
    // it then moves 1/10th the way from the old point to the new point (in UW terms).
    // This is likely a remnant from the v2 code that searched much less precisely.
    // Consider a (much) less precise search, discarding (or tuning) the 1/10th lag, or
    // performing the 1/10th step in x,y,z space before performing the surface search.
    // Profiling shows this routine is one of the most expensive parts of Remesh.
    vec2d close_uw = surfPtr->ClosestUW( movePnt, moveUW.x(),  moveUW.y() );

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
    vector< Face* > connectFaces;
    GetConnectFaces( connectFaces );

    if ( ( int )connectFaces.size() < 3 )
    {
        return;
    }

    double worst_qual = 0.0;
    Face* worst_face = NULL;
    for ( int i = 0 ; i < ( int )connectFaces.size() ; i++ )
    {
        double q = connectFaces[i]->ComputeCosSmallAng();
        if ( q > worst_qual )
        {
            worst_qual = q;
            worst_face  = connectFaces[i];
        }
    }

    //==== Good Faces -> Don't Bother ====//
    //if ( worst_qual < 0.707 )
    //  return;

    if ( worst_face )
    {
        vec3d orig_pos = pnt;
        Edge* far_edge = worst_face->FindEdgeWithout( this );

        //==== Find Target Pos ====//
        vec3d proj = proj_pnt_on_ray( far_edge->n0->pnt, far_edge->n1->pnt, orig_pos );
        vec3d dir = orig_pos - proj;
        dir.normalize();

        double len = 0.866 * dist( far_edge->n0->pnt, far_edge->n1->pnt );
        vec3d target_pos = ( far_edge->n0->pnt + far_edge->n1->pnt ) * 0.5 + dir * len;

        pnt = pnt + ( target_pos - pnt ) * 0.02;            // Move 1% Towards Target

        bool move_back = false;
        for ( int i = 0 ; i < ( int )connectFaces.size() ; i++ )
        {
            double q = connectFaces[i]->ComputeCosSmallAng();
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
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

bool Edge::SetFace( Face* f )
{
    if ( f0 && f1 )
    {
        printf( "Edge: More Than 2 Faces %16.10f %16.10f %16.10f\n",
//          f0->ComputeQual(), f1->ComputeQual(), f->ComputeQual() );
                f0->Area(), f1->Area(), f->Area() );
        Face* badface = f;
        if ( f0->Area() < badface->Area() )
        {
            f0 = badface;
        }
        if ( f1->Area() < badface->Area() )
        {
            f1 = badface;
        }

        printf( "     vertex %f %f %f\n", f0->n0->pnt.x(), f0->n0->pnt.y(), f0->n0->pnt.z() );
        printf( "     vertex %f %f %f\n", f0->n1->pnt.x(), f0->n1->pnt.y(), f0->n1->pnt.z() );
        printf( "     vertex %f %f %f\n", f0->n2->pnt.x(), f0->n2->pnt.y(), f0->n2->pnt.z() );

        printf( "     vertex %f %f %f\n", f1->n0->pnt.x(), f1->n0->pnt.y(), f1->n0->pnt.z() );
        printf( "     vertex %f %f %f\n", f1->n1->pnt.x(), f1->n1->pnt.y(), f1->n1->pnt.z() );
        printf( "     vertex %f %f %f\n", f1->n2->pnt.x(), f1->n2->pnt.y(), f1->n2->pnt.z() );

        printf( "     vertex %f %f %f\n", f->n0->pnt.x(), f->n0->pnt.y(), f->n0->pnt.z() );
        printf( "     vertex %f %f %f\n", f->n1->pnt.x(), f->n1->pnt.y(), f->n1->pnt.z() );
        printf( "     vertex %f %f %f\n", f->n2->pnt.x(), f->n2->pnt.y(), f->n2->pnt.z() );

        printf( "   Face = %f %f %f\n        %f %f %f\n        %f %f %f \n",
                badface->n0->pnt.x(), badface->n0->pnt.y(), badface->n0->pnt.z(),
                badface->n1->pnt.x(), badface->n1->pnt.y(), badface->n1->pnt.z(),
                badface->n2->pnt.x(), badface->n2->pnt.y(), badface->n2->pnt.z() );

        printf( "   Border %d %d %d\n", badface->e0->border, badface->e1->border, badface->e2->border );


        return false;
    }
    if ( f0 )
    {
        f1 = f;
    }
    else
    {
        f0 = f;
    }

    return true;
}

void Edge::RemoveFace( Face* f )
{
    if ( f0 == f )
    {
        f0 = NULL;
    }
    else
    {
        f1 = NULL;
    }
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

bool Edge::ContainsNode( Node* in )
{
    if ( in == n0 || in == n1 )
    {
        return true;
    }

    return false;
}

Face* Edge::OtherFace( Face* f )
{
    if ( f == f0 )
    {
        return f1;
    }
    else if ( f == f1 )
    {
        return f0;
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

void Edge::ReplaceFace( Face* f, Face* replace_f )
{
    if ( f0 == f )
    {
        f0 = replace_f;
    }
    else if ( f1 == f )
    {
        f1 = replace_f;
    }
}

bool Edge::BothAdjoiningFacesInterior()
{
    if (( f0 && f0->deleteFlag ) || ( f0 == NULL ) )
        if (( f1 && f1->deleteFlag ) || ( f1 == NULL ) )
        {
            return true;
        }
    return false;
}

void Edge::NodeForgetEdge()
{
    n0->RemoveConnectEdge( this );
    n1->RemoveConnectEdge( this );
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
Face::Face()
{
    m_DeleteMeFlag = false;
    debugFlag = false;
    n0 = n1 = n2 = n3 = NULL;
    e0 = e1 = e2 = e3 = NULL;
    deleteFlag = false;
    rgb[0] = rgb[1] = rgb[2] = 0;
}

Face::Face( Node* nn0, Node* nn1, Node* nn2, Edge* ee0, Edge* ee1, Edge* ee2 )
{
    m_DeleteMeFlag = false;
    debugFlag = false;
    SetNodesEdges( nn0, nn1, nn2, ee0, ee1, ee2 );
    deleteFlag = false;
}

Face::Face( Node* nn0, Node* nn1, Node* nn2, Node* nn3, Edge* ee0, Edge* ee1, Edge* ee2, Edge* ee3 )
{
    m_DeleteMeFlag = false;
    debugFlag = false;
    SetNodesEdges( nn0, nn1, nn2, nn3, ee0, ee1, ee2, ee3 );
    deleteFlag = false;
}

Face::~Face()
{
}

void Face::SetNodesEdges( Node* nn0, Node* nn1, Node* nn2, Edge* ee0, Edge* ee1, Edge* ee2 )
{
    n0 = nn0;
    n1 = nn1;
    n2 = nn2;
    n3 = NULL;
    e0 = ee0;
    e1 = ee1;
    e2 = ee2;
    e3 = NULL;
}

void Face::SetNodesEdges( Node* nn0, Node* nn1, Node* nn2, Node* nn3, Edge* ee0, Edge* ee1, Edge* ee2, Edge* ee3 )
{
    n0 = nn0;
    n1 = nn1;
    n2 = nn2;
    n3 = nn3;
    e0 = ee0;
    e1 = ee1;
    e2 = ee2;
    e3 = ee3;
}

Edge* Face::FindEdge( Node* nn0, Node* nn1 )
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
    if ( e3 )
    {
        if ( e3->n0 == nn0 && e3->n1 == nn1 )
        {
            return e3;
        }
        if ( e3->n0 == nn1 && e3->n1 == nn0 )
        {
            return e3;
        }
    }
    return NULL;
}

Edge* Face::FindEdgeWithout( Node* node_ptr )
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
    if ( e3 )
    {
        if ( e2->n0 != node_ptr && e2->n1 != node_ptr )
        {
            return e2;
        }
    }

    return NULL;
}

Edge* Face::FindLongEdge()
{
    if ( !e0 || !e1 || !e2 )
    {
        return NULL;
    }

    double dsqr0 = dist_squared( e0->n0->pnt, e0->n1->pnt );
    double dsqr1 = dist_squared( e1->n0->pnt, e1->n1->pnt );
    double dsqr2 = dist_squared( e2->n0->pnt, e2->n1->pnt );

    Edge * e = e0;

    if ( dsqr1 < dsqr0 )
    {
        e = e1;
        dsqr0 = dsqr1;
    }

    if ( dsqr2 < dsqr0 )
    {
        e = e2;
        dsqr0 = dsqr2;
    }

    if ( e3 )
    {
        double dsqr3 = dist_squared( e3->n0->pnt, e3->n1->pnt );
        if ( dsqr3 < dsqr0 )
        {
            e = e3;
        }
    }

    return e;
}

void Face::ReplaceNode( Node* curr_node, Node* replace_node )
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
    else if ( n3 == curr_node )
    {
        n3 = replace_node;
    }
    else
    {
        assert( 0 );
    }
}

void Face::ReplaceEdge( Edge* curr_edge, Edge* replace_edge )
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
    else if ( e3 == curr_edge )
    {
        e3 = replace_edge;
    }
    else
    {
        assert( 0 );
    }
}

double Face::ComputeTriQual()
{
    if ( n3 )
    {
        printf( "Attempt Tri quality calculation on Quad.\n" );
        // Force error in Address Sanitizer
        int *p = NULL;
        *p = 1;
    }
    return ComputeTriQual( n0, n1, n2 );
}

double Face::ComputeTriQual( Node* n0, Node* n1, Node* n2 )
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

double Face::ComputeCosSmallAng()
{
    double minang, ang0, ang1, ang2, ang3;
    if ( n3 )
    {
        ComputeCosAngles( n0, n1, n2, n3, &ang0, &ang1, &ang2, &ang3 );
        minang = max( ang0, max( ang1, max( ang2, ang3 ) ) );
    }
    else
    {
        ComputeCosAngles( n0, n1, n2, &ang0, &ang1, &ang2 );
        minang = max( ang0, max( ang1, ang2 ) );
    }

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

vec3d Face::Normal()
{
    return Normal( n0, n1, n2 );
}

vec3d Face::Normal( Node* n0, Node* n1, Node* n2 )
{
    return cross( n1->pnt - n0->pnt, n2->pnt - n0->pnt );
}



void Face::ComputeCosAngles( Node* n0, Node* n1, Node* n2, double* ang0, double* ang1, double* ang2 )
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

void Face::ComputeCosAngles( Node* n0, Node* n1, Node* n2, Node* n3, double* ang0, double* ang1, double* ang2, double* ang3 )
{
    double dsqr01 = dist_squared( n0->pnt, n1->pnt );
    double dsqr12 = dist_squared( n1->pnt, n2->pnt );
    double dsqr20 = dist_squared( n2->pnt, n0->pnt );

    double dsqr23 = dist_squared( n2->pnt, n3->pnt );
    double dsqr30 = dist_squared( n3->pnt, n0->pnt );

    double dsqr13 = dist_squared( n1->pnt, n3->pnt );

    double d01 = sqrt( dsqr01 );
    double d12 = sqrt( dsqr12 );
    //double d20 = sqrt( dsqr20 );
    double d23 = sqrt( dsqr23 );
    double d30 = sqrt( dsqr30 );
    //double d13 = sqrt( dsqr13 );

    *ang0 = ( -dsqr13 + dsqr01 + dsqr30 ) / ( 2.0 * d01 * d30 );
    *ang1 = ( -dsqr20 + dsqr01 + dsqr12 ) / ( 2.0 * d01 * d12 );
    *ang2 = ( -dsqr13 + dsqr12 + dsqr23 ) / ( 2.0 * d12 * d23 );
    *ang3 = ( -dsqr20 + dsqr30 + dsqr23 ) / ( 2.0 * d30 * d23 );
}

// XOR (^) of anything with itself will return zero.  So, by performing a bitwise XOR chain of all the pointers
// n0^n1^n2^a^b, a and b clobber their match among n0,n1,n2 leaving just the odd pointer out to be returned.
Node* Face::OtherNodeTri( Node* a, Node* b )
{
    if ( n3 )
    {
        printf( "Attempt OtherNodeTri on Quad.\n" );
        // Force error in Address Sanitizer
        int *p = NULL;
        *p = 1;
    }
    return (Node *) ((uintptr_t) n0 ^ (uintptr_t) n1 ^ (uintptr_t) n2 ^ (uintptr_t) a ^ (uintptr_t) b);
}

bool Face::Contains( Node* a, Node* b )
{
    if ( a == b )
    {
        return false;
    }

    if ( a == n0 || a == n1 || a == n2 )
    {
        if ( b == n0 || b == n1 || b == n2 )
        {
            return true;
        }
    }

    if ( n3 )
    {
        if ( a == n3 || b == n3 )
        {
            return true;
        }
    }

    return false;
}

bool Face::Contains( Edge* e )
{
    if ( e3 )
    {
        return ( e == e0 || e == e1 || e == e2 || e == e3 );
    }
    return ( e == e0 || e == e1 || e == e2 );
}

bool Face::CorrectOrder( Node* en0, Node* en1 )
{
    if ( en0 == n0 && en1 == n1 )
    {
        return true;
    }
    if ( en0 == n1 && en1 == n2 )
    {
        return true;
    }

    if ( !n3 ) // Triangle
    {
        if ( en0 == n2 && en1 == n0 )
        {
            return true;
        }
    }
    else
    {
        if ( en0 == n2 && en1 == n3 )
        {
            return true;
        }
        if ( en0 == n3 && en1 == n0 )
        {
            return true;
        }
    }

    return false;
}

double Face::Area()
{
    if ( !n3 )
    {
        return area( n0->pnt, n1->pnt, n2->pnt );
    }
    else
    {
        return area( n0->pnt, n1->pnt, n2->pnt ) + area( n0->pnt, n2->pnt, n3->pnt );
    }
}

void Face::ComputeCenterPnt( Surf* surfPtr, vec3d &cen, vec2d &uwcen ) const
{
    if ( !n3 )
    {
        uwcen = ( n0->uw + n1->uw + n2->uw ) * ( 1.0 / 3.0 );
        cen = ( n0->pnt + n1->pnt + n2->pnt ) * ( 1.0 / 3.0 );
    }
    else
    {
        uwcen = ( n0->uw + n1->uw + n2->uw + n3->uw ) * ( 1.0 / 4.0 );
        cen = ( n0->pnt + n1->pnt + n2->pnt + n3->pnt ) * ( 1.0 / 4.0 );
    }

    uwcen = surfPtr->ClosestUW( cen, uwcen[0], uwcen[1] );
    cen = surfPtr->CompPnt( uwcen[0], uwcen[1] );
}

vec3d Face::ComputeCenterPnt( Surf* surfPtr ) const
{
    vec2d avg_uw;
    vec3d avg_p;

    ComputeCenterPnt( surfPtr, avg_p, avg_uw );

    return avg_p;
}

vec3d Face::ComputeCenterNormal( Surf* surfPtr ) const
{
    vec2d avg_uw;

    if ( !n3 )
    {
        avg_uw = ( n0->uw + n1->uw + n2->uw ) * ( 1.0 / 3.0 );
    }
    else
    {
        avg_uw = ( n0->uw + n1->uw + n2->uw + n3->uw ) * ( 1.0 / 4.0 );
    }

    return surfPtr->CompNorm( avg_uw[0], avg_uw[1] );
}

void Face::LoadAdjFaces( int num_levels, set< Face* > & faceSet )
{
    Face* f;
    faceSet.insert( this );

    num_levels--;
    if ( num_levels <= 0 )
    {
        return;
    }

    if ( !e0->border )
    {
        f = e0->OtherFace( this );
        if ( f )
        {
            f->LoadAdjFaces( num_levels, faceSet );
        }
    }
    if ( !e1->border )
    {
        f = e1->OtherFace( this );
        if ( f )
        {
            f->LoadAdjFaces( num_levels, faceSet );
        }
    }
    if ( !e2->border )
    {
        f = e2->OtherFace( this );
        if ( f )
        {
            f->LoadAdjFaces( num_levels, faceSet );
        }
    }
    if ( e3 )
    {
        if ( !e3->border )
        {
            f = e3->OtherFace( this );
            if ( f )
            {
                f->LoadAdjFaces( num_levels, faceSet );
            }
        }
    }
}

void Face::AddBorderEdges( list< Edge* > &edgeList )
{
    if ( e0->OtherFace( this ) == NULL )
    {
        edgeList.push_back( e0 );
    }
    if ( e1->OtherFace( this ) == NULL )
    {
        edgeList.push_back( e1 );
    }
    if ( e2->OtherFace( this ) == NULL )
    {
        edgeList.push_back( e2 );
    }

    if ( e3 )
    {
        if ( e3->OtherFace( this ) == NULL )
        {
            edgeList.push_back( e3 );
        }
    }
}

void Face::BuildRemovalSet( set < Face* > &remFaces, set < Edge* > &remEdges, set < Node* > &remNodes )
{
    //==== Check Edges ====//
    if ( e0->BothAdjoiningFacesInterior() )
    {
        remEdges.insert( e0 );
    }
    if ( e1->BothAdjoiningFacesInterior() )
    {
        remEdges.insert( e1 );
    }
    if ( e2->BothAdjoiningFacesInterior() )
    {
        remEdges.insert( e2 );
    }

    if ( e3 )
    {
        if ( e2->BothAdjoiningFacesInterior() )
        {
            remEdges.insert( e3 );
        }
    }

    //==== Check Nodes ====//
    if ( n0->AllInteriorConnectedFaces() )
    {
        remNodes.insert( n0 );
    }
    if ( n1->AllInteriorConnectedFaces() )
    {
        remNodes.insert( n1 );
    }
    if ( n2->AllInteriorConnectedFaces() )
    {
        remNodes.insert( n2 );
    }

    if ( n3 )
    {
        if ( n3->AllInteriorConnectedFaces() )
        {
            remNodes.insert( n3 );
        }
    }

    remFaces.insert( this );
}

void Face::EdgeForgetFace()
{
    e0->ReplaceFace( this, NULL );
    e1->ReplaceFace( this, NULL );
    e2->ReplaceFace( this, NULL );
    if ( e3 )
    {
        e3->ReplaceFace( this, NULL );
    }
}

void Face::GetNodePts( vector <vec3d> &pts )
{
    pts.push_back( n0->pnt );
    pts.push_back( n1->pnt );
    pts.push_back( n2->pnt );
    if ( n3 )
    {
        pts.push_back( n3->pnt );
    }
}
