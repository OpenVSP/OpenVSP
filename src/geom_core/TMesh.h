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
#include "Matrix.h"
#include "BndBox.h"
#include "DragFactors.h"
#include "XmlUtil.h"

#include <vector>               //jrg windows?? 
#include <algorithm>            //jrg windows??
#include <string>
#include <map>
#include <list>
using namespace std;            //jrg windows??

extern "C"
{
    void triangulate( const char *, struct triangulateio *, struct triangulateio *,
                      struct triangulateio * );
}

class TEdge;
class TTri;
class TBndBox;
class NBndBox;
class TNodeGroup;
class TMesh;

class TetraMassProp
{
public:
    TetraMassProp( string id, double den, vec3d& p0, vec3d& p1, vec3d& p2, vec3d& p3 );
    TetraMassProp()         {}
    ~TetraMassProp()        {}

    void SetPointMass( double massIn, vec3d& posIn );           // For Point Mass

    bool m_PointMassFlag;

    vec3d m_v0;
    vec3d m_v1;
    vec3d m_v2;
    vec3d m_v3;

    string m_CompId;

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
    TriShellMassProp( string id, double mass_area_in, vec3d& p0, vec3d& p1, vec3d& p2 );
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

//===========================================================================================================//
//================================================ DegenGeom ================================================//
//===========================================================================================================//

class DegenGeomTetraMassProp
{
public:
    DegenGeomTetraMassProp( string id, vec3d& p0, vec3d& p1, vec3d& p2, vec3d& p3 );
    DegenGeomTetraMassProp()        {}
    ~DegenGeomTetraMassProp()       {}

    vec3d m_v0;
    vec3d m_v1;
    vec3d m_v2;
    vec3d m_v3;

    string m_CompId;

    vec3d m_CG;

    double m_Vol;

    double m_Ixx;
    double m_Iyy;
    double m_Izz;

    double m_Ixy;
    double m_Ixz;
    double m_Iyz;
};

class DegenGeomTriShellMassProp
{
public:
    DegenGeomTriShellMassProp( string id, vec3d& p0, vec3d& p1, vec3d& p2 );
    ~DegenGeomTriShellMassProp()        {}

    vec3d m_v0;
    vec3d m_v1;
    vec3d m_v2;

    vec3d m_CG;

    string m_CompId;

    double m_TriArea;

    double m_Ixx;
    double m_Iyy;
    double m_Izz;

    double m_Ixy;
    double m_Ixz;
    double m_Iyz;
};

//===========================================================================================================//
//============================================== END DegenGeom ==============================================//
//===========================================================================================================//

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

    vec3d m_Pnt;
    vec3d m_UWPnt;
    int m_ID;

    vector< TTri* > m_TriVec;               // For WaterTight Check
    vector< TEdge* > m_EdgeVec;         // For WaterTight Check

    vector< TNode* > m_MergeVec;

    vector< TNode* > m_SplitNodeVec;

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

//  TNode* mapNode;

    int m_IsectFlag;

    enum { HAS_UNKNOWN = 0, HAS_XYZ = 1, HAS_UW = 2 };

protected:
    bool m_XYZFlag;
    int m_CoordInfo;
};

class TNodeGroup
{
public:
    vector< TNode* > nVec;
};

class TEdge
{
public:
    TEdge();
    TEdge( TNode* n0, TNode* n1, TTri* par_tri );
    virtual ~TEdge()        {}

    TNode* m_N0;
    TNode* m_N1;

    TTri* m_Tri0;                           // For WaterTight Check
    TTri* m_Tri1;

    virtual void SetParTri( TTri* par_tri )
    {
        m_ParTri = par_tri;
    }
    virtual TTri* GetParTri()
    {
        return m_ParTri;
    }
    virtual TMesh* GetParTMesh();

protected:
    TTri* m_ParTri; // Tri that edge is apart of

};



class TTri
{
public:
    TTri();
    virtual ~TTri();

    TNode* m_N0;
    TNode* m_N1;
    TNode* m_N2;

