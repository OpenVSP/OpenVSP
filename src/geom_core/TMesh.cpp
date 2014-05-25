//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

//******************************************************************************
//
//    External Geometry Class
//
//
//   J.R. Gloudemans - 11/12/03
//
//
//******************************************************************************

#ifdef WIN32
#include <windows.h>
#endif

#include "TMesh.h"
#include "BndBox.h"

#include "Tritri.h"
#include "Defines.h"

#include "triangle.h"
#include "Util.h"
#include "Geom.h"

#include <map>
#include <set>
#include <algorithm>


//===============================================//
//                  TNode
//===============================================//
TNode::TNode()
{
    m_ID = -1;
//  mapNode = 0;
    m_IsectFlag = 0;
    m_XYZFlag = true; // true if xyz
    m_CoordInfo = HAS_UNKNOWN;
}

TNode::~TNode()
{


}

void TNode::MakePntUW()
{
    if ( m_XYZFlag )
    {
        std::swap( m_Pnt, m_UWPnt );
        m_XYZFlag = false;
    }
}

void TNode::MakePntXYZ()
{
    if( !m_XYZFlag )
    {
        std::swap( m_Pnt, m_UWPnt );
        m_XYZFlag = true;
    }
}

vec3d TNode::GetXYZPnt()
{
    if ( m_XYZFlag )
    {
        return m_Pnt;
    }
    else
    {
        return m_UWPnt;
    }
}

vec3d TNode::GetUWPnt()
{
    if ( !m_XYZFlag )
    {
        return m_Pnt;
    }
    else
    {
        return m_UWPnt;
    }
}

void TNode::SetXYZPnt( const vec3d & pnt )
{
    if ( m_XYZFlag )
    {
        m_Pnt = pnt;
    }
    else
    {
        m_UWPnt = pnt;
    }

    m_CoordInfo |= HAS_XYZ;
}

void TNode::SetUWPnt( const vec3d & pnt )
{
    if ( !m_XYZFlag )
    {
        m_Pnt = pnt;
    }
    else
    {
        m_UWPnt = pnt;
    }

    m_CoordInfo |= HAS_UW;
}

//=======================================================================//
//===================            TEdge          =========================//
//=======================================================================//

TEdge::TEdge()
{
    m_N0 = m_N1 = 0;
    m_ParTri = NULL;
    m_Tri0 = m_Tri1 = NULL;
}

TEdge::TEdge( TNode* n0, TNode* n1, TTri* par_tri )
{
    m_N0 = n0;
    m_N1 = n1;
    m_ParTri = par_tri;
    m_Tri0 = m_Tri1 = NULL;
}

TMesh* TEdge::GetParTMesh()
{
    if ( GetParTri() == NULL )
    {
        return NULL;
    }
    return GetParTri()->GetTMeshPtr();
}

//=======================================================================//
//=======================================================================//
//=======================================================================//
TetraMassProp::TetraMassProp( string id, double denIn, vec3d& p0, vec3d& p1, vec3d& p2, vec3d& p3 )
{
    m_CompId = id;
    m_PointMassFlag = false;
    m_Density = denIn;

    m_v0 = p0;
    m_v1 = p1 - p0;
    m_v2 = p2 - p0;
    m_v3 = p3 - p0;

    m_CG = m_v1 + m_v2 + m_v3;
    m_CG = ( m_CG * 0.25 ) + p0;

    m_Vol  = tetra_volume( m_v1, m_v2, m_v3 );
    m_Mass = m_Density * fabs( m_Vol );

    double Ix = m_Mass / 10.0 * ( m_v1.x() * m_v1.x() + m_v2.x() * m_v2.x() + m_v3.x() * m_v3.x() +
                                  m_v1.x() * m_v2.x() + m_v1.x() * m_v3.x() + m_v2.x() * m_v3.x() );

    double Iy = m_Mass / 10.0 * ( m_v1.y() * m_v1.y() + m_v2.y() * m_v2.y() + m_v3.y() * m_v3.y() +
                                  m_v1.y() * m_v2.y() + m_v1.y() * m_v3.y() + m_v2.y() * m_v3.y() );

    double Iz = m_Mass / 10.0 * ( m_v1.z() * m_v1.z() + m_v2.z() * m_v2.z() + m_v3.z() * m_v3.z() +
                                  m_v1.z() * m_v2.z() + m_v1.z() * m_v3.z() + m_v2.z() * m_v3.z() );

    m_Ixx = Iy + Iz;
    m_Iyy = Ix + Iz;
    m_Izz = Ix + Iy;

    m_Ixy = m_Mass / 20.0 * ( 2.0 * ( m_v1.x() * m_v1.y() + m_v2.x() * m_v2.y() + m_v3.x() * m_v3.y() ) +
                              m_v1.x() * m_v2.y() + m_v2.x() * m_v1.y() + m_v1.x() * m_v3.y() + m_v3.x() * m_v1.y() + m_v2.x() * m_v3.y() + m_v3.x() * m_v2.y() );

    m_Iyz = m_Mass / 20.0 * ( 2.0 * ( m_v1.y() * m_v1.z() + m_v2.y() * m_v2.z() + m_v3.y() * m_v3.z() ) +
                              m_v1.y() * m_v2.z() + m_v2.y() * m_v1.z() + m_v1.y() * m_v3.z() + m_v3.y() * m_v1.z() + m_v2.y() * m_v3.z() + m_v3.y() * m_v2.z() );

    m_Ixz = m_Mass / 20.0 * ( 2.0 * ( m_v1.x() * m_v1.z() + m_v2.x() * m_v2.z() + m_v3.x() * m_v3.z() ) +
                              m_v1.x() * m_v2.z() + m_v2.x() * m_v1.z() + m_v1.x() * m_v3.z() + m_v3.x() * m_v1.z() + m_v2.x() * m_v3.z() + m_v3.x() * m_v2.z() );

}


void TetraMassProp::SetPointMass( double massIn, vec3d& pos )
{
    m_CompId = "NONE";
    m_PointMassFlag = true;
    m_Density = 0.0;
    m_CG = pos;
    m_Vol  = 0.0;
    m_Mass = massIn;

    m_Ixx = 0.0;
    m_Iyy = 0.0;
    m_Izz = 0.0;

    m_Ixy = 0.0;
    m_Iyz = 0.0;
    m_Ixz = 0.0;

}








//=======================================================================//
//=======================================================================//
//=======================================================================//
TriShellMassProp::TriShellMassProp( string id, double mass_area_in, vec3d& p0, vec3d& p1, vec3d& p2 )
{
    m_CompId = id;

    m_MassArea = mass_area_in;

    m_CG = ( p0 + p1 + p2 ) * ( 1.0 / 3.0 );

    m_v0 = p0 - m_CG;
    m_v1 = p1 - m_CG;
    m_v2 = p2 - m_CG;

    m_TriArea = area( m_v0, m_v1, m_v2 );

    m_Mass = m_TriArea * m_MassArea;

    double Ix = m_Mass / 10.0 * ( m_v0.x() * m_v0.x() + m_v1.x() * m_v1.x() + m_v2.x() * m_v2.x() +
                                  m_v0.x() * m_v1.x() + m_v0.x() * m_v2.x() + m_v1.x() * m_v2.x() );

    double Iy = m_Mass / 10.0 * ( m_v0.y() * m_v0.y() + m_v1.y() * m_v1.y() + m_v2.y() * m_v2.y() +
                                  m_v0.y() * m_v1.y() + m_v0.y() * m_v2.y() + m_v1.y() * m_v2.y() );

    double Iz = m_Mass / 10.0 * ( m_v0.z() * m_v0.z() + m_v1.z() * m_v1.z() + m_v2.z() * m_v2.z() +
                                  m_v0.z() * m_v1.z() + m_v0.z() * m_v2.z() + m_v1.z() * m_v2.z() );

    m_Ixx = Iy + Iz;
    m_Iyy = Ix + Iz;
    m_Izz = Ix + Iy;

    m_Ixy = m_Mass / 20.0 * ( 2.0 * ( m_v0.x() * m_v0.y() + m_v1.x() * m_v1.y() + m_v2.x() * m_v2.y() ) +
                              m_v0.x() * m_v1.y() + m_v1.x() * m_v0.y() + m_v0.x() * m_v2.y() + m_v2.x() * m_v0.y() + m_v1.x() * m_v2.y() + m_v2.x() * m_v1.y() );

    m_Iyz = m_Mass / 20.0 * ( 2.0 * ( m_v0.y() * m_v0.z() + m_v1.y() * m_v1.z() + m_v2.y() * m_v2.z() ) +
                              m_v0.y() * m_v1.z() + m_v1.y() * m_v0.z() + m_v0.y() * m_v2.z() + m_v2.y() * m_v0.z() + m_v1.y() * m_v2.z() + m_v2.y() * m_v1.z() );

    m_Ixz = m_Mass / 20.0 * ( 2.0 * ( m_v0.x() * m_v0.z() + m_v1.x() * m_v1.z() + m_v2.x() * m_v2.z() ) +
                              m_v0.x() * m_v1.z() + m_v1.x() * m_v0.z() + m_v0.x() * m_v2.z() + m_v2.x() * m_v0.z() + m_v1.x() * m_v2.z() + m_v2.x() * m_v1.z() );


}

//===============================================//
//===============================================//
//===============================================//
//===============================================//
//                  TMesh
//===============================================//
//===============================================//
//===============================================//
//===============================================//
TMesh::TMesh()
{
    m_DeleteMeFlag = false;
    m_TheoArea = m_WetArea = 0.0;
    m_TheoVol    = m_WetVol = 0.0;
    m_HalfBoxFlag = false;
    m_SurfNum = 0;
}

TMesh::~TMesh()
{

    int i;

    for ( i = 0 ; i < ( int )m_TVec.size() ; i++ )
    {
        delete m_TVec[i];
    }
    for ( i = 0 ; i < ( int )m_NVec.size() ; i++ )
    {
        delete m_NVec[i];
    }
    for ( i = 0 ; i < ( int )m_EVec.size() ; i++ )
    {
        delete m_EVec[i];
    }

}

void TMesh::copy( TMesh* m )
{
    CopyAttributes( m );
    m_TVec.clear();
    m_NVec.clear();

    for ( int i = 0 ; i < ( int )m->m_TVec.size() ; i++ )
    {
        TTri* tri = new TTri();
        tri->m_N0   = new TNode();
        tri->m_N1   = new TNode();
        tri->m_N2   = new TNode();

        tri->m_Norm    = m->m_TVec[i]->m_Norm;
        tri->m_N0->m_Pnt = m->m_TVec[i]->m_N0->m_Pnt;
        tri->m_N1->m_Pnt = m->m_TVec[i]->m_N1->m_Pnt;
        tri->m_N2->m_Pnt = m->m_TVec[i]->m_N2->m_Pnt;

        m_TVec.push_back( tri );
        m_NVec.push_back( tri->m_N0 );
        m_NVec.push_back( tri->m_N1 );
        m_NVec.push_back( tri->m_N2 );
    }
}

void TMesh::CopyFlatten( TMesh* m )
{
    CopyAttributes( m );
    m_TVec.clear();
    m_NVec.clear();

    for ( int i = 0 ; i < ( int )m->m_TVec.size() ; i++ )
    {
        TTri* orig_tri = m->m_TVec[i];

        if ( orig_tri->m_SplitVec.size() )
        {
            for ( int s = 0 ; s < ( int )orig_tri->m_SplitVec.size() ; s++ )
            {
                TTri* s_tri = orig_tri->m_SplitVec[s];
                if ( !s_tri->m_InteriorFlag )
                {
                    AddTri( s_tri->m_N0, s_tri->m_N1, s_tri->m_N2, s_tri->m_Norm );
                }
            }
        }
        else
        {
            if ( !orig_tri->m_InteriorFlag )
            {
                AddTri( orig_tri->m_N0, orig_tri->m_N1, orig_tri->m_N2, orig_tri->m_Norm );
            }
        }
    }
}

void TMesh::CopyAttributes( TMesh* m )
{
    m_PtrID     = m->m_PtrID;
    m_SurfNum   = m->m_SurfNum;
    m_NameStr   = m->m_NameStr;
    m_MaterialID = m->m_MaterialID;
    m_Color      = m->m_Color;

    m_TheoArea   = m->m_TheoArea;
    m_WetArea    = m->m_WetArea;
    m_TheoVol    = m->m_TheoVol;
    m_GuessVol   = m->m_GuessVol;
    m_WetVol     = m->m_WetVol;

    m_MassPrior = m->m_MassPrior;
    m_Density   = m->m_Density;
    m_ShellFlag = m->m_ShellFlag;
    m_ShellMassArea = m->m_ShellMassArea;

    m_DragFactors = m->m_DragFactors;

    m_UWPnts = m->m_UWPnts;
    m_XYZPnts = m->m_XYZPnts;
}

xmlNodePtr TMesh::EncodeXml( xmlNodePtr & node )
{
    xmlNodePtr tmesh_node = xmlNewChild( node, NULL, BAD_CAST "TMesh", NULL );
    XmlUtil::AddIntNode( tmesh_node, "Num_Tris", ( int )m_TVec.size() );
    EncodeTriList( tmesh_node );
    return tmesh_node;
}

xmlNodePtr TMesh::EncodeTriList( xmlNodePtr & node )
{
    xmlNodePtr tri_list_node = xmlNewChild( node, NULL, BAD_CAST "Tri_List", NULL );
    vector<vec3d> tri;
    tri.resize( 4 );
    for ( int i = 0 ; i < ( int ) m_TVec.size() ; i++ )
    {
        tri[0] = m_TVec[i]->m_N0->m_Pnt;
        tri[1] = m_TVec[i]->m_N1->m_Pnt;
        tri[2] = m_TVec[i]->m_N2->m_Pnt;
        tri[3] = m_TVec[i]->m_Norm;
        XmlUtil::AddVectorVec3dNode( tri_list_node, "Tri", tri );
    }
    return tri_list_node;
}

void TMesh::DecodeXml( xmlNodePtr & node )
{
    xmlNodePtr tri_list_node = XmlUtil::GetNode( node, "Tri_List", 0 );
    if ( tri_list_node )
    {
        int num_tris = 0;
        xmlNodePtr num_tri_node = XmlUtil::GetNode( node, "Num_Tris", 0 );
        if ( num_tri_node )
        {
            num_tris = XmlUtil::ExtractInt( num_tri_node );
        }
        else
        {
            num_tris = XmlUtil::GetNumNames( tri_list_node, "Tri" );
        }

        DecodeTriList( tri_list_node, num_tris );
    }
}

void TMesh::DecodeTriList( xmlNodePtr & node, int num_tris )
{
    m_TVec.resize( num_tris );
    vector<vec3d> tri;
    tri.resize( 4 );
    xmlNodePtr tri_node;

    for ( int i = 0 ; i < num_tris ; i++ )
    {
        tri_node = XmlUtil::GetNode( node, "Tri", i );
        if ( tri_node )
        {
            tri = XmlUtil::GetVectorVec3dNode( tri_node );
            m_TVec[i] = new TTri();
            // Create Nodes
            m_TVec[i]->m_N0 = new TNode();
            m_TVec[i]->m_N1 = new TNode();
            m_TVec[i]->m_N2 = new TNode();

            // Insert Data
            m_TVec[i]->m_N0->m_Pnt = tri[0];
            m_TVec[i]->m_N1->m_Pnt = tri[1];
            m_TVec[i]->m_N2->m_Pnt = tri[2];
            m_TVec[i]->m_Norm = tri[3];
            tri.clear();
        }
    }
}

void TMesh::LoadGeomAttributes( Geom* geomPtr )
{
    /*color       = geomPtr->getColor();
    materialID    = geomPtr->getMaterialID();*/
    m_PtrID        = geomPtr->GetID();

    m_MassPrior     = geomPtr->m_MassPrior();
    m_Density       = geomPtr->m_Density();
    m_ShellMassArea = geomPtr->m_MassArea();
    m_ShellFlag     = geomPtr->m_ShellFlag();

    //==== Check for Alternate Output Name ====//
    m_NameStr   = geomPtr->GetName();

    geomPtr->LoadDragFactors( m_DragFactors );

}

int TMesh::RemoveDegenerate()
{
    vector< TTri* > degTVec;

    int num_deleted = 0;
    for ( int t = 0 ; t < ( int )m_TVec.size() ; t++ )
    {
        TTri* tri = m_TVec[t];
        int addTriFlag = 1;

        if ( dist( tri->m_N0->m_Pnt, tri->m_N1->m_Pnt ) < 0.000001 )
        {
            addTriFlag = 0;
        }
        else if ( dist( tri->m_N0->m_Pnt, tri->m_N2->m_Pnt ) < 0.000001 )
        {
            addTriFlag = 0;
        }
        else if ( dist( tri->m_N1->m_Pnt, tri->m_N2->m_Pnt ) < 0.000001 )
        {
            addTriFlag = 0;
        }

        if ( addTriFlag )
        {
            degTVec.push_back( tri );
        }
        else
        {
            num_deleted++;
            delete tri;
        }
    }

    m_TVec = degTVec;

    return num_deleted;

}

