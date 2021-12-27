//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// HumanGeom.h:
// Rob McDonald
//
//////////////////////////////////////////////////////////////////////

#if !defined(VSP_HUMAN_GEOM__INCLUDED_)
#define VSP_HUMAN_GEOM__INCLUDED_

#include <set>
#include <unordered_set>
#include <map>

#include "Geom.h"
#include "XSec.h"
#include "Defines.h"
#include "Vec3d.h"
#include "Vec2d.h"
#include "VspUtil.h"
#include "Vsp1DCurve.h"
#include "ResultsMgr.h"

#include "attachment.h"
#include "pin_mesh.h"
#include "skeleton.h"

#define REAL_T float

#define NUM_MESH_TRI 2885
#define NUM_MESH_VERT 1488

#define NUM_SKEL 18
#define NUM_SKEL_VERT 11

class HumanGeom : public Geom {
public:
    HumanGeom(Vehicle *vehicle_ptr);

    virtual ~HumanGeom();

    // These enums are only used locally -- do not move to API.
    enum joints {
          ORIGIN,      // 0
          BACK,        // 1
          WAIST,       // 2
          HEAD,        // 3
          LHIP,        // 4
          LKNEE,       // 5
          LANKLE,      // 6
          LTOE,        // 7
          RHIP,        // 8
          RKNEE,       // 9
          RANKLE,      // 10
          RTOE,        // 11
          LSHOULDER,   // 12
          LELBOW,      // 13
          LHAND,       // 14
          RSHOULDER,   // 15
          RELBOW,      // 16
          RHAND        // 17
    };

    enum bones {
        UPSPINE,        // 0
        LOWSPINE,       // 1
        NECK,           // 2
        LPELVIS,        // 3
        LTHIGH,         // 4
        LSHIN,          // 5
        LFOOT,          // 6
        RPELVIS,        // 7
        RTHIGH,         // 8
        RSHIN,          // 9
        RFOOT,          // 10
        LCLAV,          // 11
        LBICEP,         // 12
        LFOREARM,       // 13
        RCLAV,          // 14
        RBICEP,         // 15
        RFOREARM        // 16
    };

    enum PRESET_POSE {
        STANDING,
        SITTING
    };

    virtual void SetPreset();

    virtual double CalculateVolume();

    virtual void UpdateBBox();
    virtual void UpdateDrawObj();

    virtual void UpdateSymmAttach();

    virtual void ValidateParms( );

    virtual void LoadDrawObjs( vector< DrawObj* > & draw_obj_vec );

    virtual int  GetNumXSecSurfs() const
    {
        return 0;
    }

    virtual void Scale();

    virtual int GetNumMainMeshs() const
    {
        return 1;
    }
    virtual int GetNumTotalMeshs() const;

    virtual int GetNumMainSurfs() const
    {
        return 0;
    }

    virtual void CreateDegenGeom( vector<DegenGeom> &dgs, bool preview = false );

    virtual vector< TMesh* > CreateTMeshVec() const;

    virtual void SetupMesh( Pinocchio::Mesh &m );
    template < typename vertmat > void SetupSkel( const vertmat & vm, Pinocchio::DataSkeleton &skeleton );
    static Pinocchio::Attachment * SetupAttach( const Pinocchio::Mesh &m, const Pinocchio::Skeleton &skeleton );

    template < typename vertmat > void CopyVertsToMesh( const vertmat & vm, Pinocchio::Mesh &m );
    template < typename vertmat > void CopyMeshToVerts( const Pinocchio::Mesh &m, vertmat & vm );

    void CopyVertsToSkel( const vector < vec3d > & sv );

    IntParm m_LenUnit;
    IntParm m_MassUnit;

    IntParm m_PresetPose;  // Used by GUI when set preset pose is triggered.

    IntParm m_GenderFlag;

    Parm m_Stature;
    Parm m_Stature_pct;

    Parm m_BMI;
    Parm m_BMI_pct;
    Parm m_Mass;
    Parm m_Volume;
    BoolParm m_AutoDensity;

    Parm m_Age;
    Parm m_SitFrac;

    BoolParm m_ShowSkelFlag;


    BoolParm m_RLSymFlag;

    Parm m_ElbowRt;
    Parm m_ShoulderABADRt;
    Parm m_ShoulderFERt;
    Parm m_ShoulderIERt;

