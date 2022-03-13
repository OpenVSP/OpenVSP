//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// SurfaceIntersectionMgr
//
//////////////////////////////////////////////////////////////////////

#include "SurfaceIntersectionMgr.h"
#include "VspUtil.h"
#include "SubSurfaceMgr.h"
#include "StringUtil.h"
#include <cfloat>  //For DBL_EPSILON

#include "eli/geom/intersect/intersect_surface.hpp"

#include "MeshAnalysis.h"

#ifdef DEBUG_CFD_MESH
#include <direct.h>
#endif


//=============================================================//
Wake::Wake()
{
    m_CompID = 0;
    m_Angle = 0;
    m_Scale = 1;
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

double Wake::DistToClosestLeadingEdgePnt( vec3d& pnt )
{
    double close_dist = 1.0e12;

    vector < double > pmap;
    m_LeadingEdge.get_pmap( pmap );

    for ( int i = 0; i < (int)pmap.size(); i++ )
    {
        curve_point_type v( m_LeadingEdge.f( pmap[i] ) );
        vec3d le( v.x(), v.y(), v.z() );

        double d = dist( le, pnt );
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
    if ( std::abs( dist_p0 ) < tol && std::abs( dist_p1 ) < tol )
    {
        m_LeadingCurves.push_back( curve );
    }
}

void Wake::BuildSurfs()
{
    //==== Find Comp ID & Build Surf ====//
    for ( int c = 0; c < (int)m_LeadingCurves.size(); c++ )
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
            s->SetSurfaceCfdType( vsp::CFD_TRANSPARENT );
            s->SetCompID( m_CompID );
            s->SetUnmergedCompID( unmerged_comp_id );
            s->SetRefGeomID( geom_id );
            s->SetSurfID( m_SurfVec.size() );
            s->SetWakeParentSurfID( wakeParentSurfID );

            if ( WakeMgr.GetStretchMeshFlag() )
            {
                s->GetSurfCore()->MakeWakeSurf( le_crv.GetCurve(), WakeMgr.GetEndX(), m_Angle );
            }
            else
            {
                s->GetSurfCore()->MakeWakeSurf( le_crv.GetCurve(), WakeMgr.GetEndX(), m_Angle, WakeMgr.GetStartStretchX(), m_Scale );
            }

            s->GetSurfCore()->BuildPatches( s );

            m_SurfVec.push_back( s );
        }
    }
}

//=============================================================//
//=============================================================//
WakeMgrSingleton::WakeMgrSingleton()
{
    m_EndX = 1.0;
    m_StartStretchX = 0.0;
    m_StretchMeshFlag = false;
}

WakeMgrSingleton::~WakeMgrSingleton()
{
    ClearWakes();
}

vec3d WakeMgrSingleton::ComputeTrailEdgePnt( vec3d p, double angle_deg )
{

    double z = p.z() + ( m_EndX - p.x() ) * tan( DEG2RAD( angle_deg ) );

    return vec3d( m_EndX, p[1], z );
}

void WakeMgrSingleton::SetLeadingEdges( vector < piecewise_curve_type >& wake_leading_edges )
{
    m_LeadingEdgeVec = wake_leading_edges;
}


void WakeMgrSingleton::ClearWakes()
{
    for ( int i = 0; i < (int)m_WakeVec.size(); i++ )
    {
        delete m_WakeVec[i];
    }
    m_WakeVec.clear();

}

