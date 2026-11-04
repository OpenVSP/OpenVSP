//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// SurfaceIntersectionMgr
//
//////////////////////////////////////////////////////////////////////

#include <chrono>
#include <cstring>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <atomic>
#include <exception>
#include <iterator>
#include <limits>
#include <queue>
#include <array>
#include "SurfaceIntersectionMgr.h"
#include "ResultsMgr.h"
#include "VspUtil.h"
#include "SubSurfaceMgr.h"
#include "StringUtil.h"
#include "StlHelper.h"
#include "FileUtil.h"
#include "LocaleUtil.h"
#include <cfloat>  //For DBL_EPSILON
#include "ModeMgr.h"

#include "eli/geom/intersect/intersect_surface.hpp"
#include "eli/geom/intersect/minimum_distance_curve.hpp"

#include "MeshAnalysis.h"


#ifdef DEBUG_CFD_MESH
#define DEBUG_TIME_OUTPUT
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

double Wake::DistToClosestLeadingEdgePnt( const vec3d& pnt )
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

void WakeMgrSingleton::SetLeadingEdges( const vector < piecewise_curve_type >& wake_leading_edges )
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
    // The far field and symmetry surfaces go last but take the lowest IDs, so the wakes are
    // numbered past the highest ID in use.
    int last_id = -1;
    for ( int i = 0; i < (int)surf_vec.size(); i++ )
    {
        last_id = std::max( last_id, surf_vec[i]->GetSurfID() );
    }
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

void WakeMgrSingleton::UpdateDrawObjs()
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
    m_WakeDO.m_GeomChanged = true;
}

void WakeMgrSingleton::LoadDrawObjs( vector< DrawObj* >& draw_obj_vec )
{
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
    m_DrawObjsStale = false;

    m_MessageName = "SurfIntersectMessage";

    m_ProgressTotal = 0;
    m_ProgressDone = 0;
    m_WriteBar = nullptr;

    // A mesh belongs to the model it was built from, so it has to go when that model does.
    m_RenewListener.SetMgr( this );
    m_RenewListener.Register( "MeshRenew" );

#ifdef DEBUG_CFD_MESH
    m_DebugDir  = string( "MeshDebug/" );
    MakeDirectory( m_DebugDir );
    m_DebugFile = fopen( "MeshDebug/log.txt", "w" );
    m_DebugDraw = false;

    m_DebugSurfCnt = 0;
    m_DebugSortedUWFile = nullptr;
    m_DebugMeshUWFile = nullptr;
    m_DebugTriMeshFile = nullptr;
#endif

}

#ifdef DEBUG_CFD_MESH

// Start a fresh pass over the surfaces.
//
// Anything still open belongs to a pass that did not finish -- a run that threw, or one whose
// last surface failed to triangulate -- so it is finished off here rather than left to be
// written into by the new pass.
void SurfaceIntersectionSingleton::BeginDebugSurfFiles()
{
    EndDebugSurfFiles();

    // Made again each pass rather than once when the manager was built.  The path is
    // relative, and the working directory can move under a long lived process, in which case
    // every file below would silently fail to open.
    MakeDirectory( m_DebugDir );

    m_DebugSurfCnt = 0;
}

// Finish the master scripts, if a pass opened any.
//
// They are closed here, at the end of the pass, rather than when the surface count reaches
// the last surface.  The count is not to be trusted for this: a surface that fails to
// triangulate never increments it, so the last surface need not be the one that carries the
// final number, and nothing would close the files at all.
void SurfaceIntersectionSingleton::EndDebugSurfFiles()
{
    if ( m_DebugSortedUWFile )
    {
        fprintf( m_DebugSortedUWFile, "figure(1)\n" );
        fprintf( m_DebugSortedUWFile, "axis off\n" );
        fprintf( m_DebugSortedUWFile, "axis equal\n" );
        fprintf( m_DebugSortedUWFile, "hold off\n" );

        fclose( m_DebugSortedUWFile );
        m_DebugSortedUWFile = nullptr;
    }

    if ( m_DebugMeshUWFile )
    {
        fprintf( m_DebugMeshUWFile, "figure(1)\n" );
        fprintf( m_DebugMeshUWFile, "axis off\n" );
        fprintf( m_DebugMeshUWFile, "axis equal\n" );
        fprintf( m_DebugMeshUWFile, "hold off\n" );

        fclose( m_DebugMeshUWFile );
        m_DebugMeshUWFile = nullptr;
    }

    if ( m_DebugTriMeshFile )
    {
        for ( int ifig = 2 ; ifig <= 4 ; ifig++ )
        {
            fprintf( m_DebugTriMeshFile, "figure(%d)\n", ifig );
            fprintf( m_DebugTriMeshFile, "axis off\n" );
            fprintf( m_DebugTriMeshFile, "axis equal\n" );
            fprintf( m_DebugTriMeshFile, "hold off\n" );
        }

        fclose( m_DebugTriMeshFile );
        m_DebugTriMeshFile = nullptr;
    }
}

#endif

SurfaceIntersectionSingleton::~SurfaceIntersectionSingleton()
{
    CleanUp();

#ifdef DEBUG_CFD_MESH
    EndDebugSurfFiles();

    if ( m_DebugFile )
    {
        fclose( m_DebugFile );
        m_DebugFile = nullptr;
    }
#endif

}

// Set true to build and show the draw objects for debugging the intersector: the planes each
// chain is approximated by, the points that were deleted, and the two patches every
// intersection segment was found between.  Off, none of it is gathered.
static const bool DEBUG_ISECT_DRAW = false;

// Where AddIntersectionSeg puts what it makes.  Set for the length of one piece of the
// intersection, so a piece's output stays together whichever thread ran it.
static thread_local IsectOutput *tl_isect_out = nullptr;

// How many threads the mesher may use: what the machine says it can run at once, never less
// than one.
int SurfaceIntersectionSingleton::MeshThreadCount()
{
    int nthread = ( int )std::thread::hardware_concurrency();

    if ( nthread < 1 )
    {
        nthread = 1;
    }

    return nthread;
}

// How many threads a stage of nitem independent pieces should use.
int SurfaceIntersectionSingleton::StageThreadCount( int nitem )
{
    int nthread = 1;

    if ( GetSettingsPtr()->m_ParallelMeshFlag )
    {
        nthread = MeshThreadCount();
    }

    if ( nthread > nitem )
    {
        nthread = nitem;
    }

    if ( nthread < 1 )
    {
        nthread = 1;
    }

    return nthread;
}

// Run body( i ) for every i below n.  A thread takes the next piece that has not been started
// rather than a fixed share handed out up front, because one piece can cost many times what
// another does and a fixed split would leave threads idle waiting on the slowest share.
void SurfaceIntersectionSingleton::RunIndexed( int n, int nthread, const std::function< void( int ) > &body,
                                               ProgressStage *bar )
{
    // The first exception a work item threw, if any.  One that escaped a thread would end the
    // process, so a worker keeps it here instead and the caller is given it once the threads
    // are back -- the same thing that happens on the one-thread path.
    std::exception_ptr err;

    if ( nthread > 1 )
    {
        std::atomic< int > next( 0 );
        vector < std::thread > pool;
        std::mutex errmutex;

        try
        {
            for ( int t = 0 ; t < nthread ; t++ )
            {
                pool.push_back( std::thread( [&]()
                {
                    // A new thread starts in the process-wide locale, not the caller's.
                    CNumericLocaleGuard locale_guard;

                    while ( true )
                    {
                        int i = next++;

                        if ( i >= n )
                        {
                            break;
                        }

                        try
                        {
                            body( i );

                            if ( bar )
                            {
                                bar->Step();
                            }
                        }
                        catch ( ... )
                        {
                            std::lock_guard< std::mutex > errlock( errmutex );

                            if ( !err )
                            {
                                err = std::current_exception();
                            }

                            // Hand out no more work.  The items already running finish.
                            next = n;
                        }
                    }
                } ) );
            }
        }
        catch ( ... )
        {
            // Starting a thread failed partway through.  The ones already running still have
            // to be joined below, or their destructors end the process too.
            next = n;
            err = std::current_exception();
        }

        for ( int t = 0 ; t < ( int )pool.size() ; t++ )
        {
            pool[t].join();
        }
    }
    else
    {
        for ( int i = 0 ; i < n ; i++ )
        {
            body( i );

            if ( bar )
            {
                bar->Step();
            }
        }
    }

    if ( err )
    {
        std::rethrow_exception( err );
    }
}

// A stage of independent pieces, each of which reads what it needs and writes only its own
// answer, shown to the user as one progress bar counting the pieces.
void SurfaceIntersectionSingleton::RunStage( const string &label, int n,
                                             const std::function< void( int ) > &body )
{
    ProgressStage bar( this, label, n );

    RunIndexed( n, StageThreadCount( n ), body, &bar );
}

// The ASCII export writers spend most of their time turning numbers into text: on the larger
// test models that formatting is a third of the whole mesh run.  It depends on nothing but the
// item being written, so it can be done on several threads as long as the results reach the
// file in the order the items came in.
//
// The threads format chunks into a ring of slots while this thread writes the finished ones out
// in order, so the formatting goes on while the file takes the text.  A thread waits rather than
// run more than the ring ahead of the file, so a mesh whose text runs to a gigabyte is never held
// in memory.
void SurfaceIntersectionSingleton::WriteChunked( FILE* fp, int n,
        const std::function< void( int, int, string & ) > &body, ProgressStage *bar )
{
    if ( !fp || n <= 0 )
    {
        return;
    }

    if ( !bar )
    {
        bar = m_WriteBar;
    }

    // Large enough that a chunk is worth handing to a thread, small enough that the chunks in
    // flight are a few megabytes between them.
    const int chunkitems = 2048;

    int nchunk = ( n + chunkitems - 1 ) / chunkitems;
    int nthread = StageThreadCount( nchunk );

    if ( nthread <= 1 )
    {
        string out;
        for ( int c = 0 ; c < nchunk ; c++ )
        {
            int ibeg = c * chunkitems;
            int iend = std::min( n, ibeg + chunkitems );

            out.clear();
            body( ibeg, iend, out );
            fwrite( out.data(), 1, out.size(), fp );

            if ( bar )
            {
                bar->Step( iend - ibeg );
            }
        }
        return;
    }

    int nslot = 2 * nthread;
    vector < string > slot( nslot );
    vector < char > ready( nslot, 0 );  // Slot holds its chunk, formatted
    int written = 0;                    // Chunks gone to the file
    bool stop = false;
    std::exception_ptr err;
    std::atomic < int > next( 0 );
    std::mutex mtx;
    std::condition_variable cv;

    vector < std::thread > pool;
    try
    {
        for ( int t = 0 ; t < nthread ; t++ )
        {
            pool.push_back( std::thread( [&]()
            {
                // A new thread starts in the process-wide locale, not the caller's.
                CNumericLocaleGuard locale_guard;

                while ( true )
                {
                    int c = next++;
                    if ( c >= nchunk )
                    {
                        break;
                    }

                    {
                        std::unique_lock < std::mutex > lock( mtx );
                        cv.wait( lock, [&]() { return stop || c < written + nslot; } );
                        if ( stop )
                        {
                            break;
                        }
                    }

                    int ibeg = c * chunkitems;
                    int iend = std::min( n, ibeg + chunkitems );
                    string &out = slot[ c % nslot ];

                    try
                    {
                        out.clear();
                        body( ibeg, iend, out );
                    }
                    catch ( ... )
                    {
                        std::lock_guard < std::mutex > lock( mtx );
                        if ( !err )
                        {
                            err = std::current_exception();
                        }
                        stop = true;
                        cv.notify_all();
                        break;
                    }

                    {
                        std::lock_guard < std::mutex > lock( mtx );
                        ready[ c % nslot ] = 1;
                    }
                    cv.notify_all();
                }
            } ) );
        }
    }
    catch ( ... )
    {
        // Starting a thread failed partway through.  The ones already running still have to be
        // joined below, or their destructors end the process too.
        std::lock_guard < std::mutex > lock( mtx );
        if ( !err )
        {
            err = std::current_exception();
        }
        stop = true;
        cv.notify_all();
    }

    for ( int c = 0 ; c < nchunk ; c++ )
    {
        int k = c % nslot;
        {
            std::unique_lock < std::mutex > lock( mtx );
            cv.wait( lock, [&]() { return stop || ready[k]; } );
            if ( stop )
            {
                break;
            }
        }

        // No thread touches a ready slot until it has been written and handed back.
        fwrite( slot[k].data(), 1, slot[k].size(), fp );

        {
            std::lock_guard < std::mutex > lock( mtx );
            ready[k] = 0;
            written = c + 1;
        }
        cv.notify_all();

        if ( bar )
        {
            bar->Step( std::min( n, ( c + 1 ) * chunkitems ) - c * chunkitems );
        }
    }

    for ( int t = 0 ; t < ( int )pool.size() ; t++ )
    {
        pool[t].join();
    }

    if ( err )
    {
        std::rethrow_exception( err );
    }
}

void SurfaceIntersectionSingleton::WriteFileStage( const string &label, int n, const std::function< void() > &write )
{
    ProgressStage bar( this, label, n );

    ProgressStage *outer = m_WriteBar;
    m_WriteBar = &bar;

    try
    {
        write();
    }
    catch ( ... )
    {
        m_WriteBar = outer;
        throw;
    }

    m_WriteBar = outer;
}

void SurfaceIntersectionSingleton::IntersectSurfaces()
{
    addOutputText( "CLEAR_TERMINAL" );

    m_MeshInProgress = true;

#ifdef DEBUG_TIME_OUTPUT
    addOutputText( "Init Timer\n" );
#endif

    addOutputText( "Transfer Mesh Settings\n" );
    TransferMeshSettings();

    addOutputText( "Fetching Bezier Surfaces\n" );

    vector< XferSurf > xfersurfs;
    FetchSurfs( xfersurfs );

    // UpdateWakes must be before m_Vehicle->HideAll() to prevent components 
    // being being added to or removed from the Surface Intersection set
    addOutputText( "Update Wakes\n" );
    UpdateWakes();
    WakeMgr.SetStretchMeshFlag( false );

    // Hide all geoms after fetching their surfaces
    m_Vehicle->HideAll();

    addOutputText( "Cleanup\n" );
    CleanUp();

    LoadSurfs( xfersurfs );

    if ( GetSettingsPtr()->m_IntersectSubSurfs )
    {
        addOutputText( "Transfer Subsurf Data\n" );
        TransferSubSurfData();
    }

    addOutputText( "Clean Merge Surfs\n" );
    CleanMergeSurfs( /* skip_duplicate_removal */ false );

    addOutputText( "Identify CompID Names\n" );
    IdentifyCompIDNames();

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

    addOutputText( "Build Grid\n" );
    BuildGrid();

    // addOutputText( "Intersect\n" ); // Output in intersect() itself.
    Intersect();

    RecordIntCurves();

    addOutputText( "Adapt Curves for Trimmed CAD\n" );
    BuildChainCADCurves();

    addOutputText( "Exporting Files\n" );
    ExportFiles();

    RecordResults();

    m_DrawObjsStale = true;

    addOutputText( "Done\n" );

    m_MeshInProgress = false;
    MessageMgr::getInstance().Send( "ScreenMgr", "UpdateAllScreens" );
}

string SurfaceIntersectionSingleton::SplitStitchSurfaces( const string &file_name, bool step_flag, int set, int degen_set,
                                                          bool use_mode, const string &mode_id )
{
    // The run in progress owns the surfaces and curves
    if ( m_MeshInProgress )
    {
        return "Surface Intersection is running";
    }

    m_MeshInProgress = true;

    TransferMeshSettings();

    // The sets asked for, in place of those the Surface Intersection settings hold
    m_IntersectSettings.m_SelectedSetIndex = set;
    m_IntersectSettings.m_SelectedDegenSetIndex = degen_set;
    m_IntersectSettings.m_UseMode = use_mode;
    m_IntersectSettings.m_ModeID = mode_id;

    // Every piece of each surface kept apart.  Split and join rejoins a wing's round tip cap
    // into one patch whose border meets itself, which cannot be stitched.
    m_IntersectSettings.m_SplitJoinSurfsFlag = false;

    vector< XferSurf > xfersurfs;
    FetchSurfs( xfersurfs );

    // A negative component cuts nothing here, so it is a body like any other, facing out
    for ( int i = 0 ; i < ( int )xfersurfs.size() ; i++ )
    {
        if ( xfersurfs[i].m_SurfCfdType == vsp::CFD_NEGATIVE )
        {
            xfersurfs[i].m_SurfCfdType = vsp::CFD_NORMAL;
        }
    }

    CleanUp();

    LoadSurfs( xfersurfs );

    CleanMergeSurfs( /* skip_duplicate_removal */ false );

    IdentifyCompIDNames();

    if ( m_SurfVec.size() == 0 )
    {
        CleanUp();
        m_MeshInProgress = false;
        return "No surfaces to export";
    }

    // Match the border curves, which stitches each body back together from its pieces.  No
    // wakes: the bodies are written as they are.
    MatchBorderCurves();

    // The borders are the only curves
    LoadBorderCurves();
    SplitBorderCurves();
    BuildCurves();

    // Nothing is intersected, so no curve lies inside another body
    BuildNURBSCurvesVec( false );

    string delim = StringUtil::get_delim( GetSettingsPtr()->m_CADLabelDelim );

    if ( step_flag )
    {
        WriteSTEPFile( file_name, GetSettingsPtr()->m_CADLenUnit, GetSettingsPtr()->m_STEPTol,
                       GetSettingsPtr()->m_STEPMergePoints, GetSettingsPtr()->m_CADLabelID,
                       GetSettingsPtr()->m_CADLabelSurfNo, GetSettingsPtr()->m_CADLabelSplitNo,
                       GetSettingsPtr()->m_CADLabelName, delim, GetSettingsPtr()->m_STEPRepresentation );
    }
    else
    {
        WriteIGESFile( file_name, GetSettingsPtr()->m_CADLenUnit, GetSettingsPtr()->m_CADLabelID,
                       GetSettingsPtr()->m_CADLabelSurfNo, GetSettingsPtr()->m_CADLabelSplitNo,
                       GetSettingsPtr()->m_CADLabelName, delim );
    }

    CleanUp();

    m_MeshInProgress = false;

    return string();
}

void SurfaceIntersectionSingleton::LimitedIntersectSurfaces( const vector < string > & geomvec, vector < vector < vec3d > > & ptchains, vector < vector < vec3d > > & uwchains )
{
    addOutputText( "CLEAR_TERMINAL" );

    m_MeshInProgress = true;

#ifdef DEBUG_TIME_OUTPUT
    addOutputText( "Init Timer\n" );
#endif

    addOutputText( "Transfer Mesh Settings\n" );
    TransferMeshSettings();

    addOutputText( "Fetching Bezier Surfaces\n" );

    vector< XferSurf > xfersurfs;
    FetchXFerSurfs( geomvec, xfersurfs );

    // UpdateWakes must be before m_Vehicle->HideAll() to prevent components
    // being being added to or removed from the Surface Intersection set
    addOutputText( "Update Wakes\n" );
    UpdateWakes();
    WakeMgr.SetStretchMeshFlag( false );

    addOutputText( "Cleanup\n" );
    CleanUp();

    LoadSurfs( xfersurfs );

    addOutputText( "Clean Merge Surfs\n" );
    CleanMergeSurfs( /* skip_duplicate_removal */ false );

    addOutputText( "Identify CompID Names\n" );
    IdentifyCompIDNames();

    if ( m_SurfVec.size() == 0 )
    {
        addOutputText( "No Surfaces To Mesh\n" );
        m_MeshInProgress = false;
        MessageMgr::getInstance().Send( "ScreenMgr", "UpdateAllScreens" );
        return;
    }

    addOutputText( "Build Grid\n" );
    BuildGrid();

    // addOutputText( "Intersect\n" ); // Output in intersect() itself.
    Intersect();

    addOutputText( "Binary Adaptation Curve Approximation\n" );

    BuildIntChain( geomvec[0], ptchains, uwchains );

    addOutputText( "Done\n" );

    m_MeshInProgress = false;
    MessageMgr::getInstance().Send( "ScreenMgr", "UpdateAllScreens" );
}

void MeshRenewListener::MessageCallback( const MessageBase* from, const MessageData& data )
{
    if ( m_Mgr && data.m_String == "Renew" )
    {
        m_Mgr->RenewMesh();
    }
}

// Everything built from the vehicle's geometry goes.  The intersection and meshing managers
// outlive any one model, so without this a mesh -- and everything drawn from it -- survives
// into a model it has nothing to do with.
void SurfaceIntersectionSingleton::RecordResults()
{
    m_LastResultID = string();

    Results* res = ResultsMgr.CreateResults( "SurfaceIntersection", "Surface intersection results." );

    if ( !res )
    {
        return;
    }

    m_LastResultID = res->GetID();

    res->Add( new NameValData( "Num_Surfs", GetNumSurfs(), "Number of surfaces the intersection ran on." ) );
    res->Add( new NameValData( "Num_Chains", GetNumChains(), "Number of intersection curves found." ) );
    res->Add( new NameValData( "Num_Curve_Pnts", GetNumCurvePnts(), "Number of raw intersection points over all the intersection curves." ) );

    // Each curve's raw points, one after another, and how many belong to each
    vector < int > npnt_vec( m_RawCurveAVec.size() );
    vector < vec3d > pnt_vec;
    pnt_vec.reserve( GetNumCurvePnts() );
    for ( int i = 0 ; i < ( int )m_RawCurveAVec.size() ; i++ )
    {
        npnt_vec[i] = ( int )m_RawCurveAVec[i].size();
        pnt_vec.insert( pnt_vec.end(), m_RawCurveAVec[i].begin(), m_RawCurveAVec[i].end() );
    }

    res->Add( new NameValData( "Curve_Num_Pnts", npnt_vec, "Number of raw points on each intersection curve." ) );
    res->Add( new NameValData( "Curve_Pnts", pnt_vec, "Raw points of every intersection curve, curve after curve." ) );
}

int SurfaceIntersectionSingleton::GetNumCurvePnts() const
{
    int npnt = 0;

    for ( int i = 0 ; i < ( int )m_RawCurveAVec.size() ; i++ )
    {
        npnt += ( int )m_RawCurveAVec[i].size();
    }

    return npnt;
}

void SurfaceIntersectionSingleton::RenewMesh()
{
    CleanUp();
}

// addOutputText appends to one buffer, so calls coming from several surfaces at once have to
// be kept from overlapping.
void SurfaceIntersectionSingleton::ReportProgress( const string &str, int output_type )
{
    static std::mutex outmutex;
    std::lock_guard< std::mutex > lock( outmutex );

    addOutputText( str, output_type );
}

void SurfaceIntersectionSingleton::BeginProgress( const string &label, int n, int output_type )
{
    m_ProgressLabel = label;
    m_ProgressTotal = n;
    m_ProgressDone = 0;

    StepProgress( output_type );
}

// Draw the bar at a given count.  The line is redrawn in place, so every call has to write the
// same width -- a shorter line would leave the tail of the previous one behind.  The counts only
// ever grow, so the width is stable as long as the last draw uses the total.
void SurfaceIntersectionSingleton::DrawProgress( int done, char term, int output_type )
{
    const int nbar = 24;

    // A count that was only an estimate stops at the total.
    if ( done > m_ProgressTotal )
    {
        done = m_ProgressTotal;
    }

    int nfill = ( nbar * done ) / m_ProgressTotal;

    char bar[nbar + 1];
    for ( int i = 0; i < nbar; i++ )
    {
        if ( i < nfill )
        {
            bar[i] = '=';
        }
        else
        {
            bar[i] = ' ';
        }
    }
    bar[nbar] = '\0';

    char str[256];
    snprintf( str, sizeof( str ), "%-16s [%s] %d/%d%c", m_ProgressLabel.c_str(), bar, done, m_ProgressTotal, term );

    ReportProgress( str, output_type );
}

void SurfaceIntersectionSingleton::StepProgress( int output_type )
{
    if ( output_type == QUIET_OUTPUT || m_ProgressTotal <= 0 )
    {
        return;
    }

    DrawProgress( m_ProgressDone, '\r', output_type );
}

// Count one finished piece of work, and redraw only on every stride'th, for a stage with too
// many pieces to draw each one.
void SurfaceIntersectionSingleton::StepProgressEvery( int stride, int output_type )
{
    int done = ++m_ProgressDone;

    if ( done % stride == 0 )
    {
        StepProgress( output_type );
    }
}

void SurfaceIntersectionSingleton::EndProgress( int output_type )
{
    if ( output_type == QUIET_OUTPUT || m_ProgressTotal <= 0 )
    {
        m_ProgressTotal = 0;
        return;
    }

    // Threads redraw as they finish, so the last one to land is not necessarily the highest
    // count.  Draw the finished bar once here so the stage always ends showing all of its work.
    DrawProgress( m_ProgressTotal, '\n', output_type );

    m_ProgressTotal = 0;
}

ProgressStage::ProgressStage( SurfaceIntersectionSingleton *mgr, const string &label, int n )
{
    m_Mgr = mgr;
    m_Live = ( n > 0 && mgr->m_ProgressTotal <= 0 );
    m_N = n;
    m_Stride = std::max( 1, n / 200 );

    if ( m_Live )
    {
        m_Mgr->BeginProgress( label, n, SurfaceIntersectionSingleton::VOCAL_OUTPUT );
    }
}

ProgressStage::~ProgressStage()
{
    End();
}

void ProgressStage::End()
{
    if ( m_Live )
    {
        m_Mgr->EndProgress( SurfaceIntersectionSingleton::VOCAL_OUTPUT );
        m_Live = false;
    }
}

void ProgressStage::Restart( const string &label )
{
    if ( m_Live )
    {
        m_Mgr->BeginProgress( label, m_N, SurfaceIntersectionSingleton::VOCAL_OUTPUT );
    }
}

// Redraw when the count passes a multiple of the stride.
void ProgressStage::Step( int count )
{
    if ( !m_Live )
    {
        return;
    }

    int done = ( m_Mgr->m_ProgressDone += count );

    if ( done / m_Stride != ( done - count ) / m_Stride )
    {
        m_Mgr->StepProgress( SurfaceIntersectionSingleton::VOCAL_OUTPUT );
    }
}

void SurfaceIntersectionSingleton::CleanUp()
{
    m_DrawObjsStale = false;

    int i;
    //==== Delete Old Surfs ====//
    for ( i = 0 ; i < ( int )m_SurfVec.size() ; i++ )
    {
        delete m_SurfVec[i];
    }
    m_SurfVec.clear();

    for ( i = 0 ; i < ( int )m_RemovedFlatFaces.size() ; i++ )
    {
        delete m_RemovedFlatFaces[i];
    }
    m_RemovedFlatFaces.clear();
    m_RemovedFaceComps.clear();

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
    vector < Puw* >().swap( m_DelPuwVec );

    //==== Delete IPnts ====//
    for ( i = 0 ; i < ( int )m_DelIPntVec.size() ; i++ )
    {
        delete m_DelIPntVec[i];
    }
    vector < IPnt* >().swap( m_DelIPntVec );

    //==== Delete IPntGroups ====//
    for ( i = 0 ; i < ( int )m_DelIPntGroupVec.size() ; i++ )
    {
        delete m_DelIPntGroupVec[i];
    }
    vector < IPntGroup* >().swap( m_DelIPntGroupVec );

    //==== Delete IPntGroups ====//
    for ( i = 0 ; i < ( int )m_DelISegChainVec.size() ; i++ )
    {
        delete m_DelISegChainVec[i];
    }
    vector < ISegChain* >().swap( m_DelISegChainVec );

    for ( i = 0 ; i < ( int )m_DelICurveVec.size() ; i++ )
    {
        delete m_DelICurveVec[i];
    }
    vector < ICurve* >().swap( m_DelICurveVec );

    vector < IPnt* >().swap( m_AllIPnts ); // Pointers should already be deallocated
    vector < IPntEdge >().swap( m_AllIPntEdges );
    vector < int >().swap( m_AllIPntPartner );

    m_PossCoPlanarSurfMap.clear();
    m_CoplanarPairs.clear();

    debugPnts.clear();

    m_SimpleSubSurfaceVec.clear();

    m_IPatchADrawLines.clear();
    m_IPatchBDrawLines.clear();

    m_ChainCADVec.clear();

    // The intersection curves RecordIntCurves keeps for drawing and results.
    vector < vector < vec3d > >().swap( m_RawCurveAVec );
    vector < vector < vec3d > >().swap( m_RawCurveBVec );
    vector < bool >().swap( m_BorderCurveFlagVec );
    vector < bool >().swap( m_PatchJoinFlagVec );
    vector < bool >().swap( m_NonManifoldCurveFlagVec );

    // Clean up DrawObj's
    m_RawIsectCurveDO = DrawObj();
    m_RawIsectPtsDO = DrawObj();
    for ( int k = 0; k < NUM_CUBIC; k++ )
    {
        m_CubicCurveDO[k] = DrawObj();
        m_CubicEndPtsDO[k] = DrawObj();
        m_CubicCtrlPtsDO[k] = DrawObj();
    }
    m_RawBorderCurveDO = DrawObj();
    m_RawBorderPtsDO = DrawObj();

    // The curves that bound one patch rather than two.  Emptied with the rest: UpdateDrawObjs
    // adds to these rather than rebuilding them, so anything left here is drawn again on top
    // of the next mesh.
    m_RawNonManifoldCurveDO = DrawObj();
    m_RawNonManifoldPtsDO = DrawObj();

    m_RawPatchJoinCurveDO = DrawObj();
    m_RawPatchJoinPtsDO = DrawObj();

    m_ApproxPlanesDO = DrawObj();

    m_DelPtsDO = DrawObj();

    m_IPatchADO.clear();
    m_IPatchBDO.clear();

}

static void RegisterSurfaceIntersectionAnalysis()
{
    SurfaceIntersectionMgr.RegisterAnalysis();
}

static AnalysisRegistrar g_SurfaceIntersectionRegistrar( RegisterSurfaceIntersectionAnalysis );

