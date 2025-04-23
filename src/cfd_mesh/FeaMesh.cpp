//
// Created by Rob McDonald on 9/13/22.
//

#include "FeaMesh.h"
#include "main.h"  // For version numbers
#include "StructureMgr.h"
#include "FeaMeshMgr.h"
#include "FileUtil.h"
#include "StringUtil.h"

// Although this appears to be an angle comparison (via the dot product), it is actually the signed distance
// between the point and the plane.  Hence, a comparison to the mesh minimum length as a tolerance is appropriate.
bool PartTrim::CullPtByTrimGroup( const vec3d &pt, int isymm, double tol )
{
    if ( isymm != m_TrimSymm )
    {
        return false;
    }

    // Number of planes in this trim group.
    int numplane = m_TrimPt.size();
    for ( int iplane = 0; iplane < numplane; iplane++ )
    {
        vec3d u = pt - m_TrimPt[ iplane ];
        double dp = dot( u, m_TrimNorm[ iplane ] );  // m_TrimNorm is always a unit vector.
        if ( dp < tol )
        {
            return false;
        }
    }

    for ( int isurf = 0; isurf < m_TrimSurf.size(); isurf++ )
    {
        VspSurf &surf = m_TrimSurf[ isurf ];
        double u, w;
        surf.FindNearest( u, w, pt );
        vec3d p, n;
        p = surf.CompPnt( u, w );
        n = surf.CompAveNorm( u, w );

        vec3d v = pt - p;
        double dp = dot( v, n );
        if ( dp < tol )
        {
            return false;
        }
    }

    return true;
}

