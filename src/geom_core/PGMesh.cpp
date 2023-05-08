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

#include "triangle.h"
#include "triangle_api.h"

#include <algorithm>
#include <cfloat>

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

PGNode::PGNode()
{
    m_DeleteMeFlag = false;
    m_ID = -1;
}

PGNode::PGNode( const vec3d& p )
{
    m_Pnt = p;
    m_ID = -1;
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

void PGNode::AddConnectEdge( PGEdge* e )
{
    if ( vector_contains_val( m_EdgeVec, e ) )
    {
        return;
    }
    m_EdgeVec.push_back( e );
}

void PGNode::RemoveConnectEdge( PGEdge* e )
{
    vector_remove_val( m_EdgeVec, e );
}

void PGNode::EdgeForgetNode( PGEdge* e )
{
    e->ReplaceNode( this, nullptr );
}

bool PGNode::ColinearNode( double tol )
{
    if ( m_EdgeVec.size() == 2 )
    {
        PGEdge *e0 = m_EdgeVec[0];
        PGEdge *e1 = m_EdgeVec[1];

        if ( e0->SameFaces( e1 ) )
        {
            PGNode *n0 = e0->OtherNode( this );
            PGNode *n1 = e1->OtherNode( this );

            double t;
            double d = sqrt( pointSegDistSquared( m_Pnt, n0->m_Pnt, n1->m_Pnt, &t ) );
            double l = dist( n0->m_Pnt, n1->m_Pnt );

            if ( d / l < tol )
            {
                return true;
            }
        }
    }
    return false;
}

bool PGNode::Check()
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

void PGNode::DumpMatlab()
{
    printf( "\nx = %.*e;\n", DBL_DIG + 3, m_Pnt.x() );
    printf( "\ny = %.*e;\n", DBL_DIG + 3, m_Pnt.y() );
    printf( "\nz = %.*e;\n", DBL_DIG + 3, m_Pnt.z() );

    printf( "plot3( x, y, z, 'bo' );\n" );
    printf( "hold on\n" );
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

PGEdge::PGEdge()
{
    m_N0 = m_N1 = nullptr;
    m_DeleteMeFlag = false;
}

PGEdge::PGEdge( PGNode* PGNode0, PGNode* PGNode1 )
{
    m_N0 = PGNode0;
    m_N1 = PGNode1;
    m_DeleteMeFlag = false;
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

bool PGEdge::UsedBy( PGFace* f ) const
{
    return vector_contains_val( m_FaceVec, f );
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

void PGEdge::RemoveFace( PGFace* f )
{
    vector_remove_val( m_FaceVec, f );
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
    return NULL;
}

void PGEdge::ReplaceNode( PGNode* curr_PGNode, PGNode* replace_PGNode )
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

void PGEdge::NodesForgetEdge()
{
    m_N0->RemoveConnectEdge( this );
    m_N1->RemoveConnectEdge( this );
}

void PGEdge::SortFaces()
{
    sort( m_FaceVec.begin(), m_FaceVec.end() );
}

bool PGEdge::SameFaces( PGEdge *e2 )
{
    // Comparison of std::vector checks size and then contents.  Since these are pointers that were previously sorted,
    // this should work.
    return ( m_FaceVec == e2->m_FaceVec );
}

bool PGEdge::Check()
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

void PGEdge::DumpMatlab()
{
    printf( "x = [%.*e %.*e];\n", DBL_DIG + 3, m_N0->m_Pnt.x(), DBL_DIG + 3, m_N1->m_Pnt.x() );
    printf( "y = [%.*e %.*e];\n", DBL_DIG + 3, m_N0->m_Pnt.y(), DBL_DIG + 3, m_N1->m_Pnt.y() );
    printf( "z = [%.*e %.*e];\n", DBL_DIG + 3, m_N0->m_Pnt.z(), DBL_DIG + 3, m_N1->m_Pnt.z() );

    printf( "plot3( x, y, z, 'b+:' );\n" );
    printf( "hold on\n" );
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

PGFace::PGFace()
{
    deleteFlag = false;
}

PGFace::~PGFace()
{
}

PGEdge* PGFace::FindEdge( PGNode* nn0, PGNode* nn1 ) const
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
        return NULL;
    }

    PGNode *nprev = NULL;
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

void PGFace::GetNodesAsTris( vector < PGNode* > & trinodVec )
{
    if ( m_TriNodeVec.size() == 0 )
    {
        Triangulate();
    }

    trinodVec = m_TriNodeVec;
}

void PGFace::Triangulate()
{
    Triangulate_triangle();
}

void PGFace::Triangulate_triangle()
{
    ClearTris();

    vector < PGNode* > nodVec;
    GetNodes( nodVec );

    // index to size-1 because first/last point is repeated.
    int npt = nodVec.size() - 1;

    if ( npt < 3 )
    {
        return;
    }

    // Get node data into simple point vector.
    vector < vec3d > ptVec( npt );
    for ( int i = 0; i < npt; i++ )
    {
        ptVec[i] = nodVec[i]->m_Pnt;
    }

    // Rotate along normal.
    Matrix4d mat;
    mat.rotatealongX( m_Nvec );
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
    out.pointlist   = NULL;

    in.segmentlist  = ( int * ) malloc( npt * 2 * sizeof( int ) );
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
        m_TriNodeVec.resize( out.numberoftriangles * 3 );
        for ( int i = 0; i < out.numberoftriangles * 3; i++ )
        {
            m_TriNodeVec[i] = nodVec[ out.trianglelist[i] ];
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

void PGFace::ClearTris()
{
    m_TriNodeVec.clear();
}

void PGFace::AddEdge( PGEdge* e )
{
    if ( vector_contains_val( m_EdgeVec, e ) )
    {
        return;
    }
    m_EdgeVec.push_back( e );
}

void PGFace::RemoveEdge( PGEdge* e )
{
    vector_remove_val( m_EdgeVec, e );
}

bool PGFace::Contains( PGEdge* e ) const
{
    return vector_contains_val( m_EdgeVec, e );
}

bool PGFace::Contains( PGNode* n ) const
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

void PGFace::EdgeForgetFace()
{
    for ( int i = 0; i < m_EdgeVec.size(); i++ )
    {
        m_EdgeVec[i]->RemoveFace( this );
    }
}

bool PGFace::Check()
{
    for ( int i = 0; i < m_EdgeVec.size(); i++ )
    {
        if ( !m_EdgeVec[i] )
        {
            return false;
        }
    }
    return true;
}

void PGFace::DumpMatlab()
{
    vector < PGNode* > nodVec;
    GetNodes( nodVec );

    int num = nodVec.size();

    int i;
    printf( "\nx = [" );
    for ( i = 0; i < num - 1; i++ )
    {
        printf( "%.*e;\n", DBL_DIG + 3, nodVec[i]->m_Pnt.x() );
    }
    printf( "%.*e];\n", DBL_DIG + 3, nodVec[i]->m_Pnt.x() );

    printf( "\ny = [" );
    for ( i = 0; i < num - 1; i++ )
    {
        printf( "%.*e;\n", DBL_DIG + 3, nodVec[i]->m_Pnt.y() );
    }
    printf( "%.*e];\n", DBL_DIG + 3, nodVec[i]->m_Pnt.y() );

    printf( "\nz = [" );
    for ( i = 0; i < num - 1; i++ )
    {
        printf( "%.*e;\n", DBL_DIG + 3, nodVec[i]->m_Pnt.z() );
    }
    printf( "%.*e];\n", DBL_DIG + 3, nodVec[i]->m_Pnt.z() );

    printf( "plot3( x, y, z, 'kx-' );\n" );
    printf( "hold on\n" );

}

PGNode * PGFace::FindDoubleBackNode( PGEdge* & edouble )
{
    edouble = NULL;
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

    return NULL;
}

// Split edge e0 into e0,e1.  Direction indeterminte (e1 can go before or after e0).
// e0 can also appear more than once in this face.
void PGFace::SplitEdge( PGEdge *e0, PGEdge *e1 )
{

    PGNode* ns = e0->SharedNode( e1 );
    PGNode* n0 = e0->OtherNode( ns );
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

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

PGMesh::PGMesh()
{
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

PGNode* PGMesh::AddNode( vec3d p )
{
    PGNode* nptr = new PGNode( p );
    m_NodeList.push_back( nptr );
    nptr->m_List_it = --m_NodeList.end();
    return nptr;
}

void PGMesh::RemoveNode( PGNode* nptr )
{
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
    e->m_N1->RemoveConnectEdge( e );

    m_GarbageEdgeVec.push_back( e );
    m_EdgeList.erase( e->m_List_it );

    e->m_DeleteMeFlag = true;
}

void PGMesh::RemoveEdgeMergeFaces( PGEdge* e )
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

            for ( int i = 0; i < ev1.size(); i++ )
            {
                ev1[ i ]->RemoveFace( f1 );
                ev1[ i ]->AddConnectFace( f0 );
            }

            RemoveFace( f1 );
        }
    }
}

PGEdge* PGMesh::FindEdge( const PGNode* n0, const PGNode* n1 ) const
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

void PGMesh::RemoveFace( PGFace* fptr ) // Seems insufficient.
{
    m_GarbageFaceVec.push_back( fptr );
    m_FaceList.erase( fptr->m_List_it );
}

void PGMesh::RemoveNodeMergeEdges( PGNode* n )
{
    if ( n->m_EdgeVec.size() == 2 )
    {
        PGEdge *e0 = n->m_EdgeVec[0];
        PGEdge *e1 = n->m_EdgeVec[1];

        PGNode *n0 = e0->OtherNode( n );
        PGNode *n1 = e1->OtherNode( n );

        vector< PGFace* > fv = e1->m_FaceVec;
        for ( int i = 0; i < fv.size(); i++ )
        {
            PGFace *f = fv[i];

            e1->RemoveFace( f );  // Remove face from edge's facevec
            f->RemoveEdge( e1 );  // Remove edge from face's edgevec
        }

        e0->ReplaceNode( n, n1 ); // Only changes node pointer, does not recurse.
        n1->AddConnectEdge( e0 ); // Adds edge to node's edge vec.

        RemoveEdge( e1 ); // Calls both nodes to RemoveConnectEdge - removes edge from node edgevec
                          // Adds to garbage vec
                          // Removes from mesh edge list.
                          // Sets deleteme flag

        RemoveNode( n );  // Adds to garbage vec
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

void PGMesh::StartMatlab()
{
    printf( "clear all;\n" );
    printf( "close all;\n" );
    printf( "format compact;\n" );
}

void PGMesh::SplitEdge( PGEdge *e0, PGNode *n )
{
    PGNode *n0 = e0->m_N0;
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
}

void PGMesh::SplitFaceFromDoubleBackNode( PGFace *f, PGEdge *e, PGNode *n )
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

    vec3d dir = n->m_Pnt - nother->m_Pnt;
    dir.normalize();

    int imindist = -1;
    double mindist = DBL_MAX;
    double tmindist = 0;

    for ( int i = 0; i < ehull.size(); i++ )
    {
        PGEdge* eh = ehull[i];

        double s, t;
        line_line_intersect( nother->m_Pnt, n->m_Pnt, eh->m_N0->m_Pnt, eh->m_N1->m_Pnt, &s, &t );

        vec3d p = ( 1.0 - t ) * eh->m_N0->m_Pnt + t * eh->m_N1->m_Pnt;

        double d = dist( n->m_Pnt, p );

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

        PGNode *n0 = e2split->m_N0;
        PGNode *n1 = e2split->m_N1;

        vec3d p = ( 1.0 - tmindist ) * n0->m_Pnt + tmindist * n1->m_Pnt;

        PGNode * newnode = AddNode( p );

        // Loop over all uw's in n0's map.
        for ( map < int, vec2d >::iterator it0 = n0->m_TagUWMap.begin(); it0 != n0->m_TagUWMap.end(); ++it0 )
        {
            int tag = it0->first;
            vec2d uw0 = it0->second;

            // Try to find uw with matching tag in n1's map.
            map < int, vec2d >::iterator it1 = n1->m_TagUWMap.find( tag );

            // If there is a match
            if ( it1 != n1->m_TagUWMap.end() )
            {
                // Interpolate uw and add to newnode's map.
                vec2d uw1 = it1->second;
                vec2d uw = ( 1.0 - tmindist ) * uw0 + tmindist * uw1;

                newnode->m_TagUWMap[ tag ] = uw;
            }
        }

        SplitEdge( e2split, newnode );

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
        ev[i]->RemoveFace( f0 );
        ev[i]->AddConnectFace( f1 );
    }
    ev1.push_back( e );
    f1->m_EdgeVec = ev1;

    e->AddConnectFace( f0 );
    e->AddConnectFace( f1 );
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

void PGMesh::WriteVSPGeom( FILE* file_id, const Matrix4d & XFormMat  )
{
    WriteVSPGeomPnts( file_id, XFormMat );
    WriteVSPGeomFaces( file_id );
    WriteVSPGeomParts( file_id );
    WriteVSPGeomWakes( file_id );
    WriteVSPGeomAlternateTris( file_id );
}

void PGMesh::WriteVSPGeomPnts( FILE* file_id, const Matrix4d & XFormMat )
{
    fprintf( file_id, "%d\n", m_NodeList.size() );

    //==== Write Out Nodes ====//
    vec3d v;

    int inode = 1; // Start numbering at 1
    list< PGNode* >::iterator n;
    for ( n = m_NodeList.begin() ; n != m_NodeList.end(); ++n )
    {
        // Assign ID number.
        ( *n )->m_ID = inode;
        inode++;

        // Apply Transformations
        v = XFormMat.xform( ( *n )->m_Pnt );
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
            fprintf( file_id, " %d", nodVec[i]->m_ID );
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
        int part = SubSurfaceMgr.GetPart( tag );
        fprintf( file_id, "%d %d", part, tag );

        for ( int i = 0; i < npt; i++ )
        {
            // Try to find uw with matching tag.
            map < int, vec2d >::iterator it1 = nodVec[i]->m_TagUWMap.find( tag );

            vec2d uw;
            // If there is a match
            if ( it1 != nodVec[i]->m_TagUWMap.end() )
            {
                uw = it1->second;
            }

            fprintf( file_id, " %16.10g %16.10g", uw.x(), uw.y() );
        }
        fprintf( file_id, "\n" );
    }
}

void PGMesh::WriteVSPGeomWakes( FILE* file_id )
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
            fprintf( file_id, "%d", nodVec[i]->m_ID );

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

        fprintf( file_id, "%d %d", iface, npt );
        for ( int i = 0; i < npt; i++ )
        {
            fprintf( file_id, " %d", nodVec[i]->m_ID );
        }
        fprintf( file_id, "\n" );

        iface++;
    }
}

void PGMesh::WriteTagFiles( string file_name )
{
    int ntagfile = 0;

    std::vector < int > partvec;
    SubSurfaceMgr.MakePartList( partvec );
    vector < SubSurface* > ssurfs = SubSurfaceMgr.GetSubSurfs();

    for ( int ipart = 0; ipart < partvec.size(); ipart++ )
    {
        int part = partvec[ ipart ];

        for ( int iss = 0; iss < ssurfs.size(); iss++ )
        {
            SubSurface *ssurf = ssurfs[iss];
            int tag = ssurf->m_Tag;

            if ( SubSurfaceMgr.ExistPartAndTag( part, tag ) )
            {
                ntagfile++;
            }
        }
    }

    if ( ntagfile > 0 )
    {
        string base_name = file_name;
        std::string::size_type loc = base_name.find_last_of( "." );
        if ( loc != base_name.npos )
        {
            base_name = base_name.substr( 0, loc );
        }
        string taglist_name = base_name + ".taglist";

        FILE* taglist_fid = fopen( taglist_name.c_str(), "w" );
        if ( taglist_fid )
        {

            fprintf( taglist_fid, "%d\n", ntagfile );

            for ( int ipart = 0; ipart < partvec.size(); ipart++ )
            {
                int part = partvec[ ipart ];

                for ( int iss = 0; iss < ssurfs.size(); iss++ )
                {
                    SubSurface *ssurf = ssurfs[iss];
                    int tag = ssurf->m_Tag;

                    if ( SubSurfaceMgr.ExistPartAndTag( part, tag ) )
                    {
                        vector < int > parttag;
                        parttag.push_back( part );
                        parttag.push_back( tag );

                        string ptagname = SubSurfaceMgr.GetTagNames( parttag );

                        string tagfile_name = base_name + ptagname + ".tag";

                        fprintf( taglist_fid, "%s\n", tagfile_name.c_str() );

                        FILE* fid = fopen( tagfile_name.c_str(), "w" );
                        if ( fid )
                        {
                            WriteTagFile( fid, part, tag );

                            fclose( fid );
                        }
                    }
                }
            }

            fclose( taglist_fid );
        }
    }
}

void PGMesh::WriteTagFile( FILE* file_id, int part, int tag )
{
    //==== Write Tri IDs for each tag =====//

    int iface = 1;
    list< PGFace* >::iterator f;
    for ( f = m_FaceList.begin() ; f != m_FaceList.end(); ++f )
    {
        int singletag = ( *f )->m_Tag;

        if ( SubSurfaceMgr.MatchPartAndTag( singletag, part, tag ) )
        {
            fprintf( file_id, "%d\n", iface );
        }
        iface++;
    }
}

PGNode* FindEndNode( const vector < PGEdge* > & eVec )
{
    if ( eVec.size() == 0 )
    {
        return NULL;
    }

    if ( eVec.size() == 1 )
    {
        if ( eVec[0] )
        {
            return eVec[0]->m_N0;
        }
        return NULL;
    }

    PGEdge* e0 = eVec[0];
    PGEdge* e1 = eVec[1];

    if ( !e0 || !e1 )
    {
        return NULL;
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

            nprev = nnext;
        }
    }
}
