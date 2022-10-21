//
// Created by Rob McDonald on 9/13/22.
//

#include "FeaMesh.h"
#include "main.h"  // For version numbers
#include "StructureMgr.h"
#include "FeaMeshMgr.h"

FeaMesh::FeaMesh( string & struct_id )
{
    m_ID = struct_id;

    m_TotalMass = 0;

    m_NumFeaParts = 0;
    m_NumFeaSubSurfs = 0;
    m_NumFeaFixPoints = 0;

    m_NumEls = 0;
    m_NumTris = 0;
    m_NumQuads = 0;
    m_NumBeams = 0;

    m_MeshDOUpToDate = false;
    m_MeshReady = false;
}

FeaMesh::~FeaMesh()
{
    Cleanup();
}

void FeaMesh::Cleanup()
{
    //==== Delete Old Elements ====//
    for ( unsigned int i = 0; i < m_FeaElementVec.size(); i++ )
    {
        m_FeaElementVec[i]->DeleteAllNodes();
        delete m_FeaElementVec[i];
    }
    m_FeaElementVec.clear();
    m_FeaNodeVec.clear();

    for ( unsigned int i = 0; i < m_AllPntVec.size(); i++ )
    {
        delete m_AllPntVec[i];
    }
    m_AllPntVec.clear();

    m_SimpleSubSurfaceVec.clear();
    m_PartSurfOrientation.clear();

    m_FixPntVec.clear();

    m_IndMap.clear();
    m_PntShift.clear();

    m_StructName = "";

    m_TotalMass = 0;

    m_MeshReady = false;

    m_NumFeaParts = 0;
    m_NumFeaSubSurfs = 0;
    m_NumFeaFixPoints = 0;

    m_FeaPartNameVec.clear();
    m_FeaPartTypeVec.clear();
    m_FeaPartNumSurfVec.clear();
    m_FeaPartIncludedElementsVec.clear();
    m_FeaPartPropertyIndexVec.clear();
    m_FeaPartCapPropertyIndexVec.clear();

    m_NumEls = 0;
    m_NumTris = 0;
    m_NumQuads = 0;
    m_NumBeams = 0;

    m_DrawBrowserNameVec.clear();
    m_DrawBrowserPartIndexVec.clear();
    m_DrawElementFlagVec.clear();
    m_FixPointFeaPartFlagVec.clear();
    m_DrawCapFlagVec.clear();

    m_MeshDOUpToDate = false;
    m_FeaTriElementDO.clear();
    m_FeaQuadElementDO.clear();
    m_CapFeaElementDO.clear();
    m_FeaNodeDO.clear();
    m_ElOrientationDO.clear();
    m_CapNormDO.clear();
    m_SSTriElementDO.clear();
    m_SSQuadElementDO.clear();
    m_SSCapFeaElementDO.clear();
    m_SSFeaNodeDO.clear();
    m_SSElOrientationDO.clear();
    m_SSCapNormDO.clear();
}


void FeaMesh::SetDrawElementFlag( int index, bool flag )
{
    if ( index >= 0 && index < m_DrawElementFlagVec.size() && m_DrawElementFlagVec.size() > 0 )
    {
        m_DrawElementFlagVec[index] = flag;
    }
}

void FeaMesh::SetDrawCapFlag( int index, bool flag )
{
    if ( index >= 0 && index < m_DrawCapFlagVec.size() && m_DrawCapFlagVec.size() > 0 )
    {
        m_DrawCapFlagVec[index] = flag;
    }
}

bool FeaMesh::FeaDataAvailable()
{
    return m_FeaElementVec.size() > 0;
}

void FeaMesh::SetAllDisplayFlags( bool flag )
{
    for ( unsigned int i = 0; i < m_DrawElementFlagVec.size(); i++ )
    {
        SetDrawElementFlag( i, flag );
    }

    for ( unsigned int i = 0; i < m_DrawCapFlagVec.size(); i++ )
    {
        SetDrawCapFlag( i, flag );
    }
}