FeaMesh::FeaMesh( const string & struct_id )
{
    m_ID = struct_id;

    m_TotalMass = 0;
    m_LenScale = 1.0;

    m_NumFeaParts = 0;
    m_NumFeaSubSurfs = 0;
    m_NumFeaTrimParts = 0;
    m_NumFeaFixPoints = 0;

    m_NumNodes = 0;

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

    for ( unsigned int i = 0; i < m_FeaOffBodyFixPointNodeVec.size(); i++ )
    {
        delete m_FeaOffBodyFixPointNodeVec[i];
    }
    m_FeaOffBodyFixPointNodeVec.clear();

    m_FeaNodeVec.clear();

    m_SimpleSubSurfaceVec.clear();
    m_PartSurfOrientation.clear();

    m_FixPntVec.clear();

    m_TrimVec.clear();

    m_FeaNodeVecUsed.clear();

    m_StructName = "";

    m_TotalMass = 0;

    m_MeshReady = false;

    m_NumFeaParts = 0;
    m_NumFeaSubSurfs = 0;
    m_NumFeaFixPoints = 0;
    m_NumFeaTrimParts = 0;

    m_FeaPartNameVec.clear();
    m_FeaPartTypeVec.clear();
    m_FeaPartNumSurfVec.clear();
    m_FeaPartKeepDelShellElementsVec.clear();
    m_FeaPartCreateBeamElementsVec.clear();
    m_FeaPartPropertyIndexVec.clear();
    m_FeaPartCapPropertyIndexVec.clear();
    m_FeaPartNumChainsVec.clear();

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
    m_BCNodeDO.m_PntVec.clear();

    double line_length = GetGridDensityPtr()->m_MinLen / 3.0; // Length of orientation vectors

    m_BCNodeDO.m_GeomID = GetID() + "_BCNodes";
    m_BCNodeDO.m_Type = DrawObj::VSP_POINTS;
    m_BCNodeDO.m_PointSize = 10.0;
    m_BCNodeDO.m_PointColor = vec3d( 1, 0, 0 );
    for ( unsigned int j = 0; j < (int)m_FeaNodeVec.size(); j++ )
    {
        if ( m_FeaNodeVec[j]->m_BCs.AsNum() != 0 ) // Some BC is set.
        {
            m_BCNodeDO.m_PntVec.push_back( m_FeaNodeVec[j]->m_Pnt );
        }
    }
    m_BCNodeDO.m_GeomChanged = true;

    char str[256];
    for ( int iprt = 0; iprt < m_NumFeaParts; iprt++ )
    {
        snprintf( str, sizeof( str ), "%s_Node_Tag_%d", GetID().c_str(), iprt );
        m_FeaNodeDO[iprt].m_GeomID = string( str );
        snprintf( str, sizeof( str ), "%s_T_Element_Tag_%d", GetID().c_str(), iprt );
        m_FeaTriElementDO[iprt].m_GeomID = string( str );
        snprintf( str, sizeof( str ), "%s_Q_Element_Tag_%d", GetID().c_str(), iprt );
        m_FeaQuadElementDO[iprt].m_GeomID = string( str );
        snprintf( str, sizeof( str ), "%s_Cap_Element_Tag_%d", GetID().c_str(), iprt );
        m_CapFeaElementDO[iprt].m_GeomID = string( str );

        m_FeaNodeDO[iprt].m_GeomChanged = true;
        m_FeaTriElementDO[iprt].m_GeomChanged = true;
        m_FeaQuadElementDO[iprt].m_GeomChanged = true;
        m_CapFeaElementDO[iprt].m_GeomChanged = true;

        for ( unsigned int j = 0; j < (int)m_FeaNodeVec.size(); j++ )
        {
            if ( m_FeaNodeVecUsed[ j ] )
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
                if ( m_FeaNodeVecUsed[ j ] )
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
                vec3d c = DrawObj::Color( DrawObj::reasonColorMap( m_FeaElementVec[j]->GetReason() ) );

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


                        m_FeaTriElementDO[iprt].m_FaceColorVec.push_back( c );
                        m_FeaTriElementDO[iprt].m_FaceColorVec.push_back( c );
                        m_FeaTriElementDO[iprt].m_FaceColorVec.push_back( c );

                        m_FeaTriElementDO[iprt].m_FaceAlphaVec.push_back( 1.0f );
                        m_FeaTriElementDO[iprt].m_FaceAlphaVec.push_back( 1.0f );
                        m_FeaTriElementDO[iprt].m_FaceAlphaVec.push_back( 1.0f );
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

                        m_FeaQuadElementDO[iprt].m_FaceColorVec.push_back( c );
                        m_FeaQuadElementDO[iprt].m_FaceColorVec.push_back( c );
                        m_FeaQuadElementDO[iprt].m_FaceColorVec.push_back( c );
                        m_FeaQuadElementDO[iprt].m_FaceColorVec.push_back( c );

                        m_FeaQuadElementDO[iprt].m_FaceAlphaVec.push_back( 1.0f );
                        m_FeaQuadElementDO[iprt].m_FaceAlphaVec.push_back( 1.0f );
                        m_FeaQuadElementDO[iprt].m_FaceAlphaVec.push_back( 1.0f );
                        m_FeaQuadElementDO[iprt].m_FaceAlphaVec.push_back( 1.0f );
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

        snprintf( str, sizeof( str ), "%s_Tri_Norm_%d", GetID().c_str(), iprt );
        m_ElOrientationDO[iprt].m_GeomID = string( str );
        m_ElOrientationDO[iprt].m_Type = DrawObj::VSP_LINES;
        m_ElOrientationDO[iprt].m_LineWidth = 1.0;
        m_ElOrientationDO[iprt].m_LineColor = vec3d( 0.0, 0.0, 0.0 );
        m_ElOrientationDO[iprt].m_GeomChanged = true;

        snprintf( str, sizeof( str ), "%s_Cap_Norm_%d", GetID().c_str(), iprt );
        m_CapNormDO[iprt].m_GeomID = string( str );
        m_CapNormDO[iprt].m_Type = DrawObj::VSP_LINES;
        m_CapNormDO[iprt].m_LineWidth = 1.0;
        m_CapNormDO[iprt].m_LineColor = vec3d( 0.0, 0.0, 0.0 );
        m_CapNormDO[iprt].m_GeomChanged = true;

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

                vec3d norm_pnt = m_FeaElementVec[j]->m_Mids[0]->m_Pnt + line_length * beam->m_Norm0;

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
        snprintf( str, sizeof( str ), "%s_SSNode_Tag_%d", GetID().c_str(), iss );
        m_SSFeaNodeDO[iss].m_GeomID = string( str );
        snprintf( str, sizeof( str ), "%s_SSTriElement_Tag_%d", GetID().c_str(), iss );
        m_SSTriElementDO[iss].m_GeomID = string( str );
        snprintf( str, sizeof( str ), "%s_SSQuadElement_Tag_%d", GetID().c_str(), iss );
        m_SSQuadElementDO[iss].m_GeomID = string( str );
        snprintf( str, sizeof( str ), "%s_SSCap_Element_Tag_%d", GetID().c_str(), iss );
        m_SSCapFeaElementDO[iss].m_GeomID = string( str );

        m_SSFeaNodeDO[iss].m_GeomChanged = true;
        m_SSTriElementDO[iss].m_GeomChanged = true;
        m_SSQuadElementDO[iss].m_GeomChanged = true;
        m_SSCapFeaElementDO[iss].m_GeomChanged = true;

        for ( unsigned int j = 0; j < (int)m_FeaNodeVec.size(); j++ )
        {
            if ( m_FeaNodeVecUsed[ j ] )
            {
                if ( m_FeaNodeVec[j]->HasTag( iss + m_NumFeaParts ) )
                {
                    m_SSFeaNodeDO[iss].m_PntVec.push_back( m_FeaNodeVec[j]->m_Pnt );
                }
            }
        }

        for ( int j = 0; j < m_FeaElementVec.size(); j++ )
        {
            vec3d c = DrawObj::Color( DrawObj::reasonColorMap( m_FeaElementVec[j]->GetReason() ) );

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

                m_SSTriElementDO[iss].m_FaceColorVec.push_back( c );
                m_SSTriElementDO[iss].m_FaceColorVec.push_back( c );
                m_SSTriElementDO[iss].m_FaceColorVec.push_back( c );

                m_SSTriElementDO[iss].m_FaceAlphaVec.push_back( 1.0f );
                m_SSTriElementDO[iss].m_FaceAlphaVec.push_back( 1.0f );
                m_SSTriElementDO[iss].m_FaceAlphaVec.push_back( 1.0f );
            }
            else if (( m_FeaElementVec[j]->GetFeaSSIndex() == iss ) &&
                     ( m_FeaElementVec[j]->GetElementType() == FeaElement::FEA_QUAD_4 || m_FeaElementVec[j]->GetElementType() == FeaElement::FEA_QUAD_8 ) )
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

                m_SSQuadElementDO[iss].m_FaceColorVec.push_back( c );
                m_SSQuadElementDO[iss].m_FaceColorVec.push_back( c );
                m_SSQuadElementDO[iss].m_FaceColorVec.push_back( c );
                m_SSQuadElementDO[iss].m_FaceColorVec.push_back( c );

                m_SSQuadElementDO[iss].m_FaceAlphaVec.push_back( 1.0f );
                m_SSQuadElementDO[iss].m_FaceAlphaVec.push_back( 1.0f );
                m_SSQuadElementDO[iss].m_FaceAlphaVec.push_back( 1.0f );
                m_SSQuadElementDO[iss].m_FaceAlphaVec.push_back( 1.0f );
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

        snprintf( str, sizeof( str ), "%s_SSTri_Norm_%u", GetID().c_str(), iss );
        m_SSElOrientationDO[iss].m_GeomID = string( str );
        m_SSElOrientationDO[iss].m_Type = DrawObj::VSP_LINES;
        m_SSElOrientationDO[iss].m_LineWidth = 1.0;
        m_SSElOrientationDO[iss].m_LineColor = vec3d( 0.0, 0.0, 0.0 );
        m_SSElOrientationDO[iss].m_GeomChanged = true;

        snprintf( str, sizeof( str ), "%s_SSCap_Norm_%u", GetID().c_str(), iss );
        m_SSCapNormDO[iss].m_GeomID = string( str );
        m_SSCapNormDO[iss].m_Type = DrawObj::VSP_LINES;
        m_SSCapNormDO[iss].m_LineWidth = 1.0;
        m_SSCapNormDO[iss].m_LineColor = vec3d( 0.0, 0.0, 0.0 );
        m_SSCapNormDO[iss].m_GeomChanged = true;

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

                vec3d norm_pnt = m_FeaElementVec[j]->m_Mids[0]->m_Pnt + line_length * beam->m_Norm0;

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
            m_FeaNodeDO[iprt].m_PointColor = vec3d( 0.0, 0.0, 0.0 );

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
                 st_settings->m_ColorFacesFlag )   // At least mesh or tags are visible.
            {
                if ( !m_FixPointFeaPartFlagVec[iprt] )
                {
                    if ( st_settings->m_DrawMeshFlag &&
                         st_settings->m_ColorFacesFlag ) // Both are visible.
                    {
                        if ( st_settings->m_ColorTagReason == vsp::TAG )
                        {
                            m_FeaTriElementDO[iprt].m_Type = DrawObj::VSP_WIRE_SHADED_TRIS;
                            m_FeaQuadElementDO[iprt].m_Type = DrawObj::VSP_WIRE_SHADED_QUADS;
                        }
                        else
                        {
                            m_FeaTriElementDO[iprt].m_Type = DrawObj::VSP_WIRE_MAPPED_TRIS;
                            m_FeaQuadElementDO[iprt].m_Type = DrawObj::VSP_WIRE_MAPPED_QUADS;
                        }

                        m_FeaTriElementDO[iprt].m_LineColor = vec3d( 0.4, 0.4, 0.4 );
                        m_FeaQuadElementDO[iprt].m_LineColor = vec3d( 0.4, 0.4, 0.4 );
                    }
                    else if ( st_settings->m_DrawMeshFlag ) // Mesh only
                    {
                        m_FeaTriElementDO[iprt].m_Type = DrawObj::VSP_WIRE_HIDDEN_TRIS;
                        m_FeaTriElementDO[iprt].m_LineColor = vec3d( 0.4, 0.4, 0.4 );

                        m_FeaQuadElementDO[iprt].m_Type = DrawObj::VSP_WIRE_HIDDEN_QUADS;
                        m_FeaQuadElementDO[iprt].m_LineColor = vec3d( 0.4, 0.4, 0.4 );
                    }
                    else // Tags only
                    {
                        if ( st_settings->m_ColorTagReason == vsp::TAG )
                        {
                            m_FeaTriElementDO[iprt].m_Type = DrawObj::VSP_SHADED_TRIS;
                            m_FeaQuadElementDO[iprt].m_Type = DrawObj::VSP_SHADED_QUADS;
                        }
                        else
                        {
                            m_FeaTriElementDO[iprt].m_Type = DrawObj::VSP_MAPPED_TRIS;
                            m_FeaQuadElementDO[iprt].m_Type = DrawObj::VSP_MAPPED_QUADS;
                        }
                    }
                }
            }
            else
            {
                m_FeaTriElementDO[iprt].m_Type = DrawObj::VSP_WIRE_HIDDEN_TRIS;
                m_FeaTriElementDO[iprt].m_LineColor = vec3d( 0.4, 0.4, 0.4 );

                m_FeaQuadElementDO[iprt].m_Type = DrawObj::VSP_WIRE_HIDDEN_QUADS;
                m_FeaQuadElementDO[iprt].m_LineColor = vec3d( 0.4, 0.4, 0.4 );
            }

            if ( st_settings->m_ColorFacesFlag )
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
                m_CapFeaElementDO[iprt].m_LineColor = vec3d( 0.0, 0.0, 0.0 );
            }

            m_FeaTriElementDO[iprt].m_Visible = ( st_settings->m_DrawMeshFlag || st_settings->m_ColorFacesFlag ) && m_DrawElementFlagVec[iprt];
            m_FeaQuadElementDO[iprt].m_Visible = ( st_settings->m_DrawMeshFlag || st_settings->m_ColorFacesFlag ) && m_DrawElementFlagVec[iprt];
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
            m_SSFeaNodeDO[iss].m_PointColor = vec3d( 0.0, 0.0, 0.0 );

            m_SSTriElementDO[iss].m_Type = DrawObj::VSP_SHADED_TRIS;
            m_SSQuadElementDO[iss].m_Type = DrawObj::VSP_SHADED_QUADS;
            m_SSCapFeaElementDO[iss].m_Type = DrawObj::VSP_LINES;
            m_SSCapFeaElementDO[iss].m_LineWidth = 3.0;

            if ( st_settings->m_DrawMeshFlag ||
                 st_settings->m_ColorFacesFlag )   // At least mesh or tags are visible.
            {

                if ( st_settings->m_DrawMeshFlag &&
                     st_settings->m_ColorFacesFlag ) // Both are visible.
                {
                    if ( st_settings->m_ColorTagReason == vsp::TAG )
                    {
                        m_SSTriElementDO[iss].m_Type = DrawObj::VSP_WIRE_SHADED_TRIS;
                        m_SSQuadElementDO[iss].m_Type = DrawObj::VSP_WIRE_SHADED_QUADS;
                    }
                    else
                    {
                        m_SSTriElementDO[iss].m_Type = DrawObj::VSP_WIRE_MAPPED_TRIS;
                        m_SSQuadElementDO[iss].m_Type = DrawObj::VSP_WIRE_MAPPED_QUADS;
                    }

                    m_SSTriElementDO[iss].m_LineColor = vec3d( 0.4, 0.4, 0.4 );
                    m_SSQuadElementDO[iss].m_LineColor = vec3d( 0.4, 0.4, 0.4 );
                }
                else if ( st_settings->m_DrawMeshFlag ) // Mesh only
                {
                    m_SSTriElementDO[iss].m_Type = DrawObj::VSP_WIRE_HIDDEN_TRIS;
                    m_SSTriElementDO[iss].m_LineColor = vec3d( 0.4, 0.4, 0.4 );
                    m_SSQuadElementDO[iss].m_Type = DrawObj::VSP_WIRE_HIDDEN_QUADS;
                    m_SSQuadElementDO[iss].m_LineColor = vec3d( 0.4, 0.4, 0.4 );
                }
                else // Tags only
                {

                    if ( st_settings->m_ColorTagReason == vsp::TAG )
                    {
                        m_SSTriElementDO[iss].m_Type = DrawObj::VSP_SHADED_TRIS;
                        m_SSQuadElementDO[iss].m_Type = DrawObj::VSP_SHADED_QUADS;
                    }
                    else
                    {
                        m_SSTriElementDO[iss].m_Type = DrawObj::VSP_MAPPED_TRIS;
                        m_SSQuadElementDO[iss].m_Type = DrawObj::VSP_MAPPED_QUADS;
                    }
                }
            }
            else
            {
                m_SSTriElementDO[iss].m_Type = DrawObj::VSP_WIRE_HIDDEN_TRIS;
                m_SSTriElementDO[iss].m_LineColor = vec3d( 0.4, 0.4, 0.4 );
                m_SSQuadElementDO[iss].m_Type = DrawObj::VSP_WIRE_HIDDEN_QUADS;
                m_SSQuadElementDO[iss].m_LineColor = vec3d( 0.4, 0.4, 0.4 );
            }

            if ( st_settings->m_ColorFacesFlag )
            {
                // Color sequence -- go around color wheel ncstep times with slight
                // offset from ncgrp basic colors.
                // Note, (cnt/ncgrp) uses integer division resulting in floor.
                double deg = ((( m_NumFeaParts + iss ) % ncgrp ) * ncstep + (( m_NumFeaParts + iss ) / ncgrp ) ) * nctodeg;
                double deg2 = ((( 1 + m_NumFeaParts + m_NumFeaSubSurfs + iss ) % ncgrp ) * ncstep + (( 1 + m_NumFeaParts + m_NumFeaSubSurfs + iss ) / ncgrp ) ) * nctodeg;
                vec3d rgb = m_SSTriElementDO[iss].ColorWheel( deg );
                rgb.normalize();

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

                m_SSCapFeaElementDO[iss].m_LineColor = vec3d( 0.0, 0.0, 0.0 );
            }

            m_SSTriElementDO[ iss ].m_Visible = ( st_settings->m_DrawMeshFlag || st_settings->m_ColorFacesFlag ) && m_DrawElementFlagVec[ iss + m_NumFeaParts ];
            m_SSQuadElementDO[ iss ].m_Visible = ( st_settings->m_DrawMeshFlag || st_settings->m_ColorFacesFlag ) && m_DrawElementFlagVec[ iss + m_NumFeaParts ];

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

    m_BCNodeDO.m_Visible = st_settings->m_DrawBCNodesFlag;
    draw_obj_vec.push_back( &m_BCNodeDO );

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
        WriteNASTRAN();
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

void FeaMesh::WriteNASTRAN()
{
    string dat_fn = GetStructSettingsPtr()->GetExportFileName( vsp::FEA_NASTRAN_FILE_NAME );

    string bdf_fn = dat_fn;
    int pos = bdf_fn.find( ".dat" );
    if ( pos >= 0 )
    {
        bdf_fn.erase( pos, bdf_fn.length() - 1 );
    }
    bdf_fn.append( ".bdf" );


    FILE* dat_fp = fopen( dat_fn.c_str(), "w" ); // Open *_NASTRAN.dat

    // Create temporary file to store NASTRAN bulk data. Case control information (SETs) will be
    //  defined in the *_NASTRAN.dat file prior to the bulk data (elements, gridpoints, etc.)
    FILE* bdf_fp = std::tmpfile();

    FILE* bdf_header_fp = fopen( bdf_fn.c_str(), "w" );

    if ( dat_fp && bdf_header_fp && bdf_fp )
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

        WriteNASTRAN( dat_fp, bdf_header_fp, bdf_fp, nkey_fp );

        CloseNASTRAN( dat_fp, bdf_header_fp, bdf_fp, nkey_fp );
    }
}

