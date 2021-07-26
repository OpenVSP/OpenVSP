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

string GetFeaFormat( double input )
{
    if ( std::abs( input ) < 10.0 )
    {
        return string( "%8.5f" );
    }
    else if ( std::abs( input ) < 100.0 )
    {
        return string( "%8.4f" );
    }
    else if ( std::abs( input ) < 10000.0 )
    {
        return string( "%8.3f" );
    }
    else
    {
        return string( "%8.3g" );
    }
}

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
    fprintf( fp, "GRID,%8d,        ,", m_Index );

    double x = m_Pnt.x();
    double y = m_Pnt.y();
    double z = m_Pnt.z();

    if ( std::abs( x ) < 10.0 )
    {
        fprintf( fp, "%8.5f,", x );
    }
    else if ( std::abs( x ) < 100.0 )
    {
        fprintf( fp, "%8.4f,", x );
    }
    else
    {
        fprintf( fp, "%8.3f,", x );
    }

    if ( std::abs( y ) < 10.0 )
    {
        fprintf( fp, "%8.5f,", y );
    }
    else if ( std::abs( y ) < 100.0 )
    {
        fprintf( fp, "%8.4f,", y );
    }
    else
    {
        fprintf( fp, "%8.3f,", y );
    }

    if ( std::abs( z ) < 10.0 )
    {
        fprintf( fp, "%8.5f\n", z );
    }
    else if ( std::abs( z ) < 100.0 )
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
    fprintf( fp, "%d %f %f %f\n", m_Index, m_Pnt.x(), m_Pnt.y(), m_Pnt.z() );
}

//////////////////////////////////////////////////////
//================== FeaElement ====================//
//////////////////////////////////////////////////////

