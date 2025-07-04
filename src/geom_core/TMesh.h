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

#ifndef TRI_TEST_H
#define TRI_TEST_H


#include "Vec2d.h"
#include "Vec3d.h"
#include "Matrix4d.h"
#include "BndBox.h"
#include "XmlUtil.h"

#include <vector>               //jrg windows?? 
#include <algorithm>            //jrg windows??
#include <string>
#include <unordered_map>
#include <list>
using namespace std;            //jrg windows??

#include "APIDefines.h"

//#ifndef DEBUG_TMESH
//#define DEBUG_TMESH
//#endif

//#ifndef COMPARE_TRIANGLE
//#define COMPARE_TRIANGLE
//#endif

class TEdge;
class TTri;
class TBndBox;
class NBndBox;
class TMesh;

struct dba_point
{
    double x, y;
};

struct dba_edge
{
    int a, b;
};

int dba_errlog( void* stream, const char* fmt, ...);

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

class TetraMassProp
{
public:
    TetraMassProp( const string& id, double den, const vec3d& p0, const vec3d& p1, const vec3d& p2, const vec3d& p3 );
    TetraMassProp()         {
        m_Density = 0;
        m_Vol = 0;
        m_Mass = 0;

        m_Ixx = 0;
        m_Iyy = 0;
        m_Izz = 0;

        m_Ixy = 0;
        m_Ixz = 0;
        m_Iyz = 0;
    }
    ~TetraMassProp()        {}

    void SetDistributedMass( double massIn, const vec3d& cgIn, const double & IxxIn, const double & IyyIn, const double & IzzIn,
                                                               const double & IxyIn, const double & IxzIn, const double & IyzIn,
                                                               Matrix4d transMatIn );

    vec3d m_v0;
    vec3d m_v1;
    vec3d m_v2;
    vec3d m_v3;

    string m_CompId;
    string m_Name;

    vec3d m_CG;

    double m_Density;
    double m_Vol;
    double m_Mass;

    double m_Ixx;
    double m_Iyy;
    double m_Izz;

    double m_Ixy;
    double m_Ixz;
    double m_Iyz;
};



class TriShellMassProp
{
public:
    TriShellMassProp( const string& id, double mass_area_in, const vec3d& p0, const vec3d& p1, const vec3d& p2 );
    TriShellMassProp()     {
        m_MassArea = 0;
        m_TriArea = 0;
        m_Mass = 0;

        m_Ixx = 0;
        m_Iyy = 0;
        m_Izz = 0;

        m_Ixy = 0;
        m_Ixz = 0;
        m_Iyz = 0;
    }
    ~TriShellMassProp()     {}

    vec3d m_v0;
    vec3d m_v1;
    vec3d m_v2;

    vec3d m_CG;

    string m_CompId;

    double m_MassArea;
    double m_TriArea;
    double m_Mass;

    double m_Ixx;
    double m_Iyy;
    double m_Izz;

    double m_Ixy;
    double m_Ixz;
    double m_Iyz;
};

class TNode
{
public:
    /*
        TNode()         { static int cnt = 0;   cnt++;
                            printf("TNode Construct Cnt = %d \n", cnt); }
        ~TNode()        { static int cnt = 0;   cnt++;
                            printf("TNode Destruct Cnt = %d \n", cnt); }
    */
    TNode();
    virtual ~TNode();

    virtual void CopyFrom( const TNode* node);
    virtual void MakePntUW();
    virtual void MakePntXYZ();
    virtual void SetXYZFlag( bool flag )
    {
        m_XYZFlag = flag;
    }
    virtual bool GetXYZFlag() const
    {
        return m_XYZFlag;
    }
    virtual void SetCoordInfo( int flag )
    {
        m_CoordInfo = flag;
    }
    virtual int GetCoordInfo() const
    {
        return m_CoordInfo;
    }
    virtual vec3d GetXYZPnt();
    virtual vec3d GetUWPnt();
    virtual void SetXYZPnt( const vec3d & pnt );
    virtual void SetUWPnt( const vec3d & pnt );

    vec3d m_Pnt;
    vec3d m_UWPnt;
    int m_ID;

