//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
//// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// ProjectionMgr.cpp: Projection Mgr Singleton.
//
//////////////////////////////////////////////////////////////////////

#define _USE_MATH_DEFINES
#include <cmath>

#include "ProjectionMgr.h"
#include "ParmMgr.h"
#include "Vehicle.h"
#include "MeshGeom.h"
#include "ModeMgr.h"

#include "triangle.h"
#include "triangle_api.h"

#include "StlHelper.h"

//==== Constructor ====//
ProjectionMgrSingleton::ProjectionMgrSingleton()
{
    m_TargetSetIndex = DEFAULT_SET;
    m_BoundarySetIndex = DEFAULT_SET;

    m_TargetGeomID = string();
    m_BoundaryGeomID = string();
    m_DirectionGeomID = string();

    Init();
}

void ProjectionMgrSingleton::Init()
{
}

void ProjectionMgrSingleton::Wype()
{
}

void ProjectionMgrSingleton::Renew()
{
    Wype();
    Init();
}

void ProjectionMgrSingleton::LoadDrawObjs( vector< DrawObj* > & draw_obj_vec )
{
    Vehicle *veh = VehicleMgr.GetVehicle();
    if ( !veh )
    {
        assert( false );
        return;
    }

}

void ProjectionMgrSingleton::SetGeomIDs( const string &tid, const string &bid, const string &did )
{
    m_TargetGeomID = tid;
    m_BoundaryGeomID = bid;
    m_DirectionGeomID = did;
}

vec3d ProjectionMgrSingleton::GetDirection( int dirtype, const string &dirid )
{
    vec3d dir;

    switch ( dirtype )
    {
        case vsp::X_PROJ:
            dir = vec3d( 1.0, 0.0, 0.0 );
            break;
        case vsp::Y_PROJ:
            dir = vec3d( 0.0, 1.0, 0.0 );
            break;
        case vsp::Z_PROJ:
            dir = vec3d( 0.0, 0.0, 1.0 );
            break;
        case vsp::GEOM_PROJ:
            Vehicle *veh = VehicleMgr.GetVehicle();
            if ( veh )
            {
                Geom* g = veh->FindGeom( dirid );
                if ( g )
                {
                    const VspSurf *s = g->GetSurfPtr(0);
                    if ( s )
                    {
                        dir = vec3d( 0, 0, 0 ) - s->CompNorm01( 0, 0 );
                    }
                }
            }
            break;
    }
    return dir;
}


void ProjectionMgrSingleton::UpdateDirection()
{
    Vehicle *veh = VehicleMgr.GetVehicle();
    vec3d dir = GetDirection( veh->m_DirectionType(), m_DirectionGeomID );

    switch ( veh->m_DirectionType() )
    {
        case vsp::X_PROJ:
        case vsp::Y_PROJ:
        case vsp::Z_PROJ:
        case vsp::GEOM_PROJ:
            veh->m_XComp = dir.x();
            veh->m_YComp = dir.y();
            veh->m_ZComp = dir.z();
            break;
    }
}

vec3d ProjectionMgrSingleton::GetDirection()
{
    Vehicle *veh = VehicleMgr.GetVehicle();
    vec3d v = vec3d( veh->m_XComp(), veh->m_YComp(), veh->m_ZComp() );
    v.normalize();
    return v;
}

Results* ProjectionMgrSingleton::Project( )
{
    Vehicle *veh = VehicleMgr.GetVehicle();

    int ttype = veh->m_TargetType();
    int tset = m_TargetSetIndex;

    bool thullflag = veh->m_TargetHullFlag();
    bool bhullflag = veh->m_BoundaryHullFlag();

    if ( ttype == vsp::MODE_TARGET )
    {
        Mode *m = ModeMgr.GetMode( m_ModeID );
        if ( m )
        {
            m->ApplySettings();
            tset = m->m_NormalSet();
        }
    }

    vec3d dir = GetDirection();

    switch ( veh->m_BoundaryType() )
    {
        case vsp::NO_BOUNDARY:
            if ( ttype == vsp::SET_TARGET || ttype == vsp::MODE_TARGET )
            {
                return Project( tset, thullflag, dir);
            }
            else
            {
                return Project( m_TargetGeomID, thullflag, dir);
            }
            break;
        case vsp::SET_BOUNDARY:
            if ( ttype == vsp::SET_TARGET || ttype == vsp::MODE_TARGET )
            {
                return Project( tset, thullflag, m_BoundarySetIndex, bhullflag, dir);
            }
            else
            {
                return Project( m_TargetGeomID, thullflag, m_BoundarySetIndex, bhullflag, dir);
            }
            break;
        case vsp::GEOM_BOUNDARY:
            if ( ttype == vsp::SET_TARGET || ttype == vsp::MODE_TARGET )
            {
                return Project( tset, thullflag, m_BoundaryGeomID, bhullflag, dir);
            }
            else
            {
                return Project( m_TargetGeomID, thullflag, m_BoundaryGeomID, bhullflag, dir);
            }
            break;
    }

    return nullptr;
}

inline double SphericalArea( const vector < vec3d > & azel )
{
    size_t cnt = azel.size();
    if ( cnt < 3 ) return 0.0;
    double a = 0.0;

    for ( size_t i = 0; i < cnt - 1; i++ )
    {
        // Order appears flipped to change direction of integration to fix sign of result.
        const double & az_2 = azel[i].y();
        const double & az_1 = azel[i + 1].y();
        const double & el_2 = azel[i].z();
        const double & el_1 = azel[i + 1].z();

        // Area of the trapezoid between the equator and a segment in az,el spherical coordinates.
        // https://en.wikipedia.org/wiki/Spherical_trigonometry#From_latitude_and_longitude
        const double E_4 = 2.0 * atan( tan( 0.5 * ( az_2 - az_1 ) ) * sin( 0.5 * ( el_2 + el_1 ) ) / cos( 0.5 * ( el_2 - el_1 ) ) );
        a += E_4;
    }
    return a;
}

