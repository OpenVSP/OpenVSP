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
#include "TwoDNN.h"
#include "XferSurf.h"

#include <cassert>

#include <string>
#include <vector>
#include <list>
#include <set>
#include <map>
using namespace std;

class SurfaceIntersectionSingleton;
class SCurve;
class ISegChain;

// Tolerance used with nanoflann to merge coincident points.
// 1e-6 Causes OpenABF to crash when it is sent an empty set
// 1e-6 Also caused CheckWaterTight() to merge non-coincident points, introducing bad tris.
// 1e-12 Caused CheckWaterTight() to find a bunch of false positive non-watertight edges.
// 1e-9 Compromise that still caused some issues.
// Fixed problem of not using averaged points along intersections, so those points should now match to machine
// precision and this tolerance should be able to be much smaller.
// Read as a squared distance by IndexPntNodes and the searches beside it, so what is applied
// is 1e-6: points closer together than that are one point.  The figures in the note above are
// distances, which is not what this number is.
#define PT_MERGE_TOL 1e-12

//////////////////////////////////////////////////////////////////////
class Surf
{
public:

    Surf();
    virtual ~Surf();

    void BuildClean();

    void GetBorderCurve( const vec3d &uw0, const vec3d &uw1, Bezier_curve & crv ) const;
    int UWPointOnBorder( double u, double w, double tol ) const;

    vec3d CompPnt( double u, double w ) const;
    vec3d CompPnt01( double u, double w ) const;
    vec3d CompNorm( double u, double w ) const;

    const SurfCore* GetSurfCore() const
    {
        return &m_SurfCore;
    }

    SurfCore* GetSurfCore()
    {
        return &m_SurfCore;
    }

    double TargetLen( double u, double w, double gap, double radfrac, int &reason );
    void BuildTargetMap( vector< MapSource* > &sources, int sid );
    void WalkMap( int istart, int jstart, int kstart );
    void WalkMap( int istart, int jstart );

    void LimitTargetMap();
    void LimitTargetMap( const MSCloud &es_cloud, const MSTree &es_tree, double minmap );
    double InterpTargetMap( double u, double w, int &reason );
    void UWtoTargetMapij( double u, double w, int &i, int &j, double &fraci, double &fracj );
    void UWtoTargetMapij( double u, double w, int &i, int &j );

    void ApplyES( const vec3d &uw, double t, int reason );

    // The same, for a caller that already knows where uw lands in space.  The point is only
    // used to measure how far the map's corners are from it, and evaluating the surface for
    // it again is the most expensive thing this routine does.
    void ApplyESAtPnt( const vec3d &uw, const vec3d &p, double t, int reason );

    vec2d ClosestUW( const vec3d & pnt_in, double guess_u, double guess_w ) const;
    vec2d ClosestUW( const vec3d & pnt_in ) const;

    // The UW of a point to split an edge at: the nearest point on the surface that is
    // equidistant from the edge's two ends.  See the definition for why the midpoint will
    // not do.
    vec2d SplitUW( const vec3d & p0, const vec3d & p1, const vec2d & uw0, const vec2d & uw1 ) const;
    double SplitSearch( const vec3d & p0, const vec3d & p1,
                        double ulo, double uhi, double wlo, double whi,
                        double &u, double &w ) const;

    void AddBorderCurve( double ua, double wa, double ub, double wb );
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
    void SetName( const string &name )
    {
        m_Name = name;
    }
    string GetName()
    {
        return m_Name;
    }

    string GetDisplayName();

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
    void SetSplitNum( int id )
    {
        m_SplitNum = id;
    }
    int GetSplitNum()
    {
        return m_SplitNum;
    }

    void SetFeaPartIndex( int ind )
    {
        m_FeaPartIndex = ind;
    }
    int GetFeaPartIndex()
    {
        return m_FeaPartIndex;
    }

    // void Draw();