void FeaMesh::WriteNASTRAN( FILE *dat_fp, FILE *bdf_header_fp, FILE *bdf_fp, FILE *nkey_fp )
{
    FeaMeshMgr.ResetPropMatUse();

    if ( dat_fp && bdf_header_fp && bdf_fp )
    {
        WriteNASTRANHeader( dat_fp );

        // Write bulk data to temp file
        fprintf( bdf_header_fp, "BEGIN BULK\n" );


        int set_cnt = 1;

        WriteNASTRANSPC1( bdf_fp );

        WriteNASTRANNodes( dat_fp, bdf_fp, nkey_fp, set_cnt );

        WriteNASTRANElements( dat_fp, bdf_fp, nkey_fp, set_cnt );

        FeaMeshMgr.WriteNASTRANProperties( bdf_header_fp );

        FeaMeshMgr.WriteNASTRANMaterials( bdf_header_fp );

        fprintf( bdf_fp, "\nENDDATA\n" );
    }
}

void FeaMesh::WriteNASTRANHeader( FILE* dat_fp )
{
    if ( dat_fp )
    {
        // Comments can be at top of NASTRAN file before case control section
        fprintf( dat_fp, "$ NASTRAN Data File Generated from %s\n", VSPVERSION4 );
        fprintf( dat_fp, "$ %s\n", m_StructName.c_str() );
        fprintf( dat_fp, "$ Num_Nodes:       %llu\n", m_NumNodes );
        fprintf( dat_fp, "$ Num_Els:         %llu\n", m_NumEls );
        fprintf( dat_fp, "$ Num_Tris:        %llu\n", m_NumTris );
        fprintf( dat_fp, "$ Num_Quads:       %llu\n", m_NumQuads );
        fprintf( dat_fp, "$ Num_Beams:       %llu\n", m_NumBeams );
        fprintf( dat_fp, "$ Node_Offset:     %llu\n", m_StructSettings.m_NodeOffset );
        fprintf( dat_fp, "$ Element_Offset:  %llu\n", m_StructSettings.m_ElementOffset );
        fprintf( dat_fp, "\n" );
    }
}

