//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

//////////////////////////////////////////////////////////////////////
// ISegChain.h
// J.R Gloudemans
//////////////////////////////////////////////////////////////////////

#if !defined(ISEGCHAIN_ISEGCHAIN__INCLUDED_)
#define ISEGCHAIN_ISEGCHAIN__INCLUDED_

#include "nanoflann.hpp"

#include "Surf.h"
#include "GridDensity.h"
#include "BezierCurve.h"
#include "SCurve.h"

#include "Vec2d.h"
#include "Vec3d.h"

#include "BndBox.h"

#include "MapSource.h"

#include <cassert>

#include <vector>
#include <deque>
#include <list>
using namespace std;

class ISegChain;
class SharedPnt;
class ISeg;
class ISegSplit;
class IPntBin;
class SurfaceIntersectionSingleton;
class Ipnt;

struct IPntCloud;

typedef KDTreeSingleIndexAdaptor< L2_Simple_Adaptor< double, IPntCloud > , IPntCloud, 3 > IPntTree;


//==== UW Point on Surface ====//
class Puw
{
public:

    Puw();
    Puw( Surf* s, const vec2d &uw );
    virtual ~Puw();
    Surf* m_Surf;
    vec2d m_UW;
};

//==== Triangle edge an intersection point lies on ====//
// A segment of an intersection curve is found between two flat triangles, one from each
// surface's patch, and each of its ends lies on an edge of one of them.  The segment next along
// the curve lies in the triangle on the other side of that edge, so the two share it.
struct IPntEdge
{
    enum { NONE, U_LINE, W_LINE, DIAGONAL };

    IPntEdge()
    {
        m_Kind = NONE;
        m_SurfID = -1;
        m_Val[0] = m_Val[1] = m_Val[2] = m_Val[3] = 0.0;
        m_Side = 0;
        m_Along = 0.0;
    }

    int m_Kind;

    // The surface the edge is on
    int m_SurfID;

    // A patch border: the u or w it runs along.  A patch's diagonal: the patch's u and w bounds.
    double m_Val[4];

    // Which side of the edge the segment is on: for a border, +1 where its patch lies above the
    // line and -1 below; for a diagonal, which of the patch's two triangles
    int m_Side;

    // Where along the edge the point is: the other parameter on a border, the fraction of the
    // way from the patch's first corner to its opposite one on a diagonal
    double m_Along;
};

//==== Shared Intersection Point ====//
class IPnt
{
public:
    IPnt();
    IPnt( Puw* p0, Puw* p1 );
    virtual ~IPnt();

    void CompPnt();

    void CompPnt_WithMetrics();
    double CalcDave();
    void DumpMatlab( FILE* fp, int figno );
    void GetDOPts( vector < vec3d > &pts );

    Puw* GetPuw( Surf* surf  );

    void AddPuws( IPnt* ip );

    void AddSegRef( ISeg* seg );
    void RemoveSegRef( ISeg* seg );

    bool m_UsedFlag;
    bool m_GroupedFlag;
    vec3d m_Pnt;
    deque< Puw* >  m_Puws;
    deque< ISeg* > m_Segs;

    // The edge the point lies on, and the point of the segment across it, where there is one
    IPntEdge m_Edge;
    IPnt* m_Partner;
};

//==== Intersection Segment ====//
class ISeg
{
public:
    ISeg();
    ISeg( Surf* sA, Surf* sB, IPnt* ip0, IPnt* ip1 );
    virtual ~ISeg();

    IPnt* m_IPnt[2];            // End Points of Seg

    Surf* m_SurfA;
    Surf* m_SurfB;

    void Copy( const ISeg & s );
    void FlipDir();
    double MinDist( ISeg* seg );
    double MinDist( IPnt* ip  );
    void JoinBack( ISeg* seg );
    void JoinFront( ISeg* seg );
    ISeg* Split( const ISegSplit &split, SurfaceIntersectionSingleton *MeshMgr );

    bool Match( ISeg* seg );


    // void Draw();

};

class ISegSplit
{
public:

    ISegSplit()
    {
        m_Index = 0;
        m_Fract = 0.0;
        m_Surf = nullptr;
        m_OtherFlag = false;
    }

    int m_Index;
    double m_Fract;
    Surf* m_Surf;
    vec2d m_UW;
    vec3d m_Pnt;

    // The split in the parameters of the chain's other surface, where they are known
    bool m_OtherFlag;
    vec2d m_UWOther;
};

//==== Bound Box Surrounding ISeg Chains ====//
class ISegBox
{
public:

