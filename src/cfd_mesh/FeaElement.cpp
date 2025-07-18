//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// FeaElement.cpp
//
// Justin Gravett
//////////////////////////////////////////////////////////////////////

#define _USE_MATH_DEFINES
#include <cmath>

#include "FeaElement.h"
#include "StructureMgr.h"
#include "FeaMeshMgr.h"
#include "StringUtil.h"
#include "StlHelper.h"

using namespace StringUtil;

//////////////////////////////////////////////////////
//==================== FeaNode =====================//
//////////////////////////////////////////////////////

void FeaNode::AddTag( int index )
{
    //==== Check For Duplicate Tags ====//
    for ( int i = 0; i < (int)m_Tags.size(); i++ )
    {
        if ( m_Tags[i] == index )
        {
            return;
        }
    }

    m_Tags.push_back( index );
}

bool FeaNode::HasTag( int index )
{
    for ( int i = 0; i < (int)m_Tags.size(); i++ )
    {
        if ( m_Tags[i] == index )
        {
            return true;
        }
    }
    return false;
}

bool FeaNode::HasOnlyTag( int index )
{
    for ( int i = 0; i < (int)m_Tags.size(); i++ )
    {
        if ( m_Tags[i] != index )
        {
            return false;
        }
    }
    return true;
}

long long int FeaNode::GetIndex()
{
    return m_Index;
}

void FeaNode::WriteNASTRAN( FILE* fp, long long int noffset, bool includeBC )
{
    double x = m_Pnt.x();
    double y = m_Pnt.y();
    double z = m_Pnt.z();

    string bcstr;
    if ( includeBC )
    {
        bcstr = m_BCs.AsNASTRAN();
    }

    string fmt = "GRID    ,%8lld,        ," + NasFmt( x ) + "," + NasFmt( y ) + "," + NasFmt( z ) + ",        ,%s\n";
    fprintf( fp, fmt.c_str(), m_Index + noffset, x, y, z, bcstr.c_str() );
}

void FeaNode::WriteNASTRAN_SPC1( FILE* fp, long long int noffset )
{
    if ( m_BCs.AsNum() > 0 )
    {
        string bcstr = m_BCs.AsNASTRAN();

        fprintf( fp, "SPC1    ,       1,%8s,%8lld\n",  bcstr.c_str(), m_Index + noffset );
    }
}

void FeaNode::WriteCalculix( FILE* fp, long long int noffset )
{
    if ( true )   // print 'just' nodes.
    {
        fprintf( fp, "%lld,%f,%f,%f\n", m_Index + noffset, m_Pnt.x(), m_Pnt.y(), m_Pnt.z() );
    }
    else          // also print node tags for debugging.
    {
        fprintf( fp, "%lld,%f,%f,%f  ** ", m_Index + noffset, m_Pnt.x(), m_Pnt.y(), m_Pnt.z() );
        for ( int i = 0; i < m_Tags.size(); i++ )
        {
            fprintf( fp, "%d ", m_Tags[ i ] );
        }
        fprintf( fp, "\n" );
    }
}

void FeaNode::WriteCalculixBCs( FILE* fp, long long int noffset )
{
    int ndof = 6;

    vector < bool > bv = m_BCs.AsBoolVec( ndof );
    for ( int i = 0; i < ndof; i++ )
    {
        if ( bv[i] )
        {
            fprintf( fp, "%lld,%d\n", m_Index + noffset, i + 1 );
        }
    }
}

void FeaNode::WriteGmsh( FILE* fp, long long int noffset )
{
    fprintf( fp, "%lld %f %f %f\n", m_Index + noffset, m_Pnt.x(), m_Pnt.y(), m_Pnt.z() );
}

//////////////////////////////////////////////////////
//================== FeaElement ====================//
//////////////////////////////////////////////////////

FeaElement::FeaElement()
{
    m_FeaPartSurfNum = -1;
    m_ElementType = FEA_TRI_6;
    m_FeaPartIndex = -1;
    m_FeaSSIndex = -1;
    m_reason = -1;
    m_ChainIndex = -1;
}

void FeaElement::DeleteAllNodes()
{
    int i;
    for ( i = 0; i < (int)m_Corners.size(); i++ )
    {
        delete m_Corners[i];
    }
    for ( i = 0; i < (int)m_Mids.size(); i++ )
    {
        delete m_Mids[i];
    }

    m_Corners.clear();
    m_Mids.clear();
}
void FeaElement::LoadNodes( vector< FeaNode* > & node_vec )
{
    int i;
    for ( i = 0; i < (int)m_Corners.size(); i++ )
    {
        node_vec.push_back( m_Corners[i] );
    }
    for ( i = 0; i < (int)m_Mids.size(); i++ )
    {
        node_vec.push_back( m_Mids[i] );
    }
}

//////////////////////////////////////////////////////
//==================== FeaTri ======================//
//////////////////////////////////////////////////////