void SurfaceIntersectionSingleton::RegisterAnalysis()
{
    if ( !AnalysisMgr.FindAnalysis( "SurfaceIntersection" ) )
    {
        SurfaceIntersectionAnalysis* sia = new SurfaceIntersectionAnalysis();

        if ( sia && !AnalysisMgr.RegisterAnalysis( sia ) )
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

    vector < string > prop_id_vec;

    for ( size_t i = 0; i < ss_vec.size(); i++ )
    {
        m_SimpleSubSurfaceVec[i] = SimpleSubSurface();
        m_SimpleSubSurfaceVec[ i ].CopyFrom( ss_vec[ i ], prop_id_vec );
    }
}

//==== Get vector of SimpleSubSurfaces from geom by ID and surf number ====//
vector< SimpleSubSurface > SurfaceIntersectionSingleton::GetSimpSubSurfs( const string &geom_id, int surfnum, int comp_id )
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

    if ( geom->isNonSurfaceType() )
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
        if ( ( m_SimpleSubSurfaceVec[i].m_MainSurfIndx == -1 || imain == m_SimpleSubSurfaceVec[i].m_MainSurfIndx ) &&
             strcmp( m_SimpleSubSurfaceVec[i].GetCompID().c_str(), geom_id.c_str() ) == 0 )
        {
            ret_vec.push_back( m_SimpleSubSurfaceVec[i] );
        }
    }

    return ret_vec;
}

int SurfaceIntersectionSingleton::GetSimpSubSurfIndex( const string &ss_id )
{
    for ( int i = 0; i < m_SimpleSubSurfaceVec.size(); i++ )
    {
        if ( m_SimpleSubSurfaceVec[i].GetSSID() == ss_id )
        {
            return i;
        }
    }
    return -1;
}

void SurfaceIntersectionSingleton::addOutputText( string str, int output_type )
{
    if ( output_type != QUIET_OUTPUT )
    {
#ifdef DEBUG_TIME_OUTPUT
        static auto tprev = std::chrono::high_resolution_clock::now();
        auto tnow = std::chrono::high_resolution_clock::now();
        char buf[256];
        snprintf( buf, sizeof( buf ), "%9.3fms ", std::chrono::duration_cast< std::chrono::microseconds >( tnow - tprev ).count() / 1000.0 );
        tprev = tnow;
        str.insert( 0, buf );
#endif

        MessageData data;
        data.m_String = m_MessageName;
        data.m_StringVec.push_back( str );
        MessageMgr::getInstance().Send( "ScreenMgr", nullptr, data );
    }
}

void SurfaceIntersectionSingleton::FetchXFerSurfs( const vector < string > & geomvec, vector< XferSurf > &xfersurfs )
{
    m_Vehicle->FetchXFerSurfs( geomvec, xfersurfs, 0, GetSettingsPtr()->m_SplitJoinSurfsFlag,
                               GetSettingsPtr()->m_HalfMeshFlag, false, StageThreadCount( std::numeric_limits< int >::max() ) );
}

// n_ref asks each Geom for that many levels of tessellation refinement on the lines its
// surfaces carry along.  Only the POGS surface output wants anything but the default.
void SurfaceIntersectionSingleton::FetchSurfs( vector< XferSurf > &xfersurfs, int n_ref, bool cfdcaps )
{
    int normal_set = GetSettingsPtr()->m_SelectedSetIndex;
    int degen_set = GetSettingsPtr()->m_SelectedDegenSetIndex;

    if ( GetSettingsPtr()->m_UseMode )
    {
        Mode *m = ModeMgr.GetMode( GetSettingsPtr()->m_ModeID );
        if ( m )
        {
            m->ApplySettings();
            normal_set = m->m_NormalSet();
            degen_set = m->m_DegenSet();
        }
    }

    // As many threads as the stage may use; the fetch uses no more than it has surfaces.
    m_Vehicle->FetchXFerSurfs( normal_set, degen_set, xfersurfs, n_ref,
                               GetSettingsPtr()->m_SplitJoinSurfsFlag, GetSettingsPtr()->m_HalfMeshFlag, cfdcaps,
                               StageThreadCount( std::numeric_limits< int >::max() ) );
}

void SurfaceIntersectionSingleton::LoadSurfs( vector< XferSurf > &xfersurfs, double scale, int start_surf_id )
{
    int maxcompid = -1;
    for ( int i = 0; i < xfersurfs.size(); i++ )
    {
        Surf* surfPtr = new Surf();
        surfPtr->GetSurfCore()->SetSurf( xfersurfs[i].m_Surface );

        surfPtr->SetGeomID( xfersurfs[i].m_GeomID );

        surfPtr->SetName( xfersurfs[i].m_Name );

        surfPtr->SetSplitNum( xfersurfs[i].m_SplitNum );

        surfPtr->SetMainSurfID( xfersurfs[i].m_SurfIndx );

        //Xfersurfs created each time and flips normal if geom is a negative volume
        if ( xfersurfs[i].m_SurfCfdType == vsp::CFD_NEGATIVE )
        {
            xfersurfs[i].m_FlipNormal = !xfersurfs[i].m_FlipNormal;
        }

        surfPtr->SetFlipFlag( xfersurfs[i].m_FlipNormal );

        surfPtr->GetSurfCore()->GetSurf()->scale( scale );

        //Sets whether NORMAL, NEGATIVE, TRANSPARENT
        surfPtr->SetSurfaceCfdType(xfersurfs[i].m_SurfCfdType);

        surfPtr->SetFeaOrientationType( xfersurfs[i].m_FeaOrientationType );
        surfPtr->SetFeaOrientation( xfersurfs[i].m_FeaOrientation );
        surfPtr->SetFeaPartSurfNum( xfersurfs[i].m_FeaPartSurfNum );
        surfPtr->SetFeaSymmIndex( xfersurfs[i].m_FeaSymmIndex );

        //Sets whether NORMAL_SURF, WING_SURF, DISK_SURF, PROP_SURF
        surfPtr->SetSurfaceVSPType(xfersurfs[i].m_SurfType);

        surfPtr->SetSurfaceThickSurf(xfersurfs[i].m_ThickSurf);

        surfPtr->SetSurfacePlateNum(xfersurfs[i].m_PlateNum);
        surfPtr->SetSurfaceCopyIndex(xfersurfs[i].m_CopyIndex);

        surfPtr->SetPlanarUWAspect( xfersurfs[i].m_PlanarUWAspect );

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
        surfPtr->SetUWTess( xfersurfs[i].m_UTess, xfersurfs[i].m_WTess );
        surfPtr->SetUWRegions( xfersurfs[i].m_UWRegions );
        surfPtr->SetJoinLines( xfersurfs[i].m_JoinLines );
        m_SurfVec.push_back( surfPtr );
    }

    // Each surface's patches are its own, so they are built on several threads.
    int nfirst = ( int )m_SurfVec.size() - ( int )xfersurfs.size();
    int nload = ( int )xfersurfs.size();
    RunStage( "Load Surfaces", nload, [&]( int k )
    {
        Surf* surfPtr = m_SurfVec[ nfirst + k ];
        surfPtr->GetSurfCore()->BuildPatches( surfPtr );
    } );

    // Don't include FEA parts (with negative Comp index)
    if ( maxcompid >= 0 )
    {
        m_NumComps = maxcompid + 1;
    }
}

// How closely two borders, or two flat faces, must lie along each other to be one, relative to the
// size of the surfaces compared.  Two bodies given by different control nets -- a spinner turned
// against its nacelle -- trace the same circle to a few parts in ten thousand of its size.
// Borders are compared this way only around a flat face two bodies shared, which
// DeleteDuplicateSurfs found by where it lies and removed from both, so that the two bodies close
// each other off there.
static const double BORDER_GEOM_RELTOL = 1.0e-3;

