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


#include "Defines.h"
#include "Vec2d.h"
#include "Geom.h"
#include "VspUtil.h"
#include "ResultsMgr.h"
#include <set>
#include <map>

class MeshInfo
{
public:
    MeshInfo()
    {
        m_NumOpenMeshesMerged = m_NumOpenMeshedDeleted = m_NumDegenerateTriDeleted = 0;
    }

    int m_NumOpenMeshesMerged;
    int m_NumOpenMeshedDeleted;
    int m_NumDegenerateTriDeleted;
    vector < string > m_MergedMeshes;
    vector < string > m_DeletedMeshes;
};


class MeshGeom : public Geom
{
private:
    int m_BigEndianFlag;

    vector< TTri* > m_IndexedTriVec;
    vector< TNode* > m_IndexedNodeVec;

    vector< TMesh* > m_SliceVec;

public:
//  enum { SLICE_PLANAR, SLICE_AWAVE };

    MeshGeom( Vehicle* vehicle_ptr );
    ~MeshGeom();

    //! MeshGeom's EncodeXml Implementation
    /**
       MeshGeom's EncodeXml Method does not write out each TTri's splitVec.
       So make sure that FlattenTMeshVec has been called on MeshGeom
       before calling EncodeXml.
    */
    virtual xmlNodePtr EncodeXml( xmlNodePtr & node );
    virtual xmlNodePtr DecodeXml( xmlNodePtr & node );

    double m_TotalTheoArea;
    double m_TotalWetArea;
    double m_TotalTheoVol;
    double m_TotalWetVol;

    vec3d m_CenterOfGrav;

    double m_TotalMass;
    double m_TotalIxx;
    double m_TotalIyy;
    double m_TotalIzz;
    double m_TotalIxy;
    double m_TotalIxz;
    double m_TotalIyz;

    vector < TMesh* > m_TMeshVec;
    vector < vector < vec3d > > m_PolyVec;

    // Scale Transformation Matrix
    Matrix4d m_ScaleMatrix;
    Parm m_ScaleFromOrig;

    virtual void UpdateBBox();
    virtual void UpdateDrawObj();

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
    virtual void WriteStl( FILE* stl_file, int tag );

    virtual void InitIndexedMesh( const vector < TMesh* > &meshvec, int & offset );
    virtual void BuildIndexedMesh( int partOffset, bool half_flag = false );
    virtual int  GetNumIndexedPnts() const
    {
        return m_IndexedNodeVec.size();
    }
    virtual int  GetNumIndexedTris() const
    {
        return m_IndexedTriVec.size();
    }
    virtual int  GetNumIndexedParts() const
    {
        return m_TMeshVec.size();
    }

    virtual void WriteNascartPnts( FILE* file_id );
    virtual void WriteCart3DPnts( FILE* file_id );
    virtual void WriteOBJPnts( FILE* file_id );
    virtual void WriteVSPGeomPnts( FILE* file_id );
    virtual int  WriteGMshNodes( FILE* file_id, int node_offset );
    virtual void WriteFacetNodes( FILE* file_id );
    virtual int  WriteNascartTris( FILE* file_id, int offset );
    virtual int  WriteCart3DTris( FILE* file_id, int offset );
    virtual int  WriteOBJTris( FILE* file_id, int offset );
    virtual int  WriteVSPGeomTris( FILE* file_id, int offset );
    virtual int  WriteGMshTris( FILE* file_id, int node_offset, int tri_offset );
    virtual void WriteFacetTriParts( FILE* file_id, int &offset, int &tri_count, int &part_count );
    virtual int  WriteNascartParts( FILE* file_id, int offset );
    virtual int  WriteCart3DParts( FILE* file_id );
    virtual int  WriteVSPGeomParts( FILE* file_id );
    virtual int  WriteVSPGeomWakes( FILE* file_id, int offset );
    virtual void WritePovRay( FILE* fid, int comp_num );
    virtual void WriteX3D( xmlNodePtr node );
    virtual void CreateGeomResults( Results* res );

    virtual void CreatePtCloudGeom();

    virtual void Scale();

    //==== Intersection, Splitting and Trimming ====//
    virtual void IntersectTrim( vector< DegenGeom > &degenGeom, bool degen = true, int intSubsFlag = 1 );

    virtual void MassSliceX( int numSlice, bool writefile = true );
    virtual void degenGeomMassSliceX( vector< DegenGeom > &degenGeom );
    virtual void AreaSlice( int numSlices, vec3d norm, bool autoBounds, double start = 0, double end = 0 );

    virtual void WaveStartEnd( const double &sliceAngle, const vec3d &center );
    virtual void WaveDragSlice( int numSlices, double sliceAngle, int coneSections,
                             const vector <string> & Flow_vec, bool Symm = false );

    virtual void MergeRemoveOpenMeshes( MeshInfo* info, bool deleteopen = true );

    virtual void CreatePrism( vector< TetraMassProp* >& tetraVec, TTri* tri, double len );
    virtual void createDegenGeomPrism( vector< DegenGeomTetraMassProp* >& tetraVec, TTri* tri, double len );

    virtual void AddPointMass( TetraMassProp* pm )
    {
        m_PointMassVec.push_back( pm );
    }
    vector< TetraMassProp* > m_PointMassVec;

    virtual void WaterTightCheck( FILE* fid );
    virtual void DeleteMarkedMeshes();
    virtual void AddHalfBox( string id );

    virtual TMesh* GetMeshByID( const string & id );

    virtual void UpdateSurf() {}
    virtual int GetNumMainSurfs() const
    {
        return 0;
    }

    virtual void CreateDegenGeom( vector<DegenGeom> &dgs, bool preview = false );

    virtual vector< TMesh* > CreateTMeshVec() const;
    virtual void FlattenTMeshVec();
    virtual void FlattenSliceVec();
    virtual Matrix4d GetTotalTransMat() const;
    virtual void TransformMeshVec( vector<TMesh*> & meshVec, const Matrix4d & TransMat ) const;

    virtual vector< string > GetTMeshNames();
    virtual vector< string > GetTMeshIDs();
    virtual set< string > GetTMeshPtrIDs();

    virtual void SubTagTris( bool tag_subs );

    BoolParm m_ViewMeshFlag;
    BoolParm m_ViewSliceFlag;
    IntParm m_StartColorDegree;

    // Debug Attributes

    enum { DRAW_XYZ = 1, DRAW_UV = 2, DRAW_TAGS = 4, DRAW_BOTH = 3 };
    IntParm m_DrawType;
    BoolParm m_DrawSubSurfs;

protected:
    virtual void ApplyScale(); // this is for intersectTrim
    vector<TMesh*> m_SubSurfVec;

};

#endif