FeaElement::FeaElement()
{
    m_ElementType = FEA_TRI_6;
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

    string format_string = "CTRIA6,%8d,%8d,%8d,%8d,%8d,%8d,%8d,%8d,\n      ," + GetFeaFormat( theta_material ) + "\n";

    fprintf( fp, format_string.c_str(), id, property_index + 1,
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
    fprintf( fp, "CQUAD8,%8d,%8d,%8d,%8d,%8d,%8d,%8d,%8d,+\n+,%8d,%8d\n", id, property_index + 1,
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
    fprintf( fp, "%d,%d,%d,%d\n", id,
             m_Corners[0]->GetIndex(), m_Mids[0]->GetIndex(), m_Corners[1]->GetIndex() );

    m_ElementIndex = id; // Save element index 
}

void FeaBeam::WriteCalculixNormal( FILE* fp )
{
    string format_string = "%8d,%8d," + GetFeaFormat( m_DispVec.x() ) + "," + GetFeaFormat( m_DispVec.y() ) + "," + GetFeaFormat( m_DispVec.z() ) + "\n";
    fprintf( fp, format_string.c_str(), m_ElementIndex, m_Corners[0]->GetIndex(), m_DispVec.x(), m_DispVec.y(), m_DispVec.z() );
}

void FeaBeam::WriteNASTRAN( FILE* fp, int id, int property_index )
{
    string format_string = "CBAR,%8d,%8d,%8d,%8d," + GetFeaFormat( m_DispVec.x() ) + "," +
        GetFeaFormat( m_DispVec.y() ) + "," + GetFeaFormat( m_DispVec.z() ) + "\n";

    fprintf( fp, format_string.c_str(), id, property_index + 1, m_Corners[0]->GetIndex(), 
             m_Corners[1]->GetIndex(), m_DispVec.x(), m_DispVec.y(), m_DispVec.z() );
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
    string format_string = "CONM2,%8d,%8d,        ," + GetFeaFormat( m_Mass ) + "\n";

    fprintf( fp, format_string.c_str(), id, m_Corners[0]->GetIndex(), m_Mass );
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
        m_Dim1 = fea_prop->m_Dim1.Get();
        m_Dim2 = fea_prop->m_Dim2.Get();
        m_Dim3 = fea_prop->m_Dim3.Get();
        m_Dim4 = fea_prop->m_Dim4.Get();
        m_Dim5 = fea_prop->m_Dim5.Get();
        m_Dim6 = fea_prop->m_Dim6.Get();
        m_CrossSectType = fea_prop->m_CrossSectType.Get();
        m_SimpleFeaMatIndex = fea_prop->m_FeaMaterialIndex();

        FeaMaterial* fea_mat = StructureMgr.GetFeaMaterial( m_SimpleFeaMatIndex );

        if ( fea_mat )
        {
            m_MaterialName = fea_mat->GetName();
        }
    }
}

void SimpleFeaProperty::WriteNASTRAN( FILE* fp, int prop_id )
{
    if ( m_FeaPropertyType == vsp::FEA_SHELL )
    {
        string format_string = "PSHELL,%8d,%8d," + GetFeaFormat( m_Thickness ) + ",      -1,        ,        ,        ,        ,\n     ,        ,        ,        \n";

        // Note: For plane strain analysis, material identification number for bending is set to -1
        fprintf( fp, format_string.c_str(), prop_id, m_SimpleFeaMatIndex + 1, m_Thickness );
    }
    if ( m_FeaPropertyType == vsp::FEA_BEAM )
    {
        if ( m_CrossSectType == vsp::FEA_XSEC_GENERAL )
        {
            string format_string = "PBAR,%8d,%8d," + GetFeaFormat( m_CrossSecArea ) + "," +
                GetFeaFormat( m_Izz ) + "," + GetFeaFormat( m_Iyy ) + "," + GetFeaFormat( m_Ixx ) +
                ",        ,        ,\n    ,        ,        ,        ,        ,        ,        ,        ,        ,\n    ,        ,        ," +
                GetFeaFormat( m_Izy ) + "\n";

            fprintf( fp, format_string.c_str(), prop_id, m_SimpleFeaMatIndex + 1, m_CrossSecArea, m_Izz, m_Iyy, m_Ixx, m_Izy );
        }
        else if ( m_CrossSectType == vsp::FEA_XSEC_CIRC )
        {
            string format_string = "PBARL,%8d,%8d,        ,     ROD,\n     ," + GetFeaFormat( m_Dim1 ) + "\n";

            fprintf( fp, format_string.c_str(), prop_id, m_SimpleFeaMatIndex + 1, m_Dim1 );
        }
        else if ( m_CrossSectType == vsp::FEA_XSEC_PIPE )
        {
            string format_string = "PBARL,%8d,%8d,        ,    TUBE,\n     ," + GetFeaFormat( m_Dim1 ) + "," + GetFeaFormat( m_Dim2 ) + "\n";

            fprintf( fp, format_string.c_str(), prop_id, m_SimpleFeaMatIndex + 1, m_Dim1, 2 * m_Dim2 );
        }
        else if ( m_CrossSectType == vsp::FEA_XSEC_I )
        {
            string format_string = "PBARL,%8d,%8d,        ,       I,\n     ," + GetFeaFormat( m_Dim1 ) + "," + GetFeaFormat( m_Dim2 ) + "," 
                + GetFeaFormat( m_Dim3 ) + "," + GetFeaFormat( m_Dim4 ) + "," + GetFeaFormat( m_Dim5 ) + "," + GetFeaFormat( m_Dim6 ) + "\n";

            fprintf( fp, format_string.c_str(), prop_id, m_SimpleFeaMatIndex + 1, m_Dim1, m_Dim2, m_Dim3, m_Dim4, m_Dim5, m_Dim6 );
        }
        else if ( m_CrossSectType == vsp::FEA_XSEC_RECT )
        {
            string format_string = "PBARL,%8d,%8d,        ,     BAR,\n     ," + GetFeaFormat( m_Dim1 ) + "," + GetFeaFormat( m_Dim2 ) + "\n";

            fprintf( fp, format_string.c_str(), prop_id, m_SimpleFeaMatIndex + 1, m_Dim1, m_Dim2 );
        }
        else if ( m_CrossSectType == vsp::FEA_XSEC_BOX )
        {
            string format_string = "PBARL,%8d,%8d,        ,     BOX,\n     ," + GetFeaFormat( m_Dim1 ) + "," + GetFeaFormat( m_Dim2 ) + ","
                + GetFeaFormat( m_Dim3 ) + "," + GetFeaFormat( m_Dim4 ) + "\n";

            fprintf( fp, format_string.c_str(), prop_id, m_SimpleFeaMatIndex + 1, m_Dim1, m_Dim2, m_Dim3, m_Dim4 );
        }
    }
}

void SimpleFeaProperty::WriteCalculix( FILE* fp, const string &ELSET )
{
    if ( m_FeaPropertyType == vsp::FEA_SHELL )
    {
        fprintf( fp, "*SHELL SECTION, ELSET=%s, MATERIAL=%s\n", ELSET.c_str(), m_MaterialName.c_str() );
        fprintf( fp, "%g\n", m_Thickness );
    }
    if ( m_FeaPropertyType == vsp::FEA_BEAM )
    {
        if ( m_CrossSectType == vsp::FEA_XSEC_GENERAL )
        {
            // Note: *BEAM GENERAL SECTION is supported by Abaqus but not Calculix. Calculix depends on BEAM SECTION properties
            //  where the cross-section dimensions must be explicitly defined. 
            fprintf( fp, "*BEAM GENERAL SECTION, SECTION=GENERAL, ELSET=%s, MATERIAL=%s\n", ELSET.c_str(), m_MaterialName.c_str() );
            fprintf( fp, "%g,%g,%g,%g,%g\n", m_CrossSecArea, m_Izz, m_Izy, m_Iyy, m_Ixx );
        }
        else if ( m_CrossSectType == vsp::FEA_XSEC_CIRC )
        {
            fprintf( fp, "*BEAM SECTION, SECTION=CIRC, ELSET=%s, MATERIAL=%s\n", ELSET.c_str(), m_MaterialName.c_str() );
            fprintf( fp, "%f\n", m_Dim1 );
        }
        else if ( m_CrossSectType == vsp::FEA_XSEC_PIPE )
        {
            fprintf( fp, "*BEAM SECTION, SECTION=PIPE, ELSET=%s, MATERIAL=%s\n", ELSET.c_str(), m_MaterialName.c_str() );
            fprintf( fp, "%f,%f\n", m_Dim1, ( m_Dim1 - m_Dim2 ) ); 
        }
        else if ( m_CrossSectType == vsp::FEA_XSEC_I )
        {
            fprintf( fp, "*BEAM SECTION, SECTION=I, ELSET=%s, MATERIAL=%s\n", ELSET.c_str(), m_MaterialName.c_str() );
            fprintf( fp, "%f,%f,%f,%f,%f,%f,%f\n", ( m_Dim1 / 2 ), m_Dim1, m_Dim2, m_Dim3, m_Dim5, m_Dim6, m_Dim4 );
        }
        else if ( m_CrossSectType == vsp::FEA_XSEC_RECT )
        {
            fprintf( fp, "*BEAM SECTION, SECTION=RECT, ELSET=%s, MATERIAL=%s\n", ELSET.c_str(), m_MaterialName.c_str() );
            fprintf( fp, "%f,%f\n", m_Dim1, m_Dim2 );
        }
        else if ( m_CrossSectType == vsp::FEA_XSEC_BOX )
        {
            fprintf( fp, "*BEAM SECTION, SECTION=PIPE, ELSET=%s, MATERIAL=%s\n", ELSET.c_str(), m_MaterialName.c_str() );
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
        m_MassDensity = fea_mat->m_MassDensity.Get();
        m_ElasticModulus = fea_mat->m_ElasticModulus.Get();
        m_PoissonRatio = fea_mat->m_PoissonRatio.Get();
        m_ThermalExpanCoeff = fea_mat->m_ThermalExpanCoeff.Get();
        m_Name = fea_mat->GetName();
    }
}

void SimpleFeaMaterial::WriteNASTRAN( FILE* fp, int mat_id )
{
    fprintf( fp, "MAT1,%8d,%8.3g,%8.3g,%8.5g,%8.3g,%8.3g\n", mat_id, m_ElasticModulus, GetShearModulus(), m_PoissonRatio, m_MassDensity, m_ThermalExpanCoeff );
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