void FeaMesh::UpdateDrawObjs()
{
    if ( !m_MeshReady )
    {
        return;
    }

    m_MeshDOUpToDate = true;
    // FeaParts:
    m_FeaNodeDO.resize( m_NumFeaParts );
    m_FeaTriElementDO.resize( m_NumFeaParts );
    m_FeaQuadElementDO.resize( m_NumFeaParts );
    m_CapFeaElementDO.resize( m_NumFeaParts );
    m_ElOrientationDO.resize( m_NumFeaParts );
    m_CapNormDO.resize( m_NumFeaParts );

    double line_length = GetGridDensityPtr()->m_MinLen / 3.0; // Length of orientation vectors

    char str[256];
    for ( int iprt = 0; iprt < m_NumFeaParts; iprt++ )
    {
        sprintf( str, "%s_Node_Tag_%d", GetID().c_str(), iprt );
        m_FeaNodeDO[iprt].m_GeomID = string( str );
        sprintf( str, "%s_T_Element_Tag_%d", GetID().c_str(), iprt );
        m_FeaTriElementDO[iprt].m_GeomID = string( str );
        sprintf( str, "%s_Q_Element_Tag_%d", GetID().c_str(), iprt );
        m_FeaQuadElementDO[iprt].m_GeomID = string( str );
        sprintf( str, "%s_Cap_Element_Tag_%d", GetID().c_str(), iprt );
        m_CapFeaElementDO[iprt].m_GeomID = string( str );

        for ( unsigned int j = 0; j < (int)m_FeaNodeVec.size(); j++ )
        {
            if ( m_PntShift[j] >= 0 )
            {
                if ( m_FeaNodeVec[j]->HasTag( iprt ) )
                {
                    m_FeaNodeDO[iprt].m_PntVec.push_back( m_FeaNodeVec[j]->m_Pnt );
                }
            }
        }


        if ( m_FixPointFeaPartFlagVec[iprt] )
        {
            for ( unsigned int j = 0; j < (int)m_FeaNodeVec.size(); j++ )
            {
                if ( m_PntShift[j] >= 0 )
                {
                    if ( m_FeaNodeVec[j]->HasTag( iprt ) && m_FeaNodeVec[j]->m_FixedPointFlag )
                    {
                        m_FeaTriElementDO[iprt].m_PntVec.push_back( m_FeaNodeVec[j]->m_Pnt );
                    }
                }
            }
        }
        else
        {
            for ( int j = 0; j < m_FeaElementVec.size(); j++ )
            {
                if ( m_FeaElementVec[j]->GetFeaPartIndex() == iprt && m_FeaElementVec[j]->GetFeaSSIndex() < 0 )
                {
                    if ( m_FeaElementVec[j]->GetElementType() == FeaElement::FEA_TRI_3 || m_FeaElementVec[j]->GetElementType() == FeaElement::FEA_TRI_6 )
                    {
                        vec3d norm = cross( m_FeaElementVec[j]->m_Corners[1]->m_Pnt - m_FeaElementVec[j]->m_Corners[0]->m_Pnt, m_FeaElementVec[j]->m_Corners[2]->m_Pnt - m_FeaElementVec[j]->m_Corners[0]->m_Pnt );
                        norm.normalize();
                        m_FeaTriElementDO[iprt].m_PntVec.push_back( m_FeaElementVec[j]->m_Corners[0]->m_Pnt );
                        m_FeaTriElementDO[iprt].m_PntVec.push_back( m_FeaElementVec[j]->m_Corners[1]->m_Pnt );
                        m_FeaTriElementDO[iprt].m_PntVec.push_back( m_FeaElementVec[j]->m_Corners[2]->m_Pnt );
                        m_FeaTriElementDO[iprt].m_NormVec.push_back( norm );
                        m_FeaTriElementDO[iprt].m_NormVec.push_back( norm );
                        m_FeaTriElementDO[iprt].m_NormVec.push_back( norm );
                    }
                    else if ( m_FeaElementVec[j]->GetElementType() == FeaElement::FEA_QUAD_4 || m_FeaElementVec[j]->GetElementType() == FeaElement::FEA_QUAD_8 )
                    {
                        vec3d norm = cross( m_FeaElementVec[j]->m_Corners[1]->m_Pnt - m_FeaElementVec[j]->m_Corners[0]->m_Pnt, m_FeaElementVec[j]->m_Corners[2]->m_Pnt - m_FeaElementVec[j]->m_Corners[0]->m_Pnt );
                        norm.normalize();
                        m_FeaQuadElementDO[iprt].m_PntVec.push_back( m_FeaElementVec[j]->m_Corners[0]->m_Pnt );
                        m_FeaQuadElementDO[iprt].m_PntVec.push_back( m_FeaElementVec[j]->m_Corners[1]->m_Pnt );
                        m_FeaQuadElementDO[iprt].m_PntVec.push_back( m_FeaElementVec[j]->m_Corners[2]->m_Pnt );
                        m_FeaQuadElementDO[iprt].m_PntVec.push_back( m_FeaElementVec[j]->m_Corners[3]->m_Pnt );
                        m_FeaQuadElementDO[iprt].m_NormVec.push_back( norm );
                        m_FeaQuadElementDO[iprt].m_NormVec.push_back( norm );
                        m_FeaQuadElementDO[iprt].m_NormVec.push_back( norm );
                        m_FeaQuadElementDO[iprt].m_NormVec.push_back( norm );
                    }
                }
            }
        }


        for ( int j = 0; j < m_FeaElementVec.size(); j++ )
        {
            if ( m_FeaElementVec[j]->GetFeaPartIndex() == iprt && m_FeaElementVec[j]->GetElementType() == FeaElement::FEA_BEAM && m_FeaElementVec[j]->GetFeaSSIndex() < 0 )
            {
                m_CapFeaElementDO[iprt].m_PntVec.push_back( m_FeaElementVec[j]->m_Corners[0]->m_Pnt );
                m_CapFeaElementDO[iprt].m_PntVec.push_back( m_FeaElementVec[j]->m_Corners[1]->m_Pnt );

                // Normal Vec is not required, load placeholder
                m_CapFeaElementDO[iprt].m_NormVec.push_back( m_FeaElementVec[j]->m_Corners[0]->m_Pnt );
                m_CapFeaElementDO[iprt].m_NormVec.push_back( m_FeaElementVec[j]->m_Corners[1]->m_Pnt );
            }
        }

        sprintf( str, "%s_Tri_Norm_%d", GetID().c_str(), iprt );
        m_ElOrientationDO[iprt].m_GeomID = string( str );
        m_ElOrientationDO[iprt].m_Type = DrawObj::VSP_LINES;
        m_ElOrientationDO[iprt].m_LineWidth = 1.0;
        m_ElOrientationDO[iprt].m_LineColor = vec3d( 0.0, 0.0, 0.0 );

        sprintf( str, "%s_Cap_Norm_%d", GetID().c_str(), iprt );
        m_CapNormDO[iprt].m_GeomID = string( str );
        m_CapNormDO[iprt].m_Type = DrawObj::VSP_LINES;
        m_CapNormDO[iprt].m_LineWidth = 1.0;
        m_CapNormDO[iprt].m_LineColor = vec3d( 0.0, 0.0, 0.0 );

        vector < vec3d > el_orient_pnt_vec, cap_norm_pnt_vec;

        for ( int j = 0; j < m_FeaElementVec.size(); j++ )
        {
            if ( m_FeaElementVec[j]->GetFeaPartIndex() == iprt &&
                 ( m_FeaElementVec[j]->GetElementType() == FeaElement::FEA_TRI_3 ||
                   m_FeaElementVec[j]->GetElementType() == FeaElement::FEA_TRI_6 ) &&
                 m_FeaElementVec[j]->GetFeaSSIndex() < 0 )
            {
                vec3d center = ( m_FeaElementVec[j]->m_Corners[0]->m_Pnt + m_FeaElementVec[j]->m_Corners[1]->m_Pnt + m_FeaElementVec[j]->m_Corners[2]->m_Pnt ) / 3.0;

                // Define orientation vec:
                FeaTri* tri = dynamic_cast<FeaTri*>( m_FeaElementVec[j] );
                assert( tri );

                vec3d orient_pnt = center + line_length * tri->m_Orientation;

                el_orient_pnt_vec.push_back( center );
                el_orient_pnt_vec.push_back( orient_pnt );
            }
            else if ( m_FeaElementVec[j]->GetFeaPartIndex() == iprt &&
                      ( m_FeaElementVec[j]->GetElementType() == FeaElement::FEA_QUAD_4 ||
                        m_FeaElementVec[j]->GetElementType() == FeaElement::FEA_QUAD_8 ) &&
                      m_FeaElementVec[j]->GetFeaSSIndex() < 0 )
            {
                vec3d center = ( m_FeaElementVec[j]->m_Corners[0]->m_Pnt + m_FeaElementVec[j]->m_Corners[1]->m_Pnt + m_FeaElementVec[j]->m_Corners[2]->m_Pnt + m_FeaElementVec[j]->m_Corners[3]->m_Pnt ) / 4.0;

                // Define orientation vec:
                FeaQuad* quad = dynamic_cast<FeaQuad*>( m_FeaElementVec[j] );
                assert( quad );

                vec3d orient_pnt = center + line_length * quad->m_Orientation;

                el_orient_pnt_vec.push_back( center );
                el_orient_pnt_vec.push_back( orient_pnt );
            }
            else if ( m_FeaElementVec[j]->GetFeaPartIndex() == iprt && m_FeaElementVec[j]->GetElementType() == FeaElement::FEA_BEAM && m_FeaElementVec[j]->GetFeaSSIndex() < 0 )
            {
                FeaBeam* beam = dynamic_cast<FeaBeam*>( m_FeaElementVec[j] );
                assert( beam );

                vec3d norm_pnt = m_FeaElementVec[j]->m_Mids[0]->m_Pnt + line_length * beam->m_DispVec;

                cap_norm_pnt_vec.push_back( m_FeaElementVec[j]->m_Mids[0]->m_Pnt );
                cap_norm_pnt_vec.push_back( norm_pnt );
            }
        }

        m_ElOrientationDO[iprt].m_PntVec = el_orient_pnt_vec;
        m_CapNormDO[iprt].m_PntVec = cap_norm_pnt_vec;
    }

    // FeaSubSurfaces:
    m_SSFeaNodeDO.resize( m_NumFeaSubSurfs );
    m_SSTriElementDO.resize( m_NumFeaSubSurfs );
    m_SSQuadElementDO.resize( m_NumFeaSubSurfs );
    m_SSCapFeaElementDO.resize( m_NumFeaSubSurfs );
    m_SSElOrientationDO.resize( m_NumFeaSubSurfs );
    m_SSCapNormDO.resize( m_NumFeaSubSurfs );

    for ( int iss = 0; iss < m_NumFeaSubSurfs; iss++ )
    {
        sprintf( str, "%s_SSNode_Tag_%d", GetID().c_str(), iss );
        m_SSFeaNodeDO[iss].m_GeomID = string( str );
        sprintf( str, "%s_SSTriElement_Tag_%d", GetID().c_str(), iss );
        m_SSTriElementDO[iss].m_GeomID = string( str );
        sprintf( str, "%s_SSQuadElement_Tag_%d", GetID().c_str(), iss );
        m_SSQuadElementDO[iss].m_GeomID = string( str );
        sprintf( str, "%s_SSCap_Element_Tag_%d", GetID().c_str(), iss );
        m_SSCapFeaElementDO[iss].m_GeomID = string( str );

        for ( unsigned int j = 0; j < (int)m_FeaNodeVec.size(); j++ )
        {
            if ( m_PntShift[j] >= 0 )
            {
                if ( m_FeaNodeVec[j]->HasTag( iss + m_NumFeaParts ) )
                {
                    m_SSFeaNodeDO[iss].m_PntVec.push_back( m_FeaNodeVec[j]->m_Pnt );
                }
            }
        }

        for ( int j = 0; j < m_FeaElementVec.size(); j++ )
        {
            if (( m_FeaElementVec[j]->GetFeaSSIndex() == iss ) &&
                ( m_FeaElementVec[j]->GetElementType() == FeaElement::FEA_TRI_3 || m_FeaElementVec[j]->GetElementType() == FeaElement::FEA_TRI_6 ) )
            {
                vec3d norm = cross( m_FeaElementVec[j]->m_Corners[1]->m_Pnt - m_FeaElementVec[j]->m_Corners[0]->m_Pnt, m_FeaElementVec[j]->m_Corners[2]->m_Pnt - m_FeaElementVec[j]->m_Corners[0]->m_Pnt );
                norm.normalize();
                m_SSTriElementDO[iss].m_PntVec.push_back( m_FeaElementVec[j]->m_Corners[0]->m_Pnt );
                m_SSTriElementDO[iss].m_PntVec.push_back( m_FeaElementVec[j]->m_Corners[1]->m_Pnt );
                m_SSTriElementDO[iss].m_PntVec.push_back( m_FeaElementVec[j]->m_Corners[2]->m_Pnt );
                m_SSTriElementDO[iss].m_NormVec.push_back( norm );
                m_SSTriElementDO[iss].m_NormVec.push_back( norm );
                m_SSTriElementDO[iss].m_NormVec.push_back( norm );
            }
            else if (( m_FeaElementVec[j]->GetFeaSSIndex() == iss ) &&
                     ( m_FeaElementVec[j]->GetElementType() == FeaElement::FEA_QUAD_4 || m_FeaElementVec[j]->GetElementType() == FeaElement::FEA_QUAD_4 ) )
            {
                vec3d norm = cross( m_FeaElementVec[j]->m_Corners[1]->m_Pnt - m_FeaElementVec[j]->m_Corners[0]->m_Pnt, m_FeaElementVec[j]->m_Corners[2]->m_Pnt - m_FeaElementVec[j]->m_Corners[0]->m_Pnt );
                norm.normalize();
                m_SSQuadElementDO[iss].m_PntVec.push_back( m_FeaElementVec[j]->m_Corners[0]->m_Pnt );
                m_SSQuadElementDO[iss].m_PntVec.push_back( m_FeaElementVec[j]->m_Corners[1]->m_Pnt );
                m_SSQuadElementDO[iss].m_PntVec.push_back( m_FeaElementVec[j]->m_Corners[2]->m_Pnt );
                m_SSQuadElementDO[iss].m_PntVec.push_back( m_FeaElementVec[j]->m_Corners[3]->m_Pnt );
                m_SSQuadElementDO[iss].m_NormVec.push_back( norm );
                m_SSQuadElementDO[iss].m_NormVec.push_back( norm );
                m_SSQuadElementDO[iss].m_NormVec.push_back( norm );
                m_SSQuadElementDO[iss].m_NormVec.push_back( norm );
            }
        }

        for ( int j = 0; j < m_FeaElementVec.size(); j++ )
        {
            if (( m_FeaElementVec[j]->GetFeaSSIndex() == iss ) && m_FeaElementVec[j]->GetElementType() == FeaElement::FEA_BEAM )
            {
                m_SSCapFeaElementDO[iss].m_PntVec.push_back( m_FeaElementVec[j]->m_Corners[0]->m_Pnt );
                m_SSCapFeaElementDO[iss].m_PntVec.push_back( m_FeaElementVec[j]->m_Corners[1]->m_Pnt );

                // Normal Vec is not required, load placeholder
                m_SSCapFeaElementDO[iss].m_NormVec.push_back( m_FeaElementVec[j]->m_Corners[0]->m_Pnt );
                m_SSCapFeaElementDO[iss].m_NormVec.push_back( m_FeaElementVec[j]->m_Corners[1]->m_Pnt );
            }
        }

        sprintf( str, "%s_SSTri_Norm_%u", GetID().c_str(), iss );
        m_SSElOrientationDO[iss].m_GeomID = string( str );
        m_SSElOrientationDO[iss].m_Type = DrawObj::VSP_LINES;
        m_SSElOrientationDO[iss].m_LineWidth = 1.0;
        m_SSElOrientationDO[iss].m_LineColor = vec3d( 0.0, 0.0, 0.0 );

        sprintf( str, "%s_SSCap_Norm_%u", GetID().c_str(), iss );
        m_SSCapNormDO[iss].m_GeomID = string( str );
        m_SSCapNormDO[iss].m_Type = DrawObj::VSP_LINES;
        m_SSCapNormDO[iss].m_LineWidth = 1.0;
        m_SSCapNormDO[iss].m_LineColor = vec3d( 0.0, 0.0, 0.0 );

        vector < vec3d > ss_el_orient_pnt_vec, ss_cap_norm_pnt_vec;

        for ( int j = 0; j < m_FeaElementVec.size(); j++ )
        {
            if (( m_FeaElementVec[j]->GetFeaSSIndex() == iss ) &&
                ( m_FeaElementVec[j]->GetElementType() == FeaElement::FEA_TRI_3 || m_FeaElementVec[j]->GetElementType() == FeaElement::FEA_TRI_6 ) )
            {
                vec3d center = ( m_FeaElementVec[j]->m_Corners[0]->m_Pnt + m_FeaElementVec[j]->m_Corners[1]->m_Pnt + m_FeaElementVec[j]->m_Corners[2]->m_Pnt ) / 3.0;

                // Define orientation vec:
                FeaTri* tri = dynamic_cast<FeaTri*>( m_FeaElementVec[j] );
                assert( tri );

                vec3d orient_pnt = center + line_length * tri->m_Orientation;

                ss_el_orient_pnt_vec.push_back( center );
                ss_el_orient_pnt_vec.push_back( orient_pnt );
            }
            else if (( m_FeaElementVec[j]->GetFeaSSIndex() == iss ) &&
                     ( m_FeaElementVec[j]->GetElementType() == FeaElement::FEA_QUAD_4 || m_FeaElementVec[j]->GetElementType() == FeaElement::FEA_QUAD_8 ) )
            {
                vec3d center = ( m_FeaElementVec[j]->m_Corners[0]->m_Pnt + m_FeaElementVec[j]->m_Corners[1]->m_Pnt + m_FeaElementVec[j]->m_Corners[2]->m_Pnt + m_FeaElementVec[j]->m_Corners[3]->m_Pnt ) / 4.0;

                // Define orientation vec:
                FeaQuad* quad = dynamic_cast<FeaQuad*>( m_FeaElementVec[j] );
                assert( quad );

                vec3d orient_pnt = center + line_length * quad->m_Orientation;

                ss_el_orient_pnt_vec.push_back( center );
                ss_el_orient_pnt_vec.push_back( orient_pnt );
            }
            else if (( m_FeaElementVec[j]->GetFeaSSIndex() == iss ) && m_FeaElementVec[j]->GetElementType() == FeaElement::FEA_BEAM )
            {
                FeaBeam* beam = dynamic_cast<FeaBeam*>( m_FeaElementVec[j] );
                assert( beam );

                vec3d norm_pnt = m_FeaElementVec[j]->m_Mids[0]->m_Pnt + line_length * beam->m_DispVec;

                ss_cap_norm_pnt_vec.push_back( m_FeaElementVec[j]->m_Mids[0]->m_Pnt );
                ss_cap_norm_pnt_vec.push_back( norm_pnt );
            }
        }

        m_SSElOrientationDO[iss].m_PntVec = ss_el_orient_pnt_vec;
        m_SSCapNormDO[iss].m_PntVec = ss_cap_norm_pnt_vec;
    }
}