string ProjectionMgrSingleton::PointVisibility( vector < TMesh* > &targetTMeshVec, vec3d cen, vector< TMesh* > & result_tmv, bool poly_visible,
                                                const vector<string> & cutout_vec )
{
    bool intSubsFlag = !cutout_vec.empty();
    CSGMesh( targetTMeshVec, intSubsFlag, cutout_vec );

    for ( int i = 0; i < ( int )targetTMeshVec.size(); i++ )
    {
        targetTMeshVec[i]->SetIgnoreSubSurface();
    }
    FlattenTMeshVec( targetTMeshVec );

    Matrix4d centranslatemat;
    // Equivalent to 180 deg rotation about Z, but without floating point error.
    centranslatemat.scalex( -1.0 );
    centranslatemat.scaley( -1.0 );

    centranslatemat.translatev( -cen );

    TransformMesh( targetTMeshVec, centranslatemat );

    TMesh *target_tm = MergeTMeshVec( targetTMeshVec );

    target_tm = OctantSplitMesh( target_tm );

    constexpr double scalerad = 1.0e15 / M_PI;
    BndBox bb;
    target_tm->UpdateBBox( bb );
    bb.Update( vec3d() ); // Make sure origin is in BBox
    const double r = std::max( 1.0, bb.DiagDist() );

    Matrix4d clipper2sphericalmat;
    clipper2sphericalmat.translatef( r, 0, 0 );
    clipper2sphericalmat.scaley( 1.0 / scalerad );
    clipper2sphericalmat.scalez( 1.0 / scalerad );

    Clipper2Lib::Paths64 targetvec;
    MeshToSphericalPathsVec( target_tm, targetvec, scalerad );

    Dump( targetvec, "before.m" );

    Clipper2Lib::Paths64 solution;
    Union( targetvec, solution );

    // Calculate 'inverse sphere' -- draw polygon around visible region instead of occluded region.
    if ( poly_visible )
    {
        Clipper2Lib::Paths64 sphdomain;
        SphericalDomainPath( sphdomain, scalerad );

        Clipper2Lib::Clipper64 clpr;
        clpr.PreserveCollinear( false );
        clpr.AddSubject( sphdomain );
        clpr.AddClip( solution );

        Clipper2Lib::Paths64 sol;
        if ( !clpr.Execute( Clipper2Lib::ClipType::Difference, Clipper2Lib::FillRule::NonZero, sol ) )
        {
            printf( "Clipper error\n" );
        }
        solution = sol;
    }

    ClosePaths( solution );

    // Remove small polygons.
    // In some circumstances, near zero-area slivers can be created.  This filters them out.
    // If the slivers remain, Triangle will crash when trying to triangulate the domain.
    Clipper2Lib::Paths64 clean;
    for ( int i = 0; i < solution.size(); i++ )
    {
        // This 'area' is calculated in scaled azimuth/elevation space and then un-scaled.
        // The full domain has an area of 2 pi^2 ~= 19.7 when un-scaled.
        // Typical slivers are observed with an area of 10-16 or smaller.
        double area = Clipper2Lib::Area( solution[i] ) / ( scalerad * scalerad );
        if ( std::abs( area ) > 1.0e-10 )
        {
            clean.push_back( solution[i] );
        }
    }
    solution = clean;

    // Calculate elevation bounds of visibility domain at zero azimuth
    vector < double > fwdbounds;
    for ( int i = 0; i < solution.size(); i++ )
    {
        for ( int j = 0; j < solution[i].size() - 1; j++ )
        {
            if ( solution[i][j].x == 0 )
            {
                fwdbounds.push_back( solution[i][j].y / scalerad * 180.0 / M_PI );
            }
            else if (  solution[i][j+1].x != 0 && sgn( solution[i][j].x ) != sgn( solution[i][j + 1].x ) )
            {
                double f = ( 0.0 - solution[i][j].x ) / ( solution[i][j+1].x - solution[i][j].x );
                double y = solution[i][j].y + f * ( solution[i][j+1].y - solution[i][j].y );
                fwdbounds.push_back( y / scalerad * 180.0 / M_PI );
            }
        }
    }

    // Find first forward visibility bound below the horizon.
    double overnose = -90.0;
    for ( int i = 0; i < fwdbounds.size(); i++ )
    {
        if ( fwdbounds[i] < 0.0 )
        {
            overnose = std::max( overnose, fwdbounds[i] );
        }
    }



    vector < vector < vec3d > > solutionPolyVec3d;
    vector < vector < vec3d > > solutionPolyVecSpherical;
    vector < double > oct_area;
    vector < string > oct_label;

   // if ( solution.size() > 0 )
    {
        // Calculate per-octant area fractions.
        for ( int ioct = 0; ioct < 8; ioct++ )
        {
            Clipper2Lib::Paths64 sphdomain;
            string label;
            OctantDomainPath( ioct, sphdomain, scalerad, label );

            Clipper2Lib::Clipper64 clpr;
            clpr.PreserveCollinear( false );
            clpr.AddSubject( sphdomain );
            clpr.AddClip( solution );

            Clipper2Lib::Paths64 sol;
            if ( !clpr.Execute( Clipper2Lib::ClipType::Intersection, Clipper2Lib::FillRule::NonZero, sol ) )
            {
                printf( "Clipper error\n" );
            }
            ClosePaths( sol );

            vector < vector < vec3d > > solPolyVec3d;

            PathsToPolyVec( sol, solPolyVec3d, 1, 2 ); // put az, el results in 1, 2 components of vec3d
            TransformPolyVec( solPolyVec3d, clipper2sphericalmat );

            double asum = 0;
            for ( int i = 0; i < solPolyVec3d.size(); i++ )
            {
                double area = SphericalArea( solPolyVec3d[i] );
                asum += area;
            }

            oct_area.push_back( 2.0 * asum / M_PI ); // Area fraction.
            oct_label.push_back( label );
        }

        vector < bool > isHole;
        MarkHoles( solution, isHole );

        PathsToPolyVec( solution, solutionPolyVec3d, 1, 2 ); // put az, el results in 1, 2 components of vec3d

        RefinePolyVec( solutionPolyVec3d, scalerad );

        TransformPolyVec( solutionPolyVec3d, clipper2sphericalmat );

        // Put y,z coordinates into 2d vec
        vector < vector < vec2d > > solutionPolyVec2d;
        Poly3dToPoly2d( solutionPolyVec3d, solutionPolyVec2d );

        // Copy spherical polyvec
        solutionPolyVecSpherical = solutionPolyVec3d;

        Matrix4d spherical2degmat;
        spherical2degmat.scaley( 180.0 / M_PI );
        spherical2degmat.scalez( 180.0 / M_PI );

        TransformPolyVec( solutionPolyVecSpherical, spherical2degmat );

        Dump( solutionPolyVecSpherical, "tri.m" );

        result_tmv = Triangulate( solutionPolyVec2d, solutionPolyVec3d, isHole, true, r );

        MeshToCartesian( result_tmv );

        PolyVecToCartesian( solutionPolyVec3d );

        centranslatemat.affineInverse();
        TransformMesh( result_tmv, centranslatemat );

        TransformPolyVec( solutionPolyVec3d, centranslatemat );
    }

    int npt = 0;
    for ( int i = 0; i < solutionPolyVec3d.size(); i++ )
    {
        npt += 2 * solutionPolyVec3d[i].size();
    }

    vector < vec3d > pts;
    pts.reserve( npt );
    for ( int i = 0; i < solutionPolyVec3d.size(); i++ )
    {
        for ( int j = 0; j < solutionPolyVec3d[i].size() - 1; j++ )
        {
            pts.push_back( solutionPolyVec3d[i][j] );
            pts.push_back( solutionPolyVec3d[i][j + 1] );
        }
    }

    //==== Create Results ====//
    Results* res = ResultsMgr.CreateResults( "Point_Visibility", "Visibility from a point." );
    if ( res )
    {
        res->Add( new NameValData( "Center", cen, "Visibility center point." ) );
        res->Add( new NameValData( "Pts", pts, "Visibility domain boundary in cartesian coordinates." ) );
        res->Add( new NameValData( "Poly_Visible", poly_visible, "Flag for domain boundary to contain visible region (true) instead of occluded region (false)." ) );

        res->Add( new NameValData( "Octant_Area_Frac", oct_area, "Fraction of solid angle enclosed in domain boundary in each octant." ) );
        res->Add( new NameValData( "Octant_Labels", oct_label, "Labels for octants (Up/Down, Fore/Aft, Left/Right)." ) );

        res->Add( new NameValData( "Fwd_Bounds", fwdbounds, "Forward elevation bounds of visibility domain (degrees)." ) );
        res->Add( new NameValData( "Over_Nose", overnose, "Elevation of first forward visibility domain boundary down from the horizon (degrees)." ) );

        res->Add( new NameValData( "Result", overnose, "Point visibility result" ) );

        for ( int i = 0; i < solutionPolyVecSpherical.size(); i++ )
        {
            res->Add( new NameValData( "Vis_Boundary", solutionPolyVecSpherical[i], "Visibility domain boundary in spherical coordinates (r, az, el)." ) );
        }
    }

    return res->GetID();
}

