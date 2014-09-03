//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

//////////////////////////////////////////////////////////////////////
// Surf.h
// J.R Gloudemans
//////////////////////////////////////////////////////////////////////

#if !defined(SURF_SURF__INCLUDED_)
#define SURF_SURF__INCLUDED_

#include "Vec2d.h"
#include "Vec3d.h"

#include "Mesh.h"
#include "GridDensity.h"
#include "SurfPatch.h"
#include "MapSource.h"
#include "SurfCore.h"

#include <assert.h>

#include <string>
#include <vector>
#include <list>
#include <set>
#include <map>
using namespace std;

class CfdMeshMgrSingleton;
class SCurve;
class ISegChain;

//////////////////////////////////////////////////////////////////////
class Surf
{
public:

    Surf();
    virtual ~Surf();

    void BuildClean();

    void ReadSurf( FILE* file_id );
    void LoadControlPnts( vector< vector< vec3d > > & pnts );
    void ExtractBorderControlPnts( const vec3d &uw0, const vec3d &uw1, Bezier_curve & crv );
    void ExtractBorderControlPnts( const vec3d &uw0, const vec3d &uw1, vector< vec3d > & control_pnts );

    vec3d CompPnt( double u, double w ) const;

    const SurfCore* GetSurfCore() const
    {
        return &m_SurfCore;
    }

    SurfCore* GetSurfCore()
    {
        return &m_SurfCore;
    }

    double TargetLen( double u, double w, double gap, double radfrac );
    void BuildTargetMap( vector< MapSource* > &sources, int sid );
    void WalkMap( int istart, int jstart, int kstart, int icurrent, int jcurrent);
    void WalkMap( int istart, int jstart, int icurrent, int jcurrent );
    void LimitTargetMap();
    void LimitTargetMap( MSCloud &es_cloud, MSTree &es_tree, double minmap );
    double InterpTargetMap( double u, double w );
    void UWtoTargetMapij( double u, double w, int &i, int &j, double &fraci, double &fracj );
    void UWtoTargetMapij( double u, double w, int &i, int &j );

    void ApplyES( vec3d uw, double t );

    vec2d ClosestUW( vec3d & pnt, double guess_u, double guess_w, double guess_del_u, double guess_del_w, double tol );

    vec2d ClosestUW( vec3d & pnt_in, double guess_u, double guess_w ) const;
    void CompDeltaUW( vec3d& pnt_in, vec3d& guess_pnt, double norm_uw[2], double delta_uw[2] ) const;


    void FindBorderCurves();

    void SetGridDensityPtr( GridDensity*  gp )
    {
        m_GridDensityPtr = gp;
        m_Mesh.SetGridDensityPtr( gp );
    }

    void SetCompID( int id )
    {
        m_CompID = id;
    }
    int  GetCompID()
    {
        return m_CompID;
    }
    void SetUnmergedCompID( int id )
    {
        m_UnmergedCompID = id;
    }
    int GetUnmergedCompID()
    {
        return m_UnmergedCompID;
    }
    void SetGeomID( const string &id )
    {
        m_GeomID = id;
    }
    string GetGeomID()
    {
        return m_GeomID;
    }
    void SetRefGeomID( const string &id )
    {
        m_RefGeomID = id;
    }
    string GetRefGeomID()
    {
        return m_RefGeomID;
    }
    void SetSurfID( int id )
    {
        m_SurfID = id;
    }
    int  GetSurfID()
    {
        return m_SurfID;
    }
    string GetCompName()
    {
        return m_CompName;
    }

    void Draw();

    void LoadSCurves( vector< SCurve* > & scurve_vec );
    void BuildGrid();

    void WriteSTL( const char* filename );

    Mesh* GetMesh()
    {
        return &m_Mesh;
    }

    void Intersect( Surf* surfPtr );
    void IntersectLineSeg( vec3d & p0, vec3d & p1, vector< double > & t_vals );
    void IntersectLineSegMesh( vec3d & p0, vec3d & p1, vector< double > & t_vals );

    bool BorderCurveOnSurface( Surf* surfPtr );
    void PlaneBorderCurveIntersect( Surf* surfPtr, SCurve* brdPtr );

    BndBox& GetBBox()
    {
        return m_BBox;
    }
    vector< SurfPatch* >& GetPatchVec()
    {
        return m_PatchVec;
    }

    void InitMesh( vector< ISegChain* > chains );


    void BuildDistMap();
    double GetUScale( double w );
    double GetWScale( double u );

    bool ValidUW( vec2d & uw );

    bool BorderMatch( Surf* otherSurf );

    void SetWakeFlag( bool flag )
    {
        m_WakeFlag = flag;
    }
    bool GetWakeFlag()
    {
        return m_WakeFlag;
    }

    void SetWakeParentSurfID( int id )
    {
        m_WakeParentSurfID = id;
    }
    int GetWakeParentSurfID()
    {
        return m_WakeParentSurfID;
    }

    void SetTransFlag( bool flag )
    {
        m_TransFlag = flag;
    }
    bool GetTransFlag()
    {
        return m_TransFlag;
    }

    virtual void SetSymPlaneFlag( bool flag );
    bool GetSymPlaneFlag()
    {
        return m_SymPlaneFlag;
    }

    void SetFarFlag( bool flag )
    {
        m_FarFlag = flag;
    }
    bool GetFarFlag()
    {
        return m_FarFlag;
    }

    void SetFlipFlag( bool flag )
    {
        m_FlipFlag = flag;
    }
    bool GetFlipFlag()
    {
        return m_FlipFlag;
    }
    void FlipFlipFlag()
    {
        m_FlipFlag = !m_FlipFlag;
    }
    void SetBaseTag( int tag )
    {
        m_BaseTag = tag;
    }
    int GetBaseTag()
    {
        return m_BaseTag;
    }

    void SetDefaultParmMap();
    void Subtag( bool tag_subs );
    // Mapping Functions
    // Converts surf uw to VspSurf uw
    vec2d Convert2VspSurf( double u, double w );
    // Converts VspSurf uw to surf uw
    vec2d Convert2Surf( double u, double w );

protected:

    int m_CompID;
    int m_UnmergedCompID; // Comp ID that does not change when open components are merged
    string m_GeomID;
    string m_RefGeomID;   // Geom ID of the surface a wake attaches to
    string m_CompName;
    int m_SurfID;

    int m_BaseTag; // Tag number that will be applied to all triangles of this surface

    bool m_FlipFlag;

    bool m_WakeFlag;
    int m_WakeParentSurfID;

    bool m_TransFlag;
    bool m_SymPlaneFlag;
    bool m_FarFlag;

    SurfCore m_SurfCore;

    BndBox m_BBox;
    vector< SurfPatch* > m_PatchVec;

    vector< SCurve* > m_SCurveVec;

    GridDensity* m_GridDensityPtr;

    Mesh m_Mesh;

    int m_NumMap;
    vector< vector< MapSource > > m_SrcMap;

    int m_NumWScalePnts;
    bool m_ScaleUFlag;
    vector< double > m_UScaleMap;
    vector< double > m_WScaleMap;

    int m_VspSurfInd;
    vector< double > u_to_vspsurf;
    vector< double > w_to_vspsurf;
    map< double, int > u_to_surf;
    map< double, int > w_to_surf;

    double InterpolateToVspSurf( const vector< double> & vec, const double & surf_val ) const;

};


#endif
