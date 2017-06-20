//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// FeaElement.cpp
//
// Justin Gravett
//////////////////////////////////////////////////////////////////////

#include "FeaElement.h"
#include "StructureMgr.h"
#include "FeaMeshMgr.h"

//////////////////////////////////////////////////////
//==================== FeaNode =====================//
//////////////////////////////////////////////////////

void FeaNode::AddTag( int index )
{
    //==== Check For Duplicate Tags ====//
    for ( int i = 0; i < (int)m_Tags.size(); i++ )
    {
        if ( m_Tags[i].m_FeaPartTagIndex == index )
        {
            return;
        }
    }

    FeaNodeTag tag;
    tag.m_FeaPartTagIndex = index;
    m_Tags.push_back( tag );
}

bool FeaNode::HasTag( int index )
{
    for ( int i = 0; i < (int)m_Tags.size(); i++ )
    {
        if ( m_Tags[i].m_FeaPartTagIndex == index )
        {
            return true;
        }
    }
    return false;
}

bool FeaNode::HasOnlyIndex( int index )
{
    for ( int i = 0; i < (int)m_Tags.size(); i++ )
    {
        if ( m_Tags[i].m_FeaPartTagIndex != index )
        {
            return false;
        }
    }
    return true;
}

int FeaNode::GetIndex()
{
    return m_Index;
}

void FeaNode::WriteNASTRAN( FILE* fp )
{
    fprintf( fp, "GRID,%d,,", m_Index );

    double x = m_Pnt.x();
    double y = m_Pnt.y();
    double z = m_Pnt.z();

    if ( fabs( x ) < 10.0 )
    {
        fprintf( fp, "%8.5f,", x );
    }
    else if ( fabs( x ) < 100.0 )
    {
        fprintf( fp, "%8.4f,", x );
    }
    else
    {
        fprintf( fp, "%8.3f,", x );
    }

    if ( fabs( y ) < 10.0 )
    {
        fprintf( fp, "%8.5f,", y );
    }
    else if ( fabs( y ) < 100.0 )
    {
        fprintf( fp, "%8.4f,", y );
    }
    else
    {
        fprintf( fp, "%8.3f,", y );
    }

    if ( fabs( z ) < 10.0 )
    {
        fprintf( fp, "%8.5f\n", z );
    }
    else if ( fabs( z ) < 100.0 )
    {
        fprintf( fp, "%8.4f\n", z );
    }
    else
    {
        fprintf( fp, "%8.3f\n", z );
    }

}

void FeaNode::WriteCalculix( FILE* fp )
{
    fprintf( fp, "%d,%f,%f,%f\n", m_Index, m_Pnt.x(), m_Pnt.y(), m_Pnt.z() );
}

void FeaNode::WriteGmsh( FILE* fp )
{
    fprintf( fp, "%d %16.10f %16.10f %16.10f\n", m_Index, m_Pnt.x(), m_Pnt.y(), m_Pnt.z() );
}

//////////////////////////////////////////////////////
//================== FeaElement ====================//
//////////////////////////////////////////////////////