Results* ProjectionMgrSingleton::Project( int tset, bool thullflag, const vec3d & dir )
{
    Vehicle* veh = VehicleMgr.GetVehicle();

    vector < TMesh* > targetTMeshVec = veh->CreateTMeshVec( tset );

    if ( thullflag )
    {
        TMesh *tm = MakeConvexHull( targetTMeshVec );
        DeleteTMeshVec( targetTMeshVec );
        targetTMeshVec.push_back( tm );
    }

    Results* res = Project( targetTMeshVec, dir );
    DeleteTMeshVec( targetTMeshVec );
    return res;
}

Results* ProjectionMgrSingleton::Project( int tset, bool thullflag, int bset, bool bhullflag, const vec3d & dir )
{
    Vehicle* veh = VehicleMgr.GetVehicle();

    vector < TMesh* > targetTMeshVec = veh->CreateTMeshVec( tset );
    vector < TMesh* > boundaryTMeshVec = veh->CreateTMeshVec( bset );

    if ( thullflag )
    {
        TMesh *tm = MakeConvexHull( targetTMeshVec );
        DeleteTMeshVec( targetTMeshVec );
        targetTMeshVec.push_back( tm );
    }

    if ( bhullflag )
    {
        TMesh *tm = MakeConvexHull( boundaryTMeshVec );
        DeleteTMeshVec( boundaryTMeshVec );
        boundaryTMeshVec.push_back( tm );
    }

    Results* res = Project( targetTMeshVec, boundaryTMeshVec, dir );

    DeleteTMeshVec( targetTMeshVec );
    DeleteTMeshVec( boundaryTMeshVec );
    return res;
}

Results* ProjectionMgrSingleton::Project( int tset, bool thullflag, const string &bgeom, bool bhullflag, const vec3d & dir )
{
    Vehicle* veh = VehicleMgr.GetVehicle();

    vector < TMesh* > targetTMeshVec = veh->CreateTMeshVec( tset );
    vector < TMesh* > boundaryTMeshVec = veh->CreateTMeshVec( bgeom );

    if ( thullflag )
    {
        TMesh *tm = MakeConvexHull( targetTMeshVec );
        DeleteTMeshVec( targetTMeshVec );
        targetTMeshVec.push_back( tm );
    }

    if ( bhullflag )
    {
        TMesh *tm = MakeConvexHull( boundaryTMeshVec );
        DeleteTMeshVec( boundaryTMeshVec );
        boundaryTMeshVec.push_back( tm );
    }

    Results* res = Project( targetTMeshVec, boundaryTMeshVec, dir );

    DeleteTMeshVec( targetTMeshVec );
    DeleteTMeshVec( boundaryTMeshVec );
    return res;
}

Results* ProjectionMgrSingleton::Project( const string &tgeom, bool thullflag, const vec3d & dir )
{
    Vehicle* veh = VehicleMgr.GetVehicle();

    vector < TMesh* > targetTMeshVec = veh->CreateTMeshVec( tgeom );

    if ( thullflag )
    {
        TMesh *tm = MakeConvexHull( targetTMeshVec );
        DeleteTMeshVec( targetTMeshVec );
        targetTMeshVec.push_back( tm );
    }

    Results* res = Project( targetTMeshVec, dir );
    DeleteTMeshVec( targetTMeshVec );
    return res;
}

Results* ProjectionMgrSingleton::Project( const string &tgeom, bool thullflag, int bset, bool bhullflag, const vec3d & dir )
{
    Vehicle* veh = VehicleMgr.GetVehicle();

    vector < TMesh* > targetTMeshVec = veh->CreateTMeshVec( tgeom );
    vector < TMesh* > boundaryTMeshVec = veh->CreateTMeshVec( bset );

    if ( thullflag )
    {
        TMesh *tm = MakeConvexHull( targetTMeshVec );
        DeleteTMeshVec( targetTMeshVec );
        targetTMeshVec.push_back( tm );
    }

    if ( bhullflag )
    {
        TMesh *tm = MakeConvexHull( boundaryTMeshVec );
        DeleteTMeshVec( boundaryTMeshVec );
        boundaryTMeshVec.push_back( tm );
    }

    Results* res = Project( targetTMeshVec, boundaryTMeshVec, dir );

    DeleteTMeshVec( targetTMeshVec );
    DeleteTMeshVec( boundaryTMeshVec );
    return res;
}

Results* ProjectionMgrSingleton::Project( const string &tgeom, bool thullflag, const string &bgeom, bool bhullflag, const vec3d & dir )
{
    Vehicle* veh = VehicleMgr.GetVehicle();

    vector < TMesh* > targetTMeshVec = veh->CreateTMeshVec( tgeom );
    vector < TMesh* > boundaryTMeshVec = veh->CreateTMeshVec( bgeom );

    if ( thullflag )
    {
        TMesh *tm = MakeConvexHull( targetTMeshVec );
        DeleteTMeshVec( targetTMeshVec );
        targetTMeshVec.push_back( tm );
    }

    if ( bhullflag )
    {
        TMesh *tm = MakeConvexHull( boundaryTMeshVec );
        DeleteTMeshVec( boundaryTMeshVec );
        boundaryTMeshVec.push_back( tm );
    }

    Results* res = Project( targetTMeshVec, boundaryTMeshVec, dir );
    DeleteTMeshVec( targetTMeshVec );
    DeleteTMeshVec( boundaryTMeshVec );
    return res;
}

Results* ProjectionMgrSingleton::Project( vector < TMesh* > &targetTMeshVec, const vec3d & dir )
{
    Matrix4d mat;
    mat.rotatealongX( dir );

    TransformMesh( targetTMeshVec, mat );

    m_BBox.Reset();
    UpdateBBox( targetTMeshVec );

    Matrix4d toclipper, fromclipper;
    double scale = BuildToFromClipper( toclipper, fromclipper );

    TransformMesh( targetTMeshVec, toclipper );

    vector < Clipper2Lib::Paths64 > targetvec;
    vector < string > targetids;
    MeshToPathsVec( targetTMeshVec, targetvec, targetids );

    vector < Clipper2Lib::Paths64 > utargetvec;

    Union( targetvec, utargetvec, targetids );

    //==== Create Results ====//
    Results* res = ResultsMgr.CreateResults( "Projection", "Projected area results." );

    Vehicle* veh = VehicleMgr.GetVehicle();

    vector < string > namevec( targetids.size() );
    for ( int i = 0; i < targetids.size(); i++ )
    {
        Geom *g = veh->FindGeom( targetids[i] );
        if ( g )
        {
            namevec[i] = g->GetName();
        }
    }

    res->Add( new NameValData( "Comp_Names", namevec, "Names of target Geoms." ) );
    res->Add( new NameValData( "Comp_IDs", targetids, "GeomID's of target Geoms." ) );
    res->Add( new NameValData( "Direction", dir, "Projection direction vector." ) );

    AreaReport( res, "Comp_Areas", "Component projected areas.", utargetvec, scale );

    Clipper2Lib::Paths64 solution;
    Union( utargetvec, solution );

    vector < bool > isHole;
    AreaReport( res, "Area", "Projected area.", solution, scale, isHole, true );

    if ( solution.size() > 0 )
    {
        ClosePaths( solution );

        vector < vector < vec3d > > solutionPolyVec3d;
        PathsToPolyVec( solution, solutionPolyVec3d );

        TransformPolyVec( solutionPolyVec3d, fromclipper );

        for ( int i = 0; i < solutionPolyVec3d.size(); i++ )
        {
            res->Add( new NameValData( "Planar_Path", solutionPolyVec3d[i], "Path outline of projection in two-dimensional projected plane." ) );
        }

        vector < vector < vec2d > > solutionPolyVec2d;
        Poly3dToPoly2d( solutionPolyVec3d, solutionPolyVec2d );

        vector < TMesh* > solutionTMeshVec = Triangulate( solutionPolyVec2d, solutionPolyVec3d, isHole );

        mat.affineInverse();
        TransformPolyVec( solutionPolyVec3d, mat );

        for ( int i = 0; i < solutionPolyVec3d.size(); i++ )
        {
            res->Add( new NameValData( "Path", solutionPolyVec3d[i], "Path outline of projection in three-dimensional space." ) );
        }

        TransformMesh( solutionTMeshVec, mat );

        string id = MakeMeshGeom( solutionTMeshVec, solutionPolyVec3d );

        res->Add( new NameValData( "Mesh_GeomID", id, "GeomID of MeshGeom of the projected area." ) );

        // Clear pointers, they have been transferred to the MeshGeom.
        // Do not delete TMeshs.
        solutionTMeshVec.clear();
    }
    else
    {
        res->Add( new NameValData( "Mesh_GeomID", string( "" ), "Empty GeomID.  Projection had no solution." ) );
    }

    return res;
}