void SurfaceIntersectionSingleton::CleanMergeSurfs(  bool skip_duplicate_removal )
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

    if ( !skip_duplicate_removal )
    {
        DeleteDuplicateSurfs();
    }

    //==== Combine Components With Matching Surface Edges ====//
    unordered_map< int, int > mergeCompMap;
    for ( int s = 0 ; s < ( int )m_SurfVec.size() - 1 ; s++ )
    {
        for ( int t = s + 1 ; t < ( int )m_SurfVec.size() ; t++ )
        {
            int compA = m_SurfVec[s]->GetCompID();
            int compB = m_SurfVec[t]->GetCompID();

            // Surfaces whose boxes do not come near each other cannot share a border, and BorderMatch is
            // expensive enough to be worth not asking.
            if ( compA == compB ||
                 !Compare( m_SurfVec[s]->GetBBox(), m_SurfVec[t]->GetBBox(), 1.0e-6 ) )
            {
                continue;
            }

            if ( m_SurfVec[s]->BorderMatch( m_SurfVec[t] ) )
            {
                // Only merge like-type surfaces.  I.e. normal, negative, etc.
                if ( m_SurfVec[s]->GetSurfaceCfdType() == m_SurfVec[t]->GetSurfaceCfdType() )
                {
                    mergeCompMap[compB] = compA;
                }
            }
        }
    }

    // Two bodies that shared a flat face, removed from both, close each other off.
    for ( int k = 0 ; k < ( int )m_RemovedFaceComps.size() ; k++ )
    {
        mergeCompMap[ m_RemovedFaceComps[k].second ] = m_RemovedFaceComps[k].first;
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

// How flat a surface must be, and how far apart two may be, to lie in one plane: relative to
// the size of the surfaces compared
static const double COPLANAR_RELTOL = 1.0e-6;

// A grid of points over a surface, and the parameters of each.
static void SampleSurf( Surf* srf, int n, vector< vec3d > &pnts, vector< vec2d > &uws )
{
    SurfCore* sc = srf->GetSurfCore();

    pnts.clear();
    uws.clear();

    for ( int i = 0 ; i <= n ; i++ )
    {
        double u = sc->GetMinU() + ( sc->GetMaxU() - sc->GetMinU() ) * i / n;

        for ( int j = 0 ; j <= n ; j++ )
        {
            double w = sc->GetMinW() + ( sc->GetMaxW() - sc->GetMinW() ) * j / n;

            uws.push_back( vec2d( u, w ) );
            pnts.push_back( sc->CompPnt( u, w ) );
        }
    }
}

// Whether every point of a's grid lies within tol of surface b.  Each search for the nearest
// point of b starts from the nearest point of b's grid.
static bool SurfWithin( const vector< vec3d > &apnts, Surf* b, const vector< vec3d > &bpnts, const vector< vec2d > &buws, double tol )
{
    for ( int i = 0 ; i < ( int )apnts.size() ; i++ )
    {
        int jmin = 0;
        double dmin = dist_squared( apnts[i], bpnts[0] );
        for ( int j = 1 ; j < ( int )bpnts.size() ; j++ )
        {
            double d = dist_squared( apnts[i], bpnts[j] );
            if ( d < dmin )
            {
                dmin = d;
                jmin = j;
            }
        }

        double u, w;
        if ( b->GetSurfCore()->FindNearest( u, w, apnts[i], buws[jmin].x(), buws[jmin].y() ) > tol )
        {
            return false;
        }
    }

    return true;
}

// Whether q lies inside the component comp, by the number of its surfaces a ray from q crosses.
static bool InsideComp( const vector< Surf* > &surfs, int comp, const vec3d &q, double len )
{
    vec3d p0 = q;
    vec3d p1 = q + vec3d( len, 1.0e-4, 1.0e-4 );

    vector< double > t;
    for ( int i = 0 ; i < ( int )surfs.size() ; i++ )
    {
        if ( surfs[i]->GetCompID() == comp )
        {
            surfs[i]->IntersectLineSeg( p0, p1, t );
        }
    }

    return t.size() % 2 == 1;
}

// A face of a positive body and a face of a negative body that lie one on the other -- the end
// faces of a flow-through engine, where the negative body's face sits a hair outside the
// positive body's, or the face where a flowpath passes from a positive body to a negative
// one -- are paired.  Each is meshed and cut by the other bodies, but not by the
// other face or its body, and the inside/outside tests take the pairing as their verdict
// between the two bodies.
void SurfaceIntersectionSingleton::PairCoincidentFaces()
{
    int nsurf = ( int )m_SurfVec.size();

    // Long enough for a ray to leave every body.
    BndBox all;
    for ( int s = 0 ; s < nsurf ; s++ )
    {
        all.Update( m_SurfVec[s]->GetBBox() );
    }
    double xlen = 1.0 + all.GetMax( 0 ) - all.GetMin( 0 );

    // The box around each component.  A negative face is set off from a positive one by a
    // fraction of the whole body, not of the face, so the faces are compared at a fraction of
    // the bodies' size.
    map < int, BndBox > compbox;
    for ( int s = 0 ; s < nsurf ; s++ )
    {
        compbox[ m_SurfVec[s]->GetCompID() ].Update( m_SurfVec[s]->GetBBox() );
    }

    int n = 8;
    vector< vector< vec3d > > pnts( nsurf );
    vector< vector< vec2d > > uws( nsurf );

    // The box around each surface's component
    vector< const BndBox* > ownbox( nsurf );
    for ( int t = 0 ; t < nsurf ; t++ )
    {
        ownbox[t] = &compbox[ m_SurfVec[t]->GetCompID() ];
    }

    // Only a positive face and a negative one are paired, so each surface is tried against the
    // thick surfaces of the other kind, in order.
    vector< int > normal;
    vector< int > negative;
    for ( int t = 0 ; t < nsurf ; t++ )
    {
        if ( m_SurfVec[t]->GetSurfaceThickSurf() )
        {
            if ( m_SurfVec[t]->GetSurfaceCfdType() == vsp::CFD_NORMAL )
            {
                normal.push_back( t );
            }
            else if ( m_SurfVec[t]->GetSurfaceCfdType() == vsp::CFD_NEGATIVE )
            {
                negative.push_back( t );
            }
        }
    }

    for ( int s = 0 ; s + 1 < nsurf ; s++ )
    {
        int stype = m_SurfVec[s]->GetSurfaceCfdType();
        if ( m_SurfVec[s]->GetCoincidentSurf() || !m_SurfVec[s]->GetSurfaceThickSurf() ||
             ( stype != vsp::CFD_NORMAL && stype != vsp::CFD_NEGATIVE ) )
        {
            continue;
        }

        BndBox &bbs = m_SurfVec[s]->GetBBox();

        const vector< int > *other = &negative;
        if ( stype == vsp::CFD_NEGATIVE )
        {
            other = &normal;
        }

        vector< int >::const_iterator first = std::upper_bound( other->begin(), other->end(), s );

        for ( vector< int >::const_iterator it = first ; it != other->end() && !m_SurfVec[s]->GetCoincidentSurf() ; ++it )
        {
            int t = *it;

            if ( m_SurfVec[t]->GetCoincidentSurf() || m_SurfVec[s]->GetCompID() == m_SurfVec[t]->GetCompID() )
            {
                continue;
            }

            // Faces this close together, for the size of their bodies, are taken as one face.
            BndBox bodies = *ownbox[s];
            bodies.Update( *ownbox[t] );
            double tol = 1.0e-3 * bodies.DiagDist();

            BndBox &bbt = m_SurfVec[t]->GetBBox();

            bool samebox = true;
            for ( int k = 0 ; k < 3 && samebox ; k++ )
            {
                if ( std::abs( bbs.GetMin( k ) - bbt.GetMin( k ) ) > tol ||
                     std::abs( bbs.GetMax( k ) - bbt.GetMax( k ) ) > tol )
                {
                    samebox = false;
                }
            }

            if ( !samebox )
            {
                continue;
            }

            if ( pnts[s].empty() )
            {
                SampleSurf( m_SurfVec[s], n, pnts[s], uws[s] );
            }
            if ( pnts[t].empty() )
            {
                SampleSurf( m_SurfVec[t], n, pnts[t], uws[t] );
            }

            if ( SurfWithin( pnts[s], m_SurfVec[t], pnts[t], uws[t], tol ) &&
                 SurfWithin( pnts[t], m_SurfVec[s], pnts[s], uws[s], tol ) )
            {
                // The two bodies overlap where a point just off the faces, on one side or the
                // other, lies inside both.
                int mid = ( int )uws[s].size() / 2;
                vec3d n = m_SurfVec[s]->CompNorm( uws[s][mid].x(), uws[s][mid].y() );
                double off = 1.0e-2 * bbs.DiagDist();

                bool overlap = false;
                for ( int side = -1 ; side <= 1 ; side += 2 )
                {
                    vec3d q = pnts[s][mid] + n * ( side * off );
                    if ( InsideComp( m_SurfVec, m_SurfVec[s]->GetCompID(), q, xlen ) &&
                         InsideComp( m_SurfVec, m_SurfVec[t]->GetCompID(), q, xlen ) )
                    {
                        overlap = true;
                    }
                }

                m_SurfVec[s]->SetCoincidentSurf( m_SurfVec[t], overlap );
                m_SurfVec[t]->SetCoincidentSurf( m_SurfVec[s], overlap );
            }
        }
    }
}

// Whether every point of a grid over each of the surfaces of one group lies within tol of a
// surface of the other.  The nearest point is searched for over the whole surface: a flat face
// is often a cap whose parameters collapse to a point, where a search started nearby can stall.
bool SurfaceIntersectionSingleton::FlatGroupWithin( const vector< int > &ga, const vector< int > &gb, double tol )
{
    vector< BndBox > boxes( gb.size() );
    for ( int j = 0 ; j < ( int )gb.size() ; j++ )
    {
        boxes[j] = m_SurfVec[ gb[j] ]->GetBBox();
        boxes[j].Expand( tol );
    }

    for ( int i = 0 ; i < ( int )ga.size() ; i++ )
    {
        vector< vec3d > apnts;
        vector< vec2d > auws;
        SampleSurf( m_SurfVec[ ga[i] ], 8, apnts, auws );

        // The points are searched for on several threads, each through copies of its own of the
        // surfaces it searches; a copy finds the same nearest point the surface does.
        int npnt = ( int )apnts.size();
        int nthread = StageThreadCount( npnt );
        std::atomic< bool > missed( false );

        RunIndexed( nthread, nthread, [&]( int t )
        {
            vector< std::unique_ptr< SurfCore > > cores( gb.size() );

            for ( int k = t ; k < npnt && !missed ; k += nthread )
            {
                bool found = false;
                for ( int j = 0 ; j < ( int )gb.size() && !found ; j++ )
                {
                    if ( !boxes[j].CheckPnt( apnts[k] ) )
                    {
                        continue;
                    }

                    if ( !cores[j] )
                    {
                        cores[j].reset( new SurfCore( *m_SurfVec[ gb[j] ]->GetSurfCore() ) );
                    }

                    double u, w;
                    found = cores[j]->FindNearest( u, w, apnts[k] ) <= tol;
                }
                if ( !found )
                {
                    missed = true;
                }
            }
        } );

        if ( missed )
        {
            return false;
        }
    }
    return true;
}

// The flat faces of the solid bodies, gathered by component and by the plane they lie in, facing
// one way.  Two such groups of different components that cover the same region of one plane are
// one face, however each was cut into patches and whatever control nets they have: pressed back
// to back, both go, and the two bodies close each other off there; facing the same way, the
// later one goes.
void SurfaceIntersectionSingleton::MatchFlatFaceGroups( vector< bool > &delflag, vector< bool > &flatface,
                                                        map < pair < string, string >, int > &dupboth,
                                                        map < pair < string, string >, int > &dupone )
{
    int nsurf = ( int )m_SurfVec.size();

    vector< vector< int > > groups;
    vector< BndBox > boxes;

    for ( int i = 0 ; i < nsurf ; i++ )
    {
        Surf* srf = m_SurfVec[i];
        if ( delflag[i] || srf->GetCompID() < 0 || srf->GetSurfaceCfdType() != vsp::CFD_NORMAL ||
             !srf->GetSurfaceThickSurf() || srf->GetWakeFlag() || srf->GetSymPlaneFlag() || srf->GetFarFlag() )
        {
            continue;
        }

        srf->FitPlane( COPLANAR_RELTOL );
        if ( !srf->GetPlaneFlag() )
        {
            continue;
        }

        int g = -1;
        for ( int k = 0 ; k < ( int )groups.size() && g < 0 ; k++ )
        {
            Surf* rep = m_SurfVec[ groups[k][0] ];
            double tol = COPLANAR_RELTOL * std::max( rep->GetBBox().DiagDist(), srf->GetBBox().DiagDist() );
            if ( rep->GetCompID() == srf->GetCompID() && dot( rep->GetPlaneNorm(), srf->GetPlaneNorm() ) > 0.0 &&
                 rep->SamePlane( srf, tol ) )
            {
                g = k;
            }
        }

        if ( g < 0 )
        {
            groups.push_back( vector< int >() );
            boxes.push_back( BndBox() );
            g = ( int )groups.size() - 1;
        }
        groups[g].push_back( i );
        boxes[g].Update( srf->GetBBox() );
    }

    vector< bool > used( groups.size(), false );

    for ( int g = 0 ; g < ( int )groups.size() ; g++ )
    {
        for ( int h = g + 1 ; h < ( int )groups.size() && !used[g] ; h++ )
        {
            Surf* a = m_SurfVec[ groups[g][0] ];
            Surf* b = m_SurfVec[ groups[h][0] ];

            if ( used[h] || a->GetCompID() == b->GetCompID() )
            {
                continue;
            }

            double size = std::max( boxes[g].DiagDist(), boxes[h].DiagDist() );
            double tol = BORDER_GEOM_RELTOL * size;

            if ( !Compare( boxes[g], boxes[h], tol ) || !a->SamePlane( b, COPLANAR_RELTOL * size ) ||
                 !FlatGroupWithin( groups[g], groups[h], tol ) || !FlatGroupWithin( groups[h], groups[g], tol ) )
            {
                continue;
            }

            used[g] = true;
            used[h] = true;

            bool opposite = dot( a->GetPlaneNorm(), b->GetPlaneNorm() ) < 0.0;

            pair < string, string > geoms = make_pair( a->GetGeomID(), b->GetGeomID() );
            if ( opposite )
            {
                dupboth[ geoms ]++;
                m_RemovedFaceComps.push_back( pair< int, int >( a->GetCompID(), b->GetCompID() ) );
            }
            else
            {
                dupone[ geoms ]++;
            }

            for ( int k = 0 ; k < ( int )groups[h].size() ; k++ )
            {
                delflag[ groups[h][k] ] = true;
                flatface[ groups[h][k] ] = opposite;
            }
            for ( int k = 0 ; k < ( int )groups[g].size() && opposite ; k++ )
            {
                delflag[ groups[g][k] ] = true;
                flatface[ groups[g][k] ] = true;
            }
        }
    }
}

void SurfaceIntersectionSingleton::DeleteDuplicateSurfs()
{
    unsigned int nsurf = m_SurfVec.size();

    vector < bool > delflag( nsurf );
    vector < bool > flatface( nsurf );
    for ( int i = 0 ; i < nsurf ; i++ )
    {
        delflag[i] = false;
        flatface[i] = false;
    }

    // Two surfaces can only be duplicates if they occupy the same box.  SurfMatch copies the
    // other surface eight times over, once for each way round it might be turned, so pairs that
    // cannot possibly match are worth turning away before it is asked.
    double bbtol = 1.0e-6;

    // Pairs of coincident surfaces between two Geoms: those of which both are left out, and those of which one is.
    map < pair < string, string >, int > dupboth;
    map < pair < string, string >, int > dupone;

    for ( int s = 0 ; s + 1 < nsurf ; s++ )
    {
        BndBox &bbs = m_SurfVec[s]->GetBBox();

        for ( int t = s + 1 ; t < nsurf ; t++ )
        {
            BndBox &bbt = m_SurfVec[t]->GetBBox();

            bool samebox = true;

            for ( int k = 0; k < 3 && samebox; k++ )
            {
                if ( std::abs( bbs.GetMin( k ) - bbt.GetMin( k ) ) > bbtol ||
                     std::abs( bbs.GetMax( k ) - bbt.GetMax( k ) ) > bbtol )
                {
                    samebox = false;
                }
            }

            if ( !samebox )
            {
                continue;
            }

            bool opposite = false;
            if ( m_SurfVec[s]->GetSurfCore()->SurfMatch( m_SurfVec[t]->GetSurfCore(), opposite ) )
            {
                if ( m_SurfVec[s]->GetFlipFlag() != m_SurfVec[t]->GetFlipFlag() )
                {
                    opposite = !opposite;
                }

                // Two faces pressed back to back both go.  Two facing the same way, of the same
                // kind, are one surface given twice, and one of them stays.
                pair < string, string > geoms = make_pair( m_SurfVec[s]->GetGeomID(), m_SurfVec[t]->GetGeomID() );
                if ( opposite || m_SurfVec[s]->GetSurfaceCfdType() != m_SurfVec[t]->GetSurfaceCfdType() )
                {
                    delflag[s] = true;
                    dupboth[ geoms ]++;
                }
                else
                {
                    dupone[ geoms ]++;
                }
                delflag[t] = true;
            }
        }
    }

    // The CFD mesher, which closes two bodies off around a flat face they share, also matches a
    // flat face given by different control nets, or cut into different patches, by where it lies.
    if ( CoplanarSurfsMeshed() )
    {
        MatchFlatFaceGroups( delflag, flatface, dupboth, dupone );
    }

    for ( int k = 0; k < 2; k++ )
    {
        map < pair < string, string >, int > *dupcount = &dupboth;
        string fate = "both of each are left out of the mesh";
        if ( k == 1 )
        {
            dupcount = &dupone;
            fate = "one of each is left out of the mesh";
        }

        for ( map < pair < string, string >, int >::iterator it = dupcount->begin(); it != dupcount->end(); ++it )
        {
            string sname = it->first.first;
            Geom* sgeom = m_Vehicle->FindGeom( sname );
            if ( sgeom )
            {
                sname = sgeom->GetName();
            }

            string tname = it->first.second;
            Geom* tgeom = m_Vehicle->FindGeom( tname );
            if ( tgeom )
            {
                tname = tgeom->GetName();
            }

            if ( it->first.first == it->first.second )
            {
                addOutputText( "Warning: " + to_string( it->second ) + " pairs of surfaces of " + sname + " coincide, perhaps from symmetry about a plane it lies on; " + fate + "\n" );
            }
            else
            {
                addOutputText( "Warning: " + to_string( it->second ) + " pairs of surfaces of " + sname + " and " + tname + " coincide; " + fate + "\n" );
            }
        }
    }

    // A flat face two bodies shared is kept aside: the borders around it are matched later.
    vector < Surf* > keepSurf;
    for ( int i = 0 ; i < nsurf ; i++ )
    {
        if ( delflag[i] && flatface[i] )
        {
            m_RemovedFlatFaces.push_back( m_SurfVec[i] );
        }
        else if ( delflag[i] )
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

// Where a patch has been put back together out of two, a side of it is one curve while the
// patches across from it still have two: the join it was made across is an edge on that
// side.  Curves pair off one to one, so the long one is cut where the short ones meet and
// the pieces pair as they should.  The cut adds no edge of its own -- it puts a break where
// the patch across from it already breaks.
static vec3d SCurvePnt( SCurve* c, double t )
{
    vec3d uw = c->CompPntUW( t );
    return c->GetSurf()->CompPnt( uw.x(), uw.y() );
}

// The side of its surface a border curve runs along, as the surface's own exact curve over just
// the stretch the border curve covers, and where along the side that stretch starts and ends.
// The side is cut to the stretch because a side that comes back on itself passes every point
// twice, and the search must land on this curve's pass.
struct SCurveSide
{
    piecewise_curve_type m_Crv;
    double m_S0 = 0.0;
    double m_S1 = 0.0;
};

// False for a curve that does not run along a side of its surface.
static bool GetSCurveSide( SCurve* c, SCurveSide &side )
{
    Bezier_curve border;
    c->GetBorderCurve( border );

    piecewise_curve_type crv = border.GetCurve();

    if ( crv.number_segments() == 0 )
    {
        return false;
    }

    vec3d uw0 = c->CompPntUW( 0.0 );
    vec3d uw1 = c->CompPntUW( 1.0 );

    // A side of constant u runs in w, and one of constant w runs in u.
    if ( std::abs( uw0.x() - uw1.x() ) < 1.0e-12 )
    {
        side.m_S0 = uw0.y();
        side.m_S1 = uw1.y();
    }
    else
    {
        side.m_S0 = uw0.x();
        side.m_S1 = uw1.x();
    }

    double lo = min( side.m_S0, side.m_S1 );
    double hi = max( side.m_S0, side.m_S1 );

    piecewise_curve_type before, after;

    crv.split( before, after, lo );
    after.split( side.m_Crv, before, hi );

    return side.m_Crv.number_segments() > 0;
}

// Where along the curve the point p lies, or a negative number if it does not lie on it.
static double FindOnSCurve( const SCurveSide &side, const vec3d &p, double tol )
{
    curve_point_type pt;
    pt << p.x(), p.y(), p.z();

    double s;
    double d = eli::geom::intersect::minimum_distance( s, side.m_Crv, pt );

    if ( d > tol )
    {
        return -1.0;
    }

    return ( s - side.m_S0 ) / ( side.m_S1 - side.m_S0 );
}

// What matching needs of one border curve: its two ends, its side, and a box around it.  They
// depend only on the curve, so they are worked out once and kept rather than again for every
// curve it is tried against.
//
// The box turns a point away before it is searched for.  A Bezier stays inside the hull of its
// control points, so a box around the side's, grown by the tolerance, holds every point that
// could be found on the curve.
struct SCurveTake
{
    vec3d m_End0;
    vec3d m_End1;
    bool m_HasSide = false;
    SCurveSide m_Side;
    BndBox m_Box;
};

static void TakeSCurve( SCurve* c, double tol, SCurveTake &take )
{
    take.m_End0 = SCurvePnt( c, 0.0 );
    take.m_End1 = SCurvePnt( c, 1.0 );
    take.m_HasSide = GetSCurveSide( c, take.m_Side );

    take.m_Box.Reset();

    if ( take.m_HasSide )
    {
        piecewise_curve_type::bounding_box_type bbox;
        take.m_Side.m_Crv.get_bounding_box( bbox );

        take.m_Box.Update( vec3d( bbox.get_max() ) );
        take.m_Box.Update( vec3d( bbox.get_min() ) );
        take.m_Box.Expand( tol );
    }
}

// The border curves with an end within tol of a point, found through a tree of every curve's
// ends.  A cut changes the ends of the curve it cuts and adds a curve, and the tree holds the ends
// as they were when it was built, so the curves changed since are looked at directly.  The tree is
// built again once they are many.
class BorderEndIndex
{
public:
    BorderEndIndex( const vector< vector< SCurveTake > > &takes, double tol ) : m_Takes( takes ), m_Tol( tol )
    {
        Build();
    }

    // Curve c of surface i has changed or is new.
    void Changed( int i, int c )
    {
        m_Changed.push_back( pair< int, int >( i, c ) );
        if ( m_Changed.size() > 256 )
        {
            Build();
        }
    }

    // The curves, as ( surface, curve ) in order, with an end within tol of p.
    void Near( const vec3d &p, vector< pair< int, int > > &found )
    {
        found.clear();

        // Searched a little wide, so a point at just under tol is not lost to how the tree sums
        // the distance; the test below is the one that decides.
        FindPntNodesWithinTolSq( m_Cloud, p, 4.0 * m_Tol * m_Tol, m_Found );
        for ( size_t k = 0 ; k < m_Found.size() ; k++ )
        {
            Add( m_Owner[ m_Found[k] ], p, found );
        }
        for ( size_t k = 0 ; k < m_Changed.size() ; k++ )
        {
            Add( m_Changed[k], p, found );
        }

        std::sort( found.begin(), found.end() );
        found.erase( std::unique( found.begin(), found.end() ), found.end() );
    }

private:

    void Build()
    {
        m_Cloud.Cleanup();
        m_Owner.clear();
        for ( int i = 0 ; i < ( int )m_Takes.size() ; i++ )
        {
            for ( int c = 0 ; c < ( int )m_Takes[i].size() ; c++ )
            {
                m_Cloud.AddPntNode( m_Takes[i][c].m_End0 );
                m_Cloud.AddPntNode( m_Takes[i][c].m_End1 );
                m_Owner.push_back( pair< int, int >( i, c ) );
                m_Owner.push_back( pair< int, int >( i, c ) );
            }
        }
        BuildPntNodeIndex( m_Cloud );
        m_Changed.clear();
    }

    void Add( const pair< int, int > &ic, const vec3d &p, vector< pair< int, int > > &found ) const
    {
        const SCurveTake &t = m_Takes[ ic.first ][ ic.second ];
        if ( dist( t.m_End0, p ) < m_Tol || dist( t.m_End1, p ) < m_Tol )
        {
            found.push_back( ic );
        }
    }

    const vector< vector< SCurveTake > > &m_Takes;
    double m_Tol;
    PntNodeCloud m_Cloud;
    vector< pair< int, int > > m_Owner;       // The curve each point of the cloud is an end of
    vector< pair< int, int > > m_Changed;
    vector< unsigned int > m_Found;
};

// Split border curve a of surface srf at t, the first piece in its place and the second at the
// end of the list.
static void SplitSCurveAt( vector< SCurve* > &cva, int a, double t, Surf* srf )
{
    // Both pieces stay on the line the curve was on
    const ParmLine line = cva[a]->GetParmLine();

    vec3d uw0 = line.OnLine( cva[a]->CompPntUW( 0.0 ) );
    vec3d uwt = line.OnLine( cva[a]->CompPntUW( t ) );
    vec3d uw1 = line.OnLine( cva[a]->CompPntUW( 1.0 ) );

    vector< vec3d > pnts( 2 );
    SCurve* c0 = new SCurve( srf );
    pnts[0] = uw0;
    pnts[1] = uwt;
    c0->InterpolateLinear( pnts );
    c0->PromoteTo( 3 );
    c0->SetParmLine( line );

    SCurve* c1 = new SCurve( srf );
    pnts[0] = uwt;
    pnts[1] = uw1;
    c1->InterpolateLinear( pnts );
    c1->PromoteTo( 3 );
    c1->SetParmLine( line );

    delete cva[a];
    cva[a] = c0;
    cva.push_back( c1 );
}

// Whether a border curve runs along the border of a flat face DeleteDuplicateSurfs removed from
// two bodies.
bool SurfaceIntersectionSingleton::OnRemovedFaceRim( SCurve* c )
{
    Surf* srf = c->GetSurf();
    Bezier_curve uwcrv = c->GetUWCrv();
    vector< vec3d > pnts;
    for ( int j = 0 ; j <= 4 ; j++ )
    {
        vec3d uw = uwcrv.CompPnt01( 0.25 * j );
        pnts.push_back( srf->CompPnt( uw.x(), uw.y() ) );
    }

    for ( int f = 0 ; f < ( int )m_RemovedFlatFaces.size() ; f++ )
    {
        Surf* face = m_RemovedFlatFaces[f];
        double tol = BORDER_GEOM_RELTOL * std::max( srf->GetBBox().DiagDist(), face->GetBBox().DiagDist() );
        if ( face->PntsOnBorder( pnts, tol ) )
        {
            return true;
        }
    }
    return false;
}

// Whether c runs along a removed face's rim, asked once per curve and kept in onrim.
bool SurfaceIntersectionSingleton::OnRemovedFaceRimKept( SCurve* c, std::unordered_map< SCurve*, bool > &onrim )
{
    std::unordered_map< SCurve*, bool >::iterator it = onrim.find( c );
    if ( it != onrim.end() )
    {
        return it->second;
    }
    bool r = OnRemovedFaceRim( c );
    onrim[ c ] = r;
    return r;
}

// The two bodies around a removed face may cut their rims into pieces at different places, a
// spinner turned against its nacelle.  Each rim curve is cut where a rim curve of another surface
// ends along it, so the pieces pair up end to end.
void SurfaceIntersectionSingleton::SplitFaceRimsToMatch()
{
    bool changed = true;
    int guard = 0;
    const int guardmax = 1000;

    // Whether each curve runs along a rim, asked once per curve.  A cut curve is forgotten
    // before it is deleted, so a new curve at its address is asked again.
    std::unordered_map< SCurve*, bool > onrim;

    while ( changed && guard < guardmax )
    {
        changed = false;
        guard++;

        for ( int i = 0 ; i < ( int )m_SurfVec.size() && !changed ; i++ )
        {
            vector< SCurve* > &cva = m_SurfVec[i]->GetSCurveVec();

            for ( int a = 0 ; a < ( int )cva.size() && !changed ; a++ )
            {
                if ( !OnRemovedFaceRimKept( cva[a], onrim ) )
                {
                    continue;
                }

                for ( int j = 0 ; j < ( int )m_SurfVec.size() && !changed ; j++ )
                {
                    if ( j == i )
                    {
                        continue;
                    }

                    double tol = BORDER_GEOM_RELTOL * std::max( m_SurfVec[i]->GetBBox().DiagDist(), m_SurfVec[j]->GetBBox().DiagDist() );

                    SCurveTake ta;
                    TakeSCurve( cva[a], tol, ta );
                    if ( !ta.m_HasSide )
                    {
                        continue;
                    }

                    vector< SCurve* > &cvb = m_SurfVec[j]->GetSCurveVec();

                    for ( int b = 0 ; b < ( int )cvb.size() && !changed ; b++ )
                    {
                        if ( !OnRemovedFaceRimKept( cvb[b], onrim ) )
                        {
                            continue;
                        }

                        for ( int e = 0 ; e < 2 && !changed ; e++ )
                        {
                            vec3d end = SCurvePnt( cvb[b], ( double )e );

                            if ( dist( end, ta.m_End0 ) < tol || dist( end, ta.m_End1 ) < tol || !ta.m_Box.CheckPnt( end ) )
                            {
                                continue;
                            }

                            double t = FindOnSCurve( ta.m_Side, end, tol );
                            if ( t < 0.01 || t > 0.99 )
                            {
                                continue;
                            }

                            onrim.erase( cva[a] );
                            SplitSCurveAt( cva, a, t, m_SurfVec[i] );
                            changed = true;
                        }
                    }
                }
            }
        }
    }

    if ( guard >= guardmax )
    {
        addOutputText( "\tWarning: gave up matching the rims of removed flat faces after " +
                       to_string( guardmax ) + " passes.\n" );
    }
}

void SurfaceIntersectionSingleton::SplitBordersToMatch()
{
    double tol = 1.0e-5;
    bool changed = true;
    int guard = 0;
    const int guardmax = 1000;
    int nsurf = m_SurfVec.size();

    // Every curve, taken once.  There is one pass for every cut made, and a cut only ever changes
    // the curves of the surface being cut, so they are taken here and that one surface's are
    // taken again when it changes.
    vector< vector< SCurveTake > > takes( nsurf );

    // A curve is clean once it has been tried against every curve it shares an end with and
    // none was cut from it.  It stays clean until a curve with an end at one of its own is cut,
    // so a pass looks only at the curves that are not.
    vector< vector< char > > dirty( nsurf );

    for ( int i = 0 ; i < nsurf ; i++ )
    {
        vector< SCurve* > &cv = m_SurfVec[i]->GetSCurveVec();

        takes[i].resize( cv.size() );
        dirty[i].assign( cv.size(), 1 );

        for ( int c = 0 ; c < ( int )cv.size() ; c++ )
        {
            TakeSCurve( cv[c], tol, takes[i][c] );
        }
    }

    BorderEndIndex ends( takes, tol );
    vector< pair< int, int > > cand, cand1;

    while ( changed && guard < guardmax )
    {
        changed = false;
        guard++;

        for ( int i = 0 ; i < nsurf && !changed ; i++ )
        {
            vector< SCurve* > &cva = m_SurfVec[i]->GetSCurveVec();

            for ( int a = 0 ; a < ( int )cva.size() && !changed ; a++ )
            {
                if ( !dirty[i][a] )
                {
                    continue;
                }

                vec3d a0 = takes[i][a].m_End0;
                vec3d a1 = takes[i][a].m_End1;

                // The curves of other surfaces that share an end with this one, in the order
                // of their surfaces and then their curves.
                ends.Near( a0, cand );
                ends.Near( a1, cand1 );
                cand.insert( cand.end(), cand1.begin(), cand1.end() );
                std::sort( cand.begin(), cand.end() );
                cand.erase( std::unique( cand.begin(), cand.end() ), cand.end() );

                for ( int k = 0 ; k < ( int )cand.size() && !changed ; k++ )
                {
                    int j = cand[k].first;
                    int b = cand[k].second;

                    if ( j == i )
                    {
                        continue;
                    }

                    vec3d b0 = takes[j][b].m_End0;
                    vec3d b1 = takes[j][b].m_End1;

                    // One end of the short curve has to sit on an end of the long one,
                    // and its other end somewhere along the middle of it.
                    vec3d far_end;
                    bool shares = false;

                    if ( dist( b0, a0 ) < tol || dist( b0, a1 ) < tol )
                    {
                        far_end = b1;
                        shares = true;
                    }
                    else if ( dist( b1, a0 ) < tol || dist( b1, a1 ) < tol )
                    {
                        far_end = b0;
                        shares = true;
                    }

                    if ( !shares || dist( far_end, a0 ) < tol || dist( far_end, a1 ) < tol )
                    {
                        continue;
                    }

                    if ( !takes[i][a].m_HasSide || !takes[i][a].m_Box.CheckPnt( far_end ) )
                    {
                        continue;
                    }

                    double t = FindOnSCurve( takes[i][a].m_Side, far_end, tol );

                    if ( t < 0.01 || t > 0.99 )
                    {
                        continue;
                    }

                    SplitSCurveAt( cva, a, t, m_SurfVec[i] );

                    // Only this surface's curves moved; take them again.
                    takes[i].resize( cva.size() );
                    TakeSCurve( cva[a], tol, takes[i][a] );
                    TakeSCurve( cva.back(), tol, takes[i].back() );

                    int anew = cva.size() - 1;
                    ends.Changed( i, a );
                    ends.Changed( i, anew );

                    // Every curve with an end where the cut curve ended, or where it is cut, may
                    // now pair with one of its pieces.  The pieces' own ends are put back on the
                    // curve's parameter line, so they are searched about as well as a0 and a1.
                    dirty[i].resize( cva.size(), 1 );
                    vec3d at[5] = { a0, a1, takes[i][a].m_End1, takes[i][a].m_End0, takes[i][anew].m_End1 };
                    for ( int e = 0 ; e < 5 ; e++ )
                    {
                        ends.Near( at[e], cand1 );
                        for ( int m = 0 ; m < ( int )cand1.size() ; m++ )
                        {
                            dirty[ cand1[m].first ][ cand1[m].second ] = 1;
                        }
                    }
                    dirty[i][a] = 1;
                    dirty[i][anew] = 1;

                    changed = true;
                }

                if ( !changed )
                {
                    dirty[i][a] = 0;
                }
            }
        }
    }

    // One cut per pass, so a model that needs many takes many.  Running out says the
    // boundaries were not all brought into agreement, which shows up much later as borders
    // that never matched, so say it here where it can still be tied to a cause.
    if ( guard >= guardmax )
    {
        addOutputText( "\tWarning: gave up matching border curves after " +
                       to_string( guardmax ) + " passes.\n" );
    }
}
void SurfaceIntersectionSingleton::BuildGrid()
{
    MatchBorderCurves();

    //==== Build Wake Surfaces (If Defined) ====//
    WakeMgr.CreateWakesAppendBorderCurves( m_ICurveVec, GetGridDensityPtr() );
    WakeMgr.AppendWakeSurfs( m_SurfVec );
}

void SurfaceIntersectionSingleton::MatchBorderCurves()
{
    int i, j;
    vector< SCurve* > scurve_vec;

    for ( i = 0 ; i < ( int )m_SurfVec.size() ; i++ )
    {
        m_SurfVec[i]->FindBorderCurves();
    }

    SplitBordersToMatch();

    if ( !m_RemovedFlatFaces.empty() )
    {
        SplitFaceRimsToMatch();
    }

    for ( i = 0 ; i < ( int )m_SurfVec.size() ; i++ )
    {
        m_SurfVec[i]->LoadSCurves( scurve_vec );
    }

    // Every border curve against every other, which is quadratic, and the costly part of a
    // comparison is putting each curve into xyz.  Convert each one once, and let a bounding box
    // turn away the pairs that cannot possibly meet before anything is compared point by point.
    int nsc = ( int )scurve_vec.size();

    vector< Bezier_curve > xyzcrvs( nsc );
    vector< BndBox > crvboxes( nsc );

    for ( i = 0 ; i < nsc ; i++ )
    {
        xyzcrvs[i] = scurve_vec[i]->GetUWCrv();
        xyzcrvs[i].UWCurveToXYZCurve( scurve_vec[i]->GetSurf() );
        xyzcrvs[i].GetBBox( crvboxes[i] );
    }

    // The curves that run around a flat face DeleteDuplicateSurfs removed from two bodies
    vector< bool > onface( nsc, false );
    for ( i = 0 ; i < nsc && !m_RemovedFlatFaces.empty() ; i++ )
    {
        onface[i] = OnRemovedFaceRim( scurve_vec[i] );
    }

    // A border curve is matched once.  The first pass pieces each surface of the model back
    // together from the patches it was split into.  The second pairs what is left across
    // surfaces, which is where coincident surfaces were deleted and two bodies close each
    // other off; bodies that only touch, like the two root caps of a symmetric wing, stay
    // closed on their own.
    for ( int pass = 0 ; pass < 2 ; pass++ )
    {
        for ( i = 0 ; i < nsc ; i++ )
        {
            for ( j = i + 1 ; j < nsc ; j++ )
            {
                if ( scurve_vec[i]->GetICurve() )
                {
                    break;
                }

                if ( scurve_vec[j]->GetICurve() )
                {
                    continue;
                }

                if ( pass == 0 && scurve_vec[i]->GetSurf()->GetUnmergedCompID() != scurve_vec[j]->GetSurf()->GetUnmergedCompID() )
                {
                    continue;
                }

                if ( !Compare( crvboxes[i], crvboxes[j], 1.0e-5 ) )
                {
                    continue;
                }

                // Around a removed flat face, a match by control points is checked by where
                // the curves lie.
                double geomtol = -1.0;
                if ( onface[i] && onface[j] )
                {
                    geomtol = BORDER_GEOM_RELTOL * std::max( scurve_vec[i]->GetSurf()->GetBBox().DiagDist(),
                                                             scurve_vec[j]->GetSurf()->GetBBox().DiagDist() );
                }

                ICurve* icrv = new ICurve;
                if ( icrv->Match( scurve_vec[i], scurve_vec[j], xyzcrvs[i], xyzcrvs[j], geomtol ) )
                {
                    m_ICurveVec.push_back( icrv );

                    // A backwards match turns the second curve around, so the copy held for it is
                    // no longer the curve it names.  Take it again before it is compared to
                    // anything else.
                    xyzcrvs[j] = scurve_vec[j]->GetUWCrv();
                    xyzcrvs[j].UWCurveToXYZCurve( scurve_vec[j]->GetSurf() );
                    xyzcrvs[j].GetBBox( crvboxes[j] );
                }
                else
                {
                    delete icrv;
                }
            }
        }
    }


    //==== Check For SCurves Not Matched ====//
    int num_unmatched = 0;
    map< int, int > open_borders;
    map< int, string > open_names;
    for ( i = 0 ; i < ( int )scurve_vec.size() ; i++ )
    {
        if ( !scurve_vec[i]->GetICurve() )
        {
            ICurve* icrv = new ICurve;
            icrv->SetACurve( scurve_vec[i] );
            m_ICurveVec.push_back( icrv );
            num_unmatched++;

            // A border of a solid surface that meets no other border, and does not lie on
            // another surface, leaves its component open
            Surf* srf = scurve_vec[i]->GetSurf();
            int cfdtype = srf->GetSurfaceCfdType();
            bool solid = srf->GetCompID() >= 0 && srf->GetSurfaceThickSurf() &&
                         ( cfdtype == vsp::CFD_NORMAL || cfdtype == vsp::CFD_NEGATIVE ) &&
                         !srf->GetWakeFlag() && !srf->GetSymPlaneFlag() && !srf->GetFarFlag();

            // A half mesh is cut at y = 0 and left open there: nothing at or below the plane is
            // kept, to the tolerance CleanMergeSurfs removes surfaces to.
            if ( solid && GetSettingsPtr()->m_HalfMeshFlag )
            {
                BndBox crvbox;
                xyzcrvs[i].GetBBox( crvbox );
                if ( crvbox.GetMax( 1 ) <= 1.0e-6 )
                {
                    solid = false;
                }
            }

            // The same test, to the same tolerance, as BorderCurveOnSurface, which lays such a
            // border on the surface it lies on.
            bool onsurf = false;
            for ( j = 0 ; j < ( int )m_SurfVec.size() && solid && !onsurf ; j++ )
            {
                if ( m_SurfVec[j] != srf && m_SurfVec[j]->CurveOnSurface( xyzcrvs[i], 1.0e-05 ) )
                {
                    onsurf = true;
                }
            }

            if ( solid && !onsurf )
            {
                string name = srf->GetGeomID();
                Geom* geom = m_Vehicle->FindGeom( srf->GetGeomID() );
                if ( geom )
                {
                    name = geom->GetName();
                }
                open_borders[ srf->GetCompID() ]++;
                open_names[ srf->GetCompID() ] = name;
            }
        }
    }

    // Each open component once.  Two of one name -- a Geom and its symmetric copy -- are told
    // apart by their component numbers.
    map< string, int > namecount;
    map< int, string >::iterator on;
    for ( on = open_names.begin() ; on != open_names.end(); ++on )
    {
        namecount[ on->second ]++;
    }

    map< int, int >::iterator ob;
    for ( ob = open_borders.begin() ; ob != open_borders.end(); ++ob )
    {
        string name = open_names[ ob->first ];
        if ( namecount[ name ] > 1 )
        {
            name += " (component " + to_string( ob->first ) + ")";
        }

        string msg = "Warning: " + name + " is not closed: " + to_string( ob->second ) +
                     " of its border curves meet no other surface";

        // Inside and outside are decided by casting rays against each component, which only
        // works for a closed one.
        if ( RemovesInteriorTris() )
        {
            msg += ", so triangles where it meets other components may be wrongly kept or removed";
        }
        msg += ".\n";

        addOutputText( msg );
        printf( "%s", msg.c_str() );
    }
    if ( !open_borders.empty() )
    {
        fflush( stdout );
    }

#ifdef DEBUG_CFD_MESH
    fprintf( m_DebugFile, "SurfaceIntersectionSingleton::MatchBorderCurves \n" );
    fprintf( m_DebugFile, "  Num unmatched SCurves = %d \n", num_unmatched );

    for ( i = 0 ; i < ( int )m_ICurveVec.size() ; i++ )
    {
        m_ICurveVec[i]->DebugEdgeMatching( m_DebugFile );
    }

#endif
}

void SurfaceIntersectionSingleton::ExportFiles()
{
    if ( GetSettingsPtr()->GetExportFileFlag( vsp::INTERSECT_IGES_FILE_NAME ) || GetSettingsPtr()->GetExportFileFlag( vsp::INTERSECT_STEP_FILE_NAME ) )
    {
        BuildNURBSCurvesVec(); // Note: Must be called before BuildNURBSSurfMap

        // One map for both files
        BuildNURBSSurfMap();
    }

    string delim = StringUtil::get_delim( GetSettingsPtr()->m_CADLabelDelim );

    WriteNURBSSurfsCAD( GetSettingsPtr()->GetExportFileFlag( vsp::INTERSECT_IGES_FILE_NAME ), GetSettingsPtr()->GetExportFileName( vsp::INTERSECT_IGES_FILE_NAME ),
                        GetSettingsPtr()->GetExportFileFlag( vsp::INTERSECT_STEP_FILE_NAME ), GetSettingsPtr()->GetExportFileName( vsp::INTERSECT_STEP_FILE_NAME ),
                        GetSettingsPtr()->m_CADLenUnit, GetSettingsPtr()->m_STEPTol, GetSettingsPtr()->m_STEPMergePoints,
                        GetSettingsPtr()->m_CADLabelID, GetSettingsPtr()->m_CADLabelSurfNo, GetSettingsPtr()->m_CADLabelSplitNo,
                        GetSettingsPtr()->m_CADLabelName, delim, GetSettingsPtr()->m_STEPRepresentation );
}

Surf* SurfaceIntersectionSingleton::FindSurf( int surf_id )
{
    int indx = FindSurfIndx( surf_id );
    if ( indx >= 0 )
    {
        return m_SurfVec[ indx ];
    }
    return nullptr;
}

int SurfaceIntersectionSingleton::FindSurfIndx( int surf_id )
{
    for ( size_t i = 0; i < m_SurfVec.size(); i++ )
    {
        if ( m_SurfVec[i]->GetSurfID() == surf_id )
        {
            return i;
        }
    }
    return -1;
}

// The label a CAD file gives a surface: its Geom's ID, its component's name, its surface number
// and its split number, each as asked for, in that order
string SurfaceIntersectionSingleton::CADSurfLabel( const NURBS_Surface &nurbs_surf, bool label_id, bool label_surf_num,
                                                   bool label_split_num, bool label_name, const string &label_delim )
{
    // Match NURBS_Surface to index in m_SurfVec using m_SurfID. 
    Surf* current_surf = FindSurf( nurbs_surf.m_SurfID );

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

        // Looked up without adding, as the CAD files are written on two threads at once
        int comp = current_surf->GetCompID();
        if ( current_surf->GetFeaPartIndex() >= 0 )
        {
            // FEA Part
            comp = current_surf->GetFeaPartIndex();
        }

        unordered_map < int, string >::const_iterator it = m_CompIDNameMap.find( comp );
        if ( it != m_CompIDNameMap.end() )
        {
            label.append( it->second );
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
        label.append( to_string( nurbs_surf.m_SurfID ) );
    }

    return label;
}

void SurfaceIntersectionSingleton::WriteNURBSSurfsCAD( bool iges_flag, const string &iges_fn, bool step_flag, const string &step_fn,
                                                       int len_unit, double tol, bool merge_pnts, bool label_id, bool label_surf_num,
                                                       bool label_split_num, bool label_name, const string &label_delim, int representation )
{
    if ( !iges_flag || !step_flag || m_NURBSSurfVec.empty() )
    {
        if ( iges_flag )
        {
            WriteNURBSSurfsIGES( iges_fn, len_unit, label_id, label_surf_num, label_split_num, label_name, label_delim );
        }
        if ( step_flag )
        {
            WriteNURBSSurfsSTEP( step_fn, len_unit, tol, merge_pnts, label_id, label_surf_num, label_split_num, label_name,
                                 label_delim, representation );
        }
        return;
    }

    // The IGES and STEP libraries share nothing, so the two files are written at once.  Evaluating
    // a surface writes scratch inside it, so the IGES writer works on its own copy of each.
    vector < piecewise_surface_type > surf_copy( m_NURBSSurfVec.size() );
    vector < NURBS_Surface > iges_surf_vec = m_NURBSSurfVec;
    for ( size_t si = 0; si < m_NURBSSurfVec.size(); si++ )
    {
        surf_copy[si] = *m_NURBSSurfVec[si].GetSurf();
        iges_surf_vec[si].SetSurf( &surf_copy[si] );
    }

    RunStage( "Write IGES/STEP", 2, [&]( int k )
    {
        if ( k == 0 )
        {
            WriteNURBSSurfsIGES( iges_surf_vec, iges_fn, len_unit, label_id, label_surf_num, label_split_num, label_name,
                                 label_delim );
        }
        else
        {
            WriteNURBSSurfsSTEP( step_fn, len_unit, tol, merge_pnts, label_id, label_surf_num, label_split_num, label_name,
                                 label_delim, representation );
        }
    } );
}

void SurfaceIntersectionSingleton::WriteIGESFile( const string& filename, int len_unit,
                                                  bool label_id, bool label_surf_num, bool label_split_num,
                                                  bool label_name, const string &label_delim )
{
    BuildNURBSSurfMap();

    WriteNURBSSurfsIGES( filename, len_unit, label_id, label_surf_num, label_split_num, label_name, label_delim );
}

void SurfaceIntersectionSingleton::WriteNURBSSurfsIGES( const string& filename, int len_unit,
                                                        bool label_id, bool label_surf_num, bool label_split_num,
                                                        bool label_name, const string &label_delim )
{
    WriteNURBSSurfsIGES( m_NURBSSurfVec, filename, len_unit, label_id, label_surf_num, label_split_num, label_name, label_delim );
}

void SurfaceIntersectionSingleton::WriteNURBSSurfsIGES( vector < NURBS_Surface > &surf_vec, const string& filename, int len_unit,
                                                        bool label_id, bool label_surf_num, bool label_split_num,
                                                        bool label_name, const string &label_delim )
{
    if ( surf_vec.size() == 0 )
    {
        addOutputText( "Error: Can't Export IGES - No Valid Surfaces\n" );
        return;
    }

    IGESutil iges( len_unit );

    for ( size_t si = 0; si < surf_vec.size(); si++ )
    {
        string label = CADSurfLabel( surf_vec[si], label_id, label_surf_num, label_split_num, label_name, label_delim );

        // A surface no complete loop bounds is no face, as in STEP; written anyway it would be
        // a whole untrimmed surface
        if ( !surf_vec[si].HasClosedExtLoop() )
        {
            printf( "ERROR: No complete IGES boundary on surface %d, which is left out\n", surf_vec[si].m_SurfID );
            continue;
        }

        DLL_IGES_ENTITY_128 isurf = surf_vec[si].WriteIGESSurf( &iges, label.c_str() );

        surf_vec[si].WriteIGESLoops( &iges, isurf, label.c_str() );
    }

    iges.WriteFile( filename, true );
}

void SurfaceIntersectionSingleton::WriteSTEPFile( const string& filename, int len_unit, double tol,
                                                  bool merge_pnts, bool label_id, bool label_surf_num, bool label_split_num,
                                                  bool label_name, const string &label_delim, int representation )
{
    BuildNURBSSurfMap();

    WriteNURBSSurfsSTEP( filename, len_unit, tol, merge_pnts, label_id, label_surf_num, label_split_num, label_name,
                         label_delim, representation );
}

void SurfaceIntersectionSingleton::WriteNURBSSurfsSTEP( const string& filename, int len_unit, double tol,
                                                        bool merge_pnts, bool label_id, bool label_surf_num, bool label_split_num,
                                                        bool label_name, const string &label_delim, int representation )
{
    STEPutil step( len_unit, tol );

    if ( m_NURBSSurfVec.size() == 0 )
    {
        addOutputText( "Error: Can't Export STEP - No Valid Surfaces\n" );
        return;
    }

    STEP_Topology topo( m_NURBSCurveVec, m_NURBSSurfVec );

    // Every surface is written before any face, so an edge can be placed on both of its surfaces
    // whichever face asks for it first
    vector < SdaiSurface* > surf_vec( m_NURBSSurfVec.size() );
    vector < string > label_vec( m_NURBSSurfVec.size() );

    for ( size_t si = 0; si < m_NURBSSurfVec.size(); si++ )
    {
        string label = CADSurfLabel( m_NURBSSurfVec[si], label_id, label_surf_num, label_split_num, label_name, label_delim );

        surf_vec[si] = m_NURBSSurfVec[si].WriteSTEPSurf( &step, label, merge_pnts );
        label_vec[si] = label;

        topo.SetSurf( m_NURBSSurfVec[si].m_SurfID, surf_vec[si] );
    }

    // The faces of normal and negative surfaces, which can bound a solid, and the rest
    vector < SdaiAdvanced_face* > adv_vec[2];
    vector < int > face_vec[2];

    for ( size_t si = 0; si < m_NURBSSurfVec.size(); si++ )
    {
        int type = m_NURBSSurfVec[si].m_SurfType;
        int ikind = 1;
        if ( type == vsp::CFD_NORMAL || type == vsp::CFD_NEGATIVE )
        {
            ikind = 0;
        }

        vector < int > ids;
        vector < SdaiAdvanced_face* > adv = m_NURBSSurfVec[si].WriteSTEPLoops( &step, &topo, surf_vec[si], ids, label_vec[si], merge_pnts );
        adv_vec[ikind].insert( adv_vec[ikind].end(), adv.begin(), adv.end() );
        face_vec[ikind].insert( face_vec[ikind].end(), ids.begin(), ids.end() );
    }

    // Each kind in shells of faces joined through the edges they share: one body is one shell,
    // however its components were grouped, and bodies that do not touch are shells apart
    vector < vector < SdaiAdvanced_face* > > shell_vec;
    vector < bool > closed_vec;
    int nopen = 0;

    for ( int ikind = 0; ikind < 2; ikind++ )
    {
        vector < vector < int > > shells;
        vector < bool > closed;
        topo.Shells( face_vec[ikind], shells, closed );

        for ( size_t s = 0; s < shells.size(); s++ )
        {
            vector < SdaiAdvanced_face* > shell;
            for ( size_t k = 0; k < shells[s].size(); k++ )
            {
                shell.push_back( adv_vec[ikind][ shells[s][k] ] );
            }

            if ( ikind == 0 && !closed[s] )
            {
                nopen++;
            }

            shell_vec.push_back( shell );
            closed_vec.push_back( closed[s] );
        }
    }

    if ( topo.GetMaxEndGap() > tol )
    {
        char str[256];
        snprintf( str, sizeof( str ), "Warning: STEP curve ends lie up to %g from their shared vertex, more than the tolerance %g\n",
                  topo.GetMaxEndGap(), tol );
        addOutputText( str );
    }

    if ( representation == vsp::STEP_SHELL )
    {
        step.RepresentManifoldShell( shell_vec, closed_vec );
    }
    else
    {
        if ( nopen > 0 )
        {
            char str[256];
            snprintf( str, sizeof( str ), "Warning: %d STEP shells are not closed, and are written as surfaces instead of solids\n", nopen );
            addOutputText( str );
        }

        step.RepresentBREPSolid( shell_vec, closed_vec );
    }

    step.WriteFile( filename );
}

void SurfaceIntersectionSingleton::BuildNURBSSurfMap()
{
    m_NURBSSurfVec.clear();

    // Each surface's loops are built from its own curves alone, so the surfaces are done on
    // several threads, and the ones kept gathered in order after.
    int nsurf = ( int )m_SurfVec.size();
    vector < NURBS_Surface > built( nsurf );
    vector < char > keep( nsurf, 0 );

    RunStage( "Trim Surfaces", nsurf, [&]( int si )
    {
        if ( m_SurfVec[si]->GetIgnoreSurfFlag() )
        {
            return;
        }

        NURBS_Surface &nurbs_surf = built[si];
        // Create NURBS Surface
        nurbs_surf.InitNURBSSurf( m_SurfVec[si] );

        nurbs_surf.m_SurfType = m_SurfVec[si]->GetSurfaceCfdType();
        nurbs_surf.m_WakeFlag = m_SurfVec[si]->GetWakeFlag();
        nurbs_surf.m_FlipFlag = m_SurfVec[si]->GetFlipFlag();

        // Identify all border and intersection NURBS curves on the surface
        vector < NURBS_Curve > nurbs_curve_vec = nurbs_surf.MatchNURBSCurves( m_NURBSCurveVec );
        nurbs_surf.SetNURBSCurveVec( nurbs_curve_vec );

        // Build NURBS loops
        nurbs_surf.BuildNURBSLoopMap();

        if ( nurbs_surf.m_NURBSLoopVec.size() == 1 &&
             nurbs_surf.m_NURBSLoopVec[0].m_BorderLoopFlag &&
             nurbs_surf.m_NURBSLoopVec[0].m_InternalLoopFlag &&
             nurbs_surf.m_SurfType != vsp::CFD_NEGATIVE )
        {
            // Indicates that the surface is completely enclosed.  A normal surface buried
            // inside another component bounds nothing and goes.  A negative one does not:
            // being wholly inside the component it cuts is what makes it the wall of the
            // cavity, which is as real a boundary as the outer skin.  Only a negative
            // surface entirely outside its component can be dropped, and that is the test
            // below.
            return;
        }
        else if ( nurbs_surf.m_NURBSLoopVec.size() == 1 &&
                  !nurbs_surf.m_NURBSLoopVec[0].m_InternalLoopFlag &&
                  nurbs_surf.m_SurfType == vsp::CFD_NEGATIVE )
        {
            return; // external negative surfaces are removed
        }

        keep[si] = 1;
    } );

    int nkeep = 0;
    for ( int si = 0; si < nsurf; si++ )
    {
        nkeep += keep[si];
    }
    m_NURBSSurfVec.reserve( nkeep );

    for ( int si = 0; si < nsurf; si++ )
    {
        if ( keep[si] )
        {
            m_NURBSSurfVec.push_back( std::move( built[si] ) );
        }
    }
}

void SurfaceIntersectionSingleton::BuildNURBSCurvesVec( bool classify, bool cad )
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

    // Whether each chain lies inside another component, and inside a negative one.  The ray
    // tests only read the surfaces' patches, so the chains are taken on several threads.
    vector< ISegChain* > chains( m_ISegChainList.begin(), m_ISegChainList.end() );
    int nchain = ( int )chains.size();
    vector < char > internal_vec( nchain, 0 );
    vector < char > negative_vec( nchain, 0 );

    if ( classify )
    {
        RunStage( "Classify Curves", nchain, [&]( int k )
        {
            ISegChain* chain = chains[k];

            if ( chain->m_PatchJoinFlag )
            {
                return;
            }

            bool internal_flag = false;
            bool in_negative = false;

            // Check if the curve is interenal or external
            // Identify test point
            vec3d cp;
            if ( chain->m_ISegVec.size() <= 2 )
            {
                // Take midpoint of first segment
                cp = ( chain->m_ISegVec[0]->m_IPnt[0]->m_Pnt + chain->m_ISegVec[0]->m_IPnt[1]->m_Pnt ) / 2.0;
            }
            else
            {
                // Identify point approximately halfway on border curve
                cp = chain->m_ISegVec[chain->m_ISegVec.size() / 2]->m_IPnt[0]->m_Pnt;
            }

            // Check 3 directions and take majority result
            vec3d xep = cp + vec3d( x_dist, 1.0e-4, 1.0e-4 );
            vec3d yep = cp + vec3d( 1.0e-4, y_dist, 1.0e-4 );
            vec3d zep = cp + vec3d( 1.0e-4, 1.0e-4, z_dist );

            // Check if the curve is inside any component by checking the number of intersections from 3 vectors
            // beginning at the midpoint of the curve. Checking 3 vectors prevents a false positive or negative
            // from a vector that exactly aligns with another curve
            for ( size_t j = 0; j < m_NumComps; j++ )
            {
                if ( j == chain->m_SurfA->GetCompID() || j == chain->m_SurfB->GetCompID() )
                {
                    continue;
                }

                // Each component's crossings are counted on their own
                vector< double > x_vec, y_vec, z_vec;

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

            if ( internal_flag )
            {
                // Check if inside a negative component
                for ( size_t j = 0; j < m_NumComps; j++ )
                {
                    if ( j == chain->m_SurfA->GetCompID() || j == chain->m_SurfB->GetCompID() )
                    {
                        continue;
                    }

                    vector< double > x_vec, y_vec, z_vec;

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

            internal_vec[k] = internal_flag;
            negative_vec[k] = in_negative;
        } );
    }

    list< ISegChain* >::iterator i_seg;

    bool cached = cad && m_ChainCADVec.size() == m_ISegChainList.size();

    // The chain each curve comes from
    vector< ISegChain* > curve_chain;

    int icurve = 0;
    int ichain = -1;
    for ( i_seg = m_ISegChainList.begin(); i_seg != m_ISegChainList.end(); ++i_seg )
    {
        ichain++;

        // A patch join is a crease for the mesher inside one surface, which the surface
        // carries exactly already
        if ( ( *i_seg )->m_PatchJoinFlag )
        {
            continue;
        }

        bool internal_flag = false, ss_flag = false, wake_flag = false;
        bool in_negative = false;

        if ( classify )
        {
            internal_flag = internal_vec[ ichain ];
            in_negative = negative_vec[ ichain ];
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
        if ( cached )
        {
            nurbs_curve = m_ChainCADVec[ ichain ];
        }

        nurbs_curve.m_BorderFlag = ( *i_seg )->m_BorderFlag;
        nurbs_curve.m_InternalFlag = internal_flag;
        nurbs_curve.m_InsideNegativeFlag = in_negative;
        nurbs_curve.m_SubSurfFlag = ss_flag;
        nurbs_curve.m_WakeFlag = wake_flag;
        nurbs_curve.m_SurfA_Type = ( *i_seg )->m_SurfA->GetSurfaceCfdType();
        nurbs_curve.m_SurfB_Type = ( *i_seg )->m_SurfB->GetSurfaceCfdType();
        nurbs_curve.m_SurfA_ID = ( *i_seg )->m_SurfA->GetSurfID();
        nurbs_curve.m_SurfB_ID = ( *i_seg )->m_SurfB->GetSurfID();
        nurbs_curve.m_CurveID = icurve;
        icurve++;

        m_NURBSCurveVec.push_back( nurbs_curve );
        curve_chain.push_back( *i_seg );
    }

    if ( cached )
    {
        // Built already, for drawing
        return;
    }

    // A curve is adapted on its chain's two surfaces alone, so curves on different surfaces are
    // done on several threads, and the warnings said in order after.
    int ncurve = ( int )m_NURBSCurveVec.size();
    vector< Surf* > sa( ncurve ), sb( ncurve );
    for ( int k = 0 ; k < ncurve ; k++ )
    {
        sa[k] = curve_chain[k]->m_ACurve.GetSurf();
        sb[k] = curve_chain[k]->m_BCurve.GetSurf();
    }
    vector< string > warn( ncurve );

    ProgressStage bar( this, "Adapt Curves", ncurve );

    RunChainsBySurface( sa, sb, [&]( int k )
    {
        if ( cad )
        {
            m_NURBSCurveVec[k].InitCAD( curve_chain[k]->m_ACurve, curve_chain[k]->m_BCurve, GetSettingsPtr()->m_STEPTol, &warn[k] );
        }
        else
        {
            m_NURBSCurveVec[k].InitPolyline( curve_chain[k]->m_ACurve, curve_chain[k]->m_BCurve, GetSettingsPtr()->m_RelCurveTol );
        }
    }, &bar );

    bar.End();

    for ( int k = 0 ; k < ncurve ; k++ )
    {
        printf( "%s", warn[k].c_str() );
    }
}

void SurfaceIntersectionSingleton::BuildCurves()
{
    // A chain's curves are made from its own points alone, so the chains are done on several
    // threads.
    vector< ISegChain* > chains( m_ISegChainList.begin(), m_ISegChainList.end() );
    int n = ( int )chains.size();
    RunStage( "Build Curves", n, [&]( int k )
    {
        chains[k]->BuildCurves();
    } );
}

// Intersect every pair of surfaces, in two passes.
//
// The first pass decides which pairs are worth the patch work.  It has to run on one thread:
// deciding involves projecting one surface's border curves onto the other, and an evaluation
// writes scratch buffers that belong to the surface, so two threads asking about the same
// It is also the cheap pass.
//
// The second pass walks the patch trees, which is the expensive part and touches the surfaces
// only through their control points.  Pairs are handed out one at a time rather than in fixed
// shares, because on a real model nearly every pair is rejected on its bounding box and the
// few that survive are not evenly spread.  Each pair writes into its own bucket; the buckets
// are folded back in pair order, so the result does not depend on which thread ran what.
void SurfaceIntersectionSingleton::IntersectPairs()
{
    char str[256];

    int n = ( int )m_SurfVec.size();

    FindCoplanarSurfs();

    vector < pair < int, int > > work;

    BeginProgress( "Intersect scan", n, VOCAL_OUTPUT );

    for ( int i = 0 ; i < n ; i++ )
    {
        for ( int j = i + 1 ; j < n ; j++ )
        {
            if ( m_SurfVec[i]->IntersectPrepare( m_SurfVec[j], this ) )
            {
                work.push_back( pair< int, int >( i, j ) );
            }
        }

        m_ProgressDone++;
        StepProgress( VOCAL_OUTPUT );
    }

    EndProgress( VOCAL_OUTPUT );

    // One piece of work for each patch of a pair's first surface, rather than one for the pair:
    // a single pair can cost as much as all the others together, and as one piece it would
    // leave every other thread waiting on it.  The pieces are taken in order and their output
    // gathered in that order, so the result is the same however they are run.
    vector < pair < int, int > > piece;
    vector < int > patch_vec;

    for ( int k = 0 ; k < ( int )work.size() ; k++ )
    {
        m_SurfVec[ work[k].first ]->FindIntersectPatches( m_SurfVec[ work[k].second ], patch_vec );

        for ( int i = 0 ; i < ( int )patch_vec.size() ; i++ )
        {
            piece.push_back( pair< int, int >( k, patch_vec[i] ) );
        }
    }

    int nwork = ( int )piece.size();

    vector < IsectOutput > out( nwork );

    RunStage( "Intersect", nwork, [&]( int k )
    {
        tl_isect_out = &out[k];

        const pair < int, int > &surfs = work[ piece[k].first ];
        m_SurfVec[ surfs.first ]->IntersectPatch( piece[k].second, m_SurfVec[ surfs.second ], this );

        tl_isect_out = nullptr;
    } );

    // The pieces' output is gathered in one go, so each list is sized once for all of it.
    size_t npuw = m_DelPuwVec.size();
    size_t nipnt = 0;
    for ( int k = 0 ; k < nwork ; k++ )
    {
        npuw += out[k].m_Puws.size();
        nipnt += out[k].m_IPnts.size();
    }
    m_DelPuwVec.reserve( npuw );
    m_DelIPntVec.reserve( m_DelIPntVec.size() + nipnt );
    m_AllIPnts.reserve( m_AllIPnts.size() + nipnt );
    m_AllIPntEdges.reserve( m_AllIPntEdges.size() + nipnt );

    for ( int k = 0 ; k < nwork ; k++ )
    {
        m_DelPuwVec.insert( m_DelPuwVec.end(), out[k].m_Puws.begin(), out[k].m_Puws.end() );
        m_DelIPntVec.insert( m_DelIPntVec.end(), out[k].m_IPnts.begin(), out[k].m_IPnts.end() );
        for ( size_t j = 0 ; j < out[k].m_IPnts.size() ; j++ )
        {
            out[k].m_IPnts[j]->m_AllIndex = ( int )m_AllIPnts.size();
            m_AllIPnts.push_back( out[k].m_IPnts[j] );
        }
        m_AllIPntEdges.insert( m_AllIPntEdges.end(), out[k].m_Edges.begin(), out[k].m_Edges.end() );
        m_IPatchADrawLines.insert( m_IPatchADrawLines.end(), std::make_move_iterator( out[k].m_PatchADraw.begin() ), std::make_move_iterator( out[k].m_PatchADraw.end() ) );
        m_IPatchBDrawLines.insert( m_IPatchBDrawLines.end(), std::make_move_iterator( out[k].m_PatchBDraw.begin() ), std::make_move_iterator( out[k].m_PatchBDraw.end() ) );

        // Gathered, so the piece's own lists go now rather than with the rest at the end.
        out[k] = IsectOutput();
    }
}

void SurfaceIntersectionSingleton::Intersect()
{
    char str[256];
    int n = m_SurfVec.size();

    // Always called; the switch is applied inside, to the user's subsurfaces only.
    BuildSubSurfIntChains();

    //==== Quad Tree Intersection - Intersection Segments Get Loaded at AddIntersectionSeg ===//
    IntersectPairs();

    // WriteISegs();

    BuildChains();
    // DebugWriteChains( "BuildChains", false );

    CleanChains();
    // DebugWriteChains( "CleanChains", false );

    RefineChains();
    // DebugWriteChains( "RefineChains", false );

    MergeFeaPartSSEdgeOverlap(); // Only applicable to FEA Mesh

    addOutputText( "LoadBorderCurves\n" );
    LoadBorderCurves();
    // DebugWriteChains( "LoadBorderCurves", false );

    MergeInteriorChainIPnts();
    // DebugWriteChains( "MergeInteriorChainIPnts", false );

    SetFixPointBorderNodes(); // Only applicable to FEA Mesh
    CheckFixPointIntersects(); // Only applicable to FEA Mesh

    SplitCoplanarBorders();

    addOutputText( "SplitBorderCurves\n" );
    SplitBorderCurves();
    // DebugWriteChains( "SplitBorderCurves", false );

    addOutputText( "IntersectSplitChains\n" );
    IntersectSplitChains();
    // DebugWriteChains( "IntersectSplitChains", false );

    addOutputText( "SplitNearlyTouchingChains\n" );
    SplitNearlyTouchingChains();

    BuildCurves();
    // DebugWriteChains( "BuildCurves", false );

    DeleteUnusedIPnts();
}

// Delete the items of v not marked to keep, and keep the rest in order.  There can be millions,
// each a reach to somewhere else in memory, so the list is walked in pieces on several threads.
template < class T >
static void DeleteUnkept( SurfaceIntersectionSingleton &mgr, vector< T* > &v, ProgressStage &bar )
{
    int n = ( int )v.size();
    const int piece = 65536;
    int npiece = ( n + piece - 1 ) / piece;
    vector< vector< T* > > kept( npiece );

    mgr.RunIndexed( npiece, mgr.StageThreadCount( npiece ), [&]( int k )
    {
        int iend = std::min( n, ( k + 1 ) * piece );
        for ( int i = k * piece ; i < iend ; i++ )
        {
            if ( v[i]->m_KeepFlag )
            {
                kept[k].push_back( v[i] );
            }
            else
            {
                delete v[i];
            }
        }

        bar.Step( iend - k * piece );
    } );

    size_t nkeep = 0;
    for ( int k = 0 ; k < npiece ; k++ )
    {
        nkeep += kept[k].size();
    }

    vector< T* > keep;
    keep.reserve( nkeep );
    for ( int k = 0 ; k < npiece ; k++ )
    {
        keep.insert( keep.end(), kept[k].begin(), kept[k].end() );
    }
    v.swap( keep );
}

// Delete the intersection points, and the surface points, that no chain uses.  Merging a chain's
// interior points keeps one of each pair that meet, and the chains that were cleaned away leave
// theirs; nothing reaches either again.
void SurfaceIntersectionSingleton::DeleteUnusedIPnts()
{
    for ( list< ISegChain* >::const_iterator c = m_ISegChainList.begin(); c != m_ISegChainList.end(); ++c )
    {
        const vector< ISeg* > &segs = ( *c )->m_ISegVec;
        for ( size_t i = 0; i < segs.size(); i++ )
        {
            KeepIPnt( segs[i]->m_IPnt[0] );
            KeepIPnt( segs[i]->m_IPnt[1] );
        }
        const vector< IPnt* > &tess = ( *c )->m_TessVec;
        for ( size_t i = 0; i < tess.size(); i++ )
        {
            KeepIPnt( tess[i] );
        }
    }

    ProgressStage bar( this, "Delete IPnts", ( int )( m_DelIPntVec.size() + m_DelPuwVec.size() ) );

    DeleteUnkept( *this, m_DelIPntVec, bar );
    DeleteUnkept( *this, m_DelPuwVec, bar );
    bar.End();

    // It held every point the patches found, and only building the chains reads it
    vector< IPnt* >().swap( m_AllIPnts );
}

// Mark a point a chain uses, and its surface points.  Only building the chains reads its segments.
void SurfaceIntersectionSingleton::KeepIPnt( IPnt* ip )
{
    if ( ip->m_KeepFlag )
    {
        return;
    }
    ip->m_KeepFlag = true;
    for ( size_t i = 0; i < ip->m_Puws.size(); i++ )
    {
        ip->m_Puws[i]->m_KeepFlag = true;
    }
    vector< ISeg* >().swap( ip->m_Segs );
}

// The corners at the ends of edge i of triangle tri.  Tri 1 is corners 0 2 3, tri 2 is corners
// 0 1 2.  Corner 0 is (umin, wmin), 1 (umax, wmin), 2 (umax, wmax), 3 (umin, wmax); the diagonal
// runs from 0 to 2.
static void TriEdgeCorners( int tri, int i, int &a, int &b )
{
    const int tri1[3][2] = { { 0, 2 }, { 2, 3 }, { 3, 0 } };
    const int tri2[3][2] = { { 0, 1 }, { 1, 2 }, { 0, 2 } };
    if ( tri == 2 )
    {
        a = tri2[i][0];
        b = tri2[i][1];
    }
    else
    {
        a = tri1[i][0];
        b = tri1[i][1];
    }
}

// How far point pnt is from edge i of triangle tri, corners q, and how far along it from its first corner
static double EdgeDist( const vec3d &pnt, const vec3d q[4], int tri, int i, double &t )
{
    int a, b;
    TriEdgeCorners( tri, i, a, b );
    const vec3d &c0 = q[ a ];
    const vec3d &c1 = q[ b ];
    vec3d d = c1 - c0;
    double dd = dot( d, d );
    t = 0.0;
    if ( dd > 0.0 )
    {
        t = std::min( std::max( dot( pnt - c0, d ) / dd, 0.0 ), 1.0 );
    }
    return dist( pnt, c0 + d * t );
}

// The edge of triangle tri, corners q, that a segment from ip0 to ip1 lies along, or -1.  Both ends
// lie on it to within the roundoff of their coordinates.
static int AlongEdge( const vec3d &ip0, const vec3d &ip1, const vec3d q[4], int tri )
{
    for ( int i = 0; i < 3; i++ )
    {
        int a, b;
        TriEdgeCorners( tri, i, a, b );
        double tol = 1.0e3 * DBL_EPSILON * ( q[ a ].mag() + q[ b ].mag() );

        double t;
        if ( EdgeDist( ip0, q, tri, i, t ) <= tol && EdgeDist( ip1, q, tri, i, t ) <= tol )
        {
            return i;
        }
    }
    return -1;
}

// The edge of triangle tri of patch p, corners q, that point pnt is nearest, and how near it is.
// Edge skip of the triangle is passed over.
static double NearestEdge( const vec3d &pnt, const SurfPatch &p, const vec3d q[4], int tri, int skip, IPntEdge &edge )
{
    double best = 1.0e300;
    for ( int i = 0; i < 3; i++ )
    {
        if ( i == skip )
        {
            continue;
        }

        double t;
        double dst = EdgeDist( pnt, q, tri, i, t );

        if ( dst >= best )
        {
            continue;
        }
        best = dst;

        edge = IPntEdge();
        edge.m_SurfID = p.get_surf_ptr()->GetSurfID();

        int a, b;
        TriEdgeCorners( tri, i, a, b );
        double du = p.get_u_max() - p.get_u_min();
        double dw = p.get_w_max() - p.get_w_min();

        if ( a == 0 && b == 2 )
        {
            edge.m_Kind = IPntEdge::DIAGONAL;
            edge.m_Val[0] = p.get_u_min();
            edge.m_Val[1] = p.get_u_max();
            edge.m_Val[2] = p.get_w_min();
            edge.m_Val[3] = p.get_w_max();
            edge.m_Side = tri;
            edge.m_Along = t;
        }
        else if ( a == 0 && b == 1 )
        {
            edge.m_Kind = IPntEdge::W_LINE;
            edge.m_Val[0] = p.get_w_min();
            edge.m_Side = 1;
            edge.m_Along = p.get_u_min() + t * du;
        }
        else if ( a == 2 && b == 3 )
        {
            edge.m_Kind = IPntEdge::W_LINE;
            edge.m_Val[0] = p.get_w_max();
            edge.m_Side = -1;
            edge.m_Along = p.get_u_max() - t * du;
        }
        else if ( a == 1 && b == 2 )
        {
            edge.m_Kind = IPntEdge::U_LINE;
            edge.m_Val[0] = p.get_u_max();
            edge.m_Side = -1;
            edge.m_Along = p.get_w_min() + t * dw;
        }
        else
        {
            edge.m_Kind = IPntEdge::U_LINE;
            edge.m_Val[0] = p.get_u_min();
            edge.m_Side = 1;
            edge.m_Along = p.get_w_max() - t * dw;
        }
    }
    return best;
}

// The edge a segment's end lies on: of the two triangles it was found between, the edge nearest
// it.  An edge of one triangle that the whole segment lies along is crossed nowhere, so it is
// passed over: the segment ends where it leaves the other triangle, or at a corner of this one.
static IPntEdge FindEdge( const vec3d &pnt, const SurfPatch &pA, const vec3d qa[4], int triA, int skipA,
                          const SurfPatch &pB, const vec3d qb[4], int triB, int skipB )
{
    IPntEdge ea, eb;
    double da = NearestEdge( pnt, pA, qa, triA, skipA, ea );
    double db = NearestEdge( pnt, pB, qb, triB, skipB, eb );

    if ( da <= db )
    {
        return ea;
    }
    return eb;
}

int QuadPairPnts::Find( const SurfPatch& pA, const SurfPatch& pB, const vec3d &p )
{
    for ( int k = 0 ; k < m_N ; k++ )
    {
        if ( memcmp( m_Pnt[k].v, p.v, sizeof( p.v ) ) == 0 )
        {
            return k;
        }
    }

    assert( m_N < MAX_PNTS );

    int k = m_N;
    m_N++;
    m_Pnt[k] = p;
    pA.find_closest_uw_planar_approx( p, m_PlaneA[k].v );
    pB.find_closest_uw_planar_approx( p, m_PlaneB[k].v );
    m_Projected[k] = false;
    return k;
}

void QuadPairPnts::Project( const SurfPatch& pA, const SurfPatch& pB, int k )
{
    if ( !m_Projected[k] )
    {
        pA.find_closest_uw( m_Pnt[k], m_PlaneA[k].v, m_ProjA[k].v );
        pB.find_closest_uw( m_Pnt[k], m_PlaneB[k].v, m_ProjB[k].v );
        m_Projected[k] = true;
    }
}

void SurfaceIntersectionSingleton::AddIntersectionSeg( const SurfPatch& pA, const SurfPatch& pB, const vec3d & ip0, const vec3d & ip1,
                                                       const vec3d qa[4], const vec3d qb[4], int triA, int triB, QuadPairPnts &pnts )
{
    double d = dist_squared( ip0, ip1 );
    if ( d < DBL_EPSILON )
    {
        return;
    }

    int k0 = pnts.Find( pA, pB, ip0 );
    int k1 = pnts.Find( pA, pB, ip1 );

    const vec2d &plane_uwA0 = pnts.m_PlaneA[k0];
    const vec2d &plane_uwB0 = pnts.m_PlaneB[k0];
    const vec2d &plane_uwA1 = pnts.m_PlaneA[k1];
    const vec2d &plane_uwB1 = pnts.m_PlaneB[k1];

    // Intersections that lie exactly on a patch boundary will actually intersect both patches
    // that share that boundary.  So, detect intersections that lie on the patch minimum edge
    // and don't carry those forward.  Don't do this if the minimum parameter is zero.  I.e.
    // there is no prior patch.
    //
    // Skip this dedup for FEA structure×structure pairs. After Y-mirror (FeaSymmIndex), planar
    // part intersections (e.g. XY floor × poly-spar web) can be reported only on the w_min/u_min
    // side of a split while the w_max twin is missed — dropping the min-edge copy then deletes
    // the entire chain on one symmetry side.
    bool skip_min_edge_dedup = false;
    if ( pA.get_surf_ptr() && pB.get_surf_ptr() &&
         pA.get_surf_ptr()->GetSurfaceCfdType() == vsp::CFD_STRUCTURE &&
         pB.get_surf_ptr()->GetSurfaceCfdType() == vsp::CFD_STRUCTURE )
    {
        skip_min_edge_dedup = true;
    }

    double tol = 1e-10; // Tolerance buildup due to SurfPatch::find_closest_uw_planar_approx and other inaccuracies

    if ( !skip_min_edge_dedup )
    {
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
    }

    // A segment along an edge of each triangle lies where the two edges coincide, and its ends
    // keep the edges nearest them.
    int alongA = AlongEdge( ip0, ip1, qa, triA );
    int alongB = AlongEdge( ip0, ip1, qb, triB );
    if ( alongA >= 0 && alongB >= 0 )
    {
        alongA = -1;
        alongB = -1;
    }

    pnts.Project( pA, pB, k0 );
    pnts.Project( pA, pB, k1 );

    vec2d proj_uwA0 = pnts.m_ProjA[k0];
    vec2d proj_uwB0 = pnts.m_ProjB[k0];
    vec2d proj_uwA1 = pnts.m_ProjA[k1];
    vec2d proj_uwB1 = pnts.m_ProjB[k1];

    Puw* puwA0 = new Puw( pA.get_surf_ptr(), proj_uwA0 );
    tl_isect_out->m_Puws.push_back( puwA0 );

    Puw* puwB0 = new Puw( pB.get_surf_ptr(), proj_uwB0 );
    tl_isect_out->m_Puws.push_back( puwB0 );

    IPnt* ipnt0 = new IPnt( puwA0, puwB0 );
    ipnt0->m_Pnt = ip0;
    tl_isect_out->m_IPnts.push_back( ipnt0 );
    tl_isect_out->m_Edges.push_back( FindEdge( ip0, pA, qa, triA, alongA, pB, qb, triB, alongB ) );

    Puw* puwA1 = new Puw( pA.get_surf_ptr(), proj_uwA1 );
    tl_isect_out->m_Puws.push_back( puwA1 );

    Puw* puwB1 = new Puw( pB.get_surf_ptr(), proj_uwB1 );
    tl_isect_out->m_Puws.push_back( puwB1 );

    IPnt* ipnt1 = new IPnt( puwA1, puwB1 );
    ipnt1->m_Pnt = ip1;
    tl_isect_out->m_IPnts.push_back( ipnt1 );
    tl_isect_out->m_Edges.push_back( FindEdge( ip1, pA, qa, triA, alongA, pB, qb, triB, alongB ) );

    // Identify rectangles to represent final patches
    if ( DEBUG_ISECT_DRAW )
    {
        tl_isect_out->m_PatchADraw.push_back( pA.GetPatchDrawLines() );
        tl_isect_out->m_PatchBDraw.push_back( pB.GetPatchDrawLines() );
    }

    new ISeg( pA.get_surf_ptr(), pB.get_surf_ptr(), ipnt0, ipnt1 );
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
#ifdef DEBUG_CFD_MESH
    int i;
    char str2[256];
    snprintf( str2, sizeof( str2 ), "%sISegs.m", m_DebugDir.c_str() );
    FILE* fp = fopen( str2, "w" );

    fprintf( fp, "clear all;\n" );
    fprintf( fp, "close all;\n" );

    ISeg* seg;

    fprintf( fp, "x=[" );
    for ( i = 0; i < (int)m_AllIPnts.size()-1; i++ )
    {
        seg = m_AllIPnts[i]->m_Segs[0];
        fprintf( fp, "%.19e %.19e;\n", seg->m_IPnt[0]->m_Pnt.x(), seg->m_IPnt[1]->m_Pnt.x() );
    }
    seg = m_AllIPnts[i]->m_Segs[0];
    fprintf( fp, "%.19e %.19e];\n\n", seg->m_IPnt[0]->m_Pnt.x(), seg->m_IPnt[1]->m_Pnt.x() );

    fprintf( fp, "y=[" );
    for ( i = 0; i < (int)m_AllIPnts.size()-1; i++ )
    {
        seg = m_AllIPnts[i]->m_Segs[0];
        fprintf( fp, "%.19e %.19e;\n", seg->m_IPnt[0]->m_Pnt.y(), seg->m_IPnt[1]->m_Pnt.y() );
    }
    seg = m_AllIPnts[i]->m_Segs[0];
    fprintf( fp, "%.19e %.19e];\n\n", seg->m_IPnt[0]->m_Pnt.y(), seg->m_IPnt[1]->m_Pnt.y() );

    fprintf( fp, "z=[" );
    for ( i = 0; i < (int)m_AllIPnts.size()-1; i++ )
    {
        seg = m_AllIPnts[i]->m_Segs[0];
        fprintf( fp, "%.19e %.19e;\n", seg->m_IPnt[0]->m_Pnt.z(), seg->m_IPnt[1]->m_Pnt.z() );
    }
    seg = m_AllIPnts[i]->m_Segs[0];
    fprintf( fp, "%.19e %.19e];\n\n", seg->m_IPnt[0]->m_Pnt.z(), seg->m_IPnt[1]->m_Pnt.z() );

    fprintf( fp, "plot3( x', y', z' );\n" );
    fprintf( fp, "axis equal;\n" );
    fprintf( fp, "axis off;\n" );

    fclose( fp );
#endif
}

// Partner each intersection point with the one across the triangle edge it lies on.  Points are
// taken by the two surfaces they are on and by their edge, in order along it: each crossing of
// the edge by the curve leaves one point on either side, which are partners.
struct IPntEdgeEntry
{
    int m_SurfLo, m_SurfHi;
    int m_Index;                // In m_AllIPnts
    const IPntEdge* m_Edge;

    bool SameEdge( const IPntEdgeEntry &o ) const
    {
        const IPntEdge &a = *m_Edge;
        const IPntEdge &b = *o.m_Edge;
        return m_SurfLo == o.m_SurfLo && m_SurfHi == o.m_SurfHi && a.m_SurfID == b.m_SurfID && a.m_Kind == b.m_Kind &&
               a.m_Val[0] == b.m_Val[0] && a.m_Val[1] == b.m_Val[1] && a.m_Val[2] == b.m_Val[2] && a.m_Val[3] == b.m_Val[3];
    }

    bool operator<( const IPntEdgeEntry &o ) const
    {
        const IPntEdge &a = *m_Edge;
        const IPntEdge &b = *o.m_Edge;
        if ( m_SurfLo != o.m_SurfLo ) return m_SurfLo < o.m_SurfLo;
        if ( m_SurfHi != o.m_SurfHi ) return m_SurfHi < o.m_SurfHi;
        if ( a.m_SurfID != b.m_SurfID ) return a.m_SurfID < b.m_SurfID;
        if ( a.m_Kind != b.m_Kind ) return a.m_Kind < b.m_Kind;
        for ( int k = 0; k < 4; k++ )
        {
            if ( a.m_Val[k] != b.m_Val[k] ) return a.m_Val[k] < b.m_Val[k];
        }
        return a.m_Along < b.m_Along;
    }
};

void SurfaceIntersectionSingleton::LinkIPntPartners()
{
    vector< IPntEdgeEntry > entries;
    entries.reserve( m_AllIPnts.size() );
    m_AllIPntPartner.assign( m_AllIPnts.size(), -1 );

    for ( size_t i = 0; i < m_AllIPnts.size(); i++ )
    {
        IPnt* ip = m_AllIPnts[i];

        if ( m_AllIPntEdges[i].m_Kind == IPntEdge::NONE || ip->m_Puws.size() != 2 )
        {
            continue;
        }

        int sa = ip->m_Puws[0]->m_Surf->GetSurfID();
        int sb = ip->m_Puws[1]->m_Surf->GetSurfID();

        IPntEdgeEntry e;
        e.m_SurfLo = std::min( sa, sb );
        e.m_SurfHi = std::max( sa, sb );
        e.m_Index = ( int )i;
        e.m_Edge = &m_AllIPntEdges[i];
        entries.push_back( e );
    }

    std::sort( entries.begin(), entries.end() );

    // Neighbors along an edge on opposite sides of it, close enough together for one crossing of
    // the edge: within ten times the longer of their segments.  Anything else is left to the
    // search.
    size_t i = 0;
    while ( i + 1 < entries.size() )
    {
        IPnt* a = m_AllIPnts[ entries[i].m_Index ];
        IPnt* b = m_AllIPnts[ entries[i + 1].m_Index ];

        bool partners = false;
        if ( entries[i].SameEdge( entries[i + 1] ) && entries[i].m_Edge->m_Side != entries[i + 1].m_Edge->m_Side )
        {
            double seglen = 0.0;
            if ( !a->m_Segs.empty() )
            {
                seglen = std::max( seglen, dist( a->m_Segs[0]->m_IPnt[0]->m_Pnt, a->m_Segs[0]->m_IPnt[1]->m_Pnt ) );
            }
            if ( !b->m_Segs.empty() )
            {
                seglen = std::max( seglen, dist( b->m_Segs[0]->m_IPnt[0]->m_Pnt, b->m_Segs[0]->m_IPnt[1]->m_Pnt ) );
            }
            partners = dist( a->m_Pnt, b->m_Pnt ) <= 10.0 * seglen;
        }

        if ( partners )
        {
            m_AllIPntPartner[ entries[i].m_Index ] = entries[i + 1].m_Index;
            m_AllIPntPartner[ entries[i + 1].m_Index ] = entries[i].m_Index;
            i += 2;
        }
        else
        {
            i++;
        }
    }
}

void SurfaceIntersectionSingleton::BuildChains()
{
    // The points are copied out in pieces on several threads: each copy reaches into a point of
    // its own, scattered through memory, and on a large model there are millions.
    int npnt = ( int )m_AllIPnts.size();
    PntNodeCloud i_pnt_cloud;
    i_pnt_cloud.m_PntNodes.resize( npnt );

    const int piece = 65536;
    int npiece = ( npnt + piece - 1 ) / piece;

    // The pieces, the tree and the partners, then every point walked into chains.
    ProgressStage bar( this, "Build Chains", npiece + 2 + npnt );

    RunIndexed( npiece, StageThreadCount( npiece ), [&]( int k )
    {
        int iend = std::min( npnt, ( k + 1 ) * piece );
        for ( int i = k * piece ; i < iend ; i++ )
        {
            i_pnt_cloud.m_PntNodes[i].m_Pnt = m_AllIPnts[i]->m_Pnt;
        }
    }, &bar );

    // The tree is built, by its constructor, while the partners are linked; neither touches
    // what the other writes.  The build takes every thread but the one linking.
    int ntree = StageThreadCount( std::numeric_limits< int >::max() ) - 1;
    if ( ntree < 1 )
    {
        ntree = 1;
    }
    std::unique_ptr< PNTree > index;
    RunIndexed( 2, StageThreadCount( 2 ), [&]( int k )
    {
        if ( k == 0 )
        {
            index.reset( new PNTree( 3, i_pnt_cloud, KDTreeSingleIndexAdaptorParams( 10, KDTreeSingleIndexAdaptorFlags::None, ntree ) ) );
        }
        else
        {
            LinkIPntPartners();
        }
    }, &bar );

    for ( int i = 0; i < (int)m_AllIPnts.size(); i++ )
    {
        bar.Step();

        if ( !m_AllIPnts[i]->m_UsedFlag && m_AllIPnts[i]->m_Segs.size() > 0 )
        {
            ISeg* seg = m_AllIPnts[i]->m_Segs[0];
            seg->m_IPnt[0]->m_UsedFlag = true;
            seg->m_IPnt[1]->m_UsedFlag = true;
            ISegChain* chain = new ISegChain;           // Create New Chain
            chain->m_SurfA = seg->m_SurfA;
            chain->m_SurfB = seg->m_SurfB;
            chain->m_ISegVec.push_back( seg );
            ExpandChain( chain, index.get() );
            if ( chain->Valid() )
            {
                m_ISegChainList.push_back( chain );
            }
            else
            {
                delete chain;
                chain = nullptr;
            }
        }
    }

    // The chains are built, and nothing past them asks what edge a point was on.
    vector < IPntEdge >().swap( m_AllIPntEdges );
    vector < int >().swap( m_AllIPntPartner );
}

void SurfaceIntersectionSingleton::CleanChains()
{
    vector< ISegChain* > chains;
    vector< Surf* > sa;
    list< ISegChain* >::iterator c;
    for ( c = m_ISegChainList.begin(); c != m_ISegChainList.end(); ++c )
    {
        if ( ( *c )->m_SurfA == ( *c )->m_SurfB )
        {
            // Do not clean SubSurface curves
            continue;
        }

        chains.push_back( *c );
        sa.push_back( ( *c )->m_SurfA );
    }

    // Cleaning a chain evaluates its A surface only and changes only its own segments.
    ProgressStage bar( this, "Clean Chains", ( int )chains.size() );

    RunBySurface( sa, [&]( int k )
    {
        CleanChain( chains[k] );
    }, &bar );
}

void SurfaceIntersectionSingleton::CleanChain( ISegChain* c )
{
    if ( c->m_ISegVec.size() < 2 )
    {
        return;
    }

    int j;
    double tol = 1e-3;


    // Calculate segment lengths and chain length.
    double s = 0;
    list < double > ds;
    list < ISeg* > segList;
    for ( j = 0; j < ( int ) c->m_ISegVec.size(); j++ )
    {
        segList.push_back( c->m_ISegVec[j] );

        vec2d uw1 = c->m_ISegVec[j]->m_IPnt[0]->GetPuw( c->m_SurfA )->m_UW;
        vec3d pt1 = c->m_ISegVec[j]->m_IPnt[0]->GetPuw( c->m_SurfA )->m_Surf->CompPnt( uw1[0], uw1[1] );
        vec2d uw2 = c->m_ISegVec[j]->m_IPnt[1]->GetPuw( c->m_SurfA )->m_UW;
        vec3d pt2 = c->m_ISegVec[j]->m_IPnt[1]->GetPuw( c->m_SurfA )->m_Surf->CompPnt( uw2[0], uw2[1] );
        double d = dist( pt1, pt2 );
        s += d;
        ds.push_back( d );
    }

    // Average segment length.
    double save = s / c->m_ISegVec.size();


    list< ISeg* >::iterator sit;
    list< double >::iterator dsit;
    for ( sit = segList.begin(), dsit = ds.begin(); sit != segList.end(); ++sit, ++dsit )
    {
        if ( segList.size() > 1 && ( (*dsit) / save) < tol ) // Small segment detected
        {
            // Iterators for first and second segments to merge together.
            list< ISeg* >::iterator sfirstit;
            list< ISeg* >::iterator ssecondit;

            list< double >::iterator dsfirstit;
            list< double >::iterator dssecondit;

            list< ISeg* >::iterator snextit = next( sit );

            if ( sit == segList.begin() ) // First element, must merge with next.
            {
                sfirstit = sit;
                ssecondit = next( sfirstit );

                dsfirstit = dsit;
                dssecondit = next( dsfirstit );
            }
            else if ( snextit == segList.end() ) // Last element, must merge with previous.
            {
                ssecondit = sit;
                sfirstit = prev( ssecondit );

                dssecondit = dsit;
                dsfirstit = prev( dssecondit );
            }
            else // Interior elements, can go either way.
            {
                if ( true ) // Always choose forward merge for now, make smart choice later.
                {
                    sfirstit = sit;
                    ssecondit = next( sfirstit );

                    dsfirstit = dsit;
                    dssecondit = next( dsfirstit );
                }
                else
                {
                    ssecondit = sit;
                    sfirstit = prev( ssecondit );

                    dssecondit = dsit;
                    dsfirstit = prev( dssecondit );
                }
            }

            // Merge segments.
            (*ssecondit)->m_IPnt[1]->RemoveSegRef( *ssecondit );
            (*sfirstit)->m_IPnt[1] = (*ssecondit)->m_IPnt[1];
            (*sfirstit)->m_IPnt[1]->AddSegRef( *sfirstit );
            delete (*ssecondit);

            // Add lengths.
            (*dsfirstit) += (*dssecondit);


            // Remove ssecondit, but ssecondit will become invalid upon removal.  So, before removal, we need
            // to check if sit is sseconit and if so set sit to something that will continue working for the
            // next iteration.  In the normal case, the previous iterator works as the for loop will soon
            // increment to the next iterator.  However, we must be careful not to increment before the start
            // of the list.  In that case, we actually skip checking one entry.

            if ( sit == ssecondit )
            {
                if ( sit == segList.begin() )
                {
                    ++sit;
                    ++dsit;
                }
                else
                {
                    --sit;
                    --dsit;
                }
            }
            // Remove current segment.
            segList.erase( ssecondit );
            ds.erase( dssecondit );
        }
    }

    // Put list back into the chain.
    c->m_ISegVec.assign( segList.begin(), segList.end() );
}

// Copies of one surface for threads to evaluate at once: each is held by one thread at a time,
// and a thread finding none free makes another.  The copies evaluate to the same bits as the
// surface.
struct SurfCorePool
{
    Surf* m_Surf = nullptr;
    std::mutex m_Mutex;
    vector< const SurfCore* > m_Free;
    vector< std::unique_ptr< SurfCore > > m_All;

    const SurfCore* Take()
    {
        {
            std::lock_guard< std::mutex > lock( m_Mutex );
            if ( !m_Free.empty() )
            {
                const SurfCore* c = m_Free.back();
                m_Free.pop_back();
                return c;
            }
        }

        // Nothing evaluates the surface itself while its copies are in use, so it is read here
        // without the lock.
        SurfCore* c = new SurfCore( *m_Surf->GetSurfCore() );
        std::lock_guard< std::mutex > lock( m_Mutex );
        m_All.push_back( std::unique_ptr< SurfCore >( c ) );
        return c;
    }

    void Give( const SurfCore* c )
    {
        std::lock_guard< std::mutex > lock( m_Mutex );
        m_Free.push_back( c );
    }
};

void SurfaceIntersectionSingleton::RefineChains()
{
    // After the intersection chains are formed, refine them so that the value returned by CompPnt
    // will be the same with respect to each parent surface of the intersection point. Note, this 
    // must be done before the border curves are spit at each intersection chain end point.
    vector< ISegChain* > chains;
    list< ISegChain* >::iterator c;
    for ( c = m_ISegChainList.begin(); c != m_ISegChainList.end(); ++c )
    {
        if ( ( *c )->m_SurfA == ( *c )->m_SurfB )
        {
            // Do not refine SubSurface curves
            continue;
        }

        chains.push_back( *c );
    }

    // Refining a chain evaluates its two surfaces and writes only the places its points have
    // on them.
    int n = ( int )chains.size();
    vector< Surf* > sa( n );
    vector< Surf* > sb( n );
    for ( int k = 0 ; k < n ; k++ )
    {
        sa[k] = chains[k]->m_SurfA;
        sb[k] = chains[k]->m_SurfB;
    }

    // A surface carrying more than a thread's share of the points to refine would hold up every
    // chain on it, one at a time.  Such a surface is evaluated instead through copies, one for
    // each thread working on it at once, and its chains wait on nothing but their other surface.
    int nthread = StageThreadCount( n );
    std::map< Surf*, double > load;
    double total = 0.0;
    for ( int k = 0 ; k < n ; k++ )
    {
        double npnt = ( double )chains[k]->m_ISegVec.size() + 1.0;
        load[ sa[k] ] += npnt;
        load[ sb[k] ] += npnt;
        total += npnt;
    }

    std::map< Surf*, SurfCorePool > pools;
    for ( std::map< Surf*, double >::iterator it = load.begin(); it != load.end(); ++it )
    {
        if ( nthread > 1 && it->second > total / nthread )
        {
            pools[ it->first ].m_Surf = it->first;
        }
    }

    vector< Surf* > ka( sa );
    vector< Surf* > kb( sb );
    for ( int k = 0 ; k < n ; k++ )
    {
        if ( pools.count( sa[k] ) )
        {
            ka[k] = nullptr;
        }
        if ( pools.count( sb[k] ) )
        {
            kb[k] = nullptr;
        }
    }

    ProgressStage bar( this, "Refine Chains", n );

    RunChainsBySurface( ka, kb, [&]( int k )
    {
        SurfCorePool* pa = nullptr;
        SurfCorePool* pb = nullptr;
        const SurfCore* coreA = sa[k]->GetSurfCore();
        const SurfCore* coreB = sb[k]->GetSurfCore();
        if ( !ka[k] )
        {
            pa = &pools.find( sa[k] )->second;
            coreA = pa->Take();
        }
        if ( !kb[k] )
        {
            pb = &pools.find( sb[k] )->second;
            coreB = pb->Take();
        }

        RefineISegChain( chains[k], coreA, coreB );

        if ( pa )
        {
            pa->Give( coreA );
        }
        if ( pb )
        {
            pb->Give( coreB );
        }
    }, &bar );
}

// Record that chain k waits on a chain, in the first of its two places that is free
static void AddWaiting( std::array< int, 2 > &waiting, int k )
{
    if ( waiting[0] < 0 )
    {
        waiting[0] = k;
    }
    else
    {
        assert( waiting[1] < 0 );
        waiting[1] = k;
    }
}

void SurfaceIntersectionSingleton::RunChainsBySurface( const vector< Surf* > &sa, const vector< Surf* > &sb,
                                                       const std::function< void( int ) > &body, ProgressStage *bar )
{
    // A chain waits for the chain before it on each of its surfaces.  Each chain is the next on
    // at most two surfaces after another, so it is waited on by at most two.
    int n = ( int )sa.size();
    vector< int > npending( n, 0 );
    vector< std::array< int, 2 > > waiting( n, std::array< int, 2 >{ { -1, -1 } } );
    std::map< Surf*, int > last;
    for ( int k = 0 ; k < n ; k++ )
    {
        int pa = -1;
        int pb = -1;
        std::map< Surf*, int >::iterator it = last.find( sa[k] );
        if ( sa[k] && it != last.end() )
        {
            pa = it->second;
        }
        it = last.find( sb[k] );
        if ( sb[k] && it != last.end() )
        {
            pb = it->second;
        }
        if ( pa >= 0 )
        {
            AddWaiting( waiting[ pa ], k );
            npending[k]++;
        }
        if ( pb >= 0 && pb != pa )
        {
            AddWaiting( waiting[ pb ], k );
            npending[k]++;
        }
        last[ sa[k] ] = k;
        last[ sb[k] ] = k;
    }

    // A thread takes the earliest chain with nothing left to wait on, so none sits blocked
    // behind a chain another thread is running.  On one thread that is every chain in order.
    std::priority_queue< int, vector< int >, std::greater< int > > ready;
    for ( int k = 0 ; k < n ; k++ )
    {
        if ( npending[k] == 0 )
        {
            ready.push( k );
        }
    }

    int ndone = 0;
    bool failed = false;
    std::mutex readymutex;
    std::condition_variable readycv;

    int nthread = StageThreadCount( n );
    RunIndexed( nthread, nthread, [&]( int t )
    {
        while ( true )
        {
            int k;
            {
                std::unique_lock< std::mutex > lock( readymutex );
                while ( ready.empty() && ndone < n && !failed )
                {
                    readycv.wait( lock );
                }
                if ( ready.empty() || failed )
                {
                    return;
                }
                k = ready.top();
                ready.pop();
            }

            // A failure stops the others taking more, so none waits forever on this chain.
            try
            {
                body( k );

                if ( bar )
                {
                    bar->Step();
                }
            }
            catch ( ... )
            {
                std::lock_guard< std::mutex > lock( readymutex );
                failed = true;
                readycv.notify_all();
                throw;
            }

            std::lock_guard< std::mutex > lock( readymutex );
            ndone++;
            for ( int i = 0 ; i < 2 ; i++ )
            {
                int w = waiting[k][i];
                if ( w >= 0 )
                {
                    npending[w]--;
                    if ( npending[w] == 0 )
                    {
                        ready.push( w );
                    }
                }
            }
            readycv.notify_all();
        }
    } );
}

void SurfaceIntersectionSingleton::RunBySurface( const vector< Surf* > &s, const std::function< void( int ) > &body,
                                                 ProgressStage *bar )
{
    // The items of each surface, in order, and the surfaces in the order first met
    vector< vector< int > > groups;
    std::unordered_map< Surf*, int > gindex;
    for ( int k = 0 ; k < ( int )s.size() ; k++ )
    {
        std::unordered_map< Surf*, int >::iterator it = gindex.find( s[k] );
        if ( it == gindex.end() )
        {
            it = gindex.insert( std::make_pair( s[k], ( int )groups.size() ) ).first;
            groups.push_back( vector< int >() );
        }
        groups[ it->second ].push_back( k );
    }

    // The largest groups are handed out first, so none is left to run alone at the end.
    vector< int > order( groups.size() );
    for ( int g = 0 ; g < ( int )order.size() ; g++ )
    {
        order[g] = g;
    }
    std::stable_sort( order.begin(), order.end(), [&]( int a, int b )
    {
        return groups[a].size() > groups[b].size();
    } );

    int ngroup = ( int )groups.size();
    RunIndexed( ngroup, StageThreadCount( ngroup ), [&]( int g )
    {
        const vector< int > &items = groups[ order[g] ];
        for ( int i = 0 ; i < ( int )items.size() ; i++ )
        {
            body( items[i] );

            if ( bar )
            {
                bar->Step();
            }
        }
    } );
}

void SurfaceIntersectionSingleton::RefineISegChainSeg( ISegChain* c, IPnt* ipnt, bool endpnt, const SurfCore* coreA,
                                                       const SurfCore* coreB )
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

        vec3d pA = coreA->CompPnt( uA, wA );
        vec3d pB = coreB->CompPnt( uB, wB );
        dist1 = ( pA - pB ).mag();

        vec3d pmid = 0.5 * ( pA + pB );
        surface_point_type mid_point;
        mid_point << pmid.x(), pmid.y(), pmid.z();

        // uwtol is a distance in u or v coordinates for a point to be considered on the border of a surface.
        // The points are generated through the intersection algorithm and should be fairly broadly spaced.
        double uwtol = 1e-4;
        borderA = coreA->UWPointOnBorder( uA, wA, uwtol );
        borderB = coreB->UWPointOnBorder( uB, wB, uwtol );

        // Only a chain's ends lie on a border.  Solved against the border, every interior point
        // near one lands on the one place the border crosses the other surface, and a curve
        // running into a border at a shallow angle collapses there into a run of points with
        // the same u,w on both surfaces.
        if ( !endpnt )
        {
            borderA = SurfCore::NOBNDY;
            borderB = SurfCore::NOBNDY;
        }

        if ( borderA == SurfCore::NOBNDY && borderB == SurfCore::NOBNDY )
        {
            ret = eli::geom::intersect::intersect(uA, wA, uB, wB, dist, *( coreA->GetSurf() ),
                                                  *( coreB->GetSurf() ), mid_point,
                                                  auw->m_UW[0], auw->m_UW[1], buw->m_UW[0], buw->m_UW[1] );
        }
        else
        {
            SurfCore surfA = *coreA;
            SurfCore surfB = *coreB;

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

        pA = coreA->CompPnt( uA, wA );
        pB = coreB->CompPnt( uB, wB );
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

void SurfaceIntersectionSingleton::RefineISegChain( ISegChain* c, const SurfCore* coreA, const SurfCore* coreB )
{

    int nseg = c->m_ISegVec.size();

    RefineISegChainSeg( c, c->m_ISegVec[0]->m_IPnt[0], true, coreA, coreB );

    for ( int i = 0; i < nseg; i++ )
    {
        RefineISegChainSeg( c, c->m_ISegVec[i]->m_IPnt[1], i == nseg - 1, coreA, coreB );
    }
}

// Add seg to the front or back of a chain being built, turned so its end joinIPnt is next to
// the chain
static void AddChainSeg( TwoEndedVector< ISeg* > &segs, ISeg* seg, bool frontFlag, IPnt* joinIPnt )
{
    if ( frontFlag )
    {
        if ( seg->m_IPnt[0] == joinIPnt )
        {
            seg->FlipDir();
        }
        segs.push_front( seg );
    }
    else
    {
        if ( seg->m_IPnt[1] == joinIPnt )
        {
            seg->FlipDir();
        }
        segs.push_back( seg );
    }
}

// Add seg to the front or back of a chain being built, joined to the segment at that end
static void AddChainSeg( TwoEndedVector< ISeg* > &segs, ISeg* seg, bool frontFlag )
{
    if ( frontFlag )
    {
        ISeg* frontSeg = segs.front();
        frontSeg->JoinFront( seg );
        segs.push_front( seg );
    }
    else
    {
        ISeg* backSeg  = segs.back();
        backSeg->JoinBack( seg );
        segs.push_back( seg );
    }
}

void SurfaceIntersectionSingleton::ExpandChain( ISegChain* chain, PNTree* PN_tree )
{
    TwoEndedVector< ISeg* > segs( chain->m_ISegVec.front() );

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
            testIPnt = segs.front()->m_IPnt[0];
        }
        else
        {
            testIPnt = segs.back()->m_IPnt[1];
        }

        IPnt* matchIPnt = nullptr;

        // The point across the edge this one lies on is the next along the curve, however far
        // the two patches' flat approximations of it put them apart
        bool by_partner = false;
        IPnt* partner = nullptr;
        if ( testIPnt->m_AllIndex >= 0 && m_AllIPntPartner[ testIPnt->m_AllIndex ] >= 0 )
        {
            partner = m_AllIPnts[ m_AllIPntPartner[ testIPnt->m_AllIndex ] ];
        }
        if ( partner && !partner->m_UsedFlag && partner->m_Puws.size() == 2 &&
             partner->m_Puws[0]->m_Surf == testIPnt->m_Puws[0]->m_Surf &&
             partner->m_Puws[1]->m_Surf == testIPnt->m_Puws[1]->m_Surf )
        {
            matchIPnt = partner;
            by_partner = true;
        }

        const double query_pt[3] = { testIPnt->m_Pnt.x(), testIPnt->m_Pnt.y(), testIPnt->m_Pnt.z() };
        size_t ret_index[num_results];
        double out_dist_sqr[num_results];
        nanoflann::KNNResultSet < double > resultSet( num_results );
        resultSet.init( ret_index, out_dist_sqr );
        if ( !by_partner )
        {
            PN_tree->findNeighbors( resultSet, query_pt, nanoflann::SearchParameters() );
        }

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
            if ( firstIter && expandFront && !by_partner && ( dist( segs.front()->m_IPnt[0]->m_Pnt, matchIPnt->m_Pnt ) > dist( segs.back()->m_IPnt[1]->m_Pnt, matchIPnt->m_Pnt ) ) )
            {
                // This segment's orientation needs to be reversed because expandFront was set true on the first iteration,
                // but the back point is closer. 
                IPnt* temp_pnt = segs.front()->m_IPnt[0];
                segs.front()->m_IPnt[0] = segs.front()->m_IPnt[1];
                segs.front()->m_IPnt[1] = temp_pnt;
            }

            firstIter = false;

            ISeg* seg = matchIPnt->m_Segs[0];
            if ( by_partner )
            {
                // Joined through the partner, which is the end next to the chain
                AddChainSeg( segs, seg, expandFront, matchIPnt );
            }
            else
            {
                AddChainSeg( segs, seg, expandFront );
            }
            seg->m_IPnt[0]->m_UsedFlag = true;
            seg->m_IPnt[1]->m_UsedFlag = true;
        }
    }

    chain->m_ISegVec = segs.GetVector();
}

void SurfaceIntersectionSingleton::WriteChains()
{
    FILE* fp;
    char str[256];
    snprintf( str, sizeof( str ), "RawInt_UW0.dat" );
    fp = fopen( str, "w" );

    for ( int i = 1 ; i < ( int )debugUWs.size() ; i += 2 )
    {
        fprintf( fp, "MOVE \n" );
        fprintf( fp, "%.19e %.19e\n", debugUWs[i - 1].x(), debugUWs[i - 1].y() );
        fprintf( fp, "%.19e %.19e\n", debugUWs[i].x(), debugUWs[i].y() );
    }

    fclose( fp );


    //==== Write Chains ====//
    list< ISegChain* >::iterator c;
    for ( int i = 0 ; i < ( int )m_SurfVec.size() ; i++ )
    {
        snprintf( str, sizeof( str ), "Intersection_UW%d.dat", i );
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
                for ( int s = 0 ; s < ( int )( *c )->m_ISegVec.size() ; s++ )
                {
                    IPnt* ip0 = ( *c )->m_ISegVec[s]->m_IPnt[0];
                    IPnt* ip1 = ( *c )->m_ISegVec[s]->m_IPnt[1];

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
                        fprintf( fp, "%.19e %.19e\n", puw0->m_UW.x(), puw0->m_UW.y() );
                        fprintf( fp, "%.19e %.19e\n", puw1->m_UW.x(), puw1->m_UW.y() );
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
                        fprintf( fp, "%.19e %.19e\n", split->m_UW.x(), split->m_UW.y() );
                        fprintf( fp, "%.19e %.19e\n", split->m_UW.x() + 0.1, split->m_UW.y() + 0.1 );
                    }

                }
            }
        }
        fclose( fp );
    }
}


void SurfaceIntersectionSingleton::WriteChainSplits( const char* name, vector< ISegChain* > chainvec )
{
#ifdef DEBUG_CFD_MESH
    for ( int i = 0 ; i < ( int )m_SurfVec.size() ; i++ )
    {
        char str[256];
        snprintf( str, sizeof( str ), "%s%sSplits%d.m", m_DebugDir.c_str(), name, i );
        FILE* fp = fopen( str, "w" );

        int cnt = 0;
        vector< ISegChain* >::iterator c;
        for ( c = chainvec.begin() ; c != chainvec.end(); ++c )
        {
            if ( !( *c )->m_BorderFlag && m_SurfVec[i] == ( *c )->m_SurfA || m_SurfVec[i] == ( *c )->m_SurfB )
            {
                bool first;

                fprintf( fp, "u=[" );
                first = true;
                //==== Draw Intersections ====//
                for ( int s = 0 ; s < ( int )( *c )->m_SplitVec.size() ; s++ )
                {
                    ISegSplit* split = ( *c )->m_SplitVec[s];
                    if (  split->m_Surf == m_SurfVec[i] )
                    {
                        if ( !first ) fprintf( fp, ";\n" );
                        first = false;
                        fprintf( fp, "%.19e", split->m_UW.x() );
                    }
                }
                fprintf( fp, "];\n" );

                fprintf( fp, "w=[" );
                first = true;
                //==== Draw Intersections ====//
                for ( int s = 0 ; s < ( int )( *c )->m_SplitVec.size() ; s++ )
                {
                    ISegSplit* split = ( *c )->m_SplitVec[s];
                    if (  split->m_Surf == m_SurfVec[i] )
                    {
                        if ( !first ) fprintf( fp, ";\n" );
                        first = false;
                        fprintf( fp, "%.19e", split->m_UW.y() );
                    }
                }
                fprintf( fp, "];\n" );

                fprintf( fp, "x=[" );
                first = true;
                //==== Draw Intersections ====//
                for ( int s = 0 ; s < ( int )( *c )->m_SplitVec.size() ; s++ )
                {
                    ISegSplit* split = ( *c )->m_SplitVec[s];
                    if (  split->m_Surf == m_SurfVec[i] )
                    {
                        if ( !first ) fprintf( fp, ";\n" );
                        first = false;
                        fprintf( fp, "%.19e", split->m_Pnt.x() );
                    }
                }
                fprintf( fp, "];\n" );

                fprintf( fp, "y=[" );
                first = true;
                //==== Draw Intersections ====//
                for ( int s = 0 ; s < ( int )( *c )->m_SplitVec.size() ; s++ )
                {
                    ISegSplit* split = ( *c )->m_SplitVec[s];
                    if (  split->m_Surf == m_SurfVec[i] )
                    {
                        if ( !first ) fprintf( fp, ";\n" );
                        first = false;
                        fprintf( fp, "%.19e", split->m_Pnt.y() );
                    }
                }
                fprintf( fp, "];\n" );

                fprintf( fp, "z=[" );
                first = true;
                //==== Draw Intersections ====//
                for ( int s = 0 ; s < ( int )( *c )->m_SplitVec.size() ; s++ )
                {
                    ISegSplit* split = ( *c )->m_SplitVec[s];
                    if (  split->m_Surf == m_SurfVec[i] )
                    {
                        if ( !first ) fprintf( fp, ";\n" );
                        first = false;
                        fprintf( fp, "%.19e", split->m_Pnt.z() );
                    }
                }
                fprintf( fp, "];\n" );

                fprintf( fp, "f=[" );
                first = true;
                //==== Draw Intersections ====//
                for ( int s = 0 ; s < ( int )( *c )->m_SplitVec.size() ; s++ )
                {
                    ISegSplit* split = ( *c )->m_SplitVec[s];
                    if (  split->m_Surf == m_SurfVec[i] )
                    {
                        if ( !first ) fprintf( fp, ";\n" );
                        first = false;
                        fprintf( fp, "%.19e", split->m_Fract );
                    }
                }
                fprintf( fp, "];\n" );


                fprintf( fp, "indx=[" );
                first = true;
                //==== Draw Intersections ====//
                for ( int s = 0 ; s < ( int )( *c )->m_SplitVec.size() ; s++ )
                {
                    ISegSplit* split = ( *c )->m_SplitVec[s];
                    if (  split->m_Surf == m_SurfVec[i] )
                    {
                        if ( !first ) fprintf( fp, ";\n" );
                        first = false;
                        fprintf( fp, "%d", split->m_Index );
                    }
                }
                fprintf( fp, "];\n" );


                fprintf( fp, "figure(1)\n" );
                fprintf( fp, "plot( u, w, 'x-');\n hold on;\n" );
                fprintf( fp, "if ( numel(u) > 0 );\n" );
                fprintf( fp, "    text( u(round(end/2)), w(round(end/2)), 'Surf: %d Chain: %d' );\n", i, cnt );
                fprintf( fp, "end\n" );

                fprintf( fp, "figure(2)\n" );
                fprintf( fp, "plot3( x, y, z, 'x-' );\n hold on;\n" );
                fprintf( fp, "if ( numel(u) > 0 );\n" );
                fprintf( fp, "    text( x(round(end/2)), y(round(end/2)), z(round(end/2)), 'Surf: %d Chain: %d' );\n", i, cnt );
                fprintf( fp, "end\n" );
                fprintf( fp, "pause\n" );

                cnt++;
            }
        }
        fclose( fp );
    }
#endif
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
        chain->m_ALine = m_ICurveVec[i]->m_SCurve_A->GetParmLine();
        chain->m_BLine = m_ICurveVec[i]->m_SCurve_B->GetParmLine();

        vector< vec3d > uwA = m_ICurveVec[i]->m_SCurve_A->GetUWTessPnts();
        vector< vec3d > uwB = m_ICurveVec[i]->m_SCurve_B->GetUWTessPnts();

        // Each segment starts where the one before it ends, from the same places on both
        // surfaces, so its first point takes the last one's position.
        IPnt* prev_end = nullptr;
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

            if ( prev_end )
            {
                p0->m_Pnt = prev_end->m_Pnt;
            }
            else
            {
                p0->CompPnt();
            }
            p1->CompPnt();
            prev_end = p1;

            ISeg* seg = new ISeg( surfA, surfB, p0, p1 );
            chain->m_ISegVec.push_back( seg );

        }

        if( chain->Valid() )
        {
            m_ISegChainList.push_front( chain );
        }
        else
        {
            delete chain;
            chain = nullptr;
        }
    }

