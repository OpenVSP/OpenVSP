//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// CfdMeshMgr
//
//////////////////////////////////////////////////////////////////////

#include "CfdMeshMgr.h"
#include "SubSurfaceMgr.h"
#include "main.h"
#include "MeshAnalysis.h"
#include "ModeMgr.h"
#include "FileUtil.h"

#include <algorithm>

#ifdef DEBUG_CFD_MESH
// #include <direct.h>
#include <sys/stat.h>
#include <sys/types.h>
#endif


CfdMeshMgrSingleton::CfdMeshMgrSingleton() : SurfaceIntersectionSingleton()
{
    m_Vehicle = VehicleMgr.GetVehicle();

    m_CurrMainSurfIndx = 0;

    m_MeshInProgress = false;

    m_MessageName = "CFDMessage";
}

CfdMeshMgrSingleton::~CfdMeshMgrSingleton()
{
    CleanUp();

#ifdef DEBUG_CFD_MESH
    if ( m_DebugFile )
    {
        fclose( m_DebugFile );
    }
#endif

}

void CfdMeshMgrSingleton::GenerateMesh()
{
    m_MeshInProgress = true;

#ifdef DEBUG_TIME_OUTPUT
    addOutputText( "Init Timer\n" );
#endif

    addOutputText( "Transfer Mesh Settings\n" );
    TransferMeshSettings();

    addOutputText( "Fetching Bezier Surfaces\n" );

    vector< XferSurf > xfersurfs;
    FetchSurfs( xfersurfs );

    // UpdateSourcesAndWakes must be before m_Vehicle->HideAll() to prevent components 
    // being being added to or removed from the CFD Mesh set
    addOutputText( "Update Sources And Wakes\n" );
    UpdateSourcesAndWakes();
    WakeMgr.SetStretchMeshFlag( true );

    // Hide all geoms after fetching their surfaces
    m_Vehicle->HideAll();

    addOutputText( "Cleanup\n" );
    CleanUp();

    addOutputText( "Loading Bezier Surfaces\n" );
    LoadSurfs( xfersurfs );

    if ( GetSettingsPtr()->m_IntersectSubSurfs )
    {
        addOutputText( "Transfer Subsurface Data\n" );
        TransferSubSurfData();
    }

    addOutputText( "Clean Merge Surfaces\n" );
    CleanMergeSurfs( /* skip_duplicate_removal */ false );

    if ( m_SurfVec.size() == 0 )
    {
        addOutputText( "No Surfaces To Mesh\n" );
        m_MeshInProgress = false;
        MessageMgr::getInstance().Send( "ScreenMgr", "UpdateAllScreens" );
        return;
    }

    addOutputText( "Update Domain\n" );
    UpdateDomain();

    addOutputText( "Build Domain\n" );
    BuildDomain();

    addOutputText( "Build Grid\n" );
    BuildGrid();

    // addOutputText( "Intersect\n" ); // Output in intersect() itself.
    Intersect();

    if ( false )
    {
    addOutputText( "Degen Corners\n" );
    FindDegenCorners();

    addOutputText( "Add Degen Corner Chains\n" );
    AddDegenCornerChains();
    }

    addOutputText( "Binary Adaptation Curve Approximation\n" );
    BinaryAdaptIntCurves();

    addOutputText( "Build Target Map\n" );
    BuildTargetMap( CfdMeshMgrSingleton::VOCAL_OUTPUT );

    // addOutputText( "InitMesh\n" ); // Output inside InitMesh
    InitMesh( );

    addOutputText( "Sub Tag tris\n" );
    SubTagTris();

    addOutputText( "Remesh\n" );
    Remesh( CfdMeshMgrSingleton::VOCAL_OUTPUT );

    if ( GetSettingsPtr()->m_ConvertToQuadsFlag )
    {
        addOutputText( "ConvertToQuads\n" );
        ConvertToQuads();
    }

    addOutputText( "ConnectBorderNodes\n" );
    ConnectBorderNodes( false );        // No Wakes
    ConnectBorderNodes( true );         // Only Wakes

    addOutputText( "Post Mesh\n" );
    PostMesh();

    //addOutputText( "Triangle Quality\n");
    //string qual = CfdMeshMgr.GetQualString();
    //addOutputText( qual.c_str() );

    addOutputText( "Build Single Tag Map\n" );
    SubSurfaceMgr.BuildSingleTagMap();

    addOutputText( "Exporting Files\n" );
    ExportFiles();

    addOutputText( "Check Water Tight\n" );
    string resultTxt = CheckWaterTight();
    addOutputText( resultTxt );

    UpdateDrawObjs();

    m_MeshInProgress = false;
    MessageMgr::getInstance().Send( "ScreenMgr", "UpdateAllScreens" );
}

void CfdMeshMgrSingleton::TransferMeshSettings()
{
    m_CfdSettings = SimpleCfdMeshSettings();
    m_CfdSettings.CopyFrom( m_Vehicle->GetCfdSettingsPtr() );

    m_CfdGridDensity = SimpleCfdGridDensity();
    m_CfdGridDensity.CopyFrom( m_Vehicle->GetCfdGridDensityPtr() );

    if ( m_CfdSettings.m_ConvertToQuadsFlag )
    {
        // Increase target edge length because tris are split into quads.
        // A tri with edge length 1.0 will result in an average quad edge of 0.349
        m_CfdGridDensity.ScaleMesh( 2.536 );
    }
}

void CfdMeshMgrSingleton::CleanUp()
{
    SurfaceIntersectionSingleton::CleanUp();

    int i;
    //==== Delete Stored Nodes ====//
    for ( i = 0 ; i < ( int )m_nodeStore.size() ; i++ )
    {
        delete m_nodeStore[i];
    }
    m_nodeStore.clear();

    //==== Delete Bad Edges ====//
    for ( i = 0 ; i < ( int )m_BadEdges.size() ; i++ )
    {
        delete m_BadEdges[i];
    }
    m_BadEdges.clear();

    //==== Delete Bad Triangles ====//
    for ( i = 0 ; i < ( int )m_BadFaces.size() ; i++ )
    {
        delete m_BadFaces[i];
    }
    m_BadFaces.clear();

    // These are freed elsewhere, just need to clear references to them.
    m_DegenCorners.clear();
    m_DegenCornerChains.clear();

    // Clean up DrawObj's
    m_MeshBadEdgeDO = DrawObj();
    m_MeshBadTriDO = DrawObj();
    m_MeshBadQuadDO = DrawObj();

    m_BBoxLineStripDO = DrawObj();
    m_BBoxLinesDO = DrawObj();
    m_BBoxLineStripSymSplit = DrawObj();
    m_BBoxLineSymSplit = DrawObj();

    m_DegenCornerPointDO = DrawObj();
    m_DegenCornerEdgeDO = DrawObj();

    m_TagDO.clear();
    m_ReasonDO.clear();
}

void CfdMeshMgrSingleton::AdjustAllSourceLen( double mult )
{
    vector<string> geomVec = m_Vehicle->GetGeomVec();
    for ( int g = 0 ; g < ( int )geomVec.size() ; g++ )
    {
        Geom* gi = m_Vehicle->FindGeom( geomVec[g] );
        if ( gi )
        {
            vector< BaseSource* > sVec = gi->GetCfdMeshMainSourceVec();
            for ( int s = 0 ; s < ( int )sVec.size() ; s++ )
            {
                sVec[s]->AdjustLen( mult );
            }
        }
    }
}

void CfdMeshMgrSingleton::AdjustAllSourceRad( double mult )
{
    vector<string> geomVec = m_Vehicle->GetGeomVec();
    for ( int g = 0 ; g < ( int )geomVec.size() ; g++ )
    {
        Geom* gi = m_Vehicle->FindGeom( geomVec[g] );
        if ( gi )
        {
            vector< BaseSource* > sVec = gi->GetCfdMeshMainSourceVec();
            for ( int s = 0 ; s < ( int )sVec.size() ; s++ )
            {
                sVec[s]->AdjustRad( mult );
            }
        }
    }
}

void CfdMeshMgrSingleton::GUI_Val( const string &name, double val )
{
    BaseSource* source = GetCurrSource();
    if ( name == "GlobalEdgeSize"  )
    {
        GetGridDensityPtr()->m_BaseLen = val;
    }
    else if ( name == "MinEdgeSize"  )
    {
        GetGridDensityPtr()->m_MinLen = val;
    }
    else if ( name == "MaxGapSize"  )
    {
        GetGridDensityPtr()->m_MaxGap = val;
    }
    else if ( name == "NumCircSeg"  )
    {
        GetGridDensityPtr()->m_NCircSeg = val;
    }
    else if ( name == "GrowRatio"  )
    {
        GetGridDensityPtr()->m_GrowRatio = val;
    }
    else if ( name == "Length1" && source )
    {
        source->m_Len = val;
    }
    else if ( name == "Radius1" && source )
    {
        source->m_Rad = val;
    }
    else if ( name == "FarLength" )
    {
        GetGridDensityPtr()->m_FarMaxLen = val;
    }
    else if ( name == "FarGapSize"  )
    {
        GetGridDensityPtr()->m_FarMaxGap = val;
    }
    else if ( name == "FarCircSeg"  )
    {
        GetGridDensityPtr()->m_FarNCircSeg = val;
    }
    else
    {
        if ( source )
        {
            source->SetNamedVal( name, val );
        }
    }
}

void CfdMeshMgrSingleton::GUI_Val( const string &name, int val )
{
    if ( name == "SourceID" )
    {
        Geom* geom = m_Vehicle->FindGeom( m_CurrSourceGeomID );
        if( geom )
        {
            vector< BaseSource* > sVec = geom->GetCfdMeshMainSourceVec();
            if ( val >= 0 && val < ( int )sVec.size() )
            {
                geom->SetCurrSourceID( val );
            }
        }
    }
//  m_Vehicle->triggerDraw();

}

void CfdMeshMgrSingleton::GUI_Val( const string &name, const string &val )
{
    if ( name == "SourceName" )
    {
        BaseSource* s = GetCurrSource();
        if ( s )
        {
            s->SetName( val );
        }
    }
}

BaseSource* CfdMeshMgrSingleton::GetCurrSource()
{
    BaseSource* s = nullptr;
    Geom* g = m_Vehicle->FindGeom( m_CurrSourceGeomID );
    if( g )
    {
        int sourceID = g->GetCurrSourceID();
        vector< BaseSource* > sVec = g->GetCfdMeshMainSourceVec();

        if ( sourceID >= 0 && sourceID < ( int )sVec.size() )
        {
            s = sVec[sourceID];
        }
    }
    return s;
}

BaseSource* CfdMeshMgrSingleton::AddSource( int type )
{
    BaseSource* ret_source = nullptr;
    Geom* curr_geom = nullptr;
    curr_geom = m_Vehicle->FindGeom( m_CurrSourceGeomID );
    if ( !curr_geom )
    {
        return ret_source;
    }

    char str[256];
    int num_sources = curr_geom->GetCfdMeshMainSourceVec().size();

    if ( type == vsp::POINT_SOURCE )
    {
        PointSource* source = new PointSource();
        snprintf( str, sizeof( str ), "PointSource_srf_%d_%d", m_CurrMainSurfIndx, num_sources );
        source->SetName( str );
        source->m_Len = 0.1;
        source->m_Rad = 1.0;
        source->m_ULoc = 0.0;
        source->m_WLoc = 0.0;
        source->m_MainSurfIndx = m_CurrMainSurfIndx;

        curr_geom->AddCfdMeshSource( source );
        ret_source = source;
    }
    else if ( type == vsp::LINE_SOURCE )
    {
        LineSource* source = new LineSource();
        snprintf( str, sizeof( str ), "LineSource_srf_%d_%d", m_CurrMainSurfIndx, num_sources );
        source->SetName( str );
        source->m_Len = 0.1;
        source->m_Len2 = 0.1;
        source->m_Rad = 1.0;
        source->m_Rad2 = 2.0;
        source->m_ULoc1 = 0.0;
        source->m_WLoc1 = 0.0;
        source->m_ULoc2 = 1.0;
        source->m_WLoc2 = 0.0;
        source->m_MainSurfIndx = m_CurrMainSurfIndx;

        curr_geom->AddCfdMeshSource( source );
        ret_source = source;
    }
    else if ( type == vsp::BOX_SOURCE )
    {
        BoxSource* source = new BoxSource();
        snprintf( str, sizeof( str ), "BoxSource_srf_%d_%d", m_CurrMainSurfIndx, num_sources );
        source->SetName( str );
        source->m_Len = 0.1;
        source->m_Rad = 1.0;
        source->m_ULoc1 = 0.0;
        source->m_WLoc1 = 0.0;
        source->m_ULoc2 = 1.0;
        source->m_WLoc2 = 0.0;
        source->m_MainSurfIndx = m_CurrMainSurfIndx;

        curr_geom->AddCfdMeshSource( source );
        ret_source = source;
    }
    else if ( type == vsp::ULINE_SOURCE )
    {
        ULineSource* source = new ULineSource();
        snprintf( str, sizeof( str ), "ULineSource_srf_%d_%d", m_CurrMainSurfIndx, num_sources );
        source->SetName( str );
        source->m_Len = 0.1;
        source->m_Rad = 1.0;
        source->m_Val = 0.5;
        source->m_MainSurfIndx = m_CurrMainSurfIndx;

        curr_geom->AddCfdMeshSource( source );
        ret_source = source;
    }
    else if ( type == vsp::WLINE_SOURCE )
    {
        WLineSource* source = new WLineSource();
        snprintf( str, sizeof( str ), "WLineSource_srf_%d_%d", m_CurrMainSurfIndx, num_sources );
        source->SetName( str );
        source->m_Len = 0.1;
        source->m_Rad = 1.0;
        source->m_Val = 0.5;
        source->m_MainSurfIndx = m_CurrMainSurfIndx;

        curr_geom->AddCfdMeshSource( source );
        ret_source = source;
    }

    //==== Highlight/Edit The New Source ====//
    vector< BaseSource* > sVec = curr_geom->GetCfdMeshMainSourceVec();
    curr_geom->SetCurrSourceID( ( int )sVec.size() - 1 );

    return  ret_source;


}

void CfdMeshMgrSingleton::DeleteCurrSource()
{
    Geom* curr_geom = nullptr;

    curr_geom = m_Vehicle->FindGeom( m_CurrSourceGeomID );

    if ( curr_geom )
    {
        curr_geom->DelCurrSource();
    }
}

void CfdMeshMgrSingleton::DeleteAllSources()
{
    GetGridDensityPtr()->ClearSources();
    vector<string> geomVec = m_Vehicle->GetGeomVec();
    for ( int g = 0 ; g < ( int )geomVec.size() ; g++ )
    {
        Geom* geom = m_Vehicle->FindGeom( geomVec[g] );
        if ( geom )
        {
            geom->DelAllSources();
            geom->UpdateSources();
        }
    }
}

void CfdMeshMgrSingleton::Update()
{
    if ( !GetMeshInProgress() )
    {
        UpdateSourcesAndWakes();
    }

    UpdateDomain();

    m_Vehicle->GetCfdGridDensityPtr()->Update( 1.0 );
}

void CfdMeshMgrSingleton::UpdateSourcesAndWakes()
{
    GetGridDensityPtr()->ClearSources();

    int nset = GetCfdSettingsPtr()->m_SelectedSetIndex;
    int dset = GetCfdSettingsPtr()->m_SelectedDegenSetIndex;

    if ( GetCfdSettingsPtr()->m_UseMode )
    {
        Mode *m = ModeMgr.GetMode( GetCfdSettingsPtr()->m_ModeID );
        if ( m )
        {
            // Do not apply settings every time Sources and Wakes are updated.
            // m->ApplySettings();
            nset = m->m_NormalSet();
            dset = m->m_DegenSet();
        }
    }

    vector<string> geomVec = m_Vehicle->GetGeomVec();
    for ( int g = 0 ; g < ( int )geomVec.size() ; g++ )
    {
        Geom* geom = m_Vehicle->FindGeom( geomVec[g] );
        if ( geom )
        {
            if ( geom->GetSetFlag( nset ) || geom->GetSetFlag( dset ) )
            {
                geom->UpdateSources();
                vector< BaseSimpleSource* > sVec = geom->GetCfdMeshSimpSourceVec();

                for ( int s = 0 ; s < ( int )sVec.size() ; s++ )
                {
                    GetGridDensityPtr()->AddSource( sVec[s] );
                }
            }
        }
    }

    UpdateWakes();
}

void CfdMeshMgrSingleton::UpdateDomain()
{
    m_Vehicle->UpdateBBox();
    m_Domain = m_Vehicle->GetBndBox();

    vec3d lwh = vec3d( m_Domain.GetMax( 0 ) - m_Domain.GetMin( 0 ),
                       m_Domain.GetMax( 1 ) - m_Domain.GetMin( 1 ),
                       m_Domain.GetMax( 2 ) - m_Domain.GetMin( 2 ) );

    vec3d xyzc = m_Domain.GetCenter();

    if ( GetCfdSettingsPtr()->m_FarMeshFlag )
    {
        if( !GetCfdSettingsPtr()->m_FarCompFlag )
        {
            if ( GetCfdSettingsPtr()->m_FarAbsSizeFlag )
            {
                GetCfdSettingsPtr()->m_FarXScale = GetCfdSettingsPtr()->m_FarLength / lwh.x();
                GetCfdSettingsPtr()->m_FarYScale = GetCfdSettingsPtr()->m_FarWidth / lwh.y();
                GetCfdSettingsPtr()->m_FarZScale = GetCfdSettingsPtr()->m_FarHeight / lwh.z();

                lwh = vec3d( GetCfdSettingsPtr()->m_FarLength, GetCfdSettingsPtr()->m_FarWidth, GetCfdSettingsPtr()->m_FarHeight );
            }
            else
            {
                lwh.scale_x( GetCfdSettingsPtr()->m_FarXScale );
                lwh.scale_y( GetCfdSettingsPtr()->m_FarYScale );
                lwh.scale_z( GetCfdSettingsPtr()->m_FarZScale );

                GetCfdSettingsPtr()->m_FarLength = lwh.x();
                GetCfdSettingsPtr()->m_FarWidth = lwh.y();
                GetCfdSettingsPtr()->m_FarHeight = lwh.z();
            }

            if ( GetCfdSettingsPtr()->m_FarManLocFlag )
            {
                vec3d xyz0 = vec3d( GetCfdSettingsPtr()->m_FarXLocation, GetCfdSettingsPtr()->m_FarYLocation, GetCfdSettingsPtr()->m_FarZLocation );
                xyzc = xyz0;
                xyzc.v[0] += lwh.v[0] / 2.0;
            }
            else
            {
                vec3d xyz0 = xyzc;
                xyz0.v[0] -= lwh.v[0] / 2.0;

                GetCfdSettingsPtr()->m_FarXLocation = xyz0.x();
                GetCfdSettingsPtr()->m_FarYLocation = xyz0.y();
                GetCfdSettingsPtr()->m_FarZLocation = xyz0.z();
            }

            m_Domain.Reset();
            m_Domain.Update( xyzc - lwh / 2.0 );
            m_Domain.Update( xyzc + lwh / 2.0 );
        }
        else
        {
            m_Domain.Expand( 1.0 );
        }
    }
    else
    {
        m_Domain.Expand( 1.0 );
    }

    if ( GetCfdSettingsPtr()->m_HalfMeshFlag )
    {
        m_Domain.SetMin( 1, 0.0 );
    }

    CfdMeshMgr.UpdateBBoxDO( m_Domain );
}

void CfdMeshMgrSingleton::AddDefaultSources()
{
    if ( GetGridDensityPtr()->GetNumSources() == 0 )
    {
        vector<string> geomVec = m_Vehicle->GetGeomVec();
        for ( int g = 0 ; g < ( int )geomVec.size() ; g++ )
        {
            double base_len = GetGridDensityPtr()->m_BaseLen;
            Geom* gi = m_Vehicle->FindGeom( geomVec[g] );
            if ( gi )
            {
                gi->AddDefaultSources( base_len );
            }
        }
    }
}

void CfdMeshMgrSingleton::AddDefaultSourcesCurrGeom()
{
    Geom* curr_geom = nullptr;
    curr_geom = m_Vehicle->FindGeom( m_CurrSourceGeomID );
    if ( !curr_geom )
    {
        return;
    }

    double base_len = GetGridDensityPtr()->m_BaseLen;
    curr_geom->AddDefaultSources( base_len );
}

void CfdMeshMgrSingleton::BuildDomain()
{
    vector< Surf* > FFBox = CreateDomainSurfs();

    int inc = FFBox.size();

    //==== Bump up ID's for 'normal' components & add far field surfs ====//
    if ( inc > 0 )
    {
        for ( int i = 0 ; i < (int)m_SurfVec.size() ; i++ )
        {
            m_SurfVec[i]->SetCompID( m_SurfVec[i]->GetCompID() + inc );
            m_SurfVec[i]->SetSurfID( m_SurfVec[i]->GetSurfID() + inc );
            m_SurfVec[i]->SetUnmergedCompID( m_SurfVec[i]->GetUnmergedCompID() + inc );
        }

        for ( int i = 0 ; i < (int)FFBox.size() ; i++ )
        {
            m_SurfVec.push_back( FFBox[i] );
        }
    }


    //==== Mark & Change Modes for Component Far Field ====//
    if ( GetCfdSettingsPtr()->m_FarMeshFlag )
    {
        if ( GetCfdSettingsPtr()->m_FarCompFlag )
        {
            for ( int i = 0 ; i < (int)m_SurfVec.size() ; i++ )
            {
                if ( m_SurfVec[i]->GetGeomID() == GetCfdSettingsPtr()->m_FarGeomID )
                {
                    m_SurfVec[i]->SetFarFlag( true );
                    m_SurfVec[i]->SetSurfaceCfdType(vsp::CFD_TRANSPARENT);
                    m_SurfVec[i]->FlipFlipFlag();
                }
            }
        }
    }
}