Results* ProjectionMgrSingleton::Project( vector < TMesh* > &targetTMeshVec, vector < TMesh* > &boundaryTMeshVec, const vec3d & dir )
{
    Matrix4d mat;
    mat.rotatealongX( dir );

    TransformMesh( targetTMeshVec, mat );
    TransformMesh( boundaryTMeshVec, mat );

    m_BBox.Reset();
    UpdateBBox( targetTMeshVec );
    UpdateBBox( boundaryTMeshVec );

    Matrix4d toclipper, fromclipper;
    double scale = BuildToFromClipper( toclipper, fromclipper );

    TransformMesh( targetTMeshVec, toclipper );
    TransformMesh( boundaryTMeshVec, toclipper );

    vector < Clipper2Lib::Paths64 > targetvec;
    vector < string > targetids;
    MeshToPathsVec( targetTMeshVec, targetvec, targetids );

    vector < Clipper2Lib::Paths64 > utargetvec;

    Union( targetvec, utargetvec, targetids );

    //==== Create Results ====//
    Results* res = ResultsMgr.CreateResults( "Projection", "Projected area with bounding mesh results." );

    Vehicle* veh = VehicleMgr.GetVehicle();

    vector < string > namevec( targetids.size() );
    for ( int i = 0; i < targetids.size(); i++ )
    {
        Geom *g = veh->FindGeom( targetids[i] );
        if ( g )
        {
            namevec[i] = g->GetName();
        }
    }

    res->Add( new NameValData( "Comp_Names", namevec, "Names of target Geoms." ) );
    res->Add( new NameValData( "Comp_IDs", targetids, "GeomID's of target Geoms." ) );
    res->Add( new NameValData( "Direction", dir, "Projection direction vector." ) );

    AreaReport( res, "Comp_Areas", "Component projected areas.", utargetvec, scale );

    Clipper2Lib::Paths64 boundary;
    MeshToPaths( boundaryTMeshVec, boundary );


    Clipper2Lib::Paths64 bunion;

    Union( boundary, bunion );

    vector < bool > isHole;
    AreaReport( res, "Boundary_Area", "Boundary projected area.", bunion, scale, isHole );

    vector < Clipper2Lib::Paths64 > solvec;
    Intersect( utargetvec, bunion, solvec );

    AreaReport( res, "Comp_Bounded_Areas", "Bounded component projected areas.", solvec, scale );

    Clipper2Lib::Paths64 solution;

    Union( solvec, solution );

    AreaReport( res, "Area", "Bounded projected area.", solution, scale, isHole, true );

    if ( solution.size() > 0 )
    {
        ClosePaths( solution );

        vector < vector < vec3d > > solutionPolyVec3d;
        PathsToPolyVec( solution, solutionPolyVec3d );

        TransformPolyVec( solutionPolyVec3d, fromclipper );

        for ( int i = 0; i < solutionPolyVec3d.size(); i++ )
        {
            res->Add( new NameValData( "Planar_Path", solutionPolyVec3d[i], "Path outline of projection in two-dimensional projected plane." ) );
        }

        vector < vector < vec2d > > solutionPolyVec2d;
        Poly3dToPoly2d( solutionPolyVec3d, solutionPolyVec2d );

        vector < TMesh* > solutionTMeshVec = Triangulate( solutionPolyVec2d, solutionPolyVec3d, isHole );

        mat.affineInverse();
        TransformPolyVec( solutionPolyVec3d, mat );

        for ( int i = 0; i < solutionPolyVec3d.size(); i++ )
        {
            res->Add( new NameValData( "Path", solutionPolyVec3d[i], "Path outline of projection in three-dimensional space." ) );
        }

        TransformMesh( solutionTMeshVec, mat );

        string id =  MakeMeshGeom( solutionTMeshVec, solutionPolyVec3d );

        res->Add( new NameValData( "Mesh_GeomID", id, "GeomID of MeshGeom of the projected area." ) );

        // Clear pointers, they have been transferred to the MeshGeom.
        // Do not delete TMeshs.
        solutionTMeshVec.clear();
    }
    else
    {
        res->Add( new NameValData( "Mesh_GeomID", string( "" ), "Empty GeomID.  Projection had no solution." ) );
    }

    return res;
}

bool TMeshCompare( TMesh* a, TMesh* b )
{
    return ( a->m_OriginGeomID < b->m_OriginGeomID );
}

void ProjectionMgrSingleton::ExportProjectLines( vector < TMesh* > targetTMeshVec )
{
    Vehicle *veh = VehicleMgr.GetVehicle();

    if ( veh )
    {
        // Sort targetTMeshVec ids to match with targetids, which is alphabetized in Union();
        std::sort( targetTMeshVec.begin(), targetTMeshVec.end(), TMeshCompare );

        m_BBox.Reset();
        UpdateBBox( targetTMeshVec );

        Matrix4d toclipper, fromclipper;
        double scale = BuildToFromClipper( toclipper, fromclipper, false ); // Do not translate to bounding box max

        TransformMesh( targetTMeshVec, toclipper );

        // Project in X, Y, and Z directions:
        vector < vec2d > proj_dir_vec;
        proj_dir_vec.resize( 3 );
        proj_dir_vec[vsp::X_DIR] = vec2d( 1, 2 );
        proj_dir_vec[vsp::Y_DIR] = vec2d( 0, 2 );
        proj_dir_vec[vsp::Z_DIR] = vec2d( 0, 1 );

        for ( int k = 0; k < proj_dir_vec.size(); k++ )
        {
            vector < Clipper2Lib::Paths64 > targetvec;
            vector < string > targetids;

            MeshToPathsVec( targetTMeshVec, targetvec, targetids, proj_dir_vec[k].x(), proj_dir_vec[k].y() );

            vector < Clipper2Lib::Paths64 > utargetvec;

            Union( targetvec, utargetvec, targetids );

            // Geom Projection Lines:
            for ( unsigned int i = 0; i < utargetvec.size(); i++ )
            {
                vector < vector < vec3d > > TargetPolyVec3d;

                ClosePaths( utargetvec[i] );

                PathsToPolyVec( utargetvec[i], TargetPolyVec3d, proj_dir_vec[k].x(), proj_dir_vec[k].y() );

                TransformPolyVec( TargetPolyVec3d, fromclipper );

                Geom* curr_geom = veh->FindGeom( targetids[i] );

                if ( curr_geom )
                {
                    // Send geom projection lines 
                    curr_geom->SetGeomProjectVec3d( TargetPolyVec3d, k );
                }
            }

            Clipper2Lib::Paths64 solution;
            Union( utargetvec, solution );

            // Total projection Lines:
            if ( solution.size() > 0 )
            {
                vector < vector < vec3d > > SolutionPolyVec3d;

                ClosePaths( solution );

                PathsToPolyVec( solution, SolutionPolyVec3d, proj_dir_vec[k].x(), proj_dir_vec[k].y() );

                TransformPolyVec( SolutionPolyVec3d, fromclipper );

                // Send total projection lines 
                veh->SetVehProjectVec3d( SolutionPolyVec3d, k );
            }
        }
    }
}

