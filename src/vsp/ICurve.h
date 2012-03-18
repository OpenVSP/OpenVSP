//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

//////////////////////////////////////////////////////////////////////
// ICurve.h
// J.R Gloudemans
//////////////////////////////////////////////////////////////////////

#if !defined(ICURVE_ICURVE__INCLUDED_)
#define ICURVE_ICURVE__INCLUDED_

#include "Surf.h"
#include "GridDensity.h"
#include "bezier_curve.h"

#include "vec2d.h"
#include "vec3d.h"

#include "bbox.h"

#include <assert.h>

#include <vector>
#include <deque>
#include <list>
using namespace std;

class ISegChain;
class SharedPnt;
class ISeg;


//class ISeg
//{
//public:
//
//	ISeg();
//	virtual ~ISeg();
//
//	void Copy( ISeg & s );
//	void FlipDir();
//	vec2d* GetPnt( Surf* sPtr, int ind );
//
//	Surf* m_SurfA;
//	Surf* m_SurfB;
//
//	vec2d m_AUW[2];		// End Points UW Space
//	vec2d m_BUW[2];
//
//	vec3d m_Pnt[2];
//
//	SharedPnt* m_SharedPnt[2];
//
//	bool m_DebugFlag;
//
//};

//class SharedPnt
//{
//public:
//
//	vec3d m_Pnt;
//	vector< ISeg* > m_Segs;
//};
//
//class ISegSplit
//{
//public:
//
//	Surf* m_Surf;
//	ISegChain* m_Chain;
//
//	int m_Index;
//	vec2d m_UW;
//
//	Surf* m_OtherSurf;
//	vec2d m_OtherUW;
//
//	bool Match( ISegSplit* split );
//};
//
//
//class ISegBox
//{
//public:
//
//	ISegBox()				{ m_SubBox[0] = m_SubBox[1] = NULL;  m_ChainPtr = NULL; m_Surf = NULL; }
//
//	int m_BeginInd;
//	int m_EndInd;
//
//	Surf* m_Surf;
//	ISegChain* m_ChainPtr;
//
//	bbox m_Box;
//
//	ISegBox* m_SubBox[2];
//
//	void BuildSubDivide();
//
//	void Intersect( ISegBox* box );
//
//	void Draw();
//
//};
//
//
//class ISegChain
//{
//public:
//
//	ISegChain();
//	virtual ~ISegChain();
//
//	deque < ISeg* > m_ISegDeque;
//
//	void FlipDir();
//
//	void AddSeg( ISeg* s );
//	void AddChain( ISegChain* B );
//
//	double MatchDist( ISeg* s );
//	double ChainDist( ISegChain* B );
//
//	void Intersect( Surf* surfPtr, ISegChain* B );
//
//	void GroupSplit( Surf* surfPtr, int index, vec2d int_pnt );
//
//	void BuildBoxes();
//
//	void Draw();
//
//	ISegBox m_ISegBoxA;
//	ISegBox m_ISegBoxB;
//
//	Surf* m_SurfA;
//	Surf* m_SurfB;
//
//
//};

//////////////////////////////////////////////////////////////////////
class ICurve
{
public:

	ICurve();
	virtual ~ICurve();

	bool Match( SCurve* crv_A, SCurve* crv_B );
	void Tesselate( GridDensity* grid_den );
	void SetACurve( SCurve* crv_A )						{ m_SCurve_A = crv_A; }


	void Draw();

	void DebugEdgeMatching(FILE* fp);

	SCurve* m_SCurve_A;
	SCurve* m_SCurve_B;

protected:




};


#endif 