void FeaTri::Create( vec3d & p0, vec3d & p1, vec3d & p2, bool highorder )
{
    DeleteAllNodes();
    m_Corners.push_back( new FeaNode( p0 ) );
    m_Corners.push_back( new FeaNode( p1 ) );
    m_Corners.push_back( new FeaNode( p2 ) );

    if ( highorder )
    {
        m_ElementType = FEA_TRI_6;

        vec3d p01 = ( p0 + p1 ) * 0.5;
        vec3d p12 = ( p1 + p2 ) * 0.5;
        vec3d p20 = ( p2 + p0 ) * 0.5;

        m_Mids.push_back( new FeaNode( p01 ) );
        m_Mids.push_back( new FeaNode( p12 ) );
        m_Mids.push_back( new FeaNode( p20 ) );
    }
    else
    {
        m_ElementType = FEA_TRI_3;
    }
}

void FeaTri::WriteCalculix( FILE* fp, int id, long long int noffset, long long int eoffset )
{
    if ( m_ElementType == FEA_TRI_3 )
    {
        fprintf( fp, "%lld,%lld,%lld,%lld\n", id + eoffset,
                 m_Corners[0]->GetIndex() + noffset, m_Corners[1]->GetIndex() + noffset, m_Corners[2]->GetIndex() + noffset );
    }
    else
    {
        fprintf( fp, "%lld,%lld,%lld,%lld,%lld,%lld,%lld\n", id + eoffset,
                 m_Corners[0]->GetIndex() + noffset, m_Corners[1]->GetIndex() + noffset, m_Corners[2]->GetIndex() + noffset,
                 m_Mids[0]->GetIndex() + noffset, m_Mids[1]->GetIndex() + noffset, m_Mids[2]->GetIndex() + noffset );
    }
}

void FeaTri::WriteNASTRAN( FILE* fp, int id, int property_index, long long int noffset, long long int eoffset )
{
    vec3d v01 = m_Corners[1]->m_Pnt - m_Corners[0]->m_Pnt;
    vec3d v12 = m_Corners[2]->m_Pnt - m_Corners[1]->m_Pnt;
    v01.normalize();
    v12.normalize();
    vec3d norm = cross( v01, v12);
    norm.normalize();

    double theta_material = ( 180.0 / M_PI ) * signed_angle( v01, m_Orientation, norm );

    if ( theta_material < 0 )
    {
        theta_material += 180.0;
    }


    if ( m_ElementType == FEA_TRI_3 )
    {
        string format_string = "CTRIA3  ,%8lld,%8lld,%8lld,%8lld,%8lld," + NasFmt( theta_material ) + "\n";

        fprintf( fp, format_string.c_str(), id + eoffset, property_index + 1,
                 m_Corners[0]->GetIndex() + noffset, m_Corners[1]->GetIndex() + noffset, m_Corners[2]->GetIndex() + noffset,
                 theta_material );
    }
    else
    {
        string format_string = "CTRIA6  ,%8lld,%8lld,%8lld,%8lld,%8lld,%8lld,%8lld,%8lld,\n        ," + NasFmt( theta_material ) + "\n";

        fprintf( fp, format_string.c_str(), id + eoffset, property_index + 1,
                 m_Corners[0]->GetIndex() + noffset, m_Corners[1]->GetIndex() + noffset, m_Corners[2]->GetIndex() + noffset,
                 m_Mids[0]->GetIndex() + noffset, m_Mids[1]->GetIndex() + noffset, m_Mids[2]->GetIndex() + noffset, theta_material );
    }
}

void FeaTri::WriteGmsh( FILE* fp, int id, int fea_part_index, long long int noffset, long long int eoffset )
{
    if ( m_ElementType == FEA_TRI_3 )
    {
        // 3-node triangle element type (2)
        fprintf( fp, "%lld 2 1 %d %lld %lld %lld\n", id + eoffset, fea_part_index,
                 m_Corners[0]->GetIndex() + noffset, m_Corners[1]->GetIndex() + noffset, m_Corners[2]->GetIndex() + noffset );
    }
    else
    {
        // 6-node second order triangle element type (9)
        fprintf( fp, "%lld 9 1 %d %lld %lld %lld %lld %lld %lld\n", id + eoffset, fea_part_index,
                 m_Corners[0]->GetIndex() + noffset, m_Corners[1]->GetIndex() + noffset, m_Corners[2]->GetIndex() + noffset,
                 m_Mids[0]->GetIndex() + noffset,m_Mids[1]->GetIndex() + noffset, m_Mids[2]->GetIndex() + noffset );
    }
}

void FeaTri::WriteSTL( FILE* fp )
{
    vec3d p0 = m_Corners[0]->m_Pnt;
    vec3d p1 = m_Corners[1]->m_Pnt;
    vec3d p2 = m_Corners[2]->m_Pnt;
    vec3d v01 = p1 - p0;
    vec3d v12 = p2 - p1;
    vec3d norm = cross( v01, v12 );
    norm.normalize();

    fprintf( fp, " facet normal  %2.10le %2.10le %2.10le\n",  norm.x(), norm.y(), norm.z() );
    fprintf( fp, "   outer loop\n" );

    fprintf( fp, "     vertex %2.10le %2.10le %2.10le\n", p0.x(), p0.y(), p0.z() );
    fprintf( fp, "     vertex %2.10le %2.10le %2.10le\n", p1.x(), p1.y(), p1.z() );
    fprintf( fp, "     vertex %2.10le %2.10le %2.10le\n", p2.x(), p2.y(), p2.z() );

    fprintf( fp, "   endloop\n" );
    fprintf( fp, " endfacet\n" );
}