void FeaMesh::LoadDrawObjs( vector< DrawObj* > &draw_obj_vec, SimpleFeaMeshSettings* st_settings )
{
    if ( !m_MeshDOUpToDate )
    {
        return;
    }

    if ( m_DrawElementFlagVec.size() == m_NumFeaParts + m_NumFeaSubSurfs )
    {
        // Calculate constants for color sequence.
        const int ncgrp = 6; // Number of basic colors
        const double ncstep = (int)ceil( (double)( 4 * ( m_NumFeaParts + m_NumFeaSubSurfs ) ) / (double)ncgrp );
        const double nctodeg = 360.0 / ( ncgrp*ncstep );

        for ( int iprt = 0; iprt < m_NumFeaParts; iprt++ )
        {
            m_FeaNodeDO[iprt].m_Type = DrawObj::VSP_POINTS;
            m_FeaNodeDO[iprt].m_PointSize = 6.0;
            m_FeaNodeDO[iprt].m_PointColor = vec3d( 0, 0, 1 );

            if ( m_FixPointFeaPartFlagVec[iprt] )
            {
                m_FeaTriElementDO[iprt].m_Type = DrawObj::VSP_POINTS;
                m_FeaTriElementDO[iprt].m_PointSize = 9.0;
                m_FeaTriElementDO[iprt].m_PointColor = vec3d( 0, 0, 1 );

                m_FeaQuadElementDO[iprt].m_Type = DrawObj::VSP_POINTS;
                m_FeaQuadElementDO[iprt].m_PointSize = 9.0;
                m_FeaQuadElementDO[iprt].m_PointColor = vec3d( 0, 0, 1 );
            }
            else
            {
                m_FeaTriElementDO[iprt].m_Type = DrawObj::VSP_SHADED_TRIS;
                m_FeaQuadElementDO[iprt].m_Type = DrawObj::VSP_SHADED_QUADS;
            }
            m_CapFeaElementDO[iprt].m_Type = DrawObj::VSP_LINES;
            m_CapFeaElementDO[iprt].m_LineWidth = 3.0;

            if ( st_settings->m_DrawMeshFlag ||
                 st_settings->m_ColorTagsFlag )   // At least mesh or tags are visible.
            {
                if ( !m_FixPointFeaPartFlagVec[iprt] )
                {
                    if ( st_settings->m_DrawMeshFlag &&
                         st_settings->m_ColorTagsFlag ) // Both are visible.
                    {
                        m_FeaTriElementDO[iprt].m_Type = DrawObj::VSP_WIRE_SHADED_TRIS;
                        m_FeaTriElementDO[iprt].m_LineColor = vec3d( 0.4, 0.4, 0.4 );

                        m_FeaQuadElementDO[iprt].m_Type = DrawObj::VSP_WIRE_SHADED_QUADS;
                        m_FeaQuadElementDO[iprt].m_LineColor = vec3d( 0.4, 0.4, 0.4 );
                    }
                    else if ( st_settings->m_DrawMeshFlag ) // Mesh only
                    {
                        m_FeaTriElementDO[iprt].m_Type = DrawObj::VSP_HIDDEN_TRIS;
                        m_FeaTriElementDO[iprt].m_LineColor = vec3d( 0.4, 0.4, 0.4 );

                        m_FeaQuadElementDO[iprt].m_Type = DrawObj::VSP_HIDDEN_QUADS;
                        m_FeaQuadElementDO[iprt].m_LineColor = vec3d( 0.4, 0.4, 0.4 );
                    }
                    else // Tags only
                    {
                        m_FeaTriElementDO[iprt].m_Type = DrawObj::VSP_SHADED_TRIS;

                        m_FeaQuadElementDO[iprt].m_Type = DrawObj::VSP_SHADED_QUADS;
                    }
                }
            }
            else
            {
                m_FeaTriElementDO[iprt].m_Type = DrawObj::VSP_HIDDEN_TRIS;
                m_FeaTriElementDO[iprt].m_LineColor = vec3d( 0.4, 0.4, 0.4 );

                m_FeaQuadElementDO[iprt].m_Type = DrawObj::VSP_HIDDEN_QUADS;
                m_FeaQuadElementDO[iprt].m_LineColor = vec3d( 0.4, 0.4, 0.4 );
            }

            if ( st_settings->m_ColorTagsFlag )
            {
                // Color sequence -- go around color wheel ncstep times with slight
                // offset from ncgrp basic colors.
                // Note, (cnt/ncgrp) uses integer division resulting in floor.
                double deg = (( iprt % ncgrp ) * ncstep + ( iprt / ncgrp ) ) * nctodeg;
                double deg2 = ((( m_NumFeaParts + iprt ) % ncgrp ) * ncstep + (( m_NumFeaParts + iprt ) / ncgrp ) ) * nctodeg;
                vec3d rgb = m_FeaTriElementDO[iprt].ColorWheel( deg );
                rgb.normalize();

                if ( m_FixPointFeaPartFlagVec[iprt] )
                {
                    m_FeaTriElementDO[iprt].m_PointColor = rgb;
                    m_FeaQuadElementDO[iprt].m_PointColor = rgb;
                }

                m_FeaNodeDO[iprt].m_PointColor = rgb;
                m_CapFeaElementDO[iprt].m_LineColor = m_CapFeaElementDO[iprt].ColorWheel( deg2 );

                for ( int icomp = 0; icomp < 3; icomp++ )
                {
                    m_FeaTriElementDO[iprt].m_MaterialInfo.Ambient[icomp] = (float)rgb.v[icomp] / 5.0f;
                    m_FeaTriElementDO[iprt].m_MaterialInfo.Diffuse[icomp] = 0.4f + (float)rgb.v[icomp] / 10.0f;
                    m_FeaTriElementDO[iprt].m_MaterialInfo.Specular[icomp] = 0.04f + 0.7f * (float)rgb.v[icomp];
                    m_FeaTriElementDO[iprt].m_MaterialInfo.Emission[icomp] = (float)rgb.v[icomp] / 20.0f;

                    m_FeaQuadElementDO[iprt].m_MaterialInfo.Ambient[icomp] = (float)rgb.v[icomp] / 5.0f;
                    m_FeaQuadElementDO[iprt].m_MaterialInfo.Diffuse[icomp] = 0.4f + (float)rgb.v[icomp] / 10.0f;
                    m_FeaQuadElementDO[iprt].m_MaterialInfo.Specular[icomp] = 0.04f + 0.7f * (float)rgb.v[icomp];
                    m_FeaQuadElementDO[iprt].m_MaterialInfo.Emission[icomp] = (float)rgb.v[icomp] / 20.0f;
                }
                m_FeaTriElementDO[iprt].m_MaterialInfo.Ambient[3] = 1.0f;
                m_FeaTriElementDO[iprt].m_MaterialInfo.Diffuse[3] = 1.0f;
                m_FeaTriElementDO[iprt].m_MaterialInfo.Specular[3] = 1.0f;
                m_FeaTriElementDO[iprt].m_MaterialInfo.Emission[3] = 1.0f;

                m_FeaTriElementDO[iprt].m_MaterialInfo.Shininess = 32.0f;

                m_FeaQuadElementDO[iprt].m_MaterialInfo.Ambient[3] = 1.0f;
                m_FeaQuadElementDO[iprt].m_MaterialInfo.Diffuse[3] = 1.0f;
                m_FeaQuadElementDO[iprt].m_MaterialInfo.Specular[3] = 1.0f;
                m_FeaQuadElementDO[iprt].m_MaterialInfo.Emission[3] = 1.0f;

                m_FeaQuadElementDO[iprt].m_MaterialInfo.Shininess = 32.0f;
            }
            else
            {
                for ( int icomp = 0; icomp < 4; icomp++ )
                {
                    m_FeaTriElementDO[iprt].m_MaterialInfo.Ambient[icomp] = 1.0f;
                    m_FeaTriElementDO[iprt].m_MaterialInfo.Diffuse[icomp] = 1.0f;
                    m_FeaTriElementDO[iprt].m_MaterialInfo.Specular[icomp] = 1.0f;
                    m_FeaTriElementDO[iprt].m_MaterialInfo.Emission[icomp] = 1.0f;

                    m_FeaQuadElementDO[iprt].m_MaterialInfo.Ambient[icomp] = 1.0f;
                    m_FeaQuadElementDO[iprt].m_MaterialInfo.Diffuse[icomp] = 1.0f;
                    m_FeaQuadElementDO[iprt].m_MaterialInfo.Specular[icomp] = 1.0f;
                    m_FeaQuadElementDO[iprt].m_MaterialInfo.Emission[icomp] = 1.0f;
                }
                m_FeaTriElementDO[iprt].m_MaterialInfo.Shininess = 1.0f;
                m_FeaQuadElementDO[iprt].m_MaterialInfo.Shininess = 1.0f;

                // No color needed for mesh only.
                m_FeaNodeDO[iprt].m_PointColor = vec3d( 0.0, 0.0, 0.0 );
                m_CapFeaElementDO[iprt].m_LineColor = vec3d( 0.0, 0.0, 0.0 );
            }

            m_FeaTriElementDO[iprt].m_Visible = ( st_settings->m_DrawMeshFlag || st_settings->m_ColorTagsFlag ) && m_DrawElementFlagVec[iprt];
            m_FeaQuadElementDO[iprt].m_Visible = ( st_settings->m_DrawMeshFlag || st_settings->m_ColorTagsFlag ) && m_DrawElementFlagVec[iprt];
            m_FeaNodeDO[ iprt ].m_Visible = st_settings->m_DrawNodesFlag && m_DrawElementFlagVec[iprt];
            m_CapFeaElementDO[ iprt ].m_Visible = m_DrawCapFlagVec[ iprt ];
            m_ElOrientationDO[iprt].m_Visible = st_settings->m_DrawElementOrientVecFlag && m_DrawElementFlagVec[iprt];
            m_CapNormDO[iprt].m_Visible = st_settings->m_DrawElementOrientVecFlag && m_DrawCapFlagVec[iprt];

            draw_obj_vec.push_back( &m_FeaNodeDO[iprt] );
            draw_obj_vec.push_back( &m_FeaTriElementDO[iprt] );
            draw_obj_vec.push_back( &m_FeaQuadElementDO[iprt] );
            draw_obj_vec.push_back( &m_CapFeaElementDO[iprt] );
            draw_obj_vec.push_back( &m_ElOrientationDO[iprt] );
            draw_obj_vec.push_back( &m_CapNormDO[iprt] );
        }

        for ( int iss = 0; iss < m_NumFeaSubSurfs; iss++ )
        {
            m_SSFeaNodeDO[iss].m_Type = DrawObj::VSP_POINTS;
            m_SSFeaNodeDO[iss].m_PointSize = 6.0;
            m_SSTriElementDO[iss].m_Type = DrawObj::VSP_SHADED_TRIS;
            m_SSQuadElementDO[iss].m_Type = DrawObj::VSP_SHADED_QUADS;
            m_SSCapFeaElementDO[iss].m_Type = DrawObj::VSP_LINES;
            m_SSCapFeaElementDO[iss].m_LineWidth = 3.0;

            if ( st_settings->m_DrawMeshFlag ||
                 st_settings->m_ColorTagsFlag )   // At least mesh or tags are visible.
            {

                if ( st_settings->m_DrawMeshFlag &&
                     st_settings->m_ColorTagsFlag ) // Both are visible.
                {
                    m_SSTriElementDO[iss].m_Type = DrawObj::VSP_WIRE_SHADED_TRIS;
                    m_SSTriElementDO[iss].m_LineColor = vec3d( 0.4, 0.4, 0.4 );
                    m_SSQuadElementDO[iss].m_Type = DrawObj::VSP_WIRE_SHADED_QUADS;
                    m_SSQuadElementDO[iss].m_LineColor = vec3d( 0.4, 0.4, 0.4 );
                }
                else if ( st_settings->m_DrawMeshFlag ) // Mesh only
                {
                    m_SSTriElementDO[iss].m_Type = DrawObj::VSP_HIDDEN_TRIS;
                    m_SSTriElementDO[iss].m_LineColor = vec3d( 0.4, 0.4, 0.4 );
                    m_SSQuadElementDO[iss].m_Type = DrawObj::VSP_HIDDEN_QUADS;
                    m_SSQuadElementDO[iss].m_LineColor = vec3d( 0.4, 0.4, 0.4 );
                }
                else // Tags only
                {
                    m_SSTriElementDO[iss].m_Type = DrawObj::VSP_SHADED_TRIS;
                    m_SSQuadElementDO[iss].m_Type = DrawObj::VSP_SHADED_QUADS;
                }
            }
            else
            {
                m_SSTriElementDO[iss].m_Type = DrawObj::VSP_HIDDEN_TRIS;
                m_SSTriElementDO[iss].m_LineColor = vec3d( 0.4, 0.4, 0.4 );
                m_SSQuadElementDO[iss].m_Type = DrawObj::VSP_HIDDEN_QUADS;
                m_SSQuadElementDO[iss].m_LineColor = vec3d( 0.4, 0.4, 0.4 );
            }

            if ( st_settings->m_ColorTagsFlag )
            {
                // Color sequence -- go around color wheel ncstep times with slight
                // offset from ncgrp basic colors.
                // Note, (cnt/ncgrp) uses integer division resulting in floor.
                double deg = ((( m_NumFeaParts + iss ) % ncgrp ) * ncstep + (( m_NumFeaParts + iss ) / ncgrp ) ) * nctodeg;
                double deg2 = ((( 1 + m_NumFeaParts + m_NumFeaSubSurfs + iss ) % ncgrp ) * ncstep + (( 1 + m_NumFeaParts + m_NumFeaSubSurfs + iss ) / ncgrp ) ) * nctodeg;
                vec3d rgb = m_SSTriElementDO[iss].ColorWheel( deg );
                rgb.normalize();

                m_SSFeaNodeDO[iss].m_PointColor = rgb;
                m_SSCapFeaElementDO[iss].m_LineColor = m_SSCapFeaElementDO[iss].ColorWheel( deg2 );

                for ( int icomp = 0; icomp < 3; icomp++ )
                {
                    m_SSTriElementDO[iss].m_MaterialInfo.Ambient[icomp] = (float)rgb.v[icomp] / 5.0f;
                    m_SSTriElementDO[iss].m_MaterialInfo.Diffuse[icomp] = 0.4f + (float)rgb.v[icomp] / 10.0f;
                    m_SSTriElementDO[iss].m_MaterialInfo.Specular[icomp] = 0.04f + 0.7f * (float)rgb.v[icomp];
                    m_SSTriElementDO[iss].m_MaterialInfo.Emission[icomp] = (float)rgb.v[icomp] / 20.0f;

                    m_SSQuadElementDO[iss].m_MaterialInfo.Ambient[icomp] = (float)rgb.v[icomp] / 5.0f;
                    m_SSQuadElementDO[iss].m_MaterialInfo.Diffuse[icomp] = 0.4f + (float)rgb.v[icomp] / 10.0f;
                    m_SSQuadElementDO[iss].m_MaterialInfo.Specular[icomp] = 0.04f + 0.7f * (float)rgb.v[icomp];
                    m_SSQuadElementDO[iss].m_MaterialInfo.Emission[icomp] = (float)rgb.v[icomp] / 20.0f;
                }
                m_SSTriElementDO[iss].m_MaterialInfo.Ambient[3] = 1.0f;
                m_SSTriElementDO[iss].m_MaterialInfo.Diffuse[3] = 1.0f;
                m_SSTriElementDO[iss].m_MaterialInfo.Specular[3] = 1.0f;
                m_SSTriElementDO[iss].m_MaterialInfo.Emission[3] = 1.0f;

                m_SSQuadElementDO[iss].m_MaterialInfo.Ambient[3] = 1.0f;
                m_SSQuadElementDO[iss].m_MaterialInfo.Diffuse[3] = 1.0f;
                m_SSQuadElementDO[iss].m_MaterialInfo.Specular[3] = 1.0f;
                m_SSQuadElementDO[iss].m_MaterialInfo.Emission[3] = 1.0f;

                m_SSTriElementDO[iss].m_MaterialInfo.Shininess = 32.0f;
                m_SSQuadElementDO[iss].m_MaterialInfo.Shininess = 32.0f;
            }
            else
            {
                for ( int icomp = 0; icomp < 4; icomp++ )
                {
                    m_SSTriElementDO[iss].m_MaterialInfo.Ambient[icomp] = 1.0f;
                    m_SSTriElementDO[iss].m_MaterialInfo.Diffuse[icomp] = 1.0f;
                    m_SSTriElementDO[iss].m_MaterialInfo.Specular[icomp] = 1.0f;
                    m_SSTriElementDO[iss].m_MaterialInfo.Emission[icomp] = 1.0f;

                    m_SSQuadElementDO[iss].m_MaterialInfo.Ambient[icomp] = 1.0f;
                    m_SSQuadElementDO[iss].m_MaterialInfo.Diffuse[icomp] = 1.0f;
                    m_SSQuadElementDO[iss].m_MaterialInfo.Specular[icomp] = 1.0f;
                    m_SSQuadElementDO[iss].m_MaterialInfo.Emission[icomp] = 1.0f;
                }
                m_SSTriElementDO[iss].m_MaterialInfo.Shininess = 1.0f;
                m_SSQuadElementDO[iss].m_MaterialInfo.Shininess = 1.0f;

                m_SSFeaNodeDO[iss].m_PointColor = vec3d( 0.0, 0.0, 0.0 );
                m_SSCapFeaElementDO[iss].m_LineColor = vec3d( 0.0, 0.0, 0.0 );
            }

            m_SSTriElementDO[ iss ].m_Visible = ( st_settings->m_DrawMeshFlag || st_settings->m_ColorTagsFlag ) && m_DrawElementFlagVec[ iss + m_NumFeaParts ];
            m_SSQuadElementDO[ iss ].m_Visible = ( st_settings->m_DrawMeshFlag || st_settings->m_ColorTagsFlag ) && m_DrawElementFlagVec[ iss + m_NumFeaParts ];

            m_SSFeaNodeDO[ iss ].m_Visible = st_settings->m_DrawNodesFlag && m_DrawElementFlagVec[ iss + m_NumFeaParts];
            m_SSCapFeaElementDO[ iss ].m_Visible = m_DrawCapFlagVec[ iss + m_NumFeaParts];
            m_SSElOrientationDO[ iss ].m_Visible = st_settings->m_DrawElementOrientVecFlag && m_DrawElementFlagVec[ iss + m_NumFeaParts ];
            m_SSCapNormDO[ iss ].m_Visible = st_settings->m_DrawElementOrientVecFlag && m_DrawCapFlagVec[ iss + m_NumFeaParts ];

            draw_obj_vec.push_back( &m_SSFeaNodeDO[iss] );
            draw_obj_vec.push_back( &m_SSTriElementDO[iss] );
            draw_obj_vec.push_back( &m_SSQuadElementDO[iss] );
            draw_obj_vec.push_back( &m_SSCapFeaElementDO[iss] );
            draw_obj_vec.push_back( &m_SSElOrientationDO[iss] );
            draw_obj_vec.push_back( &m_SSCapNormDO[iss] );
        }
    }
}