void ProjectionMgrSingleton::MeshToCartesian( vector < TMesh* > & tmv )
{
    for ( int i = 0 ; i < ( int )tmv.size() ; i++ )
    {
        for ( int j = 0 ; j < ( int )tmv[i]->m_NVec.size() ; j++ )
        {
            tmv[i]->m_NVec[j]->m_Pnt = ToCartesian( tmv[i]->m_NVec[j]->m_Pnt );
        }
    }
}

void ProjectionMgrSingleton::PolyVecToCartesian( vector < vector < vec3d > > & polyvec )
{
    for ( int i = 0 ; i < ( int )polyvec.size() ; i++ )
    {
        for ( int j = 0 ; j < ( int )polyvec[i].size() ; j++ )
        {
            polyvec[i][j] = ToCartesian( polyvec[i][j] );
        }
    }
}

void ProjectionMgrSingleton::TransformMesh( vector < TMesh* > & tmv, const Matrix4d & mat )
{
    vec3d zeroV = mat.xform( vec3d( 0.0, 0.0, 0.0 ) );

    for ( int i = 0 ; i < ( int )tmv.size() ; i++ )
    {
        for ( int j = 0 ; j < ( int )tmv[i]->m_NVec.size() ; j++ )
        {
            tmv[i]->m_NVec[j]->m_Pnt = mat.xform( tmv[i]->m_NVec[j]->m_Pnt );
        }

        for ( int j = 0 ; j < ( int )tmv[i]->m_TVec.size() ; j++ )
        {
            tmv[i]->m_TVec[j]->m_Norm = mat.xform( tmv[i]->m_TVec[j]->m_Norm ) - zeroV;
        }
    }
}

void ProjectionMgrSingleton::TransformPolyVec( vector < vector < vec3d > > & polyvec, const Matrix4d & mat )
{
    for ( int i = 0 ; i < ( int )polyvec.size() ; i++ )
    {
        for ( int j = 0 ; j < ( int )polyvec[i].size() ; j++ )
        {
            polyvec[i][j] = mat.xform( polyvec[i][j] );
        }
    }
}

void ProjectionMgrSingleton::UpdateBBox( vector < TMesh* > & tmv )
{
    for ( int i = 0 ; i < ( int )tmv.size() ; i++ )
    {
        for ( int j = 0 ; j < ( int )tmv[i]->m_TVec.size() ; j++ )
        {
            for ( int k = 0; k < 3; k++ )
            {
                m_BBox.Update( tmv[i]->m_TVec[j]->GetTriNode( k )->m_Pnt );
            }
        }
    }
}

void ProjectionMgrSingleton::MeshToPaths( const vector < TMesh* > & tmv, Clipper2Lib::Paths64 & pths )
{
    unsigned int ntri = 0;
    for ( unsigned int i = 0 ; i < ( int )tmv.size() ; i++ )
    {
        ntri += tmv[i]->m_TVec.size();
    }
    pths.resize( ntri );

    unsigned int itri = 0;
    for ( unsigned int i = 0 ; i < ( int )tmv.size() ; i++ )
    {
        for ( int j = 0 ; j < ( int )tmv[i]->m_TVec.size() ; j++ )
        {
            pths[itri].resize(3);

            for ( int k = 0; k < 3; k++ )
            {
                vec3d p = tmv[i]->m_TVec[j]->GetTriNode( k )->m_Pnt;
                pths[itri][k] = Clipper2Lib::Point64( (int64_t) p.y(), (int64_t) p.z() );
            }

            if ( !Clipper2Lib::IsPositive( pths[itri] ) )
            {
                std::reverse( pths[itri].begin(), pths[itri].end() );
            }

            itri++;
        }
    }
}

void ProjectionMgrSingleton::MeshToPathsVec( const vector < TMesh* > & tmv, vector < Clipper2Lib::Paths64 > & pthvec, vector < string > & ids, int keepdir1, int keepdir2 )
{
    pthvec.resize( tmv.size() );
    ids.resize( tmv.size() );

    for ( int i = 0 ; i < ( int )tmv.size() ; i++ )
    {
        pthvec[i].resize( tmv[i]->m_TVec.size() );
        ids[i] = tmv[i]->m_OriginGeomID;

        for ( int j = 0 ; j < ( int )tmv[i]->m_TVec.size() ; j++ )
        {
            pthvec[i][j].resize( 3 );

            for ( int k = 0; k < 3; k++ )
            {
                vec3d p = tmv[i]->m_TVec[j]->GetTriNode( k )->m_Pnt;
                pthvec[i][j][k] = Clipper2Lib::Point64( (int64_t) p.v[keepdir1], (int64_t) p.v[keepdir2] );
            }

            if ( !Clipper2Lib::IsPositive( pthvec[i][j] ) )
            {
                std::reverse( pthvec[i][j].begin(), pthvec[i][j].end() );
            }
        }
    }
}

void ProjectionMgrSingleton::MeshToSphericalPathsVec( TMesh* tm, Clipper2Lib::Paths64 & pth, const double &scalerad )
{
    pth.resize( tm->m_TVec.size() );

    double thtol = 1.0 * M_PI / 180.0; // degrees of arc per edge.

    double tol = 1e-6;
    Clipper2Lib::Paths64 polepths;

    for ( int j = 0 ; j < ( int )tm->m_TVec.size() ; j++ )
    {
        TTri *t = tm->m_TVec[j];

        vector < vec3d > ptlist;

        int polepoint = -1;
        for ( int k = 0; k < 3; k++ )
        {
            vec3d s = t->GetTriNode( k )->m_Pnt;
            vec3d e = t->GetTriNode( k + 1 )->m_Pnt;
            double th = angle( s, e );
            int nref = ceil( th / thtol );

            for ( int i = 0; i < nref; i++ )
            {
                double frac = ( double )i / ( double )nref;
                vec3d p = s + frac * ( e - s );
                ptlist.push_back( p );
            }

            if ( std::abs( s.x() ) < tol && std::abs( s.y() ) < tol )
            {
                polepoint = k;
            }
        }

        pth[j].resize( ptlist.size() );

        // Get triangle center to determine octant for transformation.
        vec3d pcen = t->ComputeCenter();

        for ( int k = 0; k < ptlist.size(); k++ )
        {
            vec3d s = ToSpherical2( ptlist[k], pcen );

            pth[j][k] = Clipper2Lib::Point64( ( int64_t ) ( scalerad * s.y() ), ( int64_t ) ( scalerad * s.z() ) );
        }

        if ( !Clipper2Lib::IsPositive( pth[j] ) )
        {
            std::reverse( pth[j].begin(), pth[j].end() );
        }

        if ( polepoint >= 0 ) // Add triangles to fill out az,el domain around pole.
        {
            for ( int i = 0; i < 2; i++ )
            {
                constexpr int incr[] = {-1, 1};

                int o = polepoint + incr[i];

                vec3d pole = ToSpherical2( t->GetTriNode( polepoint )->m_Pnt, pcen );
                vec3d other = ToSpherical2( t->GetTriNode( o )->m_Pnt, pcen );

                vec3d o2 = other;
                o2.set_z( pole.z() );

                Clipper2Lib::Path64 p;
                p.resize( 3 );
                p[0] = Clipper2Lib::Point64( ( int64_t ) ( scalerad * pole.y() ), ( int64_t ) ( scalerad * pole.z() ) );
                p[1] = Clipper2Lib::Point64( ( int64_t ) ( scalerad * o2.y() ), ( int64_t ) ( scalerad * o2.z() ) );
                p[2] = Clipper2Lib::Point64( ( int64_t ) ( scalerad * other.y() ), ( int64_t ) ( scalerad * other.z() ) );

                if ( !Clipper2Lib::IsPositive( p ) )
                {
                    std::reverse( p.begin(), p.end() );
                }
                polepths.push_back( p );
            }
        }
    }
    pth.insert( pth.end(), polepths.begin(), polepths.end() );
}

