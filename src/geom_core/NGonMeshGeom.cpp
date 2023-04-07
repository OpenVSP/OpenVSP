//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#include "NGonMeshGeom.h"
#include "Vehicle.h"

//==== Constructor ====//
NGonMeshGeom::NGonMeshGeom( Vehicle* vehicle_ptr ) : Geom( vehicle_ptr )
{
    m_Name = "NGonMeshGeom";
    m_Type.m_Name = "NGonMesh";
    m_Type.m_Type = NGON_GEOM_TYPE;

    // Disable Parameters that don't make sense for NGonMeshGeom
    m_SymPlanFlag.Deactivate();
    m_SymAxFlag.Deactivate();
    m_SymRotN.Deactivate();
    m_Density.Deactivate();
    m_ShellFlag.Deactivate();
    m_MassArea.Deactivate();
    m_MassPrior.Deactivate();

    m_ScaleMatrix.loadIdentity();
    m_ScaleFromOrig.Init( "Scale_From_Original", "XForm", this, 1, 1.0e-5, 1.0e12 );

    Update();
}

//==== Destructor ====//
NGonMeshGeom::~NGonMeshGeom()
{
}

void NGonMeshGeom::UpdateSurf()
{
}

//==== Get Total Transformation Matrix from Original Points ====//
Matrix4d NGonMeshGeom::GetTotalTransMat() const
{
    Matrix4d retMat;
    retMat.initMat( m_ScaleMatrix );
    retMat.postMult( m_ModelMatrix );

    return retMat;
}

void NGonMeshGeom::Scale()
{
    double currentScale = m_Scale() / m_LastScale();
    m_ScaleFromOrig *= currentScale;
    m_ScaleMatrix.loadIdentity();
    m_ScaleMatrix.scale( m_ScaleFromOrig() );
    m_LastScale = m_Scale();
}

void NGonMeshGeom::UpdateBBox()
{
    int i;
    m_BBox.Reset();
    Matrix4d transMat = GetTotalTransMat();

}

//==== Encode XML ====//
xmlNodePtr NGonMeshGeom::EncodeXml( xmlNodePtr & node )
{
    Geom::EncodeXml( node );
    xmlNodePtr ngon_node = xmlNewChild( node, NULL, BAD_CAST "NGonMeshGeom", NULL );

    return ngon_node;
}

//==== Decode XML ====//
xmlNodePtr NGonMeshGeom::DecodeXml( xmlNodePtr & node )
{
    Geom::DecodeXml( node );

    xmlNodePtr ngon_node = XmlUtil::GetNode( node, "NGonMeshGeom", 0 );
    if ( ngon_node )
    {

    }

    return ngon_node;
}

void NGonMeshGeom::BuildFromTMesh( const vector< TNode* > nodeVec, const vector< TTri* > &triVec )
{

    vector < PGNode* > nod( nodeVec.size() );
    for ( int i = 0; i < nodeVec.size(); i++ )
    {
        vec2d uw( nodeVec[i]->m_UWPnt.x(), nodeVec[i]->m_UWPnt.y() );
        nod[i] = m_PGMesh.AddNode( nodeVec[i]->m_Pnt, uw );
    }

    for ( int i = 0; i < triVec.size(); i++ )
    {
        PGFace *f = m_PGMesh.AddFace( );
        f->m_Nvec = triVec[i]->m_Norm;

        PGEdge *e1 = m_PGMesh.AddEdge( nod[triVec[i]->m_N0->m_ID], nod[triVec[i]->m_N1->m_ID] );
        PGEdge *e2 = m_PGMesh.AddEdge( nod[triVec[i]->m_N1->m_ID], nod[triVec[i]->m_N2->m_ID] );
        PGEdge *e3 = m_PGMesh.AddEdge( nod[triVec[i]->m_N2->m_ID], nod[triVec[i]->m_N0->m_ID] );

        e1->AddConnectFace( f );
        e2->AddConnectFace( f );
        e3->AddConnectFace( f );

        f->AddEdge( e1 );
        f->AddEdge( e2 );
        f->AddEdge( e3 );
    }
}

void NGonMeshGeom::UpdateDrawObj()
{
    m_WireShadeDrawObj_vec.clear();
    m_WireShadeDrawObj_vec.resize( 1 );

    unsigned int num_tris = m_PGMesh.m_FaceList.size();
    unsigned int pi = 0;

    m_WireShadeDrawObj_vec[0].m_PntVec.resize( num_tris * 4 );
    m_WireShadeDrawObj_vec[0].m_NormVec.resize( num_tris * 4 );


    list< PGFace* >::iterator f;
    for ( f = m_PGMesh.m_FaceList.begin() ; f != m_PGMesh.m_FaceList.end(); ++f )
    {
        vector< PGNode* > nodVec;
        (*f)-> GetNodes( nodVec );
        vec3d norm = (*f)->m_Nvec;

        for ( int i = 0; i < nodVec.size(); i++ )
        {
            m_WireShadeDrawObj_vec[0].m_PntVec[pi] = nodVec[i]->m_Pnt;
            m_WireShadeDrawObj_vec[0].m_NormVec[pi] = norm;
            pi++;
        }
    }

    // Flag the DrawObjects as changed
    for ( int i = 0 ; i < ( int )m_WireShadeDrawObj_vec.size(); i++ )
    {
        m_WireShadeDrawObj_vec[i].m_GeomChanged = true;
    }
}