double FeaTri::ComputeMass( int property_index )
{
    double mass = 0.0;
    if ( m_Corners.size() < 3 )
    {
        return mass;
    }

    double a = area( m_Corners[0]->m_Pnt, m_Corners[1]->m_Pnt, m_Corners[2]->m_Pnt );

    double avg_t = 0;
    int mat_index = -1;

    vector < SimpleFeaProperty > simp_prop_vec = FeaMeshMgr.GetSimplePropertyVec();

    if ( property_index < simp_prop_vec.size() && property_index >= 0 && simp_prop_vec.size() > 0 )
    {
        avg_t = simp_prop_vec[property_index].m_Thickness;
        mat_index = simp_prop_vec[property_index].GetSimpFeaMatIndex();
    }

    double avg_d = 0;

    vector < SimpleFeaMaterial > simp_mat_vec = FeaMeshMgr.GetSimpleMaterialVec();

    if ( mat_index < simp_mat_vec.size() && mat_index >= 0 && simp_mat_vec.size() > 0 )
    {
        avg_d = simp_mat_vec[mat_index].m_MassDensity;
    }

    mass = a * avg_t * avg_d;
    return mass;
}

//////////////////////////////////////////////////////
//=================== FeaQuad ======================//
//////////////////////////////////////////////////////

void FeaQuad::Create( vec3d & p0, vec3d & p1, vec3d & p2, vec3d & p3, bool highorder )
{
    DeleteAllNodes();
    m_Corners.push_back( new FeaNode( p0 ) );
    m_Corners.push_back( new FeaNode( p1 ) );
    m_Corners.push_back( new FeaNode( p2 ) );
    m_Corners.push_back( new FeaNode( p3 ) );

    if ( highorder )
    {
        m_ElementType = FEA_QUAD_8;

        vec3d p01 = ( p0 + p1 ) * 0.5;
        vec3d p12 = ( p1 + p2 ) * 0.5;
        vec3d p23 = ( p2 + p3 ) * 0.5;
        vec3d p30 = ( p3 + p0 ) * 0.5;

        m_Mids.push_back( new FeaNode( p01 ) );
        m_Mids.push_back( new FeaNode( p12 ) );
        m_Mids.push_back( new FeaNode( p23 ) );
        m_Mids.push_back( new FeaNode( p30 ) );
    }
    else
    {
        m_ElementType = FEA_QUAD_4;
    }
}

void FeaQuad::WriteCalculix( FILE* fp, int id, long long int noffset, long long int eoffset )
{
    if ( m_ElementType == FEA_QUAD_4 )
    {
        fprintf( fp, "%lld,%lld,%lld,%lld,%lld\n", id + eoffset,
                 m_Corners[0]->GetIndex() + noffset, m_Corners[1]->GetIndex() + noffset, m_Corners[2]->GetIndex() + noffset, m_Corners[3]->GetIndex() + noffset );
    }
    else
    {
        fprintf( fp, "%lld,%lld,%lld,%lld,%lld,%lld,%lld,%lld,%lld\n", id + eoffset,
                 m_Corners[0]->GetIndex() + noffset, m_Corners[1]->GetIndex() + noffset, m_Corners[2]->GetIndex() + noffset, m_Corners[3]->GetIndex() + noffset,
                 m_Mids[0]->GetIndex() + noffset, m_Mids[1]->GetIndex() + noffset, m_Mids[2]->GetIndex() + noffset, m_Mids[3]->GetIndex() + noffset );
    }
}
void FeaQuad::WriteNASTRAN( FILE* fp, int id, int property_index, long long int noffset, long long int eoffset )
{
    vec3d v01 = m_Corners[1]->m_Pnt - m_Corners[0]->m_Pnt;
    vec3d v12 = m_Corners[2]->m_Pnt - m_Corners[1]->m_Pnt;
    v01.normalize();
    v12.normalize();
    vec3d norm = cross( v01, v12);
    norm.normalize();

    double theta_material = ( 180.0 / M_PI ) * signed_angle( v01, m_Orientation, norm );

    if ( theta_material < 0 )
    {
        theta_material += 180.0;
    }

    if ( m_ElementType == FEA_QUAD_4 )
    {
        string format_string = "CQUAD4  ,%8lld,%8d,%8lld,%8lld,%8lld,%8lld," + NasFmt( theta_material ) + "\n";

        fprintf( fp, format_string.c_str(), id + eoffset, property_index + 1,
                 m_Corners[0]->GetIndex() + noffset, m_Corners[1]->GetIndex() + noffset, m_Corners[2]->GetIndex() + noffset, m_Corners[3]->GetIndex() + noffset, theta_material );
    }
    else
    {
        string format_string = "CQUAD8  ,%8lld,%8d,%8lld,%8lld,%8lld,%8lld,%8lld,%8lld,\n        ,%8lld,%8lld,        ,        ,        ,        ," + NasFmt( theta_material ) + "\n";

        fprintf( fp, format_string.c_str(), id + eoffset, property_index + 1,
                 m_Corners[0]->GetIndex() + noffset, m_Corners[1]->GetIndex() + noffset, m_Corners[2]->GetIndex() + noffset, m_Corners[3]->GetIndex() + noffset,
                 m_Mids[0]->GetIndex() + noffset, m_Mids[1]->GetIndex() + noffset, m_Mids[2]->GetIndex() + noffset, m_Mids[3]->GetIndex() + noffset, theta_material );

    }
}