void ProjectionMgrSingleton::SphericalDomainPath( Clipper2Lib::Paths64 & pth, const double &scalerad )
{
    pth.resize( 1 );
    pth[0].resize( 5 );

    const double scaledeg = scalerad * M_PI / 180.0;
    const double daz = scaledeg * 5.0;

    pth[0][0] = Clipper2Lib::Point64( ( int64_t ) ( 180.0 * scaledeg ), ( int64_t ) -( 90.0 * scaledeg ) );
    pth[0][1] = Clipper2Lib::Point64( ( int64_t ) ( 180.0 * scaledeg ), ( int64_t ) ( 90.0 * scaledeg ) );
    pth[0][2] = Clipper2Lib::Point64( ( int64_t ) -( 180.0 * scaledeg ), ( int64_t ) ( 90.0 * scaledeg ) );
    pth[0][3] = Clipper2Lib::Point64( ( int64_t ) -( 180.0 * scaledeg ), ( int64_t ) -( 90.0 * scaledeg ) );
    pth[0][4] = pth[0][0];
}

void ProjectionMgrSingleton::OctantDomainPath( int ioct, Clipper2Lib::Paths64 & pth, const double &scalerad, string & label )
{
    // Set up arrays to index through octants.
    const double az[] = {-180, -90, 0, 90, -180, -90, 0, 90 };
    const double el[] = { 0, 0, 0, 0, -90, -90, -90, -90 }; // Lower then upper.
    const char ud[] = { 'U', 'U', 'U', 'U', 'D', 'D', 'D', 'D'};
    const char fa[] = { 'A', 'F', 'F', 'A', 'A', 'F', 'F', 'A' };
    const char lr[] = { 'R', 'R', 'L', 'L', 'R', 'R', 'L', 'L' };

    label = string( 1, ud[ioct] ) + string( 1, fa[ioct] ) + string( 1, lr[ioct] );

    const int64_t azmin = ( int64_t ) ( az[ ioct ] * scalerad * M_PI / 180.0 );
    const int64_t azmax = ( int64_t ) ( ( az[ ioct ] + 90.0 ) * scalerad * M_PI / 180.0 );
    const int64_t elmin = ( int64_t ) ( el[ ioct ] * scalerad * M_PI / 180.0 );
    const int64_t elmax = ( int64_t ) ( ( el[ ioct ] + 90.0 ) * scalerad * M_PI / 180.0 );

    pth.resize( 1 );
    pth[0].resize( 5 );
    pth[0][0] = Clipper2Lib::Point64( azmax, elmin );
    pth[0][1] = Clipper2Lib::Point64( azmax, elmax );
    pth[0][2] = Clipper2Lib::Point64( azmin, elmax );
    pth[0][3] = Clipper2Lib::Point64( azmin, elmin );
    pth[0][4] = pth[0][0];
}

void ProjectionMgrSingleton::PathsToPolyVec( const Clipper2Lib::Paths64 & pths, vector < vector < vec3d > > & polyvec, int keepdir1, int keepdir2 )
{
    polyvec.clear();
    polyvec.reserve( pths.size() );

    int k = 0;
    for ( int i = 0; i < pths.size(); i++ )
    {
        if ( pths[i].size() >= 3 )
        {
            polyvec.resize( polyvec.size() + 1 );
            polyvec[k].resize( pths[i].size() );
            for ( int j = 0; j < pths[i].size(); j++ )
            {
                Clipper2Lib::Point64 p = pths[i][j];
                vec3d pv;
                pv[keepdir1] = p.x;
                pv[keepdir2] = p.y;
                polyvec[k][j] = pv;
            }
            k++;
        }
    }
}

void ProjectionMgrSingleton::RefinePolyVec( vector < vector < vec3d > > & polyvec, const double & scalerad )
{
    double thtol = scalerad * M_PI / 180.0; // degrees of arc per edge.

    for ( int i = 0; i < polyvec.size(); i++ )
    {
        vector < vec3d > pv;
        for ( int j = 0; j < polyvec[ i ].size() - 1; j++ )
        {
            vec3d s = polyvec[ i ][ j ];
            s.set_x( 0 );
            vec3d e = polyvec[ i ][ j + 1 ];
            e.set_x( 0 );
            double th = dist( s, e );

            // Always at least 1, but less than ceil()
            int nref = std::max( 1, (int) round( th / thtol ) );

            for ( int k = 0; k < nref; k++ )
            {
                double frac = (double) k / (double) nref;
                vec3d p = s + frac * ( e - s );
                pv.push_back( p );
            }
        }
        // Place last point of last edge.
        pv.push_back( polyvec[ i ].back() );
        // Replace polyvec with refined version.
        polyvec[ i ] = pv;
    }
}

void ProjectionMgrSingleton::Poly3dToPoly2d( vector < vector < vec3d > > & invec, vector < vector < vec2d > > & outvec )
{
    outvec.resize( invec.size() );
    for ( int i = 0; i < invec.size(); i++ )
    {
        outvec[i].resize( invec[i].size() );
        for ( int j = 0; j < invec[i].size(); j++ )
        {
            outvec[i][j] = vec2d( invec[i][j].y(), invec[i][j].z() );
        }
    }
}

double ProjectionMgrSingleton::BuildToFromClipper( Matrix4d & toclip, Matrix4d & fromclip, bool translate_to_max )
{
    vec3d center = m_BBox.GetCenter();
    double scale = 1e15 / m_BBox.GetLargestDist();

    toclip.loadIdentity();
    toclip.scale( scale );
    toclip.translatef( -center.x(), -center.y(), -center.z() );

    fromclip.loadIdentity();

    // Check flag to translate to the bounding box max or bounding box center
    if ( translate_to_max ) 
    {
        fromclip.translatef( m_BBox.GetMax( 0 ), center.y(), center.z() );
    }
    else
    {
        fromclip.translatef( center.x(), center.y(), center.z() );
    }

    fromclip.scale( 1.0/scale );

    return scale;
}

void ProjectionMgrSingleton::ClosePaths( Clipper2Lib::Paths64 & pths )
{
    for ( int i = 0; i < pths.size(); i++ )
    {
        if ( pths[i].size() >= 3 )
        {
            pths[i].push_back( pths[i][0] );
        }
    }
}

void ProjectionMgrSingleton::Union( Clipper2Lib::Paths64 & pths, Clipper2Lib::Paths64 & sol )
{
    Clipper2Lib::Clipper64 clpr;
    clpr.PreserveCollinear( false );
    clpr.AddSubject( pths );

    if ( !clpr.Execute( Clipper2Lib::ClipType::Union, Clipper2Lib::FillRule::Positive, sol ) )
    {
        printf( "Clipper error\n" );
    }
}

void ProjectionMgrSingleton::Union( vector < Clipper2Lib::Paths64 > & pthsvec,  Clipper2Lib::Paths64 & sol )
{
    // Append all paths into one path.
    Clipper2Lib::Paths64 pth;

    for ( int j = 0; j < pthsvec.size(); j++ )
    {
        pth.insert( pth.end(), pthsvec[j].begin(), pthsvec[j].end() );
    }

    // Then union into solution.
    Union( pth, sol );
}