void FeaMesh::WriteNASTRANNodes( FILE* dat_fp, FILE* bdf_fp, FILE* nkey_fp, int &set_cnt )
{
    unsigned long long int noffset = m_StructSettings.m_NodeOffset;

    if ( dat_fp && bdf_fp )
    {
        vector < long long int > node_id_vec;
        string name;

        //==== Write Fixed Points ====//
        for ( size_t i = 0; i < m_NumFeaFixPoints; i++ )
        {
            node_id_vec.clear();

            FixPoint fxpt = m_FixPntVec[i];

            fprintf( bdf_fp, "\n" );
            fprintf( bdf_fp, "$ %s %s Fixed Gridpoints\n", m_FeaPartNameVec[ fxpt.m_FeaPartIndex ].c_str(), m_StructName.c_str() );

            for ( unsigned int j = 0; j < (int)fxpt.m_Pnt.size(); j++ )
            {
                if ( fxpt.m_NodeIndex[j] >= 0 )
                {
                    string fmt = "GRID    ,%8lld,        ," + StringUtil::NasFmt( fxpt.m_Pnt[j][0] ) + "," + StringUtil::NasFmt( fxpt.m_Pnt[j][1] ) + "," + StringUtil::NasFmt( fxpt.m_Pnt[j][2] ) + ",        ,%s\n";
                    fprintf( bdf_fp, fmt.c_str(), fxpt.m_NodeIndex[j] + noffset, fxpt.m_Pnt[j][0], fxpt.m_Pnt[j][1], fxpt.m_Pnt[j][2], "" );

                    node_id_vec.push_back( fxpt.m_NodeIndex[j] );
                }
            }

            // Write FEA part node set
            name = m_FeaPartNameVec[fxpt.m_FeaPartIndex] + "_" + m_StructName + "_FixedGridpoints";
            WriteNASTRANSet( dat_fp, nkey_fp, set_cnt, node_id_vec, name, noffset );
        }

        //==== Write Spider Points ====//
        for ( size_t i = 0; i < m_NumFeaFixPoints; i++ )
        {
            FixPoint fxpt = m_FixPntVec[ i ];

            if ( fxpt.m_OnBody )
            {
                node_id_vec.clear();

                for ( unsigned int j = 0; j < (int)fxpt.m_Pnt.size(); j++ )
                {
                    if ( fxpt.m_NodeIndex[ j ] >= 0 )
                    {
                        fprintf( bdf_fp, "\n" );
                        fprintf( bdf_fp, "$ Node %d spider points\n", (int) ( fxpt.m_NodeIndex[ j ] + noffset ) );

                        for ( int ispider = 0; ispider < fxpt.m_SpiderIndex[ j ].size(); ispider++ )
                        {
                            int spider_ind = (int) fxpt.m_SpiderIndex[ j ][ ispider ];

                            m_FeaNodeVec[ spider_ind ]->WriteNASTRAN( bdf_fp, noffset );
                            node_id_vec.push_back( m_FeaNodeVec[ spider_ind ]->m_Index );
                        }

                        // Write FEA part node set
                        name = "Node_" + to_string( fxpt.m_NodeIndex[ j ] + noffset ) + "_SpiderPoints";
                        WriteNASTRANSet( dat_fp, nkey_fp, set_cnt, node_id_vec, name, noffset );
                    }
                }
            }
        }

        // FeaPart Nodes
        for ( unsigned int i = 0; i < m_NumFeaParts; i++ )
        {
            node_id_vec.clear();

            bool partheader = false;

            if ( m_FeaPartTypeVec[i] != vsp::FEA_FIX_POINT )
            {
                for ( unsigned int j = 0; j < (int)m_FeaNodeVec.size(); j++ )
                {
                    if ( m_FeaNodeVecUsed[ j ] && m_FeaNodeVec[ j ]->m_Index > m_FixPtOffset )
                    {
                        if ( m_FeaNodeVec[ j ]->HasOnlyTag( i ) )
                        {
                            if ( !partheader )
                            {
                                partheader = true;

                                fprintf( bdf_fp, "\n" );
                                fprintf( bdf_fp, "$ %s %s Gridpoints\n", m_FeaPartNameVec[i].c_str(), m_StructName.c_str() );
                            }

                            m_FeaNodeVec[j]->WriteNASTRAN( bdf_fp, noffset );
                            node_id_vec.push_back( m_FeaNodeVec[j]->m_Index );
                        }
                    }
                }
            }

            // Write FEA part node set
            name = m_FeaPartNameVec[i] + "_" + m_StructName + "_Gridpoints";
            WriteNASTRANSet( dat_fp, nkey_fp, set_cnt, node_id_vec, name, noffset );
        }

        // SubSurface Nodes
        for ( unsigned int i = 0; i < m_NumFeaSubSurfs; i++ )
        {
            bool ssheader = false;

            node_id_vec.clear();

            for ( unsigned int j = 0; j < (int)m_FeaNodeVec.size(); j++ )
            {
                if ( m_FeaNodeVecUsed[ j ] && m_FeaNodeVec[ j ]->m_Index > m_FixPtOffset )
                {
                    if ( m_FeaNodeVec[ j ]->HasOnlyTag( i + m_NumFeaParts ) )
                    {
                        if ( !ssheader )
                        {
                            ssheader = true;

                            fprintf( bdf_fp, "\n" );
                            fprintf( bdf_fp, "$ %s %s Gridpoints\n", m_SimpleSubSurfaceVec[i].GetName().c_str(), m_StructName.c_str() );
                        }
                        m_FeaNodeVec[j]->WriteNASTRAN( bdf_fp, noffset );
                        node_id_vec.push_back( m_FeaNodeVec[j]->m_Index );
                    }
                }
            }

            // Write subsurface node set
            name = m_SimpleSubSurfaceVec[i].GetName() + "_" + m_StructName + "_Gridpoints";
            WriteNASTRANSet( dat_fp, nkey_fp, set_cnt, node_id_vec, name, noffset );
        }

        node_id_vec.clear();

        // Intersection Nodes
        bool IntersectHeader = false;
        for ( unsigned int j = 0; j < (int)m_FeaNodeVec.size(); j++ )
        {
            if ( m_FeaNodeVecUsed[ j ] && m_FeaNodeVec[ j ]->m_Index > m_FixPtOffset )
            {
                if ( m_FeaNodeVec[j]->m_Tags.size() > 1 && !m_FeaNodeVec[j]->m_FixedPointFlag )
                {
                    if ( !IntersectHeader )
                    {
                        fprintf( bdf_fp, "\n" );
                        fprintf( bdf_fp, "$ %s Intersections\n", m_StructName.c_str() );
                        IntersectHeader = true;
                    }
                    m_FeaNodeVec[j]->WriteNASTRAN( bdf_fp, noffset );
                    node_id_vec.push_back( m_FeaNodeVec[j]->m_Index );
                }
            }
        }

        // Write intersection node set
        name = m_StructName + "_Intersection_Gridpoints";
        WriteNASTRANSet( dat_fp, nkey_fp, set_cnt, node_id_vec, name, noffset );

        node_id_vec.clear();

        //==== Remaining Nodes ====//
        bool RemainingHeader = false;
        for ( int i = 0; i < (int)m_FeaNodeVec.size(); i++ )
        {
            node_id_vec.clear();

            if ( m_FeaNodeVecUsed[ i ] && m_FeaNodeVec[i]->m_Tags.size() == 0 && m_FeaNodeVec[ i ]->m_Index > m_FixPtOffset )
            {
                if ( !RemainingHeader )
                {
                    fprintf( bdf_fp, "\n" );
                    fprintf( bdf_fp, "$ %s Remainingnodes\n", m_StructName.c_str() );
                    RemainingHeader = true;
                }
                m_FeaNodeVec[i]->WriteNASTRAN( bdf_fp, noffset );
                node_id_vec.push_back( m_FeaNodeVec[i]->m_Index );
            }
        }

        // Write remaining node set
        name = m_StructName + "_Remaining_Gridpoints";
        WriteNASTRANSet( dat_fp, nkey_fp, set_cnt, node_id_vec, name, noffset );
    }
}

