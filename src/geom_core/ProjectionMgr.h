//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// ProjectionMgr.h: interface for the Projection Singleton.
//
//////////////////////////////////////////////////////////////////////

#if !defined(PROJECTION__INCLUDED_)
#define PROJECTION__INCLUDED_

#include "clipper2/clipper.h"

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

    virtual vec3d GetDirection( int dirtype, const string &dirid );

    virtual void UpdateDirection();
    virtual vec3d GetDirection();

    virtual void Renew();

    virtual void LoadDrawObjs( vector< DrawObj* > & draw_obj_vec );

    virtual Results* Project( );

    virtual Results* Project( int tset, bool thullflag, const vec3d & dir );
    virtual Results* Project( int tset, bool thullflag, int bset, bool bhullflag, const vec3d & dir );
    virtual Results* Project( int tset, bool thullflag, const string &bgeom, bool bhullflag, const vec3d & dir );

    virtual Results* Project( const string &tgeom, bool thullflag, const vec3d & dir );
    virtual Results* Project( const string &tgeom, bool thullflag, int bset, bool bhullflag, const vec3d & dir );
    virtual Results* Project( const string &tgeom, bool thullflag, const string &bgeom, bool bhullflag, const vec3d & dir );

    virtual string MakeMeshGeom();

    virtual void ExportProjectLines( vector < TMesh* > targetTMeshVec );

    int m_TargetSetIndex;
    int m_BoundarySetIndex;

    string m_ModeID;
    string m_TargetGeomID;
    string m_BoundaryGeomID;
    string m_DirectionGeomID;


    vector < vector < vec3d > > m_SolutionPolyVec3d;
    vector < vector < vec2d > > m_SolutionPolyVec2d;
    vector < TMesh* > m_SolutionTMeshVec;

    vector < bool > m_IsHole;


protected:

    virtual void MeshToCartesian( vector < TMesh* > & tmv );
    virtual void PolyVecToCartesian( vector < vector < vec3d > > & polyvec );

    virtual void TransformMesh( vector < TMesh* > & tmv, const Matrix4d & mat );
    virtual void TransformPolyVec( vector < vector < vec3d > > & polyvec, const Matrix4d & mat );

    virtual void UpdateBBox( vector < TMesh* > & tmv );

    virtual void MeshToPaths( const vector < TMesh* > & tmv, Clipper2Lib::Paths64 & pths );

    virtual void MeshToPathsVec( const vector < TMesh* > & tmv, vector < Clipper2Lib::Paths64 > & pths, vector < string > & ids, int keepdir1 = 1, int keepdir2 = 2 );

    virtual void PathsToPolyVec( const Clipper2Lib::Paths64 & pths, vector < vector < vec3d > > & polyvec, int keepdir1 = 1, int keepdir2 = 2 );

    virtual void Poly3dToPoly2d( vector < vector < vec3d > > & invec, vector < vector < vec2d > > & outvec );

    virtual double BuildToFromClipper( Matrix4d & toclip, Matrix4d & fromclip, bool translate_to_max = true );

    virtual void ClosePaths( Clipper2Lib::Paths64 & pths );

    virtual void Union( Clipper2Lib::Paths64 & pths, Clipper2Lib::Paths64 & sol );
    virtual void Union( vector < Clipper2Lib::Paths64 > & pthsvec,  Clipper2Lib::Paths64 & sol );
    virtual void Union( vector < Clipper2Lib::Paths64 > & pthsvec, vector < Clipper2Lib::Paths64 > & solvec, vector < string > & ids );

    virtual void Intersect( Clipper2Lib::Paths64 & pthA, Clipper2Lib::Paths64 & pthB, Clipper2Lib::Paths64 & sol );
    virtual void Intersect( vector < Clipper2Lib::Paths64 > & pthsvecA, Clipper2Lib::Paths64 & pthB, vector < Clipper2Lib::Paths64 > & solvec );

    void Triangulate( bool addspherepoints = false, double r = 0.0 );
    virtual void Triangulate_TRI( vector < vector < int > > &connlist, const vector < vec3d > & addpts );

    virtual bool PtInHole( const vec2d &p );

    virtual void AreaReport( Results* res, const string &resname, const string &doc, const Clipper2Lib::Paths64 & pths, double scale, bool holerpt = false );

    virtual void AreaReport( Results* res, const string &resname, const string &doc, const vector < Clipper2Lib::Paths64 > & pthsvec, double scale );

    virtual Results* Project( vector < TMesh* > &targetTMeshVec, const vec3d & dir );
    virtual Results* Project( vector < TMesh* > &targetTMeshVec, vector < TMesh* > &boundaryTMeshVec, const vec3d & dir );

    virtual void Dump( vector < vector < vec3d > > & pthsvec, const string & fname );
    virtual void Dump( Clipper2Lib::Paths64 & pthsvec, const string & fname );

    BndBox m_BBox;

private:

    ProjectionMgrSingleton();
    ProjectionMgrSingleton( ProjectionMgrSingleton const& copy ) = delete;            // Not Implemented
    ProjectionMgrSingleton& operator=( ProjectionMgrSingleton const& copy ) = delete; // Not Implemented

    static void Init();
    static void Wype();

};

#define ProjectionMgr ProjectionMgrSingleton::getInstance()

#endif
