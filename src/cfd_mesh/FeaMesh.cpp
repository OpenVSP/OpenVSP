//
// Created by Rob McDonald on 9/13/22.
//

#include "FeaMesh.h"

FeaMesh::FeaMesh()
{
    m_NumFeaParts = 0;
    m_NumFeaSubSurfs = 0;
    m_NumFeaFixPoints = 0;

    m_NumEls = 0;
    m_NumTris = 0;
    m_NumQuads = 0;
    m_NumBeams = 0;

    m_ID = "FEAMESH"; // Replace with random UUID

    m_FeaGridDensityPtr = NULL;
    m_StructSettingsPtr = NULL;
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

    m_FixPntVec.clear();

    m_IndMap.clear();
    m_PntShift.clear();

    m_NumFeaParts = 0;
    m_NumFeaSubSurfs = 0;
    m_NumFeaFixPoints = 0;

    m_NumEls = 0;
    m_NumTris = 0;
    m_NumQuads = 0;
    m_NumBeams = 0;

    m_DrawBrowserNameVec.clear();
    m_DrawBrowserPartIndexVec.clear();
    m_DrawElementFlagVec.clear();
    m_FixPointFeaPartFlagVec.clear();
    m_DrawCapFlagVec.clear();

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


void FeaMesh::LoadDrawObjs( vector< DrawObj* > &draw_obj_vec )
{
    if ( m_DrawElementFlagVec.size() == m_NumFeaParts + m_NumFeaSubSurfs )
    {
        // Calculate constants for color sequence.
        const int ncgrp = 6; // Number of basic colors
        const double ncstep = (int)ceil( (double)( 4 * ( m_NumFeaParts + m_NumFeaSubSurfs ) ) / (double)ncgrp );
        const double nctodeg = 360.0 / ( ncgrp*ncstep );

        for ( int iprt = 0; iprt < m_NumFeaParts; iprt++ )
        {
            m_FeaNodeDO[iprt].m_Type = DrawObj::VSP_POINTS;
            m_FeaNodeDO[iprt].m_PointSize = 4.0;

            if ( m_FixPointFeaPartFlagVec[iprt] )
            {
                m_FeaTriElementDO[iprt].m_Type = DrawObj::VSP_POINTS;
                m_FeaTriElementDO[iprt].m_PointSize = 7.0;

                m_FeaQuadElementDO[iprt].m_Type = DrawObj::VSP_POINTS;
                m_FeaQuadElementDO[iprt].m_PointSize = 7.0;
            }
            else
            {
                m_FeaTriElementDO[iprt].m_Type = DrawObj::VSP_SHADED_TRIS;
                m_FeaQuadElementDO[iprt].m_Type = DrawObj::VSP_SHADED_QUADS;
            }
            m_CapFeaElementDO[iprt].m_Type = DrawObj::VSP_LINES;
            m_CapFeaElementDO[iprt].m_LineWidth = 3.0;

            if ( GetStructSettingsPtr()->m_DrawMeshFlag ||
                 GetStructSettingsPtr()->m_ColorTagsFlag )   // At least mesh or tags are visible.
            {
                if ( !m_FixPointFeaPartFlagVec[iprt] )
                {
                    if ( GetStructSettingsPtr()->m_DrawMeshFlag &&
                         GetStructSettingsPtr()->m_ColorTagsFlag ) // Both are visible.
                    {
                        m_FeaTriElementDO[iprt].m_Type = DrawObj::VSP_WIRE_SHADED_TRIS;
                        m_FeaTriElementDO[iprt].m_LineColor = vec3d( 0.4, 0.4, 0.4 );

                        m_FeaQuadElementDO[iprt].m_Type = DrawObj::VSP_WIRE_SHADED_QUADS;
                        m_FeaQuadElementDO[iprt].m_LineColor = vec3d( 0.4, 0.4, 0.4 );
                    }
                    else if ( GetStructSettingsPtr()->m_DrawMeshFlag ) // Mesh only
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

            if ( GetStructSettingsPtr()->m_ColorTagsFlag )
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

            m_FeaTriElementDO[iprt].m_Visible = ( GetStructSettingsPtr()->m_DrawMeshFlag || GetStructSettingsPtr()->m_ColorTagsFlag ) && m_DrawElementFlagVec[iprt];
            m_FeaQuadElementDO[iprt].m_Visible = ( GetStructSettingsPtr()->m_DrawMeshFlag || GetStructSettingsPtr()->m_ColorTagsFlag ) && m_DrawElementFlagVec[iprt];
            m_FeaNodeDO[ iprt ].m_Visible = GetStructSettingsPtr()->m_DrawNodesFlag && m_DrawElementFlagVec[iprt];
            m_CapFeaElementDO[ iprt ].m_Visible = m_DrawCapFlagVec[ iprt ];
            m_ElOrientationDO[iprt].m_Visible = GetStructSettingsPtr()->m_DrawElementOrientVecFlag && m_DrawElementFlagVec[iprt];
            m_CapNormDO[iprt].m_Visible = GetStructSettingsPtr()->m_DrawElementOrientVecFlag && m_DrawCapFlagVec[iprt];

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
            m_SSFeaNodeDO[iss].m_PointSize = 3.0;
            m_SSTriElementDO[iss].m_Type = DrawObj::VSP_SHADED_TRIS;
            m_SSQuadElementDO[iss].m_Type = DrawObj::VSP_SHADED_QUADS;
            m_SSCapFeaElementDO[iss].m_Type = DrawObj::VSP_LINES;
            m_SSCapFeaElementDO[iss].m_LineWidth = 3.0;

            if ( GetStructSettingsPtr()->m_DrawMeshFlag ||
                 GetStructSettingsPtr()->m_ColorTagsFlag )   // At least mesh or tags are visible.
            {

                if ( GetStructSettingsPtr()->m_DrawMeshFlag &&
                     GetStructSettingsPtr()->m_ColorTagsFlag ) // Both are visible.
                {
                    m_SSTriElementDO[iss].m_Type = DrawObj::VSP_WIRE_SHADED_TRIS;
                    m_SSTriElementDO[iss].m_LineColor = vec3d( 0.4, 0.4, 0.4 );
                    m_SSQuadElementDO[iss].m_Type = DrawObj::VSP_WIRE_SHADED_QUADS;
                    m_SSQuadElementDO[iss].m_LineColor = vec3d( 0.4, 0.4, 0.4 );
                }
                else if ( GetStructSettingsPtr()->m_DrawMeshFlag ) // Mesh only
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

            if ( GetStructSettingsPtr()->m_ColorTagsFlag )
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

            m_SSTriElementDO[ iss ].m_Visible = ( GetStructSettingsPtr()->m_DrawMeshFlag || GetStructSettingsPtr()->m_ColorTagsFlag ) && m_DrawElementFlagVec[ iss + m_NumFeaParts ];
            m_SSQuadElementDO[ iss ].m_Visible = ( GetStructSettingsPtr()->m_DrawMeshFlag || GetStructSettingsPtr()->m_ColorTagsFlag ) && m_DrawElementFlagVec[ iss + m_NumFeaParts ];

            m_SSFeaNodeDO[ iss ].m_Visible = GetStructSettingsPtr()->m_DrawNodesFlag && m_DrawElementFlagVec[ iss + m_NumFeaParts];
            m_SSCapFeaElementDO[ iss ].m_Visible = m_DrawCapFlagVec[ iss + m_NumFeaParts];
            m_SSElOrientationDO[ iss ].m_Visible = GetStructSettingsPtr()->m_DrawElementOrientVecFlag && m_DrawElementFlagVec[ iss + m_NumFeaParts ];
            m_SSCapNormDO[ iss ].m_Visible = GetStructSettingsPtr()->m_DrawElementOrientVecFlag && m_DrawCapFlagVec[ iss + m_NumFeaParts ];

            draw_obj_vec.push_back( &m_SSFeaNodeDO[iss] );
            draw_obj_vec.push_back( &m_SSTriElementDO[iss] );
            draw_obj_vec.push_back( &m_SSQuadElementDO[iss] );
            draw_obj_vec.push_back( &m_SSCapFeaElementDO[iss] );
            draw_obj_vec.push_back( &m_SSElOrientationDO[iss] );
            draw_obj_vec.push_back( &m_SSCapNormDO[iss] );
        }
    }
}