void FeaMesh::WriteNASTRANSPC1( FILE *bdf_fp )
{
    unsigned long long int noffset = m_StructSettings.m_NodeOffset;

    if ( bdf_fp )
    {
        for ( unsigned int i = 0; i < m_NumFeaParts; i++ )
        {
            bool FPHeader = false;

            if ( m_FeaPartTypeVec[i] == vsp::FEA_FIX_POINT ) // FixedPoint Nodes
            {
                for ( unsigned int j = 0; j < (int)m_FeaNodeVec.size(); j++ )
                {
                    if ( m_FeaNodeVecUsed[ j ] )
                    {
                        if ( m_FeaNodeVec[j]->m_Tags.size() > 0 && m_FeaNodeVec[j]->m_FixedPointFlag && m_FeaNodeVec[j]->HasTag( i ) )
                        {
                            if ( m_FeaNodeVec[ j ]->m_BCs.AsNum() > 0 )
                            {
                                if ( !FPHeader )
                                {
                                    FPHeader = true;

                                    fprintf( bdf_fp, "\n" );
                                    fprintf( bdf_fp, "$ %s %s Fixed point constraints\n", m_FeaPartNameVec[i].c_str(), m_StructName.c_str() );
                                }
                               m_FeaNodeVec[j]->WriteNASTRAN_SPC1( bdf_fp, noffset );
                            }
                        }
                    }
                }
            }
        }

        // FeaPart Nodes
        for ( unsigned int i = 0; i < m_NumFeaParts; i++ )
        {
            bool partheader = false;

            if ( m_FeaPartTypeVec[i] != vsp::FEA_FIX_POINT )
            {
                for ( unsigned int j = 0; j < (int)m_FeaNodeVec.size(); j++ )
                {
                    if ( m_FeaNodeVecUsed[ j ] )
                    {
                        if ( m_FeaNodeVec[ j ]->HasOnlyTag( i ) )
                        {
                            if ( m_FeaNodeVec[ j ]->m_BCs.AsNum() > 0 )
                            {
                                if ( !partheader )
                                {
                                    partheader = true;

                                    fprintf( bdf_fp, "\n" );
                                    fprintf( bdf_fp, "$ %s %s Constraints\n", m_FeaPartNameVec[i].c_str(), m_StructName.c_str() );
                                }
                                m_FeaNodeVec[j]->WriteNASTRAN_SPC1( bdf_fp, noffset );
                            }
                        }
                    }
                }
            }
        }

        // SubSurface Nodes
        for ( unsigned int i = 0; i < m_NumFeaSubSurfs; i++ )
        {
            bool ssheader = false;

            for ( unsigned int j = 0; j < (int)m_FeaNodeVec.size(); j++ )
            {
                if ( m_FeaNodeVecUsed[ j ] )
                {
                    if ( m_FeaNodeVec[ j ]->HasOnlyTag( i + m_NumFeaParts ) )
                    {
                        if ( m_FeaNodeVec[ j ]->m_BCs.AsNum() > 0 )
                        {
                            if ( !ssheader )
                            {
                                ssheader = true;

                                fprintf( bdf_fp, "\n" );
                                fprintf( bdf_fp, "$ %s %s Constraints\n", m_SimpleSubSurfaceVec[i].GetName().c_str(), m_StructName.c_str() );
                            }
                            m_FeaNodeVec[j]->WriteNASTRAN_SPC1( bdf_fp, noffset );
                        }
                    }
                }
            }
        }

        // Intersection Nodes
        bool IntersectHeader = false;
        for ( unsigned int j = 0; j < (int)m_FeaNodeVec.size(); j++ )
        {
            if ( m_FeaNodeVecUsed[ j ] )
            {
                if ( m_FeaNodeVec[j]->m_Tags.size() > 1 && !m_FeaNodeVec[j]->m_FixedPointFlag )
                {
                    if ( m_FeaNodeVec[ j ]->m_BCs.AsNum() > 0 )
                    {
                        if ( !IntersectHeader )
                        {
                            fprintf( bdf_fp, "\n" );
                            fprintf( bdf_fp, "$ %s Intersection constraints\n", m_StructName.c_str() );
                            IntersectHeader = true;
                        }
                        m_FeaNodeVec[j]->WriteNASTRAN_SPC1( bdf_fp, noffset );
                    }
                }
            }
        }

        //==== Remaining Nodes ====//
        bool RemainingHeader = false;
        for ( int i = 0; i < (int)m_FeaNodeVec.size(); i++ )
        {
            if ( m_FeaNodeVecUsed[ i ] && m_FeaNodeVec[i]->m_Tags.size() == 0 )
            {
                if ( m_FeaNodeVec[ i ]->m_BCs.AsNum() > 0 )
                {
                    if ( !RemainingHeader )
                    {
                        fprintf( bdf_fp, "\n" );
                        fprintf( bdf_fp, "$ %s Remaining node constraints\n", m_StructName.c_str() );
                        RemainingHeader = true;
                    }
                    m_FeaNodeVec[i]->WriteNASTRAN_SPC1( bdf_fp, noffset );
                }
            }
        }

    }
}

void FeaMesh::WriteNASTRANElements( FILE* dat_fp, FILE* bdf_fp, FILE* nkey_fp, int &set_cnt )
{
    unsigned long long int noffset = m_StructSettings.m_NodeOffset;
    unsigned long long int eoffset = m_StructSettings.m_ElementOffset;

    if ( dat_fp && bdf_fp )
    {
        string name;
        vector < long long int > shell_elem_id_vec, beam_elem_id_vec;
        int elem_id = 1;

        // Write FeaFixPoints
        for ( unsigned int i = 0; i < m_NumFeaFixPoints; i++ )
        {
            FixPoint fxpt = m_FixPntVec[i];
            if ( fxpt.m_PtMassFlag )
            {
                bool FPHeader = false;

                vector < long long int > mass_elem_id_vec;

                for ( int j = 0; j < m_FeaElementVec.size(); j++ )
                {
                    if ( m_FeaElementVec[j]->GetElementType() == FeaElement::FEA_POINT_MASS && m_FeaElementVec[j]->GetFeaPartIndex() == fxpt.m_FeaPartIndex && m_FeaElementVec[j]->GetFeaSSIndex() < 0 )
                    {
                        if ( !FPHeader )
                        {
                            FPHeader = true;

                            fprintf( bdf_fp, "\n" );
                            fprintf( bdf_fp, "$ %s %s\n", m_FeaPartNameVec[fxpt.m_FeaPartIndex].c_str(), m_StructName.c_str() );
                        }

                        m_FeaElementVec[j]->WriteNASTRAN( bdf_fp, elem_id, -1, noffset, eoffset ); // property ID ignored for Point Masses
                        mass_elem_id_vec.push_back( elem_id );
                        elem_id++;
                    }
                }

                // Write mass element set
                name = m_FeaPartNameVec[fxpt.m_FeaPartIndex] + "_" +  m_StructName + "_MassElements";
                WriteNASTRANSet( dat_fp, nkey_fp, set_cnt, mass_elem_id_vec, name, eoffset );
            }
        }

        // Write FeaParts
        for ( unsigned int i = 0; i < m_NumFeaParts; i++ )
        {
            if ( m_FeaPartTypeVec[i] != vsp::FEA_FIX_POINT )
            {
                shell_elem_id_vec.clear();

                int property_id = m_FeaPartPropertyIndexVec[i];
                int cap_property_id = m_FeaPartCapPropertyIndexVec[i];

                // Write shell elements
                bool partheader = false;

                for ( int j = 0; j < m_FeaElementVec.size(); j++ )
                {
                    if ( m_FeaElementVec[j]->GetFeaPartIndex() == i && m_FeaElementVec[j]->GetFeaSSIndex() < 0 && m_FeaElementVec[j]->GetElementType() != FeaElement::FEA_BEAM )
                    {
                        if ( !partheader )
                        {
                            partheader = true;

                            fprintf( bdf_fp, "\n" );
                            fprintf( bdf_fp, "$ %s %s shell\n", m_FeaPartNameVec[i].c_str(), m_StructName.c_str()  );
                        }

                        m_FeaElementVec[j]->WriteNASTRAN( bdf_fp, elem_id, property_id, noffset, eoffset );
                        shell_elem_id_vec.push_back( elem_id );
                        FeaMeshMgr.MarkPropMatUsed( property_id );

                        elem_id++;
                    }
                }

                // Write shell element set
                name = m_FeaPartNameVec[i] + "_" + m_StructName + "_ShellElements";
                WriteNASTRANSet( dat_fp, nkey_fp, set_cnt, shell_elem_id_vec, name, eoffset );

                for ( int ichain = 0; ichain < m_FeaPartNumChainsVec[ i ]; ichain++ )
                {

                    beam_elem_id_vec.clear();

                    // Write beam elements
                    partheader = false;

                    for ( int j = 0; j < m_FeaElementVec.size(); j++ )
                    {
                        if ( m_FeaElementVec[j]->GetFeaPartIndex() == i &&
                             m_FeaElementVec[j]->GetFeaSSIndex() < 0 &&
                             m_FeaElementVec[j]->GetChainIndex() == ichain &&
                             m_FeaElementVec[j]->GetElementType() == FeaElement::FEA_BEAM )
                        {
                            if ( !partheader )
                            {
                                partheader = true;

                                fprintf( bdf_fp, "\n" );
                                fprintf( bdf_fp, "$ %s %s %d beam\n", m_FeaPartNameVec[i].c_str(), m_StructName.c_str(), ichain );
                            }

                            m_FeaElementVec[j]->WriteNASTRAN( bdf_fp, elem_id, cap_property_id, noffset, eoffset );
                            beam_elem_id_vec.push_back( elem_id );
                            FeaMeshMgr.MarkPropMatUsed( cap_property_id );

                            elem_id++;
                        }
                    }
                    // Write beam element set
                    name = m_FeaPartNameVec[i] + "_" + m_StructName + "_" + to_string( ichain ) + "_BeamElements";
                    WriteNASTRANSet( dat_fp, nkey_fp, set_cnt, beam_elem_id_vec, name, eoffset );
                }
            }
        }

        // Write FeaSubSurfaces
        for ( unsigned int i = 0; i < m_NumFeaSubSurfs; i++ )
        {
            int property_id = m_SimpleSubSurfaceVec[i].GetFeaPropertyIndex();
            int cap_property_id = m_SimpleSubSurfaceVec[i].GetCapFeaPropertyIndex();

            shell_elem_id_vec.clear();

            // Write shell elements
            bool ssheader = false;

            for ( int j = 0; j < m_FeaElementVec.size(); j++ )
            {
                if ( m_FeaElementVec[j]->GetFeaSSIndex() == i && m_FeaElementVec[j]->GetElementType() != FeaElement::FEA_BEAM )
                {
                    if ( !ssheader )
                    {
                        ssheader = true;

                        fprintf( bdf_fp, "\n" );
                        fprintf( bdf_fp, "$ %s %s shell\n", m_SimpleSubSurfaceVec[i].GetName().c_str(), m_StructName.c_str() );
                    }

                    m_FeaElementVec[j]->WriteNASTRAN( bdf_fp, elem_id, property_id, noffset, eoffset );
                    shell_elem_id_vec.push_back( elem_id );
                    FeaMeshMgr.MarkPropMatUsed( property_id );

                    elem_id++;
                }
            }

            // Write shell element set
            name = m_SimpleSubSurfaceVec[i].GetName() + "_" + m_StructName + "_ShellElements";
            WriteNASTRANSet( dat_fp, nkey_fp, set_cnt, shell_elem_id_vec, name, eoffset );

            for ( int ichain = 0; ichain < m_FeaPartNumChainsVec[ i ]; ichain++ )
            {

                beam_elem_id_vec.clear();

                // Write beam elements
                ssheader = false;

                for ( int j = 0; j < m_FeaElementVec.size(); j++ )
                {
                    if ( m_FeaElementVec[j]->GetFeaSSIndex() == i &&
                         m_FeaElementVec[j]->GetChainIndex() == ichain &&
                         m_FeaElementVec[j]->GetElementType() == FeaElement::FEA_BEAM )
                    {
                        if ( !ssheader )
                        {
                            ssheader = true;

                            fprintf( bdf_fp, "\n" );
                            fprintf( bdf_fp, "$ %s %s %d beam\n", m_SimpleSubSurfaceVec[i].GetName().c_str(), m_StructName.c_str(), ichain );
                        }

                        m_FeaElementVec[j]->WriteNASTRAN( bdf_fp, elem_id, cap_property_id, noffset, eoffset );
                        beam_elem_id_vec.push_back( elem_id );
                        FeaMeshMgr.MarkPropMatUsed( cap_property_id );

                        elem_id++;
                    }
                }
                // Write beam element set
                name = m_SimpleSubSurfaceVec[i].GetName() + "_" + m_StructName + "_" + to_string( ichain ) + "_BeamElements";
                WriteNASTRANSet( dat_fp, nkey_fp, set_cnt, beam_elem_id_vec, name, eoffset );
            }
        }
    }
}