void ProjectionMgrSingleton::Union( vector < Clipper2Lib::Paths64 > & pthsvec, vector < Clipper2Lib::Paths64 > & solvec, vector < string > & ids )
{
    // Sort ids and make unique.
    vector < string > uids = ids;
    std::sort( uids.begin(), uids.end() );
    vector < string >::iterator sit;
    sit = std::unique( uids.begin(), uids.end() );
    uids.resize( distance( uids.begin(), sit ) );

    solvec.resize( uids.size() );

    for ( int i = 0; i < uids.size(); i++ )
    {
        // Append all matching paths into one path.
        Clipper2Lib::Paths64 pth;

        for ( int j = 0; j < ids.size(); j++ )
        {
            if ( uids[i] == ids[j] ) // Match, append.
            {
                pth.insert( pth.end(), pthsvec[j].begin(), pthsvec[j].end() );
            }
        }

        // Then union into its own set.
        Union( pth, solvec[i] );
    }

    // Copy unique ids over passed in id vector.
    ids = uids;
}

void ProjectionMgrSingleton::Intersect( Clipper2Lib::Paths64 & pthA, Clipper2Lib::Paths64 & pthB, Clipper2Lib::Paths64 & sol )
{
    Clipper2Lib::Clipper64 clpr;
    clpr.PreserveCollinear( false );
    clpr.AddSubject( pthA );
    clpr.AddClip( pthB );

    if ( !clpr.Execute( Clipper2Lib::ClipType::Intersection, Clipper2Lib::FillRule::Positive, sol ) )
    {
        printf( "Clipper error\n" );
    }
}

void ProjectionMgrSingleton::Intersect( vector < Clipper2Lib::Paths64 > & pthsvecA, Clipper2Lib::Paths64 & pthB, vector < Clipper2Lib::Paths64 > & solvec )
{
    solvec.resize( pthsvecA.size() );

    for ( int i = 0; i < pthsvecA.size(); i++ )
    {
        Intersect( pthsvecA[i], pthB, solvec[i] );
    }
}

vector < TMesh* > ProjectionMgrSingleton::Triangulate( const vector < vector < vec2d > > &solutionPolyVec2d, const vector < vector < vec3d > > &solutionPolyVec3d, const vector < bool > &isHole, const bool addspherepoints, const double r )
{
    vector < TMesh * > tmv;
    vector < vec3d > addpts;

    if ( addspherepoints )
    {
        int naz = 20;
        int nel = 20;
        for ( int iaz = 0; iaz < naz; iaz++ )
        {
            // -pi to pi
            double az = ( -1.0 + 2.0 * ( double ) iaz / ( double ) naz ) * M_PI;
            for ( int iel = 0; iel < nel; iel++ )
            {
                // -pi/2 to pi/2
                double el = ( -0.5 + (double) iel / (double) nel ) * M_PI;
                vec2d p2d = vec2d( az, el );

                if ( !PtInHole( p2d, solutionPolyVec2d, isHole ) )
                {
                    vec3d p( r, az, el );
                    addpts.push_back( p );
                }
            }
        }

        // // Add points according to a Fibonacci lattice
        // // https://extremelearning.com.au/how-to-evenly-distribute-points-on-a-sphere-more-effectively-than-the-canonical-fibonacci-lattice/
        // // This works very well.  However, the Delaunay triangulation done in polar coordinates does an ugly job
        // // connecting the points near the poles.  The simple globe-like distribution above gives better results
        // // overall.
        // double goldenRatio = 0.5 * ( 1.0 + sqrt( 5.0 ) );
        // double epsilon = 0.36; // empirical factor for good average nearest point distance optimization.
        //
        // int nadd = 400;
        //
        // for ( int i = 0; i < nadd; i++ )
        // {
        //     double az = clampAngle( 2.0 * M_PI * (double) i / goldenRatio );
        //     double el = 0.5 * M_PI - ( acos( 1.0 - 2.0 * ( (double) i + epsilon ) / ( (double) nadd - 1.0 + 2.0 * epsilon ) ) );
        //
        //     vec2d p2d = vec2d( az, el );
        //     if ( !PtInHole( p2d ) )
        //     {
        //         vec3d p( r, az, el );
        //         addpts.push_back( p );
        //     }
        // }
    }

    vector < vector < int > > connlist;

    Triangulate_TRI( solutionPolyVec3d, connlist, addpts );

    int ntri = connlist.size();

    if ( ntri > 0 )
    {
        TMesh *tMesh = new TMesh();
        tmv.push_back( tMesh );

        for ( int i = 0; i < solutionPolyVec3d.size(); i++ )
        {
            for ( int j = 0; j < ( int ) solutionPolyVec3d[ i ].size() - 1; j++ )
            {
                TNode *n = new TNode();
                n->m_Pnt = solutionPolyVec3d[ i ][ j ];
                tMesh->m_NVec.push_back( n );
            }
        }

        for ( int i = 0; i < addpts.size(); i++ )
        {
            TNode *n = new TNode();
            n->m_Pnt = addpts[ i ];
            tMesh->m_NVec.push_back( n );
        }

        //==== Load Triangles if No New Point Created ====//
        for ( int i = 0; i < ntri; i++ )
        {
            TTri *tPtr = new TTri( tMesh );

            //==== Put Nodes Into Tri ====//
            tPtr->m_N0 = tMesh->m_NVec[ connlist[ i ][ 0 ] ];
            tPtr->m_N1 = tMesh->m_NVec[ connlist[ i ][ 1 ] ];
            tPtr->m_N2 = tMesh->m_NVec[ connlist[ i ][ 2 ] ];

            //tPtr->m_Tags = m_Tags; // Set split tri to have same tags as original triangle
            tPtr->m_Norm = vec3d( -1, 0, 0 );

            vec3d c = tPtr->ComputeCenter();
            vec2d c2d = vec2d( c.y(), c.z() );

            if ( PtInHole( c2d, solutionPolyVec2d, isHole ) )
            {
                delete tPtr;
            }
            else
            {
                tMesh->m_TVec.push_back( tPtr );
            }
        }
    }

    return tmv;
}