void CfdMeshMgrSingleton::BuildGrid()
{
    SurfaceIntersectionSingleton::BuildGrid();

    for ( int i = 0 ; i < ( int )m_SurfVec.size() ; i++ )
    {
        m_SurfVec[i]->SetGridDensityPtr( GetGridDensityPtr() );
    }
}

void CfdMeshMgrSingleton::BuildTargetMap( int output_type )
{
    MSCloud ms_cloud;
    vector< MapSource* > allsources;

    int i;
    for ( i = 0 ; i < ( int )m_SurfVec.size() ; i++ )
    {
        m_SurfVec[i]->BuildTargetMap( allsources, i );
        m_SurfVec[i]->LimitTargetMap();
    }

    // Set up split sources to provide a source at the endpoint of curves where
    // mesh information is hard to transfer.
    list< MapSource* > splitSources;

    list< ISegChain* >::iterator c;
    for ( c = m_ISegChainList.begin() ; c != m_ISegChainList.end(); ++c )
    {
        vector< ISegSplit* >::iterator s;
        for ( s = ( *c )->m_SplitVec.begin(); s != ( *c )->m_SplitVec.end(); ++s )
        {
            Surf* srf = ( *s )->m_Surf;
            vec2d uw = ( *s )->m_UW;
            // Initialize source with strength from underlying surface map.
            int reason = -1;
            double str = srf->InterpTargetMap( uw.x(), uw.y(), reason );

            vec3d pt = ( *s )->m_Pnt;

            MapSource *ss = new MapSource;
            ss->m_pt = pt;
            ss->m_str = str;
            ss->m_reason = reason;

            splitSources.push_back( ss );
        }
    }

    // Number of times to propagate intersection edges through surfaces
    int nedgeprop = 4;

    for ( i = 0; i < nedgeprop; i ++ )
    {
        for ( c = m_ISegChainList.begin() ; c != m_ISegChainList.end(); ++c )
        {
            ( *c )->CalcDensity( GetGridDensityPtr(), splitSources );
            ( *c )->SpreadDensity();
        }
    }

    if( GetGridDensityPtr()->m_RigorLimit )
    {
        if ( output_type != CfdMeshMgrSingleton::QUIET_OUTPUT )
        {
            addOutputText( " Rigorous 3D Limiting\n", output_type );
        }

        for ( i = 0 ; i < ( int )m_SurfVec.size() ; i++ )
        {
            ms_cloud.sources.clear();
            ms_cloud.sources.reserve( allsources.size() );

            double minmap = numeric_limits<double>::max( );

            for( int j = 0; j < ( int )allsources.size(); j++ )
            {
                if( allsources[j]->m_surfid != i )
                {
                    if( allsources[j]->m_str < minmap )
                    {
                        minmap = allsources[j]->m_str;
                    }
                    ms_cloud.sources.push_back( allsources[j] );
                }
            }

            MSTree ms_tree( 3, ms_cloud, KDTreeSingleIndexAdaptorParams( 10 ) );
            ms_tree.buildIndex();

            m_SurfVec[i]->LimitTargetMap( ms_cloud, ms_tree, minmap );
        }

        for ( c = m_ISegChainList.begin() ; c != m_ISegChainList.end(); ++c )
        {
            ( *c )->CalcDensity( GetGridDensityPtr(), splitSources );
        }
    }

    // Clean up split sources.
    list< MapSource* >::iterator ss;
    for ( ss = splitSources.begin(); ss != splitSources.end(); ++ss )
    {
        delete ( *ss );
    }

    splitSources.clear();
}

void CfdMeshMgrSingleton::Remesh( int output_type )
{
    char str[256];
    int total_num_tris = 0;
    int nsurf = ( int )m_SurfVec.size();
    for ( int i = 0 ; i < nsurf ; ++i )
    {
        int num_tris = 0;

        int num_rev_removed = 0;

        for ( int iter = 0 ; iter < 10 ; ++iter )
        {
            num_tris = 0;
            m_SurfVec[i]->GetMesh()->Remesh();

            num_rev_removed = m_SurfVec[ i ]->GetMesh()->RemoveRevFaces();


            num_tris += m_SurfVec[ i ]->GetMesh()->GetNumFaces();

            snprintf( str, sizeof( str ), "Surf %3d/%3d Iter %2d/10 Num Tris = %8d %s\n", i + 1, nsurf, iter + 1, num_tris, m_SurfVec[i]->GetDisplayName().c_str() );

            if ( output_type != CfdMeshMgrSingleton::QUIET_OUTPUT )
            {
                addOutputText( str, output_type );
            }
        }
        total_num_tris += num_tris;

        if ( num_rev_removed > 0 )
        {
            snprintf( str, sizeof( str ), "%d Reversed tris collapsed in final iteration.\n", num_rev_removed );
            if ( output_type != CfdMeshMgrSingleton::QUIET_OUTPUT )
            {
                addOutputText( str, output_type );
            }
        }
        m_SurfVec[i]->GetMesh()->DumpGarbage();
    }

    WakeMgr.StretchWakes();

    snprintf( str, sizeof( str ), "Total Num Tris = %d\n", total_num_tris );
    addOutputText( str, output_type );
}

void CfdMeshMgrSingleton::PostMesh()
{
    int nsurf = ( int )m_SurfVec.size();
    for ( int i = 0 ; i < nsurf ; ++i )
    {
        m_SurfVec[ i ]->GetMesh()->LoadSimpFaces();
        m_SurfVec[i]->GetMesh()->Clear();
        Subtag( m_SurfVec[i] );
        m_SurfVec[ i ]->GetMesh()->CondenseSimpFaces();
    }
}

void CfdMeshMgrSingleton::ConvertToQuads()
{
    int nsurf = ( int )m_SurfVec.size();
    for ( int i = 0 ; i < nsurf ; ++i )
    {
        m_SurfVec[i]->GetMesh()->ConvertToQuads();
    }
}

string CfdMeshMgrSingleton::GetQualString()
{
    //list< Tri* >::iterator t;
    //int total_num_tris = 0;
    //int num_bad_tris[5];                  // Small Angles 0-5, 5-10, 10-15, 15-20, 20-25

    //for ( int i = 0 ; i < 5 ; i++ )
    //  num_bad_tris[i] = 0;

    //for ( int i = 0 ; i < (int)m_SurfVec.size() ; i++ )
    //{
    //  list< Tri* > tlist = m_SurfVec[i]->GetMesh()->GetTriList();

    //  for ( t = tlist.begin() ; t != tlist.end(); t++ )
    //  {
    //      total_num_tris++;
    //      double q = (*t)->ComputeQual()*180.0/M_PI;
    //      if ( q < 5.0 )          num_bad_tris[0]++;
    //      else if ( q < 10.0 )    num_bad_tris[1]++;
    //      else if ( q < 15.0 )    num_bad_tris[2]++;
    //      else if ( q < 20.0 )    num_bad_tris[3]++;
    //      else if ( q < 25.0 )    num_bad_tris[4]++;
    //  }
    //}
    //Stringc ret_string;
    //if ( total_num_tris > 0 )
    //{
    //  char str[512];
    //  sprintf( str, "0-5 Deg = %3.5f, 5-10=%3.5f   10-15=%3.5f   15-20=%3.5f   20-25=%3.5f\n",
    //      (float)num_bad_tris[0]/(float)total_num_tris,  (float)num_bad_tris[1]/(float)total_num_tris,
    //      (float)num_bad_tris[2]/(float)total_num_tris,  (float)num_bad_tris[3]/(float)total_num_tris,
    //      (float)num_bad_tris[4]/(float)total_num_tris );

    //  ret_string = str;
    //}
    //return ret_string;
    return "";
}

void CfdMeshMgrSingleton::ExportFiles()
{
    if ( GetSettingsPtr()->GetExportFileFlag( vsp::CFD_STL_FILE_NAME ) )
    {
        if ( !m_Vehicle->m_STLMultiSolid() )
        {
            WriteSTL( GetSettingsPtr()->GetExportFileName( vsp::CFD_STL_FILE_NAME ) );
        }
        else
        {
            WriteTaggedSTL( GetSettingsPtr()->GetExportFileName( vsp::CFD_STL_FILE_NAME ) );
        }
    }
    if ( GetSettingsPtr()->GetExportFileFlag( vsp::CFD_POLY_FILE_NAME ) )
    {
        WriteTetGen( GetSettingsPtr()->GetExportFileName( vsp::CFD_POLY_FILE_NAME ) );
    }

    string dat_fn;
    if (  GetSettingsPtr()->GetExportFileFlag( vsp::CFD_DAT_FILE_NAME ) )
    {
        dat_fn = GetSettingsPtr()->GetExportFileName( vsp::CFD_DAT_FILE_NAME );
    }
    string key_fn;
    if (  GetSettingsPtr()->GetExportFileFlag( vsp::CFD_KEY_FILE_NAME ) )
    {
        key_fn = GetSettingsPtr()->GetExportFileName( vsp::CFD_KEY_FILE_NAME );
    }
    string obj_fn;
    if (  GetSettingsPtr()->GetExportFileFlag( vsp::CFD_OBJ_FILE_NAME ) )
    {
        obj_fn = GetSettingsPtr()->GetExportFileName( vsp::CFD_OBJ_FILE_NAME );
    }
    string tri_fn;
    if (  GetSettingsPtr()->GetExportFileFlag( vsp::CFD_TRI_FILE_NAME ) )
    {
        tri_fn = GetSettingsPtr()->GetExportFileName( vsp::CFD_TRI_FILE_NAME );
    }
    string gmsh_fn;
    if (  GetSettingsPtr()->GetExportFileFlag( vsp::CFD_GMSH_FILE_NAME ) )
    {
        gmsh_fn = GetSettingsPtr()->GetExportFileName( vsp::CFD_GMSH_FILE_NAME );
    }
    string vspaero_fn;
    if (  GetSettingsPtr()->GetExportFileFlag( vsp::CFD_VSPGEOM_FILE_NAME ) )
    {
        vspaero_fn = GetSettingsPtr()->GetExportFileName( vsp::CFD_VSPGEOM_FILE_NAME );
    }

    WriteNASCART_Obj_Tri_Gmsh( dat_fn, key_fn, obj_fn, tri_fn, gmsh_fn, vspaero_fn );

    if ( GetSettingsPtr()->GetExportFileFlag( vsp::CFD_FACET_FILE_NAME ) )
    {
        WriteFacet( GetSettingsPtr()->GetExportFileName( vsp::CFD_FACET_FILE_NAME ) );
    }

    if ( GetSettingsPtr()->GetExportFileFlag( vsp::CFD_TKEY_FILE_NAME ) )
    {
        SubSurfaceMgr.WriteTKeyFile(GetSettingsPtr()->GetExportFileName(vsp::CFD_TKEY_FILE_NAME));
    }
}

void CfdMeshMgrSingleton::WriteTaggedSTL( const string &filename )
{
    //==== Find All Points and Tri Counts ====//
    vector< vec3d > allPntVec;
    vector < vector < int > > allPntKey;
    allPntKey.resize( m_SurfVec.size() );
    int k = 0;
    for ( int i = 0 ; i < ( int )m_SurfVec.size() ; i++ )
    {
        vector< vec3d >& sPntVec = m_SurfVec[i]->GetMesh()->GetSimpPntVec();
        allPntKey[i].resize( sPntVec.size() );
        for ( int v = 0 ; v < ( int )sPntVec.size() ; v++ )
        {
            allPntVec.push_back( sPntVec[v] );
            allPntKey[i][v] = k;
            k++;
        }
    }

    //==== Build Map ====//
    PntNodeCloud pnCloud;
    pnCloud.AddPntNodes( allPntVec );

    double tol = 1e-6;
    //==== Use NanoFlann to Find Close Points and Group ====//
    IndexPntNodes( pnCloud, tol );

    //==== Assemble Normal Tris ====//
    vector< SimpFace > allFaceVec;
    for ( int i = 0 ; i < ( int )m_SurfVec.size() ; i++ )
    {
        vector < SimpFace >& sFaceVec = m_SurfVec[ i ]->GetMesh()->GetSimpFaceVec();
        vector< vec3d >& sPntVec = m_SurfVec[i]->GetMesh()->GetSimpPntVec();
        for ( int f = 0 ; f <  ( int )sFaceVec.size() ; f++ )
        {
            int i0 = pnCloud.GetNodeUsedIndex( allPntKey[i][sFaceVec[f].ind0] );
            int i1 = pnCloud.GetNodeUsedIndex( allPntKey[i][sFaceVec[f].ind1] );
            int i2 = pnCloud.GetNodeUsedIndex( allPntKey[i][sFaceVec[f].ind2] );
            SimpFace sface;
            sface.ind0 = i0;
            sface.ind1 = i1;
            sface.ind2 = i2;

            if( sFaceVec[f].m_isQuad )
            {
                sface.m_isQuad = true;
                int i3 = pnCloud.GetNodeUsedIndex( allPntKey[i][sFaceVec[f].ind3] );
                sface.ind3 = i3;
            }

            sface.m_Tags = sFaceVec[f].m_Tags;
            allFaceVec.push_back( sface );
        }
    }
    //==== Assemble All Used Points ====//
    vector< vec3d > allUsedPntVec;
    for ( int i = 0 ; i < ( int )allPntVec.size() ; i++ )
    {
        if ( pnCloud.UsedNode( i ) )
        {
            allUsedPntVec.push_back( allPntVec[i] );
        }
    }

    FILE* file_id = fopen( filename.c_str(), "w" );
    if ( file_id )
    {
        std::vector< int > tags = SubSurfaceMgr.GetAllTags();
        for ( int itag = 0; itag < ( int ) tags.size(); itag++ )
        {
            std::string tagname = SubSurfaceMgr.GetTagNames( itag );
            fprintf( file_id, "solid %s\n", tagname.c_str() );

            for ( int f = 0; f < ( int ) allFaceVec.size(); f++ )
            {
                SimpFace* sface = &allFaceVec[f];
                int t = SubSurfaceMgr.GetTag( sface->m_Tags );

                if ( t == tags[itag] )
                {
                    const vec3d& p0 = allUsedPntVec[sface->ind0];
                    const vec3d& p1 = allUsedPntVec[sface->ind1];
                    const vec3d& p2 = allUsedPntVec[sface->ind2];
                    vec3d v01 = p1 - p0;
                    vec3d v12 = p2 - p1;
                    vec3d norm = cross( v01, v12 );
                    norm.normalize();

                    fprintf( file_id, " facet normal  %2.10le %2.10le %2.10le\n",  norm.x(), norm.y(), norm.z() );
                    fprintf( file_id, "   outer loop\n" );

                    fprintf( file_id, "     vertex %2.10le %2.10le %2.10le\n", p0.x(), p0.y(), p0.z() );
                    fprintf( file_id, "     vertex %2.10le %2.10le %2.10le\n", p1.x(), p1.y(), p1.z() );
                    fprintf( file_id, "     vertex %2.10le %2.10le %2.10le\n", p2.x(), p2.y(), p2.z() );

                    fprintf( file_id, "   endloop\n" );
                    fprintf( file_id, " endfacet\n" );

                    if ( sface->m_isQuad ) // Split quad and write additional tri.
                    {
                        vec3d p3 = allUsedPntVec[sface->ind3];
                        vec3d v23 = p3 - p2;
                        vec3d v30 = p0 - p3;
                        norm = cross( v23, v30 );
                        norm.normalize();

                        fprintf( file_id, " facet normal  %2.10le %2.10le %2.10le\n",  norm.x(), norm.y(), norm.z() );
                        fprintf( file_id, "   outer loop\n" );

                        fprintf( file_id, "     vertex %2.10le %2.10le %2.10le\n", p0.x(), p0.y(), p0.z() );
                        fprintf( file_id, "     vertex %2.10le %2.10le %2.10le\n", p2.x(), p2.y(), p2.z() );
                        fprintf( file_id, "     vertex %2.10le %2.10le %2.10le\n", p3.x(), p3.y(), p3.z() );

                        fprintf( file_id, "   endloop\n" );
                        fprintf( file_id, " endfacet\n" );
                    }
                }
            }
            fprintf( file_id, "endsolid %s\n", tagname.c_str() );
        }

        fclose( file_id );
    }
}

void CfdMeshMgrSingleton::WriteSTL( const string &filename )
{
    FILE* file_id = fopen( filename.c_str(), "w" );
    if ( file_id )
    {
        int numwake = 0;
        fprintf( file_id, "solid\n" );
        for ( int i = 0 ; i < ( int )m_SurfVec.size() ; i++ )
        {
            if ( !m_SurfVec[i]->GetWakeFlag() )
            {
                m_SurfVec[ i ]->GetMesh()->WriteSimpleSTL( file_id );
            }
            else
            {
                numwake++;
            }
        }
        fprintf( file_id, "endsolid\n" );

        if( numwake > 0 )
        {
            fprintf( file_id, "solid wake\n" );
            for ( int i = 0 ; i < ( int )m_SurfVec.size() ; i++ )
            {
                if ( m_SurfVec[i]->GetWakeFlag() )
                {
                    m_SurfVec[ i ]->GetMesh()->WriteSimpleSTL( file_id );
                }
            }
            fprintf( file_id, "endsolid wake\n" );
        }
        fclose( file_id );
    }
}

void CfdMeshMgrSingleton::WriteTetGen( const string &filename )
{
    FILE* fp = fopen( filename.c_str(), "w" );
    if ( !fp )
    {
        return;
    }

    int face_cnt = 0;
    vector< vec3d > allPntVec;
    vector < vector < int > > allPntKey;
    allPntKey.resize( m_SurfVec.size() );
    int k = 0;
    for ( int i = 0 ; i < ( int )m_SurfVec.size() ; i++ )
    {
        vector< vec3d >& sPntVec = m_SurfVec[i]->GetMesh()->GetSimpPntVec();
        allPntKey[i].resize( sPntVec.size() );
        for ( int v = 0 ; v < ( int )sPntVec.size() ; v++ )
        {
            allPntVec.push_back( sPntVec[v] );
            allPntKey[i][v] = k;
            k++;
        }
        face_cnt += m_SurfVec[ i ]->GetMesh()->GetSimpFaceVec().size();
    }

    //==== Build Map ====//
    PntNodeCloud pnCloud;
    pnCloud.AddPntNodes( allPntVec );

    double tol = 1e-6;

    //==== Use NanoFlann to Find Close Points and Group ====//
    IndexPntNodes( pnCloud, tol );

    //===== Write Num Pnts and Tris ====//
    fprintf( fp, "# Part 1 - node list\n" );
    fprintf( fp, "%lld 3 0 0\n", pnCloud.m_NumUsedPts );

    //==== Write Model Pnts ====//
    for ( int i = 0 ; i < ( int )allPntVec.size() ; i++ )
    {
        if ( pnCloud.UsedNode( i ) )
        {
            fprintf( fp, "%d %.16g %.16g %.16g\n", i + 1, allPntVec[i].x(), allPntVec[i].y(), allPntVec[i].z() );
        }
    }

    //==== Write Tris ====//
    fprintf( fp, "# Part 2 - facet list\n" );
    // <# of facets> <boundary markers (0 or 1)> 
    fprintf( fp, "%d 1\n", face_cnt );

    for ( int i = 0 ; i < ( int )m_SurfVec.size() ; i++ )
    {
        vector < SimpFace >& sFaceVec = m_SurfVec[ i ]->GetMesh()->GetSimpFaceVec();
        vector< vec3d >& sPntVec = m_SurfVec[i]->GetMesh()->GetSimpPntVec();
        for ( int f = 0 ; f < ( int )sFaceVec.size() ; f++ )
        {
            int i0 = pnCloud.GetNodeUsedIndex( allPntKey[i][sFaceVec[f].ind0] );
            int i1 = pnCloud.GetNodeUsedIndex( allPntKey[i][sFaceVec[f].ind1] );
            int i2 = pnCloud.GetNodeUsedIndex( allPntKey[i][sFaceVec[f].ind2] );
            int ind1 = i0 + 1;
            int ind2 = i1 + 1;
            int ind3 = i2 + 1;
            int tag = SubSurfaceMgr.GetTag( sFaceVec[f].m_Tags );

            // <# of polygons> [# of holes] [boundary marker]
            fprintf( fp, "1 0 %d\n", tag );
            if( sFaceVec[f].m_isQuad )
            {
                int i3 = pnCloud.GetNodeUsedIndex( allPntKey[i][sFaceVec[f].ind3] );
                int ind4 = i3 + 1;

                // <# of corners> <corner 1> <corner 2> <corner 3> <corner 4>
                fprintf( fp, "4 %d %d %d %d\n", ind1, ind2, ind3, ind4 );
            }
            else
            {
                // <# of corners> <corner 1> <corner 2> <corner 3>
                fprintf( fp, "3 %d %d %d\n", ind1, ind2, ind3 );
            }
        }
    }

    fprintf( fp, "# Part 3 - Hole List\n" );

    int set = GetCfdSettingsPtr()->m_SelectedSetIndex;

    if ( GetCfdSettingsPtr()->m_UseMode )
    {
        Mode *m = ModeMgr.GetMode( GetCfdSettingsPtr()->m_ModeID );
        if ( m )
        {
            // Don't apply settings here, mesh should already be created.
            // m->ApplySettings();
            set = m->m_NormalSet();
        }
    }

    vector<string> geomVec = m_Vehicle->GetGeomVec();
    vector< vec3d > interiorPntVec;
    for ( int i = 0 ; i < ( int )geomVec.size() ; i++ )
    {
        Geom* geom = m_Vehicle->FindGeom( geomVec[i] );
        if ( geom )
        {
            if ( geom->GetSetFlag( set ) )
            {
                if ( GetCfdSettingsPtr()->m_FarMeshFlag && GetCfdSettingsPtr()->m_FarCompFlag )
                {
                    if ( geom->GetID() != GetCfdSettingsPtr()->m_FarGeomID )
                    {
                        geom->GetInteriorPnts( interiorPntVec );
                    }
                }
                else
                {
                    geom->GetInteriorPnts( interiorPntVec );
                }
            }
        }
    }

    if ( GetCfdSettingsPtr()->m_HalfMeshFlag )
    {
        vector< vec3d > tmpPntVec;
        for ( int i = 0 ; i < ( int )interiorPntVec.size() ; i++ )
        {
            if ( std::abs( interiorPntVec[i].y() ) < 1.0e-4 )
            {
                interiorPntVec[i].set_y( 1.0e-5 );
            }

            if ( interiorPntVec[i].y() > 0.0 )
            {
                tmpPntVec.push_back( interiorPntVec[i] );
            }
        }
        interiorPntVec = tmpPntVec;
    }

    fprintf( fp, "%d\n", ( int )interiorPntVec.size() );
    for ( int i = 0 ; i < ( int )interiorPntVec.size() ; i++ )
    {
        const vec3d& p = interiorPntVec[i];
        fprintf( fp, "%d  %.16g %.16g %.16g\n", i + 1, p.x(), p.y(), p.z()  );
    }

    fprintf( fp, "# Part 4 - region list\n" );
    fprintf( fp, "0\n" );

    fclose( fp );
}