void CloseNASTRAN( FILE *dat_fp, FILE *bdf_header_fp, FILE *bdf_fp, FILE *nkey_fp )
{
    if ( dat_fp )
    {
        fclose( dat_fp );
    }

    if ( bdf_header_fp && bdf_fp )
    {
        AppendFile_BtoA( bdf_header_fp, bdf_fp );

        // Close open files
        fclose( bdf_header_fp );
        fclose( bdf_fp );
    }

    if ( nkey_fp )
    {
        fclose( nkey_fp );
    }
}

void FeaMesh::WriteNASTRANSet( FILE* dat_fp, FILE* nkey_fp, int & set_num, vector < long long int > set_ids, const string &set_name, const long long int &offset )
{
    if ( set_ids.size() > 0 && dat_fp )
    {
        fprintf( dat_fp, "\n$ %d, %s\n", set_num, set_name.c_str() );
        fprintf( dat_fp, "SET %d = ", set_num );

        for ( size_t i = 0; i < set_ids.size(); i++ )
        {
            fprintf( dat_fp, "%lld", set_ids[i] + offset );

            if ( i != set_ids.size() - 1 )
            {
                fprintf( dat_fp, "," );

                if ( ( i + 1 ) % 9 == 0 ) // 9 IDs per line
                {
                    fprintf( dat_fp, "\n" );
                }
            }
        }

        fprintf( dat_fp, "\n" );

        if ( nkey_fp ) // Write to NASTRAN key file if defined
        {
            fprintf( nkey_fp, "%d,%s\n", set_num, set_name.c_str() );
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
        fprintf( fp, "** Num_Nodes:       %llu\n", m_NumNodes );
        fprintf( fp, "** Num_Els:         %llu\n", m_NumEls );
        fprintf( fp, "** Num_Tris:        %llu\n", m_NumTris );
        fprintf( fp, "** Num_Quads:       %llu\n", m_NumQuads );
        fprintf( fp, "** Num_Beams:       %llu\n", m_NumBeams );
        fprintf( fp, "** Node_Offset:     %llu\n", m_StructSettings.m_NodeOffset );
        fprintf( fp, "** Element_Offset:  %llu\n", m_StructSettings.m_ElementOffset );
        fprintf( fp, "\n" );
    }
}

void FeaMesh::WriteCalculixNodes( FILE* fp )
{
    unsigned long long int noffset = m_StructSettings.m_NodeOffset;

    // This code does not currently support mixed quads and tris.
    // Element sets must be made unique.  Properties and orientation should only be written for
    // element sets that are actually used.

    if ( fp )
    {
        int elem_id = 0;
        char str[256];

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
                    fprintf( fp, "%llu,%f,%f,%f\n", fxpt.m_NodeIndex[j] + noffset, fxpt.m_Pnt[j][0], fxpt.m_Pnt[j][1], fxpt.m_Pnt[j][2] );
                }
            }

            fprintf( fp, "\n" );
        }

        //==== Write nodes from FeaParts ====//
        for ( unsigned int i = 0; i < m_NumFeaParts; i++ )
        {
            if ( m_FeaPartTypeVec[i] != vsp::FEA_FIX_POINT )
            {
                bool NodeHeader = false;

                for ( unsigned int j = 0; j < (int)m_FeaNodeVec.size(); j++ )
                {
                    if ( m_FeaNodeVecUsed[ j ] )
                    {
                        if ( m_FeaNodeVec[ j ]->HasOnlyTag( i ) )
                        {
                            if ( !NodeHeader )
                            {
                                fprintf( fp, "** %s %s\n", m_FeaPartNameVec[i].c_str(), m_StructName.c_str() );
                                fprintf( fp, "*NODE, NSET=N%s_%s\n", m_FeaPartNameVec[i].c_str(), m_StructName.c_str() );
                                NodeHeader = true;
                            }
                            m_FeaNodeVec[j]->WriteCalculix( fp, noffset );
                        }
                    }
                }

                fprintf( fp, "\n" );
            }
        }

        //==== Write SubSurfaces ====//
        for ( unsigned int i = 0; i < m_NumFeaSubSurfs; i++ )
        {
            bool SubSurfHeader = false;

            for ( unsigned int j = 0; j < (int)m_FeaNodeVec.size(); j++ )
            {
                if ( m_FeaNodeVecUsed[ j ] )
                {
                    if ( m_FeaNodeVec[ j ]->HasOnlyTag( i + m_NumFeaParts ) )
                    {
                        if ( !SubSurfHeader )
                        {
                            fprintf( fp, "** %s %s\n", m_SimpleSubSurfaceVec[i].GetName().c_str(), m_StructName.c_str() );
                            fprintf( fp, "*NODE, NSET=N%s_%s\n", m_SimpleSubSurfaceVec[i].GetName().c_str(), m_StructName.c_str() );
                            SubSurfHeader = true;
                        }
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
            if ( m_FeaNodeVecUsed[ j ] )
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
            if ( m_FeaNodeVecUsed[ i ] &&
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
    unsigned long long int noffset = m_StructSettings.m_NodeOffset;
    unsigned long long int eoffset = m_StructSettings.m_ElementOffset;

    // This code does not currently support mixed quads and tris.
    // Element sets must be made unique.  Properties and orientation should only be written for
    // element sets that are actually used.

    if ( fp )
    {
        int elem_id = 1;
        char str[256];

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

        //==== Write elements from FeaParts ====//
        for ( unsigned int i = 0; i < m_NumFeaParts; i++ )
        {
            if ( m_FeaPartTypeVec[i] != vsp::FEA_FIX_POINT )
            {
                fprintf( fp, "** %s %s\n", m_FeaPartNameVec[i].c_str(), m_StructName.c_str() );

                int surf_num = m_FeaPartNumSurfVec[i];

                for ( int isurf = 0; isurf < surf_num; isurf++ )
                {
                    if ( !m_StructSettings.m_ConvertToQuadsFlag && m_FeaPartKeepDelShellElementsVec[i] == vsp::FEA_KEEP )
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

                    if ( m_StructSettings.m_ConvertToQuadsFlag && m_FeaPartKeepDelShellElementsVec[i] == vsp::FEA_KEEP )
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

                    if ( m_FeaPartCreateBeamElementsVec[i] )
                    {
                        for ( int ichain = 0; ichain < m_FeaPartNumChainsVec[i]; ichain++ )
                        {
                            fprintf( fp, "*ELEMENT, TYPE=B32R, ELSET=EB%s_%s_%d_%d_CAP\n", m_FeaPartNameVec[i].c_str(), m_StructName.c_str(), isurf, ichain );

                            for ( int j = 0; j < m_FeaElementVec.size(); j++ )
                            {
                                if ( m_FeaElementVec[j]->GetFeaPartIndex() == i &&
                                     m_FeaElementVec[j]->GetElementType() == FeaElement::FEA_BEAM &&
                                     m_FeaElementVec[j]->GetFeaSSIndex() < 0 &&
                                     m_FeaElementVec[j]->GetChainIndex() == ichain &&
                                     m_FeaElementVec[j]->GetFeaPartSurfNum() == isurf )
                                {
                                    elem_id++;
                                    m_FeaElementVec[j]->WriteCalculix( fp, elem_id, noffset, eoffset );
                                }
                            }

                            if ( m_StructSettings.m_BeamPerElementNormal )
                            {
                                // Write Normal Vectors
                                fprintf( fp, "\n" );
                                fprintf( fp, "*NORMAL\n" );

                                for ( int j = 0; j < m_FeaElementVec.size(); j++ )
                                {
                                    if ( m_FeaElementVec[j]->GetFeaPartIndex() == i &&
                                         m_FeaElementVec[j]->GetElementType() == FeaElement::FEA_BEAM &&
                                         m_FeaElementVec[j]->GetFeaSSIndex() < 0 &&
                                         m_FeaElementVec[j]->GetChainIndex() == ichain &&
                                         m_FeaElementVec[j]->GetFeaPartSurfNum() == isurf )
                                    {
                                        FeaBeam* beam = dynamic_cast<FeaBeam*>( m_FeaElementVec[j] );
                                        assert( beam );
                                        beam->WriteCalculixNormal( fp, noffset, eoffset );
                                    }
                                }
                            }

                            fprintf( fp, "\n" );
                        }
                    }
                }
            }
        }

        //==== Write SubSurfaces ====//
        for ( unsigned int i = 0; i < m_NumFeaSubSurfs; i++ )
        {
            int surf_num = m_SimpleSubSurfaceVec[i].GetFeaOrientationVec().size();

            for ( int isurf = 0; isurf < surf_num; isurf++ )
            {
                if ( !m_StructSettings.m_ConvertToQuadsFlag && m_SimpleSubSurfaceVec[i].m_KeepDelShellElements == vsp::FEA_KEEP )
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

                if ( m_StructSettings.m_ConvertToQuadsFlag && m_SimpleSubSurfaceVec[i].m_KeepDelShellElements == vsp::FEA_KEEP )
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

                if ( m_SimpleSubSurfaceVec[i].m_CreateBeamElements )
                {
                    for ( int ichain = 0; ichain < m_FeaPartNumChainsVec[i]; ichain++ )
                    {

                        fprintf( fp, "\n" );
                        fprintf( fp, "*ELEMENT, TYPE=B32R, ELSET=EB%s_%s_%d_%d_CAP\n", m_SimpleSubSurfaceVec[i].GetName().c_str(), m_StructName.c_str(), isurf, ichain );

                        for ( int j = 0; j < m_FeaElementVec.size(); j++ )
                        {
                            if ( m_FeaElementVec[j]->GetFeaSSIndex() == i &&
                                 m_FeaElementVec[j]->GetElementType() == FeaElement::FEA_BEAM &&
                                 m_FeaElementVec[j]->GetChainIndex() == ichain &&
                                 m_FeaElementVec[j]->GetFeaPartSurfNum() == isurf )
                            {
                                elem_id++;
                                m_FeaElementVec[j]->WriteCalculix( fp, elem_id, noffset, eoffset );
                            }
                        }


                        if ( m_StructSettings.m_BeamPerElementNormal )
                        {
                            // Write Normal Vectors
                            fprintf( fp, "\n" );
                            fprintf( fp, "*NORMAL\n" );

                            for ( int j = 0; j < m_FeaElementVec.size(); j++ )
                            {
                                if ( m_FeaElementVec[j]->GetFeaSSIndex() == i &&
                                     m_FeaElementVec[j]->GetElementType() == FeaElement::FEA_BEAM &&
                                     m_FeaElementVec[j]->GetChainIndex() == ichain &&
                                     m_FeaElementVec[j]->GetFeaPartSurfNum() == isurf )
                                {
                                    FeaBeam* beam = dynamic_cast<FeaBeam*>( m_FeaElementVec[j] );
                                    assert( beam );
                                    beam->WriteCalculixNormal( fp, noffset, eoffset );
                                }
                            }
                        }

                        fprintf( fp, "\n" );
                    }
                }
            }
        }
    }
}

void FeaMesh::WriteCalculixBCs( FILE* fp )
{
    unsigned long long int noffset = m_StructSettings.m_NodeOffset;

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
                    if ( m_FeaPartKeepDelShellElementsVec[i] == vsp::FEA_KEEP )
                    {
                        FeaMeshMgr.MarkPropMatUsed( property_id );

                        fprintf( fp, "\n" );
                        char ostr[256];
                        snprintf( ostr, sizeof( str ), "O%s_%s_%d", m_FeaPartNameVec[i].c_str(), m_StructName.c_str(), isurf );

                        if ( !m_StructSettings.m_ConvertToQuadsFlag )
                        {
                            snprintf( str, sizeof( str ), "E%s_%s_%d", m_FeaPartNameVec[ i ].c_str(), m_StructName.c_str(), isurf );
                            FeaMeshMgr.GetSimplePropertyVec()[ property_id ].WriteCalculix( fp, str, ostr );
                        }
                        else
                        {
                            snprintf( str, sizeof( str ), "E%s_%s_%d", m_FeaPartNameVec[ i ].c_str(), m_StructName.c_str(), isurf );
                            FeaMeshMgr.GetSimplePropertyVec()[ property_id ].WriteCalculix( fp, str, ostr );
                        }

                        vec3d orient = m_PartSurfOrientation[i][isurf];
                        // int otype = srf->GetFeaOrientationType();
                        fprintf( fp, "\n" );
                        fprintf( fp, "*ORIENTATION, NAME=%s, SYSTEM=RECTANGULAR\n", ostr );
                        fprintf( fp, "%f,%f,%f,0.0,0.0,1.0\n", orient.x(), orient.y(), orient.z());
                    }

                    if ( m_FeaPartCreateBeamElementsVec[i] )
                    {
                        FeaMeshMgr.MarkPropMatUsed( cap_property_id );

                        for ( int ichain = 0; ichain < m_FeaPartNumChainsVec[i]; ichain++ )
                        {

                            fprintf( fp, "\n" );
                            snprintf( str, sizeof( str ), "EB%s_%s_%d_%d_CAP", m_FeaPartNameVec[i].c_str(), m_StructName.c_str(), isurf, ichain );
                            FeaMeshMgr.GetSimplePropertyVec()[cap_property_id].WriteCalculix( fp, str, "" );

                            if ( !m_StructSettings.m_BeamPerElementNormal )
                            {
                                for ( int j = 0; j < m_FeaElementVec.size(); j++ )
                                {
                                    if ( m_FeaElementVec[ j ]->GetFeaPartIndex() == i &&
                                         m_FeaElementVec[ j ]->GetElementType() == FeaElement::FEA_BEAM &&
                                         m_FeaElementVec[ j ]->GetFeaSSIndex() < 0 &&
                                         m_FeaElementVec[ j ]->GetChainIndex() == ichain &&
                                         m_FeaElementVec[ j ]->GetFeaPartSurfNum() == isurf )
                                    {
                                        FeaBeam *beam = dynamic_cast<FeaBeam *>( m_FeaElementVec[ j ] );
                                        assert( beam );
                                        beam->WriteCalculixNormal( fp );
                                        break;
                                    }
                                }
                            }
                        }
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
                if ( m_SimpleSubSurfaceVec[i].m_KeepDelShellElements == vsp::FEA_KEEP )
                {
                    FeaMeshMgr.MarkPropMatUsed( property_id );

                    fprintf( fp, "\n" );
                    char ostr[256];
                    snprintf( ostr, sizeof( ostr ), "O%s_%s_%d", m_SimpleSubSurfaceVec[i].GetName().c_str(), m_StructName.c_str(), isurf );

                    if ( !m_StructSettings.m_ConvertToQuadsFlag )
                    {
                        snprintf( str, sizeof( str ), "E%s_%s_%d", m_SimpleSubSurfaceVec[i].GetName().c_str(), m_StructName.c_str(), isurf );
                        FeaMeshMgr.GetSimplePropertyVec()[property_id].WriteCalculix( fp, str, ostr );
                    }
                    else
                    {
                        snprintf( str, sizeof( str ), "E%s_%s_%d", m_SimpleSubSurfaceVec[i].GetName().c_str(), m_StructName.c_str(), isurf );
                        FeaMeshMgr.GetSimplePropertyVec()[property_id].WriteCalculix( fp, str, ostr );
                    }

                    vec3d orient = ovec[isurf];
                    // int otype = m_SimpleSubSurfaceVec[i].GetFeaOrientationType();
                    fprintf( fp, "\n" );
                    fprintf( fp, "*ORIENTATION, NAME=%s, SYSTEM=RECTANGULAR\n", ostr );
                    fprintf( fp, "%f,%f,%f,0.0,0.0,1.0\n", orient.x(), orient.y(), orient.z() );
                }

                if ( m_SimpleSubSurfaceVec[i].m_CreateBeamElements )
                {
                    FeaMeshMgr.MarkPropMatUsed( cap_property_id );

                    fprintf( fp, "\n" );
                    snprintf( str, sizeof( str ), "EB%s_%s_%d_CAP", m_SimpleSubSurfaceVec[i].GetName().c_str(), m_StructName.c_str(), isurf );
                    FeaMeshMgr.GetSimplePropertyVec()[cap_property_id].WriteCalculix( fp, str, "" );

                    if ( !m_StructSettings.m_BeamPerElementNormal )
                    {
                        for ( int j = 0; j < m_FeaElementVec.size(); j++ )
                        {
                            if ( m_FeaElementVec[ j ]->GetFeaSSIndex() == i &&
                                 m_FeaElementVec[ j ]->GetElementType() == FeaElement::FEA_BEAM &&
                                 m_FeaElementVec[ j ]->GetFeaPartSurfNum() == isurf )
                            {
                                FeaBeam *beam = dynamic_cast<FeaBeam *>( m_FeaElementVec[ j ] );
                                assert( beam );
                                beam->WriteCalculixNormal( fp );
                                break;
                            }
                        }
                    }
                }
            }
        }

        fprintf( fp, "\n" );
    }
}

void FeaMesh::WriteGmsh()
{
    string fn = GetStructSettingsPtr()->GetExportFileName( vsp::FEA_GMSH_FILE_NAME );
    FILE* fp = fopen( fn.c_str(), "w" );

    if ( fp )
    {
        WriteGmsh( fp );
        fclose( fp );
    }
}

void FeaMesh::WriteGmsh( FILE* fp )
{
    if ( fp )
    {
        WriteGmshHeader( fp );

        fprintf( fp, "$PhysicalNames\n" );
        WriteGmshNames( fp );
        fprintf( fp, "$EndPhysicalNames\n" );

        // Count FeaNodes
        int node_count = 0;
        for ( unsigned int j = 0; j < (int)m_FeaNodeVec.size(); j++ )
        {
            if ( m_FeaNodeVecUsed[ j ] )
            {
                node_count++;
            }
        }

        //==== Write Nodes ====//
        fprintf( fp, "$Nodes\n" );
        fprintf( fp, "%u\n", node_count );
        WriteGmshNodes( fp );

        //==== Write FeaElements ====//
        fprintf( fp, "$Elements\n" );
        fprintf( fp, "%d\n", (int)m_FeaElementVec.size() );

        int ele_cnt = 1;
        WriteGmshElements( fp, ele_cnt );

        // Note: Material properties are not supported in *.msh file
    }
}

void FeaMesh::WriteGmshHeader( FILE* fp )
{
    //=====================================================================================//
    //============== Write Gmsh File ======================================================//
    //=====================================================================================//
    fprintf( fp, "$MeshFormat\n" );
    fprintf( fp, "2.2 0 %d\n", ( int )sizeof( double ) );
    fprintf( fp, "$EndMeshFormat\n" );
}

void FeaMesh::WriteGmshNames( FILE* fp )
{
    //==== Group and Name FeaParts ====//
    fprintf( fp, "%llu\n", m_NumFeaParts - m_NumFeaFixPoints );
    for ( unsigned int i = 0; i < m_NumFeaParts; i++ )
    {
        if ( m_FeaPartTypeVec[i] != vsp::FEA_FIX_POINT )
        {
            fprintf( fp, "9 %d \"%s\"\n", i + 1, m_FeaPartNameVec[i].c_str() );
        }
    }
}

void FeaMesh::WriteGmshNodes( FILE* fp )
{
    unsigned long long int noffset = m_StructSettings.m_NodeOffset;

    for ( unsigned int j = 0; j < (int)m_FeaNodeVec.size(); j++ )
    {
        if ( m_FeaNodeVecUsed[ j ] )
        {
            m_FeaNodeVec[j]->WriteGmsh( fp, noffset );
        }
    }
}

void FeaMesh::WriteGmshElements( FILE* fp, int & ele_cnt )
{
    unsigned long long int noffset = m_StructSettings.m_NodeOffset;
    unsigned long long int eoffset = m_StructSettings.m_ElementOffset;

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
}

void FeaMesh::WriteSTL()
{
    string fn = GetStructSettingsPtr()->GetExportFileName( vsp::FEA_STL_FILE_NAME );
    FILE* fp = fopen( fn.c_str(), "w" );

    if ( fp )
    {
        WriteSTL( fp );
        fclose( fp );
    }
}

void FeaMesh::WriteSTL( FILE* fp )
{
    if ( fp )
    {
        for ( unsigned int i = 0; i < m_NumFeaParts; i++ )
        {
            int surf_num = m_FeaPartNumSurfVec[i];

            for ( int isurf = 0; isurf < surf_num; isurf++ )
            {
                if ( m_FeaPartKeepDelShellElementsVec[i] == vsp::FEA_KEEP )
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
                if ( m_SimpleSubSurfaceVec[i].m_KeepDelShellElements == vsp::FEA_KEEP )
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
    string fn = GetStructSettingsPtr()->GetExportFileName( vsp::FEA_MASS_FILE_NAME );
    FILE* fp = fopen( fn.c_str(), "w" );

    if ( fp )
    {
        ComputeWriteMass( fp );
        fclose( fp );
    }
}

void FeaMesh::ComputeWriteMass( FILE* fp )
{
    m_TotalMass = 0.0;

    if ( fp )
    {
        fprintf( fp, "...FEA Mesh...\n" );
        fprintf( fp, "Mass_Unit: %s\n", m_MassUnit.c_str() );
        fprintf( fp, "Num_Els: %llu\n", m_NumEls );
        fprintf( fp, "Num_Tris: %llu\n", m_NumTris );
        fprintf( fp, "Num_Quads: %llu\n", m_NumQuads );
        fprintf( fp, "Num_Beams: %llu\n", m_NumBeams );
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
        GetStructSettingsPtr()->m_ColorFacesFlag = fea_struct->GetStructSettingsPtr()->m_ColorFacesFlag.Get();
        GetStructSettingsPtr()->m_ColorTagReason = fea_struct->GetStructSettingsPtr()->m_ColorTagReason.Get();

        GetStructSettingsPtr()->m_DrawNodesFlag = fea_struct->GetStructSettingsPtr()->m_DrawNodesFlag.Get();
        GetStructSettingsPtr()->m_DrawBCNodesFlag = fea_struct->GetStructSettingsPtr()->m_DrawBCNodesFlag.Get();
        GetStructSettingsPtr()->m_DrawElementOrientVecFlag = fea_struct->GetStructSettingsPtr()->m_DrawElementOrientVecFlag.Get();

        GetStructSettingsPtr()->m_DrawBorderFlag = fea_struct->GetStructSettingsPtr()->m_DrawBorderFlag.Get();
        GetStructSettingsPtr()->m_DrawIsectFlag = fea_struct->GetStructSettingsPtr()->m_DrawIsectFlag.Get();
        GetStructSettingsPtr()->m_DrawRawFlag = fea_struct->GetStructSettingsPtr()->m_DrawRawFlag.Get();
        GetStructSettingsPtr()->m_DrawBinAdaptFlag = fea_struct->GetStructSettingsPtr()->m_DrawBinAdaptFlag.Get();
        GetStructSettingsPtr()->m_DrawCurveFlag = fea_struct->GetStructSettingsPtr()->m_DrawCurveFlag.Get();
        GetStructSettingsPtr()->m_DrawPntsFlag = fea_struct->GetStructSettingsPtr()->m_DrawPntsFlag.Get();
    }
}