    vector< TTri* > m_TriVec;               // For WaterTight Check
    vector< TEdge* > m_EdgeVec;         // For WaterTight Check

    vector< TNode* > m_MergeVec;

//  TNode* mapNode;

    int m_IsectFlag;

    enum { HAS_UNKNOWN = 0, HAS_XYZ = 1, HAS_UW = 2 };

protected:
    bool m_XYZFlag;
    int m_CoordInfo;
};

class TEdge
{
public:
    TEdge();
    TEdge( TNode* n0, TNode* n1, TTri* par_tri );
    virtual ~TEdge()        {}

    virtual void SetParTri( TTri* par_tri )
    {
        m_ParTri = par_tri;
    }
    virtual TTri* GetParTri()
    {
        return m_ParTri;
    }

    virtual void SwapEdgeDirection();
    virtual void SortNodesByU();
    virtual TTri* GetOtherTri( TTri *t );

    virtual bool UsesNode( TNode *n );

    virtual bool DuplicateEdge( TEdge * other );

    TNode* m_N0;
    TNode* m_N1;

    TTri* m_Tri0;                           // For WaterTight Check
    TTri* m_Tri1;

protected:
    TTri* m_ParTri; // Tri that edge is apart of

};



class TTri
{
public:
    TTri( TMesh* tmesh );
    virtual ~TTri();

    virtual bool CleanupEdgeVec();
    virtual void CopyFrom( const TTri* tri );
    virtual bool SplitTri( bool dumpCase );              // Split Tri to Fit ISect Edges

    virtual void FlipTri();
    virtual void OrientTri( vector < int > & tri );
    virtual void OrientConnList( vector < vector < int > > & cl );
    virtual bool CompConnList( const vector < vector < int > > & cla, const vector < vector < int > > & clb );
    virtual void SortTri( vector < int > & tri );
    virtual void SortConnList( vector < vector < int > > & cl );

    virtual bool TriangulateSplit( int flattenAxis, const vector < vec3d > &ptvec, bool dumpCase );
    virtual bool TriangulateSplit_TRI( int flattenAxis, const vector < vec3d > &ptvec, bool dumpCase,
                                       vector < vector < int > > & connlist );
    virtual bool TriangulateSplit_DBA( int flattenAxis, const vector < vec3d > &ptvec, bool dumpCase,
                                       vector < vector < int > > & connlist, const vector < vector < int > > & otherconnlist );
    virtual vec3d ComputeCenter()
    {
        return ( m_N0->m_Pnt + m_N1->m_Pnt + m_N2->m_Pnt ) / 3.0;
    }
    virtual vec3d ComputeCenterUW()
    {
        return ( m_N0->m_UWPnt + m_N1->m_UWPnt + m_N2->m_UWPnt ) / 3.0;
    }
    virtual double ComputeArea()
    {
        return area( m_N0->m_Pnt, m_N1->m_Pnt, m_N2->m_Pnt );
    }
    virtual double ComputeYZArea()
    {
        vec3d t1, t2, t3;
        t1.set_xyz( 0.0, m_N0->m_Pnt.y(), m_N0->m_Pnt.z() );
        t2.set_xyz( 0.0, m_N1->m_Pnt.y(), m_N1->m_Pnt.z() );
        t3.set_xyz( 0.0, m_N2->m_Pnt.y(), m_N2->m_Pnt.z() );
        return area( t1, t2, t3 );
    }

    virtual TNode* GetTriNode( int i )
    {
        if ( i == -1 ) // Loop before the start to make iterating on edges easy.
            return m_N2;
        if ( i == 0 )
            return m_N0;
        if ( i == 1 )
            return m_N1;
        if ( i == 2 )
            return m_N2;
        if ( i == 3 ) // Loop back to start to make iterating on edges easy.
            return m_N0;
        return nullptr;
    }

    virtual void ComputeCosAngles( double* ang0, double* ang1, double* ang2 );

    virtual void SplitEdges( TNode* n01, TNode* n12, TNode* n20 );

    virtual vec3d CompNorm();
    virtual bool  ShareEdge( TTri* t );
    virtual bool MatchEdge( TNode* n0, TNode* n1, TNode* nA, TNode* nB, double tol );