void FeaQuad::WriteGmsh( FILE* fp, int id, int fea_part_index, long long int noffset, long long int eoffset )
{
    if ( m_ElementType == FEA_QUAD_4 )
    {
        // 4-node quadrangle element type (3)
        fprintf( fp, "%lld 3 1 %d %lld %lld %lld %lld\n", id + eoffset, fea_part_index,
                 m_Corners[0]->GetIndex() + noffset, m_Corners[1]->GetIndex() + noffset, m_Corners[2]->GetIndex() + noffset, m_Corners[3]->GetIndex() + noffset );
    }
    else
    {
        // 8-node second order quadrangle element type (16)
        fprintf( fp, "%lld 16 1 %d %lld %lld %lld %lld %lld %lld %lld %lld\n", id + eoffset, fea_part_index,
                 m_Corners[0]->GetIndex() + noffset, m_Corners[1]->GetIndex() + noffset, m_Corners[2]->GetIndex() + noffset, m_Corners[3]->GetIndex() + noffset,
                 m_Mids[0]->GetIndex() + noffset, m_Mids[1]->GetIndex() + noffset, m_Mids[2]->GetIndex() + noffset, m_Mids[3]->GetIndex() + noffset );
    }
}

void FeaQuad::WriteSTL( FILE* fp )
{
    vec3d p0 = m_Corners[0]->m_Pnt;
    vec3d p1 = m_Corners[1]->m_Pnt;
    vec3d p2 = m_Corners[2]->m_Pnt;
    vec3d v01 = p1 - p0;
    vec3d v12 = p2 - p1;
    vec3d norm = cross( v01, v12 );
    norm.normalize();

    fprintf( fp, " facet normal  %2.10le %2.10le %2.10le\n",  norm.x(), norm.y(), norm.z() );
    fprintf( fp, "   outer loop\n" );

    fprintf( fp, "     vertex %2.10le %2.10le %2.10le\n", p0.x(), p0.y(), p0.z() );
    fprintf( fp, "     vertex %2.10le %2.10le %2.10le\n", p1.x(), p1.y(), p1.z() );
    fprintf( fp, "     vertex %2.10le %2.10le %2.10le\n", p2.x(), p2.y(), p2.z() );

    fprintf( fp, "   endloop\n" );
    fprintf( fp, " endfacet\n" );

    vec3d p3 = m_Corners[3]->m_Pnt;
    vec3d v23 = p3 - p2;
    vec3d v30 = p0 - p3;
    norm = cross( v23, v30 );
    norm.normalize();

    fprintf( fp, " facet normal  %2.10le %2.10le %2.10le\n",  norm.x(), norm.y(), norm.z() );
    fprintf( fp, "   outer loop\n" );

    fprintf( fp, "     vertex %2.10le %2.10le %2.10le\n", p0.x(), p0.y(), p0.z() );
    fprintf( fp, "     vertex %2.10le %2.10le %2.10le\n", p2.x(), p2.y(), p2.z() );
    fprintf( fp, "     vertex %2.10le %2.10le %2.10le\n", p3.x(), p3.y(), p3.z() );

    fprintf( fp, "   endloop\n" );
    fprintf( fp, " endfacet\n" );
}

double FeaQuad::ComputeMass( int property_index )
{
    double mass = 0.0;
    if ( m_Corners.size() < 4 )
    {
        return mass;
    }

    double a012 = area( m_Corners[0]->m_Pnt, m_Corners[1]->m_Pnt, m_Corners[2]->m_Pnt );
    double a023 = area( m_Corners[0]->m_Pnt, m_Corners[2]->m_Pnt, m_Corners[3]->m_Pnt );
    double a = a012 + a023;

    double avg_t = 0;
    int mat_index = -1;

    vector < SimpleFeaProperty > simp_prop_vec = FeaMeshMgr.GetSimplePropertyVec();

    if ( property_index < simp_prop_vec.size() && property_index >= 0 && simp_prop_vec.size() > 0 )
    {
        avg_t = simp_prop_vec[property_index].m_Thickness;
        mat_index = simp_prop_vec[property_index].GetSimpFeaMatIndex();
    }

    double avg_d = 0;

    vector < SimpleFeaMaterial > simp_mat_vec = FeaMeshMgr.GetSimpleMaterialVec();

    if ( mat_index < simp_mat_vec.size() && mat_index >= 0 && simp_mat_vec.size() > 0 )
    {
        avg_d = simp_mat_vec[mat_index].m_MassDensity;
    }

    mass = a * avg_t * avg_d;
    return mass;
}