void FeaMesh::ExportFeaMesh()
{
    if ( !m_MeshReady )
    {
        return;
    }

    if ( GetStructSettingsPtr()->GetExportFileFlag( vsp::FEA_STL_FILE_NAME ) )
    {
        WriteSTL();
    }

    if ( GetStructSettingsPtr()->GetExportFileFlag( vsp::FEA_NASTRAN_FILE_NAME ) )
    {
        WriteNASTRAN( GetStructSettingsPtr()->GetExportFileName( vsp::FEA_NASTRAN_FILE_NAME ) );
    }

    if ( GetStructSettingsPtr()->GetExportFileFlag( vsp::FEA_CALCULIX_FILE_NAME ) )
    {
        WriteCalculix();
    }

    if ( GetStructSettingsPtr()->GetExportFileFlag( vsp::FEA_GMSH_FILE_NAME ) )
    {
        WriteGmsh();
    }

    if ( GetStructSettingsPtr()->GetExportFileFlag( vsp::FEA_MASS_FILE_NAME ) )
    {
        ComputeWriteMass();
        string mass_output = "Total Mass = " + std::to_string( m_TotalMass ) + "\n";
        FeaMeshMgr.addOutputText( mass_output );
    }
}

void FeaMesh::WriteNASTRAN( const string &filename )
{
    FeaMeshMgr.ResetPropMatUse();

    int noffset = m_StructSettings.m_NodeOffset;
    int eoffset = m_StructSettings.m_ElementOffset;

    // Create temporary file to store NASTRAN bulk data. Case control information (SETs) will be
    //  defined in the *_NASTRAN.dat file prior to the bulk data (elements, gridpoints, etc.)
    FILE* temp = std::tmpfile();

    FILE* fp = fopen( filename.c_str(), "w" ); // Open *_NASTRAN.dat
    if ( fp && temp )
    {
        FILE* nkey_fp = NULL;
        if ( GetStructSettingsPtr()->GetExportFileFlag( vsp::FEA_NKEY_FILE_NAME ) )
        {
            string nkey_fname = GetStructSettingsPtr()->GetExportFileName( vsp::FEA_NKEY_FILE_NAME );
            nkey_fp = fopen( nkey_fname.c_str(), "w" ); // Open *_NASTRAN.nkey
            if ( nkey_fp )
            {
                fprintf( nkey_fp, "$ NASTRAN Tag Key File Generated from %s\n", VSPVERSION4 );
                fprintf( nkey_fp, "%s\n\n", nkey_fname.c_str() );
            }
        }

        // Comments can be at top of NASTRAN file before case control section
        fprintf( fp, "$NASTRAN Data File Generated from %s\n", VSPVERSION4 );
        fprintf( fp, "$Num_Els: %u\n", m_NumEls );
        fprintf( fp, "$Num_Tris: %u\n", m_NumTris );
        fprintf( fp, "$Num_Quads: %u\n", m_NumQuads );
        fprintf( fp, "$Num_Beams %u\n", m_NumBeams );

        // Write bulk data to temp file
        fprintf( temp, "\nBEGIN BULK\n" );

        int set_cnt = 1;
        int max_grid_id = 0;
        vector < int > grid_id_vec;
        string name;

        // FeaPart Nodes
        for ( unsigned int i = 0; i < m_NumFeaParts; i++ )
        {
            grid_id_vec.clear();

            fprintf( temp, "\n" );
            fprintf( temp, "$%s Gridpoints\n", m_FeaPartNameVec[i].c_str() );

            if ( m_FeaPartTypeVec[i] != vsp::FEA_FIX_POINT )
            {
                for ( unsigned int j = 0; j < (int)m_FeaNodeVec.size(); j++ )
                {
                    if ( m_PntShift[j] >= 0 )
                    {
                        if ( m_FeaNodeVec[ j ]->HasOnlyTag( i ) )
                        {
                            m_FeaNodeVec[j]->WriteNASTRAN( temp, noffset );
                            grid_id_vec.push_back( m_FeaNodeVec[j]->m_Index );
                            max_grid_id = max( max_grid_id, m_FeaNodeVec[j]->m_Index );
                        }
                    }
                }
            }
            else if ( m_FeaPartTypeVec[i] == vsp::FEA_FIX_POINT ) // FixedPoint Nodes
            {
                for ( unsigned int j = 0; j < (int)m_FeaNodeVec.size(); j++ )
                {
                    if ( m_PntShift[j] >= 0 )
                    {
                        if ( m_FeaNodeVec[j]->m_Tags.size() > 1 && m_FeaNodeVec[j]->m_FixedPointFlag && m_FeaNodeVec[j]->HasTag( i ) )
                        {
                            m_FeaNodeVec[j]->WriteNASTRAN( temp, noffset );
                            grid_id_vec.push_back( m_FeaNodeVec[j]->m_Index );
                            max_grid_id = max( max_grid_id, m_FeaNodeVec[j]->m_Index );
                        }
                    }
                }
            }

            // Write FEA part node set
            name = m_FeaPartNameVec[i] + "_Gridpoints";
            WriteNASTRANSet( fp, nkey_fp, set_cnt, grid_id_vec, name, noffset );
        }

        // SubSurface Nodes
        for ( unsigned int i = 0; i < m_NumFeaSubSurfs; i++ )
        {
            fprintf( temp, "\n" );
            fprintf( temp, "$%s Gridpoints\n", m_SimpleSubSurfaceVec[i].GetName().c_str() );

            grid_id_vec.clear();

            for ( unsigned int j = 0; j < (int)m_FeaNodeVec.size(); j++ )
            {
                if ( m_PntShift[j] >= 0 )
                {
                    if ( m_FeaNodeVec[ j ]->HasOnlyTag( i + m_NumFeaParts ) )
                    {
                        m_FeaNodeVec[j]->WriteNASTRAN( temp, noffset );
                        grid_id_vec.push_back( m_FeaNodeVec[j]->m_Index );
                        max_grid_id = max( max_grid_id, m_FeaNodeVec[j]->m_Index );
                    }
                }
            }

            // Write subsurface node set
            name = m_SimpleSubSurfaceVec[i].GetName() + "_Gridpoints";
            WriteNASTRANSet( fp, nkey_fp, set_cnt, grid_id_vec, name, noffset );
        }

        // Intersection Nodes
        fprintf( temp, "\n" );
        fprintf( temp, "$Intersections\n" );

        grid_id_vec.clear();

        for ( unsigned int j = 0; j < (int)m_FeaNodeVec.size(); j++ )
        {
            if ( m_PntShift[j] >= 0 )
            {
                if ( m_FeaNodeVec[j]->m_Tags.size() > 1 && !m_FeaNodeVec[j]->m_FixedPointFlag )
                {
                    m_FeaNodeVec[j]->WriteNASTRAN( temp, noffset );
                    grid_id_vec.push_back( m_FeaNodeVec[j]->m_Index );
                    max_grid_id = max( max_grid_id, m_FeaNodeVec[j]->m_Index );
                }
            }
        }

        // Write intersection node set
        name = "Intersection_Gridpoints";
        WriteNASTRANSet( fp, nkey_fp, set_cnt, grid_id_vec, name, noffset );

        //==== Remaining Nodes ====//
        fprintf( temp, "\n" );
        fprintf( temp, "$Remainingnodes\n" );
        for ( int i = 0; i < (int)m_FeaNodeVec.size(); i++ )
        {
            grid_id_vec.clear();

            if ( m_PntShift[i] >= 0 && m_FeaNodeVec[i]->m_Tags.size() == 0 )
            {
                m_FeaNodeVec[i]->WriteNASTRAN( temp, noffset );
            }

            // Write remaining node set
            name = "Remaining_Gridpoints";
            WriteNASTRANSet( fp, nkey_fp, set_cnt, grid_id_vec, name, noffset );
        }

        int elem_id = max_grid_id + 1; // First element ID begins after last gridpoint ID
        vector < int > shell_elem_id_vec, beam_elem_id_vec;

        // Write FeaParts
        for ( unsigned int i = 0; i < m_NumFeaParts; i++ )
        {
            if ( m_FeaPartTypeVec[i] != vsp::FEA_FIX_POINT )
            {
                fprintf( temp, "\n" );
                fprintf( temp, "$%s\n", m_FeaPartNameVec[i].c_str() );

                shell_elem_id_vec.clear();
                beam_elem_id_vec.clear();

                int property_id = m_FeaPartPropertyIndexVec[i];
                int cap_property_id = m_FeaPartCapPropertyIndexVec[i];

                for ( int j = 0; j < m_FeaElementVec.size(); j++ )
                {
                    if ( m_FeaElementVec[j]->GetFeaPartIndex() == i && m_FeaElementVec[j]->GetFeaSSIndex() < 0 )
                    {
                        if ( m_FeaElementVec[j]->GetElementType() != FeaElement::FEA_BEAM )
                        {
                            m_FeaElementVec[j]->WriteNASTRAN( temp, elem_id, property_id, noffset, eoffset );
                            shell_elem_id_vec.push_back( elem_id );
                            FeaMeshMgr.MarkPropMatUsed( property_id );
                        }
                        else
                        {
                            m_FeaElementVec[j]->WriteNASTRAN( temp, elem_id, cap_property_id, noffset, eoffset );
                            beam_elem_id_vec.push_back( elem_id );
                            FeaMeshMgr.MarkPropMatUsed( cap_property_id );
                        }

                        elem_id++;
                    }
                }

                // Write shell element set
                name = m_FeaPartNameVec[i] + "_ShellElements";
                WriteNASTRANSet( fp, nkey_fp, set_cnt, shell_elem_id_vec, name, eoffset );

                // Write beam element set
                name = m_FeaPartNameVec[i] + "_BeamElements";
                WriteNASTRANSet( fp, nkey_fp, set_cnt, beam_elem_id_vec, name, eoffset );
            }
        }

        // Write FeaFixPoints
        for ( unsigned int i = 0; i < m_NumFeaFixPoints; i++ )
        {
            FixPoint fxpt = m_FixPntVec[i];
            if ( fxpt.m_PtMassFlag )
            {
                fprintf( temp, "\n" );
                fprintf( temp, "$%s\n", m_FeaPartNameVec[fxpt.m_FeaPartIndex].c_str() );

                vector < int > mass_elem_id_vec;

                for ( int j = 0; j < m_FeaElementVec.size(); j++ )
                {
                    if ( m_FeaElementVec[j]->GetElementType() == FeaElement::FEA_POINT_MASS && m_FeaElementVec[j]->GetFeaPartIndex() == fxpt.m_FeaPartIndex && m_FeaElementVec[j]->GetFeaSSIndex() < 0 )
                    {
                        m_FeaElementVec[j]->WriteNASTRAN( temp, elem_id, -1, noffset, eoffset ); // property ID ignored for Point Masses
                        mass_elem_id_vec.push_back( elem_id );
                        elem_id++;
                    }
                }

                // Write mass element set
                name = m_FeaPartNameVec[fxpt.m_FeaPartIndex] + "_MassElements";
                WriteNASTRANSet( fp, nkey_fp, set_cnt, mass_elem_id_vec, name, eoffset );
            }
        }

        // Write FeaSubSurfaces
        for ( unsigned int i = 0; i < m_NumFeaSubSurfs; i++ )
        {
            fprintf( temp, "\n" );
            fprintf( temp, "$%s\n", m_SimpleSubSurfaceVec[i].GetName().c_str() );

            int property_id = m_SimpleSubSurfaceVec[i].GetFeaPropertyIndex();
            int cap_property_id = m_SimpleSubSurfaceVec[i].GetCapFeaPropertyIndex();

            shell_elem_id_vec.clear();
            beam_elem_id_vec.clear();

            for ( int j = 0; j < m_FeaElementVec.size(); j++ )
            {
                if ( m_FeaElementVec[j]->GetFeaSSIndex() == i )
                {
                    if ( m_FeaElementVec[j]->GetElementType() != FeaElement::FEA_BEAM )
                    {
                        m_FeaElementVec[j]->WriteNASTRAN( temp, elem_id, property_id, noffset, eoffset );
                        shell_elem_id_vec.push_back( elem_id );
                        FeaMeshMgr.MarkPropMatUsed( property_id );
                    }
                    else
                    {
                        m_FeaElementVec[j]->WriteNASTRAN( temp, elem_id, cap_property_id, noffset, eoffset );
                        beam_elem_id_vec.push_back( elem_id );
                        FeaMeshMgr.MarkPropMatUsed( cap_property_id );
                    }

                    elem_id++;
                }
            }

            // Write shell element set
            name = m_SimpleSubSurfaceVec[i].GetName() + "_ShellElements";
            WriteNASTRANSet( fp, nkey_fp, set_cnt, shell_elem_id_vec, name, eoffset );

            // Write beam element set
            name = m_SimpleSubSurfaceVec[i].GetName() + "_BeamElements";
            WriteNASTRANSet( fp, nkey_fp, set_cnt, beam_elem_id_vec, name, eoffset );
        }

        //==== Properties ====//
        fprintf( temp, "\n" );
        fprintf( temp, "$Properties\n" );

        for ( unsigned int i = 0; i < FeaMeshMgr.GetSimplePropertyVec().size(); i++ )
        {
            FeaMeshMgr.GetSimplePropertyVec()[i].WriteNASTRAN( temp, i + 1 );
        }

        //==== Materials ====//
        fprintf( temp, "\n" );
        fprintf( temp, "$Materials\n" );

        for ( unsigned int i = 0; i < FeaMeshMgr.GetSimpleMaterialVec().size(); i++ )
        {
            FeaMeshMgr.GetSimpleMaterialVec()[i].WriteNASTRAN( temp, i + 1 );
        }

        fprintf( temp, "\nENDDATA\n" );

        // Obtain file size:
        fseek( temp, 0, SEEK_END );
        long lSize = ftell( temp );
        rewind( temp );

        // Allocate memory to contain the whole file:
        char * buffer = (char*)malloc( sizeof( char )*lSize + 1 );
        if ( buffer == NULL )
        {
            printf( "WriteNASTRAN memory error\n" );
        }

        // Copy the file into the buffer:
        size_t result = fread( buffer, 1, lSize, temp );
        buffer[ result ] = '\0';
        if ( result != lSize )
        {
            printf( "WriteNASTRAN reading error\n" );
        }

        // The whole file is now loaded in the memory buffer. Write to NASTRAN file
        fprintf( fp, "%s", buffer );

        // Close open files and free memory
        fclose( fp );
        fclose( temp );
        free( buffer );

        if ( nkey_fp )
        {
            fclose( nkey_fp );
        }
    }
}

