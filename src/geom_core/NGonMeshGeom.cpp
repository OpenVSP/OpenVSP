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
    retMat.postMult( GetFlipMat() );
    retMat.postMult( m_ModelMatrix );

    return retMat;
}

void NGonMeshGeom::ApplyScale( double currentScale )
{
    m_ScaleFromOrig *= currentScale;
    m_ScaleMatrix.loadIdentity();
    m_ScaleMatrix.scale( m_ScaleFromOrig() );
}

void NGonMeshGeom::UpdateBBox()
{
    BndBox new_box;
    BuildPGBndBox( new_box );

    // The box was built into m_BBox directly, which left the BBox report-out Parms reading
    // zero for the life of the Geom, and left m_ScaleIndependentBBox empty -- so an
    // NGonMeshGeom counted for nothing in the Vehicle's scale independent box, which is what
    // sizes the gear ground plane, the auxiliary geom reference lengths and the engine
    // extension.  Assign them the way every other UpdateBBox does.
    if ( new_box != m_BBox )
    {
        m_BbXLen = new_box.GetMax( 0 ) - new_box.GetMin( 0 );
        m_BbYLen = new_box.GetMax( 1 ) - new_box.GetMin( 1 );
        m_BbZLen = new_box.GetMax( 2 ) - new_box.GetMin( 2 );

        m_BbXMin = new_box.GetMin( 0 );
        m_BbYMin = new_box.GetMin( 1 );
        m_BbZMin = new_box.GetMin( 2 );

        m_BBox = new_box;
        m_ScaleIndependentBBox = m_BBox;
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
        m_PGMulti.WriteVSPGeom( file_id, trans, GetFlipReversesNormal() );

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


//==== PGMeshRole: shared by NGonMeshGeom and a Clone of one ====//

void PGMeshRole::BuildPGBndBox( BndBox &bbox ) const
{
    bbox.Reset();

    PGMulti *pgmulti = GetPGMulti();
    if ( !pgmulti )
    {
        bbox.Update( vec3d( 0.0, 0.0, 0.0 ) );
        return;
    }

    PGMesh *pgm = pgmulti->GetActiveMesh();
    if ( pgm->m_NodeList.size() > 0 )
    {
        Matrix4d trans = GetPGTransMat();

        list< PGNode* >::iterator n;
        for ( n = pgm->m_NodeList.begin(); n != pgm->m_NodeList.end(); ++n )
        {
            bbox.Update( trans.xform( (*n)->m_Pt->m_Pnt ) );
        }
    }
    else
    {
        bbox.Update( vec3d( 0.0, 0.0, 0.0 ) );
    }
}

// The faces and their outlines.  Diagnostics (bad edges, wakes, labels, probes) are drawn
// only by the Geom that owns the mesh.
void PGMeshRole::BuildPGDrawObjs( vector < DrawObj > &draw_obj_vec ) const
{
    PGMulti *pgmulti = GetPGMulti();
    if ( !pgmulti )
    {
        draw_obj_vec.clear();
        return;
    }

    Matrix4d trans = GetPGTransMat();
    vec3d zeroV = trans.xform( vec3d( 0.0, 0.0, 0.0 ) );
    PGMesh *pgm = pgmulti->GetActiveMesh();

    unsigned int num_uniq_tags = pgmulti->GetNumTags();

    // Resize only when the tag count changes -- reusing the existing DrawObjs preserves
    // their point/normal vector allocations from the previous update.
    if ( draw_obj_vec.size() != num_uniq_tags * 2 )
    {
        draw_obj_vec.clear();
        draw_obj_vec.resize( num_uniq_tags * 2 );
    }
    for ( int i = 0; i < ( int )draw_obj_vec.size(); i++ )
    {
        draw_obj_vec[i].m_PntVec.clear();
        draw_obj_vec[i].m_NormVec.clear();
    }

    unordered_map<int, DrawObj*> face_dobj_map;
    unordered_map<int, DrawObj*> outline_dobj_map;
    map< std::vector<int>, int >::const_iterator mit;
    map< std::vector<int>, int > tagMap = pgmulti->GetSingleTagMap();
    int cnt = 0;
    for ( mit = tagMap.begin(); mit != tagMap.end() ; ++mit )
    {
        outline_dobj_map[ mit->second ] = &draw_obj_vec[ cnt ];
        face_dobj_map[ mit->second ] = &draw_obj_vec[ cnt + num_uniq_tags ];
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
        vec3d norm = trans.xform( (*f)->m_Nvec ) - zeroV;
        norm.normalize();

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

    // The renderer uploads vertices only when this is set, and clears it after each pass.
    for ( int i = 0 ; i < ( int )draw_obj_vec.size(); i++ )
    {
        draw_obj_vec[i].m_GeomChanged = true;
    }
}

void NGonMeshGeom::UpdateDrawObj()
{
    Matrix4d trans = GetTotalTransMat();

    PGMesh *pgm = m_PGMulti.GetActiveMesh();

    BuildPGDrawObjs( m_WireShadeDrawObj_vec );

    //==== Bounding Box ====//
    m_HighlightDrawObj.m_PntVec = m_BBox.GetBBoxDrawLines();
    m_HighlightDrawObj.m_GeomChanged = true;

    // Flag the DrawObjects as changed
    for ( int i = 0 ; i < ( int )m_WireShadeDrawObj_vec.size(); i++ )
    {
        m_WireShadeDrawObj_vec[i].m_GeomChanged = true;
    }
}

// The wakes and the mesh defects (edges with too few or too many faces, co-linear loops,
// nodes that double back), placed by placer.
void NGonMeshGeom::BuildMarkerDrawObjs( Geom* placer, vector< DrawObj > &marker_vec )
{
    PGMeshRole* source = Geom::CastTo< PGMeshRole >( placer );
    if ( !source )
    {
        marker_vec.clear();
        return;
    }

    Matrix4d trans = source->GetPGTransMat();

    PGMesh *pgm = m_PGMulti.GetActiveMesh();

    int nwwake = pgm->m_WingWakeVec.size();
    int nbwake = pgm->m_BodyWakeVec.size();
    int nbpwake = pgm->m_BodyNodeWakeVec.size();

    marker_vec.resize( nwwake + nbwake + nbpwake + NUM_NGON_DEFECT_MARKERS );

    // Calculate constants for color sequence.
    const int ncgrp = nwwake + nbwake + nbpwake; // Number of basic colors
    const int ncstep = 1;
    const double nctodeg = 360.0/(ncgrp*ncstep);

    int iwake = 0;
    for ( int iwwake = 0; iwwake < nwwake + nbwake; iwwake++, iwake++ )
    {
        DrawObj &wake_do = marker_vec[ iwake ];

        // Color sequence -- go around color wheel ncstep times with slight
        // offset from ncgrp basic colors.
        // Note, (cnt/ncgrp) uses integer division resulting in floor.
        double deg = 0 + ( ( iwake % ncgrp ) * ncstep + ( iwake / ncgrp ) ) * nctodeg;

        if ( deg > 360 )
        {
            deg = (int)deg % 360;
        }

        vec3d rgb = wake_do.ColorWheel( deg );
        rgb.normalize();

        wake_do.m_Type = DrawObj::VSP_LINE_STRIP;
        wake_do.m_LineWidth = 5;
        wake_do.m_LineColor = rgb;
        wake_do.m_Screen = DrawObj::VSP_MAIN_SCREEN;

        char str[255];
        snprintf( str, sizeof( str ),  "_%d", iwake );
        wake_do.m_GeomID = placer->GetID() + "Feature_" + str;

        wake_do.m_GeomChanged = true;

        vector< PGNode* > nodVec;
        if ( iwwake < nwwake )
        {
            GetNodes( pgm->m_WingWakeVec[iwwake], nodVec );
        }
        else
        {
            GetNodes( pgm->m_BodyWakeVec[iwwake - nwwake], nodVec );
        }

        wake_do.m_PntVec.resize( nodVec.size() );
        for ( int i = 0; i < nodVec.size(); i++ )
        {
            if ( nodVec[i] )
            {
                wake_do.m_PntVec[i] = trans.xform( nodVec[i]->m_Pt->m_Pnt );
            }
        }
    }

    for ( int ibpwake = 0; ibpwake < nbpwake; ibpwake++, iwake++ )
    {
        DrawObj &node_do = marker_vec[ iwake ];

        // Color sequence -- go around color wheel ncstep times with slight
        // offset from ncgrp basic colors.
        // Note, (cnt/ncgrp) uses integer division resulting in floor.
        double deg = 0 + ( ( iwake % ncgrp ) * ncstep + ( iwake / ncgrp ) ) * nctodeg;

        if ( deg > 360 )
        {
            deg = (int)deg % 360;
        }

        vec3d rgb = node_do.ColorWheel( deg );
        rgb.normalize();

        node_do.m_Type = DrawObj::VSP_POINTS;
        node_do.m_PointSize = 8;
        node_do.m_PointColor = rgb;
        node_do.m_Screen = DrawObj::VSP_MAIN_SCREEN;

        char str[255];
        snprintf( str, sizeof( str ),  "_%d", iwake );
        node_do.m_GeomID = placer->GetID() + "Feature_" + str;

        node_do.m_GeomChanged = true;

        node_do.m_PntVec.clear();
        node_do.m_PntVec.push_back( trans.xform( pgm->m_BodyNodeWakeVec[ibpwake]->m_Pt->m_Pnt ) );
    }

    DrawObj &bad_few = marker_vec[ iwake + NGON_BAD_EDGE_FEW ];
    DrawObj &bad_many = marker_vec[ iwake + NGON_BAD_EDGE_MANY ];
    DrawObj &colinear = marker_vec[ iwake + NGON_COLINEAR_LOOP ];
    DrawObj &double_back = marker_vec[ iwake + NGON_DOUBLE_BACK_NODE ];

    bad_few.m_PntVec.clear();
    bad_few.m_LineWidth = 8;
    bad_few.m_LineColor = DrawObj::Color( DrawObj::BLUE );
    bad_few.m_Screen = DrawObj::VSP_MAIN_SCREEN;
    bad_few.m_GeomID = placer->GetID() + "Bad_Edges_Few";
    bad_few.m_Type = DrawObj::VSP_LINES;


    bad_many.m_PntVec.clear();
    bad_many.m_LineWidth = 8;
    bad_many.m_LineColor = DrawObj::Color( DrawObj::RED );
    bad_many.m_Screen = DrawObj::VSP_MAIN_SCREEN;
    bad_many.m_GeomID = placer->GetID() + "Bad_Edges_Many";
    bad_many.m_Type = DrawObj::VSP_LINES;


    list< PGEdge* >::iterator e;
    for ( e = pgm->m_EdgeList.begin() ; e != pgm->m_EdgeList.end(); ++e )
    {
        if ( ( *e )->m_FaceVec.size() < 2 )
        {
            bad_few.m_PntVec.push_back( trans.xform( ( *e )->m_N0->m_Pt->m_Pnt ) );
            bad_few.m_PntVec.push_back( trans.xform( ( *e )->m_N1->m_Pt->m_Pnt ) );
        }
        if ( ( *e )->m_FaceVec.size() > 2 )
        {
            bad_many.m_PntVec.push_back( trans.xform( ( *e )->m_N0->m_Pt->m_Pnt ) );
            bad_many.m_PntVec.push_back( trans.xform( ( *e )->m_N1->m_Pt->m_Pnt ) );
        }
    }
    bad_few.m_GeomChanged = true;
    bad_many.m_GeomChanged = true;

    colinear.m_PntVec.clear();
    colinear.m_LineWidth = 8;
    colinear.m_LineColor = DrawObj::Color( DrawObj::ORANGE );
    colinear.m_Screen = DrawObj::VSP_MAIN_SCREEN;
    colinear.m_GeomID = placer->GetID() + "Colinear";
    colinear.m_Type = DrawObj::VSP_LINES;

    for ( int i = 0; i < pgm->m_EdgeLoopVec.size(); i++ )
    {
        vector < PGEdge * > eloop = pgm->m_EdgeLoopVec[ i ];

        for ( int j = 0; j < eloop.size(); j++ )
        {
            PGEdge *e = eloop[ j ];
            colinear.m_PntVec.push_back( trans.xform( e->m_N0->m_Pt->m_Pnt ) );
            colinear.m_PntVec.push_back( trans.xform( e->m_N1->m_Pt->m_Pnt ) );
        }
    }
    colinear.m_GeomChanged = true;

    double_back.m_PntVec.clear();
    double_back.m_PointSize = 10;
    double_back.m_PointColor = DrawObj::Color( DrawObj::BLACK );
    double_back.m_Screen = DrawObj::VSP_MAIN_SCREEN;
    double_back.m_GeomID = placer->GetID() + "DoubleBack";
    double_back.m_Type = DrawObj::VSP_POINTS;

    for ( int i = 0; i < pgm->m_DoubleBackNode.size(); i++ )
    {
        double_back.m_PntVec.push_back( trans.xform( pgm->m_DoubleBackNode[i]->m_Pt->m_Pnt ) );
    }
    double_back.m_GeomChanged = true;
}

// The wakes, or the defects in their place -- this Geom's switch says which.
void NGonMeshGeom::SetMarkerVisibility( Geom* placer, vector< DrawObj > &marker_vec )
{
    bool visible = placer->GetSetFlag( vsp::SET_SHOWN );
    int nwake = ( int )marker_vec.size() - NUM_NGON_DEFECT_MARKERS;

    for ( int i = 0; i < ( int )marker_vec.size(); i++ )
    {
        if ( i < nwake )
        {
            marker_vec[i].m_Visible = !m_ShowNonManifoldEdges() && visible;
        }
        else
        {
            marker_vec[i].m_Visible = m_ShowNonManifoldEdges() && visible;
        }
    }
}


void PGMeshRole::LoadPGDrawObjs( vector < DrawObj > &draw_obj_vec, int drawtype, bool visible ) const
{
    PGMulti *pgmulti = GetPGMulti();
    if ( !pgmulti )
    {
        return;
    }

    unsigned int num_uniq_tags = pgmulti->GetNumTags();

    // BuildPGDrawObjs makes one pair per tag; anything else means they are not built yet.
    if ( draw_obj_vec.size() < num_uniq_tags * 2 )
    {
        return;
    }

    // Calculate constants for color sequence.
    const int ncgrp = 6; // Number of basic colors
    const int ncstep = (int)ceil((double)num_uniq_tags/(double)ncgrp);
    const double nctodeg = 360.0/(ncgrp*ncstep);

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

        vec3d rgb = draw_obj_vec[i].ColorWheel( deg );
        rgb.normalize();

        for ( int j = 0; j < 2; j++ )
        {
            int k = j * num_uniq_tags + i;
            draw_obj_vec[ k ].m_MaterialInfo.Ambient[ 0 ] = ( float ) rgb.x() / 5.0f;
            draw_obj_vec[ k ].m_MaterialInfo.Ambient[ 1 ] = ( float ) rgb.y() / 5.0f;
            draw_obj_vec[ k ].m_MaterialInfo.Ambient[ 2 ] = ( float ) rgb.z() / 5.0f;
            draw_obj_vec[ k ].m_MaterialInfo.Ambient[ 3 ] = ( float ) 1.0f;

            draw_obj_vec[ k ].m_MaterialInfo.Diffuse[ 0 ] = 0.4f + ( float ) rgb.x() / 10.0f;
            draw_obj_vec[ k ].m_MaterialInfo.Diffuse[ 1 ] = 0.4f + ( float ) rgb.y() / 10.0f;
            draw_obj_vec[ k ].m_MaterialInfo.Diffuse[ 2 ] = 0.4f + ( float ) rgb.z() / 10.0f;
            draw_obj_vec[ k ].m_MaterialInfo.Diffuse[ 3 ] = 1.0f;

            draw_obj_vec[ k ].m_MaterialInfo.Specular[ 0 ] = 0.04f + 0.7f * ( float ) rgb.x();
            draw_obj_vec[ k ].m_MaterialInfo.Specular[ 1 ] = 0.04f + 0.7f * ( float ) rgb.y();
            draw_obj_vec[ k ].m_MaterialInfo.Specular[ 2 ] = 0.04f + 0.7f * ( float ) rgb.z();
            draw_obj_vec[ k ].m_MaterialInfo.Specular[ 3 ] = 1.0f;

            draw_obj_vec[ k ].m_MaterialInfo.Emission[ 0 ] = ( float ) rgb.x() / 20.0f;
            draw_obj_vec[ k ].m_MaterialInfo.Emission[ 1 ] = ( float ) rgb.y() / 20.0f;
            draw_obj_vec[ k ].m_MaterialInfo.Emission[ 2 ] = ( float ) rgb.z() / 20.0f;
            draw_obj_vec[ k ].m_MaterialInfo.Emission[ 3 ] = 1.0f;

            draw_obj_vec[ k ].m_MaterialInfo.Shininess = 32.0f;

            draw_obj_vec[ k ].m_LineColor = rgb;
        }


        // Outline.
        draw_obj_vec[i].m_Type = DrawObj::VSP_LINES;
        draw_obj_vec[i].m_Visible = visible;
        // Faces.
        int k = i + num_uniq_tags;
        draw_obj_vec[k].m_Visible = visible;

        switch( drawtype )
        {
            case vsp::DRAW_TYPE::GEOM_DRAW_WIRE:
                draw_obj_vec[k].m_Type = DrawObj::VSP_WIRE_TRIS;
                draw_obj_vec[k].m_Visible = false;
                break;

            case vsp::DRAW_TYPE::GEOM_DRAW_HIDDEN:
                draw_obj_vec[k].m_Type = DrawObj::VSP_HIDDEN_TRIS;
                break;

            case vsp::DRAW_TYPE::GEOM_DRAW_SHADE:
                draw_obj_vec[k].m_Type = DrawObj::VSP_SHADED_TRIS;
                draw_obj_vec[i].m_Visible = false;
                break;

            case vsp::DRAW_TYPE::GEOM_DRAW_NONE:
                draw_obj_vec[k].m_Type = DrawObj::VSP_SHADED_TRIS;
                draw_obj_vec[k].m_Visible = false;
                draw_obj_vec[i].m_Visible = false;
                break;

                // Does not support Texture Mapping.  Render Shaded instead.
            case vsp::DRAW_TYPE::GEOM_DRAW_TEXTURE:
                draw_obj_vec[k].m_Type = DrawObj::VSP_SHADED_TRIS;
                draw_obj_vec[i].m_Visible = false;
                break;
        }
    }
}

void NGonMeshGeom::LoadDrawObjs( vector< DrawObj* > & draw_obj_vec )
{
    bool visible = GetSetFlag( vsp::SET_SHOWN );

    Geom::LoadDrawObjs( draw_obj_vec );

    LoadPGDrawObjs( m_WireShadeDrawObj_vec, m_GuiDraw.GetDrawType(), visible );

    for ( int i = 0; i < m_LabelDO_vec.size(); i++ )
    {
        m_LabelDO_vec[i].m_Visible = visible;
        draw_obj_vec.push_back( &m_LabelDO_vec[i] );
    }
}

vector< TMesh* > PGMeshRole::BuildPGTMeshVec( const Geom* geom_ptr ) const
{
    PGMulti *pgmulti = GetPGMulti();
    if ( !pgmulti )
    {
        return vector< TMesh* > ();
    }

    PGMesh *pgm = pgmulti->GetActiveMesh();
    vector<TMesh*> retTMeshVec(1);
    retTMeshVec[0] = new TMesh();
    retTMeshVec[0]->LoadGeomAttributes( geom_ptr );


//    retTMeshVec[0]->m_SurfCfdType = cfdsurftype;
//    retTMeshVec[0]->m_ThickSurf = thicksurf;
//    retTMeshVec[0]->m_SurfType = surftype;
//    retTMeshVec[0]->m_SurfNum = indx;
//    retTMeshVec[0]->m_PlateNum = platenum;
//    retTMeshVec[0]->m_UWPnts = uw_pnts;
//    retTMeshVec[0]->m_XYZPnts = pnts;
//    retTMeshVec[0]->m_Wmin = uw_pnts[0][0].y();


    // Placed as each triangle is built; a flip swaps two corners to keep the winding.
    Matrix4d TransMat = GetPGTransMat();
    bool flipnormal = GetRoleShapeFlipNormal();

    for ( list< PGFace* >::const_iterator f = pgm->m_FaceList.begin() ; f != pgm->m_FaceList.end(); ++f )
    {
        vector< PGNode* > nodVec;
        (*f)->GetNodesAsTris( nodVec );

        int ntri = nodVec.size() / 3;

        vec3d norm = TransMat.xformnorm( (*f)->m_Nvec );

        for ( int i = 0; i < ntri; i++ )
        {
            int inod = 3 * i;
            int i1 = inod + 1;
            int i2 = inod + 2;
            if ( flipnormal )
            {
                std::swap( i1, i2 );
            }

            vec3d v0 = TransMat.xform( nodVec[ inod ]->m_Pt->m_Pnt );
            vec3d v1 = TransMat.xform( nodVec[ i1 ]->m_Pt->m_Pnt );
            vec3d v2 = TransMat.xform( nodVec[ i2 ]->m_Pt->m_Pnt );
            retTMeshVec[0]->AddTri( v0, v1, v2, norm, (*f)->m_iQuad, (*f)->m_jref, (*f)->m_kref );
        }
    }

    return retTMeshVec;
}

vector<TMesh*> NGonMeshGeom::CreateTMeshVec( bool skipnegflipnormal, const int & n_ref ) const
{
    return BuildPGTMeshVec( this );
}