void ProjectionMgrSingleton::Triangulate_TRI( const vector < vector < vec3d > > &solutionPolyVec3d, vector < vector < int > > &connlist, const vector < vec3d > &addpts )
{
    int nseg = 0;
    for ( int i = 0; i < solutionPolyVec3d.size(); i++ )
    {
        nseg += solutionPolyVec3d[i].size() - 1; // Subtract off repeated first point.
    }

    int nadd = addpts.size();
    int npt = nseg + nadd;

    //==== Dump Into Triangle ====//
    context* ctx;
    triangleio in, out;
    int tristatus = TRI_NULL;

    // init
    ctx = triangle_context_create();

    memset( &in, 0, sizeof( in ) ); // Load Zeros
    memset( &out, 0, sizeof( out ) );

    //==== PreAllocate Data For In/Out ====//
    in.pointlist    = ( REAL * ) malloc( npt * 2 * sizeof( REAL ) );
    out.pointlist   = nullptr;

    in.segmentlist  = ( int * ) malloc( nseg * 2 * sizeof( int ) );
    out.segmentlist  = nullptr;
    out.trianglelist  = nullptr;

    in.numberofpointattributes = 0;
    in.pointattributelist = nullptr;
    in.pointmarkerlist = nullptr;
    in.numberofholes = 0;
    in.numberoftriangles = 0;
    in.numberofpointattributes = 0;
    in.numberofedges = 0;
    in.trianglelist = nullptr;
    in.trianglearealist = nullptr;
    in.edgelist = nullptr;
    in.edgemarkerlist = nullptr;
    in.segmentmarkerlist = nullptr;

    //==== Load Points into Triangle Struct ====//
    in.numberofpoints = npt;
    in.numberofsegments = nseg;

    int ptcnt = 0;
    int segcnt = 0;
    for ( int i = 0; i < solutionPolyVec3d.size(); i++ )
    {
        int firstseg = segcnt;
        for ( int j = 0 ; j < ( int )solutionPolyVec3d[i].size() - 1 ; j++ )
        {
            vec3d pnt = solutionPolyVec3d[i][j];

            in.pointlist[ptcnt] = pnt.y();
            ptcnt++;
            in.pointlist[ptcnt] = pnt.z();
            ptcnt++;

            in.segmentlist[2 * segcnt] = segcnt;
            if ( j == solutionPolyVec3d[i].size() - 2 )
            {
                in.segmentlist[2 * segcnt + 1] = firstseg;
            }
            else
            {
                in.segmentlist[2 * segcnt + 1] = segcnt + 1;
            }
            segcnt++;
        }
    }

    for ( int i = 0; i < nadd; i++ )
    {
        vec3d pnt = addpts[i];
        in.pointlist[ptcnt] = pnt.y();
        ptcnt++;
        in.pointlist[ptcnt] = pnt.z();
        ptcnt++;
    }

    char cmdline[] = "zpQ";

    //==== Constrained Delaunay Triangulation ====//
    tristatus = triangle_context_options( ctx, cmdline );
    if ( tristatus != TRI_OK ) printf( "triangle_context_options Error\n" );

    // Triangulate the polygon
    tristatus = triangle_mesh_create( ctx, &in );
    if ( tristatus != TRI_OK ) printf( "triangle_mesh_create Error\n" );

    if ( tristatus == TRI_OK )
    {
        triangle_mesh_copy( ctx, &out, 1, 1 );

        connlist.clear();
        connlist.resize( out.numberoftriangles);

        //==== Load Triangles if No New Point Created ====//
        ptcnt = 0;
        for ( int i = 0; i < out.numberoftriangles; i++ )
        {
            connlist[i].push_back( out.trianglelist[ptcnt] );
            connlist[i].push_back( out.trianglelist[ptcnt + 1] );
            connlist[i].push_back( out.trianglelist[ptcnt + 2] );

            ptcnt += 3;
        }
    }

    //==== Free Local Memory ====//
    if ( in.pointlist )
    {
        free( in.pointlist );
    }
    if ( in.segmentlist )
    {
        free( in.segmentlist );
    }

    if ( out.pointlist )
    {
        free( out.pointlist );
    }
    if ( out.pointmarkerlist )
    {
        free( out.pointmarkerlist );
    }
    if ( out.trianglelist )
    {
        free( out.trianglelist );
    }
    if ( out.segmentlist )
    {
        free( out.segmentlist );
    }
    if ( out.segmentmarkerlist )
    {
        free( out.segmentmarkerlist );
    }

    // cleanup
    triangle_context_destroy( ctx );

}

bool ProjectionMgrSingleton::PtInHole( const vec2d &p, const vector < vector < vec2d > > &polyvec2d, const vector < bool > &isHole )
{
    int incount = 0;
    for ( int i = 0; i < polyvec2d.size(); i++ )
    {
        bool in = PointInPolygon( p, polyvec2d[i] );

        if ( in && isHole[i] )
        {
            incount -= 1;
        }
        else if ( in && !isHole[i] )
        {
            incount += 1;
        }
    }

    if ( incount == 0 )
    {
        return true;
    }

    if ( incount > 1 || incount < 0 )
    {
        printf( "Ambiguous triangle location.\n" );
    }

    return false;
}

string ProjectionMgrSingleton::MakeMeshGeom( const vector < TMesh* > &tmv, const vector < vector < vec3d > > &solutionPolyVec3d )
{
    Vehicle* vehiclePtr = VehicleMgr.GetVehicle();

    GeomType type = GeomType( MESH_GEOM_TYPE, "MESH", true );
    string id = vehiclePtr->AddGeom( type );
    Geom *geom_ptr = vehiclePtr->FindGeom( id );
    if ( geom_ptr )
    {
        MeshGeom* mesh_geom = ( MeshGeom* )( geom_ptr );

        mesh_geom->m_PolyVec = solutionPolyVec3d;

        mesh_geom->m_TMeshVec = tmv;
        mesh_geom->m_SurfDirty = true;
        mesh_geom->Update();

        vehiclePtr->SetActiveGeom( id );

    }
    return id;
}

void ProjectionMgrSingleton::MarkHoles( const Clipper2Lib::Paths64 & pths, vector < bool > &isHole )
{
    isHole.resize( pths.size() );

    for ( int i = 0; i < pths.size(); i++ )
    {
        double area = Clipper2Lib::Area( pths[i] );

        if ( area < 0.0 )
        {
            isHole[i] = true;
        }
        else
        {
            isHole[i] = false;
        }
    }
}

void ProjectionMgrSingleton::AreaReport( Results* res, const string &resname, const string &doc, const Clipper2Lib::Paths64 & pths, double scale, vector < bool > &isHole, bool holerpt )
{
    if ( holerpt )
    {
        isHole.resize( pths.size() );
    }

    double asum = 0;
    for ( int i = 0; i < pths.size(); i++ )
    {
        double area = Clipper2Lib::Area( pths[i] );
        asum += area;

        if ( holerpt )
        {
            if ( area < 0.0 )
            {
                isHole[i] = true;
            }
            else
            {
                isHole[i] = false;
            }
        }
    }

    res->Add( new NameValData( resname, asum/(scale*scale), doc ) );
}

void ProjectionMgrSingleton::AreaReport( Results* res, const string &resname, const string &doc, const vector < Clipper2Lib::Paths64 > & pthsvec, double scale )
{
    vector < double > areavec( pthsvec.size() );

    for ( int i = 0; i < pthsvec.size(); i++ )
    {
        double asum = 0;
        for ( int j = 0; j < pthsvec[i].size(); j++ )
        {
            double area = Clipper2Lib::Area( pthsvec[i][j] );
            asum += area;
        }

        areavec[i] = asum/(scale*scale);
    }

    res->Add( new NameValData( resname, areavec, doc ) );
}

void ProjectionMgrSingleton::Dump( vector < vector < vec3d > > & pthsvec, const string & fname )
{
    FILE *fp = fopen( fname.c_str(), "w" );

    fprintf( fp, "clear all\nformat compact\nclose all\n\n" );

    for ( int i = 0; i < pthsvec.size(); i++ )
    {
        fprintf( fp, "l%d = [", i );
        for ( int k = 0; k < pthsvec[ i ].size(); k++ )
        {
            vec3d p = pthsvec[ i ][ k ];
            fprintf( fp, "%f %f %f;\n", p.x(), p.y(), p.z() );
        }
        // Repeat first point.
        vec3d p = pthsvec[ i ][ 0 ];
        fprintf( fp, "%f %f %f];\n\n", p.x(), p.y(), p.z() );

        // fprintf( fp, "plot( l%d(:,2), l%d(:,3) );\nhold on\n\n", i, i );
        fprintf( fp, "fill( l%d(:,2), l%d(:,3), 'b' );\nhold on\n\n", i, i );
    }
    fclose( fp );
}

void ProjectionMgrSingleton::Dump( Clipper2Lib::Paths64 & pth, const string & fname )
{
    vector < vector < vec3d > > polyvec;
    PathsToPolyVec( pth, polyvec, 1, 2 );

    Dump( polyvec, fname );
}