void FeaMesh::WriteNASTRANSet( FILE* Nastran_fid, FILE* NKey_fid, int & set_num, vector < int > set_ids, const string &set_name, const int &offset )
{
    if ( set_ids.size() > 0 && Nastran_fid )
    {
        fprintf( Nastran_fid, "\n$ %d, %s\n", set_num, set_name.c_str() );
        fprintf( Nastran_fid, "SET %d = ", set_num );

        for ( size_t i = 0; i < set_ids.size(); i++ )
        {
            fprintf( Nastran_fid, "%d", set_ids[i] + offset );

            if ( i != set_ids.size() - 1 )
            {
                fprintf( Nastran_fid, "," );

                if ( ( i + 1 ) % 9 == 0 ) // 9 IDs per line
                {
                    fprintf( Nastran_fid, "\n" );
                }
            }
        }

        fprintf( Nastran_fid, "\n" );

        if ( NKey_fid ) // Write to NASTRAN key file if defined
        {
            fprintf( NKey_fid, "%d,%s\n", set_num, set_name.c_str() );
        }

        set_num++; // Increment set identification number
    }
}

void FeaMesh::WriteCalculix()
{
    string fn = GetStructSettingsPtr()->GetExportFileName( vsp::FEA_CALCULIX_FILE_NAME );
    FILE* fp = fopen( fn.c_str(), "w" );

    if ( fp )
    {
        WriteCalculix( fp );
        fclose( fp );
    }
}

void FeaMesh::WriteCalculix( FILE* fp )
{
    FeaMeshMgr.ResetPropMatUse();

    if ( fp )
    {
        fprintf( fp, "** Calculix structure data file generated from %s\n", VSPVERSION4 );
        WriteCalculixHeader( fp );
        WriteCalculixNodes( fp );
        WriteCalculixElements( fp );
        WriteCalculixBCs( fp );
        WriteCalculixProperties( fp );
        FeaMeshMgr.WriteCalculixMaterials( fp );
    }
}

void FeaMesh::WriteCalculixHeader( FILE* fp )
{
    if ( fp )
    {
        fprintf( fp, "** %s\n", m_StructName.c_str() );
        fprintf( fp, "** Num_Nodes:       %u\n", m_NumNodes );
        fprintf( fp, "** Num_Els:         %u\n", m_NumEls );
        fprintf( fp, "** Num_Tris:        %u\n", m_NumTris );
        fprintf( fp, "** Num_Quads:       %u\n", m_NumQuads );
        fprintf( fp, "** Num_Beams:       %u\n", m_NumBeams );
        fprintf( fp, "** Node_Offset:     %u\n", m_StructSettings.m_NodeOffset );
        fprintf( fp, "** Element_Offset:  %u\n", m_StructSettings.m_ElementOffset );
        fprintf( fp, "\n" );
    }
}

