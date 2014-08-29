//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

#if !defined(SURF_CORE__INCLUDED_)
#define SURF_CORE__INCLUDED_

#include "Vec2d.h"
#include "Vec3d.h"

#include "Mesh.h"
#include "GridDensity.h"
#include "SurfPatch.h"
#include "MapSource.h"

#include <assert.h>

#include <string>
#include <vector>
#include <list>
#include <set>
#include <map>

//////////////////////////////////////////////////////////////////////
class SurfCore
{
public:

    SurfCore();
    virtual ~SurfCore();

    //===== Bezier Funcs ====//
    vec3d CompPnt( double u, double w );
    vec3d CompTanU( double u, double w );
    vec3d CompTanW( double u, double w );
    vec3d CompTanUU( double u, double w );
    vec3d CompTanWW( double u, double w );
    vec3d CompTanUW( double u, double w );

    vec3d CompPnt01( double u, double w );
    vec3d CompTanU01( double u, double w );
    vec3d CompTanW01( double u, double w );
    vec3d CompTanUU01( double u, double w );
    vec3d CompTanWW01( double u, double w );
    vec3d CompTanUW01( double u, double w );

    void CompCurvature( double u, double w, double& k1, double& k2, double& ka, double& kg );


    double GetUWArea()
    {
        return m_MaxU * m_MaxW;
    }

    int GetNumU()
    {
        return m_NumU;
    }
    int GetNumW()
    {
        return m_NumW;
    }
    double GetMaxU()
    {
        return m_MaxU;
    }
    double GetMaxW()
    {
        return m_MaxW;
    }

    void SetControlPnts( vector< vector < vec3d > > pnts );
    vector< vector< vec3d > > GetControlPnts()
    {
        return m_Pnts;
    }


    bool LessThanY( double val );
    bool OnYZeroPlane();
    bool PlaneAtYZero();

    void LoadBorderCurves( vector< vector <vec3d> > & borderCurves );

    bool SurfMatch( SurfCore* otherSurf );

protected:

    int m_NumU;
    int m_NumW;

    double m_MaxU;
    double m_MaxW;
    vector< vector< vec3d > > m_Pnts;           // Control Pnts


    vec3d CompBez( double u, double w,
                   void ( *uBlendFun )( double u, double& F1, double& F2, double& F3, double& F4 ),
                   void ( *wBlendFun )( double u, double& F1, double& F2, double& F3, double& F4 ) );

    static void BlendFuncs( double u, double& F1, double& F2, double& F3, double& F4 );
    static void BlendDerivFuncs( double u, double& F1, double& F2, double& F3, double& F4 );
    static void BlendDeriv2Funcs( double u, double& F1, double& F2, double& F3, double& F4 );
};

#endif