    vec3d m_Norm;

    vector< TEdge* > m_ISectEdgeVec;        // List of Intersection Edges
    vector< TTri* > m_SplitVec;             // List of split tris
    vector< TNode* > m_NVec;                // Nodes for split tris
    vector< TEdge* > m_EVec;                // Edges for split tris
    TEdge* m_PEArr[3];                          // Perimeter Edge Array

    virtual void CopyFrom( const TTri* tri );
    virtual void SplitTri();              // Split Tri to Fit ISect Edges
    virtual void TriangulateSplit( int flattenAxis );
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
    virtual double ComputeAwArea()
    {
        vec3d t1, t2, t3;
        t1.set_xyz( 0.0, m_N0->m_Pnt.y(), m_N0->m_Pnt.z() );
        t2.set_xyz( 0.0, m_N1->m_Pnt.y(), m_N1->m_Pnt.z() );
        t3.set_xyz( 0.0, m_N2->m_Pnt.y(), m_N2->m_Pnt.z() );
        return area( t1, t2, t3 );
    }

    virtual TNode* GetTriNode( int i )
    {
        if ( i == 0 )
            return m_N0;
        if ( i == 1 )
            return m_N1;
        if ( i == 2 )
            return m_N2;
        return NULL;
    }

    virtual void ComputeCosAngles( double* ang0, double* ang1, double* ang2 );

    virtual void SplitEdges( TNode* n01, TNode* n12, TNode* n20 );

    virtual vec3d CompNorm();
    virtual bool  ShareEdge( TTri* t );
    virtual bool MatchEdge( TNode* n0, TNode* n1, TNode* nA, TNode* nB, double tol );

    virtual void SetTMeshPtr( TMesh* tmesh )
    {
        m_TMesh = tmesh;
    }
    virtual TMesh* GetTMeshPtr()
    {
        return m_TMesh;
    }

    virtual void BuildPermEdges();

    virtual int OnEdge( vec3d & p, TEdge* e, double onEdgeTol, double * t = NULL );
    virtual vec3d CompPnt( const vec3d & uw_pnt );

    int m_InteriorFlag;
    string m_ID;
    vector<int> m_Tags;
    double m_Mass;
    int m_InvalidFlag;

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

    BndBox m_Box;
    vector< TTri* > m_TriVec;

    TBndBox* m_SBoxVec[8];      // Split Bnd Boxes

    void SplitBox();
    void AddTri( TTri* t );
    virtual void Intersect( TBndBox* iBox, bool UWFlag = false );
    virtual void NumCrossXRay( vec3d & orig, vector<double> & tParmVec );
    virtual void RayCast( vec3d & orig, vec3d & dir, vector<double> & tParmVec );
    virtual void AddLeafNodes( vector< TBndBox* > & leafVec );

    virtual void SegIntersect( vec3d & p0, vec3d & p1, vector< vec3d > & ipntVec );
    virtual bool CheckIntersect( TBndBox* iBox );
    virtual double MinDistance( TBndBox* iBox, double curr_min_dist );

};

class NBndBox
{
public:
    NBndBox();
    virtual ~NBndBox();

    BndBox m_Box;
    vector< TNode* > m_NodeVec;

    NBndBox* m_SBoxVec[8];      // Split Bnd Boxes

    void SplitBox( double maxSize );
    void AddNode( TNode* n );
    virtual void AddLeafNodes( vector< NBndBox* > & leafVec );

};

class Geom;

class TMesh
{
public:
    TMesh();
    virtual ~TMesh();

    vector< TTri* >  m_TVec;
    vector< TNode* > m_NVec;
    vector< TEdge* > m_EVec;

    TBndBox m_TBox;

    void copy( TMesh* m );
    void CopyFlatten( TMesh* m );
    virtual xmlNodePtr EncodeXml( xmlNodePtr & node );
    virtual void DecodeXml( xmlNodePtr & node );
    virtual xmlNodePtr EncodeTriList( xmlNodePtr & node );
    virtual void DecodeTriList( xmlNodePtr & node, int num_tris );

