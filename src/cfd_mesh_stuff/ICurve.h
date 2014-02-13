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
#include "BezierCurve.h"

#include "Vec2d.h"
#include "Vec3d.h"

#include "BndBox.h"

#include <assert.h>

#include <vector>
#include <deque>
#include <list>
using namespace std;

class ISegChain;
class SharedPnt;
class ISeg;


//////////////////////////////////////////////////////////////////////
class ICurve
{
public:

    ICurve();
    virtual ~ICurve();

    bool Match( SCurve* crv_A, SCurve* crv_B );
    void BorderTesselate( );
    void PlaneBorderTesselate( SCurve* sca, SCurve* scb );
    void SetACurve( SCurve* crv_A )
    {
        m_SCurve_A = crv_A;
    }


    void Draw();

    void DebugEdgeMatching( FILE* fp );


    SCurve* m_SCurve_A;
    SCurve* m_SCurve_B;
    bool m_PlaneBorderIntersectFlag;


protected:



};


#endif