void TMesh::CheckIfClosed()
{
    int n, s, t;

    MatchNodes();

    //==== Clear Refs to Tris ====//
    for ( n = 0 ; n < ( int )m_NVec.size() ; n++ )
    {
        m_NVec[n]->m_TriVec.clear();
    }

    //==== Check If All Tris Have 3 Edges ====//
    for ( t = 0 ; t < ( int )m_TVec.size() ; t++ )
    {
        m_TVec[t]->m_N0->m_TriVec.push_back( m_TVec[t] );
        m_TVec[t]->m_N1->m_TriVec.push_back( m_TVec[t] );
        m_TVec[t]->m_N2->m_TriVec.push_back( m_TVec[t] );
        m_TVec[t]->m_E0 = 0;
        m_TVec[t]->m_E1 = 0;
        m_TVec[t]->m_E2 = 0;
    }

    //==== Create Edges For Adjacent Tris ====//
    for ( n = 0 ; n < ( int )m_NVec.size() ; n++ )
    {
        TNode* n0 = m_NVec[n];
        for ( t = 0 ; t < ( int )n0->m_TriVec.size() ; t++ )
        {
            for ( s = t + 1 ; s < ( int )n0->m_TriVec.size() ; s++ )
            {
                FindEdge( n0, n0->m_TriVec[t], n0->m_TriVec[s] );
            }
        }
    }

    //==== Check If All Tris Have 3 Edges ====//
    m_NonClosedTriVec.clear();
    vector< TTri* > ivTriVec;
    for ( t = 0 ; t < ( int )m_TVec.size() ; t++ )
    {
        if ( !m_TVec[t]->m_E0 || !m_TVec[t]->m_E1 || !m_TVec[t]->m_E2 )
        {
            m_TVec[t]->m_InvalidFlag = 1;
            m_NonClosedTriVec.push_back( m_TVec[t] );
        }
    }
}

void TMesh::MergeNonClosed( TMesh* tm )
{
    if ( m_DeleteMeFlag || tm->m_DeleteMeFlag )     // Already Deleted
    {
        return;
    }

    if ( m_NonClosedTriVec.size() == 0 )            // No Open Tris
    {
        return;
    }

    //if ( nonClosedTriVec.size() != tm->nonClosedTriVec.size() )   // Num of Open Tris does not match
    //  return;

    bool match_flag;
    for ( int i = 0 ; i < ( int )m_NonClosedTriVec.size() ; i++ )
    {
        TTri* t0 = m_NonClosedTriVec[i];
        match_flag = false;
        for ( int j = 0 ; j < ( int )tm->m_NonClosedTriVec.size() ; j++ )
        {
            TTri* t1 = tm->m_NonClosedTriVec[j];
            if ( t0->ShareEdge( t1 ) )
            {
                match_flag = true;
                break;
            }
        }
        if ( match_flag == false )
        {
            break;
        }
    }

    if ( match_flag )
    {
        for ( int t = 0 ; t < ( int )tm->m_TVec.size() ; t++ )
        {
            TTri* tri = tm->m_TVec[t];
            AddTri( tri->m_N0->m_Pnt, tri->m_N1->m_Pnt, tri->m_N2->m_Pnt, tri->m_Norm );
        }
        for ( int i = 0 ; i < ( int )m_NonClosedTriVec.size() ; i++ )
        {
            m_NonClosedTriVec[i]->m_InvalidFlag = 0;
        }
        m_NonClosedTriVec.clear();

        CheckIfClosed();                // Recheck For NonClosed Tris

        tm->m_DeleteMeFlag = true;
    }
}

void TMesh::Intersect( TMesh* tm )
{
    m_TBox.Intersect( &tm->m_TBox );
}

void TMesh::Split( int meshFlag )
{
    int t;
    for ( t = 0 ; t < ( int )m_TVec.size() ; t++ )
    {
        m_TVec[t]->SplitTri( meshFlag );
    }
}


void TMesh::DeterIntExt( vector< TMesh* >& meshVec )
{
    for ( int t = 0 ; t < ( int )m_TVec.size() ; t++ )
    {
        TTri* tri = m_TVec[t];

        //==== Do Interior Tris ====//
        if ( tri->m_SplitVec.size() )
        {
            tri->m_InteriorFlag = 1;
            for ( int s = 0 ; s < ( int )tri->m_SplitVec.size() ; s++ )
            {
                DeterIntExtTri( tri->m_SplitVec[s], meshVec );
            }
        }
        else
        {
            DeterIntExtTri( tri, meshVec );
        }
    }
}

void TMesh::DeterIntExtTri( TTri* tri, vector< TMesh* >& meshVec )
{
    vec3d orig = ( tri->m_N0->m_Pnt + tri->m_N1->m_Pnt ) * 0.5;
    orig = ( orig + tri->m_N2->m_Pnt ) * 0.5;
    tri->m_InteriorFlag = 0;

    for ( int m = 0 ; m < ( int )meshVec.size() ; m++ )
    {
        if ( meshVec[m] != this )
        {
            vector<double > tParmVec;
            meshVec[m]->m_TBox.NumCrossXRay( orig, tParmVec );
            if ( tParmVec.size() % 2 )
            {
                tri->m_InteriorFlag = 1;
                return ;
            }
        }
    }
}

int TMesh::DeterIntExtPnt( const vec3d& pnt, vector< TMesh* >& meshVec, TMesh* ignoreMesh ) // 1 Interior 0 Exterior
{
    return 0;
}

void TMesh::MassDeterIntExt( vector< TMesh* >& meshVec )
{
    for ( int t = 0 ; t < ( int )m_TVec.size() ; t++ )
    {
        TTri* tri = m_TVec[t];

        //==== Do Interior Tris ====//
        if ( tri->m_SplitVec.size() )
        {
            tri->m_InteriorFlag = 1;
            for ( int s = 0 ; s < ( int )tri->m_SplitVec.size() ; s++ )
            {
                MassDeterIntExtTri( tri->m_SplitVec[s], meshVec );
            }
        }
        else
        {
            MassDeterIntExtTri( tri, meshVec );
        }
    }
}


void TMesh::MassDeterIntExtTri( TTri* tri, vector< TMesh* >& meshVec )
{
    vec3d orig = ( tri->m_N0->m_Pnt + tri->m_N1->m_Pnt ) * 0.5;
    orig = ( orig + tri->m_N2->m_Pnt ) * 0.5;
    tri->m_InteriorFlag = 1;
    int prior = -1;

    for ( int m = 0 ; m < ( int )meshVec.size() ; m++ )
    {
        if ( meshVec[m] != this )
        {
            vector<double > tParmVec;
            meshVec[m]->m_TBox.NumCrossXRay( orig, tParmVec );
            if ( tParmVec.size() % 2 )
            {
                if ( meshVec[m]->m_MassPrior > prior )
                {
                    tri->m_InteriorFlag = 0;
                    tri->m_ID = meshVec[m]->m_PtrID;
                    tri->m_Mass = meshVec[m]->m_Density;
                    prior = meshVec[m]->m_MassPrior;
                }
            }
        }
    }
}

double TMesh::ComputeTheoArea()
{
    m_TheoArea = 0;
    for ( int t = 0 ; t < ( int )m_TVec.size() ; t++ )
    {
        m_TheoArea += m_TVec[t]->ComputeArea();
    }
    return m_TheoArea;
}

double TMesh::ComputeWetArea()
{
    m_WetArea = 0;

    for ( int t = 0 ; t < ( int )m_TVec.size() ; t++ )
    {
        TTri* tri = m_TVec[t];

        //==== Do Interior Tris ====//
        if ( tri->m_SplitVec.size() )
        {
            for ( int s = 0 ; s < ( int )tri->m_SplitVec.size() ; s++ )
            {
                if ( !tri->m_SplitVec[s]->m_InteriorFlag )
                {
                    m_WetArea += tri->m_SplitVec[s]->ComputeArea();
                }
            }
        }
        else if ( !tri->m_InteriorFlag )
        {
            m_WetArea += tri->ComputeArea();
        }
    }
    return m_WetArea;
}

double TMesh::ComputeAwaveArea()
{
    m_WetArea = 0;

    for ( int t = 0 ; t < ( int )m_TVec.size() ; t++ )
    {
        TTri* tri = m_TVec[t];

        //==== Do Interior Tris ====//
        if ( tri->m_SplitVec.size() )
        {
            for ( int s = 0 ; s < ( int )tri->m_SplitVec.size() ; s++ )
            {
                if ( !tri->m_SplitVec[s]->m_InteriorFlag )
                {
                    m_WetArea += tri->m_SplitVec[s]->ComputeAwArea();
                }
            }
        }
        else if ( !tri->m_InteriorFlag )
        {
            m_WetArea += tri->ComputeAwArea();
        }
    }
    return m_WetArea;
}

double TMesh::ComputeTheoVol()
{
    m_TheoVol = 0.0;
    for ( int t = 0 ; t < ( int )m_TVec.size() ; t++ )
    {
        TTri* tri = m_TVec[t];

        m_TheoVol += tetra_volume( tri->m_N0->m_Pnt, tri->m_N1->m_Pnt, tri->m_N2->m_Pnt );
    }

    return m_TheoVol;

}
double TMesh::ComputeTrimVol()
{
    double trimVol = 0;

    for ( int t = 0 ; t < ( int )m_TVec.size() ; t++ )
    {
        TTri* tri = m_TVec[t];

        //==== Do Interior Tris ====//
        if ( tri->m_SplitVec.size() )
        {
            for ( int s = 0 ; s < ( int )tri->m_SplitVec.size() ; s++ )
            {
                if ( !tri->m_SplitVec[s]->m_InteriorFlag )
                {
                    trimVol += tetra_volume( tri->m_SplitVec[s]->m_N0->m_Pnt, tri->m_SplitVec[s]->m_N1->m_Pnt, tri->m_SplitVec[s]->m_N2->m_Pnt );
                }
            }
        }
        else if ( !tri->m_InteriorFlag )
        {
            trimVol += tetra_volume( tri->m_N0->m_Pnt, tri->m_N1->m_Pnt, tri->m_N2->m_Pnt );
        }
    }
    return trimVol;
}

void TMesh::AddTri( const vec3d & v0, const vec3d & v1, const vec3d & v2, const vec3d & norm )
{
    // Use For XYZ Tri
    TTri* ttri = new TTri();
    ttri->m_Norm = norm;
    ttri->SetTMeshPtr( this );

    ttri->m_N0 = new TNode();
    ttri->m_N1 = new TNode();
    ttri->m_N2 = new TNode();

    ttri->m_N0->m_Pnt = v0;
    ttri->m_N1->m_Pnt = v1;
    ttri->m_N2->m_Pnt = v2;

    ttri->m_N0->SetCoordInfo( TNode::HAS_XYZ );
    ttri->m_N1->SetCoordInfo( TNode::HAS_XYZ );
    ttri->m_N1->SetCoordInfo( TNode::HAS_XYZ );

    m_TVec.push_back( ttri );
    m_NVec.push_back( ttri->m_N0 );
    m_NVec.push_back( ttri->m_N1 );
    m_NVec.push_back( ttri->m_N2 );
}

void TMesh::AddTri( TNode* node0, TNode* node1, TNode* node2, const vec3d & norm )
{
    TTri* ttri = new TTri();
    ttri->m_Norm = norm;
    ttri->SetTMeshPtr( this );

    ttri->m_N0 = new TNode();
    ttri->m_N1 = new TNode();
    ttri->m_N2 = new TNode();

    ttri->m_N0->m_Pnt = node0->m_Pnt;
    ttri->m_N1->m_Pnt = node1->m_Pnt;
    ttri->m_N2->m_Pnt = node2->m_Pnt;
    ttri->m_N0->m_UWPnt = node0->m_UWPnt;
    ttri->m_N1->m_UWPnt = node1->m_UWPnt;
    ttri->m_N2->m_UWPnt = node2->m_UWPnt;
    ttri->m_N0->SetXYZFlag( node0->GetXYZFlag() );
    ttri->m_N1->SetXYZFlag( node1->GetXYZFlag() );
    ttri->m_N2->SetXYZFlag( node2->GetXYZFlag() );
    ttri->m_N0->SetCoordInfo( node0->GetCoordInfo() );
    ttri->m_N1->SetCoordInfo( node1->GetCoordInfo() );
    ttri->m_N2->SetCoordInfo( node2->GetCoordInfo() );

    ttri->m_N0->m_IsectFlag = node0->m_IsectFlag;
    ttri->m_N1->m_IsectFlag = node1->m_IsectFlag;
    ttri->m_N2->m_IsectFlag = node2->m_IsectFlag;

    m_TVec.push_back( ttri );
    m_NVec.push_back( ttri->m_N0 );
    m_NVec.push_back( ttri->m_N1 );
    m_NVec.push_back( ttri->m_N2 );



}

void TMesh::AddTri( const vec3d & v0, const vec3d & v1, const vec3d & v2, const vec3d & norm, const vec3d & uw0,
                    const vec3d & uw1, const vec3d & uw2 )
{
    // AddTri with both xyz and uw info
    AddTri( v0, v1, v2, norm );
    TTri* tri = m_TVec.back();
    tri->m_N0->m_UWPnt = uw0;
    tri->m_N1->m_UWPnt = uw1;
    tri->m_N2->m_UWPnt = uw2;

    tri->m_N0->SetCoordInfo( TNode::HAS_XYZ | TNode::HAS_UW );
    tri->m_N1->SetCoordInfo( TNode::HAS_XYZ | TNode::HAS_UW );
    tri->m_N2->SetCoordInfo( TNode::HAS_XYZ | TNode::HAS_UW );
}

void TMesh::AddUWTri( const vec3d & uw0, const vec3d & uw1, const vec3d & uw2, const vec3d & norm )
{
    // Use For XYZ Tri
    TTri* ttri = new TTri();
    ttri->m_Norm = norm;
    ttri->SetTMeshPtr( this );

    ttri->m_N0 = new TNode();
    ttri->m_N1 = new TNode();
    ttri->m_N2 = new TNode();

    ttri->m_N0->m_Pnt = uw0;
    ttri->m_N1->m_Pnt = uw1;
    ttri->m_N2->m_Pnt = uw2;

    ttri->m_N0->SetCoordInfo( TNode::HAS_UW );
    ttri->m_N1->SetCoordInfo( TNode::HAS_UW );
    ttri->m_N1->SetCoordInfo( TNode::HAS_UW );
    ttri->m_N0->SetXYZFlag( false );
    ttri->m_N1->SetXYZFlag( false );
    ttri->m_N2->SetXYZFlag( false );

    m_TVec.push_back( ttri );
    m_NVec.push_back( ttri->m_N0 );
    m_NVec.push_back( ttri->m_N1 );
    m_NVec.push_back( ttri->m_N2 );
}

void TMesh::LoadBndBox()
{
    for ( int i = 0 ; i < ( int )m_TVec.size() ; i++ )
    {
        m_TBox.AddTri( m_TVec[i] );
    }

    m_TBox.SplitBox();
}

//==== Write STL Tris =====//
void TMesh::WriteSTLTris( FILE* file_id, Matrix4d XFormMat )
{
    int t, s;
    vec3d norm;
    vec3d v0, v1, v2;
    vec3d d30, d21;

    for ( t = 0 ; t < ( int )m_TVec.size() ; t++ )
    {
        TTri* tri = m_TVec[t];

        if ( tri->m_SplitVec.size() )
        {
            for ( s = 0 ; s < ( int )tri->m_SplitVec.size() ; s++ )
            {
                if ( !tri->m_SplitVec[s]->m_InteriorFlag )
                {
                    v0 = XFormMat.xform( tri->m_SplitVec[s]->m_N0->m_Pnt );
                    v1 = XFormMat.xform( tri->m_SplitVec[s]->m_N1->m_Pnt );
                    v2 = XFormMat.xform( tri->m_SplitVec[s]->m_N2->m_Pnt );

                    d21 = v2 - v1;

                    if ( d21.mag() > 0.000001 )
                    {
                        norm = cross( d21, v0 - v1 );
                        norm.normalize();

                        fprintf( file_id, " facet normal  %2.10le %2.10le %2.10le\n",  norm.x(), norm.y(), norm.z() );
                        fprintf( file_id, "   outer loop\n" );
                        fprintf( file_id, "     vertex %2.10le %2.10le %2.10le\n", v0.x(), v0.y(), v0.z() );
                        fprintf( file_id, "     vertex %2.10le %2.10le %2.10le\n", v1.x(), v1.y(), v1.z() );
                        fprintf( file_id, "     vertex %2.10le %2.10le %2.10le\n", v2.x(), v2.y(), v2.z() );
                        fprintf( file_id, "   endloop\n" );
                        fprintf( file_id, " endfacet\n" );
                    }
                }
            }
        }
        else
        {
            if ( !tri->m_InteriorFlag )
            {
                v0 = XFormMat.xform( tri->m_N0->m_Pnt );
                v1 = XFormMat.xform( tri->m_N1->m_Pnt );
                v2 = XFormMat.xform( tri->m_N2->m_Pnt );

                d21 = v2 - v1;

                if ( d21.mag() > 0.000001 )
                {
                    norm = cross( d21, v0 - v1 );
                    norm.normalize();

                    fprintf( file_id, " facet normal  %2.10le %2.10le %2.10le\n",  norm.x(), norm.y(), norm.z() );
                    fprintf( file_id, "   outer loop\n" );
                    fprintf( file_id, "     vertex %2.10le %2.10le %2.10le\n", v0.x(), v0.y(), v0.z() );
                    fprintf( file_id, "     vertex %2.10le %2.10le %2.10le\n", v1.x(), v1.y(), v1.z() );
                    fprintf( file_id, "     vertex %2.10le %2.10le %2.10le\n", v2.x(), v2.y(), v2.z() );
                    fprintf( file_id, "   endloop\n" );
                    fprintf( file_id, " endfacet\n" );
                }
            }
        }
    }
}

