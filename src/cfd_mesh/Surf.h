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
#include "SimpleMeshSettings.h"
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

    void GetBorderCurve( const vec3d &uw0, const vec3d &uw1, Bezier_curve & crv ) const;

    vec3d CompPnt( double u, double w ) const;
    vec3d CompPnt01( double u, double w ) const;

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
    void WalkMap( int istart, int jstart, int kstart );
    void WalkMap( int istart, int jstart );
    void LimitTargetMap();
    void LimitTargetMap( MSCloud &es_cloud, MSTree &es_tree, double minmap );
    double InterpTargetMap( double u, double w );
    void UWtoTargetMapij( double u, double w, int &i, int &j, double &fraci, double &fracj );
    void UWtoTargetMapij( double u, double w, int &i, int &j );

    void ApplyES( vec3d uw, double t );

    vec2d ClosestUW( vec3d & pnt_in, double guess_u, double guess_w ) const;
    vec2d ClosestUW( vec3d & pnt_in ) const;

    void FindBorderCurves();

    void SetGridDensityPtr( SimpleGridDensity*  gp )
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
    void SetMainSurfID( int id )
    {
        m_MainSurfID = id;
    }
    int GetMainSurfID()
    {
        return m_MainSurfID;
    }

    void SetFeaPartIndex( int ind )
    {
        m_FeaPartIndex = ind;
    }
    int GetFeaPartIndex()
    {
        return m_FeaPartIndex;
    }

    void Draw();

    void LoadSCurves( vector< SCurve* > & scurve_vec );
    void BuildGrid();

    void WriteSTL( const char* filename );

    Mesh* GetMesh()
    {
        return &m_Mesh;
    }

    void Intersect( Surf* surfPtr, CfdMeshMgrSingleton *MeshMgr );
    void IntersectLineSeg( vec3d & p0, vec3d & p1, vector< double > & t_vals );
    void IntersectLineSegMesh( vec3d & p0, vec3d & p1, vector< double > & t_vals );

    bool BorderCurveOnSurface( Surf* surfPtr, CfdMeshMgrSingleton *MeshMgr );
    void PlaneBorderCurveIntersect( Surf* surfPtr, SCurve* brdPtr, CfdMeshMgrSingleton *MeshMgr );

    BndBox& GetBBox()
    {
        return m_BBox;
    }
    void SetBBox( const vec3d &pmin, const vec3d &pmax );

    vector< SurfPatch* >& GetPatchVec()
    {
        return m_PatchVec;
    }
    void SetPatchVec( const vector< SurfPatch* > &pvec )
    {
        m_PatchVec = pvec;
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

    //Getter/Setter Surface CFD TYPE (NEG, TRANS, NORM)
    void SetSurfaceCfdType( int surfCfdType )
    {
        m_SurfCfdType = surfCfdType;
    }
    int GetSurfaceCfdType()
    {
        return m_SurfCfdType;
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

    void Subtag( bool tag_subs );

protected:

    int m_CompID;
    int m_UnmergedCompID; // Comp ID that does not change when open components are merged
    string m_GeomID;
    string m_RefGeomID;   // Geom ID of the surface a wake attaches to
    int m_SurfID;
    int m_MainSurfID;

    int m_BaseTag; // Tag number that will be applied to all triangles of this surface

    bool m_FlipFlag;

    bool m_WakeFlag;
    int m_WakeParentSurfID;

    bool m_SymPlaneFlag;
    bool m_FarFlag;

    int m_SurfCfdType;

    SurfCore m_SurfCore;

    BndBox m_BBox;
    vector< SurfPatch* > m_PatchVec;

    vector< SCurve* > m_SCurveVec;

    SimpleGridDensity* m_GridDensityPtr;

    Mesh m_Mesh;

    int m_NumMap;
    vector< vector< MapSource > > m_SrcMap;

    bool m_ScaleUFlag;
    vector< double > m_UScaleMap;
    vector< double > m_WScaleMap;

    int m_FeaPartIndex;

};


#endif