//////////////////////////////////////////////////////
//=================== FeaBeam ======================//
//////////////////////////////////////////////////////

void FeaBeam::Create( vec3d &p0, vec3d &p1, vec3d &norm0, vec3d &norm1 )
{
    m_ElementType = FEA_BEAM;
    DeleteAllNodes();

    m_Corners.push_back( new FeaNode( p0 ) );
    m_Corners.push_back( new FeaNode( p1 ) );

    vec3d p01 = ( p0 + p1 ) * 0.5;

    m_Mids.push_back( new FeaNode( p01 ) );

    m_Norm0 = norm0;
    m_Norm1 = norm1;
}

void FeaBeam::WriteCalculix( FILE* fp, int id, long long int noffset, long long int eoffset )
{
    fprintf( fp, "%lld,%lld,%lld,%lld\n", id + eoffset,
             m_Corners[0]->GetIndex() + noffset, m_Mids[0]->GetIndex() + noffset, m_Corners[1]->GetIndex() + noffset );

    m_ElementIndex = id; // Save element index 
}

void FeaBeam::WriteCalculixNormal( FILE* fp, long long int noffset, long long int eoffset )
{
    vec3d norm = ( m_Norm0 + m_Norm1 ) * 0.5;
    norm.normalize();

    fprintf( fp, "%lld,%lld,%f,%f,%f\n", m_ElementIndex + eoffset, m_Corners[0]->GetIndex() + noffset, m_Norm0.x(), m_Norm0.y(), m_Norm0.z() );
    fprintf( fp, "%lld,%lld,%f,%f,%f\n", m_ElementIndex + eoffset, m_Mids[0]->GetIndex() + noffset, norm.x(), norm.y(), norm.z() );
    fprintf( fp, "%lld,%lld,%f,%f,%f\n", m_ElementIndex + eoffset, m_Corners[1]->GetIndex() + noffset, m_Norm1.x(), m_Norm1.y(), m_Norm1.z() );
}

void FeaBeam::WriteCalculixNormal( FILE* fp )
{
    fprintf( fp, "%f,%f,%f\n", m_Norm0.x(), m_Norm0.y(), m_Norm0.z() );
}

void FeaBeam::WriteNASTRAN( FILE* fp, int id, int property_index, long long int noffset, long long int eoffset )
{
    string format_string = "CBAR    ,%8lld,%8d,%8lld,%8lld," + NasFmt( m_Norm0.x() ) + "," +
                           NasFmt( m_Norm0.y() ) + "," + NasFmt( m_Norm0.z() ) + "\n";

    fprintf( fp, format_string.c_str(), id + eoffset, property_index + 1, m_Corners[0]->GetIndex() + noffset,
             m_Corners[1]->GetIndex() + noffset, m_Norm0.x(), m_Norm0.y(), m_Norm0.z() );
}

void FeaBeam::WriteGmsh( FILE* fp, int id, int fea_part_index, long long int noffset, long long int eoffset )
{
    // 2 node line line (1)
    fprintf( fp, "%lld 1 1 %d %lld %lld\n", id + eoffset, fea_part_index,
             m_Corners[0]->GetIndex() + noffset, m_Corners[1]->GetIndex() + noffset );
}

double FeaBeam::ComputeMass( int property_index )
{
    double mass = 0.0;

    if ( m_Corners.size() < 2 )
    {
        return mass;
    }

    double length = dist( m_Corners[0]->m_Pnt, m_Corners[1]->m_Pnt );

    double area = 0;
    int mat_index = -1;

    vector < SimpleFeaProperty > simp_prop_vec = FeaMeshMgr.GetSimplePropertyVec();

    if ( property_index < simp_prop_vec.size() && property_index >= 0 && simp_prop_vec.size() > 0 )
    {
        area = simp_prop_vec[property_index].m_CrossSecArea;
        mat_index = simp_prop_vec[property_index].GetSimpFeaMatIndex();
    }

    double avg_d = 0;

    vector < SimpleFeaMaterial > simp_mat_vec = FeaMeshMgr.GetSimpleMaterialVec();

    if ( mat_index < simp_mat_vec.size() && mat_index >= 0 && simp_mat_vec.size() > 0 )
    {
        avg_d = simp_mat_vec[mat_index].m_MassDensity;
    }

    mass = length * area * avg_d;

    return mass;
}

//////////////////////////////////////////////////////
//================ FeaPointMass ====================//
//////////////////////////////////////////////////////

void FeaPointMass::Create( vec3d & p0, double mass )
{
    m_ElementType = FEA_POINT_MASS;

    DeleteAllNodes();

    m_Corners.push_back( new FeaNode( p0 ) );

    m_Mass = mass;
}

