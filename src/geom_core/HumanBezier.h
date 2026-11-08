//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// HumanBezier.h:
// The surface representation of a Human: overlapping closed parts, each a bicubic
// B-spline net linear in the anthropometric inputs, posed by HumanGeom through its tet
// mesh.  The data come from src/geom_core/humanshape/human_bezier.py.
//
//////////////////////////////////////////////////////////////////////

#if !defined(VSP_HUMAN_BEZIER__INCLUDED_)
#define VSP_HUMAN_BEZIER__INCLUDED_

#include <vector>

#include "Matrix4d.h"
#include "Vec3d.h"
#include "VspSurf.h"

using std::vector;

// One stored part, as laid out in HumanBezierData.h.
struct HumanBezPartData
{
    const char *m_Name;
    bool m_Centre;          // Mirror symmetric; otherwise a right part that also gives the left one.
    int m_NU;               // Control points around.
    int m_NV;               // Control points along.
    const double *m_UKnots;
    const double *m_VKnots;
    const double *m_PBar;
    const double *m_Q;
};

// One part's control net, m_Pnts[i][j] with i around and j along.
struct HumanBezNet
{
    HumanBezNet()
    {
        m_Part = 0;
        m_Mirror = false;
    }

    int m_Part;             // The stored part.
    bool m_Mirror;          // The mirror image of the stored right part, around index i at ( 2 - i ) mod nu.
    vector < vector < vec3d > > m_Pnts;
};

// The nets of every part in the data pose: torso, head, then each right part followed by its
// left one.
void BuildHumanBezierNets( int gender, const vector < double > &anthro, vector < HumanBezNet > &nets );

// The surfaces of the nets, moved by place.
void BuildHumanBezierSurfs( int gender, const vector < HumanBezNet > &nets, const Matrix4d &place,
                            vector < VspSurf > &surfs );

// The right and left eyes, each at a fixed point of the head's surface.
void GetHumanBezierEyes( int gender, const vector < HumanBezNet > &nets, const vector < VspSurf > &surfs,
                         vec3d &reye, vec3d &leye );

#endif // !defined(VSP_HUMAN_BEZIER__INCLUDED_)