void FeaMesh::WriteCalculixNodes( FILE* fp )
{
    int noffset = m_StructSettings.m_NodeOffset;

    // This code does not currently support mixed quads and tris.
    // Element sets must be made unique.  Properties and orientation should only be written for
    // element sets that are actually used.

    if ( fp )
    {
        int elem_id = 0;
        char str[256];

        //==== Write nodes from FeaParts ====//
        for ( unsigned int i = 0; i < m_NumFeaParts; i++ )
        {
            if ( m_FeaPartTypeVec[i] != vsp::FEA_FIX_POINT )
            {
                fprintf( fp, "** %s %s\n", m_FeaPartNameVec[i].c_str(), m_StructName.c_str() );
                fprintf( fp, "*NODE, NSET=N%s_%s\n", m_FeaPartNameVec[i].c_str(), m_StructName.c_str() );

                for ( unsigned int j = 0; j < (int)m_FeaNodeVec.size(); j++ )
                {
                    if ( m_PntShift[j] >= 0 )
                    {
                        if ( m_FeaNodeVec[ j ]->HasOnlyTag( i ) )
                        {
                            m_FeaNodeVec[j]->WriteCalculix( fp, noffset );
                        }
                    }
                }

                fprintf( fp, "\n" );
            }
        }

        //==== Write Fixed Points ====//
        for ( size_t i = 0; i < m_NumFeaFixPoints; i++ )
        {
            FixPoint fxpt = m_FixPntVec[i];

            fprintf( fp, "** Fixed Point: %s %s\n", m_FeaPartNameVec[fxpt.m_FeaPartIndex].c_str(), m_StructName.c_str() );
            fprintf( fp, "*NODE, NSET=N%s_%s\n", m_FeaPartNameVec[fxpt.m_FeaPartIndex].c_str(), m_StructName.c_str() );

            // for ( unsigned int j = 0; j < (int)m_FeaNodeVec.size(); j++ )
            // {
            //     if ( m_PntShift[j] >= 0 )
            //     {
            //         if ( m_FeaNodeVec[j]->m_Tags.size() > 1 &&
            //              m_FeaNodeVec[j]->m_FixedPointFlag &&
            //              m_FeaNodeVec[j]->HasTag( fxpt.m_FeaPartIndex ) )
            //         {
            //             m_FeaNodeVec[j]->WriteCalculix( fp, noffset );
            //         }
            //     }
            // }

            for ( unsigned int j = 0; j < (int)fxpt.m_Pnt.size(); j++ )
            {
                if ( fxpt.m_NodeIndex[j] >= 0 )
                {
                    fprintf( fp, "%d,%f,%f,%f\n", fxpt.m_NodeIndex[j] + noffset, fxpt.m_Pnt[j][0], fxpt.m_Pnt[j][1], fxpt.m_Pnt[j][2] );
                }
            }

            fprintf( fp, "\n" );
        }

        //==== Write SubSurfaces ====//
        for ( unsigned int i = 0; i < m_NumFeaSubSurfs; i++ )
        {
            fprintf( fp, "** %s %s\n", m_SimpleSubSurfaceVec[i].GetName().c_str(), m_StructName.c_str() );
            fprintf( fp, "*NODE, NSET=N%s_%s\n", m_SimpleSubSurfaceVec[i].GetName().c_str(), m_StructName.c_str() );

            for ( unsigned int j = 0; j < (int)m_FeaNodeVec.size(); j++ )
            {
                if ( m_PntShift[j] >= 0 )
                {
                    if ( m_FeaNodeVec[ j ]->HasOnlyTag( i + m_NumFeaParts ) )
                    {
                        m_FeaNodeVec[j]->WriteCalculix( fp, noffset );
                    }
                }
            }
            fprintf( fp, "\n" );
        }

        //==== Intersection Nodes ====//
        bool IntersectHeader = false;
        for ( unsigned int j = 0; j < (int)m_FeaNodeVec.size(); j++ )
        {
            if ( m_PntShift[j] >= 0 )
            {
                if ( m_FeaNodeVec[j]->m_Tags.size() > 1 &&
                     !m_FeaNodeVec[j]->m_FixedPointFlag )
                {
                    if ( !IntersectHeader )
                    {
                        fprintf( fp, "** Intersections %s\n", m_StructName.c_str() );
                        fprintf( fp, "*NODE, NSET=Nintersections_%s\n", m_StructName.c_str() );
                        IntersectHeader = true;
                    }
                    m_FeaNodeVec[j]->WriteCalculix( fp, noffset );
                }
            }
        }
        if ( IntersectHeader )
        {
            fprintf( fp, "\n" );
        }

        //==== Remaining Nodes ====//
        bool RemainingHeader = false;
        for ( int i = 0; i < (int)m_FeaNodeVec.size(); i++ )
        {
            if ( m_PntShift[i] >= 0 &&
                 m_FeaNodeVec[i]->m_Tags.size() == 0 )
            {
                if ( !RemainingHeader )
                {
                    fprintf( fp, "** Remaining Nodes %s\n", m_StructName.c_str() );
                    fprintf( fp, "*NODE, NSET=RemainingNodes_%s\n", m_StructName.c_str() );
                    RemainingHeader = true;
                }
                m_FeaNodeVec[i]->WriteCalculix( fp, noffset );
            }
        }
        if ( RemainingHeader )
        {
            fprintf( fp, "\n" );
        }
    }
}

void FeaMesh::WriteCalculixElements( FILE* fp )
{
    int noffset = m_StructSettings.m_NodeOffset;
    int eoffset = m_StructSettings.m_ElementOffset;

    // This code does not currently support mixed quads and tris.
    // Element sets must be made unique.  Properties and orientation should only be written for
    // element sets that are actually used.

    if ( fp )
    {
        int elem_id = 0;
        char str[256];

        //==== Write elements from FeaParts ====//
        for ( unsigned int i = 0; i < m_NumFeaParts; i++ )
        {
            if ( m_FeaPartTypeVec[i] != vsp::FEA_FIX_POINT )
            {
                fprintf( fp, "** %s %s\n", m_FeaPartNameVec[i].c_str(), m_StructName.c_str() );

                int surf_num = m_FeaPartNumSurfVec[i];

                for ( int isurf = 0; isurf < surf_num; isurf++ )
                {
                    if ( !m_StructSettings.m_ConvertToQuadsFlag && ( m_FeaPartIncludedElementsVec[i] == vsp::FEA_SHELL || m_FeaPartIncludedElementsVec[i] == vsp::FEA_SHELL_AND_BEAM ) )
                    {
                        int nnode = 3;
                        if ( m_StructSettings.m_HighOrderElementFlag ) nnode = 6;

                        fprintf( fp, "*ELEMENT, TYPE=S%d, ELSET=E%s_%s_%d\n", nnode, m_FeaPartNameVec[i].c_str(), m_StructName.c_str(), isurf );

                        for ( int j = 0; j < m_FeaElementVec.size(); j++ )
                        {
                            if ( m_FeaElementVec[j]->GetFeaPartIndex() == i &&
                                 ( m_FeaElementVec[j]->GetElementType() == FeaElement::FEA_TRI_3 || m_FeaElementVec[j]->GetElementType() == FeaElement::FEA_TRI_6 ) &&
                                 m_FeaElementVec[j]->GetFeaSSIndex() < 0 &&
                                 m_FeaElementVec[j]->GetFeaPartSurfNum() == isurf )
                            {
                                elem_id++;
                                m_FeaElementVec[j]->WriteCalculix( fp, elem_id, noffset, eoffset );
                            }
                        }
                        fprintf( fp, "\n" );
                    }

                    if ( m_StructSettings.m_ConvertToQuadsFlag && ( m_FeaPartIncludedElementsVec[i] == vsp::FEA_SHELL || m_FeaPartIncludedElementsVec[i] == vsp::FEA_SHELL_AND_BEAM ) )
                    {
                        int nnode = 4;
                        if ( m_StructSettings.m_HighOrderElementFlag ) nnode = 8;

                        fprintf( fp, "*ELEMENT, TYPE=S%d, ELSET=E%s_%s_%d\n", nnode, m_FeaPartNameVec[i].c_str(), m_StructName.c_str(), isurf );

                        for ( int j = 0; j < m_FeaElementVec.size(); j++ )
                        {
                            if ( m_FeaElementVec[j]->GetFeaPartIndex() == i &&
                                 ( m_FeaElementVec[j]->GetElementType() == FeaElement::FEA_QUAD_4 || m_FeaElementVec[j]->GetElementType() == FeaElement::FEA_QUAD_8 ) &&
                                 m_FeaElementVec[j]->GetFeaSSIndex() < 0 &&
                                 m_FeaElementVec[j]->GetFeaPartSurfNum() == isurf )
                            {
                                elem_id++;
                                m_FeaElementVec[j]->WriteCalculix( fp, elem_id, noffset, eoffset );
                            }
                        }
                        fprintf( fp, "\n" );
                    }

                    if ( m_FeaPartIncludedElementsVec[i] == vsp::FEA_BEAM || m_FeaPartIncludedElementsVec[i] == vsp::FEA_SHELL_AND_BEAM )
                    {
                        fprintf( fp, "*ELEMENT, TYPE=B32, ELSET=EB%s_%s_%d_CAP\n", m_FeaPartNameVec[i].c_str(), m_StructName.c_str(), isurf );

                        for ( int j = 0; j < m_FeaElementVec.size(); j++ )
                        {
                            if ( m_FeaElementVec[j]->GetFeaPartIndex() == i &&
                                 m_FeaElementVec[j]->GetElementType() == FeaElement::FEA_BEAM &&
                                 m_FeaElementVec[j]->GetFeaSSIndex() < 0 &&
                                 m_FeaElementVec[j]->GetFeaPartSurfNum() == isurf )
                            {
                                elem_id++;
                                m_FeaElementVec[j]->WriteCalculix( fp, elem_id, noffset, eoffset );
                            }
                        }

                        // Write Normal Vectors
                        fprintf( fp, "\n" );
                        fprintf( fp, "*NORMAL\n" );

                        for ( int j = 0; j < m_FeaElementVec.size(); j++ )
                        {
                            if ( m_FeaElementVec[j]->GetFeaPartIndex() == i &&
                                 m_FeaElementVec[j]->GetElementType() == FeaElement::FEA_BEAM &&
                                 m_FeaElementVec[j]->GetFeaSSIndex() < 0 &&
                                 m_FeaElementVec[j]->GetFeaPartSurfNum() == isurf )
                            {
                                FeaBeam* beam = dynamic_cast<FeaBeam*>( m_FeaElementVec[j] );
                                assert( beam );
                                beam->WriteCalculixNormal( fp, noffset, eoffset );
                            }
                        }

                        fprintf( fp, "\n" );
                    }
                }
            }
        }

        //==== Write Fixed Points ====//
        for ( size_t i = 0; i < m_NumFeaFixPoints; i++ )
        {
            FixPoint fxpt = m_FixPntVec[i];

            if ( fxpt.m_PtMassFlag )
            {
                fprintf( fp, "\n" );
                fprintf( fp, "** Fixed Point: %s %s\n", m_FeaPartNameVec[fxpt.m_FeaPartIndex].c_str(), m_StructName.c_str() );
                fprintf( fp, "*ELEMENT, TYPE=MASS, ELSET=EP%s_%s\n", m_FeaPartNameVec[fxpt.m_FeaPartIndex].c_str(), m_StructName.c_str() );

                for ( int j = 0; j < m_FeaElementVec.size(); j++ )
                {
                    if ( m_FeaElementVec[j]->GetFeaPartIndex() == fxpt.m_FeaPartIndex &&
                         m_FeaElementVec[j]->GetElementType() == FeaElement::FEA_POINT_MASS &&
                         m_FeaElementVec[j]->GetFeaSSIndex() < 0 )
                    {
                        elem_id++;
                        m_FeaElementVec[j]->WriteCalculix( fp, elem_id, noffset, eoffset );
                    }
                }

                fprintf( fp, "\n" );

                fprintf( fp, "*MASS, ELSET=EP%s_%s\n", m_FeaPartNameVec[fxpt.m_FeaPartIndex].c_str(), m_StructName.c_str() );
                fprintf( fp, "%f\n", fxpt.m_PtMass );
                fprintf( fp, "\n" );
            }
        }

        //==== Write SubSurfaces ====//
        for ( unsigned int i = 0; i < m_NumFeaSubSurfs; i++ )
        {
            int surf_num = m_SimpleSubSurfaceVec[i].GetFeaOrientationVec().size();

            for ( int isurf = 0; isurf < surf_num; isurf++ )
            {
                if ( !m_StructSettings.m_ConvertToQuadsFlag & ( m_SimpleSubSurfaceVec[i].m_IncludedElements == vsp::FEA_SHELL || m_SimpleSubSurfaceVec[i].m_IncludedElements == vsp::FEA_SHELL_AND_BEAM ) )
                {
                    int nnode = 3;
                    if ( m_StructSettings.m_HighOrderElementFlag ) nnode = 6;

                    fprintf( fp, "\n*ELEMENT, TYPE=S%d, ELSET=E%s_%s_%d\n", nnode, m_SimpleSubSurfaceVec[i].GetName().c_str(), m_StructName.c_str(), isurf );

                    for ( int j = 0; j < m_FeaElementVec.size(); j++ )
                    {
                        if ( m_FeaElementVec[j]->GetFeaSSIndex() == i &&
                             ( m_FeaElementVec[j]->GetElementType() == FeaElement::FEA_TRI_3 || m_FeaElementVec[j]->GetElementType() == FeaElement::FEA_TRI_6 ) &&
                             m_FeaElementVec[j]->GetFeaPartSurfNum() == isurf )
                        {
                            elem_id++;
                            m_FeaElementVec[j]->WriteCalculix( fp, elem_id, noffset, eoffset );
                        }
                    }
                    fprintf( fp, "\n" );
                }

                if ( m_StructSettings.m_ConvertToQuadsFlag && ( m_SimpleSubSurfaceVec[i].m_IncludedElements == vsp::FEA_SHELL || m_SimpleSubSurfaceVec[i].m_IncludedElements == vsp::FEA_SHELL_AND_BEAM ) )
                {
                    int nnode = 4;
                    if ( m_StructSettings.m_HighOrderElementFlag ) nnode = 8;

                    fprintf( fp, "\n*ELEMENT, TYPE=S%d, ELSET=E%s_%s_%d\n", nnode, m_SimpleSubSurfaceVec[i].GetName().c_str(), m_StructName.c_str(), isurf );

                    for ( int j = 0; j < m_FeaElementVec.size(); j++ )
                    {
                        if ( m_FeaElementVec[j]->GetFeaSSIndex() == i &&
                             ( m_FeaElementVec[j]->GetElementType() == FeaElement::FEA_QUAD_4 || m_FeaElementVec[j]->GetElementType() == FeaElement::FEA_QUAD_8 ) &&
                             m_FeaElementVec[j]->GetFeaPartSurfNum() == isurf )
                        {
                            elem_id++;
                            m_FeaElementVec[j]->WriteCalculix( fp, elem_id, noffset, eoffset );
                        }
                    }
                    fprintf( fp, "\n" );
                }

                if ( m_SimpleSubSurfaceVec[i].m_IncludedElements == vsp::FEA_BEAM || m_SimpleSubSurfaceVec[i].m_IncludedElements == vsp::FEA_SHELL_AND_BEAM )
                {
                    fprintf( fp, "\n" );
                    fprintf( fp, "*ELEMENT, TYPE=B32, ELSET=EB%s_%s_%d_CAP\n", m_SimpleSubSurfaceVec[i].GetName().c_str(), m_StructName.c_str(), isurf );

                    for ( int j = 0; j < m_FeaElementVec.size(); j++ )
                    {
                        if ( m_FeaElementVec[j]->GetFeaSSIndex() == i &&
                             m_FeaElementVec[j]->GetElementType() == FeaElement::FEA_BEAM &&
                             m_FeaElementVec[j]->GetFeaPartSurfNum() == isurf )
                        {
                            elem_id++;
                            m_FeaElementVec[j]->WriteCalculix( fp, elem_id, noffset, eoffset );
                        }
                    }

                    // Write Normal Vectors
                    fprintf( fp, "\n" );
                    fprintf( fp, "*NORMAL\n" );

                    for ( int j = 0; j < m_FeaElementVec.size(); j++ )
                    {
                        if ( m_FeaElementVec[j]->GetFeaSSIndex() == i &&
                             m_FeaElementVec[j]->GetElementType() == FeaElement::FEA_BEAM &&
                             m_FeaElementVec[j]->GetFeaPartSurfNum() == isurf )
                        {
                            FeaBeam* beam = dynamic_cast<FeaBeam*>( m_FeaElementVec[j] );
                            assert( beam );
                            beam->WriteCalculixNormal( fp, noffset, eoffset );
                        }
                    }

                    fprintf( fp, "\n" );
                }
            }
        }
    }
}