    virtual void RemoveDuplicateEdges();

    virtual TMesh* GetTMeshPtr()
    {
        return m_TMesh;
    }

    virtual void BuildPermEdges();

    virtual bool InTri( const vec3d & p );
    virtual int OnEdge( const vec3d & p, TEdge* e, double onEdgeTol, double &t );
    virtual vec3d CompPnt( const vec3d & uw_pnt );
    virtual vec3d CompUW( const vec3d & pnt );

    virtual int WakeEdge();

    TNode* m_N0;
    TNode* m_N1;
    TNode* m_N2;

    vec3d m_Norm;

    vector< TEdge* > m_ISectEdgeVec;        // List of Intersection Edges
    vector< TTri* > m_SplitVec;             // List of split tris
    vector< TNode* > m_NVec;                // Nodes for split tris
    vector< TEdge* > m_EVec;                // Edges for split tris
    TEdge* m_PEArr[3];                          // Perimeter Edge Array

    bool m_IgnoreTriFlag;
    vector< bool > m_insideSurf;
    string m_GeomID;  // Used by slice triangles for later use by Wave Drag and Mass Properties.
    vector<int> m_Tags;
    double m_Density;
    int m_InvalidFlag;
    int m_iQuad;

    TEdge* m_E0;
    TEdge* m_E1;
    TEdge* m_E2;

protected:
    TMesh* m_TMesh;

private:

    virtual int DupEdge( TEdge* e0, TEdge* e1, double tol );

};

class TBndBox
{
public:
    TBndBox();
    virtual ~TBndBox();

    virtual void Reset();

    void SplitBox();
    void AddTri( TTri* t );
    virtual void Intersect( TBndBox* iBox, bool UWFlag = false );
    virtual void RayCast( const vec3d & orig, const vec3d & dir, vector<double> & tParmVec, vector <TTri*> & triVec ) const;

    virtual bool CheckIntersect( TBndBox* iBox );
    virtual double MinDistance( TBndBox* iBox, double curr_min_dist, vec3d &p1, vec3d &p2 );

    virtual bool CheckIntersect( const vec3d &org, const vec3d &norm );
    virtual double MinDistance( const vec3d &org, const vec3d &norm, double curr_min_dist, vec3d &p1, vec3d &p2 );
    virtual double MaxDistance( const vec3d &org, const vec3d &norm, double curr_max_dist, vec3d &p1, vec3d &p2 );
    virtual double MaxDistanceRay( const vec3d &org, const vec3d &norm, double curr_max_dist, vec3d &p1, vec3d &p2 );

    virtual double MinAngle( const vec3d &org, const vec3d &norm, const vec3d& ptaxis, const vec3d& axis, double curr_min_angle, int ccw, vec3d &p1, vec3d &p2 );

    BndBox m_Box;
    vector< TTri* > m_TriVec;

    TBndBox* m_SBoxVec[8];      // Split Bnd Boxes
};

class Geom;

class TMesh
{
public:
    TMesh();
    virtual void Init();
    virtual ~TMesh();
    virtual void Wype();

    void copy( TMesh* m );
    void CopyFlatten( TMesh* m );
    virtual xmlNodePtr EncodeXml( xmlNodePtr & node );
    virtual void DecodeXml( xmlNodePtr & node );
    virtual xmlNodePtr EncodeTriList( xmlNodePtr & node );
    virtual void DecodeTriList( xmlNodePtr & node, int num_tris );

    void LoadGeomAttributes( const Geom* geomPtr );
    int  RemoveDegenerate();
    void RemoveIsectEdges();
    void Intersect( TMesh* tm, bool UWFlag = false );
    bool CheckIntersect( TMesh* tm );
    double MinDistance( TMesh* tm, double curr_min_dist, vec3d &p1, vec3d &p2 );
    bool CheckIntersect( const vec3d &org, const vec3d &norm );
    double MinDistance( const vec3d &org, const vec3d &norm, double curr_min_dist, vec3d &p1, vec3d &p2 );
    double MaxDistance( const vec3d &org, const vec3d &norm, double curr_max_dist, vec3d &p1, vec3d &p2 );
    double MaxDistanceRay( const vec3d &org, const vec3d &norm, double curr_min_dist, vec3d &p1, vec3d &p2 );
    double MinAngle( const vec3d &org, const vec3d &norm, const vec3d& ptaxis, const vec3d& axis, double curr_min_angle, int ccw, vec3d &p1, vec3d &p2 );
    void Split();

