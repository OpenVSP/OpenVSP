//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

//******************************************************************************
//
//   Mesh Geometry Class
//
//
//   J.R. Gloudemans - 11/7/94
//   Sterling Software
//
//
//******************************************************************************

#ifndef MESH_GEOM_H
#define MESH_GEOM_H



#include "Vec2d.h"
#include "Geom.h"
#include "GeomInterface.h"
#include "VspUtil.h"
#include "ResultsMgr.h"
#include <set>
#include <unordered_map>

//==== A Geom whose shape is a mesh of triangles ====//
// Implemented by MeshGeom.  Triangles are kept in the Geom's own frame and placed on output,
// so a Clone can borrow them.  The shared output code lives here.
class TMeshRole : virtual public GeomInterface
{
public:
    // The behavior type this role belongs to; checked by Geom::CastTo.
    static int BehaviorType()   { return MESH_GEOM_TYPE; }

    virtual ~TMeshRole()   {}

    //==== What each implementation answers for itself ====//
    // The triangles, in this Geom's own frame.
    virtual const vector< TMesh* > & GetTMeshVecInSelf() const = 0;
    // A fresh copy, owned by the caller.
    virtual vector< TMesh* > CreateTMeshVecInSelf( bool skipnegflipnormal, const int &n_ref ) const = 0;

    // Where this Geom stands them.
    virtual Matrix4d GetTMeshTransMat() const = 0;
    // The shape's own scaling, not part of the placement.  A Clone applies it too.
    virtual Matrix4d GetTMeshScaleMat() const = 0;

    // Which DrawObj draws each tag combination.  Owned by the mesh's Geom, not SubSurfaceMgr,
    // whose global map the last meshing run rebuilds.
    virtual const map< vector < int >, int > & GetTMeshSingleTagMap() const = 0;

    // Colour wheel start for the first subsurface tag; zero is red.
    virtual int GetTMeshColorStartDegree() const = 0;

    // The slices cut through the mesh, in the Geom's own frame, and which of the two is shown.
    virtual const vector< TMesh* > & GetTMeshSliceVec() const = 0;
    virtual bool GetTMeshViewMeshFlag() const = 0;
    virtual bool GetTMeshViewSliceFlag() const = 0;

protected:
    // Placed copies of the triangles, for analyses.  Attributes come from geom_ptr.
    vector< TMesh* > BuildTMeshVec( const Geom* geom_ptr ) const;

    // Fill draw objects from the placed triangles: one per mesh, or one per tag when tags are shown.
    void BuildTMeshDrawObjs( const vector< TMesh* > &tmesh_vec, bool bytag,
                             vector< DrawObj > &draw_obj_vec ) const;

    // The triangles' extent, placed.
    void BuildTMeshBndBox( BndBox &bbox ) const;

    // The mesh written as stereolithography triangles, placed.
    void WriteTMeshStl( FILE* file_id ) const;

    // The mesh in degenerate form, one entry per mesh, named and parented as geom_ptr.
    void BuildTMeshDegenGeom( Geom* geom_ptr, vector< DegenGeom > &dgs ) const;

public:
    // Pick the primitive for draw objects holding loose triangles; the usual Geom route picks a
    // structured mesh, which draws nothing.
    static void SetTriDrawObjTypes( vector< DrawObj > &draw_obj_vec, int drawtype );

    // Colour one draw object per subsurface tag, going round the colour wheel.
    static void SetTagDrawObjColors( vector< DrawObj > &draw_obj_vec, int startdegree, int num_uniq_tags );
};

class MeshGeom : public Geom, public TMeshRole
{
private:
    int m_BigEndianFlag;

public:
//  enum { SLICE_PLANAR, SLICE_AWAVE };

    MeshGeom( Vehicle* vehicle_ptr );
    ~MeshGeom();

    vector < TMesh* > m_TMeshVec;
    vector < TMesh* > m_SliceVec;
    vector < vector < vec3d > > m_PolyVec;
    vector < deque < TEdge > > m_Wakes;

    // Scale Transformation Matrix
    Matrix4d m_ScaleMatrix;
    Parm m_ScaleFromOrig;

    BoolParm m_ViewMeshFlag;
    BoolParm m_ViewSliceFlag;
    IntParm m_StartColorDegree;

    // Debug Attributes

    enum { DRAW_XYZ = 1, DRAW_UV = 2, DRAW_TAGS = 4, DRAW_BOTH = 3 };
    IntParm m_DrawType;
    BoolParm m_DrawSubSurfs;

    //! MeshGeom's EncodeXml Implementation
    /**
       MeshGeom's EncodeXml Method does not write out each TTri's splitVec.
       So make sure that FlattenTMeshVec has been called on MeshGeom
       before calling EncodeXml.
    */
    virtual xmlNodePtr EncodeXml( xmlNodePtr & node );
    virtual xmlNodePtr DecodeXml( xmlNodePtr & node );

