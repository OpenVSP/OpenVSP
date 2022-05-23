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
    Puw( Surf* s, vec2d  uw );
    virtual ~Puw();
    Surf* m_Surf;
    vec2d m_UW;
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

    int m_Index;
    bool m_UsedFlag;
    bool m_GroupedFlag;
    vec3d m_Pnt;
    deque< Puw* >  m_Puws;
    deque< ISeg* > m_Segs;
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
    ISeg* Split( Surf* sPtr, vec2d & uw, SurfaceIntersectionSingleton *MeshMgr );

    bool Match( ISeg* seg );


    // void Draw();

};

class ISegSplit
{
public:

    int m_Index;
    double m_Fract;
    Surf* m_Surf;
    vec2d m_UW;
    vec3d m_Pnt;
};

//==== Bound Box Surrounding ISeg Chains ====//
class ISegBox
{
public:

    ISegBox()
    {
        m_BeginInd = 0;
        m_EndInd = 0;
        m_SubBox[0] = m_SubBox[1] = NULL;
        m_ChainPtr = NULL;
        m_Surf = NULL;
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

    void AddSeg( ISeg* s );
    void AddSeg( ISeg* seg, bool frontFlag );
    void AddChain( ISegChain* B );

    double MatchDist( ISeg* s );
    double ChainDist( ISegChain* B );
    bool Match( ISegChain* B );

    void Intersect( Surf* surfPtr, ISegChain* B );

    void AddSplit( Surf* surfPtr, int index, vec2d int_pnt );
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