void CfdMeshMgrSingleton::WriteNASCART_Obj_Tri_Gmsh( const string &dat_fn, const string &key_fn, const string &obj_fn, const string &tri_fn, const string &gmsh_fn, const string & vspgeom_fn )
{
#ifdef DEBUG_CFD_MESH
    //==== Find Smallest Edge ====//
    double small_edge = 1.0e12;
    for ( int i = 0 ; i < ( int )m_SurfVec.size() ; i++ )
    {
        vector < SimpFace >& sFaceVec = m_SurfVec[i]->GetMesh()->GetSimpFaceVec();
        vector< vec3d >& sPntVec = m_SurfVec[i]->GetMesh()->GetSimpPntVec();
        for ( int f = 0 ; f < ( int )sFaceVec.size() ; f++ )
        {
            double el0 = dist_squared( sPntVec[sFaceVec[f].ind0], sPntVec[sFaceVec[f].ind1] );
            double el1 = dist_squared( sPntVec[sFaceVec[f].ind1], sPntVec[sFaceVec[f].ind2] );
            double el2 = dist_squared( sPntVec[sFaceVec[f].ind2], sPntVec[sFaceVec[f].ind0] );
            if ( el0 < small_edge )
            {
                small_edge = el0;
            }
            if ( el1 < small_edge )
            {
                small_edge = el1;
            }
            if ( el2 < small_edge )
            {
                small_edge = el2;
            }
        }
    }

    fprintf( m_DebugFile, "CfdMeshMgr::WriteNASCART Small Edge Length = %f \n", sqrt( small_edge ) );
#endif

//  //==== Any Files To Write? ====//
//  if ( !dat_fn && !key_fn && !obj_fn && !tri_fn )
//      return;

    // Used when comparing W parameter to TMAGIC
    double tol = 1e-12;

    //==== Find All Points and Tri Counts ====//
    vector< vec3d > allPntVec;
    vector< vec3d > wakeAllPntVec;
    vector < vector < int > > allPntKey;
    allPntKey.resize( m_SurfVec.size() );
    vector < vector < int > > allWPntKey;
    allWPntKey.resize( m_SurfVec.size() );
    int k = 0;
    int wk = 0;
    for ( int i = 0 ; i < ( int )m_SurfVec.size() ; i++ )
    {
        vector< vec3d >& sPntVec = m_SurfVec[i]->GetMesh()->GetSimpPntVec();
        allPntKey[i].resize( sPntVec.size() );
        allWPntKey[i].resize( sPntVec.size() );
        for ( int v = 0 ; v < ( int )sPntVec.size() ; v++ )
        {
            if ( m_SurfVec[i]->GetWakeFlag() )
            {
                wakeAllPntVec.push_back( sPntVec[v] );
                allWPntKey[i][v] = wk;
                wk++;
            }
            else
            {
                allPntVec.push_back( sPntVec[v] );
                allPntKey[i][v] = k;
                k++;
            }
        }
    }

    //==== Build Map ====//
    PntNodeCloud pnCloud;
    pnCloud.AddPntNodes( allPntVec );

    double tol2 = 1e-6;
    //==== Use NanoFlann to Find Close Points and Group ====//
    IndexPntNodes( pnCloud, tol2 );

    //==== Build Wake Map If Available ====//
    PntNodeCloud wakepnCloud;
    if ( wakeAllPntVec.size() )
    {
        wakepnCloud.AddPntNodes( wakeAllPntVec );
        IndexPntNodes( wakepnCloud, tol2 );
    }

    //==== Assemble Normal Tris ====//
    vector< SimpFace > allFaceVec;
    int ntristrict = 0;
    vector< int > allSurfIDVec;
    vector< vector< vec2d > > allUWVec;
    vector < pair < int, int > > wedges;
    for ( int i = 0 ; i < ( int )m_SurfVec.size() ; i++ )
    {
        if ( !m_SurfVec[i]->GetWakeFlag() )
        {
            vector < SimpFace >& sFaceVec = m_SurfVec[ i ]->GetMesh()->GetSimpFaceVec();
            vector< vec3d >& sPntVec = m_SurfVec[i]->GetMesh()->GetSimpPntVec();
            vector< vec2d >& sUWVec = m_SurfVec[i]->GetMesh()->GetSimpUWPntVec();
            for ( int t = 0 ; t <  ( int )sFaceVec.size() ; t++ )
            {
                SimpFace sface;
                sface.ind0 = pnCloud.GetNodeUsedIndex( allPntKey[i][sFaceVec[t].ind0] ) + 1;
                sface.ind1 = pnCloud.GetNodeUsedIndex( allPntKey[i][sFaceVec[t].ind1] ) + 1;
                sface.ind2 = pnCloud.GetNodeUsedIndex( allPntKey[i][sFaceVec[t].ind2] ) + 1;

                if( sFaceVec[t].m_isQuad )
                {
                    sface.m_isQuad = true;
                    sface.ind3 = pnCloud.GetNodeUsedIndex( allPntKey[i][sFaceVec[t].ind3] ) + 1;
                    ntristrict++; // Bonus tri for split quad.
                }

                sface.m_Tags = sFaceVec[t].m_Tags;
                ntristrict++;
                allFaceVec.push_back( sface );
                allSurfIDVec.push_back( m_SurfVec[i]->GetSurfID() );

                vector< vec2d > uwFace( 4 );
                uwFace[0] = sUWVec[ sFaceVec[t].ind0 ];
                uwFace[1] = sUWVec[ sFaceVec[t].ind1 ];
                uwFace[2] = sUWVec[ sFaceVec[t].ind2 ];
                if( sFaceVec[t].m_isQuad )
                {
                    uwFace[3] = sUWVec[ sFaceVec[t].ind3 ];
                }
                allUWVec.push_back( uwFace );

                if ( m_SurfVec[i]->GetSurfaceVSPType() == vsp::WING_SURF )
                {
                    bool n0 = uwFace[0].y() <= ( TMAGIC + tol );
                    bool n1 = uwFace[1].y() <= ( TMAGIC + tol );
                    bool n2 = uwFace[2].y() <= ( TMAGIC + tol );
                    bool n3 = false;
                    if( sFaceVec[t].m_isQuad )
                    {
                        n3 = uwFace[3].y() <= ( TMAGIC + tol );
                    }

                    if ( ( n0 + n1 + n2 + n3 ) == 2 ) // Two true, one or two false.
                    {
                        // Perform index lookup as above.
                        int i0 = pnCloud.GetNodeUsedIndex( allPntKey[i][sFaceVec[t].ind0] ) + 1;
                        int i1 = pnCloud.GetNodeUsedIndex( allPntKey[i][sFaceVec[t].ind1] ) + 1;
                        int i2 = pnCloud.GetNodeUsedIndex( allPntKey[i][sFaceVec[t].ind2] ) + 1;


                        int i3 = -1;
                        if( sFaceVec[t].m_isQuad )
                        {
                            i3 = pnCloud.GetNodeUsedIndex( allPntKey[i][sFaceVec[t].ind3] ) + 1;
                        }

                        // Add nodes to wake edges, lowest u first.
                        if ( n0 && n1 )
                        {
                            if ( uwFace[0].x() < uwFace[1].x() )
                            {
                                wedges.push_back( pair< int, int>( i0, i1 ) );
                            }
                            else
                            {
                                wedges.push_back( pair< int, int>( i1, i0 ) );
                            }
                        }
                        else if ( n1 && n2 )
                        {
                            if ( uwFace[1].x() < uwFace[2].x() )
                            {
                                wedges.push_back( pair< int, int>( i1, i2 ) );
                            }
                            else
                            {
                                wedges.push_back( pair< int, int>( i2, i1 ) );
                            }
                        }
                        else if ( !sFaceVec[t].m_isQuad && n2 && n0 ) // Triangle only
                        {
                            if ( uwFace[2].x() < uwFace[0].x() )
                            {
                                wedges.push_back( pair< int, int>( i2, i0 ) );
                            }
                            else
                            {
                                wedges.push_back( pair< int, int>( i0, i2 ) );
                            }
                        }
                        else if ( sFaceVec[t].m_isQuad && n2 && n3 ) // Quad only
                        {
                            if ( uwFace[2].x() < uwFace[3].x() )
                            {
                                wedges.push_back( pair< int, int>( i2, i3 ) );
                            }
                            else
                            {
                                wedges.push_back( pair< int, int>( i3, i2 ) );
                            }
                        }
                        else if ( sFaceVec[t].m_isQuad && n3 && n0 ) // Quad only
                        {
                            if ( uwFace[3].x() < uwFace[0].x() )
                            {
                                wedges.push_back( pair< int, int>( i3, i0 ) );
                            }
                            else
                            {
                                wedges.push_back( pair< int, int>( i0, i3 ) );
                            }
                        }
                    }
                }
            }

        }
    }

    sort( wedges.begin(), wedges.end() );

    vector < pair < int, int > >::iterator it;
    it = unique( wedges.begin(), wedges.end() );
    wedges.resize( distance( wedges.begin(), it ) );

    list < pair < int, int > > wlist( wedges.begin(), wedges.end() );

    vector < deque < pair < int, int > > > wakes;

    while ( !wlist.empty() )
    {
        list < pair < int, int > >::iterator wit = wlist.begin();

        int iwake = wakes.size();
        wakes.resize( iwake + 1 );
        wakes[iwake].push_back( *wit );
        wit = wlist.erase( wit );

        while ( wit != wlist.end() )
        {
            if ( wakes[iwake].back().second == (*wit).first )
            {
                wakes[iwake].push_back( *wit );
                wlist.erase( wit );
                wit = wlist.begin();
                continue;
            }
            else if ( wakes[iwake].begin()->first == (*wit).second )
            {
                wakes[iwake].push_front( *wit );
                wlist.erase( wit );
                wit = wlist.begin();
                continue;
            }
            wit++;
        }
    }


    //==== Assemble All Used Points ====//
    vector< vec3d > allUsedPntVec;
    for ( int i = 0 ; i < ( int )allPntVec.size() ; i++ )
    {
        if ( pnCloud.UsedNode( i ) )
        {
            allUsedPntVec.push_back( allPntVec[i] );
        }
    }

    //==== Assemble Wake Tris ====//
    int wakeIndOffset = allUsedPntVec.size();
    for ( int i = 0 ; i < ( int )m_SurfVec.size() ; i++ )
    {
        if ( m_SurfVec[i]->GetWakeFlag() )
        {
            vector < SimpFace >& sFaceVec = m_SurfVec[ i ]->GetMesh()->GetSimpFaceVec();
            vector< vec3d >& sPntVec = m_SurfVec[i]->GetMesh()->GetSimpPntVec();
            vector< vec2d >& sUWVec = m_SurfVec[i]->GetMesh()->GetSimpUWPntVec();
            for ( int f = 0 ; f < ( int )sFaceVec.size() ; f++ )
            {
                int i0 = wakepnCloud.GetNodeUsedIndex( allWPntKey[i][sFaceVec[f].ind0] );
                int i1 = wakepnCloud.GetNodeUsedIndex( allWPntKey[i][sFaceVec[f].ind1] );
                int i2 = wakepnCloud.GetNodeUsedIndex( allWPntKey[i][sFaceVec[f].ind2] );

                SimpFace sface;
                sface.ind0 = i0 + 1 + wakeIndOffset;
                sface.ind1 = i1 + 1 + wakeIndOffset;
                sface.ind2 = i2 + 1 + wakeIndOffset;

                if( sFaceVec[f].m_isQuad )
                {
                    sface.m_isQuad = true;
                    int i3 = wakepnCloud.GetNodeUsedIndex( allWPntKey[i][sFaceVec[f].ind3] );
                    sface.ind3 = i3 + 1 + wakeIndOffset;
                    ntristrict++; // Bonus tri for split quad
                }

                sface.m_Tags = sFaceVec[f].m_Tags;
                ntristrict++;
                allFaceVec.push_back( sface );
                allSurfIDVec.push_back( m_SurfVec[i]->GetSurfID() );

                vector< vec2d > uwFace( 4 );
                uwFace[0] = sUWVec[ sFaceVec[f].ind0 ];
                uwFace[1] = sUWVec[ sFaceVec[f].ind1 ];
                uwFace[2] = sUWVec[ sFaceVec[f].ind2 ];
                if( sFaceVec[f].m_isQuad )
                {
                    uwFace[3] = sUWVec[ sFaceVec[f].ind3 ];
                }
                allUWVec.push_back( uwFace );

            }
        }
    }

    //==== Assemble All Used Points ====//
    for ( int i = 0 ; i < ( int )wakeAllPntVec.size() ; i++ )
    {
        if ( wakepnCloud.UsedNode( i ) )
        {
            allUsedPntVec.push_back( wakeAllPntVec[i] );
        }
    }

    //=====================================================================================//
    //==== Write NASCART File =================================================================//
    //=====================================================================================//
    if ( dat_fn.length() != 0 )
    {
        FILE* fp = fopen( dat_fn.c_str(), "w" );

        if ( fp )
        {
            //===== Write Num Pnts and Tris ====//
            fprintf( fp, "%d %d\n", ( int )allUsedPntVec.size(), ntristrict );

            //==== Write Pnts ====//
            for ( int i = 0 ; i < ( int )allUsedPntVec.size() ; i++ )
            {
                fprintf( fp, "%.16g %.16g %.16g\n", allUsedPntVec[i].x(), allUsedPntVec[i].z(), -allUsedPntVec[i].y() );
            }

            //==== Write Tris ====//
            for ( int i = 0 ; i < ( int )allFaceVec.size() ; i++ )
            {
                fprintf( fp, "%d %d %d %d.0\n",
                         allFaceVec[i].ind0, allFaceVec[i].ind2, allFaceVec[i].ind1,
                         SubSurfaceMgr.GetTag( allFaceVec[i].m_Tags ) );

                if( allFaceVec[i].m_isQuad )
                {
                    fprintf( fp, "%d %d %d %d.0\n",
                             allFaceVec[i].ind0, allFaceVec[i].ind3, allFaceVec[i].ind2,
                             SubSurfaceMgr.GetTag( allFaceVec[i].m_Tags ) );
                }
            }
            fclose( fp );
        }
    }

    if ( key_fn.length() != 0 )
    {
        SubSurfaceMgr.WriteNascartKeyFile( key_fn );
    }

    //=====================================================================================//
    //==== Write OBJ File =================================================================//
    //=====================================================================================//
    if ( obj_fn.length() != 0 )
    {
        FILE* fp = fopen( obj_fn.c_str(), "w" );

        if ( fp )
        {
            //==== Write Pnts ====//
            for ( int i = 0 ; i < ( int )allUsedPntVec.size() ; i++ )
            {
                fprintf( fp, "v %16.10f %16.10f %16.10f\n", allUsedPntVec[i].x(), allUsedPntVec[i].z(), -allUsedPntVec[i].y() );
            }
            fprintf( fp, "\n" );

            //==== Write Tris ====//
            for ( int i = 0 ; i < ( int )allFaceVec.size() ; i++ )
            {
                if( allFaceVec[i].m_isQuad )
                {
                    fprintf( fp, "f %d %d %d %d \n", allFaceVec[i].ind0, allFaceVec[i].ind1, allFaceVec[i].ind2, allFaceVec[i].ind3 );
                }
                else
                {
                    fprintf( fp, "f %d %d %d \n", allFaceVec[i].ind0, allFaceVec[i].ind1, allFaceVec[i].ind2 );
                }
            }
            fclose( fp );
        }
    }


    //=====================================================================================//
    //==== Write TRI File for Cart3D ======================================================//
    //=====================================================================================//
    if ( tri_fn.length() != 0 )
    {
        FILE* fp = fopen( tri_fn.c_str(), "w" );

        if ( fp )
        {
            //==== Write Pnt Count and Tri Count ====//
            fprintf( fp, "%d %d\n", ( int )allUsedPntVec.size(), ntristrict );

            //==== Write Pnts ====//
            for ( int i = 0 ; i < ( int )allUsedPntVec.size() ; i++ )
            {
                fprintf( fp, "%16.10g %16.10g %16.10g\n", allUsedPntVec[i].x(), allUsedPntVec[i].y(), allUsedPntVec[i].z() );
            }

            //==== Write Tris ====//
            for ( int i = 0 ; i < ( int )allFaceVec.size() ; i++ )
            {
                fprintf( fp, "%d %d %d \n", allFaceVec[i].ind0, allFaceVec[i].ind1, allFaceVec[i].ind2 );
                if( allFaceVec[i].m_isQuad )
                {
                    fprintf( fp, "%d %d %d \n", allFaceVec[i].ind0, allFaceVec[i].ind2, allFaceVec[i].ind3 );
                }
            }

            //==== Write Component ID ====//
            for ( int i = 0 ; i < ( int )allFaceVec.size() ; i++ )
            {
                fprintf( fp, "%d \n", SubSurfaceMgr.GetTag( allFaceVec[i].m_Tags ) );
                if( allFaceVec[i].m_isQuad )
                {
                    fprintf( fp, "%d \n", SubSurfaceMgr.GetTag( allFaceVec[i].m_Tags ) );
                }
            }

            fclose( fp );
        }
    }

    //=====================================================================================//
    //==== Write gmsh File           ======================================================//
    //=====================================================================================//
    if ( gmsh_fn.length() != 0 )
    {
        FILE* fp = fopen( gmsh_fn.c_str(), "w" );
        if ( fp )
        {
            fprintf( fp, "$MeshFormat\n" );
            fprintf( fp, "2.2 0 %d\n", ( int )sizeof( double ) );
            fprintf( fp, "$EndMeshFormat\n" );

            //==== Write Nodes ====//
            fprintf( fp, "$Nodes\n" );
            fprintf( fp, "%d\n", ( int )allUsedPntVec.size() );
            for ( int i = 0 ; i < ( int )allUsedPntVec.size() ; i++ )
            {
                fprintf( fp, "%d %16.10f %16.10f %16.10f\n", i + 1,
                         allUsedPntVec[i].x(), allUsedPntVec[i].y(), allUsedPntVec[i].z() );
            }
            fprintf( fp, "$EndNodes\n" );

            //==== Write Tris ====//
            fprintf( fp, "$Elements\n" );
            fprintf( fp, "%d\n", ( int )allFaceVec.size() );

            int ele_cnt = 1;
            for ( int i = 0 ; i < ( int )allFaceVec.size() ; i++ )
            {
                if( allFaceVec[i].m_isQuad )
                {
                    fprintf( fp, "%d 3 0 %d %d %d %d \n", ele_cnt, allFaceVec[i].ind0, allFaceVec[i].ind1, allFaceVec[i].ind2, allFaceVec[i].ind3 );
                }
                else
                {
                    fprintf( fp, "%d 2 0 %d %d %d \n", ele_cnt, allFaceVec[i].ind0, allFaceVec[i].ind1, allFaceVec[i].ind2 );
                }
                ele_cnt++;
            }

            fprintf( fp, "$EndElements\n" );
            fclose( fp );
        }
    }

    //=====================================================================================//
    //==== Write VSPGEOM File for VSPAERO =================================================//
    //=====================================================================================//
    if ( vspgeom_fn.length() != 0 )
    {
        bool allowquads = true;
        FILE* fp = fopen( vspgeom_fn.c_str(), "w" );

        if ( fp )
        {
            fprintf( fp, "# vspgeom v2\n" );
            //==== Write Pnt Count ====//
            fprintf( fp, "%d\n", ( int )allUsedPntVec.size() );

            //==== Write Pnts ====//
            for ( int i = 0 ; i < ( int )allUsedPntVec.size() ; i++ )
            {
                fprintf( fp, "%16.10g %16.10g %16.10g\n", allUsedPntVec[i].x(), allUsedPntVec[i].y(), allUsedPntVec[i].z() );
            }

            if ( allowquads )
            {
                //==== Write Face Count ====//
                fprintf( fp, "%d\n", ( int )allFaceVec.size() );

                //==== Write Faces ====//
                for ( int i = 0 ; i < ( int )allFaceVec.size() ; i++ )
                {
                    if( allFaceVec[i].m_isQuad )
                    {
                        fprintf( fp, "4 %d %d %d %d \n", allFaceVec[i].ind0, allFaceVec[i].ind1, allFaceVec[i].ind2, allFaceVec[i].ind3 );
                    }
                    else
                    {
                        fprintf( fp, "3 %d %d %d \n", allFaceVec[i].ind0, allFaceVec[i].ind1, allFaceVec[i].ind2 );
                    }
                }
            }
            else
            {
                //==== Write Face Count ====//
                fprintf( fp, "%d\n", ntristrict );

                //==== Write Tris Only ====//
                for ( int i = 0 ; i < ( int )allFaceVec.size() ; i++ )
                {
                    fprintf( fp, "3 %d %d %d \n", allFaceVec[i].ind0, allFaceVec[i].ind1, allFaceVec[i].ind2 );
                    if( allFaceVec[i].m_isQuad )
                    {
                        fprintf( fp, "3 %d %d %d \n", allFaceVec[i].ind0, allFaceVec[i].ind2, allFaceVec[i].ind3 );
                    }
                }
            }

            if ( allowquads )
            {
                //==== Write Component ID ====//
                for ( int i = 0 ; i < ( int )allFaceVec.size() ; i++ )
                {
                    int tag = SubSurfaceMgr.GetTag( allFaceVec[i].m_Tags );
                    int part = SubSurfaceMgr.GetPart( allFaceVec[i].m_Tags );

                    if( allFaceVec[i].m_isQuad )
                    {
                        fprintf( fp, "%d %d %16.10g %16.10g %16.10g %16.10g %16.10g %16.10g %16.10g %16.10g\n", part, tag,
                             allUWVec[i][0].x(), allUWVec[i][0].y(),
                             allUWVec[i][1].x(), allUWVec[i][1].y(),
                             allUWVec[i][2].x(), allUWVec[i][2].y(),
                             allUWVec[i][3].x(), allUWVec[i][3].y() );
                    }
                    else
                    {
                        fprintf( fp, "%d %d %16.10g %16.10g %16.10g %16.10g %16.10g %16.10g\n", part, tag,
                                 allUWVec[i][0].x(), allUWVec[i][0].y(),
                                 allUWVec[i][1].x(), allUWVec[i][1].y(),
                                 allUWVec[i][2].x(), allUWVec[i][2].y() );
                    }
                }
            }
            else
            {
                //==== Write Component ID ====//
                for ( int i = 0 ; i < ( int )allFaceVec.size() ; i++ )
                {
                    int tag = SubSurfaceMgr.GetTag( allFaceVec[i].m_Tags );
                    int part = SubSurfaceMgr.GetPart( allFaceVec[i].m_Tags );

                    fprintf( fp, "%d %d %16.10g %16.10g %16.10g %16.10g %16.10g %16.10g\n", part, tag,
                             allUWVec[i][0].x(), allUWVec[i][0].y(),
                             allUWVec[i][1].x(), allUWVec[i][1].y(),
                             allUWVec[i][2].x(), allUWVec[i][2].y() );
                    if( allFaceVec[i].m_isQuad )
                    {
                        fprintf( fp, "%d %d %16.10g %16.10g %16.10g %16.10g %16.10g %16.10g\n", part, tag,
                                 allUWVec[i][0].x(), allUWVec[i][0].y(),
                                 allUWVec[i][2].x(), allUWVec[i][2].y(),
                                 allUWVec[i][3].x(), allUWVec[i][3].y() );
                    }
                }
            }

            int nwake = wakes.size();
            // Wake line data.
            fprintf( fp, "%d\n", nwake );

            for ( int iwake = 0; iwake < nwake; iwake++ )
            {

                int iprt = 0;
                int iwe;
                int nwe = wakes[iwake].size();
                fprintf( fp, "%d ", nwe + 1 );
                for ( iwe = 0; iwe < nwe; iwe++ )
                {
                    fprintf( fp, "%d", wakes[iwake][iwe].first );

                    if ( iprt < 9 )
                    {
                        fprintf( fp, " " );
                        iprt++;
                    }
                    else
                    {
                        fprintf( fp, "\n" );
                        iprt = 0;
                    }
                }
                fprintf( fp, "%d\n", wakes[iwake][iwe-1].second );

            }

            // Write alternate triangulation data.
            if ( allowquads )
            {
                //==== Write Faces ====//
                for ( int i = 0 ; i < ( int )allFaceVec.size() ; i++ )
                {
                    if( allFaceVec[i].m_isQuad )
                    {
                        fprintf( fp, "%d 2 %d %d %d %d %d %d\n", i + 1, allFaceVec[i].ind0, allFaceVec[i].ind1, allFaceVec[i].ind2, allFaceVec[i].ind0, allFaceVec[i].ind2, allFaceVec[i].ind3 );
                    }
                    else
                    {
                        fprintf( fp, "%d 1 %d %d %d\n", i + 1, allFaceVec[i].ind0, allFaceVec[i].ind1, allFaceVec[i].ind2 );
                    }
                }
            }
            else
            {
                int iface = 1;

                //==== Write Tris Only ====//
                for ( int i = 0 ; i < ( int )allFaceVec.size() ; i++ )
                {
                    fprintf( fp, "%d 1 %d %d %d \n", iface, allFaceVec[i].ind0, allFaceVec[i].ind1, allFaceVec[i].ind2 );
                    iface++;
                    if( allFaceVec[i].m_isQuad )
                    {
                        fprintf( fp, "%d 1 %d %d %d \n", iface, allFaceVec[i].ind0, allFaceVec[i].ind2, allFaceVec[i].ind3 );
                        iface++;
                    }
                }
            }

            if ( allowquads )
            {
                //==== Write Component ID ====//
                for ( int i = 0 ; i < ( int )allFaceVec.size() ; i++ )
                {
                    int tag = SubSurfaceMgr.GetTag( allFaceVec[i].m_Tags );
                    int part = SubSurfaceMgr.GetPart( allFaceVec[i].m_Tags );

                    if( allFaceVec[i].m_isQuad )
                    {
                        fprintf( fp, "%d %d %d %16.10g %16.10g %16.10g %16.10g %16.10g %16.10g %16.10g %16.10g %16.10g %16.10g %16.10g %16.10g\n", i + 1, part, tag,
                             allUWVec[i][0].x(), allUWVec[i][0].y(),
                             allUWVec[i][1].x(), allUWVec[i][1].y(),
                             allUWVec[i][2].x(), allUWVec[i][2].y(),
                             allUWVec[i][0].x(), allUWVec[i][0].y(),
                             allUWVec[i][2].x(), allUWVec[i][2].y(),
                             allUWVec[i][3].x(), allUWVec[i][3].y() );
                    }
                    else
                    {
                        fprintf( fp, "%d %d %d %16.10g %16.10g %16.10g %16.10g %16.10g %16.10g\n", i + 1, part, tag,
                                 allUWVec[i][0].x(), allUWVec[i][0].y(),
                                 allUWVec[i][1].x(), allUWVec[i][1].y(),
                                 allUWVec[i][2].x(), allUWVec[i][2].y() );
                    }
                }
            }
            else
            {
                int iface = 1;
                //==== Write Component ID ====//
                for ( int i = 0 ; i < ( int )allFaceVec.size() ; i++ )
                {
                    int tag = SubSurfaceMgr.GetTag( allFaceVec[i].m_Tags );
                    int part = SubSurfaceMgr.GetPart( allFaceVec[i].m_Tags );

                    fprintf( fp, "%d %d %d %16.10g %16.10g %16.10g %16.10g %16.10g %16.10g\n", iface, part, tag,
                             allUWVec[i][0].x(), allUWVec[i][0].y(),
                             allUWVec[i][1].x(), allUWVec[i][1].y(),
                             allUWVec[i][2].x(), allUWVec[i][2].y() );
                    iface++;
                    if( allFaceVec[i].m_isQuad )
                    {
                        fprintf( fp, "%d %d %d %16.10g %16.10g %16.10g %16.10g %16.10g %16.10g\n", iface, part, tag,
                                 allUWVec[i][0].x(), allUWVec[i][0].y(),
                                 allUWVec[i][2].x(), allUWVec[i][2].y(),
                                 allUWVec[i][3].x(), allUWVec[i][3].y() );
                        iface++;
                    }
                }
            }

            fclose( fp );
        }

        WriteTagFiles( vspgeom_fn, allFaceVec, allowquads );

        SubSurfaceMgr.WriteVSPGEOMKeyFile( vspgeom_fn );

        vector < string > gidvec;
        vector < int > partvec;
        vector < int > surfvec;
        SubSurfaceMgr.GetPartData( gidvec, partvec, surfvec );

        vector < string > all_files;
        m_Vehicle->WriteControlSurfaceFile( vspgeom_fn, gidvec, partvec, surfvec, all_files );
    }
}

