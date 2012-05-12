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

#include "Surf.h"
#include "GridDensity.h"
#include "bezier_curve.h"
#include "SCurve.h"

#include "vec2d.h"
#include "vec3d.h"

#include "bbox.h"

#include "mapSource.h"

#include <assert.h>

#include <vector>
#include <deque>
#include <list>
using namespace std;

class ISegChain;
class SharedPnt;
class ISeg;
class IPntBin;

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
	Puw* GetPuw( Surf* surf  );

	void AddPuws( IPnt* ip );

	void AddSegRef( ISeg* seg );
	void RemoveSegRef( ISeg* seg );

	int m_Index;
	bool m_UsedFlag;
	vec3d m_Pnt;
	deque< Puw* >  m_Puws;
	deque< ISeg* > m_Segs;
};

class IPntBin
{
public:
	int m_ID;							// x*1000 + y*1000 + z*1000

	deque< IPnt* > m_IPnts;
	vector< int > m_AdjBins;			// Adjancent Bins

	deque< ISegChain* > m_Chains;

	IPnt* Match( IPnt* ip, map< int, IPntBin > & binMap );
	void AddCompareIPnts( IPnt* ip, vector< IPnt* > & compareIPntVec );

	static int ComputeID( vec3d & pos );


};

//==== Intersection Segment ====//
class ISeg
{
public:
	ISeg();
	ISeg( Surf* sA, Surf* sB, IPnt* ip0, IPnt* ip1 );
	virtual ~ISeg();

	IPnt* m_IPnt[2];			// End Points of Seg

	Surf* m_SurfA;				
	Surf* m_SurfB;

	void Copy( ISeg & s );
	void FlipDir();
	double MinDist( ISeg* seg );
	double MinDist( IPnt* ip  );
	void JoinBack( ISeg* seg );
	void JoinFront( ISeg* seg );
	ISeg* Split( Surf* sPtr, vec2d & uw );

	bool Match( ISeg* seg );


	void Draw();

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

	ISegBox()				{ m_SubBox[0] = m_SubBox[1] = NULL;  m_ChainPtr = NULL; m_Surf = NULL; }
	virtual ~ISegBox();

	int m_BeginInd;
	int m_EndInd;

	Surf* m_Surf;
	ISegChain* m_ChainPtr;

	bbox m_Box;

	ISegBox* m_SubBox[2];

	void BuildSubDivide();

	void Intersect( ISegBox* box );

	void Draw();

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

	void Intersect( Surf* surfPtr, ISegChain* B );

	void AddSplit( Surf* surfPtr, int index, vec2d int_pnt );
	void AddBorderSplit( IPnt* ip, Puw* uw );

	void MergeSplits();
	void RemoveChainEndSplits();
	vector< ISegChain* > SortAndSplit();
	void MergeInteriorIPnts();

	void BuildCurves();
	void TransferTess();
	void ApplyTess();

	void CalcDensityBuildES( MSCloud &es_cloud, GridDensity* grid_den );
	void Tessellate( MSTree &es_tree, MSCloud &es_cloud, GridDensity* grid_den );

	void BuildBoxes();

	void Draw();

	bool Valid();

	bool m_BorderFlag;

	deque < ISeg* > m_ISegDeque;

	ISegBox m_ISegBoxA;
	ISegBox m_ISegBoxB;

	Surf* m_SurfA;
	Surf* m_SurfB;

	vector< ISegSplit* > m_SplitVec;

	SCurve m_ACurve;				// UW Curve for Surf A
	SCurve m_BCurve;

	deque< IPnt* > m_TessVec;

	vector< IPnt* > m_CreatedIPnts;



};

//==== Group of IPnts ====//
class IPntGroup
{
public:
	IPntGroup()						{}
	virtual ~IPntGroup()			{}

	vector< IPnt* > m_IPntVec;

	double GroupDist( IPntGroup* g );
	double GroupDistFract( IPntGroup* g );
	void AddGroup( IPntGroup* g );
};



#endif 