    void LoadSCurves( vector< SCurve* > & scurve_vec );
    vector< SCurve* >& GetSCurveVec()  { return m_SCurveVec; }
    void BuildGrid();

    void WriteSTL( const char* filename );

    Mesh* GetMesh()
    {
        return &m_Mesh;
    }

    bool IntersectPrepare( Surf* surfPtr, SurfaceIntersectionSingleton *MeshMgr );
    // The patches whose boxes meet surfPtr's, which are the only ones IntersectPatch has work for
    void FindIntersectPatches( Surf* surfPtr, vector < int > &patch_vec );
    void IntersectPatch( int ipatch, Surf* surfPtr, SurfaceIntersectionSingleton *MeshMgr );
    void IntersectLineSeg( vec3d & p0, vec3d & p1, vector< double > & t_vals );

    bool BorderCurveOnSurface( Surf* surfPtr, SurfaceIntersectionSingleton *MeshMgr );
    void PlaneBorderCurveIntersect( Surf* surfPtr, SCurve* brdPtr, SurfaceIntersectionSingleton *MeshMgr );

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

    void InitMesh( const vector< ISegChain* > &chains, const vector < vec2d > &adduw, SurfaceIntersectionSingleton *MeshMgr );

    void FindSTBox( const vec2d &st, int &i_match, int &j_match );

    void BuildDistMap();
    void CleanupDistMap();
    vec2d GetST( const vec2d &uw );
    vec2d GetUW( const vec2d &st );

    bool ValidUW( vec2d & uw, double slop = 1.0e-4 ) const;

    bool BorderMatch( Surf* otherSurf );
    bool BorderMatch( int iborder, Surf* otherSurf );

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

    void SetFeaPartSurfNum( int num )
    {
        m_FeaPartSurfNum = num;
    }
    int GetFeaPartSurfNum()
    {
        return m_FeaPartSurfNum;
    }

    void SetFeaOrientationType( int oType )
    {
        m_FeaOrientationType = oType;
    }
    int GetFeaOrientationType()
    {
        return m_FeaOrientationType;
    }

    void SetFeaOrientation( const vec3d &o )
    {
        m_FeaOrientation = o;
    }
    vec3d GetFeaElementOrientation( double u, double w );
    vec3d GetFeaElementOrientation( double u, double w, int type, const vec3d & defaultorientation );
    vec3d GetFeaElementOrientation();

    void SetFeaSymmIndex( int s )
    {
        m_FeaSymmIndex = s;
    }

    int GetFeaSymmIndex()
    {
        return m_FeaSymmIndex;
    }

    //Getter/Setter Surface VSP TYPE (NORMAL_SURF, WING_SURF, DISK_SURF)
    void SetSurfaceVSPType( int surfVspType )
    {
        m_SurfVspType = surfVspType;
    }
    int GetSurfaceVSPType()
    {
        return m_SurfVspType;
    }

    //Getter/Setter Surface thick flag
    void SetSurfaceThickSurf( bool thicksurf )
    {
        m_ThickSurf = thicksurf;
    }
    bool GetSurfaceThickSurf()
    {
        return m_ThickSurf;
    }

    void SetSurfacePlateNum( int p )
    {
        m_PlateNum = p;
    }
    int GetSurfacePlateNum()
    {
        return m_PlateNum;
    }