void CfdMeshMgrSingleton::WriteTagFiles( string file_name, const vector< SimpFace > &allFaceVec, bool allowquads )
{
    int ntagfile = 0;
    int ncsffile = 0;

    std::vector < int > partvec;
    SubSurfaceMgr.MakePartList( partvec );

    for ( int ipart = 0; ipart < partvec.size(); ipart++ )
    {
        int part = partvec[ ipart ];

        for ( int iss = 0; iss < m_SimpleSubSurfaceVec.size(); iss++ )
        {
            SimpleSubSurface ssurf = m_SimpleSubSurfaceVec[iss];
            int tag = ssurf.m_Tag;

            if ( SubSurfaceMgr.ExistPartAndTag( part, tag ) )
            {
                ntagfile++;

                if ( ssurf.GetType() == vsp::SS_CONTROL )
                {
                    ncsffile++;
                }
            }
        }
    }

    if ( ntagfile > 0 )
    {
        string base_name = GetBasename( file_name );

        string base_path, base_fname;
        GetPathFile( base_name, base_path, base_fname );

        string taglist_name = base_name + ".ALL.taglist";
        string csf_taglist_name = base_name + ".ControlSurfaces.taglist";

        FILE* taglist_fid = fopen( taglist_name.c_str(), "w" );
        FILE* csf_taglist_fid = NULL;
        if ( ncsffile > 0 )
        {
            csf_taglist_fid = fopen( csf_taglist_name.c_str(), "w" );
        }

        if ( taglist_fid )
        {

            fprintf( taglist_fid, "%d\n", ntagfile );

            if ( csf_taglist_fid )
            {
                fprintf( csf_taglist_fid, "%d\n", ncsffile );
            }

            for ( int ipart = 0; ipart < partvec.size(); ipart++ )
            {
                int part = partvec[ ipart ];

                for ( int iss = 0; iss < m_SimpleSubSurfaceVec.size(); iss++ )
                {
                    SimpleSubSurface ssurf = m_SimpleSubSurfaceVec[iss];
                    int tag = ssurf.m_Tag;

                    if ( SubSurfaceMgr.ExistPartAndTag( part, tag ) )
                    {
                        vector < int > parttag;
                        parttag.push_back( part );
                        parttag.push_back( tag );

                        string str = SubSurfaceMgr.m_TagNames[ part ];
                        int pos = str.find_first_of( '_' );
                        string gname = str.substr( 0, pos );
                        string sname = str.substr( pos + 2 );

                        string ptagname = gname + sname + "_" + SubSurfaceMgr.m_TagNames[tag];

                        string tagfile_name = base_name + ptagname + ".tag";
                        string tagfile_localname = base_fname + ptagname;

                        fprintf( taglist_fid, "%s\n", tagfile_localname.c_str() );

                        if ( csf_taglist_fid )
                        {
                            if ( ssurf.GetType() == vsp::SS_CONTROL )
                            {
                                fprintf( csf_taglist_fid, "%s\n", tagfile_localname.c_str() );
                            }
                        }

                        FILE* fid = fopen( tagfile_name.c_str(), "w" );
                        if ( fid )
                        {
                            WriteTagFile( fid, part, tag, allFaceVec, allowquads );

                            fclose( fid );
                        }
                    }
                }
            }

            fclose( taglist_fid );

            if ( csf_taglist_fid )
            {
                fclose( csf_taglist_fid );
            }
        }
    }
}

void CfdMeshMgrSingleton::WriteTagFile( FILE* file_id, int part, int tag, const vector< SimpFace > &allFaceVec, bool allowquads )
{
    //==== Write Tri IDs for each tag =====//
    int count = 0;
    if ( allowquads )
    {
        for ( int i = 0 ; i < ( int )allFaceVec.size() ; i++ )
        {
            if ( SubSurfaceMgr.MatchPartAndTag( allFaceVec[i].m_Tags, part, tag ) )
            {
                count++;
            }
        }
    }
    else
    {
        for ( int i = 0 ; i < ( int )allFaceVec.size() ; i++ )
        {
            if ( SubSurfaceMgr.MatchPartAndTag( allFaceVec[i].m_Tags, part, tag ) )
            {
                count++;
            }

            if( allFaceVec[i].m_isQuad )
            {
                if ( SubSurfaceMgr.MatchPartAndTag( allFaceVec[i].m_Tags, part, tag ) )
                {
                    count++;
                }
            }
        }
    }
    fprintf( file_id, "%d\n\n", count );

    if ( allowquads )
    {
        for ( int i = 0 ; i < ( int )allFaceVec.size() ; i++ )
        {
            if ( SubSurfaceMgr.MatchPartAndTag( allFaceVec[i].m_Tags, part, tag ) )
            {
                fprintf( file_id, "%d\n", i + 1 );
            }
        }
    }
    else
    {
        int iface = 0;

        for ( int i = 0 ; i < ( int )allFaceVec.size() ; i++ )
        {
            if ( SubSurfaceMgr.MatchPartAndTag( allFaceVec[i].m_Tags, part, tag ) )
            {
                fprintf( file_id, "%d\n", iface + 1 );
            }
            iface++;

            if( allFaceVec[i].m_isQuad )
            {
                if ( SubSurfaceMgr.MatchPartAndTag( allFaceVec[i].m_Tags, part, tag ) )
                {
                    fprintf( file_id, "%d\n", iface + 1 );
                }
                iface++;
            }
        }
    }
}

void CfdMeshMgrSingleton::WriteFacet( const string &facet_fn )
{
    // Note: Wake mesh not included in Facet export

    //==== Find All Points and Tri Counts ====//
    vector< vec3d > allPntVec;
    vector < vector < int > > allPntKey;
    allPntKey.resize( m_SurfVec.size() );
    int k = 0;
    for ( int i = 0; i < (int)m_SurfVec.size(); i++ )
    {
        vector< vec3d >& sPntVec = m_SurfVec[i]->GetMesh()->GetSimpPntVec();
        allPntKey[i].resize( sPntVec.size() );
        for ( int v = 0; v < (int)sPntVec.size(); v++ )
        {
            if ( !m_SurfVec[i]->GetWakeFlag() )
            {
                allPntVec.push_back( sPntVec[v] );
                allPntKey[i][v] = k;
                k++;
            }
        }
    }

    //==== Build Map ====//
    PntNodeCloud pnCloud;
    pnCloud.AddPntNodes( allPntVec );

    double tol = 1e-6;
    //==== Use NanoFlann to Find Close Points and Group ====//
    IndexPntNodes( pnCloud, tol );

    //==== Assemble Normal Tris ====//
    vector< SimpFace > allFaceVec;
    for ( int i = 0; i < (int)m_SurfVec.size(); i++ )
    {
        if ( !m_SurfVec[i]->GetWakeFlag() )
        {
            vector < SimpFace >& sFaceVec = m_SurfVec[ i ]->GetMesh()->GetSimpFaceVec();
            vector< vec3d >& sPntVec = m_SurfVec[i]->GetMesh()->GetSimpPntVec();
            for ( int f = 0; f < (int)sFaceVec.size(); f++ )
            {
                int i0 = pnCloud.GetNodeUsedIndex( allPntKey[i][sFaceVec[f].ind0] );
                int i1 = pnCloud.GetNodeUsedIndex( allPntKey[i][sFaceVec[f].ind1] );
                int i2 = pnCloud.GetNodeUsedIndex( allPntKey[i][sFaceVec[f].ind2] );
                SimpFace sface;
                sface.ind0 = i0 + 1;
                sface.ind1 = i1 + 1;
                sface.ind2 = i2 + 1;
                sface.m_Tags = sFaceVec[f].m_Tags;
                if ( sFaceVec[f].m_isQuad )
                {
                    sface.m_isQuad = true;
                    int i3 = pnCloud.GetNodeUsedIndex( allPntKey[i][sFaceVec[f].ind3] );
                    sface.ind3 = i3 + 1;
                }
                allFaceVec.push_back( sface );
            }
        }
    }
    //==== Assemble All Used Points ====//
    vector< vec3d > allUsedPntVec;
    for ( int i = 0; i < (int)allPntVec.size(); i++ )
    {
        if ( pnCloud.UsedNode( i ) )
        {
            allUsedPntVec.push_back( allPntVec[i] );
        }
    }

    //=====================================================================================//
    //==== Write Facet File for Xpatch ====================================================//
    //=====================================================================================//
    if ( facet_fn.length() != 0 )
    {
        FILE* fp = fopen( facet_fn.c_str(), "w" );

        if ( fp )
        {
            fprintf( fp, "Exported from %s\n", VSPVERSION4 ); // Title/comment line
            fprintf( fp, "1 \n" ); // Number of "Big" parts (1 Vehicle broken into small parts by geom and subsurface)

            fprintf( fp, "%s\n", m_Vehicle->GetName().c_str() ); // Name of "Big" part

            // mirror -> i, a b c d
            //     if i = 0 -> no mirror
            //     if i = 1 -> "Big" part is mirrored across plane defined by ax+by+cz-d=0
            fprintf( fp, "0, 0.000 1.000 0.000 0.000 \n" );

            fprintf( fp, "%d \n", (int)allUsedPntVec.size() ); // # of nodes in "Big" part

            //==== Write All Pnts (Nodes) ====//
            for ( int i = 0; i < (int)allUsedPntVec.size(); i++ )
            {
                fprintf( fp, "%16.10g %16.10g %16.10g\n", allUsedPntVec[i].x(), allUsedPntVec[i].y(), allUsedPntVec[i].z() );
            }

            vector < int > face_offset; // vector of number of faces for each tag

            int materialID = 0; // Default Material ID of PEC (Referred to as "iCoat" in XPatch facet file documentation)

            vector < int > all_tag_vec = SubSurfaceMgr.GetAllTags(); // vector of tags, where each tag identifies a part or group of facets

            //==== Get # of facets for each part ====//
            for ( unsigned int i = 0; i < all_tag_vec.size(); i++ )
            {
                int tag_count = 0;

                for ( unsigned int j = 0; j < allFaceVec.size(); j++ )
                {
                    if ( all_tag_vec[i] == SubSurfaceMgr.GetTag( allFaceVec[j].m_Tags ) )
                    {
                        tag_count++;

                        // Count extra tri as quads will be split.
                        if( allFaceVec[j].m_isQuad )
                        {
                            tag_count++;
                        }
                    }
                }

                face_offset.push_back( tag_count );
            }

            fprintf( fp, "%zu \n", face_offset.size() ); // # of "Small" parts

            int facet_count = 0; // counter for number of tris/facets

                                 //==== Write Out Tris ====//
            for ( unsigned int i = 0; i < all_tag_vec.size(); i++ )
            {
                int curr_tag = all_tag_vec[i];
                bool new_section = true; // flag to write small part section header

                for ( unsigned int j = 0; j < allFaceVec.size(); j++ )
                {
                    if ( curr_tag == SubSurfaceMgr.GetTag( allFaceVec[j].m_Tags ) ) // only write out current tris for surrent tag
                    {
                        if ( new_section ) // write small part header and get material ID for small part
                        {
                            string name = SubSurfaceMgr.GetTagNames( allFaceVec[j].m_Tags );
                            fprintf( fp, "%s\n", name.c_str() ); // Write name of small part
                            fprintf( fp, "%d 3\n", face_offset[i] ); // Number of facets for the part, 3 nodes per facet

                            new_section = false;
                        }

                        facet_count++;

                        // 3 nodes of facet, material ID, component ID, running facet #:
                        fprintf( fp, "%d %d %d %d %u %d\n", allFaceVec[j].ind0, allFaceVec[j].ind1, allFaceVec[j].ind2, materialID, i + 1, facet_count );

                        // Split quad into two tris.  It may be possible to write out tris and quads separately by
                        // grouping them appropriately and then writing separate groups.  However, without documentation
                        // of the facet file format, this approach is the least risky.
                        if( allFaceVec[j].m_isQuad )
                        {
                            facet_count++;
                            fprintf( fp, "%d %d %d %d %u %d\n", allFaceVec[j].ind0, allFaceVec[j].ind2, allFaceVec[j].ind3, materialID, i + 1, facet_count );
                        }
                    }
                }
            }
            fclose( fp );
        }
    }
}

