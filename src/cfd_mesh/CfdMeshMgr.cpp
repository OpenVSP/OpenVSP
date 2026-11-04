//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// CfdMeshMgr
//
//////////////////////////////////////////////////////////////////////

#include "CfdMeshMgr.h"
#include "UnformattedFile.h"
#include "ResultsMgr.h"
#include "SubSurfaceMgr.h"
#include "main.h"
#include "MeshAnalysis.h"
#include "ModeMgr.h"
#include "FileUtil.h"
#include "HingeGeom.h"
#include "StlHelper.h"
#include "StringUtil.h"
#include "FastFormat.h"
#include "LocaleUtil.h"

#include <algorithm>
#include <tuple>
#include <functional>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <deque>
#include <unordered_set>

#include "StringUtil.h"



// Set true to gather, and report after every mesh, how far the edges came out from the
// length they were sized to.  Off, none of it is gathered.
static const bool TARGET_LENGTH_REPORT = false;

// Three numbers as printf's "%<width>.<prec>g %<width>.<prec>g %<width>.<prec>g\n" writes them
static void AppendGTriple( string &out, double a, double b, double c, int width, int prec )
{
    FastFormat::AppendG( out, a, width, prec );
    out += ' ';
    FastFormat::AppendG( out, b, width, prec );
    out += ' ';
    FastFormat::AppendG( out, c, width, prec );
    out += '\n';
}

// Three numbers as printf's "%<width>.<prec>f %<width>.<prec>f %<width>.<prec>f\n" writes them
static void AppendFTriple( string &out, double a, double b, double c, int width, int prec )
{
    FastFormat::AppendF( out, a, width, prec );
    out += ' ';
    FastFormat::AppendF( out, b, width, prec );
    out += ' ';
    FastFormat::AppendF( out, c, width, prec );
    out += '\n';
}

// A face's parameter pairs as printf's " %16.10g %16.10g" writes each one, the corners taken in
// the order given and each pair scaled as the VSPGEOM file wants it
static void AppendUWCorners( string &out, const vec2d* uw, const int* corner, int n, double uscale, double wscale )
{
    for ( int i = 0 ; i < n ; i++ )
    {
        out += ' ';
        FastFormat::AppendG( out, uw[ corner[i] ].x() / uscale, 16, 10 );
        out += ' ';
        FastFormat::AppendG( out, uw[ corner[i] ].y() / wscale, 16, 10 );
    }
}

// Integers as printf's "%d %d ... %d" writes them, each followed by sep
static void AppendInts( string &out, const int* v, int n, char sep )
{
    for ( int i = 0 ; i < n ; i++ )
    {
        FastFormat::AppendInt( out, v[i] );
        out += sep;
    }
}

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

    // The debug log belongs to the base class, which closes it.
}

