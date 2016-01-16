//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// CfdMeshMgr
//
//////////////////////////////////////////////////////////////////////

#include "CfdMeshMgr.h"
//#include "CfdMeshScreen.h"
//#include "feaStructScreen.h"
#include "Geom.h"
#include "Vehicle.h"
#include "VehicleMgr.h"
#include "SurfPatch.h"
#include "Tri.h"
#include "Util.h"
#include "SubSurfaceMgr.h"
#include "SubSurface.h"
#include "APIDefines.h"
#include "SurfCore.h"

#ifdef DEBUG_CFD_MESH
#include <direct.h>
#endif

//=============================================================//
Wake::Wake( WakeMgr* mgr )
{
    m_WakeMgrPtr = mgr;
    assert( m_WakeMgrPtr );
}

Wake::~Wake()
{


}

/*
void Wake::Draw()
{
    glColor3ub( 255, 255, 0 );
    glBegin( GL_LINE_STRIP );
    for ( int i = 0 ; i < (int)m_LeadingEdge.size() ; i++ )
    {
        glVertex3dv( m_LeadingEdge[i].data() );
    }
    glEnd();

    //for ( int i = 0 ; i < (int)m_LeadingCurves.size() ; i++ )
    //  m_LeadingCurves[i]->m_SCurve_A->Draw();

    //for ( int i = 0 ; i < (int)m_SurfVec.size() ; i++ )
    //  m_SurfVec[i]->Draw();

}
*/

double Wake::DistToClosestLeadingEdgePnt( vec3d & pnt )
{
    double close_dist = 1.0e12;

    for ( int i = 0 ; i < ( int )m_LeadingEdge.size() ; i++ )
    {
        double d = dist( m_LeadingEdge[i], pnt );
        if ( d < close_dist )
        {
            close_dist = d;
        }
    }
    return close_dist;
}

void Wake::MatchBorderCurve( ICurve* curve )
{
    //==== Compute EndPoints of Curve ====//
    vec3d uw;
    Surf* s = curve->m_SCurve_A->GetSurf();
    uw = curve->m_SCurve_A->CompPntUW( 0.0 );
    vec3d p0 = s->CompPnt( uw.x(), uw.y() );
    uw = curve->m_SCurve_A->CompPntUW( 1.0 );
    vec3d p1 = s->CompPnt( uw.x(), uw.y() );

    double dist_p0 = DistToClosestLeadingEdgePnt( p0 );
    double dist_p1 = DistToClosestLeadingEdgePnt( p1 );

    double tol = 1.0e-08;
    if ( fabs( dist_p0 ) < tol && fabs( dist_p1 ) < tol )
    {
        m_LeadingCurves.push_back( curve );
    }
}

void Wake::BuildSurfs(  )
{
    //==== Find Comp ID & Build Surf ====//
    for ( int c = 0 ; c < ( int )m_LeadingCurves.size() ; c++ )
    {
        m_CompID = m_LeadingCurves[c]->m_SCurve_A->GetSurf()->GetCompID();
        int unmerged_comp_id = m_LeadingCurves[c]->m_SCurve_A->GetSurf()->GetUnmergedCompID();
        int wakeParentSurfID = m_LeadingCurves[c]->m_SCurve_A->GetSurf()->GetSurfID();
        string geom_id = m_LeadingCurves[c]->m_SCurve_A->GetSurf()->GetGeomID();

        Bezier_curve le_crv;

        m_LeadingCurves[c]->m_SCurve_A->GetBorderCurve( le_crv );

        if ( le_crv.GetNumSections() > 0 )
        {
            Surf* s = new Surf();
            s->SetWakeFlag( true );
            s->SetSurfaceCfdType(vsp::CFD_TRANSPARENT);
            s->SetCompID( m_CompID );
            s->SetUnmergedCompID( unmerged_comp_id );
            s->SetRefGeomID( geom_id );
            s->SetSurfID( m_SurfVec.size() );
            s->SetWakeParentSurfID( wakeParentSurfID );
            s->GetSurfCore()->MakeWakeSurf( le_crv, m_WakeMgrPtr->GetEndX(), m_WakeMgrPtr->GetAngle() );
            s->GetSurfCore()->BuildPatches( s );

            m_SurfVec.push_back( s );
        }
    }
}

//=============================================================//
//=============================================================//
WakeMgr::WakeMgr()
{
    m_EndX = 1.0;
    m_Angle = 0.0;
}

WakeMgr::~WakeMgr()
{
    ClearWakes();
}

vec3d WakeMgr::ComputeTrailEdgePnt( vec3d p )
{

    double z = p.z() + ( m_EndX - p.x() ) * tan( DEG2RAD( m_Angle ) );

    return vec3d( m_EndX, p[1], z );
}

void WakeMgr::SetLeadingEdges( vector < vector < vec3d > > & wake_leading_edges )
{
    m_LeadingEdgeVec = wake_leading_edges;
}


void WakeMgr::ClearWakes()
{
    for ( int i = 0 ; i < ( int )m_WakeVec.size() ; i++ )
    {
        delete m_WakeVec[i];
    }
    m_WakeVec.clear();

}

void WakeMgr::CreateWakesAppendBorderCurves( vector< ICurve* > & border_curves )
{
    int i, j;
    ClearWakes();

    //==== Create Wakes ====//
    for ( i = 0 ; i < ( int )m_LeadingEdgeVec.size() ; i++ )
    {
        Wake* w = new Wake( this );
        m_WakeVec.push_back( w );
        w->m_LeadingEdge = m_LeadingEdgeVec[i];
    }

    //==== Match Wake To Border Curves From Model ====//
    for ( i = 0 ; i < ( int )m_WakeVec.size() ; i++ )
    {
        for ( j = 0 ; j < ( int )border_curves.size() ; j++ )
        {
            m_WakeVec[i]->MatchBorderCurve( border_curves[j] );

        }
        m_WakeVec[i]->BuildSurfs();
    }

    //==== Find Matching Intersection/Border Curves =====//
    vector< Surf* > wake_surfs = GetWakeSurfs();
    vector< SCurve* > scurve_vec;
    for ( i = 0 ; i < ( int )wake_surfs.size() ; i++ )
    {
        wake_surfs[i]->BuildDistMap();
        wake_surfs[i]->SetGridDensityPtr( CfdMeshMgr.GetGridDensityPtr() );
        wake_surfs[i]->FindBorderCurves();
        wake_surfs[i]->LoadSCurves( scurve_vec );
    }

    //==== Load Leading Edge SCurves ====//
    vector < SCurve* > leading_edge_scurves;
    for ( i = 0 ; i < ( int )m_WakeVec.size() ; i++ )
    {
        for ( int j = 0 ; j < ( int )m_WakeVec[i]->m_LeadingCurves.size() ; j++ )
        {
            leading_edge_scurves.push_back( m_WakeVec[i]->m_LeadingCurves[j]->m_SCurve_A );
        }
    }

    //==== Match Leading Edge SCurves With Wake SCurves ====//
    for ( i = 0 ; i < ( int )scurve_vec.size() ; i++ )
    {
        for ( j = 0 ; j < ( int )leading_edge_scurves.size() ; j++ )
        {
            ICurve* icrv = new ICurve;
            if ( icrv->Match( leading_edge_scurves[j], scurve_vec[i] ) )
            {
                border_curves.push_back( icrv );
            }
            else
            {
                delete icrv;
            }
        }
    }

    //==== Match Border Curves ====//
    for ( i = 0 ; i < ( int )scurve_vec.size() ; i++ )
    {
        for ( j = i + 1 ; j < ( int )scurve_vec.size() ; j++ )
        {
            ICurve* icrv = new ICurve;
            if ( icrv->Match( scurve_vec[i], scurve_vec[j] ) )
            {
                border_curves.push_back( icrv );
            }
            else
            {
                delete icrv;
            }
        }
    }

    //==== Check For SCurves Not Matched ====//
    for ( i = 0 ; i < ( int )scurve_vec.size() ; i++ )
    {
        if ( !scurve_vec[i]->GetICurve() )
        {
            ICurve* icrv = new ICurve;
            icrv->SetACurve( scurve_vec[i] );
            border_curves.push_back( icrv );
        }
    }
}

vector< Surf* > WakeMgr::GetWakeSurfs()
{
    vector< Surf* > svec;
    for ( int i = 0 ; i < ( int )m_WakeVec.size() ; i++ )
    {
        for ( int j = 0 ; j < ( int )m_WakeVec[i]->m_SurfVec.size() ; j++ )
        {
            svec.push_back( m_WakeVec[i]->m_SurfVec[j] );
        }
    }
    return svec;
}

void WakeMgr::AppendWakeSurfs( vector< Surf* > & surf_vec )
{
    int last_id = surf_vec.back()->GetSurfID();
    vector< Surf* > wake_surf_vec = GetWakeSurfs();

    for ( int i = 0 ; i < ( int )wake_surf_vec.size() ; i++ )
    {
        wake_surf_vec[i]->SetSurfID( last_id + 1 + i );
        surf_vec.push_back( wake_surf_vec[i] );
    }

}

void WakeMgr::StretchWakes()
{
    double scale = CfdMeshMgr.GetCfdSettingsPtr()->GetWakeScale();
    for ( int i = 0 ; i < ( int )m_WakeVec.size() ; i++ )
    {
        for ( int j = 0 ; j < ( int )m_WakeVec[i]->m_SurfVec.size() ; j++ )
        {
            Mesh* msh = m_WakeVec[i]->m_SurfVec[j]->GetMesh();
            msh->StretchSimpPnts( m_StartStretchX, m_EndX, scale, m_Angle );
        }
    }
}

/*
void WakeMgr::Draw()
{
    double scale = CfdMeshMgr.GetWakeScale();
    double factor = scale - 1.0;

    glColor4ub( 255, 204, 51, 255 );        // Yellowish
    for ( int e = 0 ; e < (int)m_LeadingEdgeVec.size() ; e++ )
    {
        glBegin( GL_LINES );
        for ( int i = 0 ; i < (int)m_LeadingEdgeVec[e].size() ; i++ )
        {
            vec3d le = m_LeadingEdgeVec[e][i];
            glVertex3dv( le.data() );

            vec3d te = ComputeTrailEdgePnt( le );
            double numer = te.x()-m_StartStretchX;
            double fract = numer/(m_EndX-m_StartStretchX);
            double xx = m_StartStretchX + numer*(1.0 + factor*fract*fract);
            double zz = te.z() + (xx - te.x())*tan( DEG2RAD(m_Angle) );
            glVertex3d( xx, te.y(), zz );
        }
        glEnd();
    }
}
*/

void WakeMgr::LoadDrawObjs( vector< DrawObj* > & draw_obj_vec )
{
    double scale = CfdMeshMgr.GetCfdSettingsPtr()->GetWakeScale();
    double factor = scale - 1.0;

    vector< vec3d > wakeData;
    for ( int e = 0; e < ( int )m_LeadingEdgeVec.size(); e++ )
    {
        for ( int i = 0; i < ( int )m_LeadingEdgeVec[e].size(); i++ )
        {
            vec3d le = m_LeadingEdgeVec[e][i];
            wakeData.push_back( le );

            vec3d te = ComputeTrailEdgePnt( le );
            double numer = te.x() - m_StartStretchX;
            double fract = numer / ( m_EndX - m_StartStretchX );
            double xx = m_StartStretchX + numer * ( 1.0 + factor * fract * fract );
            double zz = te.z() + ( xx - te.x() ) * tan( DEG2RAD( m_Angle ) );
            wakeData.push_back( vec3d( xx, te.y(), zz ) );
        }
    }
    m_WakeDO.m_Screen = DrawObj::VSP_MAIN_SCREEN;
    m_WakeDO.m_GeomID = "WAKE";
    m_WakeDO.m_Type = DrawObj::VSP_LINES;
    m_WakeDO.m_LineColor = vec3d( 1, 204.0 / 255, 51.0 / 255 );
    m_WakeDO.m_PntVec = wakeData;

    draw_obj_vec.push_back( &m_WakeDO );
}

void WakeMgr::Show( bool flag )
{
    m_WakeDO.m_Visible = flag;
}

//=============================================================//
//=============================================================//


