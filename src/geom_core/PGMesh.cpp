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

void PGNode::GetTags( vector < int > & tags ) const
{
    for( std::map< int, vec2d >::const_iterator it = m_TagUWMap.begin(); it != m_TagUWMap.end(); ++it )
    {
        tags.push_back( it->first );
    }
}

bool PGNode::GetUW( int tag, vec2d &uw ) const
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

bool PGNode::UsedBy( PGEdge* e ) const
{
    return vector_contains_val( m_EdgeVec, e );
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


void PGNode::Diagnostics()
{
    printf( "Node %d\n", m_ID );
    printf( "Edges: " );

    for ( int i = 0; i < m_EdgeVec.size(); i++ )
    {
        printf( "%3d ", m_EdgeVec[i]->m_ID );
    }
    printf( "\n" );
}

bool PGNode::Validate()
{
    bool valid = true;

    for ( int i = 0; i < m_EdgeVec.size(); i++ )
    {
        if ( !m_EdgeVec[i] )
        {
            printf( "Node %d has invalid edge pointer %d\n", m_ID, i );
            valid = false;
            continue;
        }

        if ( !m_EdgeVec[i]->ContainsNode( this ) )
        {
            printf( "Edge %d does not contain node %d\n", m_EdgeVec[i]->m_ID, m_ID );
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

PGFace* PGEdge::OtherManifoldFace( PGFace* f )
{
    if ( m_FaceVec.size() != 2 )
    {
        return NULL;
    }

    if ( m_FaceVec[0] == f )
    {
        return m_FaceVec[1];
    }

    if ( m_FaceVec[1] == f )
    {
        return m_FaceVec[0];
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

void PGEdge::Diagnostics()
{
    printf( "Edge %d\n", m_ID );
    printf( "Nodes: %3d %3d\n", m_N0->m_ID, m_N1->m_ID );
    printf( "Faces: " );

    for ( int i = 0; i < m_FaceVec.size(); i++ )
    {
        printf( "%3d ", m_FaceVec[i]->m_ID );
    }
    printf( "\n" );

    m_N0->Diagnostics();
    m_N1->Diagnostics();
}

bool PGEdge::Validate()
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
        printf( "Node %d is unaware it is used by edge %d\n", m_N0->m_ID, m_ID );
        valid = false;
    }

    if ( !m_N1 )
    {
        printf( "Node 1 is invalid in edge %d\n", m_ID );
        valid = false;
    }
    else if ( !m_N1->UsedBy( this ) )
    {
        printf( "Node %d is unaware it is used by edge %d\n", m_N1->m_ID, m_ID );
        valid = false;
    }

    return valid;
}

bool PGEdge::WakeEdge( PGMesh *m )
{
    const double tol = 1e-12;

    int nface = m_FaceVec.size();

    for ( int i = 0; i < nface; i++ )
    {
        PGFace *f = m_FaceVec[i];

        int tag = f->m_Tag;
        int part = m->GetPart( tag );
        int type = m->GetType( part );
        double wmin = m->GetWmin( part );

        int thick = m->GetThickThin( part );

        if ( type == vsp::WING_SURF )
        {
            if ( !thick && nface != 1 )
            {
                return false;
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

    if ( nedge == 0 )
    {
        return NULL;
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

void PGFace::GetOtherNodes( vector< PGNode* > & nodVec, const vector< PGNode* > & skipNodVec ) const
{
    GetNodes( nodVec );
    vector_remove_vector( nodVec, skipNodVec );
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
    Triangulate_DBA();
}

void PGFace::Triangulate_triangle()
{
    ClearTris();

    vector < PGNode* > nodVec;
    GetNodes( nodVec );

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
            m_TriNodeVec.push_back( nodVec[ 0 ] );
            m_TriNodeVec.push_back( nodVec[ 1 ] );
            m_TriNodeVec.push_back( nodVec[ 2 ] );
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

    for ( int i = 0; i < m_TriNodeVec.size(); i++ )
    {
        if ( !m_TriNodeVec[i] )
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

void PGFace::Triangulate_DBA()
{
    ClearTris();

    vector < PGNode* > nodVec;
    GetNodes( nodVec );

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
            m_TriNodeVec.push_back( nodVec[ 0 ] );
            m_TriNodeVec.push_back( nodVec[ 1 ] );
            m_TriNodeVec.push_back( nodVec[ 2 ] );
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
            ptVec[i] = nodVec[i]->m_Pnt;
        }
    }

    // Rotate along normal.
    Matrix4d mat;
    mat.rotatealongX( m_Nvec );
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
        FILE *fpdump = NULL;

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

        int tris = idb->FloodFill( false, 0, 1 );

        const IDelaBella2<double>::Simplex* dela = idb->GetFirstDelaunaySimplex();

        m_TriNodeVec.reserve( tris * 3 );
        for ( int i = 0; i < tris; i++ )
        {
            // Note winding order!
            m_TriNodeVec.push_back( nodVec[ dela->v[ 0 ]->i ] );
            m_TriNodeVec.push_back( nodVec[ dela->v[ 2 ]->i ] );
            m_TriNodeVec.push_back( nodVec[ dela->v[ 1 ]->i ] );

            dela = dela->next;
        }
    }
    else
    {
        printf( "DLB Error! %d\n", verts );
    }

    for ( int i = 0; i < m_TriNodeVec.size(); i++ )
    {
        if ( !m_TriNodeVec[i] )
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

void PGFace::ReplaceEdge( PGEdge *eold, PGEdge *enew )
{
    for ( int i = 0 ; i < ( int )m_EdgeVec.size() ; i++ )
    {
        if ( eold == m_EdgeVec[i] )
        {
            m_EdgeVec[i] = enew;
        }
    }
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

void PGFace::Diagnostics()
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

bool PGFace::Validate()
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

    int ntri = nodVec.size() / 3;

    double a = 0;
    for ( int i = 0; i < ntri; i++ )
    {
        int inod = 3 * i;
        vec3d v0 = nodVec[ inod ]->m_Pnt;
        vec3d v1 = nodVec[ inod + 1 ]->m_Pnt;
        vec3d v2 = nodVec[ inod + 2 ]->m_Pnt;

        a += area( v0, v1, v2 );
    }
    return a;
}

void PGFace::WalkRegion()
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

double PGFace::ComputeTriQual()
{
    vector< PGNode* > nodVec;
    GetNodes( nodVec );

    return PGFace::ComputeTriQual( nodVec[0], nodVec[1], nodVec[2] );
}

double PGFace::ComputeTriQual( PGNode* n0, PGNode* n1, PGNode* n2 )
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

void PGFace::ComputeCosAngles( PGNode* n0, PGNode* n1, PGNode* n2, double* ang0, double* ang1, double* ang2 )
{
    double dsqr01 = dist_squared( n0->m_Pnt, n1->m_Pnt );
    double dsqr12 = dist_squared( n1->m_Pnt, n2->m_Pnt );
    double dsqr20 = dist_squared( n2->m_Pnt, n0->m_Pnt );

    double d01 = sqrt( dsqr01 );
    double d12 = sqrt( dsqr12 );
    double d20 = sqrt( dsqr20 );

    *ang0 = ( -dsqr12 + dsqr01 + dsqr20 ) / ( 2.0 * d01 * d20 );

    *ang1 = ( -dsqr20 + dsqr01 + dsqr12 ) / ( 2.0 * d01 * d12 );

    *ang2 = ( -dsqr01 + dsqr12 + dsqr20 ) / ( 2.0 * d12 * d20 );
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

PGNode* PGMesh::AddNode( vec3d p )
{
    PGNode* nptr = new PGNode( p );
    m_NodeList.push_back( nptr );
    nptr->m_List_it = --m_NodeList.end();
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
    e->m_N0 = NULL;
    e->m_N1->RemoveConnectEdge( e );
    e->m_N1 = NULL;

    for ( int i = 0; i < e->m_FaceVec.size(); i++ )
    {
        e->m_FaceVec[i]->RemoveEdge( e );
    }
    e->m_FaceVec.clear();

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

PGFace* PGMesh::AddFace( PGNode* n0, PGNode* n1, PGNode* n2,
                         const vec2d &uw0, const vec2d &uw1, const vec2d &uw2,
                         const vec3d & norm, int iQuad, int tag )
{
    PGFace *f = AddFace( );
    f->m_Nvec = norm;
    f->m_iQuad = iQuad;
    f->m_Tag = tag;

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
    printf( "Removed %d\n", iremove );

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
}

void PGMesh::MergeCoincidentNodes()
{
    vector< vec3d > allPntVec( m_NodeList.size() );

    // Make vector copy of list so nodes can be removed from list without invalidating active list iterator.
    vector< PGNode* > nVec( m_NodeList.begin(), m_NodeList.end() );

    for ( int i = 0; i < nVec.size(); i++ )
    {
        PGNode *n = nVec[ i ];
        allPntVec[ i ] = n->m_Pnt;
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

    printf( "%d coincident nodes merged\n", nmerge );


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

    ResetNodeNumbers();

    ClearTris();

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

void PGMesh::MergeDuplicateEdges()
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

    printf( "%d duplicate edges merged.\n", nmerged );

    CleanUnused();

    DumpGarbage();

    ResetNodeNumbers();

    ClearTris();
}

void PGMesh::RemoveDegenEdges()
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

    printf( "%d degen edges removed\n", ndegen );

    CleanUnused();

    DumpGarbage();

    ResetNodeNumbers();

    ClearTris();
}

void PGMesh::RemoveDegenFaces()
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

    printf( "%d degen faces removed\n", ndegen );

    CleanUnused();

    DumpGarbage();

    ResetNodeNumbers();

    ClearTris();
}


void PGMesh::RemoveNodeMergeEdges( PGNode* n )
{
    if ( n->m_EdgeVec.size() == 2 )
    {
        PGEdge *e0 = n->m_EdgeVec[0];
        PGEdge *e1 = n->m_EdgeVec[1];

        PGNode *n0 = e0->OtherNode( n );
        PGNode *n1 = e1->OtherNode( n );

        e0->ReplaceNode( n, n1 ); // Only changes node pointer, does not recurse.
        n1->AddConnectEdge( e0 ); // Adds edge to node's edge vec.

        RemoveEdge( e1 ); // Calls both nodes to RemoveConnectEdge - removes edge from node edgevec
                          // Sets nodes to null
                          // Removes edge from all faces.
                          // Clears face list.
                          // Adds to garbage vec
                          // Removes from mesh edge list.
                          // Sets deleteme flag

        RemoveNode( n ); // Removes node from all edges
                          // Clears edge list
                          // Clears TagUWMap
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

PGNode * PGMesh::StartList( const list < PGEdge * > & elist )
{
    list < PGEdge * >::const_iterator e0 = elist.begin();
    list < PGEdge * >::const_iterator e1 = e0;
    e1++;

    if ( *e0 && *e1 )
    {
        PGNode *nshared = (*e0)->SharedNode( *e1 );
        return (*e0)->OtherNode( nshared );
    }
    return NULL;
}

PGNode * PGMesh::StartLoop( const vector < PGEdge * > & eloop )
{
    if ( eloop.size() >= 2 )
    {
        PGNode *nshared = eloop[ 0 ]->SharedNode( eloop[1] );
        return eloop[ 0 ]->OtherNode( nshared );
    }
    return NULL;
}

void PGMesh::PrintColinearEdgeStrip( list < PGEdge * > &eloop )
{
    if ( !eloop.empty() )
    {
        double stot = 0;

        list< PGEdge* >::iterator e;

        for ( e = eloop.begin(); e != eloop.end(); e++ )
        {
            PGEdge *ei = *e;
            stot += dist( ei->m_N0->m_Pnt, ei->m_N1->m_Pnt );
        }

        double s = 0;

        printf( "stot: %f\n", stot );
        int ie = 0;
        for ( e = eloop.begin(); e != eloop.end(); e++ )
        {
            PGEdge *ei = *e;

            s += dist( ei->m_N0->m_Pnt, ei->m_N1->m_Pnt );
            printf( "%3d  %f  %f\n", ie, s/stot, s );
            ie++;
        }
        printf( "\n" );
    }
}


void PGMesh::PrintColinearEdgeStrip( const vector < PGEdge * > &eloop, PGNode * ni )
{
    if ( !eloop.empty() )
    {
        printf( "n %3d: %3d ", eloop.size(), ni->m_ID );
        for ( int ie = 0; ie < eloop.size(); ie++ )
        {
            PGEdge *ei = eloop[ ie ];
            // Advance ni to end of edge ei.
            ni = ei->OtherNode( ni );

            if ( ni )
            {
                printf( "%3d ", ni->m_ID );
            }
            else
            {
                printf( "XXX " );
            }

        }
        printf( "\n" );

    }
}

void PGMesh::PrintColinearEdgeLoop( const vector < PGEdge * > &eloop )
{
    if ( !eloop.empty() )
    {
        PGNode *ni = StartLoop( eloop );

        if ( ni )
        {
            PrintColinearEdgeStrip( eloop, ni );
        }
    }
}

void PGMesh::PrintColinearEdgeLoops()
{
    printf( "%d loops\n", m_EdgeLoopVec.size() );
    for ( int iloop = 0; iloop < m_EdgeLoopVec.size(); iloop++ )
    {
        vector < PGEdge * > eloop = m_EdgeLoopVec[ iloop ];
        printf( "loop %3d ", iloop );
        PrintColinearEdgeLoop( eloop );
    }
}

vector < PGNode * > PGMesh::MakeNodeVec( list < PGEdge * > elist )
{
    // Initialize ni at start of loop
    PGNode *ni = StartList( elist );

    vector < PGNode * > nodvec;
    nodvec.push_back( ni );
    for ( list < PGEdge * >::const_iterator ie = elist.begin(); ie != elist.end(); ie++ )
    {
        PGEdge *ei = *ie;
        // Advance ni to end of edge ei.
        ni = ei->OtherNode( ni );

        nodvec.push_back( ni );
    }
    return nodvec;
}

vector < PGNode * > PGMesh::MakeNodeVec( vector < PGEdge * > eloop )
{
    // Initialize ni at start of loop
    PGNode *ni = StartLoop( eloop );

    vector < PGNode * > nodvec;
    nodvec.push_back( ni );
    for ( int ie = 0; ie < eloop.size(); ie++ )
    {
        PGEdge *ei = eloop[ ie ];
        // Advance ni to end of edge ei.
        ni = ei->OtherNode( ni );

        nodvec.push_back( ni );
    }
    return nodvec;
}

std::pair < int, int > PGMesh::CheckLoopForDuplciateNodes( vector < PGEdge * > eloop, bool print )
{
    std::pair < int, int > match( -1, -1 );

    if ( !eloop.empty())
    {
        vector < PGNode * > nodvec = MakeNodeVec( eloop );

        match = vector_find_duplicates( nodvec, 1 );

        if ( print )
        {
            if ( match.first != -1 )
            {
                printf( "Loop has duplicate nodes at %d %d\n", match.first, match.second );
            }
            else
            {
                printf( "Loop has no duplicate nodes.\n" );
            }
        }
    }

    return match;
}


void PGMesh::SimplifyColinearEdgeLoops( bool print )
{
    if ( print )
    {
        printf( "before\n" );

        PrintColinearEdgeLoops();
    }

    for ( int iloop = 0; iloop < m_EdgeLoopVec.size(); iloop++ )
    {
        vector < PGEdge * > eloop = m_EdgeLoopVec[ iloop ];

        if ( !eloop.empty() )
        {
            // find duplicates.
            std::pair < int, int > match = CheckLoopForDuplciateNodes( eloop );

            while ( match.first != -1 )
            {
                // Decrement to convert node sequence to edge index.
                int ie = match.first - 1;
                int je = match.second - 1;

                vector < PGEdge * > new_loop;
                for ( int ke = ie + 1; ke <= je; ke++ )
                {
                    new_loop.push_back( eloop[ ke ] );
                }
                m_EdgeLoopVec.push_back( new_loop );

                eloop.erase( eloop.begin() + ie + 1, eloop.begin() + je + 1 );
                m_EdgeLoopVec[ iloop ] = eloop;

                // Re-search in case we need to go again
                match = CheckLoopForDuplciateNodes( eloop );
            }
        }
    }

    if ( print )
    {
        printf( "after\n" );

        PrintColinearEdgeLoops();

        for ( int iloop = 0; iloop < m_EdgeLoopVec.size(); iloop++ )
        {
            vector < PGEdge * > eloop = m_EdgeLoopVec[ iloop ];

            std::pair < int, int > match = CheckLoopForDuplciateNodes( eloop );

            if ( match.first != -1 )
            {
                printf( "Loop %d has duplicate nodes at %d %d\n", iloop, match.first, match.second );
            }
        }
    }
}

bool ColinearEdges( PGEdge *e1, PGEdge *e2 )
{
    double tol = 1e-6;
    double tol2 = 1e-10;

    if ( !e1 || !e2 || e1 == e2 )
    {
        return false;
    }

    PGNode *n1 = e1->SharedNode( e2 );

    if ( !n1 )
    {
        return false;
    }

    PGNode *n0 = e1->OtherNode( n1 );
    PGNode *n2 = e2->OtherNode( n1 );

    if ( n0 && n1 && n2 )
    {
        vec3d p0 = n0->m_Pnt;
        vec3d p1 = n1->m_Pnt;
        vec3d p2 = n2->m_Pnt;

        vec3d v10 = p1 - p0;
        vec3d v21 = p2 - p1;
        vec3d v20 = p2 - p0;

        double d10 = v10.mag();
        double d21 = v21.mag();
        double d20 = v20.mag();

        bool flip12 = false;
        if ( d10 > d20 ) // p1 is furthest from p0
        {
            flip12 = true;
        }
        else // p2 is furthese from p0
        {
            flip12 = false;
        }

        double t;
        double d;
        double l;

        if ( flip12 )
        {
            d = sqrt( pointSegDistSquared( p2, p0, p1, &t ) );
            l = d10;
        }
        else
        {
            d = sqrt( pointSegDistSquared( p1, p0, p2, &t ) );
            l = d21;
        }

        if ( d / l < tol && d < tol2 )
        {
            return true;
        }
    }
    return false;
}

bool PGMesh::ExtendCoLinearLoop( vector < PGEdge * > & eloop, PGNode * n0, PGEdge *e, PGNode *n )
{
    e->m_InCurrentLoopFlag = true;

    int nedg = n->m_EdgeVec.size();

    for ( int i = 0; i < nedg; i++ )
    {
        PGEdge * ei = n->m_EdgeVec[ i ];
        if ( ei && (ei != e) && (!ei->m_InLoopFlag) && (!ei->m_InCurrentLoopFlag) )
        {
            PGNode * ni = ei->OtherNode( n );

            if ( ni )
            {
                if ( ni == n0 ) // Loop closed.  Add ending edge
                {
                    eloop.push_back( ei );
                    ei->m_InLoopFlag = true;
                    return true;
                }

                if ( ColinearEdges( e, ei ) )
                {
                    if ( ExtendCoLinearLoop( eloop, n0, ei, ni ) )
                    {
                        eloop.push_back( ei );
                        ei->m_InLoopFlag = true;

                        if ( e->OtherNode( n ) == n0 ) // Adds starting edge.
                        {
                            eloop.push_back( e );
                            e->m_InLoopFlag = true;

                        }

                        return true;
                    }
                }
            }
        }
    }

    e->m_InCurrentLoopFlag = false;
    return false;
}

void PGMesh::FindAllColinearEdgeLoops()
{
    m_EdgeLoopVec.clear();

    vector< PGEdge* > eVec( m_EdgeList.begin(), m_EdgeList.end() );
    for ( int i = 0; i < eVec.size(); i++ )
    {
        PGEdge *e1 = eVec[ i ];

        if ( !e1->m_InLoopFlag )
        {
            vector < PGEdge * > eloop;
            if ( ExtendCoLinearLoop( eloop, e1->m_N0, e1, e1->m_N1 ))
            {
                m_EdgeLoopVec.push_back( eloop );
            }
        }
    }
    ResetEdgeLoopFlags();
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


void PGMesh::ExtendWake( vector < PGEdge * > & wake, PGEdge *e, PGNode *n )
{
    e->m_InCurrentLoopFlag = true;

    int nedg = n->m_EdgeVec.size();

    for ( int i = 0; i < nedg; i++ )
    {
        PGEdge * ei = n->m_EdgeVec[ i ];
        if ( ei && ei != e && !ei->m_InLoopFlag && !ei->m_InCurrentLoopFlag )
        {
            if ( ei->WakeEdge( this ) )
            {
                PGNode * ni = ei->OtherNode( n );

                wake.push_back( ei );
                ei->m_InLoopFlag = true;

                ExtendWake( wake, ei, ni );
                return;
            }
        }
    }

    e->m_InCurrentLoopFlag = false;
}


void PGMesh::IdentifyWakes()
{
    m_WakeVec.clear();

    list< PGEdge* >::iterator e;
    for ( e = m_EdgeList.begin() ; e != m_EdgeList.end(); ++e )
    {
        if ( !( ( *e )->m_InLoopFlag ) && ( *e )->WakeEdge( this ) )
        {
            (*e)->m_InLoopFlag = true;

            vector < PGEdge * > wake;
            wake.push_back( *e );

            ExtendWake( wake, (*e), (*e)->m_N0 );

            std::reverse( wake.begin(), wake.end() );

            ExtendWake( wake, (*e), (*e)->m_N1 );


            m_WakeVec.push_back( wake );
        }
    }

    printf( "IdentifyWakes() %d wakes found.\n", m_WakeVec.size() );

    ResetEdgeLoopFlags();
}

vector < int > PGMesh::BuildLoopDirectionVec( const vector < PGEdge * > &eloop, bool print )
{
    int nseg = eloop.size();

    vector < int > dirvec( nseg, 1 );

    // Initialize ni at start of loop
    PGNode *n0 = StartLoop( eloop );
    if ( !n0 ) return dirvec;
    PGEdge *ei = eloop[ 0 ];
    if ( !ei ) return dirvec;
    PGNode *n1 = ei->OtherNode( n0 );
    if ( !n1 ) return dirvec;

    vec3d v0 = n1->m_Pnt - n0->m_Pnt;
    v0.normalize();

    n0 = n1;
    for ( int ie = 1; ie < nseg; ie++ )
    {
        ei = eloop[ ie ];
        n1 = ei->OtherNode( n0 );

        vec3d vi = n1->m_Pnt - n0->m_Pnt;
        vi.normalize();

        if ( dot( v0, vi ) < 0 )
        {
            dirvec[ ie ] = -1;
        }

        n0 = n1;
    }

    if ( print )
    {
        printf( "        " );

        for ( int ie = 0; ie < nseg; ie++ )
        {
            printf( "%3d ", dirvec[ ie ] );
        }
        printf( "\n" );
    }

    return dirvec;
}

int PGMesh::RotateLoopToDirectionChange( vector < PGEdge * > &eloop, bool print )
{
    vector < int > dirvec = BuildLoopDirectionVec( eloop );

    int iswitch = vector_find_val( dirvec, -1 );

    // Rotate such that iswitch is first in loop.
    std::rotate( eloop.begin(), eloop.begin() + iswitch, eloop.end() );

    std::rotate( dirvec.begin(), dirvec.begin() + iswitch, dirvec.end() );

    if ( print )
    {
        printf( "        " );

        for ( int ie = 0; ie < dirvec.size(); ie++ )
        {
            printf( "%3d ", dirvec[ ie ] );
        }
        printf( "\n" );
    }

    return vector_find_val( dirvec, 1 );
}

void PGMesh::SealColinearEdgeLoop( vector < PGEdge * > &eloop )
{
    double tol = 1e-4;

    // Turn loop into two parallel strips.
    int irev = RotateLoopToDirectionChange( eloop );

    vector < PGEdge * > estripA( eloop.begin(), eloop.begin() + irev );
    vector < PGEdge * > estripB( eloop.begin() + irev, eloop.end() );
    reverse( estripB.begin(), estripB.end() );

    list < PGEdge* > elistA( estripA.begin(), estripA.end() );
    list < PGEdge* > elistB( estripB.begin(), estripB.end() );


    // Find starting node for each strip.
    PGNode *n0 = estripA[0]->SharedNode( estripB[0] );

    PGNode *n0A = n0;
    PGNode *n0B = n0;

    // Start at base of each edge.
    list< PGEdge* >::iterator eA = elistA.begin();
    list< PGEdge* >::iterator eB = elistB.begin();

    while ( next(eA) != elistA.end() || next(eB) != elistB.end() )
    {
        PGNode *nA = (*eA)->OtherNode( n0A );
        PGNode *nB = (*eB)->OtherNode( n0B );

        // Project A onto B and B onto A.
        double tA, tB;
        pointLineDistSquared( nA->m_Pnt, n0B->m_Pnt, nB->m_Pnt, &tB );
        pointLineDistSquared( nB->m_Pnt, n0A->m_Pnt, nA->m_Pnt, &tA );

        // Either tA or tB should be less than 1.  We choose path of lower t value.

        if ( tA < tB )
        {
            PGEdge * newedge = SplitEdge( (*eA), tA, n0A );
            PGNode * newnode = newedge->SharedNode( (*eA) );

            if ( (*eA)->ContainsNode( n0A ) ) // newe comes after eA
            {
                eA++; // Advance iterator as insert() inserts before iterator.
                eA = elistA.insert( eA, newedge ); // Insert and point iterator at newe
            }
            else // newe comes before eA
            {
                elistA.insert( eA, newedge ); // insert() inserts before iterator.
            }
            n0A = newnode;
            eB++; // Advance other iterator
            n0B = nB;

        }
        else
        {
            PGEdge * newedge = SplitEdge( (*eB), tB, n0B );
            PGNode * newnode = newedge->SharedNode( (*eB) );

            if ( (*eB)->ContainsNode( n0B ) ) // newe comes after eB
            {
                eB++; // Advance iterator as insert() inserts before iterator.
                eB = elistB.insert( eB, newedge ); // Insert and point iterator at newe
            }
            else // newe comes before eA
            {
                elistB.insert( eB, newedge ); // insert() inserts before iterator.
            }
            n0B = newnode;
            eA++; // Advance other iterator
            n0A = nA;
        }
    }


    vector < PGNode * > nodevecA = MakeNodeVec( elistA );
    vector < PGNode * > nodevecB = MakeNodeVec( elistB );

    if ( nodevecA.size() == nodevecA.size() )
    {
        for ( int i = 1; i < nodevecA.size() - 1; i++ )
        {
            PGNode *nA = nodevecA[ i ];
            PGNode *nB = nodevecB[ i ];

            if ( nA && nB && nA != nB )
            {
                MergeNodes( nA, nB );
            }
        }
    }

}


void PGMesh::SealColinearEdgeLoops()
{
    for ( int iloop = 0; iloop < m_EdgeLoopVec.size(); iloop++ )
    {
        SealColinearEdgeLoop( m_EdgeLoopVec[ iloop ] );
    }
    m_EdgeLoopVec.clear();
}

void PGMesh::StartMatlab()
{
    printf( "clear all;\n" );
    printf( "close all;\n" );
    printf( "format compact;\n" );
}

PGEdge * PGMesh::SplitEdge( PGEdge *e0, PGNode *n )
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

    return e1;
}

PGEdge * PGMesh::SplitEdge( PGEdge *e, double t, PGNode *n0 )
{
    PGNode *n1 = e->OtherNode( n0 );

    vec3d p = ( 1.0 - t ) * n0->m_Pnt + t * n1->m_Pnt;

    PGNode * newnode = AddNode( p );

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

void PGMesh::CullOrphanThinRegions( double tol )
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
        int part = GetPart( region_tag[ i ] );
        region_thick[ i ] = GetThickThin( part );
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

void PGMesh::Report()
{
    ResetNodeNumbers();

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


    printf( "%10d   Nodes\n", m_NodeList.size() );
    printf( "%10d   Edges\n", m_EdgeList.size() );
    printf( "%10d   Faces\n", m_FaceList.size() );

    int inode = 1; // Start numbering at 1
    list< PGNode* >::iterator n;
    for ( n = m_NodeList.begin() ; n != m_NodeList.end(); ++n )
    {
        inode++;
    }


    printf( "Non-manifold edges\n" );
    int iedge = 1;
    list< PGEdge* >::iterator e;
    for ( e = m_EdgeList.begin() ; e != m_EdgeList.end(); ++e )
    {
        if ( ( *e )->m_FaceVec.size() != 2 )
        {
            printf( "Edge %d has %d faces\n", iedge, ( *e )->m_FaceVec.size() );
            printf( "    Node %d %f %f %f\n", ( *e )->m_N0->m_ID + 1, ( *e )->m_N0->m_Pnt.x(), ( *e )->m_N0->m_Pnt.y(), ( *e )->m_N0->m_Pnt.z() );
            printf( "    Node %d %f %f %f\n", ( *e )->m_N1->m_ID + 1, ( *e )->m_N1->m_Pnt.x(), ( *e )->m_N1->m_Pnt.y(), ( *e )->m_N1->m_Pnt.z() );
        }

        iedge++;
    }



    list< PGFace* >::iterator f;
    for ( f = m_FaceList.begin() ; f != m_FaceList.end(); ++f )
    {

    }


    printf( "Done\n" );


}

void PGMesh::ResetNodeNumbers()
{
    int inode = 0; // Start numbering at 0
    list< PGNode* >::iterator n;
    for ( n = m_NodeList.begin() ; n != m_NodeList.end(); ++n )
    {
        // Assign ID number.
        ( *n )->m_ID = inode;
        inode++;
    }
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

void PGMesh::PolygonizeMesh()
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

            if ( f0->m_iQuad >= 0 &&
                 f0->m_iQuad == f1->m_iQuad &&
                 f0->m_Tag == f1->m_Tag )
            {
                RemoveEdgeMergeFaces( e );
            }
        }
    }
    DumpGarbage();
    ResetNodeNumbers();

}

void PGMesh::CleanColinearVerts()
{
    // Node colinearity tolerance.
    double tol = 1.0e-12;

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

        if ( n->ColinearNode( tol ) )
        {
            RemoveNodeMergeEdges( n );
            ncolinear++;
        }
    }

    printf( "%d colinear verts removed\n", ncolinear );

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
    ResetNodeNumbers();

    fprintf( file_id, "%d\n", m_NodeList.size() );

    //==== Write Out Nodes ====//
    vec3d v;

    list< PGNode* >::iterator n;
    for ( n = m_NodeList.begin() ; n != m_NodeList.end(); ++n )
    {
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
            fprintf( file_id, " %d", nodVec[i]->m_ID + 1 );
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
        int part = GetPart( tag );
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
            fprintf( file_id, "%d", nodVec[i]->m_ID + 1 );

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
            fprintf( file_id, " %d", nodVec[i]->m_ID + 1 );
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
        int part = GetPart( tag );
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

void PGMesh::WriteTagFiles( string file_name, vector < string > &all_fnames )
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
        FILE* csf_taglist_fid = NULL;
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

void PGMesh::WriteTagFile( FILE* file_id, int part, int tag )
{
    //==== Write Tri IDs for each tag =====//

    int count = 0;
    list< PGFace* >::iterator f;
    for ( f = m_FaceList.begin() ; f != m_FaceList.end(); ++f )
    {
        int singletag = ( *f )->m_Tag;

        if ( MatchPartAndTag( singletag, part, tag ) )
        {
            count++;
        }
    }
    fprintf( file_id, "%d\n\n", count );

    int iface = 1;
    for ( f = m_FaceList.begin() ; f != m_FaceList.end(); ++f )
    {
        int singletag = ( *f )->m_Tag;

        if ( MatchPartAndTag( singletag, part, tag ) )
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

void PGMesh::WriteSTL( string fname )
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
                vec3d v = nodVec[ inod ]->m_Pnt;
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

void PGMesh::WriteTRI( string fname )
{
    FILE* fp = fopen( fname.c_str(), "w" );

    if ( !fp )
    {
        return;
    }

    ResetNodeNumbers(); // Start numbering at 1.

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
        vec3d v = ( *n )->m_Pnt;
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
            fprintf( fp, "%d ", nodVec[ inod ]->m_ID + 1 );
            inod++;
            fprintf( fp, "%d ", nodVec[ inod ]->m_ID + 1 );
            inod++;
            fprintf( fp, "%d\n", nodVec[ inod ]->m_ID + 1 );
            inod++;
        }
    }
    fclose( fp );
}

string PGMesh::GetTagNames( const vector<int> & tags )
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

string PGMesh::GetTagNames( int indx )
{
    if ( indx < m_TagKeys.size() && indx >= 0 )
    {
        return GetTagNames( m_TagKeys[indx] );
    }
    return string( "Error_Tag" );
}

string PGMesh::GetTagIDs( const vector<int>& tags )
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

string PGMesh::GetTagIDs( int indx )
{
    if ( indx < m_TagKeys.size() && indx >= 0 )
    {
        return GetTagIDs( m_TagKeys[indx] );
    }
    return string( "Error_Tag" );
}

string PGMesh::GetGID( const int& tag )
{
    string id_list = GetTagIDs( tag - 1 );

    // Find position of token _Surf
    int spos = id_list.find( "_Surf" );
    string gid = id_list.substr( 0, spos );
    string gid_bare = gid.substr( 0, 10 );

    return gid_bare;
}

bool PGMesh::MatchPartAndTag( const vector < int > & tags, int part, int tag )
{
    if ( tags.size() > 0 )
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

bool PGMesh::MatchPartAndTag( int singletag, int part, int tag )
{
    if ( m_TagKeys.size() >= singletag )
    {
        return MatchPartAndTag( m_TagKeys[ singletag - 1 ], part, tag );
    }
    return false;
}

bool PGMesh::ExistPartAndTag( int part, int tag )
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

void PGMesh::MakePartList( std::vector < int > & partvec )
{
    std::set< int > partset;
    for ( int i = 0 ; i < ( int )m_TagKeys.size() ; i++ )
    {
        if ( m_TagKeys[i].size() > 0 )
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

int PGMesh::GetTag( const vector<int> & tags )
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

vector< int > PGMesh::GetTagVec( const int &tin )
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

int PGMesh::GetType( int part )
{
    return m_TypeVec[ part - 1 ];
}

int PGMesh::GetThickThin( int part )
{
    return m_ThickVec[ part - 1 ];
}

double PGMesh::GetWmin( int part )
{
    return m_WminVec[ part - 1 ];
}

int PGMesh::GetPart( const vector<int> & tags )
{
    if ( tags.size() > 0 )
    {
        return tags[0];
    }
    return -1;
}

int PGMesh::GetPart( int tag )
{
    if ( m_TagKeys.size() >= tag )
    {
        vector < int > tags = m_TagKeys[ tag - 1 ];
        return GetPart( tags );
    }
    return -1;
}

void PGMesh::GetPartData( vector < string > &gidvec, vector < int > &partvec, vector < int > &surfvec )
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
        surfvec.push_back( stoi( snum.c_str() ) );
    }
}

void PGMesh::BuildFromTMesh( TMesh* tmi )
{
    // Archive tag data at time of NGonMeshGeom creation.
    m_TagNames = SubSurfaceMgr.m_TagNames;
    m_TagIDs = SubSurfaceMgr.m_TagIDs;
    m_ThickVec = SubSurfaceMgr.m_CompThick;
    m_TypeVec = SubSurfaceMgr.m_CompTypes;
    m_WminVec = SubSurfaceMgr.m_CompWmin;
    m_TagKeys = SubSurfaceMgr.GetTagKeys();
    m_SingleTagMap = SubSurfaceMgr.GetSingleTagMap();

    vector < PGNode* > nod;

    for ( int j = 0; j < tmi->m_NVec.size(); j++ )
    {
        TNode *nj = tmi->m_NVec[j];
        nj->m_ID = nod.size();
        nod.push_back( AddNode( nj->m_Pnt ) );
    }

    for ( int j = 0; j < tmi->m_TVec.size(); j++ )
    {
        TTri *tj = tmi->m_TVec[j];

        if ( tj->m_SplitVec.size() )
        {
            for ( int s = 0; s < ( int ) tj->m_SplitVec.size(); s++ )
            {
                TTri *ts = tj->m_SplitVec[ s ];
                if ( !ts->m_IgnoreTriFlag )
                {
                    int tag = GetTag( ts->m_Tags );
                    AddFace( nod[ ts->m_N0->m_ID ],
                             nod[ ts->m_N1->m_ID ],
                             nod[ ts->m_N2->m_ID ],
                             ts->m_N0->m_UWPnt.as_vec2d_xy(),
                             ts->m_N1->m_UWPnt.as_vec2d_xy(),
                             ts->m_N2->m_UWPnt.as_vec2d_xy(),
                             ts->m_Norm,
                             ts->m_iQuad,
                             tag );
                }
            }
        }
        else
        {
            if ( !tj->m_IgnoreTriFlag )
            {
                int tag = GetTag( tj->m_Tags );
                AddFace( nod[ tj->m_N0->m_ID ],
                         nod[ tj->m_N1->m_ID ],
                         nod[ tj->m_N2->m_ID ],
                         tj->m_N0->m_UWPnt.as_vec2d_xy(),
                         tj->m_N1->m_UWPnt.as_vec2d_xy(),
                         tj->m_N2->m_UWPnt.as_vec2d_xy(),
                         tj->m_Norm,
                         tj->m_iQuad,
                         tag );
            }
        }
    }

}

void PGMesh::BuildFromTMeshVec( const vector< TMesh* > tmv )
{
    // Archive tag data at time of NGonMeshGeom creation.
    m_TagNames = SubSurfaceMgr.m_TagNames;
    m_TagIDs = SubSurfaceMgr.m_TagIDs;
    m_ThickVec = SubSurfaceMgr.m_CompThick;
    m_TypeVec = SubSurfaceMgr.m_CompTypes;
    m_WminVec = SubSurfaceMgr.m_CompWmin;
    m_TagKeys = SubSurfaceMgr.GetTagKeys();
    m_SingleTagMap = SubSurfaceMgr.GetSingleTagMap();

    vector < PGNode* > nod;
    for ( int i = 0; i < tmv.size(); i++ )
    {
        TMesh *tmi = tmv[ i ];

        for ( int j = 0; j < tmi->m_NVec.size(); j++ )
        {
            TNode *nj = tmi->m_NVec[j];
            nj->m_ID = nod.size();
            nod.push_back( AddNode( nj->m_Pnt ) );
        }
    }

    for ( int i = 0; i < tmv.size(); i++ )
    {
        TMesh *tmi = tmv[ i ];

        for ( int j = 0; j < tmi->m_TVec.size(); j++ )
        {
            TTri *tj = tmi->m_TVec[j];

            if ( tj->m_SplitVec.size() )
            {
                for ( int s = 0; s < ( int ) tj->m_SplitVec.size(); s++ )
                {
                    TTri *ts = tj->m_SplitVec[ s ];
                    if ( !ts->m_IgnoreTriFlag )
                    {
                        int tag = GetTag( ts->m_Tags );
                        AddFace( nod[ ts->m_N0->m_ID ],
                                 nod[ ts->m_N1->m_ID ],
                                 nod[ ts->m_N2->m_ID ],
                                 ts->m_N0->m_UWPnt.as_vec2d_xy(),
                                 ts->m_N1->m_UWPnt.as_vec2d_xy(),
                                 ts->m_N2->m_UWPnt.as_vec2d_xy(),
                                 ts->m_Norm,
                                 ts->m_iQuad,
                                 tag );
                    }
                }
            }
            else
            {
                if ( !tj->m_IgnoreTriFlag )
                {
                    int tag = GetTag( tj->m_Tags );
                    AddFace( nod[ tj->m_N0->m_ID ],
                             nod[ tj->m_N1->m_ID ],
                             nod[ tj->m_N2->m_ID ],
                             tj->m_N0->m_UWPnt.as_vec2d_xy(),
                             tj->m_N1->m_UWPnt.as_vec2d_xy(),
                             tj->m_N2->m_UWPnt.as_vec2d_xy(),
                             tj->m_Norm,
                             tj->m_iQuad,
                             tag );
                }
            }
        }
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
