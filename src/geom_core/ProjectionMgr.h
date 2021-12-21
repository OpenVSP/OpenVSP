//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// ProjectionMgr.h: interface for the Projection Singleton.
//
//////////////////////////////////////////////////////////////////////

#if !defined(PROJECTION__INCLUDED_)
#define PROJECTION__INCLUDED_

#include <clipper.hpp>

#include "Vec3d.h"
#include "Vec2d.h"
#include "Matrix4d.h"
#include "TMesh.h"
#include "DrawObj.h"
#include "Parm.h"
#include "ResultsMgr.h"

#include <vector>
#include <string>

using std::string;
using std::vector;

//==== Projection Manager ====//
class ProjectionMgrSingleton
{
public:
    static ProjectionMgrSingleton& getInstance()
    {
        static ProjectionMgrSingleton instance;
        return instance;
    }

    virtual void SetGeomIDs( const string &tid, const string &bid, const string &did );

    virtual vec3d GetDirection( int dirtype, string dirid );

    virtual void UpdateDirection();
    virtual vec3d GetDirection();

    virtual void Renew();

    virtual void LoadDrawObjs( vector< DrawObj* > & draw_obj_vec );

    virtual Results* Project( );

    virtual Results* Project( int tset );
    virtual Results* Project( int tset, int bset );
    virtual Results* Project( int tset, string bgeom );

    virtual Results* Project( const string &tgeom );
    virtual Results* Project( const string &tgeom, int bset );
    virtual Results* Project( const string &tgeom, string bgeom );

    virtual Results* Project( int tset, const vec3d & dir );
    virtual Results* Project( int tset, int bset, const vec3d & dir );
    virtual Results* Project( int tset, string bgeom, const vec3d & dir );

    virtual Results* Project( const string &tgeom, const vec3d & dir );
    virtual Results* Project( const string &tgeom, int bset, const vec3d & dir );
    virtual Results* Project( const string &tgeom, string bgeom, const vec3d & dir );

    virtual string MakeMeshGeom();

    virtual void ExportProjectLines( vector < TMesh* > targetTMeshVec );

    int m_TargetSetIndex;

    int m_BoundarySetIndex;

    string m_TargetGeomID;
    string m_BoundaryGeomID;
    string m_DirectionGeomID;


    vector < vector < vec3d > > m_SolutionPolyVec3d;
    vector < vector < vec2d > > m_SolutionPolyVec2d;
    vector < TMesh* > m_SolutionTMeshVec;

    vector < bool > m_IsHole;


protected:

    virtual void CleanMesh( vector < TMesh* > & tmv );

    virtual void TransformMesh( vector < TMesh* > & tmv, const Matrix4d & mat );
    virtual void TransformPolyVec( vector < vector < vec3d > > & polyvec, const Matrix4d & mat );

    virtual void GetMesh( int set, vector < TMesh* > & tmv );
    virtual void GetMesh( string geom, vector < TMesh* > & tmv );

    virtual void UpdateBBox( vector < TMesh* > & tmv );

    virtual void MeshToPaths( const vector < TMesh* > & tmv, ClipperLib::Paths & pths );

    virtual void MeshToPathsVec( const vector < TMesh* > & tmv, vector < ClipperLib::Paths > & pths, vector < string > & ids, const int keepdir1 = 1, const int keepdir2 = 2 );

    virtual void PathsToPolyVec( const ClipperLib::Paths & pths, vector < vector < vec3d > > & polyvec, const int keepdir1 = 1, const int keepdir2 = 2 );

    virtual void Poly3dToPoly2d( vector < vector < vec3d > > & invec, vector < vector < vec2d > > & outvec );

    virtual double BuildToFromClipper( Matrix4d & toclip, Matrix4d & fromclip, bool translate_to_max = true );

    virtual void ClosePaths( ClipperLib::Paths & pths );

    virtual void Union( ClipperLib::Paths & pths, ClipperLib::Paths & sol );
    virtual void Union( vector < ClipperLib::Paths > & pthsvec,  ClipperLib::Paths & sol );
    virtual void Union( vector < ClipperLib::Paths > & pthsvec, vector < ClipperLib::Paths > & solvec, vector < string > & ids );

    virtual void Intersect( ClipperLib::Paths & pthA, ClipperLib::Paths & pthB, ClipperLib::Paths & sol );
    virtual void Intersect( vector < ClipperLib::Paths > & pthsvecA, ClipperLib::Paths & pthB, vector < ClipperLib::Paths > & solvec );

    virtual void Triangulate();

    virtual bool PtInHole( const vec2d &p );

    virtual void AreaReport( Results* res, const string &resname, const ClipperLib::Paths & pths, double scale, bool holerpt = false );

    virtual void AreaReport( Results* res, const string &resname, const vector < ClipperLib::Paths > & pthsvec, double scale );

    virtual Results* Project( vector < TMesh* > &targetTMeshVec, const vec3d & dir );
    virtual Results* Project( vector < TMesh* > &targetTMeshVec, vector < TMesh* > &boundaryTMeshVec, const vec3d & dir );

    BndBox m_BBox;

private:

    ProjectionMgrSingleton();
    ProjectionMgrSingleton( ProjectionMgrSingleton const& copy );            // Not Implemented
    ProjectionMgrSingleton& operator=( ProjectionMgrSingleton const& copy ); // Not Implemented

    static void Init();
    static void Wype();

};

#define ProjectionMgr ProjectionMgrSingleton::getInstance()

#endif