CfdMeshMgrSingleton::CfdMeshMgrSingleton() : ParmContainer()
{
    m_Vehicle = VehicleMgr.GetVehicle();

    m_HighlightChainIndex = 0;

    m_CurrMainSurfIndx = 0;

    // Array allocated to (m_NumComps + 6) later, so if this isn't reset by then, the
    // allocation will fail with a negative argument.
    m_NumComps = -10;

#ifdef DEBUG_CFD_MESH
    m_DebugDir  = Stringc( "MeshDebug/" );
    _mkdir( m_DebugDir.get_char_star() );
    m_DebugFile = fopen( "MeshDebug/log.txt", "w" );
    m_DebugDraw = false;
#endif

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

//==== Parm Changed ====//
void CfdMeshMgrSingleton::ParmChanged( Parm* parm_ptr, int type )
{
    if ( m_Vehicle )
    {
        m_Vehicle->ParmChanged( parm_ptr, Parm::SET );
    }
}

void CfdMeshMgrSingleton::GenerateMesh()
{
    CfdMeshMgr.addOutputText( "Fetching Bezier Surfaces\n" );

    vector< XferSurf > xfersurfs;
    CfdMeshMgr.FetchSurfs( xfersurfs );

    CfdMeshMgr.CleanUp();
    CfdMeshMgr.addOutputText( "Loading Bezier Surfaces\n" );
    CfdMeshMgr.LoadSurfs( xfersurfs );

    CfdMeshMgr.CleanMergeSurfs();

    if ( m_SurfVec.size() == 0 )
    {
        CfdMeshMgr.addOutputText( "No Surfaces To Mesh\n" );
        return;
    }

    CfdMeshMgr.UpdateSourcesAndWakes();
    CfdMeshMgr.UpdateDomain();
    CfdMeshMgr.BuildDomain();

    CfdMeshMgr.addOutputText( "Build Grid\n" );
    CfdMeshMgr.BuildGrid();

    CfdMeshMgr.addOutputText( "Intersect\n" );
    CfdMeshMgr.Intersect();
    CfdMeshMgr.addOutputText( "Finished Intersect\n" );

    CfdMeshMgr.addOutputText( "Build Target Map\n" );
    CfdMeshMgr.BuildTargetMap( CfdMeshMgrSingleton::CFD_OUTPUT );

    CfdMeshMgr.addOutputText( "InitMesh\n" );
    CfdMeshMgr.InitMesh( );

    CfdMeshMgr.SubTagTris();

    CfdMeshMgr.addOutputText( "Remesh\n" );
    CfdMeshMgr.Remesh( CfdMeshMgrSingleton::CFD_OUTPUT );

    //addOutputText( "Triangle Quality\n");
    //Stringc qual = CfdMeshMgr.GetQualString();
    //addOutputText( qual.get_char_star() );

    SubSurfaceMgr.BuildSingleTagMap();

    CfdMeshMgr.addOutputText( "Exporting Files\n" );
    CfdMeshMgr.ExportFiles();

    CfdMeshMgr.addOutputText( "Check Water Tight\n" );
    string resultTxt = CfdMeshMgr.CheckWaterTight();
    CfdMeshMgr.addOutputText( resultTxt.c_str() );

//  addOutputText( "Mesh Complete\n");

    //==== No Show Components ====//
    vector<string> geomVec = m_Vehicle->GetGeomVec();
//  for ( int i = 0 ; i < (int)geomVec.size() ; i++ )
//      m_Vehicle->FindGeom( geomVec[i] )->setNoShowFlag(1);
//  m_ScreenMgr->Update( GEOM_SCREEN );

    GetCfdSettingsPtr()->m_DrawMeshFlag = true;
}

void CfdMeshMgrSingleton::CleanUp()
{
    int i;
    //==== Delete Old Surfs ====//
    for ( i = 0 ; i < ( int )m_SurfVec.size() ; i++ )
    {
        delete m_SurfVec[i];
    }
    m_SurfVec.clear();

    //==== Delete ICurves =====//
    for ( i = 0 ; i < ( int )m_ICurveVec.size() ; i++ )
    {
        delete m_ICurveVec[i];
    }
    m_ICurveVec.clear();

    //==== Delete Seg Chains ====//
    list< ISegChain* >::iterator cl;
    for ( cl = m_ISegChainList.begin() ; cl != m_ISegChainList.end(); cl++ )
    {
        delete ( *cl );
    }
    m_ISegChainList.clear();

    //==== Delete Puws ====//
    for ( i = 0 ; i < ( int )m_DelPuwVec.size() ; i++ )
    {
        delete m_DelPuwVec[i];
    }
    m_DelPuwVec.clear();

    //==== Delete IPnts ====//
    for ( i = 0 ; i < ( int )m_DelIPntVec.size() ; i++ )
    {
        delete m_DelIPntVec[i];
    }
    m_DelIPntVec.clear();

    //==== Delete IPntGroups ====//
    for ( i = 0 ; i < ( int )m_DelIPntGroupVec.size() ; i++ )
    {
        delete m_DelIPntGroupVec[i];
    }
    m_DelIPntGroupVec.clear();

    //==== Delete IPntGroups ====//
    for ( i = 0 ; i < ( int )m_DelISegChainVec.size() ; i++ )
    {
        delete m_DelISegChainVec[i];
    }
    m_DelISegChainVec.clear();

    //==== Delete Stored Ndoes ====//
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
    for ( i = 0 ; i < ( int )m_BadTris.size() ; i++ )
    {
        delete m_BadTris[i];
    }
    m_BadTris.clear();

    m_BinMap.clear();
    m_PossCoPlanarSurfMap.clear();

    debugPnts.clear();

}



void CfdMeshMgrSingleton::addOutputText( const string &str, int output_type )
{
//    m_OutStream << str;

    MessageData data;
    data.m_String = "CFDMessage";
    data.m_StringVec.push_back( str );
    MessageMgr::getInstance().Send( "ScreenMgr", NULL, data );


//        ScreenMgr* screenMgr = m_Vehicle->getScreenMgr();
//        if ( screenMgr )
//        {
//                if ( output_type == CFD_OUTPUT )
//                        screenMgr->getCfdMeshScreen()->addOutputText( str );
//                else if ( output_type == FEA_OUTPUT )
//                        screenMgr->getFeaStructScreen()->addOutputText( str );
//        }
//        else
//        {
//                printf( "%s", str );
//                fflush( stdout );
//        }
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

void CfdMeshMgrSingleton::GUI_Val( string name, double val )
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

void CfdMeshMgrSingleton::GUI_Val( string name, int val )
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

void CfdMeshMgrSingleton::GUI_Val( string name, string val )
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
    BaseSource* s = NULL;
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
    BaseSource* ret_source = NULL;
    Geom* curr_geom = NULL;
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
        sprintf( str, "PointSource_srf_%d_%d", m_CurrMainSurfIndx, num_sources );
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
        sprintf( str, "LineSource_srf_%d_%d", m_CurrMainSurfIndx, num_sources );
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
        sprintf( str, "BoxSource_srf_%d_%d", m_CurrMainSurfIndx, num_sources );
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

    //==== Highlight/Edit The New Source ====//
    vector< BaseSource* > sVec = curr_geom->GetCfdMeshMainSourceVec();
    curr_geom->SetCurrSourceID( ( int )sVec.size() - 1 );

    return  ret_source;


}

void CfdMeshMgrSingleton::DeleteCurrSource()
{
    Geom* curr_geom = NULL;

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

void CfdMeshMgrSingleton::UpdateSourcesAndWakes()
{
    GetGridDensityPtr()->ClearSources();
    vector< vector< vec3d > > wake_leading_edges;

    vector<string> geomVec = m_Vehicle->GetGeomVec();
    for ( int g = 0 ; g < ( int )geomVec.size() ; g++ )
    {
        Geom* geom = m_Vehicle->FindGeom( geomVec[g] );
        if ( geom )
        {
            if ( geom->GetSetFlag( GetCfdSettingsPtr()->m_SelectedSetIndex() ) )
            {
                geom->UpdateSources();
                vector< BaseSimpleSource* > sVec = geom->GetCfdMeshSimpSourceVec();

                for ( int s = 0 ; s < ( int )sVec.size() ; s++ )
                {
                    GetGridDensityPtr()->AddSource( sVec[s] );
                }
                geom->AppendWakeEdges( wake_leading_edges );
            }
        }
    }

    m_WakeMgr.SetLeadingEdges( wake_leading_edges );
    m_Vehicle->UpdateBBox();
    BndBox box = m_Vehicle->GetBndBox();
    m_WakeMgr.SetStartStretchX( box.GetMax( 0 ) + 0.01 * box.GetLargestDist() );
    m_WakeMgr.SetEndX( box.GetMax( 0 ) + 0.5 * box.GetLargestDist() );
    m_WakeMgr.SetAngle( GetCfdSettingsPtr()->GetWakeAngle() );

}

void CfdMeshMgrSingleton::UpdateDomain()
{
    m_Vehicle->UpdateBBox();
    m_Domain = m_Vehicle->GetBndBox();

    vec3d lwh = vec3d( m_Domain.GetMax( 0 ) - m_Domain.GetMin( 0 ),
                       m_Domain.GetMax( 1 ) - m_Domain.GetMin( 1 ),
                       m_Domain.GetMax( 2 ) - m_Domain.GetMin( 2 ) );

    vec3d xyzc = m_Domain.GetCenter();

    vec3d xyz0 = xyzc;
    xyz0.v[0] = m_Domain.GetMin( 0 );

    if ( GetCfdSettingsPtr()->GetFarMeshFlag() )
    {
        if( !GetCfdSettingsPtr()->GetFarCompFlag() )
        {
            if ( GetCfdSettingsPtr()->GetFarAbsSizeFlag() )
            {
                GetCfdSettingsPtr()->m_FarXScale = GetCfdSettingsPtr()->m_FarLength() / lwh.x();
                GetCfdSettingsPtr()->m_FarYScale = GetCfdSettingsPtr()->m_FarWidth() / lwh.y();
                GetCfdSettingsPtr()->m_FarZScale = GetCfdSettingsPtr()->m_FarHeight() / lwh.z();

                lwh = vec3d( GetCfdSettingsPtr()->m_FarLength(), GetCfdSettingsPtr()->m_FarWidth(), GetCfdSettingsPtr()->m_FarHeight() );
            }
            else
            {
                lwh.scale_x( GetCfdSettingsPtr()->m_FarXScale() );
                lwh.scale_y( GetCfdSettingsPtr()->m_FarYScale() );
                lwh.scale_z( GetCfdSettingsPtr()->m_FarZScale() );

                GetCfdSettingsPtr()->m_FarLength = lwh.x();
                GetCfdSettingsPtr()->m_FarWidth = lwh.y();
                GetCfdSettingsPtr()->m_FarHeight = lwh.z();
            }

            if ( GetCfdSettingsPtr()->GetFarManLocFlag() )
            {
                xyz0 = vec3d( GetCfdSettingsPtr()->m_FarXLocation(), GetCfdSettingsPtr()->m_FarYLocation(), GetCfdSettingsPtr()->m_FarZLocation() );
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

    if ( GetCfdSettingsPtr()->GetHalfMeshFlag() )
    {
        m_Domain.SetMin( 1, 0.0 );
    }
}

void CfdMeshMgrSingleton::AddDefaultSources()
{
    if ( GetGridDensityPtr()->GetNumSources() == 0 )
    {
        vector<string> geomVec = m_Vehicle->GetGeomVec();
        for ( int g = 0 ; g < ( int )geomVec.size() ; g++ )
        {
            double base_len = GetGridDensityPtr()->GetBaseLen();
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
    Geom* curr_geom = NULL;
    curr_geom = m_Vehicle->FindGeom( m_CurrSourceGeomID );
    if ( !curr_geom )
    {
        return;
    }

    double base_len = GetGridDensityPtr()->GetBaseLen();
    curr_geom->AddDefaultSources( base_len );
}

void CfdMeshMgrSingleton::ScaleTriSize( double scale )
{
    GetGridDensityPtr()->m_BaseLen = scale * GetGridDensityPtr()->m_BaseLen();
    GetGridDensityPtr()->m_MinLen = scale * GetGridDensityPtr()->m_MinLen();
    GetGridDensityPtr()->m_MaxGap = scale * GetGridDensityPtr()->m_MaxGap();
    GetGridDensityPtr()->m_NCircSeg = GetGridDensityPtr()->m_NCircSeg() / scale;
    GetGridDensityPtr()->m_FarMaxLen = scale * GetGridDensityPtr()->m_FarMaxLen();
    GetGridDensityPtr()->m_FarMaxGap = scale * GetGridDensityPtr()->m_FarMaxGap();
    GetGridDensityPtr()->m_FarNCircSeg = GetGridDensityPtr()->m_FarNCircSeg() / scale;
    GetGridDensityPtr()->ScaleAllSources( scale );
}

void CfdMeshMgrSingleton::FetchSurfs( vector< XferSurf > &xfersurfs )
{
    m_Vehicle->FetchXFerSurfs( GetCfdSettingsPtr()->m_SelectedSetIndex(), xfersurfs );
}

void CfdMeshMgrSingleton::LoadSurfs( vector< XferSurf > &xfersurfs )
{
    int maxcompid = -1;
    for ( int i = 0; i < xfersurfs.size(); i++ )
    {
        Surf* surfPtr = new Surf();
        surfPtr->GetSurfCore()->SetSurf( xfersurfs[i].m_Surface );

        surfPtr->SetGeomID( xfersurfs[i].m_GeomID );

        //Xfersurfs created each time and flips normal if geom is a negative volume
        if ( xfersurfs[i].m_SurfCfdType == vsp::CFD_NEGATIVE )
        {
            xfersurfs[i].m_FlipNormal = !xfersurfs[i].m_FlipNormal;
        }

        surfPtr->SetFlipFlag( xfersurfs[i].m_FlipNormal );
        //Sets whether WING, DISK, NORMAL
        surfPtr->SetSurfaceType(xfersurfs[i].m_SurfType);
        //Sets whether NORMAL, NEGATIVE, TRANSPARENT
        surfPtr->SetSurfaceCfdType(xfersurfs[i].m_SurfCfdType);

        int cid = xfersurfs[i].m_CompIndx;

        if ( cid > maxcompid )
            maxcompid = cid;

        surfPtr->SetCompID( cid );
        surfPtr->SetUnmergedCompID( cid );
        surfPtr->SetSurfID( i );
        surfPtr->GetSurfCore()->BuildPatches( surfPtr );
        m_SurfVec.push_back( surfPtr );
    }
    m_NumComps = maxcompid + 1;
}

void CfdMeshMgrSingleton::CleanMergeSurfs()
{

    vector < Surf* > surfs = m_SurfVec;
    m_SurfVec.clear();

    for ( int s = 0 ; s < ( int )surfs.size(); s++ )
    {

        Surf* surfPtr = surfs[s];

        bool addSurfFlag = true;
        if ( GetCfdSettingsPtr()->GetHalfMeshFlag() && surfPtr->GetSurfCore()->LessThanY( 1e-6 ) )
        {
            addSurfFlag = false;
        }

        if ( GetCfdSettingsPtr()->GetHalfMeshFlag() && surfPtr->GetSurfCore()->PlaneAtYZero() )
        {
            addSurfFlag = false;
        }

        if ( addSurfFlag )
        {
            m_SurfVec.push_back( surfPtr );
        }
        else
        {
            delete surfPtr;
        }
    }

    DeleteDuplicateSurfs();

    //==== Combine Components With Matching Surface Edges ====//
    map< int, int > mergeCompMap;
    for ( int s = 0 ; s < ( int )m_SurfVec.size() - 1 ; s++ )
    {
        for ( int t = s + 1 ; t < ( int )m_SurfVec.size() ; t++ )
        {
            int compA = m_SurfVec[s]->GetCompID();
            int compB = m_SurfVec[t]->GetCompID();
            if ( compA != compB && m_SurfVec[s]->BorderMatch( m_SurfVec[t] ) )
            {
                mergeCompMap[compB] = compA;
            }
        }
    }
    //==== Change Comp IDs ====//
    for ( int s = 0 ; s < ( int )m_SurfVec.size() ; s++ )
    {
        int compID = m_SurfVec[s]->GetCompID();
        if ( mergeCompMap.find( compID ) != mergeCompMap.end() )
        {
            int newCompID = mergeCompMap[compID];

            // Keep looking through the merge map until we get to the root component
            while ( mergeCompMap.find( newCompID ) != mergeCompMap.end() )
            {
                newCompID = mergeCompMap[newCompID];
            }

            m_SurfVec[s]->SetCompID( newCompID );
        }
    }
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
    if ( GetCfdSettingsPtr()->GetFarMeshFlag() )
    {
        if ( GetCfdSettingsPtr()->GetFarCompFlag() )
        {
            for ( int i = 0 ; i < (int)m_SurfVec.size() ; i++ )
            {
                if ( m_SurfVec[i]->GetGeomID() == GetCfdSettingsPtr()->GetFarGeomID() )
                {
                    m_SurfVec[i]->SetFarFlag( true );
                    m_SurfVec[i]->SetSurfaceCfdType(vsp::CFD_TRANSPARENT);
                    m_SurfVec[i]->FlipFlipFlag();
                }
            }
        }
    }
}

void CfdMeshMgrSingleton::DeleteDuplicateSurfs()
{
    int nsurf = m_SurfVec.size();

    vector < bool > delflag( nsurf );
    for ( int i = 0 ; i < nsurf ; i++ )
    {
        delflag[i] = false;
    }

    for ( int s = 0 ; s < nsurf - 1 ; s++ )
    {
        for ( int t = s + 1 ; t < nsurf ; t++ )
        {
            if ( m_SurfVec[s]->GetSurfCore()->SurfMatch( m_SurfVec[t]->GetSurfCore() ) )
            {
                delflag[s] = true;
                delflag[t] = true;
            }
        }
    }

    vector < Surf* > keepSurf;
    for ( int i = 0 ; i < nsurf ; i++ )
    {
        if ( delflag[i] )
        {
            delete m_SurfVec[i];
        }
        else
        {
            keepSurf.push_back( m_SurfVec[i] );
        }
    }

    m_SurfVec = keepSurf;
}

void CfdMeshMgrSingleton::BuildGrid()
{

    int i, j;
    vector< SCurve* > scurve_vec;
    for ( i = 0 ; i < ( int )m_SurfVec.size() ; i++ )
    {
        m_SurfVec[i]->BuildDistMap();
        m_SurfVec[i]->SetGridDensityPtr( GetGridDensityPtr() );
        m_SurfVec[i]->FindBorderCurves();
        m_SurfVec[i]->LoadSCurves( scurve_vec );
    }

    for ( i = 0 ; i < ( int )scurve_vec.size() ; i++ )
    {
        for ( j = i + 1 ; j < ( int )scurve_vec.size() ; j++ )
        {
            ICurve* icrv = new ICurve;
            if ( icrv->Match( scurve_vec[i], scurve_vec[j] ) )
            {
                m_ICurveVec.push_back( icrv );
            }
            else
            {
                delete icrv;
            }
        }
    }


    //==== Check For SCurves Not Matched ====//
    int num_unmatched = 0;
    for ( i = 0 ; i < ( int )scurve_vec.size() ; i++ )
    {
        if ( !scurve_vec[i]->GetICurve() )
        {
            ICurve* icrv = new ICurve;
            icrv->SetACurve( scurve_vec[i] );
            m_ICurveVec.push_back( icrv );
            num_unmatched++;
        }
    }

    //==== Build Wake Surfaces (If Defined) ====//
    m_WakeMgr.CreateWakesAppendBorderCurves( m_ICurveVec );
    m_WakeMgr.AppendWakeSurfs( m_SurfVec );


#ifdef DEBUG_CFD_MESH
    fprintf( m_DebugFile, "CfdMeshMgr::BuildGrid \n" );
    fprintf( m_DebugFile, "  Num unmatched SCurves = %d \n", num_unmatched );

    for ( int i = 0 ; i < ( int )m_ICurveVec.size() ; i++ )
    {
        m_ICurveVec[i]->DebugEdgeMatching( m_DebugFile );
    }

#endif
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
    for ( c = m_ISegChainList.begin() ; c != m_ISegChainList.end(); c++ )
    {
        vector< ISegSplit* >::iterator s;
        for ( s = ( *c )->m_SplitVec.begin(); s != ( *c )->m_SplitVec.end(); s++ )
        {
            Surf* srf = ( *s )->m_Surf;
            vec2d uw = ( *s )->m_UW;
            // Initialize source with strength from underlying surface map.
            double str = srf->InterpTargetMap( uw.x(), uw.y() );

            vec3d pt = ( *s )->m_Pnt;

            MapSource *ss = new MapSource;
            ss->m_pt = pt;
            ss->m_str = str;

            splitSources.push_back( ss );
        }
    }

    // Number of times to propagate intersection edges through surfaces
    int nedgeprop = 4;

    for ( i = 0; i < nedgeprop; i ++ )
    {
        for ( c = m_ISegChainList.begin() ; c != m_ISegChainList.end(); c++ )
        {
            ( *c )->CalcDensity( GetGridDensityPtr(), splitSources );
            ( *c )->SpreadDensity();
        }
    }

    if( GetGridDensityPtr()->GetRigorLimit() )
    {
        if ( output_type != CfdMeshMgrSingleton::NO_OUTPUT )
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

        for ( c = m_ISegChainList.begin() ; c != m_ISegChainList.end(); c++ )
        {
            ( *c )->CalcDensity( GetGridDensityPtr(), splitSources );
        }
    }

    // Clean up split sources.
    list< MapSource* >::iterator ss;
    for ( ss = splitSources.begin(); ss != splitSources.end(); ss++ )
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

        int num_rev_removed;

        for ( int iter = 0 ; iter < 10 ; ++iter )
        {
            m_SurfVec[i]->GetMesh()->m_Iteration = iter;
            num_tris = 0;
            m_SurfVec[i]->GetMesh()->Remesh();

            num_rev_removed = m_SurfVec[i]->GetMesh()->RemoveRevTris();


            num_tris += m_SurfVec[i]->GetMesh()->GetTriList().size();

            sprintf( str, "Surf %d/%d Iter %d/10 Num Tris = %d\n", i + 1, nsurf, iter + 1, num_tris );
            if ( output_type != CfdMeshMgrSingleton::NO_OUTPUT )
            {
                addOutputText( str, output_type );
            }
        }
        total_num_tris += num_tris;

        if ( num_rev_removed > 0 )
        {
            sprintf( str, "%d Reversed tris collapsed in final iteration.\n", num_rev_removed );
            if ( output_type != CfdMeshMgrSingleton::NO_OUTPUT )
            {
                addOutputText( str, output_type );
            }
        }

        m_SurfVec[i]->GetMesh()->LoadSimpTris();
        m_SurfVec[i]->GetMesh()->Clear();
        m_SurfVec[i]->Subtag( GetCfdSettingsPtr()->GetIntersectSubSurfs() );
        m_SurfVec[i]->GetMesh()->CondenseSimpTris();
    }

    m_WakeMgr.StretchWakes();

    sprintf( str, "Total Num Tris = %d\n", total_num_tris );
    addOutputText( str, output_type );
}

void CfdMeshMgrSingleton::RemeshSingleComp( int comp_id, int output_type )
{
    char str[256];
    int total_num_tris = 0;
    int nsurf = ( int )m_SurfVec.size();
    for ( int i = 0 ; i < nsurf ; i++ )
    {
        int num_tris = 0;
        if ( m_SurfVec[i]->GetCompID() == comp_id )
        {
            for ( int iter = 0 ; iter < 10 ; iter++ )
            {
                num_tris = 0;
                m_SurfVec[i]->GetMesh()->m_Iteration = iter;
                m_SurfVec[i]->GetMesh()->Remesh();

                num_tris += m_SurfVec[i]->GetMesh()->GetTriList().size();

                sprintf( str, "Surf %d/%d Iter %d/10 Num Tris = %d\n", i + 1, nsurf, iter + 1, num_tris );
                addOutputText( str, output_type );
            }
            total_num_tris += num_tris;
        }

        m_SurfVec[i]->GetMesh()->LoadSimpTris();
        m_SurfVec[i]->GetMesh()->Clear();
        m_SurfVec[i]->Subtag( GetCfdSettingsPtr()->GetIntersectSubSurfs() );
        m_SurfVec[i]->GetMesh()->CondenseSimpTris();
    }

    sprintf( str, "Total Num Tris = %d\n", total_num_tris );
    addOutputText( str, output_type );
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

void CfdMeshMgrSingleton::PrintQual()
{
}

void CfdMeshMgrSingleton::ExportFiles()
{
    if ( GetCfdSettingsPtr()->GetExportFileFlag( vsp::CFD_STL_FILE_NAME )->Get() )
    {
        if ( !m_Vehicle->m_STLMultiSolid() )
        {
            WriteSTL( GetCfdSettingsPtr()->GetExportFileName( vsp::CFD_STL_FILE_NAME ) );
        }
        else
        {
            WriteTaggedSTL( GetCfdSettingsPtr()->GetExportFileName( vsp::CFD_STL_FILE_NAME ) );
        }
    }
    if ( GetCfdSettingsPtr()->GetExportFileFlag( vsp::CFD_POLY_FILE_NAME )->Get() )
    {
        WriteTetGen( GetCfdSettingsPtr()->GetExportFileName( vsp::CFD_POLY_FILE_NAME ) );
    }

    string dat_fn;
    if (  GetCfdSettingsPtr()->GetExportFileFlag( vsp::CFD_DAT_FILE_NAME )->Get() )
    {
        dat_fn = GetCfdSettingsPtr()->GetExportFileName( vsp::CFD_DAT_FILE_NAME );
    }
    string key_fn;
    if (  GetCfdSettingsPtr()->GetExportFileFlag( vsp::CFD_KEY_FILE_NAME )->Get() )
    {
        key_fn = GetCfdSettingsPtr()->GetExportFileName( vsp::CFD_KEY_FILE_NAME );
    }
    string obj_fn;
    if (  GetCfdSettingsPtr()->GetExportFileFlag( vsp::CFD_OBJ_FILE_NAME )->Get() )
    {
        obj_fn = GetCfdSettingsPtr()->GetExportFileName( vsp::CFD_OBJ_FILE_NAME );
    }
    string tri_fn;
    if (  GetCfdSettingsPtr()->GetExportFileFlag( vsp::CFD_TRI_FILE_NAME )->Get() )
    {
        tri_fn = GetCfdSettingsPtr()->GetExportFileName( vsp::CFD_TRI_FILE_NAME );
    }
    string gmsh_fn;
    if (  GetCfdSettingsPtr()->GetExportFileFlag( vsp::CFD_GMSH_FILE_NAME )->Get() )
    {
        gmsh_fn = GetCfdSettingsPtr()->GetExportFileName( vsp::CFD_GMSH_FILE_NAME );
    }

    WriteNASCART_Obj_Tri_Gmsh( dat_fn, key_fn, obj_fn, tri_fn, gmsh_fn );

    if ( GetCfdSettingsPtr()->GetExportFileFlag( vsp::CFD_SRF_FILE_NAME )->Get() )
    {
        WriteSurfsIntCurves( GetCfdSettingsPtr()->GetExportFileName( vsp::CFD_SRF_FILE_NAME ) );
    }

    if ( GetCfdSettingsPtr()->GetExportFileFlag( vsp::CFD_TKEY_FILE_NAME )->Get() )
    {
        SubSurfaceMgr.WriteKeyFile( GetCfdSettingsPtr()->GetExportFileName( vsp::CFD_TKEY_FILE_NAME ) );
    }
}

void CfdMeshMgrSingleton::WriteTaggedSTL( const string &filename )
{
    //==== Find All Points and Tri Counts ====//
    vector< vec3d* > allPntVec;
    for ( int i = 0 ; i < ( int )m_SurfVec.size() ; i++ )
    {
        vector< vec3d >& sPntVec = m_SurfVec[i]->GetMesh()->GetSimpPntVec();
        for ( int v = 0 ; v < ( int )sPntVec.size() ; v++ )
        {
            allPntVec.push_back( &sPntVec[v] );
        }
    }

    //==== Build Map ====//
    map< int, vector< int > > indMap;
    vector< int > pntShift;
    BuildIndMap( allPntVec, indMap, pntShift );

    //==== Assemble Normal Tris ====//
    vector< SimpTri > allTriVec;
    for ( int i = 0 ; i < ( int )m_SurfVec.size() ; i++ )
    {
        vector < SimpTri >& sTriVec = m_SurfVec[i]->GetMesh()->GetSimpTriVec();
        vector< vec3d >& sPntVec = m_SurfVec[i]->GetMesh()->GetSimpPntVec();
        for ( int t = 0 ; t <  ( int )sTriVec.size() ; t++ )
        {
            int i0 = FindPntIndex( sPntVec[sTriVec[t].ind0], allPntVec, indMap );
            int i1 = FindPntIndex( sPntVec[sTriVec[t].ind1], allPntVec, indMap );
            int i2 = FindPntIndex( sPntVec[sTriVec[t].ind2], allPntVec, indMap );
            SimpTri stri;
            stri.ind0 = pntShift[i0];
            stri.ind1 = pntShift[i1];
            stri.ind2 = pntShift[i2];
            stri.m_Tags = sTriVec[t].m_Tags;
            allTriVec.push_back( stri );
        }
    }
    //==== Assemble All Used Points ====//
    vector< vec3d* > allUsedPntVec;
    for ( int i = 0 ; i < ( int )allPntVec.size() ; i++ )
    {
        if ( pntShift[i] >= 0 )
        {
            allUsedPntVec.push_back( allPntVec[i] );
        }
    }

    FILE* file_id = fopen( filename.c_str(), "w" );
    if ( file_id )
    {
        std::vector< int > tags = SubSurfaceMgr.GetAllTags();
        for ( int i = 0; i < ( int ) tags.size(); i++ )
        {
            std::string tagname = SubSurfaceMgr.GetTagNames( i );
            fprintf( file_id, "solid %s\n", tagname.c_str() );

            for ( int j = 0; j < ( int ) allTriVec.size(); j++ )
            {
                SimpTri* stri = &allTriVec[j];
                int t = SubSurfaceMgr.GetTag( stri->m_Tags );

                if ( t == tags[i] )
                {
                    vec3d* p0 = allUsedPntVec[stri->ind0];
                    vec3d* p1 = allUsedPntVec[stri->ind1];
                    vec3d* p2 = allUsedPntVec[stri->ind2];
                    vec3d v10 = *p1 - *p0;
                    vec3d v20 = *p2 - *p1;
                    vec3d norm = cross( v10, v20 );
                    norm.normalize();

                    fprintf( file_id, " facet normal  %2.10le %2.10le %2.10le\n",  norm.x(), norm.y(), norm.z() );
                    fprintf( file_id, "   outer loop\n" );

                    fprintf( file_id, "     vertex %2.10le %2.10le %2.10le\n", p0->x(), p0->y(), p0->z() );
                    fprintf( file_id, "     vertex %2.10le %2.10le %2.10le\n", p1->x(), p1->y(), p1->z() );
                    fprintf( file_id, "     vertex %2.10le %2.10le %2.10le\n", p2->x(), p2->y(), p2->z() );

                    fprintf( file_id, "   endloop\n" );
                    fprintf( file_id, " endfacet\n" );
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
                m_SurfVec[i]->GetMesh()->WriteSTL( file_id );
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
                    m_SurfVec[i]->GetMesh()->WriteSTL( file_id );
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

    int tri_cnt = 0;
    vector< vec3d* > allPntVec;
    for ( int i = 0 ; i < ( int )m_SurfVec.size() ; i++ )
    {
        vector< vec3d >& sPntVec = m_SurfVec[i]->GetMesh()->GetSimpPntVec();
        for ( int v = 0 ; v < ( int )sPntVec.size() ; v++ )
        {
            allPntVec.push_back( &sPntVec[v] );
        }
        tri_cnt += m_SurfVec[i]->GetMesh()->GetSimpTriVec().size();
    }

    //==== Build Map ====//
    map< int, vector< int > > indMap;
    vector< int > pntShift;
    int numPnts = BuildIndMap( allPntVec, indMap, pntShift );

    //===== Write Num Pnts and Tris ====//
    fprintf( fp, "# Part 1 - node list\n" );
    fprintf( fp, "%d 3 0 0\n", numPnts );

    //==== Write Model Pnts ====//
    for ( int i = 0 ; i < ( int )allPntVec.size() ; i++ )
    {
        if ( pntShift[i] >= 0 )
        {
            fprintf( fp, "%d %.16g %.16g %.16g\n", i + 1, allPntVec[i]->x(), allPntVec[i]->y(), allPntVec[i]->z() );
        }
    }

    //==== Write Tris ====//
    fprintf( fp, "# Part 2 - facet list\n" );
    fprintf( fp, "%d 0\n", tri_cnt );

    for ( int i = 0 ; i < ( int )m_SurfVec.size() ; i++ )
    {
        vector < SimpTri >& sTriVec = m_SurfVec[i]->GetMesh()->GetSimpTriVec();
        vector< vec3d >& sPntVec = m_SurfVec[i]->GetMesh()->GetSimpPntVec();
        for ( int t = 0 ; t <  ( int )sTriVec.size() ; t++ )
        {
            int i0 = FindPntIndex( sPntVec[sTriVec[t].ind0], allPntVec, indMap );
            int i1 = FindPntIndex( sPntVec[sTriVec[t].ind1], allPntVec, indMap );
            int i2 = FindPntIndex( sPntVec[sTriVec[t].ind2], allPntVec, indMap );
            int ind1 = pntShift[i0] + 1;
            int ind2 = pntShift[i1] + 1;
            int ind3 = pntShift[i2] + 1;

            fprintf( fp, "1\n" );
            fprintf( fp, "3 %d %d %d\n", ind1, ind2, ind3 );
        }
    }

    fprintf( fp, "# Part 3 - Hole List\n" );

    vector<string> geomVec = m_Vehicle->GetGeomVec();
    vector< vec3d > interiorPntVec;
    for ( int i = 0 ; i < ( int )geomVec.size() ; i++ )
    {
        Geom* geom = m_Vehicle->FindGeom( geomVec[i] );
        if ( geom )
        {
            if ( geom->GetSetFlag( GetCfdSettingsPtr()->m_SelectedSetIndex() ) )
            {
                if ( GetCfdSettingsPtr()->GetFarMeshFlag() && GetCfdSettingsPtr()->GetFarCompFlag() )
                {
                    if ( geom->GetID() != GetCfdSettingsPtr()->GetFarGeomID() )
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

    if ( GetCfdSettingsPtr()->GetHalfMeshFlag() )
    {
        vector< vec3d > tmpPntVec;
        for ( int i = 0 ; i < ( int )interiorPntVec.size() ; i++ )
        {
            if ( fabs( interiorPntVec[i].y() ) < 1.0e-4 )
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
        vec3d p = interiorPntVec[i];
        fprintf( fp, "%d  %.16g %.16g %.16g\n", i + 1, p.x(), p.y(), p.z()  );
    }

    fprintf( fp, "# Part 4 - region list\n" );
    fprintf( fp, "0\n" );

    fclose( fp );
}

void CfdMeshMgrSingleton::WriteNASCART_Obj_Tri_Gmsh( const string &dat_fn, const string &key_fn, const string &obj_fn, const string &tri_fn, const string &gmsh_fn )
{
#ifdef DEBUG_CFD_MESH
    //==== Find Smallest Edge ====//
    double small_edge = 1.0e12;
    for ( int i = 0 ; i < ( int )m_SurfVec.size() ; i++ )
    {
        vector < SimpTri >& sTriVec = m_SurfVec[i]->GetMesh()->GetSimpTriVec();
        vector< vec3d >& sPntVec = m_SurfVec[i]->GetMesh()->GetSimpPntVec();
        for ( int t = 0 ; t <  ( int )sTriVec.size() ; t++ )
        {
            double el0 = dist_squared( sPntVec[sTriVec[t].ind0], sPntVec[sTriVec[t].ind1] );
            double el1 = dist_squared( sPntVec[sTriVec[t].ind1], sPntVec[sTriVec[t].ind2] );
            double el2 = dist_squared( sPntVec[sTriVec[t].ind2], sPntVec[sTriVec[t].ind0] );
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

    //==== Find All Points and Tri Counts ====//
    vector< vec3d* > allPntVec;
    vector< vec3d* > wakeAllPntVec;
    for ( int i = 0 ; i < ( int )m_SurfVec.size() ; i++ )
    {
        vector< vec3d >& sPntVec = m_SurfVec[i]->GetMesh()->GetSimpPntVec();
        for ( int v = 0 ; v < ( int )sPntVec.size() ; v++ )
        {
            if ( m_SurfVec[i]->GetWakeFlag() )
            {
                wakeAllPntVec.push_back( &sPntVec[v] );
            }
            else
            {
                allPntVec.push_back( &sPntVec[v] );
            }
        }
    }

    //==== Build Map ====//
    map< int, vector< int > > indMap;
    vector< int > pntShift;
    BuildIndMap( allPntVec, indMap, pntShift );

    //==== Build Wake Map If Available ====//
    map< int, vector< int > > wakeIndMap;
    vector< int > wakePntShift;
    int wakeNumPnts = 0;
    if ( wakeAllPntVec.size() )
    {
        wakeNumPnts = BuildIndMap( wakeAllPntVec, wakeIndMap, wakePntShift );
    }

    //==== Assemble Normal Tris ====//
    vector< SimpTri > allTriVec;
    vector< int > allSurfIDVec;
    for ( int i = 0 ; i < ( int )m_SurfVec.size() ; i++ )
    {
        if ( !m_SurfVec[i]->GetWakeFlag() )
        {
            vector < SimpTri >& sTriVec = m_SurfVec[i]->GetMesh()->GetSimpTriVec();
            vector< vec3d >& sPntVec = m_SurfVec[i]->GetMesh()->GetSimpPntVec();
            for ( int t = 0 ; t <  ( int )sTriVec.size() ; t++ )
            {
                int i0 = FindPntIndex( sPntVec[sTriVec[t].ind0], allPntVec, indMap );
                int i1 = FindPntIndex( sPntVec[sTriVec[t].ind1], allPntVec, indMap );
                int i2 = FindPntIndex( sPntVec[sTriVec[t].ind2], allPntVec, indMap );
                SimpTri stri;
                stri.ind0 = pntShift[i0] + 1;
                stri.ind1 = pntShift[i1] + 1;
                stri.ind2 = pntShift[i2] + 1;
                stri.m_Tags = sTriVec[t].m_Tags;
                allTriVec.push_back( stri );
                allSurfIDVec.push_back( m_SurfVec[i]->GetSurfID() );
            }
        }
    }
    //==== Assemble All Used Points ====//
    vector< vec3d* > allUsedPntVec;
    for ( int i = 0 ; i < ( int )allPntVec.size() ; i++ )
    {
        if ( pntShift[i] >= 0 )
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
            vector < SimpTri >& sTriVec = m_SurfVec[i]->GetMesh()->GetSimpTriVec();
            vector< vec3d >& sPntVec = m_SurfVec[i]->GetMesh()->GetSimpPntVec();
            for ( int t = 0 ; t <  ( int )sTriVec.size() ; t++ )
            {
                int i0 = FindPntIndex( sPntVec[sTriVec[t].ind0], wakeAllPntVec, wakeIndMap );
                int i1 = FindPntIndex( sPntVec[sTriVec[t].ind1], wakeAllPntVec, wakeIndMap );
                int i2 = FindPntIndex( sPntVec[sTriVec[t].ind2], wakeAllPntVec, wakeIndMap );
                SimpTri stri;
                stri.ind0 = wakePntShift[i0] + 1 + wakeIndOffset;
                stri.ind1 = wakePntShift[i1] + 1 + wakeIndOffset;
                stri.ind2 = wakePntShift[i2] + 1 + wakeIndOffset;
                stri.m_Tags = sTriVec[t].m_Tags;
                allTriVec.push_back( stri );
                allSurfIDVec.push_back( m_SurfVec[i]->GetSurfID() );
            }
        }
    }

    //==== Assemble All Used Points ====//
    for ( int i = 0 ; i < ( int )wakeAllPntVec.size() ; i++ )
    {
        if ( wakePntShift[i] >= 0 )
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
            fprintf( fp, "%d %d\n", ( int )allUsedPntVec.size(), ( int )allTriVec.size() );

            //==== Write Pnts ====//
            for ( int i = 0 ; i < ( int )allUsedPntVec.size() ; i++ )
            {
                fprintf( fp, "%.16g %.16g %.16g\n", allUsedPntVec[i]->x(), allUsedPntVec[i]->z(), -allUsedPntVec[i]->y() );
            }

            //==== Write Tris ====//
            for ( int i = 0 ; i < ( int )allTriVec.size() ; i++ )
            {
                fprintf( fp, "%d %d %d %d.0\n",
                         allTriVec[i].ind0, allTriVec[i].ind2, allTriVec[i].ind1,
                         SubSurfaceMgr.GetTag( allTriVec[i].m_Tags ) );
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
                fprintf( fp, "v %16.10f %16.10f %16.10f\n", allUsedPntVec[i]->x(), allUsedPntVec[i]->z(), -allUsedPntVec[i]->y() );
            }
            fprintf( fp, "\n" );

            //==== Write Tris ====//
            for ( int i = 0 ; i < ( int )allTriVec.size() ; i++ )
            {
                fprintf( fp, "f %d %d %d \n", allTriVec[i].ind0, allTriVec[i].ind1, allTriVec[i].ind2 );
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
            fprintf( fp, "%d %d\n", ( int )allUsedPntVec.size(), ( int )allTriVec.size() );

            //==== Write Pnts ====//
            for ( int i = 0 ; i < ( int )allUsedPntVec.size() ; i++ )
            {
                fprintf( fp, "%16.10g %16.10g %16.10g\n", allUsedPntVec[i]->x(), allUsedPntVec[i]->y(), allUsedPntVec[i]->z() );
            }

            //==== Write Tris ====//
            for ( int i = 0 ; i < ( int )allTriVec.size() ; i++ )
            {
                fprintf( fp, "%d %d %d \n", allTriVec[i].ind0, allTriVec[i].ind1, allTriVec[i].ind2 );
            }

            //==== Write Component ID ====//
            for ( int i = 0 ; i < ( int )allTriVec.size() ; i++ )
            {
                fprintf( fp, "%d \n", SubSurfaceMgr.GetTag( allTriVec[i].m_Tags ) );
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
                         allUsedPntVec[i]->x(), allUsedPntVec[i]->y(), allUsedPntVec[i]->z() );
            }
            fprintf( fp, "$EndNodes\n" );

            //==== Write Tris ====//
            fprintf( fp, "$Elements\n" );
            fprintf( fp, "%d\n", ( int )allTriVec.size() );

            int ele_cnt = 1;
            for ( int i = 0 ; i < ( int )allTriVec.size() ; i++ )
            {
                fprintf( fp, "%d 2 0 %d %d %d \n", ele_cnt, allTriVec[i].ind0, allTriVec[i].ind1, allTriVec[i].ind2 );
                ele_cnt++;
            }

            fprintf( fp, "$EndElements\n" );
            fclose( fp );
        }
    }
}


void CfdMeshMgrSingleton::WriteSurfsIntCurves( const string &filename )
{
    FILE* fp = fopen( filename.c_str(), "w" );
    if ( fp )
    {
        map< int, vector< int > > compMap;
        for ( int i = 0 ; i < ( int )m_SurfVec.size() ; i ++ )
        {
            int surfId = m_SurfVec[i]->GetSurfID();
            int compId = m_SurfVec[i]->GetCompID();

            compMap[compId].push_back( surfId );
        }

        fprintf( fp, "BEGIN Component_List\n" );

        fprintf( fp, "%d		// Number of Components \n", ( int )compMap.size() );

        map< int, vector< int > > :: iterator iter;

        for ( iter = compMap.begin() ; iter != compMap.end() ; iter++ )
        {
            int compId = iter->first;
            vector< int > idVec = iter->second;
            fprintf( fp, "BEGIN Component\n" );
            fprintf( fp, "%d		// Comp ID \n",       compId );
            fprintf( fp, "%d		// Num Surfs \n", ( int )idVec.size() );
            for ( int i = 0 ; i < ( int )idVec.size() ; i++ )
            {
                fprintf( fp, "%d		// Surf ID \n",   idVec[i] );
            }
            fprintf( fp, "END Component\n" );
        }
        fprintf( fp, "END Component_List\n" );

        fprintf( fp, "BEGIN Surface_List\n" );
        fprintf( fp, "%d		// Number of Cubic Bezier Surfaces \n", ( int )m_SurfVec.size() );
        for ( int s = 0 ; s < ( int )m_SurfVec.size() ; s++ )
        {
            fprintf( fp, "BEGIN Surface\n" );
            Surf* surfPtr = m_SurfVec[s];
            fprintf( fp, "%d		// Surface ID \n",    surfPtr->GetSurfID() );
            fprintf( fp, "%d		// Comp ID \n",       surfPtr->GetCompID() );

            surfPtr->GetSurfCore()->WriteSurf( fp );

            fprintf( fp, "END Surface\n" );
        }
        fprintf( fp, "END Surface_List\n" );


        vector< ISegChain* > border_curves;
        vector< ISegChain* > intersect_curves;
        list< ISegChain* >::iterator c;
        for ( c = m_ISegChainList.begin() ; c != m_ISegChainList.end(); c++ )
        {
            if ( ( *c )->m_BorderFlag )
            {
                border_curves.push_back( ( *c ) );
            }
            else
            {
                intersect_curves.push_back( ( *c ) );
            }
        }

        fprintf( fp, "BEGIN Border_Curve_List\n" );

        fprintf( fp, "%d		// Number of Border_Curves \n", ( int )border_curves.size() );

        for ( int i = 0 ; i < ( int )border_curves.size() ; i++ )
        {
            fprintf( fp, "BEGIN Border_Curve\n" );

            Surf* surfA =  border_curves[i]->m_SurfA;
            Surf* surfB =  border_curves[i]->m_SurfB;

            fprintf( fp, "%d		// Surface A ID \n", surfA->GetSurfID() );
            fprintf( fp, "%d		// Surface B ID \n", surfB->GetSurfID() );

            vector< IPnt* > ipntVec;

            for ( int j = 0 ; j < ( int )border_curves[i]->m_ISegDeque.size() ; j++ )
            {
                ipntVec.push_back( border_curves[i]->m_ISegDeque[j]->m_IPnt[0] );
            }
            ipntVec.push_back( border_curves[i]->m_ISegDeque.back()->m_IPnt[1] );
            fprintf( fp, "%d		// Number of Border Points (Au, Aw, Bu, Bw) \n", ( int )ipntVec.size() );

            for ( int j = 0 ; j < ( int )ipntVec.size() ; j++ )
            {
                Puw* pwA = ipntVec[j]->GetPuw( surfA );
                Puw* pwB = ipntVec[j]->GetPuw( surfB );
                fprintf( fp, "%d	%16.16lf, %16.16lf, %16.16lf, %16.16lf \n", j,
                         pwA->m_UW.x(), pwA->m_UW.y(), pwB->m_UW.x(), pwB->m_UW.y() );

            }
            fprintf( fp, "END Border_Curve\n" );
        }
        fprintf( fp, "END Border_Curve_List\n" );


        fprintf( fp, "BEGIN IntersectCurve_List\n" );

        fprintf( fp, "%d		// Number of Intersect_Curves \n", ( int )intersect_curves.size() );

        for ( int i = 0 ; i < ( int )intersect_curves.size() ; i++ )
        {
            fprintf( fp, "BEGIN Intersect_Curve\n" );

            Surf* surfA =  intersect_curves[i]->m_SurfA;
            Surf* surfB =  intersect_curves[i]->m_SurfB;

            fprintf( fp, "%d		// Surface A ID \n", surfA->GetSurfID() );
            fprintf( fp, "%d		// Surface B ID \n", surfB->GetSurfID() );

            vector< IPnt* > ipntVec;

            for ( int j = 0 ; j < ( int )intersect_curves[i]->m_ISegDeque.size() ; j++ )
            {
                ipntVec.push_back( intersect_curves[i]->m_ISegDeque[j]->m_IPnt[0] );
            }
            ipntVec.push_back( intersect_curves[i]->m_ISegDeque.back()->m_IPnt[1] );
            fprintf( fp, "%d		// Number of Intersect Points (Au, Aw, Bu, Bw) \n", ( int )ipntVec.size() );

            for ( int j = 0 ; j < ( int )ipntVec.size() ; j++ )
            {
                Puw* pwA = ipntVec[j]->GetPuw( surfA );
                Puw* pwB = ipntVec[j]->GetPuw( surfB );
                fprintf( fp, "%d	%16.16lf, %16.16lf, %16.16lf, %16.16lf \n", j,
                         pwA->m_UW.x(), pwA->m_UW.y(), pwB->m_UW.x(), pwB->m_UW.y() );

            }
            fprintf( fp, "END Intersect_Curve\n" );
        }


        fprintf( fp, "END IntersectCurve_List\n" );


        fclose( fp );
    }

}

string CfdMeshMgrSingleton::CheckWaterTight()
{
    vector< Tri* > triVec;

    int tri_cnt = 0;
    vector< vec3d* > allPntVec;
    for ( int i = 0 ; i < ( int )m_SurfVec.size() ; i++ )
    {
        if( m_SurfVec[i]->GetSurfaceCfdType() != vsp::CFD_TRANSPARENT || m_SurfVec[i]->GetFarFlag() || m_SurfVec[i]->GetSymPlaneFlag() )
        {
            vector< vec3d >& sPntVec = m_SurfVec[i]->GetMesh()->GetSimpPntVec();
            for ( int v = 0 ; v < ( int )sPntVec.size() ; v++ )
            {
                allPntVec.push_back( &sPntVec[v] );
            }
            tri_cnt += m_SurfVec[i]->GetMesh()->GetSimpTriVec().size();
        }
    }

    //==== Build Map ====//
    map< int, vector< int > > indMap;
    vector< int > pntShift;
    BuildIndMap( allPntVec, indMap, pntShift );

    //==== Create Nodes ====//
    for ( int i = 0 ; i < ( int )allPntVec.size() ; i++ )
    {
        if ( pntShift[i] >= 0 )
        {
            Node* n = new Node();
            n->pnt = *allPntVec[i];
            m_nodeStore.push_back( n );
        }
    }

    //==== Create Edges and Tris ====//
    int moreThanTwoTriPerEdge = 0;
    map< int, vector<Edge*> > edgeMap;
    for ( int i = 0 ; i < ( int )m_SurfVec.size() ; i++ )
    {
        if( m_SurfVec[i]->GetSurfaceCfdType() != vsp::CFD_TRANSPARENT || m_SurfVec[i]->GetFarFlag() || m_SurfVec[i]->GetSymPlaneFlag() )
        {
            vector < SimpTri >& sTriVec = m_SurfVec[i]->GetMesh()->GetSimpTriVec();
            vector< vec3d >& sPntVec = m_SurfVec[i]->GetMesh()->GetSimpPntVec();
            for ( int t = 0 ; t <  ( int )sTriVec.size() ; t++ )
            {
                int i0 = FindPntIndex( sPntVec[sTriVec[t].ind0], allPntVec, indMap );
                int i1 = FindPntIndex( sPntVec[sTriVec[t].ind1], allPntVec, indMap );
                int i2 = FindPntIndex( sPntVec[sTriVec[t].ind2], allPntVec, indMap );
                int ind1 = pntShift[i0];
                int ind2 = pntShift[i1];
                int ind3 = pntShift[i2];

                Edge* e0 = FindAddEdge( edgeMap, m_nodeStore, ind1, ind2 );
                Edge* e1 = FindAddEdge( edgeMap, m_nodeStore, ind2, ind3 );
                Edge* e2 = FindAddEdge( edgeMap, m_nodeStore, ind3, ind1 );

                Tri* tri = new Tri( m_nodeStore[ind1], m_nodeStore[ind2], m_nodeStore[ind3], e0, e1, e2 );

                if ( !e0->SetTri( tri ) )
                {
                    tri->debugFlag = true;
                    moreThanTwoTriPerEdge++;
                }
                if ( !e1->SetTri( tri ) )
                {
                    tri->debugFlag = true;
                    moreThanTwoTriPerEdge++;
                }
                if ( !e2->SetTri( tri ) )
                {
                    tri->debugFlag = true;
                    moreThanTwoTriPerEdge++;
                }
                triVec.push_back( tri );

                if ( tri->debugFlag == true )
                {
                    m_BadTris.push_back( tri );
                }
            }
        }
    }

    //==== Find Border Edges ====//
    int num_border_edges = 0;
    map<int, vector<Edge*> >::const_iterator iter;
    for ( iter = edgeMap.begin() ; iter != edgeMap.end() ; iter++ )
    {
        for ( int i = 0 ; i < ( int )iter->second.size() ; i++ )
        {
            Edge* e = iter->second[i];
            if ( !( e->t0 && e->t1 ) )
            {
                m_BadEdges.push_back( e );
                e->debugFlag = true;
                num_border_edges++;
            }

        }
    }

    //==== Clean Up ====//
    for ( iter = edgeMap.begin() ; iter != edgeMap.end() ; iter++ )
    {
        for ( int i = 0 ; i < ( int )iter->second.size() ; i++ )
        {
            if ( iter->second[i]->debugFlag == false )
            {
                delete iter->second[i];
            }
        }
    }
    for ( int i = 0 ; i < ( int )triVec.size() ; i++ )
    {
        if ( triVec[i]->debugFlag == false )
        {
            delete triVec[i];
        }
    }

    char resultStr[255];
    if ( num_border_edges || moreThanTwoTriPerEdge )
    {
        sprintf( resultStr, "NOT Water Tight : %d Border Edges, %d Edges > 2 Tris\n",
                 num_border_edges, moreThanTwoTriPerEdge );
    }
    else
    {
        sprintf( resultStr, "Is Water Tight\n" );
    }

    return string( resultStr );

}

Edge* CfdMeshMgrSingleton::FindAddEdge( map< int, vector<Edge*> > & edgeMap, vector< Node* > & nodeVec, int ind1, int ind2 )
{
    Edge* e = NULL;
    map<int, vector<Edge*> >::const_iterator iter;
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

int CfdMeshMgrSingleton::BuildIndMap( vector< vec3d* > & allPntVec, map< int, vector< int > >& indMap, vector< int > & pntShift )
{
//double max_dist = 0.0;
    double tol = 1.0e-12;
    map<int, vector<int> >::const_iterator iter;
    for ( int i = 0 ; i < ( int )allPntVec.size() ; i++ )
    {
        int combind = ( int )( ( allPntVec[i]->x() + allPntVec[i]->y() + allPntVec[i]->z() ) * 10000.0 );
        iter = indMap.find( combind );
        if ( iter == indMap.end() ) // Add Index
        {
            indMap[combind].push_back( i );
        }
        else
        {
            bool addIndexFlag = true;
            for ( int j = 0 ; j < ( int )iter->second.size() ; j++ )
            {
                int testind = iter->second[j];

                if ( fabs( allPntVec[i]->x() - allPntVec[testind]->x() ) < tol  &&
                        fabs( allPntVec[i]->y() - allPntVec[testind]->y() ) < tol  &&
                        fabs( allPntVec[i]->z() - allPntVec[testind]->z() ) < tol  )
                {
                    addIndexFlag = false;
                }
            }
            if ( addIndexFlag )
            {
                indMap[combind].push_back( i );
            }
        }
    }

    //==== Figure Out Point Shifts ====//
    pntShift.resize( allPntVec.size() );
    for ( int i = 0 ; i < ( int )pntShift.size() ; i++ )
    {
        pntShift[i] = -999;
    }

    for ( iter = indMap.begin() ; iter != indMap.end() ; iter++ )
    {
        for ( int i = 0 ; i < ( int )iter->second.size() ; i++ )
        {
            int ind = iter->second[i];
            pntShift[ind] = 1;
        }
    }

    int cnt = 0;
    for ( int i = 0 ; i < ( int )pntShift.size() ; i++ )
    {
        if ( pntShift[i] > 0 )
        {
            pntShift[i] = cnt;
            cnt++;
        }
    }

    return cnt;

}

int  CfdMeshMgrSingleton::FindPntIndex(  vec3d& pnt, vector< vec3d* > & allPntVec, map< int, vector< int > >& indMap )
{
    double tol = 1.0e-12;
    map<int, vector<int> >::const_iterator iter;

    int combind = ( int )( ( pnt.x() + pnt.y() + pnt.z() ) * 10000.0 );
    iter = indMap.find( combind );
    if ( iter != indMap.end() )
    {
        for ( int j = 0 ; j < ( int )iter->second.size() ; j++ )
        {
            int testind = iter->second[j];

            if ( fabs( pnt.x() - allPntVec[testind]->x() ) < tol  &&
                    fabs( pnt.y() - allPntVec[testind]->y() ) < tol  &&
                    fabs( pnt.z() - allPntVec[testind]->z() ) < tol  )
            {
                return testind;
            }
        }
    }

    printf( "Error: CfdMeshMgr.FindPntIndex can't find index\n" );
    return 0;
}

void CfdMeshMgrSingleton::BuildCurves()
{
    list< ISegChain* >::iterator c;
    for ( c = m_ISegChainList.begin() ; c != m_ISegChainList.end(); c++ )
    {
        ( *c )->BuildCurves();
    }
}
void CfdMeshMgrSingleton::Intersect()
{

    if ( GetCfdSettingsPtr()->GetIntersectSubSurfs() ) BuildSubSurfIntChains();

    //==== Quad Tree Intersection - Intersection Segments Get Loaded at AddIntersectionSeg ===//
    for ( int i = 0 ; i < ( int )m_SurfVec.size() ; i++ )
        for ( int j = i + 1 ; j < ( int )m_SurfVec.size() ; j++ )
        {
            m_SurfVec[i]->Intersect( m_SurfVec[j] );
        }


    BuildChains();

    LoadBorderCurves();

    MergeInteriorChainIPnts();

    SplitBorderCurves();

    IntersectSplitChains();

    BuildCurves();
}

vector< Surf* > CfdMeshMgrSingleton::CreateDomainSurfs()
{
    //Create many planes
    vec3d outerBottomRight, outerBottomLeft, outerTopRight, outerTopLeft;
    vec3d innerBottomRight, innerBottomLeft, innerTopRight, innerTopLeft;
    vector<vec3d> corners;
    BndBox vehicleBBox = m_Vehicle->GetBndBox();
    CfdMeshSettings* settings = GetCfdSettingsPtr();
    double farXScale = settings->m_FarXScale.Get();
    double farZScale = settings->m_FarZScale.Get();
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
        && GetCfdSettingsPtr()->GetFarManLocFlag())
    {
        isInside = false;
    }

    //Begin making the symmetry planes and rest of the box
    int index = 0;
    vector < vec3d > p0;
    if (GetCfdSettingsPtr()->GetHalfMeshFlag() &&
            GetCfdSettingsPtr()->GetSymSplittingOnFlag() && isInside)
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
    if (GetCfdSettingsPtr()->GetHalfMeshFlag() &&
        GetCfdSettingsPtr()->GetSymSplittingOnFlag() && isInside)
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
    if (GetCfdSettingsPtr()->GetHalfMeshFlag() &&
        GetCfdSettingsPtr()->GetSymSplittingOnFlag() && isInside)
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
    if (GetCfdSettingsPtr()->GetHalfMeshFlag() &&
        GetCfdSettingsPtr()->GetSymSplittingOnFlag() && isInside)
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
    if ( GetCfdSettingsPtr()->GetHalfMeshFlag())
    {
        if (GetCfdSettingsPtr()->GetSymSplittingOnFlag() && isInside)
        {
            ndomain = 5;
        }
        else
        {
            ndomain = 1;
        }
    }

    // Box outer domain, half or full mesh.
    if ( GetCfdSettingsPtr()->GetFarMeshFlag() && !GetCfdSettingsPtr()->GetFarCompFlag())
    {
        if (GetCfdSettingsPtr()->GetSymSplittingOnFlag() && isInside)
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
        if( GetCfdSettingsPtr()->GetSymSplittingOnFlag() &&
                i < 5 && GetCfdSettingsPtr()->GetHalfMeshFlag() && isInside)
        {
            domainSurfs[i]->SetSymPlaneFlag( true );
        }
        //If symmetry plane and inside plane is not within outer plane
        else if (GetCfdSettingsPtr()->GetSymSplittingOnFlag() &&
                 i < 1 && GetCfdSettingsPtr()->GetHalfMeshFlag() && !isInside)
        {
            domainSurfs[i]->SetSymPlaneFlag( true );
        }
        //If symmetry splitting is off
        else if (!GetCfdSettingsPtr()->GetSymSplittingOnFlag() &&
                 i < 1 && GetCfdSettingsPtr()->GetHalfMeshFlag())
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
    bool PrintProgress = false;
#ifdef DEBUG_CFD_MESH
    PrintProgress = true;
#endif

    if ( PrintProgress )
    {
        printf( "MatchWakes\n" );
    }
    MatchWakes();

    if ( PrintProgress )
    {
        printf( "TessellateChains\n" );
    }
    TessellateChains();

//DebugWriteChains( "Tess_UW", true );

    if ( PrintProgress )
    {
        printf( "MergeBorderEndPoints\n" );
    }
    MergeBorderEndPoints();

    AddWakeCoPlanarSurfaceChains();

    if ( PrintProgress )
    {
        printf( "BuildMesh\n" );
    }
    BuildMesh();

    if ( PrintProgress )
    {
        printf( "RemoveInteriorTris\n" );
    }
    RemoveInteriorTris();

    if ( PrintProgress )
    {
        printf( "ConnectBorderEdges\n" );
    }
    ConnectBorderEdges( false );        // No Wakes
    ConnectBorderEdges( true );         // Only Wakes
}

void CfdMeshMgrSingleton::AddIntersectionSeg( SurfPatch& pA, SurfPatch& pB, vec3d & ip0, vec3d & ip1 )
{
    double d = dist_squared( ip0, ip1 );
    if ( d < DBL_EPSILON )
    {
        return;
    }

    vec2d proj_uwA0;
    pA.find_closest_uw( ip0, proj_uwA0.v );
    Puw* puwA0 = new Puw( pA.get_surf_ptr(), proj_uwA0 );
    m_DelPuwVec.push_back( puwA0 );

    vec2d proj_uwB0;
    pB.find_closest_uw( ip0, proj_uwB0.v );
    Puw* puwB0 = new Puw( pB.get_surf_ptr(), proj_uwB0 );
    m_DelPuwVec.push_back( puwB0 );

    IPnt* ipnt0 = new IPnt( puwA0, puwB0 );
    ipnt0->m_Pnt = ip0;
    m_DelIPntVec.push_back( ipnt0 );

    vec2d proj_uwA1;
    pA.find_closest_uw( ip1, proj_uwA1.v );
    Puw* puwA1 = new Puw( pA.get_surf_ptr(), proj_uwA1 );
    m_DelPuwVec.push_back( puwA1 );

    vec2d proj_uwB1;
    pB.find_closest_uw( ip1, proj_uwB1.v );
    Puw* puwB1 = new Puw( pB.get_surf_ptr(), proj_uwB1 );
    m_DelPuwVec.push_back( puwB1 );

    IPnt* ipnt1 = new IPnt( puwA1, puwB1 );
    ipnt1->m_Pnt = ip1;
    m_DelIPntVec.push_back( ipnt1 );

    new ISeg( pA.get_surf_ptr(), pB.get_surf_ptr(), ipnt0, ipnt1 );

    int id0 = IPntBin::ComputeID( ipnt0->m_Pnt );
    m_BinMap[id0].m_ID = id0;
    m_BinMap[id0].m_IPnts.push_back( ipnt0 );

    int id1 = IPntBin::ComputeID( ipnt1->m_Pnt );
    m_BinMap[id1].m_ID = id1;
    m_BinMap[id1].m_IPnts.push_back( ipnt1 );

#ifdef DEBUG_CFD_MESH

    static bool onetime = true;
    static int ipntcnt = 0;
    static double max_dist = 0.0;
    if ( onetime )
    {
        fprintf( m_DebugFile, "CfdMeshMgr::AddIntersectionSeg \n" );
        onetime = false;
    }

    double dA0 = dist( ip0, puwA0->m_Surf->CompPnt( puwA0->m_UW.x(), puwA0->m_UW.y() ) );
    double dB0 = dist( ip0, puwB0->m_Surf->CompPnt( puwB0->m_UW.x(), puwB0->m_UW.y() ) );

    double dA1 = dist( ip1, puwA0->m_Surf->CompPnt( puwA1->m_UW.x(), puwA1->m_UW.y() ) );
    double dB1 = dist( ip1, puwB0->m_Surf->CompPnt( puwB1->m_UW.x(), puwB1->m_UW.y() ) );

    double tol = 1.0e-8;
    double total_d = dA0 + dB0 + dA1 + dB1;

    if ( total_d > max_dist )
    {
        max_dist = total_d;
        fprintf( m_DebugFile, "  Proj Pnt Dist = %f    %d \n", max_dist, ipntcnt );
    }
    ipntcnt++;

#endif


//if ( pA.get_surf_ptr() == m_SurfVec[0]  )
//{
//  debugUWs.push_back( proj_uwA0 );
//  debugUWs.push_back( proj_uwA1 );
//}


}


ISeg* CfdMeshMgrSingleton::CreateSurfaceSeg(  Surf* surfA, vec2d & uwA0, vec2d & uwA1, Surf* surfB, vec2d & uwB0, vec2d & uwB1   )
{
    Puw* puwA0 = new Puw( surfA, uwA0 );
    m_DelPuwVec.push_back( puwA0 );

    Puw* puwB0 = new Puw( surfB, uwB0 );
    m_DelPuwVec.push_back( puwB0 );

    IPnt* ipnt0 = new IPnt( puwA0, puwB0 );
    ipnt0->m_Pnt = surfA->CompPnt( uwA0.x(), uwA0.y() );
    m_DelIPntVec.push_back( ipnt0 );

    Puw* puwA1 = new Puw( surfA, uwA1 );
    m_DelPuwVec.push_back( puwA1 );

    Puw* puwB1 = new Puw( surfB, uwB1 );
    m_DelPuwVec.push_back( puwB1 );

    IPnt* ipnt1 = new IPnt( puwA1, puwB1 );
    ipnt1->m_Pnt = surfA->CompPnt( uwA1.x(), uwA1.y() );
    m_DelIPntVec.push_back( ipnt1 );

    ISeg* iseg01 = new ISeg( surfA, surfB, ipnt0, ipnt1 );

    return iseg01;
}

void CfdMeshMgrSingleton::BuildChains()
{
    //==== Load Adjoining Bins =====//
    map< int, IPntBin >::const_iterator iter;
    for ( iter = m_BinMap.begin() ; iter != m_BinMap.end() ; iter++ )
    {
        int id = ( *iter ).second.m_ID;
        map< int, IPntBin >::const_iterator adj;
        for ( int i = -3 ; i < 4 ; i++ )        // Check All Nearby Bins
        {
            if ( i != 0 &&  m_BinMap.find( id + i ) != m_BinMap.end()  )
            {
                m_BinMap[id].m_AdjBins.push_back( id + i );
            }
        }
    }

    //==== Create Chains ====//
    for ( iter = m_BinMap.begin() ; iter != m_BinMap.end() ; iter++ )
    {
        int id = ( *iter ).second.m_ID;
        for ( int i = 0 ; i < ( int )m_BinMap[id].m_IPnts.size() ; i++ )
        {
            if ( !m_BinMap[id].m_IPnts[i]->m_UsedFlag )
            {
                ISeg* seg = m_BinMap[id].m_IPnts[i]->m_Segs[0];
                seg->m_IPnt[0]->m_UsedFlag = true;
                seg->m_IPnt[1]->m_UsedFlag = true;
                ISegChain* chain = new ISegChain;           // Create New Chain
                chain->m_SurfA = seg->m_SurfA;
                chain->m_SurfB = seg->m_SurfB;
                chain->m_ISegDeque.push_back( seg );
                ExpandChain( chain );
                if ( chain->Valid() )
                {
                    m_ISegChainList.push_back( chain );
                }
                else
                {
                    delete chain;
                    chain = NULL;
                }
            }
        }
    }

#ifdef DEBUG_CFD_MESH

    int num_bins = 0;
    int total_num_segs = 0;
    for ( iter = m_BinMap.begin() ; iter != m_BinMap.end() ; iter++ )
    {
        num_bins++;
        int id = ( *iter ).second.m_ID;
        total_num_segs += m_BinMap[id].m_IPnts.size();
    }

    double avg_num_segs = ( double )total_num_segs / ( double )num_bins;

    fprintf( m_DebugFile, "CfdMeshMgr::BuildChains \n" );
    fprintf( m_DebugFile, "   Num Bins = %d \n", num_bins );
    fprintf( m_DebugFile, "   Avg Num Segs per Bin = %f\n", avg_num_segs );

    fprintf( m_DebugFile, "   Num Chains %d \n", m_ISegChainList.size() );
#endif

}

void CfdMeshMgrSingleton::ExpandChain( ISegChain* chain )
{
    bool stillExpanding = true;
    bool expandFront = true;
    while ( stillExpanding )
    {
        IPnt* testIPnt;
        if ( expandFront )
        {
            testIPnt = chain->m_ISegDeque.front()->m_IPnt[0];
        }
        else
        {
            testIPnt = chain->m_ISegDeque.back()->m_IPnt[1];
        }

        int binID = IPntBin::ComputeID( testIPnt->m_Pnt );

        IPnt* matchIPnt = m_BinMap[binID].Match( testIPnt, m_BinMap );

        if ( !matchIPnt && !expandFront )   // No more matches in back of chain
        {
            stillExpanding = false;
        }
        else if ( !matchIPnt )                  // No more matches in front, try back
        {
            expandFront = false;
        }
        else
        {
            ISeg* seg = matchIPnt->m_Segs[0];
            chain->AddSeg( seg, expandFront );
            seg->m_IPnt[0]->m_UsedFlag = true;
            seg->m_IPnt[1]->m_UsedFlag = true;
        }
    }
}

void CfdMeshMgrSingleton::WriteChains()
{
    FILE* fp;
    char str[256];
    sprintf( str, "RawInt_UW0.dat" );
    fp = fopen( str, "w" );

    for ( int i = 1 ; i < ( int )debugUWs.size() ; i += 2 )
    {
        fprintf( fp, "MOVE \n" );
        fprintf( fp, "%f %f\n", debugUWs[i - 1].x(), debugUWs[i - 1].y() );
        fprintf( fp, "%f %f\n", debugUWs[i].x(), debugUWs[i].y() );
    }

    fclose( fp );


    //==== Write Chains ====//
    list< ISegChain* >::iterator c;
    for ( int i = 0 ; i < ( int )m_SurfVec.size() ; i++ )
    {
        sprintf( str, "Intersection_UW%d.dat", i );
        fp = fopen( str, "w" );
        int c_ind = 0;
        for ( c = m_ISegChainList.begin() ; c != m_ISegChainList.end(); c++ )
        {
            if ( ( *c )->m_SurfA == m_SurfVec[i] || ( *c )->m_SurfB == m_SurfVec[i] )
            {
                int ncol = 10;
                if ( c_ind % ncol == 0 )
                {
                    fprintf( fp, "color WHITE\n" );
                }
                else if ( c_ind % ncol == 1 )
                {
                    fprintf( fp, "color RED\n" );
                }
                else if ( c_ind % ncol == 2 )
                {
                    fprintf( fp, "color PURPLE\n" );
                }
                else if ( c_ind % ncol == 3 )
                {
                    fprintf( fp, "color DARK_BLUE\n" );
                }
                else if ( c_ind % ncol == 4 )
                {
                    fprintf( fp, "color DARK_ORANGE\n" );
                }
                else if ( c_ind % ncol == 5 )
                {
                    fprintf( fp, "color AQUA\n" );
                }
                else if ( c_ind % ncol == 6 )
                {
                    fprintf( fp, "color GREEN\n" );
                }
                else if ( c_ind % ncol == 7 )
                {
                    fprintf( fp, "color DARK_RED\n" );
                }
                else if ( c_ind % ncol == 8 )
                {
                    fprintf( fp, "color DARK_PURPLE\n" );
                }
                else if ( c_ind % ncol == 9 )
                {
                    fprintf( fp, "color LIGHT_BLUE\n" );
                }
                c_ind++;
//BLACK,WHITE, RED, BLUE, GREEN, PURPLE, YELLOW, GREY or GRAY,
//AQUA,DARK_BLUE,DARK_RED, DARK_PURPLE, DARK_ORANGE, DARK_GREY or
//DARK_GRAY and LIGHT_BLUE
                for ( int s = 0 ; s < ( int )( *c )->m_ISegDeque.size() ; s++ )
                {
                    IPnt* ip0 = ( *c )->m_ISegDeque[s]->m_IPnt[0];
                    IPnt* ip1 = ( *c )->m_ISegDeque[s]->m_IPnt[1];

                    int puwind = -1;
                    for ( int p = 0 ; p < ( int )ip0->m_Puws.size() ; p++ )
                    {
                        if ( ip0->m_Puws[p]->m_Surf == m_SurfVec[i] )
                        {
                            puwind = p;
                        }
                    }

                    if ( puwind >= 0 )
                    {
                        Puw* puw0 = ip0->m_Puws[puwind];
                        Puw* puw1 = ip1->m_Puws[puwind];

                        fprintf( fp, "MOVE \n" );
                        fprintf( fp, "%f %f\n", puw0->m_UW.x(), puw0->m_UW.y() );
                        fprintf( fp, "%f %f\n", puw1->m_UW.x(), puw1->m_UW.y() );
                    }
                }

                //==== Draw Intersections ====//
                for ( int s = 0 ; s < ( int )( *c )->m_SplitVec.size() ; s++ )
                {
                    ISegSplit* split = ( *c )->m_SplitVec[s];
                    if ( !( *c )->m_BorderFlag && split->m_Surf == m_SurfVec[i] )
                    {
                        fprintf( fp, "color YELLOW\n" );
                        fprintf( fp, "MOVE \n" );
                        fprintf( fp, "%f %f\n", split->m_UW.x(), split->m_UW.y() );
                        fprintf( fp, "%f %f\n", split->m_UW.x() + 0.1, split->m_UW.y() + 0.1 );
                    }

                }
            }
        }
        fclose( fp );
    }
}

void CfdMeshMgrSingleton::LoadBorderCurves()
{

    //==== Tesselate Border Curves ====//
    for ( int i = 0 ; i < ( int )m_ICurveVec.size() ; i++ )
    {
        if ( !m_ICurveVec[i]->m_SCurve_B )                              // Non Closed Solid
        {
            m_ICurveVec[i]->m_SCurve_B = m_ICurveVec[i]->m_SCurve_A;
        }

        if ( m_ICurveVec[i]->m_PlaneBorderIntersectFlag )
        {
            Surf* SurfA = m_ICurveVec[i]->m_SCurve_A->GetSurf();
            if ( !SurfA->GetSymPlaneFlag() )
            {
                m_ICurveVec[i]->PlaneBorderTesselate( m_ICurveVec[i]->m_SCurve_A, m_ICurveVec[i]->m_SCurve_B );
            }
            else
            {
                m_ICurveVec[i]->PlaneBorderTesselate( m_ICurveVec[i]->m_SCurve_B, m_ICurveVec[i]->m_SCurve_A );
            }
        }
        else
        {
            m_ICurveVec[i]->BorderTesselate( );
        }

        //==== Create New Chain ====//
        ISegChain* chain = new ISegChain;
        chain->m_BorderFlag = true;

        Surf* surfA = m_ICurveVec[i]->m_SCurve_A->GetSurf();
        Surf* surfB = m_ICurveVec[i]->m_SCurve_B->GetSurf();

        chain->m_SurfA = surfA;
        chain->m_SurfB = surfB;

        vector< vec3d > uwA = m_ICurveVec[i]->m_SCurve_A->GetUWTessPnts();
        vector< vec3d > uwB = m_ICurveVec[i]->m_SCurve_B->GetUWTessPnts();

        for ( int j = 1 ; j < ( int )uwA.size() ; j++ )
        {
            Puw* puwA0 = new Puw( surfA, vec2d( uwA[j - 1][0], uwA[j - 1][1] ) );
            Puw* puwA1 = new Puw( surfA, vec2d( uwA[j][0],   uwA[j][1] ) );
            Puw* puwB0 = new Puw( surfB, vec2d( uwB[j - 1][0], uwB[j - 1][1] ) );
            Puw* puwB1 = new Puw( surfB, vec2d( uwB[j][0],   uwB[j][1] ) );

            m_DelPuwVec.push_back( puwA0 );         // Save to delete later
            m_DelPuwVec.push_back( puwA1 );
            m_DelPuwVec.push_back( puwB0 );
            m_DelPuwVec.push_back( puwB1 );

            IPnt* p0 = new IPnt( puwA0, puwB0 );
            IPnt* p1 = new IPnt( puwA1, puwB1 );

            m_DelIPntVec.push_back( p0 );           // Save to delete later
            m_DelIPntVec.push_back( p1 );

            p0->CompPnt();
            p1->CompPnt();

            ISeg* seg = new ISeg( surfA, surfB, p0, p1 );
            chain->m_ISegDeque.push_back( seg );

        }

        if( chain->Valid() )
        {
            m_ISegChainList.push_front( chain );
        }
        else
        {
            delete chain;
            chain = NULL;
        }
    }

#ifdef DEBUG_CFD_MESH
    fprintf( m_DebugFile, "CfdMeshMgr::LoadBorderCurves \n" );
    fprintf( m_DebugFile, "   Total Num Chains = %d \n", m_ISegChainList.size() );

    list< ISegChain* >::iterator c;
    for ( c = m_ISegChainList.begin() ; c != m_ISegChainList.end(); c++ )
    {
        fprintf( m_DebugFile, "   Chain Num Segs = %d Border %d \n",
                 ( *c )->m_ISegDeque.size(), ( int )( ( *c )->m_BorderFlag ) );
    }
#endif

}

void CfdMeshMgrSingleton::BuildTestIntChains()
{
// For right now just add dummy line to first surface
    if ( m_SurfVec.size() > 0 )
    {
        for ( int i = 0 ; i < ( int )m_SurfVec.size(); i++ )
        {
            Surf* surf = m_SurfVec[i];
            double max_u = surf->GetSurfCore()->GetMaxU();
            double max_w = surf->GetSurfCore()->GetMaxW();
            int num_secs = 10;
            double delta = ( 0.9 - 0.1 ) / num_secs;
            vector< vec2d > uw_pnts;

            for ( int j = 0; j <= num_secs; j++ )
            {
                double d = delta * j;
                uw_pnts.push_back( vec2d( ( 0.1 + d )*max_u, 0.25 * max_w ) );
            }

            ISegChain* chain = new ISegChain;
            chain->m_SurfA = surf;
            chain->m_SurfB = surf;

            for ( int j = 1; j < ( int )uw_pnts.size(); j++ )
            {

                Puw* puw0A = new Puw( surf, uw_pnts[j - 1] );
                Puw* puw1A = new Puw( surf, uw_pnts[j] );
                Puw* puw0B = new Puw( surf, uw_pnts[j - 1] );
                Puw* puw1B = new Puw( surf, uw_pnts[j] );

                m_DelPuwVec.push_back( puw0A );         // Save to delete later
                m_DelPuwVec.push_back( puw1A );
                m_DelPuwVec.push_back( puw0B );         // Save to delete later
                m_DelPuwVec.push_back( puw1B );

                IPnt* p0 = new IPnt( puw0A, puw0B );
                IPnt* p1 = new IPnt( puw1A, puw1B );

                m_DelIPntVec.push_back( p0 );           // Save to delete later
                m_DelIPntVec.push_back( p1 );

                p0->CompPnt();
                p1->CompPnt();

                ISeg* seg = new ISeg( surf, surf, p0, p1 );
                chain->m_ISegDeque.push_back( seg );
            }

            if( chain->Valid() )
            {
                m_ISegChainList.push_back( chain );
            }
            else
            {
                delete chain;
                chain = NULL;
            }
        }
    }

}

void CfdMeshMgrSingleton::BuildSubSurfIntChains()
{
    // Adds subsurface intersection chains
    vec2d uw_pnt0;
    vec2d uw_pnt1;
    int num_sects = 100; // Number of segments to break subsurface segments up into

    // If there is an issue with having a watertight mesh between the intersection of two
    // components near a forced subsurface line, try increasing num_sects especially for highly
    // curved surfaces

    SubSurfaceMgr.PrepareToSplit(); // Prepare All SubSurfaces for Split
    for ( int s = 0 ; s < ( int )m_SurfVec.size() ; s++ )
    {
        // Get all SubSurfaces for the specified geom
        Surf* surf = m_SurfVec[s];
        vector< SubSurface* > ss_vec = SubSurfaceMgr.GetSubSurfs( surf->GetGeomID() );

        // Split SubSurfs
        for ( int ss = 0 ; ss < ( int ) ss_vec.size(); ss++ )
        {
            ss_vec[ss]->SplitSegsU( surf->GetSurfCore()->GetMinU() );
            ss_vec[ss]->SplitSegsU( surf->GetSurfCore()->GetMaxU() );
            ss_vec[ss]->SplitSegsW( surf->GetSurfCore()->GetMinW() );
            ss_vec[ss]->SplitSegsW( surf->GetSurfCore()->GetMaxW() );

            vector< SSLineSeg >& segs = ss_vec[ss]->GetSplitSegs();
            ISegChain* chain = NULL;

            bool new_chain = true;
            bool is_poly = ss_vec[ss]->GetPolyFlag();

            // Build Intersection Chains
            for ( int ls = 0; ls < ( int )segs.size(); ls++ )
            {
                if ( new_chain && chain )
                {
                    if ( chain->Valid() )
                    {
                        m_ISegChainList.push_back( chain );
                    }
                    else
                    {
                        delete chain;
                        chain = NULL;
                    }
                }

                if ( new_chain )
                {
                    chain = new ISegChain;
                    chain->m_SurfA = surf;
                    chain->m_SurfB = surf;
                    if ( !is_poly )
                    {
                        new_chain = false;
                    }
                }

                SSLineSeg l_seg = segs[ls];
                vec3d lp0, lp1;

                lp0 = l_seg.GetP0();
                lp1 = l_seg.GetP1();
                uw_pnt0 = vec2d( lp0.x(), lp0.y() );
                uw_pnt1 = vec2d( lp1.x(), lp1.y() );
                double max_u, max_w, tol;
                double min_u, min_w;
                tol = 1e-6;
                min_u = surf->GetSurfCore()->GetMinU();
                min_w = surf->GetSurfCore()->GetMinW();
                max_u = surf->GetSurfCore()->GetMaxU();
                max_w = surf->GetSurfCore()->GetMaxW();

                if ( uw_pnt0[0] < min_u || uw_pnt0[1] < min_w || uw_pnt1[0] < min_u || uw_pnt1[1] < min_w )
                {
                    new_chain = true;
                    continue; // Skip if either point has a value not on this surface
                }
                if ( uw_pnt0[0] > max_u || uw_pnt0[1] > max_w || uw_pnt1[0] > max_u || uw_pnt1[1] > max_w )
                {
                    new_chain = true;
                    continue; // Skip if either point has a value not on this surface
                }
                if ( ((fabs( uw_pnt0[0]-max_u ) < tol && fabs( uw_pnt1[0]-max_u ) < tol) ||
                     (fabs( uw_pnt0[1]-max_w ) < tol && fabs( uw_pnt1[1]-max_w ) < tol) ||
                     (fabs( uw_pnt0[0]-min_u ) < tol && fabs( uw_pnt1[0]-min_u ) < tol) ||
                     (fabs( uw_pnt0[1]-min_w ) < tol && fabs( uw_pnt1[1]-min_w ) < tol))
                     && is_poly  )
                {
                    new_chain = true;
                    continue; // Skip if both end points are on the same edge of the surface
                }

                double delta_u = ( uw_pnt1[0] - uw_pnt0[0] ) / num_sects;
                double delta_w = ( uw_pnt1[1] - uw_pnt0[1] ) / num_sects;

                vector< vec2d > uw_pnts;
                uw_pnts.resize( num_sects + 1 );
                uw_pnts[0] = uw_pnt0;
                uw_pnts[num_sects] = uw_pnt1;

                // Add additional points between the segment endpoints to hopefully make the curve planar with the surface
                for ( int p = 1 ; p < num_sects ; p++ )
                {
                    uw_pnts[p] = vec2d( uw_pnt0[0] + delta_u * p, uw_pnt0[1] + delta_w * p );
                }

                for ( int p = 1 ; p < ( int ) uw_pnts.size() ; p++ )
                {
                    Puw* puwA0 = new Puw( surf, uw_pnts[p - 1] );
                    Puw* puwA1 = new Puw( surf, uw_pnts[p] );
                    Puw* puwB0 = new Puw( surf, uw_pnts[p - 1] );
                    Puw* puwB1 = new Puw( surf, uw_pnts[p] );

                    m_DelPuwVec.push_back( puwA0 );         // Save to delete later
                    m_DelPuwVec.push_back( puwA1 );
                    m_DelPuwVec.push_back( puwB0 );
                    m_DelPuwVec.push_back( puwB1 );

                    IPnt* p0 = new IPnt( puwA0, puwB0 );
                    IPnt* p1 = new IPnt( puwA1, puwB1 );

                    m_DelIPntVec.push_back( p0 );           // Save to delete later
                    m_DelIPntVec.push_back( p1 );

                    p0->CompPnt();
                    p1->CompPnt();

                    ISeg* seg = new ISeg( surf, surf, p0, p1 );
                    chain->m_ISegDeque.push_back( seg );
                }
            }
            if ( chain )
            {
                if ( chain->Valid() )
                {
                    m_ISegChainList.push_back( chain );
                }
                else
                {
                    delete chain;
                    chain = NULL;
                }
            }
        }
    }
}

void CfdMeshMgrSingleton::SplitBorderCurves()
{
    vector< IPnt* > splitPnts;
    list< ISegChain* >::iterator c;
    for ( c = m_ISegChainList.begin() ; c != m_ISegChainList.end(); c++ )
    {
        if  ( !( *c )->m_BorderFlag ) // Non Border Chains
        {
            splitPnts.push_back( ( *c )->m_ISegDeque.front()->m_IPnt[0] );
            splitPnts.push_back( ( *c )->m_ISegDeque.back()->m_IPnt[1]  );
        }
    }

    for ( c = m_ISegChainList.begin() ; c != m_ISegChainList.end(); c++ )
    {
        if ( ( *c )->m_BorderFlag )
        {
            for ( int i = 0 ; i < ( int )splitPnts.size() ; i++ )
            {
                Puw* uwA = splitPnts[i]->GetPuw( ( *c )->m_SurfA );
                Puw* uwB = splitPnts[i]->GetPuw( ( *c )->m_SurfB );
                if ( uwA )
                {
                    ( *c )->AddBorderSplit( splitPnts[i], uwA );
                }
                else if ( uwB )
                {
                    ( *c )->AddBorderSplit( splitPnts[i], uwB );
                }
            }
        }
    }

    //==== Load Only Border Chains ====//
    vector< ISegChain* > chains;
    for ( c = m_ISegChainList.begin() ; c != m_ISegChainList.end(); c++ )
    {
        if ( ( *c )->m_BorderFlag )
        {
            chains.push_back( ( *c ) );
        }
    }

    //==== Split Chains ====//
    for ( int i = 0 ; i < ( int )chains.size() ; i++ )
    {
        chains[i]->MergeSplits();
        vector< ISegChain* > new_chains = chains[i]->SortAndSplit();
        for ( int j = 0 ; j < ( int )new_chains.size() ; j++ )
        {
            new_chains[j]->m_BorderFlag = true;
            if ( new_chains[j]->Valid() )
            {
                m_ISegChainList.push_back( new_chains[j] );
            }
        }
    }

    //==== Load Only Border Chains ====//
    chains.clear();
    for ( c = m_ISegChainList.begin() ; c != m_ISegChainList.end(); c++ )
    {
        if ( ( *c )->m_BorderFlag )
        {
            chains.push_back( ( *c ) );
        }
    }

    ////==== Check if Border Chains Lie On Another Surfaces ====//
    //for ( int i = 0 ; i < (int)chains.size() ; i++ )
    //{
    //  Surf* surfA = chains[i]->m_SurfA;
    //  vector< Surf* > surfBVec = GetPossCoPlanarSurfs( surfA );
    //  if ( surfBVec.size() )
    //  {
    //      for ( int s = 0 ; s < (int)surfBVec.size() ; s++ )
    //      {
    //          Surf* surfB = surfBVec[s];
    //          vector< ISegChain* > new_chains = chains[i]->FindCoPlanarChains( surfB, surfA );
    //          for ( int j = 0 ; j < (int)new_chains.size() ; j++ )
    //          {
    //              new_chains[j]->m_BorderFlag = true;
    //              m_ISegChainList.push_back( new_chains[j] );
    //          }
    //      }
    //  }
    //}

#ifdef DEBUG_CFD_MESH
    m_DebugDraw = true;
    for ( c = m_ISegChainList.begin() ; c != m_ISegChainList.end(); c++ )
    {
        vector< vec3d > crv;
        for ( int i = 0 ; i < ( int )( *c )->m_ISegDeque.size() ; i++ )
        {
            IPnt* ip = ( *c )->m_ISegDeque[i]->m_IPnt[0];
            crv.push_back( ip->m_Pnt );

            if ( i == ( int )( *c )->m_ISegDeque.size() - 1 )
            {
                IPnt* ip = ( *c )->m_ISegDeque[i]->m_IPnt[1];
                crv.push_back( ip->m_Pnt );
            }
        }
        m_DebugCurves.push_back( crv );
        int crv_cnt = ( int )m_DebugCurves.size();
        int r = ( crv_cnt % 3 ) * 85;
        int g = ( ( crv_cnt + 2 ) % 5 ) * 55;
        int b = ( ( crv_cnt + 4 ) % 4 ) * 63;
        m_DebugColors.push_back( vec3d( r, g, b ) );
    }
#endif

}

void CfdMeshMgrSingleton::IntersectSplitChains()
{
    //==== Intersect Intersection Curves (Not Border Curves) ====//
    list< ISegChain* >::iterator c;
    vector< ISegChain* > chains;
    for ( c = m_ISegChainList.begin() ; c != m_ISegChainList.end(); c++ )
    {
        if ( !( *c )->m_BorderFlag )
        {
            chains.push_back( ( *c ) );
        }
    }

    //==== Build Bounding Boxes Around Intersection Curves ====//
    for ( int i = 0 ; i < ( int )chains.size() ; i++ )
    {
        chains[i]->BuildBoxes();
    }

    //==== Do Intersection ====//
    for ( int i = 0 ; i < ( int )chains.size() ; i++ )
    {
        for ( int j = i + 1 ; j < ( int )chains.size() ; j++ )
        {
            if ( chains[i]->m_SurfA == chains[j]->m_SurfA || chains[i]->m_SurfA == chains[j]->m_SurfB )
            {
                chains[i]->Intersect( chains[i]->m_SurfA, chains[j] );
            }
            else if ( chains[i]->m_SurfB == chains[j]->m_SurfA || chains[i]->m_SurfB == chains[j]->m_SurfB )
            {
                chains[i]->Intersect( chains[i]->m_SurfB, chains[j] );
            }
        }
    }

    //==== Merge Splits ====//
    for ( int i = 0 ; i < ( int )chains.size() ; i++ )
    {
        chains[i]->MergeSplits();
    }

    //==== Remove Chain End Splits ====//
    for ( int i = 0 ; i < ( int )chains.size() ; i++ )
    {
        chains[i]->RemoveChainEndSplits();
    }

    //==== Split Chains ====//
    for ( int i = 0 ; i < ( int )chains.size() ; i++ )
    {
        vector< ISegChain* > new_chains = chains[i]->SortAndSplit();
        for ( int j = 0 ; j < ( int )new_chains.size() ; j++ )
        {
            if ( new_chains[j]->Valid() )
            {
                m_ISegChainList.push_back( new_chains[j] );
            }

        }
    }
}

void CfdMeshMgrSingleton::MergeInteriorChainIPnts()
{
    //==== Merge Interior IPnts in Chains ====//
    list< ISegChain* >::iterator c;
    for ( c = m_ISegChainList.begin() ; c != m_ISegChainList.end(); c++ )
    {
        ( *c )->MergeInteriorIPnts();
    }
}

void CfdMeshMgrSingleton::TessellateChains()
{
    //==== Tessellate Chains ====//
    list< ISegChain* >::iterator c;
    for ( c = m_ISegChainList.begin() ; c != m_ISegChainList.end(); c++ )
    {
        if( ( *c )->GetWakeAttachChain() == NULL ) // Non wake-attach chains.
        {
            ( *c )->Tessellate();
            ( *c )->TransferTess();
            ( *c )->ApplyTess();
        }
    }

    for ( c = m_ISegChainList.begin() ; c != m_ISegChainList.end(); c++ )
    {
        if( ( *c )->GetWakeAttachChain() != NULL ) // Only wake-attach chains.
        {
            vector< double > u = ( *c )->GetWakeAttachChain()->m_ACurve.GetUTessPnts();
            ( *c )->m_ACurve.Tesselate( u ); // Copy tessellation from matching chain.
            ( *c )->TransferTess();
            ( *c )->ApplyTess();
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

    //  ////==== Compute Total Distance Betwee Points ====//
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

    for ( d = m_ISegChainList.begin() ; d != m_ISegChainList.end(); d++ )
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
    list< ISegChain* >::iterator c, d;
    for ( c = m_ISegChainList.begin() ; c != m_ISegChainList.end(); c++ )
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
    for ( c = m_ISegChainList.begin() ; c != m_ISegChainList.end(); c++ )
    {
        if ( ( *c )->m_BorderFlag )
        {
            borderVec.push_back( ( *c ) );
        }
    }


    //==== Find Wake Surfaces ====//
    map< Surf*, vector< Surf* > >::iterator iter;

    for ( iter = m_PossCoPlanarSurfMap.begin() ; iter != m_PossCoPlanarSurfMap.end() ; iter++ )
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
    Surf* surfA = chainIn->m_SurfA;

    vector< ISegChain* > new_chains = chainIn->FindCoPlanarChains( sPtr, surfA );

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
    //==== Load Chain End Points into Groups - Border Points First ====//
    list< ISegChain* >::iterator c;
    list < IPntGroup* > iPntGroupList;
    for ( c = m_ISegChainList.begin() ; c != m_ISegChainList.end(); c++ )
    {
        if ( ( *c )->m_BorderFlag )
        {
            iPntGroupList.push_back( new IPntGroup );
            m_DelIPntGroupVec.push_back( iPntGroupList.back() );
            iPntGroupList.back()->m_IPntVec.push_back( ( *c )->m_TessVec.front() ); // Add Front Point
            iPntGroupList.push_back( new IPntGroup );
            m_DelIPntGroupVec.push_back( iPntGroupList.back() );
            iPntGroupList.back()->m_IPntVec.push_back( ( *c )->m_TessVec.back() );  // Add Back Point

        }
    }
    //==== Add Rest of Chain Points ====//
    for ( c = m_ISegChainList.begin() ; c != m_ISegChainList.end(); c++ )
    {
        if ( !( *c )->m_BorderFlag )
        {
            iPntGroupList.push_back( new IPntGroup );
            m_DelIPntGroupVec.push_back( iPntGroupList.back() );
            iPntGroupList.back()->m_IPntVec.push_back( ( *c )->m_TessVec.front() ); // Add Front Point
            iPntGroupList.push_back( new IPntGroup );
            m_DelIPntGroupVec.push_back( iPntGroupList.back() );
            iPntGroupList.back()->m_IPntVec.push_back( ( *c )->m_TessVec.back() );  // Add Back Point
        }
    }

    // tol_fract previously was compared to the distance between groups as a fraction of the local edge length.
    // However, it currently is simply compared to the distance between groups.
    // Consequently, while a reasonable value was previously 1e-2, a much smaller value is now appropriate.
    double tol_fract = GetGridDensityPtr()->m_MinLen() / 100.0;
    MergeIPntGroups( iPntGroupList, tol_fract );

    //==== Merge Ipnts In Groups ====//
    list< IPntGroup* >::iterator g;
    vector< IPnt* > merged_ipnts;
    for ( g = iPntGroupList.begin() ; g != iPntGroupList.end(); g++ )
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
    for ( c = m_ISegChainList.begin() ; c != m_ISegChainList.end(); c++ )
    {
        IPnt* ip = ( *c )->m_TessVec.front();
        int cnt = 0;
        for ( g = iPntGroupList.begin() ; g != iPntGroupList.end(); g++ )
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
        for ( g = iPntGroupList.begin() ; g != iPntGroupList.end(); g++ )
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

void CfdMeshMgrSingleton::MergeIPntGroups( list< IPntGroup* > & iPntGroupList, double tol_fract )
{
    list< IPntGroup* >::iterator g;

    //===== Merge Two Closest Groups While Under Tol ====//
    IPntGroup* nearG1 = NULL;
    IPntGroup* nearG2 = NULL;
    double nearDistFract;
    bool stopFlag = false;
    while( !stopFlag )
    {
        stopFlag = true;
        nearDistFract = 1.0e12;

        //==== Find Closest Two Groups ====//
        list< IPntGroup* >::iterator g;
        for ( g = iPntGroupList.begin() ; g != iPntGroupList.end(); g++ )
        {
            list< IPntGroup* >::iterator h;
            for ( h = iPntGroupList.begin() ; h != iPntGroupList.end(); h++ )
            {
                if ( ( *g ) != ( *h ) )
                {
                    double df = ( *g )->GroupDist( ( *h ) );
                    if ( df < nearDistFract )
                    {
                        nearDistFract = df;
                        nearG1 = ( *g );
                        nearG2 = ( *h );
                    }
                }
            }
        }

        if ( nearDistFract < tol_fract )
        {
            if ( nearG1 && nearG2 )
            {
                nearG1->AddGroup( nearG2 );
            }
//          delete nearG2;
            iPntGroupList.remove( nearG2 );
            stopFlag = false;
        }
    }
}

void CfdMeshMgrSingleton::BuildMesh()
{
    //==== Mesh Each Surface ====//
    for ( int s = 0 ; s < ( int )m_SurfVec.size() ; s++ )
    {
        vector< ISegChain* > surf_chains;
        list< ISegChain* >::iterator c;
        for ( c = m_ISegChainList.begin() ; c != m_ISegChainList.end(); c++ )
        {
            if ( ( ( *c )->m_SurfA == m_SurfVec[s] || ( *c )->m_SurfB == m_SurfVec[s] ) )
            {
                surf_chains.push_back( ( *c ) );
            }
        }
        m_SurfVec[s]->InitMesh( surf_chains );
    }
}

// Determines if a triangle should be deleted based on its type and whether or not it is inside every other surface
bool CfdMeshMgrSingleton::SetDeleteTriFlag( int aType, bool symPlane, vector < bool > aInB )
{
    bool deleteTri = false;

    for ( int b = 0 ; b < ( int )m_SurfVec.size() ; b++ )
    {
        int c = m_SurfVec[b]->GetCompID();
        bool aInThisB = aInB[c];
        int bType = m_SurfVec[b]->GetSurfaceCfdType();

        // Can make absolute decisions about deleting a triangle or not in the cases below
        if ( aInThisB )
        {
            // Trim Symmetry plane
            if ( symPlane && m_SurfVec[b]->GetFarFlag() == true &&
                 GetCfdSettingsPtr()->GetFarCompFlag() == true )
            {
                return true;
            }
            // Normal(Positive) inside another Normal, or Negative inside another Negative
            if ( aType == bType && aType != vsp::CFD_TRANSPARENT && aInThisB )
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
        }
    }

    // Check non-absolute cases
    for ( int b = 0 ; b < ( int )m_SurfVec.size() ; b++ )
    {
        int c = m_SurfVec[b]->GetCompID();
        bool aInThisB = aInB[c];
        int bType = m_SurfVec[b]->GetSurfaceCfdType();

        if ( aInThisB )
        {
            if ( aType == vsp::CFD_NEGATIVE && bType == vsp::CFD_NORMAL )
            {
                return false;
            }
            else if ( aType == vsp::CFD_TRANSPARENT && bType == vsp::CFD_NORMAL )
            {
                return true;
            }
        }
        else
        {
            if ( aType == vsp::CFD_NEGATIVE )
            {
                deleteTri = true;
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
    list< Tri* >::iterator t;
    for ( s = 0 ; s < ( int )m_SurfVec.size() ; ++s ) // every surface
    {
        int tri_comp_id = m_SurfVec[s]->GetCompID();
        list <Tri*> triList = m_SurfVec[s]->GetMesh()->GetTriList();
        for ( t = triList.begin() ; t != triList.end(); ++t ) // every triangle
        {
            vector< vector< double > > t_vec_vec;

            if (GetCfdSettingsPtr()->GetSymSplittingOnFlag())
            {
                t_vec_vec.resize( m_NumComps + 10 );  // + 10 to handle possibility of outer domain and symmetry plane.
                ( *t )->insideSurf.resize(m_NumComps + 10);
                ( *t )->insideCount.resize(m_NumComps + 10);
            }
            else
            {
                t_vec_vec.resize( m_NumComps + 6 );
                ( *t )->insideSurf.resize(m_NumComps + 6);
                ( *t )->insideCount.resize(m_NumComps + 6);
            }

            vec3d cp = ( *t )->ComputeCenterPnt( m_SurfVec[s] );
            vec3d ep = cp + vec3d( x_dist, 1.0e-4, 1.0e-4 );

            for ( int i = 0 ; i < ( int )m_SurfVec.size() ; ++i )
            {
                int comp_id = m_SurfVec[i]->GetCompID();
                if ( comp_id != tri_comp_id ) // Don't check self intersection.
                {
                    if ( m_SurfVec[i]->GetSurfaceCfdType() != vsp::CFD_TRANSPARENT) // Don't check against transparent surf.
                    {
                        m_SurfVec[i]->IntersectLineSeg( cp, ep, t_vec_vec[comp_id] );
                    }
                    else if ( m_SurfVec[i]->GetFarFlag() == true && m_SurfVec[s]->GetSymPlaneFlag() == true &&
                              GetCfdSettingsPtr()->GetFarCompFlag() == true ) // Unless trimming sym plane by outer domain
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

                if ( m_SurfVec[s]->GetSymPlaneFlag() == true && m_SurfVec[i]->GetFarFlag() == true &&
                     GetCfdSettingsPtr()->GetFarCompFlag() == true )
                {
                    if ( ( int )( t_vec_vec[c].size() + 1 ) % 2 == 1 ) // +1 Reverse action on sym plane wrt outer boundary.
                    {
                        ( *t )->insideSurf[c] = true;
                    }
                }
                else
                {

                    if ( ( int )t_vec_vec[c].size() % 2 == 1)
                    {
                        ( *t )->insideSurf[c] = true;
                    }
                }
            }
        }

        for ( t = triList.begin() ; t != triList.end(); ++t ) // every triangle
        {
            //==== Load Adjoining Tris - NOT Crossing Borders ====//
            set< Tri* > triSet;
            ( *t )->LoadAdjTris( 3, triSet );

            set<Tri*>::iterator st;

            for ( int i = 0 ; i < ( int )m_SurfVec.size() ; ++i )
            {
                int c = m_SurfVec[i]->GetCompID();

                for ( st = triSet.begin() ; st != triSet.end() ; ++st )
                {
                    if ( ( *t )->insideSurf[c] )
                    {
                        ( *st )->insideCount[c]++;
                    }
                    else
                    {
                        ( *st )->insideCount[c]--;
                    }
                }
            }
        }
    }

    //==== Check Vote and Mark Interior Tris =====//
    for ( s = 0 ; s < ( int )m_SurfVec.size() ; ++s )
    {
        list <Tri*> triList = m_SurfVec[s]->GetMesh()->GetTriList();
        for ( t = triList.begin() ; t != triList.end(); t++ )
        {
            for ( int i = 0 ; i < ( int )m_SurfVec.size() ; ++i )
            {
                int aType = m_SurfVec[s]->GetSurfaceCfdType();
                int bType = m_SurfVec[i]->GetSurfaceCfdType();
                int c = m_SurfVec[i]->GetCompID();

                if ( ( *t )->insideCount[c] > 0 )
                {
                    ( *t )->insideSurf[c] = true;
                }
                else if ( ( *t )->insideCount[c] < 0 )
                {
                    ( *t )->insideSurf[c] = false;
                }
                else // Can't determine if Tri is inside or outside based on neighbor votes
                {
                    printf( "IntExtCount ZERO!\n" );
                }
            }
        }
    }

    for ( int a = 0 ; a < ( int )m_SurfVec.size() ; a++ )
    {
        int tri_comp_id = m_SurfVec[a]->GetCompID();
        list< Tri * > triList = m_SurfVec[a]->GetMesh()->GetTriList();
        for ( t = triList.begin(); t != triList.end(); ++t )
        {
            // Determine if the triangle should be deleted
            ( *t )->deleteFlag = SetDeleteTriFlag( m_SurfVec[a]->GetSurfaceCfdType(), m_SurfVec[a]->GetSymPlaneFlag(), ( *t )->insideSurf );
        }
    }

    //==== Check For Half Mesh ====//
    if ( GetCfdSettingsPtr()->GetHalfMeshFlag() )
    {
        for ( s = 0 ; s < ( int )m_SurfVec.size() ; s++ )
        {
            if ( m_SurfVec[s]->GetSymPlaneFlag() == false )
            {
                list <Tri*> triList = m_SurfVec[s]->GetMesh()->GetTriList();
                for ( t = triList.begin() ; t != triList.end(); t++ )
                {
                    vec3d cp = ( *t )->ComputeCenterPnt( m_SurfVec[s] );
                    if ( cp[1] < -1.0e-10 )
                    {
                        ( *t )->deleteFlag = true;
                    }
                }
            }

            if( !GetCfdSettingsPtr()->GetFarMeshFlag() ) // Don't keep symmetry plane.
            {
                if ( m_SurfVec[s]->GetSymPlaneFlag() == true )
                {
                    list <Tri*> triList = m_SurfVec[s]->GetMesh()->GetTriList();
                    for ( t = triList.begin() ; t != triList.end(); t++ )
                    {
                        ( *t )->deleteFlag = true;
                    }
                }
            }
        }
    }

    //==== Remove Tris, Edges and Nodes ====//
    for ( s = 0 ; s < ( int )m_SurfVec.size() ; s++ )
    {
        m_SurfVec[s]->GetMesh()->RemoveInteriorTrisEdgesNodes();
    }
}

void CfdMeshMgrSingleton::ConnectBorderEdges( bool wakeOnly )
{
    list< Edge* >::iterator e;
    list< Edge* > edgeList;
    list< Tri* >::iterator t;
    for ( int s = 0 ; s < ( int )m_SurfVec.size() ; s++ )
    {
        if ( m_SurfVec[s]->GetWakeFlag() == wakeOnly )
        {
            list <Tri*> triList = m_SurfVec[s]->GetMesh()->GetTriList();
            for ( t = triList.begin() ; t != triList.end(); t++ )
            {
                if ( ( *t )->e0->OtherTri( ( *t ) ) == NULL )
                {
                    edgeList.push_back( ( *t )->e0 );
                }
                if ( ( *t )->e1->OtherTri( ( *t ) ) == NULL )
                {
                    edgeList.push_back( ( *t )->e1 );
                }
                if ( ( *t )->e2->OtherTri( ( *t ) ) == NULL )
                {
                    edgeList.push_back( ( *t )->e2 );
                }
            }
        }
    }

    int i, j, k;
//  int num_grid = 10;
    int num_grid = 1;  // jrg change back to 10????

    vector< vector< vector< list< Edge* > > > > edgeGrid;
    edgeGrid.resize( num_grid );
    for (  i = 0 ; i < num_grid ; i++ )
    {
        edgeGrid[i].resize( num_grid );
    }
    for (  i = 0 ; i < num_grid ; i++ )
        for (  j = 0 ; j < num_grid ; j++ )
        {
            edgeGrid[i][j].resize( num_grid );
        }

    BndBox box;
    for ( e = edgeList.begin() ; e != edgeList.end() ; e++ )
    {
        box.Update( ( *e )->n0->pnt );
        box.Update( ( *e )->n1->pnt );
    }

    double slop = 1.0e-5;
    double min_x = box.GetMin( 0 ) - slop;
    double min_y = box.GetMin( 1 ) - slop;
    double min_z = box.GetMin( 2 ) - slop;
    double dx = 2 * slop + ( box.GetMax( 0 ) - box.GetMin( 0 ) ) / ( double )( num_grid );
    double dy = 2 * slop + ( box.GetMax( 1 ) - box.GetMin( 1 ) ) / ( double )( num_grid );
    double dz = 2 * slop + ( box.GetMax( 2 ) - box.GetMin( 2 ) ) / ( double )( num_grid );

    for ( e = edgeList.begin() ; e != edgeList.end() ; e++ )
    {
        double mx = min( ( *e )->n0->pnt[0], ( *e )->n1->pnt[0] );
        double my = min( ( *e )->n0->pnt[1], ( *e )->n1->pnt[1] );
        double mz = min( ( *e )->n0->pnt[2], ( *e )->n1->pnt[2] );

        int ix = ( int )( ( mx - min_x ) / dx );
        int iy = ( int )( ( my - min_y ) / dy );
        int iz = ( int )( ( mz - min_z ) / dz );
        edgeGrid[ix][iy][iz].push_back( ( *e ) );
    }

    for ( i = 0 ; i < num_grid ; i++ )
        for ( j = 0 ; j < num_grid ; j++ )
            for ( k = 0 ; k < num_grid ; k++ )
            {
                if ( edgeGrid[i][j][k].size() > 0 )
                {
                    MatchBorderEdges( edgeGrid[i][j][k] );
                }
            }

}

void CfdMeshMgrSingleton::MatchBorderEdges( list< Edge* > edgeList )
{
    list< Edge* >::iterator e;
    list< Edge* >::iterator f;

    //==== Match Edges ====//
    double dist_tol = 0.01 * 0.01;
    bool stopFlag = false;
    if ( edgeList.size() <= 1  )
    {
        stopFlag = true;
    }

    while( !stopFlag )
    {
        double close_dist = 1.0e12;
        Edge* close_e = NULL;
        Edge* close_f = NULL;
        for ( e = edgeList.begin() ; e != edgeList.end() ; e++ )
        {
            for ( f = edgeList.begin() ; f != edgeList.end() ; f++ )
            {
                if ( ( *e ) != ( *f ) )
                {
                    double d0011 = dist_squared( ( *e )->n0->pnt, ( *f )->n0->pnt ) +
                                   dist_squared( ( *e )->n1->pnt, ( *f )->n1->pnt );
                    double d0110 = dist_squared( ( *e )->n0->pnt, ( *f )->n1->pnt ) +
                                   dist_squared( ( *e )->n1->pnt, ( *f )->n0->pnt );

                    double d = min( d0011, d0110 );
                    if ( d < close_dist )
                    {
                        close_dist = d;
                        close_e = ( *e );
                        close_f = ( *f );
                    }
                }
            }
            if ( close_dist < dist_tol )
            {
                break;
            }
        }

        if ( close_e && close_f )
        {
            if ( close_dist < dist_tol )
            {
//printf("Match Edge %f %d \n",close_dist, edgeList.size()  );
                //==== Merge the 2 Edges ====//
                double d0011 = dist_squared( close_e->n0->pnt, close_f->n0->pnt ) +
                               dist_squared( close_e->n1->pnt, close_f->n1->pnt );

                double d0110 = dist_squared( close_e->n0->pnt, close_f->n1->pnt ) +
                               dist_squared( close_e->n1->pnt, close_f->n0->pnt );

                if ( d0011 < d0110 )
                {
                    //close_e->n0->pnt = (close_e->n0->pnt + close_f->n0->pnt)*0.5;
                    //close_e->n1->pnt = (close_e->n1->pnt + close_f->n1->pnt)*0.5;
                    close_f->n0->pnt = close_e->n0->pnt;
                    close_f->n1->pnt = close_e->n1->pnt;
                }
                else
                {
                    //close_e->n0->pnt = (close_e->n0->pnt + close_f->n1->pnt)*0.5;
                    //close_e->n1->pnt = (close_e->n1->pnt + close_f->n0->pnt)*0.5;
                    close_f->n1->pnt = close_e->n0->pnt;
                    close_f->n0->pnt = close_e->n1->pnt;
                }
                edgeList.remove( close_e );
                edgeList.remove( close_f );
            }
            else
            {
                close_e->debugFlag = true;
//              printf("Close Dist = %f\n", close_dist );
                edgeList.remove( close_e );
                edgeList.remove( close_f );

            }
        }

        if ( edgeList.size() <= 1  )
        {
            stopFlag = true;
        }
    }

#ifdef DEBUG_CFD_MESH
    if ( edgeList.size() != 0 )
    {
        for ( e = edgeList.begin() ; e != edgeList.end() ; e++ )
        {
            ( *e )->debugFlag = true;
        }

        fprintf( m_DebugFile, "CfdMeshMgr::MatchBorderEdges Missing Edges %d\n", edgeList.size() );
        for ( e = edgeList.begin() ; e != edgeList.end() ; e++ )
        {
            fprintf( m_DebugFile, "      Missing Edge : %f %f %f   %f %f %f\n",
                     ( *e )->n0->pnt.x(), ( *e )->n0->pnt.y(), ( *e )->n0->pnt.z(),
                     ( *e )->n1->pnt.x(), ( *e )->n1->pnt.y(), ( *e )->n1->pnt.z() );
        }
    }
#endif
}

void CfdMeshMgrSingleton::HighlightNextChain()
{
    m_HighlightChainIndex++;
    if ( m_HighlightChainIndex >= ( int )m_ISegChainList.size() )
    {
        m_HighlightChainIndex = 0;
    }
}

void CfdMeshMgrSingleton::DebugWriteChains( const char* name, bool tessFlag )
{

    for ( int i = 0 ; i < ( int )m_SurfVec.size() ; i++ )
    {
        char str[256];
        sprintf( str, "%s%d.dat", name, i );
        FILE* fp = fopen( str, "w" );

        int cnt = 0;
        list< ISegChain* >::iterator c;
        for ( c = m_ISegChainList.begin() ; c != m_ISegChainList.end(); c++ )
        {
            if ( m_SurfVec[i] == ( *c )->m_SurfA || m_SurfVec[i] == ( *c )->m_SurfB )
            {
                if ( cnt % 9 == 0 )
                {
                    fprintf( fp, "COLOR RED\n" );
                }
                else if ( cnt % 9 == 1 )
                {
                    fprintf( fp, "COLOR BLUE\n" );
                }
                else if ( cnt % 9 == 2 )
                {
                    fprintf( fp, "COLOR GREEN\n" );
                }
                else if ( cnt % 9 == 3 )
                {
                    fprintf( fp, "COLOR PURPLE\n" );
                }
                else if ( cnt % 9 == 4 )
                {
                    fprintf( fp, "COLOR YELLOW\n" );
                }
                else if ( cnt % 9 == 5 )
                {
                    fprintf( fp, "COLOR DARK_ORANGE\n" );
                }
                else if ( cnt % 9 == 6 )
                {
                    fprintf( fp, "COLOR GREY\n" );
                }
                else if ( cnt % 9 == 7 )
                {
                    fprintf( fp, "COLOR DARK_PURPLE\n" );
                }
                else if ( cnt % 9 == 8 )
                {
                    fprintf( fp, "COLOR AQUA\n" );
                }


                if ( tessFlag == false )
                {
                    for ( int j = 0 ; j < ( int )( *c )->m_ISegDeque.size() ; j++ )
                    {
                        fprintf( fp, "MOVE \n" );
                        vec2d uw0 = ( *c )->m_ISegDeque[j]->m_IPnt[0]->GetPuw( m_SurfVec[i] )->m_UW;
                        vec2d uw1 = ( *c )->m_ISegDeque[j]->m_IPnt[1]->GetPuw( m_SurfVec[i] )->m_UW;
                        vec2d tmp = uw0 + ( uw1 - uw0 ) * 0.1;
                        uw1 = uw1 + ( uw0 - uw1 ) * 0.1;
                        uw0 = tmp;
                        fprintf( fp, "%f %f\n", uw0[0], uw0[1] );
                        fprintf( fp, "%f %f\n", uw1[0], uw1[1] );
                    }
                }
                else
                {
                    fprintf( fp, "MOVE \n" );
                    for ( int j = 1 ; j < ( int )( *c )->m_TessVec.size() ; j++ )
                    {
                        vec2d uw0 = ( *c )->m_TessVec[j - 1]->GetPuw( m_SurfVec[i] )->m_UW;
                        vec2d uw1 = ( *c )->m_TessVec[j]->GetPuw( m_SurfVec[i] )->m_UW;
                        vec2d tmp = uw0 + ( uw1 - uw0 ) * 0.1;
                        uw1 = uw1 + ( uw0 - uw1 ) * 0.1;
                        uw0 = tmp;
                        fprintf( fp, "%f %f\n", uw0[0], uw0[1] );
                        fprintf( fp, "%f %f\n", uw1[0], uw1[1] );
                    }
                }
                cnt++;
            }
        }
        fclose( fp );
    }
}

void CfdMeshMgrSingleton::AddPossCoPlanarSurf( Surf* surfA, Surf* surfB )
{
    vector< Surf* > surfVec = GetPossCoPlanarSurfs( surfA );

    //==== Check If SurfB Already Stored ====//
    for ( int i = 0 ; i < ( int )surfVec.size() ; i++ )
    {
        if ( surfVec[i] == surfB )
        {
            return;
        }
    }

    m_PossCoPlanarSurfMap[surfA].push_back( surfB );
}

vector< Surf* > CfdMeshMgrSingleton::GetPossCoPlanarSurfs( Surf* surfPtr )
{
    if ( m_PossCoPlanarSurfMap.find( surfPtr ) != m_PossCoPlanarSurfMap.end() )
    {
        return m_PossCoPlanarSurfMap[surfPtr];
    }

    vector< Surf* > retSurfVec;
    return retSurfVec;
}



void CfdMeshMgrSingleton::TestStuff()
{
    if ( !m_SurfVec.size() )
    {
        return;
    }

    Surf* sPtr = m_SurfVec[0];

    vector< SurfPatch* > pVec = sPtr->GetPatchVec();

    if ( !pVec.size() )
    {
        return;
    }

    SurfPatch sp0;
    SurfPatch sp1;
    SurfPatch sp2;
    SurfPatch sp3;

    pVec[0]->split_patch( sp0, sp1, sp2, sp3 );

    vec3d psurf = pVec[0]->comp_pnt_01( 0.3, 0.3 );
    vec3d ppatch = sp0.comp_pnt_01( 0.6, 0.6 );
    double d = dist( psurf, ppatch );

    psurf = pVec[0]->comp_pnt_01( 0.6, 0.6 );
    ppatch = sp3.comp_pnt_01( 0.2, 0.2 );
    d = dist( psurf, ppatch );

    psurf = pVec[0]->comp_pnt_01( 0.3, 0.6 );
    ppatch = sp2.comp_pnt_01( 0.6, 0.2 );
    d = dist( psurf, ppatch );

    psurf = pVec[0]->comp_pnt_01( 0.6, 0.3 );
    ppatch = sp1.comp_pnt_01( 0.2, 0.6 );
    d = dist( psurf, ppatch );

    sp3.split_patch( sp0, sp1, sp2, sp3 );
    psurf = pVec[0]->comp_pnt_01( 0.6, 0.6 );
    ppatch = sp0.comp_pnt_01( 0.4, 0.4 );
    d = dist( psurf, ppatch );





}

void CfdMeshMgrSingleton::LoadDrawObjs( vector< DrawObj* > &draw_obj_vec )
{
    GetGridDensityPtr()->Highlight( GetCurrSource() );
    GetGridDensityPtr()->Show( GetCfdSettingsPtr()->m_DrawSourceFlag.Get() );
    GetGridDensityPtr()->LoadDrawObjs( draw_obj_vec );

    m_WakeMgr.Show( GetCfdSettingsPtr()->m_DrawSourceFlag.Get() );
    m_WakeMgr.LoadDrawObjs( draw_obj_vec );

    if( GetCfdSettingsPtr()->m_DrawFarPreFlag.Get() && GetCfdSettingsPtr()->GetFarMeshFlag() )
    {
        UpdateBBoxDO( m_Domain );
    }
    m_BBoxLineStripDO.m_Visible = GetCfdSettingsPtr()->m_DrawFarPreFlag.Get() && GetCfdSettingsPtr()->GetFarMeshFlag();
    draw_obj_vec.push_back( &m_BBoxLineStripDO );
    m_BBoxLinesDO.m_Visible = GetCfdSettingsPtr()->m_DrawFarPreFlag.Get() && GetCfdSettingsPtr()->GetFarMeshFlag();
    draw_obj_vec.push_back( &m_BBoxLinesDO );

    //Symmetry Splitting
    m_BBoxLineStripSymSplit.m_Visible = GetCfdSettingsPtr()->m_DrawFarPreFlag.Get()
                                        && GetCfdSettingsPtr()->GetFarMeshFlag()
                                        && GetCfdSettingsPtr()->GetSymSplittingOnFlag()
                                        && !GetCfdSettingsPtr()->GetFarCompFlag();
    draw_obj_vec.push_back( &m_BBoxLineStripSymSplit );
    m_BBoxLineSymSplit.m_Visible = m_BBoxLineStripSymSplit.m_Visible;
    draw_obj_vec.push_back( &m_BBoxLineSymSplit );

    // Render Tag Colors
    int num_tags = SubSurfaceMgr.GetNumTags();
    m_TagDO.resize( num_tags );
    map<int, DrawObj*> tag_dobj_map;
    map< std::vector<int>, int >::const_iterator mit;
    map< int, DrawObj* >::const_iterator dmit;
    map< std::vector<int>, int > tagMap = SubSurfaceMgr.GetSingleTagMap();
    int cnt = 0;

    // Calculate constants for color sequence.
    const int ncgrp = 6; // Number of basic colors
    const int ncstep = (int)ceil((double)num_tags/(double)ncgrp);
    const double nctodeg = 360.0/(ncgrp*ncstep);

    char str[256];
    for ( mit = tagMap.begin(); mit != tagMap.end() ; mit++ )
    {
        m_TagDO[cnt] = DrawObj();
        tag_dobj_map[ mit->second ] = &m_TagDO[cnt];
        sprintf( str, "%s_TAG_%d", GetID().c_str(), cnt );
        m_TagDO[cnt].m_GeomID = string( str );

        m_TagDO[cnt].m_Type = DrawObj::VSP_SHADED_TRIS;
        m_TagDO[cnt].m_Visible = false;
        if ( GetCfdSettingsPtr()->m_DrawMeshFlag.Get() ||
             GetCfdSettingsPtr()->m_ColorTagsFlag.Get() )   // At least mesh or tags are visible.
        {
            m_TagDO[cnt].m_Visible = true;

            if ( GetCfdSettingsPtr()->m_DrawMeshFlag.Get() &&
                 GetCfdSettingsPtr()->m_ColorTagsFlag.Get() ) // Both are visible.
            {
                m_TagDO[cnt].m_Type = DrawObj::VSP_HIDDEN_TRIS_CFD;
                m_TagDO[cnt].m_LineColor = vec3d( 0.4, 0.4, 0.4 );
            }
            else if ( GetCfdSettingsPtr()->m_DrawMeshFlag.Get() ) // Mesh only
            {
                m_TagDO[cnt].m_Type = DrawObj::VSP_HIDDEN_TRIS_CFD;
                m_TagDO[cnt].m_LineColor = vec3d( 0.4, 0.4, 0.4 );
            }
            else // Tags only
            {
                m_TagDO[cnt].m_Type = DrawObj::VSP_SHADED_TRIS;
            }
        }

        if ( GetCfdSettingsPtr()->m_ColorTagsFlag.Get() )
        {
            // Color sequence -- go around color wheel ncstep times with slight
            // offset from ncgrp basic colors.
            // Note, (cnt/ncgrp) uses integer division resulting in floor.
            double deg = ((cnt % ncgrp) * ncstep + (cnt / ncgrp)) * nctodeg;
            vec3d rgb = m_TagDO[cnt].ColorWheel( deg );
            rgb.normalize();

            for ( int i = 0; i < 3; i++ )
            {
                m_TagDO[cnt].m_MaterialInfo.Ambient[i] = (float)rgb.v[i]/5.0f;
                m_TagDO[cnt].m_MaterialInfo.Diffuse[i] = 0.4f + (float)rgb.v[i]/10.0f;
                m_TagDO[cnt].m_MaterialInfo.Specular[i] = 0.04f + 0.7f * (float)rgb.v[i];
                m_TagDO[cnt].m_MaterialInfo.Emission[i] = (float)rgb.v[i]/20.0f;
            }
            m_TagDO[cnt].m_MaterialInfo.Ambient[3] = 1.0f;
            m_TagDO[cnt].m_MaterialInfo.Diffuse[3] = 1.0f;
            m_TagDO[cnt].m_MaterialInfo.Specular[3] = 1.0f;
            m_TagDO[cnt].m_MaterialInfo.Emission[3] = 1.0f;

            m_TagDO[cnt].m_MaterialInfo.Shininess = 32.0f;
        }
        else
        {
            // No color needed for mesh only.
        }

        draw_obj_vec.push_back( &m_TagDO[cnt] );
        cnt++;
    }

    for ( int i = 0 ; i < ( int )m_SurfVec.size() ; i++ )
    {
        vector< vec3d > pVec = m_SurfVec[i]->GetMesh()->GetSimpPntVec();
        for ( int t = 0 ; t < ( int )m_SurfVec[i]->GetMesh()->GetSimpTriVec().size() ; t++ )
        {
            if ( ( !m_SurfVec[i]->GetWakeFlag() || GetCfdSettingsPtr()->m_DrawWakeFlag.Get() ) &&
                    ( !m_SurfVec[i]->GetFarFlag() || GetCfdSettingsPtr()->m_DrawFarFlag.Get() ) &&
                    ( !m_SurfVec[i]->GetSymPlaneFlag() || GetCfdSettingsPtr()->m_DrawSymmFlag.Get() ) )
            {
                SimpTri* stri = &m_SurfVec[i]->GetMesh()->GetSimpTriVec()[t];
                dmit = tag_dobj_map.find( SubSurfaceMgr.GetTag( stri->m_Tags ) );
                if ( dmit == tag_dobj_map.end() )
                {
                    continue;
                }

                DrawObj* obj = dmit->second;
                vec3d norm = cross( pVec[stri->ind1] - pVec[stri->ind0], pVec[stri->ind2] - pVec[stri->ind0] );
                norm.normalize();
                obj->m_PntVec.push_back( pVec[stri->ind0] );
                obj->m_PntVec.push_back( pVec[stri->ind1] );
                obj->m_PntVec.push_back( pVec[stri->ind2] );
                obj->m_NormVec.push_back( norm );
                obj->m_NormVec.push_back( norm );
                obj->m_NormVec.push_back( norm );
            }
        }
    }

    // Render bad edges
    m_MeshBadEdgeDO.m_GeomID = GetID() + "BADEDGE";
    m_MeshBadEdgeDO.m_Type = DrawObj::VSP_LINES;
    m_MeshBadEdgeDO.m_Visible = GetCfdSettingsPtr()->m_DrawBadFlag.Get();
    m_MeshBadEdgeDO.m_LineColor = vec3d( 1, 0, 0 );
    m_MeshBadEdgeDO.m_LineWidth = 3.0;

    vector< vec3d > badEdgeData;

    vector< Edge* >::iterator e;
    for ( e = m_BadEdges.begin() ; e != m_BadEdges.end(); e++ )
    {
        badEdgeData.push_back( ( *e )->n0->pnt );
        badEdgeData.push_back( ( *e )->n1->pnt );
    }
    m_MeshBadEdgeDO.m_PntVec = badEdgeData;
    // Normal Vec is not required, load placeholder.
    m_MeshBadEdgeDO.m_NormVec = badEdgeData;

    draw_obj_vec.push_back( &m_MeshBadEdgeDO );

    m_MeshBadTriDO.m_GeomID = GetID() + "BADTRI";
    m_MeshBadTriDO.m_Type = DrawObj::VSP_HIDDEN_TRIS_CFD;
    m_MeshBadTriDO.m_Visible = GetCfdSettingsPtr()->m_DrawBadFlag.Get();
    m_MeshBadTriDO.m_LineColor = vec3d( 1, 0, 0 );
    m_MeshBadTriDO.m_LineWidth = 3.0;

    vector< vec3d > badTriData;
    vector< Tri* >::iterator t;
    for ( t = m_BadTris.begin() ; t != m_BadTris.end(); t++ )
    {
        badTriData.push_back( ( *t )->n0->pnt );
        badTriData.push_back( ( *t )->n1->pnt );
        badTriData.push_back( ( *t )->n2->pnt );
    }
    m_MeshBadTriDO.m_PntVec = badTriData;
    // Normal Vec is not required, load placeholder.
    m_MeshBadTriDO.m_NormVec = badTriData;

    draw_obj_vec.push_back( &m_MeshBadTriDO );
}

/*
void CfdMeshMgr::Draw()
{
    bool isShown = m_Vehicle->getScreenMgr()->getCfdMeshScreen()->isShown();
    if ( !isShown )
        return;

    UpdateSourcesAndWakes();
    UpdateDomain();

    glLineWidth( 1.0 );
    glColor4ub( 255, 0, 0, 255 );

    BaseSource* source = GetCurrSource();

    if ( m_DrawSourceFlag )
    {
        GetGridDensityPtr()->Draw(source);
        m_WakeMgr.Draw();
    }

    if ( m_DrawFarPreFlag )
    {
        if ( m_FarMeshFlag )
        {
            glColor4ub( 0, 200, 0, 255 );
            Draw_BBox( m_Domain );
        }
    }

    if ( m_DrawMeshFlag )
    {
    ////////glLineWidth( 1.0 );
    ////////glColor4ub( 255, 0, 0, 255 );
    ////////glBegin( GL_LINES );
    ////////for ( int i = 0 ; i < debugPnts.size() ; i+=2 )
    ////////{
    ////////    if ( i%4 == 0 )
    ////////        glColor4ub( 255, 0, 0, 255 );
    ////////    else
    ////////        glColor4ub( 0, 0, 255, 255 );
    ////////    glVertex3dv( debugPnts[i].data() );
    ////////    glVertex3dv( debugPnts[i+1].data() );
    ////////}
    ////////glEnd();
    ////////glColor4ub( 0, 0, 0, 255 );
    ////////glPointSize(4.0);
    ////////glBegin( GL_POINTS );
    ////////for ( int i = 0 ; i < debugPnts.size() ; i++ )
    ////////{
    ////////    glVertex3dv( debugPnts[i].data() );
    ////////}
    ////////glEnd();


        //==== Draw Mesh ====//
        glPolygonOffset(2.0, 1);

        glCullFace( GL_BACK );                      // Cull Back Faces For Trans
        glEnable( GL_CULL_FACE );

#ifndef __APPLE__
        glEnable(GL_POLYGON_OFFSET_EXT);
#endif

        glColor4ub( 220, 220, 220, 255 );
        for ( int i = 0 ; i < (int)m_SurfVec.size() ; i++ )
        {

            vector< vec3d > pVec = m_SurfVec[i]->GetMesh()->GetSimpPntVec();
            for ( int t = 0 ; t < (int)m_SurfVec[i]->GetMesh()->GetSimpTriVec().size() ; t++ )
            {
                if ( !m_SurfVec[i]->GetWakeFlag() &&
                        ( !m_SurfVec[i]->GetFarFlag() || m_DrawFarFlag ) &&
                        ( !m_SurfVec[i]->GetSymPlaneFlag() || m_DrawSymmFlag ) )
                {
                    SimpTri* stri = &m_SurfVec[i]->GetMesh()->GetSimpTriVec()[t];
                    glBegin( GL_POLYGON );
                        glVertex3dv( pVec[stri->ind0].data() );
                        glVertex3dv( pVec[stri->ind1].data() );
                        glVertex3dv( pVec[stri->ind2].data() );
                    glEnd();
                }
            }
        }

        glLineWidth(1.0);
        glColor4ub( 100, 0, 100, 255 );
        for ( int i = 0 ; i < (int)m_SurfVec.size() ; i++ )
        {
            if ( !m_SurfVec[i]->GetWakeFlag() )
                glColor4ub( 100, 0, 100, 255 );
            else
                glColor4ub( 0, 100, 0, 255 );

            vector< vec3d > pVec = m_SurfVec[i]->GetMesh()->GetSimpPntVec();
            for ( int t = 0 ; t < (int)m_SurfVec[i]->GetMesh()->GetSimpTriVec().size() ; t++ )
            {
                if ( ( !m_SurfVec[i]->GetWakeFlag() || m_DrawWakeFlag ) &&
                        ( !m_SurfVec[i]->GetFarFlag() || m_DrawFarFlag ) &&
                        ( !m_SurfVec[i]->GetSymPlaneFlag() || m_DrawSymmFlag ) )
                {
                    SimpTri* stri = &m_SurfVec[i]->GetMesh()->GetSimpTriVec()[t];
                    glBegin( GL_LINE_LOOP );
                        glVertex3dv( pVec[stri->ind0].data() );
                        glVertex3dv( pVec[stri->ind1].data() );
                        glVertex3dv( pVec[stri->ind2].data() );
                    glEnd();
                }
            }
        }
        glDisable( GL_CULL_FACE );
#ifndef __APPLE__
        glDisable(GL_POLYGON_OFFSET_EXT);
#endif
    }


#ifdef DEBUG_CFD_MESH
    if ( m_DebugDraw )
    {
        for ( int i = 0 ; i < (int)m_DebugCurves.size() ; i++ )
        {
            glPointSize( 4.0 );
            glLineWidth( 2.0 );
            vec3d rgb = m_DebugColors[i];
            glColor4ub( (GLbyte)rgb[0], (GLbyte)rgb[1], (GLbyte)rgb[2], 255 );

            glBegin( GL_LINE_STRIP );
            for ( int j = 0 ; j < (int)m_DebugCurves[i].size() ; j++ )
            {
                glVertex3dv( m_DebugCurves[i][j].data() );
            }
            glEnd();

            glBegin( GL_POINTS );
            for ( int j = 0 ; j < (int)m_DebugCurves[i].size() ; j++ )
            {
                glVertex3dv( m_DebugCurves[i][j].data() );
            }
            glEnd();

        }
    }

#endif


    if ( m_DrawBadFlag )
    {
        vector< Edge* >::iterator e;

        glLineWidth( 3.0 );
        glColor3ub( 255, 0, 0 );
        glBegin( GL_LINES );
        for ( e = m_BadEdges.begin() ; e != m_BadEdges.end(); e++ )
        {
            glVertex3dv( (*e)->n0->pnt.data() );
            glVertex3dv( (*e)->n1->pnt.data() );
        }
        glEnd();


        vector< Tri* >::iterator t;

        for ( t = m_BadTris.begin() ; t != m_BadTris.end(); t++ )
        {
            glBegin( GL_POLYGON );
                glVertex3dv( (*t)->n0->pnt.data() );
                glVertex3dv( (*t)->n1->pnt.data() );
                glVertex3dv( (*t)->n2->pnt.data() );
            glEnd();

            glBegin( GL_LINE_LOOP );
                glVertex3dv( (*t)->n0->pnt.data() );
                glVertex3dv( (*t)->n1->pnt.data() );
                glVertex3dv( (*t)->n2->pnt.data() );
            glEnd();
        }

    }

    //glLineWidth( 1.0 );
    //glColor4ub( 150, 150, 150, 255 );
    //for ( int s = 0 ; s < (int)m_SurfVec.size() ; s++ )
    //{
    //  m_SurfVec[s]->Draw();
    //}

    //glPointSize( 4.0 );
    //glLineWidth( 1.0 );
    //glColor4ub( 255, 0, 0, 255 );
    //for ( int s = 0 ; s < (int)debugPatches.size() ; s++ )
    //{
    //  debugPatches[s]->Draw();
    //}


    //////for ( int r = 0 ; r < debugRayIsect.size() ; r++ )
    //////{
    //////  glColor4ub( 255, 0, 0, 255 );
    //////  glBegin( GL_LINE_STRIP );
    //////  for ( int i = 0 ; i < debugRayIsect[r].size() ; i++ )
    //////  {
    //////      glVertex3dv( debugRayIsect[r][i].data() );
    //////  }
    //////  glEnd();
    //////
    //////  glColor4ub( 255, 0, 255, 255 );
    //////  glBegin( GL_POINTS );
    //////  for ( int i = 0 ; i < debugRayIsect[r].size() ; i++ )
    //////  {
    //////      glVertex3dv( debugRayIsect[r][i].data() );
    //////  }
    //////  glEnd();
    //////}

    //glLineWidth( 2.0 );
    //int cnt = 0;
    //list< ISegChain* >::iterator c;
    //for ( c = m_ISegChainList.begin() ; c != m_ISegChainList.end(); c++ )
    //{
    //  if ( cnt%4 == 0 )               glColor4ub( 0, 255, 255, 100 );
    //  else if ( cnt%4 == 1 )          glColor4ub( 255, 0, 0, 100 );
    //  else if ( cnt%4 == 2 )          glColor4ub( 0, 255, 0, 100 );
    //  else if ( cnt%4 == 3 )          glColor4ub( 0, 0, 255, 100 );
    //  if ( cnt == m_HighlightChainIndex )
    //  {
    //      glColor4ub( 255, 255, 255, 100 );
    //      //if ( (*c)->m_SurfA ) (*c)->m_SurfA->Draw();
    //      //if ( (*c)->m_SurfB ) (*c)->m_SurfB->Draw();

    //      glColor4ub( 0, 255, 255, 255 );
    //      glLineWidth( 2.0 );
    //      glPointSize( 6.0 );
    //  }
    //  else
    //  {
    //      glColor4ub( 255, 0, 0, 255 );
    //      glLineWidth( 1.0 );
    //      glPointSize( 3.0 );
    //  }


    //  (*c)->Draw();

    //  cnt++;

    ////    //(*c)->m_ISegBoxA.Draw();
    ////    //(*c)->m_ISegBoxB.Draw();
    //}

}
*/

void CfdMeshMgrSingleton::UpdateBBoxDO( BndBox box )
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

    //===== Symmetry Plane Splitting Lines =====//
    if ( GetCfdSettingsPtr()->GetSymSplittingOnFlag() )
    {
        UpdateBBoxDOSymSplit( box );
    }
}

void CfdMeshMgrSingleton::UpdateBBoxDOSymSplit( BndBox box )
{
    vec3d temp = vec3d( box.GetMin( 0 ), box.GetMin( 1 ), box.GetMin( 2 ) );
    vector< vec3d > symLinestrip, symLine;
    vec3d outerBottomRight, outerBottomLeft, outerTopRight, outerTopLeft;
    vec3d innerBottomRight, innerBottomLeft, innerTopRight, innerTopLeft;
    vector<vec3d> corners;
    BndBox vehicleBBox = m_Vehicle->GetBndBox();
    double min = GetCfdSettingsPtr()->m_FarXScale.Get();
    double scale = 2.0;

    //Grab the minimum of the two scales
    if (GetCfdSettingsPtr()->m_FarZScale.Get() < min)
    {
        min = GetCfdSettingsPtr()->m_FarZScale.Get();
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
        && GetCfdSettingsPtr()->GetFarManLocFlag()))
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

    m_BBoxLineSymSplit.m_GeomID = GetID() + "BBOXL1";
    m_BBoxLineSymSplit.m_Type = DrawObj::VSP_LINES;
    m_BBoxLineSymSplit.m_LineWidth = 1.0;
    m_BBoxLineSymSplit.m_LineColor = vec3d(0, 200.0 / 255, 0);
    m_BBoxLineSymSplit.m_PntVec = symLine;
}

/*
//==== Compose Modeling Matrix ====//
void CfdMeshMgr::Draw_BBox( bbox box )
{
  double temp[3];
  temp[0] = box.get_min(0);
  temp[1] = box.get_min(1);
  temp[2] = box.get_min(2);

  glBegin( GL_LINE_STRIP );
    glVertex3dv(temp);
    temp[0] = box.get_max(0);
    glVertex3dv(temp);
    temp[1] = box.get_max(1);
    glVertex3dv(temp);
    temp[2] = box.get_max(2);
    glVertex3dv(temp);
    temp[0] = box.get_min(0);
    glVertex3dv(temp);
    temp[2] = box.get_min(2);
    glVertex3dv(temp);
    temp[1] = box.get_min(1);
    glVertex3dv(temp);
    temp[2] = box.get_max(2);
    glVertex3dv(temp);
    temp[0] = box.get_max(0);
    glVertex3dv(temp);
    temp[2] = box.get_min(2);
    glVertex3dv(temp);
  glEnd();

  glBegin( GL_LINE_STRIP );
    temp[2] = box.get_max(2);
    glVertex3dv(temp);
    temp[1] = box.get_max(1);
    glVertex3dv(temp);
  glEnd();

  glBegin( GL_LINE_STRIP );
    temp[2] = box.get_min(2);
    glVertex3dv(temp);
    temp[0] = box.get_min(0);
    glVertex3dv(temp);
  glEnd();

  glBegin( GL_LINE_STRIP );
    temp[2] = box.get_max(2);
    glVertex3dv(temp);
    temp[1] = box.get_min(1);
    glVertex3dv(temp);
  glEnd();

}
*/

void CfdMeshMgrSingleton::SetICurveVec( ICurve* newcurve, int loc )
{
    m_ICurveVec[loc] = newcurve;
}

void CfdMeshMgrSingleton::SubTagTris()
{
    SubSurfaceMgr.ClearTagMaps();
    map< string, int > tag_map;
    map< string, set<int> > geom_comp_map;
    map< int, int >  comp_num_map; // map from an unmerged component number to the surface number of geom
    int tag_number = 0;

    for ( int i = 0; i < (int)m_SurfVec.size(); i++ )
    {
        Surf* surf = m_SurfVec[i];
        string geom_id = surf->GetGeomID();
        string id = geom_id + to_string( (long long) surf->GetUnmergedCompID() );
        string name;

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

            if ( geom_ptr )
            {
                name = geom_ptr->GetName() + to_string( (long long)geom_comp_map[geom_id].size() );
                if ( surf->GetWakeFlag() ) name = geom_ptr->GetName()
                                                 + to_string( (long long)comp_num_map[ surf->GetUnmergedCompID() ] )
                                                 + "_Wake";
            }

            SubSurfaceMgr.m_CompNames.push_back(name);
        }

        surf->SetBaseTag( tag_map[id] );

    }

    SubSurfaceMgr.SetSubSurfTags( tag_number );
    SubSurfaceMgr.BuildCompNameMap();
}

