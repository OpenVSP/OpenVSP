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

#include "tri_tri_intersect.h"

#include "Geom.h"
#include "SubSurfaceMgr.h"
#include "PntNodeMerge.h"
#include "VspCurve.h" // for #define TMAGIC

#include "triangle.h"
#include "triangle_api.h"


#include <math.h>

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
    m_UWPnt = vec3d( -1, -1, 0 );
}

TNode::~TNode()
{


}

void TNode::CopyFrom( const TNode* node )
{
    m_Pnt = node->m_Pnt;
    m_UWPnt = node->m_UWPnt;
    m_XYZFlag = node->GetXYZFlag();
    m_CoordInfo = node->GetCoordInfo();
    m_IsectFlag = node->m_IsectFlag;
    m_ID = node->m_ID;
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

// Sort edges in increasing U (du > 0), unless du is large, then flip.
// For the case of a periodic wake (duct), one edge must close the curve and its u parameter
// will jump from nearly the largest parameter back to zero.  Detect the large positive jump
// and reverse the sort in this case.
void TEdge::SortNodesByU()
{
    double du = m_N1->m_UWPnt.x() - m_N0->m_UWPnt.x();
    if ( ( du < 0 && du > -1 ) || du > 1 )
    {
        TNode * ntmp = m_N1;
        m_N1 = m_N0;
        m_N0 = ntmp;
    }
}

//=======================================================================//
//=======================================================================//
//=======================================================================//
TetraMassProp::TetraMassProp( const string& id, double denIn, const vec3d& p0, const vec3d& p1, const vec3d& p2, const vec3d& p3 )
{
    m_CompId = id;
    m_Density = denIn;

    m_v0 = vec3d( 0, 0, 0 );
    m_v1 = p1 - p0;
    m_v2 = p2 - p0;
    m_v3 = p3 - p0;

    m_CG = m_v1 + m_v2 + m_v3;
    m_CG = ( m_CG * 0.25 ) + p0;

    m_Vol  = tetra_volume( m_v1, m_v2, m_v3 );
    m_Mass = m_Density * std::abs( m_Vol );

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


void TetraMassProp::SetPointMass( double massIn, const vec3d& pos )
{
    m_CompId = "NONE";
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
TriShellMassProp::TriShellMassProp( const string& id, double mass_area_in, const vec3d& p0, const vec3d& p1, const vec3d& p2 )
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

//===========================================================================================================//
//================================================ DegenGeom ================================================//
//===========================================================================================================//

DegenGeomTetraMassProp::DegenGeomTetraMassProp( const string& id, const vec3d& p0, const vec3d& p1, const vec3d& p2, const vec3d& p3 )
{
    m_CompId = id;

    m_v0 = vec3d( 0, 0, 0 );
    m_v1 = p1 - p0;
    m_v2 = p2 - p0;
    m_v3 = p3 - p0;

    m_CG = m_v1 + m_v2 + m_v3;
    m_CG = ( m_CG * 0.25 ) + p0;

    m_Vol  = std::abs( tetra_volume( m_v1, m_v2, m_v3 ) );

    double Ix = m_Vol / 10.0 * ( m_v1.x() * m_v1.x() + m_v2.x() * m_v2.x() + m_v3.x() * m_v3.x() +
                                 m_v1.x() * m_v2.x() + m_v1.x() * m_v3.x() + m_v2.x() * m_v3.x() );

    double Iy = m_Vol / 10.0 * ( m_v1.y() * m_v1.y() + m_v2.y() * m_v2.y() + m_v3.y() * m_v3.y() +
                                 m_v1.y() * m_v2.y() + m_v1.y() * m_v3.y() + m_v2.y() * m_v3.y() );

    double Iz = m_Vol / 10.0 * ( m_v1.z() * m_v1.z() + m_v2.z() * m_v2.z() + m_v3.z() * m_v3.z() +
                                 m_v1.z() * m_v2.z() + m_v1.z() * m_v3.z() + m_v2.z() * m_v3.z() );

    m_Ixx = Iy + Iz;
    m_Iyy = Ix + Iz;
    m_Izz = Ix + Iy;

    m_Ixy = m_Vol / 20.0 * ( 2.0 * ( m_v1.x() * m_v1.y() + m_v2.x() * m_v2.y() + m_v3.x() * m_v3.y() ) +
                             m_v1.x() * m_v2.y() + m_v2.x() * m_v1.y() + m_v1.x() * m_v3.y() + m_v3.x() * m_v1.y() + m_v2.x() * m_v3.y() + m_v3.x() * m_v2.y() );

    m_Iyz = m_Vol / 20.0 * ( 2.0 * ( m_v1.y() * m_v1.z() + m_v2.y() * m_v2.z() + m_v3.y() * m_v3.z() ) +
                             m_v1.y() * m_v2.z() + m_v2.y() * m_v1.z() + m_v1.y() * m_v3.z() + m_v3.y() * m_v1.z() + m_v2.y() * m_v3.z() + m_v3.y() * m_v2.z() );

    m_Ixz = m_Vol / 20.0 * ( 2.0 * ( m_v1.x() * m_v1.z() + m_v2.x() * m_v2.z() + m_v3.x() * m_v3.z() ) +
                             m_v1.x() * m_v2.z() + m_v2.x() * m_v1.z() + m_v1.x() * m_v3.z() + m_v3.x() * m_v1.z() + m_v2.x() * m_v3.z() + m_v3.x() * m_v2.z() );

}


DegenGeomTriShellMassProp::DegenGeomTriShellMassProp( const string& id, const vec3d& p0, const vec3d& p1, const vec3d& p2 )
{
    m_CompId = id;

    m_CG = ( p0 + p1 + p2 ) * ( 1.0 / 3.0 );

    m_v0 = p0 - m_CG;
    m_v1 = p1 - m_CG;
    m_v2 = p2 - m_CG;

    m_TriArea = area( m_v0, m_v1, m_v2 );

    double Ix = m_TriArea / 10.0 * ( m_v0.x() * m_v0.x() + m_v1.x() * m_v1.x() + m_v2.x() * m_v2.x() +
                                     m_v0.x() * m_v1.x() + m_v0.x() * m_v2.x() + m_v1.x() * m_v2.x() );

    double Iy = m_TriArea / 10.0 * ( m_v0.y() * m_v0.y() + m_v1.y() * m_v1.y() + m_v2.y() * m_v2.y() +
                                     m_v0.y() * m_v1.y() + m_v0.y() * m_v2.y() + m_v1.y() * m_v2.y() );

    double Iz = m_TriArea / 10.0 * ( m_v0.z() * m_v0.z() + m_v1.z() * m_v1.z() + m_v2.z() * m_v2.z() +
                                     m_v0.z() * m_v1.z() + m_v0.z() * m_v2.z() + m_v1.z() * m_v2.z() );

    m_Ixx = Iy + Iz;
    m_Iyy = Ix + Iz;
    m_Izz = Ix + Iy;

    m_Ixy = m_TriArea / 20.0 * ( 2.0 * ( m_v0.x() * m_v0.y() + m_v1.x() * m_v1.y() + m_v2.x() * m_v2.y() ) +
                                 m_v0.x() * m_v1.y() + m_v1.x() * m_v0.y() + m_v0.x() * m_v2.y() + m_v2.x() * m_v0.y() + m_v1.x() * m_v2.y() + m_v2.x() * m_v1.y() );

    m_Iyz = m_TriArea / 20.0 * ( 2.0 * ( m_v0.y() * m_v0.z() + m_v1.y() * m_v1.z() + m_v2.y() * m_v2.z() ) +
                                 m_v0.y() * m_v1.z() + m_v1.y() * m_v0.z() + m_v0.y() * m_v2.z() + m_v2.y() * m_v0.z() + m_v1.y() * m_v2.z() + m_v2.y() * m_v1.z() );

    m_Ixz = m_TriArea / 20.0 * ( 2.0 * ( m_v0.x() * m_v0.z() + m_v1.x() * m_v1.z() + m_v2.x() * m_v2.z() ) +
                                 m_v0.x() * m_v1.z() + m_v1.x() * m_v0.z() + m_v0.x() * m_v2.z() + m_v2.x() * m_v0.z() + m_v1.x() * m_v2.z() + m_v2.x() * m_v1.z() );

}



//===========================================================================================================//
//============================================== END DegenGeom ==============================================//
//===========================================================================================================//


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
    m_MaterialID = 0;
    m_SurfCfdType = vsp::CFD_NORMAL;
    m_ThickSurf = true;
    m_MassPrior = 0;
    m_Density = 0;
    m_ShellMassArea = 0;
    m_ShellFlag = false;
    m_DeleteMeFlag = false;
    m_TheoArea = m_WetArea = 0.0;
    m_TheoVol    = m_WetVol = 0.0;
    m_HalfBoxFlag = false;
    m_SurfNum = 0;
    m_AreaCenter = vec3d(0,0,0);
    m_GuessVol = 0;
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

        tri->m_N0->m_UWPnt = m->m_TVec[i]->m_N0->m_UWPnt;
        tri->m_N1->m_UWPnt = m->m_TVec[i]->m_N1->m_UWPnt;
        tri->m_N2->m_UWPnt = m->m_TVec[i]->m_N2->m_UWPnt;

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
                if ( !s_tri->m_IgnoreTriFlag )
                {
                    AddTri( s_tri->m_N0, s_tri->m_N1, s_tri->m_N2, s_tri->m_Norm );
                    m_TVec.back()->m_Tags = s_tri->m_Tags;
                }
            }
        }
        else
        {
            if ( !orig_tri->m_IgnoreTriFlag )
            {
                AddTri( orig_tri->m_N0, orig_tri->m_N1, orig_tri->m_N2, orig_tri->m_Norm );
                m_TVec.back()->m_Tags = orig_tri->m_Tags;
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
    m_SurfCfdType = m->m_SurfCfdType;
    m_ThickSurf = m->m_ThickSurf;

    m_TheoArea   = m->m_TheoArea;
    m_WetArea    = m->m_WetArea;
    m_TheoVol    = m->m_TheoVol;
    m_GuessVol   = m->m_GuessVol;
    m_WetVol     = m->m_WetVol;

    m_MassPrior = m->m_MassPrior;
    m_Density   = m->m_Density;
    m_ShellFlag = m->m_ShellFlag;
    m_ShellMassArea = m->m_ShellMassArea;

    m_SurfType = m->m_SurfType;

    m_UWPnts = m->m_UWPnts;
    m_XYZPnts = m->m_XYZPnts;

    m_AreaCenter = m->m_AreaCenter;
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

    xmlNodePtr iter_node;

    iter_node = node->xmlChildrenNode;

    int i = 0;
    while( iter_node != NULL )
    {
        if ( !xmlStrcmp( iter_node->name, ( const xmlChar * )"Tri" ) )
        {
            tri = XmlUtil::GetVectorVec3dNode( iter_node );
            m_TVec[i] = new TTri();
            // Create Nodes
            m_TVec[i]->m_N0 = new TNode();
            m_TVec[i]->m_N1 = new TNode();
            m_TVec[i]->m_N2 = new TNode();

            m_NVec.push_back( m_TVec[i]->m_N0 );
            m_NVec.push_back( m_TVec[i]->m_N1 );
            m_NVec.push_back( m_TVec[i]->m_N2 );


            // Insert Data
            m_TVec[i]->m_N0->m_Pnt = tri[0];
            m_TVec[i]->m_N1->m_Pnt = tri[1];
            m_TVec[i]->m_N2->m_Pnt = tri[2];
            m_TVec[i]->m_Norm = tri[3];
            tri.clear();

            i++;
        }
        iter_node = iter_node->next;
    }
}

void TMesh::LoadGeomAttributes( const Geom* geomPtr )
{
    /*color       = geomPtr->getColor();
    materialID    = geomPtr->getMaterialID();*/
    m_PtrID        = geomPtr->GetID();

    m_MassPrior     = geomPtr->m_MassPrior();
    m_Density       = geomPtr->m_Density();
    m_ShellMassArea = geomPtr->m_MassArea();
    m_ShellFlag     = geomPtr->m_ShellFlag();

    if ( geomPtr->m_NegativeVolumeFlag() )
    {
        m_SurfCfdType = vsp::CFD_NEGATIVE;
    }

    //==== Check for Alternate Output Name ====//
    m_NameStr   = geomPtr->GetName();
}

void TMesh::RemoveIsectEdges()
{
    for ( int j = 0 ; j < ( int )m_TVec.size() ; j++ )
    {
        TTri* tri = m_TVec[j];
        for ( int e = 0 ; e < ( int )tri->m_ISectEdgeVec.size() ; e++ )
        {
            delete tri->m_ISectEdgeVec[e]->m_N0;
            delete tri->m_ISectEdgeVec[e]->m_N1;
            delete tri->m_ISectEdgeVec[e];
        }
        tri->m_ISectEdgeVec.erase( tri->m_ISectEdgeVec.begin(), tri->m_ISectEdgeVec.end() );
    }
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

    bool match_flag = false;
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
        if ( ! match_flag )
        {
            break;
        }
    }

    if ( match_flag )
    {
        MergeTMeshes( tm );

        CheckIfClosed();                // Recheck For NonClosed Tris

        tm->m_DeleteMeFlag = true;
    }
}

void TMesh::MergeTMeshes( TMesh* tm )
{
    for ( int t = 0 ; t < ( int )tm->m_TVec.size() ; t++ )
    {
        TTri* tri = tm->m_TVec[t];
        AddTri( tri );
        m_TVec.back()->m_InvalidFlag = 0;
    }

    for ( int i = 0 ; i < ( int )m_NonClosedTriVec.size() ; i++ )
    {
        m_NonClosedTriVec[i]->m_InvalidFlag = 0;
    }
    m_NonClosedTriVec.clear();
}

void TMesh::Intersect( TMesh* tm, bool UWFlag )
{
    m_TBox.Intersect( &tm->m_TBox, UWFlag );
}

bool TMesh::CheckIntersect( TMesh* tm )
{
    return m_TBox.CheckIntersect( &tm->m_TBox );
}

double TMesh::MinDistance( TMesh* tm, double curr_min_dist )
{
    return m_TBox.MinDistance( &tm->m_TBox, curr_min_dist );
}

void TMesh::Split()
{
    int t;
    for ( t = 0 ; t < ( int )m_TVec.size() ; t++ )
    {
        m_TVec[t]->SplitTri();
    }
}

bool TMesh::DecideIgnoreTri( int aType, const vector < int > & bTypes, const vector < bool > & thicksurf, const vector < bool > & aInB )
{
    // Always delete Stiffener tris
    if ( aType == vsp::CFD_STIFFENER )
    {
        return true;
    }

    for ( int b = 0 ; b < ( int )aInB.size() ; b++ )
    {
        bool aInThisB = aInB[b];
        int bType = bTypes[b];
        bool bThick = thicksurf[b];

        // Can make absolute decisions about deleting a triangle or not in the cases below
        if ( aInThisB && bThick )
        {
            // Normal(Positive) inside another Normal, or Negative inside another Negative
            if ( aType == bType && ( aType != vsp::CFD_TRANSPARENT && aType != vsp::CFD_STRUCTURE ) )
            {
                return true;
            }
            // Always delete Normal tris inside Negative surfaces
            else if ( aType == vsp::CFD_NORMAL && bType == vsp::CFD_NEGATIVE )
            {
                return true;
            }
            // Never delete Transparent tris inside Negative surfaces
            else if ( aType == vsp::CFD_TRANSPARENT && bType == vsp::CFD_NEGATIVE )
            {
                return false;
            }
            // Always delete Structure tris inside Negative surfaces
            else if ( aType == vsp::CFD_STRUCTURE && bType == vsp::CFD_NEGATIVE )
            {
                return true;
            }
        }
    }

    // Default condition for ignoretri.
    // The default value is applied for a triangle that is not inside
    // any other object.  I.e. an isolated thing in 'free space'.
    //
    // vsp::CFD_NORMAL, vsp::CFD_TRANSPARENT
    int ignoretri = false;

    // Flip sense of default value.  These things do not exist in 'free space'.
    if ( aType == vsp::CFD_NEGATIVE ||
         aType == vsp::CFD_STRUCTURE ||
         aType == vsp::CFD_STIFFENER )   // Stiffener is special case -- always true previously.
    {
        ignoretri = true;
    }

    // Check non-absolute cases
    for ( int b = 0 ; b < ( int )aInB.size() ; b++ )
    {
        bool aInThisB = aInB[b];
        int bType = bTypes[b];
        bool bThick = thicksurf[b];

        if ( aInThisB && bThick )
        {
            if ( ( aType == vsp::CFD_NEGATIVE || aType == vsp::CFD_STRUCTURE ) && bType == vsp::CFD_NORMAL )
            {
                return false;
            }
            else if ( aType == vsp::CFD_TRANSPARENT && bType == vsp::CFD_NORMAL )
            {
                return true;
            }
        }
    }

    return ignoretri;
}

void TMesh::SetIgnoreTriFlag( vector< TMesh* >& meshVec, const vector < int > & bTypes, const vector < bool > & thicksurf )
{
    for ( int t = 0 ; t < ( int )m_TVec.size() ; t++ )
    {
        TTri* tri = m_TVec[t];

        //==== Do Interior Tris ====//
        if ( tri->m_SplitVec.size() )
        {
            tri->m_IgnoreTriFlag = true;
            for ( int s = 0 ; s < ( int )tri->m_SplitVec.size() ; s++ )
            {
                tri->m_SplitVec[s]->m_IgnoreTriFlag = DecideIgnoreTri( m_SurfCfdType, bTypes, thicksurf, tri->m_SplitVec[s]->m_insideSurf );
            }
        }
        else
        {
            tri->m_IgnoreTriFlag = DecideIgnoreTri( m_SurfCfdType, bTypes, thicksurf, tri->m_insideSurf );
        }
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
    tri->m_IgnoreTriFlag = false;
    int prior = -1;

    vec3d dir( 1.0, 0.000001, 0.000001 );

    int nmesh = meshVec.size();
    tri->m_insideSurf.resize( nmesh, false );

    for ( int m = 0 ; m < ( int )meshVec.size() ; m++ )
    {
        if ( meshVec[m] != this && meshVec[m]->m_ThickSurf )
        {
            vector<double > tParmVec;
            meshVec[m]->m_TBox.RayCast( orig, dir, tParmVec );
            if ( tParmVec.size() % 2 )
            {
                tri->m_insideSurf[m] = true;

                // Priority assignment for wave drag.  Mass prop may need some adjustments.
                if ( meshVec[m]->m_MassPrior > prior ) // Should possibly check that priority is only for vsp::CFD_NORMAL
                {
                    tri->m_ID = meshVec[m]->m_PtrID;
                    tri->m_Density = meshVec[m]->m_Density;
                    prior = meshVec[m]->m_MassPrior;
                }
            }
        }
    }
}

double TMesh::ComputeTheoArea()
{
    m_TheoArea = 0;
    m_TagTheoAreaVec.clear();
    unsigned int ntags = SubSurfaceMgr.GetNumTags() - 1;
    if ( ntags > 0 )
    {
        m_TagTheoAreaVec.resize( ntags, 0.0);
    }

    for ( int t = 0 ; t < ( int )m_TVec.size() ; t++ )
    {
        double area = m_TVec[t]->ComputeArea();
        m_TheoArea += area;
        if ( ntags > 0 )
        {
            int itag = SubSurfaceMgr.GetTag(m_TVec[t]->m_Tags) - 1;
            if ( itag >= 0 && itag < ntags )
            {
                m_TagTheoAreaVec[itag] += area;
            }
        }
    }
    return m_TheoArea;
}

double TMesh::ComputeWetArea()
{
    m_WetArea = 0;
    m_AreaCenter = vec3d(0,0,0);
    m_TagWetAreaVec.clear();
    int ntags = SubSurfaceMgr.GetNumTags() - 1;
    if ( ntags > 0 )
    {
        m_TagWetAreaVec.resize( ntags, 0.0);
    }

    for ( int t = 0 ; t < ( int )m_TVec.size() ; t++ )
    {
        TTri* tri = m_TVec[t];

        // TMesh::SubTag guarantees that split tris have same tags as normal tris.
        // So, just look up tag index once per tri.
        int itag = SubSurfaceMgr.GetTag( tri->m_Tags ) - 1;

        //==== Do Interior Tris ====//
        if ( tri->m_SplitVec.size() )
        {
            for ( int s = 0 ; s < ( int )tri->m_SplitVec.size() ; s++ )
            {
                if ( !tri->m_SplitVec[s]->m_IgnoreTriFlag )
                {
                    double area = tri->m_SplitVec[s]->ComputeArea();
                    m_AreaCenter = m_AreaCenter + tri->m_SplitVec[s]->ComputeCenter()*area;
                    vec3d center = tri->m_SplitVec[s]->ComputeCenter();
                    m_WetArea += area;
                    if ( itag >= 0 && itag < ntags )
                    {
                        m_TagWetAreaVec[itag] += area;
                    }
                }
            }
        }
        else if ( !tri->m_IgnoreTriFlag )
        {
            double area = tri->ComputeArea();
            m_AreaCenter = m_AreaCenter + tri->ComputeCenter()*area;
            vec3d center = tri->ComputeCenter();
            m_WetArea += tri->ComputeArea();
            if ( itag >= 0 && itag < ntags )
            {
                m_TagWetAreaVec[itag] += area;
            }
        }
    }
    m_AreaCenter = m_AreaCenter/m_WetArea;
    return m_WetArea;
}

double TMesh::ComputeWaveDragArea( const std::map< string, int > &idmap )
{
    m_WetArea = 0;
    m_AreaCenter = vec3d(0,0,0);

    for ( int t = 0 ; t < ( int )m_TVec.size() ; t++ )
    {
        TTri* tri = m_TVec[t];

        //==== Do Interior Tris ====//
        // For WaveDragArea, they should all be split tris by definition.
        if ( tri->m_SplitVec.size() )
        {
            for ( int s = 0 ; s < ( int )tri->m_SplitVec.size() ; s++ )
            {
                if ( !tri->m_SplitVec[s]->m_IgnoreTriFlag )
                {
                    double area = tri->m_SplitVec[s]->ComputeYZArea();
                    m_AreaCenter = m_AreaCenter + tri->m_SplitVec[s]->ComputeCenter()*area;
                    m_WetArea += area;

                    std::map<string, int>::const_iterator it = idmap.find( tri->m_SplitVec[s]->m_ID );
                    if ( it != idmap.end() )
                    {
                        m_CompAreaVec[ it->second ] += area;
                    }
                }
            }
        }
        else if ( !tri->m_IgnoreTriFlag )
        {
            double area = tri->ComputeYZArea();
            m_AreaCenter = m_AreaCenter + tri->ComputeCenter()*area;
            m_WetArea += area;

            std::map<string, int>::const_iterator it = idmap.find( tri->m_ID );
            if ( it != idmap.end() )
            {
                m_CompAreaVec[ it->second ] += area;
            }
        }
    }
    m_AreaCenter = m_AreaCenter/m_WetArea;
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
                if ( !tri->m_SplitVec[s]->m_IgnoreTriFlag )
                {
                    trimVol += tetra_volume( tri->m_SplitVec[s]->m_N0->m_Pnt, tri->m_SplitVec[s]->m_N1->m_Pnt, tri->m_SplitVec[s]->m_N2->m_Pnt );
                }
            }
        }
        else if ( !tri->m_IgnoreTriFlag )
        {
            trimVol += tetra_volume( tri->m_N0->m_Pnt, tri->m_N1->m_Pnt, tri->m_N2->m_Pnt );
        }
    }
    return trimVol;
}