void FeaMesh::WriteCalculixBCs( FILE* fp )
{
    int noffset = m_StructSettings.m_NodeOffset;

    if ( fp )
    {
        if ( m_BCVec.size() > 0 )
        {
            fprintf( fp, "*BOUNDARY\n" );
        }

        for ( unsigned int j = 0; j < (int)m_FeaNodeVec.size(); j++ )
        {
            m_FeaNodeVec[j]->WriteCalculixBCs( fp, noffset );
        }

        if ( m_BCVec.size() > 0 )
        {
            fprintf( fp, "\n" );
        }
    }
}

void FeaMesh::WriteCalculixProperties( FILE* fp )
{
    if ( fp )
    {
        char str[256];

        fprintf( fp, "** Properties %s\n", m_StructName.c_str() );

        //==== FeaProperties ====//
        for ( unsigned int i = 0; i < m_NumFeaParts; i++ )
        {
            if ( m_FeaPartTypeVec[i] != vsp::FEA_FIX_POINT )
            {
                int property_id = m_FeaPartPropertyIndexVec[i];
                int cap_property_id = m_FeaPartCapPropertyIndexVec[i];

                int surf_num = m_FeaPartNumSurfVec[i];

                for ( int isurf = 0; isurf < surf_num; isurf++ )
                {
                    if ( m_FeaPartIncludedElementsVec[i] == vsp::FEA_SHELL || m_FeaPartIncludedElementsVec[i] == vsp::FEA_SHELL_AND_BEAM )
                    {
                        FeaMeshMgr.MarkPropMatUsed( property_id );

                        fprintf( fp, "\n" );
                        char ostr[256];
                        sprintf( ostr, "O%s_%s_%d", m_FeaPartNameVec[i].c_str(), m_StructName.c_str(), isurf );

                        if ( !m_StructSettings.m_ConvertToQuadsFlag )
                        {
                            sprintf( str, "E%s_%s_%d", m_FeaPartNameVec[ i ].c_str(), m_StructName.c_str(), isurf );
                            FeaMeshMgr.GetSimplePropertyVec()[ property_id ].WriteCalculix( fp, str, ostr );
                        }
                        else
                        {
                            sprintf( str, "E%s_%s_%d", m_FeaPartNameVec[ i ].c_str(), m_StructName.c_str(), isurf );
                            FeaMeshMgr.GetSimplePropertyVec()[ property_id ].WriteCalculix( fp, str, ostr );
                        }

                        vec3d orient = m_PartSurfOrientation[i][isurf];
                        // int otype = srf->GetFeaOrientationType();
                        fprintf( fp, "\n" );
                        fprintf( fp, "*ORIENTATION, NAME=%s, SYSTEM=RECTANGULAR\n", ostr );
                        fprintf( fp, "%f,%f,%f,0.0,0.0,1.0\n", orient.x(), orient.y(), orient.z());
                    }

                    if ( m_FeaPartIncludedElementsVec[i] == vsp::FEA_BEAM || m_FeaPartIncludedElementsVec[i] == vsp::FEA_SHELL_AND_BEAM )
                    {
                        FeaMeshMgr.MarkPropMatUsed( cap_property_id );

                        fprintf( fp, "\n" );
                        sprintf( str, "EB%s_%s_%d_CAP", m_FeaPartNameVec[i].c_str(), m_StructName.c_str(), isurf );
                        FeaMeshMgr.GetSimplePropertyVec()[cap_property_id].WriteCalculix( fp, str, "" );
                    }
                }
            }
        }

        for ( unsigned int i = 0; i < m_NumFeaSubSurfs; i++ )
        {
            int property_id = m_SimpleSubSurfaceVec[i].GetFeaPropertyIndex();
            int cap_property_id = m_SimpleSubSurfaceVec[i].GetCapFeaPropertyIndex();

            std::vector < vec3d > ovec = m_SimpleSubSurfaceVec[i].GetFeaOrientationVec();
            int surf_num = ovec.size();
            for ( int isurf = 0; isurf < surf_num; isurf++ )
            {
                if ( m_SimpleSubSurfaceVec[i].m_IncludedElements == vsp::FEA_SHELL || m_SimpleSubSurfaceVec[i].m_IncludedElements == vsp::FEA_SHELL_AND_BEAM )
                {
                    FeaMeshMgr.MarkPropMatUsed( property_id );

                    fprintf( fp, "\n" );
                    char ostr[256];
                    sprintf( ostr, "O%s_%s_%d", m_SimpleSubSurfaceVec[i].GetName().c_str(), m_StructName.c_str(), isurf );

                    if ( !m_StructSettings.m_ConvertToQuadsFlag )
                    {
                        sprintf( str, "E%s_%s_%d", m_SimpleSubSurfaceVec[i].GetName().c_str(), m_StructName.c_str(), isurf );
                        FeaMeshMgr.GetSimplePropertyVec()[property_id].WriteCalculix( fp, str, ostr );
                    }
                    else
                    {
                        sprintf( str, "E%s_%s_%d", m_SimpleSubSurfaceVec[i].GetName().c_str(), m_StructName.c_str(), isurf );
                        FeaMeshMgr.GetSimplePropertyVec()[property_id].WriteCalculix( fp, str, ostr );
                    }

                    vec3d orient = ovec[isurf];
                    // int otype = m_SimpleSubSurfaceVec[i].GetFeaOrientationType();
                    fprintf( fp, "\n" );
                    fprintf( fp, "*ORIENTATION, NAME=%s, SYSTEM=RECTANGULAR\n", ostr );
                    fprintf( fp, "%f,%f,%f,0.0,0.0,1.0\n", orient.x(), orient.y(), orient.z() );
                }

                if ( m_SimpleSubSurfaceVec[i].m_IncludedElements == vsp::FEA_BEAM || m_SimpleSubSurfaceVec[i].m_IncludedElements == vsp::FEA_SHELL_AND_BEAM )
                {
                    FeaMeshMgr.MarkPropMatUsed( cap_property_id );

                    fprintf( fp, "\n" );
                    sprintf( str, "EB%s_%s_%d_CAP", m_SimpleSubSurfaceVec[i].GetName().c_str(), m_StructName.c_str(), isurf );
                    FeaMeshMgr.GetSimplePropertyVec()[cap_property_id].WriteCalculix( fp, str, "" );
                }
            }
        }

        fprintf( fp, "\n" );
    }
}

void FeaMesh::WriteGmsh()
{
    int noffset = m_StructSettings.m_NodeOffset;
    int eoffset = m_StructSettings.m_ElementOffset;

    string fn = GetStructSettingsPtr()->GetExportFileName( vsp::FEA_GMSH_FILE_NAME );
    FILE* fp = fopen( fn.c_str(), "w" );
    if ( fp )
    {
        //=====================================================================================//
        //============== Write Gmsh File ======================================================//
        //=====================================================================================//
        fprintf( fp, "$MeshFormat\n" );
        fprintf( fp, "2.2 0 %d\n", ( int )sizeof( double ) );
        fprintf( fp, "$EndMeshFormat\n" );

        // Count FeaNodes
        int node_count = 0;
        for ( unsigned int j = 0; j < (int)m_FeaNodeVec.size(); j++ )
        {
            if ( m_PntShift[j] >= 0 )
            {
                node_count++;
            }
        }

        //==== Group and Name FeaParts ====//
        fprintf( fp, "$PhysicalNames\n" );
        fprintf( fp, "%u\n", m_NumFeaParts - m_NumFeaFixPoints );
        for ( unsigned int i = 0; i < m_NumFeaParts; i++ )
        {
            if ( m_FeaPartTypeVec[i] != vsp::FEA_FIX_POINT )
            {
                fprintf( fp, "9 %d \"%s\"\n", i + 1, m_FeaPartNameVec[i].c_str() );
            }
        }
        fprintf( fp, "$EndPhysicalNames\n" );

        //==== Write Nodes ====//
        fprintf( fp, "$Nodes\n" );
        fprintf( fp, "%u\n", node_count );

        for ( unsigned int j = 0; j < (int)m_FeaNodeVec.size(); j++ )
        {
            if ( m_PntShift[j] >= 0 )
            {
                m_FeaNodeVec[j]->WriteGmsh( fp, noffset );
            }
        }

        fprintf( fp, "$EndNodes\n" );

        //==== Write FeaElements ====//
        fprintf( fp, "$Elements\n" );
        fprintf( fp, "%d\n", (int)m_FeaElementVec.size() );

        int ele_cnt = 1;

        for ( unsigned int j = 0; j < m_NumFeaParts; j++ )
        {
            for ( int i = 0; i < (int)m_FeaElementVec.size(); i++ )
            {
                if ( m_FeaElementVec[i]->GetFeaPartIndex() == j )
                {
                    m_FeaElementVec[i]->WriteGmsh( fp, ele_cnt, j + 1, noffset, eoffset );
                    ele_cnt++;
                }
            }
        }

        fprintf( fp, "$EndElements\n" );
        fclose( fp );

        // Note: Material properties are not supported in *.msh file
    }
}

