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

#define _USE_MATH_DEFINES
#include <cmath>

#ifdef WIN32
#include <windows.h>
#endif

#define _USE_MATH_DEFINES
#include <cmath>

#include "TMesh.h"
#include "PGMesh.h"
#include "TMeshSweptVolume.h"

#include "tri_tri_intersect.h"
#include "predicates.h"
#include "Mathematics/ConvexHull3.h"

#include "Geom.h"
#include "SubSurfaceMgr.h"
#include "PntNodeMerge.h"
#include "VspCurve.h" // for #define TMAGIC

#include "triangle.h"
#include "triangle_api.h"

#include "VspUtil.h"

#include "delabella.h"
#include "StlHelper.h"

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

bool TNode::OnBoundary()
{
    for ( int t = 0; t < m_TriVec.size(); t++ )
    {
        if ( m_TriVec[t]->OnBoundary( this ) )
        {
            return true;
        }
    }
    return false;
}

vec3d TNode::CompNorm()
{
    vec3d nsum;
    double Asum;
    for ( int t = 0; t < m_TriVec.size(); t++ )
    {
        double a = m_TriVec[t]->ComputeArea();
        vec3d n = m_TriVec[t]->CompNorm();

        Asum += a;
        nsum += n * a;
    }
    nsum *= 1.0/Asum;
    nsum.normalize();
    return nsum;
}

//=======================================================================//
//===================            TEdge          =========================//
//=======================================================================//

TEdge::TEdge()
{
    m_N0 = m_N1 = 0;
    m_ParTri = nullptr;
    m_Tri0 = m_Tri1 = nullptr;
}

TEdge::TEdge( TNode* n0, TNode* n1, TTri* par_tri )
{
    m_N0 = n0;
    m_N1 = n1;
    m_ParTri = par_tri;
    m_Tri0 = m_Tri1 = nullptr;
}

void TEdge::SwapEdgeDirection()
{
    TNode * ntmp = m_N1;
    m_N1 = m_N0;
    m_N0 = ntmp;
}

// Sort edges in increasing U (du > 0)
void TEdge::SortNodesByU()
{
    double du = m_N1->m_UWPnt.x() - m_N0->m_UWPnt.x();
    if ( du < 0 )
    {
        SwapEdgeDirection();
    }
}

TTri* TEdge::GetOtherTri( TTri *t )
{
    if ( t == m_Tri0 )
    {
        return m_Tri1;
    }
    else if ( t == m_Tri1 )
    {
        return m_Tri0;
    }

    return NULL;
}

TTri* TEdge::OtherTri( TTri* t )
{
    if ( t )
    {
        if ( m_Tri0 && m_Tri0 == t )
        {
            return m_Tri1;
        }

        if ( m_Tri1 && m_Tri1 == t )
        {
            return m_Tri0;
        }
    }
    return NULL;
}

bool TEdge::UsesNode( TNode *n )
{
    if ( m_N0 == n || m_N1 == n )
    {
        return true;
    }
    return false;
}

void TEdge::ReplaceTri( TTri *told, TTri *tnew )
{
    if ( m_Tri0 == told )
    {
        m_Tri0 = tnew;
    }

    if ( m_Tri1 == told )
    {
        m_Tri1 = tnew;
    }
}

bool TEdge::DuplicateEdge( TEdge * other )
{
    if ( other->UsesNode( m_N0 ) &&
         other->UsesNode( m_N1 ) )
    {
        return true;
    }
    return false;
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
    m_Mass = m_Density * m_Vol;

    vector < double > vx = { m_v1.x() * m_v1.x(), m_v2.x() * m_v2.x(), m_v3.x() * m_v3.x(), m_v1.x() * m_v2.x(), m_v1.x() * m_v3.x(), m_v2.x() * m_v3.x() };
    double Ix = m_Mass / 10.0 * compsum( vx );

    vector < double > vy = { m_v1.y() * m_v1.y(), m_v2.y() * m_v2.y(), m_v3.y() * m_v3.y(), m_v1.y() * m_v2.y(), m_v1.y() * m_v3.y(), m_v2.y() * m_v3.y() };
    double Iy = m_Mass / 10.0 * compsum( vy );

    vector < double > vz = { m_v1.z() * m_v1.z(), m_v2.z() * m_v2.z(), m_v3.z() * m_v3.z(), m_v1.z() * m_v2.z(), m_v1.z() * m_v3.z(), m_v2.z() * m_v3.z() };
    double Iz = m_Mass / 10.0 * compsum( vz );

    m_Ixx = Iy + Iz;
    m_Iyy = Ix + Iz;
    m_Izz = Ix + Iy;

    vector < double > vxy1 = { m_v1.x() * m_v1.y(), m_v2.x() * m_v2.y(), m_v3.x() * m_v3.y() };
    vector < double > vxy2 = { m_v1.x() * m_v2.y(), m_v2.x() * m_v1.y(), m_v1.x() * m_v3.y(), m_v3.x() * m_v1.y(), m_v2.x() * m_v3.y(), m_v3.x() * m_v2.y() };
    m_Ixy = m_Mass / 20.0 * ( 2.0 * compsum( vxy1 ) + compsum( vxy2 ) );

    vector < double > vyz1 = { m_v1.y() * m_v1.z(), m_v2.y() * m_v2.z(), m_v3.y() * m_v3.z() };
    vector < double > vyz2 = { m_v1.y() * m_v2.z(), m_v2.y() * m_v1.z(), m_v1.y() * m_v3.z(), m_v3.y() * m_v1.z(), m_v2.y() * m_v3.z(), m_v3.y() * m_v2.z() };
    m_Iyz = m_Mass / 20.0 * ( 2.0 * compsum( vyz1 ) + compsum( vyz2 ) );

    vector < double > vxz1 = { m_v1.x() * m_v1.z(), m_v2.x() * m_v2.z(), m_v3.x() * m_v3.z()};
    vector < double > vxz2 = { m_v1.x() * m_v2.z(), m_v2.x() * m_v1.z(), m_v1.x() * m_v3.z(), m_v3.x() * m_v1.z(), m_v2.x() * m_v3.z(), m_v3.x() * m_v2.z() };
    m_Ixz = m_Mass / 20.0 * ( 2.0 * compsum( vxz1 ) + compsum( vxz2 ) );

}

void TetraMassProp::SetDistributedMass( double massIn, const vec3d& cgIn, const double & IxxIn, const double & IyyIn, const double & IzzIn,
                                                                          const double & IxyIn, const double & IxzIn, const double & IyzIn, Matrix4d transMatIn )
{
    m_CompId = "NONE";
    m_Density = 0.0;
    m_CG = transMatIn.xform( cgIn );
    m_Vol  = 0.0;
    m_Mass = massIn;

    double Idat[16];
    Matrix4d::setIdentity( Idat );

    Idat[0] = IxxIn;
    Idat[5] = IyyIn;
    Idat[10] = IzzIn;

    Idat[4] = Idat[1] = -IxyIn;
    Idat[8] = Idat[2] = -IxzIn;
    Idat[9] = Idat[6] = -IyzIn;

    Matrix4d Imat;
    Imat.initMat( Idat );

    transMatIn.zeroTranslations();
    transMatIn.affineInverse();

    Imat.matMult( transMatIn );
    transMatIn.affineInverse();
    Imat.postMult( transMatIn );

    Imat.getMat( Idat );

    m_Ixx = Idat[0];
    m_Iyy = Idat[5];
    m_Izz = Idat[10];

    m_Ixy = -Idat[1];
    m_Ixz = -Idat[2];
    m_Iyz = -Idat[6];
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

    vector < double > vx = { m_v0.x() * m_v0.x(), m_v1.x() * m_v1.x(), m_v2.x() * m_v2.x(), m_v0.x() * m_v1.x(), m_v0.x() * m_v2.x(), m_v1.x() * m_v2.x() };
    double Ix = m_Mass / 10.0 * compsum( vx );

    vector < double > vy = { m_v0.y() * m_v0.y(), m_v1.y() * m_v1.y(), m_v2.y() * m_v2.y(), m_v0.y() * m_v1.y(), m_v0.y() * m_v2.y(), m_v1.y() * m_v2.y() };
    double Iy = m_Mass / 10.0 * compsum( vy );

    vector < double > vz = {m_v0.z() * m_v0.z(), m_v1.z() * m_v1.z(), m_v2.z() * m_v2.z(), m_v0.z() * m_v1.z(), m_v0.z() * m_v2.z(), m_v1.z() * m_v2.z() };
    double Iz = m_Mass / 10.0 * compsum( vz );

    m_Ixx = Iy + Iz;
    m_Iyy = Ix + Iz;
    m_Izz = Ix + Iy;

    vector < double > vxy1 = { m_v0.x() * m_v0.y(), m_v1.x() * m_v1.y(), m_v2.x() * m_v2.y() };
    vector < double > vxy2 = { m_v0.x() * m_v1.y(), m_v1.x() * m_v0.y(), m_v0.x() * m_v2.y(), m_v2.x() * m_v0.y(), m_v1.x() * m_v2.y(), m_v2.x() * m_v1.y() };
    m_Ixy = m_Mass / 20.0 * ( 2.0 * compsum( vxy1 ) + compsum( vxy2 ) );

    vector < double > vyz1 = { m_v0.y() * m_v0.z(), m_v1.y() * m_v1.z(), m_v2.y() * m_v2.z() };
    vector < double > vyz2 = { m_v0.y() * m_v1.z(), m_v1.y() * m_v0.z(), m_v0.y() * m_v2.z(), m_v2.y() * m_v0.z(), m_v1.y() * m_v2.z(), m_v2.y() * m_v1.z() };
    m_Iyz = m_Mass / 20.0 * ( 2.0 * compsum( vyz1 ) + compsum( vyz2 ) );

    vector < double > vxz1 = { m_v0.x() * m_v0.z(), m_v1.x() * m_v1.z(), m_v2.x() * m_v2.z() };
    vector < double > vxz2 = { m_v0.x() * m_v1.z(), m_v1.x() * m_v0.z(), m_v0.x() * m_v2.z(), m_v2.x() * m_v0.z(), m_v1.x() * m_v2.z(), m_v2.x() * m_v1.z() };
    m_Ixz = m_Mass / 20.0 * ( 2.0 * compsum( vxz1 ) + compsum( vxz2 ) );


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
    Init();
}

void TMesh::Init()
{
    m_MaterialID = 0;
    m_SurfCfdType = vsp::CFD_NORMAL;
    m_SurfType = vsp::NORMAL_SURF;
    m_ThickSurf = true;
    m_FlatPatch = false;
    m_MassPrior = 0;
    m_Density = 0;
    m_ShellMassArea = 0;
    m_ShellFlag = false;
    m_DeleteMeFlag = false;
    m_TheoArea = m_WetArea = 0.0;
    m_TheoVol    = m_WetVol = 0.0;
    m_SurfNum = 0;
    m_PlateNum = -1;
    m_CopyIndex = -1;
    m_AreaCenter = vec3d(0,0,0);
    m_GuessVol = 0;
    m_Wmin = DBL_MAX;
    m_Uscale = DBL_MIN;
    m_Wscale = DBL_MIN;


    m_OriginGeomID = "";
    m_NameStr = "";

    m_SurfType = -1;
}

TMesh::~TMesh()
{
    Wype();
}

void TMesh::Wype()
{
    for ( int i = 0 ; i < ( int )m_TVec.size() ; i++ )
    {
        delete m_TVec[i];
    }
    m_TVec.clear();

    for ( int i = 0 ; i < ( int )m_NVec.size() ; i++ )
    {
        delete m_NVec[i];
    }
    m_NVec.clear();

    for ( int i = 0 ; i < ( int )m_EVec.size() ; i++ )
    {
        delete m_EVec[i];
    }
    m_EVec.clear();


    m_TBox.Reset();

    m_InGroup.clear();

    m_CompAreaVec.clear();
    m_TagTheoAreaVec.clear();
    m_TagWetAreaVec.clear();

    m_NonClosedTriVec.clear();

    m_VertVec.clear();

    m_UWPnts.clear();
    m_XYZPnts.clear();

    m_NAMap.clear();
    m_NSMMap.clear();
    m_EAMap.clear();
    m_ESMMap.clear();

    Init();
}

void TMesh::copy( TMesh* m )
{
    CopyAttributes( m );
    m_TVec.clear();
    m_NVec.clear();

    for ( int i = 0 ; i < ( int )m->m_TVec.size() ; i++ )
    {
        TTri* tri = new TTri( this );
        tri->m_N0   = new TNode();
        tri->m_N1   = new TNode();
        tri->m_N2   = new TNode();

        tri->m_Norm    = m->m_TVec[i]->m_Norm;
        tri->m_iQuad    = m->m_TVec[i]->m_iQuad;
        tri->m_jref     = m->m_TVec[i]->m_jref;
        tri->m_kref     = m->m_TVec[i]->m_kref;
        tri->m_ID    = m->m_TVec[i]->m_ID;
        tri->m_Tags = m->m_TVec[i]->m_Tags;
        tri->m_GeomID = m->m_TVec[i]->m_GeomID;
        tri->m_Density = m->m_TVec[i]->m_Density;

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

void TMesh::copyFewerNodes( TMesh* m )
{
    CopyAttributes( m );
    m_TVec.clear();
    m_NVec.clear();

    m_NVec.reserve( m->m_NVec.size() );
    for ( int i = 0 ; i < ( int )m->m_NVec.size() ; i++ )
    {
        m->m_NVec[i]->m_ID = i;
        m_NVec.push_back( new TNode() );
        m_NVec[i]->m_Pnt = m->m_NVec[i]->m_Pnt;
        m_NVec[i]->m_UWPnt = m->m_NVec[i]->m_UWPnt;
        m_NVec[i]->m_ID = m->m_NVec[i]->m_ID;
    }

    m_TVec.reserve( m->m_TVec.size() );
    for ( int i = 0 ; i < ( int )m->m_TVec.size() ; i++ )
    {
        TTri* tri = new TTri( this );
        tri->m_N0 = m_NVec[ m->m_TVec[i]->m_N0->m_ID ];
        tri->m_N1 = m_NVec[ m->m_TVec[i]->m_N1->m_ID ];
        tri->m_N2 = m_NVec[ m->m_TVec[i]->m_N2->m_ID ];

        tri->m_Norm = m->m_TVec[i]->m_Norm;
        tri->m_iQuad = m->m_TVec[i]->m_iQuad;
        tri->m_jref = m->m_TVec[i]->m_jref;
        tri->m_kref = m->m_TVec[i]->m_kref;
        tri->m_ID = m->m_TVec[i]->m_ID;
        tri->m_Tags = m->m_TVec[i]->m_Tags;
        tri->m_GeomID = m->m_TVec[i]->m_GeomID;
        tri->m_Density = m->m_TVec[i]->m_Density;

        m_TVec.push_back( tri );
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
                    AddTri( s_tri->m_N0, s_tri->m_N1, s_tri->m_N2, s_tri->m_Norm, orig_tri->m_iQuad );
                    m_TVec.back()->m_Tags = s_tri->m_Tags;
                    m_TVec.back()->m_jref = s_tri->m_jref;
                    m_TVec.back()->m_kref = s_tri->m_kref;
                    m_TVec.back()->m_ID = s_tri->m_ID;
                    m_TVec.back()->m_GeomID = s_tri->m_GeomID;
                    m_TVec.back()->m_Density = s_tri->m_Density;
                }
            }
        }
        else
        {
            if ( !orig_tri->m_IgnoreTriFlag )
            {
                AddTri( orig_tri->m_N0, orig_tri->m_N1, orig_tri->m_N2, orig_tri->m_Norm, orig_tri->m_iQuad );
                m_TVec.back()->m_Tags = orig_tri->m_Tags;
                m_TVec.back()->m_jref = orig_tri->m_jref;
                m_TVec.back()->m_kref = orig_tri->m_kref;
                m_TVec.back()->m_ID = orig_tri->m_ID;
                m_TVec.back()->m_GeomID = orig_tri->m_GeomID;
                m_TVec.back()->m_Density = orig_tri->m_Density;
            }
        }
    }
}

void TMesh::FlattenInPlace()
{
    vector< TTri* > origTVec = m_TVec;
    m_TVec.clear();
    vector< TNode* > origNVec = m_NVec;
    m_NVec.clear();

    for ( int i = 0 ; i < ( int )origTVec.size() ; i++ )
    {
        TTri* orig_tri = origTVec[i];

        if ( orig_tri->m_SplitVec.size() )
        {
            for ( int s = 0 ; s < ( int )orig_tri->m_SplitVec.size() ; s++ )
            {
                TTri* s_tri = orig_tri->m_SplitVec[s];
                if ( !s_tri->m_IgnoreTriFlag )
                {
                    AddTri( s_tri->m_N0, s_tri->m_N1, s_tri->m_N2, s_tri->m_Norm, orig_tri->m_iQuad );
                    m_TVec.back()->m_Tags = s_tri->m_Tags;
                    m_TVec.back()->m_jref = s_tri->m_jref;
                    m_TVec.back()->m_kref = s_tri->m_kref;
                    m_TVec.back()->m_ID = s_tri->m_ID;
                    m_TVec.back()->m_GeomID = s_tri->m_GeomID;
                    m_TVec.back()->m_Density = s_tri->m_Density;
                }
            }
        }
        else
        {
            if ( !orig_tri->m_IgnoreTriFlag )
            {
                AddTri( orig_tri->m_N0, orig_tri->m_N1, orig_tri->m_N2, orig_tri->m_Norm, orig_tri->m_iQuad );
                m_TVec.back()->m_Tags = orig_tri->m_Tags;
                m_TVec.back()->m_jref = orig_tri->m_jref;
                m_TVec.back()->m_kref = orig_tri->m_kref;
                m_TVec.back()->m_ID = orig_tri->m_ID;
                m_TVec.back()->m_GeomID = orig_tri->m_GeomID;
                m_TVec.back()->m_Density = orig_tri->m_Density;
            }
        }

        delete orig_tri;
    }

    for ( int i = 0 ; i < ( int )origNVec.size() ; i++ )
    {
        delete origNVec[i];
    }
}

void TMesh::CopyAttributes( TMesh* m )
{
    m_OriginGeomID     = m->m_OriginGeomID;
    m_SurfNum   = m->m_SurfNum;
    m_PlateNum = m->m_PlateNum;
    m_CopyIndex = m->m_CopyIndex;
    m_NameStr   = m->m_NameStr;
    m_MaterialID = m->m_MaterialID;
    m_SurfCfdType = m->m_SurfCfdType;
    m_ThickSurf = m->m_ThickSurf;
    m_FlatPatch = m->m_FlatPatch;
    m_InGroup = m->m_InGroup;

    m_TheoArea   = m->m_TheoArea;
    m_WetArea    = m->m_WetArea;
    m_CompAreaVec = m->m_CompAreaVec;
    m_TagTheoAreaVec = m->m_TagTheoAreaVec;
    m_TagWetAreaVec = m->m_TagWetAreaVec;
    m_TheoVol    = m->m_TheoVol;
    m_GuessVol   = m->m_GuessVol;
    m_WetVol     = m->m_WetVol;

    m_MassPrior = m->m_MassPrior;
    m_Density   = m->m_Density;
    m_ShellFlag = m->m_ShellFlag;
    m_ShellMassArea = m->m_ShellMassArea;

    m_SurfType = m->m_SurfType;
    m_Wmin = m->m_Wmin;
    m_Uscale = m->m_Uscale;
    m_Wscale = m->m_Wscale;

    m_UWPnts = m->m_UWPnts;
    m_XYZPnts = m->m_XYZPnts;

    m_AreaCenter = m->m_AreaCenter;
}

xmlNodePtr TMesh::EncodeXml( xmlNodePtr & node )
{
    xmlNodePtr tmesh_node = xmlNewChild( node, nullptr, BAD_CAST "TMesh", nullptr );
    XmlUtil::AddIntNode( tmesh_node, "Num_Tris", ( int )m_TVec.size() );
    EncodeTriList( tmesh_node );
    return tmesh_node;
}

