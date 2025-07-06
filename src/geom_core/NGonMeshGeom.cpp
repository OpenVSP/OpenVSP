//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#include "NGonMeshGeom.h"
#include "Vehicle.h"
#include "SubSurfaceMgr.h"
#include "MeshGeom.h"
#include "FileUtil.h"

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

    m_ShowNonManifoldEdges.Init( "ShowNonManifoldEdges", "Draw", this, false, false, true );

    m_ActiveMesh.Init( "ActiveMesh", "RefMesh", this, 0, 0, 10 );

    Update();
}

//==== Destructor ====//
NGonMeshGeom::~NGonMeshGeom()
{
}

void NGonMeshGeom::UpdateSurf()
{
    m_MainSurfVec.clear();
    if ( m_PGMulti.m_ActiveMesh != m_ActiveMesh() )
    {
        m_PGMulti.m_ActiveMesh = m_ActiveMesh();
    }
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

    PGMesh * pgm = m_PGMulti.GetActiveMesh();
    if ( pgm->m_NodeList.size() > 0 )
    {
        Matrix4d transMat = GetTotalTransMat();

        list< PGNode* >::iterator n;
        for ( n = pgm->m_NodeList.begin(); n != pgm->m_NodeList.end(); ++n )
        {
            m_BBox.Update( transMat.xform( (*n)->m_Pt->m_Pnt ) );
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

void NGonMeshGeom::SplitLEGeom()
{
    PGMesh *pgm = m_PGMulti.GetActiveMesh();
    // Make vector copy of list so faces can be removed from list without invalidating active list iterator.
    vector< PGFace* > fVec( pgm->m_FaceList.begin(), pgm->m_FaceList.end() );

    for ( int i = 0; i < fVec.size(); i++ )
    {
        PGFace *f = fVec[i];

        PGEdge *e = NULL;
        PGNode *n = f->FindDoubleBackNode( e );
        if ( n )
        {
            pgm->SplitFaceFromDoubleBackNode( f, e, n );
        }
    }
}

void NGonMeshGeom::Triangulate()
{
    PGMesh *pgm = m_PGMulti.GetActiveMesh();
    pgm->Triangulate();

    pgm->DumpGarbage();

    m_SurfDirty = true;
    Update();
}

void NGonMeshGeom::Report()
{
    m_PGMulti.Report();
}

void NGonMeshGeom::ClearTris()
{
    PGMesh *pgm = m_PGMulti.GetActiveMesh();
    pgm->ClearTris();
}

void NGonMeshGeom::RemovePotentialFiles( const string& file_name )
{
    // Main *.vspgeom file.
    if ( FileExist( file_name ) )
    {
        remove( file_name.c_str() );
    }

    string base_name = GetBasename( file_name );

    string key_name = base_name + ".vkey";
    if ( FileExist( key_name ) )
    {
        remove( key_name.c_str() );
    }

    string csf_name = base_name + ".csf";
    if ( FileExist( csf_name ) )
    {
        remove( csf_name.c_str() );
    }

    string taglist_name = base_name + ".ALL.taglist";
    if ( FileExist( taglist_name ) )
    {
        remove( taglist_name.c_str() );
    }

    string csf_taglist_name = base_name + ".ControlSurfaces.taglist";
    if ( FileExist( csf_taglist_name ) )
    {
        remove( csf_taglist_name.c_str() );
    }

    string tagfile_wildcard = base_name + "*.tag";
    std::vector < std::filesystem::path > tagfiles;
    tagfiles = get_files_matching_pattern( tagfile_wildcard );
    remove_files( tagfiles );
}


void NGonMeshGeom::WriteVSPGEOM( string fname, vector < string > &all_fnames )
{
    RemovePotentialFiles( fname );

    Matrix4d trans = GetTotalTransMat();

    FILE *file_id = fopen( fname.c_str(), "w" );

    if ( file_id )
    {
        all_fnames.push_back( fname );
        m_PGMulti.WriteVSPGeom( file_id, trans );

        fclose ( file_id );

        m_PGMulti.WriteTagFiles( fname, all_fnames );

        //==== Write Out tag key file ====//

        m_PGMulti.WriteVSPGEOMKeyFile( fname, all_fnames );

        vector < string > gidvec;
        vector < int > partvec;
        vector < int > surfvec;
        m_PGMulti.GetPartData( gidvec, partvec, surfvec );

        m_Vehicle->WriteControlSurfaceFile( fname, gidvec, partvec, surfvec, all_fnames );
    }
}

void NGonMeshGeom::UpdateDrawObj()
{
    Matrix4d trans = GetTotalTransMat();
    vec3d zeroV = m_ModelMatrix.xform( vec3d( 0.0, 0.0, 0.0 ) );

    PGMesh *pgm = m_PGMulti.GetActiveMesh();

    unsigned int num_uniq_tags = m_PGMulti.GetNumTags();

    m_WireShadeDrawObj_vec.clear();
    m_WireShadeDrawObj_vec.resize( num_uniq_tags * 2 );

    unordered_map<int, DrawObj*> face_dobj_map;
    unordered_map<int, DrawObj*> outline_dobj_map;
    map< std::vector<int>, int >::const_iterator mit;
    map< std::vector<int>, int > tagMap = m_PGMulti.GetSingleTagMap();
    int cnt = 0;
    for ( mit = tagMap.begin(); mit != tagMap.end() ; ++mit )
    {
        outline_dobj_map[ mit->second ] = &m_WireShadeDrawObj_vec[ cnt ];
        face_dobj_map[ mit->second ] = &m_WireShadeDrawObj_vec[ cnt + num_uniq_tags ];
        cnt++;
    }

    for ( list< PGFace* >::iterator f = pgm->m_FaceList.begin() ; f != pgm->m_FaceList.end(); ++f )
    {
        DrawObj* d_obj = outline_dobj_map[ (*f)->m_Tag ];

        for ( int i = 0; i < (*f)->m_EdgeVec.size(); i++ )
        {
            PGEdge *e = (*f)->m_EdgeVec[i];
            if ( true ) // e
            {
                d_obj->m_PntVec.push_back( trans.xform( e->m_N0->m_Pt->m_Pnt ) );
                d_obj->m_PntVec.push_back( trans.xform( e->m_N1->m_Pt->m_Pnt ) );
            }
        }
    }

    for ( list< PGFace* >::iterator f = pgm->m_FaceList.begin() ; f != pgm->m_FaceList.end(); ++f )
    {
        vector< PGNode* > nodVec;
        (*f)->GetNodesAsTris( nodVec );
        vec3d norm = m_ModelMatrix.xform( (*f)->m_Nvec ) - zeroV;

        DrawObj* d_obj = face_dobj_map[ (*f)->m_Tag ];

        for ( int i = 0; i < nodVec.size(); i++ )
        {
            if ( nodVec[i] && nodVec[i]->m_Pt )
            {
                d_obj->m_PntVec.push_back( trans.xform( nodVec[ i ]->m_Pt->m_Pnt ) );
                d_obj->m_NormVec.push_back( norm );
            }
        }
    }

    int nwwake = pgm->m_WingWakeVec.size();
    int nbwake = pgm->m_BodyWakeVec.size();
    int nbpwake = pgm->m_BodyNodeWakeVec.size();

    m_WakeEdgeDrawObj_vec.resize( nwwake + nbwake );

    // Calculate constants for color sequence.
    const int ncgrp = nwwake + nbwake + nbpwake; // Number of basic colors
    const int ncstep = 1;
    const double nctodeg = 360.0/(ncgrp*ncstep);

    int iwake = 0;
    for ( int iwwake = 0; iwwake < nwwake; iwwake++, iwake++ )
    {

        // Color sequence -- go around color wheel ncstep times with slight
        // offset from ncgrp basic colors.
        // Note, (cnt/ncgrp) uses integer division resulting in floor.
        double deg = 0 + ( ( iwake % ncgrp ) * ncstep + ( iwake / ncgrp ) ) * nctodeg;

        if ( deg > 360 )
        {
            deg = (int)deg % 360;
        }

        vec3d rgb = m_WakeEdgeDrawObj_vec[iwake].ColorWheel( deg );
        rgb.normalize();

        m_WakeEdgeDrawObj_vec[iwake].m_Type = DrawObj::VSP_LINE_STRIP;
        m_WakeEdgeDrawObj_vec[iwake].m_LineWidth = 5;
        m_WakeEdgeDrawObj_vec[iwake].m_LineColor = rgb;
        m_WakeEdgeDrawObj_vec[iwake].m_Screen = DrawObj::VSP_MAIN_SCREEN;

        char str[255];
        snprintf( str, sizeof( str ),  "_%d", iwake );
        m_WakeEdgeDrawObj_vec[iwake].m_GeomID = m_ID + "Feature_" + str;

        m_WakeEdgeDrawObj_vec[iwake].m_GeomChanged = true;

        vector< PGNode* > nodVec;
        GetNodes( pgm->m_WingWakeVec[iwwake], nodVec );

        m_WakeEdgeDrawObj_vec[iwake].m_PntVec.resize( nodVec.size() );
        for ( int i = 0; i < nodVec.size(); i++ )
        {
            if ( nodVec[i] )
            {
                m_WakeEdgeDrawObj_vec[iwake].m_PntVec[i] = trans.xform( nodVec[i]->m_Pt->m_Pnt );
            }
        }
    }

    for ( int ibwake = 0; ibwake < nbwake; ibwake++, iwake++ )
    {

        // Color sequence -- go around color wheel ncstep times with slight
        // offset from ncgrp basic colors.
        // Note, (cnt/ncgrp) uses integer division resulting in floor.
        double deg = 0 + ( ( iwake % ncgrp ) * ncstep + ( iwake / ncgrp ) ) * nctodeg;

        if ( deg > 360 )
        {
            deg = (int)deg % 360;
        }

        vec3d rgb = m_WakeEdgeDrawObj_vec[iwake].ColorWheel( deg );
        rgb.normalize();

        m_WakeEdgeDrawObj_vec[iwake].m_Type = DrawObj::VSP_LINE_STRIP;
        m_WakeEdgeDrawObj_vec[iwake].m_LineWidth = 5;
        m_WakeEdgeDrawObj_vec[iwake].m_LineColor = rgb;
        m_WakeEdgeDrawObj_vec[iwake].m_Screen = DrawObj::VSP_MAIN_SCREEN;

        char str[255];
        snprintf( str, sizeof( str ),  "_%d", iwake );
        m_WakeEdgeDrawObj_vec[iwake].m_GeomID = m_ID + "Feature_" + str;

        m_WakeEdgeDrawObj_vec[iwake].m_GeomChanged = true;

        vector< PGNode* > nodVec;
        GetNodes( pgm->m_BodyWakeVec[ibwake], nodVec );

        m_WakeEdgeDrawObj_vec[iwake].m_PntVec.resize( nodVec.size() );
        for ( int i = 0; i < nodVec.size(); i++ )
        {
            if ( nodVec[i] )
            {
                m_WakeEdgeDrawObj_vec[iwake].m_PntVec[i] = trans.xform( nodVec[i]->m_Pt->m_Pnt );
            }
        }
    }


    m_WakeNodeDrawObj_vec.resize( nbpwake );

    for ( int ibpwake = 0; ibpwake < nbpwake; ibpwake++, iwake++ )
    {
        // Color sequence -- go around color wheel ncstep times with slight
        // offset from ncgrp basic colors.
        // Note, (cnt/ncgrp) uses integer division resulting in floor.
        double deg = 0 + ( ( iwake % ncgrp ) * ncstep + ( iwake / ncgrp ) ) * nctodeg;

        if ( deg > 360 )
        {
            deg = (int)deg % 360;
        }

        vec3d rgb = m_WakeEdgeDrawObj_vec[iwake].ColorWheel( deg );
        rgb.normalize();

        m_WakeNodeDrawObj_vec[ibpwake].m_Type = DrawObj::VSP_POINTS;
        m_WakeNodeDrawObj_vec[ibpwake].m_PointSize = 8;
        m_WakeNodeDrawObj_vec[ibpwake].m_PointColor = rgb;
        m_WakeNodeDrawObj_vec[ibpwake].m_Screen = DrawObj::VSP_MAIN_SCREEN;

        char str[255];
        snprintf( str, sizeof( str ),  "_%d", iwake );
        m_WakeNodeDrawObj_vec[ibpwake].m_GeomID = m_ID + "Feature_" + str;

        m_WakeNodeDrawObj_vec[ibpwake].m_GeomChanged = true;

        vec3d pt = trans.xform( pgm->m_BodyNodeWakeVec[ibpwake]->m_Pt->m_Pnt );
        m_WakeNodeDrawObj_vec[ibpwake].m_PntVec.push_back( pt );
    }

    //==== Bounding Box ====//
    m_HighlightDrawObj.m_PntVec = m_BBox.GetBBoxDrawLines();

    // Flag the DrawObjects as changed
    for ( int i = 0 ; i < ( int )m_WireShadeDrawObj_vec.size(); i++ )
    {
        m_WireShadeDrawObj_vec[i].m_GeomChanged = true;
    }


    m_BadEdgeTooFewDO.m_PntVec.clear();
    m_BadEdgeTooFewDO.m_LineWidth = 8;
    m_BadEdgeTooFewDO.m_LineColor = DrawObj::Color( DrawObj::BLUE );
    m_BadEdgeTooFewDO.m_Screen = DrawObj::VSP_MAIN_SCREEN;
    m_BadEdgeTooFewDO.m_GeomID = GetID() + "Bad_Edges_Few";
    m_BadEdgeTooFewDO.m_Type = DrawObj::VSP_LINES;


    m_BadEdgeTooManyDO.m_PntVec.clear();
    m_BadEdgeTooManyDO.m_LineWidth = 8;
    m_BadEdgeTooManyDO.m_LineColor = DrawObj::Color( DrawObj::RED );
    m_BadEdgeTooManyDO.m_Screen = DrawObj::VSP_MAIN_SCREEN;
    m_BadEdgeTooManyDO.m_GeomID = GetID() + "Bad_Edges_Many";
    m_BadEdgeTooManyDO.m_Type = DrawObj::VSP_LINES;


    list< PGEdge* >::iterator e;
    for ( e = pgm->m_EdgeList.begin() ; e != pgm->m_EdgeList.end(); ++e )
    {
        if ( ( *e )->m_FaceVec.size() < 2 )
        {
            m_BadEdgeTooFewDO.m_PntVec.push_back( trans.xform( ( *e )->m_N0->m_Pt->m_Pnt ) );
            m_BadEdgeTooFewDO.m_PntVec.push_back( trans.xform( ( *e )->m_N1->m_Pt->m_Pnt ) );
        }
        if ( ( *e )->m_FaceVec.size() > 2 )
        {
            m_BadEdgeTooManyDO.m_PntVec.push_back( trans.xform( ( *e )->m_N0->m_Pt->m_Pnt ) );
            m_BadEdgeTooManyDO.m_PntVec.push_back( trans.xform( ( *e )->m_N1->m_Pt->m_Pnt ) );
        }
    }
    m_BadEdgeTooFewDO.m_GeomChanged = true;
    m_BadEdgeTooManyDO.m_GeomChanged = true;

    m_CoLinearLoopDO.m_PntVec.clear();
    m_CoLinearLoopDO.m_LineWidth = 8;
    m_CoLinearLoopDO.m_LineColor = DrawObj::Color( DrawObj::ORANGE );
    m_CoLinearLoopDO.m_Screen = DrawObj::VSP_MAIN_SCREEN;
    m_CoLinearLoopDO.m_GeomID = GetID() + "Colinear";
    m_CoLinearLoopDO.m_Type = DrawObj::VSP_LINES;

    for ( int i = 0; i < pgm->m_EdgeLoopVec.size(); i++ )
    {
        vector < PGEdge * > eloop = pgm->m_EdgeLoopVec[ i ];

        for ( int j = 0; j < eloop.size(); j++ )
        {
            PGEdge *e = eloop[ j ];
            m_CoLinearLoopDO.m_PntVec.push_back( trans.xform( e->m_N0->m_Pt->m_Pnt ) );
            m_CoLinearLoopDO.m_PntVec.push_back( trans.xform( e->m_N1->m_Pt->m_Pnt ) );
        }
    }
    m_CoLinearLoopDO.m_GeomChanged = true;

    m_DoubleBackNodeDO.m_PntVec.clear();
    m_DoubleBackNodeDO.m_PointSize = 10;
    m_DoubleBackNodeDO.m_PointColor = DrawObj::Color( DrawObj::BLACK );
    m_DoubleBackNodeDO.m_Screen = DrawObj::VSP_MAIN_SCREEN;
    m_DoubleBackNodeDO.m_GeomID = GetID() + "DoubleBack";
    m_DoubleBackNodeDO.m_Type = DrawObj::VSP_POINTS;

    for ( int i = 0; i < pgm->m_DoubleBackNode.size(); i++ )
    {
        m_DoubleBackNodeDO.m_PntVec.push_back( trans.xform( pgm->m_DoubleBackNode[i]->m_Pt->m_Pnt ) );
    }
    m_DoubleBackNodeDO.m_GeomChanged = true;

    // Debug DrawObj's to display jref, kref at each face center.
    if ( false )
    {
        int i = 0;
        m_LabelDO_vec.resize( pgm->m_FaceList.size() );
        for ( list< PGFace* >::iterator f = pgm->m_FaceList.begin() ; f != pgm->m_FaceList.end(); ++f )
        {
            m_LabelDO_vec[i].m_GeomID = GenerateRandomID( 4 ) + "_Probe";
            m_LabelDO_vec[i].m_Type = DrawObj::VSP_PROBE;
            m_LabelDO_vec[i].m_Screen = DrawObj::VSP_MAIN_SCREEN;
            m_LabelDO_vec[i].m_TextColor = vec3d( 0, 0, 0 );
            m_LabelDO_vec[i].m_TextSize = 1;

            m_LabelDO_vec[i].m_VisibleDirFlag = true;
            m_LabelDO_vec[i].m_VisTol = 89.0;
            m_LabelDO_vec[i].m_VisibleDir = ( *f )->m_Nvec;

            m_LabelDO_vec[i].m_Probe.Step = DrawObj::VSP_PROBE_STEP_COMPLETE;
            m_LabelDO_vec[i].m_Probe.Pt = ( *f )->ComputeCenter();
            m_LabelDO_vec[i].m_Probe.Norm = ( *f )->m_Nvec;
            m_LabelDO_vec[i].m_Probe.Len = 0.01;
            m_LabelDO_vec[i].m_GeomChanged = true;

            char str[255];
            snprintf( str, sizeof(str), "%d, %d", (*f)->m_jref, (*f)->m_kref );
            m_LabelDO_vec[i].m_Probe.Label = string( str );

            i++;
        }
    }
}

void NGonMeshGeom::LoadDrawObjs( vector< DrawObj* > & draw_obj_vec )
{
    PGMesh *pgm = m_PGMulti.GetActiveMesh();
    bool visible = GetSetFlag( vsp::SET_SHOWN );
    unsigned int num_uniq_tags = m_PGMulti.GetNumTags();

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
        m_WireShadeDrawObj_vec[i].m_Visible = visible;
        // Faces.
        int k = i + num_uniq_tags;
        m_WireShadeDrawObj_vec[k].m_Visible = visible;

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

    // Load Wake Lines
    for ( int i = 0; i < m_WakeEdgeDrawObj_vec.size(); i++ )
    {
        m_WakeEdgeDrawObj_vec[i].m_Visible = !m_ShowNonManifoldEdges() && visible;
        draw_obj_vec.push_back( &m_WakeEdgeDrawObj_vec[i] );
    }

    for ( int i = 0; i < m_WakeNodeDrawObj_vec.size(); i++ )
    {
        m_WakeNodeDrawObj_vec[i].m_Visible = !m_ShowNonManifoldEdges() && visible;
        draw_obj_vec.push_back( &m_WakeNodeDrawObj_vec[i] );
    }

    m_BadEdgeTooFewDO.m_Visible = m_ShowNonManifoldEdges() && visible;
    m_BadEdgeTooManyDO.m_Visible = m_ShowNonManifoldEdges() && visible;
    draw_obj_vec.push_back( &m_BadEdgeTooFewDO );
    draw_obj_vec.push_back( &m_BadEdgeTooManyDO );

    m_CoLinearLoopDO.m_Visible = m_ShowNonManifoldEdges() && visible;
    draw_obj_vec.push_back( &m_CoLinearLoopDO );

    m_DoubleBackNodeDO.m_Visible = m_ShowNonManifoldEdges() && visible;
    draw_obj_vec.push_back( &m_DoubleBackNodeDO );

    for ( int i = 0; i < m_LabelDO_vec.size(); i++ )
    {
        m_LabelDO_vec[i].m_Visible = visible;
        draw_obj_vec.push_back( &m_LabelDO_vec[i] );
    }
}

vector<TMesh*> NGonMeshGeom::CreateTMeshVec()
{
    PGMesh *pgm = m_PGMulti.GetActiveMesh();
    vector<TMesh*> retTMeshVec(1);
    retTMeshVec[0] = new TMesh();
    retTMeshVec[0]->LoadGeomAttributes( this );


//    retTMeshVec[0]->m_SurfCfdType = cfdsurftype;
//    retTMeshVec[0]->m_ThickSurf = thicksurf;
//    retTMeshVec[0]->m_SurfType = surftype;
//    retTMeshVec[0]->m_SurfNum = indx;
//    retTMeshVec[0]->m_PlateNum = platenum;
//    retTMeshVec[0]->m_UWPnts = uw_pnts;
//    retTMeshVec[0]->m_XYZPnts = pnts;
//    retTMeshVec[0]->m_Wmin = uw_pnts[0][0].y();


    for ( list< PGFace* >::const_iterator f = pgm->m_FaceList.begin() ; f != pgm->m_FaceList.end(); ++f )
    {
        vector< PGNode* > nodVec;
        (*f)->GetNodesAsTris( nodVec );

        int ntri = nodVec.size() / 3;

        for ( int i = 0; i < ntri; i++ )
        {
            int inod = 3 * i;
            vec3d v0 = nodVec[ inod ]->m_Pt->m_Pnt;
            vec3d v1 = nodVec[ inod + 1 ]->m_Pt->m_Pnt;
            vec3d v2 = nodVec[ inod + 2 ]->m_Pt->m_Pnt;
            retTMeshVec[0]->AddTri( v0, v2, v1, (*f)->m_Nvec, (*f)->m_iQuad, (*f)->m_jref, (*f)->m_kref );
        }
    }

    // Apply Transformations
    Matrix4d TransMat = GetTotalTransMat();
    TransformMeshVec( retTMeshVec, TransMat );

    return retTMeshVec;
}