void FeaPointMass::WriteCalculix( FILE* fp, int id, long long int noffset, long long int eoffset )
{
    fprintf( fp, "%lld,%lld\n", id + eoffset, m_Corners[0]->GetIndex() + noffset );
}

void FeaPointMass::WriteNASTRAN( FILE* fp, int id, int property_index, long long int noffset, long long int eoffset )
{
    // Note: property_index ignored
    string format_string = "CONM2   ,%8lld,%8lld,        ," + NasFmt( m_Mass ) + "\n";

    fprintf( fp, format_string.c_str(), id + eoffset, m_Corners[0]->GetIndex() + noffset, m_Mass );
}

//////////////////////////////////////////////////////
//=============== SimpleFeaProperty ================//
//////////////////////////////////////////////////////
void SimpleFeaProperty::CopyFrom( FeaProperty* fea_prop, const vector < string > &mat_id_vec )
{
    if ( fea_prop )
    {
        m_Name = fea_prop->GetName();
        m_FeaPropertyType = fea_prop->m_FeaPropertyType.Get();
        m_Thickness = fea_prop->m_Thickness_FEM.Get();
        m_CrossSecArea = fea_prop->m_CrossSecArea_FEM.Get();
        m_Izz = fea_prop->m_Izz_FEM.Get();
        m_Iyy = fea_prop->m_Iyy_FEM.Get();
        m_Izy = fea_prop->m_Izy_FEM.Get();
        m_Ixx = fea_prop->m_Ixx_FEM.Get();
        m_Dim1 = fea_prop->m_Dim1_FEM.Get();
        m_Dim2 = fea_prop->m_Dim2_FEM.Get();
        m_Dim3 = fea_prop->m_Dim3_FEM.Get();
        m_Dim4 = fea_prop->m_Dim4_FEM.Get();
        m_Dim5 = fea_prop->m_Dim5_FEM.Get();
        m_Dim6 = fea_prop->m_Dim6_FEM.Get();
        m_CrossSectType = fea_prop->m_CrossSectType.Get();
        m_ID = fea_prop->GetID();
        m_FeaMatID = fea_prop->m_FeaMaterialID;
        m_Used = false;

        m_SimpleFeaMatIndex = vector_find_val( mat_id_vec, m_FeaMatID );

        FeaMaterial* fea_mat = StructureMgr.GetFeaMaterial( m_FeaMatID );

        if ( fea_mat )
        {
            m_MaterialName = fea_mat->GetName();
        }
    }
}

void SimpleFeaProperty::WriteNASTRAN( FILE* fp, int prop_id ) const
{
    if ( !m_Used )
    {
        return;
    }

    fprintf( fp, "$ %s using %s\n", m_Name.c_str(), m_MaterialName.c_str() );
    if ( m_FeaPropertyType == vsp::FEA_SHELL )
    {
        string format_string = "PSHELL  ,%8d,%8d," + NasFmt( m_Thickness ) + ",%8d,        ,%8d,        ,        ,\n        ,        ,        ,        \n";

        fprintf( fp, format_string.c_str(), prop_id, m_SimpleFeaMatIndex + 1, m_Thickness, m_SimpleFeaMatIndex + 1, m_SimpleFeaMatIndex + 1 );
    }
    if ( m_FeaPropertyType == vsp::FEA_BEAM )
    {
        if ( m_CrossSectType == vsp::FEA_XSEC_GENERAL )
        {
            string format_string = "PBAR    ,%8d,%8d," + NasFmt( m_CrossSecArea ) + "," + NasFmt( m_Izz ) + "," + NasFmt( m_Iyy ) + "," + NasFmt( m_Ixx )
                   + ",        ,        ,\n        ,        ,        ,        ,        ,        ,        ,        ,        ,\n        ,        ,        ,"
                   + NasFmt( m_Izy ) + "\n";

            fprintf( fp, format_string.c_str(), prop_id, m_SimpleFeaMatIndex + 1, m_CrossSecArea, m_Izz, m_Iyy, m_Ixx, m_Izy );
        }
        else if ( m_CrossSectType == vsp::FEA_XSEC_CIRC )
        {
            string format_string = "PBARL   ,%8d,%8d,        ,     ROD,\n        ," + NasFmt( m_Dim1 ) + "\n";

            fprintf( fp, format_string.c_str(), prop_id, m_SimpleFeaMatIndex + 1, m_Dim1 );
        }
        else if ( m_CrossSectType == vsp::FEA_XSEC_PIPE )
        {
            string format_string = "PBARL   ,%8d,%8d,        ,    TUBE,\n        ," + NasFmt( m_Dim1 ) + "," + NasFmt( m_Dim2 ) + "\n";

            fprintf( fp, format_string.c_str(), prop_id, m_SimpleFeaMatIndex + 1, m_Dim1, 2 * m_Dim2 );
        }
        else if ( m_CrossSectType == vsp::FEA_XSEC_I )
        {
            string format_string = "PBARL   ,%8d,%8d,        ,       I,\n        ," + NasFmt( m_Dim1 ) + "," + NasFmt( m_Dim2 ) + ","
                                  + NasFmt( m_Dim3 ) + "," + NasFmt( m_Dim4 ) + "," + NasFmt( m_Dim5 ) + "," + NasFmt( m_Dim6 ) + "\n";

            fprintf( fp, format_string.c_str(), prop_id, m_SimpleFeaMatIndex + 1, m_Dim1, m_Dim2, m_Dim3, m_Dim4, m_Dim5, m_Dim6 );
        }
        else if ( m_CrossSectType == vsp::FEA_XSEC_RECT )
        {
            string format_string = "PBARL   ,%8d,%8d,        ,     BAR,\n        ," + NasFmt( m_Dim1 ) + "," + NasFmt( m_Dim2 ) + "\n";

            fprintf( fp, format_string.c_str(), prop_id, m_SimpleFeaMatIndex + 1, m_Dim1, m_Dim2 );
        }
        else if ( m_CrossSectType == vsp::FEA_XSEC_BOX )
        {
            string format_string = "PBARL   ,%8d,%8d,        ,     BOX,\n        ," + NasFmt( m_Dim1 ) + "," + NasFmt( m_Dim2 ) + ","
                                   + NasFmt( m_Dim3 ) + "," + NasFmt( m_Dim4 ) + "\n";

            fprintf( fp, format_string.c_str(), prop_id, m_SimpleFeaMatIndex + 1, m_Dim1, m_Dim2, m_Dim3, m_Dim4 );
        }
    }
}