// Wrapper
void TMesh::AddTri( const vec3d & p0, const vec3d & p1, const vec3d & p2 )
{
    double dist_tol = 1.0e-12;

    vec3d v01 = p1 - p0;
    vec3d v02 = p2 - p0;
    vec3d v12 = p2 - p1;

    if ( v01.mag() < dist_tol || v02.mag() < dist_tol || v12.mag() < dist_tol )
    {
        return;
    }

    vec3d norm = cross( v01, v02 );
    norm.normalize();

    AddTri( p0, p1, p2, norm );
}

// Base.  i.e. does m_TVec.push_back()
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

// Base
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

// Wrapper
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

// Base
void TMesh::AddTri( const TTri* tri)
{
    // Copies an existing triangle and pushes back into the existing
    TTri* new_tri = new TTri();

    new_tri->CopyFrom( tri );
    m_TVec.push_back( new_tri );
    m_NVec.push_back( new_tri->m_N0 );
    m_NVec.push_back( new_tri->m_N1 );
    m_NVec.push_back( new_tri->m_N2 );
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
    m_TBox.Reset();

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
                if ( !tri->m_SplitVec[s]->m_IgnoreTriFlag )
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
            if ( !tri->m_IgnoreTriFlag )
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
    m_IgnoreTriFlag = false;
    m_InvalidFlag  = 0;
    m_Density = 1.0;
    m_TMesh = NULL;
    m_PEArr[0] = m_PEArr[1] = m_PEArr[2] = NULL;
}