    Parm m_HipABADRt;
    Parm m_HipFERt;
    Parm m_KneeRt;
    Parm m_AnkleRt;

    Parm m_ElbowLt;
    Parm m_ShoulderABADLt;
    Parm m_ShoulderFELt;
    Parm m_ShoulderIELt;

    Parm m_HipABADLt;
    Parm m_HipFELt;
    Parm m_KneeLt;
    Parm m_AnkleLt;

    Parm m_Back;
    Parm m_Waist;


protected:
    virtual void ApplyScale(); // this is for intersectTrim
    virtual void UpdateSurf();

    static const int m_half_tris[NUM_MESH_TRI][3];
    static const int m_skel_indx[NUM_SKEL];

    static const REAL_T m_male_half_verts[NUM_MESH_VERT][3];
    static const REAL_T m_male_half_pcs[NUM_MESH_VERT*3][200];

    static const REAL_T m_male_skel_verts[NUM_SKEL_VERT][3];
    static const REAL_T m_male_skel_pcs[NUM_SKEL_VERT*3][200];

    static const REAL_T m_male_coeffs[200][6];

    static const REAL_T m_female_half_verts[NUM_MESH_VERT][3];
    static const REAL_T m_female_half_pcs[NUM_MESH_VERT*3][200];

    static const REAL_T m_female_skel_verts[NUM_SKEL_VERT][3];
    static const REAL_T m_female_skel_pcs[NUM_SKEL_VERT*3][200];

    static const REAL_T m_female_coeffs[200][6];

    static void ComputeScore( const REAL_T C[200][6], const vector < double > &X, vector < double > &score );

    static void ComputeResultsMesh( const REAL_T P[][200], const vector < double > &score, const REAL_T Ybar[][3], vector < vec3d > &Y );
    static void ComputeResultsSkel( const REAL_T P[][200], const vector < double > &score, const REAL_T Ybar[][3], vector < vec3d > &Y );

    template < typename vertmat >
    static void ComputeAvePts( const vertmat &A, const vertmat &B, int n, vertmat &C );

private:

    static Vsp1DCurve m_MaleStatureECDF;
    static Vsp1DCurve m_FemaleStatureECDF;

    static Vsp1DCurve m_MaleBMIECDF;
    static Vsp1DCurve m_FemaleBMIECDF;

    static Pinocchio::Mesh m_MasterMesh;
    static Pinocchio::Attachment *m_MasterAttach;

    static unordered_set < int > m_VertCopySet;
    // m_VertCopySet contains the lower indices [0:NUM_MESH_VERT-1] that were double-referenced instead of reflected to
    // form the upper indices [NUM_MESH_VERT:2*NUM_MESH_VERT-1].
    //
    // When iterating over all indices k in [0:2*NUM_MESH_VERT-1], the lower half are assumed 'normal'.  The upper half
    // are either in this set (referenced to lower half indices k-NUM_MESH_VERT) or are reflections of the lower half
    // point.  Test with: m_VertCopySet.count( klower );


    vector < vec3d > m_MainVerts;

    vector < vec3d > m_SkelVerts;
    vector < vec3d > m_PoseSkelVerts;

    vector < vector < vec3d > > m_Verts;

    vector < bool > m_FlipNormal;


    void ComputeShoulderTrans( const int &ishoulder, const int &ielbow, const double &ang1, const double &ang2, Matrix4d &T );
    void ComputeElbowTrans( const int &ishoulder, const int &ielbow, const int &ihand, const double &ang, const double &ang2, Matrix4d &T );

    void ComputeHipTrans( const int &iwaist, const int &ihip, const int &iknee, const double &ang1, const double &ang2, Matrix4d &T );
    void ComputeKneeTrans( const int &ihip, const int &iknee, const int &iankle, const double &ang, Matrix4d &T );
    void ComputeAnkleTrans( const int &iknee, const int &iankle, const int &itoe, const double &ang, Matrix4d &T );

    void ComputeBackTrans( const int &ineck, const int &iback, const int &iwaist, const double &ang, Matrix4d &T );
    void ComputeWaistTrans( const int &iback, const int &iwaist, const int &ihip, const double &ang, Matrix4d &T );

    double Get_mm2UX();

    static void DebugDump();
};

#endif // !defined(VSP_HUMAN_GEOM__INCLUDED_)