string CfdMeshMgrSingleton::CheckWaterTight()
{
    vector< Face* > faceVec;

    vector< vec3d > allPntVec;
    vector < vector < int > > allPntKey;
    allPntKey.resize( m_SurfVec.size() );
    int k = 0;
    for ( int i = 0 ; i < ( int )m_SurfVec.size() ; i++ )
    {
        if( m_SurfVec[i]->GetSurfaceCfdType() != vsp::CFD_TRANSPARENT || m_SurfVec[i]->GetFarFlag() || m_SurfVec[i]->GetSymPlaneFlag() )
        {
            vector< vec3d >& sPntVec = m_SurfVec[i]->GetMesh()->GetSimpPntVec();
            allPntKey[i].resize( sPntVec.size() );
            for ( int v = 0 ; v < ( int )sPntVec.size() ; v++ )
            {
                allPntVec.push_back( sPntVec[v] );
                allPntKey[i][v] = k;
                k++;
            }
        }
    }

    //==== Build Map ====//
    PntNodeCloud pnCloud;
    pnCloud.AddPntNodes( allPntVec );

    double tol = 1e-6;
    //==== Use NanoFlann to Find Close Points and Group ====//
    IndexPntNodes( pnCloud, tol );

    //==== Create Nodes ====//
    for ( int i = 0 ; i < ( int )allPntVec.size() ; i++ )
    {
        if ( pnCloud.UsedNode( i ) )
        {
            Node* n = new Node();
            n->pnt = allPntVec[i];
            m_nodeStore.push_back( n );
        }
    }

    //==== Create Edges and Tris ====//
    int moreThanTwoTriPerEdge = 0;
    unordered_map< int, vector<Edge*> > edgeMap;
    for ( int i = 0 ; i < ( int )m_SurfVec.size() ; i++ )
    {
        if( m_SurfVec[i]->GetSurfaceCfdType() != vsp::CFD_TRANSPARENT || m_SurfVec[i]->GetFarFlag() || m_SurfVec[i]->GetSymPlaneFlag() )
        {
            vector < SimpFace >& sFaceVec = m_SurfVec[ i ]->GetMesh()->GetSimpFaceVec();
            vector< vec3d >& sPntVec = m_SurfVec[i]->GetMesh()->GetSimpPntVec();
            for ( int f = 0 ; f < ( int )sFaceVec.size() ; f++ )
            {
                int ind1 = pnCloud.GetNodeUsedIndex( allPntKey[i][sFaceVec[f].ind0] );
                int ind2 = pnCloud.GetNodeUsedIndex( allPntKey[i][sFaceVec[f].ind1] );
                int ind3 = pnCloud.GetNodeUsedIndex( allPntKey[i][sFaceVec[f].ind2] );

                Edge *e0 = nullptr, *e1 = nullptr, *e2 = nullptr, *e3 = nullptr;

                Face* face = nullptr;
                if ( sFaceVec[f].m_isQuad )
                {
                    int ind4 = pnCloud.GetNodeUsedIndex( allPntKey[i][sFaceVec[f].ind3] );

                    e0 = FindAddEdge( edgeMap, m_nodeStore, ind1, ind2 );
                    e1 = FindAddEdge( edgeMap, m_nodeStore, ind2, ind3 );
                    e2 = FindAddEdge( edgeMap, m_nodeStore, ind3, ind4 );
                    e3 = FindAddEdge( edgeMap, m_nodeStore, ind4, ind1 );

                    face = new Face( m_nodeStore[ind1], m_nodeStore[ind2], m_nodeStore[ind3],  m_nodeStore[ind4], e0, e1, e2, e3 );
                }
                else
                {
                    e0 = FindAddEdge( edgeMap, m_nodeStore, ind1, ind2 );
                    e1 = FindAddEdge( edgeMap, m_nodeStore, ind2, ind3 );
                    e2 = FindAddEdge( edgeMap, m_nodeStore, ind3, ind1 );
                    face = new Face( m_nodeStore[ind1], m_nodeStore[ind2], m_nodeStore[ind3], e0, e1, e2 );
                }

                if ( !e0->SetFace( face ) )
                {
                    face->debugFlag = true;
                    moreThanTwoTriPerEdge++;
                }
                if ( !e1->SetFace( face ) )
                {
                    face->debugFlag = true;
                    moreThanTwoTriPerEdge++;
                }
                if ( !e2->SetFace( face ) )
                {
                    face->debugFlag = true;
                    moreThanTwoTriPerEdge++;
                }

                if ( sFaceVec[f].m_isQuad )
                {
                    if ( !e3->SetFace( face ) )
                    {
                        face->debugFlag = true;
                        moreThanTwoTriPerEdge++;
                    }
                }

                faceVec.push_back( face );

                if ( face->debugFlag )
                {
                    m_BadFaces.push_back( face );
                }
            }
        }
    }

    //==== Find Border Edges ====//
    int num_border_edges = 0;
    unordered_map<int, vector<Edge*> >::const_iterator iter;
    for ( iter = edgeMap.begin() ; iter != edgeMap.end() ; ++iter )
    {
        for ( int i = 0 ; i < ( int )iter->second.size() ; i++ )
        {
            Edge* e = iter->second[i];
            if ( !( e->f0 && e->f1 ) )
            {
                m_BadEdges.push_back( e );
                e->debugFlag = true;
                num_border_edges++;
            }

        }
    }

    //==== Clean Up ====//
    for ( iter = edgeMap.begin() ; iter != edgeMap.end() ; ++iter )
    {
        for ( int i = 0 ; i < ( int )iter->second.size() ; i++ )
        {
            if ( ! iter->second[i]->debugFlag )
            {
                delete iter->second[i];
            }
        }
    }
    for ( int i = 0 ; i < ( int )faceVec.size() ; i++ )
    {
        if ( ! faceVec[i]->debugFlag )
        {
            delete faceVec[i];
        }
    }

    char resultStr[255];
    if ( num_border_edges || moreThanTwoTriPerEdge )
    {
        snprintf( resultStr, sizeof( resultStr ), "NOT Water Tight : %d Border Edges, %d Edges > 2 Tris\n",
                 num_border_edges, moreThanTwoTriPerEdge );
    }
    else
    {
        snprintf( resultStr, sizeof( resultStr ), "Is Water Tight\n" );
    }

    return string( resultStr );

}

Edge* CfdMeshMgrSingleton::FindAddEdge( unordered_map< int, vector<Edge*> > & edgeMap, vector< Node* > & nodeVec, int ind1, int ind2 )
{
    Edge* e = nullptr;
    unordered_map<int, vector<Edge*> >::const_iterator iter;
    int combind = ind1 + ind2;
    iter = edgeMap.find( combind );

    if ( iter != edgeMap.end() )    // Found Edge Vec so Check
    {
        vector<Edge*> eVec = edgeMap[combind];
        for ( int i = 0 ; i < ( int )eVec.size() ; i++ )
        {
            if ( eVec[i]->ContainsNodes( nodeVec[ind1], nodeVec[ind2] ) )
            {
                return eVec[i];
            }
        }
    }

    //==== No Edge Found so Create
    e = new Edge( nodeVec[ind1], nodeVec[ind2] );
    edgeMap[combind].push_back( e );

    return e;
}

vector< Surf* > CfdMeshMgrSingleton::CreateDomainSurfs()
{
    //Create many planes
    vec3d outerBottomRight, outerBottomLeft, outerTopRight, outerTopLeft;
    vec3d innerBottomRight, innerBottomLeft, innerTopRight, innerTopLeft;
    vector<vec3d> corners;
    BndBox vehicleBBox = m_Vehicle->GetBndBox();
    SimpleCfdMeshSettings* settings = GetCfdSettingsPtr();
    double farXScale = settings->m_FarXScale;
    double farZScale = settings->m_FarZScale;
    double min = farXScale;
    double scale = 2.0;
    bool isInside = true;

    //Grab the minimum of the two scales
    if (farZScale < min)
    {
        min = farZScale;
    }

    //Makes center plane smaller if outer plane smaller than inner
    //Currently min can be set to 1.0 (the size of the BBox)
    if (min < scale)
    {
        scale = (min + 1.0)/2.0;
    }

    vehicleBBox.Scale(vec3d(scale, scale, scale));
    corners = vehicleBBox.GetCornerPnts();
    vehicleBBox.Reset();

    //Symmetry Plane Vertices
    outerBottomRight = m_Domain.GetCornerPnt(1);
    outerBottomLeft = m_Domain.GetCornerPnt(0);
    outerTopRight = m_Domain.GetCornerPnt(5);
    outerTopLeft = m_Domain.GetCornerPnt(4);
    innerBottomRight = vec3d(corners[2][0], outerBottomRight[1], corners[2][2]);
    innerBottomLeft = vec3d(corners[1][0], outerBottomLeft[1], corners[1][2]);
    innerTopRight = vec3d(corners[6][0], outerTopRight[1], corners[6][2]);
    innerTopLeft = vec3d(corners[5][0], outerTopLeft[1], corners[5][2]);

    //Checks to see if inner plane is outside of outer plane
    if ((innerBottomLeft.x() < outerBottomLeft.x() || innerBottomRight.x() > outerBottomRight.x()
            || innerTopLeft.z() > outerTopLeft.z() ||  innerBottomLeft.z() < outerBottomLeft.z())
        && GetCfdSettingsPtr()->m_FarManLocFlag)
    {
        isInside = false;
    }

    //Begin making the symmetry planes and rest of the box
    int index = 0;
    vector < vec3d > p0;
    if (GetCfdSettingsPtr()->m_HalfMeshFlag &&
            GetCfdSettingsPtr()->m_SymSplittingOnFlag && isInside)
    {
        p0.resize( 10 );
        p0[index++] = innerBottomRight;
        p0[index++] = innerBottomLeft;
        p0[index++] = outerBottomRight;
        p0[index++] = outerBottomRight;
        p0[index++] = innerTopRight;
    }
    else
    {
        p0.resize( 6 );
        p0[index++] = outerBottomRight;
    }
    p0[index++] = m_Domain.GetCornerPnt( 2 ); // Pilot's right
    p0[index++] = m_Domain.GetCornerPnt( 0 ); // Upstream of aircraft
    p0[index++] = m_Domain.GetCornerPnt( 3 ); // Downstream of aircraft
    p0[index++] = m_Domain.GetCornerPnt( 0 ); // Below aircraft
    p0[index++] = m_Domain.GetCornerPnt( 5 ); // Above aircraft

    vector < vec3d > p1;
    index = 0;
    if (GetCfdSettingsPtr()->m_HalfMeshFlag &&
        GetCfdSettingsPtr()->m_SymSplittingOnFlag && isInside)
    {
        p1.resize( 10 );
        p1[index++] = innerBottomLeft;
        p1[index++] = outerBottomLeft;
        p1[index++] = outerBottomLeft;
        p1[index++] = innerBottomRight;
        p1[index++] = innerTopLeft;
    }
    else
    {
        p1.resize( 6 );
        p1[index++] = outerBottomLeft;
    }
    p1[index++] = m_Domain.GetCornerPnt( 3 );
    p1[index++] = m_Domain.GetCornerPnt( 2 );
    p1[index++] = m_Domain.GetCornerPnt( 1 );
    p1[index++] = m_Domain.GetCornerPnt( 1 );
    p1[index++] = m_Domain.GetCornerPnt( 4 );

    vector < vec3d > p2;
    index = 0;
    if (GetCfdSettingsPtr()->m_HalfMeshFlag &&
        GetCfdSettingsPtr()->m_SymSplittingOnFlag && isInside)
    {
        p2.resize( 10 );
        p2[index++] = innerTopRight;
        p2[index++] = innerTopLeft;
        p2[index++] = innerBottomRight;
        p2[index++] = outerTopRight;
        p2[index++] = outerTopRight;
    }
    else
    {
        p2.resize( 6 );
        p2[index++] = outerTopRight;
    }
    p2[index++] = m_Domain.GetCornerPnt( 6 );
    p2[index++] = m_Domain.GetCornerPnt( 4 );
    p2[index++] = m_Domain.GetCornerPnt( 7 );
    p2[index++] = m_Domain.GetCornerPnt( 2 );
    p2[index++] = m_Domain.GetCornerPnt( 7 );

    vector < vec3d > p3;
    index = 0;
    if (GetCfdSettingsPtr()->m_HalfMeshFlag &&
        GetCfdSettingsPtr()->m_SymSplittingOnFlag && isInside)
    {
        p3.resize( 10 );
        p3[index++] = innerTopLeft;
        p3[index++] = outerTopLeft;
        p3[index++] = innerBottomLeft;
        p3[index++] = innerTopRight;
        p3[index++] = outerTopLeft;
    }
    else
    {
        p3.resize( 6 );
        p3[index++] = outerTopLeft;
    }

    p3[index++] = m_Domain.GetCornerPnt( 7 );
    p3[index++] = m_Domain.GetCornerPnt( 6 );
    p3[index++] = m_Domain.GetCornerPnt( 5 );
    p3[index++] = m_Domain.GetCornerPnt( 3 );
    p3[index++] = m_Domain.GetCornerPnt( 6 );

    // Default, no additional surfaces.
    int ndomain = 0;

    // Half mesh with no outer domain or component outer domain
    if ( GetCfdSettingsPtr()->m_HalfMeshFlag )
    {
        if (GetCfdSettingsPtr()->m_SymSplittingOnFlag && isInside)
        {
            ndomain = 5;
        }
        else
        {
            ndomain = 1;
        }
    }

    // Box outer domain, half or full mesh.
    if ( GetCfdSettingsPtr()->m_FarMeshFlag && !GetCfdSettingsPtr()->m_FarCompFlag)
    {
        if (GetCfdSettingsPtr()->m_SymSplittingOnFlag && isInside)
        {
            ndomain = 10;
        }
        else
        {
            ndomain = 6;
        }
    }

    vector< Surf* > domainSurfs;

    for ( int i = 0 ; i < ndomain ; ++i )
    {
        Surf* ptr = new Surf;
        domainSurfs.push_back( ptr );

        domainSurfs[i]->SetSurfID( i );
        domainSurfs[i]->SetCompID( i );
        domainSurfs[i]->SetUnmergedCompID( i );

        domainSurfs[i]->SetSurfaceCfdType(vsp::CFD_TRANSPARENT);

        //If symmetry and plane is inside outer plane
        if( GetCfdSettingsPtr()->m_SymSplittingOnFlag &&
                i < 5 && GetCfdSettingsPtr()->m_HalfMeshFlag && isInside)
        {
            domainSurfs[i]->SetSymPlaneFlag( true );
        }
        //If symmetry plane and inside plane is not within outer plane
        else if (GetCfdSettingsPtr()->m_SymSplittingOnFlag &&
                 i < 1 && GetCfdSettingsPtr()->m_HalfMeshFlag && !isInside)
        {
            domainSurfs[i]->SetSymPlaneFlag( true );
        }
        //If symmetry splitting is off
        else if (!GetCfdSettingsPtr()->m_SymSplittingOnFlag &&
                 i < 1 && GetCfdSettingsPtr()->m_HalfMeshFlag )
        {
            domainSurfs[i]->SetSymPlaneFlag( true );
        }
        else
        {
            domainSurfs[i]->SetFarFlag( true );
        }

        threed_point_type pt0, pt1, pt2, pt3;

        p0[i].get_pnt(pt0);
        p1[i].get_pnt(pt1);
        p2[i].get_pnt(pt2);
        p3[i].get_pnt(pt3);

        domainSurfs[i]->GetSurfCore()->MakePlaneSurf( pt0, pt1, pt2, pt3 );
        domainSurfs[i]->GetSurfCore()->BuildPatches( domainSurfs[i] );
    }
    return domainSurfs;
}

void CfdMeshMgrSingleton::InitMesh( )
{
    addOutputText( "MatchWakes\n" );
    MatchWakes();

    addOutputText( "TessellateChains\n" );
    TessellateChains();

//DebugWriteChains( "Tess_UW", true );

    addOutputText( "MergeBorderEndPoints\n" );
    MergeBorderEndPoints();

    AddWakeCoPlanarSurfaceChains();

    // addOutputText( "BuildMesh\n" );  Output in BuildMesh
    BuildMesh();

    addOutputText( "RemoveInteriorTris\n" );
    RemoveInteriorTris();

    addOutputText( "RemoveTrimTris\n" );
    RemoveTrimTris();
}

void CfdMeshMgrSingleton::TessellateChains()
{
    //==== Tessellate Chains ====//
    list< ISegChain* >::iterator c;
    for ( c = m_ISegChainList.begin() ; c != m_ISegChainList.end(); ++c )
    {
        if( ( *c )->GetWakeAttachChain() == nullptr ) // Non wake-attach chains.
        {
            ( *c )->Tessellate();
            ( *c )->TransferTess();
            ( *c )->ApplyTess( this );
        }
    }

    for ( c = m_ISegChainList.begin() ; c != m_ISegChainList.end(); ++c )
    {
        if( ( *c )->GetWakeAttachChain() != nullptr ) // Only wake-attach chains.
        {
            vector< double > u = ( *c )->GetWakeAttachChain()->m_ACurve.GetUTessPnts();
            ( *c )->m_ACurve.Tesselate( u ); // Copy tessellation from matching chain.
            ( *c )->TransferTess();
            ( *c )->ApplyTess( this );
        }
    }


    ////==== Check for Zero Length Chains ====//
    //for ( c = m_ISegChainList.begin() ; c != m_ISegChainList.end(); c++ )
    //{
    //  //double d = dist( (*c)->m_TessVec.front()->m_Pnt, (*c)->m_TessVec.back()->m_Pnt );
    //  //if ( d < 0.001 )
    //  //{
    //  //  printf("Zero Length Chain\n");
    //  //}

    //  ////==== Compute Total Distance Between Points ====//
    //  //double total_dist = 0;
    //  //for ( int i = 0 ; i < (*c)->m_TessVec.size() ; i++ )
    //  //{
    //  //  Puw* puwa = (*c)->m_TessVec[i]->GetPuw( (*c)->m_SurfA );
    //  //  Puw* puwb = (*c)->m_TessVec[i]->GetPuw( (*c)->m_SurfB );
    //  //  vec2d auw = puwa->m_UW;
    //  //  vec2d buw = puwb->m_UW;
    //  //  vec3d pa = (*c)->m_SurfA->CompPnt( auw[0], auw[1] );
    //  //  vec3d pb = (*c)->m_SurfB->CompPnt( buw[0], buw[1] );
    //  //  double d = dist( pa, pb );
    //  //  total_dist += d;
    //  //}
    //  //printf("Total Chain Delta = %f \n", total_dist );
    //}
}

// Given a chain connecting a wake to its forming trailing edge, this routine
// finds the matching chain that connects the top and bottom surface of the
// trailing edge.  It then sets the pointer from the argument chain to the
// matching chain.
void CfdMeshMgrSingleton::SetWakeAttachChain( ISegChain* c )
{
    list< ISegChain* >::iterator d;
    Surf* sca = c->m_SurfA;

    for ( d = m_ISegChainList.begin() ; d != m_ISegChainList.end(); ++d )
    {
        Surf* sda = ( *d )->m_SurfA;

        if( ( c != ( *d ) ) && ( *d )->m_BorderFlag && ( sca->GetSurfID() == sda->GetSurfID() ) )
        {
            if( c->Match( ( *d ) ) )
            {
                c->SetWakeAttachChain( ( *d ) );
            }
        }
    }
}

void CfdMeshMgrSingleton::MatchWakes()
{
    //==== Match Wakes ====//
    list< ISegChain* >::iterator c;
    for ( c = m_ISegChainList.begin() ; c != m_ISegChainList.end(); ++c )
    {
        Surf* sca = ( *c )->m_SurfA;
        Surf* scb = ( *c )->m_SurfB;

        // Looking for border curves
        if( ( *c )->m_BorderFlag )
        {
            // From construction, on attachment curve, the wake is always the 'B' surface.
            // Where only one surface is a wake.
            if( scb->GetWakeFlag() && ( ! sca->GetWakeFlag() ) )
            {
                // And where the wake parent matches the other surface.
                if( scb->GetWakeParentSurfID() == sca->GetSurfID() )
                {
                    // After all this, we know that
                    // *c is a pointer to a chain that connects a wake
                    // to its constructing wing trailing edge.
                    SetWakeAttachChain( ( *c ) );
                }
            }
        }
    }
}

void CfdMeshMgrSingleton::AddWakeCoPlanarSurfaceChains()
{
    //==== Find All Border Chains ===//
    list< ISegChain* >::iterator c;
    vector< ISegChain* > borderVec;
    for ( c = m_ISegChainList.begin() ; c != m_ISegChainList.end(); ++c )
    {
        if ( ( *c )->m_BorderFlag )
        {
            borderVec.push_back( ( *c ) );
        }
    }


    //==== Find Wake Surfaces ====//
    unordered_map< Surf*, vector< Surf* > >::iterator iter;

    for ( iter = m_PossCoPlanarSurfMap.begin() ; iter != m_PossCoPlanarSurfMap.end() ; ++iter )
    {
        if ( iter->first->GetWakeFlag() )
        {
            vector< Surf* > possSurfVec = iter->second;
            for ( int b = 0 ; b < ( int )borderVec.size() ; b++ )
            {
                ISegChain* chain = borderVec[b];

                bool checkFlag = false;
                for ( int s = 0 ; s < ( int )possSurfVec.size() ; s++ )
                {
                    if ( chain->m_SurfA == possSurfVec[s] ||  chain->m_SurfB == possSurfVec[s] )
                    {
                        checkFlag = true;
                    }
                }
                if ( checkFlag )
                {
                    AddSurfaceChain( iter->first, chain );
                }
            }
        }
    }
}


void CfdMeshMgrSingleton::AddSurfaceChain( Surf* sPtr, ISegChain* chainIn )
{
    //==== Check if Border Chains Lie On Another Surfaces ====//
    vector< ISegChain* > new_chains = chainIn->FindCoPlanarChains( sPtr, this );

    for ( int i = 0 ; i < ( int )new_chains.size() ; i++ )
    {
        new_chains[i]->m_BorderFlag = true;
        new_chains[i]->m_SurfA = sPtr;
        new_chains[i]->m_SurfB = sPtr;

        if( new_chains[i]->Valid() )
        {
            m_ISegChainList.push_back( new_chains[i] );
        }
    }
}

void CfdMeshMgrSingleton::MergeBorderEndPoints()
{
    IPntCloud cloud;
    cloud.m_IPnts.reserve( m_ISegChainList.size() * 2 );

    list< ISegChain* >::iterator c;
    for ( c = m_ISegChainList.begin() ; c != m_ISegChainList.end(); ++c )
    {
        if( (*c)->m_BorderFlag )
        {
            cloud.m_IPnts.push_back(( *c )->m_TessVec.front()); // Add Front Point
            cloud.m_IPnts.push_back(( *c )->m_TessVec.back());  // Add Back Point
        }
    }

    for ( c = m_ISegChainList.begin() ; c != m_ISegChainList.end(); ++c )
    {
        if( !(*c)->m_BorderFlag )
        {
            cloud.m_IPnts.push_back(( *c )->m_TessVec.front()); // Add Front Point
            cloud.m_IPnts.push_back(( *c )->m_TessVec.back());  // Add Back Point
        }
    }


    // tol_fract previously was compared to the distance between groups as a fraction of the local edge length.
    // However, it currently is simply compared to the distance between groups.
    // Consequently, while a reasonable value was previously 1e-2, a much smaller value is now appropriate.
    // In addition, this tolerance is actually in terms of r^2.
    // This would be better if we had a better representative minimum feature length that was the minimum of:
    //     Grid minimum edge length
    //     Shortest edge length specified in any source
    //     Shortest length of any boundary
    //     Shortest length of any intersection curve
    double tol = GetGridDensityPtr()->m_MinLen / 100.0;
    tol = tol * tol;

    MergeEndPointCloud( cloud, tol );
}

