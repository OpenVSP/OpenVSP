//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#include "NGonMeshGeom.h"
#include "Vehicle.h"
#include "SubSurfaceMgr.h"

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
    m_BBox.Reset();

    if ( m_PGMesh.m_NodeList.size() > 0 )
    {
        Matrix4d transMat = GetTotalTransMat();

        list< PGNode* >::iterator n;
        for ( n = m_PGMesh.m_NodeList.begin(); n != m_PGMesh.m_NodeList.end(); ++n )
        {
            m_BBox.Update( transMat.xform( (*n)->m_Pnt ) );
        }
    }
    else
    {
        m_BBox.Update( vec3d( 0.0, 0.0, 0.0 ) );
    }
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
        f->m_iQuad = triVec[i]->m_iQuad;
        f->m_Tag = SubSurfaceMgr.GetTag( triVec[i]->m_Tags );

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

void NGonMeshGeom::PolygonizeMesh()
{
    // Make vector copy of list so edges can be removed from list without invalidating active list iterator.
    vector< PGEdge* > eVec( m_PGMesh.m_EdgeList.begin(), m_PGMesh.m_EdgeList.end() );

    for ( int i = 0; i < eVec.size(); i++ )
    {
        PGEdge* e = eVec[i];

        // Verify removal of (*e) is OK.
        if ( e->m_FaceVec.size() == 2 )
        {
            PGFace *f0 = e->m_FaceVec[0];
            PGFace *f1 = e->m_FaceVec[1];

            if ( f0->m_iQuad >= 0 &&
                 f0->m_iQuad == f1->m_iQuad &&
                 f0->m_Tag == f1->m_Tag )
            {
                m_PGMesh.RemoveEdgeMergeFaces( e );
            }
        }
    }
}

void NGonMeshGeom::UpdateDrawObj()
{
    Matrix4d trans = GetTotalTransMat();
    vec3d zeroV = m_ModelMatrix.xform( vec3d( 0.0, 0.0, 0.0 ) );

    unsigned int num_uniq_tags = SubSurfaceMgr.GetNumTags();

    m_WireShadeDrawObj_vec.clear();
    m_WireShadeDrawObj_vec.resize( num_uniq_tags * 2 );

    map<int, DrawObj*> face_dobj_map;
    map<int, DrawObj*> outline_dobj_map;
    map< std::vector<int>, int >::const_iterator mit;
    map< std::vector<int>, int > tagMap = SubSurfaceMgr.GetSingleTagMap();
    int cnt = 0;
    for ( mit = tagMap.begin(); mit != tagMap.end() ; ++mit )
    {
        outline_dobj_map[ mit->second ] = &m_WireShadeDrawObj_vec[ cnt ];
        face_dobj_map[ mit->second ] = &m_WireShadeDrawObj_vec[ cnt + num_uniq_tags ];
        cnt++;
    }

    for ( list< PGFace* >::iterator f = m_PGMesh.m_FaceList.begin() ; f != m_PGMesh.m_FaceList.end(); ++f )
    {
        DrawObj* d_obj = outline_dobj_map[ (*f)->m_Tag ];

        for ( int i = 0; i < (*f)->m_EdgeVec.size(); i++ )
        {
            PGEdge *e = (*f)->m_EdgeVec[i];
            d_obj->m_PntVec.push_back( trans.xform( e->m_N0->m_Pnt ) );
            d_obj->m_PntVec.push_back( trans.xform( e->m_N1->m_Pnt ) );
        }
    }

    for ( list< PGFace* >::iterator f = m_PGMesh.m_FaceList.begin() ; f != m_PGMesh.m_FaceList.end(); ++f )
    {
        vector< PGNode* > nodVec;
        (*f)->GetNodesAsTris( nodVec );
        vec3d norm = m_ModelMatrix.xform( (*f)->m_Nvec ) - zeroV;

        DrawObj* d_obj = face_dobj_map[ (*f)->m_Tag ];

        for ( int i = 0; i < nodVec.size(); i++ )
        {
            if ( nodVec[i] )
            {
                d_obj->m_PntVec.push_back( trans.xform( nodVec[ i ]->m_Pnt ) );
                d_obj->m_NormVec.push_back( norm );
            }
        }
    }

    //==== Bounding Box ====//
    m_HighlightDrawObj.m_PntVec = m_BBox.GetBBoxDrawLines();

    // Flag the DrawObjects as changed
    for ( int i = 0 ; i < ( int )m_WireShadeDrawObj_vec.size(); i++ )
    {
        m_WireShadeDrawObj_vec[i].m_GeomChanged = true;
    }
}

