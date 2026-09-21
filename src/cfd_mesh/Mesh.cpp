//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// Mesh
//
//////////////////////////////////////////////////////////////////////

#include "Mesh.h"

#include <map>
#include <set>
#include "Surf.h"
#include "PntNodeMerge.h"
#include "VspUtil.h"
#include <triangle.h>
#include <triangle_api.h>
#include "delabella.h"
#include "SurfaceIntersectionMgr.h"
#include <algorithm>
#include <numeric>
#include <random>

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

    m_Surf = nullptr;
    m_GridDensity = nullptr;
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

    if ( n->edgeVec.empty() )
    {
        return;
    }

    // Only the shortest target in the star matters -- every other edge is capped against it.
    // Finding it is a pass over half a dozen pointers.
    //
    // The shortest edge is left alone either way: the growth ratio is greater than one, so
    // its own target is never above the limit it sets.
    double minlen = n->edgeVec[0]->target_len;

    for ( int i = 1; i < ( int )n->edgeVec.size(); i++ )
    {
        if ( n->edgeVec[i]->target_len < minlen )
        {
            minlen = n->edgeVec[i]->target_len;
        }
    }

    double limitlen = minlen * m_GridDensity->m_GrowRatio;

    for ( int i = 0; i < ( int )n->edgeVec.size(); i++ )
    {
        if ( n->edgeVec[i]->target_len > limitlen )
        {
            n->edgeVec[i]->target_len = limitlen;
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

    // Neither step may switch itself off for the rest of the pass.  The two steps feed each other
    // -- splitting a long edge makes short ones and collapsing a short edge makes long ones -- so a
    // round that finds nothing for one step says nothing about the rounds after it.
    //
    // The loop stops early only at a real fixed point: neither step has anything to do in the same
    // round.
    for ( int i = 0 ; i < 20 ; i++ )
    {
        num_split = Split( 1 );
        num_collapse = Collapse( 1 );

        if ( num_split == 0 && num_collapse == 0 )
        {
            break;
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
    //==== Use nanoflann (via PntNodeMerge) to group coincident points ====//
    // simpPntVec holds one copy of each face corner, so shared vertices appear as
    // exact duplicates.  IndexPntNodes builds a kd-tree, assigns every duplicate the
    // same representative, and numbers the representatives 0..m_NumUsedPts-1.
    PntNodeCloud pnCloud;
    pnCloud.AddPntNodes( simpPntVec );
    IndexPntNodes( pnCloud, PT_MERGE_TOL );

    //==== Remap face indices to the merged (used) point indices ====//
    for ( int i = 0 ; i < ( int )simpFaceVec.size() ; i++ )
    {
        simpFaceVec[i].ind0 = pnCloud.GetNodeUsedIndex( simpFaceVec[i].ind0 );
        simpFaceVec[i].ind1 = pnCloud.GetNodeUsedIndex( simpFaceVec[i].ind1 );
        simpFaceVec[i].ind2 = pnCloud.GetNodeUsedIndex( simpFaceVec[i].ind2 );
        if ( simpFaceVec[i].m_isQuad )
        {
            simpFaceVec[i].ind3 = pnCloud.GetNodeUsedIndex( simpFaceVec[i].ind3 );
        }
    }

    //==== Reduce Point and UW Vec ====//
    // UsedNode( i ) is true only for the representative of each group.  Walking i in
    // ascending order visits representatives in the same order IndexPntNodes numbered
    // them, so the k'th kept point lands at used index k.
    vector< vec3d > rePntVec;
    vector< vec2d > reUWVec;
    rePntVec.reserve( pnCloud.m_NumUsedPts );
    reUWVec.reserve( pnCloud.m_NumUsedPts );
    for ( int i = 0 ; i < ( int )simpPntVec.size() ; i++ )
    {
        if ( pnCloud.UsedNode( i ) )
        {
            rePntVec.push_back( simpPntVec[i] );
            reUWVec.push_back( simpUWPntVec[i] );
        }
    }

    simpPntVec = rePntVec;
    simpUWPntVec = reUWVec;
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
                // Filter with a multiply (len > 1.41 * target_len) so the per-edge
                // division is only paid for the few edges that are actually long.
                double len = ( *e )->GetLength();
                if ( len > 1.41 * ( *e )->target_len )
                {
                    double rat = len / ( *e )->target_len;
                    longEdges.emplace_back( pair< Edge*, double >( ( *e ), rat ) );
                }
            }
        }

        //==== Sort Matches By Length ====//
        sort( longEdges.begin(), longEdges.end(), LongEdgePairLengthCompare );

        // A tenth of the candidates, but never none: with fewer than ten candidates integer division
        // gives zero, and a step that does nothing reports that it has nothing to do.  Collapse takes
        // the same floor; the two steps feed each other, so a floor on only one tips the balance
        // towards that one.
        int num_split = longEdges.size() / 10;
        if ( num_split < 1 && !longEdges.empty() )
        {
            num_split = 1;
        }
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

// Wang 2006 gives the contraction parameter as Cc = 1/sqrt(2): an edge below this fraction
// of its target is one the mesher wants to collapse away.  Nothing should deliberately build
// one.
static const double CC_LENGTH_RATIO = 0.707;

// The angle below which a triangle is ill-shaped whatever its size.
static const double COLLAPSE_QUAL_ANGLE = 17.0 * M_PI / 180.0;

// Is this edge the shortest edge of an ill-shaped face, and how ill-shaped?  M_PI when it is
// nobody's shortest edge -- the answer that admits nothing.
//
// Only the shortest edge of a face is offered.  It is the one whose removal deletes the
// face; collapsing a longer one drags a whole neighbourhood about to fix one triangle.
static double ShortEdgeOfPoorFace( Edge* e )
{
    double worst = M_PI;
    double le = e->GetLength();

    Face* ff[2] = { e->f0, e->f1 };

    for ( int k = 0 ; k < 2 ; k++ )
    {
        Face* f = ff[k];

        if ( !f || f->m_DeleteMeFlag || f->IsQuad() )
        {
            continue;
        }

        double s0 = dist( f->n0->pnt, f->n1->pnt );
        double s1 = dist( f->n1->pnt, f->n2->pnt );
        double s2 = dist( f->n2->pnt, f->n0->pnt );

        if ( le > min( s0, min( s1, s2 ) ) )
        {
            continue;
        }

        double q = f->ComputeTriQual();

        if ( q < worst )
        {
            worst = q;
        }
    }

    return worst;
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
            if ( *e )
            {
                if ( ValidCollapse( *e ) )
                {
                    double rat = ( *e )->GetLength() / ( *e )->target_len;

                    // Short for the size that was asked for, or short for the triangle it sits
                    // on.  Only the first of those was ever asked.
                    //
                    // The length test compares an edge against the target field and nothing
                    // else, so it can only see a triangle that is too small.  It cannot see one
                    // that is the right size and the wrong shape.  A cap -- a triangle whose
                    // apex has fallen onto the far side -- is exactly that: its height is far
                    // under its base, but where the target field is already fine, that height
                    // is not under target and no edge of it is ever offered to the collapse.
                    //
                    // Measurement on poormesh.vsp3: 462 of the 470 triangles left under five
                    // degrees have an angle over 160, and every one of them survived every pass
                    // untouched.  Swapping cannot reach them either -- accepting a flip that
                    // lowers the largest angle as well as one that raises the smallest changes
                    // the output not at all -- so the length test is the whole of the reason
                    // they stay.
                    //
                    // Wang 2006 selects on length because it assumes an isotropic starting
                    // mesh; the papers that deal with degenerate faces (Botsch and Kobbelt
                    // 2001) select on shape.  Asking both questions is the smaller change.
                    //
                    // The two are put on one scale so that one sorted list and one budget still
                    // serve: how short against target, or how flat against the angle below
                    // which a triangle is considered ill-shaped.  Both are fractions of the
                    // limit that admitted the edge, so the worst offender of either kind sorts
                    // to the front.
                    double score = rat;
                    bool candidate = false;

                    if ( rat < CC_LENGTH_RATIO )
                    {
                        candidate = true;
                    }
                    else
                    {
                        double qworst = ShortEdgeOfPoorFace( *e );

                        if ( qworst < COLLAPSE_QUAL_ANGLE )
                        {
                            candidate = true;
                            score = qworst / COLLAPSE_QUAL_ANGLE;
                        }
                    }

                    if ( candidate )
                    {
                        shortEdges.emplace_back( pair< Edge*, double >( ( *e ), score ) );
                    }
                }
            }
        }

        //==== Sort Matches By Length ====//
        sort( shortEdges.begin(), shortEdges.end(), ShortEdgePairLengthCompare );

        // A tenth of the candidates, but never none: below ten candidates integer division gives
        // zero, and a step that does nothing reports that it has nothing to do.
        int num_colapse = shortEdges.size() / 10;
        if ( num_colapse < 1 && !shortEdges.empty() )
        {
            num_colapse = 1;
        }
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

// Is this face wound against the surface it lies on?
bool Mesh::FaceReversed( Face* f )
{
    if ( !f )
    {
        return false;
    }

    vec3d nface = f->Normal();
    vec3d nsurf = f->ComputeCenterNormal( m_Surf );

    double dprod = dot( nface, nsurf );

    if ( m_Surf->GetFlipFlag() )
    {
        dprod = -dprod;
    }

    return dprod < 0.0;
}

bool Mesh::TriReversed( const vec3d &p0, const vec3d &p1, const vec3d &p2,
                        const vec2d &uw0, const vec2d &uw1, const vec2d &uw2 )
{
    vec3d nface = cross( p1 - p0, p2 - p0 );

    vec2d avg = ( uw0 + uw1 + uw2 ) * ( 1.0 / 3.0 );
    vec3d nsurf = m_Surf->CompNorm( avg[0], avg[1] );

    double dprod = dot( nface, nsurf );

    if ( m_Surf->GetFlipFlag() )
    {
        dprod = -dprod;
    }

    return dprod < 0.0;
}

// Collapse away the faces that are not fit to keep.
//
// A face is unfit if it faces the wrong way, or if it has been squeezed until it has no
// inside left.  Both are removed the same way, by collapsing one of the face's edges.
//
// Reversal alone does not catch every unfit face: a face that is squeezed flat and stops there
// never reverses, and one with no area has no meaningful normal for the reversal test to read.
// Such a face reaches the assembled mesh, where it cannot be oriented and shows up as an edge
// held by more than two triangles.  Asking about the face itself, rather than about which way
// it happens to point, catches both.
int Mesh::RemoveIllFormedFaces()
{
    int badcount = 0;

    vector < Face* > remFaces;

    list< Face* >::iterator f;
    for ( f = faceList.begin() ; f != faceList.end(); ++f )
    {
        bool rev = FaceReversed( *f );
        bool deg = ( *f )->Degenerate();

        if ( rev || deg )
        {
            remFaces.push_back( *f );

            badcount++;
        }
    }

    // Any of the face's edges will do to be rid of it, so all of them are offered.
    //
    // The shortest is asked first, because collapsing it disturbs the least.  The shortest alone
    // is not enough: a splinter lying along the edge of a tip cap has its short side on the border
    // itself, a border may not be collapsed, and its corners are pinned so smoothing cannot reach
    // it either.  Its other two edges usually run inward and collapse perfectly well.
    for ( int i = 0; i < ( int )remFaces.size(); i++ )
    {
        Face* fc = remFaces[i];

        if ( !fc || fc->m_DeleteMeFlag )
        {
            continue;
        }

        Edge* cand[5] = { fc->FindShortEdge(), fc->e0, fc->e1, fc->e2, fc->e3 };

        for ( int j = 0; j < 5; j++ )
        {
            if ( cand[j] && ValidCollapse( cand[j] ) && CollapseEdge( cand[j], true ) )
            {
                break;
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

Node* Mesh::AddNode( const vec3d &p, const vec2d &uw_in )
{
    Node* nptr = new Node( p, uw_in );
    nodeList.push_back( nptr );
    nptr->list_ptr = --nodeList.end();
    return nptr;
}

void Mesh::RemoveNode( Node* nptr )
{
    // Asked for twice where a collapse finds the same node at both ends of what it is
    // collapsing, as it does among coincident triangles.  The second erase would work from an
    // iterator that has already been used.  Edges and faces are dropped the same way.
    if ( nptr && !nptr->m_DeleteMeFlag )
    {
        garbageNodeVec.push_back( nptr );
        nodeList.erase( nptr->list_ptr );

        nptr->m_DeleteMeFlag = true;
    }
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
    return nullptr;
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
    if ( eptr && !eptr->m_DeleteMeFlag )
    {
        if ( eptr->n0 )
        {
            eptr->n0->RemoveConnectEdge( eptr );
        }
        if ( eptr->n1 )
        {
            eptr->n1->RemoveConnectEdge( eptr );
        }

        garbageEdgeVec.push_back( eptr );

        edgeList.erase( eptr->list_ptr );

        eptr->m_DeleteMeFlag = true;
    }
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
    return nullptr;
}

Face* Mesh::AddFace( Node* nn0, Node* nn1, Node* nn2, Edge* ee0, Edge* ee1, Edge* ee2 )
{
    Face* fptr = new Face( nn0, nn1, nn2, ee0, ee1, ee2 );
    faceList.push_back( fptr );
    fptr->list_ptr = --faceList.end();

    // Every operation that builds a face here hands it edges it has just made or just freed,
    // so an edge that will not take it means the mesh was already wrong.
    bool ok = ee0->SetFace( fptr );
    ok = ee1->SetFace( fptr ) && ok;
    ok = ee2->SetFace( fptr ) && ok;
    assert( ok );

    return fptr;
}

Face* Mesh::AddFace( Node* nn0, Node* nn1, Node* nn2, Node* nn3, Edge* ee0, Edge* ee1, Edge* ee2, Edge* ee3 )
{
    Face* fptr = new Face( nn0, nn1, nn2, nn3, ee0, ee1, ee2, ee3 );
    faceList.push_back( fptr );
    fptr->list_ptr = --faceList.end();

    bool ok = ee0->SetFace( fptr );
    ok = ee1->SetFace( fptr ) && ok;
    ok = ee2->SetFace( fptr ) && ok;
    ok = ee3->SetFace( fptr ) && ok;
    assert( ok );

    return fptr;
}

void Mesh::RemoveFace( Face* fptr )
{
    if ( fptr && ! fptr->m_DeleteMeFlag )
    {
        garbageFaceVec.push_back( fptr );
        faceList.erase( fptr->list_ptr );
        fptr->m_DeleteMeFlag = true;
        fptr->EdgeForgetFace();
    }
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

// A collapse may not leave a triangle worse than this.  Half a degree is what Face
// ::Degenerate already calls unfit, so anything at or under it is a face the mesher would
// immediately want to be rid of again.
static const double MIN_COLLAPSE_ANGLE = 0.5 * M_PI / 180.0;

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

    // Not the midpoint projected to the surface: across a tip that lands back on the side the
    // edge came from, and the split does not shorten anything.  See Surf::SplitUW.
    vec2d uws = m_Surf->SplitUW( n0->pnt, n1->pnt, n0->uw, n1->uw );
    vec3d ps  = m_Surf->CompPnt( uws.x(), uws.y() );

    // A split must not turn a face over.  CollapseEdge refuses a move that would, through
    // ValidNodeMove.  A reversed face is collapsed away by RemoveIllFormedFaces, which is how a
    // tip ends up with edges shared by more than two triangles.
    //
    // The four faces the split would build are checked before anything is created.  A split
    // that would turn one over does not happen; the edge stays as it is.
    {
        bool wouldreverse = false;

        if ( fa )
        {
            Node* na = fa->OtherNodeTri( n0, n1 );

            if ( na )
            {
                if ( TriReversed( n0->pnt, ps, na->pnt, n0->uw, uws, na->uw ) ||
                     TriReversed( n1->pnt, na->pnt, ps, n1->uw, na->uw, uws ) )
                {
                    wouldreverse = true;
                }
            }
        }

        if ( fb && !wouldreverse )
        {
            Node* nb = fb->OtherNodeTri( n0, n1 );

            if ( nb )
            {
                if ( TriReversed( n0->pnt, nb->pnt, ps, n0->uw, nb->uw, uws ) ||
                     TriReversed( n1->pnt, ps, nb->pnt, n1->uw, uws, nb->uw ) )
                {
                    wouldreverse = true;
                }
            }
        }

        if ( wouldreverse )
        {
            return;
        }
    }

    // A split must not manufacture an edge shorter than the size that was asked for.
    //
    // The criterion that chose this edge looked only at the edge being consumed.  The two
    // halves it becomes are bounded by that criterion -- an edge over Cs * target halves to
    // something at or above Cc * target -- but the edge from the new point to the apex of an
    // adjacent face is not.  Its length is the height of that face, which has nothing to do
    // with the base that was measured.
    //
    // Splitting the long base of a thin triangle therefore produces an edge far under target, and
    // splitting again halves the height once more, while that target never moves.
    //
    // The target field is graded, so the parent edge's own target is a good local scale and
    // costs nothing to reuse.
    {
        double shortest = CC_LENGTH_RATIO * edge->target_len;

        bool wouldbeshort = false;

        if ( fa )
        {
            Node* na = fa->OtherNodeTri( n0, n1 );

            if ( na && dist( na->pnt, ps ) < shortest )
            {
                wouldbeshort = true;
            }
        }

        if ( fb && !wouldbeshort )
        {
            Node* nb = fb->OtherNodeTri( n0, n1 );

            if ( nb && dist( nb->pnt, ps ) < shortest )
            {
                wouldbeshort = true;
            }
        }

        if ( wouldbeshort )
        {
            // The edge is genuinely too long, but bisecting it is the wrong answer for the
            // shape it sits on.  A thin triangle wants its long edge swapped away, not cut
            // in half.  SwapEdge only acts when it raises the smallest angle, so this either
            // improves the pair or leaves them alone.
            SwapEdge( edge );
            return;
        }
    }

    // What the split needs from each face, gathered before the mesh is touched.  Giving up
    // once a face has been removed and its two replacements not yet built leaves a hole with
    // a hanging node in it, which nothing downstream can repair.
    Node* na = nullptr;
    Edge* ea0 = nullptr;
    Edge* ea1 = nullptr;

    if ( fa )
    {
        na = fa->OtherNodeTri( n0, n1 );

        if ( !na )
        {
            return;
        }

        ea0 = fa->FindEdge( n0, na );
        ea1 = fa->FindEdge( na, n1 );

        if ( !ea0 || !ea1 )
        {
            return;
        }
    }

    Node* nb = nullptr;
    Edge* eb0 = nullptr;
    Edge* eb1 = nullptr;

    if ( fb )
    {
        nb = fb->OtherNodeTri( n0, n1 );

        if ( !nb )
        {
            return;
        }

        eb0 = fb->FindEdge( n0, nb );
        eb1 = fb->FindEdge( nb, n1 );

        if ( !eb0 || !eb1 )
        {
            return;
        }
    }

    Node* ns  = AddNode( ps, uws );
    Edge* es0 = AddEdge( n0, ns );
    Edge* es1 = AddEdge( ns, n1 );
    es0->ridge = edge->ridge;
    es1->ridge = edge->ridge;
    es0->border = edge->border; // Should be impossible.
    es1->border = edge->border; // Should be impossible.

    if ( fa )
    {
        Edge* ea = AddEdge( na, ns );

        ea0->RemoveFace( fa );
        ea1->RemoveFace( fa );

        CheckFace( AddFace( n0, ns, na, ea0, ea, es0 ) );
        CheckFace( AddFace( n1, na, ns, ea1, es1, ea ) );

        RemoveFace( fa );
    }

    if ( fb )
    {
        Edge* eb = AddEdge( ns, nb );

        eb0->RemoveFace( fb );
        eb1->RemoveFace( fb );

        CheckFace( AddFace( n0, nb, ns, es0, eb, eb0 ) );
        CheckFace( AddFace( n1, ns, nb, es1, eb1, eb ) );

        RemoveFace( fb );
    }

    RemoveEdge( edge );

    ComputeTargetEdgeLength( ns );
    LimitTargetEdgeLength( ns );

    // "Neighboring edge swapping is performed to improve the local configuration, in terms
    // of both approximation of the geometry and the element quality" -- Wang 2006, 5.1.
    //
    // Inserting a point can leave the edges around it badly connected, and waiting until the
    // end of the pass to swap lets twenty rounds of splitting build on the bad connection.
    // SwapEdge acts only when it raises the smallest angle of the pair, so this cannot make
    // the neighbourhood worse.
    for ( int i = 0; i < ( int )ns->edgeVec.size(); i++ )
    {
        Edge* e = ns->edgeVec[i];

        if ( !e || e->m_DeleteMeFlag )
        {
            continue;
        }

        // The edges opposite the new point are the ones whose connection it may have
        // spoiled; the edges meeting it were just built to fit.
        Face* ff[2] = { e->f0, e->f1 };

        for ( int k = 0; k < 2; k++ )
        {
            if ( !ff[k] || ff[k]->m_DeleteMeFlag )
            {
                continue;
            }

            Edge* opp = ff[k]->FindEdgeWithout( ns );

            if ( opp && !opp->m_DeleteMeFlag && !opp->border && !opp->ridge )
            {
                SwapEdge( opp );
            }
        }
    }
}

void Mesh::SwapEdge( Edge* edge )
{
    if ( !edge )
    {
        return;
    }

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

    // A flip replaces the shared edge with one joining the two opposite corners.  If those
    // two are already joined, the flip builds a SECOND edge between the same pair of nodes,
    // which is not a surface any more: the pair of edges bounds no area and the faces on
    // either side of it are shared three ways.  ThreeEdgesThreeFaces catches only the
    // valence-3 case, which is one instance of this and not the general one.
    {
        Node* sa = edge->f0->OtherNodeTri( edge->n0, edge->n1 );
        Node* sb = edge->f1->OtherNodeTri( edge->n0, edge->n1 );

        if ( !sa || !sb || sa == sb )
        {
            return;
        }

        for ( int i = 0; i < ( int )sa->edgeVec.size(); i++ )
        {
            Edge* ee = sa->edgeVec[i];
            if ( ee && !ee->m_DeleteMeFlag && ee->OtherNode( sa ) == sb )
            {
                return;
            }
        }
    }

    Node* n0 = edge->n0;
    Node* n1 = edge->n1;

    if ( !n0 || !n1 )
    {
        return;
    }

    if ( !fa->CorrectOrder( n0, n1 ) )
    {
        n0 = edge->n1;
        n1 = edge->n0;
    }

    Node* na = fa->OtherNodeTri( n0, n1 );
    Node* nb = fb->OtherNodeTri( n0, n1 );

    assert( na != nb );

    if ( !na || !nb )
    {
        return;
    }

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

    // The three tests below ask whether the swap would fold the surface over, by the angle
    // between face normals.  A face with no area has no normal to read and angle() reports
    // it as perfectly aligned with anything, so the tests would pass on nothing.  A face
    // that bad is one the swap is wanted for, and the pair it becomes was already required
    // to be better shaped than the pair it replaces, so let it through deliberately.
    if ( norma.mag() > 0.0 && normb.mag() > 0.0 )
    {
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
    }

    Edge* ea0 = fa->FindEdge( n0, na );
    Edge* ea1 = fa->FindEdge( na, n1 );
    Edge* eb0 = fb->FindEdge( n0, nb );
    Edge* eb1 = fb->FindEdge( nb, n1 );

    if ( !ea0 || !ea1 || !eb0 || !eb1 ) return;

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

    CheckFace( fa );
    CheckFace( fb );
}

bool Mesh::ThreeEdgesThreeFaces( Edge* edge )
{
    Node* n0 = edge->n0;
    Node* n1 = edge->n1;

    // Test the O(1) edge valence before building the (deduplicated, heap-allocating)
    // connected-face list.  Interior nodes have ~6 edges, so this skips GetConnectFaces
    // entirely for all but the rare valence-3 nodes.
    if ( n0->edgeVec.size() == 3 )
    {
        vector< Face* > f;
        n0->GetConnectFaces( f );
        if ( f.size() == 3 )
        {
            return true;
        }
    }

    if ( n1->edgeVec.size() == 3 )
    {
        vector< Face* > fvec1;
        n1->GetConnectFaces( fvec1 );
        if ( fvec1.size() == 3 )
        {
            return true;
        }
    }

    return false;
}


// Return the two edges of TRIANGLE face f other than skipedge (which must be one of f's
// three edges).  eatn0 is the returned edge that touches node n0; eatn1 is the other.
// This replaces two Face::FindEdge scans: the two wanted edges are identified by pointer
// comparison against skipedge (no Edge dereference), then ordered with a single membership
// test -- far fewer scattered Edge loads than matching endpoints in FindEdge.
//
// Triangle-only by construction: the "two other edges touching the apex" concept does not
// apply to a quad (four edges, no single apex).  ValidCollapse, the only caller, is itself
// tri-only (fa->OtherNodeTri crashes on a quad before we get here).  Still, if handed a quad
// (e3 set) or an edge that is not one of f's, return nullptrs so the caller bails safely
// rather than acting on a wrong edge.
static void OtherTriEdges( Face* f, Edge* skipedge, Node* n0, Edge*& eatn0, Edge*& eatn1 )
{
    eatn0 = nullptr;
    eatn1 = nullptr;

    if ( f->e3 ) // quad -- not handled here
    {
        return;
    }

    Edge* o0;
    Edge* o1;
    if ( f->e0 == skipedge )
    {
        o0 = f->e1;
        o1 = f->e2;
    }
    else if ( f->e1 == skipedge )
    {
        o0 = f->e0;
        o1 = f->e2;
    }
    else if ( f->e2 == skipedge )
    {
        o0 = f->e0;
        o1 = f->e1;
    }
    else // skipedge is not an edge of this face
    {
        return;
    }

    if ( o0 && ( o0->n0 == n0 || o0->n1 == n0 ) )
    {
        eatn0 = o0;
        eatn1 = o1;
    }
    else
    {
        eatn0 = o1;
        eatn1 = o0;
    }
}


bool Mesh::ValidCollapse( Edge* edge )
{
    if ( !edge )
    {
        return false;
    }

    if ( edge->m_DeleteMeFlag )
    {
        return false;
    }

    if ( edge->border || edge->ridge )
    {
        return false;
    }

    if ( !edge->n0 || !edge->n1 )
    {
        return false;
    }

    Node* n0 = edge->n0;
    Node* n1 = edge->n1;

    //////if ( edge->n0->fixed || edge->n1->fixed )
    //////  return false;
    if ( edge->n0->fixed && edge->n1->fixed )
    {
        return false;
    }

    Face* fa = edge->f0;
    Face* fb = edge->f1;

    if ( !fa || !fb )
    {
        return false;
    }

    if ( fa->m_DeleteMeFlag || fb->m_DeleteMeFlag )
    {
        return false;
    }

    // The two sides of an edge have to be two faces.  Where a surface has been laid against
    // itself the mesher can build the same triangle twice, and the pair share all three of
    // their edges; collapsing between them would drop each of those twice.
    if ( fa == fb )
    {
        return false;
    }

    Node* na = fa->OtherNodeTri( n0, n1 );
    Node* nb = fb->OtherNodeTri( n0, n1 );

    if ( !na || !nb )
    {
        return false;
    }

    if ( na == nb )
    {
        return false;
    }

    // The link condition.
    //
    // Contracting an edge is topology-preserving exactly when the vertices adjacent to BOTH
    // ends are precisely the vertices opposite the edge -- two of them for an interior edge,
    // one for a boundary edge.  Any further shared neighbour means the two vertex stars meet
    // somewhere other than along this edge, and merging the ends pinches the surface there:
    // a handle is cut, or two sheets are joined at a point, and the result is not a surface.
    //
    // The checks below this comment test particular configurations one and two faces out.  They
    // catch some instances of that and not the general case: a shared neighbour further around the
    // ring passes them and pinches the mesh.
    {
        int nshared = 0;

        for ( int i = 0; i < ( int )n0->edgeVec.size(); i++ )
        {
            Edge* ei = n0->edgeVec[i];
            if ( !ei || ei->m_DeleteMeFlag )
            {
                continue;
            }
            Node* vi = ei->OtherNode( n0 );
            if ( !vi || vi == n1 )
            {
                continue;
            }

            for ( int j = 0; j < ( int )n1->edgeVec.size(); j++ )
            {
                Edge* ej = n1->edgeVec[j];
                if ( !ej || ej->m_DeleteMeFlag )
                {
                    continue;
                }
                if ( ej->OtherNode( n1 ) == vi )
                {
                    nshared++;
                    break;
                }
            }
        }

        // fa and fb both exist here, so this is an interior edge and exactly two shared
        // neighbours are expected -- na and nb.
        if ( nshared != 2 )
        {
            return false;
        }
    }

    //==== Check 3 Faces in a Face Case =====//
    Edge* e0a;
    Edge* e1a;
    OtherTriEdges( fa, edge, n0, e0a, e1a );

    if ( !e0a || !e1a )
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

    Edge* e0b;
    Edge* e1b;
    OtherTriEdges( fb, edge, n0, e0b, e1b );

    if ( !e0b || !e1b )
    {
        return false;
    }

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

bool Mesh::ValidNodeMove( Node* nptr, const vec3d & move_to, Face* ignoreFace, Face* ignoreFace2 )
{
    int i;
    bool valid_flag = true;
    vector < Face* > faceVec;
    nptr->GetConnectFaces( faceVec );

    vector < vec3d > normals;
    normals.reserve( faceVec.size() );
    for ( i = 0 ; i < ( int )faceVec.size() ; i++ )
    {
        if ( faceVec[i] != ignoreFace && faceVec[i] != ignoreFace2 )
        {
            normals.push_back( faceVec[i]->Normal() );
        }
    }

    vec3d save_pos = nptr->pnt;
    nptr->pnt = move_to;

    vector < vec3d > move_normals;
    move_normals.reserve( normals.size() );
    for ( i = 0 ; i < ( int )faceVec.size() ; i++ )
    {
        if ( faceVec[i] != ignoreFace && faceVec[i] != ignoreFace2 )
        {
            move_normals.push_back( faceVec[i]->Normal() );
        }
    }

    for ( i = 0 ; i < ( int )normals.size() ; i++ )
    {
        // A face the move leaves with no area has no normal, and angle() reads that as no
        // turn at all.  Flattening a face onto a line is the move most worth refusing.
        if ( move_normals[i].mag() <= 0.0 )
        {
            valid_flag = false;
            break;
        }

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
    Edge* hedge = nullptr;
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

// Signed area of a triangle in the parametric domain.  Only the sign is used: if it changes
// across an operation, the triangle turned inside out and the mesh overlapped itself.
// Smallest angle of a triangle on three points, in radians.  Face::ComputeTriQual asks the
// same question of an existing face; this asks it of a face that does not exist yet.
static double TriMinAngle( const vec3d &p0, const vec3d &p1, const vec3d &p2 )
{
    double s[3];
    s[0] = dist( p1, p2 );
    s[1] = dist( p0, p2 );
    s[2] = dist( p0, p1 );

    if ( s[0] <= 0.0 || s[1] <= 0.0 || s[2] <= 0.0 )
    {
        return 0.0;
    }

    std::sort( s, s + 3 );

    // The smallest angle faces the shortest side.
    double cosv = ( s[1] * s[1] + s[2] * s[2] - s[0] * s[0] ) / ( 2.0 * s[1] * s[2] );

    if ( cosv > 1.0 )
    {
        cosv = 1.0;
    }
    if ( cosv < -1.0 )
    {
        cosv = -1.0;
    }

    return acos( cosv );
}

static double SignedUWArea( const vec2d &a, const vec2d &b, const vec2d &c )
{
    return 0.5 * ( ( b.x() - a.x() ) * ( c.y() - a.y() ) -
                   ( c.x() - a.x() ) * ( b.y() - a.y() ) );
}

double Mesh::CollapseConfigQuality( Edge* edge, const vec3d &pc, const vec2d &uwc, bool &flipped, double &qbefore )
{
    flipped = false;
    qbefore = M_PI;

    Node* n0 = edge->n0;
    Node* n1 = edge->n1;
    Face* fa = edge->f0;
    Face* fb = edge->f1;

    double worst = M_PI;

    for ( int side = 0; side < 2; side++ )
    {
        Node* nn = n0;
        if ( side == 1 )
        {
            nn = n1;
        }

        vector < Face* > faceVec;
        nn->GetConnectFaces( faceVec );

        for ( int i = 0; i < ( int )faceVec.size(); i++ )
        {
            Face* f = faceVec[i];

            // These two vanish in the collapse, so their shape afterwards is not a question.
            if ( !f || f == fa || f == fb || f->m_DeleteMeFlag || f->n3 )
            {
                continue;
            }

            Node* fn[3] = { f->n0, f->n1, f->n2 };

            if ( !fn[0] || !fn[1] || !fn[2] )
            {
                continue;
            }

            vec3d p[3];
            vec2d uw[3];

            for ( int k = 0; k < 3; k++ )
            {
                if ( fn[k] == n0 || fn[k] == n1 )
                {
                    p[k] = pc;
                    uw[k] = uwc;
                }
                else
                {
                    p[k] = fn[k]->pnt;
                    uw[k] = fn[k]->uw;
                }
            }

            double abefore = SignedUWArea( fn[0]->uw, fn[1]->uw, fn[2]->uw );
            double aafter = SignedUWArea( uw[0], uw[1], uw[2] );

            if ( abefore * aafter <= 0.0 )
            {
                flipped = true;
                return 0.0;
            }

            double qnow = TriMinAngle( fn[0]->pnt, fn[1]->pnt, fn[2]->pnt );

            if ( qnow < qbefore )
            {
                qbefore = qnow;
            }

            double q = TriMinAngle( p[0], p[1], p[2] );

            if ( q < worst )
            {
                worst = q;
            }
        }
    }

    return worst;
}

bool Mesh::CollapseEdge( Edge* edge, bool repair )
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

    if ( !ea0 || !ea1 || !eb0 || !eb1 ) return false;

    Face* fa0 = ea0->OtherFace( fa );
    Face* fa1 = ea1->OtherFace( fa );
    Face* fb0 = eb0->OtherFace( fb );
    Face* fb1 = eb1->OtherFace( fb );

    if ( !fa0 || !fa1 || !fb0 || !fb1 ) return false;

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




    // Where the two ends meet.
    //
    // Wang 2006 5.2: "either the two end points are merged to create one vertex or a new
    // vertex is created ... In practice, both options are checked and the configuration
    // ... is adopted."  Both configurations are tried here and judged on the shape they leave
    // behind.
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
        vec3d cand_p[3];
        vec2d cand_uw[3];

        vec3d psplit = ( n0->pnt + n1->pnt ) * 0.5;
        vec2d uwsplit = ( n0->uw + n1->uw ) * 0.5;
        cand_uw[0] = m_Surf->ClosestUW( psplit, uwsplit[0], uwsplit[1] );
        cand_p[0]  = m_Surf->CompPnt( cand_uw[0].x(), cand_uw[0].y() );

        cand_p[1] = n0->pnt;
        cand_uw[1] = n0->uw;

        cand_p[2] = n1->pnt;
        cand_uw[2] = n1->uw;

        int best = -1;
        double bestq = -1.0;
        double qbefore = M_PI;

        for ( int k = 0; k < 3; k++ )
        {
            bool flipped = false;
            double qb = M_PI;
            double q = CollapseConfigQuality( edge, cand_p[k], cand_uw[k], flipped, qb );
            qbefore = qb;

            // Wang 2006 5.2 step 2: a negative area means the collapse overlapped the mesh.
            if ( flipped )
            {
                continue;
            }

            if ( q > bestq )
            {
                bestq = q;
                best = k;
            }
        }

        if ( best < 0 )
        {
            return false;         // every way of doing this would overlap
        }

        // Wang 2006 5.2 step 3: the new configuration must not contain a triangle whose
        // minimum angle tends to zero.
        //
        // Stated as a bare floor this refuses to improve a neighbourhood that is already
        // below the floor, which is the one place the improvement is most wanted.  The rule
        // that does what is meant is that the collapse may not make the neighbourhood worse:
        // either it comes out acceptable, or it comes out better than it went in.
        // Stated as "better than it was" this permits a collapse that leaves a triangle at
        // very nearly zero, so long as the one it replaced was slightly worse.  Locally that
        // reads as progress; over a surface it spirals, because the measurement only covers
        // the faces beside the edge while the consequences land further out.  A plain floor is what
        // holds.   [delete "Tried, and it degenerated most of a surface."]
        // A collapse made for size may not leave a triangle at nearly zero.  A collapse made
        // to remove a face that is already unfit may, because refusing it leaves the unfit
        // face in the mesh, which is the worse of the two outcomes.  Overlap is refused
        // either way.
        if ( !repair && bestq < MIN_COLLAPSE_ANGLE )
        {
            return false;
        }

        pc = cand_p[best];
        uwc = cand_uw[best];
    }

    // Both faces beside the edge go away in this collapse, so neither end's move is judged
    // against them.
    if ( !ValidNodeMove( n0, pc, fa, fb ) )
    {
        return false;
    }
    if ( !ValidNodeMove( n1, pc, fa, fb ) )
    {
        return false;
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

        if( ea0->ns )
        {
            eca->ns = ea0->ns;
            ea0->ns = nullptr;
        }

        if( ea1->ns )
        {
            eca->ns = ea1->ns;
            ea1->ns = nullptr;
        }
    }
    if ( eb0->border || eb1->border )
    {
        ecb->border = true;

        if( eb0->ns )
        {
            ecb->ns = eb0->ns;
            eb0->ns = nullptr;
        }

        if( eb1->ns )
        {
            ecb->ns = eb1->ns;
            eb1->ns = nullptr;
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

    CheckFace( fa0 );
    CheckFace( fa1 );
    CheckFace( fb0 );
    CheckFace( fb1 );

    return true;
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
    Node* closest_node = nullptr;

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
        // vec3d cent = ( edge->n0->pnt + edge->n1->pnt ) * 0.5;
        vec2d uwcent = ( edge->n0->uw  + edge->n1->uw ) * 0.5;

        int reason = -1;
        edge->target_len = m_Surf->InterpTargetMap( uwcent.x(), uwcent.y(), reason );
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

void Mesh::CheckFace( Face* f )
{
    if ( !f || f->m_DeleteMeFlag )
    {
        return;
    }

    Node* fn[4] = { f->n0, f->n1, f->n2, f->n3 };
    Edge* fe[4] = { f->e0, f->e1, f->e2, f->e3 };

    int nv = 3;

    if ( fn[3] )
    {
        nv = 4;
    }

    for ( int i = 0 ; i < nv ; i++ )
    {
        assert( fn[i] );
        assert( fe[i] );

        if ( !fn[i] || !fe[i] )
        {
            return;
        }

        assert( fe[i]->n0 && fe[i]->n1 );
        assert( fe[i]->n0 != fe[i]->n1 );
        assert( f->Contains( fe[i]->n0, fe[i]->n1 ) );
        assert( fe[i]->f0 == f || fe[i]->f1 == f );
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

// Where the retry shuffle seeds start.
static const unsigned int CFD_MESH_SEED_BASE = 1;

// The shuffle only has to break whatever ordering the triangulator choked on, so the sequence
// is seeded from the attempt number and not from the machine.  Seeding from random_device
// instead makes a surface that needed a retry come out differently on every run, and a mesh
// that cannot be reproduced cannot be compared or debugged.
vector< int > Mesh::RandomizePointOrder( vector< vec2d > & uw, vector< MeshSeg > & segs, unsigned int seed )
{
    int npt = (int)uw.size();

    // perm[new_idx] = old_idx
    vector< int > perm( npt );
    iota( perm.begin(), perm.end(), 0 );
    shuffle( perm.begin(), perm.end(), mt19937{ seed } );

    vector< int > inv_perm( npt );
    for ( int i = 0; i < npt; i++ )
    {
        inv_perm[perm[i]] = i;
    }

    vector< vec2d > uw_shuffled( npt );
    for ( int i = 0; i < npt; i++ )
    {
        uw_shuffled[i] = uw[perm[i]];
    }
    uw.swap( uw_shuffled );

    for ( int j = 0; j < (int)segs.size(); j++ )
    {
        segs[j].m_Index[0] = inv_perm[segs[j].m_Index[0]];
        segs[j].m_Index[1] = inv_perm[segs[j].m_Index[1]];
    }

    return perm;
}

void Mesh::RandomizeSegOrder( vector< MeshSeg > & segs, unsigned int seed )
{
    shuffle( segs.begin(), segs.end(), mt19937{ seed } );
}

// Points on a lattice across the domain, at the spacing the mesh was asked for, far enough
// from what is already there to be worth adding.
//
// The fallback triangulator adds nothing of its own: it joins up the points it is given.  Given
// only the curve points, it spans the middle of the patch with whatever triangles reach across
// it, and a patch that is a whole side of a body is a long way across.  Seeding the inside
// first means a surface that the main triangulator refused still comes out near the size it
// was meant to be, instead of needing the remesher to dig it out of a hole it may not manage.
static void SeedInteriorPoints( const vector< vec2d > & uw_prime, const vector< MeshSeg > & segs,
                                double spacing, vector< vec2d > & seeds )
{
    seeds.clear();

    if ( spacing <= 0.0 || uw_prime.empty() )
    {
        return;
    }

    double xlo = uw_prime[0].x(), xhi = xlo, ylo = uw_prime[0].y(), yhi = ylo;

    for ( int i = 1; i < ( int )uw_prime.size(); i++ )
    {
        xlo = min( xlo, uw_prime[i].x() );
        xhi = max( xhi, uw_prime[i].x() );
        ylo = min( ylo, uw_prime[i].y() );
        yhi = max( yhi, uw_prime[i].y() );
    }

    int nx = ( int )( ( xhi - xlo ) / spacing );
    int ny = ( int )( ( yhi - ylo ) / spacing );

    if ( nx < 1 || ny < 1 )
    {
        return;
    }

    // Anything closer than this to a point already given, or to a constraint, is left out --
    // a seed on top of the existing work only makes slivers.
    double clear = 0.5 * spacing;
    double clear2 = clear * clear;

    for ( int i = 1; i < nx; i++ )
    {
        for ( int j = 1; j < ny; j++ )
        {
            vec2d p( xlo + i * spacing, ylo + j * spacing );

            bool ok = true;

            for ( int k = 0; k < ( int )uw_prime.size() && ok; k++ )
            {
                double dx = uw_prime[k].x() - p.x();
                double dy = uw_prime[k].y() - p.y();

                if ( dx * dx + dy * dy < clear2 )
                {
                    ok = false;
                }
            }

            for ( int k = 0; k < ( int )segs.size() && ok; k++ )
            {
                const vec2d &a = uw_prime[ segs[k].m_Index[0] ];
                const vec2d &b = uw_prime[ segs[k].m_Index[1] ];

                double ex = b.x() - a.x(), ey = b.y() - a.y();
                double elen2 = ex * ex + ey * ey;

                if ( elen2 <= 0.0 )
                {
                    continue;
                }

                double t = ( ( p.x() - a.x() ) * ex + ( p.y() - a.y() ) * ey ) / elen2;

                if ( t < 0.0 ) t = 0.0;
                if ( t > 1.0 ) t = 1.0;

                double cx = a.x() + t * ex - p.x();
                double cy = a.y() + t * ey - p.y();

                if ( cx * cx + cy * cy < clear2 )
                {
                    ok = false;
                }
            }

            if ( ok )
            {
                seeds.push_back( p );
            }
        }
    }
}

bool Mesh::InitMesh_DBA( const vector< vec2d > & uw_prime, const vector< MeshSeg > & segs_indexes,
                          vector< vector< int > > & connlist, vector< vec2d > & points_out,
                          double spacing )
{
    // The curve points first, so the constraint indices still refer to them, then the seeds.
    vector< vec2d > pts = uw_prime;

    vector< vec2d > seeds;
    SeedInteriorPoints( uw_prime, segs_indexes, spacing, seeds );

    pts.insert( pts.end(), seeds.begin(), seeds.end() );

    int npt  = pts.size();
    int nedg = segs_indexes.size();

    dba_point* cloud  = new dba_point[npt];
    dba_edge*  bounds = new dba_edge[nedg];

    for ( int i = 0; i < npt; i++ )
    {
        cloud[i].x = pts[i].x();
        cloud[i].y = pts[i].y();
    }

    for ( int i = 0; i < nedg; i++ )
    {
        bounds[i].a = segs_indexes[i].m_Index[0];
        bounds[i].b = segs_indexes[i].m_Index[1];
    }

    IDelaBella2< double >* idb = IDelaBella2< double >::Create();

    bool success = false;
    int verts = idb->Triangulate( npt, &cloud->x, &cloud->y, sizeof( dba_point ) );

    if ( verts > 0 )
    {
        idb->ConstrainEdges( nedg, &bounds->a, &bounds->b, sizeof( dba_edge ) );

        int tris = idb->FloodFill( false, 0, 1 );

        const IDelaBella2< double >::Simplex* dela = idb->GetFirstDelaunaySimplex();

        connlist.resize( tris );
        for ( int i = 0; i < tris; i++ )
        {
            connlist[i] = { dela->v[2]->i, dela->v[1]->i, dela->v[0]->i };
            dela = dela->next;
        }

        points_out = pts;
        success = true;
    }
    else
    {
        printf( "DBA Error in InitMesh_DBA: %d\n", verts );
    }

    delete[] cloud;
    delete[] bounds;
    idb->Destroy();

    return success;
}

bool Mesh::InitMesh_TRI( const vector< vec2d > & uw_prime, const vector< MeshSeg > & segs_indexes,
                         vector< vector< int > > & connlist, vector< vec2d > & points_out, int relax,
                         double areascale )
{
    int num_pnts  = uw_prime.size();
    int num_edges = segs_indexes.size();

    context* ctx;
    triangleio in, out;
    int tristatus = TRI_NULL;

    ctx = triangle_context_create();

    memset( &in, 0, sizeof( in ) );
    memset( &out, 0, sizeof( out ) );

    in.pointlist   = ( REAL * ) malloc( num_pnts * 2 * sizeof( REAL ) );
    in.segmentlist = ( int * )  malloc( num_edges * 2 * sizeof( int ) );
    out.pointlist     = nullptr;
    out.segmentlist   = nullptr;
    out.trianglelist  = nullptr;

    in.numberofpointattributes = 0;
    in.pointattributelist  = nullptr;
    in.pointmarkerlist     = nullptr;
    in.numberofholes       = 0;
    in.numberoftriangles   = 0;
    in.numberofedges       = 0;
    in.trianglelist        = nullptr;
    in.trianglearealist    = nullptr;
    in.edgelist            = nullptr;
    in.edgemarkerlist      = nullptr;
    in.segmentmarkerlist   = nullptr;

    in.numberofpoints = num_pnts;

    int cnt = 0;
    for ( int j = 0; j < num_pnts; j++ )
    {
        in.pointlist[cnt++] = uw_prime[j].x();
        in.pointlist[cnt++] = uw_prime[j].y();
    }

    in.numberofsegments = num_edges;
    cnt = 0;
    for ( int j = 0; j < num_edges; j++ )
    {
        in.segmentlist[cnt++] = segs_indexes[j].m_Index[0];
        in.segmentlist[cnt++] = segs_indexes[j].m_Index[1];
    }

    double est_num_tris = ( uw_prime.size() / 4 ) * ( uw_prime.size() / 4 );
    if ( est_num_tris < 1 )     est_num_tris = 1;
    if ( est_num_tris > 10000 ) est_num_tris = 10000;

    BndBox box;
    for ( int i = 0; i < num_pnts; i++ )
    {
        box.Update( vec3d( uw_prime[i].x(), uw_prime[i].y(), 0 ) );
    }

    double uw_area = ( box.GetMax( 0 ) - box.GetMin( 0 ) ) * ( box.GetMax( 1 ) - box.GetMin( 1 ) );
    double uw_tri_area = areascale * 4.0 * uw_area / est_num_tris;
    if ( uw_tri_area < 1.0e-4 ) uw_tri_area = 1.0e-4;

    // z  number from zero      p  respect the segments      YY  add no points on them
    // Q  quiet                   a  limit triangle size        q20  no angle under 20 degrees
    //
    // The last two are what a triangulation fails on: the quality bound cannot always be met,
    // and a size limit can conflict with the segments.  Relaxing them in turn keeps a hard
    // surface with the triangulator, which respects the segments, rather than dropping it to a
    // fallback that ignores the sizing altogether.
    char str[256];

    if ( relax <= 0 )
    {
        snprintf( str, sizeof( str ), "zpYYQa%8.6fq20", uw_tri_area );
    }
    else if ( relax == 1 )
    {
        snprintf( str, sizeof( str ), "zpYYQa%8.6f", uw_tri_area );
    }
    else if ( relax == 2 )
    {
        snprintf( str, sizeof( str ), "zpYYQ" );
    }
    else
    {
        // One Y instead of two: the outer boundary is still left alone, so the patch still
        // meets its neighbours where it did, but an interior segment may be split.  That is
        // what a segment insertion failure needs -- two constraints that cross cannot both be
        // held without a point where they meet.
        snprintf( str, sizeof( str ), "zpYQa%8.6fq20", uw_tri_area );
    }

    tristatus = triangle_context_options( ctx, str );
    if ( tristatus != TRI_OK ) printf( "triangle_context_options Error\n" );

    tristatus = triangle_mesh_create( ctx, &in );
    if ( tristatus != TRI_OK ) printf( "triangle_mesh_create Error\n" );

    if ( tristatus == TRI_OK )
    {
        triangle_mesh_copy( ctx, &out, 1, 1 );

        points_out.resize( out.numberofpoints );
        for ( int i = 0; i < out.numberofpoints; i++ )
        {
            points_out[i] = vec2d( out.pointlist[i * 2], out.pointlist[i * 2 + 1] );
        }

        connlist.reserve( out.numberoftriangles );
        cnt = 0;
        for ( int i = 0; i < out.numberoftriangles; i++ )
        {
            connlist.push_back( { out.trianglelist[cnt], out.trianglelist[cnt + 1], out.trianglelist[cnt + 2] } );
            cnt += 3;
        }

    }

    if ( in.pointlist )         free( in.pointlist );
    if ( in.segmentlist )       free( in.segmentlist );
    if ( out.pointlist )        free( out.pointlist );
    if ( out.pointmarkerlist )  free( out.pointmarkerlist );
    if ( out.trianglelist )     free( out.trianglelist );
    if ( out.segmentlist )      free( out.segmentlist );
    if ( out.segmentmarkerlist) free( out.segmentmarkerlist );

    triangle_context_destroy( ctx );

    return tristatus == TRI_OK;
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
    FILE* fp = nullptr;
    static FILE* fpmas = nullptr;

    if ( namecnt == 0 )
    {
        char str2[256];
        snprintf( str2, sizeof( str2 ), "%sSortedUnscaledMesh_UW.m", MeshMgr->m_DebugDir.c_str() );
        fpmas = fopen( str2, "w" );

        fprintf( fpmas, "clear all; format compact; close all;\n" );
        fprintf( fpmas, "figure(1); hold on\n" );
    }

    vector< vec2d > sorted = uw_points;
    sort( sorted.begin(), sorted.end(), vec2dCompare );

    snprintf( str, sizeof( str ), "%sSortedUnscaledMesh_UW%d.m", MeshMgr->m_DebugDir.c_str(), namecnt );
    fp = fopen( str, "w" );

    if (fpmas )
    {
        snprintf( str, sizeof( str ), "SortedUnscaledMesh_UW%d.m", namecnt );
        fprintf( fpmas, "run( '%s' );\n", str );
    }

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

    if ( namecnt == MeshMgr->GetTotalNumSurfs() - 1 )
    {
        fprintf( fpmas, "figure(1)\n");
        fprintf( fpmas, "axis off\n" );
        fprintf( fpmas, "axis equal\n" );
        fprintf( fpmas, "hold off\n" );

        fclose( fpmas );
        fpmas = nullptr;
    }
#endif

    //==== Scale UW Pnts ====//
    vector< vec2d > uw_prime( uw_points.size() );
    for ( i = 0 ; i < ( int )uw_points.size() ; i++ )
    {
        uw_prime[i] = m_Surf->GetST( uw_points[ i ] );
    }

#ifdef DEBUG_CFD_MESH

    static FILE* fpmas2 = nullptr;

    if ( namecnt == 0 )
    {
        char str2[256];
        snprintf( str2, sizeof( str2 ), "%sMesh_UW.m", MeshMgr->m_DebugDir.c_str() );
        fpmas2 = fopen( str2, "w" );

        fprintf( fpmas2, "clear all; format compact; close all;\n" );
        fprintf( fpmas2, "figure(1); hold on\n" );
    }


    snprintf( str, sizeof( str ), "%sMesh_UW%d.m", MeshMgr->m_DebugDir.c_str(), namecnt );
    fp = fopen( str, "w" );

    if ( fpmas2 )
    {
        snprintf( str, sizeof( str ), "Mesh_UW%d.m", namecnt );
        fprintf( fpmas2, "run( '%s' );\n", str );
    }

    fprintf( fp, "u = [" );
    for ( i = 0 ; i < num_edges ; i++ )
    {
        int ind0 = segs_indexes[i].m_Index[0];
        int ind1 = segs_indexes[i].m_Index[1];
        fprintf( fp, "%.19e %.19e", uw_prime[ind0].x(), uw_prime[ind1].x() );

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
        fprintf( fp, "%.19e %.19e", uw_prime[ind0].y(), uw_prime[ind1].y() );

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

    if ( namecnt == MeshMgr->GetTotalNumSurfs() - 1 )
    {
        fprintf( fpmas2, "figure(1)\n");
        fprintf( fpmas2, "axis off\n" );
        fprintf( fpmas2, "axis equal\n" );
        fprintf( fpmas2, "hold off\n" );

        fclose( fpmas2 );
        fpmas2 = nullptr;
    }
#endif


#ifdef DEBUG_CFD_MESH
    snprintf( str, sizeof( str ), "%sTriInput_%d.dat", MeshMgr->m_DebugDir.c_str(), namecnt );
    fp = fopen( str, "w" );

    fprintf( fp, "%d\n", uw_prime.size() );

    for ( i = 0; i < (int)uw_prime.size(); i++ )
    {
        fprintf( fp, "%d %.19e %.19e\n", i, uw_prime[i].x(), uw_prime[i].y() );
    }

    fprintf( fp, "%d\n", segs_indexes.size() );

    for ( i = 0; i < (int)segs_indexes.size(); i++ )
    {
        fprintf( fp, "%d %d %d\n", i, segs_indexes[i].m_Index[0], segs_indexes[i].m_Index[1] );
    }
    fclose( fp );
#endif


    //==== Attempt Triangulation ====//
    vector< vector< int > > connlist;
    vector< vec2d > points_out;

    bool success = InitMesh_TRI( uw_prime, segs_indexes, connlist, points_out );

    int trimethod = 0;      // 0 first try, 1..5 retry number, 9 fell back to DBA, -1 nothing worked

    // Nudge the size limit before asking for anything less.  A segment insertion failure is a
    // robustness limit reached on one particular arrangement of added points; moving the size
    // limit moves every one of them, which usually steps around it and still returns a mesh
    // built to the sizing that was asked for.
    const double areatry[] = { 0.8, 1.25, 0.6, 1.6 };

    for ( int r = 0; !success && r < 4; r++ )
    {
        connlist.clear();
        points_out.clear();
        success = InitMesh_TRI( uw_prime, segs_indexes, connlist, points_out, 0, areatry[r] );

        if ( success )
        {
            trimethod = 20 + r;
        }
    }

    // Then ask for less: without the quality bound, then without the size limit either.
    for ( int r = 1; !success && r <= 2; r++ )
    {
        connlist.clear();
        points_out.clear();
        success = InitMesh_TRI( uw_prime, segs_indexes, connlist, points_out, r );

        if ( success )
        {
            trimethod = 10 + r;
        }
    }

    if ( !success )
    {
        int n = 0;
        while ( !success && n < 5 )
        {
#ifdef DEBUG_CFD_MESH
            printf( "  Triangulation failed for surface %d %s %s, randomizing point order for %d time and trying again\n", namecnt, m_Surf->GetName().c_str(), m_Surf->GetGeomID().c_str(), n );
#endif
            RandomizePointOrder( uw_prime, segs_indexes, CFD_MESH_SEED_BASE + n );

            connlist.clear();
            points_out.clear();
            success = InitMesh_TRI( uw_prime, segs_indexes, connlist, points_out );
            n++;

            if ( success )
            {
                trimethod = n;
            }
        }

#ifdef DEBUG_CFD_MESH
        if ( success )
        {
            printf( "  Randomization Succeeded after %d tries\n\n", n );
        }
        else
        {
            printf ("  Randomization failed after %d tries\n", n );
        }
#endif
    }
    // Last thing before giving the surface to a triangulator that ignores the sizing: let the
    // interior segments be split.
    if ( !success )
    {
        connlist.clear();
        points_out.clear();
        success = InitMesh_TRI( uw_prime, segs_indexes, connlist, points_out, 3 );

        if ( success )
        {
            trimethod = 13;
        }
    }

    if ( !success )
    {
#ifdef DEBUG_CFD_MESH
        printf( "  Triangulation failed for surface %d %s %s, falling back to DBA\n", namecnt, m_Surf->GetName().c_str(), m_Surf->GetGeomID().c_str() );
#endif
        connlist.clear();
        points_out.clear();
        // The size the mesh was asked for, worked out the way the main triangulator works it out.
        double dba_est = ( uw_prime.size() / 4 ) * ( uw_prime.size() / 4 );
        if ( dba_est < 1 )     dba_est = 1;
        if ( dba_est > 10000 ) dba_est = 10000;

        BndBox dbabox;
        for ( int k = 0; k < ( int )uw_prime.size(); k++ )
        {
            dbabox.Update( vec3d( uw_prime[k].x(), uw_prime[k].y(), 0 ) );
        }

        double dba_area = ( dbabox.GetMax( 0 ) - dbabox.GetMin( 0 ) ) *
                          ( dbabox.GetMax( 1 ) - dbabox.GetMin( 1 ) );
        double dba_tri_area = 4.0 * dba_area / dba_est;
        if ( dba_tri_area < 1.0e-4 ) dba_tri_area = 1.0e-4;

        success = InitMesh_DBA( uw_prime, segs_indexes, connlist, points_out, sqrt( 2.0 * dba_tri_area ) );

        trimethod = 9;

#ifdef DEBUG_CFD_MESH
        if ( success )
        {
            printf( "  DBA Succeeded\n\n" );
        }
        else
        {
            printf( "  DBA Failed\n\n" );
        }
#endif
    }

#ifdef DEBUG_CFD_MESH
    if ( !success ) printf( "  Triangulation failed for surface %d\n", namecnt );
#endif

    if ( !success )
    {
        trimethod = -1;
    }

    // Say so when a surface did not triangulate the way it was asked to.  The surfaces it happens
    // to are the ones worth looking at -- the mesh on them is not the mesh that was asked for.
    if ( trimethod != 0 )
    {
        const char *how = "retried with different points";

        if ( trimethod >= 20 )
        {
            how = "retried with a different size limit";
        }
        else if ( trimethod >= 10 )
        {
            how = "retried with the quality bound relaxed";
        }
        else if ( trimethod == 9 )
        {
            how = "FELL BACK to the second triangulator";
        }
        else if ( trimethod < 0 )
        {
            how = "COULD NOT BE TRIANGULATED";
        }

        printf( "Surface %d (%s %s): %s.\n", m_Surf->GetSurfID(),
                m_Surf->GetName().c_str(), m_Surf->GetGeomID().c_str(), how );
        fflush( stdout );
    }

    //==== Clear All Node, Edge, Tri Data ====//
    Clear();

    if ( success )
    {
        //==== Create Nodes ====//
        vector< Node* > nodeVec;
        for ( i = 0; i < (int)points_out.size(); i++ )
        {
            vec2d uw;
            if ( i < num_pnts )
            {
                uw = uw_points[i];
            }
            else
            {
                uw = m_Surf->GetUW( points_out[i] );
            }

            vec3d pnt = m_Surf->CompPnt( uw.v[0], uw.v[1] );
            nodeVec.push_back( AddNode( pnt, uw ) );
        }

        //==== Load Triangles ====//
        for ( i = 0; i < (int)connlist.size(); i++ )
        {
            Node* n0 = nodeVec[connlist[i][0]];
            Node* n1;
            Node* n2;

            if ( !m_Surf->GetFlipFlag() )
            {
                n1 = nodeVec[connlist[i][1]];
                n2 = nodeVec[connlist[i][2]];
            }
            else
            {
                n1 = nodeVec[connlist[i][2]];
                n2 = nodeVec[connlist[i][1]];
            }

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

            AddFace( n0, n1, n2, e0, e1, e2 );
        }


        for ( j = 0; j < (int)segs_indexes.size(); j++ )
        {
            Node* n0 = nodeVec[segs_indexes[j].m_Index[0]];
            Node* n1 = nodeVec[segs_indexes[j].m_Index[1]];

            n0->pnt = segs_indexes[j].m_P[0];
            n1->pnt = segs_indexes[j].m_P[1];

            Edge *e = n0->FindEdge( n1 );

            if ( e )
            {
                e->border = true;

                n0->fixed = true;
                n1->fixed = true;

                vec2d uw = segs_indexes[j].m_UWmid;
                vec3d pnt = segs_indexes[j].m_Pmid;

                Node* nsplit = AddNode( pnt, uw );
                nsplit->fixed = true;

                e->ns = nsplit;
            }
        }
    }

#ifdef DEBUG_CFD_MESH
        static FILE* fpmas3 = nullptr;

        if ( namecnt == 0 )
        {
            char str2[256];
            snprintf( str2, sizeof( str2 ), "%sUWTriMeshOut.m", MeshMgr->m_DebugDir.c_str() );
            fpmas3 = fopen( str2, "w" );

            fprintf( fpmas3, "clear all; format compact; close all;\n" );
            fprintf( fpmas3, "figure(2); hold on\n" );
            fprintf( fpmas3, "figure(3); hold on\n" );
            fprintf( fpmas3, "figure(4); hold on\n" );
        }

        snprintf( str, sizeof( str ), "%sUWTriMeshOut%d.m", MeshMgr->m_DebugDir.c_str(), namecnt );
        fp = fopen( str, "w" );

        if (fpmas3 )
        {
            snprintf( str, sizeof( str ), "UWTriMeshOut%d.m", namecnt );
            fprintf( fpmas3, "run( '%s' );\n", str );
        }

        fprintf( fp, "clear all\nformat compact\n" );
        fprintf( fp, "t = [" );
        for ( i = 0 ; i < (int)connlist.size() ; i++ )
        {
            fprintf( fp, "%d, %d, %d", connlist[i][0] + 1, connlist[i][1] + 1, connlist[i][2] + 1 );

            if ( i < (int)connlist.size() - 1 )
                fprintf( fp, ";\n" );
            else
                fprintf( fp, "];\n" );
        }

        fprintf( fp, "uprm = [" );
        for ( i = 0; i < (int)points_out.size(); i++ )
        {
            fprintf( fp, "%f", points_out[i].x() );

            if ( i < (int)points_out.size() - 1 )
                fprintf( fp, ";\n" );
            else
                fprintf( fp, "];\n" );
        }

        fprintf( fp, "wprm = [" );
        for ( i = 0; i < (int)points_out.size(); i++ )
        {
            fprintf( fp, "%f", points_out[i].y() );

            if ( i < (int)points_out.size() - 1 )
                fprintf( fp, ";\n" );
            else
                fprintf( fp, "];\n" );
        }

        fprintf( fp, "u = [" );
        for ( i = 0; i < (int)points_out.size(); i++ )
        {
            vec2d uw;
            if ( i < num_pnts )
            {
                uw = uw_points[i];
            }
            else
            {
                uw = m_Surf->GetUW( points_out[i] );
            }

            fprintf( fp, "%f", uw.x() );

            if ( i < (int)points_out.size() - 1 )
                fprintf( fp, ";\n" );
            else
                fprintf( fp, "];\n" );
        }

        fprintf( fp, "w = [" );
        for ( i = 0; i < (int)points_out.size(); i++ )
        {
            vec2d uw;
            if ( i < num_pnts )
            {
                uw = uw_points[i];
            }
            else
            {
                uw = m_Surf->GetUW( points_out[i] );
            }

            fprintf( fp, "%f", uw.y() );

            if ( i < (int)points_out.size() - 1 )
                fprintf( fp, ";\n" );
            else
                fprintf( fp, "];\n" );
        }

        fprintf( fp, "x = [" );
        for ( i = 0; i < (int)points_out.size(); i++ )
        {
            vec2d uw;
            if ( i < num_pnts )
            {
                uw = uw_points[i];
            }
            else
            {
                uw = m_Surf->GetUW( points_out[i] );
            }

            vec3d pnt = m_Surf->CompPnt( uw.v[0], uw.v[1] );

            fprintf( fp, "%f", pnt.x() );

            if ( i < (int)points_out.size() - 1 )
                fprintf( fp, ";\n" );
            else
                fprintf( fp, "];\n" );
        }

        fprintf( fp, "y = [" );
        for ( i = 0; i < (int)points_out.size(); i++ )
        {
            vec2d uw;
            if ( i < num_pnts )
            {
                uw = uw_points[i];
            }
            else
            {
                uw = m_Surf->GetUW( points_out[i] );
            }

            vec3d pnt = m_Surf->CompPnt( uw.v[0], uw.v[1] );

            fprintf( fp, "%f", pnt.y() );

            if ( i < (int)points_out.size() - 1 )
                fprintf( fp, ";\n" );
            else
                fprintf( fp, "];\n" );
        }

        fprintf( fp, "z = [" );
        for ( i = 0; i < (int)points_out.size(); i++ )
        {
            vec2d uw;
            if ( i < num_pnts )
            {
                uw = uw_points[i];
            }
            else
            {
                uw = m_Surf->GetUW( points_out[i] );
            }

            vec3d pnt = m_Surf->CompPnt( uw.v[0], uw.v[1] );

            fprintf( fp, "%f", pnt.z() );

            if ( i < (int)points_out.size() - 1 )
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

        if ( namecnt == MeshMgr->GetTotalNumSurfs() - 1 )
        {
            fprintf( fpmas3, "figure(2)\n");
            fprintf( fpmas3, "axis off\n" );
            fprintf( fpmas3, "axis equal\n" );
            fprintf( fpmas3, "hold off\n" );

            fprintf( fpmas3, "figure(3)\n");
            fprintf( fpmas3, "axis off\n" );
            fprintf( fpmas3, "axis equal\n" );
            fprintf( fpmas3, "hold off\n" );

            fprintf( fpmas3, "figure(4)\n");
            fprintf( fpmas3, "axis off\n" );
            fprintf( fpmas3, "axis equal\n" );
            fprintf( fpmas3, "hold off\n" );

            fclose( fpmas3 );
            fpmas3 = nullptr;
        }

        namecnt++;
#endif



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
        if (( *e )->f0 == nullptr || ( *e )->f1 == nullptr )
        {
            ( *e )->ridge = true;
            ( *e )->n0->fixed = true;
            ( *e )->n1->fixed = true;
        }
    }


}

void Mesh::WriteSimpleSTL( const char* file_name )
{
    FILE* file_id = fopen( file_name, "w" );
    if ( file_id )
    {
        fprintf( file_id, "solid\n" );

        WriteSimpleSTL( file_id );

        fprintf( file_id, "endsolid\n" );
        fclose( file_id );
    }
}



void Mesh::WriteSimpleSTL( FILE* file_id )
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

        fprintf( file_id, " facet normal  %2.10le %2.10le %2.10le\n", norm.x(), norm.y(), norm.z() );
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

            fprintf( file_id, " facet normal  %2.10le %2.10le %2.10le\n", norm.x(), norm.y(), norm.z() );
            fprintf( file_id, "   outer loop\n" );

            fprintf( file_id, "     vertex %2.10le %2.10le %2.10le\n", p0.x(), p0.y(), p0.z() );
            fprintf( file_id, "     vertex %2.10le %2.10le %2.10le\n", p2.x(), p2.y(), p2.z() );
            fprintf( file_id, "     vertex %2.10le %2.10le %2.10le\n", p3.x(), p3.y(), p3.z() );

            fprintf( file_id, "   endloop\n" );
            fprintf( file_id, " endfacet\n" );
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
    list< Face* >::iterator f;
    for ( f = faceList.begin() ; f != faceList.end(); ++f )
    {
        ( *f )->WriteSTL( file_id );
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
    unordered_map< Edge*, splitData > splitEdgeMap;

    // Loop over all edges.
    for ( list< Edge* >::iterator e = origEdgeList.begin() ; e != origEdgeList.end(); ++e )
    {
        Node* n0 = ( *e )->n0;
        Node* n1 = ( *e )->n1;

        Node* ns = nullptr;

        if( ( *e )->ns )
        {
            ns = ( *e )->ns;
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


            double len = dist( n0->pnt, n1->pnt );
            double len2 = dist( n0->pnt, ns->pnt );
            if ( len2 > len )
            {
                printf( "n0->pnt %f %f %f\n", n0->pnt.x(), n0->pnt.y(), n0->pnt.z() );
                printf( "n1->pnt %f %f %f\n", n1->pnt.x(), n1->pnt.y(), n1->pnt.z() );
                printf( "psplit %f %f %f\n", psplit.x(), psplit.y(), psplit.z() );
                printf( "uwsplit %f %f\n", uwsplit.x(), uwsplit.y() );
                printf( "uws %f %f\n", uws.x(), uws.y() );
                printf( "ps %f %f %f\n", ps.x(), ps.y(), ps.z() );
                printf( "\n\n" );
            }
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

        if ( !e0 || !e1 || !e2 ) continue;

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

    Edge* hl_edge = nullptr;
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











