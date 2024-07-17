//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

//******************************************************************************
//
//    Polygon Mesh Geometry Class
//
//
//   Rob McDonald
//
//
//******************************************************************************

#include "PGMesh.h"
#include "StlHelper.h"
#include "Matrix4d.h"
#include "VspUtil.h"
#include "SubSurfaceMgr.h"
#include "FileUtil.h"
#include "PntNodeMerge.h"

#include "triangle.h"
#include "triangle_api.h"

#include "delabella.h"

#include <algorithm>
#include <cfloat>

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

PGPoint::PGPoint()
{
    m_DeleteMeFlag = false;
    m_ID = -1;
}


PGPoint::PGPoint( const vec3d& p )
{
    m_Pnt = p;
    m_ID = -1;
    m_DeleteMeFlag = false;
}

PGPoint::~PGPoint()
{
}

void PGPoint::AddConnectNode( PGNode* n )
{
    if ( vector_contains_val( m_NodeVec, n ) )
    {
        return;
    }
    m_NodeVec.push_back( n );
}

void PGPoint::RemoveConnectNode( const PGNode* n )
{
    vector_remove_val( m_NodeVec, const_cast < PGNode* > ( n ) );
}

void PGPoint::NodeForgetPoint( PGNode* n ) const
{
    n->m_Pt = nullptr;
}

bool PGPoint::Check() const
{
    if ( m_DeleteMeFlag )
    {
        return false;
    }

    for ( int i = 0; i < m_NodeVec.size(); i++ )
    {
        if ( !m_NodeVec[i] )
        {
            return false;
        }
    }
    return true;
}


//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

PGNode::PGNode( PGPoint *pptr )
{
    m_Pt = pptr;
    m_DeleteMeFlag = false;
}

PGNode::~PGNode()
{
}

void PGNode::GetConnectNodes( vector< PGNode* > & cnVec ) const
{
    cnVec.resize( m_EdgeVec.size() );

    for ( int i = 0 ; i < ( int )m_EdgeVec.size() ; i++ )
    {
        cnVec[i] = m_EdgeVec[ i ]->OtherNode( this );
    }
}

void PGNode::GetConnectFaces( vector< PGFace* > & cfVec ) const
{
    cfVec.clear();
    for ( int i = 0 ; i < ( int )m_EdgeVec.size() ; i++ )
    {
        vector< PGFace* > fvec = m_EdgeVec[i]->m_FaceVec;

        for ( int j = 0; j < ( int )fvec.size(); j++ )
        {
            PGFace* f = fvec[j];
            if ( f && !vector_contains_val( cfVec, f ) )
            {
                cfVec.push_back( f );
            }
        }
    }
}

void PGNode::GetTags( vector < int > & tags ) const
{
    for( std::map< int, vec2d >::const_iterator it = m_TagUWMap.begin(); it != m_TagUWMap.end(); ++it )
    {
        tags.push_back( it->first );
    }
}

bool PGNode::GetUW( const int tag, vec2d &uw ) const
{
    // Try to find uw with matching tag.
    map < int, vec2d >::const_iterator it = m_TagUWMap.find( tag );

    // If there is a match
    if ( it != m_TagUWMap.end() )
    {
        uw = it->second;
        return true;
    }

    // No match.
    uw = vec2d();
    return false;
}

PGEdge * PGNode::FindEdge( const PGNode* n ) const
{
    for ( int k = 0; k < (int)m_EdgeVec.size(); k++ )
    {
        if ( m_EdgeVec[k]->OtherNode( this ) == n )
        {
            return m_EdgeVec[k];
        }
    }
    return nullptr;
}

bool PGNode::UsedBy( const PGEdge* e ) const
{
    return vector_contains_val( m_EdgeVec, const_cast < PGEdge* > ( e ) );
}

void PGNode::AddConnectEdge( PGEdge* e )
{
    if ( vector_contains_val( m_EdgeVec, e ) )
    {
        return;
    }
    m_EdgeVec.push_back( e );
}

void PGNode::RemoveConnectEdge( const PGEdge* e )
{
    vector_remove_val( m_EdgeVec, const_cast < PGEdge* > ( e ) );
}

void PGNode::EdgeForgetNode( PGEdge* e ) const
{
    e->ReplaceNode( this, nullptr );
}

bool PGNode::ColinearNode() const
{
    // Node colinearity tolerance.
    double tol = 1.0e-12;

    if ( m_EdgeVec.size() == 2 )
    {
        PGEdge *e0 = m_EdgeVec[0];
        PGEdge *e1 = m_EdgeVec[1];

        if ( e0->SameFaces( e1 ) )
        {
            if ( e0->m_FaceVec.size() == 0 ) // Should be invalid
            {
                return false;
            }
            if ( e0->m_FaceVec.size() == 1 ) // Only check straightness on manifold == 1 edges.
            {
                PGFace *f = e0->m_FaceVec[0];
                vec3d nvec = f->m_Nvec;

                vector < PGNode* > nods;
                f->GetNodes( nods );

                vector < PGNode* > triNodeVec;
                PGFace::Triangulate( nods, nvec, triNodeVec );
                double Area1 = PGFace::ComputeArea( triNodeVec );

                if ( nods.size() > 1 )
                {
                    if ( nods[0] == this )
                    {
                        nods.pop_back();
                        nods[0] = nods.back();
                    }
                    else
                    {
                        vector_remove_val( nods, const_cast< PGNode* > ( this ) );
                    }
                }


                triNodeVec.clear();
                PGFace::Triangulate( nods, nvec, triNodeVec );
                double Area2 = PGFace::ComputeArea( triNodeVec );

                if ( Area2 >= 0.9 * Area1 )
                {
                    return true;
                }
                else
                {
                    printf( "Areas ratio %f false\n", Area2 / Area1 );
                }

            }
            else // For manifold >=2 edges, logical check suffices.
            {
                return true;
            }
        }
    }
    return false;
}

bool PGNode::Check() const
{
    if ( m_DeleteMeFlag )
    {
        return false;
    }

    if ( !m_Pt )
    {
        return false;
    }

    for ( int i = 0; i < m_EdgeVec.size(); i++ )
    {
        if ( !m_EdgeVec[i] )
        {
            return false;
        }
    }
    return true;
}

bool PGNode::DoubleBackNode( int &i, int &j ) const
{
    double tol = 1e-10;

    for ( i = 0; i < m_EdgeVec.size() - 1; i++ )
    {
        PGEdge *ei = m_EdgeVec[i];
        PGNode *ni = ei->OtherNode( this );

        for ( j = i + 1; j < m_EdgeVec.size(); j++ )
        {
            PGEdge *ej = m_EdgeVec[j];
            PGNode *nj = ej->OtherNode( this );

            double ti, tj;
            double di, dj;
            vec3d pi, pj;
            di = sqrt( pointLineDistSquared( ni->m_Pt->m_Pnt, m_Pt->m_Pnt, nj->m_Pt->m_Pnt, ti, pi ) );
            dj = sqrt( pointLineDistSquared( nj->m_Pt->m_Pnt, m_Pt->m_Pnt, ni->m_Pt->m_Pnt, tj, pj ) );

            // if ei and ej are co-linear and in the same direction,
            // return true;
            if ( ti > 0.0 && ti < 1.0 && di < tol )
            {
                return true;
            }

            if ( tj > 0.0 && tj < 1.0 && dj < tol )
            {
                return true;
            }
        }
    }

    return false;
}

void PGNode::SealDoubleBackNode( PGMesh *pgm )
{
    int i, j;
    while ( DoubleBackNode( i, j ) )
    {
        PGEdge *ei = m_EdgeVec[i];
        PGNode *ni = ei->OtherNode( this );

        PGEdge *ej = m_EdgeVec[j];
        PGNode *nj = ej->OtherNode( this );

        double ti, tj;
        vec3d pi, pj;
        pointLineDistSquared( ni->m_Pt->m_Pnt, m_Pt->m_Pnt, nj->m_Pt->m_Pnt, tj, pi );
        pointLineDistSquared( nj->m_Pt->m_Pnt, m_Pt->m_Pnt, ni->m_Pt->m_Pnt, ti, pj );

        if ( ti < tj )
        {
            PGEdge *newedge = pgm->SplitEdge( ei, ti, this );
            PGNode *newnode = newedge->SharedNode( ei );
            pgm->MergeNodes( nj, newnode );

            nj->SealDoubleBackNode( pgm );
        }
        else
        {
            PGEdge *newedge = pgm->SplitEdge( ej, tj, this );
            PGNode *newnode = newedge->SharedNode( ej );
            pgm->MergeNodes( ni, newnode );

            ni->SealDoubleBackNode( pgm );
        }
    }
}

void PGNode::DumpMatlab() const
{
    printf( "\nx = %.*e;\n", DBL_DIG + 3, m_Pt->m_Pnt.x() );
    printf( "\ny = %.*e;\n", DBL_DIG + 3, m_Pt->m_Pnt.y() );
    printf( "\nz = %.*e;\n", DBL_DIG + 3, m_Pt->m_Pnt.z() );

    printf( "plot3( x, y, z, 'bo' );\n" );
    printf( "hold on\n" );
}


void PGNode::Diagnostics() const
{
    printf( "Node %d\n", m_Pt->m_ID );
    printf( "Edges: " );

    for ( int i = 0; i < m_EdgeVec.size(); i++ )
    {
        printf( "%3d ", m_EdgeVec[i]->m_ID );
    }
    printf( "\n" );
}