void NGonMeshGeom::LoadDrawObjs( vector< DrawObj* > & draw_obj_vec )
{
    unsigned int num_uniq_tags = SubSurfaceMgr.GetNumTags();

    // Calculate constants for color sequence.
    const int ncgrp = 6; // Number of basic colors
    const int ncstep = (int)ceil((double)num_uniq_tags/(double)ncgrp);
    const double nctodeg = 360.0/(ncgrp*ncstep);

    Geom::LoadDrawObjs( draw_obj_vec );
    for ( int i = 0 ; i < num_uniq_tags ; i++ )
    {
        // Color sequence -- go around color wheel ncstep times with slight
        // offset from ncgrp basic colors.
        // Note, (cnt/ncgrp) uses integer division resulting in floor.
        double deg = 0 + ( ( i % ncgrp ) * ncstep + ( i / ncgrp ) ) * nctodeg;

        if ( deg > 360 )
        {
            deg = (int)deg % 360;
        }

        vec3d rgb = m_WireShadeDrawObj_vec[i].ColorWheel( deg );
        rgb.normalize();

        for ( int j = 0; j < 2; j++ )
        {
            int k = j * num_uniq_tags + i;
            m_WireShadeDrawObj_vec[ k ].m_MaterialInfo.Ambient[ 0 ] = ( float ) rgb.x() / 5.0f;
            m_WireShadeDrawObj_vec[ k ].m_MaterialInfo.Ambient[ 1 ] = ( float ) rgb.y() / 5.0f;
            m_WireShadeDrawObj_vec[ k ].m_MaterialInfo.Ambient[ 2 ] = ( float ) rgb.z() / 5.0f;
            m_WireShadeDrawObj_vec[ k ].m_MaterialInfo.Ambient[ 3 ] = ( float ) 1.0f;

            m_WireShadeDrawObj_vec[ k ].m_MaterialInfo.Diffuse[ 0 ] = 0.4f + ( float ) rgb.x() / 10.0f;
            m_WireShadeDrawObj_vec[ k ].m_MaterialInfo.Diffuse[ 1 ] = 0.4f + ( float ) rgb.y() / 10.0f;
            m_WireShadeDrawObj_vec[ k ].m_MaterialInfo.Diffuse[ 2 ] = 0.4f + ( float ) rgb.z() / 10.0f;
            m_WireShadeDrawObj_vec[ k ].m_MaterialInfo.Diffuse[ 3 ] = 1.0f;

            m_WireShadeDrawObj_vec[ k ].m_MaterialInfo.Specular[ 0 ] = 0.04f + 0.7f * ( float ) rgb.x();
            m_WireShadeDrawObj_vec[ k ].m_MaterialInfo.Specular[ 1 ] = 0.04f + 0.7f * ( float ) rgb.y();
            m_WireShadeDrawObj_vec[ k ].m_MaterialInfo.Specular[ 2 ] = 0.04f + 0.7f * ( float ) rgb.z();
            m_WireShadeDrawObj_vec[ k ].m_MaterialInfo.Specular[ 3 ] = 1.0f;

            m_WireShadeDrawObj_vec[ k ].m_MaterialInfo.Emission[ 0 ] = ( float ) rgb.x() / 20.0f;
            m_WireShadeDrawObj_vec[ k ].m_MaterialInfo.Emission[ 1 ] = ( float ) rgb.y() / 20.0f;
            m_WireShadeDrawObj_vec[ k ].m_MaterialInfo.Emission[ 2 ] = ( float ) rgb.z() / 20.0f;
            m_WireShadeDrawObj_vec[ k ].m_MaterialInfo.Emission[ 3 ] = 1.0f;

            m_WireShadeDrawObj_vec[ k ].m_MaterialInfo.Shininess = 32.0f;

            m_WireShadeDrawObj_vec[ k ].m_LineColor = rgb;
        }


        // Outline.
        m_WireShadeDrawObj_vec[i].m_Type = DrawObj::VSP_LINES;
        m_WireShadeDrawObj_vec[i].m_Visible = GetSetFlag( vsp::SET_SHOWN );
        // Faces.
        int k = i + num_uniq_tags;
        m_WireShadeDrawObj_vec[k].m_Visible = GetSetFlag( vsp::SET_SHOWN );

        switch( m_GuiDraw.GetDrawType() )
        {
            case vsp::DRAW_TYPE::GEOM_DRAW_WIRE:
                m_WireShadeDrawObj_vec[k].m_Type = DrawObj::VSP_WIRE_TRIS;
                m_WireShadeDrawObj_vec[k].m_Visible = false;
                break;

            case vsp::DRAW_TYPE::GEOM_DRAW_HIDDEN:
                m_WireShadeDrawObj_vec[k].m_Type = DrawObj::VSP_HIDDEN_TRIS;
                break;

            case vsp::DRAW_TYPE::GEOM_DRAW_SHADE:
                m_WireShadeDrawObj_vec[k].m_Type = DrawObj::VSP_SHADED_TRIS;
                m_WireShadeDrawObj_vec[i].m_Visible = false;
                break;

            case vsp::DRAW_TYPE::GEOM_DRAW_NONE:
                m_WireShadeDrawObj_vec[k].m_Type = DrawObj::VSP_SHADED_TRIS;
                m_WireShadeDrawObj_vec[k].m_Visible = false;
                m_WireShadeDrawObj_vec[i].m_Visible = false;
                break;

                // Does not support Texture Mapping.  Render Shaded instead.
            case vsp::DRAW_TYPE::GEOM_DRAW_TEXTURE:
                m_WireShadeDrawObj_vec[k].m_Type = DrawObj::VSP_SHADED_TRIS;
                m_WireShadeDrawObj_vec[i].m_Visible = false;
                break;
        }
    }


    for ( int i = 0; i < m_FeatureDrawObj_vec.size(); i++ )
    {
        m_FeatureDrawObj_vec[i].m_Type = DrawObj::VSP_LINE_STRIP;
    }

}