xmlNodePtr TMesh::EncodeTriList( xmlNodePtr & node )
{
    xmlNodePtr tri_list_node = xmlNewChild( node, nullptr, BAD_CAST "Tri_List", nullptr );
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
    while( iter_node != nullptr )
    {
        if ( !xmlStrcmp( iter_node->name, ( const xmlChar * )"Tri" ) )
        {
            tri = XmlUtil::GetVectorVec3dNode( iter_node );
            m_TVec[i] = new TTri( this );
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
    m_OriginGeomID        = geomPtr->GetID();

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

void TMesh::BuildEdges()
{
    int n, e, s, t;

    for ( e = 0; e < (int ) m_EVec.size(); e++ )
    {
        delete m_EVec[e];
    }
    m_EVec.clear();

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
        for ( t = 0 ; t < ( int )n0->m_TriVec.size() - 1 ; t++ )
        {
            for ( s = t + 1 ; s < ( int )n0->m_TriVec.size() ; s++ )
            {
                FindEdge( n0, n0->m_TriVec[t], n0->m_TriVec[s] );
            }
        }
    }
}

void TMesh::CheckIfClosed()
{
    int t;

    BuildEdges();

    //==== Check If All Tris Have 3 Edges ====//
    m_NonClosedTriVec.clear();
    for ( t = 0 ; t < ( int )m_TVec.size() ; t++ )
    {
        if ( !m_TVec[t]->m_E0 || !m_TVec[t]->m_E1 || !m_TVec[t]->m_E2 )
        {
            m_TVec[t]->m_InvalidFlag = true;
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
            if ( t0->MergeSharedEdges( t1 ) )
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

void TMesh::MergeTMeshes( const TMesh* tm )
{
    for ( int t = 0 ; t < ( int )tm->m_TVec.size() ; t++ )
    {
        TTri* tri = tm->m_TVec[t];
        AddTri( tri );
        m_TVec.back()->m_InvalidFlag = false;
    }

    for ( int i = 0 ; i < ( int )m_NonClosedTriVec.size() ; i++ )
    {
        m_NonClosedTriVec[i]->m_InvalidFlag = false;
    }
    m_NonClosedTriVec.clear();

    m_InGroup.insert( m_InGroup.end(), tm->m_InGroup.begin(), tm->m_InGroup.end() );
    std::sort( m_InGroup.begin(), m_InGroup.end() );
    m_InGroup.erase( std::unique( m_InGroup.begin(), m_InGroup.end() ), m_InGroup.end() );
}

void TMesh::Intersect( TMesh* tm, bool UWFlag, bool checkSharedEdges )
{
    m_TBox.Intersect( &tm->m_TBox, UWFlag, checkSharedEdges );
}

bool TMesh::CheckIntersect( TMesh* tm )
{
    return m_TBox.CheckIntersect( &tm->m_TBox );
}

double TMesh::MinDistance( TMesh* tm, double curr_min_dist, vec3d &p1, vec3d &p2  )
{
    return m_TBox.MinDistance( &tm->m_TBox, curr_min_dist, p1, p2  );
}

bool TMesh::CheckIntersect( const vec3d &org, const vec3d &norm )
{
    return m_TBox.CheckIntersect( org, norm );
}

double TMesh::MinDistance( const vec3d &org, const vec3d &norm, double curr_min_dist, vec3d &p1, vec3d &p2 )
{
    return m_TBox.MinDistance( org, norm, curr_min_dist, p1, p2 );
}

double TMesh::MaxDistance( const vec3d &org, const vec3d &norm, double curr_max_dist, vec3d &p1, vec3d &p2 )
{
    return m_TBox.MaxDistance( org, norm, curr_max_dist, p1, p2 );
}

double TMesh::MaxDistanceRay( const vec3d &org, const vec3d &norm, double curr_min_dist, vec3d &p1, vec3d &p2 )
{
    return m_TBox.MaxDistanceRay( org, norm, curr_min_dist, p1, p2 );
}

double TMesh::MinAngle( const vec3d &org, const vec3d &norm, const vec3d& ptaxis, const vec3d& axis, double curr_min_angle, int ccw, vec3d &p1, vec3d &p2 )
{
    return m_TBox.MinAngle( org, norm, ptaxis, axis, curr_min_angle, ccw, p1, p2 );
}

void TMesh::Split()
{
    int t;
    for ( t = 0 ; t < ( int )m_TVec.size() ; t++ )
    {
        //printf( "Splitting tri %d\n", t );
        bool dumpCase = false;
        if ( false && t == 198 )
        {
            dumpCase = true;
        }
        bool erf = m_TVec[ t ]->SplitTri( dumpCase );
        if ( !erf )
        {
//            printf( "Fail in triangle %d\n", t );
        }
    }
}

void TMesh::MakeFromPGMesh( PGMesh *m )
{
    list< PGFace* >::iterator f;
    for ( f = m->m_FaceList.begin() ; f != m->m_FaceList.end(); ++f )
    {
        vector < PGNode* > nodVec;
        ( *f )->GetNodesAsTris( nodVec );

        int tag = ( *f )->m_Tag;
        vec3d norm = ( *f )->m_Nvec;
        vector < int > tags = m->m_PGMulti->GetTagVec( tag );
        int iQuad = ( *f )->m_iQuad;
        int jref = ( *f )->m_jref;
        int kref = ( *f )->m_kref;

        int npt = nodVec.size();
        int ntri = npt/3;

        int inod = 0;
        for ( int i = 0; i < ntri; i++ )
        {
            vec2d uw;

            vec3d v0 =  nodVec[ inod ]->m_Pt->m_Pnt;
            nodVec[ inod ]->GetUW( tag, uw );
            vec3d uw0( uw.x(), uw.y(), 0.0 );
            inod++;

            vec3d v1 =  nodVec[ inod ]->m_Pt->m_Pnt;
            nodVec[ inod ]->GetUW( tag, uw );
            vec3d uw1( uw.x(), uw.y(), 0.0 );
            inod++;

            vec3d v2 =  nodVec[ inod ]->m_Pt->m_Pnt;
            nodVec[ inod ]->GetUW( tag, uw );
            vec3d uw2( uw.x(), uw.y(), 0.0 );
            inod++;

            AddTri( v0, v1, v2, norm, uw0, uw1, uw2, iQuad, jref, kref );
            TTri* tri = m_TVec.back();

            tri->m_Tags = tags;
        }
    }
}

void TMesh::SubMesh( int imesh, TMesh *tm )
{
    int nvert = tm->m_NVec.size();

    vector < bool > copyvert( nvert, false );
    int ntri_inmesh = 0;
    int nvert_inmesh = 0;

    for ( int ivert = 0; ivert < nvert; ivert++ )
    {
        tm->m_NVec[ivert]->m_ID = ivert;
    }

    for ( int itri = 0; itri < tm->m_TVec.size(); itri++ )
    {
        TTri *torig = tm->m_TVec[ itri ];

        if ( torig->m_ID == imesh && torig->m_iQuad >= 0 )
        {
            copyvert[ torig->m_N0->m_ID ] = true;
            copyvert[ torig->m_N1->m_ID ] = true;
            copyvert[ torig->m_N2->m_ID ] = true;
            ntri_inmesh++;
        }
    }

    for ( int ivert = 0; ivert < nvert; ivert++ )
    {
        if ( copyvert[ ivert ] )
        {
            nvert_inmesh++;
        }
    }

    vector < int > vertxref( nvert, -1 );

    m_NVec.resize( nvert_inmesh );

    // Copy needed verts.
    int ivert = 0;
    for ( int i = 0; i < nvert; i++ )
    {
        if ( copyvert[i] )
        {
            m_NVec[ ivert ] = new TNode();
            m_NVec[ ivert ]->CopyFrom( tm->m_NVec[i] );
            vertxref[i] = ivert;
            ivert++;
        }
    }

    // Copy desired tris.
    m_TVec.reserve( ntri_inmesh );

    for ( int itri = 0; itri < tm->m_TVec.size(); itri++ )
    {
        TTri *torig = tm->m_TVec[ itri ];

        if ( torig->m_ID == imesh && torig->m_iQuad >= 0 )
        {
            TTri *t = new TTri( this );

            t->m_N0 = m_NVec[ vertxref[ torig->m_N0->m_ID ] ];
            t->m_N1 = m_NVec[ vertxref[ torig->m_N1->m_ID ] ];
            t->m_N2 = m_NVec[ vertxref[ torig->m_N2->m_ID ] ];

            t->m_Norm = torig->m_Norm;
            t->m_Density = torig->m_Density;
            t->m_Tags = torig->m_Tags;
            t->m_GeomID = torig->m_GeomID;
            t->m_InvalidFlag = torig->m_InvalidFlag;
            t->m_IgnoreTriFlag = torig->m_IgnoreTriFlag;
            t->m_iQuad = torig->m_iQuad;
            t->m_jref = torig->m_jref;
            t->m_kref = torig->m_kref;
            t->m_ID = torig->m_ID;

            m_TVec.push_back( t );
        }
    }
}

void TMesh::MakeUWTMesh( TMesh & tm )
{
    tm.CopyAttributes( this );

    int nnod = m_NVec.size();
    tm.m_NVec.reserve( nnod );

    for ( int i = 0; i < nnod; i++ )
    {
        TNode *n = new TNode();

        m_NVec[i]->m_ID = i;
        n->m_ID = i;

        // Note swap of Pnt and UWPnt.
        n->m_Pnt = m_NVec[i]->m_UWPnt;
        n->m_UWPnt = m_NVec[i]->m_Pnt;

        tm.m_NVec.push_back( n );
    }

    int ntri = m_TVec.size();
    tm.m_TVec.reserve( ntri );

    for ( int i = 0; i < ntri; i++ )
    {
        TTri *t = new TTri( &tm );
        TTri *ti = m_TVec[ i ];

        t->m_N0 = tm.m_NVec[ ti->m_N0->m_ID ];
        t->m_N1 = tm.m_NVec[ ti->m_N1->m_ID ];
        t->m_N2 = tm.m_NVec[ ti->m_N2->m_ID ];

        // t->m_Norm = ti->m_Norm;
        // t->CompNorm();
        t->m_Norm = vec3d( 0, 0, 1.0 );
        t->m_Density = ti->m_Density;
        t->m_Tags = ti->m_Tags;
        t->m_ID = ti->m_ID;
        t->m_InvalidFlag = ti->m_InvalidFlag;
        t->m_IgnoreTriFlag = ti->m_IgnoreTriFlag;
        t->m_iQuad = ti->m_iQuad;
        t->m_jref = ti->m_jref;
        t->m_kref = ti->m_kref;

        tm.m_TVec.push_back( t );
    }
}

void TMesh::SetIgnoreTriFlag( const vector < int > & bTypes, const vector < bool > & thicksurf )
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

void TMesh::SetIgnoreSubSurface()
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
                if ( tri->m_SplitVec[s]->m_Tags.size() > 1 )
                {
                    tri->m_SplitVec[s]->m_IgnoreTriFlag = true;
                }
            }
        }
        else
        {
            if ( tri->m_Tags.size() > 1 )
            {
                tri->m_IgnoreTriFlag = true;
            }
        }
    }
}

void TMesh::SetIgnoreInsideAny()
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
                vector < bool > vec = tri->m_SplitVec[s]->m_insideSurf;
                tri->m_SplitVec[s]->m_IgnoreTriFlag = std::any_of(vec.begin(), vec.end(), [](bool v) { return v; });
            }
        }
        else
        {
            vector < bool > vec = tri->m_insideSurf;
            tri->m_IgnoreTriFlag = std::any_of(vec.begin(), vec.end(), [](bool v) { return v; });
        }
    }
}

void TMesh::SetIgnoreInsideAll()
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
                vector < bool > vec = tri->m_SplitVec[s]->m_insideSurf;
                tri->m_SplitVec[s]->m_IgnoreTriFlag = std::all_of(vec.begin(), vec.end(), [](bool v) { return v; });
            }
        }
        else
        {
            vector < bool > vec = tri->m_insideSurf;
            tri->m_IgnoreTriFlag = std::all_of(vec.begin(), vec.end(), [](bool v) { return v; });
        }
    }
}

void TMesh::SetIgnoreInsideNotOne()
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
                vector < bool > vec = tri->m_SplitVec[s]->m_insideSurf;
                int c = std::count( vec.begin(), vec.end(), true );
                tri->m_SplitVec[s]->m_IgnoreTriFlag = ( c != 1 );
            }
        }
        else
        {
            vector < bool > vec = tri->m_insideSurf;
            int c = std::count( vec.begin(), vec.end(), true );
            tri->m_IgnoreTriFlag = ( c != 1 );
        }
    }
}

void TMesh::SetIgnoreOutsideAll()
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
                vector < bool > vec = tri->m_SplitVec[s]->m_insideSurf;
                tri->m_SplitVec[s]->m_IgnoreTriFlag = std::none_of(vec.begin(), vec.end(), [](bool v) { return v; });
            }
        }
        else
        {
            vector < bool > vec = tri->m_insideSurf;
            tri->m_IgnoreTriFlag = std::none_of(vec.begin(), vec.end(), [](bool v) { return v; });
        }
    }
}

void TMesh::SetIgnoreMatchMask( const vector < bool > & mask )
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
                tri->m_SplitVec[s]->m_IgnoreTriFlag = ( mask == tri->m_SplitVec[s]->m_insideSurf );
            }
        }
        else
        {
            tri->m_IgnoreTriFlag = ( mask == tri->m_insideSurf );
        }
    }
}

void TMesh::SetKeepMatchMask( const vector < bool > & mask )
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
                tri->m_SplitVec[s]->m_IgnoreTriFlag = ( mask != tri->m_SplitVec[s]->m_insideSurf );
            }
        }
        else
        {
            tri->m_IgnoreTriFlag = ( mask != tri->m_insideSurf );
        }
    }
}

void TMesh::SetIgnoreShadow( const vec3d & v, double tol )
{
    for ( int t = 0 ; t < ( int )m_TVec.size() ; t++ )
    {
        TTri* tri = m_TVec[t];

        //==== Do Interior Tris ====//
        if ( tri->m_SplitVec.size() )
        {
            for ( int s = 0 ; s < ( int )tri->m_SplitVec.size() ; s++ )
            {
                if ( dot( tri->m_SplitVec[s]->m_Norm, v ) >= -tol )
                {
                    tri->m_SplitVec[s]->m_IgnoreTriFlag = true;
                }
            }
        }
        else
        {
            if ( dot( tri->m_Norm, v ) >= -tol )
            {
                tri->m_IgnoreTriFlag = true;
            }
        }
    }
}

void TMesh::IgnoreYLessThan( const double & ytol )
{
    for ( int t = 0 ; t < ( int )m_TVec.size() ; t++ )
    {
        TTri* tri = m_TVec[t];

        //==== Do Interior Tris ====//
        if ( tri->m_SplitVec.size() )
        {
            for ( int s = 0 ; s < ( int )tri->m_SplitVec.size() ; s++ )
            {
                vec3d cen = tri->m_SplitVec[s]->ComputeCenter();
                if ( cen.y() < ytol )
                {
                    tri->m_SplitVec[s]->m_IgnoreTriFlag = true;
                }
            }
        }
        else
        {
            vec3d cen = tri->ComputeCenter();
            if ( cen.y() < ytol )
            {
                tri->m_IgnoreTriFlag = true;
            }
        }
    }
}

void TMesh::SetIgnoreAbovePlane( const vector <vec3d> & threepts )
{
    for ( int t = 0 ; t < ( int )m_TVec.size() ; t++ )
    {
        TTri* tri = m_TVec[t];

        //==== Do Interior Tris ====//
        if ( tri->m_SplitVec.size() )
        {
            for ( int s = 0 ; s < ( int )tri->m_SplitVec.size() ; s++ )
            {
                vec3d cen = tri->m_SplitVec[s]->ComputeCenter();

                if ( orient3d( (double*) threepts[0].v, (double*) threepts[1].v, (double*) threepts[2].v, (double*) cen.v ) < 0 )
                {
                    tri->m_SplitVec[s]->m_IgnoreTriFlag = true;
                }
            }
        }
        else
        {
            vec3d cen = tri->ComputeCenter();
            if ( orient3d( (double*) threepts[0].v, (double*) threepts[1].v, (double*) threepts[2].v, (double*) cen.v ) < 0 )
            {
                tri->m_IgnoreTriFlag = true;
            }
        }
    }
}

void TMesh::IgnoreAll()
{
    for ( int t = 0 ; t < ( int )m_TVec.size() ; t++ )
    {
        TTri* tri = m_TVec[t];

        //==== Do Interior Tris ====//
        if ( tri->m_SplitVec.size() )
        {
            for ( int s = 0 ; s < ( int )tri->m_SplitVec.size() ; s++ )
            {
                tri->m_SplitVec[s]->m_IgnoreTriFlag = true;
            }
        }
        else
        {
            tri->m_IgnoreTriFlag = true;
        }
    }
}

void TMesh::IgnoreNone()
{
    for ( int t = 0 ; t < ( int )m_TVec.size() ; t++ )
    {
        TTri* tri = m_TVec[t];

        tri->m_IgnoreTriFlag = false;
        //==== Do Interior Tris ====//
        if ( tri->m_SplitVec.size() )
        {
            for ( int s = 0 ; s < ( int )tri->m_SplitVec.size() ; s++ )
            {
                tri->m_SplitVec[s]->m_IgnoreTriFlag = false;
            }
        }
    }
}

void TMesh::DeterIntExt( const vector< TMesh* >& meshVec, const vec3d &dir )
{
    for ( int t = 0 ; t < ( int )m_TVec.size() ; t++ )
    {
        TTri* tri = m_TVec[t];

        //==== Do Interior Tris ====//
        if ( tri->m_SplitVec.size() )
        {
            for ( int s = 0 ; s < ( int )tri->m_SplitVec.size() ; s++ )
            {
                DeterIntExtTri( tri->m_SplitVec[s], meshVec, dir );
            }
        }
        else
        {
            DeterIntExtTri( tri, meshVec, dir );
        }
    }
}

void TMesh::DeterIntExt( TMesh* mesh, const vec3d &dir )
{
    vector <TMesh*> tmv;
    tmv.push_back( mesh );
    DeterIntExt( tmv, dir );
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

double TMesh::ComputeWaveDragArea( const std::unordered_map< string, int > &idmap )
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

                    std::unordered_map<string, int>::const_iterator it = idmap.find( tri->m_SplitVec[s]->m_GeomID );
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

            std::unordered_map<string, int>::const_iterator it = idmap.find( tri->m_GeomID );
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

void TMesh::FlipNormals()
{
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
                    tri->m_SplitVec[s]->FlipTri();
                }
            }
        }
        else if ( !tri->m_IgnoreTriFlag )
        {
            tri->FlipTri();
        }
    }
}

void TMesh::Transform( const Matrix4d & TransMat )
{
    // Build Map of nodes
    map< TNode*, int > nodeMap;
    for ( int j = 0 ; j < ( int )m_NVec.size() ; j++ )
    {
        TNode* n = m_NVec[j];
        nodeMap[n] = 1;
    }
    //==== Split Tris ====//
    for ( int j = 0 ; j < ( int )m_TVec.size() ; j++ )
    {
        TTri* t = m_TVec[j];
        for ( int k = 0 ; k < ( int )t->m_NVec.size() ; k++ )
        {
            TNode* n = t->m_NVec[k];
            nodeMap[n] = 1;
        }
    }

    // Apply Transformation to Nodes
    map<TNode*, int >::const_iterator iter;
    for ( iter = nodeMap.begin() ; iter != nodeMap.end() ; ++iter )
    {
        TNode* n = iter->first;
        n->m_Pnt = TransMat.xform( n->m_Pnt );
    }

    // Apply Transformation to each triangle's normal vector
    for ( int j = 0 ; j < ( int )m_TVec.size() ; j++ )
    {
        if ( m_TVec[j]->m_SplitVec.size() )
        {
            for ( int t = 0 ; t < ( int ) m_TVec[j]->m_SplitVec.size() ; t++ )
            {
                TTri* tri = m_TVec[j]->m_SplitVec[t];
                tri->m_Norm = TransMat.xformnorm( tri->m_Norm );
            }
        }
        else
        {
            TTri* tri = m_TVec[j];
            tri->m_Norm = TransMat.xformnorm( tri->m_Norm );
        }
    }

    // Apply Transformation to Mesh's area center
    m_AreaCenter = TransMat.xform( m_AreaCenter );
}

// Wrapper
void TMesh::AddTri( const vec3d &p0, const vec3d &p1, const vec3d &p2, const int &iQuad )
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

    AddTri( p0, p1, p2, norm, iQuad );
}

void TMesh::AddTri( const vec3d &p0, const vec3d &p1, const vec3d &p2, const int &iQuad, const int & jref, const int & kref )
{
    AddTri( p0, p1, p2, iQuad );
    TTri* tri = m_TVec.back();
    tri->m_jref = jref;
    tri->m_kref = kref;
}