void SimpleFeaProperty::WriteCalculix( FILE* fp, const string &ELSET, const string &ORIENTATION ) const
{
    if ( !m_Used )
    {
        return;
    }

    string matname = m_MaterialName;
    change_space_to_underscore( matname );

    if ( m_FeaPropertyType == vsp::FEA_SHELL )
    {
        fprintf( fp, "*SHELL SECTION, ELSET=%s, MATERIAL=%s, ORIENTATION=%s\n", ELSET.c_str(), matname.c_str(), ORIENTATION.c_str() );
        fprintf( fp, "%g\n", m_Thickness );
    }
    if ( m_FeaPropertyType == vsp::FEA_BEAM )
    {
        if ( m_CrossSectType == vsp::FEA_XSEC_GENERAL )
        {
            // Note: *BEAM GENERAL SECTION is supported by Abaqus but not Calculix. Calculix depends on BEAM SECTION properties
            //  where the cross-section dimensions must be explicitly defined. 
            fprintf( fp, "*BEAM SECTION, SECTION=GENERAL, ELSET=%s, MATERIAL=%s\n", ELSET.c_str(), matname.c_str() );
            fprintf( fp, "%g,%g,%g,%g,%g\n", m_CrossSecArea, m_Izz, m_Izy, m_Iyy, m_Ixx );
        }
        else if ( m_CrossSectType == vsp::FEA_XSEC_CIRC )
        {
            fprintf( fp, "*BEAM SECTION, SECTION=CIRC, ELSET=%s, MATERIAL=%s\n", ELSET.c_str(), matname.c_str() );
            fprintf( fp, "%f\n", m_Dim1 );
        }
        else if ( m_CrossSectType == vsp::FEA_XSEC_PIPE )
        {
            fprintf( fp, "*BEAM SECTION, SECTION=PIPE, ELSET=%s, MATERIAL=%s\n", ELSET.c_str(), matname.c_str() );
            fprintf( fp, "%f,%f\n", m_Dim1, ( m_Dim1 - m_Dim2 ) ); 
        }
        else if ( m_CrossSectType == vsp::FEA_XSEC_I )
        {
            fprintf( fp, "*BEAM SECTION, SECTION=I, ELSET=%s, MATERIAL=%s\n", ELSET.c_str(), matname.c_str() );
            fprintf( fp, "%f,%f,%f,%f,%f,%f,%f\n", ( m_Dim1 / 2 ), m_Dim1, m_Dim2, m_Dim3, m_Dim5, m_Dim6, m_Dim4 );
        }
        else if ( m_CrossSectType == vsp::FEA_XSEC_RECT )
        {
            fprintf( fp, "*BEAM SECTION, SECTION=RECT, ELSET=%s, MATERIAL=%s\n", ELSET.c_str(), matname.c_str() );
            fprintf( fp, "%f,%f\n", m_Dim1, m_Dim2 );
        }
        else if ( m_CrossSectType == vsp::FEA_XSEC_BOX )
        {
            fprintf( fp, "*BEAM SECTION, SECTION=PIPE, ELSET=%s, MATERIAL=%s\n", ELSET.c_str(), matname.c_str() );
            fprintf( fp, "%f,%f,%f,%f,%f,%f\n", m_Dim1, m_Dim2, m_Dim4, m_Dim3, m_Dim4, m_Dim3 );
        }
    }
}