vec3d TMesh::GetVertex( int index )
{
    if ( index < 0 )
    {
        index = 0;
    }
    if ( index >= ( int )m_VertVec.size() )
    {
        index = ( int )m_VertVec.size() - 1;
    }

    return m_VertVec[index];
}

int TMesh::NumVerts()
{
    return m_VertVec.size();
}


//===============================================//
//===============================================//
//===============================================//
//===============================================//
//                  TTri
//===============================================//
//===============================================//
//===============================================//
//===============================================//

TTri::TTri()
{
    static int cnt = 0;
    cnt++;
//printf("Tri Construct Cnt = %d \n", cnt);
    m_E0 = m_E1 = m_E2 = 0;
    m_N0 = m_N1 = m_N2 = 0;
    m_InteriorFlag = 0;
    m_InvalidFlag  = 0;
    m_Mass = 0.0;
    m_TMesh = NULL;
    m_PEArr[0] = m_PEArr[1] = m_PEArr[2] = NULL;
}

TTri::~TTri()
{
    static int cnt = 0;
    cnt++;
//printf("Tri Destruct Cnt = %d \n", cnt);
    int i;

    //==== Delete Edges ====//
    for ( i = 0 ; i < ( int )m_EVec.size() ; i++ )
    {
        delete m_EVec[i];
    }

    //==== Delete Perimeter Edges ====//
    for ( i = 0 ; i < 3 ; i++ )
    {
        delete m_PEArr[i];
    }

    //==== Delete Nodes - Not First 3 ====//
    for ( i = 3 ; i < ( int )m_NVec.size() ; i++ )
    {
        delete m_NVec[i];
    }

    //==== Delete Sub Tris ====//
    for ( i = 0 ; i < ( int )m_SplitVec.size() ; i++ )
    {
        delete m_SplitVec[i];
    }

    for ( i = 0 ; i < ( int )m_ISectEdgeVec.size() ; i++ )
    {
        delete m_ISectEdgeVec[i]->m_N0;
        delete m_ISectEdgeVec[i]->m_N1;
        delete m_ISectEdgeVec[i];
    }

}

void TTri::BuildPermEdges()
{
    if ( m_PEArr[0] )
    {
        for ( int i = 0 ; i < 3 ; i++ )
        {
            delete m_PEArr[i];
            m_PEArr[i] = NULL;
        }
    }
    if ( m_N0 != NULL && m_N1 != NULL && m_N2 != NULL )
    {
        m_PEArr[0] = new TEdge( m_N0, m_N1, this );
        m_PEArr[1] = new TEdge( m_N1, m_N2, this );
        m_PEArr[2] = new TEdge( m_N2, m_N0, this );
    }
}

vec3d TTri::CompNorm()
{
    vec3d p10 = m_N1->m_Pnt - m_N0->m_Pnt;
    vec3d p20 = m_N2->m_Pnt - m_N0->m_Pnt;
    vec3d cnorm = cross( p10, p20 );
    cnorm.normalize();
    m_Norm = cnorm;
    return cnorm;
}

vec3d TTri::CompPnt( const vec3d & uw_pnt )
{
    if ( m_TMesh )
    {
        return m_TMesh->CompPnt( uw_pnt );
    }

    return vec3d();
}

bool TTri::ShareEdge( TTri* t )
{
    double tol = 0.0000001;

    if ( MatchEdge( m_N0, m_N1, t->m_N0, t->m_N1, tol ) )
    {
        return true;
    }
    if ( MatchEdge( m_N0, m_N1, t->m_N1, t->m_N2, tol ) )
    {
        return true;
    }
    if ( MatchEdge( m_N0, m_N1, t->m_N0, t->m_N2, tol ) )
    {
        return true;
    }
    if ( MatchEdge( m_N1, m_N2, t->m_N0, t->m_N1, tol ) )
    {
        return true;
    }
    if ( MatchEdge( m_N1, m_N2, t->m_N1, t->m_N2, tol ) )
    {
        return true;
    }
    if ( MatchEdge( m_N1, m_N2, t->m_N0, t->m_N2, tol ) )
    {
        return true;
    }
    if ( MatchEdge( m_N0, m_N2, t->m_N0, t->m_N1, tol ) )
    {
        return true;
    }
    if ( MatchEdge( m_N0, m_N2, t->m_N1, t->m_N2, tol ) )
    {
        return true;
    }
    if ( MatchEdge( m_N0, m_N2, t->m_N0, t->m_N2, tol ) )
    {
        return true;
    }

    return false;
}

bool TTri::MatchEdge( TNode* n0, TNode* n1, TNode* nA, TNode* nB, double tol )
{
    if ( dist_squared( n0->m_Pnt, nA->m_Pnt ) < tol && dist_squared( n1->m_Pnt, nB->m_Pnt ) < tol )
    {
        n0->m_Pnt = ( n0->m_Pnt + nA->m_Pnt ) * 0.5;
        nA->m_Pnt = n0->m_Pnt;
        n1->m_Pnt = ( n1->m_Pnt + nB->m_Pnt ) * 0.5;
        nB->m_Pnt = n1->m_Pnt;
        return true;
    }
    if ( dist_squared( n0->m_Pnt, nB->m_Pnt ) < tol && dist_squared( n1->m_Pnt, nA->m_Pnt ) < tol )
    {
        n0->m_Pnt = ( n0->m_Pnt + nB->m_Pnt ) * 0.5;
        nB->m_Pnt = n0->m_Pnt;
        n1->m_Pnt = ( n1->m_Pnt + nA->m_Pnt ) * 0.5;
        nA->m_Pnt = n1->m_Pnt;
        return true;
    }

    return false;
}

//==== Load Nodes From Edges =====//
void TTri::LoadNodesFromEdges()
{
    if ( !m_E0 || !m_E1 || !m_E2 )
    {
        return;
    }

//jrg TODO: Preserve Normal Direction
    m_N0 = m_E0->m_N0;
    m_N1 = m_E0->m_N1;
    if ( m_E1->m_N0 != m_N0 && m_E1->m_N0 != m_N1 )
    {
        m_N2 = m_E1->m_N0;
    }
    else
    {
        m_N2 = m_E1->m_N1;
    }

    if ( m_N0 == m_N1 || m_N0 == m_N2 || m_N1 == m_N2 )
    {
        printf( "ERROR loadNodesFromEdges\n" );
    }
}


//==== Split A Triangle Along Edges in ISectEdges Vec =====//
void TTri::SplitTri( int meshFlag )
{
    int i, j;
    double onEdgeTol = 0.00001;
    double uvMinTol  = 0.001;
    double uvMaxTol  = 0.999;

    //==== No Need To Split ====//
    if ( m_ISectEdgeVec.size() == 0 )
    {
        return;
    }

    //==== Delete Duplicate Edges ====//
    vector< TEdge* > noDupVec;
    noDupVec.push_back( m_ISectEdgeVec[0] );
    for ( i = 1 ; i < ( int )m_ISectEdgeVec.size() ; i++ )
    {
        int dupFlag = 0;
        for ( j = 0 ; j < ( int )noDupVec.size() ; j++ )
        {
            if ( DupEdge( m_ISectEdgeVec[i], noDupVec[j], onEdgeTol ) )
            {
                dupFlag = 1;
            }
        }

        if ( !dupFlag )
        {
            noDupVec.push_back( m_ISectEdgeVec[i] );
        }
        else
        {
            delete m_ISectEdgeVec[i]->m_N0;
            delete m_ISectEdgeVec[i]->m_N1;
            delete m_ISectEdgeVec[i];
        }
    }
    m_ISectEdgeVec = noDupVec;

    //==== Add Corners of Triangle ====//           //jrg figure who should allocate data...
    m_NVec.push_back( m_N0 );
    m_NVec.push_back( m_N1 );
    m_NVec.push_back( m_N2 );

    //==== Add Edges For Perimeter ====//
    for ( i = 0 ; i < 3 ; i++ )
    {
        m_EVec.push_back(  new TEdge() );
    }

    m_EVec[0]->m_N0 = m_N0;
    m_EVec[0]->m_N1 = m_N1;
    m_EVec[1]->m_N0 = m_N1;
    m_EVec[1]->m_N1 = m_N2;
    m_EVec[2]->m_N0 = m_N2;
    m_EVec[2]->m_N1 = m_N0;

    //==== Load All Possible Nodes to Add ====//
    vector< vec3d > pVec;               // Pnts to be added
    vector< int > matchNodeIndex;       // Nodes Index Which Correspond to pVec
    for ( i = 0 ; i < ( int )m_ISectEdgeVec.size() ; i++ )
    {
        pVec.push_back( m_ISectEdgeVec[i]->m_N0->m_Pnt );
        pVec.push_back( m_ISectEdgeVec[i]->m_N1->m_Pnt );
    }
    matchNodeIndex.resize( pVec.size() );
    for ( i = 0 ; i < ( int )matchNodeIndex.size() ; i++ )
    {
        matchNodeIndex[i] = -1;
    }

    //==== Add Edge Nodes ====//
    for ( i = 0 ; i < ( int )pVec.size() ; i++ )
    {
        for ( j = 0 ; j < ( int )m_EVec.size() ; j++ )
        {
            if ( OnEdge( pVec[i], m_EVec[j], onEdgeTol ) )
            {
                //==== SplitEdge ====//
                TNode* sn = new TNode();        // New node
                sn->m_IsectFlag = 1;
                m_NVec.push_back( sn );
                matchNodeIndex[i] = m_NVec.size() - 1;
                sn->m_Pnt = pVec[i];
                TEdge* se = new TEdge();        // New Edge
                se->m_N0 = m_EVec[j]->m_N0;
                se->m_N1 = sn;
                m_EVec[j]->m_N0 = sn;               // Change Split Edge
                m_EVec.push_back( se );
                break;
            }
        }
    }

    //==== Add Other Points ====//
    for ( i = 0 ; i < ( int )pVec.size() ; i++ )
    {
        //==== Look For Existing Node ====//
        if ( matchNodeIndex[i] == -1 )
        {
            for ( j = 0 ; j < ( int )m_NVec.size() ; j++ )
            {
                double d = dist( pVec[i], m_NVec[j]->m_Pnt );
                if ( d < onEdgeTol )
                {
                    matchNodeIndex[i] = j;
                    break;
                }
            }
        }
        //==== Didnt Find One - Add New ====//
        if ( matchNodeIndex[i] == -1 )
        {
            TNode* sn = new TNode();        // New node
            sn->m_IsectFlag = 1;
            m_NVec.push_back( sn );
            matchNodeIndex[i] = m_NVec.size() - 1;
            sn->m_Pnt = pVec[i];
        }
    }

    //==== Add Edges ====//
    for ( i = 0 ; i < ( int )matchNodeIndex.size() ; i += 2 )
    {
        int ind0 = matchNodeIndex[i];
        int ind1 = matchNodeIndex[i + 1];

        if ( ind0 != ind1 )
        {
            int existFlag = 0;
            for ( j = 0 ; j < ( int )m_EVec.size() ; j++ )
            {
                if      ( m_EVec[j]->m_N0 == m_NVec[ind0] && m_EVec[j]->m_N1 == m_NVec[ind1] )
                {
                    existFlag = 1;
                }
                else if ( m_EVec[j]->m_N0 == m_NVec[ind1] && m_EVec[j]->m_N1 == m_NVec[ind0] )
                {
                    existFlag = 1;
                }
            }

            if ( !existFlag )
            {
                TEdge* se = new TEdge();        // New Edge
                se->m_N0 = m_NVec[ind0];
                se->m_N1 = m_NVec[ind1];
                m_EVec.push_back( se );
            }
        }
    }

    //==== Look For Crossing Edges ====//
    //==== jrg - this code is suspect....
    int findCross = 1;
    double u, v;
    vec3d np0, np1;
    while ( findCross )
    {
        findCross = 0;
        for ( i = 0 ; i < ( int )m_EVec.size() ; i++ )
        {
            TNode* en0 = m_EVec[i]->m_N0;
            TNode* en1 = m_EVec[i]->m_N1;
            for ( j = i + 1 ; j < ( int )m_EVec.size() ; j++ )
            {
                TNode* en2 = m_EVec[j]->m_N0;
                TNode* en3 = m_EVec[j]->m_N1;

                if ( en0 != en2 && en0 != en3 && en1 != en2 && en1 != en3 )
                {
                    double d = dist3D_Segment_to_Segment( en0->m_Pnt, en1->m_Pnt, en2->m_Pnt, en3->m_Pnt,
                                                          &u, &np0, &v, &np1 );

                    if ( d < onEdgeTol * onEdgeTol )
                    {
                        findCross = 1;

                        if      (  u < uvMinTol && v < uvMinTol )
                        {
                            findCross = 0;
                        }
                        else if (  u < uvMinTol && v > uvMaxTol )
                        {
                            findCross = 0;
                        }
                        else if (  u > uvMaxTol && v < uvMinTol )
                        {
                            findCross = 0;
                        }
                        else if (  u > uvMaxTol && v > uvMaxTol )
                        {
                            findCross = 0;
                        }

                        else if ( u < uvMinTol && v >= uvMinTol && v <= uvMaxTol )
                        {
                            //==== Break Along Line Seg 2 ====//
                            TEdge* se = new TEdge();        // New Edge
                            se->m_N0 = en0;
                            se->m_N1 = en3;
                            m_EVec[j]->m_N0 = en2;
                            m_EVec[j]->m_N1 = en0;
                            m_EVec.push_back( se );
                        }
                        else if ( u > uvMaxTol && v >= uvMinTol && v <= uvMaxTol )
                        {
                            //==== Break Along Line Seg 2 ====//
                            TEdge* se = new TEdge();        // New Edge
                            se->m_N0 = en1;
                            se->m_N1 = en3;
                            m_EVec[j]->m_N0 = en2;
                            m_EVec[j]->m_N1 = en1;
                            m_EVec.push_back( se );
                        }
                        else if ( v < uvMinTol && u >= uvMinTol && u <= uvMaxTol )
                        {
                            //==== Break Along Line Seg 1 ====//
                            TEdge* se = new TEdge();        // New Edge
                            se->m_N0 = en2;
                            se->m_N1 = en1;
                            m_EVec[i]->m_N0 = en2;
                            m_EVec[i]->m_N1 = en0;
                            m_EVec.push_back( se );
                        }
                        else if ( v > uvMaxTol && u >= uvMinTol && u <= uvMaxTol )
                        {
                            //==== Break Along Line Seg 1 ====//
                            TEdge* se = new TEdge();        // New Edge
                            se->m_N0 = en3;
                            se->m_N1 = en1;
                            m_EVec[i]->m_N0 = en3;
                            m_EVec[i]->m_N1 = en0;
                            m_EVec.push_back( se );
                        }
                        else
                        {
                            //==== New Node at Crossing Point ====//
                            TNode* sn = new TNode();
                            sn->m_IsectFlag = 1;
                            m_NVec.push_back( sn );
                            sn->m_Pnt = ( np0 + np1 ) * 0.5;
                            TEdge* se0 = new TEdge();       // New Edge
                            se0->m_N0 = en0;
                            se0->m_N1 = sn;
                            m_EVec[i]->m_N0 = sn;
                            m_EVec[i]->m_N1 = en1;
                            TEdge* se1 = new TEdge();       // New Edge
                            se1->m_N0 = en2;
                            se1->m_N1 = sn;
                            m_EVec[j]->m_N0 = sn;
                            m_EVec[j]->m_N1 = en3;

                            m_EVec.push_back( se1 );
                            m_EVec.push_back( se0 );
                        }

                        if ( findCross )
                        {
                            j = m_EVec.size();
                        }
                    }
                }
                if ( findCross )
                {
                    i = m_EVec.size();
                }
            }
        }
    }

    //==== Determine Which Axis to Flatten ====//
    vec3d zn0, zn1, zn2;
    zn0 = m_N0->m_Pnt;
    zn0.set_x( 0 );
    zn1 = m_N1->m_Pnt;
    zn1.set_x( 0 );
    zn2 = m_N2->m_Pnt;
    zn2.set_x( 0 );
//  double dx = MIN( dist( zn0, zn1 ), MIN( dist( zn1, zn2 ), dist( zn2, zn0 ) ) );
    double dx = MAX( dist( zn0, zn1 ), MAX( dist( zn1, zn2 ), dist( zn2, zn0 ) ) );

    zn0 = m_N0->m_Pnt;
    zn0.set_y( 0 );
    zn1 = m_N1->m_Pnt;
    zn1.set_y( 0 );
    zn2 = m_N2->m_Pnt;
    zn2.set_y( 0 );
//  double dy = MIN( dist( zn0, zn1 ), MIN( dist( zn1, zn2 ), dist( zn2, zn0 ) ) );
    double dy = MAX( dist( zn0, zn1 ), MAX( dist( zn1, zn2 ), dist( zn2, zn0 ) ) );

    zn0 = m_N0->m_Pnt;
    zn0.set_z( 0 );
    zn1 = m_N1->m_Pnt;
    zn1.set_z( 0 );
    zn2 = m_N2->m_Pnt;
    zn2.set_z( 0 );
//  double dz = MIN( dist( zn0, zn1 ), MIN( dist( zn1, zn2 ), dist( zn2, zn0 ) ) );
    double dz = MAX( dist( zn0, zn1 ), MAX( dist( zn1, zn2 ), dist( zn2, zn0 ) ) );

    int flattenAxis = 0;
    if ( dy >= dx && dy > dz )
    {
        flattenAxis = 1;
    }
    else if ( dz >= dx && dz > dy )
    {
        flattenAxis = 2;
    }

    //==== Use Triangle to Split Tri ====//
    if ( meshFlag )
    {
        NiceTriSplit( flattenAxis );
    }
    else
    {
        TriangulateSplit( flattenAxis );
    }

}