void CfdMeshMgrSingleton::MergeEndPointCloud( IPntCloud &cloud, double tol )
{
    list< ISegChain* >::iterator c;

    IPntTree index( 3, cloud, KDTreeSingleIndexAdaptorParams( 10 ) );
    index.buildIndex();

    list < IPntGroup* > iPntGroupList;

    for ( size_t i = 0 ; i < cloud.m_IPnts.size() ; i++ )
    {
        if ( cloud.m_IPnts[i]->m_GroupedFlag == false )
        {
            iPntGroupList.push_back( new IPntGroup );
            m_DelIPntGroupVec.push_back( iPntGroupList.back() );

            std::vector < std::pair < unsigned int, double > > ret_matches;

            nanoflann::SearchParams params;
            index.radiusSearch( &cloud.m_IPnts[i]->m_Pnt[0], tol, ret_matches, params );

            for ( size_t j = 0 ; j < ret_matches.size() ; j++ )
            {
                unsigned int m_ind = ret_matches[j].first;
                cloud.m_IPnts[ m_ind ]->m_GroupedFlag = true;
                iPntGroupList.back()->m_IPntVec.push_back( cloud.m_IPnts[ m_ind ] );
            }
        }
    }


    //==== Merge Ipnts In Groups ====//
    list< IPntGroup* >::iterator g;
    vector< IPnt* > merged_ipnts;
    for ( g = iPntGroupList.begin() ; g != iPntGroupList.end(); ++g )
    {
        IPnt* mip = new IPnt();
        m_DelIPntVec.push_back( mip );
        for ( int j = 0 ; j < ( int )( *g )->m_IPntVec.size() ; j++ )
        {
            mip->AddPuws( ( *g )->m_IPntVec[j] );
        }
        mip->CompPnt();
        merged_ipnts.push_back( mip );
    }

    //==== Replace IPnts in Chains ====//
    for ( c = m_ISegChainList.begin() ; c != m_ISegChainList.end(); ++c )
    {
        IPnt* ip = ( *c )->m_TessVec.front();
        int cnt = 0;
        for ( g = iPntGroupList.begin() ; g != iPntGroupList.end(); ++g )
        {
            for ( int j = 0 ; j < ( int )( *g )->m_IPntVec.size() ; j++ )
            {
                if ( ip == ( *g )->m_IPntVec[j] )
                {
                    ( *c )->m_TessVec.front() = merged_ipnts[cnt];
                    break;
                }
            }
            cnt++;
        }
        cnt = 0;
        ip = ( *c )->m_TessVec.back();
        for ( g = iPntGroupList.begin() ; g != iPntGroupList.end(); ++g )
        {
            for ( int j = 0 ; j < ( int )( *g )->m_IPntVec.size() ; j++ )
            {
                if ( ip == ( *g )->m_IPntVec[j] )
                {
                    ( *c )->m_TessVec.back() = merged_ipnts[cnt];
                    break;
                }
            }
            cnt++;
        }
    }
}

void CfdMeshMgrSingleton::BuildMesh()
{
    char str[256];

    //==== Mesh Each Surface ====//
    for ( int s = 0 ; s < ( int )m_SurfVec.size() ; s++ )
    {
        vector< ISegChain* > surf_chains;
        list< ISegChain* >::iterator c;
        for ( c = m_ISegChainList.begin() ; c != m_ISegChainList.end(); ++c )
        {
            if ( ( ( *c )->m_SurfA == m_SurfVec[s] || ( *c )->m_SurfB == m_SurfVec[s] ) )
            {
                surf_chains.push_back( ( *c ) );
            }
        }

        vector < vec2d > adduw;
        ForceSurfaceFixPoints( s, adduw );

        snprintf( str, sizeof( str ), "InitMesh %3d/%3zu %s\n", s+1, m_SurfVec.size(), m_SurfVec[s]->GetDisplayName().c_str() );
        addOutputText( str );
        m_SurfVec[s]->InitMesh( surf_chains, adduw, this );
    }
}

// Determines if a triangle should be deleted based on its type and whether or not it is inside every other surface
bool CfdMeshMgrSingleton::SetDeleteTriFlag( int aType, bool symPlane, vector < bool > aInB )
{
    // Always delete Stiffener tris
    if ( aType == vsp::CFD_STIFFENER )
    {
        return true;
    }

    for ( int b = 0 ; b < ( int )m_SurfVec.size() ; b++ )
    {
        int c = m_SurfVec[b]->GetCompID();
        if ( c >= 0 && c < aInB.size() )
        {
            bool aInThisB = aInB[c];

            int bType = m_SurfVec[b]->GetSurfaceCfdType();
            bool bThick = m_SurfVec[b]->GetSurfaceThickSurf();

            // Can make absolute decisions about deleting a triangle or not in the cases below
            if ( aInThisB && bThick )
            {
                // Trim Symmetry plane
                if ( symPlane && m_SurfVec[b]->GetFarFlag() &&
                     GetSettingsPtr()->m_FarCompFlag )
                {
                    return true;
                }
                // Normal(Positive) inside another Normal, or Negative inside another Negative
                if ( aType == bType && ( aType != vsp::CFD_TRANSPARENT && aType != vsp::CFD_STRUCTURE ) )
                {
                    return true;
                }
                // Always delete Normal tris inside Negative surfaces
                else if ( aType == vsp::CFD_NORMAL && bType == vsp::CFD_NEGATIVE )
                {
                    return true;
                }
                // Never delete Transparent tris inside Negative surfaces
                else if ( aType == vsp::CFD_TRANSPARENT && bType == vsp::CFD_NEGATIVE )
                {
                    return false;
                }
                // Always delete Structure tris inside Negative surfaces
                else if ( aType == vsp::CFD_STRUCTURE && bType == vsp::CFD_NEGATIVE )
                {
                    return true;
                }
            }
        }
    }

    // Default condition for deleteTri.
    // The default value is applied for a triangle that is not inside
    // any other object.  I.e. an isolated thing in 'free space'.
    //
    // vsp::CFD_NORMAL, vsp::CFD_TRANSPARENT
    bool deleteTri = false;

    // Flip sense of default value.  These things do not exist in 'free space'.
    if ( aType == vsp::CFD_NEGATIVE ||
         aType == vsp::CFD_STRUCTURE ||
         aType == vsp::CFD_STIFFENER )   // Stiffener is special case -- always true previously.
    {
        deleteTri = true;
    }

    // Check non-absolute cases
    for ( int b = 0 ; b < ( int )m_SurfVec.size() ; b++ )
    {
        int c = m_SurfVec[b]->GetCompID();
        if ( c >= 0 && c < aInB.size() )
        {
            bool aInThisB = aInB[c];
            int bType = m_SurfVec[b]->GetSurfaceCfdType();
            bool bThick = m_SurfVec[b]->GetSurfaceThickSurf();

            if ( aInThisB && bThick )
            {
                if ( ( aType == vsp::CFD_NEGATIVE || aType == vsp::CFD_STRUCTURE ) && bType == vsp::CFD_NORMAL )
                {
                    return false;
                }
                else if ( aType == vsp::CFD_TRANSPARENT && bType == vsp::CFD_NORMAL )
                {
                    return true;
                }
            }
        }
    }

    return deleteTri;
}

void CfdMeshMgrSingleton::RemoveInteriorTris()
{
    debugRayIsect.clear();

    //==== Find Max Bound Box of All Components ====//
    int s;
    BndBox big_box;
    for (  s = 0 ; s < ( int )m_SurfVec.size() ; ++s )
    {
        big_box.Update( m_SurfVec[s]->GetBBox() );
    }
    double x_dist = 1.0 + big_box.GetMax( 0 ) - big_box.GetMin( 0 );

    //==== Count Number of Component Crossings for Each Component =====//
    list< Face* >::iterator f;
    for ( s = 0 ; s < ( int )m_SurfVec.size() ; ++s ) // every surface
    {
        int s_comp_id = m_SurfVec[s]->GetCompID();
        list <Face*> faceList = m_SurfVec[ s ]->GetMesh()->GetFaceList();
        for ( f = faceList.begin() ; f != faceList.end(); ++f ) // every triangle
        {
            vector< vector< double > > t_vec_vec;

            if ( GetSettingsPtr()->m_SymSplittingOnFlag )
            {
                t_vec_vec.resize( m_NumComps + 10 );  // + 10 to handle possibility of outer domain and symmetry plane.
                ( *f )->insideSurf.resize( m_NumComps + 10);
                ( *f )->insideCount.resize( m_NumComps + 10);
            }
            else
            {
                t_vec_vec.resize( m_NumComps + 6 );
                ( *f )->insideSurf.resize( m_NumComps + 6);
                ( *f )->insideCount.resize( m_NumComps + 6);
            }

            vec3d cp = ( *f )->ComputeCenterPnt( m_SurfVec[s] );
            vec3d ep = cp + vec3d( x_dist, 1.0e-4, 1.0e-4 );

            for ( int i = 0 ; i < ( int )m_SurfVec.size() ; ++i )
            {
                int comp_id = m_SurfVec[i]->GetCompID();
                if ( i != s && comp_id != s_comp_id ) // Don't check self intersection.
                {
                    if ( m_SurfVec[s]->GetFeaSymmIndex() >=0 && m_SurfVec[s]->GetFeaSymmIndex() != m_SurfVec[i]->GetFeaSymmIndex() )
                    {
                        // Do nothing.
                    }
                    else if ( m_SurfVec[i]->GetSurfaceCfdType() != vsp::CFD_TRANSPARENT &&
                         m_SurfVec[i]->GetSurfaceCfdType() != vsp::CFD_STRUCTURE &&
                         m_SurfVec[i]->GetSurfaceCfdType() != vsp::CFD_STIFFENER ) // Don't check against transparent, structure, or stiffener surf.
                    {
                        m_SurfVec[i]->IntersectLineSeg( cp, ep, t_vec_vec[comp_id] );
                    }
                    else if ( m_SurfVec[i]->GetFarFlag() && m_SurfVec[s]->GetSymPlaneFlag() &&
                              GetSettingsPtr()->m_FarCompFlag ) // Unless trimming sym plane by outer domain
                    {
                        m_SurfVec[i]->IntersectLineSeg( cp, ep, t_vec_vec[comp_id] );
                    }
                }
            }

            // Loop over m_SurfVec instead of component id's.  Components will be addressed multiple times,
            // but it allows access to m_SurfVec[i]->GetFarFlag() without a reverse lookup on component id.
            for ( int i = 0 ; i < ( int )m_SurfVec.size() ; ++i )
            {
                int c = m_SurfVec[i]->GetCompID();

                if ( c >= 0 && c < ( *f )->insideSurf.size() )
                {
                    if ( m_SurfVec[s]->GetSymPlaneFlag() && m_SurfVec[i]->GetFarFlag() &&
                         GetSettingsPtr()->m_FarCompFlag )
                    {
                        if ( ( int )( t_vec_vec[c].size() + 1 ) % 2 == 1 ) // +1 Reverse action on sym plane wrt outer boundary.
                        {
                            ( *f )->insideSurf[c] = true;
                        }
                    }
                    else
                    {

                        if ( ( int )t_vec_vec[c].size() % 2 == 1)
                        {
                            ( *f )->insideSurf[c] = true;
                        }
                    }
                }
            }
        }

        for ( f = faceList.begin() ; f != faceList.end(); ++f ) // every face
        {
            //==== Load Adjoining Faces - NOT Crossing Borders ====//
            set< Face* > faceSet;
            ( *f )->LoadAdjFaces( 3, faceSet );

            set<Face*>::iterator sf;

            for ( int i = 0 ; i < ( int )m_SurfVec.size() ; ++i )
            {
                int c = m_SurfVec[i]->GetCompID();
                if ( c >= 0 && c < ( *f )->insideSurf.size() )
                {

                    for ( sf = faceSet.begin() ; sf != faceSet.end() ; ++sf )
                    {
                        if ( ( *f )->insideSurf[c] )
                        {
                            ( *sf )->insideCount[c]++;
                        }
                        else
                        {
                            ( *sf )->insideCount[c]--;
                        }
                    }
                }
            }
        }
    }

    //==== Check Vote and Mark Interior Tris =====//
    for ( s = 0 ; s < ( int )m_SurfVec.size() ; ++s )
    {
        list <Face*> faceList = m_SurfVec[ s ]->GetMesh()->GetFaceList();
        for ( f = faceList.begin() ; f != faceList.end(); ++f )
        {
            for ( int i = 0 ; i < ( int )m_SurfVec.size() ; ++i )
            {
                int c = m_SurfVec[i]->GetCompID();

                if ( c >= 0 && c < ( *f )->insideSurf.size() )
                {

                    if (( *f )->insideCount[c] > 0 )
                    {
                        ( *f )->insideSurf[c] = true;
                    }
                    else if (( *f )->insideCount[c] < 0 )
                    {
                        ( *f )->insideSurf[c] = false;
                    }
                    else // Can't determine if Tri is inside or outside based on neighbor votes
                    {
                        printf( "IntExtCount ZERO!\n" );
                    }

                }
            }
        }
    }

    for ( int a = 0 ; a < ( int )m_SurfVec.size() ; a++ )
    {
        list< Face * > faceList = m_SurfVec[ a ]->GetMesh()->GetFaceList();
        for ( f = faceList.begin(); f != faceList.end(); ++f )
        {
            // Determine if the triangle should be deleted
            if ( m_SurfVec[a]->GetIgnoreSurfFlag() )
            {
                ( *f )->deleteFlag = true;
            }
            else
            {
                ( *f )->deleteFlag = SetDeleteTriFlag( m_SurfVec[a]->GetSurfaceCfdType(), m_SurfVec[a]->GetSymPlaneFlag(), ( *f )->insideSurf );
            }
        }
    }

    //==== Check For Half Mesh ====//
    if ( GetSettingsPtr()->m_HalfMeshFlag )
    {
        for ( s = 0 ; s < ( int )m_SurfVec.size() ; s++ )
        {
            if ( ! m_SurfVec[s]->GetSymPlaneFlag() )
            {
                list <Face*> faceList = m_SurfVec[ s ]->GetMesh()->GetFaceList();
                for ( f = faceList.begin() ; f != faceList.end(); ++f )
                {
                    vec3d cp = ( *f )->ComputeCenterPnt( m_SurfVec[s] );
                    if ( cp[1] < -1.0e-10 )
                    {
                        ( *f )->deleteFlag = true;
                    }
                }
            }

            if( !GetSettingsPtr()->m_FarMeshFlag ) // Don't keep symmetry plane.
            {
                if ( m_SurfVec[s]->GetSymPlaneFlag() )
                {
                    list <Face*> faceList = m_SurfVec[ s ]->GetMesh()->GetFaceList();
                    for ( f = faceList.begin() ; f != faceList.end(); ++f )
                    {
                        ( *f )->deleteFlag = true;
                    }
                }
            }
        }
    }

    //==== Remove Tris, Edges and Nodes ====//
    for ( s = 0 ; s < ( int )m_SurfVec.size() ; s++ )
    {
        m_SurfVec[ s ]->GetMesh()->RemoveInteriorFacesEdgesNodes();
    }
}

void CfdMeshMgrSingleton::ConnectBorderNodes( bool wakeOnly )
{
    vector< Node* > nodeVec;
    for ( int s = 0 ; s < ( int )m_SurfVec.size() ; s++ )
    {
        if ( m_SurfVec[s]->GetWakeFlag() == wakeOnly )
        {
            list <Face*> faceList = m_SurfVec[ s ]->GetMesh()->GetFaceList();
            for ( list< Face* >::iterator f = faceList.begin() ; f != faceList.end(); ++f )
            {
                ( *f )->AddBorderNodes( nodeVec );
            }
        }
    }

    std::sort( nodeVec.begin(), nodeVec.end() );
    auto nvit = std::unique( nodeVec.begin(), nodeVec.end() );
    nodeVec.erase( nvit, nodeVec.end() );

    MatchBorderNodes( nodeVec );
}

void CfdMeshMgrSingleton::MatchBorderNodes( const vector< Node* > & nodeVec )
{
    vector < vec3d > ptVec;
    ptVec.reserve( nodeVec.size() );
    vector < int > indexVec;
    indexVec.reserve( nodeVec.size() );

    for ( int i = 0; i < nodeVec.size(); i++ )
    {
        try
        {
            ptVec.push_back( nodeVec[i]->pnt );
            indexVec.push_back( i );
        }
        catch ( int ex )
        {
        }
    }

    //==== Build Map ====//
    PntNodeCloud pnCloud;
    pnCloud.AddPntNodes( ptVec );

    double tol = 1e-10;
    //==== Use NanoFlann to Find Close Points and Group ====//
    IndexPntNodes( pnCloud, tol );

    for ( int i = 0; i < ptVec.size(); i++ )
    {
        if ( pnCloud.UsedNode( i ) )
        {
            vector < long long > matches = pnCloud.GetMatches( i );

            for ( int j = 1; j < matches.size(); j++ )
            {
                nodeVec[ indexVec[ matches[j] ] ]->pnt = nodeVec[ indexVec[i] ]->pnt;
            }
        }
    }

}

