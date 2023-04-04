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

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

PGNode::PGNode()
{
    m_DeleteMeFlag = false;
}

PGNode::PGNode( const vec3d& p, const vec2d& uw_in )
{
    m_Pnt = p;
    m_UW = uw_in;
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

void PGFace::EdgeForgetFace()
{
    for ( int i = 0; i < m_EdgeVec.size(); i++ )
    {
        m_EdgeVec[i]->RemoveFace( this );
    }
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

PGNode* PGMesh::AddNode( vec3d p, vec2d uw_in )
{
    PGNode* nptr = new PGNode( p, uw_in );
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
