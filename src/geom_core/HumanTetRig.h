//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
// HumanTetRig.h:
// The tetrahedral mesh that fills a Human, and its posing by volumetric as-rigid-as-possible
// deformation (ARAP) or by Fast Automatic Skinning Transformations (FAST), both as libigl formulates
// them.
//
//////////////////////////////////////////////////////////////////////

#if !defined(VSP_HUMAN_TET_RIG__INCLUDED_)
#define VSP_HUMAN_TET_RIG__INCLUDED_

#include <memory>
#include <vector>

#include "Matrix4d.h"
#include "Vec3d.h"

using std::vector;

struct HumanTetRigData;

// Rotations a handle may add a fraction of to its bone's transform, to spread a twist or bend along a
// bone; in the order of PARTIALS in humanshape/human_tets.py.
enum HumanTetPartial
{
    HUMAN_PARTIAL_TORSO,
    HUMAN_PARTIAL_NECK,
    HUMAN_PARTIAL_LSHOULDER_IE,
    HUMAN_PARTIAL_LPRONATION,
    HUMAN_PARTIAL_LHIP_IE,
    HUMAN_PARTIAL_RSHOULDER_IE,
    HUMAN_PARTIAL_RPRONATION,
    HUMAN_PARTIAL_RHIP_IE,
    HUMAN_NUM_PARTIALS
};

// A rotation by m_Angle (radians, as Matrix4d::rotate turns) about the line through m_Pivot along
// m_Axis, in the data pose.
struct HumanPartialRot
{
    HumanPartialRot()
    {
        m_Angle = 0.0;
    }

    vec3d m_Pivot;
    vec3d m_Axis;
    double m_Angle;
};

class HumanTetRig
{
public:
    HumanTetRig();
    ~HumanTetRig();

    // Place the tet mesh for one body: its sex, anthropometric vector, full skin mesh vertices and
    // joints (mm), in the data pose.  ARAP and FAST precompute what each needs when first asked.
    void Build( int gender, const vector < double > &vars, const vector < vec3d > &skin, const vector < vec3d > &joints );

    bool IsBuilt() const;

    // Pose the skin by the bone transformations (indexed by HumanGeom's bones) and the partial
    // rotations (indexed by HumanTetPartial).  ARAP iterates from dual quaternion skinning of the tet
    // mesh until no skin vertex moves more than tol (mm) over a block of iterations, or max_iter; it
    // returns the iterations it took.  If a method's factorization fails, the skin is left as given.
    int PoseARAP( const vector < Matrix4d > &bone_mats, const vector < HumanPartialRot > &partials, int max_iter,
                  double tol, vector < vec3d > &skin );
    void PoseFAST( const vector < Matrix4d > &bone_mats, const vector < HumanPartialRot > &partials, int iters,
                   vector < vec3d > &skin );

private:
    void PrecomputeARAP();
    void PrecomputeFAST();
    void SkinFromTets( const vector < vec3d > &tv, vector < vec3d > &skin ) const;

    std::unique_ptr < HumanTetRigData > m_Data;
};

#endif // !defined(VSP_HUMAN_TET_RIG__INCLUDED_)