void CfdMeshMgrSingleton::UpdateDrawObjs()
{
    SurfaceIntersectionSingleton::UpdateDrawObjs();

    unsigned int num_reason = vsp::NUM_MESH_REASON + 1;
    m_ReasonDO.resize( num_reason * 2 );

    char str[256];
    for ( int i = 0; i < num_reason; i++ )
    {
        snprintf( str, sizeof( str ), "%s_TREASON_%d", GetID().c_str(), i );
        m_ReasonDO[i].m_GeomID = string( str );

        snprintf( str, sizeof( str ), "%s_QREASON_%d", GetID().c_str(), i + num_reason );
        m_ReasonDO[ i + num_reason ].m_GeomID = string( str );

        m_ReasonDO[i].m_GeomChanged = true;
        m_ReasonDO[ i + num_reason ].m_GeomChanged = true;
    }

    for ( int i = 0 ; i < ( int )m_SurfVec.size() ; i++ )
    {
        vector< vec3d > pVec = m_SurfVec[i]->GetMesh()->GetSimpPntVec();
        vector < SimpFace > fVec = m_SurfVec[ i ]->GetMesh()->GetSimpFaceVec();
        for ( int f = 0 ; f < ( int ) fVec.size() ; f++ )
        {
            SimpFace* sface = &fVec[f];

            if ( sface->m_reason >= 0 && sface->m_reason < num_reason )
            {
                int ido = sface->m_reason;
                if ( sface->m_isQuad )
                {
                    ido += num_reason;
                }

                vec3d norm = cross( pVec[sface->ind1] - pVec[sface->ind0], pVec[sface->ind2] - pVec[sface->ind0] );
                norm.normalize();
                m_ReasonDO[ ido ].m_PntVec.push_back( pVec[sface->ind0] );
                m_ReasonDO[ ido ].m_PntVec.push_back( pVec[sface->ind1] );
                m_ReasonDO[ ido ].m_PntVec.push_back( pVec[sface->ind2] );
                m_ReasonDO[ ido ].m_NormVec.push_back( norm );
                m_ReasonDO[ ido ].m_NormVec.push_back( norm );
                m_ReasonDO[ ido ].m_NormVec.push_back( norm );
                if ( sface->m_isQuad )
                {
                    m_ReasonDO[ ido ].m_PntVec.push_back( pVec[sface->ind3] );
                    m_ReasonDO[ ido ].m_NormVec.push_back( norm );
                }
            }

            if ( sface->m_reason < 0 || sface->m_reason >= vsp::NUM_MESH_REASON )
            {
                printf( "Invalid reason %d\n", sface->m_reason );
            }
        }
    }


    // Render Tag Colors
    unsigned int num_tags = SubSurfaceMgr.GetNumTags();
    m_TagDO.resize( num_tags * 2 );
    unordered_map<int, DrawObj*> tag_tri_dobj_map;
    unordered_map<int, DrawObj*> tag_quad_dobj_map;
    map< std::vector<int>, int >::const_iterator mit;
    unordered_map< int, DrawObj* >::const_iterator dmit;
    map< std::vector<int>, int > tagMap = SubSurfaceMgr.GetSingleTagMap();

    int cnt = 0;

    for ( mit = tagMap.begin(); mit != tagMap.end() ; ++mit )
    {
        tag_tri_dobj_map[ mit->second ] = &m_TagDO[cnt];
        snprintf( str, sizeof( str ), "%s_TTAG_%d", GetID().c_str(), cnt );
        m_TagDO[cnt].m_GeomID = string( str );

        tag_quad_dobj_map[ mit->second ] = &m_TagDO[cnt + num_tags];
        snprintf( str, sizeof( str ), "%s_QTAG_%d", GetID().c_str(), cnt + num_tags );
        m_TagDO[cnt + num_tags].m_GeomID = string( str );

        m_TagDO[cnt].m_GeomChanged = true;
        m_TagDO[cnt + num_tags].m_GeomChanged = true;

        cnt++;
    }

    for ( int i = 0 ; i < ( int )m_SurfVec.size() ; i++ )
    {
        vector< vec3d > pVec = m_SurfVec[i]->GetMesh()->GetSimpPntVec();
        vector < SimpFace > fVec = m_SurfVec[ i ]->GetMesh()->GetSimpFaceVec();
        for ( int f = 0 ; f < ( int ) fVec.size() ; f++ )
        {
            SimpFace* sface = &fVec[f];
            if ( sface->m_isQuad )
            {
                dmit = tag_quad_dobj_map.find( SubSurfaceMgr.GetTag( sface->m_Tags ) );

                if ( dmit == tag_quad_dobj_map.end() )
                {
                    continue;
                }
            }
            else
            {
                dmit = tag_tri_dobj_map.find( SubSurfaceMgr.GetTag( sface->m_Tags ) );

                if ( dmit == tag_tri_dobj_map.end() )
                {
                    continue;
                }
            }

            DrawObj* obj = dmit->second;
            vec3d norm = cross( pVec[sface->ind1] - pVec[sface->ind0], pVec[sface->ind2] - pVec[sface->ind0] );
            norm.normalize();
            obj->m_PntVec.push_back( pVec[sface->ind0] );
            obj->m_PntVec.push_back( pVec[sface->ind1] );
            obj->m_PntVec.push_back( pVec[sface->ind2] );
            obj->m_NormVec.push_back( norm );
            obj->m_NormVec.push_back( norm );
            obj->m_NormVec.push_back( norm );
            if ( sface->m_isQuad )
            {
                obj->m_PntVec.push_back( pVec[sface->ind3] );
                obj->m_NormVec.push_back( norm );
            }

        }
    }

#ifdef DEBUG_CFD_MESH
    m_DegenCornerPointDO.m_GeomID = GetID() + "DegenCornerPts";
    m_DegenCornerPointDO.m_Type = DrawObj::VSP_POINTS;
    m_DegenCornerPointDO.m_PointSize = 10.0;
    m_DegenCornerPointDO.m_PointColor = vec3d( 0, 0, 1 );

    m_DegenCornerEdgeDO.m_GeomID = GetID() + "DegenCornerEdge";
    m_DegenCornerEdgeDO.m_Type = DrawObj::VSP_LINES;
    m_DegenCornerEdgeDO.m_LineColor = vec3d( 0, 0, 1 );
    m_DegenCornerEdgeDO.m_LineWidth = 3.0;

    m_DegenCornerPointDO.m_PntVec.clear();
    m_DegenCornerEdgeDO.m_PntVec.clear();
    for ( int i = 0; i < m_DegenCornerChains.size(); i++ )
    {
        ISegChain* dchain = m_DegenCornerChains[i];

        // Add points
        for ( int j = 0; j < dchain->m_ISegDeque.size(); j++ )
        {
            m_DegenCornerPointDO.m_PntVec.push_back( dchain->m_ISegDeque[j]->m_IPnt[0]->m_Pnt );
            m_DegenCornerPointDO.m_PntVec.push_back( dchain->m_ISegDeque[j]->m_IPnt[1]->m_Pnt );
        }

        for ( int j = 0; j < dchain->m_TessVec.size() - 1; j++ )
        {
            m_DegenCornerEdgeDO.m_PntVec.push_back( dchain->m_TessVec[j]->m_Pnt );
            m_DegenCornerEdgeDO.m_PntVec.push_back( dchain->m_TessVec[j+1]->m_Pnt );
        }
    }
    m_DegenCornerPointDO.m_GeomChanged = true;
    m_DegenCornerEdgeDO.m_GeomChanged = true;
#endif

    // Render bad edges
    m_MeshBadEdgeDO.m_GeomID = GetID() + "BADEDGE";
    m_MeshBadEdgeDO.m_Type = DrawObj::VSP_LINES;
    m_MeshBadEdgeDO.m_LineColor = vec3d( 1, 0, 0 );
    m_MeshBadEdgeDO.m_LineWidth = 3.0;

    vector< vec3d > badEdgeData;

    vector< Edge* >::iterator e;
    for ( e = m_BadEdges.begin() ; e != m_BadEdges.end(); ++e )
    {
        badEdgeData.push_back( ( *e )->n0->pnt );
        badEdgeData.push_back( ( *e )->n1->pnt );
    }
    m_MeshBadEdgeDO.m_PntVec = badEdgeData;
    m_MeshBadEdgeDO.m_GeomChanged = true;
    // Normal Vec is not required, load placeholder.
    m_MeshBadEdgeDO.m_NormVec = badEdgeData;


    m_MeshBadTriDO.m_GeomID = GetID() + "BADTRI";
    m_MeshBadTriDO.m_Type = DrawObj::VSP_CFD_HIDDEN_TRIS;
    m_MeshBadTriDO.m_LineColor = vec3d( 1, 0, 0 );
    m_MeshBadTriDO.m_LineWidth = 3.0;

    m_MeshBadQuadDO.m_GeomID = GetID() + "BADQUAD";
    m_MeshBadQuadDO.m_Type = DrawObj::VSP_CFD_HIDDEN_QUADS;
    m_MeshBadQuadDO.m_LineColor = vec3d( 1, 0, 0 );
    m_MeshBadQuadDO.m_LineWidth = 3.0;

    vector< vec3d > badTriData;
    vector< vec3d > badQuadData;
    vector< Face* >::iterator f;
    for ( f = m_BadFaces.begin() ; f != m_BadFaces.end(); ++f )
    {
        if( ( *f )->IsTri() )
        {
            ( *f )->GetNodePts( badTriData );
        }
        else
        {
            ( *f )->GetNodePts( badQuadData );
        }
    }
    m_MeshBadTriDO.m_PntVec = badTriData;
    m_MeshBadQuadDO.m_PntVec = badQuadData;
    m_MeshBadTriDO.m_GeomChanged = true;
    m_MeshBadQuadDO.m_GeomChanged = true;
    // Normal Vec is not required, load placeholder.
    m_MeshBadTriDO.m_NormVec = badTriData;
    m_MeshBadQuadDO.m_NormVec = badQuadData;
}

void CfdMeshMgrSingleton::LoadDrawObjs( vector< DrawObj* > &draw_obj_vec )
{
    if ( m_MeshInProgress )
    {
        return;
    }

    SurfaceIntersectionSingleton::LoadDrawObjs( draw_obj_vec );

    GetGridDensityPtr()->Highlight( GetCurrSource() );
    GetGridDensityPtr()->Show( GetCfdSettingsPtr()->m_DrawSourceWakeFlag );
    GetGridDensityPtr()->LoadDrawObjs( draw_obj_vec );

    m_BBoxLineStripDO.m_Visible = GetCfdSettingsPtr()->m_DrawFarPreFlag && GetCfdSettingsPtr()->m_FarMeshFlag;
    draw_obj_vec.push_back( &m_BBoxLineStripDO );
    m_BBoxLinesDO.m_Visible = GetCfdSettingsPtr()->m_DrawFarPreFlag && GetCfdSettingsPtr()->m_FarMeshFlag;
    draw_obj_vec.push_back( &m_BBoxLinesDO );

    //Symmetry Splitting
    m_BBoxLineStripSymSplit.m_Visible = GetCfdSettingsPtr()->m_DrawFarPreFlag
                                        && GetCfdSettingsPtr()->m_FarMeshFlag
                                        && GetCfdSettingsPtr()->m_SymSplittingOnFlag
                                        && !GetCfdSettingsPtr()->m_FarCompFlag;
    draw_obj_vec.push_back( &m_BBoxLineStripSymSplit );
    m_BBoxLineSymSplit.m_Visible = m_BBoxLineStripSymSplit.m_Visible;
    draw_obj_vec.push_back( &m_BBoxLineSymSplit );

    unsigned int num_reason = vsp::NUM_MESH_REASON + 1;

    if ( m_ReasonDO.size() == 2 * num_reason )
    {
        for ( int i = 0; i < num_reason; i++ )
        {

            if ( GetCfdSettingsPtr()->m_ColorFacesFlag && GetCfdSettingsPtr()->m_ColorTagReason == vsp::REASON )
            {
                m_ReasonDO[i].m_Visible = true;
                m_ReasonDO[i + num_reason].m_Visible = true;

                if ( GetCfdSettingsPtr()->m_DrawMeshFlag &&
                     GetCfdSettingsPtr()->m_ColorFacesFlag ) // Both are visible.
                {
                    m_ReasonDO[i].m_Type = DrawObj::VSP_CFD_HIDDEN_TRIS;
                    m_ReasonDO[i].m_LineColor = vec3d( 0.4, 0.4, 0.4 );

                    m_ReasonDO[i + num_reason].m_Type = DrawObj::VSP_CFD_HIDDEN_QUADS;
                    m_ReasonDO[i + num_reason].m_LineColor = vec3d( 0.4, 0.4, 0.4 );
                }
                else
                {
                    m_ReasonDO[i].m_Type = DrawObj::VSP_SHADED_TRIS;
                    m_ReasonDO[i + num_reason].m_Type = DrawObj::VSP_SHADED_QUADS;
                }
            }
            else
            {
                // Need to set some m_Type so objects are created in vsp_graphic on first time through.
                m_ReasonDO[i].m_Type = DrawObj::VSP_CFD_HIDDEN_TRIS;
                m_ReasonDO[i].m_LineColor = vec3d( 0.4, 0.4, 0.4 );
                m_ReasonDO[i].m_Visible = false;

                m_ReasonDO[i + num_reason].m_Type = DrawObj::VSP_CFD_HIDDEN_QUADS;
                m_ReasonDO[i + num_reason].m_LineColor = vec3d( 0.4, 0.4, 0.4 );
                m_ReasonDO[i + num_reason].m_Visible = false;
            }

            if ( GetCfdSettingsPtr()->m_ColorFacesFlag && GetCfdSettingsPtr()->m_ColorTagReason == vsp::REASON  )
            {
                vec3d rgb = DrawObj::Color( DrawObj::reasonColorMap( i ) );

                for ( int icomp = 0; icomp < 3; icomp++ )
                {
                    m_ReasonDO[i].m_MaterialInfo.Ambient[icomp] = (float)rgb.v[icomp];
                    m_ReasonDO[i].m_MaterialInfo.Diffuse[icomp] = 0;
                    m_ReasonDO[i].m_MaterialInfo.Specular[icomp] = 0;
                    m_ReasonDO[i].m_MaterialInfo.Emission[icomp] = 0;

                    m_ReasonDO[i + num_reason].m_MaterialInfo.Ambient[icomp] = (float)rgb.v[icomp];
                    m_ReasonDO[i + num_reason].m_MaterialInfo.Diffuse[icomp] = 0;
                    m_ReasonDO[i + num_reason].m_MaterialInfo.Specular[icomp] = 0;
                    m_ReasonDO[i + num_reason].m_MaterialInfo.Emission[icomp] = 0;
                }
                m_ReasonDO[i].m_MaterialInfo.Ambient[3] = 1.0f;
                m_ReasonDO[i].m_MaterialInfo.Diffuse[3] = 1.0f;
                m_ReasonDO[i].m_MaterialInfo.Specular[3] = 1.0f;
                m_ReasonDO[i].m_MaterialInfo.Emission[3] = 1.0f;

                m_ReasonDO[i].m_MaterialInfo.Shininess = 0; // 32.0f;


                m_ReasonDO[i + num_reason].m_MaterialInfo.Ambient[3] = 1.0f;
                m_ReasonDO[i + num_reason].m_MaterialInfo.Diffuse[3] = 1.0f;
                m_ReasonDO[i + num_reason].m_MaterialInfo.Specular[3] = 1.0f;
                m_ReasonDO[i + num_reason].m_MaterialInfo.Emission[3] = 1.0f;

                m_ReasonDO[i + num_reason].m_MaterialInfo.Shininess = 0; // 32.0f;
            }
            else
            {
                for ( int icomp = 0; icomp < 4; icomp++ )
                {
                    m_ReasonDO[i].m_MaterialInfo.Ambient[icomp] = 1.0f;
                    m_ReasonDO[i].m_MaterialInfo.Diffuse[icomp] = 1.0f;
                    m_ReasonDO[i].m_MaterialInfo.Specular[icomp] = 1.0f;
                    m_ReasonDO[i].m_MaterialInfo.Emission[icomp] = 1.0f;

                    m_ReasonDO[i + num_reason].m_MaterialInfo.Ambient[icomp] = 1.0f;
                    m_ReasonDO[i + num_reason].m_MaterialInfo.Diffuse[icomp] = 1.0f;
                    m_ReasonDO[i + num_reason].m_MaterialInfo.Specular[icomp] = 1.0f;
                    m_ReasonDO[i + num_reason].m_MaterialInfo.Emission[icomp] = 1.0f;
                }
                m_ReasonDO[i].m_MaterialInfo.Shininess = 1.0f;
                m_ReasonDO[i + num_reason].m_MaterialInfo.Shininess = 1.0f;
            }


            draw_obj_vec.push_back( &m_ReasonDO[i] );
            draw_obj_vec.push_back( &m_ReasonDO[i + num_reason] );
        }
    }

    unsigned int num_tags = SubSurfaceMgr.GetNumTags();
    // Calculate constants for color sequence.
    const int ncgrp = 6; // Number of basic colors
    const int ncstep = (int)ceil((double)num_tags/(double)ncgrp);
    const double nctodeg = 360.0/(ncgrp*ncstep);

    if ( m_TagDO.size() == 2 * num_tags )
    {
        for ( int i = 0; i < num_tags; i++ )
        {

            if ( GetCfdSettingsPtr()->m_ColorFacesFlag && GetCfdSettingsPtr()->m_ColorTagReason == vsp::TAG )   // At least mesh or tags are visible.
            {
                m_TagDO[i].m_Visible = true;
                m_TagDO[i + num_tags].m_Visible = true;

                if ( GetCfdSettingsPtr()->m_DrawMeshFlag &&
                     GetCfdSettingsPtr()->m_ColorFacesFlag ) // Both are visible.
                {
                    m_TagDO[i].m_Type = DrawObj::VSP_CFD_HIDDEN_TRIS;
                    m_TagDO[i].m_LineColor = vec3d( 0.4, 0.4, 0.4 );

                    m_TagDO[i + num_tags].m_Type = DrawObj::VSP_CFD_HIDDEN_QUADS;
                    m_TagDO[i + num_tags].m_LineColor = vec3d( 0.4, 0.4, 0.4 );
                }
                else // Tags only
                {
                    m_TagDO[i].m_Type = DrawObj::VSP_SHADED_TRIS;
                    m_TagDO[i + num_tags].m_Type = DrawObj::VSP_SHADED_QUADS;
                }
            }
            else if ( GetCfdSettingsPtr()->m_DrawMeshFlag && !GetCfdSettingsPtr()->m_ColorFacesFlag )
            {
                m_TagDO[i].m_Visible = true;
                m_TagDO[i + num_tags].m_Visible = true;

                m_TagDO[i].m_Type = DrawObj::VSP_CFD_HIDDEN_TRIS;
                m_TagDO[i].m_LineColor = vec3d( 0.4, 0.4, 0.4 );

                m_TagDO[i + num_tags].m_Type = DrawObj::VSP_CFD_HIDDEN_QUADS;
                m_TagDO[i + num_tags].m_LineColor = vec3d( 0.4, 0.4, 0.4 );
            }
            else
            {
                // Need to set some m_Type so objects are created in vsp_graphic on first time through.
                m_TagDO[i].m_Type = DrawObj::VSP_CFD_HIDDEN_TRIS;
                m_TagDO[i].m_LineColor = vec3d( 0.4, 0.4, 0.4 );
                m_TagDO[i].m_Visible = false;

                m_TagDO[i + num_tags].m_Type = DrawObj::VSP_CFD_HIDDEN_QUADS;
                m_TagDO[i + num_tags].m_LineColor = vec3d( 0.4, 0.4, 0.4 );
                m_TagDO[i + num_tags].m_Visible = false;
            }

            if ( GetCfdSettingsPtr()->m_ColorFacesFlag && GetCfdSettingsPtr()->m_ColorTagReason == vsp::TAG )
            {
                // Color sequence -- go around color wheel ncstep times with slight
                // offset from ncgrp basic colors.
                // Note, (cnt/ncgrp) uses integer division resulting in floor.
                double deg = ((i % ncgrp) * ncstep + (i / ncgrp)) * nctodeg;
                vec3d rgb = m_TagDO[i].ColorWheel( deg );
                rgb.normalize();

                for ( int icomp = 0; icomp < 3; icomp++ )
                {
                    m_TagDO[i].m_MaterialInfo.Ambient[icomp] = (float)rgb.v[icomp]/5.0f;
                    m_TagDO[i].m_MaterialInfo.Diffuse[icomp] = 0.4f + (float)rgb.v[icomp]/10.0f;
                    m_TagDO[i].m_MaterialInfo.Specular[icomp] = 0.04f + 0.7f * (float)rgb.v[icomp];
                    m_TagDO[i].m_MaterialInfo.Emission[icomp] = (float)rgb.v[icomp]/20.0f;

                    m_TagDO[i + num_tags].m_MaterialInfo.Ambient[icomp] = (float)rgb.v[icomp]/5.0f;
                    m_TagDO[i + num_tags].m_MaterialInfo.Diffuse[icomp] = 0.4f + (float)rgb.v[icomp]/10.0f;
                    m_TagDO[i + num_tags].m_MaterialInfo.Specular[icomp] = 0.04f + 0.7f * (float)rgb.v[icomp];
                    m_TagDO[i + num_tags].m_MaterialInfo.Emission[icomp] = (float)rgb.v[icomp]/20.0f;
                }
                m_TagDO[i].m_MaterialInfo.Ambient[3] = 1.0f;
                m_TagDO[i].m_MaterialInfo.Diffuse[3] = 1.0f;
                m_TagDO[i].m_MaterialInfo.Specular[3] = 1.0f;
                m_TagDO[i].m_MaterialInfo.Emission[3] = 1.0f;

                m_TagDO[i].m_MaterialInfo.Shininess = 32.0f;


                m_TagDO[i + num_tags].m_MaterialInfo.Ambient[3] = 1.0f;
                m_TagDO[i + num_tags].m_MaterialInfo.Diffuse[3] = 1.0f;
                m_TagDO[i + num_tags].m_MaterialInfo.Specular[3] = 1.0f;
                m_TagDO[i + num_tags].m_MaterialInfo.Emission[3] = 1.0f;

                m_TagDO[i + num_tags].m_MaterialInfo.Shininess = 32.0f;
            }
            else
            {
                for ( int icomp = 0; icomp < 4; icomp++ )
                {
                    m_TagDO[i].m_MaterialInfo.Ambient[icomp] = 1.0f;
                    m_TagDO[i].m_MaterialInfo.Diffuse[icomp] = 1.0f;
                    m_TagDO[i].m_MaterialInfo.Specular[icomp] = 1.0f;
                    m_TagDO[i].m_MaterialInfo.Emission[icomp] = 1.0f;

                    m_TagDO[i + num_tags].m_MaterialInfo.Ambient[icomp] = 1.0f;
                    m_TagDO[i + num_tags].m_MaterialInfo.Diffuse[icomp] = 1.0f;
                    m_TagDO[i + num_tags].m_MaterialInfo.Specular[icomp] = 1.0f;
                    m_TagDO[i + num_tags].m_MaterialInfo.Emission[icomp] = 1.0f;
                }
                m_TagDO[i].m_MaterialInfo.Shininess = 1.0f;
                m_TagDO[i + num_tags].m_MaterialInfo.Shininess = 1.0f;
            }

            draw_obj_vec.push_back( &m_TagDO[i] );
            draw_obj_vec.push_back( &m_TagDO[i + num_tags] );
        }
    }

    unordered_map<int, DrawObj*> tag_tri_dobj_map;
    unordered_map<int, DrawObj*> tag_quad_dobj_map;
    map< std::vector<int>, int >::const_iterator mit;
    unordered_map< int, DrawObj* >::const_iterator dmit;
    map< std::vector<int>, int > tagMap = SubSurfaceMgr.GetSingleTagMap();
    int cnt = 0;

    for ( mit = tagMap.begin(); mit != tagMap.end() ; ++mit )
    {
        tag_tri_dobj_map[ mit->second ] = &m_TagDO[ cnt ];
        tag_quad_dobj_map[ mit->second ] = &m_TagDO[ cnt + num_tags ];
        cnt++;
    }


    for ( int i = 0 ; i < ( int )m_SurfVec.size() ; i++ )
    {
        DrawObj* obj;

        if ( m_SurfVec[ i ]->GetWakeFlag() )
        {
            vector < SimpFace > fVec = m_SurfVec[ i ]->GetMesh()->GetSimpFaceVec();
            if ( fVec.size() > 0 )
            {
                SimpFace *sface = &fVec[ 0 ];

                dmit = tag_tri_dobj_map.find( SubSurfaceMgr.GetTag( sface->m_Tags ) );
                obj = dmit->second;
                obj->m_Visible = GetCfdSettingsPtr()->m_DrawWakeFlag ;

                dmit = tag_quad_dobj_map.find( SubSurfaceMgr.GetTag( sface->m_Tags ) );
                obj = dmit->second;
                obj->m_Visible = GetCfdSettingsPtr()->m_DrawWakeFlag ;
            }
        }
        else if ( m_SurfVec[ i ]->GetFarFlag() )
        {
            vector < SimpFace > fVec = m_SurfVec[ i ]->GetMesh()->GetSimpFaceVec();
            if ( fVec.size() > 0 )
            {
                SimpFace *sface = &fVec[ 0 ];

                dmit = tag_tri_dobj_map.find( SubSurfaceMgr.GetTag( sface->m_Tags ) );
                obj = dmit->second;
                obj->m_Visible = GetCfdSettingsPtr()->m_DrawFarFlag ;

                dmit = tag_quad_dobj_map.find( SubSurfaceMgr.GetTag( sface->m_Tags ) );
                obj = dmit->second;
                obj->m_Visible = GetCfdSettingsPtr()->m_DrawFarFlag ;
            }
        }
        else if ( m_SurfVec[ i ]->GetSymPlaneFlag() )
        {
            vector < SimpFace > fVec = m_SurfVec[ i ]->GetMesh()->GetSimpFaceVec();
            if ( fVec.size() > 0 )
            {
                SimpFace *sface = &fVec[ 0 ];

                dmit = tag_tri_dobj_map.find( SubSurfaceMgr.GetTag( sface->m_Tags ) );
                obj = dmit->second;
                obj->m_Visible = GetCfdSettingsPtr()->m_DrawSymmFlag ;

                dmit = tag_quad_dobj_map.find( SubSurfaceMgr.GetTag( sface->m_Tags ) );
                obj = dmit->second;
                obj->m_Visible = GetCfdSettingsPtr()->m_DrawSymmFlag ;
            }
        }
    }

    m_MeshBadEdgeDO.m_Visible = GetCfdSettingsPtr()->m_DrawBadFlag;
    m_MeshBadTriDO.m_Visible = GetCfdSettingsPtr()->m_DrawBadFlag;
    m_MeshBadQuadDO.m_Visible = GetCfdSettingsPtr()->m_DrawBadFlag;

    draw_obj_vec.push_back( &m_MeshBadEdgeDO );
    draw_obj_vec.push_back( &m_MeshBadTriDO );
    draw_obj_vec.push_back( &m_MeshBadQuadDO );

    draw_obj_vec.push_back( &m_DegenCornerPointDO );
    draw_obj_vec.push_back( &m_DegenCornerEdgeDO );
}