    void SetSurfaceCopyIndex( int c )
    {
        m_CopyIndex = c;
    }
    int GetSurfaceCopyIndex() const
    {
        return m_CopyIndex;
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

    // Tessellation lines inherited from the parent Geom, in this surface's parameter
    // space.  Empty for surfaces that were not fetched from a Geom.
    void SetUWTess( const vector < double > &utess, const vector < double > &wtess )
    {
        m_UTess = utess;
        m_WTess = wtess;
    }
    const vector < double > & GetUTess() const
    {
        return m_UTess;
    }
    const vector < double > & GetWTess() const
    {
        return m_WTess;
    }

    // Where this surface's parameters came from on the Geom it was cut out of.  Things that
    // were written against the Geom's surface -- subsurfaces, structured tessellation, wake
    // attachment, fixed points -- have to be asked for in those parameters, not these.
    void SetUWRegions( const vector < UWRegion > &rv )
    {
        m_UWRegions = rv;
    }
    const vector < UWRegion > & GetUWRegions() const
    {
        return m_UWRegions;
    }

    // This surface's parameters to the Geom's.  False if the point is not in any region,
    // which should not happen for a point on this surface.
    bool ToOriginalUW( double u, double w, double &uo, double &wo ) const;

    // The Geom's parameters to this surface's.  False if the point does not land on this
    // surface at all -- which is the common case, and is how a caller decides whether a
    // subsurface or a tessellation line touches this patch.
    bool ToPatchUW( double uo, double wo, double &u, double &w ) const;

    // The region a point of this surface belongs to, or -1.
    int FindRegionPatchUW( double u, double w ) const;

    // The regions, or one identity region covering the whole surface when none were
    // recorded.  Wake and far field surfaces were not cut from a Geom, so their own
    // parameters are the only ones they have.
    vector < UWRegion > GetUWRegionsOrWhole() const;

    void SetIgnoreSurfFlag( bool flag )
    {
        m_IgnoreSurfFlag = flag;
    }
    bool GetIgnoreSurfFlag()
    {
        return m_IgnoreSurfFlag;
    }

    void SetPlanarUWAspect( double uwAspect )
    {
        m_PlanarUWAspect = uwAspect;
    }
    double GetPlanarUWAspect()
    {
        return m_PlanarUWAspect;
    }

    void Subtag( bool tag_subs );

    friend double refine_intersect_pt( const vec3d& pt, Surf *sA, vec2d &uwA, Surf *sB, vec2d &uwB );

protected:

    int m_CompID;
    int m_UnmergedCompID; // Comp ID that does not change when open components are merged
    string m_GeomID;
    string m_Name;
    string m_RefGeomID;   // Geom ID of the surface a wake attaches to
    int m_SurfID;
    int m_MainSurfID;
    int m_SplitNum;

    int m_BaseTag; // Tag number that will be applied to all triangles of this surface

    bool m_FlipFlag;

    bool m_WakeFlag;
    int m_WakeParentSurfID;

    bool m_SymPlaneFlag;
    bool m_FarFlag;

    bool m_IgnoreSurfFlag; // Flag to ignore the surface after intersected

    vector < double > m_UTess;
    vector < double > m_WTess;

    vector < UWRegion > m_UWRegions;

    int m_SurfCfdType;
    int m_SurfVspType;
    bool m_ThickSurf;
    int m_PlateNum;
    int m_CopyIndex;

    double m_PlanarUWAspect;

    double LinearSTAspect() const;

    SurfCore m_SurfCore;

    BndBox m_BBox;
    vector< SurfPatch* > m_PatchVec;

    vector< SCurve* > m_SCurveVec;

    SimpleGridDensity* m_GridDensityPtr;

    Mesh m_Mesh;

    int m_NumMap;
    vector< vector< MapSource > > m_SrcMap;

    // Per-cell stamp used to visit each cell of m_SrcMap at most once during a
    // single WalkMap( istart, jstart ).  Stamped with a counter rather than
    // cleared so a walk costs nothing to set up.
    vector< unsigned int > m_WalkVisited;
    unsigned int m_WalkVisitID;

    void UtoIndexFrac( const double &u, int &indx, double &frac );

    vector < vector < vec2d > > m_STMap;
    TwoDNodeCloud m_UWMap;

    int m_FeaPartIndex; // Which FeaPart in the FeaStructure
    int m_FeaPartSurfNum; // Which surface in the FeaPart
    int m_FeaOrientationType; // Which orientation type
    vec3d m_FeaOrientation; // Surface orientation
    int m_FeaSymmIndex;

};


#endif