void CfdMeshMgrSingleton::GenerateMesh()
{
    addOutputText( "CLEAR_TERMINAL" );

    m_MeshInProgress = true;

#ifdef DEBUG_TIME_OUTPUT
    addOutputText( "Init Timer\n" );
#endif

    // What this run produced, reported at the end.  Cleared here so a run that stops early
    // cannot be read as the one before it.
    m_NumMeshTris = 0;
    m_NumBorderEdges = 0;
    m_NumOverConnEdges = 0;
    m_NumTopoSurfs = 0;

    addOutputText( "Transfer Mesh Settings\n" );
    TransferMeshSettings();

    addOutputText( "Fetching Bezier Surfaces\n" );

    vector< XferSurf > xfersurfs;

    // The POGS surface files sample along each Geom's own tessellation lines, refined by
    // however much the user asked for.  Read the copy TransferMeshSettings just took, not
    // the Vehicle's live Parms, so a setting changed while the mesher runs cannot land
    // halfway through.
    // The engines' own caps are cut apart for the CFD mesh alone.
    FetchSurfs( xfersurfs, GetCfdSettingsPtr()->m_POGSNRef, true );

    // UpdateSourcesAndWakes must be before m_Vehicle->HideAll() to prevent components 
    // being being added to or removed from the CFD Mesh set
    addOutputText( "Update Sources And Wakes\n" );
    UpdateSourcesAndWakes();
    WakeMgr.SetStretchMeshFlag( true );

    // Hide all geoms after fetching their surfaces
    m_Vehicle->HideAll();

    addOutputText( "Cleanup\n" );
    CleanUp();

    LoadSurfs( xfersurfs );

    if ( GetSettingsPtr()->m_IntersectSubSurfs )
    {
        addOutputText( "Transfer Subsurface Data\n" );
        TransferSubSurfData();
    }

    addOutputText( "Clean Merge Surfaces\n" );
    CleanMergeSurfs( /* skip_duplicate_removal */ false );

    // Only the CFD mesh pairs the faces of positive and negative bodies that lie one on the other.
    PairCoincidentFaces();

    if ( m_SurfVec.size() == 0 )
    {
        // Nothing was selected.  Worth saying out loud: the meshing set defaults to the
        // shown set and a run hides every Geom when it has taken their surfaces, so a
        // second run in a row finds nothing and would otherwise write empty files and
        // report success.  addOutputText reaches the screen only.
        addOutputText( "No Surfaces To Mesh\n" );
        printf( "No Surfaces To Mesh\n" );
        RecordResults();
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

    RecordIntCurves();

    addOutputText( "Build Target Map\n" );
    BuildTargetMap( CfdMeshMgrSingleton::VOCAL_OUTPUT );

    // addOutputText( "InitMesh\n" ); // Output inside InitMesh
    InitMesh( );

    addOutputText( "Sub Tag tris\n" );
    SubTagTris();

    Remesh( CfdMeshMgrSingleton::VOCAL_OUTPUT );

    if ( GetSettingsPtr()->m_ConvertToQuadsFlag )
    {
        addOutputText( "ConvertToQuads\n" );
        ConvertToQuads();
    }

    ConnectBorderNodes( false );        // No Wakes
    ConnectBorderNodes( true );         // Only Wakes

    PostMesh();

    //addOutputText( "Triangle Quality\n");
    //string qual = CfdMeshMgr.GetQualString();
    //addOutputText( qual.c_str() );

    addOutputText( "Build Single Tag Map\n" );
    SubSurfaceMgr.BuildSingleTagMap();

    ExportFiles();

    string resultTxt = CheckWaterTight();
    addOutputText( resultTxt );

    ClearExportMerge();

    RecordResults();

    if ( TARGET_LENGTH_REPORT )
    {
        string lenTxt = TargetLengthReport();
        addOutputText( lenTxt );
    }

    m_DrawObjsStale = true;

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

    ClearExportMerge();

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
    m_QualityDO.clear();
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
    // The screen updates on every change anywhere, a rotation of the view among them, and the
    // sources and wakes only need building again when what they are built from has changed
    if ( !GetMeshInProgress() )
    {
        string state = SourcesAndWakesState();
        if ( state != m_SourcesAndWakesState )
        {
            m_SourcesAndWakesState = state;
            UpdateSourcesAndWakes();
        }
    }

    UpdateDomain();

    m_Vehicle->GetCfdGridDensityPtr()->Update( 1.0 );
}

string CfdMeshMgrSingleton::SourcesAndWakesState()
{
    string state = to_string( GetCfdSettingsPtr()->m_SelectedSetIndex ) + " " +
                   to_string( GetCfdSettingsPtr()->m_SelectedDegenSetIndex ) + " " +
                   to_string( ( int )GetCfdSettingsPtr()->m_UseMode ) + " " +
                   GetCfdSettingsPtr()->m_ModeID;

    if ( GetCfdSettingsPtr()->m_UseMode )
    {
        Mode *m = ModeMgr.GetMode( GetCfdSettingsPtr()->m_ModeID );
        if ( m )
        {
            state += " " + to_string( m->m_NormalSet() ) + " " + to_string( m->m_DegenSet() );
        }
    }

    vector<string> geomVec = m_Vehicle->GetGeomVec();
    for ( int g = 0 ; g < ( int )geomVec.size() ; g++ )
    {
        Geom* geom = m_Vehicle->FindGeom( geomVec[g] );
        if ( geom )
        {
            state += " " + geomVec[g] + " " + to_string( geom->GetUpdateStamp() );

            vector< BaseSource* > sVec = geom->GetCfdMeshMainSourceVec();
            for ( int s = 0 ; s < ( int )sVec.size() ; s++ )
            {
                state += " " + sVec[s]->GetID() + " " + to_string( sVec[s]->GetLatestChangeCnt() );
            }
        }
    }
    return state;
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
    vector< MapSource* > allsources;

    int i;

    // Each surface's map is built and limited from that surface alone, so surfaces run side by
    // side.  The one shared thing evaluated is a constant U or W line source's Geom surface, and
    // that source locks around it.
    int nsurf = ( int )m_SurfVec.size();

    vector < vector < MapSource* > > surfsources( nsurf );

    // Biggest maps first, so the run does not end on one large surface with every other thread
    // idle.  Surfaces are independent, so the order they are taken in changes none of them.
    vector < pair < int, int > > order( nsurf );

    for ( i = 0 ; i < nsurf ; i++ )
    {
        order[i] = pair< int, int >( m_SurfVec[i]->GetTargetMapSize(), i );
    }

    sort( order.begin(), order.end(), std::greater< pair < int, int > >() );

    BeginProgress( "Surface Maps", nsurf, output_type );

    RunIndexed( nsurf, StageThreadCount( nsurf ), [&]( int k )
    {
        int isurf = order[k].second;

        m_SurfVec[isurf]->BuildTargetMap( surfsources[isurf], isurf );
        m_SurfVec[isurf]->LimitTargetMap();

        m_ProgressDone++;
        StepProgress( output_type );
    } );

    EndProgress( output_type );

    // Only rigorous limiting looks at every surface's map points together.  Each surface's list is
    // let go once copied, so the two are not both held in full.
    if ( GetGridDensityPtr()->m_RigorLimit )
    {
        size_t ntotal = 0;
        for ( i = 0 ; i < nsurf ; i++ )
        {
            ntotal += surfsources[i].size();
        }
        allsources.reserve( ntotal );

        for ( i = 0 ; i < nsurf ; i++ )
        {
            allsources.insert( allsources.end(), surfsources[i].begin(), surfsources[i].end() );
            vector< MapSource* >().swap( surfsources[i] );
        }
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

    SplitSourceSet splitSet( splitSources );

    BuildChainDistTables();

    // Number of times to propagate intersection edges through surfaces
    int nedgeprop = 4;

    int nthread = StageThreadCount( nsurf );

    // A chain is counted as it is taken up, and the bar redrawn some two hundred times in all.
    int nspread = nedgeprop * ( int )m_ISegChainList.size();
    int stride = std::max( 1, nspread / 200 );

    BeginProgress( "Chain Density", nspread, output_type );

    if ( nthread > 1 )
    {
        SpreadChainDensity( nedgeprop, nthread, splitSet, stride, output_type );
    }
    else
    {
        for ( i = 0; i < nedgeprop; i ++ )
        {
            for ( c = m_ISegChainList.begin() ; c != m_ISegChainList.end(); ++c )
            {
                ( *c )->CalcDensity( GetGridDensityPtr(), splitSet );
                ( *c )->SpreadDensity();

                StepProgressEvery( stride, output_type );
            }
        }
    }

    EndProgress( output_type );

    if( GetGridDensityPtr()->m_RigorLimit )
    {
        if ( output_type != CfdMeshMgrSingleton::QUIET_OUTPUT )
        {
            addOutputText( " Rigorous 3D Limiting\n", output_type );
        }

        BeginProgress( "Rigorous Limit", ( int )m_SurfVec.size(), output_type );

        // One tree over every surface's sources.  Each surface leaves out its own at search time
        // and is limited by the others as they stand, so its limits carry to the surfaces after it.
        MSBoundTree ms_tree;
        ms_tree.Build( allsources, nthread );

        for ( i = 0 ; i < nsurf ; i++ )
        {
            double minmap;
            if ( ms_tree.MinStrength( i, minmap ) )
            {
                // The limits on a surface's points are found side by side, then taken up in order.
                int nrow = m_SurfVec[i]->StartTargetMapLimits();
                RunIndexed( nrow, StageThreadCount( nrow ), [&]( int irow )
                {
                    m_SurfVec[i]->FindTargetMapLimits( ms_tree, minmap, i, irow );
                } );

                m_SurfVec[i]->LimitTargetMap( ms_tree, minmap, i );
                ms_tree.UpdateSurface( i );
            }

            m_ProgressDone++;
            StepProgress( output_type );
        }

        EndProgress( output_type );

        for ( c = m_ISegChainList.begin() ; c != m_ISegChainList.end(); ++c )
        {
            ( *c )->CalcDensity( GetGridDensityPtr(), splitSet );
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

// Runs work on worker threads, one queue per surface.  A surface's work is done in the order it
// was added, one piece at a time; work on different surfaces runs side by side.
class SurfWorkQueue
{
public:

    SurfWorkQueue( int nsurf, int nworker ) : m_Queue( nsurf ), m_Busy( nsurf, false ), m_Stop( false )
    {
        for ( int t = 0 ; t < nworker ; t++ )
        {
            m_Pool.push_back( std::thread( [this]() { Work(); } ) );
        }
    }

    ~SurfWorkQueue()
    {
        {
            std::lock_guard< std::mutex > lock( m_Mutex );
            m_Stop = true;
        }
        m_WorkCV.notify_all();

        for ( int t = 0 ; t < ( int )m_Pool.size() ; t++ )
        {
            m_Pool[t].join();
        }
    }

    void Add( int isurf, const std::function< void() > &f )
    {
        std::lock_guard< std::mutex > lock( m_Mutex );

        m_Queue[ isurf ].push_back( f );

        // A surface already waiting or being worked on is picked up again when its work is done.
        if ( !m_Busy[ isurf ] && m_Queue[ isurf ].size() == 1 )
        {
            m_Ready.push_back( isurf );
            m_WorkCV.notify_one();
        }
    }

    // Wait until nothing is queued or running on surface isurf.
    void WaitSurf( int isurf )
    {
        std::unique_lock< std::mutex > lock( m_Mutex );
        m_DoneCV.wait( lock, [&]() { return m_Queue[ isurf ].empty() && !m_Busy[ isurf ]; } );
    }

    // Wait until every surface is idle, then hand on the first exception any work threw.
    void WaitAll()
    {
        std::unique_lock< std::mutex > lock( m_Mutex );
        m_DoneCV.wait( lock, [&]() { return m_Ready.empty() && m_NumBusy == 0; } );

        if ( m_Err )
        {
            std::exception_ptr err = m_Err;
            m_Err = nullptr;
            std::rethrow_exception( err );
        }
    }

protected:

    void Work()
    {
        // A new thread starts in the process-wide locale, not the caller's.
        CNumericLocaleGuard locale_guard;

        std::unique_lock< std::mutex > lock( m_Mutex );

        while ( true )
        {
            m_WorkCV.wait( lock, [&]() { return m_Stop || !m_Ready.empty(); } );

            if ( m_Ready.empty() )
            {
                return;
            }

            int isurf = m_Ready.front();
            m_Ready.pop_front();

            std::function< void() > f = m_Queue[ isurf ].front();
            m_Queue[ isurf ].pop_front();
            m_Busy[ isurf ] = true;
            m_NumBusy++;

            lock.unlock();

            try
            {
                f();
            }
            catch ( ... )
            {
                std::lock_guard< std::mutex > errlock( m_ErrMutex );
                if ( !m_Err )
                {
                    m_Err = std::current_exception();
                }
            }

            lock.lock();

            m_Busy[ isurf ] = false;
            m_NumBusy--;

            if ( !m_Queue[ isurf ].empty() )
            {
                m_Ready.push_back( isurf );
                m_WorkCV.notify_one();
            }

            m_DoneCV.notify_all();
        }
    }

    vector < std::deque < std::function< void() > > > m_Queue;
    vector < bool > m_Busy;
    std::deque < int > m_Ready;
    int m_NumBusy = 0;
    bool m_Stop;

    std::mutex m_Mutex;
    std::condition_variable m_WorkCV;
    std::condition_variable m_DoneCV;

    std::mutex m_ErrMutex;
    std::exception_ptr m_Err;

    vector < std::thread > m_Pool;
};

// Density is worked out along each chain from the maps of the two surfaces it lies on, and then
// spread back onto those maps, chain after chain.  Working a chain out reads its two maps and the
// split sources, and spreading writes only its two maps, so a chain's spreading can run while
// later chains on other surfaces are worked out.  Chains are worked out here in order, each once
// its surfaces are idle, and spreading is queued on each surface in that same order, so every
// map is read and written in the order the serial loop would.  The two sides of a chain are
// separate maps unless the chain lies on one surface, and are spread separately.
void CfdMeshMgrSingleton::SpreadChainDensity( int nedgeprop, int nthread, SplitSourceSet &splitSources, int stride, int output_type )
{
    int nsurf = ( int )m_SurfVec.size();

    std::map< Surf*, int > surfindex;
    for ( int i = 0 ; i < nsurf ; i++ )
    {
        surfindex[ m_SurfVec[i] ] = i;
    }

    int nchain = ( int )m_ISegChainList.size();
    vector < ISegChain* > chains( nchain );
    vector < int > surfa( nchain, -1 );
    vector < int > surfb( nchain, -1 );

    int k = 0;
    list< ISegChain* >::iterator c;
    for ( c = m_ISegChainList.begin() ; c != m_ISegChainList.end(); ++c )
    {
        chains[k] = *c;

        std::map< Surf*, int >::iterator it = surfindex.find( ( *c )->m_ACurve.GetSurf() );
        if ( it != surfindex.end() )
        {
            surfa[k] = it->second;
        }

        it = surfindex.find( ( *c )->m_BCurve.GetSurf() );
        if ( it != surfindex.end() )
        {
            surfb[k] = it->second;
        }

        k++;
    }

    SurfWorkQueue queue( nsurf, nthread - 1 );

    for ( int ipass = 0; ipass < nedgeprop; ipass++ )
    {
        for ( k = 0 ; k < nchain ; k++ )
        {
            ISegChain* chain = chains[k];
            int ia = surfa[k];
            int ib = surfb[k];

            // A chain on a surface the mesher does not hold is done here, once everything is idle.
            if ( ia < 0 || ib < 0 )
            {
                queue.WaitAll();
                chain->CalcDensity( GetGridDensityPtr(), splitSources );
                chain->SpreadDensity();
                StepProgressEvery( stride, output_type );
                continue;
            }

            queue.WaitSurf( ia );
            queue.WaitSurf( ib );

            chain->CalcDensity( GetGridDensityPtr(), splitSources );

            if ( ia == ib )
            {
                queue.Add( ia, [chain]() { chain->SpreadDensity(); } );
            }
            else
            {
                queue.Add( ia, [chain]() { chain->SpreadDensityA(); } );
                queue.Add( ib, [chain]() { chain->SpreadDensityB(); } );
            }

            StepProgressEvery( stride, output_type );
        }
    }

    queue.WaitAll();
}

// Lay out every chain's distance table before density is spread along the chains.  The table is
// fixed by the curves alone, so this changes nothing but when it is done.  The A side evaluates
// surface A and the B side surface B, so each side is grouped by its surface and the groups run
// side by side, one surface to a thread.
void CfdMeshMgrSingleton::BuildChainDistTables()
{
    int nsurf = ( int )m_SurfVec.size();

    std::map< Surf*, int > surfindex;
    for ( int i = 0 ; i < nsurf ; i++ )
    {
        surfindex[ m_SurfVec[i] ] = i;
    }

    vector < vector < ISegChain* > > bysurfa( nsurf );
    vector < vector < ISegChain* > > bysurfb( nsurf );

    list< ISegChain* >::iterator c;
    for ( c = m_ISegChainList.begin() ; c != m_ISegChainList.end(); ++c )
    {
        std::map< Surf*, int >::iterator ita = surfindex.find( ( *c )->m_ACurve.GetSurf() );
        std::map< Surf*, int >::iterator itb = surfindex.find( ( *c )->m_BCurve.GetSurf() );

        // A chain on a surface the mesher does not hold is left to build its table when first used.
        if ( ita != surfindex.end() && itb != surfindex.end() )
        {
            bysurfa[ ita->second ].push_back( *c );
            bysurfb[ itb->second ].push_back( *c );
        }
    }

    int nthread = StageThreadCount( nsurf );

    // Every surface's A sides, then its B sides.
    ProgressStage bar( this, "Chain Distance", 2 * nsurf );

    RunIndexed( nsurf, nthread, [&]( int i )
    {
        for ( int k = 0 ; k < ( int )bysurfa[i].size() ; k++ )
        {
            bysurfa[i][k]->BuildDistTableGeom();
        }
    }, &bar );

    RunIndexed( nsurf, nthread, [&]( int i )
    {
        for ( int k = 0 ; k < ( int )bysurfb[i].size() ; k++ )
        {
            bysurfb[i][k]->BuildDistTableGeomB();
        }
    }, &bar );
}

// Remesh one surface.  Everything here reaches the model only through that surface's own
// Mesh, which is what lets surfaces be run side by side.  The two shared things it touches
// are the grid density, which is only read, and the progress text, which is locked.
void CfdMeshMgrSingleton::RemeshOneSurf( int isurf, int nsurf, int output_type, int &num_tris )
{
    num_tris = 0;

    for ( int iter = 0 ; iter < 10 ; ++iter )
    {
        num_tris = 0;
        m_SurfVec[isurf]->GetMesh()->Remesh();

        m_SurfVec[isurf]->GetMesh()->RemoveIllFormedFaces();

        num_tris += m_SurfVec[isurf]->GetMesh()->GetNumFaces();
    }

    m_SurfVec[isurf]->GetMesh()->DumpGarbage();

    m_ProgressDone++;
    StepProgress( output_type );
}

void CfdMeshMgrSingleton::Remesh( int output_type )
{
    char str[256];
    int nsurf = ( int )m_SurfVec.size();
    vector < int > surftris( nsurf, 0 );

    int nthread = StageThreadCount( nsurf );

    BeginProgress( "Remesh", nsurf, output_type );

    // Biggest surfaces first, so the long surfaces start while there is still small work to fill
    // in behind them and the run does not end on one large surface with every other thread idle.
    // The surfaces are independent, so the order they are taken in does not change any of them.
    vector < pair < int, int > > order( nsurf );

    for ( int i = 0 ; i < nsurf ; i++ )
    {
        order[i] = pair< int, int >( m_SurfVec[i]->GetMesh()->GetNumFaces(), i );
    }

    sort( order.begin(), order.end(), std::greater< pair < int, int > >() );

    RunIndexed( nsurf, nthread, [&]( int k )
    {
        int i = order[k].second;

        RemeshOneSurf( i, nsurf, output_type, surftris[i] );
    } );

    EndProgress( output_type );

    int total_num_tris = 0;

    for ( int i = 0 ; i < nsurf ; i++ )
    {
        total_num_tris += surftris[i];
    }

    WakeMgr.StretchWakes();

    m_NumMeshTris = total_num_tris;

    snprintf( str, sizeof( str ), "Total Num Tris = %d\n", total_num_tris );
    addOutputText( str, output_type );
}

void CfdMeshMgrSingleton::PostMesh()
{
    int nsurf = ( int )m_SurfVec.size();

    // Gathered here because Clear() below is the end of the edges, and the target length an
    // edge was working to lives on the edge.
    m_LengthRatios.clear();
    m_FaceLengthRatios.clear();
    m_BorderFaceLengthRatios.clear();

    // Each surface's share of what this stage collects, kept apart and joined in surface
    // order afterwards, so the answer does not depend on which thread finished first.
    vector < vector < double > > lenratio( nsurf );
    vector < vector < double > > faceratio( nsurf );
    vector < vector < double > > borderratio( nsurf );
    vector < std::set< std::vector< int > > > tagcombo( nsurf );

    // Each surface reduces its own mesh to simple faces, tags them and condenses them.  None
    // of that reaches another surface: the target map, the subsurfaces and the tag numbering
    // are all read, and everything written belongs to the surface or to the vectors above.
    RunStage( "Post Mesh", nsurf, [&]( int i )
    {
        m_SurfVec[ i ]->GetMesh()->LoadSimpFaces();

        if ( TARGET_LENGTH_REPORT )
        {
            m_SurfVec[ i ]->GetMesh()->AccumLengthRatios( lenratio[i] );

            // The same misses counted per face instead of per edge, which is what the coloured
            // picture shows: a face is as bad as its worst edge, so the share of bad faces is
            // always larger than the share of bad edges and the two must not be confused.
            const vector< SimpFace > &sfv = m_SurfVec[ i ]->GetMesh()->GetSimpFaceVec();

            for ( int f = 0 ; f < ( int )sfv.size() ; f++ )
            {
                if ( sfv[f].m_WorstLenRatio > 0.0 )
                {
                    faceratio[i].push_back( sfv[f].m_WorstLenRatio );

                    if ( sfv[f].m_OffBorder )
                    {
                        borderratio[i].push_back( sfv[f].m_WorstLenRatio );
                    }
                }
            }
        }
        m_SurfVec[i]->GetMesh()->Clear();
        Subtag( m_SurfVec[i], tagcombo[i] );
        m_SurfVec[ i ]->GetMesh()->CondenseSimpFaces();
    } );

    for ( int i = 0 ; i < nsurf ; ++i )
    {
        m_LengthRatios.insert( m_LengthRatios.end(), lenratio[i].begin(), lenratio[i].end() );
        m_FaceLengthRatios.insert( m_FaceLengthRatios.end(), faceratio[i].begin(), faceratio[i].end() );
        m_BorderFaceLengthRatios.insert( m_BorderFaceLengthRatios.end(), borderratio[i].begin(), borderratio[i].end() );
        SubSurfaceMgr.m_TagCombos.insert( tagcombo[i].begin(), tagcombo[i].end() );
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

// Each message goes out ahead of the work it names, so whatever is on screen is what is running.
// The POGS files are the slow ones and are optional, so they speak for themselves below rather
// than being covered by one label for the whole stage.
void CfdMeshMgrSingleton::ExportFiles()
{
    addOutputText( "Exporting Files\n" );

    ClearExportMerge();

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

    string pogs_fn;
    if (  GetSettingsPtr()->GetExportFileFlag( vsp::CFD_POGS_FILE_NAME ) )
    {
        addOutputText( "Building POGS Surfaces\n" );

        // The POGS files are polylines, so the curves are only adapted that far
        BuildNURBSCurvesVec( true, false ); // Note: Must be called before BuildNURBSSurfMap

        BuildNURBSSurfMap();

        pogs_fn = GetSettingsPtr()->GetExportFileName( vsp::CFD_POGS_FILE_NAME );
        WritePOGS( pogs_fn );
    }

}

void CfdMeshMgrSingleton::ClearExportMerge()
{
    m_ExportMergeBuilt = false;
    vector< int >().swap( m_ExportMergeStart );
    vector< int >().swap( m_ExportMergeGroup );
}

// The points within merging distance of point p, by their place among all the points: each
// slab's cloud holds its members, and slab[p] names the slab p was searched in.
static void FindMergeNeighbors( const vector < PntNodeCloud > &clouds, const vector < vector < int > > &members,
                                const vector < int > &slab, const vector < vec3d > &pnts, int p,
                                vector < unsigned int > &found )
{
    int k = slab[p];
    FindPntNodesWithinTolSq( clouds[k], pnts[p], PT_MERGE_TOL, found );
    for ( int m = 0 ; m < ( int )found.size() ; m++ )
    {
        found[m] = members[k][ found[m] ];
    }
}

// Every point is searched for the points within merging distance of it, on several threads.  A
// point's group is the lowest of those.  The groups can stand for every writer's merge when no
// search reaches a point of another group and every search finds its whole group; the second
// pass checks exactly that.
//
// Building one tree of every point is a long serial job, so the points are cut into slabs across
// x and each slab gets a tree of its own, built on a thread of its own.  A slab's tree also holds
// the points of its neighbours that lie within twice the merging distance of it, so a search
// from any point of the slab finds everything a tree of all the points would.
void CfdMeshMgrSingleton::BuildExportMerge()
{
    ClearExportMerge();
    m_ExportMergeBuilt = true;

    int nsurf = ( int )m_SurfVec.size();
    m_ExportMergeStart.resize( nsurf + 1 );
    m_ExportMergeStart[0] = 0;
    for ( int i = 0 ; i < nsurf ; i++ )
    {
        m_ExportMergeStart[i + 1] = m_ExportMergeStart[i] + ( int )m_SurfVec[i]->GetMesh()->GetSimpPntVec().size();
    }
    int npnt = m_ExportMergeStart[ nsurf ];

    vector < vec3d > pnts;
    pnts.reserve( npnt );
    for ( int i = 0 ; i < nsurf ; i++ )
    {
        const vector < vec3d > &sPntVec = m_SurfVec[i]->GetMesh()->GetSimpPntVec();
        pnts.insert( pnts.end(), sPntVec.begin(), sPntVec.end() );
    }

    //==== Cut into slabs of about equal count, from a sample of the x values ====//
    const int slabpnts = 50000;
    int nslab = StageThreadCount( npnt / slabpnts );
    vector < double > cut;
    {
        vector < double > xs;
        for ( int p = 0 ; p < npnt ; p += 97 )
        {
            if ( !std::isnan( pnts[p].x() ) )
            {
                xs.push_back( pnts[p].x() );
            }
        }
        std::sort( xs.begin(), xs.end() );
        for ( int k = 1 ; k < nslab && !xs.empty() ; k++ )
        {
            cut.push_back( xs[ ( size_t )k * xs.size() / nslab ] );
        }
        nslab = ( int )cut.size() + 1;
    }

    // Slab k holds lo( k ) <= x < hi( k ).  A point that is not a number goes in the first.
    vector < int > slab( npnt, 0 );
    vector < vector < int > > members( nslab );
    double margin = 2.0 * sqrt( PT_MERGE_TOL );
    for ( int p = 0 ; p < npnt ; p++ )
    {
        double x = pnts[p].x();
        if ( std::isnan( x ) )
        {
            members[0].push_back( p );
            continue;
        }

        int k = ( int )( std::upper_bound( cut.begin(), cut.end(), x ) - cut.begin() );
        slab[p] = k;
        members[k].push_back( p );

        // Near enough a neighbour's side to be found from there
        for ( int j = k - 1 ; j >= 0 && x - margin < cut[j] ; j-- )
        {
            members[j].push_back( p );
        }
        for ( int j = k + 1 ; j < nslab && x + margin >= cut[j - 1] ; j++ )
        {
            members[j].push_back( p );
        }
    }

    // Counted in points: those of every slab's tree, then each point searched for twice.
    long long nintree = 0;
    for ( int k = 0 ; k < nslab ; k++ )
    {
        nintree += members[k].size();
    }
    ProgressStage bar( this, "Merge Points", ( int )( nintree + 2LL * npnt ) );

    vector < PntNodeCloud > clouds( nslab );
    RunIndexed( nslab, StageThreadCount( nslab ), [&]( int k )
    {
        clouds[k].ReserveMorePntNodes( ( long long )members[k].size() );
        for ( int m = 0 ; m < ( int )members[k].size() ; m++ )
        {
            clouds[k].AddPntNode( pnts[ members[k][m] ] );
        }
        BuildPntNodeIndex( clouds[k] );

        bar.Step( ( int )members[k].size() );
    } );

    const int chunkitems = 4096;
    int nchunk = ( npnt + chunkitems - 1 ) / chunkitems;

    vector < int > group( npnt );
    vector < int > nnear( npnt );

    RunIndexed( nchunk, StageThreadCount( nchunk ), [&]( int c )
    {
        vector < unsigned int > found;
        int iend = std::min( npnt, ( c + 1 ) * chunkitems );
        for ( int p = c * chunkitems ; p < iend ; p++ )
        {
            FindMergeNeighbors( clouds, members, slab, pnts, p, found );

            // A point that is not a number finds nothing, itself included, and is found by
            // nothing: a group of one.
            group[p] = p;
            nnear[p] = 1;
            if ( !found.empty() )
            {
                group[p] = ( int )*std::min_element( found.begin(), found.end() );
                nnear[p] = ( int )found.size();
            }
        }

        bar.Step( iend - c * chunkitems );
    } );

    vector < int > nmember( npnt, 0 );
    for ( int p = 0 ; p < npnt ; p++ )
    {
        nmember[ group[p] ]++;
    }

    vector < char > bad( nchunk, 0 );

    RunIndexed( nchunk, StageThreadCount( nchunk ), [&]( int c )
    {
        vector < unsigned int > found;
        int iend = std::min( npnt, ( c + 1 ) * chunkitems );
        for ( int p = c * chunkitems ; p < iend && !bad[c] ; p++ )
        {
            if ( nnear[p] != nmember[ group[p] ] )
            {
                bad[c] = 1;
            }
            else if ( nnear[p] > 1 )
            {
                FindMergeNeighbors( clouds, members, slab, pnts, p, found );
                for ( int k = 0 ; k < ( int )found.size() ; k++ )
                {
                    if ( group[ found[k] ] != group[p] )
                    {
                        bad[c] = 1;
                    }
                }
            }
        }

        bar.Step( iend - c * chunkitems );
    } );

    bar.End();

    for ( int c = 0 ; c < nchunk ; c++ )
    {
        if ( bad[c] )
        {
            return;
        }
    }

    m_ExportMergeGroup.swap( group );
}

void CfdMeshMgrSingleton::MergeSurfPnts( const vector< bool > &use, vector< vector< int > > &used_key,
                                         vector< vec3d > &used_pnt, vector< int > *used_first )
{
    if ( !m_ExportMergeBuilt )
    {
        BuildExportMerge();
    }

    int nsurf = ( int )m_SurfVec.size();

    used_key.clear();
    used_key.resize( nsurf );
    used_pnt.clear();
    if ( used_first )
    {
        used_first->clear();
    }

    if ( !m_ExportMergeGroup.empty() )
    {
        // A group is numbered where its first member among the chosen points comes, which is
        // where a merge of those points alone puts it.
        vector < int > num( m_ExportMergeGroup.size(), -1 );
        int k = 0;
        for ( int i = 0 ; i < nsurf ; i++ )
        {
            if ( !use[i] )
            {
                continue;
            }

            const vector< vec3d >& sPntVec = m_SurfVec[i]->GetMesh()->GetSimpPntVec();
            used_key[i].resize( sPntVec.size() );
            for ( int v = 0 ; v < ( int )sPntVec.size() ; v++ )
            {
                int g = m_ExportMergeGroup[ m_ExportMergeStart[i] + v ];
                if ( num[g] < 0 )
                {
                    num[g] = ( int )used_pnt.size();
                    used_pnt.push_back( sPntVec[v] );
                    if ( used_first )
                    {
                        used_first->push_back( k );
                    }
                }
                used_key[i][v] = num[g];
                k++;
            }
        }
        return;
    }

    vector< vec3d > allPntVec;
    vector < vector < int > > allPntKey( nsurf );
    int k = 0;
    for ( int i = 0 ; i < nsurf ; i++ )
    {
        if ( !use[i] )
        {
            continue;
        }

        const vector< vec3d >& sPntVec = m_SurfVec[i]->GetMesh()->GetSimpPntVec();
        allPntKey[i].resize( sPntVec.size() );
        for ( int v = 0 ; v < ( int )sPntVec.size() ; v++ )
        {
            allPntVec.push_back( sPntVec[v] );
            allPntKey[i][v] = k;
            k++;
        }
    }

    PntNodeCloud pnCloud;
    pnCloud.AddPntNodes( allPntVec );
    IndexPntNodes( pnCloud, PT_MERGE_TOL );

    for ( int i = 0 ; i < nsurf ; i++ )
    {
        used_key[i].resize( allPntKey[i].size() );
        for ( int v = 0 ; v < ( int )allPntKey[i].size() ; v++ )
        {
            used_key[i][v] = ( int )pnCloud.GetNodeUsedIndex( allPntKey[i][v] );
        }
    }

    used_pnt.reserve( pnCloud.m_NumUsedPts );
    for ( int i = 0 ; i < ( int )allPntVec.size() ; i++ )
    {
        if ( pnCloud.UsedNode( i ) )
        {
            used_pnt.push_back( allPntVec[i] );
            if ( used_first )
            {
                used_first->push_back( i );
            }
        }
    }
}

void CfdMeshMgrSingleton::GroupFacesByTag( const vector< const vector < int >* > &faceTags, const vector< int > &tags,
                                            vector< vector< int > > &groups )
{
    std::map< int, int > tagpos;
    for ( int itag = 0; itag < ( int ) tags.size(); itag++ )
    {
        tagpos[ tags[itag] ] = itag;
    }

    int nface = ( int )faceTags.size();
    vector < int > pos( nface, -1 );

    const int chunkitems = 4096;
    int nchunk = ( nface + chunkitems - 1 ) / chunkitems;
    RunIndexed( nchunk, StageThreadCount( nchunk ), [&]( int c )
    {
        int iend = std::min( nface, ( c + 1 ) * chunkitems );
        for ( int f = c * chunkitems ; f < iend ; f++ )
        {
            std::map< int, int >::const_iterator it = tagpos.find( SubSurfaceMgr.GetTag( *faceTags[f] ) );
            if ( it != tagpos.end() )
            {
                pos[f] = it->second;
            }
        }
    } );

    groups.clear();
    groups.resize( tags.size() );
    for ( int f = 0; f < nface; f++ )
    {
        if ( pos[f] >= 0 )
        {
            groups[ pos[f] ].push_back( f );
        }
    }
}

void CfdMeshMgrSingleton::WriteTaggedSTL( const string &filename )
{
    //==== Find All Points and Tri Counts ====//
    size_t nallface = 0;
    for ( int i = 0 ; i < ( int )m_SurfVec.size() ; i++ )
    {
        nallface += m_SurfVec[i]->GetMesh()->GetSimpFaceVec().size();
    }

    //==== Merge the points of every surface ====//
    vector < vector < int > > usedKey;
    vector< vec3d > allUsedPntVec;
    MergeSurfPnts( vector< bool >( m_SurfVec.size(), true ), usedKey, allUsedPntVec, nullptr );

    //==== Assemble Normal Tris ====//
    // Each face's tags are pointed to, not copied.
    vector< SimpFace > allFaceVec;
    allFaceVec.reserve( nallface );
    vector< const vector < int >* > allTagsVec;
    allTagsVec.reserve( nallface );
    for ( int i = 0 ; i < ( int )m_SurfVec.size() ; i++ )
    {
        vector < SimpFace >& sFaceVec = m_SurfVec[ i ]->GetMesh()->GetSimpFaceVec();
        vector< vec3d >& sPntVec = m_SurfVec[i]->GetMesh()->GetSimpPntVec();
        for ( int f = 0 ; f <  ( int )sFaceVec.size() ; f++ )
        {
            SimpFace sface;
            sface.ind0 = usedKey[i][sFaceVec[f].ind0];
            sface.ind1 = usedKey[i][sFaceVec[f].ind1];
            sface.ind2 = usedKey[i][sFaceVec[f].ind2];

            if( sFaceVec[f].m_isQuad )
            {
                sface.m_isQuad = true;
                sface.ind3 = usedKey[i][sFaceVec[f].ind3];
            }

            allTagsVec.push_back( &sFaceVec[f].m_Tags );
            allFaceVec.push_back( sface );
        }
    }

    // The faces are numbered now, so the map is done with.
    vector < vector < int > >().swap( usedKey );

    FILE* file_id = fopen( filename.c_str(), "w" );
    if ( file_id )
    {
        ProgressStage bar( this, "Write STL", ( int )allFaceVec.size() );

        std::vector< int > tags = SubSurfaceMgr.GetAllTags();

        // Which faces belong to each tag, gathered in one pass.  Asking every face about
        // every tag in turn is a walk of the whole mesh per tag, and a model can carry
        // dozens of tags.
        vector < vector < int > > tagface;
        GroupFacesByTag( allTagsVec, tags, tagface );

        for ( int itag = 0; itag < ( int ) tags.size(); itag++ )
        {
            std::string tagname = SubSurfaceMgr.GetTagNames( itag );
            fprintf( file_id, "solid %s\n", tagname.c_str() );

            const vector < int > &face = tagface[itag];

            WriteChunked( file_id, ( int )face.size(), [&]( int ibeg, int iend, string &out )
            {
                for ( int k = ibeg; k < iend; k++ )
                {
                    const SimpFace* sface = &allFaceVec[ face[k] ];

                    const vec3d& p0 = allUsedPntVec[sface->ind0];
                    const vec3d& p1 = allUsedPntVec[sface->ind1];
                    const vec3d& p2 = allUsedPntVec[sface->ind2];
                    vec3d v01 = p1 - p0;
                    vec3d v12 = p2 - p1;
                    vec3d norm = cross( v01, v12 );
                    norm.normalize();

                    AppendSTLFacet( out, norm, p0, p1, p2 );

                    if ( sface->m_isQuad ) // Split quad and write additional tri.
                    {
                        vec3d p3 = allUsedPntVec[sface->ind3];
                        vec3d v23 = p3 - p2;
                        vec3d v30 = p0 - p3;
                        norm = cross( v23, v30 );
                        norm.normalize();

                        AppendSTLFacet( out, norm, p0, p2, p3 );
                    }
                }
            }, &bar );

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
        int nface = 0;
        for ( int i = 0 ; i < ( int )m_SurfVec.size() ; i++ )
        {
            nface += m_SurfVec[ i ]->GetMesh()->GetNumSimpFaces();
        }
        ProgressStage bar( this, "Write STL", nface );

        int numwake = 0;
        fprintf( file_id, "solid\n" );
        for ( int i = 0 ; i < ( int )m_SurfVec.size() ; i++ )
        {
            if ( !m_SurfVec[i]->GetWakeFlag() )
            {
                Mesh* msh = m_SurfVec[ i ]->GetMesh();
                WriteChunked( file_id, msh->GetNumSimpFaces(), [&]( int ibeg, int iend, string &out )
                {
                    msh->AppendSimpleSTL( ibeg, iend, out );
                }, &bar );
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
                    Mesh* msh = m_SurfVec[ i ]->GetMesh();
                    WriteChunked( file_id, msh->GetNumSimpFaces(), [&]( int ibeg, int iend, string &out )
                    {
                        msh->AppendSimpleSTL( ibeg, iend, out );
                    }, &bar );
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
    for ( int i = 0 ; i < ( int )m_SurfVec.size() ; i++ )
    {
        face_cnt += m_SurfVec[ i ]->GetMesh()->GetSimpFaceVec().size();
    }

    //==== Merge the points of every surface ====//
    vector < vector < int > > usedKey;
    vector< vec3d > usedPnt;
    vector< int > usedFirst;
    MergeSurfPnts( vector< bool >( m_SurfVec.size(), true ), usedKey, usedPnt, &usedFirst );

    ProgressStage bar( this, "Write POLY", ( int )usedPnt.size() + face_cnt );

    //===== Write Num Pnts and Tris ====//
    fprintf( fp, "# Part 1 - node list\n" );
    fprintf( fp, "%lld 3 0 0\n", ( long long int )usedPnt.size() );

    //==== Write Model Pnts ====//
    // Each is numbered by the place of its first copy among all the surfaces' points.
    WriteChunked( fp, ( int )usedPnt.size(), [&]( int ibeg, int iend, string &out )
    {
        for ( int i = ibeg ; i < iend ; i++ )
        {
            FastFormat::AppendInt( out, usedFirst[i] + 1 );
            out += ' ';
            AppendGTriple( out, usedPnt[i].x(), usedPnt[i].y(), usedPnt[i].z(), 0, 16 );
        }
    }, &bar );

    //==== Write Tris ====//
    fprintf( fp, "# Part 2 - facet list\n" );
    // <# of facets> <boundary markers (0 or 1)> 
    fprintf( fp, "%d 1\n", face_cnt );

    for ( int i = 0 ; i < ( int )m_SurfVec.size() ; i++ )
    {
        const vector < SimpFace >& sFaceVec = m_SurfVec[ i ]->GetMesh()->GetSimpFaceVec();
        const vector < int >& key = usedKey[i];
        WriteChunked( fp, ( int )sFaceVec.size(), [&]( int ibeg, int iend, string &out )
        {
            for ( int f = ibeg ; f < iend ; f++ )
            {
                const SimpFace &sf = sFaceVec[f];

                // <# of polygons> [# of holes] [boundary marker]
                out += "1 0 ";
                FastFormat::AppendInt( out, SubSurfaceMgr.GetTag( sf.m_Tags ) );
                out += '\n';

                // <# of corners> <corner 1> <corner 2> <corner 3> [<corner 4>]
                int corner[4] = { sf.ind0, sf.ind1, sf.ind2, sf.ind3 };
                int n = 3;
                if( sf.m_isQuad )
                {
                    n = 4;
                }
                FastFormat::AppendInt( out, n );
                for ( int c = 0 ; c < n ; c++ )
                {
                    out += ' ';
                    FastFormat::AppendInt( out, key[ corner[c] ] + 1 );
                }
                out += '\n';
            }
        }, &bar );
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

    //==== Any Files To Write? ====//
    if ( dat_fn.empty() && key_fn.empty() && obj_fn.empty() && tri_fn.empty() &&
         gmsh_fn.empty() && vspgeom_fn.empty() )
    {
        return;
    }

    // Used when comparing W parameter to TMAGIC
    double tol = 1e-12;

    //==== Find All Tri Counts ====//
    size_t nallface = 0;
    vector < bool > wake( m_SurfVec.size() );
    vector < bool > notwake( m_SurfVec.size() );
    for ( int i = 0 ; i < ( int )m_SurfVec.size() ; i++ )
    {
        wake[i] = m_SurfVec[i]->GetWakeFlag();
        notwake[i] = !wake[i];
        nallface += m_SurfVec[i]->GetMesh()->GetSimpFaceVec().size();
    }

    //==== Merge the points of the surfaces, then those of the wakes apart from them ====//
    vector < vector < int > > allPntKey;
    vector< vec3d > allUsedPntVec;
    MergeSurfPnts( notwake, allPntKey, allUsedPntVec, nullptr );

    vector < vector < int > > allWPntKey;
    vector< vec3d > wakeUsedPntVec;
    MergeSurfPnts( wake, allWPntKey, wakeUsedPntVec, nullptr );

    //==== Assemble Normal Tris ====//
    // Each face's tags are left where they are and pointed to, rather than copied: a copy is an
    // allocation per face.
    vector< SimpFace > allFaceVec;
    allFaceVec.reserve( nallface );
    vector< const vector < int >* > allTagsVec;
    allTagsVec.reserve( nallface );
    int ntristrict = 0;
    // Four parameter pairs per face, laid end to end.  A vector per face is an allocation
    // per face, and a mesh has a million of them.  Only the VSPGEOM file reads them.
    vector< vec2d > allUWVec;
    if ( !vspgeom_fn.empty() )
    {
        allUWVec.reserve( 4 * nallface );
    }
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
                sface.ind0 = allPntKey[i][sFaceVec[t].ind0] + 1;
                sface.ind1 = allPntKey[i][sFaceVec[t].ind1] + 1;
                sface.ind2 = allPntKey[i][sFaceVec[t].ind2] + 1;

                if( sFaceVec[t].m_isQuad )
                {
                    sface.m_isQuad = true;
                    sface.ind3 = allPntKey[i][sFaceVec[t].ind3] + 1;
                    ntristrict++; // Bonus tri for split quad.
                }

                allTagsVec.push_back( &sFaceVec[t].m_Tags );
                ntristrict++;
                allFaceVec.push_back( sface );

                vec2d uwFace[4];
                uwFace[0] = sUWVec[ sFaceVec[t].ind0 ];
                uwFace[1] = sUWVec[ sFaceVec[t].ind1 ];
                uwFace[2] = sUWVec[ sFaceVec[t].ind2 ];
                if( sFaceVec[t].m_isQuad )
                {
                    uwFace[3] = sUWVec[ sFaceVec[t].ind3 ];
                }
                if ( !vspgeom_fn.empty() )
                {
                    allUWVec.insert( allUWVec.end(), uwFace, uwFace + 4 );
                }

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
                        int i0 = allPntKey[i][sFaceVec[t].ind0] + 1;
                        int i1 = allPntKey[i][sFaceVec[t].ind1] + 1;
                        int i2 = allPntKey[i][sFaceVec[t].ind2] + 1;


                        int i3 = -1;
                        if( sFaceVec[t].m_isQuad )
                        {
                            i3 = allPntKey[i][sFaceVec[t].ind3] + 1;
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

    vector < vector < pair < int, int > > > wakes;

    while ( !wlist.empty() )
    {
        list < pair < int, int > >::iterator wit = wlist.begin();

        TwoEndedVector < pair < int, int > > wake( *wit );
        wit = wlist.erase( wit );

        while ( wit != wlist.end() )
        {
            if ( wake.back().second == (*wit).first )
            {
                wake.push_back( *wit );
                wlist.erase( wit );
                wit = wlist.begin();
                continue;
            }
            else if ( wake.front().first == (*wit).second )
            {
                wake.push_front( *wit );
                wlist.erase( wit );
                wit = wlist.begin();
                continue;
            }
            wit++;
        }

        wakes.push_back( wake.GetVector() );
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
                int i0 = allWPntKey[i][sFaceVec[f].ind0];
                int i1 = allWPntKey[i][sFaceVec[f].ind1];
                int i2 = allWPntKey[i][sFaceVec[f].ind2];

                SimpFace sface;
                sface.ind0 = i0 + 1 + wakeIndOffset;
                sface.ind1 = i1 + 1 + wakeIndOffset;
                sface.ind2 = i2 + 1 + wakeIndOffset;

                if( sFaceVec[f].m_isQuad )
                {
                    sface.m_isQuad = true;
                    int i3 = allWPntKey[i][sFaceVec[f].ind3];
                    sface.ind3 = i3 + 1 + wakeIndOffset;
                    ntristrict++; // Bonus tri for split quad
                }

                allTagsVec.push_back( &sFaceVec[f].m_Tags );
                ntristrict++;
                allFaceVec.push_back( sface );

                vec2d uwFace[4];
                uwFace[0] = sUWVec[ sFaceVec[f].ind0 ];
                uwFace[1] = sUWVec[ sFaceVec[f].ind1 ];
                uwFace[2] = sUWVec[ sFaceVec[f].ind2 ];
                if( sFaceVec[f].m_isQuad )
                {
                    uwFace[3] = sUWVec[ sFaceVec[f].ind3 ];
                }
                if ( !vspgeom_fn.empty() )
                {
                    allUWVec.insert( allUWVec.end(), uwFace, uwFace + 4 );
                }

            }
        }
    }

    //==== Assemble All Used Points ====//
    allUsedPntVec.insert( allUsedPntVec.end(), wakeUsedPntVec.begin(), wakeUsedPntVec.end() );

    // Each face's tag, looked up once for all the files that write it.
    vector < int > allTagVec( allFaceVec.size() );
    {
        const int chunkitems = 4096;
        int nface = ( int )allFaceVec.size();
        int nchunk = ( nface + chunkitems - 1 ) / chunkitems;
        RunStage( "Face Tags", nchunk, [&]( int c )
        {
            int iend = std::min( nface, ( c + 1 ) * chunkitems );
            for ( int i = c * chunkitems ; i < iend ; i++ )
            {
                allTagVec[i] = SubSurfaceMgr.GetTag( *allTagsVec[i] );
            }
        } );
    }

    // The faces are numbered now, so the maps are done with.
    vector< vec3d >().swap( wakeUsedPntVec );
    vector < vector < int > >().swap( allPntKey );
    vector < vector < int > >().swap( allWPntKey );

    //=====================================================================================//
    //==== Write NASCART File =================================================================//
    //=====================================================================================//
    if ( dat_fn.length() != 0 )
    {
        FILE* fp = fopen( dat_fn.c_str(), "w" );

        if ( fp )
        {
            ProgressStage bar( this, "Write DAT", ( int )( allUsedPntVec.size() + allFaceVec.size() ) );

            //===== Write Num Pnts and Tris ====//
            fprintf( fp, "%d %d\n", ( int )allUsedPntVec.size(), ntristrict );

            //==== Write Pnts ====//
            WriteChunked( fp, ( int )allUsedPntVec.size(), [&]( int ibeg, int iend, string &out )
            {
                for ( int i = ibeg ; i < iend ; i++ )
                {
                    AppendGTriple( out, allUsedPntVec[i].x(), allUsedPntVec[i].z(), -allUsedPntVec[i].y(), 0, 16 );
                }
            }, &bar );

            //==== Write Tris ====//
            WriteChunked( fp, ( int )allFaceVec.size(), [&]( int ibeg, int iend, string &out )
            {
                for ( int i = ibeg ; i < iend ; i++ )
                {
                    const SimpFace &f = allFaceVec[i];
                    int tag = allTagVec[i];

                    int t0[4] = { f.ind0, f.ind2, f.ind1, tag };
                    AppendInts( out, t0, 3, ' ' );
                    FastFormat::AppendInt( out, t0[3] );
                    out += ".0\n";

                    if( f.m_isQuad )
                    {
                        int t1[4] = { f.ind0, f.ind3, f.ind2, tag };
                        AppendInts( out, t1, 3, ' ' );
                        FastFormat::AppendInt( out, t1[3] );
                        out += ".0\n";
                    }
                }
            }, &bar );
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
            ProgressStage bar( this, "Write OBJ", ( int )( allUsedPntVec.size() + allFaceVec.size() ) );

            //==== Write Pnts ====//
            WriteChunked( fp, ( int )allUsedPntVec.size(), [&]( int ibeg, int iend, string &out )
            {
                for ( int i = ibeg ; i < iend ; i++ )
                {
                    out += "v ";
                    AppendFTriple( out, allUsedPntVec[i].x(), allUsedPntVec[i].z(), -allUsedPntVec[i].y(), 16, 10 );
                }
            }, &bar );
            fprintf( fp, "\n" );

            //==== Write Tris ====//
            WriteChunked( fp, ( int )allFaceVec.size(), [&]( int ibeg, int iend, string &out )
            {
                for ( int i = ibeg ; i < iend ; i++ )
                {
                    const SimpFace &f = allFaceVec[i];
                    int ind[4] = { f.ind0, f.ind1, f.ind2, f.ind3 };
                    int n = 3;
                    if( f.m_isQuad )
                    {
                        n = 4;
                    }
                    out += "f ";
                    AppendInts( out, ind, n, ' ' );
                    out += '\n';
                }
            }, &bar );
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
            // The points, the faces, then their tags.
            ProgressStage bar( this, "Write TRI", ( int )( allUsedPntVec.size() + 2 * allFaceVec.size() ) );

            //==== Write Pnt Count and Tri Count ====//
            fprintf( fp, "%d %d\n", ( int )allUsedPntVec.size(), ntristrict );

            //==== Write Pnts ====//
            WriteChunked( fp, ( int )allUsedPntVec.size(), [&]( int ibeg, int iend, string &out )
            {
                for ( int i = ibeg ; i < iend ; i++ )
                {
                    AppendGTriple( out, allUsedPntVec[i].x(), allUsedPntVec[i].y(), allUsedPntVec[i].z(), 16, 10 );
                }
            }, &bar );

            //==== Write Tris ====//
            WriteChunked( fp, ( int )allFaceVec.size(), [&]( int ibeg, int iend, string &out )
            {
                for ( int i = ibeg ; i < iend ; i++ )
                {
                    const SimpFace &f = allFaceVec[i];
                    int t0[3] = { f.ind0, f.ind1, f.ind2 };
                    AppendInts( out, t0, 3, ' ' );
                    out += '\n';

                    if( f.m_isQuad )
                    {
                        int t1[3] = { f.ind0, f.ind2, f.ind3 };
                        AppendInts( out, t1, 3, ' ' );
                        out += '\n';
                    }
                }
            }, &bar );

            //==== Write Component ID ====//
            WriteChunked( fp, ( int )allFaceVec.size(), [&]( int ibeg, int iend, string &out )
            {
                for ( int i = ibeg ; i < iend ; i++ )
                {
                    int ntri = 1;
                    if( allFaceVec[i].m_isQuad )
                    {
                        ntri = 2;
                    }
                    int tag = allTagVec[i];
                    for ( int t = 0 ; t < ntri ; t++ )
                    {
                        FastFormat::AppendInt( out, tag );
                        out += " \n";
                    }
                }
            }, &bar );

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
            ProgressStage bar( this, "Write GMSH", ( int )( allUsedPntVec.size() + allFaceVec.size() ) );

            fprintf( fp, "$MeshFormat\n" );
            fprintf( fp, "2.2 0 %d\n", ( int )sizeof( double ) );
            fprintf( fp, "$EndMeshFormat\n" );

            //==== Write Nodes ====//
            fprintf( fp, "$Nodes\n" );
            fprintf( fp, "%d\n", ( int )allUsedPntVec.size() );
            WriteChunked( fp, ( int )allUsedPntVec.size(), [&]( int ibeg, int iend, string &out )
            {
                for ( int i = ibeg ; i < iend ; i++ )
                {
                    FastFormat::AppendInt( out, i + 1 );
                    out += ' ';
                    AppendFTriple( out, allUsedPntVec[i].x(), allUsedPntVec[i].y(), allUsedPntVec[i].z(), 16, 10 );
                }
            }, &bar );
            fprintf( fp, "$EndNodes\n" );

            //==== Write Tris ====//
            fprintf( fp, "$Elements\n" );
            fprintf( fp, "%d\n", ( int )allFaceVec.size() );

            // The element number is the face's own position in the list, so a chunk can be
            // written without knowing what came before it.
            WriteChunked( fp, ( int )allFaceVec.size(), [&]( int ibeg, int iend, string &out )
            {
                for ( int i = ibeg ; i < iend ; i++ )
                {
                    const SimpFace &f = allFaceVec[i];
                    int ind[4] = { f.ind0, f.ind1, f.ind2, f.ind3 };
                    FastFormat::AppendInt( out, i + 1 );
                    if( f.m_isQuad )
                    {
                        out += " 3 0 ";
                        AppendInts( out, ind, 4, ' ' );
                    }
                    else
                    {
                        out += " 2 0 ";
                        AppendInts( out, ind, 3, ' ' );
                    }
                    out += '\n';
                }
            }, &bar );

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
        int nface = -1;
        if ( allowquads )
        {
            nface = ( int )allFaceVec.size();
        }
        else
        {
            nface = ntristrict;
        }

        FILE* fp = fopen( vspgeom_fn.c_str(), "w" );

        if ( fp )
        {
            // The points, then the faces, their tags, their parents, and the faces and tags again split to tris.
            ProgressStage bar( this, "Write VSPGEOM", ( int )( allUsedPntVec.size() + 5 * allFaceVec.size() ) );

            fprintf( fp, "# vspgeom v3\n" );
            fprintf( fp, "1\n" );  // Number of meshes.
            fprintf( fp, "%d %d %d\n", (int)allUsedPntVec.size(), nface, (int)wakes.size() );

            //==== Write Pnts ====//
            WriteChunked( fp, ( int )allUsedPntVec.size(), [&]( int ibeg, int iend, string &out )
            {
                for ( int i = ibeg ; i < iend ; i++ )
                {
                    AppendGTriple( out, allUsedPntVec[i].x(), allUsedPntVec[i].y(), allUsedPntVec[i].z(), 16, 10 );
                }
            }, &bar );

            if ( allowquads )
            {
                //==== Write Face Count ====//
                fprintf( fp, "%d\n", ( int )allFaceVec.size() );

                //==== Write Faces ====//
                WriteChunked( fp, ( int )allFaceVec.size(), [&]( int ibeg, int iend, string &out )
                {
                    for ( int i = ibeg ; i < iend ; i++ )
                    {
                        const SimpFace &f = allFaceVec[i];
                        int ind[4] = { f.ind0, f.ind1, f.ind2, f.ind3 };
                        if( f.m_isQuad )
                        {
                            out += "4 ";
                            AppendInts( out, ind, 4, ' ' );
                        }
                        else
                        {
                            out += "3 ";
                            AppendInts( out, ind, 3, ' ' );
                        }
                        out += '\n';
                    }
                }, &bar );
            }
            else
            {
                //==== Write Face Count ====//
                fprintf( fp, "%d\n", ntristrict );

                //==== Write Tris Only ====//
                WriteChunked( fp, ( int )allFaceVec.size(), [&]( int ibeg, int iend, string &out )
                {
                    char buf[256];

                    for ( int i = ibeg ; i < iend ; i++ )
                    {
                        snprintf( buf, sizeof( buf ), "3 %d %d %d \n", allFaceVec[i].ind0, allFaceVec[i].ind1, allFaceVec[i].ind2 );
                        out += buf;

                        if( allFaceVec[i].m_isQuad )
                        {
                            snprintf( buf, sizeof( buf ), "3 %d %d %d \n", allFaceVec[i].ind0, allFaceVec[i].ind2, allFaceVec[i].ind3 );
                            out += buf;
                        }
                    }
                }, &bar );
            }

            if ( allowquads )
            {
                //==== Write Component ID ====//
                WriteChunked( fp, ( int )allFaceVec.size(), [&]( int ibeg, int iend, string &out )
                {
                    const int corners[4] = { 0, 1, 2, 3 };

                    for ( int i = ibeg ; i < iend ; i++ )
                    {
                        int tag = allTagVec[i];
                        int part = SubSurfaceMgr.GetPart( *allTagsVec[i] );

                        double uscale = SubSurfaceMgr.m_CompUscale[ part - 1 ];
                        double wscale = SubSurfaceMgr.m_CompWscale[ part - 1 ];

                        int n = 3;
                        if( allFaceVec[i].m_isQuad )
                        {
                            n = 4;
                        }

                        FastFormat::AppendInt( out, part );
                        out += ' ';
                        FastFormat::AppendInt( out, tag );
                        AppendUWCorners( out, &allUWVec[4 * i], corners, n, uscale, wscale );
                        out += '\n';
                    }
                }, &bar );
            }
            else
            {
                //==== Write Component ID ====//
                WriteChunked( fp, ( int )allFaceVec.size(), [&]( int ibeg, int iend, string &out )
                {
                    char buf[256];

                    for ( int i = ibeg ; i < iend ; i++ )
                    {
                        int tag = allTagVec[i];
                        int part = SubSurfaceMgr.GetPart( *allTagsVec[i] );

                        double uscale = SubSurfaceMgr.m_CompUscale[ part - 1 ];
                        double wscale = SubSurfaceMgr.m_CompWscale[ part - 1 ];

                        snprintf( buf, sizeof( buf ), "%d %d %16.10g %16.10g %16.10g %16.10g %16.10g %16.10g\n", part, tag,
                                 allUWVec[4 * i + 0].x() / uscale, allUWVec[4 * i + 0].y() / wscale,
                                 allUWVec[4 * i + 1].x() / uscale, allUWVec[4 * i + 1].y() / wscale,
                                 allUWVec[4 * i + 2].x() / uscale, allUWVec[4 * i + 2].y() / wscale );
                        out += buf;

                        if( allFaceVec[i].m_isQuad )
                        {
                            snprintf( buf, sizeof( buf ), "%d %d %16.10g %16.10g %16.10g %16.10g %16.10g %16.10g\n", part, tag,
                                     allUWVec[4 * i + 0].x() / uscale, allUWVec[4 * i + 0].y() / wscale,
                                     allUWVec[4 * i + 2].x() / uscale, allUWVec[4 * i + 2].y() / wscale,
                                     allUWVec[4 * i + 3].x() / uscale, allUWVec[4 * i + 3].y() / wscale );
                            out += buf;
                        }
                    }
                }, &bar );
            }

            // Write parents
            WriteChunked( fp, nface, [&]( int ibeg, int iend, string &out )
            {
                for ( int i = ibeg ; i < iend ; i++ )
                {
                    FastFormat::AppendInt( out, i + 1 );
                    out += ' ';
                    FastFormat::AppendInt( out, i + 1 );
                    out += '\n';
                }
            }, &bar );

            int nwake = wakes.size();
            // Wake line data.
            fprintf( fp, "%d\n", nwake );

            for ( int iwake = 0; iwake < nwake; iwake++ )
            {

                int iprt = 0;
                int iwe;
                int nwe = wakes[iwake].size();
                fprintf( fp, "%d 0 ", nwe + 1 ); // 0 is place holder for part number.
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
                WriteChunked( fp, ( int )allFaceVec.size(), [&]( int ibeg, int iend, string &out )
                {
                    for ( int i = ibeg ; i < iend ; i++ )
                    {
                        const SimpFace &f = allFaceVec[i];
                        FastFormat::AppendInt( out, i + 1 );
                        if( f.m_isQuad )
                        {
                            int ind[6] = { f.ind0, f.ind1, f.ind2, f.ind0, f.ind2, f.ind3 };
                            out += " 2";
                            for ( int k = 0 ; k < 6 ; k++ )
                            {
                                out += ' ';
                                FastFormat::AppendInt( out, ind[k] );
                            }
                        }
                        else
                        {
                            int ind[3] = { f.ind0, f.ind1, f.ind2 };
                            out += " 1";
                            for ( int k = 0 ; k < 3 ; k++ )
                            {
                                out += ' ';
                                FastFormat::AppendInt( out, ind[k] );
                            }
                        }
                        out += '\n';
                    }
                }, &bar );
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
                WriteChunked( fp, ( int )allFaceVec.size(), [&]( int ibeg, int iend, string &out )
                {
                    const int tricorners[3] = { 0, 1, 2 };
                    const int quadcorners[6] = { 0, 1, 2, 0, 2, 3 };

                    for ( int i = ibeg ; i < iend ; i++ )
                    {
                        int tag = allTagVec[i];
                        int part = SubSurfaceMgr.GetPart( *allTagsVec[i] );

                        double uscale = SubSurfaceMgr.m_CompUscale[ part - 1 ];
                        double wscale = SubSurfaceMgr.m_CompWscale[ part - 1 ];

                        FastFormat::AppendInt( out, i + 1 );
                        out += ' ';
                        FastFormat::AppendInt( out, part );
                        out += ' ';
                        FastFormat::AppendInt( out, tag );
                        if( allFaceVec[i].m_isQuad )
                        {
                            AppendUWCorners( out, &allUWVec[4 * i], quadcorners, 6, uscale, wscale );
                        }
                        else
                        {
                            AppendUWCorners( out, &allUWVec[4 * i], tricorners, 3, uscale, wscale );
                        }
                        out += '\n';
                    }
                }, &bar );
            }
            else
            {
                int iface = 1;
                //==== Write Component ID ====//
                for ( int i = 0 ; i < ( int )allFaceVec.size() ; i++ )
                {
                    int tag = allTagVec[i];
                    int part = SubSurfaceMgr.GetPart( *allTagsVec[i] );

                    double uscale = SubSurfaceMgr.m_CompUscale[ part - 1 ];
                    double wscale = SubSurfaceMgr.m_CompWscale[ part - 1 ];

                    fprintf( fp, "%d %d %d %16.10g %16.10g %16.10g %16.10g %16.10g %16.10g\n", iface, part, tag,
                             allUWVec[4 * i + 0].x() / uscale, allUWVec[4 * i + 0].y() / wscale,
                             allUWVec[4 * i + 1].x() / uscale, allUWVec[4 * i + 1].y() / wscale,
                             allUWVec[4 * i + 2].x() / uscale, allUWVec[4 * i + 2].y() / wscale );
                    iface++;
                    if( allFaceVec[i].m_isQuad )
                    {
                        fprintf( fp, "%d %d %d %16.10g %16.10g %16.10g %16.10g %16.10g %16.10g\n", iface, part, tag,
                                 allUWVec[4 * i + 0].x() / uscale, allUWVec[4 * i + 0].y() / wscale,
                                 allUWVec[4 * i + 2].x() / uscale, allUWVec[4 * i + 2].y() / wscale,
                                 allUWVec[4 * i + 3].x() / uscale, allUWVec[4 * i + 3].y() / wscale );
                        iface++;
                    }
                }
            }

            fclose( fp );
        }

        WriteTagFiles( vspgeom_fn, allFaceVec, allTagsVec, allTagVec, allowquads );

        SubSurfaceMgr.WriteVSPGEOMKeyFile( vspgeom_fn );

        vector < string > gidvec;
        vector < int > partvec;
        vector < int > surfvec;
        SubSurfaceMgr.GetPartData( gidvec, partvec, surfvec );

        vector < string > all_files;
        m_Vehicle->WriteControlSurfaceFile( vspgeom_fn, gidvec, partvec, surfvec, all_files );
    }
}

void CfdMeshMgrSingleton::WriteTagFiles( string file_name, const vector< SimpFace > &allFaceVec,
                                         const vector< const vector < int >* > &allTagsVec, const vector< int > &allTagVec, bool allowquads )
{
    int ntagfile = 0;
    int ncsffile = 0;

    // Every face's tag combo, numbered, so that each file asks about each combo once rather
    // than about every face.  A face is found by its tag, which names its combo, unless the
    // tag is shared or unknown; then by the combo itself.
    vector < vector < int > > combos;
    std::unordered_map < int, int > tagcombo;
    std::set < int > sharedtags;
    {
        std::map < vector < int >, int > tagmap = SubSurfaceMgr.GetSingleTagMap();
        for ( std::map < vector < int >, int >::const_iterator it = tagmap.begin(); it != tagmap.end(); ++it )
        {
            if ( tagcombo.count( it->second ) )
            {
                sharedtags.insert( it->second );
            }
            tagcombo[ it->second ] = ( int )combos.size();
            combos.push_back( it->first );
        }
    }

    int nface = ( int )allFaceVec.size();
    vector < int > facecombo( nface, -1 );
    {
        const int chunkitems = 4096;
        int nchunk = ( nface + chunkitems - 1 ) / chunkitems;
        RunStage( "Tag Files", nchunk, [&]( int c )
        {
            int iend = std::min( nface, ( c + 1 ) * chunkitems );
            for ( int i = c * chunkitems ; i < iend ; i++ )
            {
                int tag = allTagVec[i];
                if ( tag != -1 && !sharedtags.count( tag ) )
                {
                    std::unordered_map < int, int >::const_iterator it = tagcombo.find( tag );
                    if ( it != tagcombo.end() )
                    {
                        facecombo[i] = it->second;
                    }
                }
            }
        } );

        std::map < vector < int >, int > extra;
        for ( int i = 0 ; i < nface ; i++ )
        {
            if ( facecombo[i] < 0 )
            {
                const vector < int > &tags = *allTagsVec[i];
                std::map < vector < int >, int >::const_iterator it = extra.find( tags );
                if ( it == extra.end() )
                {
                    it = extra.insert( std::make_pair( tags, ( int )combos.size() ) ).first;
                    combos.push_back( tags );
                }
                facecombo[i] = it->second;
            }
        }
    }

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

    vector < string > gidvec;
    vector < int > gidpartvec;
    vector < int > gidsurfvec;
    SubSurfaceMgr.GetPartData( gidvec, gidpartvec, gidsurfvec );

    int nhingefile = 0;
    vector < string > hinges;
    vector < vector < int > > hingedescendantparts;
    Vehicle *veh = VehicleMgr.GetVehicle();
    if ( veh )
    {
        // Check all geoms, whether they are in mesh or not.
        std::vector< std::string > comps = veh->GetGeomVec();
        for ( int icomp = 0; icomp < comps.size(); icomp++ )
        {
            Geom * g = veh->FindGeom( comps[icomp] );
            if ( g )
            {
                // A Clone of a hinge articulates its children the same way, so it is a
                // control surface too.
                if ( Geom::CastTo< JointRole >( g ) )
                {
                    ntagfile++;
                    nhingefile++;

                    hinges.push_back( comps[icomp] );

                    vector < string > descendants;
                    g->BuildRigidAttachedDescendantList( descendants );

                    vector < int > descpart;
                    for ( int ides = 0; ides < descendants.size(); ides++ )
                    {
                        vector < int > indvec;
                        vector_find_val_multiple( gidvec, descendants[ ides ], indvec );

                        for ( int iind = 0; iind < indvec.size(); iind++ )
                        {
                            descpart.push_back( gidpartvec[ indvec[ iind ] ] );
                        }
                    }

                    hingedescendantparts.push_back( descpart );
                }
            }
        }
    }

    if ( ntagfile > 0 )
    {
        string base_name = GetBasename( file_name );

        string base_path, base_fname;
        GetPathFile( base_name, base_path, base_fname );

        string subdir = base_path + "/" + "TaggedRegions.OpenVSP/";
        MakeDirectory( subdir );

        string taglist_name = subdir + base_fname + ".ALL.taglist";
        string csf_taglist_name = subdir + base_fname + ".ControlSurfaces.taglist";
        string hinge_taglist_name = subdir + base_fname + ".Hinges.taglist";

        StringUtil::change_space_to_underscore( base_fname );
        string base_path_nospace = subdir + base_fname;

        FILE* taglist_fid = fopen( taglist_name.c_str(), "w" );
        FILE* csf_taglist_fid = NULL;
        if ( ncsffile > 0 )
        {
            csf_taglist_fid = fopen( csf_taglist_name.c_str(), "w" );
        }

        FILE* hinge_taglist_fid = nullptr;
        if ( nhingefile > 0 )
        {
            hinge_taglist_fid = fopen( hinge_taglist_name.c_str(), "w" );
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
                        string gname;
                        string sname;
                        StringUtil::split_comp_tag_name( str, gname, sname );

                        StringUtil::change_space_to_underscore( gname );
                        StringUtil::change_space_to_underscore( sname );

                        string tname = SubSurfaceMgr.m_TagNames[tag];
                        StringUtil::change_space_to_underscore( tname );

                        string ptagname = gname + sname + "_" + tname;

                        string tagfile_name = base_path_nospace + "." + ptagname + ".tag";
                        string tagfile_localname = base_fname + "." + ptagname;

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
                            vector < char > match( combos.size() );
                            for ( int k = 0 ; k < ( int )combos.size() ; k++ )
                            {
                                match[k] = SubSurfaceMgr.MatchPartAndTag( combos[k], part, tag );
                            }

                            WriteTagFile( fid, match, facecombo, allFaceVec, allowquads );

                            fclose( fid );
                        }
                    }
                }
            }
            if ( csf_taglist_fid )
            {
                fclose( csf_taglist_fid );
            }

            if ( hinge_taglist_fid )
            {
                fprintf( hinge_taglist_fid, "%d\n", nhingefile );
            }

            for ( int ihinge = 0; ihinge < nhingefile; ihinge++ )
            {
                Geom * g = veh->FindGeom( hinges[ ihinge ] );
                if ( g )
                {
                    string hingename = g->GetName() + "_Hinge";
                    // The taglist records the space-substituted name, so the file has to be
                    // written under that same name -- and under the space-substituted path --
                    // or the reader cannot find it.  This is what the part tags above do.
                    StringUtil::change_space_to_underscore( hingename );

                    string tagfile_name = base_path_nospace + "." + hingename + ".tag";
                    string tagfile_localname = base_fname + "." + hingename;

                    fprintf( taglist_fid, "%s\n", tagfile_localname.c_str() );

                    if ( hinge_taglist_fid )
                    {
                        fprintf( hinge_taglist_fid, "%s\n", tagfile_localname.c_str() );
                    }

                    FILE* fid = fopen( tagfile_name.c_str(), "w" );
                    if ( fid )
                    {
                        vector < char > match( combos.size() );
                        for ( int k = 0 ; k < ( int )combos.size() ; k++ )
                        {
                            match[k] = SubSurfaceMgr.MatchAnyPart( combos[k], hingedescendantparts[ ihinge ] );
                        }

                        WriteTagFile( fid, match, facecombo, allFaceVec, allowquads );

                        fclose( fid );
                    }
                }
            }
            if ( hinge_taglist_fid )
            {
                fclose( hinge_taglist_fid );
            }

            fclose( taglist_fid );
        }
        else
        {
            // These two were opened before the block above, so they have to be closed
            // even when the main list could not be opened and that block never ran.
            if ( csf_taglist_fid )
            {
                fclose( csf_taglist_fid );
            }
            if ( hinge_taglist_fid )
            {
                fclose( hinge_taglist_fid );
            }
        }
    }
}

// The faces whose combo is matched, by number, after their count.
void CfdMeshMgrSingleton::WriteTagFile( FILE* file_id, const vector < char > &match, const vector < int > &faceCombo,
                                        const vector< SimpFace > &allFaceVec, bool allowquads )
{
    //==== Write Tri IDs for each tag =====//
    string out;
    int count = 0;
    int iface = 0;

    for ( int i = 0 ; i < ( int )allFaceVec.size() ; i++ )
    {
        int ncopy = 1;
        if ( !allowquads && allFaceVec[i].m_isQuad )
        {
            ncopy = 2;
        }

        for ( int k = 0 ; k < ncopy ; k++ )
        {
            if ( match[ faceCombo[i] ] )
            {
                FastFormat::AppendInt( out, iface + 1 );
                out += '\n';
                count++;
            }
            iface++;
        }
    }

    fprintf( file_id, "%d\n\n", count );
    fwrite( out.data(), 1, out.size(), file_id );
}

void CfdMeshMgrSingleton::WriteFacet( const string &facet_fn )
{
    // Note: Wake mesh not included in Facet export

    //==== Find All Tri Counts ====//
    size_t nallface = 0;
    vector < bool > notwake( m_SurfVec.size() );
    for ( int i = 0 ; i < ( int )m_SurfVec.size() ; i++ )
    {
        notwake[i] = !m_SurfVec[i]->GetWakeFlag();
        nallface += m_SurfVec[i]->GetMesh()->GetSimpFaceVec().size();
    }

    //==== Merge the points of every surface but the wakes ====//
    vector < vector < int > > allPntKey;
    vector< vec3d > allUsedPntVec;
    MergeSurfPnts( notwake, allPntKey, allUsedPntVec, nullptr );

    //==== Assemble Normal Tris ====//
    // Each face's tags are pointed to, not copied.
    vector< SimpFace > allFaceVec;
    allFaceVec.reserve( nallface );
    vector< const vector < int >* > allTagsVec;
    allTagsVec.reserve( nallface );
    for ( int i = 0; i < (int)m_SurfVec.size(); i++ )
    {
        if ( !m_SurfVec[i]->GetWakeFlag() )
        {
            vector < SimpFace >& sFaceVec = m_SurfVec[ i ]->GetMesh()->GetSimpFaceVec();
            vector< vec3d >& sPntVec = m_SurfVec[i]->GetMesh()->GetSimpPntVec();
            for ( int f = 0; f < (int)sFaceVec.size(); f++ )
            {
                int i0 = allPntKey[i][sFaceVec[f].ind0];
                int i1 = allPntKey[i][sFaceVec[f].ind1];
                int i2 = allPntKey[i][sFaceVec[f].ind2];
                SimpFace sface;
                sface.ind0 = i0 + 1;
                sface.ind1 = i1 + 1;
                sface.ind2 = i2 + 1;
                allTagsVec.push_back( &sFaceVec[f].m_Tags );
                if ( sFaceVec[f].m_isQuad )
                {
                    sface.m_isQuad = true;
                    int i3 = allPntKey[i][sFaceVec[f].ind3];
                    sface.ind3 = i3 + 1;
                }
                allFaceVec.push_back( sface );
            }
        }
    }
    // The faces are numbered now, so the map is done with.
    vector < vector < int > >().swap( allPntKey );

    //=====================================================================================//
    //==== Write Facet File for Xpatch ====================================================//
    //=====================================================================================//
    if ( facet_fn.length() != 0 )
    {
        FILE* fp = fopen( facet_fn.c_str(), "w" );

        if ( fp )
        {
            ProgressStage bar( this, "Write FACET", ( int )( allUsedPntVec.size() + allFaceVec.size() ) );

            fprintf( fp, "Exported from %s\n", VSPVERSION4 ); // Title/comment line
            fprintf( fp, "1 \n" ); // Number of "Big" parts (1 Vehicle broken into small parts by geom and subsurface)

            fprintf( fp, "%s\n", m_Vehicle->GetName().c_str() ); // Name of "Big" part

            // mirror -> i, a b c d
            //     if i = 0 -> no mirror
            //     if i = 1 -> "Big" part is mirrored across plane defined by ax+by+cz-d=0
            fprintf( fp, "0, 0.000 1.000 0.000 0.000 \n" );

            fprintf( fp, "%d \n", (int)allUsedPntVec.size() ); // # of nodes in "Big" part

            //==== Write All Pnts (Nodes) ====//
            WriteChunked( fp, ( int )allUsedPntVec.size(), [&]( int ibeg, int iend, string &out )
            {
                for ( int i = ibeg ; i < iend ; i++ )
                {
                    AppendGTriple( out, allUsedPntVec[i].x(), allUsedPntVec[i].y(), allUsedPntVec[i].z(), 16, 10 );
                }
            }, &bar );

            vector < int > face_offset; // vector of number of faces for each tag

            int materialID = 0; // Default Material ID of PEC (Referred to as "iCoat" in XPatch facet file documentation)

            vector < int > all_tag_vec = SubSurfaceMgr.GetAllTags(); // vector of tags, where each tag identifies a part or group of facets

            // The faces of each part, in the order they come, each face's tag looked up once.
            vector < vector < int > > part_faces;
            GroupFacesByTag( allTagsVec, all_tag_vec, part_faces );

            //==== Get # of facets for each part ====//
            for ( unsigned int i = 0; i < all_tag_vec.size(); i++ )
            {
                int tag_count = 0;

                for ( unsigned int k = 0; k < part_faces[i].size(); k++ )
                {
                    tag_count++;

                    // Count extra tri as quads will be split.
                    if( allFaceVec[ part_faces[i][k] ].m_isQuad )
                    {
                        tag_count++;
                    }
                }

                face_offset.push_back( tag_count );
            }

            fprintf( fp, "%zu \n", face_offset.size() ); // # of "Small" parts

            int facet_count = 0; // counter for number of tris/facets

                                 //==== Write Out Tris ====//
            for ( unsigned int i = 0; i < all_tag_vec.size(); i++ )
            {
                const vector < int > &faces = part_faces[i];

                if ( faces.empty() )
                {
                    continue;
                }

                // Write small part header
                string name = SubSurfaceMgr.GetTagNames( *allTagsVec[ faces[0] ] );
                fprintf( fp, "%s\n", name.c_str() ); // Write name of small part
                fprintf( fp, "%d 3\n", face_offset[i] ); // Number of facets for the part, 3 nodes per facet

                // The running facet number each face starts from, so a chunk can be written
                // without counting the faces before it.
                vector < int > first_facet( faces.size() );
                for ( unsigned int k = 0; k < faces.size(); k++ )
                {
                    first_facet[k] = facet_count + 1;
                    facet_count++;
                    if( allFaceVec[ faces[k] ].m_isQuad )
                    {
                        facet_count++;
                    }
                }

                WriteChunked( fp, ( int )faces.size(), [&]( int ibeg, int iend, string &out )
                {
                    for ( int k = ibeg; k < iend; k++ )
                    {
                        const SimpFace &f = allFaceVec[ faces[k] ];

                        // 3 nodes of facet, material ID, component ID, running facet #:
                        int t0[6] = { f.ind0, f.ind1, f.ind2, materialID, ( int )( i + 1 ), first_facet[k] };
                        AppendInts( out, t0, 5, ' ' );
                        FastFormat::AppendInt( out, t0[5] );
                        out += '\n';

                        // Split quad into two tris.  It may be possible to write out tris and quads separately by
                        // grouping them appropriately and then writing separate groups.  However, without documentation
                        // of the facet file format, this approach is the least risky.
                        if( f.m_isQuad )
                        {
                            int t1[6] = { f.ind0, f.ind2, f.ind3, materialID, ( int )( i + 1 ), first_facet[k] + 1 };
                            AppendInts( out, t1, 5, ' ' );
                            FastFormat::AppendInt( out, t1[5] );
                            out += '\n';
                        }
                    }
                }, &bar );
            }
            fclose( fp );
        }
    }
}

// How much of a surface's parameter space the mesh actually covers.  Which of a face's
// loops bounds it is decided against this.
static double MeshAreaUW( Surf* srf )
{
    if ( !srf || !srf->GetMesh() )
    {
        return 0.0;
    }

    const vector < vec2d > &uw_vec = srf->GetMesh()->GetSimpUWPntVec();
    const vector < SimpFace > &face_vec = srf->GetMesh()->GetSimpFaceVec();

    double area = 0.0;

    for ( int i = 0; i < ( int )face_vec.size(); i++ )
    {
        const SimpFace &f = face_vec[i];

        int ind[4] = { f.ind0, f.ind1, f.ind2, f.ind3 };
        int ntri = 1;
        if ( f.m_isQuad )
        {
            ntri = 2;
        }

        for ( int t = 0; t < ntri; t++ )
        {
            const vec2d &p0 = uw_vec[ ind[0] ];
            const vec2d &p1 = uw_vec[ ind[ t + 1 ] ];
            const vec2d &p2 = uw_vec[ ind[ t + 2 ] ];

            area += 0.5 * std::fabs( ( p1.x() - p0.x() ) * ( p2.y() - p0.y() ) -
                                     ( p2.x() - p0.x() ) * ( p1.y() - p0.y() ) );
        }
    }

    return area;
}

// Sense of every curve of a loop as that loop's face sees it, in the convention the
// topology file wants: +1 when the face lies to the left of the curve walked in its
// own direction, -1 when it lies to the right.
static void AccumulateLoopSense( const NURBS_Loop &loop, int surf_id, bool cutout_flag, bool flip_flag,
                                 std::map < int, std::map < int, int > > &curve_face_sense )
{
    int loop_sense = loop.Sense( surf_id, cutout_flag, flip_flag );

    if ( loop_sense == 0 )
    {
        return;
    }

    for ( int i = 0; i < ( int )loop.m_OrderedCurves.size(); i++ )
    {
        int sense = loop_sense;
        if ( !loop.m_OrderedCurves[i].second )
        {
            sense = -sense;
        }

        curve_face_sense[ loop.m_OrderedCurves[i].first.m_CurveID ][ surf_id ] = sense;
    }
}

// Face number and edge sense that one parent surface of a curve contributes to a
// topology record.  Both come back zero when that surface never became a face.
static void GetTopoEdgeFace( int surf_id, const std::map < int, int > &surf_id_face_num,
                             const std::map < int, std::map < int, int > > &curve_face_sense,
                             int curve_id, int &face_num, int &sense )
{
    face_num = 0;
    sense = 0;

    std::map < int, int >::const_iterator nit = surf_id_face_num.find( surf_id );
    if ( nit == surf_id_face_num.end() )
    {
        return;
    }

    face_num = nit->second;

    std::map < int, std::map < int, int > >::const_iterator cit = curve_face_sense.find( curve_id );
    if ( cit == curve_face_sense.end() )
    {
        return;
    }

    std::map < int, int >::const_iterator sit = cit->second.find( surf_id );
    if ( sit != cit->second.end() )
    {
        sense = sit->second;
    }
}

// Report the face only when its sense is known.  A surface can touch a curve without
// the curve bounding its trimmed region -- MakeExtLoopVec ignores some loops for
// trimming -- and a face index paired with no sense would be worse than none.
static void GetTopoEdgeFaceChecked( int surf_id, const std::map < int, int > &surf_id_face_num,
                                    const std::map < int, std::map < int, int > > &curve_face_sense,
                                    int curve_id, int &face_num, int &sense )
{
    GetTopoEdgeFace( surf_id, surf_id_face_num, curve_face_sense, curve_id, face_num, sense );

    if ( sense == 0 )
    {
        face_num = 0;
    }
}

// Tessellation lines for one direction of a surface.  Surfaces the mesher builds for
// itself -- the far field box and the symmetry plane -- have no parent Geom to inherit
// lines from, so they fall back to a plain subdivision of their own range.
static void POGSTess( const vector < double > &tess, double lo, double hi, vector < double > &out )
{
    out = tess;

    if ( out.size() >= 2 )
    {
        return;
    }

    out.clear();
    for ( int i = 0; i < 5; i++ )
    {
        out.push_back( lo + ( hi - lo ) * i / 4.0 );
    }
}

// Whether a point lies inside a component other than its own, which is what makes a
// sample of an untrimmed surface off the geometry.  This is the test RemoveInteriorTris
// runs on triangle centers: shoot a ray and count how many times it crosses each other
// component; an odd count means it started inside that one.
//
// The test is made against the model rather than against the mesh of the surface being
// sampled, because a surface's parameterization does not always cover it evenly -- the
// mesher collapses a degenerate row, such as the nose of a Pod, to a single node, and
// every sample along that row is really the same point on the geometry.
// Would trimming have removed a sample here?
//
// The surfaces in these files are not trimmed, so a sample the mesher would have thrown away is
// marked off the geometry with an iblank of zero instead.  That is the question
// RemoveInteriorTris asks of a triangle's centre, and it is answered the same way: cast a ray,
// count the crossings of each other component, and put the answer to SetDeleteTriFlag, which
// knows what being inside something means for a surface of this kind.
//
// Asking only whether the point is inside anything at all is right for a positive skin and
// backwards for a negative volume or a transparent disk, whose surface is kept exactly where it
// lies inside the body it cuts.
//
// t_vec_vec is the caller's scratch, sized to hold every component; it is cleared here.  It
// belongs to the caller because this is asked once per sample.
bool CfdMeshMgrSingleton::PntTrimmedAway( const vec3d &pnt, Surf *srf, double x_dist,
                                          vector < vector < double > > &t_vec_vec )
{
    for ( int c = 0 ; c < ( int )t_vec_vec.size() ; c++ )
    {
        t_vec_vec[c].clear();
    }

    int s_comp_id = srf->GetCompID();

    vec3d p0 = pnt;
    vec3d p1 = pnt + vec3d( x_dist, 1.0e-4, 1.0e-4 );

    for ( int i = 0 ; i < ( int )m_SurfVec.size() ; i++ )
    {
        int comp_id = m_SurfVec[i]->GetCompID();

        if ( m_SurfVec[i] == srf || comp_id == s_comp_id ||
             comp_id < 0 || comp_id >= ( int )t_vec_vec.size() )
        {
            continue;
        }

        if ( srf->GetFeaSymmIndex() >= 0 && m_SurfVec[i]->GetFeaSymmIndex() >= 0 &&
             srf->GetFeaSymmIndex() != m_SurfVec[i]->GetFeaSymmIndex() )
        {
            continue;
        }

        int itype = m_SurfVec[i]->GetSurfaceCfdType();

        // Transparent, structure and stiffener surfaces do not bound a volume, so they cannot
        // put a point inside anything -- unless the far field is trimming a symmetry plane.
        if ( itype != vsp::CFD_TRANSPARENT && itype != vsp::CFD_STRUCTURE &&
             itype != vsp::CFD_STIFFENER )
        {
            m_SurfVec[i]->IntersectLineSeg( p0, p1, t_vec_vec[comp_id] );
        }
        else if ( m_SurfVec[i]->GetFarFlag() && srf->GetSymPlaneFlag() &&
                  GetSettingsPtr()->m_FarCompFlag )
        {
            m_SurfVec[i]->IntersectLineSeg( p0, p1, t_vec_vec[comp_id] );
        }
    }

    vector < bool > inside( t_vec_vec.size(), false );

    for ( int i = 0 ; i < ( int )m_SurfVec.size() ; i++ )
    {
        int c = m_SurfVec[i]->GetCompID();

        if ( c < 0 || c >= ( int )inside.size() )
        {
            continue;
        }

        if ( srf->GetSymPlaneFlag() && m_SurfVec[i]->GetFarFlag() &&
             GetSettingsPtr()->m_FarCompFlag )
        {
            // The symmetry plane is kept where it is inside the outer boundary, which is the
            // other way round from everything else.
            if ( ( int )( t_vec_vec[c].size() + 1 ) % 2 == 1 )
            {
                inside[c] = true;
            }
        }
        else if ( ( int )t_vec_vec[c].size() % 2 == 1 )
        {
            inside[c] = true;
        }
    }

    vector< int > joined;
    JoinedComps( s_comp_id, joined );
    if ( !joined.empty() )
    {
        BndBox big_box;
        for ( int i = 0 ; i < ( int )m_SurfVec.size() ; i++ )
        {
            big_box.Update( m_SurfVec[i]->GetBBox() );
        }
        double ray_len = 1.0 + big_box.DiagDist();

        for ( int i = 0 ; i < ( int )joined.size() ; i++ )
        {
            int c = joined[i];
            if ( c >= 0 && c < ( int )inside.size() )
            {
                inside[c] = InsideByThreeRays( c, pnt, ray_len );
            }
        }
    }

    int pairc = -1;
    bool pairinside = false;
    CoincidentVerdict( srf, pairc, pairinside );

    if ( pairc >= 0 && pairc < ( int )inside.size() )
    {
        inside[ pairc ] = pairinside;
    }

    return SetDeleteTriFlag( srf->GetSurfaceCfdType(), srf->GetSymPlaneFlag(), inside );
}

// Whether uw lies inside polygon poly, by crossings of a ray toward +u.
static bool PolyContainsUW( const vector < vec3d > &poly, const vec3d &uw )
{
    bool inside = false;
    for ( int i = 0; i < ( int )poly.size(); i++ )
    {
        const vec3d &p0 = poly[i];
        const vec3d &p1 = poly[ ( i + 1 ) % poly.size() ];
        if ( ( p0.y() > uw.y() ) != ( p1.y() > uw.y() ) )
        {
            double u = p0.x() + ( uw.y() - p0.y() ) * ( p1.x() - p0.x() ) / ( p1.y() - p0.y() );
            if ( uw.x() < u )
            {
                inside = !inside;
            }
        }
    }
    return inside;
}

// The box of points a and b, grown by r on every side.
static void GrownBox( const vec3d &a, const vec3d &b, double r, vec3d &lo, vec3d &hi )
{
    for ( int k = 0; k < 3; k++ )
    {
        lo.v[k] = std::min( a.v[k], b.v[k] ) - r;
        hi.v[k] = std::max( a.v[k], b.v[k] ) + r;
    }
}

// Whether p lies outside the box lo hi.
static bool OutsideBox( const vec3d &p, const vec3d &lo, const vec3d &hi )
{
    return p.x() < lo.x() || p.x() > hi.x() || p.y() < lo.y() || p.y() > hi.y() || p.z() < lo.z() || p.z() > hi.z();
}

// The uw box of loop poly.  A point that is not a number leaves the box unbounded.
static void LoopBox( const vector < vec3d > &poly, vec3d &lo, vec3d &hi )
{
    lo = vec3d( 1.0e300, 1.0e300, 0.0 );
    hi = vec3d( -1.0e300, -1.0e300, 0.0 );
    for ( int k = 0; k < ( int )poly.size(); k++ )
    {
        if ( std::isnan( poly[k].x() ) || std::isnan( poly[k].y() ) )
        {
            lo = vec3d( -HUGE_VAL, -HUGE_VAL, 0.0 );
            hi = vec3d( HUGE_VAL, HUGE_VAL, 0.0 );
            return;
        }
        lo.set_x( std::min( lo.x(), poly[k].x() ) );
        lo.set_y( std::min( lo.y(), poly[k].y() ) );
        hi.set_x( std::max( hi.x(), poly[k].x() ) );
        hi.set_y( std::max( hi.y(), poly[k].y() ) );
    }
}

// Whether PolyContainsUW can find uw inside a loop of box lo hi.  Above or below the box no edge
// crosses the ray; past its far side none lies ahead of the point, and before its near side every
// crossing does, which around a closed loop is an even number of them.  slack covers the rounding
// of where an edge crosses.
static bool InLoopBox( const vec3d &lo, const vec3d &hi, double slack, const vec3d &uw )
{
    return uw.y() >= lo.y() && uw.y() <= hi.y() && uw.x() >= lo.x() - slack && uw.x() <= hi.x() + slack;
}

// A margin well past the rounding of a point projected onto a segment among points no bigger
// than those in pnt_vec.
static double RoundingSlack( const vector < vec3d > &pnt_vec )
{
    double big = 1.0;
    for ( int i = 0; i < ( int )pnt_vec.size(); i++ )
    {
        for ( int k = 0; k < 3; k++ )
        {
            big = std::max( big, std::fabs( pnt_vec[i].v[k] ) );
        }
    }
    return 1.0e-9 * big;
}

// How far, as a fraction of its uw length, a loop edge may stray from the image of its xyz chord
// before it is refined, and so how near an edge a sample is put to the side test.
static const double UW_REFINE_TOL = 0.01;

// Whether the edge q0 q1 runs along one of the face's parameter bounds.
static bool OnFaceBound( const vec3d &q0, const vec3d &q1, double umin, double umax, double wmin, double wmax )
{
    if ( q0.x() == q1.x() && ( q0.x() == umin || q0.x() == umax ) )
    {
        return true;
    }
    if ( q0.y() == q1.y() && ( q0.y() == wmin || q0.y() == wmax ) )
    {
        return true;
    }
    return false;
}

// Points between a and b, in uw, of the curve whose xyz chord is the image of a b: the xyz
// midpoint projected onto the surface, added where it falls off the uw midpoint.
static void RefineEdgeUW( Surf *srf, const vec3d &a, const vec3d &b, int depth, vector < vec3d > &out )
{
    if ( depth >= 6 )
    {
        return;
    }
    vec3d mid_uw = 0.5 * ( a + b );
    vec3d mid_xyz = 0.5 * ( srf->CompPnt( a.x(), a.y() ) + srf->CompPnt( b.x(), b.y() ) );
    vec2d p = srf->ClosestUW( mid_xyz, mid_uw.x(), mid_uw.y() );
    vec3d m( p[0], p[1], 0.0 );
    if ( dist( m, mid_uw ) <= UW_REFINE_TOL * dist( a, b ) )
    {
        return;
    }
    RefineEdgeUW( srf, a, m, depth + 1, out );
    out.push_back( m );
    RefineEdgeUW( srf, m, b, depth + 1, out );
}

// The loop polygon poly with each edge off the face's bounds refined to follow its curve.  other and
// guess, given per point, become per edge: the surface across the edge and a uw on it near the edge.
// band gets, for each edge of the result, how far the curve may lie from it in xyz: twice rel_tol of
// the length of the segment it came from, the adaptation's tolerance on that segment with room for one
// the adaptation left over it.  Zero along borders and the face's bounds, which are exact.
static void RefineLoopUW( Surf *srf, double umin, double umax, double wmin, double wmax, double rel_tol,
                          vector < vec3d > &poly, vector < int > &other, vector < vec3d > &guess, vector < double > &band )
{
    vector < vec3d > out, out_guess;
    vector < int > out_other;
    band.clear();
    int n = poly.size();
    for ( int k = 0; k < n; k++ )
    {
        const vec3d &q0 = poly[k];
        const vec3d &q1 = poly[ ( k + 1 ) % n ];
        out.push_back( q0 );
        size_t first = out.size();
        double b = 0.0;
        if ( other[k] >= 0 && !OnFaceBound( q0, q1, umin, umax, wmin, wmax ) )
        {
            RefineEdgeUW( srf, q0, q1, 0, out );
            b = 2.0 * rel_tol * dist( srf->CompPnt( q0.x(), q0.y() ), srf->CompPnt( q1.x(), q1.y() ) );
        }
        for ( size_t m = first - 1; m < out.size(); m++ )
        {
            band.push_back( b );
            out_other.push_back( other[k] );
            out_guess.push_back( guess[k] );
        }
    }
    poly = out;
    other = out_other;
    guess = out_guess;
}

// A loop as a polygon in the parameters of surface surf_id, as NURBS_Loop::GetUWPolygon builds it,
// with, at each point, the id of the curve's other parent and the point's uw on that one.  The id is
// -1 along a border curve, which runs exactly where the surface ends.
static void LoopUWAcross( const NURBS_Loop &loop, int surf_id, vector < vec3d > &uw, vector < int > &other, vector < vec3d > &guess )
{
    uw.clear();
    other.clear();
    guess.clear();
    for ( int i = 0; i < ( int )loop.m_OrderedCurves.size(); i++ )
    {
        const NURBS_Curve &crv = loop.m_OrderedCurves[i].first;
        int ida = crv.m_SurfA_ID;
        int idb = crv.m_SurfB_ID;
        if ( crv.m_BorderFlag )
        {
            ida = -1;
            idb = -1;
        }
        if ( crv.m_SurfA_ID == surf_id )
        {
            uw.insert( uw.end(), crv.m_UWPntVec_A.begin(), crv.m_UWPntVec_A.end() );
            other.insert( other.end(), crv.m_UWPntVec_A.size(), idb );
            guess.insert( guess.end(), crv.m_UWPntVec_B.begin(), crv.m_UWPntVec_B.end() );
        }
        else if ( crv.m_SurfB_ID == surf_id )
        {
            uw.insert( uw.end(), crv.m_UWPntVec_B.begin(), crv.m_UWPntVec_B.end() );
            other.insert( other.end(), crv.m_UWPntVec_B.size(), ida );
            guess.insert( guess.end(), crv.m_UWPntVec_A.begin(), crv.m_UWPntVec_A.end() );
        }
    }
}

// The normal of srf at u, w pointing out of the body it bounds.  The mesher flips a negative
// surface's normal to face the fluid inside it, so that flip is undone here.
static vec3d OutwardNorm( Surf *srf, double u, double w )
{
    vec3d n = srf->CompNorm( u, w );
    bool flip = srf->GetFlipFlag();
    if ( srf->GetSurfaceCfdType() == vsp::CFD_NEGATIVE )
    {
        flip = !flip;
    }
    if ( flip )
    {
        n = -1.0 * n;
    }
    return n;
}

// Whether normals ns and n are parallel, to within a few degrees.
static bool NormsTangent( const vec3d &ns, const vec3d &n )
{
    double m = ns.mag() * n.mag();
    if ( m <= 0.0 )
    {
        return false;
    }
    return std::abs( dot( ns, n ) ) > 0.99 * m;
}

// Whether srf at uw has a normal parallel to n, to within a few degrees.
static bool SurfacesTangent( Surf *srf, const vec3d &uw, const vec3d &n )
{
    return NormsTangent( srf->CompNorm( uw.x(), uw.y() ), n );
}

// A POGS sample near an intersection edge, whose side of the edge is taken from the surface
// across it.  What needs the sample's own surface is worked out with that surface's face; what
// is left needs only the surface across.
struct POGSCrossSample
{
    int m_Face;
    int m_Index;            // The sample, in its face
    Surf* m_Srf;
    Surf* m_Other;          // The surface across the edge, of component m_OtherComp
    int m_OtherComp;
    vec3d m_Pnt;
    vec3d m_Guess;          // A uw on m_Other near the edge
    double m_Reach;         // Farther than this, m_Other's nearest point is not across the edge
    vec3d m_Norm;           // m_Srf's normal at the sample
    bool m_SameSide;        // The sample is on the edge's kept side of the polygon
    bool m_Del;             // The polygon's verdict
};

// A CART3D triangulation, unformatted, the way pogs reads it:
//
//     WRITE(IU) NVERT,NFACE
//     WRITE(IU) (X(I),Y(I),Z(I),I=1,NVERT)        coordinates interleaved
//     WRITE(IU) (IFACE(I,1:3),I=1,NFACE)
//     WRITE(IU) (ICOMP(I),I=1,NFACE)
//
// Coordinates go out single precision -- these files are single by convention, and the
// routine that normally writes them casts down to REAL even in a double precision build.
//
// conn_vec holds three vertex numbers per triangle and comp_vec one component tag.
static bool WriteCart3DTri( const string &fn, const vector < vec3d > &pnt_vec,
                            const vector < int > &conn_vec, const vector < int > &comp_vec )
{
    UnformattedOut fp;

    if ( !fp.Open( fn ) )
    {
        return false;
    }

    fp.SetSinglePrecision( true );

    fp.BeginRecord();
    fp.Write( ( int )pnt_vec.size() );
    fp.Write( ( int )comp_vec.size() );
    fp.EndRecord();

    fp.BeginRecord();
    for ( int i = 0; i < ( int )pnt_vec.size(); i++ )
    {
        fp.Write( pnt_vec[i].x() );
        fp.Write( pnt_vec[i].y() );
        fp.Write( pnt_vec[i].z() );
    }
    fp.EndRecord();

    fp.WriteRecord( conn_vec );
    fp.WriteRecord( comp_vec );

    return true;
}

// Write a Plot3D grid file.  pogs reads these unformatted, the way egads2srf writes
// them, so each Fortran WRITE becomes one record and the records have to line up with
// the READ statements on the other side.
//
//     WRITE(IU) NBLOCK
//     WRITE(IU) (NI(N),NJ(N),1,N=1,NBLOCK)
//     WRITE(IU) X,Y,Z[,IB]        one record per block
//
// iblank_vec is left empty for a file that carries no tags, as the curve file does not.
static bool WritePlot3DGrid( const string &fn, const vector < vector < int > > &dim_vec,
                             const vector < vector < vec3d > > &pnt_vec,
                             const vector < vector < int > > &iblank_vec )
{
    UnformattedOut fp;

    if ( !fp.Open( fn ) )
    {
        return false;
    }

    int nblock = dim_vec.size();

    fp.WriteRecord( nblock );

    fp.BeginRecord();
    for ( int i = 0; i < nblock; i++ )
    {
        fp.Write( dim_vec[i] );
    }
    fp.EndRecord();

    for ( int i = 0; i < nblock; i++ )
    {
        fp.BeginRecord();

        for ( int k = 0; k < 3; k++ )
        {
            for ( int j = 0; j < ( int )pnt_vec[i].size(); j++ )
            {
                fp.Write( pnt_vec[i][j].v[k] );
            }
        }

        if ( i < ( int )iblank_vec.size() )
        {
            fp.Write( iblank_vec[i] );
        }

        fp.EndRecord();
    }

    return true;
}

// Write a Plot3D function file.  It is laid out like a grid file, except that each
// block's dimensions are followed by the number of variables the block carries, and the
// data is that many arrays rather than an assumed three.
//
//     WRITE(IU) NBLOCK
//     WRITE(IU) (NI(N),NJ(N),1,NVAR,N=1,NBLOCK)
//     WRITE(IU) Q                 one record per block
static bool WritePlot3DFunction( const string &fn, const vector < vector < int > > &dim_vec,
                                 const vector < vector < vector < double > > > &var_vec )
{
    UnformattedOut fp;

    if ( !fp.Open( fn ) )
    {
        return false;
    }

    int nblock = dim_vec.size();

    fp.WriteRecord( nblock );

    fp.BeginRecord();
    for ( int i = 0; i < nblock; i++ )
    {
        fp.Write( dim_vec[i] );
        fp.Write( ( int )var_vec[i].size() );
    }
    fp.EndRecord();

    for ( int i = 0; i < nblock; i++ )
    {
        fp.BeginRecord();
        for ( int v = 0; v < ( int )var_vec[i].size(); v++ )
        {
            fp.Write( var_vec[i][v] );
        }
        fp.EndRecord();
    }

    return true;
}

// Write the geometric component file, which names the faces in human readable groups.
// A component here is one surface of one Geom, named for the Geom, the surface's number
// within it and the Geom's ID, so a Geom named Wing with a symmetric pair contributes
// Wing_0_<ID> and Wing_1_<ID>, each owning the faces its surface was split into.
//
// The face list accepts ranges as well as single entries; single entries are written
// throughout, which is always correct whatever order the faces came out in.
bool CfdMeshMgrSingleton::WritePOGSCompFile( const string &fn, const vector < int > &face_surf_vec )
{
    FILE* fp = fopen( fn.c_str(), "w" );

    if ( !fp )
    {
        return false;
    }

    // One component per surface of each Geom, keyed on its name, which holds the Geom's ID.  The
    // Geom's name alone will not do -- nothing stops two Geoms sharing one -- and nor will the
    // component id, which is shared by components whose surfaces meet.
    //
    // Components come out in the order their first face appears, so the file reads in step
    // with the face numbering.
    vector < string > comp_name_vec;
    vector < vector < int > > comp_face_vec;

    for ( int iface = 0; iface < ( int )face_surf_vec.size(); iface++ )
    {
        Surf* srf = FindSurf( m_NURBSSurfVec[ face_surf_vec[iface] ].m_SurfID );

        string name = "Unnamed";

        if ( srf )
        {
            if ( !srf->GetName().empty() )
            {
                // The surface's number within its Geom, which counts symmetry copies
                // separately -- the main surface id does not, so a symmetric pair would
                // otherwise share a name.
                string srf_name = srf->GetName();
                StringUtil::change_space_to_underscore( srf_name );
                name = srf_name + "_" + to_string( srf->GetFeaPartSurfNum() ) + "_" + srf->GetGeomID();
            }
            else if ( srf->GetFarFlag() )
            {
                name = "FarField";
            }
            else if ( srf->GetSymPlaneFlag() )
            {
                name = "SymmetryPlane";
            }
            else
            {
                name = "Surface";
            }
        }

        int icomp = -1;
        for ( int i = 0; i < ( int )comp_name_vec.size(); i++ )
        {
            if ( comp_name_vec[i] == name )
            {
                icomp = i;
            }
        }

        if ( icomp < 0 )
        {
            icomp = comp_name_vec.size();
            comp_name_vec.push_back( name );
            comp_face_vec.push_back( vector < int >() );
        }

        comp_face_vec[icomp].push_back( iface + 1 );
    }

    for ( int i = 0; i < ( int )comp_name_vec.size(); i++ )
    {
        fprintf( fp, "%s\n", comp_name_vec[i].c_str() );

        for ( int j = 0; j < ( int )comp_face_vec[i].size(); j++ )
        {
            if ( j > 0 )
            {
                fprintf( fp, "," );
            }
            fprintf( fp, "%d", comp_face_vec[i][j] );
        }

        fprintf( fp, "\n\n" );
    }

    fclose( fp );

    return true;
}

// Write the pogs input file.  Only the grid control parameters on the second and third
// lines matter much to start with, and the user is expected to edit them; these are the
// defaults egads2srf writes, with the mesh spacing taken from the CFD Mesh base length
// rather than guessed at.
bool CfdMeshMgrSingleton::WritePOGSInputFile( const string &fn, const string &rootname, int isym )
{
    FILE* fp = fopen( fn.c_str(), "w" );

    if ( !fp )
    {
        return false;
    }

    double sharp = acos( 1.0 - CFD_SHARP_TOL ) * 180.0 / M_PI;  // Dihedral angle above which an edge counts as sharp
    double turntmax = 30.0;     // Max turning angle
    double tolsp = 1.0e-6;      // Seam point tolerance
    double tolonse = 1.0e-5;    // On-seam tolerance

    int npmin = 11;             // Min points in each direction
    double srmax = GetGridDensityPtr()->m_GrowRatio;         // Max stretching ratio
    double dsm = GetGridDensityPtr()->GetBaseLen();          // Max grid spacing at curve interior
    double maxa = 360.0 / GetGridDensityPtr()->m_NCircSeg;   // Max dihedral angle, degrees

    int nfringe = 2;            // Fringe layers
    double stenqual = 1.0;      // Donor stencil quality below which a point is an orphan
    double dswall = 1.0e-4;     // Wall normal spacing for the volume grids
    double dobnd = -10.0;       // Outer boundary marching distance

    fprintf( fp, "%s\n\n", rootname.c_str() );
    fprintf( fp, "%9.3f%9.3f%14.5E%14.5E   SHARP,TURNTMAX,TOLSP,TOLONSE\n", sharp, turntmax, tolsp, tolonse );
    fprintf( fp, "%5d%9.3f%14.5E%9.3f   NPMIN,SRMAX,DSM,MAXA\n", npmin, srmax, dsm, maxa );
    fprintf( fp, "%5d%5d%8.2f%14.5E%8.2f   ISYM,NFRINGE,STENQUAL,DSWALL,DOBND\n", isym, nfringe, stenqual, dswall, dobnd );

    fclose( fp );

    return true;
}

// Write the faces as Plot3D surface blocks with an iblank tag.  Each block is one face
// sampled along its own tessellation lines; the surfaces are not trimmed, so a sample
// that trimming removed is marked off the geometry with an iblank of zero.
bool CfdMeshMgrSingleton::WritePOGSSurfFile( const string &uvin_fn, const string &uv_fn,
                                             const vector < int > &face_surf_vec )
{
    bool ok = true;

    int nface = face_surf_vec.size();

    // The parameters of every sample, carried alongside so the companion function file
    // can be written from the same tessellation.
    vector < vector < int > > uv_dim_vec( nface );
    vector < vector < vector < double > > > uv_var_vec( nface );

    // Ray length for the interior test, long enough to leave the model from anywhere in it.
    BndBox big_box;
    for ( int i = 0 ; i < ( int )m_SurfVec.size() ; i++ )
    {
        big_box.Update( m_SurfVec[i]->GetBBox() );
    }
    double x_dist = 1.0 + big_box.GetMax( 0 ) - big_box.GetMin( 0 );

    vector < vector < double > > uvec( nface ), wvec( nface );

    // Held so the same numbers can go out in both forms.
    vector < vector < vec3d > > all_pnt_vec( nface );
    vector < vector < int > > all_iblank_vec( nface );

    for ( int iface = 0; iface < nface; iface++ )
    {
        Surf* srf = FindSurf( m_NURBSSurfVec[ face_surf_vec[iface] ].m_SurfID );

        POGSTess( srf->GetUTess(), srf->GetSurfCore()->GetMinU(), srf->GetSurfCore()->GetMaxU(), uvec[iface] );
        POGSTess( srf->GetWTess(), srf->GetSurfCore()->GetMinW(), srf->GetSurfCore()->GetMaxW(), wvec[iface] );

        // The reader takes the normal of a face to be u_vec x v_vec, and wants it
        // pointing out of the body.  Walking v backwards on a surface whose parametric
        // normal points inward turns it around, and costs nothing else: the u and w
        // values written alongside the points stay the ones that name them.
        if ( srf->GetFlipFlag() )
        {
            std::reverse( wvec[iface].begin(), wvec[iface].end() );
        }
    }

    // A surface may be evaluated by one thread at a time, and a surface can carry more than one
    // face, so the faces of one surface are sampled in turn.
    vector < Surf* > face_srf( nface );
    for ( int iface = 0; iface < nface; iface++ )
    {
        face_srf[iface] = FindSurf( m_NURBSSurfVec[ face_surf_vec[iface] ].m_SurfID );
    }

    ProgressStage sample_bar( this, "POGS Sample", nface );
    RunBySurface( face_srf, [&]( int iface )
    {
        Surf* srf = FindSurf( m_NURBSSurfVec[ face_surf_vec[iface] ].m_SurfID );

        const vector < double > &u = uvec[iface];
        const vector < double > &w = wvec[iface];

        vector < vec3d > &pnt_vec = all_pnt_vec[iface];
        pnt_vec.resize( u.size() * w.size() );

        uv_dim_vec[iface].push_back( ( int )u.size() );
        uv_dim_vec[iface].push_back( ( int )w.size() );
        uv_dim_vec[iface].push_back( 1 );

        uv_var_vec[iface].resize( 2 );
        uv_var_vec[iface][0].resize( pnt_vec.size() );
        uv_var_vec[iface][1].resize( pnt_vec.size() );

        for ( int j = 0; j < ( int )w.size(); j++ )
        {
            for ( int i = 0; i < ( int )u.size(); i++ )
            {
                int k = j * u.size() + i;

                pnt_vec[k] = srf->CompPnt( u[i], w[j] );

                uv_var_vec[iface][0][k] = u[i];
                uv_var_vec[iface][1][k] = w[j];
            }
        }
    }, &sample_bar );

    // How many components the crossing table has to hold, from the numbers actually present
    // rather than from a count plus slack.
    int ncomp = 0;
    for ( int i = 0 ; i < ( int )m_SurfVec.size() ; i++ )
    {
        ncomp = max( ncomp, m_SurfVec[i]->GetCompID() + 1 );
    }

    // Deciding which samples the trimming removed is the whole cost of writing these files, so it
    // is named on the progress line.
    addOutputText( "POGS Iblank\n" );

    vector < char > open_face( nface, 0 );
    vector < vector < POGSCrossSample > > cross( nface );

    ProgressStage iblank_bar( this, "POGS iblank", nface );
    RunBySurface( face_srf, [&]( int iface )
    {
        Surf* srf = FindSurf( m_NURBSSurfVec[ face_surf_vec[iface] ].m_SurfID );

        const vector < vec3d > &pnt_vec = all_pnt_vec[iface];

        vector < int > iblank_vec( pnt_vec.size(), 1 );

        // One scratch table per face, so a ray cast does not allocate per sample.
        vector < vector < double > > t_vec_vec( ncomp );

        // The face's trimmed region is what its loops enclose: inside a closed external loop and
        // outside every cutout.
        NURBS_Surface &nurbs_surf = m_NURBSSurfVec[ face_surf_vec[iface] ];
        vector < NURBS_Loop > ext_loop_vec, cutout_vec;
        nurbs_surf.MakeExtLoopVec( ext_loop_vec, cutout_vec );

        vector < vector < vec3d > > loop_uw, loop_guess;
        vector < vector < int > > loop_other;
        int next = 0;
        for ( int j = 0; j < ( int )ext_loop_vec.size(); j++ )
        {
            if ( ext_loop_vec[j].m_ClosedFlag )
            {
                loop_uw.push_back( vector < vec3d >() );
                loop_other.push_back( vector < int >() );
                loop_guess.push_back( vector < vec3d >() );
                LoopUWAcross( ext_loop_vec[j], nurbs_surf.m_SurfID, loop_uw.back(), loop_other.back(), loop_guess.back() );
                next++;
            }
        }
        for ( int j = 0; j < ( int )cutout_vec.size(); j++ )
        {
            if ( cutout_vec[j].m_ClosedFlag )
            {
                loop_uw.push_back( vector < vec3d >() );
                loop_other.push_back( vector < int >() );
                loop_guess.push_back( vector < vec3d >() );
                LoopUWAcross( cutout_vec[j], nurbs_surf.m_SurfID, loop_uw.back(), loop_other.back(), loop_guess.back() );
            }
        }

        // A face is one only because some of it is kept, so it has a closed external loop.  One
        // that has none has lost a curve; its samples are left to the ray cast.
        if ( next == 0 )
        {
            open_face[iface] = 1;
            for ( int i = 0; i < ( int )pnt_vec.size(); i++ )
            {
                if ( PntTrimmedAway( pnt_vec[i], srf, x_dist, t_vec_vec ) )
                {
                    iblank_vec[i] = 0;
                }
            }
            all_iblank_vec[iface] = iblank_vec;
            return;
        }

        double umin = srf->GetSurfCore()->GetMinU();
        double umax = srf->GetSurfCore()->GetMaxU();
        double wmin = srf->GetSurfCore()->GetMinW();
        double wmax = srf->GetSurfCore()->GetMaxW();
        double rel_tol = GetSettingsPtr()->m_RelCurveTol;

        vector < vector < double > > loop_band( loop_uw.size() );
        for ( int j = 0; j < ( int )loop_uw.size(); j++ )
        {
            RefineLoopUW( srf, umin, umax, wmin, wmax, rel_tol, loop_uw[j], loop_other[j], loop_guess[j], loop_band[j] );
        }

        // Border curves and edges along the face's bounds run exactly where the surface ends.
        // Only the intersection curves approximate their curve, so only their edges are kept
        // for the side test, each with the surface across it and the side of it that is kept.
        vector < vec3d > bnd_a, bnd_b, seg_a, seg_b, uw_a, uw_b, seg_guess;
        vector < double > seg_band;
        vector < int > seg_other;
        vector < bool > seg_kept_left;
        for ( int j = 0; j < ( int )loop_uw.size(); j++ )
        {
            int n = loop_uw[j].size();
            double area = 0.0;
            for ( int k = 0; k < n; k++ )
            {
                const vec3d &p0 = loop_uw[j][k];
                const vec3d &p1 = loop_uw[j][ ( k + 1 ) % n ];
                area += p0.x() * p1.y() - p1.x() * p0.y();
            }

            // The kept side of an external loop is its inside, and of a cutout its outside.
            bool kept_left = ( area > 0.0 ) != ( j >= next );

            for ( int k = 0; k < n; k++ )
            {
                const vec3d &q0 = loop_uw[j][k];
                const vec3d &q1 = loop_uw[j][ ( k + 1 ) % n ];
                if ( loop_other[j][k] < 0 || OnFaceBound( q0, q1, umin, umax, wmin, wmax ) )
                {
                    if ( j < next )
                    {
                        bnd_a.push_back( vec3d( q0.x(), q0.y(), 0.0 ) );
                        bnd_b.push_back( vec3d( q1.x(), q1.y(), 0.0 ) );
                    }
                    continue;
                }
                seg_a.push_back( srf->CompPnt( q0.x(), q0.y() ) );
                seg_b.push_back( srf->CompPnt( q1.x(), q1.y() ) );
                uw_a.push_back( vec3d( q0.x(), q0.y(), 0.0 ) );
                uw_b.push_back( vec3d( q1.x(), q1.y(), 0.0 ) );
                seg_band.push_back( loop_band[j][k] );
                seg_other.push_back( loop_other[j][k] );
                seg_guess.push_back( loop_guess[j][k] );
                seg_kept_left.push_back( kept_left );
            }
        }

        vector < int > joined;
        JoinedComps( srf->GetCompID(), joined );
        int pairc = -1;
        bool pairinside = false;
        CoincidentVerdict( srf, pairc, pairinside );

        int nu = uvec[iface].size();

        // A box around each edge and loop, grown by the reach of the test made of it and a margin
        // past rounding.  A sample outside the box is out of reach, so the test is not made.
        vector < vec3d > all_uw = bnd_a;
        all_uw.insert( all_uw.end(), bnd_b.begin(), bnd_b.end() );
        all_uw.insert( all_uw.end(), uw_a.begin(), uw_a.end() );
        all_uw.insert( all_uw.end(), uw_b.begin(), uw_b.end() );
        for ( int j = 0; j < ( int )loop_uw.size(); j++ )
        {
            all_uw.insert( all_uw.end(), loop_uw[j].begin(), loop_uw[j].end() );
        }
        double slack_uw = RoundingSlack( all_uw );
        vector < vec3d > all_xyz = seg_a;
        all_xyz.insert( all_xyz.end(), seg_b.begin(), seg_b.end() );
        double slack_xyz = RoundingSlack( all_xyz );

        vector < vec3d > bnd_lo( bnd_a.size() ), bnd_hi( bnd_a.size() );
        for ( int e = 0; e < ( int )bnd_a.size(); e++ )
        {
            GrownBox( bnd_a[e], bnd_b[e], 1.0e-12 + slack_uw, bnd_lo[e], bnd_hi[e] );
        }

        vector < vec3d > seg_lo( seg_a.size() ), seg_hi( seg_a.size() ), uw_lo( seg_a.size() ), uw_hi( seg_a.size() );
        for ( int e = 0; e < ( int )seg_a.size(); e++ )
        {
            GrownBox( seg_a[e], seg_b[e], seg_band[e] + slack_xyz, seg_lo[e], seg_hi[e] );
            GrownBox( uw_a[e], uw_b[e], UW_REFINE_TOL * dist( uw_a[e], uw_b[e] ) + slack_uw, uw_lo[e], uw_hi[e] );
        }

        // A loop holds no point beside or above or below its box, and the crossings of a ray from
        // a point before it pair off.
        vector < vec3d > loop_lo( loop_uw.size() ), loop_hi( loop_uw.size() );
        for ( int j = 0; j < ( int )loop_uw.size(); j++ )
        {
            LoopBox( loop_uw[j], loop_lo[j], loop_hi[j] );
        }

        for ( int i = 0; i < ( int )pnt_vec.size(); i++ )
        {
            vec3d suw( uvec[iface][ i % nu ], wvec[iface][ i / nu ], 0.0 );

            // A sample on the face's own border is kept.
            bool in = false;
            for ( int e = 0; e < ( int )bnd_a.size() && !in; e++ )
            {
                if ( OutsideBox( suw, bnd_lo[e], bnd_hi[e] ) )
                {
                    continue;
                }
                if ( dist( proj_pnt_on_line_seg( bnd_a[e], bnd_b[e], suw ), suw ) < 1.0e-12 )
                {
                    in = true;
                }
            }
            for ( int j = 0; j < next && !in; j++ )
            {
                in = InLoopBox( loop_lo[j], loop_hi[j], slack_uw, suw ) && PolyContainsUW( loop_uw[j], suw );
            }
            for ( int j = next; j < ( int )loop_uw.size() && in; j++ )
            {
                if ( InLoopBox( loop_lo[j], loop_hi[j], slack_uw, suw ) && PolyContainsUW( loop_uw[j], suw ) )
                {
                    in = false;
                }
            }
            bool del = !in;

            // The nearest intersection edge the sample is within the tolerance of, in either
            // space: the surface's metric can make an edge short in one and long in the other.
            int enear = -1;
            double dnear = 1.0e300;
            for ( int e = 0; e < ( int )seg_a.size(); e++ )
            {
                if ( OutsideBox( pnt_vec[i], seg_lo[e], seg_hi[e] ) && OutsideBox( suw, uw_lo[e], uw_hi[e] ) )
                {
                    continue;
                }
                double d = dist( proj_pnt_on_line_seg( seg_a[e], seg_b[e], pnt_vec[i] ), pnt_vec[i] );
                double d_uw = dist( proj_pnt_on_line_seg( uw_a[e], uw_b[e], suw ), suw );
                if ( ( d <= seg_band[e] || d_uw <= UW_REFINE_TOL * dist( uw_a[e], uw_b[e] ) ) && d < dnear )
                {
                    dnear = d;
                    enear = e;
                }
            }

            // There the polygon may put the sample on the wrong side of the curve, so the side is
            // taken from the surface across it: the sample's nearest point there, and that
            // surface's outward normal.  A component joined to this one or coincident with it
            // has its own rules, which the ray cast knows.
            if ( enear >= 0 )
            {
                Surf *o = FindSurf( seg_other[enear] );
                int oc = -1;
                if ( o )
                {
                    oc = o->GetCompID();
                }

                bool use_ray = !o || oc < 0 || oc >= ncomp || oc == srf->GetCompID() || oc == pairc ||
                               std::find( joined.begin(), joined.end(), oc ) != joined.end();

                if ( !use_ray )
                {
                    // Another face's thread may be evaluating the surface across, so the rest is
                    // left until every face is done.
                    const vec3d &a = uw_a[enear];
                    const vec3d &b = uw_b[enear];
                    bool left = ( b.x() - a.x() ) * ( suw.y() - a.y() ) - ( b.y() - a.y() ) * ( suw.x() - a.x() ) > 0.0;

                    POGSCrossSample cs;
                    cs.m_Face = iface;
                    cs.m_Index = i;
                    cs.m_Srf = srf;
                    cs.m_Other = o;
                    cs.m_OtherComp = oc;
                    cs.m_Pnt = pnt_vec[i];
                    cs.m_Guess = seg_guess[enear];
                    cs.m_Reach = 10.0 * ( dnear + seg_band[enear] ) + 1.0e-12;
                    cs.m_Norm = srf->CompNorm( suw.x(), suw.y() );
                    cs.m_SameSide = ( left == seg_kept_left[enear] );
                    cs.m_Del = del;
                    cross[iface].push_back( cs );
                }
                else
                {
                    del = PntTrimmedAway( pnt_vec[i], srf, x_dist, t_vec_vec );
                }
            }

            if ( del )
            {
                iblank_vec[i] = 0;
            }
        }

        all_iblank_vec[iface] = iblank_vec;
    }, &iblank_bar );

    // The samples whose side is taken from the surface across an edge, grouped by that surface,
    // so each surface is evaluated by one thread at a time.  The side is the sample's nearest
    // point there and that surface's outward normal.
    std::map < Surf*, vector < POGSCrossSample* > > by_other;
    for ( int iface = 0; iface < nface; iface++ )
    {
        for ( int k = 0; k < ( int )cross[iface].size(); k++ )
        {
            by_other[ cross[iface][k].m_Other ].push_back( &cross[iface][k] );
        }
    }
    vector < vector < POGSCrossSample* > > cross_group;
    for ( std::map < Surf*, vector < POGSCrossSample* > >::iterator it = by_other.begin(); it != by_other.end(); ++it )
    {
        cross_group.push_back( it->second );
    }

    RunStage( "POGS Edge Side", ( int )cross_group.size(), [&]( int g )
    {
        vector < vector < double > > t_vec_vec( ncomp );
        for ( int k = 0; k < ( int )cross_group[g].size(); k++ )
        {
            const POGSCrossSample &cs = *cross_group[g][k];
            Surf *o = cs.m_Other;
            bool del = cs.m_Del;

            vec2d ouw = o->ClosestUW( cs.m_Pnt, cs.m_Guess.x(), cs.m_Guess.y() );
            vec3d q = o->CompPnt( ouw[0], ouw[1] );
            vec3d n = OutwardNorm( o, ouw[0], ouw[1] );

            if ( dist( q, cs.m_Pnt ) > cs.m_Reach )
            {
                // The nearest point is not across this edge.
                del = PntTrimmedAway( cs.m_Pnt, cs.m_Srf, x_dist, t_vec_vec );
            }
            else if ( !NormsTangent( cs.m_Norm, n ) )
            {
                // Where the surfaces lie flat against each other the normal cannot tell the
                // sides apart, and the polygon stands.
                vector < bool > inside( ncomp, false );
                inside[ cs.m_OtherComp ] = dot( cs.m_Pnt - q, n ) < 0.0;
                bool kept_c = !SetDeleteTriFlag( cs.m_Srf->GetSurfaceCfdType(), cs.m_Srf->GetSymPlaneFlag(), inside );
                if ( cs.m_SameSide != kept_c )
                {
                    del = !del;
                }
            }

            all_iblank_vec[ cs.m_Face ][ cs.m_Index ] = !del;
        }
    } );

    // A face with no closed loop says the topology written alongside is incomplete.  Said on
    // stdout too, since a run driven from a script sees nothing addOutputText posts.
    for ( int iface = 0; iface < nface; iface++ )
    {
        if ( open_face[iface] )
        {
            char str[256];
            snprintf( str, sizeof( str ), "POGS: face %d has no closed loop; its samples were classified by ray\n", iface + 1 );
            addOutputText( str );
            printf( "%s", str );
        }
    }

    addOutputText( "Writing POGS Files\n" );

    // egads2srf writes these with the u count first and u running fastest, so the reader
    // takes the first index for u and the second for v.  Match it.
    vector < vector < int > > grid_dim_vec( nface );
    for ( int iface = 0; iface < nface; iface++ )
    {
        grid_dim_vec[iface].push_back( ( int )uvec[iface].size() );
        grid_dim_vec[iface].push_back( ( int )wvec[iface].size() );
        grid_dim_vec[iface].push_back( 1 );
    }

    ok = WritePlot3DGrid( uvin_fn, grid_dim_vec, all_pnt_vec, all_iblank_vec ) && ok;
    // ok = WritePlot3DFunction( uv_fn, uv_dim_vec, uv_var_vec ) && ok;

    return ok;
}

// Say so when a POGS file could not be written.
//
// These go out through the progress path and to stdout alike: a run driven from a script sees
// nothing addOutputText posts, and a silently missing file is the worst way to learn that a
// disk was full or a directory read only.
void CfdMeshMgrSingleton::ReportPOGSWrite( const string &fn, bool ok )
{
    if ( ok )
    {
        return;
    }

    char str[1024];
    snprintf( str, sizeof( str ), "POGS: could not write %s\n", fn.c_str() );
    addOutputText( str );
    printf( "%s", str );
}

void CfdMeshMgrSingleton::WritePOGS( const string &pogs_fn )
{
    string base = pogs_fn;

    int pos = base.find( ".i.tri" );
    if ( pos >= 0 )
    {
        base.erase( pos );
    }

    string uvin_fn = base;
    uvin_fn.append( ".uvin" );
    string cur_fn = base;
    cur_fn.append( ".cur" );
    string topo_fn = base;
    topo_fn.append( ".topo" );
    string uv_fn = base;
    uv_fn.append( ".uv" );  // Currently unused, but filename passed through
    // string cuv_fn = base;
    // cuv_fn.append( ".cuv" ); // Currently unused.
    string gcomp_fn = base;
    gcomp_fn.append( ".gcomp" );
    string pogsi_fn = base;
    pogsi_fn.append( ".pogs.i" );

    // A surface is a face of the topology only if it is part of the triangulation the
    // topology describes, and the test for that is whether it carries any mesh.  The half
    // mesh symmetry plane is the case that matters: it slices the geometry and is then
    // discarded, so the mesh is left open along the cut and the plane is not a face of
    // it.  The far field's symmetry boundary is meshed and is a face.  Wakes are kept out
    // of the triangulation, so they are kept out here too.
    //
    // A face's number is its position in this list, which is what NFACE counts and what
    // the triangulation tags its triangles with.  A Surf ID cannot serve: IDs are handed
    // out when surfaces are loaded, and by this point the far field has renumbered them,
    // half mesh trimming and duplicate removal have deleted some, and BuildNURBSSurfMap
    // has passed over any surface that ended up completely enclosed by another component
    // or is an external negative surface.  So the IDs run past the end of the face list
    // and have gaps in the middle.
    vector < int > face_surf_vec;
    std::map < int, int > surf_id_face_num;

    for ( int i = 0 ; i < ( int )m_NURBSSurfVec.size() ; i++ )
    {
        Surf* srf = FindSurf( m_NURBSSurfVec[i].m_SurfID );

        if ( !srf || srf->GetWakeFlag() || !srf->GetMesh() || srf->GetMesh()->GetSimpFaceVec().empty() )
        {
            continue;
        }

        surf_id_face_num[ m_NURBSSurfVec[i].m_SurfID ] = ( int )face_surf_vec.size() + 1;
        face_surf_vec.push_back( i );
    }

    // ISYM says what symmetry the body has: 0 for a closed body, 1/2/3 for a half body
    // on the +x/+y/+z side and the negatives for the other side, 11/12/13 for a full body
    // that is symmetric about x/y/z = 0.  A half mesh keeps +y.
    int isym = 0;
    if ( GetCfdSettingsPtr()->m_HalfMeshFlag )
    {
        isym = 2;
    }

    // Used when comparing W parameter to TMAGIC
    double tol = 1e-12;

    //==== Find All Tri Counts ====//
    // Wakes are kept out of the triangulation, so their points are left out too.
    size_t nallface = 0;
    vector < bool > notwake( m_SurfVec.size() );
    for ( int i = 0 ; i < ( int )m_SurfVec.size() ; i++ )
    {
        notwake[i] = !m_SurfVec[i]->GetWakeFlag();
        if ( notwake[i] )
        {
            nallface += m_SurfVec[i]->GetMesh()->GetSimpFaceVec().size();
        }
    }

    vector < vector < int > > allPntKey;
    vector< vec3d > allUsedPntVec;
    MergeSurfPnts( notwake, allPntKey, allUsedPntVec, nullptr );

    //==== Assemble Normal Tris ====//
    vector< SimpFace > allFaceVec;
    allFaceVec.reserve( nallface );
    int ntristrict = 0;
    for ( int i = 0 ; i < ( int )m_SurfVec.size() ; i++ )
    {
        if ( !m_SurfVec[i]->GetWakeFlag() )
        {
            vector < SimpFace >& sFaceVec = m_SurfVec[ i ]->GetMesh()->GetSimpFaceVec();
            for ( int t = 0 ; t <  ( int )sFaceVec.size() ; t++ )
            {
                SimpFace sface;
                sface.ind0 = allPntKey[i][sFaceVec[t].ind0] + 1;
                sface.ind1 = allPntKey[i][sFaceVec[t].ind1] + 1;
                sface.ind2 = allPntKey[i][sFaceVec[t].ind2] + 1;

                if( sFaceVec[t].m_isQuad )
                {
                    sface.m_isQuad = true;
                    sface.ind3 = allPntKey[i][sFaceVec[t].ind3] + 1;
                    ntristrict++; // Bonus tri for split quad.
                }

                sface.m_iSurf = i;
                ntristrict++;
                allFaceVec.push_back( sface );
            }

        }
    }


    // The faces are numbered now, so the map is done with.
    vector < vector < int > >().swap( allPntKey );


    //=====================================================================================//
    //==== Write .i.tri File for POGS =====================================================//
    //=====================================================================================//
    if ( pogs_fn.length() != 0 )
    {
        // Flatten the triangles out first, splitting any quads, so both forms of the
        // file are written from the same numbers.
        vector < int > conn_vec;
        vector < int > comp_vec;

        conn_vec.reserve( 3 * ntristrict );
        comp_vec.reserve( ntristrict );

        for ( int i = 0 ; i < ( int )allFaceVec.size() ; i++ )
        {
            // A triangle is tagged with the number of the face it lies on, the same
            // number the topology and component files use.  A surface that did not
            // become a face has none to give.
            int icomp = 0;

            if ( allFaceVec[i].m_iSurf >= 0 && allFaceVec[i].m_iSurf < ( int )m_SurfVec.size() )
            {
                std::map < int, int >::const_iterator it =
                        surf_id_face_num.find( m_SurfVec[ allFaceVec[i].m_iSurf ]->GetSurfID() );

                if ( it != surf_id_face_num.end() )
                {
                    icomp = it->second;
                }
            }

            conn_vec.push_back( allFaceVec[i].ind0 );
            conn_vec.push_back( allFaceVec[i].ind1 );
            conn_vec.push_back( allFaceVec[i].ind2 );
            comp_vec.push_back( icomp );

            if( allFaceVec[i].m_isQuad )
            {
                conn_vec.push_back( allFaceVec[i].ind0 );
                conn_vec.push_back( allFaceVec[i].ind2 );
                conn_vec.push_back( allFaceVec[i].ind3 );
                comp_vec.push_back( icomp );
            }
        }

        ReportPOGSWrite( pogs_fn,
                         WriteCart3DTri( pogs_fn, allUsedPntVec, conn_vec, comp_vec ) );
    }




    int nsrf = m_NURBSSurfVec.size();

    std::set < int > used_curve_set;

    // Sense of each curve as seen by each face that walks it.  A curve is walked by at
    // most two faces, and each of them reports independently whether it lies to the left
    // or the right of the curve's own direction.
    std::map < int, std::map < int, int > > curve_face_sense;

    int nface = face_surf_vec.size();


    for ( int iface = 0 ; iface < nface ; iface++ )
    {
        int i = face_surf_vec[iface];
        NURBS_Surface& nurbs_surf = m_NURBSSurfVec[i];

        Surf* current_surf = FindSurf( nurbs_surf.m_SurfID );
        int surf_indx = FindSurfIndx( nurbs_surf.m_SurfID );

        // Every surface knows which way it faces: the flag is set from the Geom the
        // surface came from, or where the domain surfaces are built.
        bool flip_flag = false;
        if ( current_surf )
        {
            flip_flag = current_surf->GetFlipFlag();
        }

        vector < NURBS_Loop >& loop_vec = nurbs_surf.m_NURBSLoopVec;

        // Identify if there are multiple external loops
        vector < NURBS_Loop > ext_loop_vec, cutout_vec;

        nurbs_surf.MakeExtLoopVec( ext_loop_vec,  cutout_vec );

        // A chain that never closed does not bound anything, so it names no edge and fixes
        // no orientation.  The IGES and STEP writers refuse these as incomplete loops; do
        // the same here rather than write an edge with no sense.
        //
        // The loops that came back as external are the ones that name edges.  A cutout is
        // not an edge in its own right -- the face on the other side of it names it -- but
        // the face that owns the cutout still has a sense to report on it.
        vector < const NURBS_Loop* > loop_ptr;
        vector < bool > names_edges;

        for ( int j = 0; j < ext_loop_vec.size(); j++ )
        {
            if ( ext_loop_vec[j].m_ClosedFlag )
            {
                loop_ptr.push_back( &ext_loop_vec[j] );
                names_edges.push_back( true );
            }
        }

        for ( int j = 0; j < cutout_vec.size(); j++ )
        {
            if ( cutout_vec[j].m_ClosedFlag )
            {
                loop_ptr.push_back( &cutout_vec[j] );
                names_edges.push_back( false );
            }
        }

        // Which way round a loop is walked depends on whether it bounds the face from
        // outside or punches a hole in it, and the sense follows from that.  Whether a loop
        // came back external does not settle it: where a half mesh cuts a body, the cut is
        // a border curve of the body, so the footprint it traces on the symmetry plane is
        // not made of intersection curves alone and does not read as a cutout -- though a
        // hole is exactly what it is.
        //
        // What settles it is the mesh.  The boundary encloses everything meshed, so it is
        // at least as big; a hole is smaller.  Of the loops big enough, the boundary is the
        // tightest -- which is what keeps a surface trimmed back from its own border, as
        // the symmetry plane is when a Geom stands in for the far field, from mistaking the
        // border it no longer reaches for its boundary.
        double mesh_area = MeshAreaUW( current_surf );

        int iouter = -1;
        double area_out = 0.0;

        int ilargest = -1;
        double area_largest = 0.0;

        for ( int j = 0; j < ( int )loop_ptr.size(); j++ )
        {
            double area = std::fabs( loop_ptr[j]->SignedAreaUW( nurbs_surf.m_SurfID ) );

            if ( ilargest < 0 || area > area_largest )
            {
                area_largest = area;
                ilargest = j;
            }

            // Both areas are measured off the same tessellation, so they agree closely
            // where a loop is the boundary.  The slack is for the last bit of round-off.
            if ( area >= 0.999 * mesh_area )
            {
                if ( iouter < 0 || area < area_out )
                {
                    area_out = area;
                    iouter = j;
                }
            }
        }

        // Nothing meshed on this surface, or no loop big enough to hold what was.
        if ( iouter < 0 )
        {
            iouter = ilargest;
        }

        for ( int j = 0; j < ( int )loop_ptr.size(); j++ )
        {
            if ( names_edges[j] )
            {
                const vector < pair < NURBS_Curve, bool > > &oc = loop_ptr[j]->m_OrderedCurves;

                for ( int k = 0; k < ( int )oc.size(); k++ )
                {
                    used_curve_set.insert( oc[k].first.m_CurveID );
                }
            }

            AccumulateLoopSense( *loop_ptr[j], nurbs_surf.m_SurfID, j != iouter, flip_flag, curve_face_sense );
        }
    }

    // The topology file numbers its edges by their position in used_curve_set, and those
    // numbers are what index the curve file, so both are written from the same walk of
    // the set.  A curve is written in its own direction, which is the direction the
    // topology file's edge senses are stated against.
    vector < vector < vec3d > > edge_curve_vec;
    edge_curve_vec.reserve( used_curve_set.size() );

    // The same curves in the parameter space of the faces on either side of them.
    vector < vector < int > > cuv_dim_vec;
    vector < vector < vector < double > > > cuv_var_vec;

    std::set < int >::iterator cit;
    for ( cit = used_curve_set.begin(); cit != used_curve_set.end(); cit++ )
    {
        NURBS_Curve &nurbs_curve = m_NURBSCurveVec[ *cit ];

        edge_curve_vec.push_back( nurbs_curve.m_PntVec );

        int npnt = nurbs_curve.m_PntVec.size();

        vector < int > dim;
        dim.push_back( npnt );
        dim.push_back( 1 );
        dim.push_back( 1 );
        cuv_dim_vec.push_back( dim );

        // u and w along the first of the curve's two faces, then along the second.  The
        // topology file writes the face that exists in its first pair of columns, which
        // can put the curve's B surface there, so make the same choice here to keep the
        // two files reading in the same order.
        int faceA = 0, senseA = 0;
        int faceB = 0, senseB = 0;

        GetTopoEdgeFaceChecked( nurbs_curve.m_SurfA_ID, surf_id_face_num, curve_face_sense, *cit, faceA, senseA );
        if ( nurbs_curve.m_SurfB_ID != nurbs_curve.m_SurfA_ID )
        {
            GetTopoEdgeFaceChecked( nurbs_curve.m_SurfB_ID, surf_id_face_num, curve_face_sense, *cit, faceB, senseB );
        }

        const vector < vec3d > *uw_first = &nurbs_curve.m_UWPntVec_A;
        const vector < vec3d > *uw_second = &nurbs_curve.m_UWPntVec_B;

        if ( faceA == 0 )
        {
            uw_first = &nurbs_curve.m_UWPntVec_B;
            uw_second = &nurbs_curve.m_UWPntVec_A;
        }

        // The parametric copies are sampled at the same places as the points, so they
        // line up with them one for one.
        vector < vector < double > > var( 4 );
        for ( int v = 0; v < 4; v++ )
        {
            var[v].resize( npnt, 0.0 );
        }

        for ( int i = 0; i < npnt; i++ )
        {
            if ( i < ( int )uw_first->size() )
            {
                var[0][i] = ( *uw_first )[i].x();
                var[1][i] = ( *uw_first )[i].y();
            }

            if ( i < ( int )uw_second->size() )
            {
                var[2][i] = ( *uw_second )[i].x();
                var[3][i] = ( *uw_second )[i].y();
            }
        }

        cuv_var_vec.push_back( var );
    }

    // The pogs input file names the case on its first line, without a path.
    string rootname = base;
    size_t slash = rootname.find_last_of( "/\\" );
    if ( slash != string::npos )
    {
        rootname.erase( 0, slash + 1 );
    }

    ReportPOGSWrite( uvin_fn /* + " or " + uv_fn */, WritePOGSSurfFile( uvin_fn, uv_fn, face_surf_vec ) );

    ReportPOGSWrite( gcomp_fn, WritePOGSCompFile( gcomp_fn, face_surf_vec ) );
    ReportPOGSWrite( pogsi_fn, WritePOGSInputFile( pogsi_fn, rootname, isym ) );

    // A curve file carries no iblank.
    vector < vector < int > > cur_dim_vec( edge_curve_vec.size() );
    for ( int i = 0; i < ( int )edge_curve_vec.size(); i++ )
    {
        cur_dim_vec[i].push_back( ( int )edge_curve_vec[i].size() );
        cur_dim_vec[i].push_back( 1 );
        cur_dim_vec[i].push_back( 1 );
    }

    ReportPOGSWrite( cur_fn, WritePlot3DGrid( cur_fn, cur_dim_vec, edge_curve_vec, vector < vector < int > >() ) );
    // ReportPOGSWrite( cuv_fn, WritePlot3DFunction( cuv_fn, cuv_dim_vec, cuv_var_vec ) );

    FILE* topo_fp = fopen( topo_fn.c_str(), "w" );

    ReportPOGSWrite( topo_fn, topo_fp != nullptr );

    if ( topo_fp )
    {
        fprintf( topo_fp, "%6d%6d%6d%23.12E   ISYM,NEDGE,NFACE,SCALE (Symmetry, # Non-degen. Edges & Faces)\n", isym, (int) used_curve_set.size(), nface, 1.0 );
        int i = 0;
        std::set < int >::iterator it;
        for ( it = used_curve_set.begin(); it != used_curve_set.end(); it++ )
        {
            int icurve = (*it);
            NURBS_Curve & nurbs_curve = m_NURBSCurveVec[ icurve ];

            // Column 1 = edge index
            // Column 2 = face index on one side of edge
            // Column 3 = sense of edge relative to face (+1 if face is on left side of edge in the positive edge index direction, -1 of opposite)
            // Column 4 = face index on the other side of edge, 0 if no face
            // Column 5 = similar to column 3, 0 if no face
            int faceA = 0, senseA = 0;
            int faceB = 0, senseB = 0;

            GetTopoEdgeFaceChecked( nurbs_curve.m_SurfA_ID, surf_id_face_num, curve_face_sense, icurve, faceA, senseA );

            // A curve whose two parents are the same surface lies on an open boundary
            // of that surface -- there is no face on the other side of it.
            if ( nurbs_curve.m_SurfB_ID != nurbs_curve.m_SurfA_ID )
            {
                GetTopoEdgeFaceChecked( nurbs_curve.m_SurfB_ID, surf_id_face_num, curve_face_sense, icurve, faceB, senseB );
            }

            // A parent surface that never became a face leaves its half of the pair
            // empty.  The face that does exist belongs in the first pair of columns.
            if ( faceA == 0 )
            {
                std::swap( faceA, faceB );
                std::swap( senseA, senseB );
            }

            fprintf( topo_fp, "%8d%8d%8d%8d%8d\n", i + 1, faceA, senseA, faceB, senseB );

            i++;
        }

        fclose( topo_fp );
    }

}

// How close the mesh came to the edge lengths it was asked for.
//
// Split takes an edge over sqrt(2) times its target and halves it; Collapse takes one under
// 1/sqrt(2) and removes it.  Anything between is left alone, so the algorithm's own fixed
// point is a band a factor of two wide and the spread inside it is not something the two
// operators are trying to close.  This says how wide the spread actually is, which is the
// thing to watch if that band is ever tightened or a smoother is asked to do the closing.
//
// Border edges are not counted: their target is their own length, so they would all score 1.
// How many faces are worse than a given multiple of their target, counting a face by its
// worst edge.  This is the number the coloured picture shows.
static string FaceRatioLine( vector < double > &fr, const char *what )
{
    if ( fr.empty() )
    {
        return string();
    }

    int nband = 0, n2 = 0;

    for ( int i = 0 ; i < ( int )fr.size() ; i++ )
    {
        if ( fr[i] < 1.414 )
        {
            nband++;
        }
        if ( fr[i] >= 2.0 )
        {
            n2++;
        }
    }

    char buf[256];
    snprintf( buf, sizeof( buf ),
              "  %-22s %8d faces, inside the band %5.1f%%, at or past 2x %5.1f%%\n",
              what, ( int )fr.size(), 100.0 * nband / fr.size(), 100.0 * n2 / fr.size() );

    return string( buf );
}

string CfdMeshMgrSingleton::TargetLengthReport()
{
    vector < double > &r = m_LengthRatios;

    if ( r.empty() )
    {
        return string();
    }

    sort( r.begin(), r.end() );

    double sum = 0.0;
    int n10 = 0, n25 = 0, nband = 0;

    for ( int i = 0 ; i < ( int )r.size() ; i++ )
    {
        sum += r[i];

        if ( r[i] > 0.90 && r[i] < 1.10 )
        {
            n10++;
        }
        if ( r[i] > 0.75 && r[i] < 1.25 )
        {
            n25++;
        }
        if ( r[i] > 0.707 && r[i] < 1.414 )
        {
            nband++;
        }
    }

    double npct = 100.0 / ( double )r.size();

    char buf[512];
    snprintf( buf, sizeof( buf ),
              "Edge length / target: %d edges, mean %.3f, median %.3f, min %.3f, max %.3f\n"
              "  within 10%%: %.1f%%   within 25%%: %.1f%%   inside the split/collapse band: %.1f%%\n",
              ( int )r.size(), sum / ( double )r.size(), r[ r.size() / 2 ], r.front(), r.back(),
              n10 * npct, n25 * npct, nband * npct );

    return string( buf )
           + FaceRatioLine( m_FaceLengthRatios, "by face, worst edge:" )
           + FaceRatioLine( m_BorderFaceLengthRatios, "of those, on a border:" );
}

void CfdMeshMgrSingleton::RecordResults()
{
    m_LastResultID = string();

    Results* res = ResultsMgr.CreateResults( "CFDMesh", "CFD mesh generation results." );

    if ( !res )
    {
        return;
    }

    m_LastResultID = res->GetID();

    bool tight = m_NumBorderEdges == 0 && m_NumOverConnEdges == 0;

    // The patches cut from the model, not the symmetry plane, far field or wakes added to them
    int nsurf = 0;
    for ( int i = 0 ; i < ( int )m_SurfVec.size() ; i++ )
    {
        if ( m_SurfVec[i]->GetCompID() >= 0 && !m_SurfVec[i]->GetWakeFlag() &&
             !m_SurfVec[i]->GetSymPlaneFlag() && !m_SurfVec[i]->GetFarFlag() )
        {
            nsurf++;
        }
    }

    res->Add( new NameValData( "Num_Surfs", nsurf, "Number of patches of the model's surfaces meshed." ) );
    res->Add( new NameValData( "Num_Tris", m_NumMeshTris, "Number of triangles in the mesh." ) );
    res->Add( new NameValData( "Num_Border_Edges", m_NumBorderEdges, "Number of mesh edges with one triangle on them, other than a wake's or a half mesh's along its open symmetry plane." ) );
    res->Add( new NameValData( "Num_Over_Connected_Edges", m_NumOverConnEdges, "Number of mesh edges with more than two triangles on them." ) );
    res->Add( new NameValData( "Water_Tight", tight, "Flag set when the mesh is closed." ) );
    res->Add( new NameValData( "Num_Topo_Surfs", m_NumTopoSurfs, "Number of surfaces whose interior triangles the intersection chains' topology found, without ray casts." ) );
}

// A face's corners as merged point indices, and how many it has
static int CheckFaceCorners( const SimpFace &face, const vector < int > &key, int ind[4] )
{
    ind[0] = key[ face.ind0 ];
    ind[1] = key[ face.ind1 ];
    ind[2] = key[ face.ind2 ];
    if ( face.m_isQuad )
    {
        ind[3] = key[ face.ind3 ];
        return 4;
    }
    return 3;
}

// Each side of each face, as the merged indices of its ends packed lowest first, dealt into the
// bucket its lower end falls in: counted into at when not place, else written into sides at at.
// face0 is the first face's place in the order the faces are visited.
static void DealFaceSides( const vector < SimpFace > &faces, const vector < int > &key, int face0, int nbucket,
                           long long npnt, bool place, vector < long long > &at,
                           vector < pair < unsigned long long, int > > &sides )
{
    for ( int f = 0 ; f < ( int )faces.size() ; f++ )
    {
        int ind[4];
        int n = CheckFaceCorners( faces[f], key, ind );
        for ( int c = 0 ; c < n ; c++ )
        {
            unsigned long long a = ind[c];
            unsigned long long b = ind[ ( c + 1 ) % n ];
            if ( b < a )
            {
                std::swap( a, b );
            }
            int k = ( int )( ( long long )a * nbucket / npnt );
            if ( place )
            {
                sides[ at[k]++ ] = pair < unsigned long long, int >( ( a << 32 ) | b, face0 + f );
            }
            else
            {
                at[k]++;
            }
        }
    }
}

// The Node at merged point ind, made the first time it is asked for and kept in store
static Node* CheckNodeAt( int ind, const vector < vec3d > &pnts, unordered_map < int, Node* > &nodes, vector < Node* > &store )
{
    unordered_map < int, Node* >::const_iterator it = nodes.find( ind );
    if ( it != nodes.end() )
    {
        return it->second;
    }
    Node* nd = new Node();
    nd->pnt = pnts[ ind ];
    store.push_back( nd );
    nodes[ ind ] = nd;
    return nd;
}

// Count the edges of the mesh with one face, and the faces past the second on an edge.  Only
// the edges and faces found wrong are kept, as Nodes, Edges and Faces, to be drawn.
string CfdMeshMgrSingleton::CheckWaterTight()
{
    vector < bool > checked( m_SurfVec.size(), false );
    for ( int i = 0 ; i < ( int )m_SurfVec.size() ; i++ )
    {
        if( m_SurfVec[i]->GetSurfaceCfdType() != vsp::CFD_TRANSPARENT || m_SurfVec[i]->GetFarFlag() || m_SurfVec[i]->GetSymPlaneFlag() )
        {
            checked[i] = true;
        }
    }

    vector < vector < int > > usedKey;
    vector < vec3d > usedPnt;
    MergeSurfPnts( checked, usedKey, usedPnt, nullptr );

    // Every side of every face, as the merged indices of its ends, lowest first, packed in
    // one integer, beside the face's place in the order the faces are visited.  Sorted, the
    // faces on one edge come together in that order: an edge with one face is a border, and
    // every face past the second on an edge is one too many.
    //
    // The sides are dealt into buckets by their lower end, each bucket a range of point
    // indices, so the buckets sort apart on several threads and lie end to end in order.
    int nsurf = ( int )m_SurfVec.size();
    vector < int > face0( nsurf + 1, 0 );
    for ( int i = 0 ; i < nsurf ; i++ )
    {
        face0[i + 1] = face0[i];
        if ( checked[i] )
        {
            face0[i + 1] += ( int )m_SurfVec[i]->GetMesh()->GetSimpFaceVec().size();
        }
    }
    int nface = face0[ nsurf ];

    long long npnt = ( long long )usedPnt.size();
    int nbucket = 4 * StageThreadCount( nsurf );
    if ( npnt < nbucket )
    {
        nbucket = 1;
    }

    // Which surfaces' sides land in each bucket, counted, then placed.
    vector < vector < long long > > count( nsurf, vector < long long >( nbucket, 0 ) );

    vector < pair < unsigned long long, int > > sides;

    // Every surface counted, then placed, then every bucket sorted.
    ProgressStage bar( this, "Water Tight", 2 * nsurf + nbucket );

    RunIndexed( nsurf, StageThreadCount( nsurf ), [&]( int i )
    {
        if ( checked[i] )
        {
            DealFaceSides( m_SurfVec[i]->GetMesh()->GetSimpFaceVec(), usedKey[i], face0[i], nbucket, npnt, false, count[i], sides );
        }
    }, &bar );

    vector < long long > bucket0( nbucket + 1, 0 );
    for ( int k = 0 ; k < nbucket ; k++ )
    {
        bucket0[k + 1] = bucket0[k];
        for ( int i = 0 ; i < nsurf ; i++ )
        {
            bucket0[k + 1] += count[i][k];
        }
    }

    // Where each surface's first side in each bucket goes
    {
        vector < long long > next( bucket0.begin(), bucket0.end() - 1 );
        for ( int i = 0 ; i < nsurf ; i++ )
        {
            for ( int k = 0 ; k < nbucket ; k++ )
            {
                long long c = count[i][k];
                count[i][k] = next[k];
                next[k] += c;
            }
        }
    }

    sides.resize( bucket0[ nbucket ] );

    RunIndexed( nsurf, StageThreadCount( nsurf ), [&]( int i )
    {
        if ( checked[i] )
        {
            DealFaceSides( m_SurfVec[i]->GetMesh()->GetSimpFaceVec(), usedKey[i], face0[i], nbucket, npnt, true, count[i], sides );
        }
    }, &bar );

    RunIndexed( nbucket, StageThreadCount( nbucket ), [&]( int k )
    {
        std::sort( sides.begin() + bucket0[k], sides.begin() + bucket0[k + 1] );
    }, &bar );

    bar.End();

    // A half mesh with no faces on its symmetry plane is open along y = 0 by design, as a wake
    // is open along its edges, so those edges are not counted as borders.  An end lies on the
    // plane when it is within the distance points are merged at.
    bool open_sym = GetCfdSettingsPtr()->m_HalfMeshFlag;
    double ysym = std::sqrt( PT_MERGE_TOL );
    for ( int i = 0 ; i < nsurf ; i++ )
    {
        if ( m_SurfVec[i]->GetSymPlaneFlag() && !m_SurfVec[i]->GetMesh()->GetSimpFaceVec().empty() )
        {
            open_sym = false;
        }
    }

    int num_border_edges = 0;
    int moreThanTwoTriPerEdge = 0;
    vector < bool > badface( nface, false );
    vector < pair < int, int > > border;

    for ( size_t j = 0 ; j < sides.size() ; )
    {
        size_t r = j;
        while ( r < sides.size() && sides[r].first == sides[j].first )
        {
            r++;
        }

        if ( r - j == 1 )
        {
            int a = ( int )( sides[j].first >> 32 );
            int b = ( int )( sides[j].first & 0xffffffffULL );
            if ( !open_sym || std::abs( usedPnt[a].y() ) > ysym || std::abs( usedPnt[b].y() ) > ysym )
            {
                num_border_edges++;
                border.push_back( pair < int, int >( a, b ) );
            }
        }

        for ( size_t q = j + 2 ; q < r ; q++ )
        {
            printf( "Edge: More Than 2 Faces\n" );
            moreThanTwoTriPerEdge++;
            badface[ sides[q].second ] = true;
        }

        j = r;
    }
    vector < pair < unsigned long long, int > >().swap( sides );

    //==== Keep what is wrong, to be drawn ====//
    unordered_map < int, Node* > nodes;

    for ( int j = 0 ; j < ( int )border.size() ; j++ )
    {
        Edge* e = new Edge( CheckNodeAt( border[j].first, usedPnt, nodes, m_nodeStore ), CheckNodeAt( border[j].second, usedPnt, nodes, m_nodeStore ) );
        e->debugFlag = true;
        m_BadEdges.push_back( e );
    }

    if ( moreThanTwoTriPerEdge )
    {
        int iface = 0;
        for ( int i = 0 ; i < ( int )m_SurfVec.size() ; i++ )
        {
            if ( checked[i] )
            {
                const vector < SimpFace >& sFaceVec = m_SurfVec[ i ]->GetMesh()->GetSimpFaceVec();
                for ( int f = 0 ; f < ( int )sFaceVec.size() ; f++ )
                {
                    if ( badface[ iface ] )
                    {
                        int ind[4];
                        Face* face;
                        if ( CheckFaceCorners( sFaceVec[f], usedKey[i], ind ) == 4 )
                        {
                            face = new Face( CheckNodeAt( ind[0], usedPnt, nodes, m_nodeStore ), CheckNodeAt( ind[1], usedPnt, nodes, m_nodeStore ), CheckNodeAt( ind[2], usedPnt, nodes, m_nodeStore ), CheckNodeAt( ind[3], usedPnt, nodes, m_nodeStore ),
                                             nullptr, nullptr, nullptr, nullptr );
                        }
                        else
                        {
                            face = new Face( CheckNodeAt( ind[0], usedPnt, nodes, m_nodeStore ), CheckNodeAt( ind[1], usedPnt, nodes, m_nodeStore ), CheckNodeAt( ind[2], usedPnt, nodes, m_nodeStore ), nullptr, nullptr, nullptr );
                        }
                        face->debugFlag = true;
                        m_BadFaces.push_back( face );
                    }
                    iface++;
                }
            }
        }
    }

    m_NumBorderEdges = num_border_edges;
    m_NumOverConnEdges = moreThanTwoTriPerEdge;

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
    if ( innerBottomLeft.x() < outerBottomLeft.x() || innerBottomRight.x() > outerBottomRight.x()
      || innerTopLeft.z() > outerTopLeft.z() || innerBottomLeft.z() < outerBottomLeft.z() )
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

    // Box outer domain, half or full mesh.  A full mesh has no symmetry plane to split.
    if ( GetCfdSettingsPtr()->m_FarMeshFlag && !GetCfdSettingsPtr()->m_FarCompFlag)
    {
        if ( GetCfdSettingsPtr()->m_HalfMeshFlag && GetCfdSettingsPtr()->m_SymSplittingOnFlag && isInside )
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
        domainSurfs[i]->SetFlipFlag( false );
    }
    return domainSurfs;
}

void CfdMeshMgrSingleton::InitMesh( )
{
    addOutputText( "MatchWakes\n" );
    MatchWakes();

    addOutputText( "TessellateChains\n" );
    TessellateChains();

    TieCoincidentBorderChains();

//DebugWriteChains( "Tess_UW", true );

    addOutputText( "MergeBorderEndPoints\n" );
    MergeBorderEndPoints();

    MergeCoincidentTessPnts();

    JoinLooseChainEnds();

    AddWakeCoPlanarSurfaceChains();

    AddCoplanarChains();

    // addOutputText( "BuildMesh\n" );  Output in BuildMesh
    BuildMesh();

    addOutputText( "RemoveInteriorTris\n" );
    RemoveInteriorTris();

    addOutputText( "RemoveTrimTris\n" );
    RemoveTrimTris();
}

// The point at u along a chain's A curve.
static vec3d ChainPnt( ISegChain* c, double u )
{
    vec3d uw = c->m_ACurve.CompPntUW( u );
    return c->m_SurfA->CompPnt( uw.x(), uw.y() );
}

// Where along chain c the point q lies, near the tessellation points k and k + 1.  The interval
// that holds the point across from q is halved until it closes on it.
static double ChainParm( ISegChain* c, const vector< double > &utess, int k, const vec3d &q )
{
    for ( int m = k - 1 ; m <= k + 1 ; m++ )
    {
        if ( m < 0 || m + 1 >= ( int )utess.size() )
        {
            continue;
        }

        double ulo = utess[m];
        double uhi = utess[m + 1];

        vec3d dir = ChainPnt( c, uhi ) - ChainPnt( c, ulo );

        double glo = dot( ChainPnt( c, ulo ) - q, dir );
        double ghi = dot( ChainPnt( c, uhi ) - q, dir );

        if ( glo == 0.0 )
        {
            return ulo;
        }
        if ( ghi == 0.0 )
        {
            return uhi;
        }
        if ( ( glo < 0.0 ) == ( ghi < 0.0 ) )
        {
            continue;
        }

        for ( int i = 0 ; i < 60 ; i++ )
        {
            double umid = 0.5 * ( ulo + uhi );
            double gmid = dot( ChainPnt( c, umid ) - q, dir );

            if ( ( gmid < 0.0 ) == ( glo < 0.0 ) )
            {
                ulo = umid;
                glo = gmid;
            }
            else
            {
                uhi = umid;
            }
        }

        return 0.5 * ( ulo + uhi );
    }

    return utess[k];
}

// The distance from q to the segment from a to b.
static double SegDist( const vec3d &a, const vec3d &b, const vec3d &q )
{
    vec3d ab = b - a;
    double len2 = dot( ab, ab );
    double t = 0.0;
    if ( len2 > 0.0 )
    {
        t = dot( q - a, ab ) / len2;
    }
    t = clamp( t, 0.0, 1.0 );
    return dist( a + ab * t, q );
}

// Where a face lying on a face of another body shares an edge with both bodies, the edge is a
// border four ways over: the face and the body it closes on one chain, the other face and its
// body on another.  Which surfaces keep their triangles along it is decided only when the
// inside/outside tests are made, so the two chains are made one: the coarser takes the finer's
// points, and each point is shared by all four surfaces.
void CfdMeshMgrSingleton::TieCoincidentBorderChains()
{
    double tol = GetGridDensityPtr()->m_MinLen / 100.0;

    vector< ISegChain* > borders;
    for ( list< ISegChain* >::iterator c = m_ISegChainList.begin() ; c != m_ISegChainList.end(); ++c )
    {
        if ( ( *c )->m_BorderFlag && ( *c )->m_TessVec.size() > 1 && !( *c )->GetWakeAttachChain() )
        {
            borders.push_back( *c );
        }
    }

    set< ISegChain* > tied;

    for ( int i = 0 ; i < ( int )borders.size() ; i++ )
    {
        ISegChain* c1 = borders[i];

        // The face of the other body that one of this chain's surfaces lies on.
        Surf* other = c1->m_SurfA->GetCoincidentSurf();
        if ( !other )
        {
            other = c1->m_SurfB->GetCoincidentSurf();
        }
        if ( !other || tied.count( c1 ) )
        {
            continue;
        }

        vec3d a0 = c1->m_TessVec.front()->m_Pnt;
        vec3d a1 = c1->m_TessVec.back()->m_Pnt;

        for ( int j = 0 ; j < ( int )borders.size() ; j++ )
        {
            ISegChain* c2 = borders[j];

            if ( c2 == c1 || tied.count( c2 ) || ( c2->m_SurfA != other && c2->m_SurfB != other ) ||
                 c2->m_SurfA == c1->m_SurfA || c2->m_SurfA == c1->m_SurfB ||
                 c2->m_SurfB == c1->m_SurfA || c2->m_SurfB == c1->m_SurfB )
            {
                continue;
            }

            vec3d b0 = c2->m_TessVec.front()->m_Pnt;
            vec3d b1 = c2->m_TessVec.back()->m_Pnt;

            bool fwd = dist( a0, b0 ) < tol && dist( a1, b1 ) < tol;
            bool bkwd = dist( a0, b1 ) < tol && dist( a1, b0 ) < tol;
            if ( !fwd && !bkwd )
            {
                continue;
            }

            // The finer of the two lends its points to the other.
            ISegChain* from = c1;
            ISegChain* to = c2;
            if ( c2->m_TessVec.size() > c1->m_TessVec.size() )
            {
                from = c2;
                to = c1;
            }

            vector< double > utess = to->m_ACurve.GetUTessPnts();
            vector< vec3d > tpnts( utess.size() );
            for ( int k = 0 ; k < ( int )utess.size() ; k++ )
            {
                tpnts[k] = ChainPnt( to, utess[k] );
            }

            vector< double > ftess = from->m_ACurve.GetUTessPnts();
            vector< vec3d > fpnts( ftess.size() );
            for ( int k = 0 ; k < ( int )fpnts.size() ; k++ )
            {
                fpnts[k] = ChainPnt( from, ftess[k] );
            }
            if ( bkwd )
            {
                std::reverse( fpnts.begin(), fpnts.end() );
            }

            // Where each point of the one chain lies along the other, which it must lie on.
            int n = ( int )fpnts.size();
            vector< double > unew( n );
            unew[0] = 0.0;
            unew[n - 1] = 1.0;
            bool on = true;
            for ( int k = 1 ; k < n - 1 && on ; k++ )
            {
                int mmin = 0;
                double dmin = 1.0e12;
                for ( int m = 0 ; m + 1 < ( int )tpnts.size() ; m++ )
                {
                    double d = SegDist( tpnts[m], tpnts[m + 1], fpnts[k] );
                    if ( d < dmin )
                    {
                        dmin = d;
                        mmin = m;
                    }
                }
                unew[k] = ChainParm( to, utess, mmin, fpnts[k] );

                if ( dist( ChainPnt( to, unew[k] ), fpnts[k] ) > tol || unew[k] <= unew[k - 1] )
                {
                    on = false;
                }
            }
            if ( !on )
            {
                continue;
            }

            to->m_ACurve.Tesselate( unew );
            to->TransferTess();
            to->ApplyTess( this );

            for ( int k = 1 ; k < n - 1 ; k++ )
            {
                int kf = k;
                if ( bkwd )
                {
                    kf = n - 1 - k;
                }

                IPnt* mip = new IPnt();
                m_DelIPntVec.push_back( mip );
                mip->AddPuws( from->m_TessVec[kf] );
                mip->AddPuws( to->m_TessVec[k] );
                mip->CompPnt();

                from->m_TessVec[kf] = mip;
                to->m_TessVec[k] = mip;
            }

            tied.insert( c1 );
            tied.insert( c2 );
            break;
        }
    }
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
            ( *c )->m_ACurve.CleanupDistTable();
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
        else
        {
            m_DelISegChainVec.push_back( new_chains[i] );
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

// Union-find over the points of an IPntCloud, so that "within tol of" can be closed
// transitively.  Path compression on the lookup, union by nothing in particular.
static int FindPnt( vector< int > &parent, int i )
{
    while ( parent[i] != i )
    {
        parent[i] = parent[ parent[i] ];
        i = parent[i];
    }

    return i;
}

static void UnionPnts( vector< int > &parent, int a, int b )
{
    int ra = FindPnt( parent, a );
    int rb = FindPnt( parent, b );

    if ( ra != rb )
    {
        parent[rb] = ra;
    }
}

// Join every two points of the cloud whose squared distance is under tol.
static void UnionCloseIPnts( const IPntCloud &cloud, double tol, vector< int > &parent )
{
    PntNodeCloud pnts;
    pnts.ReserveMorePntNodes( cloud.m_IPnts.size() );
    for ( int i = 0 ; i < ( int )cloud.m_IPnts.size() ; i++ )
    {
        pnts.AddPntNode( cloud.m_IPnts[i]->m_Pnt );
    }

    BuildPntNodeIndex( pnts );

    vector< unsigned int > near;
    for ( int i = 0 ; i < ( int )cloud.m_IPnts.size() ; i++ )
    {
        FindPntNodesWithinTolSq( pnts, cloud.m_IPnts[i]->m_Pnt, tol, near );

        for ( int k = 0 ; k < ( int )near.size() ; k++ )
        {
            UnionPnts( parent, i, ( int )near[k] );
        }
    }
}

void CfdMeshMgrSingleton::MergeEndPointCloud( IPntCloud &cloud, double tol )
{
    list< ISegChain* >::iterator c;

    list < IPntGroup* > iPntGroupList;

    // Group the points that are within tol of each other, transitively.
    //
    // The point of this routine is that two chains meeting at one place end up sharing one
    // point.  Grouping by "seed a group, take everything in the radius, move on" does not
    // deliver that, because nearness is not transitive: with A near B and B near C but A not
    // near C, whichever way the groups fall A and C are told they are different points, and
    // whichever of them a chain happens to hold is where that chain ends.
    //
    // Union-find gives the transitive closure directly and cannot produce either fault: a
    // point is in exactly one set, and two points within tol are always in the same one.
    vector< int > parent( cloud.m_IPnts.size() );

    for ( size_t i = 0 ; i < parent.size() ; i++ )
    {
        parent[i] = ( int )i;
    }

    UnionCloseIPnts( cloud, tol, parent );

    //==== Collect Each Set ====//
    std::unordered_map< int, IPntGroup* > setGroup;

    for ( size_t i = 0 ; i < cloud.m_IPnts.size() ; i++ )
    {
        int r = FindPnt( parent, ( int )i );

        if ( !setGroup.count( r ) )
        {
            iPntGroupList.push_back( new IPntGroup );
            m_DelIPntGroupVec.push_back( iPntGroupList.back() );
            setGroup[r] = iPntGroupList.back();
        }

        cloud.m_IPnts[i]->m_GroupedFlag = true;
        setGroup[r]->m_IPntVec.push_back( cloud.m_IPnts[i] );
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
    //
    // Which merged point each original became, looked up rather than searched for.
    std::unordered_map< IPnt*, IPnt* > mergedOf;

    int cnt = 0;
    for ( g = iPntGroupList.begin() ; g != iPntGroupList.end(); ++g )
    {
        for ( int j = 0 ; j < ( int )( *g )->m_IPntVec.size() ; j++ )
        {
            mergedOf[ ( *g )->m_IPntVec[j] ] = merged_ipnts[cnt];
        }
        cnt++;
    }

    for ( c = m_ISegChainList.begin() ; c != m_ISegChainList.end(); ++c )
    {
        std::unordered_map< IPnt*, IPnt* >::iterator mit;

        mit = mergedOf.find( ( *c )->m_TessVec.front() );
        if ( mit != mergedOf.end() )
        {
            ( *c )->m_TessVec.front() = mit->second;
        }

        mit = mergedOf.find( ( *c )->m_TessVec.back() );
        if ( mit != mergedOf.end() )
        {
            ( *c )->m_TessVec.back() = mit->second;
        }
    }
}

// Where two curves run on top of one another for a stretch -- an intersection curve along a
// surface's border, say -- their points lie a hair apart.  A surface that has both curves has to
// make them one point, and a surface that has only one of the curves cannot follow it.  So the
// points handed to the triangulator that lie within the chain end tolerance of each other, on a
// surface they share, are made one point here, for every surface at once.  Two points next to
// each other on one chain are never made one.
static bool ShareSurf( IPnt* a, IPnt* b )
{
    for ( int i = 0 ; i < ( int )a->m_Puws.size() ; i++ )
    {
        for ( int j = 0 ; j < ( int )b->m_Puws.size() ; j++ )
        {
            if ( a->m_Puws[i]->m_Surf == b->m_Puws[j]->m_Surf )
            {
                return true;
            }
        }
    }
    return false;
}

void CfdMeshMgrSingleton::MergeCoincidentTessPnts()
{
    IPntCloud cloud;
    std::unordered_map< IPnt*, int > cloudIndex;
    std::set< std::pair< IPnt*, IPnt* > > neighbours;

    list< ISegChain* >::iterator c;
    for ( c = m_ISegChainList.begin() ; c != m_ISegChainList.end(); ++c )
    {
        vector< IPnt* > &tv = ( *c )->m_TessVec;
        for ( int j = 0 ; j < ( int )tv.size() ; j += 2 )
        {
            if ( !cloudIndex.count( tv[j] ) )
            {
                cloudIndex[ tv[j] ] = ( int )cloud.m_IPnts.size();
                cloud.m_IPnts.push_back( tv[j] );
            }
            if ( j >= 2 )
            {
                neighbours.insert( std::make_pair( tv[j - 2], tv[j] ) );
                neighbours.insert( std::make_pair( tv[j], tv[j - 2] ) );
            }
        }
    }

    if ( cloud.m_IPnts.empty() )
    {
        return;
    }

    double tol = GetGridDensityPtr()->m_MinLen / 100.0;
    tol = tol * tol;

    // The constructor builds the tree.
    IPntTree index( 3, cloud, KDTreeSingleIndexAdaptorParams( 10 ) );

    vector< int > parent( cloud.m_IPnts.size() );
    for ( size_t i = 0 ; i < parent.size() ; i++ )
    {
        parent[i] = ( int )i;
    }

    for ( size_t i = 0 ; i < cloud.m_IPnts.size() ; i++ )
    {
        std::vector < nanoflann::ResultItem < unsigned int, double > > ret_matches;
        nanoflann::SearchParameters params;
        size_t nmatch = index.radiusSearch( &cloud.m_IPnts[i]->m_Pnt[0], tol, ret_matches, params );

        for ( size_t j = 0 ; j < nmatch ; j++ )
        {
            IPnt* a = cloud.m_IPnts[i];
            IPnt* b = cloud.m_IPnts[ ret_matches[j].first ];
            if ( a != b && !neighbours.count( std::make_pair( a, b ) ) && ShareSurf( a, b ) )
            {
                UnionPnts( parent, ( int )i, ( int )ret_matches[j].first );
            }
        }
    }

    // Each set of more than one point, in the order its first point was met
    vector< vector< int > > groups;
    std::unordered_map< int, int > groupOf;
    for ( size_t i = 0 ; i < cloud.m_IPnts.size() ; i++ )
    {
        int r = FindPnt( parent, ( int )i );
        if ( !groupOf.count( r ) )
        {
            groupOf[r] = ( int )groups.size();
            groups.push_back( vector< int >() );
        }
        groups[ groupOf[r] ].push_back( ( int )i );
    }

    std::unordered_map< IPnt*, IPnt* > mergedOf;
    for ( int g = 0 ; g < ( int )groups.size() ; g++ )
    {
        vector< int > &grp = groups[g];
        if ( grp.size() < 2 )
        {
            continue;
        }

        // A set reached through a third point can still hold two neighbours
        bool ok = true;
        for ( int i = 0 ; i < ( int )grp.size() && ok ; i++ )
        {
            for ( int j = i + 1 ; j < ( int )grp.size() && ok ; j++ )
            {
                if ( neighbours.count( std::make_pair( cloud.m_IPnts[ grp[i] ], cloud.m_IPnts[ grp[j] ] ) ) )
                {
                    ok = false;
                }
            }
        }
        if ( !ok )
        {
            continue;
        }

        IPnt* mip = new IPnt();
        m_DelIPntVec.push_back( mip );
        for ( int i = 0 ; i < ( int )grp.size() ; i++ )
        {
            mip->AddPuws( cloud.m_IPnts[ grp[i] ] );
            mergedOf[ cloud.m_IPnts[ grp[i] ] ] = mip;
        }
        mip->CompPnt();
    }

    if ( mergedOf.empty() )
    {
        return;
    }

    for ( c = m_ISegChainList.begin() ; c != m_ISegChainList.end(); ++c )
    {
        vector< IPnt* > &tv = ( *c )->m_TessVec;
        for ( int j = 0 ; j < ( int )tv.size() ; j += 2 )
        {
            std::unordered_map< IPnt*, IPnt* >::iterator mit = mergedOf.find( tv[j] );
            if ( mit != mergedOf.end() )
            {
                tv[j] = mit->second;
            }
        }
    }
}

// An intersection curve between two closed surfaces ends only where it meets another curve, so a
// chain end that no other chain shares is where the curve was lost: two surfaces that meet where
// they are tangent are traced only so far into the point.  Each such end is moved onto the
// nearest point another chain passes through or ends on, within the minimum edge length -- one
// on both of the chain's surfaces where there is one -- and that point is given a place on any of
// the two it lacks.  The point halfway along the chain's end segment is moved with it.
void CfdMeshMgrSingleton::JoinLooseChainEnds()
{
    // How many chain ends each point is: two for a point a chain passes through
    std::unordered_map< IPnt*, int > ends;
    list< ISegChain* >::iterator c;
    for ( c = m_ISegChainList.begin() ; c != m_ISegChainList.end(); ++c )
    {
        vector< IPnt* > &tv = ( *c )->m_TessVec;
        for ( int j = 0 ; j < ( int )tv.size() ; j += 2 )
        {
            if ( j == 0 || j == ( int )tv.size() - 1 )
            {
                ends[ tv[j] ] += 1;
            }
            else
            {
                ends[ tv[j] ] += 2;
            }
        }
    }

    double rad = GetGridDensityPtr()->m_MinLen;
    int njoin = 0;

    for ( c = m_ISegChainList.begin() ; c != m_ISegChainList.end(); ++c )
    {
        ISegChain* chain = *c;
        vector< IPnt* > &tv = chain->m_TessVec;
        if ( chain->m_BorderFlag || chain->m_SurfA == chain->m_SurfB || tv.size() < 3 )
        {
            continue;
        }

        Surf* surfs[2] = { chain->m_SurfA, chain->m_SurfB };

        for ( int e = 0 ; e < 2 ; e++ )
        {
            int iend = 0;
            int imid = 1;
            if ( e == 1 )
            {
                iend = ( int )tv.size() - 1;
                imid = iend - 1;
            }

            IPnt* ip = tv[ iend ];
            if ( ends[ ip ] != 1 )
            {
                continue;
            }

            IPnt* best = nullptr;
            int bestshare = 0;
            double bestd = rad;

            list< ISegChain* >::iterator d;
            for ( d = m_ISegChainList.begin() ; d != m_ISegChainList.end(); ++d )
            {
                if ( *d == chain )
                {
                    continue;
                }

                vector< IPnt* > &dv = ( *d )->m_TessVec;
                for ( int j = 0 ; j < ( int )dv.size() ; j += 2 )
                {
                    IPnt* q = dv[j];

                    int share = 0;
                    for ( int s = 0 ; s < 2 ; s++ )
                    {
                        if ( q->GetPuw( surfs[s] ) )
                        {
                            share++;
                        }
                    }

                    double dq = dist( ip->m_Pnt, q->m_Pnt );
                    if ( share > 0 && dq < rad && ( share > bestshare || ( share == bestshare && dq < bestd ) ) )
                    {
                        best = q;
                        bestshare = share;
                        bestd = dq;
                    }
                }
            }

            if ( !best )
            {
                continue;
            }

            for ( int s = 0 ; s < 2 ; s++ )
            {
                if ( !best->GetPuw( surfs[s] ) )
                {
                    Puw* puw = new Puw( surfs[s], surfs[s]->ClosestUW( best->m_Pnt ) );
                    m_DelPuwVec.push_back( puw );
                    best->m_Puws.push_back( puw );
                }
            }

            tv[ iend ] = best;
            ends[ ip ] -= 1;
            ends[ best ] += 1;

            IPnt* mid = tv[ imid ];
            IPnt* other = tv[ 2 * imid - iend ];
            for ( int s = 0 ; s < 2 ; s++ )
            {
                Puw* pm = mid->GetPuw( surfs[s] );
                if ( pm )
                {
                    pm->m_UW = 0.5 * ( best->GetPuw( surfs[s] )->m_UW + other->GetPuw( surfs[s] )->m_UW );
                }
            }
            mid->CompPnt();

            njoin++;
        }
    }

    if ( njoin > 0 )
    {
        printf( "Joined %d intersection curve ends that met no other curve.\n", njoin );
        fflush( stdout );
    }
}

// Two curves that run close together can cross once each is cut into straight segments and
// taken to the parameters the triangulator works in, though the curves themselves do not.
// The triangulator cannot hold two constraints that cross.  Split the segments involved, on
// the chains themselves, until none do: a chain belongs to the surfaces on both sides of it, so
// both are given the same points.
void CfdMeshMgrSingleton::RemoveCrossingTessSegs( const vector < vector < ISegChain* > > &surf_chains, int nthread )
{
    int n = m_SurfVec.size();
    int nsplit = 0;
    int npass = 0;
    const int maxpass = 8;

    // The number of passes is not known ahead, so each pass counts the surfaces again on one line.
    ProgressStage bar( this, "Crossing Segs 1", n );

    for ( npass = 0; npass < maxpass; npass++ )
    {
        vector < vector < pair < ISegChain*, int > > > found( n );

        if ( npass > 0 )
        {
            bar.Restart( "Crossing Segs " + std::to_string( npass + 1 ) );
        }

        RunIndexed( n, nthread, [&]( int s )
        {
            m_SurfVec[s]->FindCrossingTessSegs( surf_chains[s], found[s] );
        }, &bar );

        map < ISegChain*, set < int > > tosplit;
        for ( int s = 0; s < n; s++ )
        {
            for ( int i = 0; i < ( int )found[s].size(); i++ )
            {
                tosplit[ found[s][i].first ].insert( found[s][i].second );
            }
        }

        if ( tosplit.empty() )
        {
            break;
        }

        map < ISegChain*, set < int > >::iterator it;
        for ( it = tosplit.begin(); it != tosplit.end(); ++it )
        {
            // From the end, so the indices still to be split do not move.
            set < int >::reverse_iterator j;
            for ( j = it->second.rbegin(); j != it->second.rend(); ++j )
            {
                it->first->SplitTessSeg( *j, this );
                nsplit++;
            }
        }
    }

    bar.End();

    if ( nsplit > 0 )
    {
        const char *left = "";
        if ( npass == maxpass )
        {
            left = ", and some still cross";
        }
        printf( "Split %d mesh constraint segments that crossed, in %d passes%s.\n", nsplit, npass, left );
        fflush( stdout );
    }
}

void CfdMeshMgrSingleton::BuildMesh()
{
    int n = m_SurfVec.size();

#ifdef DEBUG_CFD_MESH
    BeginDebugSurfFiles();
#endif

    // Which chains lie on each surface.  Asking the whole chain list once per surface is the
    // list walked once for every surface; one walk fills them all.
    vector < vector < ISegChain* > > surf_chains( n );

    list< ISegChain* >::iterator c;
    for ( c = m_ISegChainList.begin() ; c != m_ISegChainList.end(); ++c )
    {
        for ( int s = 0; s < n; s++ )
        {
            if ( ( *c )->m_SurfA == m_SurfVec[s] || ( *c )->m_SurfB == m_SurfVec[s] )
            {
                surf_chains[s].push_back( *c );
            }
        }
    }

    // The fix points a structure forces into a surface.  Worked out here because it reads the
    // mesh's own fix point list, which the surfaces below are not given.
    vector < vector < vec2d > > adduw( n );
    for ( int s = 0; s < n; s++ )
    {
        ForceSurfaceFixPoints( s, adduw[s] );
    }

    // Each surface builds its own mesh from its own chains, and reaches nothing the others
    // touch.  The debug output is the exception: it numbers surfaces and writes them to files
    // shared across the pass, so with it on the surfaces are taken one at a time.
    int nthread = 1;

#ifndef DEBUG_CFD_MESH
    nthread = StageThreadCount( n );
#endif

    RemoveCrossingTessSegs( surf_chains, nthread );

    BeginProgress( "InitMesh", n, VOCAL_OUTPUT );

    RunIndexed( n, nthread, [&]( int s )
    {
        m_SurfVec[s]->InitMesh( surf_chains[s], adduw[s], this );

        m_ProgressDone++;
        StepProgress( VOCAL_OUTPUT );
    } );

    EndProgress( VOCAL_OUTPUT );

#ifdef DEBUG_CFD_MESH
    EndDebugSurfFiles();
#endif
}

// Determines if a triangle should be deleted based on its type and whether or not it is inside every other surface
bool CfdMeshMgrSingleton::SetDeleteTriFlag( int aType, bool symPlane, const vector < bool > &aInB )
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

// One surface's share of the interior/exterior test.  It casts a ray from each of its own
// triangles against every other surface and writes the verdict onto its own faces, so the
// surfaces are independent of each other here.
// Face k of surface s is inside component c where inside[ k * ncross + c ], and count holds the
// votes of its neighbours the same way.  A face lying on a surface in its plane takes the
// verdict for that surface's component from coplanar[ k ] instead of from its ray.
void CfdMeshMgrSingleton::RemoveInteriorTrisOneSurf( int s, double x_dist, int ncross, const vector< vector< pair< int, bool > > > &coplanar,
                                                     vector< bool > &inside, vector< int > &count )
{
    int s_comp_id = m_SurfVec[s]->GetCompID();

    // A reference.  The list is only walked here, and copying it would copy a node per triangle.
    const list <Face*> &faceList = m_SurfVec[ s ]->GetMesh()->GetFaceList();

    inside.assign( faceList.size() * ncross, false );
    count.assign( faceList.size() * ncross, 0 );

    // Built once and emptied per triangle.  It is a vector of vectors as wide as the model has
    // components.
    vector< vector< double > > t_vec_vec( ncross );

    int pairc = -1;
    bool pairinside = false;
    CoincidentVerdict( m_SurfVec[s], pairc, pairinside );

    vector< int > joined;
    JoinedComps( s_comp_id, joined );

    BndBox big_box;
    for ( int i = 0 ; i < ( int )m_SurfVec.size() ; i++ )
    {
        big_box.Update( m_SurfVec[i]->GetBBox() );
    }
    double ray_len = 1.0 + big_box.DiagDist();

    int k = 0;
    list< Face* >::const_iterator f;
    for ( f = faceList.begin() ; f != faceList.end(); ++f, ++k ) // every triangle
    {
        ( *f )->m_Index = k;

        for ( int i = 0 ; i < ncross ; i++ )
        {
            t_vec_vec[i].clear();
        }

        vec3d cp = ( *f )->ComputeCenterPnt( m_SurfVec[s] );
        vec3d ep = cp + vec3d( x_dist, 1.0e-4, 1.0e-4 );

        for ( int i = 0 ; i < ( int )m_SurfVec.size() ; ++i )
        {
            int comp_id = m_SurfVec[i]->GetCompID();
            if ( i != s && comp_id != s_comp_id ) // Don't check self intersection.
            {
                if ( m_SurfVec[s]->GetFeaSymmIndex() >=0 && m_SurfVec[i]->GetFeaSymmIndex() >=0 &&
                     m_SurfVec[s]->GetFeaSymmIndex() != m_SurfVec[i]->GetFeaSymmIndex() )
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

            if ( c >= 0 && c < ncross )
            {
                if ( m_SurfVec[s]->GetSymPlaneFlag() && m_SurfVec[i]->GetFarFlag() &&
                     GetSettingsPtr()->m_FarCompFlag )
                {
                    if ( ( int )( t_vec_vec[c].size() + 1 ) % 2 == 1 ) // +1 Reverse action on sym plane wrt outer boundary.
                    {
                        inside[ k * ncross + c ] = true;
                    }
                }
                else
                {

                    if ( ( int )t_vec_vec[c].size() % 2 == 1)
                    {
                        inside[ k * ncross + c ] = true;
                    }
                }
            }
        }

        if ( k < ( int )coplanar.size() )
        {
            for ( int i = 0 ; i < ( int )coplanar[k].size() ; i++ )
            {
                int c = coplanar[k][i].first;
                if ( c >= 0 && c < ncross )
                {
                    inside[ k * ncross + c ] = coplanar[k][i].second;
                }
            }
        }

        for ( int i = 0 ; i < ( int )joined.size() ; i++ )
        {
            int c = joined[i];
            if ( c >= 0 && c < ncross )
            {
                inside[ k * ncross + c ] = InsideByThreeRays( c, cp, ray_len );
            }
        }

        if ( pairc >= 0 && pairc < ncross )
        {
            inside[ k * ncross + pairc ] = pairinside;
        }
    }

    // A component votes once for each of its surfaces.
    vector< pair< int, int > > compvotes;
    CompSurfCounts( ncross, compvotes );

    for ( f = faceList.begin() ; f != faceList.end(); ++f ) // every face
    {
        //==== Load Adjoining Faces - NOT Crossing Borders ====//
        set< Face* > faceSet;
        ( *f )->LoadAdjFaces( 3, faceSet );

        set<Face*>::iterator sf;

        for ( int i = 0 ; i < ( int )compvotes.size() ; ++i )
        {
            int c = compvotes[i].first;
            int vote = compvotes[i].second;

            if ( !inside[ ( *f )->m_Index * ncross + c ] )
            {
                vote = -vote;
            }

            for ( sf = faceSet.begin() ; sf != faceSet.end() ; ++sf )
            {
                count[ ( *sf )->m_Index * ncross + c ] += vote;
            }
        }
    }
}

// Each component in [0, ncross) that has a surface, with how many surfaces it has, in the
// order the components are first met among the surfaces.
void CfdMeshMgrSingleton::CompSurfCounts( int ncross, vector< pair< int, int > > &compcounts )
{
    compcounts.clear();
    vector< int > slot( ncross, -1 );

    for ( int i = 0 ; i < ( int )m_SurfVec.size() ; ++i )
    {
        int c = m_SurfVec[i]->GetCompID();
        if ( c >= 0 && c < ncross )
        {
            if ( slot[c] < 0 )
            {
                slot[c] = ( int )compcounts.size();
                compcounts.push_back( pair< int, int >( c, 0 ) );
            }
            compcounts[ slot[c] ].second++;
        }
    }
}

// The root of i's set.
static int TopoFind( vector< int > &p, int i )
{
    while ( p[i] != i )
    {
        p[i] = p[ p[i] ];
        i = p[i];
    }
    return i;
}

// A chain segment, by its chain and its ends in a fixed order.  The meshes of the surfaces
// meeting there take the ends from the same tessellation points, so they key it alike.
typedef std::tuple< ISegChain*, double, double, double, double, double, double > ChainSegKey;

static ChainSegKey MakeChainSegKey( ISegChain* c, const vec3d &a, const vec3d &b )
{
    vec3d p0 = a;
    vec3d p1 = b;
    if ( p1.x() < p0.x() || ( p1.x() == p0.x() && ( p1.y() < p0.y() || ( p1.y() == p0.y() && p1.z() < p0.z() ) ) ) )
    {
        std::swap( p0, p1 );
    }
    return std::make_tuple( c, p0.x(), p0.y(), p0.z(), p1.x(), p1.y(), p1.z() );
}

static const int TOPO_MAX_SEEDS = 5;     // Walks of each set
static const int TOPO_MAX_PASSES = 10;   // Walks of a set as conflicts are settled by ray
static const int TOPO_MAX_BORDER = 5;    // Faces along a settled conflict checked by ray

// Two regions that a walk reached with different flags for a component: region, region, component.
typedef std::tuple< int, int, int > TopoClash;

// The classification CfdMeshMgrSingleton::TopoInteriorTris describes, one step to a method.
class TopoClassifier
{
public:
    TopoClassifier( CfdMeshMgrSingleton* mgr, const vector< Surf* > &surfs, int ncross );

    void Classify( vector< vector< char > > &del, vector< char > &handled );

private:

    void FindHandledSurfs();
    void BuildRegions();
    void FindThickComps();
    void SideTests();
    void SideTest( Surf* srf, Face* f, Edge* e, ISegChain* c, int r );
    void RuleFacts();
    void LinkRegions();
    vector< char > RayFlags( int r );
    void Walk( int seed, vector< vector< char > > &out, vector< TopoClash > &clash );
    bool InsideComp( int c, const vec3d &p ) const;
    void WalkGroup( const vector< int > &group, vector< TopoClash > &settled );
    void CheckSettled();
    void CheckSettledSide( int x, int other, int c );
    void Audit();

    CfdMeshMgrSingleton* m_Mgr;
    const vector< Surf* > &m_Surfs;
    int m_NCross;
    double m_RayLen;
    double m_XDist;

    vector< char > m_Handled;               // Per surface: classified here

    // Regions: per surface, its faces and each face's region; per region, its surface, its faces
    // and a point well inside it.
    vector< vector< Face* > > m_Faces;
    vector< vector< int > > m_Region;
    vector< int > m_RegSurf;
    vector< vector< Face* > > m_RegFaces;
    vector< vec3d > m_Inner;

    vector< char > m_Thick;                 // Per component: one SetDeleteTriFlag acts on
    vector< int > m_Comps;
    vector< BndBox > m_CompBox;             // Per component: around the surfaces a ray counts

    // Per region: side tests of each component met across a chain, the components met, what is
    // known of its flags, whether it is left to the ray cast, and its flags.
    vector< std::map< int, pair< int, int > > > m_Vote;
    vector< std::set< int > > m_Touched;
    vector< std::map< int, char > > m_Fact;
    vector< char > m_BadReg;
    vector< vector< char > > m_Val;

    // The regions and faces at each chain segment, the segments each region lies along, the
    // regions each is joined to through them, and the sets those joins make.
    std::map< ChainSegKey, vector< pair< int, Face* > > > m_SegFaces;
    vector< vector< const vector< pair< int, Face* > >* > > m_RegSegs;
    vector< vector< int > > m_Adj;
    std::map< int, vector< int > > m_Groups;

    vector< TopoClash > m_Settled;          // Conflicts settled by ray
};

TopoClassifier::TopoClassifier( CfdMeshMgrSingleton* mgr, const vector< Surf* > &surfs, int ncross ) :
    m_Mgr( mgr ), m_Surfs( surfs ), m_NCross( ncross )
{
    BndBox big_box;
    for ( int s = 0 ; s < ( int )m_Surfs.size() ; s++ )
    {
        big_box.Update( m_Surfs[s]->GetBBox() );
    }
    m_RayLen = 1.0 + big_box.DiagDist();
    m_XDist = 1.0 + big_box.GetMax( 0 ) - big_box.GetMin( 0 );
}

void TopoClassifier::Classify( vector< vector< char > > &del, vector< char > &handled )
{
    FindHandledSurfs();
    BuildRegions();
    FindThickComps();
    SideTests();
    RuleFacts();
    LinkRegions();

    // The sets share no region, so they are walked side by side, and the conflicts each settles
    // are gathered in the order of the sets.
    vector< const vector< int >* > groups;
    for ( std::map< int, vector< int > >::iterator g = m_Groups.begin() ; g != m_Groups.end() ; ++g )
    {
        groups.push_back( &g->second );
    }
    int ngroup = groups.size();
    vector< vector< TopoClash > > settled( ngroup );
    m_Mgr->RunStage( "Topology Walk", ngroup, [&]( int g )
    {
        WalkGroup( *groups[g], settled[g] );
    } );
    for ( int g = 0 ; g < ngroup ; g++ )
    {
        m_Settled.insert( m_Settled.end(), settled[g].begin(), settled[g].end() );
    }

    CheckSettled();
    Audit();

    // A surface with a region left to the ray cast is left to it whole.
    for ( int r = 0 ; r < ( int )m_RegSurf.size() ; r++ )
    {
        if ( m_BadReg[r] )
        {
            m_Handled[ m_RegSurf[r] ] = 0;
        }
    }

    // Every face of a region takes the region's verdict.
    int nreg = m_RegSurf.size();
    vector< char > reg_del( nreg, 0 );
    vector< bool > fin( m_NCross );
    for ( int r = 0 ; r < nreg ; r++ )
    {
        if ( !m_Handled[ m_RegSurf[r] ] )
        {
            continue;
        }
        Surf* srf = m_Surfs[ m_RegSurf[r] ];
        for ( int c = 0 ; c < m_NCross ; c++ )
        {
            fin[c] = m_Val[r][c] != 0;
        }
        reg_del[r] = m_Mgr->SetDeleteTriFlag( srf->GetSurfaceCfdType(), srf->GetSymPlaneFlag(), fin );
    }

    int nsurf = m_Surfs.size();
    handled = m_Handled;
    del.assign( nsurf, vector< char >() );
    for ( int s = 0 ; s < nsurf ; s++ )
    {
        if ( !handled[s] )
        {
            continue;
        }
        del[s].assign( m_Faces[s].size(), 0 );
        for ( int k = 0 ; k < ( int )m_Faces[s].size() ; k++ )
        {
            del[s][k] = reg_del[ m_Region[s][k] ];
        }
    }
}

// The surfaces whose rules need nothing but the inside flags.  A structure's surfaces carry
// symmetric copies the rays tell apart, and are left to the ray cast.
void TopoClassifier::FindHandledSurfs()
{
    m_Handled.assign( m_Surfs.size(), 0 );
    for ( int s = 0 ; s < ( int )m_Surfs.size() ; s++ )
    {
        Surf* srf = m_Surfs[s];
        int type = srf->GetSurfaceCfdType();

        if ( !srf->GetIgnoreSurfFlag() && !srf->GetWakeFlag() && !srf->GetSymPlaneFlag() && !srf->GetFarFlag() &&
             srf->GetCoplanarSurfs().empty() && srf->GetFeaSymmIndex() < 0 &&
             type != vsp::CFD_STRUCTURE && type != vsp::CFD_STIFFENER &&
             srf->GetCompID() >= 0 && srf->GetCompID() < m_NCross )
        {
            m_Handled[s] = 1;
        }
    }
}

// Regions: faces of a surface joined across edges that are not borders.  Each region keeps its
// faces in the order of the surface's face list, and a point well inside it: the centre of a
// face with no border edge, if it has one.
void TopoClassifier::BuildRegions()
{
    int nsurf = m_Surfs.size();
    m_Faces.assign( nsurf, vector< Face* >() );
    m_Region.assign( nsurf, vector< int >() );
    for ( int s = 0 ; s < nsurf ; s++ )
    {
        if ( !m_Handled[s] )
        {
            continue;
        }
        const list< Face* > &fl = m_Surfs[s]->GetMesh()->GetFaceList();
        int k = 0;
        for ( list< Face* >::const_iterator f = fl.begin() ; f != fl.end() ; ++f, ++k )
        {
            ( *f )->m_Index = k;
            m_Faces[s].push_back( *f );
        }
        m_Region[s].assign( m_Faces[s].size(), -1 );

        for ( int k0 = 0 ; k0 < ( int )m_Faces[s].size() ; k0++ )
        {
            if ( m_Region[s][k0] >= 0 )
            {
                continue;
            }
            int rid = m_RegSurf.size();
            m_RegSurf.push_back( s );
            m_RegFaces.push_back( vector< Face* >() );
            vector< int > stack( 1, k0 );
            m_Region[s][k0] = rid;
            while ( !stack.empty() )
            {
                int k1 = stack.back();
                stack.pop_back();
                Face* f = m_Faces[s][k1];
                Edge* ev[4] = { f->e0, f->e1, f->e2, f->e3 };
                for ( int j = 0 ; j < 4 ; j++ )
                {
                    if ( !ev[j] || ev[j]->border )
                    {
                        continue;
                    }
                    Face* g = ev[j]->OtherFace( f );
                    if ( g && m_Region[s][ g->m_Index ] < 0 )
                    {
                        m_Region[s][ g->m_Index ] = rid;
                        stack.push_back( g->m_Index );
                    }
                }
            }
        }
        for ( int k = 0 ; k < ( int )m_Faces[s].size() ; k++ )
        {
            m_RegFaces[ m_Region[s][k] ].push_back( m_Faces[s][k] );
        }
    }

    int nreg = m_RegSurf.size();
    m_Inner.assign( nreg, vec3d() );
    for ( int r = 0 ; r < nreg ; r++ )
    {
        Face* pick = m_RegFaces[r][0];
        for ( int k = 0 ; k < ( int )m_RegFaces[r].size() ; k++ )
        {
            Face* f = m_RegFaces[r][k];
            if ( !f->e0->border && !f->e1->border && !f->e2->border && ( !f->e3 || !f->e3->border ) )
            {
                pick = f;
                break;
            }
        }
        m_Inner[r] = pick->ComputeCenterPnt( m_Surfs[ m_RegSurf[r] ] );
    }

    m_Vote.assign( nreg, std::map< int, pair< int, int > >() );
    m_Touched.assign( nreg, std::set< int >() );
    m_Fact.assign( nreg, std::map< int, char >() );
    m_BadReg.assign( nreg, 0 );
    m_Val.assign( nreg, vector< char >() );
}

// The components whose inside SetDeleteTriFlag acts on -- thick ones, normal or negative --
// and every component.
void TopoClassifier::FindThickComps()
{
    m_Thick.assign( m_NCross, 0 );
    m_CompBox.assign( m_NCross, BndBox() );
    for ( int s = 0 ; s < ( int )m_Surfs.size() ; s++ )
    {
        int c = m_Surfs[s]->GetCompID();
        int type = m_Surfs[s]->GetSurfaceCfdType();
        if ( c < 0 || c >= m_NCross )
        {
            continue;
        }
        if ( type == vsp::CFD_NORMAL || type == vsp::CFD_NEGATIVE )
        {
            m_CompBox[c].Update( m_Surfs[s]->GetBBox() );
        }
        if ( m_Surfs[s]->GetSurfaceThickSurf() && ( type == vsp::CFD_NORMAL || type == vsp::CFD_NEGATIVE ) )
        {
            m_Thick[c] = 1;
        }
        if ( std::find( m_Comps.begin(), m_Comps.end(), c ) == m_Comps.end() )
        {
            m_Comps.push_back( c );
        }
    }

    // Grown a little, so a point on a component's skin is still taken to the rays.
    for ( int c = 0 ; c < m_NCross ; c++ )
    {
        if ( !m_CompBox[c].IsEmpty() )
        {
            m_CompBox[c].Expand( 1.0e-6 * m_RayLen );
        }
    }
}

// Whether p is inside component c, by the rays the ray cast casts.  A point outside the box
// around the surfaces they count is outside without them.
bool TopoClassifier::InsideComp( int c, const vec3d &p ) const
{
    if ( !m_CompBox[c].CheckPnt( p ) )
    {
        return false;
    }
    return m_Mgr->InsideByThreeRays( c, p, m_RayLen );
}

// Each region's side tests of every component it meets across an intersection chain, and the
// regions and faces at each chain segment.  A region's side of a component is a fact where its
// tests say; a region on both sides of one chain reaches round an end of it, through a gap no
// chain closes, and is not one thing.
void TopoClassifier::SideTests()
{
    for ( int s = 0 ; s < ( int )m_Surfs.size() ; s++ )
    {
        if ( !m_Handled[s] )
        {
            continue;
        }
        Surf* srf = m_Surfs[s];
        const BorderChainMap &bmap = srf->GetMesh()->GetBorderChainMap();
        for ( int k = 0 ; k < ( int )m_Faces[s].size() ; k++ )
        {
            Face* f = m_Faces[s][k];
            int r = m_Region[s][k];
            Edge* ev[4] = { f->e0, f->e1, f->e2, f->e3 };
            for ( int j = 0 ; j < 4 ; j++ )
            {
                if ( !ev[j] || !ev[j]->border )
                {
                    continue;
                }
                BorderChainMap::const_iterator it = bmap.find( ev[j] );
                if ( it == bmap.end() )
                {
                    continue;
                }
                ISegChain* both[2] = { it->second.first, it->second.second };
                for ( int ic = 0 ; ic < 2 ; ic++ )
                {
                    if ( both[ic] )
                    {
                        m_SegFaces[ MakeChainSegKey( both[ic], ev[j]->n0->pnt, ev[j]->n1->pnt ) ].push_back( std::make_pair( r, f ) );
                        SideTest( srf, f, ev[j], both[ic], r );
                    }
                }
            }
        }
    }

    for ( int r = 0 ; r < ( int )m_RegSurf.size() ; r++ )
    {
        for ( std::map< int, pair< int, int > >::iterator v = m_Vote[r].begin() ; v != m_Vote[r].end() ; ++v )
        {
            if ( v->second.first > 0 && v->second.second > 0 )
            {
                m_BadReg[r] = 1;
            }
            if ( v->second.first != v->second.second )
            {
                m_Fact[r][ v->first ] = v->second.first > v->second.second;
            }
        }
    }
}

// The side of intersection chain c that face f of region r lies on, for the thick component of
// the surface across it: the face's centre projected onto that surface, from the uw the chain
// holds nearest edge e, and compared with its outward normal there.  Left out where the two
// surfaces lie flat against each other.
void TopoClassifier::SideTest( Surf* srf, Face* f, Edge* e, ISegChain* c, int r )
{
    if ( c->m_BorderFlag || c->m_SurfA == c->m_SurfB )
    {
        return;
    }
    Surf* o = c->m_SurfA;
    if ( o == srf )
    {
        o = c->m_SurfB;
    }
    int oc = o->GetCompID();
    if ( oc == srf->GetCompID() || oc < 0 || oc >= m_NCross || !m_Thick[oc] )
    {
        return;
    }

    m_Touched[r].insert( oc );

    vec3d cen = f->ComputeCenterPnt( srf );
    vec3d mid = 0.5 * ( e->n0->pnt + e->n1->pnt );
    IPnt* best = nullptr;
    double dbest = 1.0e300;
    for ( int t = 0 ; t < ( int )c->m_TessVec.size() ; t++ )
    {
        double d = dist( c->m_TessVec[t]->m_Pnt, mid );
        if ( d < dbest )
        {
            dbest = d;
            best = c->m_TessVec[t];
        }
    }
    Puw* pw = nullptr;
    if ( best )
    {
        pw = best->GetPuw( o );
    }
    vec2d ouw;
    if ( pw )
    {
        ouw = o->ClosestUW( cen, pw->m_UW[0], pw->m_UW[1] );
    }
    else
    {
        ouw = o->ClosestUW( cen );
    }
    vec3d q = o->CompPnt( ouw[0], ouw[1] );
    vec3d n = OutwardNorm( o, ouw[0], ouw[1] );

    vec2d fuw = f->n0->uw + f->n1->uw + f->n2->uw;
    fuw = fuw * ( 1.0 / 3.0 );
    if ( SurfacesTangent( srf, vec3d( fuw[0], fuw[1], 0.0 ), n ) )
    {
        return;
    }

    if ( dot( cen - q, n ) < 0.0 )
    {
        m_Vote[r][oc].first++;
    }
    else
    {
        m_Vote[r][oc].second++;
    }
}

// A component joined to a surface's own, or lying coincident with it, has its own rule for a
// point of that surface, which the ray cast applies face by face and this region by region:
// a joined component's flag by ray, a coincident one's as CoincidentVerdict gives it.  Where
// every side test along a chain was left out, or the tests were even, the region's flag for
// that component is taken by ray.
void TopoClassifier::RuleFacts()
{
    int nreg = m_RegSurf.size();
    for ( int r = 0 ; r < nreg ; r++ )
    {
        Surf* srf = m_Surfs[ m_RegSurf[r] ];
        vector< int > joined;
        m_Mgr->JoinedComps( srf->GetCompID(), joined );
        for ( int i = 0 ; i < ( int )joined.size() ; i++ )
        {
            if ( joined[i] >= 0 && joined[i] < m_NCross )
            {
                m_Fact[r][ joined[i] ] = InsideComp( joined[i], m_Inner[r] );
            }
        }
        int pairc = -1;
        bool pairinside = false;
        m_Mgr->CoincidentVerdict( srf, pairc, pairinside );
        if ( pairc >= 0 && pairc < m_NCross )
        {
            m_Fact[r][pairc] = pairinside;
        }
    }

    for ( int r = 0 ; r < nreg ; r++ )
    {
        for ( std::set< int >::iterator oc = m_Touched[r].begin() ; oc != m_Touched[r].end() ; ++oc )
        {
            if ( m_Fact[r].count( *oc ) == 0 )
            {
                m_Fact[r][ *oc ] = InsideComp( *oc, m_Inner[r] );
            }
        }
    }
}

// Regions joined through a chain segment, each segment's regions in a star about its first; the
// segments each region lies along; and the sets of joined regions.
void TopoClassifier::LinkRegions()
{
    int nreg = m_RegSurf.size();
    m_Adj.assign( nreg, vector< int >() );
    m_RegSegs.assign( nreg, vector< const vector< pair< int, Face* > >* >() );
    vector< int > parent( nreg );
    for ( int r = 0 ; r < nreg ; r++ )
    {
        parent[r] = r;
    }
    for ( std::map< ChainSegKey, vector< pair< int, Face* > > >::iterator sg = m_SegFaces.begin() ; sg != m_SegFaces.end() ; ++sg )
    {
        vector< int > rv;
        for ( int i = 0 ; i < ( int )sg->second.size() ; i++ )
        {
            rv.push_back( sg->second[i].first );
        }
        std::sort( rv.begin(), rv.end() );
        rv.erase( std::unique( rv.begin(), rv.end() ), rv.end() );
        for ( int i = 0 ; i < ( int )rv.size() ; i++ )
        {
            m_RegSegs[ rv[i] ].push_back( &sg->second );
            if ( i > 0 )
            {
                m_Adj[ rv[0] ].push_back( rv[i] );
                m_Adj[ rv[i] ].push_back( rv[0] );
                parent[ TopoFind( parent, rv[i] ) ] = TopoFind( parent, rv[0] );
            }
        }
    }
    for ( int r = 0 ; r < nreg ; r++ )
    {
        std::sort( m_Adj[r].begin(), m_Adj[r].end() );
        m_Adj[r].erase( std::unique( m_Adj[r].begin(), m_Adj[r].end() ), m_Adj[r].end() );
    }

    for ( int r = 0 ; r < nreg ; r++ )
    {
        m_Groups[ TopoFind( parent, r ) ].push_back( r );
    }
}

// The inside flags of region r from rays, its own facts over them.
vector< char > TopoClassifier::RayFlags( int r )
{
    int own = m_Surfs[ m_RegSurf[r] ]->GetCompID();
    vector< char > v( m_NCross, 0 );
    for ( int i = 0 ; i < ( int )m_Comps.size() ; i++ )
    {
        if ( m_Comps[i] != own && m_Thick[ m_Comps[i] ] )
        {
            v[ m_Comps[i] ] = InsideComp( m_Comps[i], m_Inner[r] );
        }
    }
    for ( std::map< int, char >::iterator ft = m_Fact[r].begin() ; ft != m_Fact[r].end() ; ++ft )
    {
        v[ ft->first ] = ft->second;
    }
    return v;
}

// The flags of every region of a set, walked from region seed with flags from its rays.  Where
// a region is reached again with other flags, the pair and the component go in clash.
void TopoClassifier::Walk( int seed, vector< vector< char > > &out, vector< TopoClash > &clash )
{
    out[seed] = RayFlags( seed );
    vector< int > queue( 1, seed );
    for ( int qi = 0 ; qi < ( int )queue.size() ; qi++ )
    {
        int r = queue[qi];
        for ( int i = 0 ; i < ( int )m_Adj[r].size() ; i++ )
        {
            int q = m_Adj[r][i];
            vector< char > cand = out[r];
            cand[ m_Surfs[ m_RegSurf[q] ]->GetCompID() ] = 0;
            for ( std::map< int, char >::iterator ft = m_Fact[q].begin() ; ft != m_Fact[q].end() ; ++ft )
            {
                cand[ ft->first ] = ft->second;
            }
            if ( out[q].empty() )
            {
                out[q] = cand;
                queue.push_back( q );
            }
            else if ( out[q] != cand )
            {
                for ( int c = 0 ; c < m_NCross ; c++ )
                {
                    if ( out[q][c] != cand[c] )
                    {
                        clash.push_back( std::make_tuple( r, q, c ) );
                    }
                }
            }
        }
    }
}

// A set is walked from an odd number of its largest regions, each seeded by its own rays, so a
// seed whose rays are wrong is outvoted rather than spreading over the set; a region takes the
// majority of what the walks bring it.  Two walks that bring a region different flags for a
// component say a crossing of it lies between regions with no chain to show it, as where a
// body's surface runs along a border.  Each region there then takes that component's flag by
// ray, and the set is walked again.  A set still in conflict is left to the ray cast.
void TopoClassifier::WalkGroup( const vector< int > &group, vector< TopoClash > &settled )
{
    vector< int > members = group;
    std::stable_sort( members.begin(), members.end(), [&]( int a, int b )
    {
        return m_RegFaces[a].size() > m_RegFaces[b].size();
    } );
    int nseeds = std::min( TOPO_MAX_SEEDS, ( int )members.size() );
    if ( nseeds % 2 == 0 )
    {
        nseeds--;
    }

    int nreg = m_RegSurf.size();
    vector< TopoClash > clash;
    vector< vector< vector< char > > > walks;
    for ( int pass = 0 ; pass < TOPO_MAX_PASSES ; pass++ )
    {
        clash.clear();
        walks.assign( nseeds, vector< vector< char > >( nreg ) );
        for ( int k = 0 ; k < nseeds ; k++ )
        {
            Walk( members[k], walks[k], clash );
        }
        if ( clash.empty() )
        {
            break;
        }

        int added = 0;
        for ( int i = 0 ; i < ( int )clash.size() ; i++ )
        {
            int rr[2] = { std::get< 0 >( clash[i] ), std::get< 1 >( clash[i] ) };
            int c = std::get< 2 >( clash[i] );
            for ( int z = 0 ; z < 2 ; z++ )
            {
                if ( m_Fact[ rr[z] ].count( c ) == 0 )
                {
                    m_Fact[ rr[z] ][c] = InsideComp( c, m_Inner[ rr[z] ] );
                    added++;
                }
            }
        }
        settled.insert( settled.end(), clash.begin(), clash.end() );
        if ( added == 0 )
        {
            break;
        }
    }

    for ( int i = 0 ; i < ( int )members.size() ; i++ )
    {
        int r = members[i];
        m_Val[r].assign( m_NCross, 0 );
        for ( int c = 0 ; c < m_NCross ; c++ )
        {
            int yes = 0;
            for ( int k = 0 ; k < nseeds ; k++ )
            {
                if ( !walks[k][r].empty() && walks[k][r][c] )
                {
                    yes++;
                }
            }
            m_Val[r][c] = 2 * yes > nseeds;
        }
    }

    if ( !clash.empty() )
    {
        for ( int i = 0 ; i < ( int )members.size() ; i++ )
        {
            m_BadReg[ members[i] ] = 1;
        }
    }
}

// Where a conflict was settled by giving two regions their own flags for a component, the
// crossing lay along their border -- or one of them holds a piece of the other side that a
// missing chain should have cut off.  Each side of each settled conflict is checked once.
void TopoClassifier::CheckSettled()
{
    std::set< TopoClash > done;
    for ( int i = 0 ; i < ( int )m_Settled.size() ; i++ )
    {
        int ra = std::get< 0 >( m_Settled[i] );
        int rb = std::get< 1 >( m_Settled[i] );
        int c = std::get< 2 >( m_Settled[i] );
        if ( done.insert( std::make_tuple( std::min( ra, rb ), std::max( ra, rb ), c ) ).second )
        {
            CheckSettledSide( ra, rb, c );
            CheckSettledSide( rb, ra, c );
        }
    }
}

// The faces of region x along the segments it shares with region other tell which: the flag of
// component c there is taken by ray, and a region the rays mostly disagree with is left to the
// ray cast.
void TopoClassifier::CheckSettledSide( int x, int other, int c )
{
    if ( m_BadReg[x] )
    {
        return;
    }
    vector< Face* > near;
    for ( int i = 0 ; i < ( int )m_RegSegs[x].size() && ( int )near.size() < TOPO_MAX_BORDER ; i++ )
    {
        const vector< pair< int, Face* > > &sf = *m_RegSegs[x][i];
        bool shared = false;
        for ( int k = 0 ; k < ( int )sf.size() ; k++ )
        {
            if ( sf[k].first == other )
            {
                shared = true;
            }
        }
        for ( int k = 0 ; shared && k < ( int )sf.size() && ( int )near.size() < TOPO_MAX_BORDER ; k++ )
        {
            if ( sf[k].first == x )
            {
                near.push_back( sf[k].second );
            }
        }
    }
    int against = 0;
    for ( int k = 0 ; k < ( int )near.size() ; k++ )
    {
        if ( InsideComp( c, near[k]->ComputeCenterPnt( m_Surfs[ m_RegSurf[x] ] ) ) != ( m_Val[x][c] != 0 ) )
        {
            against++;
        }
    }
    if ( 2 * against > ( int )near.size() )
    {
        m_BadReg[x] = 1;
    }
}

// Each region is checked at three of its faces as the ray cast would check them.  A region the
// rays put on the other side at two of them holds a crossing no chain shows, and is left to the
// ray cast.
void TopoClassifier::Audit()
{
    // Finding a face's center evaluates its surface, which one thread may do at a time, so the
    // regions of one surface are checked in turn.
    int nreg = m_RegSurf.size();
    vector< Surf* > regsurf( nreg );
    for ( int r = 0 ; r < nreg ; r++ )
    {
        regsurf[r] = m_Surfs[ m_RegSurf[r] ];
    }
    ProgressStage bar( m_Mgr, "Topology Audit", nreg );
    m_Mgr->RunBySurface( regsurf, [&]( int r )
    {
        if ( m_BadReg[r] )
        {
            return;
        }
        vector< vector< double > > t_vec_vec( m_NCross );
        vector< bool > fin( m_NCross );
        Surf* srf = m_Surfs[ m_RegSurf[r] ];
        for ( int c = 0 ; c < m_NCross ; c++ )
        {
            fin[c] = m_Val[r][c] != 0;
        }
        bool verdict = m_Mgr->SetDeleteTriFlag( srf->GetSurfaceCfdType(), srf->GetSymPlaneFlag(), fin );
        const vector< Face* > &rf = m_RegFaces[r];
        int n = rf.size();
        int pick[3] = { n / 6, n / 2, ( 5 * n ) / 6 };
        int disagree = 0;
        for ( int i = 0 ; i < 3 ; i++ )
        {
            if ( m_Mgr->PntTrimmedAway( rf[ pick[i] ]->ComputeCenterPnt( srf ), srf, m_XDist, t_vec_vec ) != verdict )
            {
                disagree++;
            }
        }
        if ( disagree >= 2 )
        {
            m_BadReg[r] = 1;
        }
    }, &bar );
}

// The triangles of each surface fall into regions no chain crosses.  Crossing an intersection
// chain with a surface of component b changes only whether a point is inside b, and which side
// of the chain is inside b is read from that surface's outward normal there.  Crossing a border
// chain changes nothing.  So once one region of a connected set has its inside flags from rays,
// every other region of the set has its flags by walking the chains.
//
// Each set is walked from several seed regions, and a region takes the majority of what the
// walks bring it.  Where the side tests along a chain disagree, or the surfaces lie flat
// against each other, a region's flag for that component is taken by ray; where two walks bring
// a region different flags, a crossing lies between regions with no chain to show it, and those
// regions take that component's flag by ray before the set is walked again.
//
// The chains do not always cut the surfaces cleanly -- a chain can be missing where two bodies
// meet -- so every region is checked by ray at a few of its faces, as is each region along a
// settled conflict.  A surface with a region that fails a check, or that lies on both sides of
// one chain, is not classified here and is left to the ray cast.
void CfdMeshMgrSingleton::TopoInteriorTris( int ncross, vector< vector< char > > &del, vector< char > &handled )
{
    TopoClassifier topo( this, m_SurfVec, ncross );
    topo.Classify( del, handled );
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

    int ncross = m_NumComps + 6;

    if ( GetSettingsPtr()->m_SymSplittingOnFlag )
    {
        ncross = m_NumComps + 10;   // room for the outer domain and the symmetry plane
    }

    // The surfaces the topology of the chains classifies need no rays.
    vector< vector< char > > topo_del;
    vector< char > topo_done( m_SurfVec.size(), 0 );
    if ( GetCfdSettingsPtr()->m_TopoInteriorFlag )
    {
        TopoInteriorTris( ncross, topo_del, topo_done );
    }
    vector< int > raysurf;
    for ( s = 0 ; s < ( int )m_SurfVec.size() ; ++s )
    {
        if ( !topo_done[s] )
        {
            raysurf.push_back( s );
        }
        else
        {
            m_NumTopoSurfs++;
        }
    }

    // Each surface's inside flags and neighbour votes, held only while they are worked out
    vector< vector< bool > > inside( m_SurfVec.size() );
    vector< vector< int > > count( m_SurfVec.size() );

    // The verdicts for faces lying on a surface in their own plane, where a ray would start on
    // that surface.  Found one surface at a time, since finding them projects onto surfaces.
    vector< vector< vector< pair< int, bool > > > > coplanar( m_SurfVec.size() );
    for ( s = 0 ; s < ( int )m_SurfVec.size() ; ++s )
    {
        if ( m_SurfVec[s]->GetCoplanarSurfs().empty() || topo_done[s] )
        {
            continue;
        }

        const list <Face*> &faceList = m_SurfVec[ s ]->GetMesh()->GetFaceList();
        coplanar[s].resize( faceList.size() );

        int k = 0;
        for ( list< Face* >::const_iterator f = faceList.begin() ; f != faceList.end(); ++f, ++k )
        {
            CoplanarVerdicts( m_SurfVec[s], ( *f )->ComputeCenterPnt( m_SurfVec[s] ), coplanar[s][k] );
        }
    }

    //==== Count Number of Component Crossings for Each Component =====//
    RunStage( "Inside/outside", ( int )raysurf.size(), [&]( int i )
    {
        int is = raysurf[i];
        RemoveInteriorTrisOneSurf( is, x_dist, ncross, coplanar[is], inside[is], count[is] );
    } );

    list< Face* >::const_iterator f;

    //==== Check Vote and Mark Interior Tris =====//
    vector< pair< int, int > > compsurfs;
    CompSurfCounts( ncross, compsurfs );

    for ( s = 0 ; s < ( int )m_SurfVec.size() ; ++s )
    {
        if ( topo_done[s] )
        {
            continue;
        }
        const list <Face*> &faceList = m_SurfVec[ s ]->GetMesh()->GetFaceList();
        for ( f = faceList.begin() ; f != faceList.end(); ++f )
        {
            int k = ( *f )->m_Index;
            for ( int i = 0 ; i < ( int )compsurfs.size() ; ++i )
            {
                int c = compsurfs[i].first;

                if ( count[s][ k * ncross + c ] > 0 )
                {
                    inside[s][ k * ncross + c ] = true;
                }
                else if ( count[s][ k * ncross + c ] < 0 )
                {
                    inside[s][ k * ncross + c ] = false;
                }
                else // Can't determine if Tri is inside or outside based on neighbor votes
                {
                    // Reported once for each surface of the component.
                    for ( int j = 0 ; j < compsurfs[i].second ; j++ )
                    {
                        printf( "IntExtCount ZERO!\n" );
                    }
                }
            }
        }
    }

    vector< bool > faceinside( ncross );
    for ( int a = 0 ; a < ( int )m_SurfVec.size() ; a++ )
    {
        const list< Face * > &faceList = m_SurfVec[ a ]->GetMesh()->GetFaceList();
        for ( f = faceList.begin(); f != faceList.end(); ++f )
        {
            // Determine if the triangle should be deleted
            if ( m_SurfVec[a]->GetIgnoreSurfFlag() )
            {
                ( *f )->deleteFlag = true;
            }
            else if ( topo_done[a] )
            {
                ( *f )->deleteFlag = topo_del[a][ ( *f )->m_Index ] != 0;
            }
            else
            {
                int k = ( *f )->m_Index;
                for ( int c = 0 ; c < ncross ; c++ )
                {
                    faceinside[c] = inside[a][ k * ncross + c ];
                }
                ( *f )->deleteFlag = SetDeleteTriFlag( m_SurfVec[a]->GetSurfaceCfdType(), m_SurfVec[a]->GetSymPlaneFlag(), faceinside );
            }
        }

        vector< bool >().swap( inside[a] );
        vector< int >().swap( count[a] );
        m_SurfVec[a]->GetMesh()->ClearBorderChainMap();
    }

    //==== Check For Half Mesh ====//
    if ( GetSettingsPtr()->m_HalfMeshFlag )
    {
        // Finding a face's centre projects onto that face's own surface and nothing else, and
        // each surface writes only its own faces' flags, so the surfaces run side by side.
        int nsurf = ( int )m_SurfVec.size();
        RunStage( "Half Mesh", nsurf, [&]( int hs )
        {
            if ( ! m_SurfVec[hs]->GetSymPlaneFlag() )
            {
                const list <Face*> &faceList = m_SurfVec[ hs ]->GetMesh()->GetFaceList();
                for ( list< Face* >::const_iterator hf = faceList.begin() ; hf != faceList.end(); ++hf )
                {
                    vec3d cp = ( *hf )->ComputeCenterPnt( m_SurfVec[hs] );
                    if ( cp[1] < -1.0e-10 )
                    {
                        ( *hf )->deleteFlag = true;
                    }
                }
            }
        } );

        for ( s = 0 ; s < ( int )m_SurfVec.size() ; s++ )
        {
            if( !GetSettingsPtr()->m_FarMeshFlag ) // Don't keep symmetry plane.
            {
                if ( m_SurfVec[s]->GetSymPlaneFlag() )
                {
                    const list <Face*> &faceList = m_SurfVec[ s ]->GetMesh()->GetFaceList();
                    for ( f = faceList.begin() ; f != faceList.end(); ++f )
                    {
                        ( *f )->deleteFlag = true;
                    }
                }
            }
        }
    }

    //==== Remove Tris, Edges and Nodes ====//
    // Each surface's mesh is its own until the border nodes are connected, so the surfaces run
    // side by side.
    int nrem = ( int )m_SurfVec.size();
    RunStage( "Remove Interior", nrem, [&]( int rs )
    {
        m_SurfVec[ rs ]->GetMesh()->RemoveInteriorFacesEdgesNodes();
    } );
}

void CfdMeshMgrSingleton::ConnectBorderNodes( bool wakeOnly )
{
    // Each surface's border nodes are gathered on their own, side by side, and joined in
    // surface order, so the list is the one a walk of every surface in turn makes.
    int nsurf = ( int )m_SurfVec.size();

    // Without a wake there are no wake nodes to connect.
    if ( wakeOnly )
    {
        bool anywake = false;
        for ( int s = 0 ; s < nsurf ; s++ )
        {
            if ( m_SurfVec[s]->GetWakeFlag() )
            {
                anywake = true;
            }
        }
        if ( !anywake )
        {
            return;
        }
    }

    vector< vector< Node* > > surfnodes( nsurf );

    string label = "Border Nodes";
    if ( wakeOnly )
    {
        label = "Wake Nodes";
    }

    RunStage( label, nsurf, [&]( int s )
    {
        if ( m_SurfVec[s]->GetWakeFlag() == wakeOnly )
        {
            const list <Face*> &faceList = m_SurfVec[ s ]->GetMesh()->GetFaceList();
            for ( list< Face* >::const_iterator f = faceList.begin() ; f != faceList.end(); ++f )
            {
                ( *f )->AddBorderNodes( surfnodes[s] );
            }
        }
    } );

    size_t ntotal = 0;
    for ( int s = 0 ; s < nsurf ; s++ )
    {
        ntotal += surfnodes[s].size();
    }

    vector< Node* > nodeVec;
    nodeVec.reserve( ntotal );
    for ( int s = 0 ; s < nsurf ; s++ )
    {
        nodeVec.insert( nodeVec.end(), surfnodes[s].begin(), surfnodes[s].end() );
        vector< Node* >().swap( surfnodes[s] );
    }

    // Each node once, in the order the surfaces and their faces were walked.  MatchBorderNodes
    // moves every node of a group to the first one's place, so which is first must not depend
    // on where the nodes happen to lie in memory.
    std::unordered_set< Node* > seen;
    vector< Node* > uniqueVec;
    uniqueVec.reserve( nodeVec.size() );
    for ( int i = 0 ; i < ( int )nodeVec.size() ; i++ )
    {
        if ( seen.insert( nodeVec[i] ).second )
        {
            uniqueVec.push_back( nodeVec[i] );
        }
    }

    MatchBorderNodes( uniqueVec );
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

    double tol = PT_MERGE_TOL;
    //==== Use NanoFlann to Find Close Points and Group ====//
    IndexPntNodes( pnCloud, tol, true );

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

// Color every face by a continuous quality measure, one color per corner.
//
// Bins suit tags and reasons, which are labels; a quality is a number, and bucketing one
// hides everything inside a bucket and invents features at the boundaries.
//
// DrawObj::m_FaceColorVec is one color per entry in m_PntVec, which for triangles is one per
// corner.  Being per corner rather than per node of the mesh, two triangles sharing a node may
// give it different values, which is what lets a discontinuous quantity draw honestly.
//
//   angle    the angle at that corner over 60 degrees, so a sliver goes red at the corner that
//            is actually bad rather than all over.
//   length   the mean of the two edges meeting at that corner over the target length there --
//            edge data drawn as corner data.  Taken as how far off it is either way, so 0.8
//            and 1.25 match and sqrt(2) lands halfway up the ramp.
void CfdMeshMgrSingleton::UpdateQualityDrawObjs()
{
    m_QualityDO.clear();
    m_QualityDO.resize( 2 );

    char str[256];
    snprintf( str, sizeof( str ), "%s_TQUAL", GetID().c_str() );
    m_QualityDO[0].m_GeomID = string( str );
    snprintf( str, sizeof( str ), "%s_QQUAL", GetID().c_str() );
    m_QualityDO[1].m_GeomID = string( str );

    m_QualityDO[0].m_GeomChanged = true;
    m_QualityDO[1].m_GeomChanged = true;

    // The material still has to be set even though the color comes per vertex.  Color material
    // only routes the array into ambient and diffuse; specular and emission stay the
    // material's, and DrawObj's default emission is opaque white, which on its own drives any
    // lit surface to white however right the colors underneath are.  Set to no highlight and
    // no glow, which is what a color map wants anyway.
    for ( int i = 0; i < 2; i++ )
    {
        for ( int k = 0; k < 4; k++ )
        {
            m_QualityDO[i].m_MaterialInfo.Ambient[k] = 1.0f;
            m_QualityDO[i].m_MaterialInfo.Diffuse[k] = 1.0f;
            m_QualityDO[i].m_MaterialInfo.Specular[k] = 0.0f;
            m_QualityDO[i].m_MaterialInfo.Emission[k] = 0.0f;
        }

        m_QualityDO[i].m_MaterialInfo.Shininess = 1.0f;
    }

    m_QualityDOMetric = GetCfdSettingsPtr()->m_ColorTagReason;

    if ( m_QualityDOMetric != vsp::QUALITY_ANGLE && m_QualityDOMetric != vsp::QUALITY_LENGTH )
    {
        return;
    }

    bool bylen = ( m_QualityDOMetric == vsp::QUALITY_LENGTH );

    // What the ramp means, so the picture can be read.  Both ends are fixed rather than taken
    // from the data, so two meshes colored the same way can be held against each other.
    for ( int i = 0; i < 2; i++ )
    {
        m_QualityDO[i].m_VertexColorFlag = true;
        m_QualityDO[i].m_ColorScaleFlag = true;

        if ( bylen )
        {
            m_QualityDO[i].m_ColorScaleTitle = "Edge length";
            m_QualityDO[i].m_ColorScaleLoLabel = "2x off";
            m_QualityDO[i].m_ColorScaleMidLabel = "1.41x off";
            m_QualityDO[i].m_ColorScaleHiLabel = "on target";
        }
        else
        {
            m_QualityDO[i].m_ColorScaleTitle = "Worst angle";
            m_QualityDO[i].m_ColorScaleLoLabel = "0 deg";
            m_QualityDO[i].m_ColorScaleMidLabel = "30 deg";
            m_QualityDO[i].m_ColorScaleHiLabel = "60 deg";
        }
    }

    for ( int i = 0 ; i < ( int )m_SurfVec.size() ; i++ )
    {
        const vector< vec3d >& pVec = m_SurfVec[i]->GetMesh()->GetSimpPntVec();
        const vector< SimpFace >& fVec = m_SurfVec[ i ]->GetMesh()->GetSimpFaceVec();

        for ( int f = 0 ; f < ( int ) fVec.size() ; f++ )
        {
            const SimpFace* sface = &fVec[f];

            int n = 3;
            int ind[4] = { sface->ind0, sface->ind1, sface->ind2, sface->ind3 };

            if ( sface->m_isQuad )
            {
                n = 4;
            }

            int ido = 0;

            if ( sface->m_isQuad )
            {
                ido = 1;
            }

            vec3d norm = cross( pVec[ind[1]] - pVec[ind[0]], pVec[ind[2]] - pVec[ind[0]] );
            norm.normalize();

            // One value for the whole face, taken from its worst corner.  Coloring each corner
            // on its own hides the defect: a triangle with two sound corners and one bad one
            // reads as mostly sound, and the bad corner is the thing worth seeing.  Wing tips
            // and trailing edges are where that matters.
            double tface = 1.0;

            for ( int k = 0 ; k < n ; k++ )
            {
                const vec3d &a = pVec[ ind[ ( k + n - 1 ) % n ] ];
                const vec3d &b = pVec[ ind[k] ];
                const vec3d &c = pVec[ ind[ ( k + 1 ) % n ] ];

                double lab = dist( a, b );
                double lbc = dist( b, c );

                double t = 1.0;

                if ( bylen )
                {
                    // Handled once for the whole face below, from the edges' own targets.
                    t = 1.0;
                }
                else
                {
                    if ( lab > 0.0 && lbc > 0.0 )
                    {
                        double cosb = dot( a - b, c - b ) / ( lab * lbc );

                        if ( cosb > 1.0 )
                        {
                            cosb = 1.0;
                        }
                        if ( cosb < -1.0 )
                        {
                            cosb = -1.0;
                        }

                        t = ( acos( cosb ) * 180.0 / M_PI ) / 60.0;
                    }
                }

                if ( t < tface )
                {
                    tface = t;
                }
            }

            if ( bylen )
            {
                // Each edge against the target it was built to, worst edge wins.  The target at the face's
                // centre is a different number wherever the target length varies quickly -- which is exactly
                // where the mesh struggles -- and does not agree with the edge length report.
                tface = 1.0;

                if ( sface->m_WorstLenRatio > 0.0 )
                {
                    // One octave off target is the bottom of the ramp; sqrt(2), where Split
                    // and Collapse act, is the middle.
                    tface = 1.0 - log2( max( sface->m_WorstLenRatio, 1.0 ) );
                }
            }

            vec3d rgb = DrawObj::qualityColorRamp( tface );

            for ( int k = 0 ; k < n ; k++ )
            {
                m_QualityDO[ ido ].m_PntVec.push_back( pVec[ ind[k] ] );
                m_QualityDO[ ido ].m_NormVec.push_back( norm );
                m_QualityDO[ ido ].m_FaceColorVec.push_back( rgb );
                m_QualityDO[ ido ].m_FaceAlphaVec.push_back( 1.0f );
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

    UpdateQualityDrawObjs();

    for ( int i = 0 ; i < ( int )m_SurfVec.size() ; i++ )
    {
        const vector< vec3d >& pVec = m_SurfVec[i]->GetMesh()->GetSimpPntVec();
        const vector< SimpFace >& fVec = m_SurfVec[ i ]->GetMesh()->GetSimpFaceVec();
        for ( int f = 0 ; f < ( int ) fVec.size() ; f++ )
        {
            const SimpFace* sface = &fVec[f];

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
        const vector< vec3d >& pVec = m_SurfVec[i]->GetMesh()->GetSimpPntVec();
        const vector< SimpFace >& fVec = m_SurfVec[ i ]->GetMesh()->GetSimpFaceVec();
        for ( int f = 0 ; f < ( int ) fVec.size() ; f++ )
        {
            const SimpFace* sface = &fVec[f];
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
        for ( int j = 0; j < dchain->m_ISegVec.size(); j++ )
        {
            m_DegenCornerPointDO.m_PntVec.push_back( dchain->m_ISegVec[j]->m_IPnt[0]->m_Pnt );
            m_DegenCornerPointDO.m_PntVec.push_back( dchain->m_ISegVec[j]->m_IPnt[1]->m_Pnt );
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

    // The two quality objects.  Their color is per vertex, so no material is set on them --
    // see UpdateQualityDrawObjs.
    if ( m_QualityDOMetric != GetCfdSettingsPtr()->m_ColorTagReason &&
         ( GetCfdSettingsPtr()->m_ColorTagReason == vsp::QUALITY_ANGLE ||
           GetCfdSettingsPtr()->m_ColorTagReason == vsp::QUALITY_LENGTH ) )
    {
        UpdateQualityDrawObjs();
    }

    if ( m_QualityDO.size() == 2 )
    {
        bool onqual = GetCfdSettingsPtr()->m_ColorFacesFlag &&
                      ( GetCfdSettingsPtr()->m_ColorTagReason == vsp::QUALITY_ANGLE ||
                        GetCfdSettingsPtr()->m_ColorTagReason == vsp::QUALITY_LENGTH );

        m_QualityDO[0].m_Visible = onqual;
        m_QualityDO[1].m_Visible = onqual;

        m_QualityDO[0].m_LineColor = vec3d( 0.4, 0.4, 0.4 );
        m_QualityDO[1].m_LineColor = vec3d( 0.4, 0.4, 0.4 );

        if ( onqual && !GetCfdSettingsPtr()->m_DrawMeshFlag )
        {
            m_QualityDO[0].m_Type = DrawObj::VSP_SHADED_TRIS;
            m_QualityDO[1].m_Type = DrawObj::VSP_SHADED_QUADS;
        }
        else
        {
            m_QualityDO[0].m_Type = DrawObj::VSP_CFD_HIDDEN_TRIS;
            m_QualityDO[1].m_Type = DrawObj::VSP_CFD_HIDDEN_QUADS;
        }

        draw_obj_vec.push_back( &m_QualityDO[0] );
        draw_obj_vec.push_back( &m_QualityDO[1] );
    }

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
            const vector< SimpFace >& fVec = m_SurfVec[ i ]->GetMesh()->GetSimpFaceVec();
            if ( fVec.size() > 0 )
            {
                const SimpFace *sface = &fVec[ 0 ];

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
            const vector< SimpFace >& fVec = m_SurfVec[ i ]->GetMesh()->GetSimpFaceVec();
            if ( fVec.size() > 0 )
            {
                const SimpFace *sface = &fVec[ 0 ];

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
            const vector< SimpFace >& fVec = m_SurfVec[ i ]->GetMesh()->GetSimpFaceVec();
            if ( fVec.size() > 0 )
            {
                const SimpFace *sface = &fVec[ 0 ];

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

    // Checks to see if inner plane is outside of outer plane
    if (! ( innerBottomLeft.x() < outerBottomLeft.x() || innerBottomRight.x() > outerBottomRight.x()
         || innerTopLeft.z() > outerTopLeft.z() || innerBottomLeft.z() < outerBottomLeft.z()) )
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
                if ( surf->GetWakeFlag() )
                {
                    name = geom_ptr->GetName()
                           + to_string( (long long)comp_num_map[ surf->GetUnmergedCompID() ] )
                           + "_Wake";

                    // A wake is never given a GeomID of its own -- it is built from a curve
                    // rather than from a Geom, and the Geom it trails from is named by
                    // GetRefGeomID.  Left as it was, the ID came out as the surface suffix
                    // alone and the key file's ID field was written empty.  That field is
                    // not free to be empty: the reader splits the row on commas with strtok,
                    // which runs empty fields together and shifts every later field along.
                    //
                    // The wake takes the ID of the Geom it belongs to.  The marker is kept on
                    // the end for anything reading the full string, and falls off the ten
                    // character GeomID that the key file asks for, so the file names the Geom
                    // the wake came from and the numbering groups it there.
                    exportid = surf->GetRefGeomID() + "_Wake";
                }
            }

            SubSurfaceMgr.m_CompNames.push_back(name);
            SubSurfaceMgr.m_CompIDs.push_back(exportid);
            SubSurfaceMgr.m_CompTypes.push_back( surf->GetSurfaceVSPType() );
            SubSurfaceMgr.m_CompPlate.push_back( surf->GetSurfacePlateNum() );
            SubSurfaceMgr.m_CompCopyIndex.push_back( surf->GetSurfaceCopyIndex() );
            SubSurfaceMgr.m_CompWmin.push_back( 0.0 );
            if ( geom_ptr )
            {
                SubSurfaceMgr.m_CompUscale.push_back( geom_ptr->GetUMax( surf->GetMainSurfID() ) );
                SubSurfaceMgr.m_CompWscale.push_back( geom_ptr->GetWMax( surf->GetMainSurfID() ) );
            }
            else
            {
                SubSurfaceMgr.m_CompUscale.push_back( 1 );
                SubSurfaceMgr.m_CompWscale.push_back( 1 );
            }

            // Set to thin.
            bool thick = false;
            if ( surf->GetSurfaceCfdType() == vsp::CFD_NORMAL || surf->GetSurfaceCfdType() == vsp::CFD_NEGATIVE )
            {
                thick = true;
            }
            SubSurfaceMgr.m_CompThick.push_back( thick );
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

void CfdMeshMgrSingleton::Subtag( Surf* surf, std::set< std::vector< int > > &combo )
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

        // Sets the reason the target length at the face's centre is what it is, which the mesh
        // is colored by.
        surf->InterpTargetMap( center.x(), center.y(), face.m_reason );

        // The centre is in this patch's parameters.  Subsurfaces are drawn in the Geom's,
        // so a patch built from two pieces of the surface has to say which piece this face
        // sits on before the question means anything.
        double uo, wo;
        bool onsurf = surf->ToOriginalUW( center.x(), center.y(), uo, wo );

        for ( int s = 0; s < (int)simp_s_surfs.size(); s++ )
        {
            if ( onsurf && simp_s_surfs[s].Subtag( vec3d( uo, wo, 0 ) ) && surf->GetCompID() >= 0 )
            {
                face.m_Tags.push_back( simp_s_surfs[s].m_Tag );
            }
        }
        combo.insert( face.m_Tags );
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
                        ISeg* frontSegC = ( *c )->m_ISegVec.front();
                        frontSegC->m_IPnt[0]->CompPnt();
                        vec3d pC0 = frontSegC->m_IPnt[0]->m_Pnt;

                        ISeg* frontSegD = ( *d )->m_ISegVec.front();
                        frontSegD->m_IPnt[0]->CompPnt();
                        vec3d pD0 = frontSegD->m_IPnt[0]->m_Pnt;

                        ISeg* backSegC = ( *c )->m_ISegVec.back();
                        backSegC->m_IPnt[1]->CompPnt();
                        vec3d pC1 = backSegC->m_IPnt[1]->m_Pnt;

                        ISeg* backSegD = ( *d )->m_ISegVec.back();
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
            chain->m_ISegVec.push_back( seg1 );

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
        GetCfdSettingsPtr()->m_DrawJoinFlag = m_Vehicle->GetCfdSettingsPtr()->m_DrawJoinFlag.Get();
        GetCfdSettingsPtr()->m_DrawCurveFlag = m_Vehicle->GetCfdSettingsPtr()->m_DrawCurveFlag.Get();
        GetCfdSettingsPtr()->m_DrawPntsFlag = m_Vehicle->GetCfdSettingsPtr()->m_DrawPntsFlag.Get();
    }
}

static void RegisterCfdMeshAnalysis()
{
    CfdMeshMgr.RegisterAnalysis();
}

static AnalysisRegistrar g_CfdMeshRegistrar( RegisterCfdMeshAnalysis );

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