void CfdMeshMgrSingleton::UpdateBBoxDO( const BndBox &box )
{
    vec3d temp = vec3d( box.GetMin( 0 ), box.GetMin( 1 ), box.GetMin( 2 ) );

    vector< vec3d > linestrip;

    linestrip.push_back( temp );
    temp[0] = box.GetMax( 0 );
    linestrip.push_back( temp );
    temp[1] = box.GetMax( 1 );
    linestrip.push_back( temp );
    temp[2] = box.GetMax( 2 );
    linestrip.push_back( temp );
    temp[0] = box.GetMin( 0 );
    linestrip.push_back( temp );
    temp[2] = box.GetMin( 2 );
    linestrip.push_back( temp );
    temp[1] = box.GetMin( 1 );
    linestrip.push_back( temp );
    temp[2] = box.GetMax( 2 );
    linestrip.push_back( temp );
    temp[0] = box.GetMax( 0 );
    linestrip.push_back( temp );
    temp[2] = box.GetMin( 2 );
    linestrip.push_back( temp );

    m_BBoxLineStripDO.m_GeomID = GetID() + "BBOXLS";
    m_BBoxLineStripDO.m_Type = DrawObj::VSP_LINE_STRIP;
    m_BBoxLineStripDO.m_LineWidth = 1.0;
    m_BBoxLineStripDO.m_LineColor = vec3d( 0, 200.0 / 255, 0 );
    m_BBoxLineStripDO.m_PntVec = linestrip;
    m_BBoxLineStripDO.m_GeomChanged = true;

    vector< vec3d > lines;

    temp[2] = box.GetMax( 2 );
    lines.push_back( temp );
    temp[1] = box.GetMax( 1 );
    lines.push_back( temp );
    temp[2] = box.GetMin( 2 );
    lines.push_back( temp );
    temp[0] = box.GetMin( 0 );
    lines.push_back( temp );
    temp[2] = box.GetMax( 2 );
    lines.push_back( temp );
    temp[1] = box.GetMin( 1 );
    lines.push_back( temp );

    m_BBoxLinesDO.m_GeomID = GetID() + "BBOXL";
    m_BBoxLinesDO.m_Type = DrawObj::VSP_LINES;
    m_BBoxLinesDO.m_LineWidth = 1.0;
    m_BBoxLinesDO.m_LineColor = vec3d( 0, 200.0 / 255, 0 );
    m_BBoxLinesDO.m_PntVec = lines;
    m_BBoxLinesDO.m_GeomChanged = true;

    //===== Symmetry Plane Splitting Lines =====//
    if ( GetCfdSettingsPtr()->m_SymSplittingOnFlag )
    {
        UpdateBBoxDOSymSplit( box );
    }
}

void CfdMeshMgrSingleton::UpdateBBoxDOSymSplit( const BndBox &box )
{
    vec3d temp = vec3d( box.GetMin( 0 ), box.GetMin( 1 ), box.GetMin( 2 ) );
    vector< vec3d > symLinestrip, symLine;
    vec3d outerBottomRight, outerBottomLeft, outerTopRight, outerTopLeft;
    vec3d innerBottomRight, innerBottomLeft, innerTopRight, innerTopLeft;
    vector<vec3d> corners;
    BndBox vehicleBBox = m_Vehicle->GetBndBox();
    double min = GetCfdSettingsPtr()->m_FarXScale;
    double scale = 2.0;

    //Grab the minimum of the two scales
    if (GetCfdSettingsPtr()->m_FarZScale < min)
    {
        min = GetCfdSettingsPtr()->m_FarZScale;
    }

    //Makes center plane smaller if outer plane smaller than inner
    //Currently min can be set to 1.0 (the size of the BBox)
    if (min < scale)
    {
        scale = (min + 1.0)/2.0;
    }

    vehicleBBox.Scale(vec3d(scale, scale, scale));
    corners = vehicleBBox.GetCornerPnts();
    vehicleBBox.Reset();

    //Symmetry Plane Vertices
    outerBottomRight = m_Domain.GetCornerPnt(1);
    outerBottomLeft = m_Domain.GetCornerPnt(0);
    outerTopRight = m_Domain.GetCornerPnt(5);
    outerTopLeft = m_Domain.GetCornerPnt(4);
    innerBottomRight = vec3d(corners[2][0], outerBottomRight[1], corners[2][2]);
    innerBottomLeft = vec3d(corners[1][0], outerBottomLeft[1], corners[1][2]);
    innerTopRight = vec3d(corners[6][0], outerTopRight[1], corners[6][2]);
    innerTopLeft = vec3d(corners[5][0], outerTopLeft[1], corners[5][2]);

    //Checks to see if inner plane is outside of outer plane
    if (!((innerBottomLeft.x() < outerBottomLeft.x() || innerBottomRight.x() > outerBottomRight.x()
         || innerTopLeft.z() > outerTopLeft.z() ||  innerBottomLeft.z() < outerBottomLeft.z())
        && GetCfdSettingsPtr()->m_FarManLocFlag))
    {
        //=== Symmetry Plane InnerBox as 'line strips' ===//
        temp = innerBottomLeft;
        symLinestrip.push_back( temp );
        temp = innerTopLeft;
        symLinestrip.push_back( temp );
        temp = innerTopRight;
        symLinestrip.push_back( temp );
        temp = innerBottomRight;
        symLinestrip.push_back( temp );
        temp = innerBottomLeft;
        symLinestrip.push_back( temp );

        //=== Symmetry Plane Lines from outer to inner as 'lines' ===//
        temp = outerTopLeft;
        symLine.push_back( temp );
        temp = innerTopLeft;
        symLine.push_back( temp );

        temp = outerBottomLeft;
        symLine.push_back( temp );
        temp = innerBottomLeft;
        symLine.push_back( temp );

        temp = outerTopRight;
        symLine.push_back( temp );
        temp = innerTopRight;
        symLine.push_back( temp );

        temp = outerBottomRight;
        symLine.push_back( temp );
        temp = innerBottomRight;
        symLine.push_back( temp );
    }

    m_BBoxLineStripSymSplit.m_GeomID = GetID() + "BBOXLS1";
    m_BBoxLineStripSymSplit.m_Type = DrawObj::VSP_LINE_STRIP;
    m_BBoxLineStripSymSplit.m_LineWidth = 1.0;
    m_BBoxLineStripSymSplit.m_LineColor = vec3d(0, 200.0 / 255, 0);
    m_BBoxLineStripSymSplit.m_PntVec = symLinestrip;
    m_BBoxLineStripSymSplit.m_GeomChanged = true;

    m_BBoxLineSymSplit.m_GeomID = GetID() + "BBOXL1";
    m_BBoxLineSymSplit.m_Type = DrawObj::VSP_LINES;
    m_BBoxLineSymSplit.m_LineWidth = 1.0;
    m_BBoxLineSymSplit.m_LineColor = vec3d(0, 200.0 / 255, 0);
    m_BBoxLineSymSplit.m_PntVec = symLine;
    m_BBoxLineSymSplit.m_GeomChanged = true;
}

void CfdMeshMgrSingleton::SubTagTris()
{
    SubSurfaceMgr.ClearTagMaps();
    unordered_map< string, int > tag_map;
    unordered_map< string, set<int> > geom_comp_map;
    unordered_map< int, int >  comp_num_map; // map from an unmerged component number to the surface number of geom
    int tag_number = 0;
    int fea_part_cnt = 1;

    for ( int i = 0; i < (int)m_SurfVec.size(); i++ )
    {
        Surf* surf = m_SurfVec[i];
        string geom_id = surf->GetGeomID();
        string id = geom_id + to_string( (long long) surf->GetUnmergedCompID() );
        string name;
        string exportid;

        geom_comp_map[geom_id].insert( surf->GetUnmergedCompID() );

        if ( !surf->GetWakeFlag() && !surf->GetSymPlaneFlag() && !surf->GetFarFlag() )
            comp_num_map[ surf->GetUnmergedCompID() ] = geom_comp_map[geom_id].size();

        if ( surf->GetWakeFlag() )
        {
            id += "_Wake";
        }
        else if ( surf->GetSymPlaneFlag() )
        {
            id = "SymPlane";
            name = "SymPlane";
        }
        else if ( surf->GetFarFlag() )
        {
            id = "FarField";
            name = "FarField";
        }

        if ( tag_map.find(id) == tag_map.end() )
        {
            tag_number++;
            tag_map[id] = tag_number;

            Geom* geom_ptr = m_Vehicle->FindGeom( geom_id );
            if ( surf->GetWakeFlag() )
                geom_ptr = m_Vehicle->FindGeom( surf->GetRefGeomID() );

            // Will need to augment name and exportid with _C _V _H markers as from MeshGeom::GetTMeshIDs and GetTMeshNames.

            if ( surf->GetCompID() < 0 )
            {
                name = geom_ptr->GetName() + "_FeaPart_" + to_string( fea_part_cnt );
                fea_part_cnt++;
            }
            else if ( geom_ptr )
            {
                string nplate;
                string idplate;
                if ( surf->GetSurfacePlateNum() == -1 )
                {
                    nplate = "_S";
                }
                else
                {
                    if ( surf->GetSurfaceVSPType() == vsp::NORMAL_SURF )
                    {
                        if ( surf->GetSurfacePlateNum() == 0 )
                        {
                            nplate = "_V";
                            idplate = "_V";
                        }
                        else if ( surf->GetSurfacePlateNum() == 1 )
                        {
                            nplate = "_H";
                            idplate = "_H";
                        }
                    }
                    else // WING_SURF with m_PlateNum == 0
                    {
                        nplate = "_C";
                        idplate = "_C";
                    }

                }

                name = geom_ptr->GetName() + nplate + "_Surf" + to_string((long long)geom_comp_map[geom_id].size() - 1 );
                exportid = geom_id + idplate + "_Surf" + to_string( (long long)geom_comp_map[geom_id].size() - 1 );
                if ( surf->GetWakeFlag() ) name = geom_ptr->GetName()
                                                 + to_string( (long long)comp_num_map[ surf->GetUnmergedCompID() ] )
                                                 + "_Wake";
            }

            SubSurfaceMgr.m_CompNames.push_back(name);
            SubSurfaceMgr.m_CompIDs.push_back(exportid);
            SubSurfaceMgr.m_CompTypes.push_back( surf->GetSurfaceVSPType() );

            // Set to thin.
            bool thick = false;
            if ( surf->GetSurfaceCfdType() == vsp::CFD_NORMAL || surf->GetSurfaceCfdType() == vsp::CFD_NEGATIVE )
            {
                thick = true;
            }
            SubSurfaceMgr.m_ThickMap[ geom_id ] = thick;
        }

        surf->SetBaseTag( tag_map[id] );

    }

    SetSimpSubSurfTags( tag_number );
    SubSurfaceMgr.BuildCompNameMap();
    SubSurfaceMgr.BuildCompIDMap();
}

void CfdMeshMgrSingleton::SetSimpSubSurfTags( int tag_offset )
{
    // tag_offset should be the number of total components in mesh

    for ( int i = 0; i < (int)m_SimpleSubSurfaceVec.size(); i++ )
    {
        m_SimpleSubSurfaceVec[i].m_Tag = tag_offset + i + 1;
        // map tag number to surface name
        SubSurfaceMgr.m_TagNames[m_SimpleSubSurfaceVec[i].m_Tag] = m_SimpleSubSurfaceVec[i].GetName();
        SubSurfaceMgr.m_TagIDs[m_SimpleSubSurfaceVec[i].m_Tag] = m_SimpleSubSurfaceVec[i].GetSSID();
    }
}

void CfdMeshMgrSingleton::Subtag( Surf* surf )
{
    vector< SimpFace >& face_vec = surf->GetMesh()->GetSimpFaceVec();
    const vector< vec2d >& pnts = surf->GetMesh()->GetSimpUWPntVec();
    vector< SimpleSubSurface > simp_s_surfs = GetSimpSubSurfs( surf->GetGeomID(), surf->GetMainSurfID() , surf->GetCompID() );

    for ( int f = 0; f < (int)face_vec.size(); f++ )
    {
        SimpFace& face = face_vec[f];
        face.m_Tags.push_back( surf->GetBaseTag() );
        vec2d center;
        if ( face.m_isQuad )
        {
            center = ( pnts[face.ind0] + pnts[face.ind1] + pnts[face.ind2] + pnts[face.ind3] ) * 1 / 4.0;
        }
        else
        {
            center = ( pnts[face.ind0] + pnts[face.ind1] + pnts[face.ind2] ) * 1 / 3.0;
        }

        surf->InterpTargetMap( center.x(), center.y(), face.m_reason );

        for ( int s = 0; s < (int)simp_s_surfs.size(); s++ )
        {
            if ( simp_s_surfs[s].Subtag( vec3d( center.x(), center.y(), 0 ) ) && surf->GetCompID() >= 0 )
            {
                face.m_Tags.push_back( simp_s_surfs[s].m_Tag );
            }
        }
        SubSurfaceMgr.m_TagCombos.insert( face.m_Tags );
    }
}

void CfdMeshMgrSingleton::FindDegenCorners()
{
    double tol = 1e-8;

    m_DegenCorners.clear();

    list< ISegChain* >::iterator c, d;
    for ( c = m_ISegChainList.begin() ; c != m_ISegChainList.end(); ++c )
    {
        if ( ( *c )->m_BorderFlag )
        {
            d = c;
            for ( ++d; d != m_ISegChainList.end(); ++d )
            {
                if ( ( *d )->m_BorderFlag )
                {
                    if (((( *c )->m_SurfA == ( *d )->m_SurfA ) || (( *c )->m_SurfA == ( *d )->m_SurfB )) &&
                        ((( *c )->m_SurfB == ( *d )->m_SurfA ) || (( *c )->m_SurfB == ( *d )->m_SurfB )))
                    {
                        ISeg* frontSegC = ( *c )->m_ISegDeque.front();
                        frontSegC->m_IPnt[0]->CompPnt();
                        vec3d pC0 = frontSegC->m_IPnt[0]->m_Pnt;

                        ISeg* frontSegD = ( *d )->m_ISegDeque.front();
                        frontSegD->m_IPnt[0]->CompPnt();
                        vec3d pD0 = frontSegD->m_IPnt[0]->m_Pnt;

                        ISeg* backSegC = ( *c )->m_ISegDeque.back();
                        backSegC->m_IPnt[1]->CompPnt();
                        vec3d pC1 = backSegC->m_IPnt[1]->m_Pnt;

                        ISeg* backSegD = ( *d )->m_ISegDeque.back();
                        backSegD->m_IPnt[1]->CompPnt();
                        vec3d pD1 = backSegD->m_IPnt[1]->m_Pnt;

                        int cornerC = -1;
                        int cornerD = -1;

                        if( dist_squared( pC0, pD0 ) < tol )
                        {
                            cornerC = 0;
                            cornerD = 0;

                            m_DegenCorners.push_back( frontSegC->m_IPnt[0] );
                        }
                        if( dist_squared( pC1, pD1 ) < tol )
                        {
                            cornerC = 1;
                            cornerD = 1;

                            m_DegenCorners.push_back( backSegC->m_IPnt[1] );
                        }
                        if( dist_squared( pC0, pD1 ) < tol )
                        {
                            cornerC = 0;
                            cornerD = 1;

                            m_DegenCorners.push_back( frontSegC->m_IPnt[0] );
                        }
                        if( dist_squared( pC1, pD0 ) < tol )
                        {
                            cornerC = 1;
                            cornerD = 0;

                            m_DegenCorners.push_back( backSegC->m_IPnt[1] );
                        }
                    }
                }
            }
        }
    }
}

void CfdMeshMgrSingleton::AddDegenCornerChains()
{
    double degen_tol = 1.0e-6;

    for ( int i = 0; i < m_DegenCorners.size(); i++ )
    {
        IPnt* corner = m_DegenCorners[i];

        for ( int j = 0; j < corner->m_Puws.size(); j++ )
        {
            Puw* puw_corner = corner->m_Puws[j];

            Surf* s = puw_corner->m_Surf;
            vec2d uw0 = puw_corner->m_UW;
            double u0 = uw0.x();
            double w0 = uw0.y();

            vector < vec3d > pts;
            pts.push_back( vec3d( u0, w0, 0.0 ) );

            // Build a vector of corner points, each double the distance from the corner as the prior one.
            // Starting at .001 * minlen, we will end at 65 * minlen.
            double l = GetGridDensityPtr()->m_MinLen * 0.001;
            s->GetSurfCore()->FindCornerPtVec( pts, u0, w0, l );


            vec3d uw1_3d = pts.back();
            vec2d uw1 = vec2d( uw1_3d.x(), uw1_3d.y() );

            Puw* puwstart0 = new Puw( s, uw0 );
            Puw* puwstart1 = new Puw( s, uw0 );

            Puw* puwend0 = new Puw( s, uw1 );
            Puw* puwend1 = new Puw( s, uw1 );


            m_DelPuwVec.push_back( puwstart0 );                 // Save to delete later
            m_DelPuwVec.push_back( puwstart1 );
            m_DelPuwVec.push_back( puwend0 );
            m_DelPuwVec.push_back( puwend1 );

            IPnt* p0 = new IPnt( puwstart0, puwstart1 );
            IPnt* p1 = new IPnt( puwend0, puwend1 );

            m_DelIPntVec.push_back( p0 );           // Save to delete later
            m_DelIPntVec.push_back( p1 );

            p0->CompPnt();
            p1->CompPnt();

            ISeg* seg1 = new ISeg( s, s, p0, p1 );

            ISegChain* chain = new ISegChain;           // Create New Chain
            chain->m_SurfA = s;
            chain->m_SurfB = s;
            chain->m_ISegDeque.push_back( seg1 );

            chain->m_BorderFlag = false;

            chain->m_ACurve.InterpolateLinear( pts );
            chain->m_ACurve.SetSurf( s );

            ICurve* icrv = new ICurve;
            icrv->SetACurve( &(chain->m_ACurve) );
            chain->m_ACurve.SetICurve( icrv );

            chain->m_BCurve = chain->m_ACurve;
            icrv->SetBCurve( &(chain->m_BCurve) );
            m_ICurveVec.push_back( icrv );

            if ( chain->Valid() && chain->m_ACurve.Length( 10 ) > degen_tol )
            {
                m_ISegChainList.push_back( chain );
                m_DegenCornerChains.push_back( chain );
            }
            else
            {
                delete chain;
                chain = nullptr;
            }
        }
    }
}

void CfdMeshMgrSingleton::UpdateDisplaySettings()
{
    if ( GetCfdSettingsPtr() )
    {
        GetCfdSettingsPtr()->m_DrawSourceWakeFlag = m_Vehicle->GetCfdSettingsPtr()->m_DrawSourceWakeFlag.Get();
        GetCfdSettingsPtr()->m_DrawFarPreFlag = m_Vehicle->GetCfdSettingsPtr()->m_DrawFarPreFlag.Get();
        GetCfdSettingsPtr()->m_DrawMeshFlag = m_Vehicle->GetCfdSettingsPtr()->m_DrawMeshFlag.Get();
        GetCfdSettingsPtr()->m_DrawWakeFlag = m_Vehicle->GetCfdSettingsPtr()->m_DrawWakeFlag.Get();
        GetCfdSettingsPtr()->m_DrawSymmFlag = m_Vehicle->GetCfdSettingsPtr()->m_DrawSymmFlag.Get();
        GetCfdSettingsPtr()->m_DrawFarFlag = m_Vehicle->GetCfdSettingsPtr()->m_DrawFarFlag.Get();
        GetCfdSettingsPtr()->m_DrawBadFlag = m_Vehicle->GetCfdSettingsPtr()->m_DrawBadFlag.Get();
        GetCfdSettingsPtr()->m_ColorFacesFlag = m_Vehicle->GetCfdSettingsPtr()->m_ColorFacesFlag.Get();
        GetCfdSettingsPtr()->m_ColorTagReason = m_Vehicle->GetCfdSettingsPtr()->m_ColorTagReason.Get();

        GetCfdSettingsPtr()->m_DrawBorderFlag = m_Vehicle->GetCfdSettingsPtr()->m_DrawBorderFlag.Get();
        GetCfdSettingsPtr()->m_DrawIsectFlag = m_Vehicle->GetCfdSettingsPtr()->m_DrawIsectFlag.Get();
        GetCfdSettingsPtr()->m_DrawRawFlag = m_Vehicle->GetCfdSettingsPtr()->m_DrawRawFlag.Get();
        GetCfdSettingsPtr()->m_DrawBinAdaptFlag = m_Vehicle->GetCfdSettingsPtr()->m_DrawBinAdaptFlag.Get();
        GetCfdSettingsPtr()->m_DrawCurveFlag = m_Vehicle->GetCfdSettingsPtr()->m_DrawCurveFlag.Get();
        GetCfdSettingsPtr()->m_DrawPntsFlag = m_Vehicle->GetCfdSettingsPtr()->m_DrawPntsFlag.Get();
    }
}

void CfdMeshMgrSingleton::RegisterAnalysis()
{
    if (!AnalysisMgr.FindAnalysis( "CfdMeshAnalysis" ))
    {
        CfdMeshAnalysis* cfda = new CfdMeshAnalysis();

        if ( cfda && !AnalysisMgr.RegisterAnalysis( cfda ) )
        {
            delete cfda;
        }
    }
}