    void SetIgnoreTriFlag( const vector < int > & bTypes, const vector < bool > & thicksurf );
    void SetIgnoreSubSurface();
    void SetIgnoreInsideAny();
    void SetIgnoreInsideNotOne();
    void SetIgnoreOutsideAll();
    void SetIgnoreMatchMask( const vector < bool > & mask );
    void SetKeepMatchMask( const vector < bool > & mask );

    void IgnoreYLessThan( const double & ytol );
    void SetIgnoreAbovePlane( const vector <vec3d> & threepts );
    void IgnoreAll();

    void DeterIntExt( const vector< TMesh* >& meshVec, const vec3d &dir = vec3d( 1.0, 0.000001, 0.000001 ) );
    void DeterIntExt( TMesh* mesh, const vec3d &dir = vec3d( 1.0, 0.000001, 0.000001 ) );

    void LoadBndBox();
    void UpdateBBox( BndBox &bbox, const Matrix4d &transMat = Matrix4d() );

    virtual double ComputeTheoArea();
    virtual double ComputeWetArea();
    virtual double ComputeWaveDragArea( const std::unordered_map< string, int > &idmap );
    virtual double ComputeTheoVol();
    virtual double ComputeTrimVol();

    virtual void FlipNormals();
    virtual void Transform( const Matrix4d & TransMat );

    virtual void AddTri( const vec3d & v0, const vec3d & v1, const vec3d & v2, const int & iQuad );
    virtual void AddTri( const vec3d & v0, const vec3d & v1, const vec3d & v2, const vec3d & norm, const int & iQuad );
    virtual void AddTri( TNode * node0, TNode * node1, TNode * node2, const vec3d & norm, const int & iQuad );
    virtual void AddTri( const vec3d & v0, const vec3d & v1, const vec3d & v2, const vec3d & norm, const vec3d & uw0,
                         const vec3d & uw1, const vec3d & uw2, const int & iQuad );
    virtual void AddTri( const TTri* tri );
    virtual void AddUWTri( const vec3d & uw0, const vec3d & uw1, const vec3d & uw2, const vec3d & norm, const int & iQuad );

    virtual void WriteSTLTris( FILE* file_id, Matrix4d XFormMat );

    virtual vec3d GetVertex( int index );
    virtual int   NumVerts();

    virtual void WaterTightCheck( FILE* fid, vector< TMesh* > & tMeshVec );
    virtual void FindEdge( TNode* node, TTri* tri0, TTri* tri1 );
    virtual void AddEdge( TTri* tri0, TTri* tri1, TNode* node0, TNode* node1 );
    virtual void SwapEdge( TEdge* edge );

    virtual void MergeNonClosed( TMesh* tm );
    virtual void MergeTMeshes( const TMesh* tm );
    virtual void BuildEdges();
    virtual void CheckIfClosed();
    virtual void BuildMergeMaps();
    virtual void BuildNodeMaps();
    virtual void BuildEdgeMaps();
    virtual void DeleteDupNodes();

    virtual void MatchNodes();
    virtual void CheckValid( FILE* fid );
    virtual void SwapEdges( double size );
    virtual vec3d ProjectOnISectPairs( vec3d & offPnt, vector< vec3d > & pairVec );

    virtual void MeshStats( double* minEdgeLen, double* minTriAng, double* maxTriAng );
    virtual void TagNeedles( double minTriAng, double minAspectRatio, int delFlag );
    virtual void MoveNode( TNode* n0, TNode* n1 );
    virtual TTri* FindTriNodes( TTri* ignoreTri, TNode* n0, TNode* n1 );
    virtual TTri* FindTriPnts( TTri* ignoreTri, TNode* n0, TNode* n1 );