void NGonMeshGeom::LoadDrawObjs( vector< DrawObj* > & draw_obj_vec )
{
    // Calculate constants for color sequence.
    const int ncgrp = 6; // Number of basic colors
    const int ncstep = (int)ceil((double)1/(double)ncgrp);
    const double nctodeg = 360.0/(ncgrp*ncstep);

    Geom::LoadDrawObjs( draw_obj_vec );
    for ( int i = 0 ; i < ( int )m_WireShadeDrawObj_vec.size() ; i++ )
    {
        if ( true )
        {
            // Color sequence -- go around color wheel ncstep times with slight
            // offset from ncgrp basic colors.
            // Note, (cnt/ncgrp) uses integer division resulting in floor.
            double deg = 100 + ( ( i % ncgrp ) * ncstep + ( i / ncgrp ) ) * nctodeg;

            if ( deg > 360 )
            {
                deg = (int)deg % 360;
            }

            vec3d rgb = m_WireShadeDrawObj_vec[i].ColorWheel( deg );
            rgb.normalize();
            m_WireShadeDrawObj_vec[i].m_MaterialInfo.Ambient[0] = (float)rgb.x()/5.0f;
            m_WireShadeDrawObj_vec[i].m_MaterialInfo.Ambient[1] = (float)rgb.y()/5.0f;
            m_WireShadeDrawObj_vec[i].m_MaterialInfo.Ambient[2] = (float)rgb.z()/5.0f;
            m_WireShadeDrawObj_vec[i].m_MaterialInfo.Ambient[3] = (float)1.0f;

            m_WireShadeDrawObj_vec[i].m_MaterialInfo.Diffuse[0] = 0.4f + (float)rgb.x()/10.0f;
            m_WireShadeDrawObj_vec[i].m_MaterialInfo.Diffuse[1] = 0.4f + (float)rgb.y()/10.0f;
            m_WireShadeDrawObj_vec[i].m_MaterialInfo.Diffuse[2] = 0.4f + (float)rgb.z()/10.0f;
            m_WireShadeDrawObj_vec[i].m_MaterialInfo.Diffuse[3] = 1.0f;

            m_WireShadeDrawObj_vec[i].m_MaterialInfo.Specular[0] = 0.04f + 0.7f * (float)rgb.x();
            m_WireShadeDrawObj_vec[i].m_MaterialInfo.Specular[1] = 0.04f + 0.7f * (float)rgb.y();
            m_WireShadeDrawObj_vec[i].m_MaterialInfo.Specular[2] = 0.04f + 0.7f * (float)rgb.z();
            m_WireShadeDrawObj_vec[i].m_MaterialInfo.Specular[3] = 1.0f;

            m_WireShadeDrawObj_vec[i].m_MaterialInfo.Emission[0] = (float)rgb.x()/20.0f;
            m_WireShadeDrawObj_vec[i].m_MaterialInfo.Emission[1] = (float)rgb.y()/20.0f;
            m_WireShadeDrawObj_vec[i].m_MaterialInfo.Emission[2] = (float)rgb.z()/20.0f;
            m_WireShadeDrawObj_vec[i].m_MaterialInfo.Emission[3] = 1.0f;

            m_WireShadeDrawObj_vec[i].m_MaterialInfo.Shininess = 32.0f;

            m_WireShadeDrawObj_vec[i].m_LineColor = rgb;
        }


        switch( m_GuiDraw.GetDrawType() )
        {
            case vsp::DRAW_TYPE::GEOM_DRAW_WIRE:
                m_WireShadeDrawObj_vec[i].m_Type = DrawObj::VSP_LINES;
                break;

            case vsp::DRAW_TYPE::GEOM_DRAW_HIDDEN:
                m_WireShadeDrawObj_vec[i].m_Type = DrawObj::VSP_LINES;
                break;

            case vsp::DRAW_TYPE::GEOM_DRAW_SHADE:
                m_WireShadeDrawObj_vec[i].m_Type = DrawObj::VSP_LINES;
                break;

            case vsp::DRAW_TYPE::GEOM_DRAW_NONE:
                m_WireShadeDrawObj_vec[i].m_Type = DrawObj::VSP_LINES;
                m_WireShadeDrawObj_vec[i].m_Visible = false;
                break;

                // Does not support Texture Mapping.  Render Shaded instead.
            case vsp::DRAW_TYPE::GEOM_DRAW_TEXTURE:
                m_WireShadeDrawObj_vec[i].m_Type = DrawObj::VSP_LINES;
                break;
        }
    }


    for ( int i = 0; i < m_FeatureDrawObj_vec.size(); i++ )
    {
        m_FeatureDrawObj_vec[i].m_Type = DrawObj::VSP_LINE_STRIP;
    }

}