    ISegBox()
    {
        m_BeginInd = 0;
        m_EndInd = 0;
        m_SubBox[0] = m_SubBox[1] = nullptr;
        m_ChainPtr = nullptr;
        m_Surf = nullptr;
    }
    virtual ~ISegBox();

    int m_BeginInd;
    int m_EndInd;

    Surf* m_Surf;
    ISegChain* m_ChainPtr;

    BndBox m_Box;

    ISegBox* m_SubBox[2];

    void BuildSubDivide();

    void Intersect( ISegBox* box );

    // void Draw();

    void AppendLineSegs( vector < vec3d > &lsegs );

};

//==== ISeg Chain - Intersection Between Two Surfaces ====//
class ISegChain
{
public:

    ISegChain();
    virtual ~ISegChain();


    void FlipDir();

    void AddSeg( ISeg* seg, bool frontFlag );

    // Add seg at the front or back, turned so its end joinIPnt is next to the chain
    void AddSeg( ISeg* seg, bool frontFlag, IPnt* joinIPnt );

    double MatchDist( ISeg* s );
    double ChainDist( ISegChain* B );
    bool Match( ISegChain* B );

    void Intersect( Surf* surfPtr, ISegChain* B );

    void AddSplit( Surf* surfPtr, int index, const vec2d &int_pnt, double t );

    // A split whose parameters on the chain's other surface are known too
    void AddSplit( Surf* surfPtr, int index, const vec2d &int_pnt, double t, const vec2d &uw_other );
    bool AddBorderSplit( Puw* uw ); // Return true if split successfully added

    void MergeSplits();
    void RemoveChainEndSplits();
    vector< ISegChain* > SortAndSplit( SurfaceIntersectionSingleton *MeshMgr );
    vector< ISegChain* > FindCoPlanarChains( Surf* surfPtr, SurfaceIntersectionSingleton *MeshMgr );
    void MergeInteriorIPnts();

    void BuildCurves();
    void TransferTess();
    void ApplyTess( SurfaceIntersectionSingleton *MeshMgr );

    void SpreadDensity( );
    void CalcDensity( SimpleGridDensity* grid_den, list< MapSource* > & splitSources );
    void Tessellate();
    void TessEndPts();

    virtual ISegChain* GetWakeAttachChain()
    {
        return m_WakeAttachChain;
    }
    virtual void SetWakeAttachChain( ISegChain* c )
    {
        m_WakeAttachChain = c;
    }

    void BuildBoxes();

    // void Draw();

    bool Valid();

    bool m_BorderFlag;
    int m_SSIntersectIndex; // Corresponds to index in FeaStructure m_FeaSubSurfVec

    // A crease where two pieces were joined into one patch, not a subsurface the user drew
    bool m_PatchJoinFlag;

    // The parameter line of each parent the chain runs along, where it is one
    ParmLine m_ALine;
    ParmLine m_BLine;

    ISegChain* m_WakeAttachChain;

    deque < ISeg* > m_ISegDeque;

    ISegBox m_ISegBoxA;
    ISegBox m_ISegBoxB;

    Surf* m_SurfA;
    Surf* m_SurfB;

    vector< ISegSplit* > m_SplitVec;

    SCurve m_ACurve;                // UW Curve for Surf A
    SCurve m_BCurve;

    deque< IPnt* > m_TessVec;

    vector< IPnt* > m_CreatedIPnts;



};

//==== Group of IPnts ====//
class IPntGroup
{
public:
    IPntGroup()                     {}
    virtual ~IPntGroup()            {}

    vector< IPnt* > m_IPntVec;

    double GroupDist( IPntGroup* g );
    void AddGroup( IPntGroup* g );
};



struct IPntCloud
{
    vector < IPnt* > m_IPnts;

    // Must return the number of data points
    inline size_t kdtree_get_point_count() const
    {
        return m_IPnts.size();
    }

    // Returns the dim'th component of the idx'th point in the class:
    inline double kdtree_get_pt( const size_t idx, int dim ) const
    {
        return m_IPnts[idx]->m_Pnt.v[dim];
    }

    // Optional bounding-box computation: return false to default to a standard bbox computation loop.
    //   Return true if the BBOX was already computed by the class and returned in "bb" so it can be avoided to redo it again.
    //   Look at bb.size() to find out the expected dimensionality (e.g. 2 or 3 for point clouds)
    template <class BBOX>
    bool kdtree_get_bbox( BBOX &bb ) const
    {
        return false;
    }
};

#endif