void TTri::TriangulateSplit( int flattenAxis )
{
    int i, j;

    //==== Dump Into Triangle ====//
    struct triangulateio in;
    struct triangulateio out;

    memset( &in, 0, sizeof( in ) ); // Load Zeros
    memset( &out, 0, sizeof( out ) );

    //==== PreAllocate Data For In/Out ====//
    in.pointlist    = ( REAL * ) malloc( m_NVec.size() * 2 * sizeof( REAL ) );
    out.pointlist   = NULL;

    in.segmentlist  = ( int * ) malloc( m_EVec.size() * 2 * sizeof( int ) );
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

    //==== Load Points into Traingle Struct ====//
    in.numberofpoints = m_NVec.size();


    //==== Find Bounds of NVec ====//
    BndBox box;
    for ( j = 0 ; j < ( int )m_NVec.size() ; j++ )
    {
        box.Update( m_NVec[j]->m_Pnt );
    }

    vec3d center = box.GetCenter();

    double min_s = 0.0001;
    double sx = max( box.GetMax( 0 ) - box.GetMin( 0 ), min_s );
    double sy = max( box.GetMax( 1 ) - box.GetMin( 1 ), min_s );
    double sz = max( box.GetMax( 2 ) - box.GetMin( 2 ), min_s );

    int cnt = 0;
    for ( j = 0 ; j < ( int )m_NVec.size() ; j++ )
    {
        vec3d pnt = m_NVec[j]->m_Pnt - center;
        pnt.scale_x( 1.0 / sx );
        pnt.scale_y( 1.0 / sy );
        pnt.scale_z( 1.0 / sz );

        if ( flattenAxis == 0 )
        {
            in.pointlist[cnt] = pnt.y();
            cnt++;
            in.pointlist[cnt] = pnt.z();
            cnt++;
        }
        else if ( flattenAxis == 1 )
        {
            in.pointlist[cnt] = pnt.x();
            cnt++;
            in.pointlist[cnt] = pnt.z();
            cnt++;
        }
        else if ( flattenAxis == 2 )
        {
            in.pointlist[cnt] = pnt.x();
            cnt++;
            in.pointlist[cnt] = pnt.y();
            cnt++;
        }
    }
//static FILE* fp = fopen( "debug.txt", "w" );
//
//static int static_cnt = 0;
//fprintf(fp, "Triangle Cnt = %d\n", static_cnt++ );
//if ( static_cnt == 843 )
//{
//  for ( i = 0 ; i < in.numberofpoints ; i++ )
//  {
//      fprintf(fp, "%f %f \n", in.pointlist[i*2],  in.pointlist[i*2 + 1] );
//  }
//
//  for ( j = 0 ; j < (int)nVec.size() ; j++ )
//  {
//      fprintf(fp, "%f %f %f \n", nVec[j]->pnt.x(), nVec[j]->pnt.y(), nVec[j]->pnt.z() );
//  }
//
//}

    //==== Match Edge Nodes to Indices in NVec ====//
    vector< int > segIndList;
    for ( i = 0 ; i < ( int )m_EVec.size() ; i++ )
    {
        for ( j = 0 ; j < ( int )m_NVec.size() ; j++ )
        {
            if ( m_EVec[i]->m_N0 == m_NVec[j] )
            {
                segIndList.push_back( j );
                break;
            }
        }
        for ( j = 0 ; j < ( int )m_NVec.size() ; j++ )
        {
            if ( m_EVec[i]->m_N1 == m_NVec[j] )
            {
                segIndList.push_back( j );
                break;
            }
        }
    }
    cnt = 0;
    for ( j = 0 ; j < ( int )segIndList.size() ; j += 2 )
    {
        in.segmentlist[cnt] = segIndList[j];
        cnt++;
        in.segmentlist[cnt] = segIndList[j + 1];
        cnt++;
    }

    in.numberofsegments = segIndList.size() / 2;

    if ( in.numberofpoints > 3 && in.numberofsegments > 3 )
    {
        //==== Check For Duplicate Points =====//
        int dupFlag = 0;
        for ( i = 0 ; i < in.numberofpoints ; i++ )
            for ( j = i + 1 ; j < in.numberofpoints ; j++ )
            {
                double del = fabs( in.pointlist[i * 2] - in.pointlist[j * 2] ) +
                             fabs( in.pointlist[i * 2 + 1] - in.pointlist[j * 2 + 1] );

                if ( del < 0.0000001 )
                {
                    dupFlag = 1;
                }
            }

        if ( !dupFlag )
        {
            //==== Constrained Delaunay Trianglulation ====//
            triangulate ( "zpQ", &in, &out, ( struct triangulateio * ) NULL );
        }
//fprintf(fp, "Triangulate in = %d out = %d \n", in.numberofpoints, out.numberofpoints );
    }


    //==== Load Triangles if No New Point Created ====//
    cnt = 0;
    for ( i = 0 ; i < out.numberoftriangles ; i++ )
    {
        if ( out.trianglelist[cnt]   < ( int )m_NVec.size() &&
                out.trianglelist[cnt + 1] < ( int )m_NVec.size() &&
                out.trianglelist[cnt + 2] < ( int )m_NVec.size() )
        {
            TTri* t = new TTri();
            t->m_N0 = m_NVec[out.trianglelist[cnt]];
            t->m_N1 = m_NVec[out.trianglelist[cnt + 1]];
            t->m_N2 = m_NVec[out.trianglelist[cnt + 2]];
            m_SplitVec.push_back( t );
        }
        else
        {
            /* jrg problem - look into...
                        printf("\n");
                        printf("m\n");
                        printf("c red\n");
                        printf("%12.8f, %12.8f, %12.8f \n", n0->pnt.x(), n0->pnt.y(), n0->pnt.z());
                        printf("%12.8f, %12.8f, %12.8f \n", n1->pnt.x(), n1->pnt.y(), n1->pnt.z());
                        printf("%12.8f, %12.8f, %12.8f \n", n2->pnt.x(), n2->pnt.y(), n2->pnt.z());
                        printf("c blue\n");
                        for ( j = 0 ; j < iSectEdgeVec.size() ; j++ )
                        {
                            printf("m\n");
                            printf("%12.8f, %12.8f, %12.8f \n", iSectEdgeVec[j]->n0->pnt.x(), iSectEdgeVec[j]->n0->pnt.y(), iSectEdgeVec[j]->n0->pnt.z());
                            printf("%12.8f, %12.8f, %12.8f \n", iSectEdgeVec[j]->n0->pnt.x(), iSectEdgeVec[j]->n1->pnt.y(), iSectEdgeVec[j]->n1->pnt.z());
                        }
            */

        }
        cnt += 3;
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

    //=== Orient Tris to Match Normal ====//
    for ( i = 0 ; i < ( int )m_SplitVec.size() ; i++ )
    {
        TTri* t = m_SplitVec[i];
        vec3d d01 = t->m_N0->m_Pnt - t->m_N1->m_Pnt;
        vec3d d21 = t->m_N2->m_Pnt - t->m_N1->m_Pnt;

        vec3d cx = cross( d21, d01 );
        cx.normalize();
        m_Norm.normalize();

        double del = dist_squared( cx, m_Norm );
        if ( del > 1.0 )
        {
            TNode* tmp = t->m_N1;
            t->m_N1 = t->m_N2;
            t->m_N2 = tmp;
        }
    }
}

void TTri::NiceTriSplit( int flattenAxis )
{
    int i, j;

    //==== Dump Into Triangle ====//
    struct triangulateio in;
    struct triangulateio out;

    memset( &in, 0, sizeof( in ) ); // Load Zeros
    memset( &out, 0, sizeof( out ) );

    //==== PreAllocate Data For In/Out ====//
    in.pointlist    = ( REAL * ) malloc( m_NVec.size() * 2 * sizeof( REAL ) );
    out.pointlist   = NULL;

    in.segmentlist  = ( int * ) malloc( m_EVec.size() * 2 * sizeof( int ) );
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

    //==== Load Points into Traingle Struct ====//
    in.numberofpoints = m_NVec.size();

    //==== Find Bounds of NVec ====//
    BndBox box;
    for ( j = 0 ; j < ( int )m_NVec.size() ; j++ )
    {
        box.Update( m_NVec[j]->m_Pnt );
    }

    vec3d center = box.GetCenter();

    double min_s = 0.0001;
    double sx = max( box.GetMax( 0 ) - box.GetMin( 0 ), min_s );
    double sy = max( box.GetMax( 1 ) - box.GetMin( 1 ), min_s );
    double sz = max( box.GetMax( 2 ) - box.GetMin( 2 ), min_s );

    int cnt = 0;
    for ( j = 0 ; j < ( int )m_NVec.size() ; j++ )
    {
        //==== Center and Rescale Triangle For Better Mapping ====//
        vec3d pnt = m_NVec[j]->m_Pnt - center;
        pnt.scale_x( 1.0 / sx );
        pnt.scale_y( 1.0 / sy );
        pnt.scale_z( 1.0 / sz );

        if ( flattenAxis == 0 )
        {
            in.pointlist[cnt] = m_NVec[j]->m_Pnt.y();
            cnt++;
            in.pointlist[cnt] = m_NVec[j]->m_Pnt.z();
            cnt++;
        }
        else if ( flattenAxis == 1 )
        {
            in.pointlist[cnt] = m_NVec[j]->m_Pnt.x();
            cnt++;
            in.pointlist[cnt] = m_NVec[j]->m_Pnt.z();
            cnt++;
        }
        else if ( flattenAxis == 2 )
        {
            in.pointlist[cnt] = m_NVec[j]->m_Pnt.x();
            cnt++;
            in.pointlist[cnt] = m_NVec[j]->m_Pnt.y();
            cnt++;
        }
    }

    //==== Match Edge Nodes to Indices in NVec ====//
    vector< int > segIndList;
    for ( i = 0 ; i < ( int )m_EVec.size() ; i++ )
    {
        for ( j = 0 ; j < ( int )m_NVec.size() ; j++ )
        {
            if ( m_EVec[i]->m_N0 == m_NVec[j] )
            {
                segIndList.push_back( j );
                break;
            }
        }
        for ( j = 0 ; j < ( int )m_NVec.size() ; j++ )
        {
            if ( m_EVec[i]->m_N1 == m_NVec[j] )
            {
                segIndList.push_back( j );
                break;
            }
        }
    }
    cnt = 0;
    for ( j = 0 ; j < ( int )segIndList.size() ; j += 2 )
    {
        in.segmentlist[cnt] = segIndList[j];
        cnt++;
        in.segmentlist[cnt] = segIndList[j + 1];
        cnt++;
    }

    in.numberofsegments = segIndList.size() / 2;

    if ( in.numberofpoints > 3 && in.numberofsegments > 3 )
    {
        //==== Check For Duplicate Points =====//
        int dupFlag = 0;
        for ( i = 0 ; i < in.numberofpoints ; i++ )
            for ( j = i + 1 ; j < in.numberofpoints ; j++ )
            {
                double del = fabs( in.pointlist[i * 2] - in.pointlist[j * 2] ) +
                             fabs( in.pointlist[i * 2 + 1] - in.pointlist[j * 2 + 1] );

                if ( del < 0.0000001 )
                {
                    dupFlag = 1;
                }
            }

        if ( !dupFlag )
        {
            //==== Constrained Delaunay Trianglulation ====//
//          triangulate ("zpq20QjS5Y", &in, &out, (struct triangulateio *) NULL);
            triangulate ( "zpQ", &in, &out, ( struct triangulateio * ) NULL );
        }
    }

//printf("Triangulate in = %d out = %d \n", in.numberofpoints, out.numberofpoints );

    //==== Map Points to Tri ====//
//printf("  n0 = %f %f %f \n", n0->pnt.x(), n0->pnt.y(), n0->pnt.z() );
//printf("  n1 = %f %f %f \n", n1->pnt.x(), n1->pnt.y(), n1->pnt.z() );
//printf("  n2 = %f %f %f \n", n2->pnt.x(), n2->pnt.y(), n2->pnt.z() );

    m_NVec.clear();         // jrg del mem...
    m_NVec.push_back( m_N0 );
    m_NVec.push_back( m_N1 );
    m_NVec.push_back( m_N2 );

    vec3d ray;
    if ( flattenAxis == 0 )
    {
        ray.set_xyz( 1.0, 0.0, 0.0 );
    }
    else if ( flattenAxis == 1 )
    {
        ray.set_xyz( 0.0, 1.0, 0.0 );
    }
    else if ( flattenAxis == 2 )
    {
        ray.set_xyz( 0.0, 0.0, 1.0 );
    }

    for ( i = 3 ; i < out.numberofpoints ; i++ )
    {
        vec3d pnt;
        if ( flattenAxis == 0 )
        {
            pnt.set_xyz( 0.0, out.pointlist[i * 2], out.pointlist[i * 2 + 1] );
        }
        else if ( flattenAxis == 1 )
        {
            pnt.set_xyz( out.pointlist[i * 2], 0.0, out.pointlist[i * 2 + 1] );
        }
        else if ( flattenAxis == 2 )
        {
            pnt.set_xyz( out.pointlist[i * 2], out.pointlist[i * 2 + 1], 0.0 );
        }

        double t;
        vec3d n0pnt  = m_N0->m_Pnt;
        vec3d n10pnt = m_N1->m_Pnt - m_N0->m_Pnt;
        vec3d n20pnt = m_N2->m_Pnt - m_N0->m_Pnt;
        plane_ray_intersect( n0pnt, n10pnt, n20pnt, pnt, ray, t );

        vec3d mapPnt = pnt + ray * t;

        TNode* sn = new TNode();        // New node
        sn->m_Pnt = mapPnt;
        sn->m_IsectFlag = 1;
        m_NVec.push_back( sn );
    }

    //==== Load Triangles if No New Point Created ====//
    cnt = 0;
    for ( i = 0 ; i < out.numberoftriangles ; i++ )
    {
        if ( out.trianglelist[cnt]   < ( int )m_NVec.size() &&
                out.trianglelist[cnt + 1] < ( int )m_NVec.size() &&
                out.trianglelist[cnt + 2] < ( int )m_NVec.size() )
        {
            TTri* t = new TTri();
            t->m_N0 = m_NVec[out.trianglelist[cnt]];
            t->m_N1 = m_NVec[out.trianglelist[cnt + 1]];
            t->m_N2 = m_NVec[out.trianglelist[cnt + 2]];
            m_SplitVec.push_back( t );
        }
        else
        {
            /* jrg problem - look into...
                        printf("\n");
                        printf("m\n");
                        printf("c red\n");
                        printf("%12.8f, %12.8f, %12.8f \n", n0->pnt.x(), n0->pnt.y(), n0->pnt.z());
                        printf("%12.8f, %12.8f, %12.8f \n", n1->pnt.x(), n1->pnt.y(), n1->pnt.z());
                        printf("%12.8f, %12.8f, %12.8f \n", n2->pnt.x(), n2->pnt.y(), n2->pnt.z());
                        printf("c blue\n");
                        for ( j = 0 ; j < iSectEdgeVec.size() ; j++ )
                        {
                            printf("m\n");
                            printf("%12.8f, %12.8f, %12.8f \n", iSectEdgeVec[j]->n0->pnt.x(), iSectEdgeVec[j]->n0->pnt.y(), iSectEdgeVec[j]->n0->pnt.z());
                            printf("%12.8f, %12.8f, %12.8f \n", iSectEdgeVec[j]->n0->pnt.x(), iSectEdgeVec[j]->n1->pnt.y(), iSectEdgeVec[j]->n1->pnt.z());
                        }
            */

        }
        cnt += 3;
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

    //=== Orient Tris to Match Normal ====//
    for ( i = 0 ; i < ( int )m_SplitVec.size() ; i++ )
    {
        TTri* t = m_SplitVec[i];
        vec3d d01 = t->m_N0->m_Pnt - t->m_N1->m_Pnt;
        vec3d d21 = t->m_N2->m_Pnt - t->m_N1->m_Pnt;

        vec3d cx = cross( d21, d01 );
        cx.normalize();
        m_Norm.normalize();

        double del = dist_squared( cx, m_Norm );
        if ( del > 1.0 )
        {
            TNode* tmp = t->m_N1;
            t->m_N1 = t->m_N2;
            t->m_N2 = tmp;
        }
    }
}


int TTri::OnEdge( vec3d & p, TEdge* e, double onEdgeTol )
{
    //==== Make Sure Not Duplicate Points ====//
    if ( dist( p, e->m_N0->m_Pnt ) < onEdgeTol )
    {
        return 0;
    }

    if ( dist( p, e->m_N1->m_Pnt ) < onEdgeTol )
    {
        return 0;
    }

    double t;
    double d = pointSegDistSquared( p, e->m_N0->m_Pnt, e->m_N1->m_Pnt, &t );

    if ( d < onEdgeTol * onEdgeTol )
    {
        return 1;
    }

    return 0;
}

int TTri::DupEdge( TEdge* e0, TEdge* e1, double tol )
{
    if ( ( dist( e0->m_N0->m_Pnt, e1->m_N0->m_Pnt ) < tol ) &&
            ( dist( e0->m_N1->m_Pnt, e1->m_N1->m_Pnt ) < tol ) )
    {
        return 1;
    }

    if ( ( dist( e0->m_N0->m_Pnt, e1->m_N1->m_Pnt ) < tol ) &&
            ( dist( e0->m_N1->m_Pnt, e1->m_N0->m_Pnt ) < tol ) )
    {
        return 1;
    }

    return 0;
}

void TTri::ComputeCosAngles( double* ang0, double* ang1, double* ang2 )
{
    double dsqr01 = dist_squared( m_N0->m_Pnt, m_N1->m_Pnt );
    double dsqr12 = dist_squared( m_N1->m_Pnt, m_N2->m_Pnt );
    double dsqr20 = dist_squared( m_N2->m_Pnt, m_N0->m_Pnt );

    double d01 = sqrt( dsqr01 );
    double d12 = sqrt( dsqr12 );
    double d20 = sqrt( dsqr20 );

    *ang0 = ( -dsqr12 + dsqr01 + dsqr20 ) / ( 2.0 * d01 * d20 );

    *ang1 = ( -dsqr20 + dsqr01 + dsqr12 ) / ( 2.0 * d01 * d12 );

    *ang2 = ( -dsqr01 + dsqr12 + dsqr20 ) / ( 2.0 * d12 * d20 );
}


void TTri::SplitEdges( TNode* n01, TNode* n12, TNode* n20 )
{
    TTri* tri;
    if ( n01 && n12 && n20 )        // Three Split - Make Four Tris
    {
        tri = new TTri();
        tri->m_N0 = m_N0;
        tri->m_N1 = n01;
        tri->m_N2 = n20;
        tri->m_Norm = m_Norm;
        m_SplitVec.push_back( tri );

        tri = new TTri();
        tri->m_N0 = m_N1;
        tri->m_N1 = n12;
        tri->m_N2 = n01;
        tri->m_Norm = m_Norm;
        m_SplitVec.push_back( tri );

        tri = new TTri();
        tri->m_N0 = m_N2;
        tri->m_N1 = n20;
        tri->m_N2 = n12;
        tri->m_Norm = m_Norm;
        m_SplitVec.push_back( tri );

        tri = new TTri();
        tri->m_N0 = n01;
        tri->m_N1 = n12;
        tri->m_N2 = n20;
        tri->m_Norm = m_Norm;
        m_SplitVec.push_back( tri );

    }



}







//===============================================//
//===============================================//
//===============================================//
//===============================================//
//                  TBndBox
//===============================================//
//===============================================//
//===============================================//
//===============================================//

TBndBox::TBndBox()
{
    for ( int i = 0 ; i < 8 ; i++ )
    {
        m_SBoxVec[i] = 0;
    }
}

TBndBox::~TBndBox()
{
    for ( int i = 0 ; i < 8 ; i++ )
    {
        if ( m_SBoxVec[i] )
        {
            delete m_SBoxVec[i];
        }
    }
}

//==== Create Oct Tree of Overlaping BndBoxes ====//
void TBndBox::SplitBox()
{
    int i;
    if ( m_TriVec.size() > 32 )
    {
        //==== Find Split Points ====//
        double hx = 0.5 * ( m_Box.GetMax( 0 ) + m_Box.GetMin( 0 ) );
        double hy = 0.5 * ( m_Box.GetMax( 1 ) + m_Box.GetMin( 1 ) );
        double hz = 0.5 * ( m_Box.GetMax( 2 ) + m_Box.GetMin( 2 ) );

        for ( i = 0 ; i < 8 ; i++ )
        {
            m_SBoxVec[i] = new TBndBox();
        }

        for ( i = 0 ; i < ( int )m_TriVec.size() ; i++ )
        {
            int cnt = 0;
            if ( m_TriVec[i]->m_N0->m_Pnt.x() > hx )
            {
                cnt += 1;
            }
            if ( m_TriVec[i]->m_N0->m_Pnt.y() > hy )
            {
                cnt += 2;
            }
            if ( m_TriVec[i]->m_N0->m_Pnt.z() > hz )
            {
                cnt += 4;
            }
            m_SBoxVec[cnt]->AddTri( m_TriVec[i] );
        }

        int contSplitFlag = 1;

        for ( i = 0 ; i < 8 ; i++ )
        {
            if ( m_TriVec.size() == m_SBoxVec[i]->m_TriVec.size() )
            {
                contSplitFlag = 0;
                break;
            }
        }
        if ( contSplitFlag )
        {
            for ( i = 0 ; i < 8 ; i++ )
            {
                m_SBoxVec[i]->SplitBox();
            }
        }
    }
}

void TBndBox::AddTri( TTri* t )
{
    m_TriVec.push_back( t );
    m_Box.Update( t->m_N0->m_Pnt );
    m_Box.Update( t->m_N1->m_Pnt );
    m_Box.Update( t->m_N2->m_Pnt );
}

void  TBndBox::AddLeafNodes( vector< TBndBox* > & leafVec )
{
    int i;

    if ( m_SBoxVec[0] )     // Keep Moving Down
    {
        for ( i = 0 ; i < 8 ; i++ )
        {
            m_SBoxVec[i]->AddLeafNodes( leafVec );
        }
    }
    else
    {
        leafVec.push_back( this );
    }
}

void TBndBox::Intersect( TBndBox* iBox )
{
    int i;

    if ( !Compare( m_Box, iBox->m_Box ) )
    {
        return;
    }

    if ( m_SBoxVec[0] )
    {
        for ( i = 0 ; i < 8 ; i++ )
        {
            iBox->Intersect( m_SBoxVec[i] );
        }
    }
    else if ( iBox->m_SBoxVec[0] )
    {
        for ( i = 0 ; i < 8 ; i++ )
        {
            iBox->m_SBoxVec[i]->Intersect( this );
        }
    }
    else
    {
        int i, j;
        int coplanarFlag;
        vec3d e0;
        vec3d e1;

        int iCnt = 0;

        for ( i = 0 ; i < ( int )m_TriVec.size() ; i++ )
        {
            TTri* t0 = m_TriVec[i];
            for ( j = 0 ; j < ( int )iBox->m_TriVec.size() ; j++ )
            {
                TTri* t1 = iBox->m_TriVec[j];

                int iflag = tri_tri_intersect_with_isectline(
                                t0->m_N0->m_Pnt.v, t0->m_N1->m_Pnt.v, t0->m_N2->m_Pnt.v,
                                t1->m_N0->m_Pnt.v, t1->m_N1->m_Pnt.v, t1->m_N2->m_Pnt.v,
                                &coplanarFlag, e0.v, e1.v );

                iCnt += iflag;
                if ( iflag && !coplanarFlag )
                {
                    TEdge* ie0 = new TEdge();
                    ie0->m_N0 = new TNode();
                    ie0->m_N0->m_Pnt = e0;
                    ie0->m_N1 = new TNode();
                    ie0->m_N1->m_Pnt = e1;

                    TEdge* ie1 = new TEdge();
                    ie1->m_N0 = new TNode();
                    ie1->m_N0->m_Pnt = e0;
                    ie1->m_N1 = new TNode();
                    ie1->m_N1->m_Pnt = e1;

                    if ( dist( e0, e1 ) > 0.000001 )
                    {
                        t0->m_ISectEdgeVec.push_back( ie0 );
                        t1->m_ISectEdgeVec.push_back( ie1 );
                    }
                    else
                    {
                        delete ie0->m_N0;
                        delete ie0->m_N1;
                        delete ie1->m_N0;
                        delete ie1->m_N1;
                        delete ie0;
                        delete ie1;
                    }

                }
            }
        }
    }
}
void  TBndBox::NumCrossXRay( vec3d & orig, vector<double> & tParmVec )
{
    int i;

    if ( orig.y() < m_Box.GetMin( 1 ) )
    {
        return;
    }
    if ( orig.y() > m_Box.GetMax( 1 ) )
    {
        return;
    }
    if ( orig.z() < m_Box.GetMin( 2 ) )
    {
        return;
    }
    if ( orig.z() > m_Box.GetMax( 2 ) )
    {
        return;
    }

    if ( m_SBoxVec[0] )
    {
        for ( i = 0 ; i < 8 ; i++ )
        {
            m_SBoxVec[i]->NumCrossXRay( orig, tParmVec );
        }
        return;
    }

    //==== Check All Tris In Box ====//
    double tparm, uparm, vparm;
    vec3d dir( 1.0, 0.0, 0.0 );
    for ( i = 0 ; i < ( int )m_TriVec.size() ; i++ )
    {
        TTri* tri = m_TriVec[i];
        int iFlag = intersect_triangle( orig.v, dir.v,
                                        tri->m_N0->m_Pnt.v, tri->m_N1->m_Pnt.v, tri->m_N2->m_Pnt.v, &tparm, &uparm, &vparm );

        if ( iFlag && tparm > 0.0 )
        {
            //==== Find If T is Already Included ====//
            int dupFlag = 0;
            for ( int j = 0 ; j < ( int )tParmVec.size() ; j++ )
            {
                if ( fabs( tparm - tParmVec[j] ) < 0.0000001 )
                {
                    dupFlag = 1;
                    break;
                }
            }

            if ( !dupFlag )
            {
                tParmVec.push_back( tparm );
            }
        }
    }

}

void TBndBox::SegIntersect( vec3d & p0, vec3d & p1, vector< vec3d > & ipntVec )
{
    int i, t;

    if ( !m_Box.CheckPnt( p0.x(), p0.y(), p0.z() ) && !m_Box.CheckPnt( p1.x(), p1.y(), p1.z() ) )
    {
        return;
    }

    if ( m_SBoxVec[0] )
    {
        for ( i = 0 ; i < 8 ; i++ )
        {
            m_SBoxVec[i]->SegIntersect( p0, p1, ipntVec );
        }
        return;
    }

    //==== Check All Tris In Box ====//
    double tparm, uparm, vparm;
    for ( t = 0 ; t < ( int )m_TriVec.size() ; t++ )
    {
        TTri* tri = m_TriVec[t];
        vec3d n0pnt  = tri->m_N0->m_Pnt;
        vec3d n10pnt = tri->m_N1->m_Pnt - tri->m_N0->m_Pnt;
        vec3d n20pnt = tri->m_N2->m_Pnt - tri->m_N0->m_Pnt;
        vec3d p10    = p1 - p0;
        if ( tri_seg_intersect( n0pnt,  n10pnt, n20pnt,
                                p0, p10, uparm, vparm, tparm ) )
        {
            vec3d pnt = p0 + ( p1 - p0 ) * tparm;
            ipntVec.push_back( pnt );
        }
    }

}


//===============================================//
//===============================================//
//===============================================//
//===============================================//
//                  NBndBox
//===============================================//
//===============================================//
//===============================================//
//===============================================//
NBndBox::NBndBox()
{
    for ( int i = 0 ; i < 8 ; i++ )
    {
        m_SBoxVec[i] = 0;
    }
}

NBndBox::~NBndBox()
{
    for ( int i = 0 ; i < 8 ; i++ )
    {
        if ( m_SBoxVec[i] )
        {
            delete m_SBoxVec[i];
        }
    }
}

//==== Create Oct Tree of Overlaping BndBoxes ====//
void NBndBox::SplitBox( double maxSize )
{
    int i;
    if ( m_NodeVec.size() > 64 && m_Box.DiagDist() > maxSize  )
    {
        //==== Find Split Points ====//
        double hx = 0.5 * ( m_Box.GetMax( 0 ) + m_Box.GetMin( 0 ) );
        double hy = 0.5 * ( m_Box.GetMax( 1 ) + m_Box.GetMin( 1 ) );
        double hz = 0.5 * ( m_Box.GetMax( 2 ) + m_Box.GetMin( 2 ) );

        for ( i = 0 ; i < 8 ; i++ )
        {
            m_SBoxVec[i] = new NBndBox();
        }

        for ( i = 0 ; i < ( int )m_NodeVec.size() ; i++ )
        {
            int cnt = 0;
            if ( m_NodeVec[i]->m_Pnt.x() > hx )
            {
                cnt += 1;
            }
            if ( m_NodeVec[i]->m_Pnt.y() > hy )
            {
                cnt += 2;
            }
            if ( m_NodeVec[i]->m_Pnt.z() > hz )
            {
                cnt += 4;
            }
            m_SBoxVec[cnt]->AddNode( m_NodeVec[i] );
        }

        int contSplitFlag = 1;
        /* Not Needed for Nodes ???
                for ( i = 0 ; i < 8 ; i++ )
                {
                    if ( triVec.size() == sBoxVec[i]->triVec.size() )
                    {
                        contSplitFlag = 0;
                        break;
                    }
                }
        */
        if ( contSplitFlag )
        {
            for ( i = 0 ; i < 8 ; i++ )
            {
                m_SBoxVec[i]->SplitBox( maxSize );
            }
        }
    }
}

void NBndBox::AddNode( TNode* n )
{
    m_NodeVec.push_back( n );
    m_Box.Update( n->m_Pnt );
}

void NBndBox::AddLeafNodes( vector< NBndBox* > & leafVec )
{
    int i;

    if ( m_SBoxVec[0] )     // Keep Moving Down
    {
        for ( i = 0 ; i < 8 ; i++ )
        {
            m_SBoxVec[i]->AddLeafNodes( leafVec );
        }
    }
    else
    {
        if ( m_NodeVec.size() )
        {
            leafVec.push_back( this );
        }
    }
}

//===============================================//
//===============================================//
//===============================================//
//===============================================//
//                  WaterTight Check
//===============================================//
//===============================================//
//===============================================//
//===============================================//

void TMesh::WaterTightCheck( FILE* fid, vector<TMesh*> & origTMeshVec )
{
    double minE, minA, maxA;

    fprintf( fid, "\n...WaterTight Check...\n" );

    MatchNodes();

    CheckValid( 0 );

    MeshStats( &minE, &minA, &maxA );

    fprintf( fid, "  Before Edge Swap and Needle Removal\n" );
    fprintf( fid, "  Min Edge Length = %f\n", minE );
    fprintf( fid, "  Min Angle = %f\n", minA );
    fprintf( fid, "  Max Angle = %f\n", maxA );

    //==== Try To Improve Triangle Quality ====//
    for ( int i = 0 ; i < 10 ; i++ )
    {
        TagNeedles( 2.0, 0.005, 1 );
        MatchNodes();

        SwapEdges( 178.0 );

        TagNeedles( 2.0, 0.005, 1 );
        MatchNodes();
    }

    MeshStats( &minE, &minA, &maxA );

    fprintf( fid, "  After Edge Swap and Needle Removal\n" );
    fprintf( fid, "  Min Edge Length = %f\n", minE );
    fprintf( fid, "  Min Angle = %f\n", minA );
    fprintf( fid, "  Max Angle = %f\n", maxA );

    CheckValid( fid );



}

void TMesh::TagNeedles( double minTriAng, double minAspectRatio, int delFlag )
{
    int t;

    //==== Only Tag Intersection Triangle ====//
    for ( t = 0 ; t < ( int )m_TVec.size() ; t++ )
    {
        if ( m_TVec[t]->m_N0->m_IsectFlag ||    m_TVec[t]->m_N1->m_IsectFlag || m_TVec[t]->m_N2->m_IsectFlag )
        {
            double a0, a1, a2;
            m_TVec[t]->ComputeCosAngles( &a0, &a1, &a2 );
            a0 = acos( a0 ) * 180.0 / PI;
            a1 = acos( a1 ) * 180.0 / PI;
            a2 = acos( a2 ) * 180.0 / PI;

            double minAng = a0;
            if ( a1 < minAng )
            {
                minAng = a1;
            }
            if ( a2 < minAng )
            {
                minAng = a2;
            }

            double maxAng = a0;
            if ( a1 > maxAng )
            {
                maxAng = a1;
            }
            if ( a2 > maxAng )
            {
                maxAng = a2;
            }

            double d01, d12, d20;
            d01 = dist_squared( m_TVec[t]->m_N0->m_Pnt, m_TVec[t]->m_N1->m_Pnt );
            d12 = dist_squared( m_TVec[t]->m_N1->m_Pnt, m_TVec[t]->m_N2->m_Pnt );
            d20 = dist_squared( m_TVec[t]->m_N2->m_Pnt, m_TVec[t]->m_N0->m_Pnt );

            if ( a0 < minTriAng && d12 / ( d20 + d01 ) < minAspectRatio )
            {
                if ( delFlag )
                {
                    MoveNode( m_TVec[t]->m_N1, m_TVec[t]->m_N2 );
                }
                else
                {
                    m_TVec[t]->m_InvalidFlag = 1;
                }
            }
            else if ( a1 < minTriAng && d20 / ( d01 + d12 ) < minAspectRatio )
            {
                if ( delFlag )
                {
                    MoveNode( m_TVec[t]->m_N2, m_TVec[t]->m_N0 );
                }
                else
                {
                    m_TVec[t]->m_InvalidFlag = 1;
                }
            }
            else if ( a2 < minTriAng && d01 / ( d12 + d20 ) < minAspectRatio )
            {
                if ( delFlag )
                {
                    MoveNode( m_TVec[t]->m_N0, m_TVec[t]->m_N1 );
                }
                else
                {
                    m_TVec[t]->m_InvalidFlag = 1;
                }
            }
        }
    }
}

void TMesh::MoveNode( TNode* n0, TNode* n1 )
{
    if ( n0->m_IsectFlag && n1->m_IsectFlag )
    {
        vec3d p = ( n0->m_Pnt + n1->m_Pnt ) * 0.5;
        n0->m_Pnt = p;
        n1->m_Pnt = p;
    }
    else if ( n0->m_IsectFlag )
    {
        n1->m_Pnt = n0->m_Pnt;
    }
    else if ( n1->m_IsectFlag )
    {
        n0->m_Pnt = n1->m_Pnt;
    }
    else
    {
        vec3d p = ( n0->m_Pnt + n1->m_Pnt ) * 0.5;
        n0->m_Pnt = p;
        n1->m_Pnt = p;
    }
}

void TMesh::FindEdge( TNode* node, TTri* tri0, TTri* tri1 )
{
    TNode* t0n0;
    TNode* t0n1;
    TNode* t1n0;
    TNode* t1n1;

    t0n0 = t0n1 = t1n0 = t1n1 = NULL;

    if ( tri0->m_N0 == node )
    {
        t0n0 = tri0->m_N1;
        t0n1 = tri0->m_N2;
    }
    else if ( tri0->m_N1 == node )
    {
        t0n0 = tri0->m_N0;
        t0n1 = tri0->m_N2;
    }
    else if ( tri0->m_N2 == node )
    {
        t0n0 = tri0->m_N0;
        t0n1 = tri0->m_N1;
    }
    if ( tri1->m_N0 == node )
    {
        t1n0 = tri1->m_N1;
        t1n1 = tri1->m_N2;
    }
    else if ( tri1->m_N1 == node )
    {
        t1n0 = tri1->m_N0;
        t1n1 = tri1->m_N2;
    }
    else if ( tri0->m_N2 == node )
    {
        t1n0 = tri1->m_N0;
        t1n1 = tri1->m_N1;
    }

    if ( t0n0 == t1n0 || t0n0 == t1n1 )
    {
        AddEdge( tri0, tri1, node, t0n0 );
    }
    else if ( t0n1 == t1n0 || t0n1 == t1n1 )
    {
        AddEdge( tri0, tri1, node, t0n1 );
    }
}

void TMesh::AddEdge( TTri* tri0, TTri* tri1, TNode* node0, TNode* node1 )
{
    if ( tri0->m_E0 && ( tri0->m_E0->m_Tri0 == tri1 || tri0->m_E0->m_Tri1 == tri1 ) )   // Check If Edge Already There
    {
        return;
    }

    if ( tri0->m_E1 && ( tri0->m_E1->m_Tri0 == tri1 || tri0->m_E1->m_Tri1 == tri1 ) )
    {
        return;
    }

    if ( tri0->m_E2 && ( tri0->m_E2->m_Tri0 == tri1 || tri0->m_E2->m_Tri1 == tri1 ) )
    {
        return;
    }

    //==== Create Edge ====//
    TEdge* edge = new TEdge();

    edge->m_N0 = node0;
    edge->m_N1 = node1;
    edge->m_Tri0 = tri0;
    edge->m_Tri1 = tri1;

    if ( !tri0->m_E0 )
    {
        tri0->m_E0 = edge;
    }
    else if ( !tri0->m_E1 )
    {
        tri0->m_E1 = edge;
    }
    else if ( !tri0->m_E2 )
    {
        tri0->m_E2 = edge;
    }

    if ( !tri1->m_E0 )
    {
        tri1->m_E0 = edge;
    }
    else if ( !tri1->m_E1 )
    {
        tri1->m_E1 = edge;
    }
    else if ( !tri1->m_E2 )
    {
        tri1->m_E2 = edge;
    }

    node0->m_EdgeVec.push_back( edge );
    node1->m_EdgeVec.push_back( edge );

    m_EVec.push_back( edge );

}

TNode* TMesh::LowNode( TNode* node )
{
    int i;

    TNode* lowN = node;

    if ( node->m_MergeVec.size() <= 1 )
    {
        return lowN;
    }

    //==== Find Lowest Node Ptr ====//
    for ( i = 0 ; i < ( int )node->m_MergeVec.size() ; i++ )
    {
        if ( ( long )( node->m_MergeVec[i] ) < ( long )lowN )
        {
            lowN = node->m_MergeVec[i];
        }
    }

    for ( i = 0 ; i < ( int )node->m_MergeVec.size() ; i++ )
    {
        if ( node->m_MergeVec[i] != lowN )
        {
            node->m_MergeVec[i]->m_ID = -999;
        }
    }
    return lowN;
}


void TMesh::SwapEdge( TEdge* edge )
{
    if ( !edge->m_Tri0 )
    {
        return;
    }
    if ( !edge->m_Tri1 )
    {
        return;
    }

    TNode *n0, *n1, *n2, *n3;
    n0 = edge->m_N0;

    if ( edge->m_Tri0->m_N0 != edge->m_N0 && edge->m_Tri0->m_N0 != edge->m_N1 )
    {
        n1 = edge->m_Tri0->m_N0;
    }
    else if ( edge->m_Tri0->m_N1 != edge->m_N0 && edge->m_Tri0->m_N1 != edge->m_N1 )
    {
        n1 = edge->m_Tri0->m_N1;
    }
    else if ( edge->m_Tri0->m_N2 != edge->m_N0 && edge->m_Tri0->m_N2 != edge->m_N1 )
    {
        n1 = edge->m_Tri0->m_N2;
    }

    n2 = edge->m_N1;

    if ( edge->m_Tri1->m_N0 != edge->m_N0 && edge->m_Tri1->m_N0 != edge->m_N1 )
    {
        n3 = edge->m_Tri1->m_N0;
    }
    else if ( edge->m_Tri1->m_N1 != edge->m_N0 && edge->m_Tri1->m_N1 != edge->m_N1 )
    {
        n3 = edge->m_Tri1->m_N1;
    }
    else if ( edge->m_Tri1->m_N2 != edge->m_N0 && edge->m_Tri1->m_N2 != edge->m_N1 )
    {
        n3 = edge->m_Tri1->m_N2;
    }

    edge->m_Tri0->m_N0 = n0;
    edge->m_Tri0->m_N1 = n1;
    edge->m_Tri0->m_N2 = n3;

    edge->m_Tri0->m_Norm = edge->m_Tri1->m_Norm;

    edge->m_Tri1->m_N0 = n1;
    edge->m_Tri1->m_N1 = n2;
    edge->m_Tri1->m_N2 = n3;

}

void TMesh::BuildMergeMaps()
{
    BuildNodeMaps();
    BuildEdgeMaps();
}

void TMesh::BuildEdgeMaps()
{
    // Build maps between shared edges using node maps
    // Make sure BuildNodeMaps has already been called before calling this method

    // Build Perimeter Edges of Each Tri and push back tri onto master nodes
    for ( int t = 0 ; t < ( int )m_TVec.size(); t++ )
    {
        m_TVec[t]->BuildPermEdges();
        m_NSMMap[m_TVec[t]->m_N0]->m_TriVec.push_back( m_TVec[t] );
        m_NSMMap[m_TVec[t]->m_N1]->m_TriVec.push_back( m_TVec[t] );
        m_NSMMap[m_TVec[t]->m_N2]->m_TriVec.push_back( m_TVec[t] );
    }

    // Loop through triangles sharing nodes to find alias edges
    map< TNode*, list<TNode*> >::iterator mit; // map iterator

    for ( mit = m_NAMap.begin(); mit != m_NAMap.end() ; mit++ ) // Loop over all master nodes
    {
        TNode* n = mit->first;
        for ( int t = 0 ; t < ( int )n->m_TriVec.size() ; t++ ) // Loop over triangles sharing the master node
        {
            TTri* tri1 = n->m_TriVec[t];

            for ( int pei = 0 ; pei < 3 ; pei++ ) // Loop over the perimeter edges of first triangle
            {
                TEdge* e1 = tri1->m_PEArr[pei];
                if ( m_ESMMap.find( e1 ) == m_ESMMap.end() // only continue if this edge isn't already a master
                        && ( m_NSMMap[e1->m_N0] == n || m_NSMMap[e1->m_N1] == n ) ) // only continue if this edge contains to common node
                {

                    for ( int t2 = t + 1 ; t2 < ( int )n->m_TriVec.size(); t2++ ) // Loop over the next triangle in list sharing the master node
                    {
                        TTri* tri2 = n->m_TriVec[t2];

                        for ( int pei2 = 0 ; pei2 < 3; pei2++ ) // Loop over the next triangle's perimeter edges
                        {
                            TEdge* e2 = tri2->m_PEArr[pei2];
                            if ( ( m_NSMMap[e1->m_N0] == m_NSMMap[e2->m_N0] && m_NSMMap[e1->m_N1] == m_NSMMap[e2->m_N1] ) ||
                                    ( m_NSMMap[e1->m_N1] == m_NSMMap[e2->m_N0] && m_NSMMap[e1->m_N0] == m_NSMMap[e2->m_N1] ) )
                            {
                                m_ESMMap[e2] = e1;
                                m_EAMap[e1].push_back( e2 );
                            }
                        }
                    }

                    m_ESMMap[e1] = e1; // Set first edge to be its own master
                }
            }
        }
    }

    // Brute Force Check
//  vector<TEdge*> edges;
//  for (int t = 0 ; t < (int) m_TVec.size(); t++ )
//  {
//      for ( int pi = 0; pi < 3 ; pi++)
//          edges.push_back( m_TVec[t]->m_PEArr[pi] );
//  }
//
//  for ( int ei = 0 ; ei < (int) edges.size(); ei++ )
//  {
//      TEdge* e1 = edges[ei];
//
//      if ( m_ESMMap.find(e1) == m_ESMMap.end() ) // continue if edge doesn't already have master
//      {
//          for ( int ei2 = ei+1 ; ei2 < (int)edges.size(); ei2++)
//          {
//              TEdge* e2 = edges[ei2];
//              if ( e1 != e2 )
//              {
//                  if ( ( m_NSMMap[e1->m_N0] == m_NSMMap[e2->m_N0] && m_NSMMap[e1->m_N1] == m_NSMMap[e2->m_N1] ) ||
//                       ( m_NSMMap[e1->m_N1] == m_NSMMap[e2->m_N0] && m_NSMMap[e1->m_N0] == m_NSMMap[e2->m_N1] ) )
//                  {
//                      m_ESMMap[e2] = e1;
//                      m_EAMap[e1].push_back(e2);
//                  }
//              }
//          }
//          m_ESMMap[e1] = e1;
//      }
//
//  }
}

void TMesh::BuildNodeMaps()
{
    // This method builds the map between a node and its aliases and a map between
    // each node to its master node

    int i, n, t;

    map< TNode*, list<TNode*> >::iterator mi;

    double tol = 1.0e-12;
    double sqtol = sqrt( tol );

    for ( n = 0 ; n < ( int )m_NVec.size() ; n++ )
    {
        m_NVec[n]->m_MergeVec.clear();
    }

    NBndBox nBox;
    for ( n = 0 ; n < ( int )m_NVec.size() ; n++ )
    {
        nBox.AddNode( m_NVec[n] );
    }
    nBox.SplitBox( sqrt( tol ) );

    //==== Find All Leaves of Oct Tree ====//
    vector< NBndBox* > leafVec;
    nBox.AddLeafNodes( leafVec );

    for ( i = 0 ; i < ( int )leafVec.size() ; i++ )
    {
        leafVec[i]->m_Box.Expand( sqtol );
    }

    for ( n = 0 ; n < ( int )m_NVec.size() ; n++ )
    {
        if ( m_NSMMap.find( m_NVec[n] ) == m_NSMMap.end() ) // This node doesn't have a master so continue
        {
            for ( i = 0 ; i < ( int )leafVec.size() ; i++ )
            {

                if ( leafVec[i]->m_Box.CheckPnt( m_NVec[n]->m_Pnt.x(), m_NVec[n]->m_Pnt.y(), m_NVec[n]->m_Pnt.z() ) )
                {
                    for ( int m = 0 ; m < ( int )leafVec[i]->m_NodeVec.size() ; m++ )
                    {
                        if ( m_NVec[n] != leafVec[i]->m_NodeVec[m] ) // If it is the same node skip
                        {
                            if ( dist_squared( m_NVec[n]->m_Pnt, leafVec[i]->m_NodeVec[m]->m_Pnt ) < tol )
                            {
                                m_NVec[n]->m_MergeVec.push_back( leafVec[i]->m_NodeVec[m] );
                                m_NAMap[m_NVec[n]].push_back( leafVec[i]->m_NodeVec[m] ); // Add node m to n's list of aliases
                                m_NSMMap[leafVec[i]->m_NodeVec[m]] = m_NVec[n]; // Set m's master to be n
                            }
                        }
                    }
                }
            }

            // Set n to be its own master
            m_NSMMap[ m_NVec[n] ] = m_NVec[n];

            // Add n to its own set of aliases
            m_NAMap[ m_NVec[n] ].push_front( m_NVec[n] );
        }
    }

}

void TMesh::DeleteDupNodes()
{
    // Build Merge Maps should have been called before this method is called
    // After this method is called Build Merge Maps will need to be called before this
    // method can be called again

    if ( m_NAMap.size() == 0 || m_NSMMap.size() == 0 )
    {
        return;
    }

    int t;
    map< TNode*, list< TNode* > >::iterator mit;
    list< TNode* >::iterator lit;

    //==== Go Thru All Tri And Set All Nodes to their Master ====//
    for ( t = 0 ; t < m_TVec.size(); t++ )
    {
        m_TVec[t]->m_N0 = m_NSMMap[m_TVec[t]->m_N0];
        m_TVec[t]->m_N1 = m_NSMMap[m_TVec[t]->m_N1];
        m_TVec[t]->m_N2 = m_NSMMap[m_TVec[t]->m_N2];
    }

    //==== Nuke Degenerate Tris ====//
    vector< TTri* > tempTVec;
    for ( t = 0 ; t < ( int )m_TVec.size() ; t++ )
    {
        if ( m_TVec[t]->m_N0 != m_TVec[t]->m_N1 &&
                m_TVec[t]->m_N0 != m_TVec[t]->m_N2 &&
                m_TVec[t]->m_N1 != m_TVec[t]->m_N2 )
        {
            tempTVec.push_back( m_TVec[t] );
        }
        else
        {
            delete m_TVec[t];
        }
    }
    m_TVec = tempTVec;

    for ( t = 0 ; t < ( int )m_TVec.size() ; t++ )
    {
        m_TVec[t]->m_N0->m_TriVec.push_back( m_TVec[t] );
        m_TVec[t]->m_N1->m_TriVec.push_back( m_TVec[t] );
        m_TVec[t]->m_N2->m_TriVec.push_back( m_TVec[t] );
        if ( m_TVec[t]->m_N0->m_ID == -999 || m_TVec[t]->m_N1->m_ID == -999 || m_TVec[t]->m_N2->m_ID == -999 )
        {
            printf( "Found -999\n" );
        }
    }

    //==== Nuke Redundant Nodes And Update NVec ====//
    m_NVec.clear();
    for ( mit = m_NAMap.begin() ; mit != m_NAMap.end() ; mit++ )
    {
        TNode* nk = mit->first;
        list< TNode* >& dnodes =  mit->second;

        for ( lit = ++dnodes.begin() ; lit != dnodes.end() ; lit++ ) // Start at second element since first is the master node itself
        {
            delete *lit;
        }

        nk->m_MergeVec.clear();
        nk->m_TriVec.clear();
        m_NVec.push_back( nk );
    }

    // Clear out Node and Edge maps since they are now useless
    m_NAMap.clear();
    m_NSMMap.clear();
    m_EAMap.clear();
    m_ESMMap.clear();

}

void TMesh::MatchNodes()
{
//  int i, n, t;
//
//  double tol = 1.0e-12;
//  double sqtol = sqrt(tol);
//
//  for ( n = 0 ; n < (int)m_NVec.size() ; n++ )
//      m_NVec[n]->m_MergeVec.clear();
//
//  NBndBox nBox;
//  for ( n = 0 ; n < (int)m_NVec.size() ; n++ )
//  {
//      nBox.AddNode( m_NVec[n] );
//  }
//  nBox.SplitBox(sqrt(tol));
//
//  //==== Find All Leaves of Oct Tree ====//
//  vector< NBndBox* > leafVec;
//  nBox.AddLeafNodes( leafVec );
//
//  for ( i = 0 ; i < (int)leafVec.size() ; i++ )
//  {
//      leafVec[i]->m_Box.Expand( sqtol );
//  }
//
//  for ( n = 0 ; n < (int)m_NVec.size() ; n++ )
//  {
//      for ( i = 0 ; i < (int)leafVec.size() ; i++ )
//      {
//          if ( leafVec[i]->m_Box.CheckPnt( m_NVec[n]->m_Pnt.x(), m_NVec[n]->m_Pnt.y(), m_NVec[n]->m_Pnt.z() ) )
//          {
//              for ( int m = 0 ; m < (int)leafVec[i]->m_NodeVec.size() ; m++ )
//              {
//                  if ( dist_squared( m_NVec[n]->m_Pnt, leafVec[i]->m_NodeVec[m]->m_Pnt ) < tol )
//                  {
//                      m_NVec[n]->m_MergeVec.push_back( leafVec[i]->m_NodeVec[m] );
//                  }
//              }
//          }
//      }
//  }
//
///*
//  for ( i = 0 ; i < leafVec.size() ; i++ )
//  {
//      for ( n = 0 ; n < leafVec[i]->nodeVec.size() ; n++ )
//      {
//          leafVec[i]->nodeVec[n]->mergeVec.push_back( leafVec[i]->nodeVec[n] );
//          for ( int m = n+1 ; m < leafVec[i]->nodeVec.size() ; m++ )
//          {
//              if ( dist_squared( leafVec[i]->nodeVec[n]->pnt, leafVec[i]->nodeVec[m]->pnt ) < tol )
//              {
//                  leafVec[i]->nodeVec[n]->mergeVec.push_back( leafVec[i]->nodeVec[m] );
//                  leafVec[i]->nodeVec[m]->mergeVec.push_back( leafVec[i]->nodeVec[n] );
//              }
//          }
//      }
//  }
//*/
//
//  //==== Go Thru All Tri And Refernce Lowest Node Ptr and Add Tag Other for Deletion ====//
//  for ( t = 0 ; t < (int)m_TVec.size() ; t++ )
//  {
//      m_TVec[t]->m_N0 = LowNode( m_TVec[t]->m_N0 );
//      m_TVec[t]->m_N1 = LowNode( m_TVec[t]->m_N1 );
//      m_TVec[t]->m_N2 = LowNode( m_TVec[t]->m_N2 );
//  }
//
//  //==== Nuke Degenerate Tris ====//
//  vector< TTri* > tempTVec;
//  for ( t = 0 ; t < (int)m_TVec.size() ; t++ )
//  {
//      if ( m_TVec[t]->m_N0 != m_TVec[t]->m_N1 &&
//           m_TVec[t]->m_N0 != m_TVec[t]->m_N2 &&
//           m_TVec[t]->m_N1 != m_TVec[t]->m_N2 )
//        tempTVec.push_back( m_TVec[t] );
//  }
//  m_TVec = tempTVec;
//
//  for ( t = 0 ; t < (int)m_TVec.size() ; t++ )
//  {
//      m_TVec[t]->m_N0->m_TriVec.push_back( m_TVec[t] );
//      m_TVec[t]->m_N1->m_TriVec.push_back( m_TVec[t] );
//      m_TVec[t]->m_N2->m_TriVec.push_back( m_TVec[t] );
//  if ( m_TVec[t]->m_N0->m_ID == -999 || m_TVec[t]->m_N1->m_ID == -999 || m_TVec[t]->m_N2->m_ID == -999 )
//      printf("Found -999\n");
//  }
//
//  //==== Nuke Redundant Nodes And Update NVec ====//
//  vector< TNode* > tempNVec;
//  for ( n = 0 ; n < (int)m_NVec.size() ; n++ )
//  {
//      m_NVec[n]->m_TriVec.clear();
//      m_NVec[n]->m_MergeVec.clear();
//      if ( m_NVec[n]->m_ID == -999 )
//          delete m_NVec[n];
//      else
//          tempNVec.push_back( m_NVec[n] );
//  }
//  m_NVec = tempNVec;
//
//
//  //==== Brute Force Check For Duplicate Nodes ====//
///****************************************************************
//  for ( n = 0 ; n < nVec.size() ; n++ )
//  {
//      for ( int m = n+1 ; m < nVec.size() ; m++ )
//      {
//          if ( dist_squared( nVec[n]->pnt, nVec[m]->pnt ) < tol )
//          {
//              printf("Duplicate Node %d %d\n", nVec[n], nVec[m] );
//          }
//      }
//  }
//*****************************************************************/
//
//  //==== Reassign Triangles ====//
//  for ( t = 0 ; t < (int)m_TVec.size() ; t++ )
//  {
//      m_TVec[t]->m_N0->m_TriVec.push_back( m_TVec[t] );
//      m_TVec[t]->m_N1->m_TriVec.push_back( m_TVec[t] );
//      m_TVec[t]->m_N2->m_TriVec.push_back( m_TVec[t] );
//  }

    BuildMergeMaps();
    DeleteDupNodes();

}

void TMesh::SwapEdges( double ang )
{
    int t, ot;

    //==== Remove Really Thin Triangles ====//
    for ( t = 0 ; t < ( int )m_TVec.size() ; t++ )
    {

        double a0, a1, a2;
        m_TVec[t]->ComputeCosAngles( &a0, &a1, &a2 );

        a0 = acos( a0 ) * 180.0 / PI;
        a1 = acos( a1 ) * 180.0 / PI;
        a2 = acos( a2 ) * 180.0 / PI;

        TEdge* edge = 0;
        if ( a0 > ang )
        {
            edge = new TEdge();
            edge->m_N0 = m_TVec[t]->m_N1;
            edge->m_N1 = m_TVec[t]->m_N2;
        }
        else if ( a1 > ang  )
        {
            edge = new TEdge();
            edge->m_N0 = m_TVec[t]->m_N0;
            edge->m_N1 = m_TVec[t]->m_N2;
        }
        else if ( a2 > ang  )
        {
            edge = new TEdge();
            edge->m_N0 = m_TVec[t]->m_N0;
            edge->m_N1 = m_TVec[t]->m_N1;
        }

        if ( edge )
        {
            edge->m_Tri0 = m_TVec[t];
            edge->m_Tri1 = 0;

            //==== Find Other Tri ====//
            for ( ot = 0 ; ot < ( int )edge->m_N0->m_TriVec.size() ; ot++ )
            {
                TTri* tri = edge->m_N0->m_TriVec[ot];
                if ( tri != edge->m_Tri0 )
                {
                    int match = 0;
                    if      ( tri->m_N0 == edge->m_N0 )
                    {
                        match++;
                    }
                    else if ( tri->m_N1 == edge->m_N0 )
                    {
                        match++;
                    }
                    else if ( tri->m_N2 == edge->m_N0 )
                    {
                        match++;
                    }
                    if      ( tri->m_N0 == edge->m_N1 )
                    {
                        match++;
                    }
                    else if ( tri->m_N1 == edge->m_N1 )
                    {
                        match++;
                    }
                    else if ( tri->m_N2 == edge->m_N1 )
                    {
                        match++;
                    }

                    if ( match == 2 )
                    {
                        edge->m_Tri1 = tri;
                    }
                }
                if ( edge->m_Tri1 )
                {
                    break;
                }
            }
            //==== Swap It ====//
            if ( edge->m_Tri1 )
            {
                SwapEdge( edge );
            }
        }
    }
}


void TMesh::CheckValid( FILE* fid )
{
    int n, s, t;

    //==== Clear Refs to Tris ====//
    for ( n = 0 ; n < ( int )m_NVec.size() ; n++ )
    {
        m_NVec[n]->m_TriVec.clear();
    }

    //==== Check If All Tris Have 3 Edges ====//
    for ( t = 0 ; t < ( int )m_TVec.size() ; t++ )
    {
        m_TVec[t]->m_N0->m_TriVec.push_back( m_TVec[t] );
        m_TVec[t]->m_N1->m_TriVec.push_back( m_TVec[t] );
        m_TVec[t]->m_N2->m_TriVec.push_back( m_TVec[t] );
        m_TVec[t]->m_E0 = 0;
        m_TVec[t]->m_E1 = 0;
        m_TVec[t]->m_E2 = 0;
    }

    //==== Create Edges For Adjacent Tris ====//
    for ( n = 0 ; n < ( int )m_NVec.size() ; n++ )
    {
        TNode* n0 = m_NVec[n];
        for ( t = 0 ; t < ( int )n0->m_TriVec.size() ; t++ )
        {
            for ( s = t + 1 ; s < ( int )n0->m_TriVec.size() ; s++ )
            {
                FindEdge( n0, n0->m_TriVec[t], n0->m_TriVec[s] );
            }
        }
    }

    //==== Check If All Tris Have 3 Edges ====//
    vector< TTri* > ivTriVec;
    for ( t = 0 ; t < ( int )m_TVec.size() ; t++ )
    {
        if ( !m_TVec[t]->m_E0 || !m_TVec[t]->m_E1 || !m_TVec[t]->m_E2 )
        {
            m_TVec[t]->m_InvalidFlag = 1;
            ivTriVec.push_back( m_TVec[t] );
        }
    }

    /**************************
        for ( it = 0 ; it < ivTriVec.size() ; it++ )
        {

            TTri* t0 = findTriPnts( ivTriVec[it], ivTriVec[it]->n0,  ivTriVec[it]->n1 );
            TTri* t1 = findTriPnts( ivTriVec[it], ivTriVec[it]->n1,  ivTriVec[it]->n2 );
            TTri* t2 = findTriPnts( ivTriVec[it], ivTriVec[it]->n2,  ivTriVec[it]->n0 );

            printf( "Invalid Tri: %d \n", ivTriVec[it] );
            printf( "   Tris: %d %d %d\n", t0, t1, t2 );
            printf( "   Edges: %d %d %d\n", ivTriVec[it]->e0, ivTriVec[it]->e1, ivTriVec[it]->e2 );
            printf( "   Nodes: %d %d %d\n", ivTriVec[it]->n0, ivTriVec[it]->n1, ivTriVec[it]->n2 );
        }
    **************************/

    if ( fid )
    {
        if ( ivTriVec.size() == 0 )
        {
            fprintf( fid, "Mesh IS WaterTight\n" );
        }
        else
        {
            fprintf( fid, "There are %d Invalid Triangles\n", ( int )ivTriVec.size() );
        }
    }

}


void TMesh::MeshStats( double* minEdgeLen, double* minTriAng, double* maxTriAng )
{
    int t;
    double d, a0, a1, a2;

    double minLenSq = 1.0e06;
    for ( t = 0 ; t < ( int )m_TVec.size() ; t++ )
    {
        d = dist_squared( m_TVec[t]->m_N0->m_Pnt, m_TVec[t]->m_N1->m_Pnt );
        if ( d < minLenSq )
        {
            minLenSq = d;
        }

        d = dist_squared( m_TVec[t]->m_N1->m_Pnt, m_TVec[t]->m_N2->m_Pnt );
        if ( d < minLenSq )
        {
            minLenSq = d;
        }

        d = dist_squared( m_TVec[t]->m_N2->m_Pnt, m_TVec[t]->m_N0->m_Pnt );
        if ( d < minLenSq )
        {
            minLenSq = d;
        }
    }
    *minEdgeLen = sqrt( minLenSq );

//  printf("Mesh Stats:\n");
//  printf("  Num Tris = %d\n", tVec.size() );
//  printf("  Min Edge Length = %f\n", *minEdgeLen );

    int minTri = 0;
    int maxTri = 0;
    double minAng = 1.0e06;
    double maxAng = -1.0e06;
    for ( t = 0 ; t < ( int )m_TVec.size() ; t++ )
    {
        m_TVec[t]->ComputeCosAngles( &a0, &a1, &a2 );

        a0 = acos( a0 ) * 180.0 / PI;
        a1 = acos( a1 ) * 180.0 / PI;
        a2 = acos( a2 ) * 180.0 / PI;

        if ( a0 < minAng )
        {
            minAng = a0;
            minTri = t;
        }
        if ( a0 > maxAng )
        {
            maxAng = a0;
            maxTri = t;
        }
        if ( a1 < minAng )
        {
            minAng = a1;
            minTri = t;
        }
        if ( a1 > maxAng )
        {
            maxAng = a1;
            maxTri = t;
        }
        if ( a2 < minAng )
        {
            minAng = a2;
            minTri = t;
        }
        if ( a2 > maxAng )
        {
            maxAng = a2;
            maxTri = t;
        }

    }

    *minTriAng = minAng;
    *maxTriAng = maxAng;

//  printf("  Min Angle = %f\n", minAng);
//  printf("  Max Angle = %f\n", maxAng);
}

TTri* TMesh::FindTriNodes( TTri* ignoreTri, TNode* n0, TNode* n1 )
{
    int t;
    for ( t = 0 ; t < ( int )m_TVec.size() ; t++ )
    {
        if ( m_TVec[t] != ignoreTri )
        {
            if ( m_TVec[t]->m_N0 == n0 && m_TVec[t]->m_N1 == n1 )
            {
                return m_TVec[t];
            }
            if ( m_TVec[t]->m_N1 == n0 && m_TVec[t]->m_N0 == n1 )
            {
                return m_TVec[t];
            }
            if ( m_TVec[t]->m_N1 == n0 && m_TVec[t]->m_N2 == n1 )
            {
                return m_TVec[t];
            }
            if ( m_TVec[t]->m_N2 == n0 && m_TVec[t]->m_N1 == n1 )
            {
                return m_TVec[t];
            }
            if ( m_TVec[t]->m_N2 == n0 && m_TVec[t]->m_N0 == n1 )
            {
                return m_TVec[t];
            }
            if ( m_TVec[t]->m_N0 == n0 && m_TVec[t]->m_N2 == n1 )
            {
                return m_TVec[t];
            }
        }
    }
    return 0;
}

TTri* TMesh::FindTriPnts( TTri* ignoreTri, TNode* n0, TNode* n1 )
{
    int t;
    double tol = 0.0000001;
    vec3d p0 = n0->m_Pnt;
    vec3d p1 = n1->m_Pnt;

    for ( t = 0 ; t < ( int )m_TVec.size() ; t++ )
    {
        if ( m_TVec[t] != ignoreTri )
        {
            vec3d pA = m_TVec[t]->m_N0->m_Pnt;
            vec3d pB = m_TVec[t]->m_N1->m_Pnt;
            vec3d pC = m_TVec[t]->m_N2->m_Pnt;

            if ( dist_squared( p0, pA ) < tol &&   dist_squared( p1, pB ) < tol )
            {
                return m_TVec[t];
            }
            if ( dist_squared( p0, pB ) < tol &&   dist_squared( p1, pA ) < tol )
            {
                return m_TVec[t];
            }
            if ( dist_squared( p0, pB ) < tol &&   dist_squared( p1, pC ) < tol )
            {
                return m_TVec[t];
            }
            if ( dist_squared( p0, pC ) < tol &&   dist_squared( p1, pB ) < tol )
            {
                return m_TVec[t];
            }
            if ( dist_squared( p0, pC ) < tol &&   dist_squared( p1, pA ) < tol )
            {
                return m_TVec[t];
            }
            if ( dist_squared( p0, pA ) < tol &&   dist_squared( p1, pC ) < tol )
            {
                return m_TVec[t];
            }
        }
    }
    return 0;
}

void TMesh::RelaxMesh( vector<TMesh*> & origTMeshVec )
{
    int n, t, e;
    TNode* node[3];
    TEdge* edge[3];

    //==== Save Edges of Intersection Curves  =====//
//  vector< vec3d > isectPairs;
    m_ISectPairs.clear();

    for ( n = 0 ; n < ( int )m_NVec.size() ; n++ )
    {
        if ( m_NVec[n]->m_IsectFlag == 1 )
        {
            for ( t = 0 ; t < ( int )m_NVec[n]->m_TriVec.size() ; t++ )
            {
                if ( m_NVec[n]->m_TriVec[t]->m_N0->m_IsectFlag == 1 &&  m_NVec[n]->m_TriVec[t]->m_N0 != m_NVec[n] )
                {
                    m_ISectPairs.push_back( m_NVec[n]->m_Pnt );
                    m_ISectPairs.push_back( m_NVec[n]->m_TriVec[t]->m_N0->m_Pnt );
                }
                if ( m_NVec[n]->m_TriVec[t]->m_N1->m_IsectFlag == 1 &&  m_NVec[n]->m_TriVec[t]->m_N0 != m_NVec[n] )
                {
                    m_ISectPairs.push_back( m_NVec[n]->m_Pnt );
                    m_ISectPairs.push_back( m_NVec[n]->m_TriVec[t]->m_N1->m_Pnt );
                }
                if ( m_NVec[n]->m_TriVec[t]->m_N2->m_IsectFlag == 1 &&  m_NVec[n]->m_TriVec[t]->m_N0 != m_NVec[n] )
                {
                    m_ISectPairs.push_back( m_NVec[n]->m_Pnt );
                    m_ISectPairs.push_back( m_NVec[n]->m_TriVec[t]->m_N2->m_Pnt );
                }
            }
        }
    }

    //==== Copy This Mesh To Project Relaxed Points On To =====//
    TMesh* saveMesh = new TMesh;
    for ( t = 0 ; t < ( int )m_TVec.size() ; t++ )
    {
        saveMesh->AddTri( m_TVec[t]->m_N0, m_TVec[t]->m_N1, m_TVec[t]->m_N2, m_TVec[t]->m_Norm );
    }
    saveMesh->LoadBndBox();

    //==== Load Current Edges in Nodes ====//
    for ( n = 0 ; n < ( int )m_NVec.size() ; n++ )
    {
        m_NVec[n]->m_EdgeVec.clear();
    }

    for ( t = 0 ; t < ( int )m_TVec.size() ; t++ )
    {
        node[0] = m_TVec[t]->m_N0;
        node[1] = m_TVec[t]->m_N1;
        node[2] = m_TVec[t]->m_N2;
        edge[0] = m_TVec[t]->m_E0;
        edge[1] = m_TVec[t]->m_E1;
        edge[2] = m_TVec[t]->m_E2;

        for ( n = 0 ; n < 3 ; n++ )
        {
            for ( e = 0 ; e < 3 ; e++ )
            {
                if ( edge[e]->m_N0 == node[n] )
                {
                    node[n]->m_EdgeVec.push_back( edge[e] );
                }
                else if ( edge[e]->m_N1 == node[n] )
                {
                    node[n]->m_EdgeVec.push_back( edge[e] );
                }
            }
        }
    }

    //==== Create An Offset Vector for Each Node ====//
    for ( int i = 0 ; i < 10 ; i++ )
    {
        vector< vec3d > offVec;
        for ( n = 0 ; n < ( int )m_NVec.size() ; n++ )
        {
            //==== Sum Up Offset Vectors ====//
            vec3d off;
            for ( e = 0 ; e < ( int )m_NVec[n]->m_EdgeVec.size() ; e++ )
            {
                if ( m_NVec[n]->m_EdgeVec[e]->m_N0 == m_NVec[n] )
                {
                    off = off + ( m_NVec[n]->m_EdgeVec[e]->m_N1->m_Pnt - m_NVec[n]->m_Pnt );
                }
                else
                {
                    off = off + ( m_NVec[n]->m_EdgeVec[e]->m_N0->m_Pnt - m_NVec[n]->m_Pnt );
                }
            }
            offVec.push_back( off );

            //==== Find Normal ====//
            m_NVec[n]->m_Norm.set_xyz( 0, 0, 0 );
            for ( t = 0 ; t < ( int )m_NVec[n]->m_TriVec.size() ; t++ )
            {
                m_NVec[n]->m_Norm = m_NVec[n]->m_Norm + m_NVec[n]->m_TriVec[t]->CompNorm();
            }
            m_NVec[n]->m_Norm.normalize();
        }

        //==== Only Move Node Attached to Isect Points ====//
        for ( t = 0 ; t < ( int )m_TVec.size() ; t++ )
        {
            if ( m_TVec[t]->m_N0->m_IsectFlag == 1 || m_TVec[t]->m_N1->m_IsectFlag == 1 || m_TVec[t]->m_N2->m_IsectFlag == 1 )
            {
                if ( m_TVec[t]->m_N0->m_IsectFlag == 0 )
                {
                    m_TVec[t]->m_N0->m_IsectFlag = 2;
                }
                if ( m_TVec[t]->m_N1->m_IsectFlag == 0 )
                {
                    m_TVec[t]->m_N1->m_IsectFlag = 2;
                }
                if ( m_TVec[t]->m_N2->m_IsectFlag == 0 )
                {
                    m_TVec[t]->m_N2->m_IsectFlag = 2;
                }
            }
        }


        //==== Move Each Node ====//
        double moveFract = 0.001;
        for ( n = 0 ; n < ( int )m_NVec.size() ; n++ )
        {
            if ( m_NVec[n]->m_IsectFlag == 1 )
            {
                vec3d offPnt = m_NVec[n]->m_Pnt + offVec[n] * moveFract;

                double moveDist = offVec[n].mag() * moveFract;
                vec3d segPnt1 = offPnt + m_NVec[n]->m_Norm * moveDist;
                vec3d segPnt2 = offPnt - m_NVec[n]->m_Norm * moveDist;

                //==== Put the Point Back on the Surface =====//
                vector< vec3d > ipVec;

                if ( m_NVec[n]->m_IsectFlag == 2 )
                {
                    saveMesh->m_TBox.SegIntersect( segPnt1, segPnt2, ipVec );
                    if ( ipVec.size() )
                    {
                        m_NVec[n]->m_Pnt = ipVec[0];
                    }
                }
                else if ( m_NVec[n]->m_IsectFlag == 1 )
                {
                    m_NVec[n]->m_Pnt = ProjectOnISectPairs( offPnt, m_ISectPairs );
                }
            }
        }


    }


}

vec3d TMesh::ProjectOnISectPairs( vec3d & offPnt, vector< vec3d > & pairVec )
{
    int i;
    double t;
    double closeDist = 1e06;
    vec3d closePnt;

    for ( i = 0 ; i < ( int )pairVec.size() ; i += 2 )
    {
        double d = pointSegDistSquared( offPnt, pairVec[i], pairVec[i + 1], &t );
        if ( d < closeDist )
        {
            closeDist = d;
            closePnt = pairVec[i] + ( pairVec[i + 1] - pairVec[i] ) * t;
        }

    }

    return closePnt;
}

void TMesh::MergeSplitNodes( vector< TTri* > & triVec, vector< TNode* > & nodeVec )
{
    int s, t;

    //==== Find All Exterior and Split Tris =====//
    for ( t = 0 ; t < ( int )m_TVec.size() ; t++ )
    {
        TTri* tri = m_TVec[t];
        if ( tri->m_SplitVec.size() )
        {
            for ( s = 0 ; s < ( int )tri->m_SplitVec.size() ; s++ )
            {
                if ( !tri->m_SplitVec[s]->m_InteriorFlag )
                {
                    triVec.push_back( tri->m_SplitVec[s] );
                }
            }
        }
        else if ( !tri->m_InteriorFlag )
        {
            triVec.push_back( tri );
        }
    }

    //==== Look Thru nVecs and Redirect Dumplicates ====//
    for ( t = 0 ; t < ( int )triVec.size() ; t++ )
    {
        triVec[t]->m_N0 = CheckDupOrAdd( triVec[t]->m_N0, nodeVec );
        triVec[t]->m_N1 = CheckDupOrAdd( triVec[t]->m_N1, nodeVec );
        triVec[t]->m_N2 = CheckDupOrAdd( triVec[t]->m_N2, nodeVec );
    }

    //==== Remove Any Bogus Tris ====//
    vector< TTri* > goodTriVec;
    for ( t = 0 ; t < ( int )triVec.size() ; t++ )
    {
        TTri* ttri = triVec[t];
        if ( ttri->m_N0->m_ID != ttri->m_N1->m_ID &&
                ttri->m_N0->m_ID != ttri->m_N2->m_ID &&
                ttri->m_N1->m_ID != ttri->m_N2->m_ID )
        {
            goodTriVec.push_back( ttri );
        }
    }
    triVec = goodTriVec;

}

//==== Check if Dupicate Node - if Not Add ====//
TNode* TMesh::CheckDupOrAdd( TNode* node, vector< TNode* > & nodeVec, double tol )
{
    for ( int i = 0 ; i < ( int )nodeVec.size() ; i++ )
    {
        TNode* n = nodeVec[i];
        if ( fabs( n->m_Pnt.x() - node->m_Pnt.x() ) < tol )
            if ( fabs( n->m_Pnt.y() - node->m_Pnt.y() ) < tol )
                if ( fabs( n->m_Pnt.z() - node->m_Pnt.z() ) < tol )
                {
                    node->m_ID = i;
                    return n;
                }
    }

    //==== Add To List ====//
    node->m_ID = nodeVec.size();
    nodeVec.push_back( node );
    return node;
}

//==== Check if Dupicate Node - if Not Add ====//
TNode* TMesh::CheckDupOrCreate( vec3d & p, vector< TNode* > & nodeVec, double tol )
{
    for ( int i = 0 ; i < ( int )nodeVec.size() ; i++ )
    {
        TNode* n = nodeVec[i];
        if ( fabs( n->m_Pnt.x() - p.x() ) < tol )
            if ( fabs( n->m_Pnt.y() - p.y() ) < tol )
                if ( fabs( n->m_Pnt.z() - p.z() ) < tol )
                {
                    return n;
                }
    }

    //==== Add To List ====//
    TNode* node = new TNode();
    node->m_Pnt = p;
    node->m_ID = nodeVec.size();
    nodeVec.push_back( node );
    return node;
}

double TMesh::Rand01()
{
    double val = ( double )rand() / ( double )RAND_MAX;
    return val;
}

void TMesh::StressTest()
{
    srand ( 2 );

    for ( int i = 0 ; i < 10000000 ; i++ )
    {
        if ( i % 10000 == 0 )
        {
            printf( "Stress Test Iter = %d\n", i );
        }
        TTri* t0 = new TTri();
        t0->m_N0   = new TNode();
        t0->m_N1   = new TNode();
        t0->m_N2   = new TNode();
        TTri* t1 = new TTri();
        t1->m_N0   = new TNode();
        t1->m_N1   = new TNode();
        t1->m_N2   = new TNode();

        t0->m_N0->m_Pnt = vec3d( Rand01(), Rand01(), Rand01() );
        t0->m_N1->m_Pnt = vec3d( Rand01(), Rand01(), Rand01() );
        t0->m_N2->m_Pnt = vec3d( Rand01(), Rand01(), Rand01() );
        t1->m_N0->m_Pnt = vec3d( Rand01(), Rand01(), Rand01() );
        t1->m_N1->m_Pnt = vec3d( Rand01(), Rand01(), Rand01() );
        t1->m_N2->m_Pnt = vec3d( Rand01(), Rand01(), Rand01() );

        vec3d e0;
        vec3d e1;
        int coplanarFlag;
        int iflag = tri_tri_intersect_with_isectline(
                        t0->m_N0->m_Pnt.v, t0->m_N1->m_Pnt.v, t0->m_N2->m_Pnt.v,
                        t1->m_N0->m_Pnt.v, t1->m_N1->m_Pnt.v, t1->m_N2->m_Pnt.v,
                        &coplanarFlag, e0.v, e1.v );

        if ( iflag && !coplanarFlag )
        {
            TEdge* ie0 = new TEdge();
            ie0->m_N0 = new TNode();
            ie0->m_N0->m_Pnt = e0;
            ie0->m_N1 = new TNode();
            ie0->m_N1->m_Pnt = e1;

            TEdge* ie1 = new TEdge();
            ie1->m_N0 = new TNode();
            ie1->m_N0->m_Pnt = e0;
            ie1->m_N1 = new TNode();
            ie1->m_N1->m_Pnt = e1;

            if ( dist( e0, e1 ) > 0.000001 )
            {
                t0->m_ISectEdgeVec.push_back( ie0 );
                t1->m_ISectEdgeVec.push_back( ie1 );
            }
        }
        t0->SplitTri();
        t1->SplitTri();

        delete t0->m_N0;
        delete t0->m_N1;
        delete t0->m_N2;
        delete t0;
        delete t1->m_N0;
        delete t1->m_N1;
        delete t1->m_N2;
        delete t1;

    }

    int done = 23;


}

void TMesh::MakeNodePntUW()
{
    for ( int t = 0 ; t < ( int )m_TVec.size() ; t++ )
    {
        TTri* tri = m_TVec[t];

        if ( tri->m_SplitVec.size() )
        {
            for ( int st = 0 ; st < ( int )tri->m_SplitVec.size() ; st++ )
            {
                TTri* stri = tri->m_SplitVec[st];
                stri->m_N0->MakePntUW();
                stri->m_N1->MakePntUW();
                stri->m_N2->MakePntUW();
            }
        }
        else
        {
            tri->m_N0->MakePntUW();
            tri->m_N1->MakePntUW();
            tri->m_N2->MakePntUW();
        }

        for ( int e = 0 ; e < ( int )tri->m_ISectEdgeVec.size() ; e++ )
        {
            tri->m_ISectEdgeVec[e]->m_N0->MakePntUW();
            tri->m_ISectEdgeVec[e]->m_N1->MakePntUW();
        }
    }
}

void TMesh::MakeNodePntXYZ()
{
    for ( int t = 0 ; t < ( int )m_TVec.size() ; t++ )
    {
        TTri* tri = m_TVec[t];

        if ( tri->m_SplitVec.size() )
        {
            for ( int st = 0 ; st < ( int )tri->m_SplitVec.size() ; st++ )
            {
                TTri* stri = tri->m_SplitVec[st];
                stri->m_N0->MakePntXYZ();
                stri->m_N1->MakePntXYZ();
                stri->m_N2->MakePntXYZ();
            }
        }
        else
        {
            tri->m_N0->MakePntXYZ();
            tri->m_N1->MakePntXYZ();
            tri->m_N2->MakePntXYZ();
        }

        for ( int e = 0 ; e < ( int )tri->m_ISectEdgeVec.size() ; e++ )
        {
            tri->m_ISectEdgeVec[e]->m_N0->MakePntXYZ();
            tri->m_ISectEdgeVec[e]->m_N1->MakePntXYZ();
        }
    }
}
vec3d TMesh::CompPnt( const vec3d & uw_pnt )
{
    // Search through uw pnts to figure out which quad the uw_pnt is in

    if ( m_UWPnts.size() == 0 )
    {
        return vec3d();
    }

    int start_u, start_v, i, j;
    vec3d p0, p1, p2, p3;

    start_u = start_v = 0;

    // Find Starting U pnt
    for ( i = 0 ; i < ( int )m_UWPnts.size() - 1 ; i++ )
    {
        if ( uw_pnt.x() > m_UWPnts[i][0].x() )
        {
            start_u = i;
        }
        else
        {
            break;
        }
    }

    // Find Starting V pnt
    for ( j = 0 ; j < ( int )m_UWPnts[start_u].size() - 1; j++ )
    {
        if ( uw_pnt.y() > m_UWPnts[start_u][j].y() )
        {
            start_v = j;
        }
        else
        {
            break;
        }
    }

    p0 = m_XYZPnts[start_u][start_v];
    p1 = m_XYZPnts[start_u + 1][start_v];
    p2 = m_XYZPnts[start_u + 1][start_v + 1];
    p3 = m_XYZPnts[start_u][start_v + 1];

    vector<double> weights;

    BilinearWeights( m_UWPnts[start_u][start_v], m_UWPnts[start_u + 1][start_v + 1], uw_pnt, weights );

    if ( weights.size() != 4 )
    {
        return vec3d();
    }

    return ( p0 * weights[0] + p1 * weights[1] + p2 * weights[2] + p3 * weights[3] );

}