    virtual void LoadDrawObjs( vector< DrawObj* > & draw_obj_vec );

    virtual int  GetNumXSecSurfs() const
    {
        return 0;
    }
    virtual int  ReadSTL( const char* file_name );
    virtual int  ReadXSec( const char* file_name );
    virtual int  ReadNascart( const char* file_name );
    virtual int  ReadTriFile( const char* file_name );
    virtual float ReadBinFloat( FILE* fptr );
    virtual int   ReadBinInt  ( FILE* fptr );
    virtual void WriteStl( FILE* pov_file );

    virtual int  GetNumIndexedParts() const
    {
        return m_TMeshVec.size();
    }
    virtual int GetNumWakes() const
    {
        return m_Wakes.size();
    }

    virtual void WriteVSPGeom( const string file_name );
    virtual void WritePovRay( FILE* fid, int comp_num );
    virtual void WriteX3D( xmlNodePtr node );
    virtual void CreateGeomResults( Results* res );

    virtual void CreatePtCloudGeom();
    virtual string CreateNGonMeshGeom( bool cullfracflag = false, double cullfrac = 0.03, int n_ref = 0, bool FindBodyWakes = false );

    virtual void ApplyScale( double currentScale );


    //==== Intersection, Splitting and Trimming ====//
    virtual void IntersectTrim( vector< DegenGeom > &degenGeom, bool degen, int intSubsFlag, bool halfFlag, const vector < string > & sub_vec = vector < string > () );

    virtual void MassSlice( vector< DegenGeom > &degenGeom, bool degen, int numSlices, int idir, bool writefile,
                            double &totalMass, vec3d &centerOfGrav, vec3d &IxxIyyIzz, vec3d &IxyIxzIyz );

    virtual void AreaSlice( int numSlices, const vec3d &norm, bool autoBounds, double start, double end, bool measureduct );

    virtual void WaveStartEnd( const double &sliceAngle, const vec3d &center );
    virtual void WaveDragSlice( int numSlices, double sliceAngle, int coneSections,
                             const vector <string> & Flow_vec, bool Symm = false );


    virtual void AddPointMass( TetraMassProp* pm )
    {
        m_PointMassVec.push_back( pm );
    }
    vector< TetraMassProp* > m_PointMassVec;

    virtual void WaterTightCheck( FILE* fid );


    virtual void CreateDegenGeom( vector<DegenGeom> &dgs, bool preview = false, const int & n_ref = 0 );

    virtual vector< TMesh* > CreateTMeshVec( bool skipnegflipnormal, const int & n_ref = 0 ) const;
    virtual Matrix4d GetTotalTransMat() const;

    virtual vector< TMesh* > CreateTMeshVecInSelf( bool skipnegflipnormal, const int &n_ref ) const;

    virtual const vector< TMesh* > & GetTMeshVecInSelf() const
    {
        return m_TMeshVec;
    }
    virtual Matrix4d GetTMeshTransMat() const
    {
        return GetTotalTransMat();
    }

    virtual Matrix4d GetTMeshScaleMat() const
    {
        return m_ScaleMatrix;
    }

    virtual const vector< TMesh* > & GetTMeshSliceVec() const
    {
        return m_SliceVec;
    }

    virtual bool GetTMeshViewMeshFlag() const
    {
        return m_ViewMeshFlag();
    }

    virtual bool GetTMeshViewSliceFlag() const
    {
        return m_ViewSliceFlag();
    }
    virtual int GetTMeshColorStartDegree() const
    {
        return m_StartColorDegree();
    }

    virtual const map< vector < int >, int > & GetTMeshSingleTagMap() const
    {
        return m_SingleTagMap;
    }

protected:

    virtual void UpdateSurf()
    {
        m_ScaleMatrix.loadIdentity();
        m_ScaleMatrix.scale( m_ScaleFromOrig() );
    }
    virtual void UpdateBBox();
    virtual void UpdateDrawObj();

    // Work out which DrawObj draws each tag combination this MeshGeom's own triangles carry.
    //
    // SubSurfaceMgr keeps a map like this, but it is global and is cleared and rebuilt from
    // scratch by whichever meshing operation ran last.  A MeshGeom that draws out of it
    // draws by whatever happens to be registered rather than by what it holds, which is no
    // use to a mesh read from a file or restored from a saved model.  PGMulti snapshots the
    // map for the same reason.
    virtual void UpdateTagMap();
    map< vector < int >, int > m_SingleTagMap;

    // Scale the triangles themselves, by m_Scale over the scale they were last left at.  This
    // is not the ApplyScale( double ) hook above -- it is how IntersectTrim gets the mesh to
    // its working size and back, and it keeps m_LastScale itself.  Named apart from the hook
    // because the two differed only by an argument list, and a mechanical conversion to the
    // hook once took this one's bookkeeping with it.
    virtual void ScaleTriangles();
    vector<TMesh*> m_SubSurfVec;

};

#endif
