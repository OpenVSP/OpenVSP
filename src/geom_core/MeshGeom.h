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
#include "VspUtil.h"
#include "ResultsMgr.h"
#include <set>
#include <unordered_map>

class MeshGeom : public Geom
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

    virtual void ApplyScale(); // this is for intersectTrim
    vector<TMesh*> m_SubSurfVec;

};

#endif