//////////////////////////////////////////////////////
//=============== SimpleFeaMaterial ================//
//////////////////////////////////////////////////////
void SimpleFeaMaterial::CopyFrom( FeaMaterial* fea_mat )
{
    if ( fea_mat )
    {
        m_FeaMaterialType = fea_mat->m_FeaMaterialType.Get();
        m_MassDensity = fea_mat->m_MassDensity_FEM.Get();
        m_ElasticModulus = fea_mat->m_ElasticModulus_FEM.Get();
        m_PoissonRatio = fea_mat->m_PoissonRatio.Get();
        m_ThermalExpanCoeff = fea_mat->m_ThermalExpanCoeff_FEM.Get();
        m_E1 = fea_mat->m_E1_FEM.Get();
        m_E2 = fea_mat->m_E2_FEM.Get();
        m_E3 = fea_mat->m_E3_FEM.Get();
        m_nu12 = fea_mat->m_nu12.Get();
        m_nu13 = fea_mat->m_nu13.Get();
        m_nu23 = fea_mat->m_nu23.Get();
        m_G12 = fea_mat->m_G12_FEM.Get();
        m_G13 = fea_mat->m_G13_FEM.Get();
        m_G23 = fea_mat->m_G23_FEM.Get();
        m_A1 = fea_mat->m_A1_FEM.Get();
        m_A2 = fea_mat->m_A2_FEM.Get();
        m_A3 = fea_mat->m_A3_FEM.Get();
        m_Name = fea_mat->GetName();
        m_Used = false;
        m_ID = fea_mat->GetID();
    }
}

void SimpleFeaMaterial::WriteNASTRAN( FILE* fp, int mat_id ) const
{
    if ( !m_Used )
    {
        return;
    }

    if ( m_FeaMaterialType == vsp::FEA_ISOTROPIC )
    {
        fprintf( fp, "$ %s\n", m_Name.c_str() );
        string fmt = "MAT1    ,%8d," + NasFmt( m_ElasticModulus ) + "," +
                                       NasFmt( GetShearModulus() ) + "," +
                                       NasFmt( m_PoissonRatio ) + "," +
                                       NasFmt( m_MassDensity ) + "," +
                                       NasFmt( m_ThermalExpanCoeff ) + "\n";

        fprintf( fp, fmt.c_str(), mat_id, m_ElasticModulus, GetShearModulus(), m_PoissonRatio, m_MassDensity, m_ThermalExpanCoeff );
    }
    else // vsp::FEA_ENG_ORTHO
    {
        fprintf( fp, "$ %s\n", m_Name.c_str() );
        // Note that MAT8 is only for shell type elements.
        string fmt = "MAT8    ,%8d," + NasFmt( m_E1 ) + "," +
                                       NasFmt( m_E2 ) + "," +
                                       NasFmt( m_nu12 ) + "," +
                                       NasFmt( m_G12 ) + "," +
                                       NasFmt( m_G13 ) + "," +
                                       NasFmt( m_G23 ) + "," +
                                       NasFmt( m_MassDensity ) + ",\n        ," +
                                       NasFmt( m_A1 ) + "," +
                                       NasFmt( m_A2 ) + "\n";

        fprintf( fp, fmt.c_str(), mat_id, m_E1, m_E2, m_nu12, m_G12, m_G13, m_G23, m_MassDensity, m_A1, m_A2  );
        // If solid elements are ever added, they will need MAT9 for isotropic materials.
    }
    fprintf( fp, "\n" );
}

void SimpleFeaMaterial::WriteCalculix( FILE* fp, int mat_id ) const
{
    if ( !m_Used )
    {
        return;
    }

    string matname = m_Name;
    change_space_to_underscore( matname );

    if ( m_FeaMaterialType == vsp::FEA_ISOTROPIC )
    {
        fprintf( fp, "*MATERIAL, NAME=%s\n", matname.c_str() );
        fprintf( fp, "*DENSITY\n" );
        fprintf( fp, "%g\n", m_MassDensity );
        fprintf( fp, "*ELASTIC, TYPE=ISO\n" );
        fprintf( fp, "%g, %g\n", m_ElasticModulus, m_PoissonRatio );
        fprintf( fp, "*EXPANSION, TYPE=ISO\n" );
        fprintf( fp, "%g\n", m_ThermalExpanCoeff );
    }
    else // vsp::FEA_ENG_ORTHO
    {
        fprintf( fp, "*MATERIAL, NAME=%s\n", matname.c_str() );
        fprintf( fp, "*DENSITY\n" );
        fprintf( fp, "%g\n", m_MassDensity );
        fprintf( fp, "*ELASTIC, TYPE=ENGINEERING CONSTANTS\n" );
        fprintf( fp, "%g, %g, %g, %g, %g, %g, %g, %g\n", m_E1, m_E2, m_E3, m_nu12, m_nu13, m_nu23, m_G12, m_G13 );
        fprintf( fp, "%g\n", m_G23 );
        fprintf( fp, "*EXPANSION, TYPE=ORTHO\n" );
        fprintf( fp, "%g, %g, %g\n", m_A1, m_A2, m_A3 );
    }
    fprintf( fp, "\n" );
}

double SimpleFeaMaterial::GetShearModulus() const
{
    return ( m_ElasticModulus / ( 2.0 * ( m_PoissonRatio + 1.0 ) ) );
}