bool PGNode::Validate() const
{
    bool valid = true;

    for ( int i = 0; i < m_EdgeVec.size(); i++ )
    {
        if ( !m_EdgeVec[i] )
        {
            printf( "Node %d has invalid edge pointer %d\n", m_Pt->m_ID, i );
            valid = false;
            continue;
        }

        if ( !m_EdgeVec[i]->ContainsNode( this ) )
        {
            printf( "Edge %d does not contain node %d\n", m_EdgeVec[i]->m_ID, m_Pt->m_ID );
            valid = false;
        }
    }

    return valid;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

PGEdge::PGEdge()
{
    m_N0 = m_N1 = nullptr;
    m_DeleteMeFlag = false;
    m_InLoopFlag = false;
    m_InCurrentLoopFlag = false;
    m_ID = -1;
}

PGEdge::PGEdge( PGNode* PGNode0, PGNode* PGNode1 )
{
    m_N0 = PGNode0;
    m_N1 = PGNode1;
    m_DeleteMeFlag = false;
    m_InLoopFlag = false;
    m_InCurrentLoopFlag = false;
    m_ID = -1;
}

PGEdge::~PGEdge()
{
}

bool PGEdge::ContainsNodes( const PGNode* in0, const PGNode* in1 ) const
{
    if ( in0 == m_N0 && in1 == m_N1 )
    {
        return true;
    }
    else if ( in0 == m_N1 && in1 == m_N0 )
    {
        return true;
    }

    return false;
}

bool PGEdge::ContainsNode( const PGNode* in ) const
{
    if ( in == m_N0 || in == m_N1 )
    {
        return true;
    }

    return false;
}

bool PGEdge::UsedBy( const PGFace* f ) const
{
    return vector_contains_val( m_FaceVec, const_cast < PGFace* > ( f ) );
}

bool PGEdge::SetNode( PGNode* n )
{
    // If node already set, return.
    if ( n == m_N0 || n == m_N1 )
    {
        return true;
    }
    // If N0 is nullptr, set that one and return.
    if ( m_N0 == nullptr )
    {
        m_N0 = n;
        return true;
    }
    // Set N1.  Will clobber non-nullptr N1.
    if ( m_N1 == nullptr )
    {
        m_N1 = n;
        return true;
    }
    return false;
}

void PGEdge::AddConnectFace( PGFace* f )
{
    if ( vector_contains_val( m_FaceVec, f ) )
    {
        return;
    }
    m_FaceVec.push_back( f );
}

void PGEdge::RemoveFace( const PGFace* f )
{
    vector_remove_val( m_FaceVec, const_cast < PGFace* > ( f ) );
}

PGNode* PGEdge::OtherNode( const PGNode* n ) const
{
    if ( n == m_N0 )
    {
        return m_N1;
    }
    else if ( n == m_N1 )
    {
        return m_N0;
    }
    else
    {
        assert( 0 );
    }

    return nullptr;
}

PGNode* PGEdge::SharedNode( const PGEdge* e ) const
{
    if ( e->ContainsNode( m_N0 ) )
    {
        return m_N0;
    }
    if ( e->ContainsNode( m_N1 ) )
    {
        return m_N1;
    }
    return nullptr;
}

PGFace* PGEdge::OtherManifoldFace( const PGFace* f ) const
{
    if ( m_FaceVec.size() != 2 )
    {
        return nullptr;
    }

    if ( m_FaceVec[0] == f )
    {
        return m_FaceVec[1];
    }

    if ( m_FaceVec[1] == f )
    {
        return m_FaceVec[0];
    }
    return nullptr;
}

void PGEdge::ReplaceNode( const PGNode* curr_PGNode, PGNode* replace_PGNode )
{
    if ( m_N0 == curr_PGNode )
    {
        m_N0 = replace_PGNode;
    }
    else if ( m_N1 == curr_PGNode )
    {
        m_N1 = replace_PGNode;
    }
    else
    {
        assert( 0 );
    }
}

void PGEdge::NodesForgetEdge() const
{
    m_N0->RemoveConnectEdge( this );
    m_N1->RemoveConnectEdge( this );
}

void PGEdge::SortFaces()
{
    sort( m_FaceVec.begin(), m_FaceVec.end() );
}

bool PGEdge::SameFaces( const PGEdge *e2 ) const
{
    // Comparison of std::vector checks size and then contents.  Since these are pointers that were previously sorted,
    // this should work.
    return ( m_FaceVec == e2->m_FaceVec );
}

bool PGEdge::Check() const
{
    if ( m_DeleteMeFlag )
    {
        return false;
    }

    if ( !m_N0 )
    {
        return false;
    }

    if ( !m_N1 )
    {
        return false;
    }

    for ( int i = 0; i < m_FaceVec.size(); i++ )
    {
        if ( !m_FaceVec[i] )
        {
            return false;
        }
    }
    return true;
}

void PGEdge::DumpMatlab() const
{
    printf( "x = [%.*e %.*e];\n", DBL_DIG + 3, m_N0->m_Pt->m_Pnt.x(), DBL_DIG + 3, m_N1->m_Pt->m_Pnt.x() );
    printf( "y = [%.*e %.*e];\n", DBL_DIG + 3, m_N0->m_Pt->m_Pnt.y(), DBL_DIG + 3, m_N1->m_Pt->m_Pnt.y() );
    printf( "z = [%.*e %.*e];\n", DBL_DIG + 3, m_N0->m_Pt->m_Pnt.z(), DBL_DIG + 3, m_N1->m_Pt->m_Pnt.z() );

    printf( "plot3( x, y, z, 'b+:' );\n" );
    printf( "hold on\n" );
}

void PGEdge::Diagnostics() const
{
    printf( "Edge %d\n", m_ID );
    printf( "Nodes: %3d %3d\n", m_N0->m_Pt->m_ID, m_N1->m_Pt->m_ID );
    printf( "Faces: " );

    for ( int i = 0; i < m_FaceVec.size(); i++ )
    {
        printf( "%3d ", m_FaceVec[i]->m_ID );
    }
    printf( "\n" );

    m_N0->Diagnostics();
    m_N1->Diagnostics();
}

bool PGEdge::Validate() const
{
    bool valid = true;

    for ( int i = 0; i < m_FaceVec.size(); i++ )
    {
        if ( !m_FaceVec[i] )
        {
            printf( "Edge %d has invalid face pointer %d\n", m_ID, i );
            valid = false;
            continue;
        }

        if ( !m_FaceVec[i]->Contains( this ) )
        {
            printf( "Face %d does not contain edge %d\n", m_FaceVec[i]->m_ID, m_ID );
            valid = false;
        }
    }

    if ( !m_N0 )
    {
        printf( "Node 0 is invalid in edge %d\n", m_ID );
        valid = false;
    }
    else if ( !m_N0->UsedBy( this ) )
    {
        printf( "Node %d is unaware it is used by edge %d\n", m_N0->m_Pt->m_ID, m_ID );
        valid = false;
    }

    if ( !m_N1 )
    {
        printf( "Node 1 is invalid in edge %d\n", m_ID );
        valid = false;
    }
    else if ( !m_N1->UsedBy( this ) )
    {
        printf( "Node %d is unaware it is used by edge %d\n", m_N1->m_Pt->m_ID, m_ID );
        valid = false;
    }

    return valid;
}

bool PGEdge::WakeEdge( const PGMesh *m, const bool ContinueCoPlanarWakes ) const
{
    const double tol = 1e-12;

    int nface = m_FaceVec.size();

    for ( int i = 0; i < nface; i++ )
    {
        PGFace *f = m_FaceVec[i];

        int tag = f->m_Tag;
        int part = m->m_PGMulti->GetPart( tag );
        int type = m->m_PGMulti->GetType( part );
        double wmin = m->m_PGMulti->GetWmin( part );

        int thick = m->m_PGMulti->GetThickThin( part );

        if ( type == vsp::WING_SURF )
        {
            if ( !ContinueCoPlanarWakes )
            {
                if ( !thick && nface != 1 )
                {
                    return false;
                }
            }

            vec2d uw0, uw1;
            if ( m_N0->GetUW( tag, uw0 ) && m_N1->GetUW( tag, uw1 ) )  // Both nodes have needed tags.
            {
                if ( uw0.y() <= ( wmin + tol ) &&
                     uw1.y() <= ( wmin + tol ) )
                {
                    return true;
                }
            }
        }
    }

    return false;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

PGFace::PGFace()
{
    m_DeleteMeFlag = false;
    m_ID = -1;
    m_iQuad = -1;
    m_Tag = -1;
    m_Region = -1;
    m_jref = 0;
    m_kref = 0;
}

PGFace::~PGFace()
{
}

PGEdge* PGFace::FindEdge( const PGNode* nn0, const PGNode* nn1 ) const
{
    for ( int i = 0; i < m_EdgeVec.size(); i++ )
    {
        if ( m_EdgeVec[i]->ContainsNodes( nn0, nn1 ) )
        {
            return m_EdgeVec[i];
        }
    }
    return nullptr;
}

// Find 'next' node of Edge i.  Walking the face's edge loop in the 'next' direction will
// result in a properly ordered set of points.
PGNode * PGFace::FindPrevNode( int istart ) const
{
    int nedge = m_EdgeVec.size();

    if ( nedge == 0 )
    {
        return nullptr;
    }

    // Preserve input index.
    istart = clampCyclic( istart, nedge );

    // Start ithis at input index
    int ithis = istart;
    PGEdge *ethis = m_EdgeVec[ithis];

    int inext = ithis + 1;
    if ( inext >= nedge )
    {
        inext -= nedge;
    }
    PGEdge *enext = m_EdgeVec[inext];

    // Loop until ethis != enext.  I.e. we are not on a double-back edge.  This will seldom occur, but we can
    // not consistently determine the direction of the loop on a double-back edge.
    int iloop = 0;
    while ( ethis == enext && iloop < nedge )
    {
        ithis = inext;
        ethis = enext;

        inext++;
        if ( inext >= nedge )
        {
            inext -= nedge;
        }
        enext = m_EdgeVec[ inext ];

        iloop++;
    }

    // Should be imposible. Looped through all edges and they were all identical.
    if ( ethis == enext )
    {
        return nullptr;
    }

    PGNode *nprev = nullptr;
    if ( enext->ContainsNode( ethis->m_N0 ) )
    {
        nprev = ethis->m_N1;
    }
    else
    {
        nprev = ethis->m_N0;
    }

    // Un-wind earlier advancement.
    while ( ithis != istart )
    {
        ithis--;
        if ( ithis < 0 )
        {
            ithis += nedge;
        }

        ethis = m_EdgeVec[ ithis ];
        nprev = ethis->OtherNode( nprev );
    }

    return nprev;
}

void PGFace::GetNodes( vector< PGNode* > & nodVec ) const
{
    nodVec.clear();
    PGNode *nprev = FindPrevNode( 0 );

    if ( !nprev )
    {
        return;
    }

    nodVec.push_back( nprev );

    for ( int i = 0 ; i < ( int )m_EdgeVec.size() ; i++ )
    {
        PGEdge *e = m_EdgeVec[i];

        PGNode *nnext = e->OtherNode( nprev );

        nodVec.push_back( nnext );

        nprev = nnext;
    }
}

void PGFace::GetOtherNodes( vector< PGNode* > & nodVec, const vector< PGNode* > & skipNodVec ) const
{
    GetNodes( nodVec );
    vector_remove_vector( nodVec, skipNodVec );
}

void PGFace::GetNodesAsTris( vector < PGNode* > & trinodVec )
{
    if ( m_TriNodeVec.empty() )
    {
        Triangulate();
    }

    trinodVec = m_TriNodeVec;
}

void PGFace::Triangulate()
{
    vector < PGNode* > nodVec;
    GetNodes(nodVec);
    Triangulate( nodVec, m_Nvec, m_TriNodeVec );
}

void PGFace::Triangulate( const vector < PGNode* > &nodVec, const vec3d & nvec, vector < PGNode* > &triNodeVec )
{
    Triangulate_DBA( nodVec, nvec, triNodeVec );
}

void PGFace::Triangulate_triangle( const vector < PGNode* > &nodVec, const vec3d & nvec, vector < PGNode* > &triNodeVec )
{
    triNodeVec.clear();

    // index to size-1 because first/last point is repeated.
    int npt = nodVec.size() - 1;

    for ( int i = 0; i < npt; i++ )
    {
        if ( !nodVec[i] )
        {
            printf( "PGFace Invalid in Triangulate_triangle\n" );
            return;
        }
    }

    if ( npt < 3 )
    {
        return;
    }

    if ( npt == 3 )
    {
        if ( nodVec[ 0 ] && nodVec[ 1 ] && nodVec[ 2 ] )
        {
            triNodeVec.push_back( nodVec[ 0 ] );
            triNodeVec.push_back( nodVec[ 1 ] );
            triNodeVec.push_back( nodVec[ 2 ] );
        }
        else
        {
            printf( "At least one of three nodes invalid in PGFace::Triangulate_triangle\n" );
        }
        return;
    }

    // Get node data into simple point vector.
    vector < vec3d > ptVec( npt );
    for ( int i = 0; i < npt; i++ )
    {
        ptVec[i] = nodVec[i]->m_Pt->m_Pnt;
    }

    // Rotate along normal.
    Matrix4d mat;
    mat.rotatealongX( nvec );
    mat.xformvec( ptVec );

    //==== Dump Into Triangle ====//
    context* ctx;
    triangleio in, out;
    int tristatus = TRI_NULL;

    // init
    ctx = triangle_context_create();

    memset( &in, 0, sizeof( in ) ); // Load Zeros
    memset( &out, 0, sizeof( out ) );

    //==== PreAllocate Data For In/Out ====//
    in.pointlist    = ( REAL * ) malloc( npt * 2 * sizeof( REAL ) );
    out.pointlist   = nullptr;

    in.segmentlist  = ( int * ) malloc( npt * 2 * sizeof( int ) );
    out.segmentlist  = nullptr;
    out.trianglelist  = nullptr;

    in.numberofpointattributes = 0;
    in.pointattributelist = nullptr;
    in.pointmarkerlist = nullptr;
    in.numberofholes = 0;
    in.numberoftriangles = 0;
    in.numberofpointattributes = 0;
    in.numberofedges = 0;
    in.trianglelist = nullptr;
    in.trianglearealist = nullptr;
    in.edgelist = nullptr;
    in.edgemarkerlist = nullptr;
    in.segmentmarkerlist = nullptr;

    //==== Load Points into Triangle Struct ====//
    in.numberofpoints = npt;
    in.numberofsegments = npt;

    int ptcnt = 0;
    int segcnt = 0;

    int firstseg = segcnt;
    for ( int j = 0 ; j < ( int )ptVec.size(); j++ )
    {
        vec3d pnt = ptVec[j];

        in.pointlist[ptcnt] = pnt.y();
        ptcnt++;
        in.pointlist[ptcnt] = pnt.z();
        ptcnt++;

        in.segmentlist[2 * segcnt] = segcnt;
        if ( j == ptVec.size() - 1 )
        {
            in.segmentlist[2 * segcnt + 1] = firstseg;
        }
        else
        {
            in.segmentlist[2 * segcnt + 1] = segcnt + 1;
        }
        segcnt++;
    }

    char cmdline[] = "zpQ";

    //==== Constrained Delaunay Trianglulation ====//
    tristatus = triangle_context_options( ctx, cmdline );
    if ( tristatus != TRI_OK ) printf( "triangle_context_options Error\n" );

    // Triangulate the polygon
    tristatus = triangle_mesh_create( ctx, &in );
    if ( tristatus != TRI_OK ) printf( "triangle_mesh_create Error\n" );

    if ( tristatus == TRI_OK )
    {
        triangle_mesh_copy( ctx, &out, 1, 1 );

        // Place result into node pointer vector.
        triNodeVec.resize( out.numberoftriangles * 3 );
        for ( int i = 0; i < out.numberoftriangles * 3; i++ )
        {
            triNodeVec[i] = nodVec[ out.trianglelist[i] ];
        }
    }

    for ( int i = 0; i < triNodeVec.size(); i++ )
    {
        if ( !triNodeVec[i] )
        {
            printf( "m_TriNodeVec Invalid in Triangulate_triangle\n" );
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

void PGFace::Triangulate_DBA( const vector < PGNode* > &nodVec, const vec3d & nvec, vector < PGNode* > &triNodeVec )
{
    triNodeVec.clear();

    // index to size-1 because first/last point is repeated.
    int npt = nodVec.size() - 1;

    for ( int i = 0; i < npt; i++ )
    {
        if ( !nodVec[i] )
        {
            printf( "PGFace Invalid in Triangulate_DBA\n" );
            return;
        }
    }

    if ( npt < 3 )
    {
        return;
    }

    if ( npt == 3 )
    {
        if ( nodVec[ 0 ] && nodVec[ 1 ] && nodVec[ 2 ] )
        {
            triNodeVec.push_back( nodVec[ 0 ] );
            triNodeVec.push_back( nodVec[ 1 ] );
            triNodeVec.push_back( nodVec[ 2 ] );
        }
        else
        {
            printf( "At least one of three nodes invalid in PGFace::Triangulate_DBA\n" );
        }
        return;
    }

    // Get node data into simple point vector.
    vector < vec3d > ptVec( npt );
    for ( int i = 0; i < npt; i++ )
    {
        if ( nodVec[i] )
        {
            ptVec[i] = nodVec[i]->m_Pt->m_Pnt;
        }
    }

    // Rotate along normal.
    Matrix4d mat;
    mat.rotatealongX( nvec );
    mat.xformvec( ptVec );

    dba_point* cloud = new dba_point[npt];
    for ( int i = 0 ; i < npt; i++ )
    {
        vec3d pnt = ptVec[ i ];

        cloud[ i ].x = pnt.y();
        cloud[ i ].y = pnt.z();
    }

    int nedg = npt;
    dba_edge* bounds = new dba_edge[nedg];
    int firstseg = 0;
    for ( int i = 0 ; i < nedg; i++ )
    {
        bounds[ i ].a = i;
        if ( i == nedg - 1 )
        {
            bounds[i].b = firstseg;
        }
        else
        {
            bounds[i].b = i + 1;
        }
    }

    if ( false )
    {
        static int idump = 0;
        FILE *fpdump = nullptr;

        string fname = string( "dlbtest_" ) + to_string( idump ) + string( ".txt" );
        fpdump = fopen( fname.c_str(), "w" );
        idump++;

        fprintf( fpdump, "%d\n", npt );
        for ( int i = 0; i < npt; i++ )
        {
            fprintf( fpdump, "%d %.18e %.18e\n", i, cloud[ i ].x, cloud[ i ].y );
        }

        fprintf( fpdump, "%d\n", nedg );
        for ( int i = 0; i < ( int ) nedg; i++ )
        {
            fprintf( fpdump, "%d %d %d\n", i, bounds[ i ].a, bounds[ i ].b );
        }
        fclose( fpdump );
    }

    IDelaBella2 < double > * idb = IDelaBella2 < double > ::Create();

    int verts = idb->Triangulate( npt, &cloud->x, &cloud->y, sizeof( dba_point ) );

    if ( verts > 0 )
    {
        idb->ConstrainEdges( nedg, &bounds->a, &bounds->b, sizeof( dba_edge ) );

        int tris = idb->FloodFill( false, nullptr, 1 );

        const IDelaBella2<double>::Simplex* dela = idb->GetFirstDelaunaySimplex();

        triNodeVec.reserve( tris * 3 );
        for ( int i = 0; i < tris; i++ )
        {
            // Note winding order!
            triNodeVec.push_back( nodVec[ dela->v[ 0 ]->i ] );
            triNodeVec.push_back( nodVec[ dela->v[ 2 ]->i ] );
            triNodeVec.push_back( nodVec[ dela->v[ 1 ]->i ] );

            dela = dela->next;
        }
    }
    else
    {
        printf( "DLB Error! %d\n", verts );
    }

    for ( int i = 0; i < triNodeVec.size(); i++ )
    {
        if ( !triNodeVec[i] )
        {
            printf( "m_TriNodeVec Invalid in Triangulate_DBA\n" );
        }
    }

    delete[] cloud;
    delete[] bounds;

    idb->Destroy();
}

void PGFace::ClearTris()
{
    m_TriNodeVec.clear();
}

void PGFace::AddEdge( PGEdge* e )
{
    m_EdgeVec.push_back( e );
}

void PGFace::RemoveEdge( PGEdge* e )
{
    vector_remove_val( m_EdgeVec, e );
}

void PGFace::ReplaceEdge( const PGEdge *eold, PGEdge *enew )
{
    for ( int i = 0 ; i < ( int )m_EdgeVec.size() ; i++ )
    {
        if ( eold == m_EdgeVec[i] )
        {
            m_EdgeVec[i] = enew;
        }
    }
}

bool PGFace::Contains( const PGEdge* e ) const
{
    return vector_contains_val( m_EdgeVec, const_cast < PGEdge* > ( e ) );
}

bool PGFace::Contains( const PGNode* n ) const
{
    for ( int i = 0; i < m_EdgeVec.size(); i++ )
    {
        if ( m_EdgeVec[i]->ContainsNode( n ) )
        {
            return true;
        }
    }
    return false;
}

/*
// Alternate form
bool PGFace::Contains( PGNode* n ) const
{
    for ( int i = 0; i < n->m_EdgeVec.size(); i++ )
    {
        if ( n->m_EdgeVec[i]->UsedBy( this ) )
        {
            return true;
        }
    }
    return false;
}
*/

void PGFace::EdgeForgetFace() const
{
    for ( int i = 0; i < m_EdgeVec.size(); i++ )
    {
        m_EdgeVec[i]->RemoveFace( this );
    }
}

bool PGFace::Check() const
{
    if ( m_DeleteMeFlag )
    {
        return false;
    }

    for ( int i = 0; i < m_EdgeVec.size(); i++ )
    {
        if ( !m_EdgeVec[i] )
        {
            return false;
        }
    }
    return true;
}

void PGFace::DumpMatlab() const
{
    vector < PGNode* > nodVec;
    GetNodes( nodVec );

    int num = nodVec.size();

    int i;
    printf( "\nx = [" );
    for ( i = 0; i < num - 1; i++ )
    {
        printf( "%.*e;\n", DBL_DIG + 3, nodVec[i]->m_Pt->m_Pnt.x() );
    }
    printf( "%.*e];\n", DBL_DIG + 3, nodVec[i]->m_Pt->m_Pnt.x() );

    printf( "\ny = [" );
    for ( i = 0; i < num - 1; i++ )
    {
        printf( "%.*e;\n", DBL_DIG + 3, nodVec[i]->m_Pt->m_Pnt.y() );
    }
    printf( "%.*e];\n", DBL_DIG + 3, nodVec[i]->m_Pt->m_Pnt.y() );

    printf( "\nz = [" );
    for ( i = 0; i < num - 1; i++ )
    {
        printf( "%.*e;\n", DBL_DIG + 3, nodVec[i]->m_Pt->m_Pnt.z() );
    }
    printf( "%.*e];\n", DBL_DIG + 3, nodVec[i]->m_Pt->m_Pnt.z() );

    printf( "plot3( x, y, z, 'kx-' );\n" );
    printf( "hold on\n" );

}

void PGFace::Diagnostics() const
{
    printf( "Face %d\n", m_ID );
    printf( "Edges: " );

    for ( int i = 0; i < m_EdgeVec.size(); i++ )
    {
        printf( "%3d ", m_EdgeVec[i]->m_ID );
    }
    printf( "\n" );

    for ( int i = 0; i < m_EdgeVec.size(); i++ )
    {
        m_EdgeVec[i]->Diagnostics();
    }
}

bool PGFace::Validate() const
{
    bool valid = true;
    for ( int i = 0; i < m_EdgeVec.size(); i++ )
    {
        if ( !m_EdgeVec[i] )
        {
            printf( "Face %d has invalid edge pointer %d\n", m_ID, i );
            valid = false;
            continue;
        }
        if ( !m_EdgeVec[i]->UsedBy( this ) )
        {
            printf( "Edge %d is unaware it is used by face %d\n", m_EdgeVec[i]->m_ID, m_ID );
            valid = false;
        }
    }
    return valid;
}

double PGFace::ComputeArea()
{
    vector < PGNode* > nodVec;
    GetNodesAsTris( nodVec );
    return ComputeArea( nodVec );
}

double PGFace::ComputeArea( const vector < PGNode* > &nodVec )
{
    int ntri = nodVec.size() / 3;

    double a = 0;
    for ( int i = 0; i < ntri; i++ )
    {
        int inod = 3 * i;
        vec3d v0 = nodVec[ inod ]->m_Pt->m_Pnt;
        vec3d v1 = nodVec[ inod + 1 ]->m_Pt->m_Pnt;
        vec3d v2 = nodVec[ inod + 2 ]->m_Pt->m_Pnt;

        a += area( v0, v1, v2 );
    }
    return a;
}

vec3d PGFace::ComputeCenter()
{
    vector < PGNode* > nodVec;
    GetNodesAsTris( nodVec );

    int ntri = nodVec.size() / 3;

    double a = 0;
    vec3d cen;
    for ( int i = 0; i < ntri; i++ )
    {
        int inod = 3 * i;
        vec3d v0 = nodVec[ inod ]->m_Pt->m_Pnt;
        vec3d v1 = nodVec[ inod + 1 ]->m_Pt->m_Pnt;
        vec3d v2 = nodVec[ inod + 2 ]->m_Pt->m_Pnt;

        vec3d ceni = ( v0 + v1 + v2 ) / 3.0;

        double ai = area( v0, v1, v2 );
        a += ai;
        cen += ai * ceni;
    }
    return cen / a;
}

void PGFace::WalkRegion() const
{
    for ( int i = 0; i < m_EdgeVec.size(); i++ )
    {
        PGEdge *e = m_EdgeVec[i];
        PGFace* f = e->OtherManifoldFace( this );

        if ( f )
        {
            if ( f->m_Region == -1 && f->m_Tag == m_Tag )
            {
                f->m_Region = m_Region;
                f->WalkRegion();
            }
        }
    }
}

PGNode * PGFace::FindDoubleBackNode( PGEdge* & edouble ) const
{
    edouble = nullptr;
    int nedge = m_EdgeVec.size();

    PGNode *nprev = FindPrevNode( 0 );
    for ( int i = 0; i < ( int )nedge; i++ )
    {
        PGEdge *e = m_EdgeVec[ i ];

        int inext = i + 1;
        if ( inext >= nedge )
        {
            inext = 0;
        }
        PGEdge *enext = m_EdgeVec[ inext ];

        PGNode *nnext = e->OtherNode( nprev );

        if ( e == enext )
        {
            // DoubleBack detected.
            edouble = e;
            return nnext;
        }

        nprev = nnext;
    }

    return nullptr;
}

// Split edge e0 into e0,e1.  Direction indeterminte (e1 can go before or after e0).
// e0 can also appear more than once in this face.
void PGFace::SplitEdge( PGEdge *e0, PGEdge *e1 )
{

    PGNode* ns = e0->SharedNode( e1 );
    PGNode* n1 = e1->OtherNode( ns );


    //int i0 = vector_find_val( m_EdgeVec, e0 );
    vector < int > i0vec;
    vector_find_val_multiple( m_EdgeVec, e0, i0vec );

    for ( int i = i0vec.size() - 1; i >= 0; i-- )
    {
        int nedge = m_EdgeVec.size();
        int i0 = i0vec[i];
        int iprev = clampCyclic( i0 - 1, nedge );
        PGEdge * eprev = m_EdgeVec[ iprev ];
        int inext = clampCyclic( i0 + 1, nedge );
        PGEdge * enext = m_EdgeVec[ inext ];

        if ( eprev->ContainsNode( n1 ) )
        {
            // enext->ContainsNode( n0 ); Should be true.
            // Insert e1 betweeen eprev and e0.
            vector_insert_after( m_EdgeVec, iprev, e1 );
        }
        else if ( enext->ContainsNode( n1 ) )
        {
            // eprev->ContainsNode( n0 ); Should be true.
            // Insert e1 betweeen e0 and enext.
            vector_insert_after( m_EdgeVec, i0, e1 );
        }
    }
}

void PGFace::GetHullEdges( vector < PGEdge* > & evec ) const
{
    // Initialize to full edge set.
    evec = m_EdgeVec;

    while ( vector_remove_consecutive_duplicates( evec ) );
}

double PGFace::ComputeTriQual() const
{
    vector< PGNode* > nodVec;
    GetNodes( nodVec );

    return PGFace::ComputeTriQual( nodVec[0], nodVec[1], nodVec[2] );
}

double PGFace::ComputeTriQual( const PGNode* n0, const PGNode* n1, const PGNode* n2 )
{
    double ang0, ang1, ang2;

    PGFace::ComputeCosAngles( n0, n1, n2, &ang0, &ang1, &ang2 );

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
}

void PGFace::ComputeCosAngles( const PGNode* n0, const PGNode* n1, const PGNode* n2, double* ang0, double* ang1, double* ang2 )
{
    double dsqr01 = dist_squared( n0->m_Pt->m_Pnt, n1->m_Pt->m_Pnt );
    double dsqr12 = dist_squared( n1->m_Pt->m_Pnt, n2->m_Pt->m_Pnt );
    double dsqr20 = dist_squared( n2->m_Pt->m_Pnt, n0->m_Pt->m_Pnt );

    double d01 = sqrt( dsqr01 );
    double d12 = sqrt( dsqr12 );
    double d20 = sqrt( dsqr20 );

    *ang0 = ( -dsqr12 + dsqr01 + dsqr20 ) / ( 2.0 * d01 * d20 );

    *ang1 = ( -dsqr20 + dsqr01 + dsqr12 ) / ( 2.0 * d01 * d12 );

    *ang2 = ( -dsqr01 + dsqr12 + dsqr20 ) / ( 2.0 * d12 * d20 );
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

void QuadFaceMergeProps( PGFace *f0, PGFace *f1 )
{
}

void JrefMergeProps( PGFace *f0, PGFace *f1 )
{
    f0->m_jref = f0->m_jref / 2;
    f0->m_ID = -f0->m_ID;
}

void KrefMergeProps( PGFace *f0, PGFace *f1 )
{
    f0->m_kref = f0->m_kref / 2;
    f0->m_ID = -f0->m_ID;
}

bool QuadTagMatch( PGFace *f0, PGFace *f1 )
{
    return f0->m_iQuad >= 0 &&
           f0->m_iQuad == f1->m_iQuad &&
           f0->m_Tag == f1->m_Tag;
}

bool JrefTagMatch( PGFace *f0, PGFace *f1 )
{
    return f0->m_Tag ==       f1->m_Tag &&
           f0->m_jref >= 1 && f1->m_jref >= 1 &&
           f0->m_jref !=      f1->m_jref &&
           f0->m_jref / 2 ==  f1->m_jref / 2 &&
           f0->m_ID > 0 &&    f1->m_ID > 0;
}

bool KrefTagMatch( PGFace *f0, PGFace *f1 )
{
    return f0->m_Tag ==       f1->m_Tag &&
           f0->m_kref >= 1 && f1->m_kref >= 1 &&
           f0->m_kref !=      f1->m_kref &&
           f0->m_kref / 2 ==  f1->m_kref / 2 &&
           f0->m_ID > 0 &&    f1->m_ID > 0;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

PGMesh::PGMesh( PGMulti* pgmulti )
{
    m_PGMulti = pgmulti;
    m_DeleteMeFlag = false;
}

PGMesh::~PGMesh()
{
    DumpGarbage();
    Clear();
}

void PGMesh::Clear()
{
    list< PGFace* >::iterator f;
    for ( f = m_FaceList.begin() ; f != m_FaceList.end(); ++f )
    {
        delete ( *f );
    }

    m_FaceList.clear();

    list< PGEdge* >::iterator e;
    for ( e = m_EdgeList.begin() ; e != m_EdgeList.end(); ++e )
    {
        delete ( *e );
    }

    m_EdgeList.clear();

    list< PGNode* >::iterator n;
    for ( n = m_NodeList.begin() ; n != m_NodeList.end(); ++n )
    {
        delete ( *n );
    }

    m_NodeList.clear();
}

void PGMesh::CleanUnused()
{
    // Copy list to vector because removal from list will corrupt list in-use.
    vector< PGFace* > fVec( m_FaceList.begin(), m_FaceList.end() );
    for ( int i = 0; i < fVec.size(); i++ )
    {
        PGFace *f = fVec[ i ];
        if ( f->m_EdgeVec.empty() )
        {
            RemoveFace( f );
        }
    }

    // Copy list to vector because removal from list will corrupt list in-use.
    vector< PGEdge* > eVec( m_EdgeList.begin(), m_EdgeList.end() );
    for ( int i = 0; i < eVec.size(); i++ )
    {
        PGEdge *e = eVec[ i ];
        if ( e->m_FaceVec.empty() )
        {
            RemoveEdge( e );
        }
    }

    // Copy list to vector because removal from list will corrupt list in-use.
    vector< PGNode* > nVec( m_NodeList.begin(), m_NodeList.end() );
    for ( int i = 0; i < nVec.size(); i++ )
    {
        PGNode *n = nVec[ i ];
        if ( n->m_EdgeVec.empty() )
        {
            RemoveNode( n );
        }
    }


}

PGNode* PGMesh::AddNode( PGPoint *pptr )
{
    PGNode* nptr = new PGNode( pptr );
    m_NodeList.push_back( nptr );
    nptr->m_List_it = --m_NodeList.end();

    pptr->AddConnectNode( nptr );

    return nptr;
}

void PGMesh::RemoveNode( PGNode* nptr )
{
    for ( int i = 0; i < nptr->m_EdgeVec.size(); i++ )
    {
        nptr->EdgeForgetNode( nptr->m_EdgeVec[i] );
    }
    nptr->m_EdgeVec.clear();

    nptr->m_TagUWMap.clear();

    nptr->m_Pt->RemoveConnectNode( nptr );
    nptr->m_Pt = nullptr;

    m_GarbageNodeVec.push_back( nptr );
    m_NodeList.erase( nptr->m_List_it );

    nptr->m_DeleteMeFlag = true;
}

PGEdge* PGMesh::AddEdge( PGNode* n0, PGNode* n1 )
{
    PGEdge* e = FindEdge( n0, n1 );
    if ( !e )
    {
        e = new PGEdge( n0, n1 );

        m_EdgeList.push_back( e );
        e->m_List_it = --m_EdgeList.end();

        n0->AddConnectEdge( e );
        n1->AddConnectEdge( e );
    }

    return e;
}

void PGMesh::RemoveEdge( PGEdge* e )
{
    e->m_N0->RemoveConnectEdge( e );
    e->m_N0 = nullptr;
    e->m_N1->RemoveConnectEdge( e );
    e->m_N1 = nullptr;

    for ( int i = 0; i < e->m_FaceVec.size(); i++ )
    {
        e->m_FaceVec[i]->RemoveEdge( e );
    }
    e->m_FaceVec.clear();

    m_GarbageEdgeVec.push_back( e );
    m_EdgeList.erase( e->m_List_it );

    e->m_DeleteMeFlag = true;
}

void PGMesh::RemoveEdgeMergeFaces( PGEdge* e, void ( * facemergeproperties ) ( PGFace *f0, PGFace *f1 ) )
{
    if ( e->m_FaceVec.size() == 2 )
    {
        // Grab faces to be merged.
        PGFace *f0 = e->m_FaceVec[0];
        PGFace *f1 = e->m_FaceVec[1];

        // Grab edge loops.
        vector < PGEdge * > ev0 = f0->m_EdgeVec;
        vector < PGEdge * > ev1 = f1->m_EdgeVec;

        // Find edge in each loop.
        int i0 = vector_find_val( ev0, e );
        int i1 = vector_find_val( ev1, e );

        if ( i0 >= 0 && i1 >= 0 )
        {
            // Rotate edge loop such that e is at the start.
            std::rotate( ev0.begin(), ev0.begin() + i0, ev0.end());
            std::rotate( ev1.begin(), ev1.begin() + i1, ev1.end());

            // Remove e from each edge loop.
            ev0.erase( ev0.begin() );
            ev1.erase( ev1.begin() );

            // Concatenate edge loops.
            ev0.insert( ev0.end(), ev1.begin(), ev1.end());

            // Assign combined loop to f0.
            f0->m_EdgeVec = ev0;

            RemoveEdge( e );

            facemergeproperties( f0, f1 );

            for ( int i = 0; i < ev1.size(); i++ )
            {
                ev1[ i ]->AddConnectFace( f0 );
            }

            RemoveFace( f1 );
        }
    }
}

void PGMesh::SwapEdge( PGEdge* e )
{
    if ( e->m_FaceVec.size() != 2 )
    {
        return;
    }

    PGFace *f0 = e->m_FaceVec[0];
    PGFace *f1 = e->m_FaceVec[1];

    if ( f0->m_EdgeVec.size() == 3 &&
         f1->m_EdgeVec.size() == 3 )
    {
        // Grab edge loops.
        vector < PGEdge * > ev0 = f0->m_EdgeVec;
        vector < PGEdge * > ev1 = f1->m_EdgeVec;

        // Find edge in each loop.
        int i0 = vector_find_val( ev0, e );
        int i1 = vector_find_val( ev1, e );

        if ( i0 >= 0 && i1 >= 0 )
        {
            // Rotate edge loop such that e is at the start.
            std::rotate( ev0.begin(), ev0.begin() + i0, ev0.end());
            std::rotate( ev1.begin(), ev1.begin() + i1, ev1.end());

            PGEdge * ea = ev0[1];
            PGEdge * eb = ev0[2];
            PGEdge * ec = ev1[1];
            PGEdge * ed = ev1[2];

            PGNode* na = ea->SharedNode( eb );
            PGNode* nc = ec->SharedNode( ed );

            if ( na && nc )
            {
                // Clear old edge.
                e->m_N0->RemoveConnectEdge( e );
                e->m_N1->RemoveConnectEdge( e );

                // Reconnect edge
                na->AddConnectEdge( e );
                e->m_N0 = na;
                nc->AddConnectEdge( e );
                e->m_N1 = nc;

                ea->RemoveFace( f0 );
                ea->AddConnectFace( f1 );
                // eb No change.
                ec->RemoveFace( f1 );
                ec->AddConnectFace( f0 );
                // ed No change.

                vector < PGEdge * > ev0new = { e, eb, ec };
                vector < PGEdge * > ev1new = { e, ed, ea };

                f0->m_EdgeVec = ev0new;
                f1->m_EdgeVec = ev1new;
            }
        }
    }
}

void PGMesh::CheckQualitySwapEdges()
{
    // Make vector copy of list so edges can be removed from list without invalidating active list iterator.
    vector< PGEdge* > eVec( m_EdgeList.begin(), m_EdgeList.end() );

    for ( int i = 0; i < eVec.size(); i++ )
    {
        PGEdge *e = eVec[ i ];

        if ( e->m_FaceVec.size() == 2 )
        {
            PGFace *f0 = e->m_FaceVec[0];
            PGFace *f1 = e->m_FaceVec[1];

            if ( f0->m_EdgeVec.size() == 3 &&
                 f1->m_EdgeVec.size() == 3 &&
                 f0->m_iQuad == f1->m_iQuad &&
                 f0->m_Tag == f1->m_Tag )
            {
                PGNode* n0 = e->m_N0;
                PGNode* n1 = e->m_N1;

                vector < PGNode* > commonNodVec = { n0, n1 };

                vector < PGNode* > t0nodVec;
                vector < PGNode* > t1nodVec;
                f0->GetOtherNodes( t0nodVec, commonNodVec );
                f1->GetOtherNodes( t1nodVec, commonNodVec );

                PGNode* na = t0nodVec[0];
                PGNode* nb = t1nodVec[0];

                double qa = f0->ComputeTriQual();
                double qb = f1->ComputeTriQual();
                double qc = PGFace::ComputeTriQual( n0, nb, na );
                double qd = PGFace::ComputeTriQual( n1, na, nb );

                // Require 5 degree improvement in minimum angle to bother flipping.  This reduces frivilous flips
                // and maintains mostly the original diagonal orientation.
                if ( min( qc, qd ) <= ( min( qa, qb ) + PI / 36.0 ) )
                {
                    continue;
                }

                SwapEdge( e );
            }
        }
    }

    ClearTris();
}

PGEdge* PGMesh::FindEdge( const PGNode* n0, const PGNode* n1 )
{
    PGEdge *e;
    e = n0->FindEdge( n1 );
    if ( !e )
    {
        e = n1->FindEdge( n0 );
    }
    return e;
}

PGFace* PGMesh::AddFace()
{
    PGFace* fptr = new PGFace();
    m_FaceList.push_back( fptr );
    fptr->m_List_it = --m_FaceList.end();


    return fptr;
}

PGFace* PGMesh::AddFace( PGNode* n0, PGNode* n1, PGNode* n2,
                         const vec2d &uw0, const vec2d &uw1, const vec2d &uw2,
                         const vec3d & norm, const int iQuad, const int tag, const int jref, const int kref )
{
    PGFace *f = AddFace( );
    f->m_Nvec = norm;
    f->m_iQuad = iQuad;
    f->m_Tag = tag;
    f->m_jref = jref;
    f->m_kref = kref;

    PGEdge *e1 = AddEdge( n0, n1 );
    PGEdge *e2 = AddEdge( n1, n2 );
    PGEdge *e3 = AddEdge( n2, n0 );

    n0->m_TagUWMap[ f->m_Tag ] = uw0;
    n1->m_TagUWMap[ f->m_Tag ] = uw1;
    n2->m_TagUWMap[ f->m_Tag ] = uw2;

    e1->AddConnectFace( f );
    e2->AddConnectFace( f );
    e3->AddConnectFace( f );

    f->AddEdge( e1 );
    f->AddEdge( e2 );
    f->AddEdge( e3 );

    return f;
}

void PGMesh::RemoveFace( PGFace* fptr )
{
    vector < PGEdge * > ev = fptr->m_EdgeVec;

    for ( int i = 0; i < ev.size(); i++ )
    {
        ev[ i ]->RemoveFace( fptr );
    }

    fptr->ClearTris();

    m_GarbageFaceVec.push_back( fptr );
    m_FaceList.erase( fptr->m_List_it );

    fptr->m_DeleteMeFlag = true;
}

void PGMesh::RemoveNegativeiQuadFaces()
{
    int iremove = 0;
    vector< PGFace* > fVec( m_FaceList.begin(), m_FaceList.end() );

    for ( int i = 0; i < fVec.size(); i++ )
    {
        PGFace *f = fVec[i];

        if ( f->m_iQuad < 0 )
        {
            RemoveFace( f );
            iremove++;
        }
    }
    // printf( "Removed %d\n", iremove );

    CleanUnused();

    DumpGarbage();
}

void PGMesh::MergeNodes( PGNode* na, PGNode* nb )
{
    // Copy unique UW map information from nb to na.
    for ( map < int, vec2d >::iterator it0 = nb->m_TagUWMap.begin(); it0 != nb->m_TagUWMap.end(); ++it0 )
    {
        int tag = it0->first;
        vec2d uw0 = it0->second;

        // Try to find uw with matching tag in na's map.
        map < int, vec2d >::iterator it1 = na->m_TagUWMap.find( tag );

        // If there is no existing entry, add tag UW pair to node.
        if ( it1 == na->m_TagUWMap.end() )
        {
            na->m_TagUWMap[ tag ] = uw0;
        }
    }

    PGEdge *eab = na->FindEdge( nb );
    if ( eab ) // Edge between na and nb exists.
    {
        RemoveEdge( eab );
    }

    // Look for edges with common destinations.
    vector < PGEdge* > evec = nb->m_EdgeVec;
    for ( int i = 0; i < evec.size(); i++ )
    {
        PGEdge *e = evec[i];

        PGNode *nc = e->OtherNode( nb );

        PGEdge *e2 = na->FindEdge( nc );

        MergeEdges( e2, e );
    }

    evec = nb->m_EdgeVec;
    for ( int i = 0; i < evec.size(); i++ )
    {
        PGEdge *e = evec[i];

        e->ReplaceNode( nb, na );
        na->AddConnectEdge( e );
        nb->RemoveConnectEdge( e );
    }

    RemoveNode( nb );
}

int PGMesh::MergeCoincidentNodes()
{
    vector< vec3d > allPntVec( m_NodeList.size() );

    // Make vector copy of list so nodes can be removed from list without invalidating active list iterator.
    vector< PGNode* > nVec( m_NodeList.begin(), m_NodeList.end() );

    for ( int i = 0; i < nVec.size(); i++ )
    {
        PGNode *n = nVec[ i ];
        allPntVec[ i ] = n->m_Pt->m_Pnt;
    }

    PntNodeCloud pnCloud;
    pnCloud.AddPntNodes( allPntVec );

    // m_BBox.GetLargestDist() *
    double tol = 1.0e-10;

    if ( tol < 1.0e-10 )
    {
        tol = 1.0e-10;
    }

    IndexPntNodes( pnCloud, tol );

    int nmerge = 0;

    vector < bool > masflag( nVec.size(), false );

    for ( int islave = 0 ; islave < ( int )nVec.size() ; islave++ )
    {
        if ( !(pnCloud.UsedNode( islave )) ) // This point is a NanoFlann slave.
        {
            int imaster = pnCloud.GetNodeBaseIndex( islave );
            masflag[imaster] = true;
            // printf( "Merge %d master: %d slave: %d\n", nmerge, imaster, islave );
            //vec3d p = nVec[imaster]->m_Pnt;
            //printf( "%f %f %f\n", p.x(), p.y(), p.z() );

            MergeNodes( nVec[imaster], nVec[islave] );

            nmerge++;
        }
    }

    // printf( "%d coincident nodes merged\n", nmerge );


//    for ( int imaster = 0 ; imaster < ( int )nVec.size() ; imaster++ )
//    {
//        if ( masflag[ imaster ] )
//        {
//            vec3d p = nVec[imaster]->m_Pnt;
//            printf( "%f %f %f\n", p.x(), p.y(), p.z() );
//        }
//    }


    CleanUnused();

    DumpGarbage();

    ClearTris();
    return nmerge;
}

void PGMesh::MergeEdges( PGEdge *ea, PGEdge *eb )
{
    if ( ea && eb && ea != eb )
    {
        for ( int i = 0; i < eb->m_FaceVec.size(); i++ )
        {
            PGFace *f = eb->m_FaceVec[i];

            ea->AddConnectFace( f );
            f->ReplaceEdge( eb, ea );

        }
        eb->m_FaceVec.clear();

        RemoveEdge( eb );
    }
}

int PGMesh::MergeDuplicateEdges()
{
    int nmerged = 0;
    vector < bool > dupflag( m_EdgeList.size(), false );

    // Copy list to vector because removal from list will corrupt list in-use.
    vector< PGEdge* > eVec( m_EdgeList.begin(), m_EdgeList.end() );
    for ( int i = 0; i < eVec.size(); i++ )
    {
        if ( !dupflag[i] )
        {
            PGEdge *e1 = eVec[ i ];
            for ( int j = i + 1; j < eVec.size(); j++ )
            {
                if ( !dupflag[j] )
                {
                    PGEdge *e2 = eVec[ j ];
                    if ( e2->ContainsNode( e1->m_N0 ) && e2->ContainsNode( e1->m_N1 ))
                    {
                        MergeEdges( e1, e2 );
                        dupflag[ j ] = true;
                        nmerged++;
                    }
                }
            }
        }
    }

    // printf( "%d duplicate edges merged.\n", nmerged );

    CleanUnused();

    DumpGarbage();

    ClearTris();
    return nmerged;
}

int PGMesh::RemoveDegenEdges()
{
    int ndegen = 0;

    vector< PGEdge* > eVec( m_EdgeList.begin(), m_EdgeList.end() );
    for ( int i = 0; i < eVec.size(); i++ )
    {
        PGEdge *e1 = eVec[ i ];

        if ( e1->m_N0 == e1->m_N1 )
        {
            RemoveEdge( e1 );
            ndegen++;
        }
    }

    // printf( "%d degen edges removed\n", ndegen );

    CleanUnused();

    DumpGarbage();

    ClearTris();
    return ndegen;
}

int PGMesh::RemoveDegenFaces()
{
    int ndegen = 0;

    // Copy list to vector because removal from list will corrupt list in-use.
    vector< PGFace* > fVec( m_FaceList.begin(), m_FaceList.end() );
    for ( int i = 0; i < fVec.size(); i++ )
    {
        PGFace *f = fVec[ i ];

        vector < PGEdge* > evec;
        f->GetHullEdges( evec );

        if ( evec.empty() )
        {
            RemoveFace( f );
            ndegen++;
        }
    }

    // printf( "%d degen faces removed\n", ndegen );

    CleanUnused();

    DumpGarbage();

    ClearTris();
    return ndegen;
}


void PGMesh::RemoveNodeMergeEdges( PGNode* n )
{
    if ( n->m_EdgeVec.size() == 2 )
    {
        PGEdge *e0 = n->m_EdgeVec[0];
        PGEdge *e1 = n->m_EdgeVec[1];

        PGNode *n1 = e1->OtherNode( n );

        e0->ReplaceNode( n, n1 ); // Only changes node pointer, does not recurse.
        n1->AddConnectEdge( e0 ); // Adds edge to node's edge vec.

        RemoveEdge( e1 ); // Calls both nodes to RemoveConnectEdge - removes edge from node edgevec
                          // Sets nodes to nullptr
                          // Removes edge from all faces.
                          // Clears face list.
                          // Adds to garbage vec
                          // Removes from mesh edge list.
                          // Sets deleteme flag

        RemoveNode( n ); // Removes node from all edges
                          // Clears edge list
                          // Clears TagUWMap
                          // Removes node from point
                          // Null's point pointer
                          // Adds to garbage vec
                          // Removes from mesh node list.
                          // Sets deleteme flag

    }
}

bool PGMesh::Check()
{
    list< PGFace* >::iterator f;
    for ( f = m_FaceList.begin() ; f != m_FaceList.end(); ++f )
    {
        if ( !( *f )->Check() )
        {
            return false;
        }
    }

    list< PGEdge* >::iterator e;
    for ( e = m_EdgeList.begin() ; e != m_EdgeList.end(); ++e )
    {
        if ( !( *e )->Check() )
        {
            return false;
        }
    }

    list< PGNode* >::iterator n;
    for ( n = m_NodeList.begin() ; n != m_NodeList.end(); ++n )
    {
        if ( !( *n )->Check() )
        {
            return false;
        }
    }

    return true;
}

void PGMesh::FindAllDoubleBackNodes()
{
    m_DoubleBackNode.clear();

    int i, j;

    list< PGNode* >::iterator n;
    for ( n = m_NodeList.begin() ; n != m_NodeList.end(); ++n )
    {
        if ( ( *n )->DoubleBackNode( i, j ) )
        {
            m_DoubleBackNode.push_back( *n );
        }
    }
}

void PGMesh::SealDoubleBackNodes()
{
    for ( int i = 0; i < m_DoubleBackNode.size(); i++ )
    {
        m_DoubleBackNode[i]->SealDoubleBackNode( this );
    }
    m_DoubleBackNode.clear();

    CleanUnused();

    DumpGarbage();
}

void PGMesh::ResetEdgeLoopFlags()
{
    list< PGEdge* >::iterator e;
    for ( e = m_EdgeList.begin() ; e != m_EdgeList.end(); ++e )
    {
        ( *e )->m_InLoopFlag = false;
        ( *e )->m_InCurrentLoopFlag = false;
    }
}


void PGMesh::ExtendWake( vector < PGEdge * > & wake, PGEdge *e, const PGNode *n, const bool ContinueCoPlanarWakes )
{
    e->m_InCurrentLoopFlag = true;

    int nedg = n->m_EdgeVec.size();

    for ( int i = 0; i < nedg; i++ )
    {
        PGEdge * ei = n->m_EdgeVec[ i ];
        if ( ei && ei != e && !ei->m_InLoopFlag && !ei->m_InCurrentLoopFlag )
        {
            if ( ei->WakeEdge( this, ContinueCoPlanarWakes ) )
            {
                PGNode * ni = ei->OtherNode( n );

                wake.push_back( ei );
                ei->m_InLoopFlag = true;

                ExtendWake( wake, ei, ni, ContinueCoPlanarWakes );
                return;
            }
        }
    }

    e->m_InCurrentLoopFlag = false;
}


void PGMesh::IdentifyWakes( const bool ContinueCoPlanarWakes )
{
    m_WakeVec.clear();

    list< PGEdge* >::iterator e;
    for ( e = m_EdgeList.begin() ; e != m_EdgeList.end(); ++e )
    {
        if ( !( ( *e )->m_InLoopFlag ) && ( *e )->WakeEdge( this, ContinueCoPlanarWakes ) )
        {
            (*e)->m_InLoopFlag = true;

            vector < PGEdge * > wake;
            wake.push_back( *e );

            ExtendWake( wake, (*e), (*e)->m_N0, ContinueCoPlanarWakes );

            std::reverse( wake.begin(), wake.end() );

            ExtendWake( wake, (*e), (*e)->m_N1, ContinueCoPlanarWakes );


            m_WakeVec.push_back( wake );
        }
    }

    // printf( "IdentifyWakes() %d wakes found.\n", m_WakeVec.size() );

    ResetEdgeLoopFlags();
}

void PGMesh::StartMatlab()
{
    printf( "clear all;\n" );
    printf( "close all;\n" );
    printf( "format compact;\n" );
}

PGEdge * PGMesh::SplitEdge( PGEdge *e0, PGNode *n )
{
    PGNode *n1 = e0->m_N1;

    e0->ReplaceNode( n1, n );
    n1->RemoveConnectEdge( e0 );
    n->AddConnectEdge( e0 );

    PGEdge *e1 = AddEdge( n, n1 );

    // Copy face vector
    e1->m_FaceVec = e0->m_FaceVec;

    //e0->DumpMatlab();
    //e1->DumpMatlab();


    vector <PGFace* > fv = e0->m_FaceVec;

    for ( int i = 0; i < fv.size(); i++ )
    {
        PGFace *f = fv[i];
        f->SplitEdge( e0, e1 );
        //f->DumpMatlab();
    }

    return e1;
}

PGEdge * PGMesh::SplitEdge( PGEdge *e, const double t, PGNode *n0 )
{
    PGNode *n1 = e->OtherNode( n0 );

    vec3d p = ( 1.0 - t ) * n0->m_Pt->m_Pnt + t * n1->m_Pt->m_Pnt;

    PGPoint *pnt = m_PGMulti->AddPoint( p );
    PGNode * newnode = AddNode( pnt );

    // Loop over all uw's in n0's map.
    for ( map < int, vec2d >::iterator it0 = n0->m_TagUWMap.begin(); it0 != n0->m_TagUWMap.end(); ++it0 )
    {
        int tag = it0->first;
        vec2d uw0 = it0->second;

        // Try to find uw with matching tag in n1's map.
        vec2d uw1;
        if ( n1->GetUW( tag, uw1 ) )
        {
            vec2d uw = ( 1.0 - t ) * uw0 + t * uw1;

            newnode->m_TagUWMap[ tag ] = uw;
        }
    }

    return SplitEdge( e, newnode );
}


void PGMesh::SplitFaceFromDoubleBackNode( PGFace *f, const PGEdge *e, PGNode *n )
{
    vector < PGEdge* > ehull;
    f->GetHullEdges( ehull );

    // Build face from hull to visualize.
    if ( false )
    {
        PGFace *fdummy = AddFace();
        for ( int i = 0; i < ehull.size(); i++ )
        {
            fdummy->AddEdge( ehull[i] );
        }
        fdummy->DumpMatlab();
        RemoveFace( fdummy );
    }

    // Find other node of double-back edge.  Will be used to establish a direction.
    PGNode *nother = e->OtherNode( n );

    vec3d dir = n->m_Pt->m_Pnt - nother->m_Pt->m_Pnt;
    dir.normalize();

    int imindist = -1;
    double mindist = DBL_MAX;
    double tmindist = 0;

    for ( int i = 0; i < ehull.size(); i++ )
    {
        PGEdge* eh = ehull[i];

        double s, t;
        line_line_intersect( nother->m_Pt->m_Pnt, n->m_Pt->m_Pnt, eh->m_N0->m_Pt->m_Pnt, eh->m_N1->m_Pt->m_Pnt, &s, &t );

        vec3d p = ( 1.0 - t ) * eh->m_N0->m_Pt->m_Pnt + t * eh->m_N1->m_Pt->m_Pnt;

        double d = dist( n->m_Pt->m_Pnt, p );

        if ( d < mindist && s > 1.0 )
        {
            mindist = d;
            imindist = i;
            tmindist = t;
        }
    }

    if ( imindist >= 0 )
    {
        PGEdge * e2split = ehull[imindist];
        PGNode * n2 = e2split->m_N1;

        PGEdge * enew0 = SplitEdge( e2split, tmindist, e2split->m_N0 );

        PGNode * newnode = enew0->OtherNode( n2 );

        PGEdge * enew = AddEdge( n, newnode );

        SplitFace( f, enew );
    }
}

// Do opposite of remove edge merge faces.
void PGMesh::SplitFace( PGFace *f0, PGEdge *e )
{
    PGNode * n0 = e->m_N0;
    PGNode * n1 = e->m_N1;

    vector < PGEdge * > ev = f0->m_EdgeVec;
    int nev = ev.size();

    // Find n0 split.
    int isplit = -1;
    for ( int i = 0; i < nev; i++ )
    {
        int inext = i + 1;
        if ( inext >= nev )
        {
            inext -= nev;
        }

        if ( ev[i]->ContainsNode( n0 ) && ev[inext]->ContainsNode( n0 ) )
        {
            isplit = inext;
        }
    }

    // Split not found.
    if ( isplit == -1 )
    {
        return;
    }

    // Rotate isplit to be first in ev.
    std::rotate( ev.begin(), ev.begin() + isplit, ev.end() );


    // find n1 split.
    isplit = -1;
    for ( int i = 0; i < nev; i++ )
    {
        int inext = i + 1;
        if ( inext >= nev )
        {
            inext -= nev;
        }

        if ( ev[i]->ContainsNode( n1 ) && ev[inext]->ContainsNode( n1 ) )
        {
            isplit = inext;
        }
    }

    // Split not found.
    if ( isplit == -1 )
    {
        return;
    }

    PGFace *f1 = AddFace();
    f1->m_iQuad = f0->m_iQuad;
    f1->m_Nvec = f0->m_Nvec;
    f1->m_Tag = f0->m_Tag;
    f1->m_jref = f0->m_jref;
    f1->m_kref = f0->m_kref;

    // Build two new edge vectors.
    vector < PGEdge * > ev0;
    vector < PGEdge * > ev1;

    for ( int i = 0; i < isplit; i++ )
    {
        ev0.push_back( ev[i] );
    }
    ev0.push_back( e );
    f0->m_EdgeVec = ev0;

    for ( int i = isplit; i < nev; i++ )
    {
        ev1.push_back( ev[i] );
        if ( !vector_contains_val( ev0, ev[i] ) )
        {
            ev[i]->RemoveFace( f0 );
        }
        ev[i]->AddConnectFace( f1 );
    }
    ev1.push_back( e );
    f1->m_EdgeVec = ev1;

    e->AddConnectFace( f0 );
    e->AddConnectFace( f1 );
}

void PGMesh::MakeRegions()
{
    m_Regions.clear();

    // Initialize faces as members of no region.
    list< PGFace* >::iterator f;
    for ( f = m_FaceList.begin() ; f != m_FaceList.end(); ++f )
    {
        ( *f )->m_Region = -1;
    }

    for ( f = m_FaceList.begin() ; f != m_FaceList.end(); ++f )
    {
        // Start new region with un-classified face.
        if ( ( *f )->m_Region == -1 )
        {
            int reg = m_Regions.size();
            m_Regions.push_back( *f );
            (*f)->m_Region = reg;
            (*f)->WalkRegion();
        }
    }
}

void PGMesh::CullOrphanThinRegions( const double tol )
{
    int nreg = m_Regions.size();
    vector < double > region_area( nreg, 0.0 );

    list< PGFace* >::iterator fit;
    for ( fit = m_FaceList.begin() ; fit != m_FaceList.end(); ++fit )
    {
        region_area[ ( *fit )->m_Region ] += (*fit)->ComputeArea();
    }

    vector < int > region_thick( nreg );
    vector < int > region_tag( nreg );

    map < int, double > tag_area;
    for ( int i = 0; i < nreg; i++ )
    {
        // Representative face.
        PGFace* f = m_Regions[i];

        region_tag[ i ] = f->m_Tag;
        int part = m_PGMulti->GetPart( region_tag[ i ] );
        region_thick[ i ] = m_PGMulti->GetThickThin( part );
        tag_area[ region_tag[ i ] ] += region_area[ i ];
    }

    vector < bool > region_cull( nreg, false );
    for ( int i = 0; i < nreg; i++ )
    {
        if ( !region_thick[i] && region_area[i] < tol * tag_area[ region_tag[i] ] )
        {
            region_cull[i] = true;
        }
    }

    // Copy list to vector because removal from list will corrupt list in-use.
    vector< PGFace* > fVec( m_FaceList.begin(), m_FaceList.end() );
    for ( int i = 0; i < fVec.size(); i++ )
    {
        PGFace *f = fVec[ i ];
        if ( region_cull[ f->m_Region ] )
        {
            RemoveFace( f );
        }
    }

    CleanUnused();

    DumpGarbage();
}

void PGMesh::Triangulate()
{
    vector< PGFace* > fVec( m_FaceList.begin(), m_FaceList.end() );

    for ( int iface = 0; iface < fVec.size(); iface++ )
    {
        PGFace *fpoly = fVec[iface];

        if ( fpoly->m_EdgeVec.size() > 3 )
        {
            vector < PGNode * > nodVec;
            fpoly->GetNodesAsTris( nodVec );

            int npt = nodVec.size();
            int ntri = npt / 3;

            for ( int i = 0; i < ntri; i++ )
            {
                int inod = 3 * i;
                nodVec[ inod ];

                PGFace *f = AddFace();

                f->m_Nvec = fpoly->m_Nvec;
                f->m_iQuad = fpoly->m_iQuad;
                f->m_Tag = fpoly->m_Tag;
                f->m_jref = fpoly->m_jref;
                f->m_kref = fpoly->m_kref;

                PGEdge *e1 = AddEdge( nodVec [ inod ], nodVec[ inod + 1 ] );
                PGEdge *e2 = AddEdge( nodVec [ inod + 1 ], nodVec[ inod + 2 ] );
                PGEdge *e3 = AddEdge( nodVec [ inod + 2 ], nodVec[ inod ] );

                e1->AddConnectFace( f );
                e2->AddConnectFace( f );
                e3->AddConnectFace( f );

                f->AddEdge( e1 );
                f->AddEdge( e2 );
                f->AddEdge( e3 );
            }

            RemoveFace( fpoly );
        }
    }
}

void PGMesh::ClearTris()
{
    list< PGFace* >::iterator f;
    for ( f = m_FaceList.begin() ; f != m_FaceList.end(); ++f )
    {
        ( *f )->ClearTris();
    }
}

void PGMulti::Report()
{
    ResetPointNumbers();

    printf( "m_TagNames %d entries\n", m_TagNames.size() );
    for ( auto it = m_TagNames.begin(); it != m_TagNames.end(); it++ )
    {
        printf( "%d %s\n", it->first, it->second.c_str() );
    }
    printf( "\n" );

    printf( "m_TagIDs %d entries\n", m_TagIDs.size() );
    for ( auto it = m_TagIDs.begin(); it != m_TagIDs.end(); it++ )
    {
        printf( "%d %s\n", it->first, it->second.c_str() );
    }
    printf( "\n" );

    printf( "m_ThickVec %d entries\n", m_ThickVec.size() );
    for ( int i = 0; i < m_ThickVec.size(); i++ )
    {
        printf( "%d %d\n", i, m_ThickVec[i] );
    }
    printf( "\n" );

    printf( "m_TypeVec %d entries\n", m_TypeVec.size() );
    for ( int i = 0; i < m_TypeVec.size(); i++ )
    {
        printf( "%d %d\n", i, m_TypeVec[i] );
    }
    printf( "\n" );

    printf( "m_WminVec %d entries\n", m_WminVec.size() );
    for ( int i = 0; i < m_WminVec.size(); i++ )
    {
        printf( "%d %g\n", i, m_WminVec[i] );
    }
    printf( "\n" );

    printf( "m_TagKeys %d entries\n", m_TagKeys.size() );
    for ( int i = 0; i < m_TagKeys.size(); i++ )
    {
        printf( "%d     ", i + 1 );
        for ( int j = 0; j < m_TagKeys[i].size(); j++ )
        {
            printf( "%d ", m_TagKeys[i][j] );
        }
        printf( "\n" );
    }
    printf( "\n" );

    printf( "m_SingleTagMap %d entries\n", m_SingleTagMap.size() );
    for ( auto it = m_SingleTagMap.begin(); it != m_SingleTagMap.end(); it++ )
    {
        printf( "%d     ", it->second );
        for ( int j = 0; j < (it->first).size(); j++ )
        {
            printf( "%d ", (it->first)[j] );
        }
        printf( "\n" );
    }
    printf( "\n" );

    printf( "GetGID( tag )\n" );
    for ( int i = 0; i < m_TagKeys.size(); i++ )
    {
        printf( "%d %s\n", i + 1, GetGID( i + 1 ).c_str() );
    }
    printf( "\n" );

    printf( "GetPart( tag )\n" );
    for ( int i = 0; i < m_TagKeys.size(); i++ )
    {
        printf( "%d %d\n", i + 1, GetPart( i + 1 ) );
    }
    printf( "\n" );


    PGMesh *pgm = GetActiveMesh();


    printf( "%10d   Nodes\n", pgm->m_NodeList.size() );
    printf( "%10d   Edges\n", pgm->m_EdgeList.size() );
    printf( "%10d   Faces\n", pgm->m_FaceList.size() );

    int inode = 1; // Start numbering at 1
    list< PGNode* >::iterator n;
    for ( n = pgm->m_NodeList.begin() ; n != pgm->m_NodeList.end(); ++n )
    {
        inode++;
    }


    printf( "Non-manifold edges\n" );
    int iedge = 1;
    list< PGEdge* >::iterator e;
    for ( e = pgm->m_EdgeList.begin() ; e != pgm->m_EdgeList.end(); ++e )
    {
        if ( ( *e )->m_FaceVec.size() != 2 )
        {
            printf( "Edge %d has %d faces\n", iedge, ( *e )->m_FaceVec.size() );
            printf( "    Node %d %f %f %f\n", ( *e )->m_N0->m_Pt->m_ID + 1, ( *e )->m_N0->m_Pt->m_Pnt.x(), ( *e )->m_N0->m_Pt->m_Pnt.y(), ( *e )->m_N0->m_Pt->m_Pnt.z() );
            printf( "    Node %d %f %f %f\n", ( *e )->m_N1->m_Pt->m_ID + 1, ( *e )->m_N1->m_Pt->m_Pnt.x(), ( *e )->m_N1->m_Pt->m_Pnt.y(), ( *e )->m_N1->m_Pt->m_Pnt.z() );
        }

        iedge++;
    }



    list< PGFace* >::iterator f;
    for ( f = pgm->m_FaceList.begin() ; f != pgm->m_FaceList.end(); ++f )
    {

    }


    printf( "Done\n" );


}

void PGMesh::ResetEdgeNumbers()
{
    int iedge = 1; // Start numbering at 1
    list< PGEdge* >::iterator e;
    for ( e = m_EdgeList.begin() ; e != m_EdgeList.end(); ++e )
    {
        // Assign ID number.
        ( *e )->m_ID = iedge;
        iedge++;
    }
}

void PGMesh::ResetFaceNumbers()
{
    int iface = 1; // Start numbering at 1
    list< PGFace* >::iterator f;
    for ( f = m_FaceList.begin() ; f != m_FaceList.end(); ++f )
    {
        // Assign ID number.
        ( *f )->m_ID = iface;
        iface++;
    }
}

bool PGMesh::Validate()
{
    bool valid = true;
    for ( list< PGNode* >::iterator n = m_NodeList.begin() ; n != m_NodeList.end(); ++n )
    {
        if ( (*n) &&  !(*n)->Validate() )
        {
            valid = false;
        }
    }

    if ( valid )
    {
        printf( "Nodes are valid.\n" );
    }
    else
    {
        printf( "Nodes are not valid.\n" );
    }

    valid = true;
    for ( list< PGEdge* >::iterator e = m_EdgeList.begin() ; e != m_EdgeList.end(); ++e )
    {
        if ( (*e) &&  !(*e)->Validate() )
        {
            valid = false;
        }
    }

    if ( valid )
    {
        printf( "Edges are valid.\n" );
    }
    else
    {
        printf( "Edges are not valid.\n" );
    }

    valid = true;
    for ( list< PGFace* >::iterator f = m_FaceList.begin() ; f != m_FaceList.end(); ++f )
    {
        if ( (*f) &&  !(*f)->Validate() )
        {
            valid = false;
        }
    }

    if ( valid )
    {
        printf( "Faces are valid.\n" );
    }
    else
    {
        printf( "Faces are not valid.\n" );
    }

    return valid;
}

void PGMesh::DumpGarbage()
{
    //==== Delete Flagged PGNodes =====//
    for ( int i = 0 ; i < ( int )m_GarbageNodeVec.size() ; i++ )
    {
        delete m_GarbageNodeVec[i];
    }
    m_GarbageNodeVec.clear();

    //==== Delete Flagged PGEdges =====//
    for ( int i = 0 ; i < ( int )m_GarbageEdgeVec.size() ; i++ )
    {
        delete m_GarbageEdgeVec[i];
    }
    m_GarbageEdgeVec.clear();

    //==== Delete Flagged PGFaces =====//
    for ( int i = 0 ; i < ( int )m_GarbageFaceVec.size() ; i++ )
    {
        delete m_GarbageFaceVec[i];
    }
    m_GarbageFaceVec.clear();
}

void PGMesh::MergeFaces( bool ( * facemergetest ) ( PGFace *f0, PGFace *f1 ), void ( * facemergeproperties ) ( PGFace *f0, PGFace *f1 )  )
{
    // Make vector copy of list so edges can be removed from list without invalidating active list iterator.
    vector< PGEdge* > eVec( m_EdgeList.begin(), m_EdgeList.end() );

    for ( int i = 0; i < eVec.size(); i++ )
    {
        PGEdge* e = eVec[i];

        // Verify removal of (*e) is OK.
        if ( e->m_FaceVec.size() == 2 )
        {
            PGFace *f0 = e->m_FaceVec[0];
            PGFace *f1 = e->m_FaceVec[1];

            if ( facemergetest( f0, f1 ) )
            {
                RemoveEdgeMergeFaces( e, facemergeproperties );
            }
        }
    }
    DumpGarbage();

    ClearTris();
}

void PGMesh::Coarsen1()
{
    ResetFaceNumbers();
    MergeFaces( &JrefTagMatch, &JrefMergeProps );

    for ( list< PGFace* >::iterator f = m_FaceList.begin(); f != m_FaceList.end(); ++f )
    {
        if ( ( *f )->m_ID > 0 )
        {
            ( *f )->m_jref = ( *f )->m_jref / 2;
            ( *f )->m_ID = -( *f )->m_ID;
        }
    }

    CleanColinearVerts();
    DumpGarbage();
    ResetFaceNumbers();
}

void PGMesh::Coarsen2()
{
    ResetFaceNumbers();
    MergeFaces( &KrefTagMatch, &KrefMergeProps );

    for ( list< PGFace* >::iterator f = m_FaceList.begin(); f != m_FaceList.end(); ++f )
    {
        if ( ( *f )->m_ID > 0 )
        {
            ( *f )->m_kref = ( *f )->m_kref / 2;
            ( *f )->m_ID = -( *f )->m_ID;
        }
    }

    CleanColinearVerts();
    DumpGarbage();
    ResetFaceNumbers();
}

void PGMesh::PolygonizeMesh()
{
    MergeFaces( &QuadTagMatch );
}

void PGMesh::CleanColinearVerts()
{
    list< PGEdge* >::iterator e;
    for ( e = m_EdgeList.begin() ; e != m_EdgeList.end(); ++e )
    {
        ( *e )->SortFaces();
    }

    // Make vector copy of list so nodes can be removed from list without invalidating active list iterator.
    vector< PGNode* > nVec( m_NodeList.begin(), m_NodeList.end() );

    int ncolinear = 0;
    for ( int i = 0; i < nVec.size(); i++ )
    {
        PGNode *n = nVec[ i ];

        if ( n->ColinearNode() )
        {
            RemoveNodeMergeEdges( n );
            ncolinear++;
        }
    }

    // printf( "%d colinear verts removed\n", ncolinear );

    DumpGarbage();
}

void PGMesh::WriteVSPGeom( FILE* file_id, const Matrix4d & XFormMat  )
{
    fprintf( file_id, "# vspgeom v2\n" );
    WriteVSPGeomPnts( file_id, XFormMat );
    WriteVSPGeomFaces( file_id );
    WriteVSPGeomParts( file_id );
    WriteVSPGeomWakes( file_id );
    WriteVSPGeomAlternateTris( file_id );
    WriteVSPGeomAlternateParts( file_id );
}

void PGMesh::WriteVSPGeomPnts( FILE* file_id, const Matrix4d & XFormMat )
{
    m_PGMulti->ResetPointNumbers();

    fprintf( file_id, "%d\n", m_NodeList.size() );

    //==== Write Out Nodes ====//
    vec3d v;

    list< PGNode* >::iterator n;
    for ( n = m_NodeList.begin() ; n != m_NodeList.end(); ++n )
    {
        // Apply Transformations
        v = XFormMat.xform( ( *n )->m_Pt->m_Pnt );
        fprintf( file_id, "%16.10g %16.10g %16.10g\n", v.x(), v.y(), v.z() ); // , tnode->m_UWPnt.x(), tnode->m_UWPnt.y() );
    }
}

void PGMesh::WriteVSPGeomFaces( FILE* file_id )
{
    fprintf( file_id, "%d\n", m_FaceList.size() );

    //==== Write Out Tris ====//
    list< PGFace* >::iterator f;
    for ( f = m_FaceList.begin() ; f != m_FaceList.end(); ++f )
    {
        vector < PGNode* > nodVec;
        ( *f )->GetNodes( nodVec );

        // index to size-1 because first/last point is repeated.
        int npt = nodVec.size() - 1;

        fprintf( file_id, "%d", npt );
        for ( int i = 0; i < npt; i++ )
        {
            fprintf( file_id, " %d", nodVec[i]->m_Pt->m_ID + 1 );
        }
        fprintf( file_id, "\n" );
    }
}

void PGMesh::WriteVSPGeomParts( FILE* file_id )
{
    //==== Write Component IDs for each Tri =====//
    int tag;

    list< PGFace* >::iterator f;
    for ( f = m_FaceList.begin() ; f != m_FaceList.end(); ++f )
    {
        vector < PGNode* > nodVec;
        ( *f )->GetNodes( nodVec );

        // index to size-1 because first/last point is repeated.
        int npt = nodVec.size() - 1;

        tag = ( *f )->m_Tag;
        int part = m_PGMulti->GetPart( tag );
        fprintf( file_id, "%d %d", part, tag );

        for ( int i = 0; i < npt; i++ )
        {
            vec2d uw;
            nodVec[i]->GetUW( tag, uw );

            fprintf( file_id, " %16.10g %16.10g", uw.x(), uw.y() );
        }
        fprintf( file_id, "\n" );
    }
}

void PGMesh::WriteVSPGeomWakes( FILE* file_id ) const
{
    int nwake = m_WakeVec.size();

    fprintf( file_id, "%d\n", nwake );

    for ( int iwake = 0; iwake < nwake; iwake++ )
    {
        vector< PGNode* > nodVec;
        GetNodes( m_WakeVec[iwake], nodVec );

        int nwn = nodVec.size();
        fprintf( file_id, "%d ", nwn );

        int iprt = 0;
        for ( int i = 0; i < nwn; i++ )
        {
            fprintf( file_id, "%d", nodVec[i]->m_Pt->m_ID + 1 );

            if ( iprt >= 9 || i == nwn - 1 )
            {
                fprintf( file_id, "\n" );
                iprt = 0;
            }
            else
            {
                fprintf( file_id, " " );
                iprt++;
            }
        }
    }
}

void PGMesh::WriteVSPGeomAlternateTris( FILE* file_id )
{
    //==== Write Out Tris ====//
    int iface = 1;
    list< PGFace* >::iterator f;
    for ( f = m_FaceList.begin() ; f != m_FaceList.end(); ++f )
    {
        vector < PGNode* > nodVec;
        ( *f )->GetNodesAsTris( nodVec );

        int npt = nodVec.size();

        fprintf( file_id, "%d %d", iface, npt / 3 );
        for ( int i = 0; i < npt; i++ )
        {
            fprintf( file_id, " %d", nodVec[i]->m_Pt->m_ID + 1 );
        }
        fprintf( file_id, "\n" );

        iface++;
    }
}

void PGMesh::WriteVSPGeomAlternateParts( FILE* file_id )
{
    //==== Write Component IDs for each Tri =====//
    int tag;

    int iface = 1;
    list< PGFace* >::iterator f;
    for ( f = m_FaceList.begin() ; f != m_FaceList.end(); ++f )
    {
        vector < PGNode* > nodVec;
        ( *f )->GetNodesAsTris( nodVec );

        int npt = nodVec.size();

        tag = ( *f )->m_Tag;
        int part = m_PGMulti->GetPart( tag );
        fprintf( file_id, "%d %d %d", iface, part, tag );

        for ( int i = 0; i < npt; i++ )
        {
            vec2d uw;
            nodVec[i]->GetUW( tag, uw );

            fprintf( file_id, " %16.10g %16.10g", uw.x(), uw.y() );
        }
        fprintf( file_id, "\n" );

        iface++;
    }
}

void PGMesh::WriteTagFiles( const string& file_name, vector < string > &all_fnames )
{
    int ntagfile = 0;
    int ncsffile = 0;

    std::vector < int > partvec;
    MakePartList( partvec );
    vector < SubSurface* > ssurfs = SubSurfaceMgr.GetSubSurfs();

    for ( int ipart = 0; ipart < partvec.size(); ipart++ )
    {
        int part = partvec[ ipart ];

        for ( int iss = 0; iss < ssurfs.size(); iss++ )
        {
            SubSurface *ssurf = ssurfs[iss];
            int tag = ssurf->m_Tag;

            if ( ExistPartAndTag( part, tag ) )
            {
                ntagfile++;

                if ( ssurf->GetType() == vsp::SS_CONTROL )
                {
                    ncsffile++;
                }
            }
        }
    }

    if ( ntagfile > 0 )
    {
        string base_name = GetBasename( file_name );

        string base_path, base_fname;
        GetPathFile( base_name, base_path, base_fname );

        string taglist_name = base_name + ".ALL.taglist";
        string csf_taglist_name = base_name + ".ControlSurfaces.taglist";

        FILE* taglist_fid = fopen( taglist_name.c_str(), "w" );
        FILE* csf_taglist_fid = nullptr;
        if ( ncsffile > 0 )
        {
            csf_taglist_fid = fopen( csf_taglist_name.c_str(), "w" );
        }

        if ( taglist_fid )
        {
            all_fnames.push_back( taglist_name );

            fprintf( taglist_fid, "%d\n", ntagfile );

            if ( csf_taglist_fid )
            {
                all_fnames.push_back( csf_taglist_name );
                fprintf( csf_taglist_fid, "%d\n", ncsffile );
            }

            for ( int ipart = 0; ipart < partvec.size(); ipart++ )
            {
                int part = partvec[ ipart ];

                for ( int iss = 0; iss < ssurfs.size(); iss++ )
                {
                    SubSurface *ssurf = ssurfs[iss];
                    int tag = ssurf->m_Tag;

                    if ( ExistPartAndTag( part, tag ) )
                    {
                        vector < int > parttag;
                        parttag.push_back( part );
                        parttag.push_back( tag );

                        string str = m_TagNames[ part ];
                        int pos = str.find_first_of( '_' );
                        string gname = str.substr( 0, pos );
                        string sname = str.substr( pos + 2 );

                        string ptagname = gname + sname + "_" + m_TagNames[tag];

                        string tagfile_name = base_name + ptagname + ".tag";
                        string tagfile_localname = base_fname + ptagname;

                        fprintf( taglist_fid, "%s\n", tagfile_localname.c_str() );

                        if ( csf_taglist_fid )
                        {
                            if ( ssurf->GetType() == vsp::SS_CONTROL )
                            {
                                fprintf( csf_taglist_fid, "%s\n", tagfile_localname.c_str() );
                            }
                        }

                        FILE* fid = fopen( tagfile_name.c_str(), "w" );
                        if ( fid )
                        {
                            all_fnames.push_back( tagfile_name );

                            WriteTagFile( fid, part, tag );

                            fclose( fid );
                        }
                    }
                }
            }

            fclose( taglist_fid );

            if ( csf_taglist_fid )
            {
                fclose( csf_taglist_fid );
            }
        }
    }
}

void PGMesh::WriteTagFile( FILE* file_id, const int part, const int tag )
{
    //==== Write Tri IDs for each tag =====//

    int count = 0;
    list< PGFace* >::iterator f;
    for ( f = m_FaceList.begin() ; f != m_FaceList.end(); ++f )
    {
        int singletag = ( *f )->m_Tag;

        if ( m_PGMulti->MatchPartAndTag( singletag, part, tag ) )
        {
            count++;
        }
    }
    fprintf( file_id, "%d\n\n", count );

    int iface = 1;
    for ( f = m_FaceList.begin() ; f != m_FaceList.end(); ++f )
    {
        int singletag = ( *f )->m_Tag;

        if ( m_PGMulti->MatchPartAndTag( singletag, part, tag ) )
        {
            fprintf( file_id, "%d\n", iface );
        }
        iface++;
    }
}

//==== Write Key File ====//
void PGMesh::WriteVSPGEOMKeyFile( const string & file_name, vector < string > &all_fnames )
{
    bool writethickthin = true;
    // figure out basename
    string base_name = GetBasename( file_name );
    string key_name = base_name + ".vkey";

    FILE* fid = fopen( key_name.c_str(), "w" );
    if ( !fid )
    {
        return;
    }

    all_fnames.push_back( key_name );

    int npart = 0;
    for ( int i = 0 ; i < ( int )m_TagKeys.size() ; i++ )
    {
        if ( m_TagKeys[i].size() == 1 )
        {
            npart++;
        }
    }

    // Write Out Header Information
    fprintf( fid, "# VSPGEOM v2 Tag Key File\n" );
    fprintf( fid, "%s\n", file_name.c_str() ); // Write out the file that this key information is for

    // Build GeomID set to have unique integer index instead of GeomID.
    std::set< string, greater< string > > gids;
    for ( int i = 0 ; i < ( int )m_TagKeys.size() ; i++ )
    {
        string id_list = GetTagIDs( m_TagKeys[i] );
        int pos = id_list.find( "_Surf" );
        string gid = id_list.substr( 0, pos );
        gids.insert( gid );
    }

    fprintf( fid, "%d\n", npart );
    fprintf( fid, "\n" );

    if ( writethickthin )
    {
        fprintf( fid, "# part#,geom#,surf#,gname,gid,thick\n" );
    }
    else
    {
        fprintf( fid, "# part#,geom#,surf#,gname,gid\n" );
    }


    for ( int i = 0 ; i < ( int )m_TagKeys.size() ; i++ )
    {
        if ( m_TagKeys[i].size() != 1 )
        {
            continue;
        }

        int part = GetPart( m_TagKeys[i] );

        string comp_list = GetTagNames( m_TagKeys[i] );

        // Find position of token _Surf
        int spos = comp_list.find( "_Surf" );

        string gname = comp_list.substr( 0, spos );

        string snum, ssnames, ssids;

        // Find position of first comma
        int cpos = comp_list.find( "," );
        if ( cpos != std::string::npos )
        {
            snum = comp_list.substr( spos + 5, cpos - ( spos + 5 ) );
            ssnames = comp_list.substr( cpos );
        }
        else
        {
            snum = comp_list.substr( spos + 5 );
        }

        string id_list = GetTagIDs( m_TagKeys[i] );

        // Find position of token _Surf
        spos = id_list.find( "_Surf" );
        string gid = id_list.substr( 0, spos );
        string gid_bare = gid.substr( 0, 10 );

        // Find position of first comma
        cpos = id_list.find( "," );
        if ( cpos != std::string::npos )
        {
            ssids = id_list.substr( cpos );
        }

        // Lookup Geom number
        int gnum = distance( gids.begin(), gids.find( gid ) );

        int thickthin = GetThickThin( part );

        // Write tag number and surface list to file
        if ( writethickthin )
        {
            fprintf( fid, "%d,%d,%s,%s,%s,%d\n", part, gnum, snum.c_str(), gname.c_str(), gid_bare.c_str(), thickthin );
        }
        else
        {
            fprintf( fid, "%d,%d,%s,%s,%s\n", part, gnum, snum.c_str(), gname.c_str(), gid_bare.c_str() );
        }
    }

    fprintf( fid, "\n" );
    fprintf( fid, "%lu\n", m_SingleTagMap.size() - 1 ); // Total number of tags ( the minus 1 is from the dummy tags )
    fprintf( fid, "\n" );

    fprintf( fid, "# tag#,part#,ssname1,ssname2,...,ssid1,ssid2,...\n" );

    for ( int i = 0 ; i < ( int )m_TagKeys.size() ; i++ )
    {
        int tag = GetTag( m_TagKeys[i] );
        int part = GetPart( m_TagKeys[i] );

        string comp_list = GetTagNames( m_TagKeys[i] );

        string ssnames, ssids;

        // Find position of first comma
        int cpos = comp_list.find( "," );
        if ( cpos != std::string::npos )
        {
            ssnames = comp_list.substr( cpos );
        }

        string id_list = GetTagIDs( m_TagKeys[i] );

        // Find position of first comma
        cpos = id_list.find( "," );
        if ( cpos != std::string::npos )
        {
            ssids = id_list.substr( cpos );
        }

        // Write tag number and surface list to file
        fprintf( fid, "%d,%d", tag, part );

        // Write subsurface information if there is any
        if( !ssnames.empty() )
        {
            // ssnames and ssids have leading commas
            fprintf( fid, "%s%s\n", ssnames.c_str(), ssids.c_str() );
        }
        else
        {
            fprintf( fid, "\n" );
        }
    }
    fclose( fid );
}

void PGMesh::WriteSTL( const string& fname )
{
    FILE* fp = fopen( fname.c_str(), "w" );

    if ( !fp )
    {
        return;
    }

    fprintf( fp, "solid\n" );

    list< PGFace* >::iterator f;
    for ( f = m_FaceList.begin() ; f != m_FaceList.end(); ++f )
    {
        vector < PGNode* > nodVec;
        ( *f )->GetNodesAsTris( nodVec );

        vec3d norm = (*f)->m_Nvec;

        int ntri = nodVec.size() / 3;
        int inod = 0;
        for ( int i = 0; i < ntri; i++ )
        {
            fprintf( fp, " facet normal  %2.10le %2.10le %2.10le\n",  norm.x(), norm.y(), norm.z() );
            fprintf( fp, "   outer loop\n" );
            for ( int j = 0; j < 3; j++ )
            {
                vec3d v = nodVec[ inod ]->m_Pt->m_Pnt;
                fprintf( fp, "     vertex %2.10le %2.10le %2.10le\n", v.x(), v.y(), v.z() );
                inod++;
            }
            fprintf( fp, "   endloop\n" );
            fprintf( fp, " endfacet\n" );
        }
    }
    fprintf( fp, "endsolid\n" );
    fclose( fp );
}

void PGMesh::WriteTRI( const string& fname )
{
    FILE* fp = fopen( fname.c_str(), "w" );

    if ( !fp )
    {
        return;
    }

    m_PGMulti->ResetPointNumbers();

    fprintf( fp, "%d\n", m_NodeList.size() );

    int ntri = 0;
    list< PGFace* >::iterator f;
    for ( f = m_FaceList.begin() ; f != m_FaceList.end(); ++f )
    {
        vector < PGNode * > nodVec;
        ( *f )->GetNodesAsTris( nodVec );
        ntri += nodVec.size() / 3;
    }

    fprintf( fp, "%d\n", ntri );

    list< PGNode* >::iterator n;
    for ( n = m_NodeList.begin() ; n != m_NodeList.end(); ++n )
    {
        vec3d v = ( *n )->m_Pt->m_Pnt;
        fprintf( fp, "%16.10g %16.10g %16.10g\n", v.x(), v.y(), v.z() );
    }

    for ( f = m_FaceList.begin() ; f != m_FaceList.end(); ++f )
    {
        vector < PGNode* > nodVec;
        ( *f )->GetNodesAsTris( nodVec );

        int nt = nodVec.size() / 3;

        int inod = 0;
        for ( int i = 0; i < nt; i++ )
        {
            fprintf( fp, "%d ", nodVec[ inod ]->m_Pt->m_ID + 1 );
            inod++;
            fprintf( fp, "%d ", nodVec[ inod ]->m_Pt->m_ID + 1 );
            inod++;
            fprintf( fp, "%d\n", nodVec[ inod ]->m_Pt->m_ID + 1 );
            inod++;
        }
    }
    fclose( fp );
}

string PGMulti::GetTagNames( const vector<int> & tags )
{
    string comp_list;
    map< int, string >::iterator si;

    int tag = GetTag( tags );

    if ( tag == -1 )
    {
        comp_list = "Error_Tag";
        return comp_list;
    }
    else
    {
        for ( int i = 0 ; i < ( int )tags.size() ; i++ )
        {
            si = m_TagNames.find( tags[i] );

            if ( si == m_TagNames.end() )
            {
                comp_list += ",Error_SubSurf";
            }
            else if ( si != m_TagNames.end() )
            {
                comp_list += "," + si->second ;
            }

            // Remove leading comma on first loop
            if ( i == 0 )
            { comp_list.erase( comp_list.begin(), comp_list.begin() + 1 ); }
        }
    }

    return comp_list;
}

string PGMulti::GetTagNames( const int indx )
{
    if ( indx < m_TagKeys.size() && indx >= 0 )
    {
        return GetTagNames( m_TagKeys[indx] );
    }
    return string( "Error_Tag" );
}

string PGMulti::GetTagIDs( const vector<int>& tags )
{
    string comp_list;
    map< int, string >::iterator si;

    int tag = GetTag( tags );

    if ( tag == -1 )
    {
        comp_list = "Error_Tag";
        return comp_list;
    }
    else
    {
        for ( int i = 0; i < (int)tags.size(); i++ )
        {
            si = m_TagIDs.find( tags[i] );

            if ( si == m_TagIDs.end() )
            {
                comp_list += ",Error_SubSurf";
            }
            else if ( si != m_TagIDs.end() )
            {
                comp_list += "," + si->second;
            }

            // Remove leading comma on first loop
            if ( i == 0 )
            {
                comp_list.erase( comp_list.begin(), comp_list.begin() + 1 );
            }
        }
    }

    return comp_list;
}

string PGMulti::GetTagIDs( const int indx )
{
    if ( indx < m_TagKeys.size() && indx >= 0 )
    {
        return GetTagIDs( m_TagKeys[indx] );
    }
    return string( "Error_Tag" );
}

string PGMulti::GetGID( const int& tag )
{
    string id_list = GetTagIDs( tag - 1 );

    // Find position of token _Surf
    int spos = id_list.find( "_Surf" );
    string gid = id_list.substr( 0, spos );
    string gid_bare = gid.substr( 0, 10 );

    return gid_bare;
}

bool PGMulti::MatchPartAndTag( const vector < int > & tags, const int part, const int tag )
{
    if ( !tags.empty() )
    {
        if ( tags[0] == part )
        {
            if ( vector_contains_val( tags, tag ) )
            {
                return true;
            }
        }
    }
    return false;
}

bool PGMulti::MatchPartAndTag( const int singletag, const int part, const int tag ) const
{
    if ( m_TagKeys.size() >= singletag )
    {
        return MatchPartAndTag( m_TagKeys[ singletag - 1 ], part, tag );
    }
    return false;
}

bool PGMulti::ExistPartAndTag( const int part, const int tag ) const
{
    for ( int i = 0 ; i < ( int )m_TagKeys.size() ; i++ )
    {
        if ( MatchPartAndTag( m_TagKeys[i], part, tag ) )
        {
            return true;
        }
    }
    return false;
}

void PGMulti::MakePartList( std::vector < int > & partvec ) const
{
    std::set< int > partset;
    for ( int i = 0 ; i < ( int )m_TagKeys.size() ; i++ )
    {
        if ( !m_TagKeys[i].empty() )
        {
            partset.insert( m_TagKeys[i][0] );
        }
    }

    partvec.clear();
    partvec.reserve( partset.size() );
    std::set< int >::iterator it;
    for ( it = partset.begin(); it != partset.end(); ++it )
    {
        partvec.push_back( *it );
    }
}

int PGMulti::GetTag( const vector<int> & tags )
{
    map< vector<int>, int >::iterator mi;
    mi = m_SingleTagMap.find( tags );

    if ( mi != m_SingleTagMap.end() )
    {
        return mi->second;
    }
    else
    {
        return -1;
    }
}

vector< int > PGMulti::GetTagVec( const int &tin )
{
    vector < int > ret;

    int t = tin;

    if ( t > 1000 )
    {
        t -= 1000;
    }

    t -= 1;

    if ( t >=0 && t < m_TagKeys.size() )
    {
        ret = m_TagKeys[ t ];
        if ( GetTag( ret ) == tin )
        {
            return ret;
        }
        printf( "Error in PGMesh::GetTagVec.  Unable to invert %d.\n", tin );
    }
    else
    {
        printf( "Error in PGMesh::GetTagVec.  Out of range %d.\n", tin );
    }

    return vector<int> { -1 };
}

int PGMulti::GetType( const int part ) const
{
    return m_TypeVec[ part - 1 ];
}

int PGMulti::GetThickThin( const int part ) const
{
    return m_ThickVec[ part - 1 ];
}

double PGMulti::GetWmin( const int part ) const
{
    return m_WminVec[ part - 1 ];
}

int PGMulti::GetPart( const vector<int> & tags )
{
    if ( !tags.empty() )
    {
        return tags[0];
    }
    return -1;
}

int PGMulti::GetPart( int tag ) const
{
    if ( m_TagKeys.size() >= tag )
    {
        vector < int > tags = m_TagKeys[ tag - 1 ];
        return GetPart( tags );
    }
    return -1;
}

void PGMulti::GetPartData( vector < string > &gidvec, vector < int > &partvec, vector < int > &surfvec )
{
    gidvec.clear();
    partvec.clear();
    surfvec.clear();

    for ( int i = 0 ; i < ( int )m_TagKeys.size() ; i++ )
    {
        if ( m_TagKeys[i].size() != 1 )
        {
            continue;
        }

        int part = GetPart( m_TagKeys[i] );

        string comp_list = GetTagNames( m_TagKeys[i] );

        // Find position of token _Surf
        int spos = comp_list.find( "_Surf" );

        string gname = comp_list.substr( 0, spos );

        string snum, ssnames, ssids;

        // Find position of first comma
        int cpos = comp_list.find( "," );
        if ( cpos != std::string::npos )
        {
            snum = comp_list.substr( spos + 5, cpos - ( spos + 5 ) );
            ssnames = comp_list.substr( cpos );
        }
        else
        {
            snum = comp_list.substr( spos + 5 );
        }

        string id_list = GetTagIDs( m_TagKeys[i] );

        // Find position of token _Surf
        spos = id_list.find( "_Surf" );
        string gid = id_list.substr( 0, spos );
        string gid_bare = gid.substr( 0, 10 );


        gidvec.push_back( gid_bare );
        partvec.push_back( part );
        surfvec.push_back( stoi( snum ) );
    }
}

std::map< std::vector<int>, int > PGMulti::GetSingleTagMap() const
{
    return m_SingleTagMap;
}

unsigned int PGMulti::GetNumTags() const
{
    return m_SingleTagMap.size();
}

void PGMesh::BuildFromTMesh( const TMesh* tmi )
{
    // Archive tag data at time of NGonMeshGeom creation.
    m_PGMulti->m_TagNames = SubSurfaceMgr.m_TagNames;
    m_PGMulti->m_TagIDs = SubSurfaceMgr.m_TagIDs;
    m_PGMulti->m_ThickVec = SubSurfaceMgr.m_CompThick;
    m_PGMulti->m_TypeVec = SubSurfaceMgr.m_CompTypes;
    m_PGMulti->m_WminVec = SubSurfaceMgr.m_CompWmin;
    m_PGMulti->m_TagKeys = SubSurfaceMgr.GetTagKeys();
    m_PGMulti->m_SingleTagMap = SubSurfaceMgr.GetSingleTagMap();

    vector < PGNode* > nod;

    for ( int j = 0; j < tmi->m_NVec.size(); j++ )
    {
        TNode *nj = tmi->m_NVec[j];
        PGPoint *pnt = m_PGMulti->AddPoint( nj->m_Pnt );
        nj->m_ID = nod.size();
        nod.push_back( AddNode( pnt ) );
    }

    for ( int j = 0; j < tmi->m_TVec.size(); j++ )
    {
        TTri *tj = tmi->m_TVec[j];

        if ( !tj->m_SplitVec.empty() )
        {
            for ( int s = 0; s < ( int ) tj->m_SplitVec.size(); s++ )
            {
                TTri *ts = tj->m_SplitVec[ s ];
                if ( !ts->m_IgnoreTriFlag )
                {
                    int tag = m_PGMulti->GetTag( ts->m_Tags );
                    AddFace( nod[ ts->m_N0->m_ID ],
                             nod[ ts->m_N1->m_ID ],
                             nod[ ts->m_N2->m_ID ],
                             ts->m_N0->m_UWPnt.as_vec2d_xy(),
                             ts->m_N1->m_UWPnt.as_vec2d_xy(),
                             ts->m_N2->m_UWPnt.as_vec2d_xy(),
                             ts->m_Norm,
                             ts->m_iQuad,
                             tag,
                             ts->m_jref,
                             ts->m_kref );
                }
            }
        }
        else
        {
            if ( !tj->m_IgnoreTriFlag )
            {
                int tag = m_PGMulti->GetTag( tj->m_Tags );
                AddFace( nod[ tj->m_N0->m_ID ],
                         nod[ tj->m_N1->m_ID ],
                         nod[ tj->m_N2->m_ID ],
                         tj->m_N0->m_UWPnt.as_vec2d_xy(),
                         tj->m_N1->m_UWPnt.as_vec2d_xy(),
                         tj->m_N2->m_UWPnt.as_vec2d_xy(),
                         tj->m_Norm,
                         tj->m_iQuad,
                         tag,
                         tj->m_jref,
                         tj->m_kref  );
            }
        }
    }

}

void PGMesh::BuildFromTMeshVec( const vector< TMesh* > &tmv )
{
    // Archive tag data at time of NGonMeshGeom creation.
    m_PGMulti->m_TagNames = SubSurfaceMgr.m_TagNames;
    m_PGMulti->m_TagIDs = SubSurfaceMgr.m_TagIDs;
    m_PGMulti->m_ThickVec = SubSurfaceMgr.m_CompThick;
    m_PGMulti->m_TypeVec = SubSurfaceMgr.m_CompTypes;
    m_PGMulti->m_WminVec = SubSurfaceMgr.m_CompWmin;
    m_PGMulti->m_TagKeys = SubSurfaceMgr.GetTagKeys();
    m_PGMulti->m_SingleTagMap = SubSurfaceMgr.GetSingleTagMap();

    vector < PGNode* > nod;
    for ( int i = 0; i < tmv.size(); i++ )
    {
        TMesh *tmi = tmv[ i ];

        for ( int j = 0; j < tmi->m_NVec.size(); j++ )
        {
            TNode *nj = tmi->m_NVec[j];
            PGPoint *pnt = m_PGMulti->AddPoint( nj->m_Pnt );
            nj->m_ID = nod.size();
            nod.push_back( AddNode( pnt ) );
        }
    }

    for ( int i = 0; i < tmv.size(); i++ )
    {
        TMesh *tmi = tmv[ i ];

        for ( int j = 0; j < tmi->m_TVec.size(); j++ )
        {
            TTri *tj = tmi->m_TVec[j];

            if ( !tj->m_SplitVec.empty() )
            {
                for ( int s = 0; s < ( int ) tj->m_SplitVec.size(); s++ )
                {
                    TTri *ts = tj->m_SplitVec[ s ];
                    if ( !ts->m_IgnoreTriFlag )
                    {
                        int tag = m_PGMulti->GetTag( ts->m_Tags );
                        AddFace( nod[ ts->m_N0->m_ID ],
                                 nod[ ts->m_N1->m_ID ],
                                 nod[ ts->m_N2->m_ID ],
                                 ts->m_N0->m_UWPnt.as_vec2d_xy(),
                                 ts->m_N1->m_UWPnt.as_vec2d_xy(),
                                 ts->m_N2->m_UWPnt.as_vec2d_xy(),
                                 ts->m_Norm,
                                 ts->m_iQuad,
                                 tag,
                                 ts->m_jref,
                                 ts->m_kref );
                    }
                }
            }
            else
            {
                if ( !tj->m_IgnoreTriFlag )
                {
                    int tag = m_PGMulti->GetTag( tj->m_Tags );
                    AddFace( nod[ tj->m_N0->m_ID ],
                             nod[ tj->m_N1->m_ID ],
                             nod[ tj->m_N2->m_ID ],
                             tj->m_N0->m_UWPnt.as_vec2d_xy(),
                             tj->m_N1->m_UWPnt.as_vec2d_xy(),
                             tj->m_N2->m_UWPnt.as_vec2d_xy(),
                             tj->m_Norm,
                             tj->m_iQuad,
                             tag,
                             tj->m_jref,
                             tj->m_kref );
                }
            }
        }
    }
}

// Copies pgm into this, but re-using all of the basic points at the bottom level.
void PGMesh::BuildFromPGMesh( const PGMesh* pgm )
{
    // Archive tag data at time of NGonMeshGeom creation.
    m_PGMulti->m_TagNames = pgm->m_PGMulti->m_TagNames;
    m_PGMulti->m_TagIDs = pgm->m_PGMulti->m_TagIDs;
    m_PGMulti->m_ThickVec = pgm->m_PGMulti->m_ThickVec;
    m_PGMulti->m_TypeVec = pgm->m_PGMulti->m_TypeVec;
    m_PGMulti->m_WminVec = pgm->m_PGMulti->m_WminVec;
    m_PGMulti->m_TagKeys = pgm->m_PGMulti->m_TagKeys;
    m_PGMulti->m_SingleTagMap = pgm->m_PGMulti->m_SingleTagMap;

    vector < PGNode * > nods;
    nods.reserve( pgm->m_PGMulti->m_PointList.size() );
    int ipt = 0; // Number all points starting at zero.  Copy to corresponding vector.
    list< PGNode* >::const_iterator n;
    for ( n = pgm->m_NodeList.begin() ; n != pgm->m_NodeList.end(); ++n )
    {
        PGPoint *p = (*n)->m_Pt;
        PGNode *nnew = AddNode( p );
        nnew->m_TagUWMap = (*n)->m_TagUWMap;
        nods.push_back( nnew );
        p->m_ID = ipt;
        ipt++;
    }

    vector < PGEdge * > edgs;
    edgs.reserve( pgm->m_EdgeList.size() );
    int ie = 0;
    list< PGEdge* >::const_iterator e;
    for ( e = pgm->m_EdgeList.begin() ; e != pgm->m_EdgeList.end(); ++e )
    {
        edgs.push_back( AddEdge( nods[ (*e)->m_N0->m_Pt->m_ID ], nods[ (*e)->m_N1->m_Pt->m_ID ] ) );
        (*e)->m_ID = ie;
        ie++;
    }

    list< PGFace* >::const_iterator f;
    for ( f = pgm->m_FaceList.begin(); f != pgm->m_FaceList.end(); ++f )
    {
        PGFace *fnew = AddFace();

        fnew->m_Nvec = (*f)->m_Nvec;
        fnew->m_iQuad = (*f)->m_iQuad;
        fnew->m_Tag = (*f)->m_Tag;
        fnew->m_jref = (*f)->m_jref;
        fnew->m_kref = (*f)->m_kref;
        fnew->m_Region = (*f)->m_Region;

        vector < PGEdge * >::iterator ei;
        for( ei = (*f)->m_EdgeVec.begin(); ei != (*f)->m_EdgeVec.end(); ++ei )
        {
            PGEdge *eadd = edgs[ (*ei)->m_ID ];
            fnew->AddEdge( eadd );
            eadd->AddConnectFace( fnew );
        }
    }

    CleanUnused();
}

PGNode* FindEndNode( const vector < PGEdge* > & eVec )
{
    if ( eVec.empty() )
    {
        return nullptr;
    }

    if ( eVec.size() == 1 )
    {
        if ( eVec[0] )
        {
            return eVec[0]->m_N0;
        }
        return nullptr;
    }

    PGEdge* e0 = eVec[0];
    PGEdge* e1 = eVec[1];

    if ( !e0 || !e1 )
    {
        return nullptr;
    }

    if ( e1->ContainsNode( e0->m_N0 ) )
    {
        return e0->m_N1;
    }

    return e0->m_N0;
}

void GetNodes( const vector < PGEdge* > & eVec, vector< PGNode* > & nodVec )
{
    nodVec.clear();
    PGNode *nprev = FindEndNode( eVec );

    if ( !nprev )
    {
        printf( "No prev node.\n" );
        return;
    }

    nodVec.push_back( nprev );

    for ( int i = 0 ; i < ( int )eVec.size() ; i++ )
    {
        PGEdge *e = eVec[i];

        if ( e )
        {
            PGNode *nnext = e->OtherNode( nprev );

            if ( nnext )
            {
                nodVec.push_back( nnext );
            }
            else
            {
                printf( "No next node.\n" );
            }

            nprev = nnext;
        }
        else
        {
            printf( "Invalid node.\n" );
        }
    }
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

PGMulti::PGMulti()
{
    AddMesh();
    m_ActiveMesh = 0;
}

PGMulti::~PGMulti()
{
    DumpGarbage();
    Clear();
}

void PGMulti::Clear()
{
    vector< PGMesh* >::iterator m;
    for ( m = m_MeshVec.begin() ; m != m_MeshVec.end(); ++m )
    {
        delete ( *m );
    }

    m_MeshVec.clear();

    list< PGPoint* >::iterator p;
    for ( p = m_PointList.begin() ; p != m_PointList.end(); ++p )
    {
        delete ( *p );
    }

    m_PointList.clear();
}

void PGMulti::CleanUnused()
{
    // Copy vector to vector because removal from vector will corrupt vector in-use.
    vector< PGMesh* > mVec( m_MeshVec.begin(), m_MeshVec.end() );
    for ( int i = 0; i < mVec.size(); i++ )
    {
        PGMesh *m = mVec[ i ];
        if ( m->m_FaceList.empty() )
        {
            RemoveMesh( m );
        }
    }

    // Copy list to vector because removal from list will corrupt list in-use.
    vector< PGPoint* > pVec( m_PointList.begin(), m_PointList.end() );
    for ( int i = 0; i < pVec.size(); i++ )
    {
        PGPoint *p = pVec[ i ];
        if ( p->m_NodeVec.empty() )
        {
            RemovePoint( p );
        }
    }
}

PGMesh* PGMulti::AddMesh()
{
    PGMesh *m = new PGMesh( this );
    m_MeshVec.push_back( m );
    return m;
}

void PGMulti::RemoveMesh( PGMesh* m )
{
    m_GarbageMeshVec.push_back( m );
    vector_remove_val( m_MeshVec, m );

    m->m_DeleteMeFlag = true;
}


PGPoint* PGMulti::AddPoint( const vec3d& p )
{
    PGPoint *pptr = new PGPoint( p );
    m_PointList.push_back( pptr );
    pptr->m_List_it = --m_PointList.end();
    return pptr;
}

void PGMulti::RemovePoint( PGPoint* pptr )
{
    for ( int i = 0; i < pptr->m_NodeVec.size(); i++ )
    {
        pptr->NodeForgetPoint( pptr->m_NodeVec[i] );
    }
    pptr->m_NodeVec.clear();

    m_GarbagePointVec.push_back( pptr );
    m_PointList.erase( pptr->m_List_it );

    pptr->m_DeleteMeFlag = true;
}

void PGMulti::DumpGarbage()
{
    //==== Delete Flagged PGMesh =====//
    for ( int i = 0 ; i < ( int )m_GarbageMeshVec.size() ; i++ )
    {
        delete m_GarbageMeshVec[i];
    }
    m_GarbageMeshVec.clear();

    //==== Delete Flagged PGPoint =====//
    for ( int i = 0 ; i < ( int )m_GarbagePointVec.size() ; i++ )
    {
        delete m_GarbagePointVec[i];
    }
    m_GarbagePointVec.clear();
}

PGMesh* PGMulti::GetActiveMesh()
{
    return m_MeshVec[ m_ActiveMesh ];
}

bool PGMulti::Check()
{
    list< PGPoint* >::iterator p;
    for ( p = m_PointList.begin() ; p != m_PointList.end(); ++p )
    {
        if ( !( *p )->Check() )
        {
            return false;
        }
    }

    return true;
}

void PGMulti::ResetPointNumbers()
{
    int inode = 0; // Start numbering at 0
    list< PGPoint* >::iterator p;
    for ( p = m_PointList.begin() ; p != m_PointList.end(); ++p )
    {
        // Assign ID number.
        ( *p )->m_ID = inode;
        inode++;
    }
}