#ifdef DEBUG_CFD_MESH
    fprintf( m_DebugFile, "SurfaceIntersectionSingleton::LoadBorderCurves \n" );
    fprintf( m_DebugFile, "   Total Num Chains = %zu \n", m_ISegChainList.size() );

    list< ISegChain* >::iterator c;
    for ( c = m_ISegChainList.begin() ; c != m_ISegChainList.end(); ++c )
    {
        fprintf( m_DebugFile, "   Chain Num Segs = %zu Border %d \n",
                 ( *c )->m_ISegVec.size(), ( int )( ( *c )->m_BorderFlag ) );
    }
#endif

}

// Whether the line where a region's original U parameter is uo is an edge of the patch, where
// the patch has a border curve, rather than a seam inside it between two joined pieces
static bool RegionEdgeOnPatchEdgeU( const UWRegion &reg, double uo, const SurfCore* core, double tol )
{
    double up = reg.ToPatchU( uo );
    return std::abs( up - core->GetMinU() ) < tol || std::abs( up - core->GetMaxU() ) < tol;
}

static bool RegionEdgeOnPatchEdgeW( const UWRegion &reg, double wo, const SurfCore* core, double tol )
{
    double wp = reg.ToPatchW( wo );
    return std::abs( wp - core->GetMinW() ) < tol || std::abs( wp - core->GetMaxW() ) < tol;
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

        // Get all SubSurfaces for the specified geom.  The user's answer to the switch that
        // turns subsurfaces off; the patch join seams do not, because they stand in for patch
        // boundaries, which are always held.
        vector < SimpleSubSurface > ss_vec;

        if ( GetSettingsPtr()->m_IntersectSubSurfs )
        {
            ss_vec = GetSimpSubSurfs( surf->GetGeomID(), surf->GetMainSurfID(), surf->GetCompID() );
        }

        // The seams where the surface this patch came from was stitched back together.  A
        // boundary between two patches is a crease the mesher must follow, and joining two
        // pieces into one patch gives that up; these put it back.  FetchXFerSurf wrote them
        // down as it made them, and as ordinary subsurfaces the clipping below puts each on
        // the patch it belongs to and drops it from the rest.
        const vector < pair < vec3d, vec3d > > &jvec = surf->GetJoinLines();

        int njoin_start = ( int )ss_vec.size();

        vector < ParmLine > jline_vec( jvec.size() );

        for ( int i = 0 ; i < ( int )jvec.size() ; i++ )
        {
            SimpleSubSurface ss;
            ss.SetAsFiniteLine( surf->GetGeomID(), surf->GetMainSurfID(),
                                surf->GetName() + "_PatchJoin", jvec[i].first, jvec[i].second );
            ss_vec.push_back( ss );

            jline_vec[i] = ParmLine::Between( jvec[i].first.x(), jvec[i].first.y(),
                                              jvec[i].second.x(), jvec[i].second.y() );
        }

        // A patch is not always a plain piece of the Geom's surface -- an end cap or a
        // trailing edge is built from two pieces of it, and may carry them reversed.
        // Subsurfaces are drawn in the Geom's parameters, so every piece has to be clipped
        // and converted on its own.  For a patch that is a plain piece the region is the identity.
        vector < UWRegion > regvec = surf->GetUWRegionsOrWhole();

        // Split SubSurfs
        for ( int ss = 0 ; ss < ( int ) ss_vec.size(); ss++ )
        {
          for ( int ir = 0 ; ir < ( int )regvec.size() ; ir++ )
          {
            const UWRegion &reg = regvec[ir];

            // Splitting modifies the subsurface, so each region starts from a fresh copy.
            SimpleSubSurface ssurf = ss_vec[ss];

            ssurf.SplitSegsU( reg.m_UMin );
            ssurf.SplitSegsU( reg.m_UMax );
            ssurf.SplitSegsW( reg.m_WMin );
            ssurf.SplitSegsW( reg.m_WMax );

            vector < vector< SSLineSeg > >& segsvec = ssurf.GetSplitSegs();

            for ( int i = 0; i < segsvec.size(); i++ )
            {
                vector< SSLineSeg >& segs = segsvec[i];

                ISegChain* chain = nullptr;

                bool new_chain = true;
                bool is_poly = ssurf.GetPolyFlag();

                // Build Intersection Chains
                for ( int ls = 0; ls < ( int )segs.size(); ls++ )
                {
                    if ( new_chain && chain )
                    {
                        if ( chain->Valid() )
                        {
                            if ( ssurf.m_CreateBeamElements )
                            {
                                chain->m_SSIntersectIndex = ss; // Identify FeaSubSurfaceIndex
                            }

                            m_ISegChainList.push_back( chain );
                        }
                        else
                        {
                            delete chain;
                            chain = nullptr;
                        }
                    }

                    if ( new_chain )
                    {
                        chain = new ISegChain;
                        chain->m_SurfA = surf;
                        chain->m_SurfB = surf;
                        chain->m_PatchJoinFlag = ( ss >= njoin_start );
                        if ( chain->m_PatchJoinFlag )
                        {
                            // The seam's line, in the patch's parameters
                            ParmLine line = jline_vec[ ss - njoin_start ];
                            if ( line.m_Kind == ParmLine::U_CONST )
                            {
                                line.m_Val = reg.ToPatchU( line.m_Val );
                            }
                            else if ( line.m_Kind == ParmLine::W_CONST )
                            {
                                line.m_Val = reg.ToPatchW( line.m_Val );
                            }
                            chain->m_ALine = line;
                            chain->m_BLine = line;
                        }
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
                    min_u = reg.m_UMin;
                    min_w = reg.m_WMin;
                    max_u = reg.m_UMax;
                    max_w = reg.m_WMax;

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
                    // A segment along the patch's own edge repeats the border curve there, and two
                    // curves on one line cannot both be held by the triangulator.  An edge of this
                    // region inside the patch -- the seam between two pieces joined into one -- has
                    // no border curve, so a segment along it, such as the join line, stays.
                    const SurfCore* core = surf->GetSurfCore();
                    if ( ( std::abs( uw_pnt0[0]-max_u ) < tol && std::abs( uw_pnt1[0]-max_u ) < tol && RegionEdgeOnPatchEdgeU( reg, max_u, core, tol ) ) ||
                         ( std::abs( uw_pnt0[1]-max_w ) < tol && std::abs( uw_pnt1[1]-max_w ) < tol && RegionEdgeOnPatchEdgeW( reg, max_w, core, tol ) ) ||
                         ( std::abs( uw_pnt0[0]-min_u ) < tol && std::abs( uw_pnt1[0]-min_u ) < tol && RegionEdgeOnPatchEdgeU( reg, min_u, core, tol ) ) ||
                         ( std::abs( uw_pnt0[1]-min_w ) < tol && std::abs( uw_pnt1[1]-min_w ) < tol && RegionEdgeOnPatchEdgeW( reg, min_w, core, tol ) ) )
                    {
                        new_chain = true;
                        continue; // Skip if both end points are on the same border edge of the surface
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

                    // Everything above was in the Geom's parameters.  From here on the
                    // surface is asked about its own.
                    for ( int p = 0 ; p < ( int ) uw_pnts.size() ; p++ )
                    {
                        uw_pnts[p] = vec2d( reg.ToPatchU( uw_pnts[p][0] ),
                                            reg.ToPatchW( uw_pnts[p][1] ) );
                    }

                    // Each segment starts where the one before it ends, from the same places on
                    // the surface, so its first point takes the last one's position.
                    IPnt* prev_end = nullptr;
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

                        if ( prev_end )
                        {
                            p0->m_Pnt = prev_end->m_Pnt;
                        }
                        else
                        {
                            p0->CompPnt();
                        }
                        p1->CompPnt();
                        prev_end = p1;

                        ISeg* seg = new ISeg( surf, surf, p0, p1 );
                        chain->m_ISegVec.push_back( seg );
                    }
                }
                if ( chain )
                {
                    if ( chain->Valid() )
                    {
                        if ( ssurf.m_CreateBeamElements )
                        {
                            chain->m_SSIntersectIndex = ss; // Identify FeaSubSurfaceIndex
                        }

                        m_ISegChainList.push_back( chain );
                    }
                    else
                    {
                        delete chain;
                        chain = nullptr;
                    }
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
            splitPnts.push_back( ( *c )->m_ISegVec.front()->m_IPnt[0] );
            splitPnts.push_back( ( *c )->m_ISegVec.back()->m_IPnt[1]  );
        }
    }

    // The split points with a place on each surface, in order.  A point can split a chain only
    // through a place on one of the chain's two surfaces.
    std::unordered_map< Surf*, vector< int > > bysurf;
    for ( int i = 0 ; i < ( int )splitPnts.size() ; i++ )
    {
        const vector< Puw* > &puws = splitPnts[i]->m_Puws;
        for ( int k = 0 ; k < ( int )puws.size() ; k++ )
        {
            vector< int > &v = bysurf[ puws[k]->m_Surf ];
            if ( v.empty() || v.back() != i )
            {
                v.push_back( i );
            }
        }
    }

    vector< int > cand;
    const vector< int > none;
    for ( c = m_ISegChainList.begin() ; c != m_ISegChainList.end(); ++c )
    {
        if ( ( *c )->m_BorderFlag )
        {
            const vector< int > *va = &none;
            const vector< int > *vb = &none;
            std::unordered_map< Surf*, vector< int > >::const_iterator it = bysurf.find( ( *c )->m_SurfA );
            if ( it != bysurf.end() )
            {
                va = &it->second;
            }
            it = bysurf.find( ( *c )->m_SurfB );
            if ( it != bysurf.end() && ( *c )->m_SurfB != ( *c )->m_SurfA )
            {
                vb = &it->second;
            }

            cand.clear();
            std::merge( va->begin(), va->end(), vb->begin(), vb->end(), std::back_inserter( cand ) );
            cand.erase( std::unique( cand.begin(), cand.end() ), cand.end() );

            for ( int k = 0 ; k < ( int )cand.size() ; k++ )
            {
                int i = cand[k];
                Puw* uw = splitPnts[i]->GetPuw( ( *c )->m_SurfA );
                if ( !uw )
                {
                    uw = splitPnts[i]->GetPuw( ( *c )->m_SurfB );
                }

                // Only a chain end on this surface's border splits it.  An end on the border of
                // the other surface the chain meets can lie within reach of this one.
                if ( uw && uw->m_Surf->GetSurfCore()->UWPointOnBorder( uw->m_UW[0], uw->m_UW[1], 1.0e-4 ) != SurfCore::NOBNDY )
                {
                    ( *c )->AddBorderSplit( uw );
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
            else
            {
                m_DelISegChainVec.push_back( new_chains[j] );
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
        for ( int i = 0 ; i < ( int )( *c )->m_ISegVec.size() ; i++ )
        {
            IPnt* ip = ( *c )->m_ISegVec[i]->m_IPnt[0];
            crv.push_back( ip->m_Pnt );

            if ( i == ( int )( *c )->m_ISegVec.size() - 1 )
            {
                ip = ( *c )->m_ISegVec[i]->m_IPnt[1];
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

    // DebugWriteChainVec( "StartChainVec", chains );

    //==== Build Bounding Boxes Around Intersection Curves ====//
    for ( int i = 0 ; i < ( int )chains.size() ; i++ )
    {
        chains[i]->BuildBoxes();
    }

    // DebugWriteChainVec( "BuildBoxesChainVec", chains );

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

    // DebugWriteChainVec( "IntersectChainVec", chains );
    // WriteChainSplits( "IntersectChainVec", chains );

    // The boxes have done their work; only the debugging draw objects look at them again.
    if ( !DEBUG_ISECT_DRAW )
    {
        for ( int i = 0 ; i < ( int )chains.size() ; i++ )
        {
            chains[i]->ClearBoxes();
        }
    }

    //==== Merge Splits ====//
    for ( int i = 0 ; i < ( int )chains.size() ; i++ )
    {
        chains[i]->MergeSplits();
    }

    // DebugWriteChainVec( "MergeSplitsChainVec", chains );
    // WriteChainSplits( "MergeSplitsChainVec", chains );

    //==== Remove Chain End Splits ====//
    for ( int i = 0 ; i < ( int )chains.size() ; i++ )
    {
        chains[i]->RemoveChainEndSplits();
    }

    // DebugWriteChainVec( "RemovechainEndChainVec", chains );
    // WriteChainSplits( "RemovechainEndChainVec", chains );

    // vector< ISegChain* > all_new_chains;
    // vector< ISegChain* > valid_new_chains;

    //==== Split Chains ====//
    for ( int i = 0 ; i < ( int )chains.size() ; i++ )
    {
        vector< ISegChain* > new_chains = chains[i]->SortAndSplit( this );
        for ( int j = 0 ; j < ( int )new_chains.size() ; j++ )
        {
            // all_new_chains.push_back( new_chains[j]);
            if ( new_chains[j]->Valid() )
            {
                // valid_new_chains.push_back( new_chains[j]);
                new_chains[j]->m_SSIntersectIndex = chains[i]->m_SSIntersectIndex; // Track SubSurface Index
                m_ISegChainList.push_back( new_chains[j] );
            }
            else
            {
                m_DelISegChainVec.push_back( new_chains[j] );
            }

        }
    }

    // DebugWriteChainVec( "all_new_chains", all_new_chains );
    // WriteChainSplits( "all_new_chains", all_new_chains );

    // DebugWriteChainVec( "valid_new_chains", valid_new_chains );
    // WriteChainSplits( "valid_new_chains", valid_new_chains );
}

// The tail of chain c after its segment k, as a chain of its own; c keeps segments 0 to k
static ISegChain* SplitChainAfterSeg( ISegChain* c, int k )
{
    ISegChain* nc = new ISegChain();
    nc->m_SurfA = c->m_SurfA;
    nc->m_SurfB = c->m_SurfB;
    nc->m_PatchJoinFlag = c->m_PatchJoinFlag;
    nc->m_ALine = c->m_ALine;
    nc->m_BLine = c->m_BLine;
    nc->m_SSIntersectIndex = c->m_SSIntersectIndex;
    nc->m_ISegVec.reserve( c->m_ISegVec.size() - k - 1 );
    for ( int j = k + 1 ; j < ( int )c->m_ISegVec.size() ; j++ )
    {
        nc->m_ISegVec.push_back( c->m_ISegVec[j] );
    }
    c->m_ISegVec.resize( k + 1 );
    return nc;
}

// Where two surfaces are tangent, the curves they meet along can come within a hair of each other
// and part again: two curves that cross there exactly are traced as two that touch, and a model
// moved the least bit off that is traced as two that pass close by.  Closer than a tenth of the
// minimum edge length, the gap between them cannot be meshed, and the mesh is made as if they
// touched: two intersection chains of the same two surfaces that pass that close, away from
// their ends, are both cut at the closest pair of their points, and the four ends made one point.
// A tree of the points a chain passes through, away from its ends.
struct ChainPntTree
{
    explicit ChainPntTree( ISegChain* c ) : m_Tree( 3, m_Cloud, KDTreeSingleIndexAdaptorParams() )
    {
        for ( int k = 0 ; k + 1 < ( int )c->m_ISegVec.size() ; k++ )
        {
            m_Cloud.AddPntNode( c->m_ISegVec[k]->m_IPnt[1]->m_Pnt );
        }
        m_Tree.buildIndex();
    }

    PntNodeCloud m_Cloud;
    PNTree m_Tree;
};

// The two surfaces a chain lies on, in an order that does not depend on which is which.
static pair< Surf*, Surf* > SurfPairKey( const ISegChain* c )
{
    if ( std::less< Surf* >()( c->m_SurfB, c->m_SurfA ) )
    {
        return make_pair( c->m_SurfB, c->m_SurfA );
    }
    return make_pair( c->m_SurfA, c->m_SurfB );
}

// Drops the point tree of a chain that has been cut.
static void ForgetChain( ISegChain* c, map< ISegChain*, ChainPntTree* > &trees )
{
    map< ISegChain*, ChainPntTree* >::iterator t = trees.find( c );
    if ( t != trees.end() )
    {
        delete t->second;
        trees.erase( t );
    }
}

void SurfaceIntersectionSingleton::SplitNearlyTouchingChains()
{
    SimpleGridDensity* gd = GetGridDensityPtr();
    if ( !gd )
    {
        return;
    }

    double tol = 0.1 * gd->m_MinLen;
    int ncut = 0;

    // After a cut, the search starts again from the first of the two chains cut.
    int jstart = 0;

    // A cut changes only the two chains it cuts, so a chain's point tree holds until it is cut.
    map< ISegChain*, ChainPntTree* > trees;

    bool found = true;
    while ( found )
    {
        found = false;

        vector< ISegChain* > chains;
        list< ISegChain* >::iterator c;
        for ( c = m_ISegChainList.begin() ; c != m_ISegChainList.end(); ++c )
        {
            if ( !( *c )->m_BorderFlag && ( *c )->m_SurfA != ( *c )->m_SurfB && ( *c )->m_ISegVec.size() > 1 )
            {
                chains.push_back( *c );
            }
        }

        // Only chains of the same two surfaces are compared; each list is in chain order.
        map< pair< Surf*, Surf* >, vector< int > > samesurfs;
        for ( int k = 0 ; k < ( int )chains.size() ; k++ )
        {
            samesurfs[ SurfPairKey( chains[k] ) ].push_back( k );
        }

        for ( int j = jstart ; j < ( int )chains.size() && !found ; j++ )
        {
            ISegChain* b = chains[j];
            const vector< int > &same = samesurfs[ SurfPairKey( b ) ];

            for ( int m = 0 ; m < ( int )same.size() && same[m] < j && !found ; m++ )
            {
                int i = same[m];
                ISegChain* a = chains[i];

                // The points b passes through, away from its ends
                ChainPntTree* bt = trees[ b ];
                if ( !bt )
                {
                    bt = new ChainPntTree( b );
                    trees[ b ] = bt;
                }
                PNTree &index = bt->m_Tree;

                vec3d ends[4] = { a->m_ISegVec.front()->m_IPnt[0]->m_Pnt, a->m_ISegVec.back()->m_IPnt[1]->m_Pnt,
                                  b->m_ISegVec.front()->m_IPnt[0]->m_Pnt, b->m_ISegVec.back()->m_IPnt[1]->m_Pnt };

                double best = tol;
                int ka_best = -1;
                int kb_best = -1;

                for ( int ka = 0 ; ka + 1 < ( int )a->m_ISegVec.size() ; ka++ )
                {
                    vec3d pa = a->m_ISegVec[ka]->m_IPnt[1]->m_Pnt;

                    // Not where two pieces of one curve meet, or a curve meets one it ends on.
                    // Where the curves crowd their points into the ends, a search from a point
                    // near one would find thousands of points only to turn every one away.
                    bool nearend = false;
                    for ( int e = 0 ; e < 4 ; e++ )
                    {
                        if ( dist( pa, ends[e] ) < 2.0 * tol )
                        {
                            nearend = true;
                        }
                    }
                    if ( nearend )
                    {
                        continue;
                    }

                    PNTreeResults matches;
                    nanoflann::SearchParameters params;
                    int nmatch = ( int )index.radiusSearch( &pa[0], best * best, matches, params );

                    for ( int m = 0 ; m < nmatch ; m++ )
                    {
                        int kb = matches[m].first;
                        vec3d pb = b->m_ISegVec[kb]->m_IPnt[1]->m_Pnt;
                        double d = dist( pa, pb );

                        bool bnearend = false;
                        for ( int e = 0 ; e < 4 ; e++ )
                        {
                            if ( dist( pb, ends[e] ) < 2.0 * tol )
                            {
                                bnearend = true;
                            }
                        }

                        if ( d < best && !bnearend )
                        {
                            best = d;
                            ka_best = ka;
                            kb_best = kb;
                        }
                    }
                }

                if ( ka_best >= 0 )
                {
                    IPnt* ip = a->m_ISegVec[ ka_best ]->m_IPnt[1];

                    m_ISegChainList.push_back( SplitChainAfterSeg( a, ka_best ) );

                    ISegChain* nb = SplitChainAfterSeg( b, kb_best );
                    b->m_ISegVec.back()->m_IPnt[1] = ip;
                    nb->m_ISegVec.front()->m_IPnt[0] = ip;
                    m_ISegChainList.push_back( nb );

                    found = true;
                    jstart = i;
                    ncut++;

                    ForgetChain( a, trees );
                    ForgetChain( b, trees );
                }
            }
        }
    }

    map< ISegChain*, ChainPntTree* >::iterator t;
    for ( t = trees.begin() ; t != trees.end(); ++t )
    {
        delete t->second;
    }

    if ( ncut > 0 )
    {
        printf( "Cut %d pairs of intersection curves where they nearly touch.\n", ncut );
        fflush( stdout );
    }
}

void SurfaceIntersectionSingleton::MergeInteriorChainIPnts()
{
    //==== Merge Interior IPnts in Chains ====//
    // A chain's interior points are its own, so the chains are merged on several threads.
    vector< ISegChain* > chains( m_ISegChainList.begin(), m_ISegChainList.end() );
    int n = ( int )chains.size();
    RunStage( "Merge Chain Pnts", n, [&]( int k )
    {
        chains[k]->MergeInteriorIPnts();
    } );
}



void SurfaceIntersectionSingleton::HighlightNextChain()
{
    m_HighlightChainIndex++;
    if ( m_HighlightChainIndex >= ( int )m_ISegChainList.size() )
    {
        m_HighlightChainIndex = 0;
    }
}

void SurfaceIntersectionSingleton::DebugWriteChainVec( const char* name, vector< ISegChain* > chainvec )
{
#ifdef DEBUG_CFD_MESH

    char str2[256];
    snprintf( str2, sizeof( str2 ), "%s%s.m", m_DebugDir.c_str(), name );
    FILE* fpmas = fopen( str2, "w" );

    fprintf( fpmas, "clear all; format compact; close all;\n" );
    fprintf( fpmas, "figure(1); hold on\n" );
    fprintf( fpmas, "figure(2); hold on\n" );

    for ( int i = 0 ; i < ( int )m_SurfVec.size() ; i++ )
    {
        char str[256];
        snprintf( str, sizeof( str ), "%s%s%d.m", m_DebugDir.c_str(), name, i );
        FILE* fp = fopen( str, "w" );

        snprintf( str, sizeof( str ), "%s%d.m", name, i );
        fprintf( fpmas, "run( '%s' );\n", str );

        int cnt = 0;
        vector< ISegChain* >::iterator c;
        for ( c = chainvec.begin() ; c != chainvec.end(); ++c )
        {
            if ( m_SurfVec[i] == ( *c )->m_SurfA || m_SurfVec[i] == ( *c )->m_SurfB )
            {

                fprintf( fp, "u=[" );
                int j;
                vec2d uw1, uw2;
                for ( j = 0 ; j < ( int )( *c )->m_ISegVec.size() - 1 ; j++ )
                {
                    uw1 = ( *c )->m_ISegVec[j]->m_IPnt[0]->GetPuw( m_SurfVec[i] )->m_UW;
                    fprintf( fp, "%.19e;\n", uw1[0] );
                }
                uw1 = ( *c )->m_ISegVec[j]->m_IPnt[0]->GetPuw( m_SurfVec[i] )->m_UW;
                uw2 = ( *c )->m_ISegVec[j]->m_IPnt[1]->GetPuw( m_SurfVec[i] )->m_UW;
                fprintf( fp, "%.19e;\n%.19e];\n", uw1[0], uw2[0] );

                fprintf( fp, "w=[" );
                for ( j = 0 ; j < ( int )( *c )->m_ISegVec.size() - 1 ; j++ )
                {
                    uw1 = ( *c )->m_ISegVec[j]->m_IPnt[0]->GetPuw( m_SurfVec[i] )->m_UW;
                    fprintf( fp, "%.19e;\n", uw1[1] );
                }
                uw1 = ( *c )->m_ISegVec[j]->m_IPnt[0]->GetPuw( m_SurfVec[i] )->m_UW;
                uw2 = ( *c )->m_ISegVec[j]->m_IPnt[1]->GetPuw( m_SurfVec[i] )->m_UW;
                fprintf( fp, "%.19e;\n%.19e];\n", uw1[1], uw2[1] );

                fprintf( fp, "figure(1)\n" );
                fprintf( fp, "plot( u, w, 'x-');\n hold on;\n" );
                fprintf( fp, "text( u(round(end/2)), w(round(end/2)), 'Surf: %d Chain: %d' );\n", i, cnt );
                fprintf( fp, "pause\n" );

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

    fclose( fpmas );


#endif
}


void SurfaceIntersectionSingleton::DebugWriteChains( const char* name, bool tessFlag )
{
#ifdef DEBUG_CFD_MESH
    if ( true )
    {
        char str2[256];
        snprintf( str2, sizeof( str2 ), "%s%s.m", m_DebugDir.c_str(), name );
        FILE* fpmas = fopen( str2, "w" );

        fprintf( fpmas, "clear all; format compact; close all;\n" );
        fprintf( fpmas, "figure(1); hold on\n" );
        fprintf( fpmas, "figure(2); hold on\n" );

        for ( int i = 0 ; i < ( int )m_SurfVec.size() ; i++ )
        {
            char str[256];
            snprintf( str, sizeof( str ), "%s%s%d.m", m_DebugDir.c_str(), name, i );
            FILE* fp = fopen( str, "w" );

            snprintf( str, sizeof( str ), "%s%d.m", name, i );
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
                        for ( j = 0 ; j < ( int )( *c )->m_ISegVec.size() - 1 ; j++ )
                        {
                            uw1 = ( *c )->m_ISegVec[j]->m_IPnt[0]->GetPuw( m_SurfVec[i] )->m_UW;
                            fprintf( fp, "%.19e;\n", uw1[0] );
                            uw1 = ( *c )->m_ISegVec[j]->m_IPnt[1]->GetPuw( m_SurfVec[i] )->m_UW;
                            fprintf( fp, "%.19e;\n", uw1[0] );
                        }
                        uw1 = ( *c )->m_ISegVec[j]->m_IPnt[0]->GetPuw( m_SurfVec[i] )->m_UW;
                        uw2 = ( *c )->m_ISegVec[j]->m_IPnt[1]->GetPuw( m_SurfVec[i] )->m_UW;
                        fprintf( fp, "%.19e;\n%.19e];\n", uw1[0], uw2[0] );

                        fprintf( fp, "w=[" );
                        for ( j = 0 ; j < ( int )( *c )->m_ISegVec.size() - 1 ; j++ )
                        {
                            uw1 = ( *c )->m_ISegVec[j]->m_IPnt[0]->GetPuw( m_SurfVec[i] )->m_UW;
                            fprintf( fp, "%.19e;\n", uw1[1] );
                            uw1 = ( *c )->m_ISegVec[j]->m_IPnt[1]->GetPuw( m_SurfVec[i] )->m_UW;
                            fprintf( fp, "%.19e;\n", uw1[1] );
                        }
                        uw1 = ( *c )->m_ISegVec[j]->m_IPnt[0]->GetPuw( m_SurfVec[i] )->m_UW;
                        uw2 = ( *c )->m_ISegVec[j]->m_IPnt[1]->GetPuw( m_SurfVec[i] )->m_UW;
                        fprintf( fp, "%.19e;\n%.19e];\n", uw1[1], uw2[1] );

                        fprintf( fp, "figure(1)\n" );
                        fprintf( fp, "plot( u, w, 'x-');\n hold on;\n" );
                        fprintf( fp, "text( u(round(end/2)), w(round(end/2)), 'Surf: %d Chain: %d' );\n", i, cnt );

                        fprintf( fp, "x=[" );
                        vec3d pt, pt2;
                        for ( j = 0 ; j < ( int )( *c )->m_ISegVec.size() - 1 ; j++ )
                        {
                            uw1 = ( *c )->m_ISegVec[j]->m_IPnt[0]->GetPuw( m_SurfVec[i] )->m_UW;
                            pt = ( *c )->m_ISegVec[j]->m_IPnt[0]->GetPuw( m_SurfVec[i] )->m_Surf->CompPnt( uw1[0], uw1[1] );
                            fprintf( fp, "%.19e;\n", pt.x() );
                            uw1 = ( *c )->m_ISegVec[j]->m_IPnt[1]->GetPuw( m_SurfVec[i] )->m_UW;
                            pt = ( *c )->m_ISegVec[j]->m_IPnt[1]->GetPuw( m_SurfVec[i] )->m_Surf->CompPnt( uw1[0], uw1[1] );
                            fprintf( fp, "%.19e;\n", pt.x() );
                        }
                        uw1 = ( *c )->m_ISegVec[j]->m_IPnt[0]->GetPuw( m_SurfVec[i] )->m_UW;
                        pt = ( *c )->m_ISegVec[j]->m_IPnt[0]->GetPuw( m_SurfVec[i] )->m_Surf->CompPnt( uw1[0], uw1[1] );
                        uw2 = ( *c )->m_ISegVec[j]->m_IPnt[1]->GetPuw( m_SurfVec[i] )->m_UW;
                        pt2 = ( *c )->m_ISegVec[j]->m_IPnt[1]->GetPuw( m_SurfVec[i] )->m_Surf->CompPnt( uw2[0], uw2[1] );
                        fprintf( fp, "%.19e;\n%.19e];\n", pt.x(), pt2.x() );

                        fprintf( fp, "y=[" );
                        for ( j = 0 ; j < ( int )( *c )->m_ISegVec.size() - 1 ; j++ )
                        {
                            uw1 = ( *c )->m_ISegVec[j]->m_IPnt[0]->GetPuw( m_SurfVec[i] )->m_UW;
                            pt = ( *c )->m_ISegVec[j]->m_IPnt[0]->GetPuw( m_SurfVec[i] )->m_Surf->CompPnt( uw1[0], uw1[1] );
                            fprintf( fp, "%.19e;\n", pt.y() );
                            uw1 = ( *c )->m_ISegVec[j]->m_IPnt[1]->GetPuw( m_SurfVec[i] )->m_UW;
                            pt = ( *c )->m_ISegVec[j]->m_IPnt[1]->GetPuw( m_SurfVec[i] )->m_Surf->CompPnt( uw1[0], uw1[1] );
                            fprintf( fp, "%.19e;\n", pt.y() );
                        }
                        uw1 = ( *c )->m_ISegVec[j]->m_IPnt[0]->GetPuw( m_SurfVec[i] )->m_UW;
                        pt = ( *c )->m_ISegVec[j]->m_IPnt[0]->GetPuw( m_SurfVec[i] )->m_Surf->CompPnt( uw1[0], uw1[1] );
                        uw2 = ( *c )->m_ISegVec[j]->m_IPnt[1]->GetPuw( m_SurfVec[i] )->m_UW;
                        pt2 = ( *c )->m_ISegVec[j]->m_IPnt[1]->GetPuw( m_SurfVec[i] )->m_Surf->CompPnt( uw2[0], uw2[1] );
                        fprintf( fp, "%.19e;\n%.19e];\n", pt.y(), pt2.y() );

                        fprintf( fp, "z=[" );
                        for ( j = 0 ; j < ( int )( *c )->m_ISegVec.size() - 1 ; j++ )
                        {
                            uw1 = ( *c )->m_ISegVec[j]->m_IPnt[0]->GetPuw( m_SurfVec[i] )->m_UW;
                            pt = ( *c )->m_ISegVec[j]->m_IPnt[0]->GetPuw( m_SurfVec[i] )->m_Surf->CompPnt( uw1[0], uw1[1] );
                            fprintf( fp, "%.19e;\n", pt.z() );
                            uw1 = ( *c )->m_ISegVec[j]->m_IPnt[1]->GetPuw( m_SurfVec[i] )->m_UW;
                            pt = ( *c )->m_ISegVec[j]->m_IPnt[1]->GetPuw( m_SurfVec[i] )->m_Surf->CompPnt( uw1[0], uw1[1] );
                            fprintf( fp, "%.19e;\n", pt.z() );
                        }
                        uw1 = ( *c )->m_ISegVec[j]->m_IPnt[0]->GetPuw( m_SurfVec[i] )->m_UW;
                        pt = ( *c )->m_ISegVec[j]->m_IPnt[0]->GetPuw( m_SurfVec[i] )->m_Surf->CompPnt( uw1[0], uw1[1] );
                        uw2 = ( *c )->m_ISegVec[j]->m_IPnt[1]->GetPuw( m_SurfVec[i] )->m_UW;
                        pt2 = ( *c )->m_ISegVec[j]->m_IPnt[1]->GetPuw( m_SurfVec[i] )->m_Surf->CompPnt( uw2[0], uw2[1] );
                        fprintf( fp, "%.19e;\n%.19e];\n", pt.z(), pt2.z() );

                        fprintf( fp, "figure(2)\n" );
                        fprintf( fp, "plot3( x, y, z, 'x-' );\n hold on;\n" );
                        fprintf( fp, "text( x(round(end/2)), y(round(end/2)), z(round(end/2)), 'Surf: %d Chain: %d' );\n", i, cnt );
                    }
                    else
                    {
                        fprintf( fp, "u=[" );
                        int j;
                        vec2d uw1;
                        for ( j = 0 ; j < ( int )( *c )->m_TessVec.size() - 1 ; j++ )
                        {
                            uw1 = ( *c )->m_TessVec[j]->GetPuw( m_SurfVec[i] )->m_UW;
                            fprintf( fp, "%.19e;\n", uw1[0] );
                        }
                        uw1 = ( *c )->m_TessVec[j]->GetPuw( m_SurfVec[i] )->m_UW;
                        fprintf( fp, "%.19e];\n", uw1[0] );

                        fprintf( fp, "w=[" );
                        for ( j = 0 ; j < ( int )( *c )->m_TessVec.size() - 1 ; j++ )
                        {
                            uw1 = ( *c )->m_TessVec[j]->GetPuw( m_SurfVec[i] )->m_UW;
                            fprintf( fp, "%.19e;\n", uw1[1] );
                        }
                        uw1 = ( *c )->m_TessVec[j]->GetPuw( m_SurfVec[i] )->m_UW;
                        fprintf( fp, "%.19e];\n", uw1[1] );

                        fprintf( fp, "figure(1)\n");
                        fprintf( fp, "plot( u, w, 'x-');\n hold on;\n" );


                        fprintf( fp, "x=[" );
                        vec3d pt;
                        for ( j = 0 ; j < ( int )( *c )->m_TessVec.size() - 1 ; j++ )
                        {
                            uw1 = ( *c )->m_TessVec[j]->GetPuw( m_SurfVec[i] )->m_UW;
                            pt = ( *c )->m_TessVec[j]->GetPuw( m_SurfVec[i] )->m_Surf->CompPnt( uw1[0], uw1[1] );
                            fprintf( fp, "%.19e;\n", pt.x() );
                        }
                        uw1 = ( *c )->m_TessVec[j]->GetPuw( m_SurfVec[i] )->m_UW;
                        pt = ( *c )->m_TessVec[j]->GetPuw( m_SurfVec[i] )->m_Surf->CompPnt( uw1[0], uw1[1] );
                        fprintf( fp, "%.19e];\n", pt.x() );

                        fprintf( fp, "y=[" );
                        for ( j = 0 ; j < ( int )( *c )->m_TessVec.size() - 1 ; j++ )
                        {
                            uw1 = ( *c )->m_TessVec[j]->GetPuw( m_SurfVec[i] )->m_UW;
                            pt = ( *c )->m_TessVec[j]->GetPuw( m_SurfVec[i] )->m_Surf->CompPnt( uw1[0], uw1[1] );
                            fprintf( fp, "%.19e;\n", pt.y() );
                        }
                        uw1 = ( *c )->m_TessVec[j]->GetPuw( m_SurfVec[i] )->m_UW;
                        pt = ( *c )->m_TessVec[j]->GetPuw( m_SurfVec[i] )->m_Surf->CompPnt( uw1[0], uw1[1] );
                        fprintf( fp, "%.19e];\n", pt.y() );

                        fprintf( fp, "z=[" );
                        for ( j = 0 ; j < ( int )( *c )->m_TessVec.size() - 1 ; j++ )
                        {
                            uw1 = ( *c )->m_TessVec[j]->GetPuw( m_SurfVec[i] )->m_UW;
                            pt = ( *c )->m_TessVec[j]->GetPuw( m_SurfVec[i] )->m_Surf->CompPnt( uw1[0], uw1[1] );
                            fprintf( fp, "%.19e;\n", pt.z() );
                        }
                        uw1 = ( *c )->m_TessVec[j]->GetPuw( m_SurfVec[i] )->m_UW;
                        pt = ( *c )->m_TessVec[j]->GetPuw( m_SurfVec[i] )->m_Surf->CompPnt( uw1[0], uw1[1] );
                        fprintf( fp, "%.19e];\n", pt.z() );

                        fprintf( fp, "figure(2)\n");
                        fprintf( fp, "plot3( x, y, z, 'x-');\n hold on;\n" );
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

        fclose( fpmas );
    }
    else
    {
        for ( int i = 0 ; i < ( int )m_SurfVec.size() ; i++ )
        {
            char str[256];
            snprintf( str, sizeof( str ), "%s%s%d.dat", m_DebugDir.c_str(), name, i );
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
                        for ( int j = 0 ; j < ( int )( *c )->m_ISegVec.size() ; j++ )
                        {
                            fprintf( fp, "MOVE \n" );
                            vec2d uw0 = ( *c )->m_ISegVec[j]->m_IPnt[0]->GetPuw( m_SurfVec[i] )->m_UW;
                            vec2d uw1 = ( *c )->m_ISegVec[j]->m_IPnt[1]->GetPuw( m_SurfVec[i] )->m_UW;
                            vec2d tmp = uw0 + ( uw1 - uw0 ) * 0.1;
                            uw1 = uw1 + ( uw0 - uw1 ) * 0.1;
                            uw0 = tmp;
                            fprintf( fp, "%.19e %.19e\n", uw0[0], uw0[1] );
                            fprintf( fp, "%.19e %.19e\n", uw1[0], uw1[1] );
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
                            fprintf( fp, "%.19e %.19e\n", uw0[0], uw0[1] );
                            fprintf( fp, "%.19e %.19e\n", uw1[0], uw1[1] );
                        }
                    }
                    cnt++;
                }
            }
            fclose( fp );
        }
    }
#endif
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

//==== Coplanar Surfaces ====//
//
// Two flat surfaces of different components that lie in one plane and meet there -- a fin's
// blunt trailing edge laid in the plane of a body's base -- cannot be intersected through their
// patches: the patches' triangles are coplanar, and a ray cast from a triangle in the plane
// starts on the other surface.  Such a pair is intersected in its plane instead.
//
//   FindCoplanarSurfs      finds the pairs, before any pair of surfaces is intersected.  The
//                          pair is then left out of the patch intersection, and a border
//                          one of them shares with a third surface is not laid on the other.
//   SplitCoplanarBorders   splits the chains on each surface where a border of the other
//                          crosses them, so that each piece of a border lies wholly inside the
//                          other surface or wholly outside it.
//   AddCoplanarChains      lays the pieces of one surface's borders that lie inside the other
//                          onto it, through the very points the border was tessellated with.
//   CoplanarVerdicts       says, for a triangle lying on the other surface, whether it is
//                          inside that surface's component.
//
// Where the two face apart they are bodies touching face to face, and both copies of the face
// they share go.  Where they face the same way one copy is kept, that of the component listed
// first, as DeleteDuplicateSurfs keeps the first of two coincident surfaces.  Only that copy's
// borders are laid onto the other then, since the other's are lost with its triangles.

static bool CoplanarCandidate( Surf* s )
{
    return s->GetCompID() >= 0 &&
           s->GetSurfaceCfdType() == vsp::CFD_NORMAL &&
           s->GetSurfaceThickSurf() &&
           !s->GetWakeFlag() &&
           !s->GetSymPlaneFlag() &&
           !s->GetFarFlag();
}

double SurfaceIntersectionSingleton::CoplanarTol( const Surf* a, const Surf* b ) const
{
    return COPLANAR_RELTOL * std::max( a->GetBBox().DiagDist(), b->GetBBox().DiagDist() );
}

void SurfaceIntersectionSingleton::FindCoplanarSurfs()
{
    m_CoplanarPairs.clear();

    if ( !CoplanarSurfsMeshed() )
    {
        return;
    }

    int n = ( int )m_SurfVec.size();

    // A plane is fitted to each surface's own control points, so the surfaces are fitted on
    // several threads.
    RunStage( "Coplanar Fit", n, [&]( int i )
    {
        if ( CoplanarCandidate( m_SurfVec[i] ) )
        {
            m_SurfVec[i]->FitPlane( COPLANAR_RELTOL );
        }
    } );

    // Only the flat candidates are paired, in order.
    vector< Surf* > flat;
    for ( int i = 0 ; i < n ; i++ )
    {
        if ( CoplanarCandidate( m_SurfVec[i] ) && m_SurfVec[i]->GetPlaneFlag() )
        {
            flat.push_back( m_SurfVec[i] );
        }
    }

    int nflat = ( int )flat.size();
    for ( int i = 0 ; i < nflat ; i++ )
    {
        Surf* a = flat[i];

        for ( int j = i + 1 ; j < nflat ; j++ )
        {
            Surf* b = flat[j];
            if ( a->GetCompID() == b->GetCompID() )
            {
                continue;
            }

            double tol = CoplanarTol( a, b );
            if ( Compare( a->GetBBox(), b->GetBBox(), tol ) && a->SamePlane( b, tol ) )
            {
                a->AddCoplanarSurf( b );
                b->AddCoplanarSurf( a );
                m_CoplanarPairs.push_back( pair< Surf*, Surf* >( a, b ) );
            }
        }
    }
}

// Which of the two keeps its copy of the face they share, or nullptr where neither does
static Surf* CoplanarKept( Surf* a, Surf* b )
{
    if ( dot( a->GetPlaneNorm(), b->GetPlaneNorm() ) < 0.0 )
    {
        return nullptr;
    }
    if ( a->GetCompID() < b->GetCompID() )
    {
        return a;
    }
    return b;
}

// A chain followed along a surface it lies on.  Its parameter is 0 at the first point and grows
// by one along each segment; within a segment the surface is evaluated along the straight line
// between the two ends' u,w, which for a border is the border itself.
class CoplanarCurve
{
public:
    CoplanarCurve( ISegChain* c, Surf* s ) : m_Chain( c ), m_Surf( s )
    {
    }

    double MaxT() const
    {
        return ( double )m_Chain->m_ISegVec.size();
    }

    void Seg( double t, int &i, double &f ) const
    {
        int n = ( int )m_Chain->m_ISegVec.size();
        t = clamp( t, 0.0, MaxT() );
        i = ( int )floor( t );
        if ( i > n - 1 )
        {
            i = n - 1;
        }
        f = t - i;
    }

    vec3d Pnt( double t ) const
    {
        int i;
        double f;
        Seg( t, i, f );
        vec2d uw0 = m_Chain->m_ISegVec[i]->m_IPnt[0]->GetPuw( m_Surf )->m_UW;
        vec2d uw1 = m_Chain->m_ISegVec[i]->m_IPnt[1]->GetPuw( m_Surf )->m_UW;
        vec2d uw = uw0 + ( uw1 - uw0 ) * f;
        return m_Surf->CompPnt( uw[0], uw[1] );
    }

    vec3d Tan( double t ) const
    {
        double h = 1.0e-4;
        double t0 = clamp( t - h, 0.0, MaxT() );
        double t1 = clamp( t + h, 0.0, MaxT() );
        return ( Pnt( t1 ) - Pnt( t0 ) ) * ( 1.0 / ( t1 - t0 ) );
    }

    ISegChain* m_Chain;
    Surf* m_Surf;
};

// Newton's method for where two curves in the plane of normal n cross, from s and t
static bool RefineCrossing( const CoplanarCurve &ca, const CoplanarCurve &cb, const vec3d &n, double &s, double &t, double tol )
{
    for ( int it = 0 ; it < 30 ; it++ )
    {
        vec3d d = ca.Pnt( s ) - cb.Pnt( t );
        vec3d ta = ca.Tan( s );
        vec3d tb = cb.Tan( t );

        double den = dot( cross( ta, tb ), n );
        if ( std::abs( den ) <= 1.0e-6 * ta.mag() * tb.mag() )
        {
            return false;
        }

        double ds = -dot( cross( d, tb ), n ) / den;
        double dt = dot( cross( ta, d ), n ) / den;

        s = clamp( s + ds, 0.0, ca.MaxT() );
        t = clamp( t + dt, 0.0, cb.MaxT() );

        if ( std::abs( ds ) < 1.0e-12 && std::abs( dt ) < 1.0e-12 )
        {
            break;
        }
    }

    return dist( ca.Pnt( s ), cb.Pnt( t ) ) <= tol;
}

// The point of the curve nearest p, from s
static bool ProjectOnCurve( const CoplanarCurve &c, const vec3d &p, double &s, double tol )
{
    for ( int it = 0 ; it < 30 ; it++ )
    {
        vec3d tan = c.Tan( s );
        double tt = dot( tan, tan );
        if ( tt <= 0.0 )
        {
            return false;
        }

        double ds = dot( p - c.Pnt( s ), tan ) / tt;
        s = clamp( s + ds, 0.0, c.MaxT() );

        if ( std::abs( ds ) < 1.0e-12 )
        {
            break;
        }
    }

    return dist( c.Pnt( s ), p ) <= tol;
}

// Split chain c, on surface s, at parameter t of its CoplanarCurve.
void SurfaceIntersectionSingleton::AddCoplanarSplit( ISegChain* c, Surf* s, double t )
{
    CoplanarCurve crv( c, s );
    int i;
    double f;
    crv.Seg( t, i, f );

    vec2d uw0 = c->m_ISegVec[i]->m_IPnt[0]->GetPuw( s )->m_UW;
    vec2d uw1 = c->m_ISegVec[i]->m_IPnt[1]->GetPuw( s )->m_UW;

    Puw* puw = new Puw( s, uw0 + ( uw1 - uw0 ) * f );
    m_DelPuwVec.push_back( puw );

    c->AddBorderSplit( puw );
}

// Split ca, a border of a, and cb, a border of b, wherever the two cross or an end of one lies
// on the other.  The chains' segments are chords, so they only say roughly where to look; each
// point is then found on the curves themselves, to within tol.
void SurfaceIntersectionSingleton::SplitCoplanarPair( ISegChain* ca, Surf* a, ISegChain* cb, Surf* b, double tol )
{
    vec3d n = a->GetPlaneNorm();
    CoplanarCurve crva( ca, a );
    CoplanarCurve crvb( cb, b );

    int na = ( int )ca->m_ISegVec.size();
    int nb = ( int )cb->m_ISegVec.size();

    for ( int i = 0 ; i < na ; i++ )
    {
        vec3d a0 = ca->m_ISegVec[i]->m_IPnt[0]->m_Pnt;
        vec3d a1 = ca->m_ISegVec[i]->m_IPnt[1]->m_Pnt;
        double la = dist( a0, a1 );

        for ( int j = 0 ; j < nb ; j++ )
        {
            vec3d b0 = cb->m_ISegVec[j]->m_IPnt[0]->m_Pnt;
            vec3d b1 = cb->m_ISegVec[j]->m_IPnt[1]->m_Pnt;
            double lb = dist( b0, b1 );

            // Far enough apart that neither curve can bend from its chord to the other
            double reach = 0.5 * std::max( la, lb ) + tol;
            BndBox boxa, boxb;
            boxa.Update( a0 );
            boxa.Update( a1 );
            boxb.Update( b0 );
            boxb.Update( b1 );
            if ( !Compare( boxa, boxb, reach ) )
            {
                continue;
            }

            double s = i + 0.5;
            double t = j + 0.5;
            if ( RefineCrossing( crva, crvb, n, s, t, tol ) )
            {
                AddCoplanarSplit( ca, a, s );
                AddCoplanarSplit( cb, b, t );
            }

            // The ends of either chain that lie on the other
            if ( j == 0 || j == nb - 1 )
            {
                vec3d e = b0;
                if ( j == nb - 1 )
                {
                    e = b1;
                }
                if ( j == 0 && nb == 1 )
                {
                    // One segment has both ends here; the second is taken below.
                    double s1 = i + 0.5;
                    if ( ProjectOnCurve( crva, b1, s1, tol ) )
                    {
                        AddCoplanarSplit( ca, a, s1 );
                    }
                }
                s = i + 0.5;
                if ( ProjectOnCurve( crva, e, s, tol ) )
                {
                    AddCoplanarSplit( ca, a, s );
                }
            }
            if ( i == 0 || i == na - 1 )
            {
                vec3d e = a0;
                if ( i == na - 1 )
                {
                    e = a1;
                }
                if ( i == 0 && na == 1 )
                {
                    double t1 = j + 0.5;
                    if ( ProjectOnCurve( crvb, a1, t1, tol ) )
                    {
                        AddCoplanarSplit( cb, b, t1 );
                    }
                }
                t = j + 0.5;
                if ( ProjectOnCurve( crvb, e, t, tol ) )
                {
                    AddCoplanarSplit( cb, b, t );
                }
            }
        }
    }
}

// The chains on a surface, in the order of the chain list; only its border chains where
// borders_only.
static void CoplanarChainsOn( Surf* s, const list< ISegChain* > &chains, bool borders_only, vector< ISegChain* > &out )
{
    out.clear();
    for ( list< ISegChain* >::const_iterator c = chains.begin() ; c != chains.end(); ++c )
    {
        if ( ( ( *c )->m_BorderFlag || !borders_only ) && ( ( *c )->m_SurfA == s || ( *c )->m_SurfB == s ) )
        {
            out.push_back( *c );
        }
    }
}

static double ChainLength( ISegChain* c )
{
    double len = 0.0;
    for ( int i = 0 ; i < ( int )c->m_ISegVec.size() ; i++ )
    {
        len += dist( c->m_ISegVec[i]->m_IPnt[0]->m_Pnt, c->m_ISegVec[i]->m_IPnt[1]->m_Pnt );
    }
    return len;
}

static BndBox ChainBox( ISegChain* c )
{
    BndBox box;
    for ( int i = 0 ; i < ( int )c->m_ISegVec.size() ; i++ )
    {
        box.Update( c->m_ISegVec[i]->m_IPnt[0]->m_Pnt );
        box.Update( c->m_ISegVec[i]->m_IPnt[1]->m_Pnt );
    }
    return box;
}

// Called before SplitBorderCurves and IntersectSplitChains, which apply the splits made here
// with their own.  Every chain on either surface is split where a border of the other crosses
// it, so that a border laid onto a surface later meets the chains already there at their ends.
void SurfaceIntersectionSingleton::SplitCoplanarBorders()
{
    vector< ISegChain* > chainsa, chainsb;

    for ( int p = 0 ; p < ( int )m_CoplanarPairs.size() ; p++ )
    {
        Surf* a = m_CoplanarPairs[p].first;
        Surf* b = m_CoplanarPairs[p].second;
        double tol = CoplanarTol( a, b );

        CoplanarChainsOn( a, m_ISegChainList, false, chainsa );
        CoplanarChainsOn( b, m_ISegChainList, false, chainsb );

        // A chain collapsed to a point neither splits nor is split.
        for ( int i = 0 ; i < ( int )chainsa.size() ; i++ )
        {
            if ( ChainLength( chainsa[i] ) <= tol )
            {
                continue;
            }
            BndBox boxa = ChainBox( chainsa[i] );

            for ( int j = 0 ; j < ( int )chainsb.size() ; j++ )
            {
                if ( ( chainsa[i]->m_BorderFlag || chainsb[j]->m_BorderFlag ) &&
                     chainsa[i] != chainsb[j] && ChainLength( chainsb[j] ) > tol &&
                     Compare( boxa, ChainBox( chainsb[j] ), tol ) )
                {
                    SplitCoplanarPair( chainsa[i], a, chainsb[j], b, tol );
                }
            }
        }
    }
}

// Is the point on surface s, within tol?  Projecting reaches into the surface's scratch
// buffers, so this is asked from one thread only.
static bool PntOnCoplanarSurf( Surf* s, const vec3d &p, double tol, vec2d &uw )
{
    BndBox box = s->GetBBox();
    box.Expand( tol );
    if ( !box.CheckPnt( p ) )
    {
        return false;
    }

    uw = s->ClosestUW( p );
    return dist( s->CompPnt( uw[0], uw[1] ), p ) <= tol;
}

// Distance from p to the nearest of the chains' tessellations
static double DistToChains( const vec3d &p, const vector< ISegChain* > &chains )
{
    double dmin = 1.0e12;
    for ( int i = 0 ; i < ( int )chains.size() ; i++ )
    {
        const vector< IPnt* > &tv = chains[i]->m_TessVec;
        for ( int j = 1 ; j < ( int )tv.size() ; j++ )
        {
            double d = dist( p, proj_pnt_on_line_seg( tv[j - 1]->m_Pnt, tv[j]->m_Pnt, p ) );
            if ( d < dmin )
            {
                dmin = d;
            }
        }
    }
    return dmin;
}

// Lay the borders of from that lie inside to onto it, each through the points it was
// tessellated with, so the two meshes meet node for node.  A border along one of to's own, or
// between from and another surface in its plane, is not wanted.
void SurfaceIntersectionSingleton::CopyCoplanarBorders( Surf* from, Surf* to, double tol )
{
    vector< ISegChain* > fromchains, tochains;
    CoplanarChainsOn( from, m_ISegChainList, true, fromchains );
    CoplanarChainsOn( to, m_ISegChainList, false, tochains );

    for ( int i = 0 ; i < ( int )fromchains.size() ; i++ )
    {
        ISegChain* c = fromchains[i];

        if ( c->m_SurfA == to || c->m_SurfB == to || c->m_TessVec.size() < 3 )
        {
            continue;
        }

        Surf* other = c->m_SurfA;
        if ( other == from )
        {
            other = c->m_SurfB;
        }
        if ( other != from && other->SamePlane( from, CoplanarTol( other, from ) ) )
        {
            continue;
        }

        if ( !Compare( ChainBox( c ), to->GetBBox(), tol ) )
        {
            continue;
        }

        // Only the points between the ends are asked: the ends are where the border crosses
        // to's own, and a surface narrower than the tolerance chain ends were joined to can leave
        // one just off it.
        int n = ( int )c->m_TessVec.size();
        vector< vec2d > uws( n );
        bool inside = true;
        for ( int j = 1 ; j < n - 1 && inside ; j++ )
        {
            inside = PntOnCoplanarSurf( to, c->m_TessVec[j]->m_Pnt, tol, uws[j] );
        }

        if ( !inside || DistToChains( c->m_TessVec[ n / 2 ]->m_Pnt, tochains ) <= tol )
        {
            continue;
        }

        ISegChain* nc = new ISegChain();
        nc->m_SurfA = to;
        nc->m_SurfB = to;
        nc->m_BorderFlag = true;

        // Each point keeps the place the border gave it, which is where the meshes take their
        // nodes on a chain from.  An end already joined to one of to's own keeps that point.
        for ( int j = 0 ; j < n ; j++ )
        {
            IPnt* ip = c->m_TessVec[j];
            if ( ( j > 0 && j < n - 1 ) || !ip->GetPuw( to ) )
            {
                if ( j == 0 || j == n - 1 )
                {
                    uws[j] = to->ClosestUW( ip->m_Pnt );
                }

                Puw* puwa = new Puw( to, uws[j] );
                Puw* puwb = new Puw( to, uws[j] );
                m_DelPuwVec.push_back( puwa );
                m_DelPuwVec.push_back( puwb );

                ip = new IPnt( puwa, puwb );
                ip->m_Pnt = c->m_TessVec[j]->m_Pnt;
                nc->m_CreatedIPnts.push_back( ip );
            }
            nc->m_TessVec.push_back( ip );

            if ( j > 0 )
            {
                nc->m_ISegVec.push_back( new ISeg( to, to, nc->m_TessVec[j - 1], ip ) );
            }
        }

        // The chains were given their curves before this one existed; the CAD and POGS
        // writers read them.
        nc->BuildCurves();

        m_ISegChainList.push_back( nc );
    }
}

// Called once the chains are tessellated and their ends joined, before the surfaces are meshed.
void SurfaceIntersectionSingleton::AddCoplanarChains()
{
    for ( int p = 0 ; p < ( int )m_CoplanarPairs.size() ; p++ )
    {
        Surf* a = m_CoplanarPairs[p].first;
        Surf* b = m_CoplanarPairs[p].second;
        double tol = CoplanarTol( a, b );

        Surf* kept = CoplanarKept( a, b );

        if ( kept != b )
        {
            CopyCoplanarBorders( a, b, tol );
        }
        if ( kept != a )
        {
            CopyCoplanarBorders( b, a, tol );
        }
    }
}

// The components joined to comp through faces lying one on the other.
void SurfaceIntersectionSingleton::JoinedComps( int comp, vector< int > &comps ) const
{
    comps.clear();

    for ( int i = 0 ; i < ( int )m_SurfVec.size() ; i++ )
    {
        Surf* other = m_SurfVec[i]->GetCoincidentSurf();
        if ( other && m_SurfVec[i]->GetCompID() == comp &&
             std::find( comps.begin(), comps.end(), other->GetCompID() ) == comps.end() )
        {
            comps.push_back( other->GetCompID() );
        }
    }
}

// Whether p lies inside component comp, by the majority of three rays along the axes.  Two
// bodies joined through their faces meet along an edge their surfaces run tangent to, and a
// single ray from near it can run along the other body's surface.
bool SurfaceIntersectionSingleton::InsideByThreeRays( int comp, const vec3d &p, double len ) const
{
    vec3d p0 = p;
    vec3d dirs[3] = { vec3d( len, 1.0e-4, 1.0e-4 ), vec3d( 1.0e-4, len, 1.0e-4 ), vec3d( 1.0e-4, 1.0e-4, len ) };

    int votes = 0;
    for ( int d = 0 ; d < 3 ; d++ )
    {
        vec3d p1 = p + dirs[d];

        vector< double > t;
        for ( int i = 0 ; i < ( int )m_SurfVec.size() ; i++ )
        {
            int type = m_SurfVec[i]->GetSurfaceCfdType();
            if ( m_SurfVec[i]->GetCompID() == comp && ( type == vsp::CFD_NORMAL || type == vsp::CFD_NEGATIVE ) )
            {
                m_SurfVec[i]->IntersectLineSeg( p0, p1, t );
            }
        }

        if ( t.size() % 2 == 1 )
        {
            votes++;
        }
    }

    return votes >= 2;
}

// For a face lying on a face of another body, that body's component and whether the face counts
// as inside it.  A ray cannot be trusted to say, so close to the other face.  A positive face on
// a negative one is inside it, so of the two faces only the negative one can remain.  Where the
// two bodies lie on the same side of the faces, the negative face, made to sit just outside the
// positive one, is outside it; where the bodies only meet at the faces, the negative face is
// the one face between them and counts as inside.  comp is -1 for any other surface.
void SurfaceIntersectionSingleton::CoincidentVerdict( Surf* s, int &comp, bool &inside ) const
{
    comp = -1;
    inside = false;

    Surf* other = s->GetCoincidentSurf();
    if ( other )
    {
        comp = other->GetCompID();
        inside = ( s->GetSurfaceCfdType() == vsp::CFD_NORMAL || !s->GetCoincidentOverlap() );
    }
}

// For a triangle of s centred on p: each component of a surface in s's plane that p lies on,
// and whether the triangle counts as inside it.
void SurfaceIntersectionSingleton::CoplanarVerdicts( Surf* s, const vec3d &p, vector< pair< int, bool > > &verdicts )
{
    verdicts.clear();

    const vector< Surf* > &others = s->GetCoplanarSurfs();
    for ( int i = 0 ; i < ( int )others.size() ; i++ )
    {
        vec2d uw;
        if ( PntOnCoplanarSurf( others[i], p, CoplanarTol( s, others[i] ), uw ) )
        {
            verdicts.push_back( pair< int, bool >( others[i]->GetCompID(), CoplanarKept( s, others[i] ) != s ) );
        }
    }
}

void SurfaceIntersectionSingleton::BuildIntChain( const string &id, vector < vector < vec3d > > & ptchains, vector < vector < vec3d > > & uwchains )
{
    vector < vector < vec3d > > ptvecvec;
    vector < vector < vec3d > > uwvecvec;

    list<ISegChain *>::iterator c;
    for ( c = m_ISegChainList.begin(); c != m_ISegChainList.end(); ++c )
    {
        if ( !(*c)->m_BorderFlag )
        {
            vector<vec3d> ptvec;
            vector<double> uvec;
            vector<vec3d> uwvec;

            SCurve * matchCurve = nullptr;
            if ( id == (*c)->m_ACurve.GetSurf()->GetGeomID() )
            {
                matchCurve = &(*c)->m_ACurve;
            }
            else
            {
                matchCurve = &(*c)->m_BCurve;
            }

            if ( matchCurve )
            {
                Bezier_curve xyzcrvA = matchCurve->GetUWCrv();
                xyzcrvA.TessAdaptXYZ( *( matchCurve->GetSurf() ), ptvec, .1, 8, uvec );

                RemoveRepeatedPnts( ptvec, uvec );

                for ( int i = 0 ; i < ( int )uvec.size() ; i++ )
                {
                    uwvec.push_back( xyzcrvA.CompPnt( uvec[ i ] ) );
                }

                ptvecvec.push_back( ptvec );
                uwvecvec.push_back( uwvec );
            }
        }
    }


    int nseg = ptvecvec.size();

    vector < bool > segused( nseg, false );

    vector < vector < vec3d > > keepptvec;
    vector < vector < vec3d > > keepuwvec;

    for ( int i = 0; i < ( int )ptvecvec.size(); i++ )
    {
        if ( !segused[ i ] )
        {
            keepptvec.push_back( ptvecvec[i] );
            keepuwvec.push_back( uwvecvec[i] );
            segused[ i ] = true;

            for ( int j = i + 1; j < ( int )ptvecvec.size(); j++ )
            {
                if ( !segused[ j ] )
                {
                    vec3d d = keepptvec.back().back() - ptvecvec[j].front();
                    if ( d.mag() < PT_MERGE_TOL )
                    {
                        keepptvec.back().insert( keepptvec.back().end(), ptvecvec[j].begin() + 1, ptvecvec[j].end() );
                        keepuwvec.back().insert( keepuwvec.back().end(), uwvecvec[j].begin() + 1, uwvecvec[j].end() );
                        segused[ j ] = true;
                        j = i;    // The loop increment restarts the search at i + 1
                    }
                    else
                    {
                        d = keepptvec.back().back() - ptvecvec[j].back();
                        if ( d.mag() < PT_MERGE_TOL )
                        {
                            keepptvec.back().insert( keepptvec.back().end(), ptvecvec[j].rbegin() + 1, ptvecvec[j].rend() );
                            keepuwvec.back().insert( keepuwvec.back().end(), uwvecvec[j].rbegin() + 1, uwvecvec[j].rend() );
                            segused[ j ] = true;
                            j = i;    // The loop increment restarts the search at i + 1
                        }
                    }
                }
            }
        }
    }

    ptchains = keepptvec;
    uwchains = keepuwvec;
}

void SurfaceIntersectionSingleton::RecordIntCurves()
{
    m_RawCurveAVec.clear();
    m_RawCurveBVec.clear();
    m_BorderCurveFlagVec.clear();
    m_PatchJoinFlagVec.clear();
    m_NonManifoldCurveFlagVec.clear();

    vector< ISegChain* > chains;
    vector< Surf* > sa;
    vector< Surf* > sb;

    list<ISegChain *>::iterator c;
    for ( c = m_ISegChainList.begin(); c != m_ISegChainList.end(); ++c )
    {
        m_BorderCurveFlagVec.push_back( (*c)->m_BorderFlag );
        m_PatchJoinFlagVec.push_back( (*c)->m_PatchJoinFlag );

        // Valence: two distinct parents is a curve between two patches; the same surface
        // on both sides means it bounds only one, and the mesh is not manifold along it.
        m_NonManifoldCurveFlagVec.push_back(
                (*c)->m_ACurve.GetSurf() == (*c)->m_BCurve.GetSurf() );

        chains.push_back( *c );
        sa.push_back( (*c)->m_ACurve.GetSurf() );
        sb.push_back( (*c)->m_BCurve.GetSurf() );
    }

    // Each side of a chain is taken to space through its own surface, so the two sides are
    // separate pieces of work: side 2k is chain k's A side, 2k + 1 its B side.
    int n = ( int )chains.size();
    m_RawCurveAVec.resize( n );
    m_RawCurveBVec.resize( n );

    vector< Surf* > side_surf( 2 * n );
    for ( int k = 0 ; k < n ; k++ )
    {
        side_surf[ 2 * k ] = sa[k];
        side_surf[ 2 * k + 1 ] = sb[k];
    }

    ProgressStage bar( this, "Record Curves", 2 * n );

    RunBySurface( side_surf, [&]( int i )
    {
        int k = i / 2;
        if ( i % 2 == 0 )
        {
            Bezier_curve xyzcrvA = chains[k]->m_ACurve.GetUWCrv();
            xyzcrvA.UWCurveToXYZCurve( sa[k] );
            xyzcrvA.GetControlPoints( m_RawCurveAVec[k] );
        }
        else
        {
            Bezier_curve xyzcrvB = chains[k]->m_BCurve.GetUWCrv();
            xyzcrvB.UWCurveToXYZCurve( sb[k] );
            xyzcrvB.GetControlPoints( m_RawCurveBVec[k] );
        }
    }, &bar );
}

void SurfaceIntersectionSingleton::BuildChainCADCurves()
{
    m_ChainCADVec.clear();
    m_ChainCADVec.resize( m_ISegChainList.size() );

    // A chain's curve is adapted on its two surfaces alone, so chains on different surfaces are
    // done on several threads, and the warnings said in order after.
    vector< ISegChain* > chains( m_ISegChainList.begin(), m_ISegChainList.end() );
    int n = ( int )chains.size();
    vector< Surf* > sa( n ), sb( n );
    for ( int k = 0 ; k < n ; k++ )
    {
        sa[k] = chains[k]->m_ACurve.GetSurf();
        sb[k] = chains[k]->m_BCurve.GetSurf();
    }
    vector< string > warn( n );

    ProgressStage bar( this, "Adapt CAD Curves", n );

    RunChainsBySurface( sa, sb, [&]( int k )
    {
        ISegChain* c = chains[k];
        NURBS_Curve &crv = m_ChainCADVec[ k ];

        // A patch join runs along a parameter line of its surface, as a border does, and is
        // drawn as that line
        crv.m_BorderFlag = c->m_BorderFlag || c->m_PatchJoinFlag;
        crv.m_SurfA_ID = c->m_SurfA->GetSurfID();
        crv.m_SurfB_ID = c->m_SurfB->GetSurfID();

        crv.InitCAD( c->m_ACurve, c->m_BCurve, GetSettingsPtr()->m_STEPTol, &warn[k] );
    }, &bar );

    bar.End();

    for ( int k = 0 ; k < n ; k++ )
    {
        printf( "%s", warn[k].c_str() );
    }
}

void SurfaceIntersectionSingleton::UpdateDrawObjs()
{
    // One color for each kind of curve, from the Okabe-Ito palette: each dark enough to read on
    // white, and told apart with red-green color blindness
    vec3d isect_color( 0.84, 0.37, 0 );
    vec3d border_color( 0, 0.45, 0.70 );
    vec3d join_color( 0, 0.62, 0.45 );

    // Draw ISegChains
    m_RawIsectCurveDO.m_GeomID = GetID() + "RAWISECTCURVE";
    m_RawIsectCurveDO.m_Type = DrawObj::VSP_LINES;
    m_RawIsectCurveDO.m_LineColor = isect_color;
    m_RawIsectCurveDO.m_LineWidth = 2.0;

    m_RawIsectPtsDO.m_GeomID = GetID() + "RAWISECTPTS";
    m_RawIsectPtsDO.m_Type = DrawObj::VSP_POINTS;
    m_RawIsectPtsDO.m_PointColor = vec3d(0.5, 0.5, 0.5);
    m_RawIsectPtsDO.m_PointSize = 10.0;

    m_RawBorderCurveDO.m_GeomID = GetID() + "RAWBORDERCURVE";
    m_RawBorderCurveDO.m_Type = DrawObj::VSP_LINES;
    m_RawBorderCurveDO.m_LineColor = border_color;
    m_RawBorderCurveDO.m_LineWidth = 2.0;

    // Curves that bound one patch rather than two, drawn apart from the rest, so a surface
    // laid against itself can be seen where it happens.
    m_RawNonManifoldCurveDO.m_GeomID = GetID() + "RAWNONMANIFOLDCURVE";
    m_RawNonManifoldCurveDO.m_Type = DrawObj::VSP_LINES;
    m_RawNonManifoldCurveDO.m_LineColor = vec3d( 0, 0, 0 );
    m_RawNonManifoldCurveDO.m_LineWidth = 3.0;

    m_RawNonManifoldPtsDO.m_GeomID = GetID() + "RAWNONMANIFOLDPTS";
    m_RawNonManifoldPtsDO.m_Type = DrawObj::VSP_POINTS;
    m_RawNonManifoldPtsDO.m_PointColor = vec3d( 0, 0, 0 );
    m_RawNonManifoldPtsDO.m_PointSize = 12.0;

    m_RawPatchJoinCurveDO.m_GeomID = GetID() + "RAWPATCHJOINCURVE";
    m_RawPatchJoinCurveDO.m_Type = DrawObj::VSP_LINES;
    m_RawPatchJoinCurveDO.m_LineColor = join_color;
    m_RawPatchJoinCurveDO.m_LineWidth = 2.0;

    m_RawPatchJoinPtsDO.m_GeomID = GetID() + "RAWPATCHJOINPTS";
    m_RawPatchJoinPtsDO.m_Type = DrawObj::VSP_POINTS;
    m_RawPatchJoinPtsDO.m_PointColor = vec3d( 0.5, 0.5, 0.5 );
    m_RawPatchJoinPtsDO.m_PointSize = 10.0;

    const char* cubic_name[ NUM_CUBIC ] = { "ISECT", "BORDER", "JOIN" };
    vec3d cubic_color[ NUM_CUBIC ] = { isect_color, border_color, join_color };

    for ( int k = 0; k < NUM_CUBIC; k++ )
    {
        m_CubicCurveDO[k].m_GeomID = GetID() + "CUBICCURVE" + cubic_name[k];
        m_CubicCurveDO[k].m_Type = DrawObj::VSP_LINES;
        m_CubicCurveDO[k].m_LineColor = cubic_color[k];
        m_CubicCurveDO[k].m_LineWidth = 2.0;

        m_CubicEndPtsDO[k].m_GeomID = GetID() + "CUBICENDPTS" + cubic_name[k];
        m_CubicEndPtsDO[k].m_Type = DrawObj::VSP_POINTS;
        m_CubicEndPtsDO[k].m_PointColor = vec3d( 0, 0, 0 );
        m_CubicEndPtsDO[k].m_PointSize = 10.0;

        m_CubicCtrlPtsDO[k].m_GeomID = GetID() + "CUBICCTRLPTS" + cubic_name[k];
        m_CubicCtrlPtsDO[k].m_Type = DrawObj::VSP_POINTS;
        m_CubicCtrlPtsDO[k].m_PointColor = vec3d( 0.5, 0.5, 0.5 );
        m_CubicCtrlPtsDO[k].m_PointSize = 7.0;
    }

    // Each curve drawn as the polyline its trimming loops are built from, with the ends of its
    // segments and its inner control points
    for ( size_t c = 0; c < m_ChainCADVec.size(); c++ )
    {
        const NURBS_Curve &crv = m_ChainCADVec[c];
        const vector < vec3d > &cp = crv.m_CADPntVec;
        int deg = crv.m_CADDeg;

        int k = CUBIC_ISECT;
        if ( m_PatchJoinFlagVec[c] )
        {
            k = CUBIC_JOIN;
        }
        else if ( m_BorderCurveFlagVec[c] )
        {
            k = CUBIC_BORDER;
        }

        for ( size_t i = 1; i < crv.m_PntVec.size(); i++ )
        {
            m_CubicCurveDO[k].m_PntVec.push_back( crv.m_PntVec[i - 1] );
            m_CubicCurveDO[k].m_PntVec.push_back( crv.m_PntVec[i] );
        }

        for ( size_t i = 0; i < cp.size(); i++ )
        {
            if ( i % deg == 0 )
            {
                m_CubicEndPtsDO[k].m_PntVec.push_back( cp[i] );
            }
            else
            {
                m_CubicCtrlPtsDO[k].m_PntVec.push_back( cp[i] );
            }
        }
    }

    m_RawBorderPtsDO.m_GeomID = GetID() + "RAWBORDERPTS";
    m_RawBorderPtsDO.m_Type = DrawObj::VSP_POINTS;
    m_RawBorderPtsDO.m_PointColor = vec3d(0.5, 0.5, 0.5);
    m_RawBorderPtsDO.m_PointSize = 10.0;

    for ( int indx = 0; indx < m_RawCurveAVec.size(); indx++ )
    {
        DrawObj *rawcurveDO;
        DrawObj *rawptsDO;

        if ( m_PatchJoinFlagVec[indx] )
        {
            rawcurveDO = &m_RawPatchJoinCurveDO;
            rawptsDO = &m_RawPatchJoinPtsDO;
        }
        else if ( m_NonManifoldCurveFlagVec[indx] )
        {
            rawcurveDO = &m_RawNonManifoldCurveDO;
            rawptsDO = &m_RawNonManifoldPtsDO;
        }
        else if ( m_BorderCurveFlagVec[indx] )
        {
            rawcurveDO = &m_RawBorderCurveDO;
            rawptsDO = &m_RawBorderPtsDO;
        }
        else
        {
            rawcurveDO = &m_RawIsectCurveDO;
            rawptsDO = &m_RawIsectPtsDO;
        }

        const vector < vec3d > *rawcrv[2] = { &m_RawCurveAVec[ indx ], &m_RawCurveBVec[ indx ] };

        for ( int k = 0; k < 2; k++ )
        {
            const vector < vec3d > &rawptvec = *rawcrv[k];

            rawptsDO->m_PntVec.insert( rawptsDO->m_PntVec.end(), rawptvec.begin(), rawptvec.end() );

            for ( int j = 1; j < rawptvec.size(); j++ )
            {
                rawcurveDO->m_PntVec.push_back( rawptvec[j - 1] );
                rawcurveDO->m_PntVec.push_back( rawptvec[j] );
            }
        }
    }

    m_RawIsectCurveDO.m_GeomChanged = true;
    m_RawIsectPtsDO.m_GeomChanged = true;
    m_RawBorderCurveDO.m_GeomChanged = true;
    m_RawBorderPtsDO.m_GeomChanged = true;

    for ( int k = 0; k < NUM_CUBIC; k++ )
    {
        m_CubicCurveDO[k].m_GeomChanged = true;
        m_CubicEndPtsDO[k].m_GeomChanged = true;
        m_CubicCtrlPtsDO[k].m_GeomChanged = true;
    }

    m_RawPatchJoinCurveDO.m_GeomChanged = true;
    m_RawPatchJoinPtsDO.m_GeomChanged = true;

    m_RawNonManifoldCurveDO.m_GeomChanged = true;
    m_RawNonManifoldPtsDO.m_GeomChanged = true;

    //=====  Visualizatino tools for SurfaceINtersectionMgr debugging =====//
    if ( DEBUG_ISECT_DRAW )
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
        m_ApproxPlanesDO.m_LineColor = vec3d( .2, .2, .2 );
        m_ApproxPlanesDO.m_LineWidth = 1.0;
        m_ApproxPlanesDO.m_NormVec = m_ApproxPlanesDO.m_PntVec;
        m_ApproxPlanesDO.m_GeomChanged = true;

        m_DelPtsDO.m_GeomID = GetID() + "m_DelPtsDO";
        m_DelPtsDO.m_Type = DrawObj::VSP_POINTS;
        m_DelPtsDO.m_PointColor = vec3d( .2, .4, .6 );
        m_DelPtsDO.m_PointSize = 10.0;
        m_DelPtsDO.m_PntVec.clear();
        m_DelPtsDO.m_GeomChanged = true;

        for ( int indx = 0; indx < m_DelIPntVec.size(); indx++ )
        {
            m_DelPtsDO.m_PntVec.push_back( m_DelIPntVec[indx]->m_Pnt );
        }

        if ( m_IPatchADrawLines.size() > 0 && m_IPatchBDrawLines.size() > 0 )
        {
            m_IPatchADO.clear();
            m_IPatchADO.resize( m_IPatchADrawLines.size() );

            for ( size_t i = 0; i < m_IPatchADrawLines.size(); i++ )
            {
                m_IPatchADO[i].m_GeomID = GetID() + "IPatchA_" + to_string( i );
                m_IPatchADO[i].m_LineColor = vec3d( .4, .5, .6 );
                m_IPatchADO[i].m_LineWidth = 1.5;
                m_IPatchADO[i].m_PntVec.clear();
                m_IPatchADO[i].m_GeomChanged = true;

                m_IPatchADO[i].m_Type = m_IPatchBDO[i].VSP_LINES;

                for ( int indx = 0; indx < m_IPatchADrawLines[i].size(); indx++ )
                {
                    m_IPatchADO[i].m_PntVec.push_back( m_IPatchADrawLines[i][indx] );
                }
            }

            m_IPatchBDO.clear();
            m_IPatchBDO.resize( m_IPatchBDrawLines.size() );

            for ( size_t i = 0; i < m_IPatchBDrawLines.size(); i++ )
            {
                m_IPatchBDO[i].m_GeomID = GetID() + "IPatchB_" + to_string( i );
                m_IPatchBDO[i].m_LineColor = vec3d( .7, .8, .9 );
                m_IPatchBDO[i].m_LineWidth = 1.5;
                m_IPatchBDO[i].m_PntVec.clear();
                m_IPatchBDO[i].m_GeomChanged = true;

                m_IPatchBDO[i].m_Type = m_IPatchBDO[i].VSP_LINES;

                for ( int indx = 0; indx < m_IPatchBDrawLines[i].size(); indx++ )
                {
                    m_IPatchBDO[i].m_PntVec.push_back( m_IPatchBDrawLines[i][indx] );
                }
            }
        }
    }
}

void SurfaceIntersectionSingleton::LoadDrawObjs( vector< DrawObj* > &draw_obj_vec )
{
    if ( m_MeshInProgress )
    {
        return;
    }

    if ( m_DrawObjsStale )
    {
        m_DrawObjsStale = false;
        UpdateDrawObjs();
    }

    m_RawIsectCurveDO.m_Visible = GetSettingsPtr()->m_DrawIsectFlag &&
                                  GetSettingsPtr()->m_DrawCurveFlag &&
                                  GetSettingsPtr()->m_DrawRawFlag;

    m_RawIsectPtsDO.m_Visible = GetSettingsPtr()->m_DrawIsectFlag &&
                                GetSettingsPtr()->m_DrawPntsFlag &&
                                GetSettingsPtr()->m_DrawRawFlag;

    m_RawBorderCurveDO.m_Visible = GetSettingsPtr()->m_DrawBorderFlag &&
                                   GetSettingsPtr()->m_DrawCurveFlag &&
                                   GetSettingsPtr()->m_DrawRawFlag;

    bool kind_flag[ NUM_CUBIC ] = { GetSettingsPtr()->m_DrawIsectFlag,
                                    GetSettingsPtr()->m_DrawBorderFlag,
                                    GetSettingsPtr()->m_DrawJoinFlag };

    for ( int k = 0; k < NUM_CUBIC; k++ )
    {
        m_CubicCurveDO[k].m_Visible = kind_flag[k] &&
                                      GetSettingsPtr()->m_DrawCurveFlag &&
                                      GetSettingsPtr()->m_DrawCubicFlag;

        m_CubicEndPtsDO[k].m_Visible = kind_flag[k] &&
                                       GetSettingsPtr()->m_DrawPntsFlag &&
                                       GetSettingsPtr()->m_DrawCubicFlag;

        m_CubicCtrlPtsDO[k].m_Visible = m_CubicEndPtsDO[k].m_Visible;
    }

    m_RawPatchJoinCurveDO.m_Visible = GetSettingsPtr()->m_DrawJoinFlag &&
                                      GetSettingsPtr()->m_DrawCurveFlag &&
                                      GetSettingsPtr()->m_DrawRawFlag;

    m_RawPatchJoinPtsDO.m_Visible = GetSettingsPtr()->m_DrawJoinFlag &&
                                    GetSettingsPtr()->m_DrawPntsFlag &&
                                    GetSettingsPtr()->m_DrawRawFlag;

    m_RawBorderPtsDO.m_Visible = GetSettingsPtr()->m_DrawBorderFlag &&
                                 GetSettingsPtr()->m_DrawPntsFlag &&
                                 GetSettingsPtr()->m_DrawRawFlag;

    // A curve that bounds one patch rather than two may be either an intersection or a
    // border, so it follows whichever of the two is being shown.
    bool anycrv = GetSettingsPtr()->m_DrawIsectFlag || GetSettingsPtr()->m_DrawBorderFlag;

    m_RawNonManifoldCurveDO.m_Visible = anycrv &&
                                        GetSettingsPtr()->m_DrawCurveFlag &&
                                        GetSettingsPtr()->m_DrawRawFlag;

    m_RawNonManifoldPtsDO.m_Visible = anycrv &&
                                      GetSettingsPtr()->m_DrawPntsFlag &&
                                      GetSettingsPtr()->m_DrawRawFlag;

    draw_obj_vec.push_back( &m_RawIsectCurveDO );
    for ( int k = 0; k < NUM_CUBIC; k++ )
    {
        draw_obj_vec.push_back( &m_CubicCurveDO[k] );
        draw_obj_vec.push_back( &m_CubicEndPtsDO[k] );
        draw_obj_vec.push_back( &m_CubicCtrlPtsDO[k] );
    }
    draw_obj_vec.push_back( &m_RawIsectPtsDO );
    draw_obj_vec.push_back( &m_RawBorderCurveDO );
    draw_obj_vec.push_back( &m_RawNonManifoldCurveDO );
    draw_obj_vec.push_back( &m_RawNonManifoldPtsDO );
    draw_obj_vec.push_back( &m_RawPatchJoinCurveDO );
    draw_obj_vec.push_back( &m_RawPatchJoinPtsDO );
    draw_obj_vec.push_back( &m_RawBorderPtsDO );

    //=====  Visualizatino tools for SurfaceINtersectionMgr debugging =====//
    if ( DEBUG_ISECT_DRAW )
    {
        m_ApproxPlanesDO.m_Visible = false;
        draw_obj_vec.push_back( &m_ApproxPlanesDO );

        m_DelPtsDO.m_Visible = false;
        draw_obj_vec.push_back( &m_DelPtsDO );

        if ( m_IPatchADrawLines.size() > 0 && m_IPatchBDrawLines.size() > 0 )
        {
            for ( size_t i = 0; i < m_IPatchADrawLines.size(); i++ )
            {
                m_IPatchADO[i].m_Visible = false;
                draw_obj_vec.push_back( &m_IPatchADO[i] );
            }

            for ( size_t i = 0; i < m_IPatchBDrawLines.size(); i++ )
            {
                m_IPatchBDO[i].m_Visible = false;
                draw_obj_vec.push_back( &m_IPatchBDO[i] );
            }
        }
    }

    WakeMgr.Show( GetSettingsPtr()->m_DrawSourceWakeFlag );
    WakeMgr.LoadDrawObjs( draw_obj_vec);
}

bool SurfaceIntersectionSingleton::GetVisBndBox( BndBox &bbox )
{
    bool anyvisible = false;

    vector< DrawObj* > draw_obj_vec;
    LoadDrawObjs( draw_obj_vec );

    for(int j = 0; j < (int)draw_obj_vec.size(); j++)
    {
        if(draw_obj_vec[j]->m_Visible)
        {
            bbox.Update( draw_obj_vec[j]->m_PntVec );
            bbox.Update( draw_obj_vec[j]->m_PntMesh );
            anyvisible = true;
        }
    }

    return anyvisible;
}

void SurfaceIntersectionSingleton::UpdateWakes()
{
    vector< piecewise_curve_type > wake_leading_edges;
    vector < double > wake_scale_vec;
    vector < double > wake_angle_vec;

    int nset = GetSettingsPtr()->m_SelectedSetIndex;
    int dset = GetSettingsPtr()->m_SelectedDegenSetIndex;

    if ( GetSettingsPtr()->m_UseMode )
    {
        Mode *m = ModeMgr.GetMode( GetSettingsPtr()->m_ModeID );
        if ( m )
        {
            // Do not apply settings every time Sources and Wakes are updated.
            // m->ApplySettings();
            nset = m->m_NormalSet();
            dset = m->m_DegenSet();
        }
    }

    vector<string> geomVec = m_Vehicle->GetGeomVec();
    for ( int g = 0; g < (int)geomVec.size(); g++ )
    {
        Geom* geom = m_Vehicle->FindGeom( geomVec[g] );
        if ( geom )
        {
            if ( geom->GetSetFlag( nset ) || geom->GetSetFlag( dset ) )
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

    WakeMgr.UpdateDrawObjs();
}

// The curve replaced is kept for CleanUp, since a border curve may still name it
void SurfaceIntersectionSingleton::SetICurveVec( ICurve* newcurve, int loc )
{
    m_DelICurveVec.push_back( m_ICurveVec[loc] );
    m_ICurveVec[loc] = newcurve;
}


void SurfaceIntersectionSingleton::UpdateDisplaySettings()
{
    if ( GetSettingsPtr() )
    {
        GetSettingsPtr()->m_DrawSourceWakeFlag = m_Vehicle->GetISectSettingsPtr()->m_DrawSourceWakeFlag.Get();

        GetSettingsPtr()->m_DrawBorderFlag = m_Vehicle->GetISectSettingsPtr()->m_DrawBorderFlag.Get();
        GetSettingsPtr()->m_DrawIsectFlag = m_Vehicle->GetISectSettingsPtr()->m_DrawIsectFlag.Get();
        GetSettingsPtr()->m_DrawJoinFlag = m_Vehicle->GetISectSettingsPtr()->m_DrawJoinFlag.Get();
        GetSettingsPtr()->m_DrawRawFlag = m_Vehicle->GetISectSettingsPtr()->m_DrawRawFlag.Get();
        GetSettingsPtr()->m_DrawCubicFlag = m_Vehicle->GetISectSettingsPtr()->m_DrawCubicFlag.Get();
        GetSettingsPtr()->m_DrawCurveFlag = m_Vehicle->GetISectSettingsPtr()->m_DrawCurveFlag.Get();
        GetSettingsPtr()->m_DrawPntsFlag = m_Vehicle->GetISectSettingsPtr()->m_DrawPntsFlag.Get();

        // Needed by update wakes - called by screen::Update
        GetSettingsPtr()->m_SelectedSetIndex = m_Vehicle->GetISectSettingsPtr()->m_SelectedSetIndex.Get();
        GetSettingsPtr()->m_SelectedDegenSetIndex = m_Vehicle->GetISectSettingsPtr()->m_SelectedDegenSetIndex.Get();
        GetSettingsPtr()->m_UseMode = m_Vehicle->GetISectSettingsPtr()->m_UseMode.Get();
        GetSettingsPtr()->m_ModeID = m_Vehicle->GetISectSettingsPtr()->m_ModeID;
    }
}