// Base.  i.e. does m_TVec.push_back()
void TMesh::AddTri( const vec3d &v0, const vec3d &v1, const vec3d &v2, const vec3d &norm, const int &iQuad )
{
    // Use For XYZ Tri
    TTri* ttri = new TTri( this );
    ttri->m_Norm = norm;

    ttri->m_iQuad = iQuad;

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

void TMesh::AddTri( const vec3d &v0, const vec3d &v1, const vec3d &v2, const vec3d &norm, const int &iQuad, const int & jref, const int & kref )
{
    AddTri( v0, v1, v2, norm, iQuad );
    TTri* tri = m_TVec.back();
    tri->m_jref = jref;
    tri->m_kref = kref;
}

// Base
void TMesh::AddTri( TNode* node0, TNode* node1, TNode* node2, const vec3d & norm, const int & iQuad )
{
    TTri* ttri = new TTri( this );
    ttri->m_Norm = norm;

    ttri->m_iQuad = iQuad;

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

void TMesh::AddTri( TNode* node0, TNode* node1, TNode* node2, const vec3d & norm, const int & iQuad, const int & jref, const int & kref )
{
    AddTri( node0, node1, node2, norm, iQuad );
    TTri* tri = m_TVec.back();
    tri->m_jref = jref;
    tri->m_kref = kref;
}

// Wrapper
void TMesh::AddTri( const vec3d & v0, const vec3d & v1, const vec3d & v2, const vec3d & norm, const vec3d & uw0,
                    const vec3d & uw1, const vec3d & uw2, const int & iQuad, const int & jref, const int & kref ){
    // AddTri with both xyz and uw info
    AddTri( v0, v1, v2, norm, iQuad );
    TTri* tri = m_TVec.back();
    tri->m_N0->m_UWPnt = uw0;
    tri->m_N1->m_UWPnt = uw1;
    tri->m_N2->m_UWPnt = uw2;

    tri->m_N0->SetCoordInfo( TNode::HAS_XYZ | TNode::HAS_UW );
    tri->m_N1->SetCoordInfo( TNode::HAS_XYZ | TNode::HAS_UW );
    tri->m_N2->SetCoordInfo( TNode::HAS_XYZ | TNode::HAS_UW );

    tri->m_jref = jref;
    tri->m_kref = kref;
}

// Base
void TMesh::AddTri( const TTri* tri)
{
    // Copies an existing triangle and pushes back into the existing
    TTri* new_tri = new TTri( this );

    new_tri->CopyFrom( tri );
    m_TVec.push_back( new_tri );
    m_NVec.push_back( new_tri->m_N0 );
    m_NVec.push_back( new_tri->m_N1 );
    m_NVec.push_back( new_tri->m_N2 );
}

void TMesh::AddUWTri( const vec3d & uw0, const vec3d & uw1, const vec3d & uw2, const vec3d & norm, const int & iQuad )
{
    // Use For XYZ Tri
    TTri* ttri = new TTri( this );
    ttri->m_Norm = norm;

    ttri->m_iQuad = iQuad;

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

void TMesh::UpdateBBox( BndBox &bbox, const Matrix4d &transMat )
{
    int j;

    for ( j = 0 ; j < ( int )m_TVec.size() ; j++ )
    {
        bbox.Update( transMat.xform( m_TVec[j]->m_N0->m_Pnt ) );
        bbox.Update( transMat.xform( m_TVec[j]->m_N1->m_Pnt ) );
        bbox.Update( transMat.xform( m_TVec[j]->m_N2->m_Pnt ) );
    }
}

void TMesh::UpdateBBox( const Matrix4d &mat, BndBox & bb )
{
    for ( int j = 0 ; j < ( int )m_TVec.size() ; j++ )
    {
        bb.Update( mat.xform( m_TVec[j]->m_N0->m_Pnt ) );
        bb.Update( mat.xform( m_TVec[j]->m_N1->m_Pnt ) );
        bb.Update( mat.xform( m_TVec[j]->m_N2->m_Pnt ) );
    }
}

void TMesh::ForceSmallYZero()
{
    double tol = 1e-10;
    for ( int i = 0; i < m_NVec.size(); i++ )
    {
        if ( std::abs( m_NVec[i]->m_Pnt.y() ) < tol )
        {
            m_NVec[i]->m_Pnt.set_y( 0.0 );
        }
    }
}

//==== Write STL Tris =====//
void TMesh::WriteIgnoredSTLTris( FILE* file_id, Matrix4d XFormMat )
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
                if ( tri->m_SplitVec[s]->m_IgnoreTriFlag )
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
            if ( tri->m_IgnoreTriFlag )
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

//==== Write STL Tris =====//
void TMesh::WriteSTLTris( FILE* file_id, Matrix4d XFormMat )
{
    int t, s;
    vec3d norm;
    vec3d v0, v1, v2;
    vec3d d21;

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

void TMesh::WriteInsideMStlTris( FILE* file_id, Matrix4d XFormMat, int minside )
{
    int t, s;
    vec3d norm;
    vec3d v0, v1, v2;
    vec3d d21;

    for ( t = 0 ; t < ( int )m_TVec.size() ; t++ )
    {
        TTri* tri = m_TVec[t];

        if ( tri->m_SplitVec.size() )
        {
            for ( s = 0 ; s < ( int )tri->m_SplitVec.size() ; s++ )
            {
                if ( tri->m_SplitVec[s]->m_insideSurf.size() > minside )
                {
                    if ( !tri->m_SplitVec[s]->m_insideSurf[ minside ] )
                    {
                        continue;
                    }
                }

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
            if ( tri->m_insideSurf.size() > minside )
            {
                if ( !tri->m_insideSurf[ minside ] )
                {
                    continue;
                }
            }

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


void TMesh::WriteIgnoredSTL( string fname, double scale )
{
    FILE* fp = fopen( fname.c_str(), "w" );

    if ( !fp )
    {
        return;
    }

    fprintf( fp, "solid\n" );
    Matrix4d mScale;
    mScale.scale( scale );
    WriteIgnoredSTLTris( fp, mScale );
    fprintf( fp, "endsolid\n" );
    fclose( fp );
}

void TMesh::WriteSTL( string fname, double scale )
{
    FILE* fp = fopen( fname.c_str(), "w" );

    if ( !fp )
    {
        return;
    }

    fprintf( fp, "solid\n" );
    Matrix4d mScale;
    mScale.scale( scale );
    WriteSTLTris( fp, mScale );
    fprintf( fp, "endsolid\n" );
    fclose( fp );
}

void TMesh::WriteOBJ( string fname, double scale )
{
    FILE* fp = fopen( fname.c_str(), "w" );

    if ( !fp )
    {
        return;
    }

    int npt = m_NVec.size();
    int ntri = m_TVec.size();

    for ( int i = 0; i < npt; i++ )
    {
        m_NVec[i]->m_ID = i;

        vec3d v = m_NVec[i]->m_Pnt;
        fprintf( fp, "v %16.10g %16.10g %16.10g\n", v.x() * scale, v.y() * scale,  v.z() * scale );
    }

    fprintf( fp, "g Surface\n" );

    for ( int i = 0; i < ntri; i++ )
    {
        TTri *ttri = m_TVec[i];
        fprintf( fp, "f %d %d %d\n", ttri->m_N0->m_ID + 1,  ttri->m_N1->m_ID + 1, ttri->m_N2->m_ID + 1 );
    }
    fclose( fp );

}

void TMesh::WriteTRI( string fname, double scale )
{
    FILE* fp = fopen( fname.c_str(), "w" );

    if ( !fp )
    {
        return;
    }

    int npt = m_NVec.size();
    int ntri = m_TVec.size();

    fprintf( fp, "%d\n", npt );
    fprintf( fp, "%d\n", ntri  );

    for ( int i = 0; i < npt; i++ )
    {
        m_NVec[i]->m_ID = i;

        vec3d v = m_NVec[i]->m_Pnt;
        fprintf( fp, "%16.10g %16.10g %16.10g\n", v.x() * scale, v.y() * scale,  v.z() * scale );
    }

    for ( int i = 0; i < ntri; i++ )
    {
        TTri *ttri = m_TVec[i];
        fprintf( fp, "%d %d %d\n", ttri->m_N0->m_ID + 1,  ttri->m_N1->m_ID + 1, ttri->m_N2->m_ID + 1 );
    }
    fclose( fp );
}

void TMesh::WriteVSPGeom( const string file_name )
{
    vector < deque < TEdge > > wakes;
    IdentifyWakes( wakes );

    //==== Open file ====//
    FILE *file_id = fopen( file_name.c_str(), "w" );

    fprintf( file_id, "# vspgeom v3\n" );
    fprintf( file_id, "1\n" );  // Number of meshes.
    fprintf( file_id, "%d %d %d\n", m_NVec.size(),
                                    m_TVec.size(),
                                    wakes.size() );
    WriteVSPGeomPnts( file_id );
    WriteVSPGeomTris( file_id );
    WriteVSPGeomParts( file_id );
    WriteVSPGeomParents( file_id );
    WriteVSPGeomWakes( file_id, wakes );
    WriteVSPGeomAlternateTris( file_id );
    WriteVSPGeomAlternateParts( file_id );

    fclose( file_id );
}

void TMesh::WriteVSPGeomPnts( FILE* file_id )
{
    //==== Write Out Nodes ====//
    vec3d v;
    Matrix4d XFormMat;
    fprintf( file_id, "%d\n", m_NVec.size() );

    for ( int i = 0 ; i < ( int )m_NVec.size() ; i++ )
    {
        TNode* tnode = m_NVec[i];
        // Apply Transformations
        v = XFormMat.xform( tnode->m_Pnt );
        fprintf( file_id, "%16.10g %16.10g %16.10g\n", v.x(), v.y(), v.z() ); // , tnode->m_UWPnt.x(), tnode->m_UWPnt.y() );
    }
}

void TMesh::WriteVSPGeomTris( FILE* file_id )
{
    fprintf( file_id, "%d\n", m_TVec.size()  );

    //==== Write Out Tris ====//
    for ( int t = 0 ; t < ( int )m_TVec.size() ; t++ )
    {
        TTri* ttri = m_TVec[t];
        fprintf(file_id, "3 %d %d %d\n", ttri->m_N0->m_ID + 1, ttri->m_N1->m_ID + 1, ttri->m_N2->m_ID + 1 );
    }
}

void TMesh::WriteVSPGeomAlternateTris( FILE* file_id )
{
    //==== Write Out Tris ====//
    for ( int t = 0 ; t < ( int )m_TVec.size() ; t++ )
    {
        TTri* ttri = m_TVec[t];
        fprintf(file_id, "%d 1 %d %d %d\n", t + 1, ttri->m_N0->m_ID + 1, ttri->m_N1->m_ID + 1, ttri->m_N2->m_ID + 1 );
    }
}

void TMesh::WriteVSPGeomParts( FILE* file_id  )
{
    //==== Write Component IDs for each Tri =====//
    int part, tag;
    for ( int t = 0 ; t < ( int )m_TVec.size() ; t++ )
    {
        TTri* ttri = m_TVec[t];
        tag = SubSurfaceMgr.GetTag( ttri->m_Tags );
        part = SubSurfaceMgr.GetPart( ttri->m_Tags );
        double uscale = SubSurfaceMgr.m_CompUscale[ part - 1 ];
        double wscale = SubSurfaceMgr.m_CompWscale[ part - 1 ];

        fprintf( file_id, "%d %d %16.10g %16.10g %16.10g %16.10g %16.10g %16.10g\n", part, tag,
                 ttri->m_N0->m_UWPnt.x() / uscale, ttri->m_N0->m_UWPnt.y() / wscale,
                 ttri->m_N1->m_UWPnt.x() / uscale, ttri->m_N1->m_UWPnt.y() / wscale,
                 ttri->m_N2->m_UWPnt.x() / uscale, ttri->m_N2->m_UWPnt.y() / wscale );
    }
}

void TMesh::WriteVSPGeomParents( FILE* file_id )
{
    for ( int t = 0 ; t < ( int )m_TVec.size() ; t++ )
    {
        fprintf( file_id, "%d %d\n", t + 1, t + 1 );
    }
}

void TMesh::WriteVSPGeomAlternateParts( FILE* file_id  )
{
    //==== Write Component IDs for each Tri =====//
    int part, tag;
    for ( int t = 0 ; t < ( int )m_TVec.size() ; t++ )
    {
        TTri* ttri = m_TVec[t];
        tag = SubSurfaceMgr.GetTag( ttri->m_Tags );
        part = SubSurfaceMgr.GetPart( ttri->m_Tags );
        double uscale = SubSurfaceMgr.m_CompUscale[ part - 1 ];
        double wscale = SubSurfaceMgr.m_CompWscale[ part - 1 ];

        fprintf( file_id, "%d %d %d %16.10g %16.10g %16.10g %16.10g %16.10g %16.10g\n", t + 1, part, tag,
                 ttri->m_N0->m_UWPnt.x() / uscale, ttri->m_N0->m_UWPnt.y() / wscale,
                 ttri->m_N1->m_UWPnt.x() / uscale, ttri->m_N1->m_UWPnt.y() / wscale,
                 ttri->m_N2->m_UWPnt.x() / uscale, ttri->m_N2->m_UWPnt.y() / wscale );
    }
}

int TMesh::WriteVSPGeomPartTagTris( FILE* file_id, int tri_offset, int part, int tag )
{
    for ( int t = 0 ; t < ( int )m_TVec.size() ; t++ )
    {
        TTri* ttri = m_TVec[t];
        if ( SubSurfaceMgr.MatchPartAndTag( ttri->m_Tags, part, tag ) )
        {
            fprintf( file_id, "%d\n", t + tri_offset + 1 );
        }
    }
    return tri_offset + m_TVec.size();
}

int TMesh::CountVSPGeomPartTagTris( int part, int tag )
{
    int count = 0;
    for ( int t = 0 ; t < ( int )m_TVec.size() ; t++ )
    {
        TTri* ttri = m_TVec[t];
        if ( SubSurfaceMgr.MatchPartAndTag( ttri->m_Tags, part, tag ) )
        {
            count++;
        }
    }
    return count;
}

// Wake edges are created such that N0.u < N1.u.
// This comparator sorts first by sgn(N0.y), abs(N0.y), then N0.u and N1.u.
bool TMOrderWakeEdges ( const TEdge &a, const TEdge &b )
{
    if ( sgn( a.m_N0->m_Pnt.y() ) < sgn( b.m_N0->m_Pnt.y() ) ) return true;
    if ( sgn( b.m_N0->m_Pnt.y() ) < sgn( a.m_N0->m_Pnt.y() ) ) return false;

    if ( abs( a.m_N0->m_Pnt.y() ) < abs( b.m_N0->m_Pnt.y() ) ) return true;
    if ( abs( b.m_N0->m_Pnt.y() ) < abs( a.m_N0->m_Pnt.y() ) ) return false;

    if ( a.m_N0->m_UWPnt.x() < b.m_N0->m_UWPnt.x() ) return true;
    if ( b.m_N0->m_UWPnt.x() < a.m_N0->m_UWPnt.x() ) return false;

    if ( a.m_N1->m_UWPnt.x() < b.m_N1->m_UWPnt.x() ) return true;
    if ( b.m_N1->m_UWPnt.x() < a.m_N1->m_UWPnt.x() ) return false;

    if ( a.m_N0->m_Pnt.x() < b.m_N0->m_Pnt.x() ) return true;
    if ( b.m_N0->m_Pnt.x() < a.m_N0->m_Pnt.x() ) return false;

    if ( a.m_N0->m_Pnt.z() < b.m_N0->m_Pnt.z() ) return true;
    if ( b.m_N0->m_Pnt.z() < a.m_N0->m_Pnt.z() ) return false;

    if ( sgn( a.m_N1->m_Pnt.y() ) < sgn( b.m_N1->m_Pnt.y() ) ) return true;
    if ( sgn( b.m_N1->m_Pnt.y() ) < sgn( a.m_N1->m_Pnt.y() ) ) return false;

    if ( abs( a.m_N1->m_Pnt.y() ) < abs( b.m_N1->m_Pnt.y() ) ) return true;
    if ( abs( b.m_N1->m_Pnt.y() ) < abs( a.m_N1->m_Pnt.y() ) ) return false;

    if ( a.m_N1->m_Pnt.x() < b.m_N1->m_Pnt.x() ) return true;
    if ( b.m_N1->m_Pnt.x() < a.m_N1->m_Pnt.x() ) return false;

    if ( a.m_N1->m_Pnt.z() < b.m_N1->m_Pnt.z() ) return true;
    if ( b.m_N1->m_Pnt.z() < a.m_N1->m_Pnt.z() ) return false;

    return false;
}

bool TMAboutEqualWakeNodes ( TNode *a, TNode *b )
{
    if ( aboutequal( a->m_Pnt.y(), b->m_Pnt.y() )
         && aboutequal( a->m_Pnt.x(), b->m_Pnt.x() )
         && aboutequal( a->m_Pnt.z(), b->m_Pnt.z() )
         && aboutequal( a->m_UWPnt.x(), b->m_UWPnt.x() )
         && aboutequal( a->m_UWPnt.y(), b->m_UWPnt.y() ) ) return true;

    return false;
}

bool TMAboutEqualWakeEdges ( const TEdge &a, const TEdge &b )
{
    if ( TMAboutEqualWakeNodes( a.m_N0, b.m_N0 )
         && TMAboutEqualWakeNodes( a.m_N1, b.m_N1 ) ) return true;

    return false;
}

void TMesh::IdentifyWakes( vector < deque < TEdge > > &wakes )
{
    vector < TEdge > wakeedges;

    for ( int t = 0 ; t < ( int )m_TVec.size() ; t++ )
    {
        TTri *ttri = m_TVec[t];
        int we = ttri->WakeEdge();

        if ( we > 0 )
        {
            TEdge e;
            if ( we == 1 )
            {
                e = TEdge( ttri->m_N0, ttri->m_N1, ttri );
            }
            else if ( we == 2 )
            {
                e = TEdge( ttri->m_N1, ttri->m_N2, ttri );
            }
            else
            {
                e = TEdge( ttri->m_N2, ttri->m_N0, ttri );
            }
            e.SortNodesByU();
            wakeedges.push_back( e );
        }
    }

    sort( wakeedges.begin(), wakeedges.end(), TMOrderWakeEdges );

    vector < TEdge >::iterator it;
    it = unique( wakeedges.begin(), wakeedges.end(), TMAboutEqualWakeEdges );

    wakeedges.resize( distance( wakeedges.begin(), it ) );

    list < TEdge > wlist( wakeedges.begin(), wakeedges.end() );


    wakes.clear();
    int iwake = 0;

    while ( !wlist.empty() )
    {
        list < TEdge >::iterator wit = wlist.begin();

        iwake = wakes.size();
        wakes.resize( iwake + 1 );
        wakes[iwake].push_back( *wit );
        wit = wlist.erase( wit );

        while ( wit != wlist.end() )
        {
            if ( TMAboutEqualWakeNodes( wakes[iwake].back().m_N1, (*wit).m_N0 ) )
            {
                wakes[iwake].push_back( *wit );
                wlist.erase( wit );
                wit = wlist.begin();
                continue;
            }
            else if ( TMAboutEqualWakeNodes( wakes[iwake].begin()->m_N0, (*wit).m_N1 ) )
            {
                wakes[iwake].push_front( *wit );
                wlist.erase( wit );
                wit = wlist.begin();
                continue;
            }
            wit++;
        }
    }

    int nwake = wakes.size();
}

void TMesh::WriteVSPGeomWakes( FILE* file_id, vector < deque < TEdge > > &wakes )
{
    int nwake = wakes.size();

    fprintf( file_id, "%d\n", nwake );

    for ( int iwake = 0; iwake < nwake; iwake++ )
    {
        int iprt = 0;
        int iwe;
        int nwe = wakes[iwake].size();

        fprintf( file_id, "%d ", nwe + 1 );

        for ( iwe = 0; iwe < nwe; iwe++ )
        {
            fprintf( file_id, "%d", wakes[iwake][iwe].m_N0->m_ID + 1 );

            if ( iprt < 9 )
            {
                fprintf( file_id, " " );
                iprt++;
            }
            else
            {
                fprintf( file_id, "\n" );
                iprt = 0;
            }
        }
        fprintf( file_id, "%d\n", wakes[iwake][iwe - 1].m_N1->m_ID + 1 );
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

TTri::TTri( TMesh* tmesh )
{
    static int cnt = 0;
    cnt++;
//printf("Tri Construct Cnt = %d \n", cnt);
    m_E0 = m_E1 = m_E2 = 0;
    m_N0 = m_N1 = m_N2 = 0;
    m_IgnoreTriFlag = false;
    m_InvalidFlag  = false;
    m_Density = 1.0;
    m_TMesh = tmesh;
    m_PEArr[0] = m_PEArr[1] = m_PEArr[2] = nullptr;
    m_iQuad = -1;
    m_ID = -1;
    m_jref = 0;
    m_kref = 0;
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

bool TTri::CleanupEdgeVec()
{
    bool unexpected = false;
    //==== Delete Split Edges ====//
    for ( int i = 0 ; i < ( int )m_EVec.size() ; i++ )
    {
        delete m_EVec[i];
        unexpected = true;
    }
    m_EVec.clear();
    return unexpected;
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
    m_GeomID = tri->m_GeomID;
    m_InvalidFlag = tri->m_InvalidFlag;
    m_IgnoreTriFlag = tri->m_IgnoreTriFlag;
    m_iQuad = tri->m_iQuad;
    m_jref = tri->m_jref;
    m_kref = tri->m_kref;
    m_ID = tri->m_ID;
}

void TTri::BuildPermEdges()
{
    if ( m_PEArr[0] )
    {
        for ( int i = 0 ; i < 3 ; i++ )
        {
            delete m_PEArr[i];
            m_PEArr[i] = nullptr;
        }
    }
    if ( m_N0 != nullptr && m_N1 != nullptr && m_N2 != nullptr )
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

bool TTri::MergeSharedEdges( TTri* t )
{
    double tol = 1.0e-12;

    if ( MatchMergeEdge( m_N0, m_N1, t->m_N0, t->m_N1, tol ) )
    {
        return true;
    }
    if ( MatchMergeEdge( m_N0, m_N1, t->m_N1, t->m_N2, tol ) )
    {
        return true;
    }
    if ( MatchMergeEdge( m_N0, m_N1, t->m_N0, t->m_N2, tol ) )
    {
        return true;
    }
    if ( MatchMergeEdge( m_N1, m_N2, t->m_N0, t->m_N1, tol ) )
    {
        return true;
    }
    if ( MatchMergeEdge( m_N1, m_N2, t->m_N1, t->m_N2, tol ) )
    {
        return true;
    }
    if ( MatchMergeEdge( m_N1, m_N2, t->m_N0, t->m_N2, tol ) )
    {
        return true;
    }
    if ( MatchMergeEdge( m_N0, m_N2, t->m_N0, t->m_N1, tol ) )
    {
        return true;
    }
    if ( MatchMergeEdge( m_N0, m_N2, t->m_N1, t->m_N2, tol ) )
    {
        return true;
    }
    if ( MatchMergeEdge( m_N0, m_N2, t->m_N0, t->m_N2, tol ) )
    {
        return true;
    }

    return false;
}

bool TTri::MatchMergeEdge( TNode* n0, TNode* n1, TNode* nA, TNode* nB, double tol )
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

bool TTri::ShareEdge( TTri* t )
{
    double tol = 1.0e-6;

    int nmatch = 0;

    if ( dist_squared( m_N0->m_Pnt, t->m_N0->m_Pnt ) < tol )
    {
        nmatch++;
    }
    else
    {
        if ( dist_squared( m_N0->m_Pnt, t->m_N1->m_Pnt ) < tol )
        {
            nmatch++;
        }
        else
        {
            if ( dist_squared( m_N0->m_Pnt, t->m_N2->m_Pnt ) < tol )
            {
                nmatch++;
            }
        }
    }

    if ( dist_squared( m_N1->m_Pnt, t->m_N0->m_Pnt ) < tol )
    {
        nmatch++;
    }
    else
    {
        if ( dist_squared( m_N1->m_Pnt, t->m_N1->m_Pnt ) < tol )
        {
            nmatch++;
        }
        else
        {
            if ( dist_squared( m_N1->m_Pnt, t->m_N2->m_Pnt ) < tol )
            {
                nmatch++;
            }
        }
    }

    if ( dist_squared( m_N2->m_Pnt, t->m_N0->m_Pnt ) < tol )
    {
        nmatch++;
    }
    else
    {
        if ( dist_squared( m_N2->m_Pnt, t->m_N1->m_Pnt ) < tol )
        {
            nmatch++;
        }
        else
        {
            if ( dist_squared( m_N2->m_Pnt, t->m_N2->m_Pnt ) < tol )
            {
                nmatch++;
            }
        }
    }

    if ( nmatch >= 2 )
    {
        return true;
    }

    return false;
}

bool TTri::MatchEdge( TNode* n0, TNode* n1, TNode* nA, TNode* nB, double tol )
{
    if ( dist_squared( n0->m_Pnt, nA->m_Pnt ) < tol && dist_squared( n1->m_Pnt, nB->m_Pnt ) < tol )
    {
        return true;
    }
    if ( dist_squared( n0->m_Pnt, nB->m_Pnt ) < tol && dist_squared( n1->m_Pnt, nA->m_Pnt ) < tol )
    {
        return true;
    }

    return false;
}

bool TTri::CheckEdge( TNode* n0, TNode* n1 )
{
    // Set up vector so we can loop through valid edges.
    vector < TEdge* > edges;
    if ( m_E0 ) edges.push_back( m_E0 );
    if ( m_E1 ) edges.push_back( m_E1 );
    if ( m_E2 ) edges.push_back( m_E2 );

    for ( int i = 0; i < edges.size(); i++ )
    {
        TEdge *e = edges[i];

        if ( e->m_N0 == n0 && e->m_N1 == n1 )
            return true;
        if ( e->m_N0 == n1 && e->m_N1 == n0 )
            return true;
    }
    return false;
}

void TTri::RemoveDuplicateEdges()
{
    vector < int > discard;
    for ( int ie = 0; ie < m_EVec.size(); ie++ )
    {
        for ( int je = ie + 1; je < m_EVec.size(); je++ )
        {
            if ( m_EVec[ ie ]->DuplicateEdge( m_EVec[ je ] ) )
            {
                discard.push_back( je );
                break;
            }
        }
    }

    vector< TEdge* > keep;

    for ( int ie = 0; ie < m_EVec.size(); ie++ )
    {
        if ( vector_contains_val( discard, ie ) )
        {
            delete m_EVec[ ie ];
        }
        else
        {
            keep.push_back( m_EVec[ ie ] );
        }
    }
    m_EVec = keep;
}

// was 1e-5
#define ON_EDGE_TOL 1e-5

//==== Split A Triangle Along Edges in ISectEdges Vec =====//
bool TTri::SplitTri( bool dumpCase )
{
    bool erflag = true;
    int i, j;
    double onEdgeTol = ON_EDGE_TOL; // was 1e-5
    double uvMinTol  = 1e-3; // was 1e-3
    double uvMaxTol  = 1.0 - uvMinTol;

    //==== No Need To Split ====//
    if ( m_ISectEdgeVec.size() == 0 )
    {
        return erflag;
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

    vector< TEdge* > keepVec;
    for ( i = 0 ; i < ( int )m_ISectEdgeVec.size(); i++ )
    {
        bool keep = true;
        if ( !m_ISectEdgeVec[i]->m_N0->GetXYZPnt().isfinite() )
        {
            keep = false;
        }

        if ( !m_ISectEdgeVec[i]->m_N1->GetXYZPnt().isfinite() )
        {
            keep = false;
        }

        if ( keep )
        {
            keepVec.push_back( m_ISectEdgeVec[i] );
        }
        else
        {
            delete m_ISectEdgeVec[i]->m_N0;
            delete m_ISectEdgeVec[i]->m_N1;
            delete m_ISectEdgeVec[i];
        }
    }
    m_ISectEdgeVec = keepVec;

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

    if ( CleanupEdgeVec() )
    {
        printf( "Unexpected m_EVec cleared in TTri::SplitTri().\n" );
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
                double t;
                onEdgeFlag = !!OnEdge( uwVec[i], m_EVec[j], onEdgeTol, t );
            }
            else
            {
                double t;
                onEdgeFlag = !!OnEdge( pVec[i], m_EVec[j], onEdgeTol, t );
            }

            if ( onEdgeFlag )
            {
                //==== SplitEdge ====//
                TNode* sn = new TNode();        // New node
                sn->m_IsectFlag = 1;
                m_NVec.push_back( sn );
                matchNodeIndex[i] = m_NVec.size() - 1;
                sn->SetXYZPnt( pVec[i] );

#ifdef DEBUG_TMESH
                if ( !PtInTri( m_NVec[0]->m_Pnt, m_NVec[1]->m_Pnt, m_NVec[2]->m_Pnt, sn->m_Pnt ) )
                {
                    printf( "Outlier point added %s : %d.\n", __FILE__, __LINE__ );
                }
#endif

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

#ifdef DEBUG_TMESH
            if ( !PtInTri( m_NVec[0]->m_Pnt, m_NVec[1]->m_Pnt, m_NVec[2]->m_Pnt, sn->m_Pnt ) )
            {
                printf( "Outlier point added %s : %d.\n", __FILE__, __LINE__ );
            }
#endif

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

#ifdef DEBUG_TMESH
                            if ( !PtInTri( m_NVec[0]->m_Pnt, m_NVec[1]->m_Pnt, m_NVec[2]->m_Pnt, sn->m_Pnt ) )
                            {
                                printf( "Outlier point added %s : %d.\n", __FILE__, __LINE__ );
                            }
#endif

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

    RemoveDuplicateEdges();

    vector < vec3d > ptvec( m_NVec.size() );

    //==== Determine Which Axis to Flatten ====//
    Matrix4d rot_mat;
    int flattenAxis = 0;
    if ( uwflag )
    {
        flattenAxis = 2;

        for ( unsigned int n=0; n < m_NVec.size(); n++ )
        {
            ptvec[n] = m_NVec[n]->m_Pnt;
        }
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
            ptvec[n] = rot_mat.xform(m_NVec[n]->m_Pnt);
        }
    }

    if ( m_EVec.size() <= 3 )
    {
        return erflag;
    }


    //==== Use Triangle to Split Tri ====//
    bool success = TriangulateSplit( flattenAxis, ptvec, dumpCase );

    erflag = !success;

    CleanupEdgeVec();


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
    return erflag;
}

void TTri::FlipTri()
{
    swap( m_N0, m_N1 );

    m_Norm = -m_Norm;
}

void TTri::OrientTri( vector < int > & tri )
{
    vec3d n0 = m_NVec[ tri[0] ]->GetXYZPnt();
    vec3d n1 = m_NVec[ tri[1] ]->GetXYZPnt();
    vec3d n2 = m_NVec[ tri[2] ]->GetXYZPnt();

    vec3d d01 = n0 - n1;
    vec3d d21 = n2 - n1;

    vec3d cx = cross( d21, d01 );

    if ( dot( cx, m_Norm ) < 0.0 )
    {
        int tmp = tri[1];
        tri[1] = tri[2];
        tri[2] = tmp;
    }
}

void TTri::OrientConnList( vector < vector < int > > & cl )
{
    for ( int i = 0; i < cl.size(); i++ )
    {
        if ( cl[i].size() > 0 )
        {
            OrientTri( cl[i] );
        }
    }
}

bool TTri::CompConnList( const vector < vector < int > > & cla, const vector < vector < int > > & clb )
{
    if ( cla.size() != clb.size() )
    {
        return false;
    }

    for ( int i = 0; i < cla.size(); i++ )
    {
        if ( cla[i].size() != clb[i].size() )
        {
            return false;
        }

        for ( int j = 0; j < cla[i].size(); j++ )
        {
            if ( cla[i][j] != clb[i][j] )
            {
                return false;
            }
        }
    }
    return true;
}

void TTri::SortTri( vector < int > & tri )
{
    int imin = vector_find_minimum( tri );
    std::rotate( tri.begin(), tri.begin() + imin, tri.end() );
}

bool clcmp( const vector < int > & a, const vector < int > & b )
{
    if ( a.size() == b.size() )
    {
        for ( int i = 0; i < a.size(); i++ )
        {
            if ( a[i] != b[i] )
            {
                return a[i] < b[i];
            }
        }
    }
    else
    {
        return a.size() < b.size();
    }
    return a < b;
}

void TTri::SortConnList( vector < vector < int > > & cl )
{
    int ntri = cl.size();

    for ( int itri = 0; itri < ntri; itri++ )
    {
        SortTri( cl[itri] );
    }

    std::sort( cl.begin(), cl.end(), clcmp );

}

bool TTri::TriangulateSplit( int flattenAxis, const vector < vec3d > &ptvec, bool dumpCase )
{
    vector < vector < int > > othercl;
    vector < vector < int > > cl_DBA;
    // TriangulateSplit_DBA( flattenAxis, ptvec, false, cl_DBA, othercl );
    bool success = TriangulateSplit_DBA( flattenAxis, ptvec, dumpCase, cl_DBA, othercl );
    if ( !success )
    {
        printf( "Trying Triangle instead.\n" );
        cl_DBA.clear();
        othercl.clear();
        success = TriangulateSplit_TRI( flattenAxis, ptvec, dumpCase, cl_DBA, othercl );

        if ( success )
        {
            printf( "Triangle succeeded\n" );
        }
    }

    bool match = true;
#ifdef DEBUG_TMESH
#ifdef COMPARE_TRIANGLE
    OrientConnList( cl_DBA );
    SortConnList( cl_DBA );

    vector < vector < int > > cl_TRI;
    TriangulateSplit_TRI( flattenAxis, ptvec, false, cl_TRI );
    OrientConnList( cl_TRI );
    SortConnList( cl_TRI );

    match = CompConnList( cl_TRI, cl_DBA );

    if ( !match )
    {
        vector < vector < int > > cl_DBA2;
        TriangulateSplit_DBA( flattenAxis, ptvec, true, cl_DBA2, cl_TRI );

        bool match2 = CompConnList( cl_DBA, cl_DBA2 );
        if ( !match2 )
        {
            printf( "Subsequent calls to DBA don't match!\n" );
        }
    }
    else
    {
        // printf( "Matching result!\n" );
    }
#endif
#endif

    int ntri = cl_DBA.size();
    for ( int i = 0; i < ntri; i++ )
    {
        if ( cl_DBA[i].size() == 3 )
        {
            TTri* t = new TTri( m_TMesh );
            t->m_N0 = m_NVec[ cl_DBA[i][0] ];
            t->m_N1 = m_NVec[ cl_DBA[i][1] ];
            t->m_N2 = m_NVec[ cl_DBA[i][2] ];
            t->m_Tags = m_Tags; // Set split tri to have same tags as original triangle
            t->m_Norm = m_Norm;
            t->m_iQuad = m_iQuad;
            t->m_jref = m_jref;
            t->m_kref = m_kref;
            t->m_ID = m_ID;
            m_SplitVec.push_back( t );
        }
    }

    return success;
}

bool TTri::TriangulateSplit_TRI( int flattenAxis, const vector < vec3d > &ptvec, bool dumpCase,
                                 vector < vector < int > > & connlist, const vector < vector < int > > & otherconnlist  )
{
    int i, j;

    int npt = ptvec.size();

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
    out.pointlist   = nullptr;

    in.segmentlist  = ( int * ) malloc( m_EVec.size() * 2 * sizeof( int ) );
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
    in.numberofpoints = m_NVec.size();


    //==== Find Bounds of NVec ====//
    BndBox box;
    for ( j = 0 ; j < npt ; j++ )
    {
        box.Update( ptvec[j] );
    }

    vec3d center = box.GetCenter();

    double min_s = 0.0001;
    double sx = max( box.GetMax( 0 ) - box.GetMin( 0 ), min_s );
    double sy = max( box.GetMax( 1 ) - box.GetMin( 1 ), min_s );
    double sz = max( box.GetMax( 2 ) - box.GetMin( 2 ), min_s );

    int cnt = 0;
    for ( j = 0 ; j < npt ; j++ )
    {
        vec3d pnt = ptvec[j] - center;
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

            //==== Constrained Delaunay Triangulation ====//
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

        connlist.reserve( out.numberoftriangles );

        //==== Load Triangles if No New Point Created ====//
        cnt = 0;
        for ( i = 0; i < out.numberoftriangles; i++ )
        {
            if ( out.trianglelist[cnt] < npt &&
                out.trianglelist[cnt + 1] < npt &&
                out.trianglelist[cnt + 2] < npt )
            {
                connlist.push_back( { out.trianglelist[ cnt ],
                                      out.trianglelist[ cnt + 1 ],
                                      out.trianglelist[ cnt + 2 ] } );
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

    if ( tristatus == TRI_OK )
    {
        return true;
    }
    return false;
}

int dba_errlog( void* stream, const char* fmt, ...)
{
    va_list arg;
    va_start(arg,fmt);
    int ret = vfprintf((FILE*)stream, fmt, arg);
    va_end(arg);
    //fflush((FILE*)stream);
    return ret;
}

bool TTri::TriangulateSplit_DBA( int flattenAxis, const vector < vec3d > &ptvec, bool dumpCase,
                                 vector < vector < int > > & connlist, const vector < vector < int > > & otherconnlist  )
{
    static int idump = 0;

    bool success = true;

    int npt = ptvec.size();

    dba_point* cloud = new dba_point[npt];


    //==== Find Bounds of NVec ====//
    BndBox box;
    for ( int i = 0 ; i < npt ; i++ )
    {
        box.Update( ptvec[i] );
        m_NVec[i]->m_ID = i;
    }

    vec3d center = box.GetCenter();

    double min_s = 0.0001;
    double sx = max( box.GetMax( 0 ) - box.GetMin( 0 ), min_s );
    double sy = max( box.GetMax( 1 ) - box.GetMin( 1 ), min_s );
    double sz = max( box.GetMax( 2 ) - box.GetMin( 2 ), min_s );

    for ( int i = 0 ; i < npt ; i++ )
    {
        vec3d pnt = ptvec[i] - center;
        pnt.scale_x( 1.0 / sx );
        pnt.scale_y( 1.0 / sy );
        pnt.scale_z( 1.0 / sz );

        if ( flattenAxis == 0 )
        {
            cloud[i].x = pnt.y();
            cloud[i].y = pnt.z();
        }
        else if ( flattenAxis == 1 )
        {
            cloud[i].x = pnt.x();
            cloud[i].y = pnt.z();
        }
        else if ( flattenAxis == 2 )
        {
            cloud[i].x = pnt.x();
            cloud[i].y = pnt.y();
        }
    }

    int nedg = m_EVec.size();

    dba_edge* bounds = new dba_edge[nedg];

    for ( int i = 0 ; i < ( int )nedg ; i++ )
    {
        bounds[i].a = m_EVec[i]->m_N0->m_ID;
        bounds[i].b = m_EVec[i]->m_N1->m_ID;
    }

#ifdef DEBUG_TMESH
    FILE *fpdump = nullptr;
    if ( dumpCase )
    {
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
        // fclose( fpdump );
    }
#endif

    IDelaBella2 < double > * idb = IDelaBella2 < double > ::Create();
#ifdef DEBUG_TMESH
    idb->SetErrLog( dba_errlog, stdout );
#endif

    int verts = idb->Triangulate( npt, &cloud->x, &cloud->y, sizeof( dba_point ) );

    if ( verts > 0 )
    {
        idb->ConstrainEdges( nedg, &bounds->a, &bounds->b, sizeof( dba_edge ) );

        int tris = idb->FloodFill( false, 0, 1 );

        const IDelaBella2<double>::Simplex* dela = idb->GetFirstDelaunaySimplex();

        connlist.clear();
        connlist.resize( tris );
        for ( int i = 0; i < tris; i++ )
        {
            // Note winding order!
            connlist[i].push_back( dela->v[ 0 ]->i );
            connlist[i].push_back( dela->v[ 1 ]->i );
            connlist[i].push_back( dela->v[ 2 ]->i );

            dela = dela->next;
        }
    }
    else
    {
        printf( "DLB Error! %d\n", verts );
        success = false;
    }

    for ( int i = 0 ; i < npt; i++ )
    {
        m_NVec[i]->m_ID = -1;
    }


#ifdef DEBUG_TMESH
    if ( dumpCase )
    {
        OrientConnList( connlist );
        SortConnList( connlist );

        fprintf( fpdump, "DLB\n" );
        fprintf( fpdump, "%d\n", connlist.size() );
        for ( int i = 0; i < connlist.size(); i++ )
        {
            fprintf( fpdump, "%d %d %d %d\n", i, connlist[i][0], connlist[i][1], connlist[i][2] );
        }

        fprintf( fpdump, "TRI\n" );
        fprintf( fpdump, "%d\n", otherconnlist.size() );
        for ( int i = 0; i < otherconnlist.size(); i++ )
        {
            fprintf( fpdump, "%d %d %d %d\n", i, otherconnlist[i][0], otherconnlist[i][1], otherconnlist[i][2] );
        }


        fclose( fpdump );
    }
#endif

    delete[] cloud;
    delete[] bounds;

    idb->Destroy();

    return success;
}

bool TTri::InTri( const vec3d & p )
{
    return PtInTri( m_N0->m_Pnt, m_N1->m_Pnt, m_N2->m_Pnt, p );
}

int TTri::OnEdge( const vec3d & p, TEdge* e, double onEdgeTol, double &t )
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

    vec3d pon;
    double d = pointSegDistSquared( p, e->m_N0->m_Pnt, e->m_N1->m_Pnt, t, pon );

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

double TTri::ComputeTriQual()
{
    return ComputeTriQual( m_N0, m_N1, m_N2 );
}

double TTri::ComputeTriQual( TNode* n0, TNode* n1, TNode* n2 )
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
}

void TTri::ComputeCosAngles( TNode* n0, TNode* n1, TNode* n2, double* ang0, double* ang1, double* ang2 )
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

void TTri::ComputeCosAngles( double* ang0, double* ang1, double* ang2 )
{
    ComputeCosAngles( m_N0, m_N1, m_N2, ang0, ang1, ang2 );
}


void TTri::SplitEdges( TNode* n01, TNode* n12, TNode* n20 )
{
    TTri* tri;
    if ( n01 && n12 && n20 )        // Three Split - Make Four Tris
    {
        tri = new TTri( m_TMesh );
        tri->m_N0 = m_N0;
        tri->m_N1 = n01;
        tri->m_N2 = n20;
        tri->m_Norm = m_Norm;
        tri->m_iQuad = m_iQuad;
        tri->m_ID = m_ID;
        tri->m_jref = m_jref;
        tri->m_kref = m_kref;
        m_SplitVec.push_back( tri );

        tri = new TTri( m_TMesh );
        tri->m_N0 = m_N1;
        tri->m_N1 = n12;
        tri->m_N2 = n01;
        tri->m_Norm = m_Norm;
        tri->m_iQuad = m_iQuad;
        tri->m_ID = m_ID;
        tri->m_jref = m_jref;
        tri->m_kref = m_kref;
        m_SplitVec.push_back( tri );

        tri = new TTri( m_TMesh );
        tri->m_N0 = m_N2;
        tri->m_N1 = n20;
        tri->m_N2 = n12;
        tri->m_Norm = m_Norm;
        tri->m_iQuad = m_iQuad;
        tri->m_ID = m_ID;
        tri->m_jref = m_jref;
        tri->m_kref = m_kref;
        m_SplitVec.push_back( tri );

        tri = new TTri( m_TMesh );
        tri->m_N0 = n01;
        tri->m_N1 = n12;
        tri->m_N2 = n20;
        tri->m_Norm = m_Norm;
        tri->m_iQuad = m_iQuad;
        tri->m_ID = m_ID;
        tri->m_jref = m_jref;
        tri->m_kref = m_kref;
        m_SplitVec.push_back( tri );

    }



}

int TTri::WakeEdge()
{
    double tol = 1e-12;
    if ( m_TMesh )
    {
        double wmin = m_TMesh->m_Wmin;
        if ( m_TMesh->m_SurfType == vsp::WING_SURF )
        {
            bool n0 = m_N0->m_UWPnt.y() <= ( wmin + tol );
            bool n1 = m_N1->m_UWPnt.y() <= ( wmin + tol );
            bool n2 = m_N2->m_UWPnt.y() <= ( wmin + tol );

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
    }
    return 0;
}

TEdge* TTri::QuadEdge()
{
    if ( m_iQuad != -1 )
    {
        if ( m_E0 )
        {
            TTri *t = m_E0->OtherTri( this );
            if ( t && t->m_iQuad == m_iQuad )
            {
                return m_E0;
            }
        }

        if ( m_E1 )
        {
            TTri *t = m_E1->OtherTri( this );
            if ( t && t->m_iQuad == m_iQuad )
            {
                return m_E1;
            }
        }

        if ( m_E2 )
        {
            TTri *t = m_E2->OtherTri( this );
            if ( t && t->m_iQuad == m_iQuad )
            {
                return m_E2;
            }
        }
    }
    return NULL;
}

TEdge* TTri::EdgeOpposite( TNode * n )
{
    if ( m_E0 && !m_E0->UsesNode( n ) )
    {
        return m_E0;
    }

    if ( m_E1 && !m_E1->UsesNode( n ) )
    {
        return m_E1;
    }

    if ( m_E2 && !m_E2->UsesNode( n ) )
    {
        return m_E2;
    }
    return NULL;
}

// XOR (^) of anything with itself will return zero.  So, by performing a bitwise XOR chain of all the pointers
// n0^n1^n2^a^b, a and b clobber their match among n0,n1,n2 leaving just the odd pointer out to be returned.
TNode* TTri::GetOtherNode( TNode* a, TNode* b )
{
    return (TNode *) ((uintptr_t) m_N0 ^ (uintptr_t) m_N1 ^ (uintptr_t) m_N2 ^ (uintptr_t) a ^ (uintptr_t) b);
}

bool TTri::CorrectOrder( TNode* n0, TNode* n1 )
{
    if ( n0 == m_N0 && n1 == m_N1 )
    {
        return true;
    }
    if ( n0 == m_N1 && n1 == m_N2 )
    {
        return true;
    }
    if ( n0 == m_N2 && n1 == m_N0 )
    {
        return true;
    }

    return false;
}

bool TTri::OnBoundary( TNode *n )
{
    TNode *n0;
    TNode *n1;

    if ( n == m_N0 )
    {
        n0 = m_N1;
        n1 = m_N2;
    }
    else if ( n == m_N1 )
    {
        n0 = m_N0;
        n1 = m_N2;
    }
    else
    {
        n0 = m_N0;
        n1 = m_N1;
    }

    if ( CheckEdge( n, n0 ) && CheckEdge( n, n1 ) )
    {
        return false;
    }
    return true;
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
    if ( m_Box.IsEmpty() )
    {
        return false;
    }

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

double TBndBox::MinDistance( TBndBox* iBox, double curr_min_dist, vec3d &p1, vec3d &p2 )
{
    if ( m_Box.IsEmpty() )
    {
        return curr_min_dist;
    }

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
            curr_min_dist = iBox->MinDistance( m_SBoxVec[i], curr_min_dist, p1, p2 );
        }
    }
    else if ( iBox->m_SBoxVec[0] )
    {
        for ( i = 0 ; i < 8 ; i++ )
        {
            curr_min_dist = iBox->m_SBoxVec[i]->MinDistance( this, curr_min_dist, p1, p2 );
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
                vec3d p1a, p2a;
                double d = tri_tri_min_dist( t0->m_N0->m_Pnt, t0->m_N1->m_Pnt, t0->m_N2->m_Pnt,
                                             t1->m_N0->m_Pnt, t1->m_N1->m_Pnt, t1->m_N2->m_Pnt, p1a, p2a);

                if ( d < curr_min_dist )
                {
                    curr_min_dist = d;
                    p1 = p1a;
                    p2 = p2a;
                }
            }
        }
    }

    return curr_min_dist;
}

bool TBndBox::CheckIntersect( const vec3d &org, const vec3d &norm )
{
    if ( m_Box.IsEmpty() )
    {
        return false;
    }

    if ( !m_Box.IntersectPlane( org, norm ) )
    {
        return false;
    }

    //==== Recursively Check Sub Boxes ====//
    if ( m_SBoxVec[0] )
    {
        for ( int i = 0 ; i < 8 ; i++ )
        {
            if ( m_SBoxVec[i]->CheckIntersect( org, norm ) )
            {
                return true;
            }
        }
    }
    else
    {
        //==== Check All Tris In Box ====//
        for ( size_t i = 0 ; i < m_TriVec.size() ; i++ )
        {
            const TTri* t0 = m_TriVec[i];

            if ( triangle_plane_intersect_test( org, norm, t0->m_N0->m_Pnt.v, t0->m_N1->m_Pnt.v, t0->m_N2->m_Pnt.v ) )
            {
                return true;
            }
        }
    }
    return false;
}

double TBndBox::MinDistance( const vec3d &org, const vec3d &norm, double curr_min_dist, vec3d &p1, vec3d &p2 )
{
    if ( m_Box.IsEmpty() )
    {
        return curr_min_dist;
    }

    double mind, maxd;
    m_Box.MinMaxDistPlane( org, norm, mind, maxd );

    // Nearest point of box (closest possible for all items in box) is farther than already observed distance.
    if ( mind > curr_min_dist )
    {
        return curr_min_dist;
    }

    //==== Recursively Check Sub Boxes ====//
    if ( m_SBoxVec[0] )
    {
        for ( int i = 0 ; i < 8 ; i++ )
        {
            curr_min_dist = m_SBoxVec[i]->MinDistance( org, norm, curr_min_dist, p1, p2 );
        }
    }
    //==== Check All Points Against Other Points ====//
    else
    {
        for ( size_t i = 0 ; i < ( int )m_TriVec.size() ; i++ )
        {
            TTri* t0 = m_TriVec[i];

            vec3d p1a, p2a;
            double d = triangle_plane_minimum_dist( org, norm, t0->m_N0->m_Pnt, t0->m_N1->m_Pnt, t0->m_N2->m_Pnt, p1a, p2a );

            if ( d < curr_min_dist )
            {
                curr_min_dist = d;
                p1 = p1a;
                p2 = p2a;
            }
        }
    }

    return curr_min_dist;
}

double TBndBox::MaxDistance( const vec3d &org, const vec3d &norm, double curr_max_dist, vec3d &p1, vec3d &p2 )
{
    if ( m_Box.IsEmpty() )
    {
        return curr_max_dist;
    }

    double mind, maxd;
    m_Box.MinMaxDistPlane( org, norm, mind, maxd );

    // Farthest point of box (farthest possible for all items in box) is closer than already observed distance.
    if ( maxd < curr_max_dist )
    {
        return curr_max_dist;
    }

    //==== Recursively Check Sub Boxes ====//
    if ( m_SBoxVec[0] )
    {
        for ( int i = 0 ; i < 8 ; i++ )
        {
            curr_max_dist = m_SBoxVec[i]->MaxDistance( org, norm, curr_max_dist, p1, p2 );
        }
    }
    //==== Check All Points Against Other Points ====//
    else
    {
        for ( size_t i = 0 ; i < ( int )m_TriVec.size() ; i++ )
        {
            TTri* t0 = m_TriVec[i];

            vec3d p1a, p2a;
            double d = triangle_plane_maximum_dist( org, norm, t0->m_N0->m_Pnt, t0->m_N1->m_Pnt, t0->m_N2->m_Pnt, p1a, p2a );

            if ( d > curr_max_dist )
            {
                curr_max_dist = d;
                p1 = p1a;
                p2 = p2a;
            }
        }
    }

    return curr_max_dist;
}

double TBndBox::MaxDistanceRay( const vec3d &org, const vec3d &norm, double curr_max_dist, vec3d &p1, vec3d &p2 )
{
    if ( m_Box.IsEmpty() )
    {
        return curr_max_dist;
    }

    double mind, maxd;
    m_Box.MinMaxDistRay( org, norm, mind, maxd );

    // Farthest point of box (farthest possible for all items in box) is closer than already observed distance.
    if ( maxd < curr_max_dist )
    {
        return curr_max_dist;
    }

    //==== Recursively Check Sub Boxes ====//
    if ( m_SBoxVec[0] )
    {
        for ( int i = 0 ; i < 8 ; i++ )
        {
            curr_max_dist = m_SBoxVec[i]->MaxDistanceRay( org, norm, curr_max_dist, p1, p2 );
        }
    }
    //==== Check All Points Against Other Points ====//
    else
    {
        for ( size_t i = 0 ; i < ( int )m_TriVec.size() ; i++ )
        {
            TTri* t0 = m_TriVec[i];

            for ( int inode = 0; inode < 3; inode++ )
            {
                TNode *n = t0->GetTriNode( inode );

                double d = dist_pnt_2_ray( org, norm, n->m_Pnt );
                if ( d > curr_max_dist )
                {
                    curr_max_dist = d;
                    p2 = n->m_Pnt;
                    p1 = proj_pnt_on_ray( org, norm, p2 );
                }
            }
        }
    }

    return curr_max_dist;
}


double TBndBox::MinAngle( const vec3d &org, const vec3d &norm, const vec3d& ptaxis, const vec3d& axis, double curr_min_angle, int ccw, vec3d &p1, vec3d &p2 )
{
    if ( m_Box.IsEmpty() )
    {
        return curr_min_angle;
    }

    double mina, maxa;
    m_Box.MinMaxAnglePlane( org, norm, ptaxis, axis, ccw, mina, maxa );

    // Nearest point of box (closest possible for all items in box) is farther than already observed distance.
    if ( mina > curr_min_angle )
    {
        return curr_min_angle;
    }

    //==== Recursively Check Sub Boxes ====//
    if ( m_SBoxVec[0] )
    {
        for ( int i = 0 ; i < 8 ; i++ )
        {
            curr_min_angle = m_SBoxVec[i]->MinAngle( org, norm, ptaxis, axis, curr_min_angle, ccw, p1, p2 );
        }
    }
    //==== Check All Points Against Other Points ====//
    else
    {
        for ( size_t i = 0 ; i < ( int )m_TriVec.size() ; i++ )
        {
            TTri* t0 = m_TriVec[i];

            vec3d p1a, p2a;

            for ( int j = 0; j < 3; j++ )
            {
                vec3d prot;
                double a = angle_pnt_2_plane( org, norm, ptaxis, axis, t0->GetTriNode( j )->m_Pnt, ccw, prot );

                if ( a < curr_min_angle )
                {
                    curr_min_angle = a;
                    p1 = t0->GetTriNode( j )->m_Pnt;
                    p2 = prot;
                }
            }
        }
    }

    return curr_min_angle;
}

// checkSharedEdges is a flag needed when meshes need to check for self-intersections.
// Setting this flag to true causes a check such that pairs of tris that share an edge are skipped.
// This avoids lots of pedantic intersections.
// However, this check is somewhat slow and is not needed in the normal case of intersecting independent
// meshes.
void TBndBox::Intersect( TBndBox* iBox, bool UWFlag, bool checkSharedEdges )
{
#ifdef DEBUG_TMESH
    static int fig = 0;
#endif

    if ( m_Box.IsEmpty() )
    {
        return;
    }

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
            iBox->Intersect( m_SBoxVec[i], UWFlag, checkSharedEdges );
        }
    }
    else if ( iBox->m_SBoxVec[0] )
    {
        for ( i = 0 ; i < 8 ; i++ )
        {
            iBox->m_SBoxVec[i]->Intersect( this, UWFlag, checkSharedEdges );
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

                if ( t0 == t1 )
                {
                    continue;
                }

                if ( checkSharedEdges && t0->ShareEdge( t1 ) )
                {
                    continue;
                }

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
                            // Figure out which tri has xyz info
                            TTri* tri;
                            int d_info = TNode::HAS_XYZ; // desired info number
                            if ( ( t0->m_N0->GetCoordInfo() & d_info ) == d_info &&
                                 ( t0->m_N1->GetCoordInfo() & d_info ) == d_info &&
                                 ( t0->m_N2->GetCoordInfo() & d_info ) == d_info )
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

#ifdef DEBUG_TMESH
                            if ( !tri->InTri( e0xyz ) || !tri->InTri( e1xyz ) )
                            {
                                printf( "%% Outlier point created %s : %d\n", __FILE__, __LINE__ );

                                printf( "t0 = [%.24e %.24e %.24e;\n", t0->m_N0->m_Pnt.x(), t0->m_N0->m_Pnt.y(), t0->m_N0->m_Pnt.z() );
                                printf( "      %.24e %.24e %.24e;\n", t0->m_N1->m_Pnt.x(), t0->m_N1->m_Pnt.y(), t0->m_N1->m_Pnt.z() );
                                printf( "      %.24e %.24e %.24e;\n", t0->m_N2->m_Pnt.x(), t0->m_N2->m_Pnt.y(), t0->m_N2->m_Pnt.z() );
                                printf( "      %.24e %.24e %.24e];\n", t0->m_N0->m_Pnt.x(), t0->m_N0->m_Pnt.y(), t0->m_N0->m_Pnt.z() );

                                printf( "t1 = [%.24e %.24e %.24e;\n", t1->m_N0->m_Pnt.x(), t1->m_N0->m_Pnt.y(), t1->m_N0->m_Pnt.z() );
                                printf( "      %.24e %.24e %.24e;\n", t1->m_N1->m_Pnt.x(), t1->m_N1->m_Pnt.y(), t1->m_N1->m_Pnt.z() );
                                printf( "      %.24e %.24e %.24e;\n", t1->m_N2->m_Pnt.x(), t1->m_N2->m_Pnt.y(), t1->m_N2->m_Pnt.z() );
                                printf( "      %.24e %.24e %.24e];\n", t1->m_N0->m_Pnt.x(), t1->m_N0->m_Pnt.y(), t1->m_N0->m_Pnt.z() );

                                printf( "e0 = [%.24e %.24e %.24e;\n", e0.x(), e0.y(), e0.z() );
                                printf( "      %.24e %.24e %.24e];\n", e1.x(), e1.y(), e1.z() );

                                printf( "figure( %d );\n", fig + 3 );
                                printf( "plot3( t0(:,1), t0(:,2), t0(:,3) )\n" );
                                printf( "hold on;\n" );
                                printf( "plot3( t1(:,1), t1(:,2), t1(:,3) )\n" );
                                printf( "plot3( e0(:,1), e0(:,2), e0(:,3), '-o' )\n" );
                                printf( "hold off;\n\n" );

                                printf( "figure( 1 );\n" );
                                printf( "plot3( t0(:,1), t0(:,2), t0(:,3) )\n" );
                                printf( "hold on;\n" );
                                printf( "plot3( t1(:,1), t1(:,2), t1(:,3) )\n" );
                                printf( "plot3( e0(:,1), e0(:,2), e0(:,3), '-o' )\n" );


                                printf( "t0 = [%.24e %.24e %.24e;\n", t0->m_N0->m_Pnt.x(), t0->m_N0->m_UWPnt.y(), t0->m_N0->m_UWPnt.z() );
                                printf( "      %.24e %.24e %.24e;\n", t0->m_N1->m_UWPnt.x(), t0->m_N1->m_UWPnt.y(), t0->m_N1->m_UWPnt.z() );
                                printf( "      %.24e %.24e %.24e;\n", t0->m_N2->m_UWPnt.x(), t0->m_N2->m_UWPnt.y(), t0->m_N2->m_UWPnt.z() );
                                printf( "      %.24e %.24e %.24e];\n", t0->m_N0->m_UWPnt.x(), t0->m_N0->m_UWPnt.y(), t0->m_N0->m_UWPnt.z() );

                                printf( "t1 = [%.24e %.24e %.24e;\n", t1->m_N0->m_UWPnt.x(), t1->m_N0->m_UWPnt.y(), t1->m_N0->m_UWPnt.z() );
                                printf( "      %.24e %.24e %.24e;\n", t1->m_N1->m_UWPnt.x(), t1->m_N1->m_UWPnt.y(), t1->m_N1->m_UWPnt.z() );
                                printf( "      %.24e %.24e %.24e;\n", t1->m_N2->m_UWPnt.x(), t1->m_N2->m_UWPnt.y(), t1->m_N2->m_UWPnt.z() );
                                printf( "      %.24e %.24e %.24e];\n", t1->m_N0->m_UWPnt.x(), t1->m_N0->m_UWPnt.y(), t1->m_N0->m_UWPnt.z() );

                                printf( "e0 = [%.24e %.24e %.24e;\n", e0xyz.x(), e0xyz.y(), e0xyz.z() );
                                printf( "      %.24e %.24e %.24e];\n", e1xyz.x(), e1xyz.y(), e1xyz.z() );

                                printf( "figure( %d );\n", fig + 103 );
                                printf( "plot( t0(:,1), t0(:,2) )\n" );
                                printf( "hold on;\n" );
                                printf( "plot( t1(:,1), t1(:,2) )\n" );
                                printf( "plot( e0(:,1), e0(:,2), '-o' )\n" );
                                printf( "hold off;\n\n" );

                                printf( "figure( 2 );\n" );
                                printf( "plot( t0(:,1), t0(:,2) )\n" );
                                printf( "hold on;\n" );
                                printf( "plot( t1(:,1), t1(:,2) )\n" );
                                printf( "plot( e0(:,1), e0(:,2), '-o' )\n" );

                                fig++;
                            }
#endif

                            // The ISectEdges added by this code end up causing problems down the line, most frequently
                            // observed as corrupted corner meshes for subsurface control surfaces.  Removing this code
                            // appears to help and does not have any adverse effects that have bene observed.  However,
                            // This is being commented out (rather than removed) to make returning to this decision
                            // later easy.
                            //
                            // This is the only call site for SplitAliasEdges, so total removal may be possible later.
                            // if ( tri->GetTMeshPtr() )
                            // {
                            //     tri->GetTMeshPtr()->SplitAliasEdges( tri, tri->m_ISectEdgeVec.back() );
                            // }

                        }
                    }
                    else
                    {
                        if ( dist( e0, e1 ) > tol )
                        {
                            if ( !t0->InTri( e0 ) || !t0->InTri( e1 ) || !t1->InTri( e0 ) || !t1->InTri( e1 ) )
                            {
                                // tri_tri_intersection_test_3d() above gives bonkers results in situations with
                                // two triangles that share a point along a colinear edge.  The resulting intersection
                                // segment fails this InTri() test, so this code seems to filter this failure out.
                                continue;
                            }

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

#ifdef DEBUG_TMESH
                            if ( !t0->InTri( e0 ) || !t0->InTri( e1 ) || !t1->InTri( e0 ) || !t1->InTri( e1 ) && false )
                            {
                                printf( "%% Outlier point created %s : %d\n", __FILE__, __LINE__ );

                                printf( "t0 = [%.24e %.24e %.24e;\n", t0->m_N0->m_Pnt.x(), t0->m_N0->m_Pnt.y(), t0->m_N0->m_Pnt.z() );
                                printf( "      %.24e %.24e %.24e;\n", t0->m_N1->m_Pnt.x(), t0->m_N1->m_Pnt.y(), t0->m_N1->m_Pnt.z() );
                                printf( "      %.24e %.24e %.24e;\n", t0->m_N2->m_Pnt.x(), t0->m_N2->m_Pnt.y(), t0->m_N2->m_Pnt.z() );
                                printf( "      %.24e %.24e %.24e];\n", t0->m_N0->m_Pnt.x(), t0->m_N0->m_Pnt.y(), t0->m_N0->m_Pnt.z() );

                                printf( "t1 = [%.24e %.24e %.24e;\n", t1->m_N0->m_Pnt.x(), t1->m_N0->m_Pnt.y(), t1->m_N0->m_Pnt.z() );
                                printf( "      %.24e %.24e %.24e;\n", t1->m_N1->m_Pnt.x(), t1->m_N1->m_Pnt.y(), t1->m_N1->m_Pnt.z() );
                                printf( "      %.24e %.24e %.24e;\n", t1->m_N2->m_Pnt.x(), t1->m_N2->m_Pnt.y(), t1->m_N2->m_Pnt.z() );
                                printf( "      %.24e %.24e %.24e];\n", t1->m_N0->m_Pnt.x(), t1->m_N0->m_Pnt.y(), t1->m_N0->m_Pnt.z() );

                                printf( "e0 = [%.24e %.24e %.24e;\n", e0.x(), e0.y(), e0.z() );
                                printf( "      %.24e %.24e %.24e];\n", e1.x(), e1.y(), e1.z() );

                                printf( "figure( %d );\n", fig + 2 );
                                printf( "plot3( t0(:,1), t0(:,2), t0(:,3) )\n" );
                                printf( "hold on;\n" );
                                printf( "plot3( t1(:,1), t1(:,2), t1(:,3) )\n" );
                                printf( "plot3( e0(:,1), e0(:,2), e0(:,3), '-o' )\n" );
                                printf( "hold off;\n\n" );

                                printf( "figure( 1 );\n" );
                                printf( "plot3( t0(:,1), t0(:,2), t0(:,3) )\n" );
                                printf( "hold on;\n" );
                                printf( "plot3( t1(:,1), t1(:,2), t1(:,3) )\n" );
                                printf( "plot3( e0(:,1), e0(:,2), e0(:,3), '-o' )\n" );

                                fig++;
                            }
#endif
                        }
                    }
                }
            }
        }
    }
}

void  TBndBox::RayCast( const vec3d & orig, const vec3d & dir, vector<double> & tParmVec, vector <TTri*> & triVec ) const
{
    if ( m_Box.IsEmpty() )
    {
        return;
    }

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
            m_SBoxVec[i]->RayCast( orig, dir, tParmVec, triVec );
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
                triVec.push_back( tri );
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
                    m_TVec[t]->m_InvalidFlag = true;
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
                    m_TVec[t]->m_InvalidFlag = true;
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
                    m_TVec[t]->m_InvalidFlag = true;
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

    t0n0 = t0n1 = t1n0 = t1n1 = nullptr;

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
    TTri *t0 = edge->m_Tri0;
    TTri *t1 = edge->m_Tri1;

    if ( !t0 || !t1 )
    {
        return;
    }

    TNode *n0, *n1, *n2, *n3;
    n0 = edge->m_N0;
    n2 = edge->m_N1;

    n1 = t0->GetOtherNode( n0, n2 );
    n3 = t1->GetOtherNode( n0, n2 );

    TEdge *e0, *e1, *e2, *e3;

    e0 = t0->EdgeOpposite( n0 );
    e1 = t0->EdgeOpposite( n2 );
    e2 = t1->EdgeOpposite( n0 );
    e3 = t1->EdgeOpposite( n2 );

    edge->m_N0 = n1;
    edge->m_N1 = n3;

    if ( t0->CorrectOrder( n0, n2 ) )
    {
        t0->m_N0 = n0;
        t0->m_N1 = n3;
        t0->m_N2 = n1;
    }
    else
    {
        t0->m_N0 = n0;
        t0->m_N1 = n1;
        t0->m_N2 = n3;
    }

    if ( t1->CorrectOrder( n0, n2 ) )
    {
        t1->m_N0 = n1;
        t1->m_N1 = n2;
        t1->m_N2 = n3;
    }
    else
    {
        t1->m_N0 = n1;
        t1->m_N1 = n3;
        t1->m_N2 = n2;
    }

    t0->m_E0 = edge;
    t0->m_E1 = e1;
    t0->m_E2 = e3;

    t1->m_E0 = edge;
    t1->m_E1 = e0;
    t1->m_E2 = e2;

    if ( e0 )
    {
        e0->ReplaceTri( t0, t1 );
    }
    if ( e3 )
    {
        e3->ReplaceTri( t1, t0 );
    }

    vector_remove_val( n0->m_TriVec, t1 );
    vector_remove_val( n2->m_TriVec, t0 );
    n1->m_TriVec.push_back( t1 );
    n3->m_TriVec.push_back( t0 );
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
    unordered_map< TNode*, list<TNode*> >::iterator mit; // map iterator

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
    unordered_map< TNode*, list< TNode* > >::iterator mit;
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

void TMesh::CheckQualitySwapEdges()
{
    // Make vector copy of list so edges can be removed from list without invalidating active list iterator.
    vector< TEdge* > eVec( m_EVec.begin(), m_EVec.end() );

    for ( int i = 0; i < eVec.size(); i++ )
    {
        TEdge *e = eVec[ i ];

        if ( e->m_Tri0 && e->m_Tri1 )
        {
            TTri *t0 = e->m_Tri0;
            TTri *t1 = e->m_Tri1;

            if ( t0->m_iQuad == t1->m_iQuad &&
                 t0->m_Tags == t1->m_Tags )
            {
                TNode* n0 = e->m_N0;
                TNode* n1 = e->m_N1;

                TNode* na = t0->GetOtherNode( n0, n1 );
                TNode* nb = t1->GetOtherNode( n0, n1 );

                double qa = t0->ComputeTriQual();
                double qb = t1->ComputeTriQual();
                double qc = TTri::ComputeTriQual( n0, nb, na );
                double qd = TTri::ComputeTriQual( n1, na, nb );

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
    int t;

    BuildEdges();

    //==== Check If All Tris Have 3 Edges ====//
    vector< TTri* > ivTriVec;
    for ( t = 0 ; t < ( int )m_TVec.size() ; t++ )
    {
        if ( !m_TVec[t]->m_E0 || !m_TVec[t]->m_E1 || !m_TVec[t]->m_E2 )
        {
            m_TVec[t]->m_InvalidFlag = true;
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
        vec3d pon;
        double d = pointSegDistSquared( offPnt, pairVec[i], pairVec[i + 1], t, pon );
        if ( d < closeDist )
        {
            closeDist = d;
            closePnt = pon;
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
        TTri* t0 = new TTri( nullptr );
        t0->m_N0   = new TNode();
        t0->m_N1   = new TNode();
        t0->m_N2   = new TNode();
        TTri* t1 = new TTri( nullptr );
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
        t0->SplitTri( false );
        t1->SplitTri( false );

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

    if ( orig_tri->m_PEArr[0] != nullptr )
    {
        double edgeTol = ON_EDGE_TOL;

        vec3d * nn;
        TEdge * orig_e = nullptr;
        double t;
        for ( int pei = 0 ; pei < 3 ; pei++ )
        {
            nn = nullptr;
            orig_e = orig_tri->m_PEArr[pei];

            for( int ei = 0; ei < 2; ei++ )
            {

                if ( orig_tri->OnEdge( es[ei], orig_e, edgeTol, t ) == 1 )
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
void TMesh::SubTag( int part_num, bool tag_subs, const vector < string > & sub_vec )
{
    // Subtag all triangles in a given TMesh
    // Split tris will be subtagged the same as their parent
    vector<SubSurface*> sub_surfs;
    if ( tag_subs )
    {
        sub_surfs = SubSurfaceMgr.GetSubSurfs( m_OriginGeomID, m_SurfNum );

        // If sub_vec is not empty, use only subsurfaces listed in sub_vec (i.e. whitelist).
        if ( !sub_vec.empty() )
        {
            vector<SubSurface*> keep;
            for ( int s = 0; s < sub_surfs.size(); s++ )
            {
                if ( vector_contains_val( sub_vec, sub_surfs[s]->GetID() ) )
                {
                    keep.push_back( sub_surfs[s] );
                }
            }
            sub_surfs = keep;
        }
    }
    int ss_num = ( int )sub_surfs.size();

    for ( int t = 0 ; t < ( int )m_TVec.size(); t ++ )
    {
        TTri* tri = m_TVec[t];
        tri->m_Tags.clear();
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

void TMesh::RefreshTagMap()
{
    for ( int t = 0 ; t < ( int )m_TVec.size(); t ++ )
    {
        TTri* tri = m_TVec[t];

        for ( int st = 0; st < ( int )tri->m_SplitVec.size() ; st++ )
        {
            if ( !tri->m_SplitVec[st]->m_IgnoreTriFlag )
            {
                SubSurfaceMgr.m_TagCombos.insert( tri->m_SplitVec[st]->m_Tags );
            }
        }

        if ( !tri->m_IgnoreTriFlag )
        {
            SubSurfaceMgr.m_TagCombos.insert( tri->m_Tags );
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
                            const vector< vector<vec3d> > & uw_pnts,
                            int n_ref,
                            int indx, int platenum, int surftype, int cfdsurftype,
                            bool thicksurf, bool flipnormal, bool skipnegflipnormal,
                            int &iQuad, bool flatpatch )
{
    TMeshVec.push_back( new TMesh() );
    int itmesh = TMeshVec.size() - 1;
    TMeshVec[itmesh]->LoadGeomAttributes( geom );
    TMeshVec[itmesh]->m_SurfCfdType = cfdsurftype;
    TMeshVec[itmesh]->m_ThickSurf = thicksurf;
    TMeshVec[itmesh]->m_FlatPatch = flatpatch;
    TMeshVec[itmesh]->m_SurfType = surftype;
    TMeshVec[itmesh]->m_SurfNum = indx;
    TMeshVec[itmesh]->m_CopyIndex = geom->GetSurfCopyIndx( indx );
    TMeshVec[itmesh]->m_PlateNum = platenum;
    TMeshVec[itmesh]->m_UWPnts = uw_pnts;
    TMeshVec[itmesh]->m_XYZPnts = pnts;
    TMeshVec[itmesh]->m_Wmin = uw_pnts[0][0].y();
    TMeshVec[itmesh]->m_Uscale = geom->GetUMax( indx );
    TMeshVec[itmesh]->m_Wscale = geom->GetWMax( indx );;

    if ( !skipnegflipnormal && cfdsurftype == vsp::CFD_NEGATIVE )
    {
        flipnormal = !flipnormal;
    }

    BuildTMeshTris( TMeshVec[itmesh], flipnormal, geom->GetWMax( indx ), platenum, n_ref, iQuad );

}

void CreateTMeshVecFromPts( const Geom * geom,
                            vector < TMesh* > & TMeshVec,
                            const vector< vector<vec3d> > & pnts,
                            const vector< vector<vec3d> > & uw_pnts,
                            int n_ref,
                            int indx, int platenum, int surftype, int cfdsurftype,
                            bool thicksurf, bool flipnormal, bool skipnegflipnormal,
                            int iustart, int iuend,
                            int &iQuad, bool flatpatch )
{
    vector< vector < vec3d > > pnts_subset;
    vector< vector < vec3d > > uw_pnts_subset;

    int n = iuend - iustart + 1;

    pnts_subset.reserve( n );
    uw_pnts_subset.reserve( n );

    for ( int iu = iustart; iu <= iuend; iu++ )
    {
        pnts_subset.push_back( pnts[iu] );
        uw_pnts_subset.push_back( uw_pnts[iu] );
    }

    CreateTMeshVecFromPts( geom,
                           TMeshVec,
                           pnts_subset, uw_pnts_subset,
                           n_ref,
                           indx, platenum, surftype, cfdsurftype,
                           thicksurf, flipnormal, skipnegflipnormal,
                           iQuad, flatpatch );
}

void CreateTMeshVecFromPtsCheckFlat( const Geom * geom,
                                     vector < TMesh* > & TMeshVec,
                                     const vector< vector<vec3d> > & pnts,
                                     const vector< vector<vec3d> > & uw_pnts,
                                     int n_ref,
                                     int indx, int platenum, int surftype, int cfdsurftype,
                                     bool thicksurf, bool flipnormal, bool skipnegflipnormal, int &iQuad )
{
    // Comparing on distance squared between two normal vectors.
    double tol = 1e-12;

    int nu = pnts.size();
    if ( nu > 1 )
    {
        int nv = pnts[0].size();
        if ( nv > 1 )
        {
            // Build normal vectors.
            vector < vector < vec3d > > nvec;
            nvec.resize( nu - 1 );
            for ( int i = 0; i < nu - 1; i++ )
            {
                nvec[i].resize( nv - 1 );
                for ( int j = 0; j < nv - 1; j++ )
                {
                    vec3d u = pnts[ i + 1 ][ j + 1 ] - pnts[ i ][ j ];
                    vec3d v = pnts[ i + 1 ][ j ] - pnts[ i ][ j + 1 ];
                    vec3d n = cross( u, v );
                    n.normalize();

                    nvec[i][j] = n;
                }
            }

            // Check for planar strips.
            vector < bool > chordwise_flat( nu - 1, true );
            for ( int i = 0; i < nu - 1; i++ )
            {
                for ( int j = 1; j < nv - 1; j++ )
                {
                    if ( dist_squared( nvec[ i ][ 0 ],  nvec[ i ][ j ] ) > tol )
                    {
                        chordwise_flat[ i ] = false;
                        break;
                    }
                }
            }

            // Build spanwise extent of planar strips.
            vector < bool > spanwise_flat_match( nu - 1, false );
            spanwise_flat_match[ 0 ] = 0;
            for ( int i = 1; i < nu - 1; i++ )
            {
                if ( chordwise_flat[ i ] && chordwise_flat[ i - 1 ] &&
                     dist_squared( nvec[ i ][ 0 ], nvec[ i - 1 ][ 0 ] ) < tol )
                {
                    spanwise_flat_match[ i ] = true;
                }
            }


            int iustart = 0;
            while ( iustart < nu - 1 )
            {
                bool flatpatch = chordwise_flat[ iustart ];

                int iuend = iustart + 1;
                while ( spanwise_flat_match[ iuend ] == flatpatch && iuend < nu - 1 )
                {
                    iuend++;
                }

                CreateTMeshVecFromPts( geom,
                                       TMeshVec,
                                       pnts,
                                       uw_pnts,
                                       n_ref,
                                       indx, platenum, surftype, cfdsurftype,
                                       thicksurf, flipnormal, skipnegflipnormal,
                                       iustart, iuend,
                                       iQuad, flatpatch );

                // Over-ride some variable copies to full range rather than patch subset.
                TMeshVec.back()->m_UWPnts = uw_pnts;
                TMeshVec.back()->m_XYZPnts = pnts;
                TMeshVec.back()->m_Wmin = uw_pnts[0][0].y();

                iustart = iuend;
            }
        }
        else
        {
            CreateTMeshVecFromPts( geom,
                                   TMeshVec,
                                   pnts,
                                   uw_pnts,
                                   n_ref,
                                   indx, platenum, surftype, cfdsurftype,
                                   thicksurf, flipnormal, skipnegflipnormal,
                                   iQuad, false );
        }
    }
    else
    {
        CreateTMeshVecFromPts( geom,
                               TMeshVec,
                               pnts,
                               uw_pnts,
                               n_ref,
                               indx, platenum, surftype, cfdsurftype,
                               thicksurf, flipnormal, skipnegflipnormal,
                               iQuad, false );
    }
}


void BuildTMeshTris( TMesh *tmesh, bool flipnormal, double wmax, int platenum, int n_ref, int &iQuad  )
{
    double tol=1.0e-12;

    vector< vector<vec3d> > *pnts = &(tmesh->m_XYZPnts);
    vector< vector<vec3d> > *uw_pnts = &(tmesh->m_UWPnts);

    vec3d norm;
    vec3d v0, v1, v2, v3;
    vec3d uw0, uw1, uw2, uw3;
    vec3d d21, d01, d03, d23, d20, d31;

    int ref_start = 1 << ( n_ref + 2 ); // Offset n_ref by 2 to prevent collisions
    int ref_end = ref_start * 2;

    int nj = (*pnts).size();
    int nk = (*pnts)[0].size();

    // Find first non-degenerate j section.
    int firstj = -1;
    for ( int j = 0; j < nj - 1; j++ )
    {
        double areaj = 0.0;

        for ( int k = 0; k < nk - 1; k++ )
        {
            v0 = (*pnts)[j][k];
            v1 = (*pnts)[j + 1][k];
            v2 = (*pnts)[j + 1][k + 1];
            v3 = (*pnts)[j][k + 1];

            areaj += area( v0, v1, v2 ) + area( v0, v2, v3 );
        }
        if ( areaj > tol )
        {
            firstj = j;
            break;
        }
    }

    // Find last non-degenerate j section.
    int lastj = -1;
    for ( int j = nj - 2; j >= 0; j-- )
    {
        double areaj = 0.0;

        for ( int k = 0; k < nk - 1; k++ )
        {
            v0 = (*pnts)[j][k];
            v1 = (*pnts)[j + 1][k];
            v2 = (*pnts)[j + 1][k + 1];
            v3 = (*pnts)[j][k + 1];

            areaj += area( v0, v1, v2 ) + area( v0, v2, v3 );
        }
        if ( areaj > tol )
        {
            lastj = j;
            break;
        }
    }

    // Use degenerate j sections to find u to set condition
    double umin = (*uw_pnts)[ firstj ][ 0 ].x();
    double umax = (*uw_pnts)[ lastj + 1 ][ nk - 1 ].x();
    double umid = 0.5 * ( umin + umax );

    int jref = ref_start;
    for ( int j = 0; j < nj - 1; j++ )
    {
        int kref = ref_start;
        for ( int k = 0; k < nk - 1; k++ )
        {
            v0 = (*pnts)[j][k];
            v1 = (*pnts)[j + 1][k];
            v2 = (*pnts)[j + 1][k + 1];
            v3 = (*pnts)[j][k + 1];

            uw0 = (*uw_pnts)[j][k];
            uw1 = (*uw_pnts)[j + 1][k];
            uw2 = (*uw_pnts)[j + 1][k + 1];
            uw3 = (*uw_pnts)[j][k + 1];

            d21 = v2 - v1;
            d01 = v0 - v1;
            d03 = v0 - v3;
            d23 = v2 - v3;

            double quadrant = ( uw0.y() + uw1.y() + uw2.y() + uw3.y() ) / wmax; // * 4 * 0.25 canceled.
            double uave = ( uw0.x() + uw1.x() + uw2.x() + uw3.x() ) / 4.0;

            // Set up evencorners based on quadrants 0 and 2.
            bool evencorners = ( ( quadrant > 0 && quadrant < 1 ) || ( quadrant > 2 && quadrant < 3 ) );

            // Flip evencorners for second half of u.
            if ( uave > umid )
            {
                evencorners = !evencorners;
            }

            // Flip evencorners for every other degenerate plate (handles cruciform).
            if ( platenum % 2 )
            {
                evencorners = !evencorners;
            }

            if ( evencorners )
            {
                d20 = v2 - v0;
                if ( d21.mag() > tol && d01.mag() > tol && d20.mag() > tol )
                {
                    norm = cross( d21, d01 );
                    norm.normalize();
                    if ( flipnormal )
                    {
                        tmesh->AddTri( v0, v2, v1, norm * -1, uw0, uw2, uw1, iQuad, jref, kref );
                    }
                    else
                    {
                        tmesh->AddTri( v0, v1, v2, norm, uw0, uw1, uw2, iQuad, jref, kref );
                    }
                }

                if ( d03.mag() > tol && d23.mag() > tol && d20.mag() > tol )
                {
                    norm = cross( d03, d23 );
                    norm.normalize();
                    if ( flipnormal )
                    {
                        tmesh->AddTri( v0, v3, v2, norm * -1, uw0, uw3, uw2, iQuad, jref, kref );
                    }
                    else
                    {
                        tmesh->AddTri( v0, v2, v3, norm, uw0, uw2, uw3, iQuad, jref, kref );
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
                        tmesh->AddTri( v0, v3, v1, norm * -1, uw0, uw3, uw1, iQuad, jref, kref );
                    }
                    else
                    {
                        tmesh->AddTri( v0, v1, v3, norm, uw0, uw1, uw3, iQuad, jref, kref );
                    }
                }

                if ( d21.mag() > tol && d23.mag() > tol && d31.mag() > tol )
                {
                    norm = cross( d23, d21 );
                    norm.normalize();
                    if ( flipnormal )
                    {
                        tmesh->AddTri( v1, v3, v2, norm * -1, uw1, uw3, uw2, iQuad, jref, kref );
                    }
                    else
                    {
                        tmesh->AddTri( v1, v2, v3, norm, uw1, uw2, uw3, iQuad, jref, kref );
                    }
                }
            }

            iQuad++;

            kref++;
            if ( kref >= ref_end )
            {
                kref = ref_start;
            }
        }

        jref++;
        if ( jref >= ref_end )
        {
            jref = ref_start;
        }
    }
}

vector<TMesh*> CopyTMeshVec( const vector<TMesh*> &tmv )
{
    vector < TMesh* > tmv_out( tmv.size() );

    for ( int i = 0; i < tmv.size(); i++ )
    {
        tmv_out[i] = new TMesh();
        tmv_out[i]->CopyFlatten( tmv[i] );
    }

    return tmv_out;
}

void DeleteTMeshVec(  vector<TMesh*> &tmv )
{
    for ( int i = 0; i < tmv.size(); i++ )
    {
        delete tmv[i];
    }
    tmv.clear();
}

TMesh* MergeTMeshVec( const vector<TMesh*> &tmv )
{
    TMesh *tm = new TMesh();

    for ( int i = 0; i < tmv.size(); i++ )
    {
        tm->MergeTMeshes( tmv[i] );
    }

    return tm;
}

void LoadBndBox( vector< TMesh* > &tmv )
{
    for ( int i = 0; i < tmv.size(); i++ )
    {
        tmv[i]->LoadBndBox();
    }
}

void UpdateBBox( BndBox &bbox, vector<TMesh*> &tmv, const Matrix4d &transMat )
{
    bbox.Reset();
    if ( tmv.size() > 0 )
    {
        for ( int i = 0 ; i < ( int )tmv.size() ; i++ )
        {
            tmv[i]->UpdateBBox( bbox, transMat );
        }
    }
    else
    {
        bbox.Update( vec3d( 0.0, 0.0, 0.0 ));
    }
}

void ApplyScale( double scalefac, vector<TMesh*> &tmv )
{
    unordered_set < TNode* > nodeSet;
    for ( int i = 0 ; i < ( int )tmv.size() ; i++ )
    {
        for ( int j = 0 ; j < ( int )tmv[i]->m_NVec.size() ; j++ )
        {
            TNode* n = tmv[i]->m_NVec[j];
            nodeSet.insert( n );
        }
        //==== Split Tris ====//
        for ( int j = 0 ; j < ( int )tmv[i]->m_TVec.size() ; j++ )
        {
            TTri* t = tmv[i]->m_TVec[j];
            for ( int k = 0 ; k < ( int )t->m_NVec.size() ; k++ )
            {
                TNode* n = t->m_NVec[k];
                nodeSet.insert( n );
            }
        }

        for ( int j = 0 ; j < (int)tmv[i]->m_XYZPnts.size() ; j++ )
        {
            for ( int k = 0 ; k < (int)tmv[i]->m_XYZPnts[j].size() ; k++ )
            {
                tmv[i]->m_XYZPnts[j][k] = tmv[i]->m_XYZPnts[j][k] * ( scalefac );
            }
        }
    }
    for ( const auto& n : nodeSet )
    {
        n->m_Pnt = n->m_Pnt * ( scalefac );
    }
}

void MergeRemoveOpenMeshes( vector<TMesh*> &tmv, MeshInfo* info, bool deleteopen )
{
    int i, j;

    //==== Check If All Closed ====//
    for ( i = 0 ; i < ( int )tmv.size() ; i++ )
    {
        tmv[i]->CheckIfClosed();
    }

    //==== Try to Merge Non Closed Meshes ====//
    // Marks mesh un-used after merge for deletion
    for ( i = 0 ; i < ( int )tmv.size() ; i++ )
    {
        for ( j = i + 1 ; j < ( int )tmv.size() ; j++ )
        {
            tmv[i]->MergeNonClosed( tmv[j] );
        }
    }
    // Keep track of merged meshes in info.
    for ( i = 0 ; i < ( int )tmv.size() ; i++ )
    {
        if ( tmv[i]->m_DeleteMeFlag )
        {
            info->m_NumOpenMeshesMerged++;
            info->m_MergedMeshes.push_back( tmv[i]->m_NameStr );
        }
    }

    // Mark any still open meshes for deletion.  Perhaps make this optional.
    if ( deleteopen )
    {
        for ( i = 0 ; i < ( int )tmv.size() ; i++ )
        {
            if ( tmv[i]->m_NonClosedTriVec.size() )
            {
                if ( !tmv[i]->m_DeleteMeFlag )
                {
                    info->m_NumOpenMeshedDeleted++;
                    info->m_DeletedMeshes.push_back( tmv[i]->m_NameStr );
                }

                tmv[i]->m_DeleteMeFlag = true;
            }
        }
    }

    DeleteMarkedMeshes( tmv );

    //==== Remove Any Degenerate Tris ====//
    for ( i = 0 ; i < ( int )tmv.size() ; i++ )
    {
        info->m_NumDegenerateTriDeleted += tmv[i]->RemoveDegenerate();
    }

}

void DeleteMarkedMeshes( vector<TMesh*> &tmv )
{
    //==== Remove meshes marked for deletion ====//
    vector< TMesh* > newTMeshVec;
    for ( int i = 0 ; i < ( int )tmv.size() ; i++ )
    {
        if ( !tmv[i]->m_DeleteMeFlag )
        {
            newTMeshVec.push_back( tmv[i] );
        }
        else
        {
            delete tmv[i];
        }
    }
    tmv = newTMeshVec;
}

void FlattenTMeshVec( vector<TMesh*> &tmv )
{
    vector<TMesh*> flatTMeshVec;
    flatTMeshVec.reserve( tmv.size() );
    for ( int i = 0 ; i < ( int )tmv.size() ; i++ )
    {
        TMesh *tm = new TMesh();
        tm->CopyFlatten( tmv[ i ] );
        if ( tm->m_TVec.size() > 0 )
        {
            flatTMeshVec.push_back( tm );
        }
        else
        {
            delete tm;
        }
        delete tmv[i];
    }
    tmv.clear();
    tmv = flatTMeshVec;
}

void TransformMeshVec( vector<TMesh*> & meshVec, const Matrix4d & TransMat )
{
    // Build Map of nodes
    unordered_set < TNode* > nodeSet;
    for ( int i = 0 ; i < ( int )meshVec.size() ; i++ )
    {
        for ( int j = 0 ; j < ( int )meshVec[i]->m_NVec.size() ; j++ )
        {
            TNode* n = meshVec[i]->m_NVec[j];
            nodeSet.insert( n );
        }
        //==== Split Tris ====//
        for ( int j = 0 ; j < ( int )meshVec[i]->m_TVec.size() ; j++ )
        {
            TTri* t = meshVec[i]->m_TVec[j];
            for ( int k = 0 ; k < ( int )t->m_NVec.size() ; k++ )
            {
                TNode* n = t->m_NVec[k];
                nodeSet.insert( n );
            }
        }
    }

    // Apply Transformation to Nodes
    for ( const auto& n : nodeSet )
    {
        n->m_Pnt = TransMat.xform( n->m_Pnt );
    }

    vec3d zeroV = vec3d( 0.0, 0.0, 0.0 );
    zeroV = TransMat.xform( zeroV );

    // Apply Transformation to each triangle's normal vector
    for ( int i = 0 ; i < ( int )meshVec.size() ; i ++ )
    {
        for ( int j = 0 ; j < ( int )meshVec[i]->m_TVec.size() ; j++ )
        {
            if ( meshVec[i]->m_TVec[j]->m_SplitVec.size() )
            {
                for ( int t = 0 ; t < ( int ) meshVec[i]->m_TVec[j]->m_SplitVec.size() ; t++ )
                {
                    TTri* tri = meshVec[i]->m_TVec[j]->m_SplitVec[t];
                    tri->m_Norm = TransMat.xform( tri->m_Norm ) - zeroV;
                }
            }
            else
            {
                TTri* tri = meshVec[i]->m_TVec[j];
                tri->m_Norm = TransMat.xform( tri->m_Norm ) - zeroV;
            }
        }

        // Apply Transformation to Mesh's area center
        meshVec[i]->m_AreaCenter = TransMat.xform( meshVec[i]->m_AreaCenter );
    }
}

vector< string > GetTMeshNames( vector<TMesh*> &tmv )
{
    vector< string > names;
    for ( int i = 0 ; i < ( int )tmv.size() ; i++ )
    {
        string plate;
        if ( tmv[ i ]->m_PlateNum == -1 )
        {
            plate = "_S";
        }
        else
        {
            if ( tmv[i]->m_SurfType == vsp::NORMAL_SURF )
            {
                if ( tmv[ i ]->m_PlateNum == 0 )
                {
                    plate = "_V";
                }
                else if ( tmv[ i ]->m_PlateNum == 1 )
                {
                    plate = "_H";
                }
            }
            else // WING_SURF with m_TMeshVec[ i ]->m_PlateNum == 0
            {
                plate = "_C";
            }

        }

        names.push_back( tmv[i]->m_NameStr + plate + "_Surf" + to_string( ( long long )tmv[i]->m_SurfNum ) );
    }

    return names;
}

vector< string > GetTMeshIDs( vector<TMesh*> &tmv )
{
    vector< string > ids;
    for ( int i = 0; i < (int)tmv.size(); i++ )
    {
        string plate;
        if ( tmv[ i ]->m_PlateNum == -1 )
        {
            // plate; // empty string.
        }
        else
        {
            if ( tmv[i]->m_SurfType == vsp::NORMAL_SURF )
            {
                if ( tmv[ i ]->m_PlateNum == 0 )
                {
                    plate = "_V";
                }
                else if ( tmv[ i ]->m_PlateNum == 1 )
                {
                    plate = "_H";
                }
            }
            else // WING_SURF with m_TMeshVec[ i ]->m_PlateNum == 0
            {
                plate = "_C";
            }

        }

        ids.push_back( tmv[i]->m_OriginGeomID + plate + "_Surf" + to_string((long long)tmv[i]->m_SurfNum ) );
    }

    return ids;
}

vector< int > GetTMeshThicks( vector<TMesh*> &tmv )
{
    vector < int > thick;

    for ( int i = 0; i < (int)tmv.size(); i++ )
    {
        thick.push_back( tmv[i]->m_ThickSurf );
    }

    return thick;
}

vector< int > GetTMeshTypes( vector<TMesh*> &tmv )
{
    vector< int > type;
    for ( int i = 0; i < (int)tmv.size(); i++ )
    {
        type.push_back( tmv[i]->m_SurfType );
    }

    return type;
}

vector< int > GetTMeshPlateNum( vector<TMesh*> &tmv )
{
    vector< int > plate;
    for ( int i = 0; i < (int)tmv.size(); i++ )
    {
        plate.push_back( tmv[i]->m_PlateNum );
    }

    return plate;
}

vector< int > GetTMeshCopyIndex( vector<TMesh*> &tmv )
{
    vector< int > copy_indx;
    for ( int i = 0; i < (int)tmv.size(); i++ )
    {
        copy_indx.push_back( tmv[i]->m_CopyIndex );
    }

    return copy_indx;
}

vector< double > GetTMeshWmins( vector<TMesh*> &tmv )
{
    vector < double > wmin( tmv.size(), 0.0 );
    for ( int i = 0; i < (int)tmv.size(); i++ )
    {
        wmin[i] = tmv[i]->m_Wmin;
    }

    return wmin;
}

vector< double > GetTMeshUscale( vector<TMesh*> &tmv )
{
    vector< double > uscale( tmv.size(), 0.0 );
    for ( int i = 0; i < (int)tmv.size(); i++ )
    {
        uscale[i] = tmv[i]->m_Uscale;
    }

    return uscale;
}

vector< double > GetTMeshWscale( vector<TMesh*> &tmv )
{
    vector< double > wscale( tmv.size(), 0.0 );
    for ( int i = 0; i < (int)tmv.size(); i++ )
    {
        wscale[i] = tmv[i]->m_Wscale;
    }

    return wscale;
}

void SubTagTris( bool tag_subs, vector<TMesh*> &tmv, const vector < string > & sub_vec )
{
    // Clear out the current Subtag Maps
    SubSurfaceMgr.ClearTagMaps();
    SubSurfaceMgr.m_CompNames = GetTMeshNames( tmv );
    SubSurfaceMgr.m_CompIDs = GetTMeshIDs( tmv );
    SubSurfaceMgr.m_CompTypes = GetTMeshTypes( tmv );
    SubSurfaceMgr.m_CompPlate = GetTMeshPlateNum( tmv );
    SubSurfaceMgr.m_CompCopyIndex = GetTMeshCopyIndex( tmv );
    SubSurfaceMgr.m_CompWmin = GetTMeshWmins( tmv );
    SubSurfaceMgr.m_CompUscale = GetTMeshUscale( tmv );
    SubSurfaceMgr.m_CompWscale = GetTMeshWscale( tmv );
    SubSurfaceMgr.m_CompThick = GetTMeshThicks( tmv );
    SubSurfaceMgr.SetSubSurfTags( tmv.size() );
    SubSurfaceMgr.BuildCompNameMap();
    SubSurfaceMgr.BuildCompIDMap();

    for ( int i = 0 ; i < ( int )tmv.size() ; i++ )
    {
        tmv[i]->SubTag( i + 1, tag_subs, sub_vec );
    }

    SubSurfaceMgr.BuildSingleTagMap();
}

void RefreshTagMaps( vector<TMesh*> &tmv )
{
    SubSurfaceMgr.PartialClearTagMaps();

    for ( int i = 0 ; i < ( int )tmv.size() ; i++ )
    {
        tmv[i]->RefreshTagMap();
    }

    SubSurfaceMgr.BuildSingleTagMap();
}


//===== Vectors of TMeshs with Bounding Boxes Already Set Up ====//
bool CheckIntersect( const vector<TMesh*> & tmesh_vec, const vector<TMesh*> & other_tmesh_vec )
{
    for ( int i = 0 ; i < (int)tmesh_vec.size() ; i++ )
    {
        for ( int j = 0 ; j < (int)other_tmesh_vec.size() ; j++ )
        {
            if ( tmesh_vec[i]->CheckIntersect( other_tmesh_vec[j] ) )
            {
                return true;
            }
        }
    }

    return false;
}

bool CheckIntersect( const vector<TMesh*> & tmesh_vec, const vec3d &org, const vec3d &norm )
{
    for ( int i = 0 ; i < (int)tmesh_vec.size() ; i++ )
    {
        if ( tmesh_vec[i]->CheckIntersect( org, norm ) )
        {
            return true;
        }
    }

    return false;
}

//===== Vectors of TMeshs with Bounding Boxes Already Set Up ====//
bool CheckIntersect( Geom* geom_ptr, const vector<TMesh*> & other_tmesh_vec )
{
    bool intsect_flag = false;

    vector< TMesh* > tmesh_vec = geom_ptr->CreateTMeshVec( false );
    for ( int i = 0 ; i < (int)tmesh_vec.size() ; i++ )
    {
        tmesh_vec[i]->LoadBndBox();
    }

    intsect_flag = CheckIntersect( tmesh_vec, other_tmesh_vec );

    for ( int i = 0 ; i < (int)tmesh_vec.size() ; i++ )
    {
        delete tmesh_vec[i];
    }

    return intsect_flag;
}

bool CheckSelfIntersect( const vector<TMesh*> & tmesh_vec )
{
    for ( int i = 0 ; i < ( int )tmesh_vec.size() ; i++ )
    {
        tmesh_vec[i]->LoadBndBox();
    }

    for ( int i = 0 ; i < (int)tmesh_vec.size() - 1 ; i++ )
    {
        for ( int j = i + 1 ; j < (int)tmesh_vec.size() ; j++ )
        {
            if ( tmesh_vec[i]->CheckIntersect( tmesh_vec[j] ) )
            {
                return true;
            }
        }
    }

    return false;
}

//==== Returns Large Neg Number If Error and 0.0 If Collision ====//
double FindMinDistance( const vector< TMesh* > & tmesh_vec, const vector< TMesh* > & other_tmesh_vec, bool & intersect_flag )
{
    intersect_flag = false;

    if ( CheckIntersect( tmesh_vec, other_tmesh_vec ) )
    {
        intersect_flag = true;
        return 0.0;
    }

    //==== Find Min Dist ====//
    double min_dist = 1.0e12;
    vec3d p1, p2;
    for ( int i = 0 ; i < (int)tmesh_vec.size() ; i++ )
    {
        for ( int j = 0 ; j < (int)other_tmesh_vec.size() ; j++ )
        {
            vec3d p1a, p2a;
            double d =  tmesh_vec[i]->MinDistance(  other_tmesh_vec[j], min_dist, p1a, p2a );
            if ( d < min_dist )
            {
                min_dist = d;
                p1 = p1a;
                p2 = p2a;
            }
        }
    }

    return min_dist;
}

//===== Find The Min Distance For Each Point And Returns Max =====//
double FindMaxMinDistance( const vector< TMesh* > & mesh_vec_1, const vector< TMesh* > & mesh_vec_2 )
{
    double max_dist = 0.0;

    if ( mesh_vec_1.size() != mesh_vec_2.size() )
        return max_dist;

    for ( int i = 0 ; i < (int)mesh_vec_1.size() ; i++ )
    {
        TMesh* tm1 = mesh_vec_1[i];
        TMesh* tm2 = mesh_vec_2[i];

        if ( tm1->m_NVec.size() == tm2->m_NVec.size() )
        {
            for ( int m = 0 ; m < tm1->m_NVec.size() ; m++ )
            {
                double d2 = dist_squared( tm1->m_NVec[m]->m_Pnt, tm2->m_NVec[m]->m_Pnt );
                max_dist = max( max_dist, d2 );
            }
        }
    }
    return sqrt( max_dist );
}

void PlaneInterferenceCheck( TMesh *primary_tm, const vec3d & org, const vec3d & norm, const string & resid, vector< TMesh* > & result_tmv )
{
    bool intersect_flag = false;
    bool interference_flag = false;

    double vol_primary = primary_tm->ComputeTheoVol();

    double min_dist = 1.0e12;
    double max_dist = -1.0e12;
    double con_dist = 1.0e12;
    double con_vol = -1; // Not the true volume.
    double vol = 0;

    vector < vec3d > pts( 2 );

    double vref = vol_primary;

    vector < vec3d > threepts;
    MakeThreePts( org, norm, threepts );

    max_dist = primary_tm->MaxDistance( org, norm, max_dist, pts[0], pts[1] );

    if ( primary_tm->CheckIntersect( org, norm ) )
    {
        intersect_flag = true;
        interference_flag = true;

        // Add origin to primary mesh bounding box.
        BndBox bb = primary_tm->m_TBox.m_Box;
        bb.Update( org );
        double len = bb.DiagDist() * 2.1;

        TMesh * slice = MakeSlice( org, norm, len );

        result_tmv.push_back( slice );
        result_tmv.push_back( primary_tm );

        MeshCutAbovePlane( result_tmv, threepts );
        FlattenTMeshVec( result_tmv ); // Not required for volume calculations, do it for visualization and later use.

        min_dist = 0.0;
        con_dist = 1.0;

        for ( int i = 0; i < result_tmv.size(); i++ )
        {
            vol += result_tmv[i]->ComputeTrimVol();
        }
        con_vol = vol / vref;
    }
    else
    {
        pts.resize( 4 );
        min_dist = primary_tm->MinDistance( org, norm, min_dist, pts[2], pts[3] );
        con_dist = min_dist;

        if ( !primary_tm->m_TVec.empty() )
        {
            TTri *tri = primary_tm->m_TVec[0];
            vec3d cen = tri->ComputeCenter();


            if ( orient3d( (double*) threepts[0].v, (double*) threepts[1].v, (double*) threepts[2].v, (double*) cen.v ) > 0 )
            {
                interference_flag = true;
                result_tmv.push_back( primary_tm );
                con_vol = 1;
                con_dist += 1.0;
            }
        }
    }

    double gcon = con_dist * con_vol;

    Results *res = ResultsMgr.FindResultsPtr( resid );
    if( res )
    {
        // Populate results.
        res->Add( new NameValData( "Interference", interference_flag, "Flag indicating the primary and secondary interfere." ) );
        res->Add( new NameValData( "Intersection", intersect_flag, "Flag indicating the primary and secondary intersect." ) );
        res->Add( new NameValData( "Min_Dist", min_dist, "Minimum distance between primary and secondary." ) );
        res->Add( new NameValData( "Max_Dist", max_dist, "Maximum distance between primary and secondary." ) );
        res->Add( new NameValData( "Pts", pts, "Min/max distance line end points." ) );
        res->Add( new NameValData( "InterferenceVol", vol, "Volume of interference." ) );
        res->Add( new NameValData( "Con_Val", gcon, "Constraint value" ) );
        res->Add( new NameValData( "Result", gcon, "Interference result" ) );
    }
}

void CCEInterferenceCheck(  TMesh *primary_tm, TMesh *secondary_tm, const string & resid, vector< TMesh* > & result_tmv )
{
    bool intersect_flag = false;
    bool interference_flag = false;

    double vol_primary = primary_tm->ComputeTheoVol();

    double min_dist = 1.0e12;
    double con_dist = 1.0e12;
    double con_vol = -1; // Not the true volume.
    double vol = 0;

    bool primary_below_secondary = false;
    vector < vec3d > pts;

    if ( primary_tm->CheckIntersect( secondary_tm ) )
    {
        intersect_flag = true;
        interference_flag = true;

        result_tmv.push_back( secondary_tm );
        result_tmv.push_back( primary_tm );

        MeshCCEIntersect( result_tmv );
        FlattenTMeshVec( result_tmv ); // Not required for volume calculations, do it for visualization and later use.

        min_dist = 0.0;
        con_dist = 1.0;

        for ( int i = 0; i < result_tmv.size(); i++ )
        {
            vol += result_tmv[i]->ComputeTrimVol();
        }
        con_vol = vol / vol_primary;
    }
    else
    {
        pts.resize( 2 );
        min_dist = primary_tm->MinDistance( secondary_tm, min_dist, pts[0], pts[1] );
        con_dist = min_dist;

        if ( !primary_tm->m_TVec.empty() && !secondary_tm->m_TVec.empty() )
        {
            TTri *trip = primary_tm->m_TVec[0];
            vec3d upish( 0.000001, 0.000001, 1.0 );

            if ( DeterIntExtTri( trip, secondary_tm, upish ) ) // a inside b
            {
                primary_below_secondary = true;
                interference_flag = true;
                result_tmv.push_back( primary_tm );
                con_vol = 1;
                con_dist += 1.0;
                delete secondary_tm;
            }
            else
            {
                delete primary_tm;
                delete secondary_tm;
            }
        }
        else
        {
            delete primary_tm;
            delete secondary_tm;
        }
    }

    double gcon = con_dist * con_vol;

    Results *res = ResultsMgr.FindResultsPtr( resid );
    if( res )
    {
        // Populate results.
        res->Add( new NameValData( "Interference", interference_flag, "Flag indicating the primary and secondary interfere." ) );
        res->Add( new NameValData( "Intersection", intersect_flag, "Flag indicating the primary and secondary intersect." ) );
        res->Add( new NameValData( "Primary_Below_Secondary", primary_below_secondary, "Flag indicating the primary is contained below the secondary." ) );
        res->Add( new NameValData( "Min_Dist", min_dist, "Minimum distance between primary and secondary." ) );
        res->Add( new NameValData( "Pts", pts, "Min/max distance line end points." ) );
        res->Add( new NameValData( "InterferenceVol", vol, "Volume of interference." ) );
        res->Add( new NameValData( "Vol_Primary", vol_primary, "Volume of primary." ) );
        res->Add( new NameValData( "Con_Val", gcon, "Constraint value" ) );
        res->Add( new NameValData( "Result", gcon, "Interference result" ) );
    }

}

void SweptVolumeInterferenceCheck(  TMesh *primary_tm, TMesh *secondary_tm, const vec3d & disp, const string & resid, vector< TMesh* > & result_tmv )
{
    TMesh * swept_secondary_tm = CreateTMeshPGMeshSweptVolumeTranslate( secondary_tm, disp );
    delete secondary_tm;
    swept_secondary_tm->LoadBndBox();

    ExteriorInterferenceCheck( primary_tm, swept_secondary_tm, resid, result_tmv );
}

void ExteriorInterferenceCheck( TMesh *primary_tm, TMesh *secondary_tm, const string & resid, vector< TMesh* > & result_tmv )
{
    bool intersect_flag = false;
    bool interference_flag = false;

    double vol_primary = primary_tm->ComputeTheoVol();
    double vol_secondary = secondary_tm->ComputeTheoVol();

    double min_dist = 1.0e12;
    double con_dist = 1.0e12;
    double con_vol = -1; // Not the true volume.
    double vol = 0;

    bool primary_in_secondary = false;
    bool secondary_in_primary = true;
    vector < vec3d > pts;


    double vref = min( vol_primary, vol_secondary );

    if ( primary_tm->CheckIntersect( secondary_tm ) )
    {
        intersect_flag = true;
        interference_flag = true;

        result_tmv.push_back( primary_tm );
        result_tmv.push_back( secondary_tm );
        MeshIntersect( result_tmv );
        FlattenTMeshVec( result_tmv ); // Not required for volume calculations, do it for visualization and later use.

        min_dist = 0.0;
        con_dist = 1.0;

        for ( int i = 0; i < result_tmv.size(); i++ )
        {
            vol += result_tmv[i]->ComputeTrimVol();
        }
        con_vol = vol / vref;
    }
    else
    {
        pts.resize( 2 );
        min_dist = primary_tm->MinDistance( secondary_tm, min_dist, pts[0], pts[1] );
        con_dist = min_dist;

        if ( !primary_tm->m_TVec.empty() && !secondary_tm->m_TVec.empty() )
        {
            TTri *trip = primary_tm->m_TVec[0];
            TTri *tris = secondary_tm->m_TVec[0];


            if ( DeterIntExtTri( trip, secondary_tm ) ) // a inside b
            {
                primary_in_secondary = true;
                interference_flag = true;
                result_tmv.push_back( primary_tm );
                con_vol = 1;
                con_dist += 1.0;
                delete secondary_tm;
            }
            else if ( DeterIntExtTri( tris, primary_tm ) ) // b inside a
            {
                secondary_in_primary = true;
                interference_flag = true;
                result_tmv.push_back( secondary_tm );
                con_vol = 1;
                con_dist += 1.0;
                delete primary_tm;
            }
            else
            {
                delete primary_tm;
                delete secondary_tm;
            }
        }
        else
        {
            delete primary_tm;
            delete secondary_tm;
        }
    }

    double gcon = con_dist * con_vol;

    Results *res = ResultsMgr.FindResultsPtr( resid );
    if( res )
    {
        // Populate results.
        res->Add( new NameValData( "Interference", interference_flag, "Flag indicating the primary and secondary interfere." ) );
        res->Add( new NameValData( "Intersection", intersect_flag, "Flag indicating the primary and secondary intersect." ) );
        res->Add( new NameValData( "Primary_In_Secondary", primary_in_secondary, "Flag indicating the primary is contained within the secondary." ) );
        res->Add( new NameValData( "Secondary_In_Primary", secondary_in_primary, "Flag indicating the secondary is contained within the primary." ) );
        res->Add( new NameValData( "Min_Dist", min_dist, "Minimum distance between primary and secondary." ) );
        res->Add( new NameValData( "Pts", pts, "Minimum distance line end points." ) );
        res->Add( new NameValData( "InterferenceVol", vol, "Volume of interference." ) );
        res->Add( new NameValData( "Vol_Primary", vol_primary, "Volume of primary." ) );
        res->Add( new NameValData( "Vol_Secondary", vol_secondary, "Volume of secondary." ) );
        res->Add( new NameValData( "Con_Val", gcon, "Constraint value" ) );
        res->Add( new NameValData( "Result", gcon, "Interference result" ) );
    }
}

void ExteriorInterferenceCheck( vector< TMesh* > & primary_tmv, vector< TMesh* > & secondary_tmv, const string & resid, vector< TMesh* > & result_tmv )
{
    CSGMesh( primary_tmv );
    FlattenTMeshVec( primary_tmv );
    TMesh *primary_tm = MergeTMeshVec( primary_tmv );
    primary_tm->LoadBndBox();

    CSGMesh( secondary_tmv );
    FlattenTMeshVec( secondary_tmv );
    TMesh *secondary_tm = MergeTMeshVec( secondary_tmv );
    secondary_tm->LoadBndBox();

    ExteriorInterferenceCheck( primary_tm, secondary_tm, resid, result_tmv );
}

string PackagingInterferenceCheck( vector< TMesh* > & primary_tmv, vector< TMesh* > & secondary_tmv, vector< TMesh* > & result_tmv )
{
    bool intersect_flag = false;
    bool interference_flag = false;

    CSGMesh( primary_tmv );
    FlattenTMeshVec( primary_tmv );
    TMesh *primary_tm = MergeTMeshVec( primary_tmv );
    primary_tm->LoadBndBox();
    double vol_primary = primary_tm->ComputeTheoVol();

    CSGMesh( secondary_tmv );
    FlattenTMeshVec( secondary_tmv );
    TMesh *secondary_tm = MergeTMeshVec( secondary_tmv );
    secondary_tm->LoadBndBox();
    double vol_secondary = secondary_tm->ComputeTheoVol();


    double min_dist = 1.0e12;
    double con_dist = 1.0e12;
    double con_vol = -1; // Not the true volume.
    double vol = 0;

    bool primary_in_secondary = false;
    bool secondary_in_primary = true;
    vector < vec3d > pts;


    double vref = vol_secondary; // min( vol_primary, vol_secondary );

    if ( primary_tm->CheckIntersect( secondary_tm ) )
    {
        intersect_flag = true;
        interference_flag = true;

        // Notice secondary first.
        result_tmv.push_back( secondary_tm );
        result_tmv.push_back( primary_tm );

        MeshSubtract( result_tmv ); // Secondary - Primary
        FlattenTMeshVec( result_tmv ); // Not required for volume calculations, do it for visualization and later use.

        min_dist = 0.0;
        con_dist = 1.0;

        for ( int i = 0; i < result_tmv.size(); i++ )
        {
            // vol += result_tmv[i]->ComputeTrimVol();
            vol += result_tmv[i]->ComputeTheoVol();
        }
        con_vol = vol / vref;
    }
    else
    {
        pts.resize( 2 );
        min_dist = primary_tm->MinDistance( secondary_tm, min_dist, pts[0], pts[1] );
        con_dist = min_dist;

        if ( !primary_tm->m_TVec.empty() && !secondary_tm->m_TVec.empty() )
        {
            TTri *tri = secondary_tm->m_TVec[0];
            if ( !DeterIntExtTri( tri, primary_tm ) ) // b not inside a
            {
                secondary_in_primary = true;
                interference_flag = true;
                result_tmv.push_back( secondary_tm );
                con_vol = 1;
                con_dist += 1.0;
                delete primary_tm;
            }
            else
            {
                delete primary_tm;
                delete secondary_tm;
            }
        }
        else
        {
            delete primary_tm;
            delete secondary_tm;
        }
    }

    double gcon = con_dist * con_vol;

    Results *res = ResultsMgr.CreateResults( "Packaging_Interference", "Packaging interference check." );
    if( res )
    {
        // Populate results.
        res->Add( new NameValData( "Interference", interference_flag, "Flag indicating the primary and secondary interfere." ) );
        res->Add( new NameValData( "Intersection", intersect_flag, "Flag indicating the primary and secondary intersect." ) );
        res->Add( new NameValData( "Primary_In_Secondary", primary_in_secondary, "Flag indicating the primary is contained within the secondary." ) );
        res->Add( new NameValData( "Secondary_In_Primary", secondary_in_primary, "Flag indicating the secondary is contained within the primary." ) );
        res->Add( new NameValData( "Min_Dist", min_dist, "Minimum distance between primary and secondary." ) );
        res->Add( new NameValData( "Pts", pts, "Minimum distance line end points." ) );
        res->Add( new NameValData( "InterferenceVol", vol, "Volume of interference." ) );
        res->Add( new NameValData( "Vol_Primary", vol_primary, "Volume of primary." ) );
        res->Add( new NameValData( "Vol_Secondary", vol_secondary, "Volume of secondary." ) );
        res->Add( new NameValData( "Con_Val", gcon, "Constraint value" ) );
        res->Add( new NameValData( "Result", gcon, "Interference result" ) );
    }

    return res->GetID();
}

string ExteriorSelfInterferenceCheck( vector< TMesh* > & primary_tmv, vector< TMesh* > & result_tmv )
{
    bool intersect_flag = false;
    bool interference_flag = false;

    double min_dist = 1.0e12;
    double con_dist = 1.0e12;
    double con_vol = -1; // Not the true volume.
    double vol = 0;

    vector < vec3d > pts;

    double vref = 1.0e12;
    for ( int i = 0 ; i < ( int )primary_tmv.size() ; i++ )
    {
        vref = min( vref, primary_tmv[i]->ComputeTheoVol() );
    }

    if ( CheckSelfIntersect( primary_tmv ) )
    {
        intersect_flag = true;
        interference_flag = true;

        result_tmv = CopyTMeshVec( primary_tmv );

        MeshIntersect( result_tmv );
        FlattenTMeshVec( result_tmv ); // Not required for volume calculations, do it for visualization and later use.

        min_dist = 0.0;
        con_dist = 1.0;

        for ( int i = 0; i < result_tmv.size(); i++ )
        {
            vol += result_tmv[i]->ComputeTrimVol();
        }
        con_vol = vol / vref;
    }
    else
    {
        pts.resize( 2 );
        for ( int i = 0 ; i < (int)primary_tmv.size() - 1 ; i++ )
        {
            for ( int j = i + 1 ; j < (int)primary_tmv.size() ; j++ )
            {
                min_dist = primary_tmv[i]->MinDistance( primary_tmv[j], min_dist, pts[0], pts[1] );
            }
        }
        con_dist = min_dist;
    }

    double gcon = con_dist * con_vol;

    Results *res = ResultsMgr.CreateResults( "External_Self_Interference", "External self interference check." );
    if( res )
    {
        // Populate results.
        res->Add( new NameValData( "Interference", interference_flag, "Flag indicating the primary and secondary interfere." ) );
        res->Add( new NameValData( "Intersection", intersect_flag, "Flag indicating the primary and secondary intersect." ) );
        res->Add( new NameValData( "Min_Dist", min_dist, "Minimum distance between primary and secondary." ) );
        res->Add( new NameValData( "Pts", pts, "Minimum distance line end points." ) );
        res->Add( new NameValData( "InterferenceVol", vol, "Volume of interference." ) );
        res->Add( new NameValData( "Con_Val", gcon, "Constraint value" ) );
        res->Add( new NameValData( "Result", gcon, "Interference result" ) );
    }

    return res->GetID();
}

bool DecideIgnoreTri( int aType, const vector < int > & bTypes, const vector < bool > & thicksurf, const vector < bool > & aInB )
{
    // Always delete Stiffener tris
    if ( aType == vsp::CFD_STIFFENER )
    {
        return true;
    }

    bool aInOneNormal = false;

    for ( int b = 0 ; b < ( int )aInB.size() ; b++ )
    {
        bool aInThisB = aInB[b];
        int bType = bTypes[b];
        bool bThick = thicksurf[b];

        // Can make absolute decisions about deleting a triangle or not in the cases below
        if ( aInThisB && bThick )
        {
            if( bType == vsp::CFD_NORMAL )
            {
                aInOneNormal = true;
            }

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
         aType == vsp::CFD_STIFFENER ||  // Stiffener is special case -- always true previously.
         aType == vsp::CFD_MEASURE_DUCT )
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
            if ( aType == vsp::CFD_MEASURE_DUCT && aInOneNormal && bType == vsp::CFD_NEGATIVE )
            {
                return false;
            }
        }
    }

    return ignoretri;
}

double IntersectSplit( vector < TMesh * > &tmv, bool intSubsFlag, const vector < string > & sub_vec )
{
    //==== Scale To 1000 Units ====//
    BndBox bbox;
    Matrix4d mat;
    UpdateBBox( bbox, tmv, mat );

    double scalefac = 1000.0 / bbox.GetLargestDist();
    ApplyScale( scalefac, tmv );

    //==== Intersect Subsurfaces to make clean lines ====//
    if ( intSubsFlag )
    {
        for ( int i = 0 ; i < ( int )tmv.size() ; i++ )
        {
            vector < double > uvec;
            vector < double > vvec;
            if ( tmv[i]->m_UWPnts.size() > 0 && tmv[i]->m_UWPnts[0].size() > 0 )
            {
                uvec.resize( tmv[i]->m_UWPnts.size() );
                for ( int j = 0 ; j < ( int )tmv[i]->m_UWPnts.size(); j++ )
                {
                    uvec[j] = tmv[i]->m_UWPnts[j][0].x();
                }
                vvec.resize( tmv[i]->m_UWPnts[0].size() );
                for ( int j = 0; j < ( int ) tmv[ i ]->m_UWPnts[ 0 ].size(); j++ )
                {
                    vvec[j] = tmv[ i ]->m_UWPnts[ 0 ][ j ].y();
                }
            }

            vector< TMesh* > sub_surf_meshes;
            vector< SubSurface* > sub_surf_vec = SubSurfaceMgr.GetSubSurfs( tmv[i]->m_OriginGeomID, tmv[i]->m_SurfNum );
            int ss;
            for ( ss = 0 ; ss < ( int )sub_surf_vec.size() ; ss++ )
            {
                string subsurf_id = sub_surf_vec[ ss ]->GetID();

                if ( sub_vec.empty() || vector_contains_val( sub_vec, subsurf_id ) )
                {
                    vector< TMesh* > tmp_vec = sub_surf_vec[ss]->CreateTMeshVec( uvec, vvec );
                    sub_surf_meshes.insert( sub_surf_meshes.end(), tmp_vec.begin(), tmp_vec.end() );
                }
            }

            if ( !sub_surf_meshes.size() )
            {
                continue;    // Skip if no sub surface meshes
            }

            // Load All surf_mesh_bboxes
            for ( ss = 0 ; ss < ( int )sub_surf_meshes.size() ; ss++ )
            {
                // Build merge maps
                tmv[i]->BuildMergeMaps();

                sub_surf_meshes[ss]->LoadBndBox();
                // Swap the tmv[i]'s nodes to be UW instead of xyz
                tmv[i]->MakeNodePntUW();
                tmv[i]->LoadBndBox();

                // Intersect TMesh with sub_surface_meshes
                tmv[i]->Intersect( sub_surf_meshes[ss], true );

                // Split the triangles
                tmv[i]->Split();

                // Make current TMesh XYZ again and reset its octtree
                tmv[i]->MakeNodePntXYZ();
                tmv[i]->m_TBox.Reset();

                // Flatten Mesh
                TMesh* f_tmesh = new TMesh();
                f_tmesh->CopyFlatten( tmv[i] );
                delete tmv[i];
                tmv[i] = f_tmesh;
            }

            sub_surf_meshes.clear();
        }
    }
    // Tag meshes before regular intersection
    SubTagTris( (bool)intSubsFlag, tmv, sub_vec );

    //==== Check For Open Meshes and Merge or Delete Them ====//
    MeshInfo info;
    MergeRemoveOpenMeshes( tmv, &info, /* deleteopen */ false );

    //==== Create Bnd Box for  Mesh Geoms ====//
    for ( int i = 0 ; i < ( int )tmv.size() ; i++ )
    {
        tmv[i]->LoadBndBox();
    }

    //==== Intersect All Mesh Geoms ====//
    for ( int i = 0 ; i < ( int )tmv.size() ; i++ )
    {
        for ( int j = i + 1 ; j < ( int )tmv.size() ; j++ )
        {
            tmv[i]->Intersect( tmv[j] );
        }
    }

    //==== Split Intersected Tri in Mesh ====//
    for ( int i = 0 ; i < ( int )tmv.size() ; i++ )
    {
        tmv[i]->Split();
    }

    return scalefac;
}

void IntersectSplitClassify( vector < TMesh * > &tmv, bool intSubsFlag, const vector < string > & sub_vec )
{
    double scalefac = IntersectSplit( tmv, intSubsFlag, sub_vec );

    //==== Determine Which Triangle Are Interior/Exterior ====//
    for ( int i = 0 ; i < ( int )tmv.size() ; i++ )
    {
        tmv[i]->DeterIntExt( tmv );
    }

    //===== Reset Scale =====//
    ApplyScale( 1.0 / scalefac, tmv );
}

void CSGMesh( vector < TMesh * > &tmv, bool intSubsFlag, const vector < string > & sub_vec )
{
    IntersectSplitClassify( tmv, intSubsFlag, sub_vec );

    // Fill vector of cfdtypes so we don't have to pass TMeshVec all the way down.
    vector < int > bTypes( tmv.size() );
    vector < bool > thicksurf( tmv.size() );
    for ( int i = 0 ; i < ( int )tmv.size() ; i++ )
    {
        bTypes[i] = tmv[i]->m_SurfCfdType;
        thicksurf[i] = tmv[i]->m_ThickSurf;
    }

    //==== Mark which triangles to ignore ====//
    for ( int i = 0 ; i < ( int )tmv.size() ; i++ )
    {
        tmv[i]->SetIgnoreTriFlag( bTypes, thicksurf );
    }
}

void MeshUnion( vector < TMesh * > &tmv )
{
    IntersectSplitClassify( tmv, false );

    //==== Mark which triangles to ignore ====//
    for ( int i = 0 ; i < ( int )tmv.size() ; i++ )
    {
        tmv[i]->SetIgnoreInsideAny();
    }
}

void MeshCCEIntersect( vector < TMesh * > &tmv )
{
    double scalefac = IntersectSplit( tmv, false );

    vec3d upish( 0.000001, 0.000001, 1.0 );

    //==== Determine Which Triangle Are Interior/Exterior ====//
    for ( int i = 0 ; i < ( int )tmv.size() ; i++ )
    {
        tmv[i]->DeterIntExt( tmv, upish );
    }

    //===== Reset Scale =====//
    ApplyScale( 1.0 / scalefac, tmv );

    //==== Mark which triangles to ignore ====//
    for ( int i = 0 ; i < ( int )tmv.size() ; i++ )
    {
        tmv[i]->SetIgnoreInsideNotOne();
    }
}

void MeshIntersect( vector < TMesh * > &tmv )
{
    IntersectSplitClassify( tmv, false );

    //==== Mark which triangles to ignore ====//
    for ( int i = 0 ; i < ( int )tmv.size() ; i++ )
    {
        tmv[i]->SetIgnoreInsideNotOne();
    }
}

// Subtract meshes i != 0 from mesh j == 0.
void MeshSubtract( vector < TMesh* > & tmv )
{
    IntersectSplitClassify( tmv, false );

    vector < bool > mask( tmv.size(), false );
    mask[0] = true;

    tmv[0]->SetIgnoreInsideAny();;
    //==== Mark which triangles to ignore ====//
    for ( int i = 1 ; i < ( int )tmv.size() ; i++ )
    {
        tmv[i]->SetKeepMatchMask( mask );
        tmv[i]->FlipNormals();
    }
}

void MeshCutAbovePlane( vector < TMesh* > & tmv, const vector <vec3d> & threepts )
{
    IntersectSplitClassify( tmv, false );

    tmv[0]->SetIgnoreOutsideAll();
    tmv[1]->SetIgnoreAbovePlane( threepts );
}

TMesh* OctantSplitMesh( TMesh* tm )
{
    tm->LoadBndBox();
    BndBox bb = tm->m_TBox.m_Box;
    bb.Update( vec3d() ); // Make sure origin is in BBox
    double len = 2.1 * bb.DiagDist();

    // Slice along X, Y, Z = 0 planes to make sure all triangles lie in a single octant.
    for ( int idir = 0; idir < 3; idir++ )
    {
        vec3d org, norm;
        norm.v[ idir ] = 1;
        TMesh * slice = MakeSlice( org, norm, len );

        slice->LoadBndBox();

        tm->Intersect( slice );

        delete slice;
    }
    tm->Split();

    TMesh *flat_tm = new TMesh;
    flat_tm->CopyFlatten( tm );
    delete tm;

    return flat_tm;
}

TMesh* MakeConvexHull(const vector< TMesh* > & tmesh_vec )
{
    TMesh* tMesh = nullptr;

    int npts = 0;
    for ( int i = 0; i < ( int )tmesh_vec.size() ; i++ )
    {
        npts += tmesh_vec[ i ]->m_NVec.size();
    }
    gte::Vector3 < double > *pts = new gte::Vector3 < double > [ npts ];

    int k = 0;
    for ( int i = 0; i < ( int )tmesh_vec.size() ; i++ )
    {
        for ( int j = 0; j < tmesh_vec[ i ]->m_NVec.size(); j++ )
        {
            pts[ k ][ 0 ] = tmesh_vec[ i ]->m_NVec[ j ]->m_Pnt.x();
            pts[ k ][ 1 ] = tmesh_vec[ i ]->m_NVec[ j ]->m_Pnt.y();
            pts[ k ][ 2 ] = tmesh_vec[ i ]->m_NVec[ j ]->m_Pnt.z();
            k++;
        }
    }

    gte::ConvexHull3 < double > ch;

    ch( npts, pts, 0 );

    if ( ch.GetDimension() == 3 )
    {
        tMesh = new TMesh();

        if ( tMesh )
        {
            std::vector < size_t > hull = ch.GetHull();

            vector < bool > vused( npts, false );
            for ( int i = 0; i < hull.size(); i++ )
            {
                vused[ hull[ i ] ] = true;
            }

            vector < int > vxref( npts, -1 );
            int iused = 0;
            for ( int i = 0 ; i < ( int )npts ; i++ )
            {
                if ( vused[ i ] )
                {
                    TNode *n = new TNode();
                    n->m_Pnt = vec3d( pts[ i ][ 0 ], pts[ i ][ 1 ], pts[ i ][ 2 ] );
                    n->m_ID = i;
                    tMesh->m_NVec.push_back( n );
                    vxref[ i ] = iused;
                    iused++;
                }
            }

            int nFaces = hull.size() / 3;
            tMesh->m_TVec.reserve( nFaces );

            for ( int i = 0; i < nFaces; i++ )
            {
                TTri *t = new TTri( tMesh );

                t->m_N0 = tMesh->m_NVec[ vxref[ hull[ 3 * i + 0 ] ] ];
                t->m_N1 = tMesh->m_NVec[ vxref[ hull[ 3 * i + 1 ] ] ];
                t->m_N2 = tMesh->m_NVec[ vxref[ hull[ 3 * i + 2 ] ] ];

                t->CompNorm();
                tMesh->m_TVec.push_back( t );
            }
        }
    }

    delete[] pts;
    return tMesh;
}

void DeterIntExtTri( TTri* tri, const vector< TMesh* >& meshVec, const vec3d &dir )
{
    vec3d orig = ( tri->m_N0->m_Pnt + tri->m_N1->m_Pnt + tri->m_N2->m_Pnt ) / 3.0;
    tri->m_IgnoreTriFlag = false;
    int prior = -1;

    int nmesh = meshVec.size();
    tri->m_insideSurf.clear();
    tri->m_insideSurf.resize( nmesh, false );

    for ( int m = 0 ; m < ( int )meshVec.size() ; m++ )
    {
        if ( meshVec[m] != tri->GetTMeshPtr() && meshVec[m]->m_ThickSurf )
        {
            vector<double > tParmVec;
            vector <TTri*> triVec;
            meshVec[m]->m_TBox.RayCast( orig, dir, tParmVec, triVec );
            if ( tParmVec.size() % 2 )
            {
                tri->m_insideSurf[m] = true;

                // Priority assignment for wave drag.  Mass prop may need some adjustments.
                if ( meshVec[m]->m_MassPrior > prior ) // Should possibly check that priority is only for vsp::CFD_NORMAL
                {
                    // Assigns GeomID to slice triangles for later use by Wave Drag and Mass Properties.
                    tri->m_GeomID = meshVec[m]->m_OriginGeomID;
                    tri->m_Density = meshVec[m]->m_Density;
                    prior = meshVec[m]->m_MassPrior;
                }
            }
        }
    }
}

bool DeterIntExtTri( TTri* tri, TMesh* mesh, const vec3d &dir )
{
    if ( tri )
    {
        vector <TMesh*> tmv;
        tmv.push_back( mesh );
        DeterIntExtTri( tri, tmv, dir );

        if ( !tri->m_insideSurf.empty() )
        {
            return tri->m_insideSurf[0];
        }
    }
    return false;
}

void WriteStl( const string &file_name, const vector< TMesh* >& meshVec )
{
    Matrix4d mat;

    FILE* fid = fopen( file_name.c_str(), "w" );
    if ( fid )
    {
        fprintf( fid, "solid\n" );

        for ( int i = 0; i < (int) meshVec.size(); i++ )
        {
            meshVec[i]->WriteSTLTris( fid, mat );
        }

        fprintf( fid, "endsolid\n" );
        fclose( fid );
    }
}

void WriteInsideMStl( const string &file_name, const vector< TMesh* >& meshVec, int minside )
{
    Matrix4d mat;

    FILE* fid = fopen( file_name.c_str(), "w" );
    if ( fid )
    {
        fprintf( fid, "solid\n" );

        for ( int i = 0; i < (int) meshVec.size(); i++ )
        {
            meshVec[i]->WriteInsideMStlTris( fid, mat, minside );
        }

        fprintf( fid, "endsolid\n" );
        fclose( fid );
    }
}

void WriteIgnoredSTL( const string &file_name, const vector< TMesh* >& meshVec )
{
    Matrix4d mat;

    FILE* fid = fopen( file_name.c_str(), "w" );
    if ( fid )
    {
        fprintf( fid, "solid\n" );

        for ( int i = 0; i < (int) meshVec.size(); i++ )
        {
            meshVec[i]->WriteIgnoredSTLTris( fid, mat );
        }

        fprintf( fid, "endsolid\n" );
        fclose( fid );
    }
}

void WriteStl( const string &file_name, TMesh* tm )
{
    Matrix4d mat;

    FILE* fid = fopen( file_name.c_str(), "w" );
    if ( fid && tm )
    {
        fprintf( fid, "solid\n" );

        tm->WriteSTLTris( fid, mat );

        fprintf( fid, "endsolid\n" );
        fclose( fid );
    }
}

void WriteInsideMStl( const string &file_name, TMesh* tm, int minside )
{
    Matrix4d mat;

    FILE* fid = fopen( file_name.c_str(), "w" );
    if ( fid && tm )
    {
        fprintf( fid, "solid\n" );

        tm->WriteInsideMStlTris( fid, mat, minside );

        fprintf( fid, "endsolid\n" );
        fclose( fid );
    }
}


void MakeThreePts( const vec3d & org, const vec3d & norm, vector <vec3d> &threepts )
{
    threepts.clear();
    threepts.reserve( 3 );

    threepts.emplace_back( 0, 1.0, 1.0 );
    threepts.emplace_back( 0, -1.0, 1.0 );
    threepts.emplace_back( 0, -1.0, -1.0 );

    Matrix4d rot, trans;
    rot.rotatealongX( norm );
    rot.affineInverse();

    trans.translatev( org );
    rot.postMult( trans );

    rot.xformvec( threepts );
}

TMesh* MakeSlice( const vec3d & org, const vec3d & norm, const double & len )
{
    TMesh* tm = MakeSlice( vsp::X_DIR, len );

    Matrix4d rot, trans;
    rot.rotatealongX( norm );
    rot.affineInverse();

    trans.translatev( org );
    rot.postMult( trans );

    tm->Transform( rot );

    return tm;
}

TMesh* MakeSlice( const int &swdir, const double & len )
{
    bool tesselate = true;
    int ntess = 10;

    int dir1, dir2;
    if ( swdir == vsp::X_DIR )
    {
        dir1 = vsp::Y_DIR;
        dir2 = vsp::Z_DIR;
    }
    else if ( swdir == vsp::Y_DIR )
    {
        dir1 = vsp::Z_DIR;
        dir2 = vsp::X_DIR;
    }
    else
    {
        dir1 = vsp::X_DIR;
        dir2 = vsp::Y_DIR;
    }

    double del1 = len;
    double s1   = -0.5 * len;
    double del2 = len;
    double s2   = -0.5 * len;

    vec3d n;
    n[ swdir ] = 1;

    TMesh* tm = new TMesh();

    tm->m_ThickSurf = false;
    tm->m_SurfCfdType = vsp::CFD_STRUCTURE;

    if ( tesselate )
    {

        double ds = 1.0 / (double) ntess;
        int iQuad = 0;
        for ( int i = 0; i < ntess; i++ )
        {
            double d10 = s1 + del1 * ds * ( double )i;
            double d11 = s1 + del1 * ds * ( double )( i + 1 );

            for ( int j = 0; j < ntess; j++ )
            {
                double d20 = s2 + del2 * ds * ( double )j;
                double d21 = s2 + del2 * ds * ( double )( j + 1 );

                vec3d p1, p2, p3, p4;
                p1[swdir] = 0;
                p1[dir1] = d10;
                p1[dir2] = d20;

                p2[swdir] = 0;
                p2[dir1] = d11;
                p2[dir2] = d20;

                p3[swdir] = 0;
                p3[dir1] = d11;
                p3[dir2] = d21;

                p4[swdir] = 0;
                p4[dir1] = d10;
                p4[dir2] = d21;

                tm->AddTri( p1, p2, p3, n, iQuad );
                tm->AddTri( p1, p3, p4, n, iQuad );
                iQuad++;
            }
        }
    }
    else
    {
        vec3d p1, p2, p3, p4;
        p1[swdir] = 0;
        p1[dir1] = s1;
        p1[dir2] = s2;

        p2[swdir] = 0;
        p2[dir1] = s1 + del1;
        p2[dir2] = s2;

        p3[swdir] = 0;
        p3[dir1] = s1 + del1;
        p3[dir2] = s2 + del2;

        p4[swdir] = 0;
        p4[dir1] = s1;
        p4[dir2] = s2 + del2;

        tm->AddTri( p1, p2, p3, n, 1 );
        tm->AddTri( p1, p3, p4, n, 1 );
    }

    return tm;
}

double MakeSlices( vector<TMesh*> &tmv, const BndBox & bbox, int numSlices, int swdir, vector < double > &slicevec, bool mpslice, bool tesselate, bool autoBounds, double start, double end, int slctype )
{
    int s, i, j;
    double offset = 0.0001; // Amount to extend slicing bounds.

    int dir1, dir2;
    if ( swdir == vsp::X_DIR )
    {
        dir1 = vsp::Y_DIR;
        dir2 = vsp::Z_DIR;
    }
    else if ( swdir == vsp::Y_DIR )
    {
        dir1 = vsp::Z_DIR;
        dir2 = vsp::X_DIR;
    }
    else
    {
        dir1 = vsp::X_DIR;
        dir2 = vsp::Y_DIR;
    }

    double swMin;
    double swMax;
    if ( autoBounds )
    {
        swMin = bbox.GetMin( swdir ) - offset;
        swMax = bbox.GetMax( swdir ) + offset;
    }
    else
    {
        swMin = start - offset;
        swMax = end + offset;
    }

    // MassProp slice always uses autobounds.  Does not need offset because slices are later shifted by width/2.
    if ( mpslice )
    {
        swMin = bbox.GetMin( swdir );
        swMax = bbox.GetMax( swdir );
    }

    double sliceW;
    if ( mpslice )
    {
        sliceW = ( swMax - swMin ) / ( double )( numSlices );
    }
    else
    {
        if ( numSlices > 1 )
        {
            sliceW = ( swMax - swMin ) / ( double )( numSlices - 1 );
        }
        else
        {
            sliceW = 0.0;
        }
    }
    slicevec.resize( numSlices );

    double del1 = 1.02 * ( bbox.GetMax( dir1 ) - bbox.GetMin( dir1 ) );
    double s1   = bbox.GetMin( dir1 ) - 0.01 * del1;
    double del2 = 1.02 * ( bbox.GetMax( dir2 ) - bbox.GetMin( dir2 ) );
    double s2   = bbox.GetMin( dir2 ) - 0.01 * del2;

    vec3d n;
    n[ swdir ] = 1;

    for ( s = 0 ; s < numSlices ; s++ )
    {
        TMesh* tm = new TMesh();

        tm->m_ThickSurf = false;
        tm->m_SurfCfdType = slctype;

        tmv.push_back( tm );

        double sw;
        if ( mpslice )
        {
            sw = swMin + ( double )s * sliceW + 0.5 * sliceW;
        }
        else
        {
            sw = swMin + ( double )s * sliceW;
        }
        slicevec[s] = sw;

        if ( tesselate )
        {
            int ntess = numSlices;
            double ds = 1.0 / (double) ntess;
            int iQuad = 0;
            for ( i = 0 ; i < ntess ; i++ )
            {
                double d10 = s1 + del1 * ds * ( double )i;
                double d11 = s1 + del1 * ds * ( double )( i + 1 );

                for ( j = 0 ; j < ntess ; j++ )
                {
                    double d20 = s2 + del2 * ds * ( double )j;
                    double d21 = s2 + del2 * ds * ( double )( j + 1 );

                    vec3d p1, p2, p3, p4;
                    p1[swdir] = sw;
                    p1[dir1] = d10;
                    p1[dir2] = d20;

                    p2[swdir] = sw;
                    p2[dir1] = d11;
                    p2[dir2] = d20;

                    p3[swdir] = sw;
                    p3[dir1] = d11;
                    p3[dir2] = d21;

                    p4[swdir] = sw;
                    p4[dir1] = d10;
                    p4[dir2] = d21;

                    tm->AddTri( p1, p2, p3, n, iQuad );
                    tm->AddTri( p1, p3, p4, n, iQuad );
                    iQuad++;
                }
            }
        }
        else
        {
            vec3d p1, p2, p3, p4;
            p1[swdir] = sw;
            p1[dir1] = s1;
            p1[dir2] = s2;

            p2[swdir] = sw;
            p2[dir1] = s1 + del1;
            p2[dir2] = s2;

            p3[swdir] = sw;
            p3[dir1] = s1 + del1;
            p3[dir2] = s2 + del2;

            p4[swdir] = sw;
            p4[dir1] = s1;
            p4[dir2] = s2 + del2;

            tm->AddTri( p1, p2, p3, n, 1 );
            tm->AddTri( p1, p3, p4, n, 1 );
        }
    }
    return sliceW;
}