    //==== Stuff Copied From Geom That Created This Mesh ====//
    string m_PtrID;
    //bool reflected_flag;
    string m_NameStr;
    int m_SurfNum; // To keep track of geoms with multiple surfaces
    int m_MaterialID;
    vec3d m_Color;
    int m_MassPrior;
    double m_Density;
    double m_ShellMassArea;
    bool m_ShellFlag;
    DragFactors m_DragFactors;

    double m_TheoArea;
    double m_WetArea;
    double m_TheoVol;
    double m_GuessVol;
    double m_WetVol;
    vec3d m_AreaCenter;

    void LoadGeomAttributes( Geom* geomPtr );
    int  RemoveDegenerate();
    void Intersect( TMesh* tm, bool UWFlag = false );
    bool CheckIntersect( TMesh* tm );
    double MinDistance( TMesh* tm, double curr_min_dist );
    void Split();
    void DeterIntExt( vector< TMesh* >& meshVec );
    void DeterIntExtTri( TTri* tri, vector< TMesh* >& meshVec );
    void MassDeterIntExt( vector< TMesh* >& meshVec );
    void MassDeterIntExtTri( TTri* tri, vector< TMesh* >& meshVec );

    void LoadBndBox();

    virtual double ComputeTheoArea();
    virtual double ComputeWetArea();
    virtual double ComputeAwaveArea();
    virtual double ComputeTheoVol();
    virtual double ComputeTrimVol();

    virtual void AddTri( const vec3d & v0, const vec3d & v1, const vec3d & v2, const vec3d & norm );
    virtual void AddTri( TNode* node0, TNode* node1, TNode* node2, const vec3d & norm );
    virtual void AddTri( const vec3d & v0, const vec3d & v1, const vec3d & v2, const vec3d & norm, const vec3d & uw0,
                         const vec3d & uw1, const vec3d & uw2 );
    virtual void AddTri( const TTri* tri );
    virtual void AddUWTri( const vec3d & uw0, const vec3d & uw1, const vec3d & uw2, const vec3d & norm );

    virtual void WriteSTLTris( FILE* file_id, Matrix4d XFormMat );

    virtual vec3d GetVertex( int index );
    virtual int   NumVerts();

    virtual void WaterTightCheck( FILE* fid, vector< TMesh* > & tMeshVec );
    virtual void FindEdge( TNode* node, TTri* tri0, TTri* tri1 );
    virtual void AddEdge( TTri* tri0, TTri* tri1, TNode* node0, TNode* node1 );
    virtual void SwapEdge( TEdge* edge );

    bool m_DeleteMeFlag;
    vector< TTri* > m_NonClosedTriVec;
    virtual void MergeNonClosed( TMesh* tm );
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

    virtual void SubTag( int part_num, bool tag_subs ); // Subtag all triangles, if split triangles exist tag them the same as their parent

    virtual void MakeNodePntUW(); // Swaps Node->m_Pnt with Node->m_UWPnt
    virtual void MakeNodePntXYZ();

    virtual void SplitAliasEdges( TTri* orig_tri, TEdge* isect_edge );
    virtual vec3d CompPnt( const vec3d & uw_pnt );

    static void StressTest();
    static double Rand01();

    vector< vec3d > m_VertVec;
    vector< vec3d > m_ISectPairs;

    bool m_HalfBoxFlag;

    vector< vector<vec3d> > m_UWPnts;
    vector< vector<vec3d> > m_XYZPnts;

protected:
    void CopyAttributes( TMesh* m );

    map< TNode*, list<TNode*> > m_NAMap; // Map from a master node to list of nodes that are aliases
    map< TNode*, TNode* > m_NSMMap;      // Map of node slave to master node
    map< TEdge*, vector<TEdge*> > m_EAMap; // Map from a master edge to a list of edges that are aliases
    map< TEdge*, TEdge* > m_ESMMap;      // Map from edge slave to master edge

};




#endif