void WakeMgrSingleton::CreateWakesAppendBorderCurves( vector< ICurve* >& border_curves, SimpleGridDensity* grid_density_ptr )
{
    int i, j;
    ClearWakes();

    //==== Create Wakes ====//
    for ( i = 0; i < (int)m_LeadingEdgeVec.size(); i++ )
    {
        Wake* w = new Wake();
        m_WakeVec.push_back( w );
        w->m_LeadingEdge = m_LeadingEdgeVec[i];
        w->m_Angle = m_WakeAngleVec[i];
        w->m_Scale = m_WakeScaleVec[i];
    }

    //==== Match Wake To Border Curves From Model ====//
    for ( i = 0; i < (int)m_WakeVec.size(); i++ )
    {
        for ( j = 0; j < (int)border_curves.size(); j++ )
        {
            m_WakeVec[i]->MatchBorderCurve( border_curves[j] );

        }
        m_WakeVec[i]->BuildSurfs();
    }

    //==== Find Matching Intersection/Border Curves =====//
    vector< Surf* > wake_surfs = GetWakeSurfs();
    vector< SCurve* > scurve_vec;
    for ( i = 0; i < (int)wake_surfs.size(); i++ )
    {
        wake_surfs[i]->BuildDistMap();
        wake_surfs[i]->SetGridDensityPtr( grid_density_ptr );
        wake_surfs[i]->FindBorderCurves();
        wake_surfs[i]->LoadSCurves( scurve_vec );
    }

    //==== Load Leading Edge SCurves ====//
    vector < SCurve* > leading_edge_scurves;
    for ( i = 0; i < (int)m_WakeVec.size(); i++ )
    {
        for ( j = 0; j < (int)m_WakeVec[i]->m_LeadingCurves.size(); j++ )
        {
            leading_edge_scurves.push_back( m_WakeVec[i]->m_LeadingCurves[j]->m_SCurve_A );
        }
    }

    //==== Match Leading Edge SCurves With Wake SCurves ====//
    for ( i = 0; i < (int)scurve_vec.size(); i++ )
    {
        for ( j = 0; j < (int)leading_edge_scurves.size(); j++ )
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
    for ( i = 0; i < (int)scurve_vec.size(); i++ )
    {
        for ( j = i + 1; j < (int)scurve_vec.size(); j++ )
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
    for ( i = 0; i < (int)scurve_vec.size(); i++ )
    {
        if ( !scurve_vec[i]->GetICurve() )
        {
            ICurve* icrv = new ICurve;
            icrv->SetACurve( scurve_vec[i] );
            border_curves.push_back( icrv );
        }
    }
}

vector< Surf* > WakeMgrSingleton::GetWakeSurfs()
{
    vector< Surf* > svec;
    for ( int i = 0; i < (int)m_WakeVec.size(); i++ )
    {
        for ( int j = 0; j < (int)m_WakeVec[i]->m_SurfVec.size(); j++ )
        {
            svec.push_back( m_WakeVec[i]->m_SurfVec[j] );
        }
    }
    return svec;
}

void WakeMgrSingleton::AppendWakeSurfs( vector< Surf* >& surf_vec )
{
    int last_id = surf_vec.back()->GetSurfID();
    vector< Surf* > wake_surf_vec = GetWakeSurfs();

    for ( int i = 0; i < (int)wake_surf_vec.size(); i++ )
    {
        wake_surf_vec[i]->SetSurfID( last_id + 1 + i );
        surf_vec.push_back( wake_surf_vec[i] );
    }

}

void WakeMgrSingleton::StretchWakes()
{
    for ( int i = 0; i < (int)m_WakeVec.size(); i++ )
    {
        for ( int j = 0; j < (int)m_WakeVec[i]->m_SurfVec.size(); j++ )
        {
            Mesh* msh = m_WakeVec[i]->m_SurfVec[j]->GetMesh();
            msh->StretchSimpPnts( m_StartStretchX, m_EndX, m_WakeVec[i]->m_Scale, m_WakeVec[i]->m_Angle );
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

void WakeMgrSingleton::LoadDrawObjs( vector< DrawObj* >& draw_obj_vec )
{
    vector< vec3d > wakeData;
    for ( int e = 0; e < (int)m_LeadingEdgeVec.size(); e++ )
    {
        double factor = m_WakeScaleVec[e] - 1.0;

        for ( int i = m_LeadingEdgeVec[e].get_t0(); i < (int)m_LeadingEdgeVec[e].get_tmax(); i++ )
        {
            curve_point_type v( m_LeadingEdgeVec[e].f( i ) );
            vec3d le( v.x(), v.y(), v.z() );

            wakeData.push_back( le );

            vec3d te = ComputeTrailEdgePnt( le, m_WakeAngleVec[e] );
            double numer = te.x() - m_StartStretchX;
            double fract = numer / ( m_EndX - m_StartStretchX );
            double xx = m_StartStretchX + numer * ( 1.0 + factor * fract * fract );
            double zz = te.z() + ( xx - te.x() ) * tan( DEG2RAD( m_WakeAngleVec[e] ) );
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

void WakeMgrSingleton::Show( bool flag )
{
    m_WakeDO.m_Visible = flag;
}

//=============================================================//
//=============================================================//


SurfaceIntersectionSingleton::SurfaceIntersectionSingleton() : ParmContainer()
{
    m_Vehicle = VehicleMgr.GetVehicle();

    m_HighlightChainIndex = 0;

    // Array allocated to (m_NumComps + 6) later, so if this isn't reset by then, the
    // allocation will fail with a negative argument.
    m_NumComps = -10;

    m_MeshInProgress = false;

    m_MessageName = "SurfIntersectMessage";

#ifdef DEBUG_CFD_MESH
    m_DebugDir  = Stringc( "MeshDebug/" );
    _mkdir( m_DebugDir.get_char_star() );
    m_DebugFile = fopen( "MeshDebug/log.txt", "w" );
    m_DebugDraw = false;
#endif

}

SurfaceIntersectionSingleton::~SurfaceIntersectionSingleton()
{
    CleanUp();

#ifdef DEBUG_CFD_MESH
    if ( m_DebugFile )
    {
        fclose( m_DebugFile );
    }
#endif

}

void SurfaceIntersectionSingleton::IntersectSurfaces()
{
    m_MeshInProgress = true;

    TransferMeshSettings();

    addOutputText( "Fetching Bezier Surfaces\n" );

    vector< XferSurf > xfersurfs;
    FetchSurfs( xfersurfs );

    // UpdateWakes must be before m_Vehicle->HideAll() to prevent components 
    // being being added to or removed from the Surface Intersection set
    UpdateWakes();
    WakeMgr.SetStretchMeshFlag( false );

    // Hide all geoms after fetching their surfaces
    m_Vehicle->HideAll();

    CleanUp();
    addOutputText( "Loading Bezier Surfaces\n" );
    LoadSurfs( xfersurfs );

    if ( GetSettingsPtr()->m_IntersectSubSurfs )
    {
        TransferSubSurfData();
    }

    CleanMergeSurfs();

    IdentifyCompIDNames();

    if ( m_SurfVec.size() == 0 )
    {
        addOutputText( "No Surfaces To Mesh\n" );
        m_MeshInProgress = false;
        return;
    }

    addOutputText( "Build Grid\n" );
    BuildGrid();

//    auto t1 = std::chrono::high_resolution_clock::now();

    addOutputText( "Intersect\n" );
    Intersect();
    addOutputText( "Finished Intersect\n" );

//    auto t2 = std::chrono::high_resolution_clock::now();
//    printf( "Intersect took %lld mus %f ms %f sec\n", std::chrono::duration_cast<std::chrono::microseconds>(t2-t1).count(), std::chrono::duration_cast<std::chrono::microseconds>(t2-t1).count()/1000.0, std::chrono::duration_cast<std::chrono::microseconds>(t2-t1).count()/1000000.0 );

    addOutputText( "Binary Adaptation Curve Approximation\n" );
    BinaryAdaptIntCurves();

    addOutputText( "Exporting Files\n" );
    ExportFiles();

    addOutputText( "Done\n" );

    m_MeshInProgress = false;
}

void SurfaceIntersectionSingleton::CleanUp()
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
    for ( cl = m_ISegChainList.begin() ; cl != m_ISegChainList.end(); ++cl )
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

    m_AllIPnts.clear(); // Pointers should already be deallocated 

    m_PossCoPlanarSurfMap.clear();

    debugPnts.clear();

    m_SimpleSubSurfaceVec.clear();

    m_IPatchADrawLines.clear();
    m_IPatchBDrawLines.clear();
}

void SurfaceIntersectionSingleton::RegisterAnalysis()
{
    string analysis_name = "SurfaceIntersection";

    if ( !AnalysisMgr.FindAnalysis( analysis_name ) )
    {
        SurfaceIntersectionAnalysis* sia = new SurfaceIntersectionAnalysis();

        if ( sia && !AnalysisMgr.RegisterAnalysis( analysis_name, sia ) )
        {
            delete sia;
        }
    }
}

void SurfaceIntersectionSingleton::TransferMeshSettings()
{
    m_IntersectSettings = SimpleIntersectSettings();
    m_IntersectSettings.CopyFrom( m_Vehicle->GetISectSettingsPtr() );
}

void SurfaceIntersectionSingleton::IdentifyCompIDNames()
{
    m_CompIDNameMap.clear();

    for ( size_t i = 0; i < m_SurfVec.size(); i++ )
    {
        if ( m_CompIDNameMap.count( m_SurfVec[i]->GetCompID() ) == 0 )
        {
            Geom* geom = m_Vehicle->FindGeom( m_SurfVec[i]->GetGeomID() );
            m_CompIDNameMap[m_SurfVec[i]->GetCompID()] = geom->GetName();
        }
    }
}

void SurfaceIntersectionSingleton::TransferSubSurfData()
{
    vector < SubSurface* > ss_vec = SubSurfaceMgr.GetSubSurfs();
    m_SimpleSubSurfaceVec.resize( ss_vec.size() );

    for ( size_t i = 0; i < ss_vec.size(); i++ )
    {
        m_SimpleSubSurfaceVec[i] = SimpleSubSurface();
        m_SimpleSubSurfaceVec[i].CopyFrom( ss_vec[i] );
    }
}

//==== Get vector of SimpleSubSurfaces from geom by ID and surf number ====//
vector< SimpleSubSurface > SurfaceIntersectionSingleton::GetSimpSubSurfs( string geom_id, int surfnum, int comp_id )
{
    vector< SimpleSubSurface > ret_vec;

    // m_CompID < 0 indicates an FeaPart surface. SubSurfaces on FeaParts is not supported at this time 
    if ( comp_id < 0 )
    {
        return ret_vec;
    }

    Geom* geom = m_Vehicle->FindGeom( geom_id );
    if ( !geom )
    {
        return ret_vec;
    }

    if ( geom->GetType().m_Type == MESH_GEOM_TYPE ||
         geom->GetType().m_Type == HUMAN_GEOM_TYPE )
    {
        return ret_vec;
    }

    int imain = geom->GetMainSurfID( surfnum );

    if ( imain < 0 )
    {
        return ret_vec;
    }

    for ( int i = 0; i < m_SimpleSubSurfaceVec.size(); i++ )
    {
        if ( imain == m_SimpleSubSurfaceVec[i].m_MainSurfIndx && strcmp( m_SimpleSubSurfaceVec[i].GetCompID().c_str(), geom_id.c_str() ) == 0 )
        {
            ret_vec.push_back( m_SimpleSubSurfaceVec[i] );
        }
    }

    return ret_vec;
}

void SurfaceIntersectionSingleton::addOutputText( const string &str, int output_type )
{
    if ( output_type != QUIET_OUTPUT )
    {

        MessageData data;
        data.m_String = m_MessageName;
        data.m_StringVec.push_back( str );
        MessageMgr::getInstance().Send( "ScreenMgr", NULL, data );
    }
}

void SurfaceIntersectionSingleton::FetchSurfs( vector< XferSurf > &xfersurfs )
{
    m_Vehicle->FetchXFerSurfs( GetSettingsPtr()->m_SelectedSetIndex, xfersurfs );
}

void SurfaceIntersectionSingleton::LoadSurfs( vector< XferSurf > &xfersurfs, int start_surf_id )
{
    int maxcompid = -1;
    for ( int i = 0; i < xfersurfs.size(); i++ )
    {
        Surf* surfPtr = new Surf();
        surfPtr->GetSurfCore()->SetSurf( xfersurfs[i].m_Surface );

        surfPtr->SetGeomID( xfersurfs[i].m_GeomID );

        surfPtr->SetMainSurfID( xfersurfs[i].m_SurfIndx );

        //Xfersurfs created each time and flips normal if geom is a negative volume
        if ( xfersurfs[i].m_SurfCfdType == vsp::CFD_NEGATIVE )
        {
            xfersurfs[i].m_FlipNormal = !xfersurfs[i].m_FlipNormal;
        }

        surfPtr->SetFlipFlag( xfersurfs[i].m_FlipNormal );

        //Sets whether NORMAL, NEGATIVE, TRANSPARENT
        surfPtr->SetSurfaceCfdType(xfersurfs[i].m_SurfCfdType);

        //Sets whether NORMAL_SURF, WING_SURF, DISK_SURF, PROP_SURF
        surfPtr->SetSurfaceVSPType(xfersurfs[i].m_SurfType);

        int cid = xfersurfs[i].m_CompIndx;

        if ( cid > maxcompid )
            maxcompid = cid;

        if ( GetSettingsPtr()->m_DemoteSurfsCubicFlag )
        {
            // Demote higher order surfaces to cubic (do not promote lower order)
            piecewise_surface_type::index_type minu, minw, maxu, maxw;
            surfPtr->GetSurfCore()->GetSurf()->degree_u( minu, maxu );
            surfPtr->GetSurfCore()->GetSurf()->degree_v( minw, maxw );

            if ( maxu > 3 )
            {
                surfPtr->GetSurfCore()->GetSurf()->to_cubic_u( GetSettingsPtr()->m_CubicSurfTolerance );
            }
            if ( maxw > 3 )
            {
                surfPtr->GetSurfCore()->GetSurf()->to_cubic_v( GetSettingsPtr()->m_CubicSurfTolerance );
            }
        }

        surfPtr->SetCompID( cid );
        surfPtr->SetUnmergedCompID( cid );
        surfPtr->SetSurfID( start_surf_id + i );
        surfPtr->GetSurfCore()->BuildPatches( surfPtr );
        m_SurfVec.push_back( surfPtr );
    }

    // Don't include FEA parts (with negative Comp index)
    if ( maxcompid >= 0 )
    {
        m_NumComps = maxcompid + 1;
    }
}

void SurfaceIntersectionSingleton::CleanMergeSurfs()
{

    vector < Surf* > surfs = m_SurfVec;
    m_SurfVec.clear();

    for ( int s = 0 ; s < ( int )surfs.size(); s++ )
    {

        Surf* surfPtr = surfs[s];

        bool addSurfFlag = true;

        if ( surfPtr->GetCompID() >= 0 )
        {
            if ( GetSettingsPtr()->m_HalfMeshFlag && surfPtr->GetSurfCore()->LessThanY( 1e-6 ) )
            {
                addSurfFlag = false;
            }
        }
        else if ( surfPtr->GetCompID() < 0 ) // Indicates FEA Part Surface (Increase Tolerance)
        {
            if ( GetSettingsPtr()->m_HalfMeshFlag && surfPtr->GetSurfCore()->LessThanY( 1e-3 ) )
            {
                addSurfFlag = false;
            }
        }

        if ( GetSettingsPtr()->m_HalfMeshFlag && surfPtr->GetSurfCore()->PlaneAtYZero() )
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

void SurfaceIntersectionSingleton::DeleteDuplicateSurfs()
{
    unsigned int nsurf = m_SurfVec.size();

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

void SurfaceIntersectionSingleton::BuildGrid()
{

    int i, j;
    vector< SCurve* > scurve_vec;
    for ( i = 0 ; i < ( int )m_SurfVec.size() ; i++ )
    {
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
    WakeMgr.CreateWakesAppendBorderCurves( m_ICurveVec, GetGridDensityPtr() );
    WakeMgr.AppendWakeSurfs( m_SurfVec );

#ifdef DEBUG_CFD_MESH
    fprintf( m_DebugFile, "CfdMeshMgr::BuildGrid \n" );
    fprintf( m_DebugFile, "  Num unmatched SCurves = %d \n", num_unmatched );

    for ( i = 0 ; i < ( int )m_ICurveVec.size() ; i++ )
    {
        m_ICurveVec[i]->DebugEdgeMatching( m_DebugFile );
    }

#endif
}

void SurfaceIntersectionSingleton::ExportFiles()
{
    if ( GetIntersectSettingsPtr()->GetExportFileFlag( vsp::INTERSECT_SRF_FILE_NAME ) )
    {
        WriteSurfsIntCurves( GetIntersectSettingsPtr()->GetExportFileName( vsp::INTERSECT_SRF_FILE_NAME ) );
    }

    if ( GetIntersectSettingsPtr()->GetExportFileFlag( vsp::INTERSECT_CURV_FILE_NAME ) )
    {
        WriteGridToolCurvFile( GetIntersectSettingsPtr()->GetExportFileName( vsp::INTERSECT_CURV_FILE_NAME ),
                               GetIntersectSettingsPtr()->m_ExportRawFlag );
    }

    if ( GetIntersectSettingsPtr()->GetExportFileFlag( vsp::INTERSECT_PLOT3D_FILE_NAME ) )
    {
        WritePlot3DFile( GetIntersectSettingsPtr()->GetExportFileName( vsp::INTERSECT_PLOT3D_FILE_NAME ),
                         GetIntersectSettingsPtr()->m_ExportRawFlag );
    }

    if ( GetIntersectSettingsPtr()->GetExportFileFlag( vsp::INTERSECT_IGES_FILE_NAME ) || GetIntersectSettingsPtr()->GetExportFileFlag( vsp::INTERSECT_STEP_FILE_NAME ) )
    {
        BuildNURBSCurvesVec(); // Note: Must be called before BuildNURBSSurfMap
    }

    if ( GetIntersectSettingsPtr()->GetExportFileFlag( vsp::INTERSECT_IGES_FILE_NAME ) )
    {
        string delim = StringUtil::get_delim( GetIntersectSettingsPtr()->m_CADLabelDelim );

        WriteIGESFile( GetIntersectSettingsPtr()->GetExportFileName( vsp::INTERSECT_IGES_FILE_NAME ), GetIntersectSettingsPtr()->m_CADLenUnit,
                       GetIntersectSettingsPtr()->m_CADLabelID, GetIntersectSettingsPtr()->m_CADLabelSurfNo, GetIntersectSettingsPtr()->m_CADLabelSplitNo,
                       GetIntersectSettingsPtr()->m_CADLabelName, delim );
    }

    if ( GetIntersectSettingsPtr()->GetExportFileFlag( vsp::INTERSECT_STEP_FILE_NAME ) )
    {
        string delim = StringUtil::get_delim( GetIntersectSettingsPtr()->m_CADLabelDelim );

        WriteSTEPFile( GetIntersectSettingsPtr()->GetExportFileName( vsp::INTERSECT_STEP_FILE_NAME ), GetIntersectSettingsPtr()->m_CADLenUnit,
                       GetIntersectSettingsPtr()->m_STEPTol, GetIntersectSettingsPtr()->m_STEPMergePoints,
                       GetIntersectSettingsPtr()->m_CADLabelID, GetIntersectSettingsPtr()->m_CADLabelSurfNo, GetIntersectSettingsPtr()->m_CADLabelSplitNo,
                       GetIntersectSettingsPtr()->m_CADLabelName, delim, GetIntersectSettingsPtr()->m_STEPRepresentation );
    }
}

void SurfaceIntersectionSingleton::WriteSurfsIntCurves( const string &filename )
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

        for ( iter = compMap.begin() ; iter != compMap.end() ; ++iter )
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
        for ( c = m_ISegChainList.begin() ; c != m_ISegChainList.end(); ++c )
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

            if ( ! GetSettingsPtr()->m_XYZIntCurveFlag )
            {
                fprintf( fp, "%d		// Number of Border Points (Au, Aw, Bu, Bw) \n", ( int )ipntVec.size() );
            }
            else
            {
                fprintf( fp, "%d		// Number of Border Points (Au, Aw, Bu, Bw, X, Y, Z) \n", ( int )ipntVec.size() );
            }

            for ( int j = 0 ; j < ( int )ipntVec.size() ; j++ )
            {
                Puw* pwA = ipntVec[j]->GetPuw( surfA );
                Puw* pwB = ipntVec[j]->GetPuw( surfB );

                if ( ! GetSettingsPtr()->m_XYZIntCurveFlag )
                {
                    fprintf( fp, "%d    %16.16lf, %16.16lf, %16.16lf, %16.16lf \n", j,
                             pwA->m_UW.x(), pwA->m_UW.y(), pwB->m_UW.x(), pwB->m_UW.y() );
                }
                else
                {
                    vec3d pA = surfA->CompPnt( pwA->m_UW.x(), pwA->m_UW.y() );
                    vec3d pB = surfB->CompPnt( pwB->m_UW.x(), pwB->m_UW.y() );
                    vec3d p = ( pA + pB ) * 0.5;

                    fprintf( fp, "%d    %16.16lf, %16.16lf, %16.16lf, %16.16lf, %16.16lf, %16.16lf, %16.16lf \n", j,
                             pwA->m_UW.x(), pwA->m_UW.y(), pwB->m_UW.x(), pwB->m_UW.y(),
                             p.x(), p.y(), p.z() );
                }
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

            if ( ! GetSettingsPtr()->m_XYZIntCurveFlag )
            {
                fprintf( fp, "%d		// Number of Intersect Points (Au, Aw, Bu, Bw) \n", ( int )ipntVec.size() );
            }
            else
            {
                fprintf( fp, "%d		// Number of Intersect Points (Au, Aw, Bu, Bw, X, Y, Z) \n", ( int )ipntVec.size() );
            }

            for ( int j = 0 ; j < ( int )ipntVec.size() ; j++ )
            {
                Puw* pwA = ipntVec[j]->GetPuw( surfA );
                Puw* pwB = ipntVec[j]->GetPuw( surfB );

                if ( ! GetSettingsPtr()->m_XYZIntCurveFlag )
                {
                    fprintf( fp, "%d    %16.16lf, %16.16lf, %16.16lf, %16.16lf \n", j,
                             pwA->m_UW.x(), pwA->m_UW.y(), pwB->m_UW.x(), pwB->m_UW.y() );
                }
                else
                {
                    vec3d pA = surfA->CompPnt( pwA->m_UW.x(), pwA->m_UW.y() );
                    vec3d pB = surfB->CompPnt( pwB->m_UW.x(), pwB->m_UW.y() );
                    vec3d p = ( pA + pB ) * 0.5;

                    fprintf( fp, "%d    %16.16lf, %16.16lf, %16.16lf, %16.16lf, %16.16lf, %16.16lf, %16.16lf \n", j,
                             pwA->m_UW.x(), pwA->m_UW.y(), pwB->m_UW.x(), pwB->m_UW.y(),
                             p.x(), p.y(), p.z() );
                }
            }
            fprintf( fp, "END Intersect_Curve\n" );
        }


        fprintf( fp, "END IntersectCurve_List\n" );


        fclose( fp );
    }

}

void SurfaceIntersectionSingleton::WriteGridToolCurvFile( const string &filename, bool rawflag )
{
    FILE* fp = fopen( filename.c_str(), "w" );
    if ( fp )
    {
        int ncurve = 0;
        if ( rawflag )
        {
            ncurve = m_RawCurveAVec.size();
        }
        else
        {
            ncurve = m_BinAdaptCurveAVec.size();
        }

        int indx = 0;
        for ( indx = 0; indx < ncurve; indx++ )
        {
            // Assume A and B curves are coincident -- just print A curve.
            vector<vec3d> ptvec;

            if ( rawflag )
            {
                ptvec = m_RawCurveAVec[ indx ];
            }
            else
            {
                ptvec = m_BinAdaptCurveAVec[ indx ];
            }

            fprintf( fp, "%zu\n", ptvec.size() );

            for ( int i = 0; i < ptvec.size(); i++ )
            {
                vec3d pt = ptvec[i];
                fprintf( fp, "%21.15e %21.15e %21.15e\n", pt.x(), pt.y(), pt.z() );
            }

        }
        fclose( fp );
    }
}

void SurfaceIntersectionSingleton::WritePlot3DFile( const string &filename, bool rawflag )
{
    FILE* fp = fopen( filename.c_str(), "w" );
    if ( fp )
    {
        int nchain = 0;
        vector < vector < vec3d > > *allpts;

        // Assume A and B curves are coincident -- just print A curve.
        if ( rawflag )
        {
            nchain = m_RawCurveAVec.size();
            allpts = &m_RawCurveAVec;
        }
        else
        {
            nchain = m_BinAdaptCurveAVec.size();
            allpts = &m_BinAdaptCurveAVec;
        }

        fprintf( fp, " %d\n", nchain );

        int ichain = 0;
        for ( ichain = 0; ichain < nchain; ichain++ )
        {
            fprintf( fp, " %d 1 1\n", (*allpts)[ichain].size() );
        }

        for ( ichain = 0; ichain < nchain; ichain++ )
        {
            for ( int i = 0; i < (*allpts)[ichain].size(); i++ )
            {
                vec3d pt = (*allpts)[ichain][i];
                fprintf( fp, "%25.17e ", pt.x() );
            }
            fprintf( fp, "\n" );

            for ( int i = 0; i < (*allpts)[ichain].size(); i++ )
            {
                vec3d pt = (*allpts)[ichain][i];
                fprintf( fp, "%25.17e ", pt.y() );
            }
            fprintf( fp, "\n" );

            for ( int i = 0; i < (*allpts)[ichain].size(); i++ )
            {
                vec3d pt = (*allpts)[ichain][i];
                fprintf( fp, "%25.17e ", pt.z() );
            }
            fprintf( fp, "\n" );
        }

        fclose( fp );
    }
}

Surf* SurfaceIntersectionSingleton::FindSurf( int surf_id )
{
    for ( size_t i = 0; i < m_SurfVec.size(); i++ )
    {
        if ( m_SurfVec[i]->GetSurfID() == surf_id )
        {
            return m_SurfVec[i];
        }
    }
    return NULL;
}

void SurfaceIntersectionSingleton::WriteIGESFile( const string& filename, int len_unit,
                                                  bool label_id, bool label_surf_num, bool label_split_num,
                                                  bool label_name, string label_delim )
{
    BuildNURBSSurfMap();

    if ( m_NURBSSurfVec.size() == 0 )
    {
        addOutputText( "Error: Can't Export IGES - No Valid Surfaces\n" );
        return;
    }

    IGESutil iges( len_unit );

    for ( size_t si = 0; si < m_NURBSSurfVec.size(); si++ )
    {
        // Match NURBS_Surface to index in m_SurfVec using m_SurfID. 
        Surf* current_surf = FindSurf( m_NURBSSurfVec[si].m_SurfID );

        string label;

        if ( label_id && current_surf )
        {
            label = current_surf->GetGeomID();
        }

        if ( label_name && current_surf )
        {
            if ( label.size() > 0 )
            {
                label.append( label_delim );
            }

            if ( current_surf->GetFeaPartIndex() >= 0 )
            {
                // FEA Part
                label.append( m_CompIDNameMap[current_surf->GetFeaPartIndex()] );
            }
            else
            {
                label.append( m_CompIDNameMap[current_surf->GetCompID()] );
            }
        }

        if ( label_surf_num && current_surf )
        {
            if ( label.size() > 0 )
            {
                label.append( label_delim );
            }
            label.append( to_string( current_surf->GetMainSurfID() ) );
        }

        if ( label_split_num )
        {
            if ( label.size() > 0 )
            {
                label.append( label_delim );
            }
            label.append( to_string( m_NURBSSurfVec[si].m_SurfID ) );
        }

        DLL_IGES_ENTITY_128 isurf = m_NURBSSurfVec[si].WriteIGESSurf( &iges, label.c_str() );
        
        m_NURBSSurfVec[si].WriteIGESLoops( &iges, isurf, label.c_str() );
    }

    iges.WriteFile( filename, true );
}

void SurfaceIntersectionSingleton::WriteSTEPFile( const string& filename, int len_unit, double tol,
                                                  bool merge_pnts, bool label_id, bool label_surf_num, bool label_split_num,
                                                  bool label_name, string label_delim, int representation  )
{
    STEPutil step( len_unit, tol );

    // Identify the SdaiB_spline_curve_with_knots. This must come before BuildNURBSSurfMap for STEP files, or the 
    // edge pointer will not be transferred between surfaces
    for ( size_t i = 0; i < m_NURBSCurveVec.size(); i++ )
    {
        // Don't write subsurface or structural entity intersections as STEP edges (surface splitting along these curve types not supported)
        if ( !m_NURBSCurveVec[i].m_SubSurfFlag && m_NURBSCurveVec[i].m_SurfA_Type != vsp::CFD_STRUCTURE )
        {
            m_NURBSCurveVec[i].WriteSTEPEdge( &step, to_string(i), merge_pnts ); // TODO: Improve STEP Edge Naming
        }
    }

    BuildNURBSSurfMap();

    if ( m_NURBSSurfVec.size() == 0 )
    {
        addOutputText( "Error: Can't Export STEP - No Valid Surfaces\n" );
        return;
    }

    // Identify the unique sets of intersected components
    vector < vector < int > > comp_id_group_vec = GetCompIDGroupVec();

    vector < vector < SdaiAdvanced_face* > > adv_vec( comp_id_group_vec.size() );
    map < string, vector < SdaiSurface* > > geom_surf_label_map;

    for ( size_t si = 0; si < m_NURBSSurfVec.size(); si++ )
    {
        // Match NURBS_Surface to index in m_SurfVec using m_SurfID. 
        Surf* current_surf = FindSurf( m_NURBSSurfVec[si].m_SurfID );

        string label;

        if ( label_id && current_surf )
        {
            label = current_surf->GetGeomID();
        }

        if ( label_name && current_surf )
        {
            if ( label.size() > 0 )
            {
                label.append( label_delim );
            }

            if ( current_surf->GetFeaPartIndex() >= 0 )
            {
                // FEA Part
                label.append( m_CompIDNameMap[current_surf->GetFeaPartIndex()] );
            }
            else
            {
                label.append( m_CompIDNameMap[current_surf->GetCompID()] );
            }
        }

        if ( label_surf_num && current_surf )
        {
            if ( label.size() > 0 )
            {
                label.append( label_delim );
            }
            label.append( to_string( current_surf->GetMainSurfID() ) );
        }

        if ( label_split_num )
        {
            if ( label.size() > 0 )
            {
                label.append( label_delim );
            }
            label.append( to_string( m_NURBSSurfVec[si].m_SurfID ) );
        }

        SdaiSurface* surf = m_NURBSSurfVec[si].WriteSTEPSurf( &step, label, merge_pnts );
        geom_surf_label_map[label].push_back( surf );

        int comp_id = -1;
        for ( size_t j = 0; j < m_SurfVec.size(); j++ )
        {
            if ( m_SurfVec[j]->GetSurfID() == m_NURBSSurfVec[si].m_SurfID )
            {
                comp_id = m_SurfVec[j]->GetCompID();
                break;
            }
        }

        for ( size_t j = 0; j < comp_id_group_vec.size(); j++ )
        {
            if ( std::count( comp_id_group_vec[j].begin(), comp_id_group_vec[j].end(), comp_id ) )
            {
                vector < SdaiAdvanced_face* > adv = m_NURBSSurfVec[si].WriteSTEPLoops( &step, surf, label, merge_pnts );
                adv_vec[j].insert( adv_vec[j].end(), adv.begin(), adv.end() );
            }
        }
    }

    map < string, vector < SdaiSurface* > >::iterator it;

    for ( it = geom_surf_label_map.begin(); it != geom_surf_label_map.end(); ++it )
    {
        SdaiGeometric_set* gset = (SdaiGeometric_set*)step.registry->ObjCreate( "GEOMETRIC_SET" );
        step.instance_list->Append( (SDAI_Application_instance*)gset, completeSE );
        gset->name_( "'" + ( *it ).first + "'" );

        for ( size_t i = 0; i < ( *it ).second.size(); i++ )
        {
            gset->elements_()->AddNode( new EntityNode( (SDAI_Application_instance*)( *it ).second[i] ) );
        }
    }

    // TODO: Don't include transparent and structure surfaces in BREP?

    if ( representation == vsp::STEP_SHELL )
    {
        step.RepresentManifoldShell( adv_vec );
    }
    else
    {
        step.RepresentBREPSolid( adv_vec );
    }

    step.WriteFile( filename );
}

vector < vector < int > > SurfaceIntersectionSingleton::GetCompIDGroupVec()
{
    // Identify the unique sets of intersected components
    map < int, vector < int > > intersection_comp_id_map;
    list< ISegChain* >::iterator i_seg;

    for ( i_seg = m_ISegChainList.begin(); i_seg != m_ISegChainList.end(); ++i_seg )
    {
        int comp_A_id = ( *i_seg )->m_SurfA->GetCompID();
        int comp_B_id = ( *i_seg )->m_SurfB->GetCompID();

        if ( !std::count( intersection_comp_id_map[comp_A_id].begin(), intersection_comp_id_map[comp_A_id].end(), comp_B_id ) )
        {
            intersection_comp_id_map[comp_A_id].push_back( comp_B_id );
        }

        if ( !std::count( intersection_comp_id_map[comp_B_id].begin(), intersection_comp_id_map[comp_B_id].end(), comp_A_id ) )
        {
            intersection_comp_id_map[comp_B_id].push_back( comp_A_id );
        }
    }

    map< int, vector < int > >::iterator i_map;
    vector < vector < int > > comp_id_group_vec;

    for ( i_map = intersection_comp_id_map.begin(); i_map != intersection_comp_id_map.end(); ++i_map )
    {
        if ( comp_id_group_vec.size() == 0 )
        {
            comp_id_group_vec.push_back( i_map->second );
            continue;
        }

        bool matched = false;
        int group_index = -1;

        for ( size_t i = 0; i < i_map->second.size(); ++i )
        {
            if ( !matched )
            {
                for ( size_t j = 0; j < comp_id_group_vec.size(); ++j )
                {
                    if ( std::count( comp_id_group_vec[j].begin(), comp_id_group_vec[j].end(), i_map->second[i] ) )
                    {
                        matched = true;
                        group_index = j;
                        break;
                    }
                }
            }
            else if ( !std::count( comp_id_group_vec[group_index].begin(), comp_id_group_vec[group_index].end(), i_map->second[i] ) )
            {
                comp_id_group_vec[group_index].push_back( i_map->second[i] );
            }
        }

        if ( !matched )
        {
            comp_id_group_vec.push_back( i_map->second );
        }
    }

    return comp_id_group_vec;
}

void SurfaceIntersectionSingleton::BuildNURBSSurfMap()
{
    m_NURBSSurfVec.clear();

    for ( size_t si = 0; si < m_SurfVec.size(); si++ )
    {
        if ( m_SurfVec[si]->GetIgnoreSurfFlag() )
        {
            continue;
        }

        NURBS_Surface nurbs_surf = NURBS_Surface();
        // Create NURBS Surface
        nurbs_surf.InitNURBSSurf( m_SurfVec[si] );

        nurbs_surf.m_SurfType = m_SurfVec[si]->GetSurfaceCfdType();
        nurbs_surf.m_WakeFlag = m_SurfVec[si]->GetWakeFlag();

        // Identify all border and intersection NURBS curves on the surface
        vector < NURBS_Curve > nurbs_curve_vec = nurbs_surf.MatchNURBSCurves( m_NURBSCurveVec );
        nurbs_surf.SetNURBSCurveVec( nurbs_curve_vec );

        // Build NURBS loops
        nurbs_surf.BuildNURBSLoopMap();

        if ( nurbs_surf.m_NURBSLoopVec.size() == 1 &&
             nurbs_surf.m_NURBSLoopVec[0].m_BorderLoopFlag &&
             nurbs_surf.m_NURBSLoopVec[0].m_InternalLoopFlag )
        {
            continue; // Indicates that the surface is completely enclosed
        }
        else if ( nurbs_surf.m_NURBSLoopVec.size() == 1 &&
                  !nurbs_surf.m_NURBSLoopVec[0].m_InternalLoopFlag &&
                  nurbs_surf.m_SurfType == vsp::CFD_NEGATIVE )
        {
            continue; // external negative surfaces are removed
        }

        m_NURBSSurfVec.push_back( nurbs_surf );
    }
}

void SurfaceIntersectionSingleton::BuildNURBSCurvesVec()
{
    // Only define the NURBS curves once to help avoid tolerance errors
    m_NURBSCurveVec.clear();

    //==== Find Max Bound Box of All Components ====//
    BndBox big_box;
    for ( size_t s = 0; s < m_SurfVec.size(); ++s )
    {
        big_box.Update( m_SurfVec[s]->GetBBox() );
    }
    double x_dist = 1.0 + big_box.GetMax( 0 ) - big_box.GetMin( 0 );
    double y_dist = 1.0 + big_box.GetMax( 1 ) - big_box.GetMin( 1 );
    double z_dist = 1.0 + big_box.GetMax( 2 ) - big_box.GetMin( 2 );

    list< ISegChain* >::iterator i_seg;

    for ( i_seg = m_ISegChainList.begin(); i_seg != m_ISegChainList.end(); ++i_seg )
    {
        bool internal_flag = false, ss_flag = false, wake_flag = false;

        // Check if the curve is interenal or external
        // Identify test point
        vec3d cp;
        if ( ( *i_seg )->m_ISegDeque.size() <= 2 )
        {
            // Take midpoint of first segment
            cp = ( ( *i_seg )->m_ISegDeque[0]->m_IPnt[0]->m_Pnt + ( *i_seg )->m_ISegDeque[0]->m_IPnt[1]->m_Pnt ) / 2.0;
        }
        else
        {
            // Identify point approximately halfway on border curve
            cp = ( *i_seg )->m_ISegDeque[( *i_seg )->m_ISegDeque.size() / 2]->m_IPnt[0]->m_Pnt;
        }

        // Check 3 directions and take majority result
        vec3d xep = cp + vec3d( x_dist, 1.0e-4, 1.0e-4 );
        vec3d yep = cp + vec3d( 1.0e-4, y_dist, 1.0e-4 );
        vec3d zep = cp + vec3d( 1.0e-4, 1.0e-4, z_dist );

        vector< double > x_vec, y_vec, z_vec;

        // Check if the curve is inside any component by checking the number of intersections from 3 vectors
        // beginning at the midpoint of the curve. Checking 3 vectors prevents a false positive or negative
        // from a vector that exactly aligns with another curve
        for ( size_t j = 0; j < m_NumComps; j++ )
        {
            if ( j == ( *i_seg )->m_SurfA->GetCompID() || j == ( *i_seg )->m_SurfB->GetCompID() )
            {
                continue;
            }

            for ( size_t i = 0; i < m_SurfVec.size(); i++ )
            {
                if ( ( m_SurfVec[i]->GetCompID() == j ) && ( m_SurfVec[i]->GetSurfaceCfdType() == vsp::CFD_NORMAL || 
                                                             m_SurfVec[i]->GetSurfaceCfdType() == vsp::CFD_NEGATIVE ) )
                {
                    m_SurfVec[i]->IntersectLineSeg( cp, xep, x_vec );
                    m_SurfVec[i]->IntersectLineSeg( cp, yep, y_vec );
                    m_SurfVec[i]->IntersectLineSeg( cp, zep, z_vec );
                }
            }

            bool x_in = x_vec.size() % 2 == 1;
            bool y_in = y_vec.size() % 2 == 1;
            bool z_in = z_vec.size() % 2 == 1;

            if ( ( x_in && y_in ) || ( x_in && z_in ) || ( y_in && z_in ) )
            {
                // Odd -> curve is inside
                internal_flag = true;
                break;
            }
        }

        bool in_negative = false;

        if ( internal_flag )
        {
            x_vec.clear();
            y_vec.clear();
            z_vec.clear();

            // Check if inside a negative component
            for ( size_t j = 0; j < m_NumComps; j++ )
            {
                if ( j == ( *i_seg )->m_SurfA->GetCompID() || j == ( *i_seg )->m_SurfB->GetCompID() )
                {
                    continue;
                }

                for ( size_t i = 0; i < m_SurfVec.size(); i++ )
                {
                    if ( ( m_SurfVec[i]->GetCompID() == j ) && m_SurfVec[i]->GetSurfaceCfdType() == vsp::CFD_NEGATIVE )
                    {
                        m_SurfVec[i]->IntersectLineSeg( cp, xep, x_vec );
                        m_SurfVec[i]->IntersectLineSeg( cp, yep, y_vec );
                        m_SurfVec[i]->IntersectLineSeg( cp, zep, z_vec );
                    }
                }

                bool x_in = x_vec.size() % 2 == 1;
                bool y_in = y_vec.size() % 2 == 1;
                bool z_in = z_vec.size() % 2 == 1;

                if ( ( x_in && y_in ) || ( x_in && z_in ) || ( y_in && z_in ) )
                {
                    // Odd -> curve is inside
                    in_negative = true;
                    break;
                }
            }
        }

        if ( !( *i_seg )->m_BorderFlag && ( ( *i_seg )->m_SurfA->GetCompID() == ( *i_seg )->m_SurfB->GetCompID() ) )
        {
            // Indicates a Sub-Surface
            ss_flag = true;
        }

        if ( ( ( *i_seg )->m_SurfA->GetWakeFlag() || ( *i_seg )->m_SurfB->GetWakeFlag() ) )
        {
            wake_flag = true;
        }

        NURBS_Curve nurbs_curve;

        nurbs_curve.m_BorderFlag = ( *i_seg )->m_BorderFlag;
        nurbs_curve.m_InternalFlag = internal_flag;
        nurbs_curve.m_InsideNegativeFlag = in_negative;
        nurbs_curve.m_SubSurfFlag = ss_flag;
        nurbs_curve.m_WakeFlag = wake_flag;
        nurbs_curve.m_SurfA_Type = ( *i_seg )->m_SurfA->GetSurfaceCfdType();
        nurbs_curve.m_SurfB_Type = ( *i_seg )->m_SurfB->GetSurfaceCfdType();
        nurbs_curve.m_SurfA_ID = ( *i_seg )->m_SurfA->GetSurfID();
        nurbs_curve.m_SurfB_ID = ( *i_seg )->m_SurfB->GetSurfID();

        nurbs_curve.InitNURBSCurve( ( *i_seg )->m_ACurve, GetSettingsPtr()->m_RelCurveTol );

        m_NURBSCurveVec.push_back( nurbs_curve );
    }
}

void SurfaceIntersectionSingleton::BuildCurves()
{
    list< ISegChain* >::iterator c;
    for ( c = m_ISegChainList.begin() ; c != m_ISegChainList.end(); ++c )
    {
        ( *c )->BuildCurves();
    }
}

void SurfaceIntersectionSingleton::Intersect()
{

    if ( GetSettingsPtr()->m_IntersectSubSurfs ) BuildSubSurfIntChains();

    //==== Quad Tree Intersection - Intersection Segments Get Loaded at AddIntersectionSeg ===//
    for ( int i = 0 ; i < ( int )m_SurfVec.size() ; i++ )
    {
        for ( int j = i + 1; j < (int) m_SurfVec.size(); j++ )
        {
            m_SurfVec[i]->Intersect( m_SurfVec[j], this );
        }
    }

    // WriteISegs();

    addOutputText( "BuildChains\n" );
    BuildChains();
    // DebugWriteChains( "BuildChains", false );

    MergeFeaPartSSEdgeOverlap(); // Only applicable to FEA Mesh

    addOutputText( "LoadBorderCurves\n" );
    LoadBorderCurves();
    // DebugWriteChains( "LoadBorderCurves", false );

    MergeInteriorChainIPnts();
    // DebugWriteChains( "MergeInteriorChainIPnts", false );

    SetFixPointBorderNodes(); // Only applicable to FEA Mesh
    CheckFixPointIntersects(); // Only applicable to FEA Mesh

    addOutputText( "SplitBorderCurves\n" );
    SplitBorderCurves();
    // DebugWriteChains( "SplitBorderCurves", false );

    addOutputText( "IntersectSplitChains\n" );
    IntersectSplitChains();
    // DebugWriteChains( "IntersectSplitChains", false );

    addOutputText( "BuildCurves\n" );
    BuildCurves();
    // DebugWriteChains( "BuildCurves", false );
}

void SurfaceIntersectionSingleton::AddIntersectionSeg( const SurfPatch& pA, const SurfPatch& pB, const vec3d & ip0, const vec3d & ip1 )
{
    double d = dist_squared( ip0, ip1 );
    if ( d < DBL_EPSILON )
    {
        return;
    }

    vec2d plane_uwA0;
    pA.find_closest_uw_planar_approx( ip0, plane_uwA0.v );

    vec2d plane_uwB0;
    pB.find_closest_uw_planar_approx( ip0, plane_uwB0.v );

    vec2d plane_uwA1;
    pA.find_closest_uw_planar_approx( ip1, plane_uwA1.v );

    vec2d plane_uwB1;
    pB.find_closest_uw_planar_approx( ip1, plane_uwB1.v );

    // Intersections that lie exactly on a patch boundary will actually intersect both patches
    // that share that boundary.  So, detect intersections that lie on the patch minimum edge
    // and don't carry those forward.  Don't do this if the minimum parameter is zero.  I.e.
    // there is no prior patch.

    double tol = 1e-10; // Tolerance buildup due to SurfPatch::find_closest_uw_planar_approx and other inaccuracies

    if ( pA.get_u_min() > 0.0 ) // if Patch A is not the very beginning of u
    {
        double lim = pA.get_u_min() + tol;
        // if both points projected to A are on the starting edge of u
        if ( plane_uwA0.v[0] <= lim && plane_uwA1.v[0] <= lim )
        {
            return;
        }
    }

    if ( pB.get_u_min() > 0.0 ) // if Patch B is not the very beginning of u
    {
        double lim = pB.get_u_min() + tol;
        // if both points projected to B are on the starting edge of u
        if ( plane_uwB0.v[0] <= lim && plane_uwB1.v[0] <= lim )
        {
            return;
        }
    }

    if ( pA.get_w_min() > 0.0 ) // if Patch A is not the very beginning of w
    {
        double lim = pA.get_w_min() + tol;
        // if both points projected to A are on the starting edge of w
        if ( plane_uwA0.v[1] <= lim && plane_uwA1.v[1] <= lim )
        {
            return;
        }
    }

    if ( pB.get_w_min() > 0.0 ) // if Patch B is not the very beginning of w
    {
        double lim = pB.get_w_min() + tol;
        // if both points projected to B are on the starting edge of w
        if ( plane_uwB0.v[1] <= lim && plane_uwB1.v[1] <= lim )
        {
            return;
        }
    }

    vec2d proj_uwA0;
    pA.find_closest_uw( ip0, plane_uwA0.v, proj_uwA0.v );

    vec2d proj_uwB0;
    pB.find_closest_uw( ip0, plane_uwB0.v, proj_uwB0.v );

    vec2d proj_uwA1;
    pA.find_closest_uw( ip1, plane_uwA1.v, proj_uwA1.v );

    vec2d proj_uwB1;
    pB.find_closest_uw( ip1, plane_uwB1.v, proj_uwB1.v );

    Puw* puwA0 = new Puw( pA.get_surf_ptr(), proj_uwA0 );
    m_DelPuwVec.push_back( puwA0 );

    Puw* puwB0 = new Puw( pB.get_surf_ptr(), proj_uwB0 );
    m_DelPuwVec.push_back( puwB0 );

    IPnt* ipnt0 = new IPnt( puwA0, puwB0 );
    ipnt0->m_Pnt = ip0;
    m_DelIPntVec.push_back( ipnt0 );

    Puw* puwA1 = new Puw( pA.get_surf_ptr(), proj_uwA1 );
    m_DelPuwVec.push_back( puwA1 );

    Puw* puwB1 = new Puw( pB.get_surf_ptr(), proj_uwB1 );
    m_DelPuwVec.push_back( puwB1 );

    IPnt* ipnt1 = new IPnt( puwA1, puwB1 );
    ipnt1->m_Pnt = ip1;
    m_DelIPntVec.push_back( ipnt1 );

    // Identify rectangles to represent final patches
    m_IPatchADrawLines.push_back( pA.GetPatchDrawLines() );
    m_IPatchBDrawLines.push_back( pB.GetPatchDrawLines() );

    new ISeg( pA.get_surf_ptr(), pB.get_surf_ptr(), ipnt0, ipnt1 );

    m_AllIPnts.push_back( ipnt0 );
    m_AllIPnts.push_back( ipnt1 );

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

    double total_d = dA0 + dB0 + dA1 + dB1;

    if ( total_d > max_dist )
    {
        max_dist = total_d;
        fprintf( m_DebugFile, "  Proj Pnt Dist = %f    %d \n", max_dist, ipntcnt );
    }
    ipntcnt++;

#endif
}


ISeg* SurfaceIntersectionSingleton::CreateSurfaceSeg(  Surf* surfA, vec2d & uwA0, vec2d & uwA1, Surf* surfB, vec2d & uwB0, vec2d & uwB1   )
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

void SurfaceIntersectionSingleton::WriteISegs()
{
    int i;
    char str2[256];
    sprintf( str2, "ISegs.m" );
    FILE* fp = fopen( str2, "w" );

    fprintf( fp, "clear all;\n" );
    fprintf( fp, "close all;\n" );

    ISeg* seg;

    fprintf( fp, "x=[" );
    for ( i = 0; i < (int)m_AllIPnts.size()-1; i++ )
    {
        seg = m_AllIPnts[i]->m_Segs[0];
        fprintf( fp, "%f %f;\n", seg->m_IPnt[0]->m_Pnt.x(), seg->m_IPnt[1]->m_Pnt.x() );
    }
    seg = m_AllIPnts[i]->m_Segs[0];
    fprintf( fp, "%f %f];\n\n", seg->m_IPnt[0]->m_Pnt.x(), seg->m_IPnt[1]->m_Pnt.x() );

    fprintf( fp, "y=[" );
    for ( i = 0; i < (int)m_AllIPnts.size()-1; i++ )
    {
        seg = m_AllIPnts[i]->m_Segs[0];
        fprintf( fp, "%f %f;\n", seg->m_IPnt[0]->m_Pnt.y(), seg->m_IPnt[1]->m_Pnt.y() );
    }
    seg = m_AllIPnts[i]->m_Segs[0];
    fprintf( fp, "%f %f];\n\n", seg->m_IPnt[0]->m_Pnt.y(), seg->m_IPnt[1]->m_Pnt.y() );

    fprintf( fp, "z=[" );
    for ( i = 0; i < (int)m_AllIPnts.size()-1; i++ )
    {
        seg = m_AllIPnts[i]->m_Segs[0];
        fprintf( fp, "%f %f;\n", seg->m_IPnt[0]->m_Pnt.z(), seg->m_IPnt[1]->m_Pnt.z() );
    }
    seg = m_AllIPnts[i]->m_Segs[0];
    fprintf( fp, "%f %f];\n\n", seg->m_IPnt[0]->m_Pnt.z(), seg->m_IPnt[1]->m_Pnt.z() );

    fprintf( fp, "plot3( x', y', z' );\n" );
    fprintf( fp, "axis equal;\n" );
    fprintf( fp, "axis off;\n" );

    fclose( fp );
}

void SurfaceIntersectionSingleton::BuildChains()
{
    PntNodeCloud i_pnt_cloud;

    for ( size_t i = 0; i < m_AllIPnts.size(); i++ )
    {
        i_pnt_cloud.AddPntNode( m_AllIPnts[i]->m_Pnt );
    }

    PNTree index( 3, i_pnt_cloud, KDTreeSingleIndexAdaptorParams() );
    index.buildIndex();

    for ( int i = 0; i < (int)m_AllIPnts.size(); i++ )
    {
        if ( !m_AllIPnts[i]->m_UsedFlag && m_AllIPnts[i]->m_Segs.size() > 0 )
        {
            ISeg* seg = m_AllIPnts[i]->m_Segs[0];
            seg->m_IPnt[0]->m_UsedFlag = true;
            seg->m_IPnt[1]->m_UsedFlag = true;
            ISegChain* chain = new ISegChain;           // Create New Chain
            chain->m_SurfA = seg->m_SurfA;
            chain->m_SurfB = seg->m_SurfB;
            chain->m_ISegDeque.push_back( seg );
            ExpandChain( chain, &index );
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

    // DebugWriteChains( "BuildChains_Intermediate", false );

    // After the intersection chains are formed, refine them so that the value returned by CompPnt
    // will be the same with respect to each parent surface of the intersection point. Note, this 
    // must be done before the border curves are spit at each intersection chain end point.
    list< ISegChain* >::iterator c;
    for ( c = m_ISegChainList.begin(); c != m_ISegChainList.end(); ++c )
    {
        if ( ( *c )->m_SurfA == ( *c )->m_SurfB )
        {
            // Do not refine SubSurface curves
            continue;
        }

        RefineISegChain( *c );
    }
}

void SurfaceIntersectionSingleton::RefineISegChainSeg( ISegChain* c, IPnt* ipnt )
{
    Puw* auw = ipnt->GetPuw( c->m_SurfA );
    Puw* buw = ipnt->GetPuw( c->m_SurfB );

    double uA;
    double wA;
    double uB;
    double wB;
    double dist, dist1, dist2, dist3;
    int ret, borderA, borderB;

    if ( auw && buw )
    {
        // Initialize variables
        uA = auw->m_UW[0];
        wA = auw->m_UW[1];
        uB = buw->m_UW[0];
        wB = buw->m_UW[1];

        vec3d pA = c->m_SurfA->CompPnt( uA, wA );
        vec3d pB = c->m_SurfB->CompPnt( uB, wB );
        dist1 = ( pA - pB ).mag();

        vec3d pmid = 0.5 * ( pA + pB );
        surface_point_type mid_point;
        mid_point << pmid.x(), pmid.y(), pmid.z();

        // uwtol is a distance in u or v coordinates for a point to be considered on the border of a surface.
        // The points are generated through the intersection algorithm and should be fairly broadly spaced.
        double uwtol = 1e-4;
        borderA = c->m_SurfA->GetSurfCore()->UWPointOnBorder( uA, wA, uwtol );
        borderB = c->m_SurfB->GetSurfCore()->UWPointOnBorder( uB, wB, uwtol );

        if ( borderA == SurfCore::NOBNDY && borderB == SurfCore::NOBNDY )
        {
            ret = eli::geom::intersect::intersect(uA, wA, uB, wB, dist, *( c->m_SurfA->GetSurfCore()->GetSurf() ),
                                                  *( c->m_SurfB->GetSurfCore()->GetSurf() ), mid_point,
                                                  auw->m_UW[0], auw->m_UW[1], buw->m_UW[0], buw->m_UW[1] );
        }
        else
        {
            SurfCore surfA = *( c->m_SurfA->GetSurfCore() );
            SurfCore surfB = *( c->m_SurfB->GetSurfCore() );

            surfA.GetSurf()->translate( -mid_point );
            surfB.GetSurf()->translate( -mid_point );

            if ( borderA == SurfCore::UMIN || borderA == SurfCore::UMAX )
            {
                Bezier_curve crv;
                if ( borderA == SurfCore::UMIN )
                {
                    crv = surfA.GetBorderCurve( SurfCore::UMIN );
                    uA = surfA.GetMinU();
                }
                else
                {
                    crv = surfA.GetBorderCurve( SurfCore::UMAX );
                    uA = surfA.GetMaxU();
                }

                dist = eli::geom::intersect::intersect( uB, wB, wA,
                                                        *( surfB.GetSurf() ),
                                                        crv.GetCurve(),
                                                        buw->m_UW[0], buw->m_UW[1], auw->m_UW[1] );
            }
            else if ( borderA == SurfCore::WMIN || borderA == SurfCore::WMAX )
            {
                Bezier_curve crv;
                if ( borderA == SurfCore::WMIN )
                {
                    crv = surfA.GetBorderCurve( SurfCore::WMIN );
                    wA = surfA.GetMinW();
                }
                else
                {
                    crv = surfA.GetBorderCurve( SurfCore::WMAX );
                    wA = surfA.GetMaxW();
                }

                dist = eli::geom::intersect::intersect( uB, wB, uA,
                                                        *( surfB.GetSurf() ),
                                                        crv.GetCurve(),
                                                        buw->m_UW[0], buw->m_UW[1], auw->m_UW[0] );
            }
            else if ( borderB == SurfCore::UMIN || borderB == SurfCore::UMAX )
            {
                Bezier_curve crv;
                if ( borderB == SurfCore::UMIN )
                {
                    crv = surfB.GetBorderCurve( SurfCore::UMIN );
                    uB = surfB.GetMinU();
                }
                else
                {
                    crv = surfB.GetBorderCurve( SurfCore::UMAX );
                    uB = surfB.GetMaxU();
                }

                dist = eli::geom::intersect::intersect( uA, wA, wB,
                                                        *( surfA.GetSurf() ),
                                                        crv.GetCurve(),
                                                        auw->m_UW[0], auw->m_UW[1], buw->m_UW[1] );
            }
            else if ( borderB == SurfCore::WMIN || borderB == SurfCore::WMAX )
            {
                Bezier_curve crv;
                if ( borderB == SurfCore::WMIN )
                {
                    crv = surfB.GetBorderCurve( SurfCore::WMIN );
                    wB = surfB.GetMinW();
                }
                else
                {
                    crv = surfB.GetBorderCurve( SurfCore::WMAX );
                    wB = surfB.GetMaxW();
                }

                dist = eli::geom::intersect::intersect( uA, wA, uB,
                                                        *( surfA.GetSurf() ),
                                                        crv.GetCurve(),
                                                        auw->m_UW[0], auw->m_UW[1], buw->m_UW[0] );
            }
            else
            {
                printf( "Error, impossible condition\n" );
            }
        }

        pA = c->m_SurfA->CompPnt( uA, wA );
        pB = c->m_SurfB->CompPnt( uB, wB );
        dist2 = ( pA - pB ).mag();
        vec3d pmid2 = 0.5 * ( pA + pB );
        dist3 = ( pmid2 - pmid ).mag();

        bool keepnew = true;
        if ( dist2 > dist1 )
        {
            // printf( "1st Refine ISegChain point failed d1 %f d %f d2 %f d3 %f A: %d B: %d\n", dist1, dist, dist2, dist3, borderA, borderB );
            // printf( "  au %f aw %f bu %f bw %f\n", auw->m_UW[0], auw->m_UW[1], buw->m_UW[0], buw->m_UW[1] );
            // printf( "  %f %f %f\n\n", pmid.x(), pmid.y(), pmid.z() );
            keepnew = false;
        }

        // Check if point has moved a significant distance.
        // if ( dist3 > 1e-3 )
        {
            // printf( "2nd Refine ISegChain point failed d1 %f d %f d2 %f d3 %f A: %d B: %d\n", dist1, dist, dist2, dist3, borderA, borderB );
            // printf( "  au %f aw %f bu %f bw %f\n", auw->m_UW[0], auw->m_UW[1], buw->m_UW[0], buw->m_UW[1] );
            // printf( "  %f %f %f\n\n", pmid.x(), pmid.y(), pmid.z() );
            // keepnew = false;
        }

        if ( keepnew )
        {
            auw->m_UW[0] = uA;
            auw->m_UW[1] = wA;
            buw->m_UW[0] = uB;
            buw->m_UW[1] = wB;
        }
    }

}

void SurfaceIntersectionSingleton::RefineISegChain( ISegChain* c )
{

    RefineISegChainSeg( c, c->m_ISegDeque[0]->m_IPnt[0] );

    for ( int i = 0; i < (int)c->m_ISegDeque.size(); i++ )
    {
        RefineISegChainSeg( c, c->m_ISegDeque[i]->m_IPnt[1] );
    }
}

void SurfaceIntersectionSingleton::ExpandChain( ISegChain* chain, PNTree* PN_tree )
{
    bool stillExpanding = true;
    bool expandFront = true;
    bool firstIter = true;
    // A minimum of 2 results is needed, since the the first result will be a point at the 
    // query location but already used by the adjacent segment. Additional results might not
    // be needed but are added just to be safe. The effect on speed should be negligible. 
    const size_t num_results = 4;

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

        IPnt* matchIPnt = NULL;

        const double query_pt[3] = { testIPnt->m_Pnt.x(), testIPnt->m_Pnt.y(), testIPnt->m_Pnt.z() };
        size_t ret_index[num_results];
        double out_dist_sqr[num_results];
        nanoflann::KNNResultSet < double > resultSet( num_results );
        resultSet.init( ret_index, out_dist_sqr );
        PN_tree->findNeighbors( resultSet, query_pt, nanoflann::SearchParams() );

        for ( size_t i = 0; i < resultSet.size(); ++i )
        {
            if ( m_AllIPnts[ret_index[i]]->m_Puws.size() != 2 || m_AllIPnts[ret_index[i]]->m_UsedFlag )
            {
                continue;
            }

            if ( m_AllIPnts[ret_index[i]]->m_Puws[0]->m_Surf == testIPnt->m_Puws[0]->m_Surf &&
                 m_AllIPnts[ret_index[i]]->m_Puws[1]->m_Surf == testIPnt->m_Puws[1]->m_Surf )
            {
                matchIPnt = m_AllIPnts[ret_index[i]];
                break;
            }
        }

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
            if ( firstIter && expandFront && ( dist( chain->m_ISegDeque.front()->m_IPnt[0]->m_Pnt, matchIPnt->m_Pnt ) > dist( chain->m_ISegDeque.back()->m_IPnt[1]->m_Pnt, matchIPnt->m_Pnt ) ) )
            {
                // This segment's orientation needs to be reversed because expandFront was set true on the first iteration,
                // but the back point is closer. 
                IPnt* temp_pnt = chain->m_ISegDeque.front()->m_IPnt[0];
                chain->m_ISegDeque.front()->m_IPnt[0] = chain->m_ISegDeque.front()->m_IPnt[1];
                chain->m_ISegDeque.front()->m_IPnt[1] = temp_pnt;
            }

            firstIter = false;

            ISeg* seg = matchIPnt->m_Segs[0];
            chain->AddSeg( seg, expandFront );
            seg->m_IPnt[0]->m_UsedFlag = true;
            seg->m_IPnt[1]->m_UsedFlag = true;
        }
    }
}

void SurfaceIntersectionSingleton::WriteChains()
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
        for ( c = m_ISegChainList.begin() ; c != m_ISegChainList.end(); ++c )
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

void SurfaceIntersectionSingleton::LoadBorderCurves()
{

    //==== Tesselate Border Curves ====//
    for ( int i = 0 ; i < ( int )m_ICurveVec.size() ; i++ )
    {
        if ( !m_ICurveVec[i]->m_SCurve_B )                              // Non Closed Solid
        {
            m_ICurveVec[i]->m_SCurve_B = m_ICurveVec[i]->m_SCurve_A;
        }

        //==== Create New Chain ====//
        ISegChain* chain = new ISegChain;

        Surf* surfA = m_ICurveVec[i]->m_SCurve_A->GetSurf();
        Surf* surfB = m_ICurveVec[i]->m_SCurve_B->GetSurf();

        if ( m_ICurveVec[i]->m_PlaneBorderIntersectFlag )
        {
            if ( !surfA->GetSymPlaneFlag() )
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

        chain->m_BorderFlag = true;

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
    fprintf( m_DebugFile, "   Total Num Chains = %zu \n", m_ISegChainList.size() );

    list< ISegChain* >::iterator c;
    for ( c = m_ISegChainList.begin() ; c != m_ISegChainList.end(); ++c )
    {
        fprintf( m_DebugFile, "   Chain Num Segs = %d Border %d \n",
                 ( *c )->m_ISegDeque.size(), ( int )( ( *c )->m_BorderFlag ) );
    }
#endif

}

void SurfaceIntersectionSingleton::BuildSubSurfIntChains()
{
    // Adds subsurface intersection chains
    vec2d uw_pnt0;
    vec2d uw_pnt1;
    const unsigned int num_sects = 100; // Number of segments to break subsurface segments up into

    // If there is an issue with having a watertight mesh between the intersection of two
    // components near a forced subsurface line, try increasing num_sects especially for highly
    // curved surfaces

    for ( int s = 0 ; s < ( int )m_SurfVec.size() ; s++ )
    {
        Surf* surf = m_SurfVec[s];

        // Get all SubSurfaces for the specified geom
        vector < SimpleSubSurface > ss_vec = GetSimpSubSurfs( surf->GetGeomID(), surf->GetMainSurfID(), surf->GetCompID() );

        // Split SubSurfs
        for ( int ss = 0 ; ss < ( int ) ss_vec.size(); ss++ )
        {
            ss_vec[ss].SplitSegsU( surf->GetSurfCore()->GetMinU() );
            ss_vec[ss].SplitSegsU( surf->GetSurfCore()->GetMaxU() );
            ss_vec[ss].SplitSegsW( surf->GetSurfCore()->GetMinW() );
            ss_vec[ss].SplitSegsW( surf->GetSurfCore()->GetMaxW() );

            vector < vector< SSLineSeg > >& segsvec = ss_vec[ss].GetSplitSegs();

            for ( int i = 0; i < segsvec.size(); i++ )
            {
                vector< SSLineSeg >& segs = segsvec[i];

                ISegChain* chain = NULL;

                bool new_chain = true;
                bool is_poly = ss_vec[ss].GetPolyFlag();

                // Build Intersection Chains
                for ( int ls = 0; ls < ( int )segs.size(); ls++ )
                {
                    if ( new_chain && chain )
                    {
                        if ( chain->Valid() )
                        {
                            if ( ss_vec[ss].m_IncludedElements != vsp::FEA_SHELL )
                            {
                                chain->m_SSIntersectIndex = ss; // Identify FeaSubSurfaceIndex
                            }

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

                    if ( uw_pnt0[0] < min_u - FLT_EPSILON || uw_pnt0[1] < min_w - FLT_EPSILON || uw_pnt1[0] < min_u - FLT_EPSILON || uw_pnt1[1] < min_w - FLT_EPSILON )
                    {
                        new_chain = true;
                        continue; // Skip if either point has a value not on this surface
                    }
                    if ( uw_pnt0[0] > max_u + FLT_EPSILON || uw_pnt0[1] > max_w + FLT_EPSILON || uw_pnt1[0] > max_u + FLT_EPSILON || uw_pnt1[1] > max_w + FLT_EPSILON )
                    {
                        new_chain = true;
                        continue; // Skip if either point has a value not on this surface
                    }
                    if ( ((std::abs( uw_pnt0[0]-max_u ) < tol && std::abs( uw_pnt1[0]-max_u ) < tol) ||
                            (std::abs( uw_pnt0[1]-max_w ) < tol && std::abs( uw_pnt1[1]-max_w ) < tol) ||
                            (std::abs( uw_pnt0[0]-min_u ) < tol && std::abs( uw_pnt1[0]-min_u ) < tol) ||
                            (std::abs( uw_pnt0[1]-min_w ) < tol && std::abs( uw_pnt1[1]-min_w ) < tol))
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
                        if ( ss_vec[ss].m_IncludedElements != vsp::FEA_SHELL )
                        {
                            chain->m_SSIntersectIndex = ss; // Identify FeaSubSurfaceIndex
                        }

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
}

void SurfaceIntersectionSingleton::SplitBorderCurves()
{
    vector< IPnt* > splitPnts;
    list< ISegChain* >::iterator c;
    for ( c = m_ISegChainList.begin() ; c != m_ISegChainList.end(); ++c )
    {
        if  ( !( *c )->m_BorderFlag ) // Non Border Chains
        {
            splitPnts.push_back( ( *c )->m_ISegDeque.front()->m_IPnt[0] );
            splitPnts.push_back( ( *c )->m_ISegDeque.back()->m_IPnt[1]  );
        }
    }

    for ( c = m_ISegChainList.begin() ; c != m_ISegChainList.end(); ++c )
    {
        if ( ( *c )->m_BorderFlag )
        {
            for ( int i = 0 ; i < ( int )splitPnts.size() ; i++ )
            {
                Puw* uwA = splitPnts[i]->GetPuw( ( *c )->m_SurfA );
                Puw* uwB = splitPnts[i]->GetPuw( ( *c )->m_SurfB );
                if ( uwA )
                {
                    ( *c )->AddBorderSplit( uwA );
                }
                else if ( uwB )
                {
                    ( *c )->AddBorderSplit( uwB );
                }
            }
        }
    }

    //==== Load Only Border Chains ====//
    vector< ISegChain* > chains;
    for ( c = m_ISegChainList.begin() ; c != m_ISegChainList.end(); ++c )
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
        vector< ISegChain* > new_chains = chains[i]->SortAndSplit( this );
        for ( int j = 0 ; j < ( int )new_chains.size() ; j++ )
        {
            new_chains[j]->m_BorderFlag = true;
            if ( new_chains[j]->Valid() )
            {
                new_chains[j]->m_SSIntersectIndex = chains[i]->m_SSIntersectIndex; // Track SubSurface Index
                m_ISegChainList.push_back( new_chains[j] );
            }
        }
    }

    //==== Load Only Border Chains ====//
    chains.clear();
    for ( c = m_ISegChainList.begin() ; c != m_ISegChainList.end(); ++c )
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
    for ( c = m_ISegChainList.begin() ; c != m_ISegChainList.end(); ++c )
    {
        vector< vec3d > crv;
        for ( int i = 0 ; i < ( int )( *c )->m_ISegDeque.size() ; i++ )
        {
            IPnt* ip = ( *c )->m_ISegDeque[i]->m_IPnt[0];
            crv.push_back( ip->m_Pnt );

            if ( i == ( int )( *c )->m_ISegDeque.size() - 1 )
            {
                ip = ( *c )->m_ISegDeque[i]->m_IPnt[1];
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

void SurfaceIntersectionSingleton::IntersectSplitChains()
{
    //==== Intersect Intersection Curves (Not Border Curves) ====//
    list< ISegChain* >::iterator c;
    vector< ISegChain* > chains;
    for ( c = m_ISegChainList.begin() ; c != m_ISegChainList.end(); ++c )
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
        vector< ISegChain* > new_chains = chains[i]->SortAndSplit( this );
        for ( int j = 0 ; j < ( int )new_chains.size() ; j++ )
        {
            if ( new_chains[j]->Valid() )
            {
                new_chains[j]->m_SSIntersectIndex = chains[i]->m_SSIntersectIndex; // Track SubSurface Index
                m_ISegChainList.push_back( new_chains[j] );
            }

        }
    }
}

void SurfaceIntersectionSingleton::MergeInteriorChainIPnts()
{
    //==== Merge Interior IPnts in Chains ====//
    list< ISegChain* >::iterator c;
    for ( c = m_ISegChainList.begin() ; c != m_ISegChainList.end(); ++c )
    {
        ( *c )->MergeInteriorIPnts();
    }
}



void SurfaceIntersectionSingleton::HighlightNextChain()
{
    m_HighlightChainIndex++;
    if ( m_HighlightChainIndex >= ( int )m_ISegChainList.size() )
    {
        m_HighlightChainIndex = 0;
    }
}

void SurfaceIntersectionSingleton::DebugWriteChains( const char* name, bool tessFlag )
{
    if ( true )
    {
        char str2[256];
        sprintf( str2, "%s.m", name );
        FILE* fpmas = fopen( str2, "w" );

        // Scale points to shift precision.
        double k = 1.0;

        // Shift points to center on point of interest, also helps precision.
        double xc = 13.526978;
        double yc = 16.2362945;
        double zc = 0.44215;


        fprintf( fpmas, "clear all; format compact; close all;\n" );
        fprintf( fpmas, "figure(1); hold on\n" );
        fprintf( fpmas, "figure(2); hold on\n" );
        fprintf( fpmas, "figure(3); hold on\n" );

        // Radius applied in Matlab to limit extent of points plotted.
        fprintf( fpmas, "r0 = 0.01;\n" );

        for ( int i = 0 ; i < ( int )m_SurfVec.size() ; i++ )
        {
            char str[256];
            sprintf( str, "%s%d.m", name, i );
            FILE* fp = fopen( str, "w" );

            fprintf( fpmas, "run( '%s' );\n", str );

            int cnt = 0;
            list< ISegChain* >::iterator c;
            for ( c = m_ISegChainList.begin() ; c != m_ISegChainList.end(); ++c )
            {
                if ( m_SurfVec[i] == ( *c )->m_SurfA || m_SurfVec[i] == ( *c )->m_SurfB )
                {

                    if ( tessFlag == false )
                    {
                        fprintf( fp, "u=[" );
                        int j;
                        vec2d uw1, uw2;
                        for ( j = 0 ; j < ( int )( *c )->m_ISegDeque.size() - 1 ; j++ )
                        {
                            uw1 = ( *c )->m_ISegDeque[j]->m_IPnt[0]->GetPuw( m_SurfVec[i] )->m_UW;
                            fprintf( fp, "%f,", uw1[0] );
                        }
                        uw1 = ( *c )->m_ISegDeque[j]->m_IPnt[0]->GetPuw( m_SurfVec[i] )->m_UW;
                        uw2 = ( *c )->m_ISegDeque[j]->m_IPnt[1]->GetPuw( m_SurfVec[i] )->m_UW;
                        fprintf( fp, "%f, %f];\n", uw1[0], uw2[0] );

                        fprintf( fp, "w=[" );
                        for ( j = 0 ; j < ( int )( *c )->m_ISegDeque.size() - 1 ; j++ )
                        {
                            uw1 = ( *c )->m_ISegDeque[j]->m_IPnt[0]->GetPuw( m_SurfVec[i] )->m_UW;
                            fprintf( fp, "%f,", uw1[1] );
                        }
                        uw1 = ( *c )->m_ISegDeque[j]->m_IPnt[0]->GetPuw( m_SurfVec[i] )->m_UW;
                        uw2 = ( *c )->m_ISegDeque[j]->m_IPnt[1]->GetPuw( m_SurfVec[i] )->m_UW;
                        fprintf( fp, "%f, %f];\n", uw1[1], uw2[1] );

                        fprintf( fp, "figure(1)\n" );
                        fprintf( fp, "plot( u, w, 'x-'); hold on;\n" );
                        fprintf( fp, "text( u(round(end/2)), w(round(end/2)), 'Surf: %d Chain: %d' );\n", i, cnt );

                        fprintf( fp, "x=[" );
                        vec3d pt, pt2;
                        for ( j = 0 ; j < ( int )( *c )->m_ISegDeque.size() - 1 ; j++ )
                        {
                            uw1 = ( *c )->m_ISegDeque[j]->m_IPnt[0]->GetPuw( m_SurfVec[i] )->m_UW;
                            pt = ( *c )->m_ISegDeque[j]->m_IPnt[0]->GetPuw( m_SurfVec[i] )->m_Surf->CompPnt( uw1[0], uw1[1] );
                            fprintf( fp, "%.19e,", ( pt.x() - xc ) * k );
                        }
                        uw1 = ( *c )->m_ISegDeque[j]->m_IPnt[0]->GetPuw( m_SurfVec[i] )->m_UW;
                        pt = ( *c )->m_ISegDeque[j]->m_IPnt[0]->GetPuw( m_SurfVec[i] )->m_Surf->CompPnt( uw1[0], uw1[1] );
                        uw2 = ( *c )->m_ISegDeque[j]->m_IPnt[1]->GetPuw( m_SurfVec[i] )->m_UW;
                        pt2 = ( *c )->m_ISegDeque[j]->m_IPnt[1]->GetPuw( m_SurfVec[i] )->m_Surf->CompPnt( uw2[0], uw2[1] );
                        fprintf( fp, "%.19e %.19e];\n", ( pt.x() - xc ) * k, ( pt2.x() - xc ) * k );

                        fprintf( fp, "y=[" );
                        for ( j = 0 ; j < ( int )( *c )->m_ISegDeque.size() - 1 ; j++ )
                        {
                            uw1 = ( *c )->m_ISegDeque[j]->m_IPnt[0]->GetPuw( m_SurfVec[i] )->m_UW;
                            pt = ( *c )->m_ISegDeque[j]->m_IPnt[0]->GetPuw( m_SurfVec[i] )->m_Surf->CompPnt( uw1[0], uw1[1] );
                            fprintf( fp, "%.19e,", ( pt.y() - yc ) * k );
                        }
                        uw1 = ( *c )->m_ISegDeque[j]->m_IPnt[0]->GetPuw( m_SurfVec[i] )->m_UW;
                        pt = ( *c )->m_ISegDeque[j]->m_IPnt[0]->GetPuw( m_SurfVec[i] )->m_Surf->CompPnt( uw1[0], uw1[1] );
                        uw2 = ( *c )->m_ISegDeque[j]->m_IPnt[1]->GetPuw( m_SurfVec[i] )->m_UW;
                        pt2 = ( *c )->m_ISegDeque[j]->m_IPnt[1]->GetPuw( m_SurfVec[i] )->m_Surf->CompPnt( uw2[0], uw2[1] );
                        fprintf( fp, "%.19e %.19e];\n", ( pt.y() - yc ) * k, ( pt2.y() - yc ) * k );

                        fprintf( fp, "z=[" );
                        for ( j = 0 ; j < ( int )( *c )->m_ISegDeque.size() - 1 ; j++ )
                        {
                            uw1 = ( *c )->m_ISegDeque[j]->m_IPnt[0]->GetPuw( m_SurfVec[i] )->m_UW;
                            pt = ( *c )->m_ISegDeque[j]->m_IPnt[0]->GetPuw( m_SurfVec[i] )->m_Surf->CompPnt( uw1[0], uw1[1] );
                            fprintf( fp, "%.19e,", ( pt.z() - zc ) * k );
                        }
                        uw1 = ( *c )->m_ISegDeque[j]->m_IPnt[0]->GetPuw( m_SurfVec[i] )->m_UW;
                        pt = ( *c )->m_ISegDeque[j]->m_IPnt[0]->GetPuw( m_SurfVec[i] )->m_Surf->CompPnt( uw1[0], uw1[1] );
                        uw2 = ( *c )->m_ISegDeque[j]->m_IPnt[1]->GetPuw( m_SurfVec[i] )->m_UW;
                        pt2 = ( *c )->m_ISegDeque[j]->m_IPnt[1]->GetPuw( m_SurfVec[i] )->m_Surf->CompPnt( uw2[0], uw2[1] );
                        fprintf( fp, "%.19e %.19e];\n", ( pt.z() - zc ) * k, ( pt2.z() - zc ) * k );

                        fprintf( fp, "figure(2)\n" );
                        fprintf( fp, "plot3( x, y, z, 'x-' ); hold on;\n" );
                        fprintf( fp, "text( x(round(end/2)), y(round(end/2)), z(round(end/2)), 'Surf: %d Chain: %d' );\n", i, cnt );

                        fprintf( fp, "r = sqrt( x.^2 + y.^2 + z.^2 );\n" );
                        fprintf( fp, "mask = r < ( r0 * %f );\n", k );
                        // Expand mask by one point in each direction to provide context -- also makes sure single points get plotted.
                        fprintf( fp, "mask([false mask(1:end-1)~=mask(2:end)]) = 1;\n" );
                        fprintf( fp, "mask([mask(1:end-1)~=mask(2:end) false]) = 1;\n" );
                        fprintf( fp, "figure(3)\n" );
                        fprintf( fp, "plot3( x(mask), y(mask), z(mask), 'x-' ); hold on;\n" );
                    }
                    else
                    {
                        fprintf( fp, "u=[" );
                        int j;
                        vec2d uw1;
                        for ( j = 0 ; j < ( int )( *c )->m_TessVec.size() - 1 ; j++ )
                        {
                            uw1 = ( *c )->m_TessVec[j]->GetPuw( m_SurfVec[i] )->m_UW;
                            fprintf( fp, "%f,", uw1[0] );
                        }
                        uw1 = ( *c )->m_TessVec[j]->GetPuw( m_SurfVec[i] )->m_UW;
                        fprintf( fp, "%f];\n", uw1[0] );

                        fprintf( fp, "w=[" );
                        for ( j = 0 ; j < ( int )( *c )->m_TessVec.size() - 1 ; j++ )
                        {
                            uw1 = ( *c )->m_TessVec[j]->GetPuw( m_SurfVec[i] )->m_UW;
                            fprintf( fp, "%f,", uw1[1] );
                        }
                        uw1 = ( *c )->m_TessVec[j]->GetPuw( m_SurfVec[i] )->m_UW;
                        fprintf( fp, "%f];\n", uw1[1] );

                        fprintf( fp, "figure(1)\n");
                        fprintf( fp, "plot( u, w, 'x-'); hold on;\n" );


                        fprintf( fp, "x=[" );
                        vec3d pt;
                        for ( j = 0 ; j < ( int )( *c )->m_TessVec.size() - 1 ; j++ )
                        {
                            uw1 = ( *c )->m_TessVec[j]->GetPuw( m_SurfVec[i] )->m_UW;
                            pt = ( *c )->m_TessVec[j]->GetPuw( m_SurfVec[i] )->m_Surf->CompPnt( uw1[0], uw1[1] );
                            fprintf( fp, "%f,", pt.x() );
                        }
                        uw1 = ( *c )->m_TessVec[j]->GetPuw( m_SurfVec[i] )->m_UW;
                        pt = ( *c )->m_TessVec[j]->GetPuw( m_SurfVec[i] )->m_Surf->CompPnt( uw1[0], uw1[1] );
                        fprintf( fp, "%f];\n", pt.x() );

                        fprintf( fp, "y=[" );
                        for ( j = 0 ; j < ( int )( *c )->m_TessVec.size() - 1 ; j++ )
                        {
                            uw1 = ( *c )->m_TessVec[j]->GetPuw( m_SurfVec[i] )->m_UW;
                            pt = ( *c )->m_TessVec[j]->GetPuw( m_SurfVec[i] )->m_Surf->CompPnt( uw1[0], uw1[1] );
                            fprintf( fp, "%f,", pt.y() );
                        }
                        uw1 = ( *c )->m_TessVec[j]->GetPuw( m_SurfVec[i] )->m_UW;
                        pt = ( *c )->m_TessVec[j]->GetPuw( m_SurfVec[i] )->m_Surf->CompPnt( uw1[0], uw1[1] );
                        fprintf( fp, "%f];\n", pt.y() );

                        fprintf( fp, "z=[" );
                        for ( j = 0 ; j < ( int )( *c )->m_TessVec.size() - 1 ; j++ )
                        {
                            uw1 = ( *c )->m_TessVec[j]->GetPuw( m_SurfVec[i] )->m_UW;
                            pt = ( *c )->m_TessVec[j]->GetPuw( m_SurfVec[i] )->m_Surf->CompPnt( uw1[0], uw1[1] );
                            fprintf( fp, "%f,", pt.z() );
                        }
                        uw1 = ( *c )->m_TessVec[j]->GetPuw( m_SurfVec[i] )->m_UW;
                        pt = ( *c )->m_TessVec[j]->GetPuw( m_SurfVec[i] )->m_Surf->CompPnt( uw1[0], uw1[1] );
                        fprintf( fp, "%f];\n", pt.z() );

                        fprintf( fp, "figure(2)\n");
                        fprintf( fp, "plot3( x, y, z, 'x-'); hold on;\n" );
                    }
                    cnt++;
                }
            }
            fclose( fp );
        }

        fprintf( fpmas, "figure(1)\n");
        fprintf( fpmas, "axis off\n" );
        fprintf( fpmas, "axis equal\n" );
        fprintf( fpmas, "hold off\n" );

        fprintf( fpmas, "figure(2)\n");
        fprintf( fpmas, "axis off\n" );
        fprintf( fpmas, "axis equal\n" );
        fprintf( fpmas, "hold off\n" );

        fprintf( fpmas, "figure(3)\n");
        fprintf( fpmas, "axis off\n" );
        fprintf( fpmas, "axis equal\n" );
        fprintf( fpmas, "hold off\n" );

        fclose( fpmas );
    }
    else
    {
        for ( int i = 0 ; i < ( int )m_SurfVec.size() ; i++ )
        {
            char str[256];
            sprintf( str, "%s%d.dat", name, i );
            FILE* fp = fopen( str, "w" );

            int cnt = 0;
            list< ISegChain* >::iterator c;
            for ( c = m_ISegChainList.begin() ; c != m_ISegChainList.end(); ++c )
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


                    if ( ! tessFlag )
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
}

void SurfaceIntersectionSingleton::AddPossCoPlanarSurf( Surf* surfA, Surf* surfB )
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

vector< Surf* > SurfaceIntersectionSingleton::GetPossCoPlanarSurfs( Surf* surfPtr )
{
    if ( m_PossCoPlanarSurfMap.find( surfPtr ) != m_PossCoPlanarSurfMap.end() )
    {
        return m_PossCoPlanarSurfMap[surfPtr];
    }

    vector< Surf* > retSurfVec;
    return retSurfVec;
}

void SurfaceIntersectionSingleton::TestStuff()
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

void SurfaceIntersectionSingleton::BinaryAdaptIntCurves()
{
    m_BinAdaptCurveAVec.clear();
    m_BinAdaptCurveBVec.clear();
    m_RawCurveAVec.clear();
    m_RawCurveBVec.clear();
    m_BorderCurveFlagVec.clear();

    list<ISegChain *>::iterator c;
    for ( c = m_ISegChainList.begin(); c != m_ISegChainList.end(); ++c )
    {
        m_BorderCurveFlagVec.push_back( (*c)->m_BorderFlag );

        vector<vec3d> ptvec;
        vector<vec3d> rawptvec;

        Bezier_curve xyzcrvA = (*c)->m_ACurve.GetUWCrv();
        xyzcrvA.TessAdaptXYZ( *(*c)->m_ACurve.GetSurf(), ptvec, GetSettingsPtr()->m_RelCurveTol, 16 );

        m_BinAdaptCurveAVec.push_back( ptvec );

        xyzcrvA.UWCurveToXYZCurve( (*c)->m_ACurve.GetSurf() );
        xyzcrvA.GetControlPoints( rawptvec );

        m_RawCurveAVec.push_back( rawptvec );


        ptvec.clear();
        rawptvec.clear();
        Bezier_curve xyzcrvB = (*c)->m_BCurve.GetUWCrv();
        xyzcrvB.TessAdaptXYZ( *(*c)->m_BCurve.GetSurf(), ptvec, GetSettingsPtr()->m_RelCurveTol, 16 );

        m_BinAdaptCurveBVec.push_back( ptvec );

        xyzcrvB.UWCurveToXYZCurve( (*c)->m_BCurve.GetSurf() );
        xyzcrvB.GetControlPoints( rawptvec );

        m_RawCurveBVec.push_back( rawptvec );

    }
}

void SurfaceIntersectionSingleton::LoadDrawObjs( vector< DrawObj* > &draw_obj_vec )
{
    if ( m_MeshInProgress )
    {
        return;
    }

    // Draw ISegChains
    m_IsectCurveDO.m_GeomID = GetID() + "ISECTCURVE";
    m_IsectCurveDO.m_Type = DrawObj::VSP_LINES;
    m_IsectCurveDO.m_Visible = GetSettingsPtr()->m_DrawIsectFlag &&
                               GetSettingsPtr()->m_DrawCurveFlag &&
                               GetSettingsPtr()->m_DrawBinAdaptFlag;
    m_IsectCurveDO.m_LineColor = vec3d(0, 0, 1);
    m_IsectCurveDO.m_LineWidth = 2.0;

    m_IsectPtsDO.m_GeomID = GetID() + "ISECTPTS";
    m_IsectPtsDO.m_Type = DrawObj::VSP_POINTS;
    m_IsectPtsDO.m_Visible = GetSettingsPtr()->m_DrawIsectFlag &&
                               GetSettingsPtr()->m_DrawPntsFlag &&
                               GetSettingsPtr()->m_DrawBinAdaptFlag;
    m_IsectPtsDO.m_PointColor = vec3d(0, 0, 0);
    m_IsectPtsDO.m_PointSize = 10.0;

    m_BorderCurveDO.m_GeomID = GetID() + "BORDERCURVE";
    m_BorderCurveDO.m_Type = DrawObj::VSP_LINES;
    m_BorderCurveDO.m_Visible = GetSettingsPtr()->m_DrawBorderFlag &&
                               GetSettingsPtr()->m_DrawCurveFlag &&
                               GetSettingsPtr()->m_DrawBinAdaptFlag;
    m_BorderCurveDO.m_LineColor = vec3d(0, 1, 0);
    m_BorderCurveDO.m_LineWidth = 2.0;

    m_BorderPtsDO.m_GeomID = GetID() + "BORDERPTS";
    m_BorderPtsDO.m_Type = DrawObj::VSP_POINTS;
    m_BorderPtsDO.m_Visible = GetSettingsPtr()->m_DrawBorderFlag &&
                               GetSettingsPtr()->m_DrawPntsFlag &&
                               GetSettingsPtr()->m_DrawBinAdaptFlag;
    m_BorderPtsDO.m_PointColor = vec3d(0, 0, 0);
    m_BorderPtsDO.m_PointSize = 10.0;

    m_RawIsectCurveDO.m_GeomID = GetID() + "RAWISECTCURVE";
    m_RawIsectCurveDO.m_Type = DrawObj::VSP_LINES;
    m_RawIsectCurveDO.m_Visible = GetSettingsPtr()->m_DrawIsectFlag &&
                                  GetSettingsPtr()->m_DrawCurveFlag &&
                                  GetSettingsPtr()->m_DrawRawFlag;
    m_RawIsectCurveDO.m_LineColor = vec3d(1, 0, 1);
    m_RawIsectCurveDO.m_LineWidth = 2.0;

    m_RawIsectPtsDO.m_GeomID = GetID() + "RAWISECTPTS";
    m_RawIsectPtsDO.m_Type = DrawObj::VSP_POINTS;
    m_RawIsectPtsDO.m_Visible = GetSettingsPtr()->m_DrawIsectFlag &&
                                GetSettingsPtr()->m_DrawPntsFlag &&
                                GetSettingsPtr()->m_DrawRawFlag;
    m_RawIsectPtsDO.m_PointColor = vec3d(0.5, 0.5, 0.5);
    m_RawIsectPtsDO.m_PointSize = 10.0;

    m_RawBorderCurveDO.m_GeomID = GetID() + "RAWBORDERCURVE";
    m_RawBorderCurveDO.m_Type = DrawObj::VSP_LINES;
    m_RawBorderCurveDO.m_Visible = GetSettingsPtr()->m_DrawBorderFlag &&
                                   GetSettingsPtr()->m_DrawCurveFlag &&
                                   GetSettingsPtr()->m_DrawRawFlag;
    m_RawBorderCurveDO.m_LineColor = vec3d(1, 1, 0);
    m_RawBorderCurveDO.m_LineWidth = 2.0;

    m_RawBorderPtsDO.m_GeomID = GetID() + "RAWBORDERPTS";
    m_RawBorderPtsDO.m_Type = DrawObj::VSP_POINTS;
    m_RawBorderPtsDO.m_Visible = GetSettingsPtr()->m_DrawBorderFlag &&
                                 GetSettingsPtr()->m_DrawPntsFlag &&
                                 GetSettingsPtr()->m_DrawRawFlag;
    m_RawBorderPtsDO.m_PointColor = vec3d(0.5, 0.5, 0.5);
    m_RawBorderPtsDO.m_PointSize = 10.0;

    m_IsectCurveDO.m_PntVec.clear();
    m_IsectPtsDO.m_PntVec.clear();
    m_BorderCurveDO.m_PntVec.clear();
    m_BorderPtsDO.m_PntVec.clear();
    m_RawIsectCurveDO.m_PntVec.clear();
    m_RawIsectPtsDO.m_PntVec.clear();
    m_RawBorderCurveDO.m_PntVec.clear();
    m_RawBorderPtsDO.m_PntVec.clear();

    for ( int indx = 0; indx < m_RawCurveAVec.size(); indx++ )
    {
        DrawObj *curveDO;
        DrawObj *ptsDO;
        DrawObj *rawcurveDO;
        DrawObj *rawptsDO;

        if ( m_BorderCurveFlagVec[indx] )
        {
            curveDO = &m_BorderCurveDO;
            ptsDO = &m_BorderPtsDO;
            rawcurveDO = & m_RawBorderCurveDO;
            rawptsDO = &m_RawBorderPtsDO;
        }
        else
        {
            curveDO = &m_IsectCurveDO;
            ptsDO = &m_IsectPtsDO;
            rawcurveDO = &m_RawIsectCurveDO;
            rawptsDO = & m_RawIsectPtsDO;
        }

        {
            vector<vec3d> ptvec;
            vector<vec3d> rawptvec;

            ptvec = m_BinAdaptCurveAVec[ indx ];

            rawptvec = m_RawCurveAVec[ indx ];

            ptsDO->m_PntVec.insert( ptsDO->m_PntVec.end(), ptvec.begin(), ptvec.end() );
            rawptsDO->m_PntVec.insert( rawptsDO->m_PntVec.end(), rawptvec.begin(), rawptvec.end() );

            for ( int j = 1; j < ptvec.size(); j++ )
            {
                curveDO->m_PntVec.push_back( ptvec[j - 1] );
                curveDO->m_PntVec.push_back( ptvec[j] );
            }

            for ( int j = 1; j < rawptvec.size(); j++ )
            {
                rawcurveDO->m_PntVec.push_back( rawptvec[j - 1] );
                rawcurveDO->m_PntVec.push_back( rawptvec[j] );
            }


            ptvec.clear();
            rawptvec.clear();

            ptvec = m_BinAdaptCurveBVec[ indx ];

            rawptvec = m_RawCurveBVec[ indx ];

            ptsDO->m_PntVec.insert( ptsDO->m_PntVec.end(), ptvec.begin(), ptvec.end() );
            rawptsDO->m_PntVec.insert( rawptsDO->m_PntVec.end(), rawptvec.begin(), rawptvec.end() );

            for ( int j = 1; j < ptvec.size(); j++ )
            {
                curveDO->m_PntVec.push_back( ptvec[j - 1] );
                curveDO->m_PntVec.push_back( ptvec[j] );
            }

            for ( int j = 1; j < rawptvec.size(); j++ )
            {
                rawcurveDO->m_PntVec.push_back( rawptvec[j - 1] );
                rawcurveDO->m_PntVec.push_back( rawptvec[j] );
            }
        }
    }

    // Normal Vec is not required, load placeholder.
    m_IsectCurveDO.m_NormVec = m_IsectCurveDO.m_PntVec;
    m_IsectPtsDO.m_NormVec = m_IsectPtsDO.m_PntVec;
    m_BorderCurveDO.m_NormVec = m_BorderCurveDO.m_PntVec;
    m_BorderPtsDO.m_NormVec = m_BorderPtsDO.m_PntVec;

    m_RawIsectCurveDO.m_NormVec = m_RawIsectCurveDO.m_PntVec;
    m_RawIsectPtsDO.m_NormVec = m_RawIsectPtsDO.m_PntVec;
    m_RawBorderCurveDO.m_NormVec = m_RawBorderCurveDO.m_PntVec;
    m_RawBorderPtsDO.m_NormVec = m_RawBorderPtsDO.m_PntVec;

    draw_obj_vec.push_back( &m_IsectCurveDO );
    draw_obj_vec.push_back( &m_IsectPtsDO );
    draw_obj_vec.push_back( &m_BorderCurveDO );
    draw_obj_vec.push_back( &m_BorderPtsDO );

    draw_obj_vec.push_back( &m_RawIsectCurveDO );
    draw_obj_vec.push_back( &m_RawIsectPtsDO );
    draw_obj_vec.push_back( &m_RawBorderCurveDO );
    draw_obj_vec.push_back( &m_RawBorderPtsDO );

    //=====  Visualizatino tools for SurfaceINtersectionMgr debugging =====//
    if ( false ) // Set to true to turn visualization tools ON
    {
        // Set each DrawObj "m_Visible" to true to turn ON
        m_ApproxPlanesDO.m_PntVec.clear();

        list<ISegChain *>::iterator c;
        for ( c = m_ISegChainList.begin(); c != m_ISegChainList.end(); ++c )
        {
            if ( !( *c )->m_BorderFlag )
            {
                ( *c )->m_ISegBoxA.AppendLineSegs( m_ApproxPlanesDO.m_PntVec );
                ( *c )->m_ISegBoxB.AppendLineSegs( m_ApproxPlanesDO.m_PntVec );
            }
        }

        m_ApproxPlanesDO.m_GeomID = GetID() + "APPROXPLANES";
        m_ApproxPlanesDO.m_Type = DrawObj::VSP_LINES;
        m_ApproxPlanesDO.m_Visible = false;
        m_ApproxPlanesDO.m_LineColor = vec3d( .2, .2, .2 );
        m_ApproxPlanesDO.m_LineWidth = 1.0;
        m_ApproxPlanesDO.m_NormVec = m_ApproxPlanesDO.m_PntVec;

        draw_obj_vec.push_back( &m_ApproxPlanesDO );

        m_DelPtsDO.m_GeomID = GetID() + "m_DelPtsDO";
        m_DelPtsDO.m_Type = DrawObj::VSP_POINTS;
        m_DelPtsDO.m_Visible = false;
        m_DelPtsDO.m_PointColor = vec3d( .2, .4, .6 );
        m_DelPtsDO.m_PointSize = 10.0;
        m_DelPtsDO.m_PntVec.clear();

        for ( int indx = 0; indx < m_DelIPntVec.size(); indx++ )
        {
            m_DelPtsDO.m_PntVec.push_back( m_DelIPntVec[indx]->m_Pnt );
        }

        draw_obj_vec.push_back( &m_DelPtsDO );

        if ( m_IPatchADrawLines.size() > 0 && m_IPatchBDrawLines.size() > 0 )
        {
            m_IPatchADO.clear();
            m_IPatchADO.resize( m_IPatchADrawLines.size() );

            for ( size_t i = 0; i < m_IPatchADrawLines.size(); i++ )
            {
                m_IPatchADO[i].m_GeomID = GetID() + "IPatchA_" + to_string( i );
                m_IPatchADO[i].m_LineColor = vec3d( .4, .5, .6 );
                m_IPatchADO[i].m_LineWidth = 1.5;
                m_IPatchADO[i].m_Visible = false;
                m_IPatchADO[i].m_PntVec.clear();

                m_IPatchADO[i].m_Type = m_IPatchBDO[i].VSP_LINES;

                for ( int indx = 0; indx < m_IPatchADrawLines[i].size(); indx++ )
                {
                    m_IPatchADO[i].m_PntVec.push_back( m_IPatchADrawLines[i][indx] );
                }

                draw_obj_vec.push_back( &m_IPatchADO[i] );
            }

            m_IPatchBDO.clear();
            m_IPatchBDO.resize( m_IPatchBDrawLines.size() );

            for ( size_t i = 0; i < m_IPatchBDrawLines.size(); i++ )
            {
                m_IPatchBDO[i].m_GeomID = GetID() + "IPatchB_" + to_string( i );
                m_IPatchBDO[i].m_LineColor = vec3d( .7, .8, .9 );
                m_IPatchBDO[i].m_LineWidth = 1.5;
                m_IPatchBDO[i].m_Visible = false;
                m_IPatchBDO[i].m_PntVec.clear();

                m_IPatchBDO[i].m_Type = m_IPatchBDO[i].VSP_LINES;

                for ( int indx = 0; indx < m_IPatchBDrawLines[i].size(); indx++ )
                {
                    m_IPatchBDO[i].m_PntVec.push_back( m_IPatchBDrawLines[i][indx] );
                }

                draw_obj_vec.push_back( &m_IPatchBDO[i] );
            }
        }
    }

    WakeMgr.Show( GetSettingsPtr()->m_DrawSourceWakeFlag );
    WakeMgr.LoadDrawObjs( draw_obj_vec);
}

void SurfaceIntersectionSingleton::UpdateWakes()
{
    vector< piecewise_curve_type > wake_leading_edges;
    vector < double > wake_scale_vec;
    vector < double > wake_angle_vec;

    vector<string> geomVec = m_Vehicle->GetGeomVec();
    for ( int g = 0; g < (int)geomVec.size(); g++ )
    {
        Geom* geom = m_Vehicle->FindGeom( geomVec[g] );
        if ( geom )
        {
            if ( geom->GetSetFlag( GetSettingsPtr()->m_SelectedSetIndex ) )
            {
                geom->AppendWakeData( wake_leading_edges, wake_scale_vec, wake_angle_vec );
            }
        }
    }

    WakeMgr.SetLeadingEdges( wake_leading_edges );
    m_Vehicle->UpdateBBox();
    BndBox box = m_Vehicle->GetBndBox();
    WakeMgr.SetStartStretchX( box.GetMax( 0 ) + 0.01 * box.GetLargestDist() );
    WakeMgr.SetEndX( box.GetMax( 0 ) + 0.5 * box.GetLargestDist() );
    WakeMgr.SetWakeScaleVec( wake_scale_vec );
    WakeMgr.SetWakeAngleVec( wake_angle_vec );

}

void SurfaceIntersectionSingleton::SetICurveVec( ICurve* newcurve, int loc )
{
    m_ICurveVec[loc] = newcurve;
}


void SurfaceIntersectionSingleton::UpdateDisplaySettings()
{
    if ( GetIntersectSettingsPtr() )
    {
        GetIntersectSettingsPtr()->m_DrawSourceWakeFlag = m_Vehicle->GetISectSettingsPtr()->m_DrawSourceWakeFlag.Get();

        GetIntersectSettingsPtr()->m_DrawBorderFlag = m_Vehicle->GetISectSettingsPtr()->m_DrawBorderFlag.Get();
        GetIntersectSettingsPtr()->m_DrawIsectFlag = m_Vehicle->GetISectSettingsPtr()->m_DrawIsectFlag.Get();
        GetIntersectSettingsPtr()->m_DrawRawFlag = m_Vehicle->GetISectSettingsPtr()->m_DrawRawFlag.Get();
        GetIntersectSettingsPtr()->m_DrawBinAdaptFlag = m_Vehicle->GetISectSettingsPtr()->m_DrawBinAdaptFlag.Get();
        GetIntersectSettingsPtr()->m_DrawCurveFlag = m_Vehicle->GetISectSettingsPtr()->m_DrawCurveFlag.Get();
        GetIntersectSettingsPtr()->m_DrawPntsFlag = m_Vehicle->GetISectSettingsPtr()->m_DrawPntsFlag.Get();

        GetIntersectSettingsPtr()->m_RelCurveTol = m_Vehicle->GetISectSettingsPtr()->m_RelCurveTol.Get();

        GetIntersectSettingsPtr()->m_IntersectSubSurfs = m_Vehicle->GetISectSettingsPtr()->m_IntersectSubSurfs.Get();
        GetIntersectSettingsPtr()->m_SelectedSetIndex = m_Vehicle->GetISectSettingsPtr()->m_SelectedSetIndex.Get();

        GetIntersectSettingsPtr()->m_XYZIntCurveFlag = m_Vehicle->GetISectSettingsPtr()->m_XYZIntCurveFlag.Get();
    }
}