TTri::~TTri()
{
    static int cnt = 0;
    cnt++;
//printf("Tri Destruct Cnt = %d \n", cnt);
    int i;

    //==== Delete Split Edges ====//
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

void TTri::CopyFrom( const TTri* tri )
{
    m_N0 = new TNode();
    m_N1 = new TNode();
    m_N2 = new TNode();

    m_N0->CopyFrom( tri->m_N0 );
    m_N1->CopyFrom( tri->m_N1 );
    m_N2->CopyFrom( tri->m_N2 );

    m_Norm = tri->m_Norm;
    m_Density = tri->m_Density;
    m_Tags = tri->m_Tags;
    m_ID = tri->m_ID;
    m_InvalidFlag = tri->m_InvalidFlag;
    m_IgnoreTriFlag = tri->m_IgnoreTriFlag;
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

vec3d TTri::CompUW( const vec3d & pnt )
{
    vec3d w = BarycentricWeights( m_N0->m_Pnt, m_N1->m_Pnt, m_N2->m_Pnt, pnt );
    vec3d uw = w.v[0] * m_N0->m_UWPnt + w.v[1] * m_N1->m_UWPnt + w.v[2] * m_N2->m_UWPnt;

    return uw;
}

bool TTri::ShareEdge( TTri* t )
{
    double tol = 1.0e-12;

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

// was 1e-5
#define ON_EDGE_TOL 1e-5

//==== Split A Triangle Along Edges in ISectEdges Vec =====//
void TTri::SplitTri()
{
    int i, j;
    double onEdgeTol = ON_EDGE_TOL; // was 1e-5
    double uvMinTol  = 1e-3; // was 1e-3
    double uvMaxTol  = 1.0 - uvMinTol;

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
                break;
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

    // Detect if currently in uw space
    bool uwflag = false;
    if ( !m_N0->GetXYZFlag() )
    {
        uwflag = true;
    }

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
    vector< vec3d > uwVec;
    pVec.resize( m_ISectEdgeVec.size() * 2 );
    uwVec.resize( m_ISectEdgeVec.size() * 2 );
    for ( i = 0 ; i < ( int )m_ISectEdgeVec.size() ; i++ )
    {
        pVec[i * 2] = m_ISectEdgeVec[i]->m_N0->GetXYZPnt();
        pVec[i * 2 + 1] = m_ISectEdgeVec[i]->m_N1->GetXYZPnt();
        uwVec[i * 2] =  m_ISectEdgeVec[i]->m_N0->GetUWPnt();
        uwVec[i * 2 + 1] =  m_ISectEdgeVec[i]->m_N1->GetUWPnt();
    }
    vector< int > matchNodeIndex;       // Nodes Index Which Correspond to pVec
    matchNodeIndex.resize( pVec.size(), -1 );

    //==== Add Edge Nodes ====//
    for ( i = 0 ; i < ( int )pVec.size() ; i++ )
    {
        for ( j = 0 ; j < ( int )m_EVec.size() ; j++ )
        {
            bool onEdgeFlag = false;

            if ( uwflag )
            {
                onEdgeFlag = !!OnEdge( uwVec[i], m_EVec[j], onEdgeTol );
            }
            else
            {
                onEdgeFlag = !!OnEdge( pVec[i], m_EVec[j], onEdgeTol );
            }

            if ( onEdgeFlag )
            {
                //==== SplitEdge ====//
                TNode* sn = new TNode();        // New node
                sn->m_IsectFlag = 1;
                m_NVec.push_back( sn );
                matchNodeIndex[i] = m_NVec.size() - 1;
                sn->SetXYZPnt( pVec[i] );
                sn->SetUWPnt( uwVec[i] );
                if ( uwflag )
                {
                    sn->MakePntUW();
                }
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
                double d = 2 * onEdgeTol;

                if ( uwflag )
                {
                    d = dist( uwVec[i], m_NVec[j]->m_Pnt );
                }
                else
                {
                    d = dist( pVec[i], m_NVec[j]->m_Pnt );
                }

                if ( d < onEdgeTol )
                {
                    matchNodeIndex[i] = j;
                    break;
                }
            }
        }
        //==== Didn't Find One - Add New ====//
        if ( matchNodeIndex[i] == -1 )
        {
            TNode* sn = new TNode();        // New node
            sn->m_IsectFlag = 1;
            m_NVec.push_back( sn );
            matchNodeIndex[i] = m_NVec.size() - 1;
            sn->SetXYZPnt( pVec[i] );
            sn->SetUWPnt( uwVec[i] );
            if ( uwflag )
            {
                sn->MakePntUW();
            }
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

                            vec3d crossing_node = ( np0 + np1 ) * 0.5;

                            // Interpolate other coordinate set
                            if ( uwflag )
                            {
                                vec3d cn_xyz = CompPnt( crossing_node );
                                sn->SetUWPnt( crossing_node );
                                sn->SetXYZPnt( cn_xyz );
                                sn->MakePntUW();
                            }
                            else
                            {
                                vec3d cn_uw = CompUW( crossing_node );
                                sn->SetUWPnt( cn_uw );
                                sn->SetXYZPnt( crossing_node );
                            }

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
    Matrix4d rot_mat;
    int flattenAxis = 0;
    if ( uwflag )
    {
        flattenAxis = 2;
    }
    else
    {
        // Rotate All Nodes into YZ Plane
        // Find angle between triangle normal and the YZ Plane
        double costheta = dot(m_Norm,vec3d(1,0,0));

        // Find axis orthogonal to both the triangles normal and YZ Plane normal
        vec3d axis = cross(m_Norm,vec3d(1,0,0));

        // Rotate Points
        rot_mat.rotate(std::acos(costheta), axis);

        for ( unsigned int n=0; n < m_NVec.size(); n++ )
        {
            m_NVec[n]->m_Pnt = rot_mat.xform(m_NVec[n]->m_Pnt);
        }
    }

    //==== Use Triangle to Split Tri ====//
    TriangulateSplit( flattenAxis );

    if ( !uwflag )
    {
        // Rotate Points Back
        rot_mat.affineInverse();
        for ( unsigned int n=0; n < m_NVec.size(); n++ )
        {
            m_NVec[n]->m_Pnt = rot_mat.xform(m_NVec[n]->m_Pnt);
        }
    }

    //=== Orient Tris to Match Normal ====//
    for ( i = 0 ; i < ( int )m_SplitVec.size() ; i++ )
    {
        TTri* t = m_SplitVec[i];
        vec3d d01 = t->m_N0->GetXYZPnt() - t->m_N1->GetXYZPnt();
        vec3d d21 = t->m_N2->GetXYZPnt() - t->m_N1->GetXYZPnt();

        vec3d cx = cross( d21, d01 );

        if ( dot( cx, m_Norm) < 0.0 )
        {
            TNode* tmp = t->m_N1;
            t->m_N1 = t->m_N2;
            t->m_N2 = tmp;
        }
    }
}

void TTri::TriangulateSplit( int flattenAxis )
{
    int i, j;

    //==== Dump Into Triangle ====//
    context* ctx;
    triangleio in, out;
    int tristatus = TRI_NULL;

    // init
    ctx = triangle_context_create();

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

    //==== Load Points into Triangle Struct ====//
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
                double del = std::abs( in.pointlist[i * 2] - in.pointlist[j * 2] ) +
                             std::abs( in.pointlist[i * 2 + 1] - in.pointlist[j * 2 + 1] );

                if ( del < 1e-10 )
                {
                    dupFlag = 1;
                }
            }

        if ( !dupFlag )
        {
            char cmdline[] = "zpQ";

            //==== Constrained Delaunay Trianglulation ====//
            tristatus = triangle_context_options( ctx, cmdline );
            if ( tristatus != TRI_OK ) printf( "triangle_context_options Error\n" );

            // Triangulate the polygon
            tristatus = triangle_mesh_create( ctx, &in );
            if ( tristatus != TRI_OK ) printf( "triangle_mesh_create Error\n" );
        }
//fprintf(fp, "Triangulate in = %d out = %d \n", in.numberofpoints, out.numberofpoints );
    }

    if ( tristatus == TRI_OK )
    {
        triangle_mesh_copy( ctx, &out, 1, 1 );

        //==== Load Triangles if No New Point Created ====//
        cnt = 0;
        for ( i = 0; i < out.numberoftriangles; i++ )
        {
            if ( out.trianglelist[cnt] < (int)m_NVec.size() &&
                out.trianglelist[cnt + 1] < (int)m_NVec.size() &&
                out.trianglelist[cnt + 2] < (int)m_NVec.size() )
            {
                TTri* t = new TTri();
                t->m_N0 = m_NVec[out.trianglelist[cnt]];
                t->m_N1 = m_NVec[out.trianglelist[cnt + 1]];
                t->m_N2 = m_NVec[out.trianglelist[cnt + 2]];
                t->m_Tags = m_Tags; // Set split tri to have same tags as original triangle
                t->m_Norm = m_Norm;
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

    //free( in.pointlist );
    //free( in.pointattributelist );
    //free( in.pointmarkerlist );

    //free( in.trianglelist );
    //free( in.triangleattributelist );
    //free( in.trianglearealist );
    //free( in.neighborlist );

    //free( in.segmentlist );
    //free( in.segmentmarkerlist );

    //free( in.holelist );
    //free( in.regionlist );

    //free( in.edgelist );
    //free( in.edgemarkerlist );

    // cleanup
    triangle_context_destroy( ctx );
}

int TTri::OnEdge( const vec3d & p, TEdge* e, double onEdgeTol, double * t )
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

    double tn;
    if ( t == NULL )
    {
        t = &tn;
    }

    double d = pointSegDistSquared( p, e->m_N0->m_Pnt, e->m_N1->m_Pnt, t );

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

int TTri::WakeEdge()
{
    double tol = 1e-12;
    int type = GetTMeshPtr()->m_SurfType;
    if ( type == vsp::WING_SURF || type == vsp::PROP_SURF )
    {
        bool n0 = m_N0->m_UWPnt.y() <= ( TMAGIC + tol );
        bool n1 = m_N1->m_UWPnt.y() <= ( TMAGIC + tol );
        bool n2 = m_N2->m_UWPnt.y() <= ( TMAGIC + tol );

        if ( n0 && n1 )
        {
            return 1;
        }
        else if ( n1 && n2 )
        {
            return 2;
        }
        else if ( n2 && n0 )
        {
            return 3;
        }
    }
    return 0;
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
        delete m_SBoxVec[i];
    }
}

void TBndBox::Reset()
{
    for ( int i = 0 ; i < 8 ; i++ )
    {
        delete m_SBoxVec[i];
        m_SBoxVec[i] = 0;
    }

    m_Box.Reset();
    m_TriVec.clear();
}

//==== Create Oct Tree of Overlapping BndBoxes ====//
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

bool TBndBox::CheckIntersect( TBndBox* iBox  )
{
    int i, j;

    //==== Compare Bounding Boxes ====//
    if ( !Compare( m_Box, iBox->m_Box ) )
    {
        return false;
    }

    //==== Recursively Check Sub Boxes ====//
    if ( m_SBoxVec[0] )
    {
        for ( i = 0 ; i < 8 ; i++ )
        {
           if ( iBox->CheckIntersect( m_SBoxVec[i] ) )
           return true;
        }
    }
    else if ( iBox->m_SBoxVec[0] )
    {
        for ( i = 0 ; i < 8 ; i++ )
        {
            if ( iBox->m_SBoxVec[i]->CheckIntersect( this ) )
                return true;
        }
    }
    else
    {
        int coplanarFlag = 0; // Must be initialized to 0 before use in tri_tri_intersection_test_3d
        vec3d e0;
        vec3d e1;

        //==== Check All Tris In One Box Against The Other ====//
        for ( i = 0 ; i < ( int )m_TriVec.size() ; i++ )
        {
            TTri* t0 = m_TriVec[i];
            for ( j = 0 ; j < ( int )iBox->m_TriVec.size() ; j++ )
            {
                TTri* t1 = iBox->m_TriVec[j];

                int iflag = tri_tri_intersection_test_3d(
                                t0->m_N0->m_Pnt.v, t0->m_N1->m_Pnt.v, t0->m_N2->m_Pnt.v,
                                t1->m_N0->m_Pnt.v, t1->m_N1->m_Pnt.v, t1->m_N2->m_Pnt.v,
                                &coplanarFlag, e0.v, e1.v );

                if ( iflag && !coplanarFlag )
                    return true;
            }
        }
    }
    return false;
}

double TBndBox::MinDistance( TBndBox* iBox, double curr_min_dist )
{
    int i, j;

    //==== Compare Bounding Boxes ====//
    if ( !Compare( m_Box, iBox->m_Box, curr_min_dist ) )
    {
        return curr_min_dist;
    }

    //==== Recursively Check Sub Boxes ====//
    if ( m_SBoxVec[0] )
    {
        for ( i = 0 ; i < 8 ; i++ )
        {
            curr_min_dist = iBox->MinDistance( m_SBoxVec[i], curr_min_dist );
        }
    }
    else if ( iBox->m_SBoxVec[0] )
    {
        for ( i = 0 ; i < 8 ; i++ )
        {
            curr_min_dist = iBox->m_SBoxVec[i]->MinDistance( this, curr_min_dist );
        }
    }
    //==== Check All Points Against Other Points ====//
    else
    {
        for ( i = 0 ; i < ( int )m_TriVec.size() ; i++ )
        {
            TTri* t0 = m_TriVec[i];
            for ( j = 0 ; j < ( int )iBox->m_TriVec.size() ; j++ )
            {

                TTri* t1 = iBox->m_TriVec[j];
                double d = tri_tri_min_dist( t0->m_N0->m_Pnt, t0->m_N1->m_Pnt, t0->m_N2->m_Pnt,
                                             t1->m_N0->m_Pnt, t1->m_N1->m_Pnt, t1->m_N2->m_Pnt);

                if ( d < curr_min_dist )
                    curr_min_dist = d;
            }
        }
    }

    return curr_min_dist;
}


void TBndBox::Intersect( TBndBox* iBox, bool UWFlag )
{
    int i;

    double tol = 1e-6; // was 1e-6

    if ( !Compare( m_Box, iBox->m_Box ) )
    {
        return;
    }

    if ( m_SBoxVec[0] )
    {
        for ( i = 0 ; i < 8 ; i++ )
        {
            iBox->Intersect( m_SBoxVec[i], UWFlag );
        }
    }
    else if ( iBox->m_SBoxVec[0] )
    {
        for ( i = 0 ; i < 8 ; i++ )
        {
            iBox->m_SBoxVec[i]->Intersect( this, UWFlag );
        }
    }
    else
    {
        for ( i = 0 ; i < ( int )m_TriVec.size() ; i++ )
        {
            TTri* t0 = m_TriVec[i];
            for ( int j = 0 ; j < ( int )iBox->m_TriVec.size() ; j++ )
            {
                TTri* t1 = iBox->m_TriVec[j];

                int coplanarFlag = 0; // Must be initialized to 0 before use in tri_tri_intersection_test_3d
                vec3d e0;
                vec3d e1;

                int iflag = tri_tri_intersection_test_3d(
                                t0->m_N0->m_Pnt.v, t0->m_N1->m_Pnt.v, t0->m_N2->m_Pnt.v,
                                t1->m_N0->m_Pnt.v, t1->m_N1->m_Pnt.v, t1->m_N2->m_Pnt.v,
                                &coplanarFlag, e0.v, e1.v );

                if ( iflag && !coplanarFlag )
                {
                    if ( UWFlag )
                    {
                        if ( dist( e0, e1 ) > tol ) // was 1e-6
                        {
                            // Figure out with tri has xyz info
                            TTri* tri;
                            int d_info = TNode::HAS_XYZ; // desired info number
                            if ( ( t0->m_N0->GetCoordInfo() & d_info ) == d_info &&  ( t0->m_N1->GetCoordInfo() & d_info ) == d_info
                                    && ( t0->m_N2->GetCoordInfo() & d_info ) == d_info )
                            {
                                tri = t0;
                            }
                            else
                            {
                                tri = t1;
                            }
                            // Use Bilinear interpolation to convert edge uw points to xyz points
                            vec3d e0xyz = tri->CompPnt( e0 );
                            vec3d e1xyz = tri->CompPnt( e1 );

                            // Create the new edges

                            TEdge* ie0 = new TEdge();
                            int info = TNode::HAS_UW | TNode::HAS_XYZ;
                            ie0->m_N0 = new TNode();
                            ie0->m_N0->SetUWPnt( e0 );
                            ie0->m_N0->SetXYZPnt( e0xyz );
                            ie0->m_N0->MakePntUW();
                            ie0->m_N0->SetCoordInfo( info );
                            ie0->m_N1 = new TNode();
                            ie0->m_N1->SetUWPnt( e1 );
                            ie0->m_N1->SetXYZPnt( e1xyz );
                            ie0->m_N1->MakePntUW();
                            ie0->m_N1->SetCoordInfo( info );

                            TEdge* ie1 = new TEdge();
                            ie1->m_N0 = new TNode();
                            ie1->m_N0->SetUWPnt( e0 );
                            ie1->m_N0->SetXYZPnt( e0xyz );
                            ie1->m_N0->MakePntUW();
                            ie1->m_N0->SetCoordInfo( info );
                            ie1->m_N1 = new TNode();
                            ie1->m_N1->SetUWPnt( e1 );
                            ie1->m_N1->SetXYZPnt( e1xyz );
                            ie1->m_N1->MakePntUW();
                            ie1->m_N1->SetCoordInfo( info );

                            t0->m_ISectEdgeVec.push_back( ie0 );
                            t1->m_ISectEdgeVec.push_back( ie1 );

                            if ( tri->GetTMeshPtr() )
                            {
                                tri->GetTMeshPtr()->SplitAliasEdges( tri, tri->m_ISectEdgeVec.back() );
                            }

                        }
                    }
                    else
                    {
                        if ( dist( e0, e1 ) > tol )
                        {
                            TEdge* ie0 = new TEdge();
                            int info = TNode::HAS_UW | TNode::HAS_XYZ;
                            ie0->m_N0 = new TNode();
                            ie0->m_N0->m_Pnt = e0;
                            ie0->m_N0->m_UWPnt = t0->CompUW( e0 );
                            ie0->m_N0->SetCoordInfo( info );
                            ie0->m_N1 = new TNode();
                            ie0->m_N1->m_Pnt = e1;
                            ie0->m_N1->m_UWPnt = t0->CompUW( e1 );
                            ie0->m_N1->SetCoordInfo( info );

                            TEdge* ie1 = new TEdge();
                            ie1->m_N0 = new TNode();
                            ie1->m_N0->m_Pnt = e0;
                            ie1->m_N0->m_UWPnt = t1->CompUW( e0 );
                            ie1->m_N0->SetCoordInfo( info );
                            ie1->m_N1 = new TNode();
                            ie1->m_N1->m_Pnt = e1;
                            ie1->m_N1->m_UWPnt = t1->CompUW( e1 );
                            ie1->m_N1->SetCoordInfo( info );

                            t0->m_ISectEdgeVec.push_back( ie0 );
                            t1->m_ISectEdgeVec.push_back( ie1 );
                        }
                    }
                }
            }
        }
    }
}

void  TBndBox::RayCast( vec3d & orig, vec3d & dir, vector<double> & tParmVec )
{
    int i;

    double coord[3];

    if( !intersectRayAABB( m_Box.GetMin().v, m_Box.GetMax().v, orig.v, dir.v, coord ) )
    {
        return;
    }

    if ( m_SBoxVec[0] )
    {
        for ( i = 0 ; i < 8 ; i++ )
        {
            m_SBoxVec[i]->RayCast( orig, dir, tParmVec );
        }
        return;
    }

    //==== Check All Tris In Box ====//
    double tparm, uparm, vparm;

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
                if ( std::abs( tparm - tParmVec[j] ) < 0.0000001 )
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

    if ( node0 )
    {
        node0->m_EdgeVec.push_back( edge );
    }
    if ( node1 )
    {
        node1->m_EdgeVec.push_back( edge );
    }

    m_EVec.push_back( edge );

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
    n1 = nullptr;
    n2 = nullptr;
    n3 = nullptr;

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

    for ( mit = m_NAMap.begin(); mit != m_NAMap.end() ; ++mit ) // Loop over all master nodes
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

    int n;

    double tol = 1.0e-12;

    //==== Build Map ====//
    PntNodeCloud pnCloud;
    pnCloud.ReserveMorePntNodes( m_NVec.size() );

    for ( n = 0 ; n < ( int )m_NVec.size() ; n++ )
    {
        m_NVec[n]->m_MergeVec.clear();
        pnCloud.AddPntNode( m_NVec[n]->m_Pnt );
    }

    //==== Use NanoFlann to Find Close Points and Group ====//
    IndexPntNodes( pnCloud, tol );

    for ( n = 0 ; n < ( int )m_NVec.size() ; n++ )
    {
        if ( pnCloud.UsedNode( n ) ) // This point is a NanoFlann master.
        {
            // Set n to be its own master
            m_NSMMap[ m_NVec[n] ] = m_NVec[n];

            // Add n to its own set of aliases
            m_NAMap[ m_NVec[n] ].push_front( m_NVec[n] );
        }
    }

    for ( int islave = 0 ; islave < ( int )m_NVec.size() ; islave++ )
    {
        if ( !(pnCloud.UsedNode( islave )) ) // This point is a NanoFlann slave.
        {
            int imaster = pnCloud.GetNodeBaseIndex( islave );
            m_NVec[imaster]->m_MergeVec.push_back( m_NVec[islave] );
            m_NAMap[m_NVec[imaster]].push_back( m_NVec[islave] ); // Add node islave to imaster's list of aliases
            m_NSMMap[m_NVec[islave]] = m_NVec[imaster]; // Set islave's master to be imaster
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
    for ( t = 0 ; t < (int)m_TVec.size(); t++ )
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
    }

    //==== Nuke Redundant Nodes And Update NVec ====//
    m_NVec.clear();
    for ( mit = m_NAMap.begin() ; mit != m_NAMap.end() ; ++mit )
    {
        TNode* nk = mit->first;
        list< TNode* >& dnodes =  mit->second;

        for ( lit = ++dnodes.begin() ; lit != dnodes.end() ; ++lit ) // Start at second element since first is the master node itself
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
        int coplanarFlag = 0; // Must be initialized to 0 before use in tri_tri_intersection_test_3d
        int iflag = tri_tri_intersection_test_3d(
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

void TMesh::SplitAliasEdges( TTri* orig_tri, TEdge* isect_edge )
{
    // Check to see if either of the new nodes are on the existing perimeter edges,
    // and if so add an edge to alias edges' triangle that will cause a split to occur
    vector < vec3d > es(2);
    vector < vec3d > exyzs(2);

    es[0] = isect_edge->m_N0->GetUWPnt();
    es[1] = isect_edge->m_N1->GetUWPnt();
    exyzs[0] = isect_edge->m_N0->GetXYZPnt();
    exyzs[1] = isect_edge->m_N1->GetXYZPnt();

    if ( orig_tri->m_PEArr[0] != NULL )
    {
        double edgeTol = ON_EDGE_TOL;

        vec3d * nn;
        TEdge * orig_e = NULL;
        double t;
        for ( int pei = 0 ; pei < 3 ; pei++ )
        {
            nn = NULL;
            orig_e = orig_tri->m_PEArr[pei];

            for( int ei = 0; ei < 2; ei++ )
            {

                if ( orig_tri->OnEdge( es[ei], orig_e, edgeTol, &t ) == 1 )
                {
                    nn = &exyzs[ei];

                    if ( t > 0.0 && t < 1.0 )
                    {
                        // Get alias vector
                        TEdge *master = m_ESMMap[orig_e];
                        vector<TEdge *> a_edges = m_EAMap[master];

                        a_edges.push_back(master);
                        a_edges.erase(find(a_edges.begin(), a_edges.end(), orig_e));
                        // Get vector of tris
                        vector<TTri *> a_tris;
                        vector<TEdge *>::iterator vit; //vector iterator
                        for (vit = a_edges.begin(); vit != a_edges.end(); ++vit)
                        {
                            TEdge *e = *vit;
                            a_tris.push_back(e->GetParTri());
                        }

                        // Loop over each tri and split appropriate edge
                        for (int ti = 0; ti < (int) a_tris.size(); ti++)
                        {
                            // Compute UW of xyz point on alias triangle
                            TTri *ta = a_tris[ti]; // alias triangle
                            if (!ta)
                            {
                                printf("Warning: Edge is missing parent triangle\n");
                                continue;
                            }

                            TEdge *ea = a_edges[ti]; // alias edge

                            vec3d uwn;
                            if (m_NSMMap[orig_e->m_N0] == m_NSMMap[ea->m_N0]) // Make sure edges are aligned the same way for the parametric line parameter
                            {
                                uwn = point_on_line(ea->m_N0->GetUWPnt(), ea->m_N1->GetUWPnt(), t);
                            }
                            else
                            {
                                uwn = point_on_line(ea->m_N1->GetUWPnt(), ea->m_N0->GetUWPnt(), t);
                            }

                            // Create Fake Edge
                            TEdge *n_edge = new TEdge();
                            n_edge->m_N0 = new TNode();
                            n_edge->m_N1 = new TNode();
                            n_edge->SetParTri(ta);
                            n_edge->m_N0->SetUWPnt(uwn);
                            n_edge->m_N0->SetXYZPnt(*nn);
                            n_edge->m_N0->MakePntUW();
                            n_edge->m_N1->SetUWPnt(ea->m_N1->GetUWPnt());
                            n_edge->m_N1->SetXYZPnt(ea->m_N1->GetXYZPnt());
                            n_edge->m_N1->MakePntUW();
                            ta->m_ISectEdgeVec.push_back(n_edge);
                        }
                    }
                }
            }
        }
    }
}

// Subtag triangles
void TMesh::SubTag( int part_num, bool tag_subs )
{
    // Subtag all triangles in a given TMesh
    // Split tris will be subtagged the same as their parent
    vector<SubSurface*> sub_surfs;
    if ( tag_subs ) sub_surfs = SubSurfaceMgr.GetSubSurfs( m_PtrID, m_SurfNum );
    int ss_num = ( int )sub_surfs.size();

    for ( int t = 0 ; t < ( int )m_TVec.size(); t ++ )
    {
        TTri* tri = m_TVec[t];
        tri->m_Tags.push_back( part_num ); // Give Tri overall surface ID number
        for ( int s = 0; s < ss_num; s++ )
        {
            if ( sub_surfs[s]->Subtag( tri ) )
            {
                tri->m_Tags.push_back( sub_surfs[s]->m_Tag );
            }
        }

        SubSurfaceMgr.m_TagCombos.insert( tri->m_Tags );

        for ( int st = 0; st < ( int )tri->m_SplitVec.size() ; st++ ) // Set split tris to have same tags as main tri
        {
            tri->m_SplitVec[st]->m_Tags = tri->m_Tags;
        }
    }
}

vec3d TMesh::CompPnt( const vec3d & uw_pnt )
{
    // Search through uw pnts to figure out which quad the uw_pnt is in

    if ( m_UWPnts.size() == 0 || m_XYZPnts.size() == 0 )
    {
        return vec3d();
    }

    int start_u, start_v;

    FindIJ( uw_pnt, start_u, start_v );

    vec3d p0, p1, p2, p3;

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

void TMesh::FindIJ( const vec3d & uw_pnt, int &start_u, int &start_v )
{
    // Search through uw pnts to figure out which quad the uw_pnt is in
    start_u = start_v = 0;

    if ( m_UWPnts.size() == 0 )
    {
        return;
    }

    // Find Starting U pnt
    for ( int i = 0 ; i < ( int )m_UWPnts.size() - 1 ; i++ )
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
    for ( int j = 0 ; j < ( int )m_UWPnts[start_u].size() - 1; j++ )
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
}

void CreateTMeshVecFromPts( const Geom * geom,
                            vector < TMesh* > & TMeshVec,
                            const vector< vector<vec3d> > & pnts,
                            const vector< vector<vec3d> > & norms,
                            const vector< vector<vec3d> > & uw_pnts,
                            int indx, int surftype, int cfdsurftype, bool thicksurf, bool flipnormal, double wmax )
{
    double tol=1.0e-12;

    TMeshVec.push_back( new TMesh() );
    int itmesh = TMeshVec.size() - 1;
    TMeshVec[itmesh]->LoadGeomAttributes( geom );
    TMeshVec[itmesh]->m_SurfCfdType = cfdsurftype;
    TMeshVec[itmesh]->m_ThickSurf = thicksurf;
    TMeshVec[itmesh]->m_SurfType = surftype;
    TMeshVec[itmesh]->m_SurfNum = indx;
    TMeshVec[itmesh]->m_UWPnts = uw_pnts;
    TMeshVec[itmesh]->m_XYZPnts = pnts;

    if ( cfdsurftype == vsp::CFD_NEGATIVE )
    {
        flipnormal = !flipnormal;
    }

    BuildTMeshTris( TMeshVec[itmesh], flipnormal, wmax );

}

void BuildTMeshTris( TMesh *tmesh, bool flipnormal, double wmax )
{
    double tol=1.0e-12;

    vector< vector<vec3d> > *pnts = &(tmesh->m_XYZPnts);
    vector< vector<vec3d> > *uw_pnts = &(tmesh->m_UWPnts);

    vec3d norm;
    vec3d v0, v1, v2, v3;
    vec3d uw0, uw1, uw2, uw3;
    vec3d d21, d01, d03, d23, d20, d31;

    for ( int j = 0 ; j < ( int )(*pnts).size() - 1 ; j++ )
    {
        for ( int k = 0 ; k < ( int )(*pnts)[0].size() - 1 ; k++ )
        {
            v0 = (*pnts)[j][k];
            v1 = (*pnts)[j + 1][k];
            v2 = (*pnts)[j + 1][k + 1];
            v3 = (*pnts)[j][k + 1];

            uw0 = (*uw_pnts)[j][k];
            uw1 = (*uw_pnts)[j + 1][k];
            uw2 = (*uw_pnts)[j + 1][k + 1];
            uw3 = (*uw_pnts)[j][k + 1];

            double quadrant = ( uw0.y() + uw1.y() + uw2.y() + uw3.y() ) / wmax; // * 4 * 0.25 canceled.

            d21 = v2 - v1;
            d01 = v0 - v1;
            d03 = v0 - v3;
            d23 = v2 - v3;

            if ( ( quadrant > 0 && quadrant < 1 ) || ( quadrant > 2 && quadrant < 3 ) )
            {
                d20 = v2 - v0;
                if ( d21.mag() > tol && d01.mag() > tol && d20.mag() > tol )
                {
                    norm = cross( d21, d01 );
                    norm.normalize();
                    if ( flipnormal )
                    {
                        tmesh->AddTri( v0, v2, v1, norm * -1, uw0, uw2, uw1 );
                    }
                    else
                    {
                        tmesh->AddTri( v0, v1, v2, norm, uw0, uw1, uw2 );
                    }
                }

                if ( d03.mag() > tol && d23.mag() > tol && d20.mag() > tol )
                {
                    norm = cross( d03, d23 );
                    norm.normalize();
                    if ( flipnormal )
                    {
                        tmesh->AddTri( v0, v3, v2, norm * -1, uw0, uw3, uw2 );
                    }
                    else
                    {
                        tmesh->AddTri( v0, v2, v3, norm, uw0, uw2, uw3 );
                    }
                }
            }
            else
            {
                d31 = v3 - v1;
                if ( d01.mag() > tol && d31.mag() > tol && d03.mag() > tol )
                {
                    norm = cross( d01, d03 );
                    norm.normalize();
                    if ( flipnormal )
                    {
                        tmesh->AddTri( v0, v3, v1, norm * -1, uw0, uw3, uw1 );
                    }
                    else
                    {
                        tmesh->AddTri( v0, v1, v3, norm, uw0, uw1, uw3 );
                    }
                }

                if ( d21.mag() > tol && d23.mag() > tol && d31.mag() > tol )
                {
                    norm = cross( d23, d21 );
                    norm.normalize();
                    if ( flipnormal )
                    {
                        tmesh->AddTri( v1, v3, v2, norm * -1, uw1, uw3, uw2 );
                    }
                    else
                    {
                        tmesh->AddTri( v1, v2, v3, norm, uw1, uw2, uw3 );
                    }
                }
            }
        }
    }
}