    virtual void SubTag( int part_num, bool tag_subs, const vector < string > & sub_vec = vector < string > () ); // Subtag all triangles, if split triangles exist tag them the same as their parent
    virtual void RefreshTagMap();

    virtual void MakeNodePntUW(); // Swaps Node->m_Pnt with Node->m_UWPnt
    virtual void MakeNodePntXYZ();

    virtual void SplitAliasEdges( TTri* orig_tri, TEdge* isect_edge );

    virtual vec3d CompPnt( const vec3d & uw_pnt );
    virtual void FindIJ( const vec3d & uw_pnt, int &start_u, int &start_v );

    static void StressTest();
    static double Rand01();

    vector< TTri* >  m_TVec;
    vector< TNode* > m_NVec;
    vector< TEdge* > m_EVec;

    TBndBox m_TBox;

    //==== Stuff Copied From Geom That Created This Mesh ====//
    string m_OriginGeomID;
    //bool reflected_flag;
    string m_NameStr;
    int m_SurfNum; // To keep track of geoms with multiple surfaces
    int m_PlateNum; // To keep track of degen plate number.  -1 for normal surfaces.
    int m_MaterialID;
    int m_SurfCfdType;
    int m_ThickSurf;
    int m_MassPrior;
    double m_Density;
    double m_ShellMassArea;
    bool m_ShellFlag;

    double m_TheoArea;
    double m_WetArea;
    vector < double > m_CompAreaVec;
    vector < double > m_TagTheoAreaVec;
    vector < double > m_TagWetAreaVec;
    double m_TheoVol;
    double m_GuessVol;
    double m_WetVol;
    vec3d m_AreaCenter;

    bool m_DeleteMeFlag;
    vector< TTri* > m_NonClosedTriVec;

    vector< vec3d > m_VertVec;

    // Surface type of parent surface.
    int m_SurfType;

    double m_Wmin;

    // UW and XYZ points used in TMesh::CompPnt only.  This is called in Intersect and SplitTri when in UV mode.  This
    // happens while intersecting subsurface curves.
    vector< vector<vec3d> > m_UWPnts;
    vector< vector<vec3d> > m_XYZPnts;

protected:
    void CopyAttributes( TMesh* m );

    unordered_map< TNode*, list<TNode*> > m_NAMap; // Map from a master node to list of nodes that are aliases
    unordered_map< TNode*, TNode* > m_NSMMap;      // Map of node slave to master node
    unordered_map< TEdge*, vector<TEdge*> > m_EAMap; // Map from a master edge to a list of edges that are aliases
    unordered_map< TEdge*, TEdge* > m_ESMMap;      // Map from edge slave to master edge

};

void CreateTMeshVecFromPts( const Geom * geom,
                            vector < TMesh* > & TMeshVec,
                            const vector< vector<vec3d> > & pnts,
                            const vector< vector<vec3d> > & norms,
                            const vector< vector<vec3d> > & uw_pnts,
                            int indx, int platenum, int surftype, int cfdsurftype, bool thicksurf, bool flipnormal, double wmax );

void BuildTMeshTris( TMesh *tmesh, bool f_norm, double wmax );

vector<TMesh*> CopyTMeshVec( const vector<TMesh*> &tmv );
void DeleteTMeshVec(  vector<TMesh*> &tmv );
TMesh* MergeTMeshVec( const vector<TMesh*> &tmv );
void LoadBndBox( vector< TMesh* > &tmv );
void UpdateBBox( BndBox &bbox, vector<TMesh*> &tmv, const Matrix4d &transMat );
void ApplyScale( double scalefac, vector<TMesh*> &tmv );
void MergeRemoveOpenMeshes( vector<TMesh*> &tmv, MeshInfo* info, bool deleteopen = true );
void DeleteMarkedMeshes( vector<TMesh*> &tmv );
void FlattenTMeshVec( vector<TMesh*> &tmv );
void TransformMesh( TMesh* mesh, const Matrix4d & TransMat );
void TransformMeshVec( vector<TMesh*> & meshVec, const Matrix4d & TransMat );
vector< string > GetTMeshNames( vector<TMesh*> &tmv );
vector< string > GetTMeshIDs( vector<TMesh*> &tmv );
unordered_map< string, int > GetThicks( vector<TMesh*> &tmv );
void SubTagTris( bool tag_subs, vector<TMesh*> &tmv, const vector < string > & sub_vec = vector < string > () );
void RefreshTagMaps( vector<TMesh*> &tmv );

