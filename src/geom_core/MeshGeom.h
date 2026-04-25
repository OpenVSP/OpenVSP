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

    vector< TTri* >  m_IndexedTriVec;
    vector< TNode* > m_IndexedNodeVec;

    vector < TMesh* > m_TMeshVec;
    vector < TMesh* > m_SliceVec;
    vector < vector < vec3d > > m_PolyVec;
    vector < deque < TEdge > > m_Wakes;

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

    static void BuildTriVec( const TMesh* mesh, vector< TTri* > &trivec );
    static void BuildTriVec( const vector < TMesh* > &meshvec, vector< TTri* > &trivec );
    virtual void InitIndexedMesh( const vector < TMesh* > &meshvec );
    static void IndexTriVec( vector < TTri* > &trivec, vector < TNode* > &nodvec );
    static void IgnoreDegenTris( vector < TTri* > &trivec );
    virtual void BuildIndexedMesh();
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

    virtual void Scale();


    //==== Intersection, Splitting and Trimming ====//
    virtual void IntersectTrim( vector< DegenGeom > &degenGeom, bool degen, int intSubsFlag, bool halfFlag, const vector < string > & sub_vec = vector < string > () );

    virtual void PreIntersectTrim( vector< DegenGeom > &degenGeom, int intSubsFlag, MeshInfo &info, Results *res );
    virtual void PostIntersectTrim( vector< DegenGeom > &degenGeom, bool degen, int intSubsFlag, MeshInfo &info, Results *res );

    virtual void MassSlice( vector< DegenGeom > &degenGeom, bool degen, int numSlices, int idir = vsp::X_DIR, bool writefile = true );

    virtual void AreaSlice( int numSlices, vec3d norm, bool autoBounds, double start, double end, bool measureduct );

    virtual void WaveStartEnd( const double &sliceAngle, const vec3d &center );
    virtual void WaveDragSlice( int numSlices, double sliceAngle, int coneSections,
                             const vector <string> & Flow_vec, bool Symm = false );


    virtual void AddPointMass( TetraMassProp* pm )
    {
        m_PointMassVec.push_back( pm );
    }
    vector< TetraMassProp* > m_PointMassVec;

    virtual void WaterTightCheck( FILE* fid );

    virtual void UpdateSurf()
    {
        m_ScaleMatrix.loadIdentity();
        m_ScaleMatrix.scale( m_ScaleFromOrig() );
    }

    virtual void CreateDegenGeom( vector<DegenGeom> &dgs, bool preview = false, const int & n_ref = 0 );

    virtual vector< TMesh* > CreateTMeshVec( bool skipnegflipnormal, const int & n_ref = 0 ) const;
    virtual Matrix4d GetTotalTransMat() const;


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