FeaElement::FeaElement()
{
    m_FeaPartIndex = -1;
    m_FeaSSIndex = -1;
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

void FeaTri::Create( vec3d & p0, vec3d & p1, vec3d & p2, vec3d & orientation )
{
    m_ElementType = FEA_TRI_6;
    DeleteAllNodes();
    m_Corners.push_back( new FeaNode( p0 ) );
    m_Corners.push_back( new FeaNode( p1 ) );
    m_Corners.push_back( new FeaNode( p2 ) );

    vec3d p01 = ( p0 + p1 ) * 0.5;
    vec3d p12 = ( p1 + p2 ) * 0.5;
    vec3d p20 = ( p2 + p0 ) * 0.5;

    m_Mids.push_back( new FeaNode( p01 ) );
    m_Mids.push_back( new FeaNode( p12 ) );
    m_Mids.push_back( new FeaNode( p20 ) );

    m_Orientation = orientation;
}

void FeaTri::WriteCalculix( FILE* fp, int id )
{
    fprintf( fp, "%d,%d,%d,%d,%d,%d,%d\n", id,
             m_Corners[0]->GetIndex(), m_Corners[1]->GetIndex(), m_Corners[2]->GetIndex(),
             m_Mids[0]->GetIndex(), m_Mids[1]->GetIndex(), m_Mids[2]->GetIndex() );
}

void FeaTri::WriteNASTRAN( FILE* fp, int id, int property_index )
{
    vec3d x_element = m_Corners[1]->m_Pnt - m_Corners[0]->m_Pnt;
    x_element.normalize();
    vec3d x_axis = vec3d( 1.0, 0.0, 0.0 );

    double theta_material = RAD_2_DEG * signed_angle( x_element, m_Orientation, x_axis );

    fprintf( fp, "CTRIA6,%d,%d,%d,%d,%d,%d,%d,%d,\n,%8.3f,,,,,,,\n", id, property_index + 1,
             m_Corners[0]->GetIndex(), m_Corners[1]->GetIndex(), m_Corners[2]->GetIndex(),
             m_Mids[0]->GetIndex(), m_Mids[1]->GetIndex(), m_Mids[2]->GetIndex(), theta_material );
}

void FeaTri::WriteGmsh( FILE* fp, int id, int fea_part_index )
{
    // 6-node second order triangle element type (9)
    fprintf( fp, "%d 9 1 %d %d %d %d %d %d %d\n", id, fea_part_index,
             m_Corners[0]->GetIndex(), m_Corners[1]->GetIndex(), m_Corners[2]->GetIndex(),
             m_Mids[0]->GetIndex(),m_Mids[1]->GetIndex(), m_Mids[2]->GetIndex() );
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

    //double avg_t = ( m_Corners[0]->m_Thick + m_Corners[1]->m_Thick + m_Corners[2]->m_Thick ) / 3.0;
    //double avg_d = ( m_Corners[0]->m_Dense + m_Corners[1]->m_Dense + m_Corners[2]->m_Dense ) / 3.0;

    mass = a * avg_t * avg_d;
    return mass;
}

//////////////////////////////////////////////////////
//=================== FeaQuad ======================//
//////////////////////////////////////////////////////

void FeaQuad::Create( vec3d & p0, vec3d & p1, vec3d & p2, vec3d & p3 )
{
    m_ElementType = FEA_QUAD_8;
    DeleteAllNodes();
    m_Corners.push_back( new FeaNode( p0 ) );
    m_Corners.push_back( new FeaNode( p1 ) );
    m_Corners.push_back( new FeaNode( p2 ) );
    m_Corners.push_back( new FeaNode( p3 ) );

    vec3d p01 = ( p0 + p1 ) * 0.5;
    vec3d p12 = ( p1 + p2 ) * 0.5;
    vec3d p23 = ( p2 + p3 ) * 0.5;
    vec3d p30 = ( p3 + p0 ) * 0.5;

    m_Mids.push_back( new FeaNode( p01 ) );
    m_Mids.push_back( new FeaNode( p12 ) );
    m_Mids.push_back( new FeaNode( p23 ) );
    m_Mids.push_back( new FeaNode( p30 ) );
}

void FeaQuad::WriteCalculix( FILE* fp, int id )
{
    fprintf( fp, "%d,%d,%d,%d,%d,%d,%d,%d,%d\n", id,
             m_Corners[0]->GetIndex(), m_Corners[1]->GetIndex(), m_Corners[2]->GetIndex(), m_Corners[3]->GetIndex(),
             m_Mids[0]->GetIndex(), m_Mids[1]->GetIndex(), m_Mids[2]->GetIndex(), m_Mids[3]->GetIndex() );
}
void FeaQuad::WriteNASTRAN( FILE* fp, int id, int property_index )
{
    fprintf( fp, "CQUAD8,%d,%d,%d,%d,%d,%d,%d,%d,+\n+,%d,%d\n", id, property_index + 1,
             m_Corners[0]->GetIndex(), m_Corners[1]->GetIndex(), m_Corners[2]->GetIndex(), m_Corners[3]->GetIndex(),
             m_Mids[0]->GetIndex(), m_Mids[1]->GetIndex(), m_Mids[2]->GetIndex(), m_Mids[3]->GetIndex() );
}

void FeaQuad::WriteGmsh( FILE* fp, int id, int fea_part_index )
{
    // 8-node second order quadrangle element type (16)
    fprintf( fp, "%d 16 1 %d %d %d %d %d %d %d %d %d\n", id, fea_part_index,
             m_Corners[0]->GetIndex(), m_Corners[1]->GetIndex(), m_Corners[2]->GetIndex(), m_Corners[3]->GetIndex(),
             m_Mids[0]->GetIndex(), m_Mids[1]->GetIndex(), m_Mids[2]->GetIndex(), m_Mids[3]->GetIndex() );
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

    //double avg_t = ( m_Corners[0]->m_Thick + m_Corners[1]->m_Thick +
    //                 m_Corners[2]->m_Thick + m_Corners[3]->m_Thick ) / 4.0;

    //double avg_d = ( m_Corners[0]->m_Dense + m_Corners[1]->m_Dense +
    //                 m_Corners[2]->m_Dense + m_Corners[3]->m_Dense ) / 4.0;

    mass = a * avg_t * avg_d;
    return mass;
}

//////////////////////////////////////////////////////
//=================== FeaBeam ======================//
//////////////////////////////////////////////////////

void FeaBeam::Create( vec3d & p0, vec3d & p1, vec3d & norm )
{
    m_ElementType = FEA_BEAM;
    DeleteAllNodes();

    m_Corners.push_back( new FeaNode( p0 ) );
    m_Corners.push_back( new FeaNode( p1 ) );

    vec3d p01 = ( p0 + p1 ) * 0.5;

    m_Mids.push_back( new FeaNode( p01 ) );

    m_DispVec = norm;
}

void FeaBeam::WriteCalculix( FILE* fp, int id )
{
    fprintf( fp, "%d,%d,%d\n", id,
             m_Corners[0]->GetIndex(), m_Corners[1]->GetIndex() );

    m_ElementIndex = id; // Save element index 
}

void FeaBeam::WriteCalculixNormal( FILE* fp )
{
    fprintf( fp, "%d,%d,%8.5f,%8.5f,%8.5f\n", m_ElementIndex, m_Corners[0]->GetIndex(), m_DispVec.x(), m_DispVec.y(), m_DispVec.z() );
}

void FeaBeam::WriteNASTRAN( FILE* fp, int id, int property_index )
{
    fprintf( fp, "CBEAM,%d,%d,%d,%d,%f,%f,%f\n", id, property_index + 1,
             m_Corners[0]->GetIndex(), m_Corners[1]->GetIndex(),
             m_DispVec.x(), m_DispVec.y(), m_DispVec.z() );
}
void FeaBeam::WriteGmsh( FILE* fp, int id, int fea_part_index )
{
    // 2 node line line (1)
    fprintf( fp, "%d 1 1 %d %d %d\n", id, fea_part_index,
             m_Corners[0]->GetIndex(), m_Corners[1]->GetIndex() );
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

void FeaPointMass::WriteCalculix( FILE* fp, int id )
{
    fprintf( fp, "%d,%d\n", id, m_Corners[0]->GetIndex() );
}

void FeaPointMass::WriteNASTRAN( FILE* fp, int id, int property_index )
{
    // Note: property_index ignored

    fprintf( fp, "CONM2,%d,%d, ,%f\n", id, m_Corners[0]->GetIndex(), m_Mass );
}

//////////////////////////////////////////////////////
//=============== SimpleFeaProperty ================//
//////////////////////////////////////////////////////
void SimpleFeaProperty::CopyFrom( FeaProperty* fea_prop )
{
    if ( fea_prop )
    {
        m_FeaPropertyType = fea_prop->m_FeaPropertyType.Get();
        m_Thickness = fea_prop->m_Thickness.Get();
        m_CrossSecArea = fea_prop->m_CrossSecArea.Get();
        m_Izz = fea_prop->m_Izz.Get();
        m_Iyy = fea_prop->m_Iyy.Get();
        m_Izy = fea_prop->m_Izy.Get();
        m_Ixx = fea_prop->m_Ixx.Get();
        m_SimpleFeaMatIndex = fea_prop->GetFeaMaterialIndex();

        FeaMaterial* fea_mat = StructureMgr.GetFeaMaterial( m_SimpleFeaMatIndex );

        if ( fea_mat )
        {
            m_MaterialName = fea_mat->GetName();
        }
    }
}

void SimpleFeaProperty::WriteNASTRAN( FILE* fp, int prop_id )
{
    if ( m_FeaPropertyType == SHELL_PROPERTY )
    {
        fprintf( fp, "PSHELL,%d,%d,%f\n", prop_id, m_SimpleFeaMatIndex + 1, m_Thickness );
    }
    if ( m_FeaPropertyType == BEAM_PROPERTY )
    {
        fprintf( fp, "PBEAM,%d,%d,%f,%f,%f,%f,%f\n", prop_id, m_SimpleFeaMatIndex + 1, m_CrossSecArea, m_Izz, m_Iyy, m_Izy, m_Ixx );
    }
}

void SimpleFeaProperty::WriteCalculix( FILE* fp, string ELSET )
{
    if ( m_FeaPropertyType == SHELL_PROPERTY )
    {
        fprintf( fp, "*SHELL SECTION, ELSET=%s, MATERIAL=%s\n", ELSET.c_str(), m_MaterialName.c_str() );
        fprintf( fp, "%g\n", m_Thickness );
    }
    if ( m_FeaPropertyType == BEAM_PROPERTY )
    {
        // Note: *BEAM GENERAL SECTION is supported by Abaqus but not Calculix. Calculix depends on BEAM SECTION properties
        //  where the cross-section dimensions must be explicitly defined. 
        fprintf( fp, "*BEAM GENERAL SECTION, SECTION=GENERAL, ELSET=%s, MATERIAL=%s\n", ELSET.c_str(), m_MaterialName.c_str() );
        fprintf( fp, "%g,%g,%g,%g,%g\n", m_CrossSecArea, m_Izz, m_Izy, m_Iyy, m_Ixx );
    }
}

//////////////////////////////////////////////////////
//=============== SimpleFeaMaterial ================//
//////////////////////////////////////////////////////
void SimpleFeaMaterial::CopyFrom( FeaMaterial* fea_mat )
{
    if ( fea_mat )
    {
        m_MassDensity = fea_mat->m_MassDensity.Get();
        m_ElasticModulus = fea_mat->m_ElasticModulus.Get();
        m_PoissonRatio = fea_mat->m_PoissonRatio.Get();
        m_ThermalExpanCoeff = fea_mat->m_ThermalExpanCoeff.Get();
        m_Name = fea_mat->GetName();
    }
}

void SimpleFeaMaterial::WriteNASTRAN( FILE* fp, int mat_id )
{
    fprintf( fp, "MAT1,%d,%g,%g,%g,%g,%g\n", mat_id, m_ElasticModulus, GetShearModulus(), m_PoissonRatio, m_MassDensity, m_ThermalExpanCoeff );
}

void SimpleFeaMaterial::WriteCalculix( FILE* fp, int mat_id )
{
    fprintf( fp, "*MATERIAL, NAME=%s\n", m_Name.c_str() );
    fprintf( fp, "*DENSITY\n" );
    fprintf( fp, "%g\n", m_MassDensity );
    fprintf( fp, "*ELASTIC, TYPE=ISO\n" );
    fprintf( fp, "%g,%g\n", m_ElasticModulus, m_PoissonRatio );
    fprintf( fp, "*EXPANSION, TYPE=ISO\n" );
    fprintf( fp, "%g\n", m_ThermalExpanCoeff );
}

double SimpleFeaMaterial::GetShearModulus()
{
    return ( m_ElasticModulus / ( 2 * ( m_PoissonRatio + 1 ) ) );
}