double FindMinDistance(const vector< TMesh* > & tmesh_vec, const vector< TMesh* > & other_tmesh_vec, bool & intersect_flag );
double FindMaxMinDistance( const vector< TMesh* > & mesh_1, const vector< TMesh* > & mesh_2 );
bool CheckIntersect( const vector<TMesh*> & tmesh_vec, const vector<TMesh*> & other_tmesh_vec );
bool CheckIntersect( const vector<TMesh*> & tmesh_vec, const vec3d &org, const vec3d &norm );
bool CheckIntersect( Geom* geom_ptr, const vector<TMesh*> & other_tmesh_vec );
bool CheckSelfIntersect( const vector<TMesh*> & tmesh_vec );
void PlaneInterferenceCheck(  TMesh *primary_tm, const vec3d & org, const vec3d & norm, const string & resid, vector< TMesh* > & result_tmv );
void CCEInterferenceCheck(  TMesh *primary_tm, TMesh *secondary_tm, const string & resid, vector< TMesh* > & result_tmv );
string PlaneAngleInterferenceCheck( vector< TMesh* > & primary_tmv, const vec3d & org, const vec3d & norm, const vec3d & ptaxis, const vec3d & axis, vector< TMesh* > & result_tmv );
string ExteriorInterferenceCheck( vector< TMesh* > & primary_tmv, vector< TMesh* > & secondary_tmv, vector< TMesh* > & result_tmv );
string PackagingInterferenceCheck( vector< TMesh* > & primary_tmv, vector< TMesh* > & secondary_tmv, vector< TMesh* > & result_tmv );
string ExteriorSelfInterferenceCheck( vector< TMesh* > & primary_tmv, vector< TMesh* > & result_tmv );
bool DecideIgnoreTri( int aType, const vector < int > & bTypes, const vector < bool > & thicksurf, const vector < bool > & aInB );
double IntersectSplit( vector < TMesh * > &tmv, bool intSubsFlag, const vector < string > & sub_vec = vector < string > () );
void IntersectSplitClassify( vector < TMesh * > &tmv, bool intSubsFlag, const vector < string > & sub_vec = vector < string > () );
void CSGMesh( vector < TMesh* > & tmv, bool intSubsFlag = false, const vector < string > & sub_vec = vector < string > () );
void MeshUnion( vector < TMesh* > & tmv );
void MeshCCEIntersect( vector < TMesh * > &tmv );
void MeshIntersect( vector < TMesh* > & tmv );
void MeshSubtract( vector < TMesh* > & tmv );
void MeshCutAbovePlane( vector < TMesh* > & tmv, const vector <vec3d> & threepts );
TMesh* OctantSplitMesh( TMesh* tm );

TMesh* MakeConvexHull(const vector< TMesh* > & tmesh_vec );

void DeterIntExtTri( TTri* tri, const vector< TMesh* >& meshVec, const vec3d &dir );
bool DeterIntExtTri( TTri* tri, TMesh* mesh, const vec3d &dir = vec3d( 1.0, 0.000001, 0.000001 )  );

void WriteStl( const string &file_name, const vector< TMesh* >& meshVec );
void WriteStl( const string &file_name, TMesh* tm );
void MakeThreePts( const vec3d & org, const vec3d & norm, vector <vec3d> &threepts );
TMesh* MakeSlice( const vec3d & org, const vec3d & norm, const double & len );
TMesh* MakeSlice( const int &swdir, const double & len );
double MakeSlices( vector<TMesh*> &tmv, const BndBox & bbox, int numSlices, int swdir, vector < double > &slicevec, bool mpslice = true, bool tesselate = true, bool autoBounds = true, double start = 0, double end = 0, int slctype = vsp::CFD_STRUCTURE );

#endif
