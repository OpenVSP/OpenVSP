//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// Face
//
//////////////////////////////////////////////////////////////////////

#include <cmath>


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
    if ( e )
    {
        for ( int i = 0; i < ( int ) edgeVec.size(); i++ ) //jrg not sure I need this check???
        {
            if ( e == edgeVec[ i ] )
            {
                return;
            }
        }
        edgeVec.push_back( e );
    }
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
    // Distinct connected faces are bounded by the edge count: each incident face is
    // shared by two of this node's edges, so there are at most edgeVec.size() of them.
    cfVec.reserve( edgeVec.size() );
    for ( int i = 0 ; i < ( int )edgeVec.size() ; i++ )
    {
        if( edgeVec[i] )
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
    return nullptr;
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

// Does this face face the same way as the surface it sits on?
//
// The same question Mesh::FaceReversed asks, with the surface's normal handed in rather than
// looked up, so that a caller trying several positions for one node pays for it once.
static bool FaceOutward( Face* f, const vec3d &nsurf, Surf* surfPtr )
{
    double dprod = dot( f->Normal(), nsurf );

    if ( surfPtr->GetFlipFlag() )
    {
        dprod = -dprod;
    }

    return dprod >= 0.0;
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
    double k2 = 1.0 / ( 3.0 * sum_area );
    for ( int i = 0 ; i < ( int )connectFaces.size() ; i++ )
    {
        if ( connectFaces[i]->n0 && connectFaces[i]->n1 && connectFaces[i]->n2 )
        {
            double k = k2 * areas[i];
            movePnt = movePnt + ( connectFaces[i]->n0->pnt + connectFaces[i]->n1->pnt + connectFaces[i]->n2->pnt ) * k;
        }
    }

    // Where the ring wants this node is a question about space, so it is asked in space: the
    // area weighted average of the surrounding triangles' corners.  That point is not on the
    // surface, and the node has to stay on it, so the node steps toward it along the surface.
    //
    // The step is a single linear one, taken through the surface's own derivatives.  A full
    // nonlinear projection of the same target converges to a point that only a tenth of a
    // relaxation step is taken toward anyway, and the next pass redoes it.
    //
    // Averaging in the surface's parameters and skipping the step entirely is cheaper still, and
    // holds up wherever a step in u moves about as far in space as the next one does.  Where it
    // does not hold up is a cap whose patch has collapsed to a line along part of its u range:
    // half the parameter range there covers almost no surface, so a parametric midpoint sits
    // nowhere near the middle of anything, and the mesh at the base of such a cap comes out
    // skewed.
    vec2d target = uw;
    surfPtr->GetSurfCore()->TangentStep( target.v[0], target.v[1], movePnt );

    // Keep the step inside the ring the node already sits in.
    //
    // Nothing bounds the linear solve on its own.  Where the patch has collapsed the two
    // parametric directions stop being independent, and a solve asked to reach a point in
    // space can answer with an arbitrarily long step along the direction that has almost no
    // length in space.  The direction is still the right one; only the distance is not to be
    // trusted, so it is held to the distance the node's own neighbours already are.
    //
    // Without it a node thrown clear of its own ring is clamped onto the edge of the patch,
    // where it can only do harm, and the trials below would spend themselves halving a step
    // that was far too long to begin with.  It bites seldom and gently.
    double rmax = 0.0;
    for ( int i = 0 ; i < ( int )connectFaces.size() ; i++ )
    {
        if ( connectFaces[i]->n0 && connectFaces[i]->n1 && connectFaces[i]->n2 )
        {
            rmax = std::max( rmax, dist( connectFaces[i]->n0->uw, uw ) );
            rmax = std::max( rmax, dist( connectFaces[i]->n1->uw, uw ) );
            rmax = std::max( rmax, dist( connectFaces[i]->n2->uw, uw ) );
        }
    }

    double step = dist( target, uw );

    if ( step > rmax && step > 0.0 )
    {
        target = uw + ( target - uw ) * ( rmax / step );
    }

    // What the faces around the node look like before it moves.
    //
    // A face is fit to keep if it faces the same way as the surface and still has a shape.
    // Both are recorded now so that each trial position can be judged against the state the
    // node found, rather than against some absolute standard the mesh may not have met.
    //
    // One surface normal is read, at the node itself, and stood for the whole ring: the faces
    // around a node cover a patch small enough that they all face much the same way, and this
    // is a question about sign, not about angle.  Reading it per face, at each face's centre,
    // costs six surface evaluations a node where the pass as a whole wants one.  It is read
    // before the move and reused across the trials -- over a step this small it is the face
    // that turns over, not the surface underneath it.
    vec3d nsurf = surfPtr->CompNorm( uw.x(), uw.y() );

    vector< bool > wasgood( connectFaces.size() );

    for ( int i = 0 ; i < ( int )connectFaces.size() ; i++ )
    {
        // A face short of a corner is judged on nothing and vetoes nothing.  The loops above
        // already decline to take such a face's centroid; asking it for a normal or an angle
        // would read through the corner that is not there.
        wasgood[i] = connectFaces[i]->n0 && connectFaces[i]->n1 && connectFaces[i]->n2 &&
                     FaceOutward( connectFaces[i], nsurf, surfPtr ) &&
                     !connectFaces[i]->Degenerate();
    }

    // Take as much of the step as the surrounding mesh can stand.
    //
    // The direction is worth keeping but the distance is not worth insisting on.  A node that
    // oversteps drags a triangle inside out, or squeezes one to a splinter, and either has to
    // be collapsed away afterwards; it is that collapsing, not the skewed triangles
    // themselves, that leaves the assembled mesh with edges held by more than two triangles.
    //
    // So each trial position is put to the ring, and if any face that was fit to keep no
    // longer is, the step is halved and asked again.  The node still moves -- it moves as far
    // as it can without ruining anything, which is what its neighbours are entitled to.
    //
    // It is held still only if even the smallest trial spoils something, and then only for
    // this pass.  The next pass asks again from wherever the neighbours have since moved to,
    // so nothing is frozen; it is merely asked to wait its turn.
    vec2d uw0 = uw;
    vec3d pnt0 = pnt;
    double frac = 0.1;

    for ( int trial = 0 ; trial < 5 ; trial++ )
    {
        uw = uw0 + ( target - uw0 ) * frac;
        pnt = surfPtr->CompPnt( uw.x(), uw.y() );

        bool turned = false;
        for ( int i = 0 ; i < ( int )connectFaces.size() && !turned ; i++ )
        {
            // A face that was already inside out is not held against this node.  It is not
            // this node's doing, and standing still would only leave it that way.
            if ( wasgood[i] && ( !FaceOutward( connectFaces[i], nsurf, surfPtr ) ||
                                 connectFaces[i]->Degenerate() ) )
            {
                turned = true;
            }
        }

        if ( !turned )
        {
            return;
        }

        frac = frac * 0.5;
    }

    uw = uw0;
    pnt = pnt0;
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
    Face* worst_face = nullptr;
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
        vec3d proj = proj_pnt_on_line( far_edge->n0->pnt, far_edge->n1->pnt, orig_pos );
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

// Take on a face, unless the edge already holds the two it may have.  The third face is the
// one that does not belong, so the edge keeps the pair it has: swapping one of them out for
// the newcomer would leave a face that is part of the mesh with no edge pointing at it.
bool Edge::SetFace( Face* f )
{
    if ( f0 && f1 )
    {
        printf( "Edge: More Than 2 Faces\n" );
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

// Forget the face given, and only that one.  An edge asked to forget a face it never held
// keeps the face it does have -- which matters where a surface is laid against itself and the
// mesher builds the same triangle twice, since an edge left pointing at a removed face reads
// it after the next DumpGarbage.  ReplaceFace, just below, checks both the same way.
void Edge::RemoveFace( Face* f )
{
    if ( f0 == f )
    {
        f0 = nullptr;
    }
    else if ( f1 == f )
    {
        f1 = nullptr;
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
    if ( !f || !f0 || !f1 )
    {
        return nullptr;
    }

    if ( f == f0 )
    {
        return f1;
    }
    else if ( f == f1 )
    {
        return f0;
    }

    return nullptr;
}

Node* Edge::OtherNode( Node* n )
{
    if ( !n || !n0 || !n1 )
    {
        return nullptr;
    }

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

    return nullptr;
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
    if (( f0 && f0->deleteFlag ) || ( f0 == nullptr ) )
        if (( f1 && f1->deleteFlag ) || ( f1 == nullptr ) )
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
    n0 = n1 = n2 = n3 = nullptr;
    e0 = e1 = e2 = e3 = nullptr;
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
    n3 = nullptr;
    e0 = ee0;
    e1 = ee1;
    e2 = ee2;
    e3 = nullptr;
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
    if ( e0 )
    {
        if ( e0->n0 == nn0 && e0->n1 == nn1 )
        {
            return e0;
        }
        if ( e0->n0 == nn1 && e0->n1 == nn0 )
        {
            return e0;
        }
    }
    if ( e1 )
    {
        if ( e1->n0 == nn0 && e1->n1 == nn1 )
        {
            return e1;
        }
        if ( e1->n0 == nn1 && e1->n1 == nn0 )
        {
            return e1;
        }
    }
    if ( e2 )
    {
        if ( e2->n0 == nn0 && e2->n1 == nn1 )
        {
            return e2;
        }
        if ( e2->n0 == nn1 && e2->n1 == nn0 )
        {
            return e2;
        }
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
    return nullptr;
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
        if ( e3->n0 != node_ptr && e3->n1 != node_ptr )
        {
            return e3;
        }
    }

    return nullptr;
}

Edge* Face::FindShortEdge()
{
    Edge* e0a = dynamic_cast< Edge* > ( e0 );
    Edge* e1a = dynamic_cast< Edge* > ( e1 );
    Edge* e2a = dynamic_cast< Edge* > ( e2 );
    Edge* e3a = dynamic_cast< Edge* > ( e3 );

    if ( !e0a || !e1a || !e2a )
    {
        return nullptr;
    }

    if ( !e0a->n0 || !e1a->n0 || !e2a->n0 || !e0a->n1 || !e1a->n1 || !e2a->n1 )
    {
        return nullptr;
    }

    double dsqr0 = dist_squared( e0a->n0->pnt, e0a->n1->pnt );
    double dsqr1 = dist_squared( e1a->n0->pnt, e1a->n1->pnt );
    double dsqr2 = dist_squared( e2a->n0->pnt, e2a->n1->pnt );

    Edge * e = e0a;

    if ( dsqr1 < dsqr0 )
    {
        e = e1a;
        dsqr0 = dsqr1;
    }

    if ( dsqr2 < dsqr0 )
    {
        e = e2a;
        dsqr0 = dsqr2;
    }

    if ( e3a )
    {
        if ( e3a->n0 && e3a->n1 )
        {
            double dsqr3 = dist_squared( e3a->n0->pnt, e3a->n1->pnt );
            if ( dsqr3 < dsqr0 )
            {
                e = e3a;
            }
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

// Below this smallest angle a triangle has no usable shape left: no normal worth reading and
// nothing downstream able to orient it.  The cutoff sits far below anything the mesher aims
// for, so this catches only faces that are past saving and leaves ordinary poor ones to be
// improved by smoothing and swapping.
static const double MIN_TRI_ANGLE = 0.5 * M_PI / 180.0;

bool Face::Degenerate()
{
    if ( n3 )
    {
        return false;                       // quads are not this routine's business
    }

    return ComputeTriQual() < MIN_TRI_ANGLE;
}

double Face::ComputeTriQual()
{
    if ( n3 )
    {
        printf( "Attempt Tri quality calculation on Quad.\n" );
        // Force error in Address Sanitizer
        int *p = nullptr;
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

    // Two corners in the same place leave the angles undefined.  Report the triangle as
    // closed flat rather than dividing by zero: a NaN compares false against every
    // threshold, so the worst triangle there is would pass every test that looks for one.
    if ( d01 <= 0.0 || d12 <= 0.0 || d20 <= 0.0 )
    {
        *ang0 = 1.0;
        *ang1 = 1.0;
        *ang2 = 1.0;
        return;
    }

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
        int *p = nullptr;
        *p = 1;
        return nullptr;
    }

    if ( !a || !b || !n0 || !n1 || !n2 )
    {
        return nullptr;
    }

    // The cancelling only works if a and b are two different corners of this face.  Given
    // anything else the XOR yields an address built out of three unrelated pointers, and
    // every caller reads through what comes back.
    if ( a == b ||
         ( a != n0 && a != n1 && a != n2 ) ||
         ( b != n0 && b != n1 && b != n2 ) )
    {
        assert( false );
        return nullptr;
    }

    return (Node *) ((uintptr_t) n0 ^ (uintptr_t) n1 ^ (uintptr_t) n2 ^ (uintptr_t) a ^ (uintptr_t) b);
}

bool Face::Contains( Node* a, Node* b )
{
    if ( a == b )
    {
        return false;
    }

    if ( n3 )
    {
        if ( a == n0 || a == n1 || a == n2 || a == n3 )
        {
            if ( b == n0 || b == n1 || b == n2 || b == n3 )
            {
                return true;
            }
        }
    }
    else
    {
        if ( a == n0 || a == n1 || a == n2 )
        {
            if ( b == n0 || b == n1 || b == n2 )
            {
                return true;
            }
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
    if ( !n0 || !n1 || !n2 )
    {
        return 0.0;
    }

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

void Face::AddBorderNodes( vector< Node* > &nodeVec )
{
    if ( e0->OtherFace( this ) == nullptr )
    {
        if ( e0->n0 )
        {
            nodeVec.push_back( e0->n0 );
        }
        if ( e0->n1 )
        {
            nodeVec.push_back( e0->n1 );
        }
    }
    if ( e1->OtherFace( this ) == nullptr )
    {
        if ( e1->n0 )
        {
            nodeVec.push_back( e1->n0 );
        }
        if ( e1->n1 )
        {
            nodeVec.push_back( e1->n1 );
        }
    }
    if ( e2->OtherFace( this ) == nullptr )
    {
        if ( e2->n0 )
        {
            nodeVec.push_back( e2->n0 );
        }
        if ( e2->n1 )
        {
            nodeVec.push_back( e2->n1 );
        }
    }

    if ( e3 )
    {
        if ( e3->OtherFace( this ) == nullptr )
        {
            if ( e3->n0 )
            {
                nodeVec.push_back( e3->n0 );
            }
            if ( e3->n1 )
            {
                nodeVec.push_back( e3->n1 );
            }
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
        if ( e3->BothAdjoiningFacesInterior() )
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
    if ( e0 )
    {
        e0->ReplaceFace( this, nullptr );
    }
    if ( e1 )
    {
        e1->ReplaceFace( this, nullptr );
    }
    if ( e2 )
    {
        e2->ReplaceFace( this, nullptr );
    }
    if ( e3 )
    {
        e3->ReplaceFace( this, nullptr );
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

void Face::WriteSTL( FILE* file_id )
{
    vec3d& p0 = n0->pnt;
    vec3d& p1 = n1->pnt;
    vec3d& p2 = n2->pnt;
    vec3d v01 = p1 - p0;
    vec3d v12 = p2 - p1;
    vec3d norm = cross( v01, v12 );
    norm.normalize();

    fprintf( file_id, " facet normal  %2.10le %2.10le %2.10le\n", norm.x(), norm.y(), norm.z() );
    fprintf( file_id, "   outer loop\n" );

    fprintf( file_id, "     vertex %2.10le %2.10le %2.10le\n", p0.x(), p0.y(), p0.z() );
    fprintf( file_id, "     vertex %2.10le %2.10le %2.10le\n", p1.x(), p1.y(), p1.z() );
    fprintf( file_id, "     vertex %2.10le %2.10le %2.10le\n", p2.x(), p2.y(), p2.z() );

    fprintf( file_id, "   endloop\n" );
    fprintf( file_id, " endfacet\n" );

    if ( n3 ) // Split quad and write additional tri.
    {
        vec3d& p3 = n3->pnt;
        vec3d v23 = p3 - p2;
        vec3d v30 = p0 - p3;
        norm = cross( v23, v30 );
        norm.normalize();

        fprintf( file_id, " facet normal  %2.10le %2.10le %2.10le\n", norm.x(), norm.y(), norm.z() );
        fprintf( file_id, "   outer loop\n" );

        fprintf( file_id, "     vertex %2.10le %2.10le %2.10le\n", p0.x(), p0.y(), p0.z() );
        fprintf( file_id, "     vertex %2.10le %2.10le %2.10le\n", p2.x(), p2.y(), p2.z() );
        fprintf( file_id, "     vertex %2.10le %2.10le %2.10le\n", p3.x(), p3.y(), p3.z() );

        fprintf( file_id, "   endloop\n" );
        fprintf( file_id, " endfacet\n" );
    }
}

bool SimpFace::CheckDegen()
{
    if ( ind0 == ind1 ||
         ind0 == ind2 ||
         ind0 == ind3 ||
         ind1 == ind2 ||
         ind1 == ind3 ||
         ind2 == ind3 )
    {
        return true;
    }

    return false;
}