void FeaMesh::WriteSTL()
{
    string fn = GetStructSettingsPtr()->GetExportFileName( vsp::FEA_STL_FILE_NAME );
    FILE* fp = fopen( fn.c_str(), "w" );
    if ( fp )
    {
        for ( unsigned int i = 0; i < m_NumFeaParts; i++ )
        {
            int surf_num = m_FeaPartNumSurfVec[i];

            for ( int isurf = 0; isurf < surf_num; isurf++ )
            {
                if ( m_FeaPartIncludedElementsVec[i] == vsp::FEA_SHELL || m_FeaPartIncludedElementsVec[i] == vsp::FEA_SHELL_AND_BEAM )
                {
                    fprintf( fp, "solid %s_%d\n", m_FeaPartNameVec[ i ].c_str(), isurf );

                    for ( int j = 0; j < m_FeaElementVec.size(); j++ )
                    {
                        if ( m_FeaElementVec[j]->GetFeaPartIndex() == i &&
                             m_FeaElementVec[j]->GetFeaSSIndex() < 0 &&
                             m_FeaElementVec[j]->GetFeaPartSurfNum() == isurf )
                        {
                            m_FeaElementVec[j]->WriteSTL( fp );
                        }
                    }

                    fprintf( fp, "endsolid %s_%d\n", m_FeaPartNameVec[ i ].c_str(), isurf );
                }
            }
        }

        for ( unsigned int i = 0; i < m_NumFeaSubSurfs; i++ )
        {
            int surf_num = m_SimpleSubSurfaceVec[i].GetFeaOrientationVec().size();

            for ( int isurf = 0; isurf < surf_num; isurf++ )
            {
                if ( m_SimpleSubSurfaceVec[i].m_IncludedElements == vsp::FEA_SHELL || m_SimpleSubSurfaceVec[i].m_IncludedElements == vsp::FEA_SHELL_AND_BEAM )
                {
                    fprintf( fp, "solid %s_%d\n", m_SimpleSubSurfaceVec[i].GetName().c_str(), isurf );

                    for ( int j = 0; j < m_FeaElementVec.size(); j++ )
                    {
                        if ( m_FeaElementVec[j]->GetFeaSSIndex() == i &&
                             m_FeaElementVec[j]->GetFeaPartSurfNum() == isurf )
                        {
                            m_FeaElementVec[j]->WriteSTL( fp );
                        }
                    }
                    fprintf( fp, "endsolid %s_%d\n", m_SimpleSubSurfaceVec[i].GetName().c_str(), isurf );
                }
            }
        }
    }
}

void FeaMesh::ComputeWriteMass()
{
    m_TotalMass = 0.0;

    FILE* fp = fopen( GetStructSettingsPtr()->GetExportFileName( vsp::FEA_MASS_FILE_NAME ).c_str(), "w" );
    if ( fp )
    {
        fprintf( fp, "...FEA Mesh...\n" );
        fprintf( fp, "Mass_Unit: %s\n", m_MassUnit.c_str() );
        fprintf( fp, "Num_Els: %u\n", m_NumEls );
        fprintf( fp, "Num_Tris: %u\n", m_NumTris );
        fprintf( fp, "Num_Quads: %u\n", m_NumQuads );
        fprintf( fp, "Num_Beams: %u\n", m_NumBeams );
        fprintf( fp, "\n" );

        if ( m_NumFeaParts > 0 )
        {
            fprintf( fp, "FeaPart_Name         Mass_Shells   Mass_Beams\n" );
        }

        // Iterate over each FeaPart index and calculate mass of each FeaElement if the current indexes match
        for ( unsigned int i = 0; i < m_NumFeaParts; i++ )
        {
            if ( m_FeaPartTypeVec[i] != vsp::FEA_FIX_POINT )
            {
                double shell_mass = 0;
                double beam_mass = 0;
                int property_id = m_FeaPartPropertyIndexVec[i];
                int cap_property_id = m_FeaPartCapPropertyIndexVec[i];

                for ( int j = 0; j < m_FeaElementVec.size(); j++ )
                {
                    if ( m_FeaElementVec[j]->GetFeaPartIndex() == i && m_FeaElementVec[j]->GetFeaSSIndex() < 0 &&
                         ( m_FeaElementVec[j]->GetElementType() == FeaElement::FEA_TRI_3 ||
                           m_FeaElementVec[j]->GetElementType() == FeaElement::FEA_TRI_6 ||
                           m_FeaElementVec[j]->GetElementType() == FeaElement::FEA_QUAD_4 ||
                           m_FeaElementVec[j]->GetElementType() == FeaElement::FEA_QUAD_8 ) )
                    {
                        shell_mass += m_FeaElementVec[j]->ComputeMass( property_id );
                    }
                    else if ( m_FeaElementVec[j]->GetFeaPartIndex() == i && m_FeaElementVec[j]->GetFeaSSIndex() < 0 && m_FeaElementVec[j]->GetElementType() == FeaElement::FEA_BEAM )
                    {
                        beam_mass += m_FeaElementVec[j]->ComputeMass( cap_property_id );
                    }
                }

                string name = m_FeaPartNameVec[i];

                fprintf( fp, "%-20s% -12.4f% -12.4f\n", name.c_str(), shell_mass, beam_mass );

                m_TotalMass += shell_mass + beam_mass;
            }
        }

        // Add point masses
        if ( m_NumFeaFixPoints > 0 )
        {
            fprintf( fp, "\n" );
            fprintf( fp, "PointMass_Name       Mass        X_loc       Y_loc       Z_loc\n" );
        }

        for ( unsigned int i = 0; i < m_NumFeaFixPoints; i++ )
        {
            FixPoint fxpt = m_FixPntVec[i];
            if ( fxpt.m_PtMassFlag )
            {
                double pnt_mass = 0;
                string name = m_FeaPartNameVec[fxpt.m_FeaPartIndex];
                vec3d pnt;

                for ( int j = 0; j < m_FeaElementVec.size(); j++ )
                {
                    if ( m_FeaElementVec[j]->GetElementType() == FeaElement::FEA_POINT_MASS && m_FeaElementVec[j]->GetFeaPartIndex() == fxpt.m_FeaPartIndex && m_FeaElementVec[j]->GetFeaSSIndex() < 0 )
                    {
                        pnt_mass += m_FeaElementVec[j]->ComputeMass( -1 ); // property ID ignored for point masses

                        vector < FeaNode* > node_vec;
                        m_FeaElementVec[j]->LoadNodes( node_vec );

                        if ( node_vec.size() > 0 )
                        {
                            pnt = node_vec[0]->m_Pnt;
                        }
                    }
                }

                fprintf( fp, "%-20s% -12.4f% -12.4f% -12.4f% -12.4f\n", name.c_str(), pnt_mass, pnt[0], pnt[1], pnt[2] );

                m_TotalMass += pnt_mass;
            }
        }

        // Iterate over each FeaSubSurface index and calculate mass of each FeaElement if the subsurface indexes match
        if ( m_NumFeaSubSurfs > 0 )
        {
            fprintf( fp, "\n" );
            fprintf( fp, "FeaSubSurf_Name      Mass_Shells   Mass_Beams\n" );
        }

        for ( unsigned int i = 0; i < m_NumFeaSubSurfs; i++ )
        {
            double shell_mass = 0;
            double beam_mass = 0;
            int property_id = m_SimpleSubSurfaceVec[i].GetFeaPropertyIndex();
            int cap_property_id = m_SimpleSubSurfaceVec[i].GetCapFeaPropertyIndex();

            for ( int j = 0; j < m_FeaElementVec.size(); j++ )
            {
                if ( m_FeaElementVec[j]->GetFeaSSIndex() == i &&
                     ( m_FeaElementVec[j]->GetElementType() == FeaElement::FEA_TRI_3 ||
                       m_FeaElementVec[j]->GetElementType() == FeaElement::FEA_TRI_6 ||
                       m_FeaElementVec[j]->GetElementType() == FeaElement::FEA_QUAD_4 ||
                       m_FeaElementVec[j]->GetElementType() == FeaElement::FEA_QUAD_8 ) )
                {
                    shell_mass += m_FeaElementVec[j]->ComputeMass( property_id );
                }
                else if ( m_FeaElementVec[j]->GetFeaSSIndex() == i && m_FeaElementVec[j]->GetElementType() == FeaElement::FEA_BEAM )
                {
                    beam_mass += m_FeaElementVec[j]->ComputeMass( cap_property_id );
                }
            }

            string name = m_SimpleSubSurfaceVec[i].GetName();

            fprintf( fp, "%-20s% -12.4f% -12.4f\n", name.c_str(), shell_mass, beam_mass );

            m_TotalMass += shell_mass + beam_mass;
        }

        fprintf( fp, "\n" );
        fprintf( fp, "FeaStruct_Name       Total_Mass\n" );
        fprintf( fp, "%-20s% -9.4f\n", m_StructName.c_str(), m_TotalMass );

        fclose( fp );
    }
}

FixPoint* FeaMesh::GetFixPointByID( const string &id )
{
    for ( int i = 0; i < m_FixPntVec.size(); i++ )
    {
        if ( m_FeaPartIDVec[ m_FixPntVec[i].m_FeaPartIndex ] == id )
        {
            return &m_FixPntVec[i];
        }
    }
    return NULL;
}

void FeaMesh::UpdateDisplaySettings()
{
    FeaStructure *fea_struct = StructureMgr.GetFeaStruct( GetID() );
    if ( GetStructSettingsPtr() && fea_struct )
    {
        GetStructSettingsPtr()->m_DrawMeshFlag = fea_struct->GetStructSettingsPtr()->m_DrawMeshFlag.Get();
        GetStructSettingsPtr()->m_ColorTagsFlag = fea_struct->GetStructSettingsPtr()->m_ColorTagsFlag.Get();

        GetStructSettingsPtr()->m_DrawNodesFlag = fea_struct->GetStructSettingsPtr()->m_DrawNodesFlag.Get();
        GetStructSettingsPtr()->m_DrawElementOrientVecFlag = fea_struct->GetStructSettingsPtr()->m_DrawElementOrientVecFlag.Get();

        GetStructSettingsPtr()->m_DrawBorderFlag = fea_struct->GetStructSettingsPtr()->m_DrawBorderFlag.Get();
        GetStructSettingsPtr()->m_DrawIsectFlag = fea_struct->GetStructSettingsPtr()->m_DrawIsectFlag.Get();
        GetStructSettingsPtr()->m_DrawRawFlag = fea_struct->GetStructSettingsPtr()->m_DrawRawFlag.Get();
        GetStructSettingsPtr()->m_DrawBinAdaptFlag = fea_struct->GetStructSettingsPtr()->m_DrawBinAdaptFlag.Get();
        GetStructSettingsPtr()->m_DrawCurveFlag = fea_struct->GetStructSettingsPtr()->m_DrawCurveFlag.Get();
        GetStructSettingsPtr()->m_DrawPntsFlag = fea_struct->GetStructSettingsPtr()->m_DrawPntsFlag.Get();
    }
}
