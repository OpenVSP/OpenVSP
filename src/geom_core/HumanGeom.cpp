//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#include <unordered_set>

#include "HumanGeom.h"
#include "ParmMgr.h"
#include "Vehicle.h"

#include "PntNodeMerge.h"

#include "UnitConversion.h"

// Pinocchio #includes
#include "pinocchioApi.h"

#include "HumanGeomData.h"

unordered_set < int > HumanGeom::m_VertCopySet;
Pinocchio::Mesh HumanGeom::m_MasterMesh;
Pinocchio::Attachment *HumanGeom::m_MasterAttach = NULL;
Vsp1DCurve HumanGeom::m_MaleStatureECDF;
Vsp1DCurve HumanGeom::m_FemaleStatureECDF;
Vsp1DCurve HumanGeom::m_MaleBMIECDF;
Vsp1DCurve HumanGeom::m_FemaleBMIECDF;

//==== Constructor ====//
HumanGeom::HumanGeom( Vehicle* vehicle_ptr ) : Geom( vehicle_ptr )
{
    m_Name = "HumanGeom";
    m_Type.m_Name = "Human";
    m_Type.m_Type = HUMAN_GEOM_TYPE;

    m_TessU.Deactivate();
    m_TessW.Deactivate();
    m_Origin.Deactivate();

    m_LenUnit.Init( "LenUnit", "Anthropometric", this, vsp::LEN_FT, vsp::LEN_MM, vsp::LEN_YD );
    m_LenUnit.SetDescript( "Length unit" );

    m_MassUnit.Init( "MassUnit", "Anthropometric", this, vsp::MASS_UNIT_LBM, vsp::MASS_UNIT_G, vsp::MASS_LBFSEC2IN );
    m_MassUnit.SetDescript( "Mass unit" );

    m_GenderFlag.Init( "GenderFlag", "Anthropometric", this, vsp::MALE, vsp::MALE, vsp::FEMALE );
    m_GenderFlag.SetDescript( "Gender" );

    m_Stature.Init( "Stature", "Anthropometric", this, 1755.0*Get_mm2UX(), 1500.0*Get_mm2UX(), 2000.0*Get_mm2UX() );
    m_Stature.SetDescript( "Height of person" );

    m_Stature_pct.Init( "Stature_pct", "Anthropometric", this, .5, 0.05, 0.95 );
    m_Stature_pct.SetDescript( "Percentile height" );

    m_BMI.Init( "BMI", "Anthropometric", this, 20.0, 16.0, 55.0 );
    m_BMI.SetDescript( "Body mass index of person" );

    m_BMI_pct.Init( "BMI_pct", "Anthropometric", this, .5, 0.05, 0.95 );
    m_BMI_pct.SetDescript( "Percentile BMI" );

    m_Mass.Init("Mass", "Anthropometric", this, 1, 0, 1e6 );
    m_Mass.SetDescript( "Mass of person" );

    m_Volume.Init( "Volume", "Anthropometric", this, 0, 0, 1e12 );
    m_Volume.SetDescript( "Volume of person" );

    m_AutoDensity.Init( "AutoDensity", "Anthropometric", this, true, false, true );
    m_AutoDensity.SetDescript( "Flag to calculate density based on mass and volume" );

    m_Age.Init( "Age", "Anthropometric", this, 30.0, 18, 80 );
    m_Age.SetDescript( "Age of person" );

    m_SitFrac.Init( "SitFrac", "Anthropometric", this, 0.51, 0.4, 0.6 );
    m_SitFrac.SetDescript( "Sitting height divided by stature" );

    m_ShowSkelFlag.Init( "ShowSkelFlag", "Anthropometric", this, 0, 0, 1 );
    m_ShowSkelFlag.SetDescript( "Flag to show or hide the skeleton." );

    m_RLSymFlag.Init( "RLSym", "Pose", this, 1, 0, 1 );
    m_RLSymFlag.SetDescript( "Set left/right pose parameters equal." );

    m_ElbowRt.Init( "ElbowRt", "Pose", this, 15.0, 0, 150.0 );
    m_ElbowRt.SetDescript( "Right elbow angle" );

    m_ShoulderABADRt.Init( "ShoulderABADRt", "Pose", this, 0, -40, 170.0 );
    m_ShoulderABADRt.SetDescript( "Right shoulder AB/AD angle" );

    m_ShoulderFERt.Init( "ShoulderFERt", "Pose", this, 0, -30, 180.0 );
    m_ShoulderFERt.SetDescript( "Right shoulder FE angle" );

    m_ShoulderIERt.Init( "ShoulderIERt", "Pose", this, 0.0, -30, 100.0 );
    m_ShoulderIERt.SetDescript( "Right shoulder IE angle" );

    m_HipABADRt.Init( "HipABADRt", "Pose", this, 0.5, -45, 45.0 );
    m_HipABADRt.SetDescript( "Right hip AB/AD angle" );

    m_HipFERt.Init( "HipFERt", "Pose", this, 2.5, -15.0, 130.0 );
    m_HipFERt.SetDescript( "Right hip FE angle" );

    m_KneeRt.Init( "KneeRt", "Pose", this, 0.0, 0.0, 140.0 );
    m_KneeRt.SetDescript( "Right knee angle" );

    m_AnkleRt.Init( "AnkleRt", "Pose", this, 0.0, -15, 15.0 );
    m_AnkleRt.SetDescript( "Right ankle angle" );

    m_ElbowLt.Init( "ElbowLt", "Pose", this, 15.0, 0, 150.0 );
    m_ElbowLt.SetDescript( "Left elbow angle" );

    m_ShoulderABADLt.Init( "ShoulderABADLt", "Pose", this, 15.0, -40, 170.0 );
    m_ShoulderABADLt.SetDescript( "Left shoulder AB/AD angle" );

    m_ShoulderFELt.Init( "ShoulderFELt", "Pose", this, 1.0, -30, 180.0 );
    m_ShoulderFELt.SetDescript( "Left shoulder FE angle" );

    m_ShoulderIELt.Init( "ShoulderIELt", "Pose", this, 0.0, -30, 100.0 );
    m_ShoulderIELt.SetDescript( "Left shoulder IE angle" );

    m_HipABADLt.Init( "HipABADLt", "Pose", this, 0.5, -45, 45.0 );
    m_HipABADLt.SetDescript( "Left hip AB/AD angle" );

    m_HipFELt.Init( "HipFELt", "Pose", this, 2.5, -15.0, 130.0 );
    m_HipFELt.SetDescript( "Left hip FE angle" );

    m_KneeLt.Init( "KneeLt", "Pose", this, 0.0, 0.0, 140.0 );
    m_KneeLt.SetDescript( "Left knee angle" );

    m_AnkleLt.Init( "AnkleLt", "Pose", this, 0.0, -15, 15.0 );
    m_AnkleLt.SetDescript( "Left ankle angle" );

    m_Back.Init( "Back", "Pose", this, 0.0, -15, 45.0 );
    m_Back.SetDescript( "Back angle" );

    m_Waist.Init( "Waist", "Pose", this, 0.0, -15, 45.0 );
    m_Waist.SetDescript( "Waist angle" );

    m_PresetPose.Init( "PresetPose", "Pose", this, HumanGeom::STANDING, HumanGeom::STANDING, HumanGeom::SITTING );
    m_PresetPose.SetDescript( "Pose to set when triggered from GUI" );

    // Setup static member variables to be used by all HumanGeom
    if ( !m_MasterAttach )
    {
        SetupMesh( m_MasterMesh );

        Pinocchio::DataSkeleton skeleton;

        REAL_T androg_skel_verts[NUM_SKEL_VERT][3];
        ComputeAvePts( m_male_skel_verts, m_female_skel_verts, NUM_SKEL_VERT, androg_skel_verts );

        SetupSkel( androg_skel_verts, skeleton );

        m_MasterAttach = SetupAttach( m_MasterMesh, skeleton );

        // 20 & over.  From NHANES
        vector < double > cprob = {.05, .10, .15, .25, .50, .75, .85, .90, .95};
        vector < double > msta = {1634, 1662, 1680, 1706, 1756, 1808, 1837, 1854, 1881};
        vector < double > fsta = {1498, 1527, 1543, 1568, 1619, 1664, 1690, 1707, 1735};
        vector < double > mbmi = {20.7, 22.2, 23.0, 24.6, 27.7, 31.6, 34.0, 36.1, 39.8};
        vector < double > fbmi = {19.6, 21.0, 22.0, 23.6, 27.7, 33.2, 36.5, 39.3, 43.3};

        m_MaleStatureECDF.InterpolatePCHIP( msta, cprob, false );
        m_FemaleStatureECDF.InterpolatePCHIP( fsta, cprob, false );

        m_MaleBMIECDF.InterpolatePCHIP( mbmi, cprob, false );
        m_FemaleBMIECDF.InterpolatePCHIP( fbmi, cprob, false );

        // DebugDump();

    }

}

//==== Destructor ====//
HumanGeom::~HumanGeom()
{

}

void HumanGeom::DebugDump()
{

    m_MasterMesh.writeObj("debug.obj");

    //output attachment
    std::ofstream astrm("attachment.out");
    for(int i = 0; i < (int)m_MasterMesh.vertices.size(); ++i)
    {
        Vector<double, -1> v = m_MasterAttach->getWeights(i);
        for(int j = 0; j < v.size(); ++j)
        {
            double d = floor(0.5 + v[j] * 10000.) / 10000.;
            astrm << d << " ";
        }
        astrm << std::endl;
    }
}


void HumanGeom::ComputeScore( const REAL_T C[200][6], const vector < double > &X, vector < double > &score )
{
    const int m = 6;
    const int n = 200;

    for ( int i = 0; i < n; i++ )
    {
        score[i] = 0.0;

        for ( int j = 0; j < m; j++ )
        {
            score[i] += C[i][j] * X[j];
        }
    }
}

// Y = Ybar + P * ( m_coeffs * X );
void HumanGeom::ComputeResultsMesh( const REAL_T P[][200], const vector < double > &score, const REAL_T Ybar[][3], vector < vec3d > &Y )
{
    const int n = 200;
    int npt = NUM_MESH_VERT;

    for ( int i = 0; i < npt; i++ )
    {
        Y[i].set_arr( Ybar[i] );

        double yadd[3];
        for ( int k = 0; k < 3; k++ )
        {
            yadd[k] = 0;
            int l = i * 3 + k;

            for ( int j = 0; j < n; j++ )
            {
                yadd[k] += P[l][j] * score[j];
            }
        }
        Y[i] += yadd;
        Y[i+NUM_MESH_VERT].set_refy( Y[i] );
    }
}

// Y = Ybar + P * ( m_coeffs * X );
void HumanGeom::ComputeResultsSkel( const REAL_T P[][200], const vector < double > &score, const REAL_T Ybar[][3], vector < vec3d > &Y )
{
    const int n = 200;
    int npt = Y.size();

    for ( int i = 0; i < npt; i++ )
    {
        Y[i].set_arr( Ybar[i] );

        double yadd[3];
        for ( int k = 0; k < 3; k++ )
        {
            yadd[k] = 0;
            int l = i * 3 + k;

            for ( int j = 0; j < n; j++ )
            {
                yadd[k] += P[l][j] * score[j];
            }
        }
        Y[i] += yadd;
    }
}

void Mat2Trans( const Matrix4d &M, Pinocchio::Transform<> &T )
{
    double qw, qx, qy, qz, tx, ty, tz;

    M.toQuat( qw, qx, qy, qz, tx, ty, tz );

    T = Pinocchio::Transform<>( Pinocchio::Quaternion<>( qw, qx, qy, qz ), 1.0, Vector3( tx, ty, tz ) );
}

void HumanGeom::CopyVertsToSkel( const vector < vec3d > & sv )
{
    m_SkelVerts.resize( NUM_SKEL );

    for ( int i = 0; i < m_SkelVerts.size(); i++ )
    {
        int ind = m_skel_indx[i];

        if ( ind < 0 )
        {
            m_SkelVerts[i].set_refy( sv[-ind] );
        }
        else
        {
            m_SkelVerts[i] = sv[ind];
        }
    }
}

double HumanGeom::Get_mm2UX()
{
    double sf = 1.0;

    switch ( m_LenUnit() )
    {
        case vsp::LEN_MM:
            sf = 1.0;
            break;
        case vsp::LEN_CM:
            sf = 0.1;
            break;
        case vsp::LEN_M:
            sf = 0.001;
            break;
        case vsp::LEN_IN:
            sf = 1.0/25.4;
            break;
        case vsp::LEN_FT:
            sf = 1.0/(25.4*12.0);
            break;
        case vsp::LEN_YD:
            sf = 1.0/(25.4*12.0*3.0);
            break;
    }
    return sf;
}

void HumanGeom::ValidateParms( )
{
    if ( m_RLSymFlag() )
    {
        m_ElbowLt = m_ElbowRt();
        m_ShoulderABADLt = m_ShoulderABADRt();
        m_ShoulderFELt = m_ShoulderFERt();
        m_ShoulderIELt = m_ShoulderIERt();

        m_HipABADLt = m_HipABADRt();
        m_HipFELt = m_HipFERt();
        m_KneeLt = m_KneeRt();
        m_AnkleLt = m_AnkleRt();
    }

    if ( UpdatedParm( m_LenUnit.GetID() ) )
    {
        m_Stature.SetLowerLimit( 0.0 );
        m_Stature.SetUpperLimit( 1e5 );

        double sf = Get_mm2UX();

        if ( m_GenderFlag() == vsp::MALE )
        {
            m_Stature = m_MaleStatureECDF.CompPnt( m_Stature_pct() ) * sf;

            m_Stature.SetLowerLimit( 1634 * sf );
            m_Stature.SetUpperLimit( 1881 * sf );
        }
        else
        {
            m_Stature = m_FemaleStatureECDF.CompPnt( m_Stature_pct() ) * sf;

            m_Stature.SetLowerLimit( 1498 * sf );
            m_Stature.SetUpperLimit( 1735 * sf );
        }
    }

    if( UpdatedParm( m_Stature.GetID() ) )
    {
        double p;
        if ( m_GenderFlag() == vsp::MALE )
        {
            m_MaleStatureECDF.FindNearest( p, m_Stature() / Get_mm2UX() );
        }
        else
        {
            m_FemaleStatureECDF.FindNearest( p, m_Stature() / Get_mm2UX() );
        }
        m_Stature_pct = p;
    }
    else
    {
        if ( m_GenderFlag() == vsp::MALE )
        {
            m_Stature = m_MaleStatureECDF.CompPnt( m_Stature_pct() ) * Get_mm2UX();
        }
        else
        {
            m_Stature = m_FemaleStatureECDF.CompPnt( m_Stature_pct() ) * Get_mm2UX();
        }
    }

    if( UpdatedParm( m_Mass.GetID() ) )
    {
        double sta_m = 0.001 * m_Stature() / Get_mm2UX();
        m_BMI = ConvertMass( m_Mass(), m_MassUnit(), vsp::MASS_UNIT_KG ) / ( sta_m * sta_m );

        double p;
        if ( m_GenderFlag() == vsp::MALE )
        {
            m_MaleBMIECDF.FindNearest( p, m_BMI() );
        }
        else
        {
            m_FemaleBMIECDF.FindNearest( p, m_BMI() );
        }
        m_BMI_pct = p;
    }
    else if( UpdatedParm( m_BMI.GetID() ) )
    {
        double sta_m = 0.001 * m_Stature() / Get_mm2UX();
        m_Mass = ConvertMass( m_BMI() * sta_m * sta_m, vsp::MASS_UNIT_KG, m_MassUnit() );

        double p;
        if ( m_GenderFlag() == vsp::MALE )
        {
            m_MaleBMIECDF.FindNearest( p, m_BMI() );
        }
        else
        {
            m_FemaleBMIECDF.FindNearest( p, m_BMI() );
        }
        m_BMI_pct = p;
    }
    else
    {
        if ( m_GenderFlag() == vsp::MALE )
        {
            m_BMI = m_MaleBMIECDF.CompPnt( m_BMI_pct() );
        }
        else
        {
            m_BMI = m_FemaleBMIECDF.CompPnt( m_BMI_pct() );
        }
        double sta_m = 0.001 * m_Stature() / Get_mm2UX();
        m_Mass = ConvertMass( m_BMI() * sta_m * sta_m, vsp::MASS_UNIT_KG, m_MassUnit() );
    }
}

void HumanGeom::SetPreset()
{
    int p = m_PresetPose();

    switch( p )
    {
        case STANDING:
            m_RLSymFlag = true;
            m_Back = 0;
            m_Waist = 0;

            m_ElbowRt = 15;
            m_ShoulderABADRt = 0;
            m_ShoulderFERt = 0;
            m_ShoulderIERt = 0;

            m_HipABADRt = 0.5;
            m_HipFERt = 2.5;
            m_KneeRt = 0.0;
            m_AnkleRt = 0.0;

            break;
        case SITTING:
            m_RLSymFlag = true;
            m_Back = 0;
            m_Waist = 0;

            m_ElbowRt = 80;
            m_ShoulderABADRt = 0;
            m_ShoulderFERt = 0;
            m_ShoulderIERt = 45;

            m_HipABADRt = 0.5;
            m_HipFERt = 80;
            m_KneeRt = 80;
            m_AnkleRt = 0.0;
            break;
    }
}

void HumanGeom::UpdateSurf()
{
    ValidateParms();

    // Compute score vector used in anthropometric calculations
    double stamm = m_Stature() / Get_mm2UX();

    vector < double > vars(6);
    vars[0] = stamm;
    vars[1] = m_BMI();
    vars[2] = m_SitFrac();
    vars[3] = m_Age();
    vars[4] = m_BMI() * m_Age();
    vars[5] = 1.0;

    vector < double > score( 200, 0.0 );

    if ( m_GenderFlag() == vsp::MALE )
    {
        ComputeScore( m_male_coeffs, vars, score );
    }
    else
    {
        ComputeScore( m_female_coeffs, vars, score );
    }

    // Compute anthropometric skeleton.
    vector < vec3d > sv( NUM_SKEL_VERT );

    if ( m_GenderFlag() == vsp::MALE )
    {
        ComputeResultsSkel( m_male_skel_pcs, score, m_male_skel_verts, sv );
    }
    else
    {
        ComputeResultsSkel( m_female_skel_pcs, score, m_female_skel_verts, sv );
    }
    CopyVertsToSkel( sv );


    // Compute pose transformations.
    Matrix4d t_rtforearm, t_rtbicep, t_rtfoot, t_rtshin, t_rtthigh, t_back, t_waist;
    Matrix4d t_ltforearm, t_ltbicep, t_ltfoot, t_ltshin, t_ltthigh;

    ComputeShoulderTrans( RSHOULDER, RELBOW, m_ShoulderABADRt(), m_ShoulderFERt(), t_rtbicep );

    ComputeElbowTrans( RSHOULDER, RELBOW, RHAND, m_ElbowRt() * M_PI / 180.0, m_ShoulderIERt() * M_PI / 180.0, t_rtforearm );
    t_rtforearm.postMult( t_rtbicep );


    ComputeShoulderTrans( LSHOULDER, LELBOW, -m_ShoulderABADLt(), m_ShoulderFELt(), t_ltbicep );

    ComputeElbowTrans( LSHOULDER, LELBOW, LHAND, m_ElbowLt() * M_PI / 180.0, -m_ShoulderIELt() * M_PI / 180.0, t_ltforearm );
    t_ltforearm.postMult( t_ltbicep );


    ComputeBackTrans( ORIGIN, BACK, WAIST, m_Back() * M_PI / 180.0, t_back );

    ComputeWaistTrans( BACK, WAIST, RHIP, m_Waist() * M_PI / 180.0, t_waist );
    t_waist.postMult( t_back );

    ComputeHipTrans( WAIST, RHIP, RKNEE, m_HipABADRt(), m_HipFERt(), t_rtthigh );
    t_rtthigh.postMult( t_waist );

    ComputeKneeTrans( RHIP, RKNEE, RANKLE, m_KneeRt() * M_PI / 180.0, t_rtshin );
    t_rtshin.postMult( t_rtthigh );

    ComputeAnkleTrans( RKNEE, RANKLE, RTOE, m_AnkleRt() * M_PI / 180.0, t_rtfoot );
    t_rtfoot.postMult( t_rtshin );


    ComputeHipTrans( WAIST, LHIP, LKNEE, -m_HipABADLt(), m_HipFELt(), t_ltthigh );
    t_ltthigh.postMult( t_waist );

    ComputeKneeTrans( LHIP, LKNEE, LANKLE, m_KneeLt() * M_PI / 180.0, t_ltshin );
    t_ltshin.postMult( t_ltthigh );

    ComputeAnkleTrans( LKNEE, LANKLE, LTOE, m_AnkleLt() * M_PI / 180.0, t_ltfoot );
    t_ltfoot.postMult( t_ltshin );


    // Pose skeleton.
    m_PoseSkelVerts = m_SkelVerts;

    m_PoseSkelVerts[WAIST] = t_back.xform( m_PoseSkelVerts[WAIST] );

    m_PoseSkelVerts[RHIP] = t_waist.xform( m_PoseSkelVerts[RHIP] );
    m_PoseSkelVerts[LHIP] = t_waist.xform( m_PoseSkelVerts[LHIP] );

    m_PoseSkelVerts[RELBOW] = t_rtbicep.xform( m_PoseSkelVerts[RELBOW] );
    m_PoseSkelVerts[RHAND] = t_rtforearm.xform( m_PoseSkelVerts[RHAND] );

    m_PoseSkelVerts[LELBOW] = t_ltbicep.xform( m_PoseSkelVerts[LELBOW] );
    m_PoseSkelVerts[LHAND] = t_ltforearm.xform( m_PoseSkelVerts[LHAND] );

    m_PoseSkelVerts[RKNEE] = t_rtthigh.xform( m_PoseSkelVerts[RKNEE] );
    m_PoseSkelVerts[RANKLE] = t_rtshin.xform( m_PoseSkelVerts[RANKLE] );
    m_PoseSkelVerts[RTOE] = t_rtfoot.xform( m_PoseSkelVerts[RTOE] );

    m_PoseSkelVerts[LKNEE] = t_ltthigh.xform( m_PoseSkelVerts[LKNEE] );
    m_PoseSkelVerts[LANKLE] = t_ltshin.xform( m_PoseSkelVerts[LANKLE] );
    m_PoseSkelVerts[LTOE] = t_ltfoot.xform( m_PoseSkelVerts[LTOE] );


    // Transfer transformations to Pinocchio vector form.
    unsigned int nbones = m_SkelVerts.size();
    std::vector< Pinocchio::Transform<> > trs( nbones );

    Mat2Trans( t_waist, trs[LOWSPINE] );

    Mat2Trans( t_waist, trs[LPELVIS] );
    Mat2Trans( t_waist, trs[RPELVIS] );

    Mat2Trans( t_rtforearm, trs[RFOREARM] );
    Mat2Trans( t_rtbicep, trs[RBICEP] );

    Mat2Trans( t_ltforearm, trs[LFOREARM] );
    Mat2Trans( t_ltbicep, trs[LBICEP] );

    Mat2Trans( t_rtfoot, trs[RFOOT] );
    Mat2Trans( t_rtshin, trs[RSHIN] );
    Mat2Trans( t_rtthigh, trs[RTHIGH] );

    Mat2Trans( t_ltfoot, trs[LFOOT] );
    Mat2Trans( t_ltshin, trs[LSHIN] );
    Mat2Trans( t_ltthigh, trs[LTHIGH] );



    // Process main geometry
    m_MainVerts.clear();
    m_MainVerts.resize( NUM_MESH_VERT * 2 );

    // Anthropometric calculation of base mesh
    if ( m_GenderFlag() == vsp::MALE )
    {
        ComputeResultsMesh( m_male_half_pcs, score, m_male_half_verts, m_MainVerts );
    }
    else
    {
        ComputeResultsMesh( m_female_half_pcs, score, m_female_half_verts, m_MainVerts );
    }

    Pinocchio::Mesh m_LocalMesh = m_MasterMesh;

    CopyVertsToMesh( m_MainVerts, m_LocalMesh );

    // Deform for pose
    Pinocchio::Mesh newmesh = m_MasterAttach->deform( m_LocalMesh, trs );

    CopyMeshToVerts( newmesh, m_MainVerts );

    // Get scale for units.
    double sf = Get_mm2UX();

    // Calculate volume of posed figure
    m_Volume = CalculateVolume() * sf * sf * sf;

    if ( m_AutoDensity() )
    {
        m_Density = m_Mass() / m_Volume();
    }

    // Final positioning & scale -- hold waist fixed & origin
    Matrix4d tcenter;
    tcenter.translatev( -m_PoseSkelVerts[WAIST] );

    Matrix4d sc;
    sc.scale( sf );
    tcenter.postMult( sc );

    tcenter.xformvec( m_SkelVerts );

    Matrix4d tfinal = t_waist;
    tfinal.affineInverse();
    tcenter.matMult( tfinal );

    tcenter.xformvec( m_MainVerts );
    tcenter.xformvec( m_PoseSkelVerts );


/*
    for ( int i = 0; i < nbones; i++ )
    {
        Pinocchio::Quaternion<> q = trs[i].getRot();

        printf( "%d ", i );

        for ( int j = 0; j < 4; j++ )
        {
            printf( "q_%d %f ", j, q[j] );
        }

        Vector3 v = trs[i].getTrans();

        for ( int j = 0; j < 3; j++ )
        {
            printf( "t_%d %f ", j, v[j] );
        }

        printf( "s %f\n", trs[i].getScale() );
    }
*/
}

double HumanGeom::CalculateVolume()
{
    int num_tris = NUM_MESH_TRI;

    double vol = 0;

    for ( int t = 0 ; t < ( int ) num_tris ; t++ )
    {
        vec3d p0 = m_MainVerts[m_half_tris[t][0]];
        vec3d p1 = m_MainVerts[m_half_tris[t][1]];
        vec3d p2 = m_MainVerts[m_half_tris[t][2]];

        vol += tetra_volume( p0, p1, p2 );
    }

    for ( int t = 0 ; t < ( int ) num_tris ; t++ )
    {
        vec3d p0 = m_MainVerts[m_half_tris[t][0] + NUM_MESH_VERT];
        vec3d p1 = m_MainVerts[m_half_tris[t][2] + NUM_MESH_VERT];
        vec3d p2 = m_MainVerts[m_half_tris[t][1] + NUM_MESH_VERT];

        vol += tetra_volume( p0, p1, p2 );
    }

    return vol;
}

void HumanGeom::UpdateBBox()
{
    m_BBox.Reset();

    for ( int j = 0 ; j < ( int )m_Verts.size() ; j++ )
    {
        for ( int i = 0; i < ( int )m_Verts[j].size(); i++ )
        {
            m_BBox.Update( m_Verts[j][i] );
        }
    }
}

// This is substantially similar to Geom::UpdateSymmAttach() and could probably be combined in a meaningful way.
void HumanGeom::UpdateSymmAttach()
{
    unsigned int num_surf = GetNumTotalMeshs();
    m_Verts.clear();
    m_FlipNormal.clear();

    m_SurfIndxVec.clear();
    m_SurfSymmMap.clear();
    m_SurfCopyIndx.clear();

    m_Verts.resize( num_surf );
    m_FlipNormal.resize( num_surf, false );

    m_SurfIndxVec.resize( num_surf, -1 );
    m_SurfSymmMap.resize( num_surf );
    m_SurfCopyIndx.resize( num_surf );

    int num_main = GetNumMainMeshs();                 // Currently hard-coded to 1.  Some of below is over-complex for this case.
    for ( int i = 0 ; i < ( int )num_main ; i++ )
    {
        m_Verts[i] = m_MainVerts;
        m_FlipNormal[i] = false;                      // Assume main mesh is properly oriented.
        m_SurfIndxVec[i] = i;
        m_SurfSymmMap[ m_SurfIndxVec[i] ].push_back( i );
        m_SurfCopyIndx[i] = 0;
    }

    m_TransMatVec.resize( num_surf, Matrix4d() );
    // Compute Relative Translation Matrix
    Matrix4d symmOriginMat;
    Matrix4d relTrans;
    if ( m_SymAncestOriginFlag() )
    {
        symmOriginMat = GetAncestorAttachMatrix( m_SymAncestor() - 1 );
    }
    else
    {
        symmOriginMat = GetAncestorModelMatrix( m_SymAncestor() - 1 );
    }
    relTrans = symmOriginMat;
    relTrans.affineInverse();
    relTrans.matMult( m_ModelMatrix.data() );

    for ( int i = 0 ; i < ( int )m_TransMatVec.size() ; i++ )
    {
        m_TransMatVec[i].initMat( relTrans.data() );
    }

    // Copy main surfs
    int symFlag = GetSymFlag();
    if ( symFlag != 0 )
    {
        int numShifts = -1;
        Matrix4d Ref; // Reflection Matrix
        Matrix4d Ref_Orig; // Original Reflection Matrix
        Matrix4d Rel; // Relative Transformation matrix with Reflection applied ( this is for the main surfaces )

        double angle = ( 360 ) / ( double )m_SymRotN();
        int currentIndex = num_main;
        bool radial = false;

        for ( int i = 0 ; i < GetNumSymFlags() ; i ++ ) // Loop through each of the set sym flags
        {
            // Find next set sym flag
            while ( true )
            {
                numShifts++;
                if ( ( ( symFlag >> numShifts ) & ( 1 << 0 ) ) || numShifts > vsp::SYM_NUM_TYPES )
                {
                    break;
                }
            }

            // Create Reflection Matrix
            if ( ( 1 << numShifts ) == vsp::SYM_XY )
            {
                Ref.loadXYRef();
            }
            else if ( ( 1 << numShifts ) == vsp::SYM_XZ )
            {
                Ref.loadXZRef();
            }
            else if ( ( 1 << numShifts ) == vsp::SYM_YZ )
            {
                Ref.loadYZRef();
            }
            else if ( ( 1 << numShifts ) == vsp::SYM_ROT_X )
            {
                Ref.loadIdentity();
                Ref.rotateX( angle );
                Ref_Orig = Ref;
                radial = true;
            }
            else if ( ( 1 << numShifts ) == vsp::SYM_ROT_Y )
            {
                Ref.loadIdentity();
                Ref.rotateY( angle );
                Ref_Orig = Ref;
                radial = true;
            }
            else if ( ( 1 << numShifts ) == vsp::SYM_ROT_Z )
            {
                Ref.loadIdentity();
                Ref.rotateZ( angle );
                Ref_Orig = Ref;
                radial = true;
            }

            // number of additional surfaces for a single reflection ( for rotational reflections it is m_SymRotN-1 times this number
            int numAddSurfs = currentIndex;
            int addIndex = 0;

            for ( int j = currentIndex ; j < currentIndex + numAddSurfs ; j++ )
            {
                if ( radial ) // rotational reflection
                {
                    for ( int k = 0 ; k < m_SymRotN() - 1 ; k++ )
                    {
                        m_Verts[j + k * numAddSurfs] = m_Verts[j - currentIndex];

                        m_SurfIndxVec[j + k * numAddSurfs] = m_SurfIndxVec[j - currentIndex];
                        m_SurfCopyIndx[j + k * numAddSurfs] = m_SurfSymmMap[ m_SurfIndxVec[j + k * numAddSurfs] ].size();
                        m_SurfSymmMap[ m_SurfIndxVec[j + k * numAddSurfs] ].push_back( j + k * numAddSurfs );
                        m_TransMatVec[j + k * numAddSurfs].initMat( m_TransMatVec[j - currentIndex].data() );
                        m_TransMatVec[j + k * numAddSurfs].postMult( Ref.data() ); // Apply Reflection

                        // Increment rotation by the angle
                        Ref.postMult( Ref_Orig.data() );
                        addIndex++;
                    }
                    // Reset reflection matrices to the beginning angle
                    Ref = Ref_Orig;
                }
                else
                {
                    m_Verts[j] = m_Verts[j - currentIndex];
                    m_FlipNormal[j] = !m_FlipNormal[j - currentIndex];

                    m_SurfIndxVec[j] = m_SurfIndxVec[j - currentIndex];
                    m_SurfCopyIndx[j] = m_SurfSymmMap[ m_SurfIndxVec[j] ].size();
                    m_SurfSymmMap[ m_SurfIndxVec[ j ] ].push_back( j );
                    m_TransMatVec[j].initMat( m_TransMatVec[j - currentIndex].data() );
                    m_TransMatVec[j].postMult( Ref.data() ); // Apply Reflection
                    addIndex++;
                }
            }

            currentIndex += addIndex;
            radial = false;
        }
    }

    Matrix4d retrun_relTrans = relTrans;
    retrun_relTrans.affineInverse();

    m_FeaTransMatVec.clear();
    m_FeaTransMatVec.resize( num_surf );

    //==== Save Transformation Matrix and Apply Transformations ====//
    for ( int i = 0 ; i < num_surf ; i++ )
    {
        m_TransMatVec[i].postMult( symmOriginMat.data() );
        m_TransMatVec[i].xformvec( m_Verts[i] );     // Apply total transformation to meshes
        m_FeaTransMatVec[i] = m_TransMatVec[i];
        m_FeaTransMatVec[i].matMult( retrun_relTrans.data() ); // m_FeaTransMatVec does not inclde the relTrans matrix
    }

}

void HumanGeom::UpdateDrawObj()
{
    // Add in SubSurfaces to TMeshVec if m_DrawSubSurfs is true
    int num_meshes = m_Verts.size();

    // Mesh Should Be Flat Before Calling this Method
    int add_ind = 0;

    m_WireShadeDrawObj_vec.resize( 1, DrawObj() );

    vec3d zeroV = m_ModelMatrix.xform( vec3d( 0.0, 0.0, 0.0 ) );
    unsigned int pi = 0;
    unsigned int ido = 0;
    unsigned int isize = 0;

    for ( int m = 0 ; m < ( int )num_meshes ; m++ )
    {
        int num_tris = NUM_MESH_TRI;

        isize = isize + num_tris * 3 * 2;
        m_WireShadeDrawObj_vec[ido].m_PntVec.resize( isize );
        m_WireShadeDrawObj_vec[ido].m_NormVec.resize( isize );
        for ( int t = 0 ; t < ( int ) num_tris ; t++ )
        {
            int i2 = 1;
            int i3 = 2;

            if( m_FlipNormal[m] )
            {
                i2 = 2;
                i3 = 1;
            }

            vec3d p0 = m_Verts[m][m_half_tris[t][0]];
            vec3d p1 = m_Verts[m][m_half_tris[t][i2]];
            vec3d p2 = m_Verts[m][m_half_tris[t][i3]];

            vec3d v0 (0, 0, 0);
            if ( dist (p0, v0) == 0 )
            {
                printf("Found zero vert %d\n", m_half_tris[t][0] );
            }
            if ( dist (p1, v0) == 0 )
            {
                printf("Found zero vert %d\n", m_half_tris[t][i2] );
            }
            if ( dist (p2, v0) == 0 )
            {
                printf("Found zero vert %d\n", m_half_tris[t][i3] );
            }

            //==== Compute Normal ====//
            vec3d p10 = p1 - p0;
            vec3d p20 = p2 - p0;
            vec3d norm = cross( p10, p20 );
            norm.normalize();

            m_WireShadeDrawObj_vec[ido].m_PntVec[pi] = p0 ;
            m_WireShadeDrawObj_vec[ido].m_PntVec[pi + 1] = p1 ;
            m_WireShadeDrawObj_vec[ido].m_PntVec[pi + 2] = p2 ;

            m_WireShadeDrawObj_vec[ido].m_NormVec[pi] = norm;
            m_WireShadeDrawObj_vec[ido].m_NormVec[pi + 1] = norm;
            m_WireShadeDrawObj_vec[ido].m_NormVec[pi + 2] = norm;
            pi += 3;
        }

        for ( int t = 0 ; t < ( int ) num_tris ; t++ )
        {
            int i2 = 2;
            int i3 = 1;

            if( m_FlipNormal[m] )
            {
                i2 = 1;
                i3 = 2;
            }

            vec3d p0 = m_Verts[m][m_half_tris[t][0]+NUM_MESH_VERT];
            vec3d p1 = m_Verts[m][m_half_tris[t][i2]+NUM_MESH_VERT];
            vec3d p2 = m_Verts[m][m_half_tris[t][i3]+NUM_MESH_VERT];

            vec3d v0 (0, 0, 0);
            if ( dist (p0, v0) == 0 )
            {
                printf("Found zero vert %d\n", m_half_tris[t][0] );
            }
            if ( dist (p1, v0) == 0 )
            {
                printf("Found zero vert %d\n", m_half_tris[t][i2] );
            }
            if ( dist (p2, v0) == 0 )
            {
                printf("Found zero vert %d\n", m_half_tris[t][i3] );
            }

            //==== Compute Normal ====//
            vec3d p10 = p1 - p0;
            vec3d p20 = p2 - p0;
            vec3d norm = cross( p10, p20 );
            norm.normalize();

            m_WireShadeDrawObj_vec[ido].m_PntVec[pi] = p0 ;
            m_WireShadeDrawObj_vec[ido].m_PntVec[pi + 1] = p1 ;
            m_WireShadeDrawObj_vec[ido].m_PntVec[pi + 2] = p2 ;

            m_WireShadeDrawObj_vec[ido].m_NormVec[pi] = norm;
            m_WireShadeDrawObj_vec[ido].m_NormVec[pi + 1] = norm;
            m_WireShadeDrawObj_vec[ido].m_NormVec[pi + 2] = norm;
            pi += 3;
        }
    }

    //==== Bounding Box ====//
    m_HighlightDrawObj.m_PntVec = m_BBox.GetBBoxDrawLines();


    // Flag the DrawObjects as changed
    for ( int i = 0 ; i < ( int )m_WireShadeDrawObj_vec.size(); i++ )
    {
        m_WireShadeDrawObj_vec[i].m_GeomChanged = true;
    }

    m_FeatureDrawObj_vec.clear();
    m_FeatureDrawObj_vec.resize(2);
    m_FeatureDrawObj_vec[0].m_GeomChanged = true;
    m_FeatureDrawObj_vec[0].m_LineWidth = 3.0;
    m_FeatureDrawObj_vec[0].m_LineColor = vec3d( 0.0, 0.0, 0.0 );
    m_FeatureDrawObj_vec[1].m_GeomChanged = true;
    m_FeatureDrawObj_vec[1].m_LineWidth = 3.0;
    m_FeatureDrawObj_vec[1].m_LineColor = vec3d( 0.0, 0.0, 1.0 );

    if( m_GuiDraw.GetDispFeatureFlag() )
    {
        m_FeatureDrawObj_vec[0].m_PntVec.resize( ( NUM_SKEL - 1 ) * 2 );
        m_FeatureDrawObj_vec[1].m_PntVec.resize( ( NUM_SKEL - 1 ) * 2 );

        const int prevarr[] = {-1, 0, 1, 0, 2, 4, 5, 6, 2, 8, 9, 10, 0, 12, 13, 0, 15, 16};

        for ( int i = 1; i < NUM_SKEL; i++ )
        {
            int iprev = prevarr[i];

            m_FeatureDrawObj_vec[0].m_PntVec[ (i - 1) * 2 ] = m_SkelVerts[iprev];
            m_FeatureDrawObj_vec[0].m_PntVec[ (i - 1) * 2 + 1 ] = m_SkelVerts[i];

            m_FeatureDrawObj_vec[1].m_PntVec[ (i - 1) * 2 ] = m_PoseSkelVerts[iprev];
            m_FeatureDrawObj_vec[1].m_PntVec[ (i - 1) * 2 + 1 ] = m_PoseSkelVerts[i];
        }
    }

    //=== Axis ===//
    m_AxisDrawObj_vec.clear();
    m_AxisDrawObj_vec.resize( 3 );
    for ( int i = 0; i < 3; i++ )
    {
        MakeDashedLine( m_AttachOrigin,  m_AttachAxis[i], 4, m_AxisDrawObj_vec[i].m_PntVec );
        vec3d c;
        c.v[i] = 1.0;
        m_AxisDrawObj_vec[i].m_LineColor = c;
        m_AxisDrawObj_vec[i].m_GeomChanged = true;
    }
}

void HumanGeom::LoadDrawObjs( vector< DrawObj* > & draw_obj_vec )
{

    char str[256];

    for ( int i = 0 ; i < ( int )m_WireShadeDrawObj_vec.size() ; i++ )
    {
        // Symmetry drawObjs have same m_ID. Make them unique by adding index
        // at the end of m_ID.
        sprintf( str, "_%d", i );
        m_WireShadeDrawObj_vec[i].m_GeomID = m_ID + str;
        m_WireShadeDrawObj_vec[i].m_Visible = GetSetFlag( vsp::SET_SHOWN );

        // Set Render Destination to Main VSP Window.
        m_WireShadeDrawObj_vec[i].m_Screen = DrawObj::VSP_MAIN_SCREEN;

        Material * material = m_GuiDraw.getMaterial();

        for ( int j = 0; j < 4; j++ )
            m_WireShadeDrawObj_vec[i].m_MaterialInfo.Ambient[j] = (float)material->m_Ambi[j];

        for ( int j = 0; j < 4; j++ )
            m_WireShadeDrawObj_vec[i].m_MaterialInfo.Diffuse[j] = (float)material->m_Diff[j];

        for ( int j = 0; j < 4; j++ )
            m_WireShadeDrawObj_vec[i].m_MaterialInfo.Specular[j] = (float)material->m_Spec[j];

        for ( int j = 0; j < 4; j++ )
            m_WireShadeDrawObj_vec[i].m_MaterialInfo.Emission[j] = (float)material->m_Emis[j];

        m_WireShadeDrawObj_vec[i].m_MaterialInfo.Shininess = (float)material->m_Shininess;


        vec3d lineColor = vec3d( m_GuiDraw.GetWireColor().x() / 255.0,
                                 m_GuiDraw.GetWireColor().y() / 255.0,
                                 m_GuiDraw.GetWireColor().z() / 255.0 );

        m_WireShadeDrawObj_vec[i].m_LineWidth = 1.0;
        m_WireShadeDrawObj_vec[i].m_LineColor = lineColor;


        switch ( m_GuiDraw.GetDrawType() )
        {
            case vsp::DRAW_TYPE::GEOM_DRAW_WIRE:
                m_WireShadeDrawObj_vec[i].m_Type = DrawObj::VSP_WIRE_TRIS;
                break;

            case vsp::DRAW_TYPE::GEOM_DRAW_HIDDEN:
                m_WireShadeDrawObj_vec[i].m_Type = DrawObj::VSP_HIDDEN_TRIS;
                break;

            case vsp::DRAW_TYPE::GEOM_DRAW_SHADE:
                m_WireShadeDrawObj_vec[i].m_Type = DrawObj::VSP_SHADED_TRIS;
                break;

            case vsp::DRAW_TYPE::GEOM_DRAW_NONE:
                m_WireShadeDrawObj_vec[i].m_Type = DrawObj::VSP_SHADED_TRIS;
                m_WireShadeDrawObj_vec[i].m_Visible = false;
                break;

            case vsp::DRAW_TYPE::GEOM_DRAW_TEXTURE:
                m_WireShadeDrawObj_vec[i].m_Type = DrawObj::VSP_SHADED_TRIS;
                break;
        }

        draw_obj_vec.push_back( &m_WireShadeDrawObj_vec[i] );

    }

    // Load Feature Lines
    if ( m_GuiDraw.GetDispFeatureFlag() && GetSetFlag( vsp::SET_SHOWN ) && m_ShowSkelFlag() )
    {
        for ( int i = 0; i < m_FeatureDrawObj_vec.size(); i++ )
        {
            m_FeatureDrawObj_vec[i].m_Screen = DrawObj::VSP_MAIN_SCREEN;
            sprintf( str, "_%d", i );
            m_FeatureDrawObj_vec[i].m_GeomID = m_ID + "Feature_" + str;
            m_FeatureDrawObj_vec[i].m_Type = DrawObj::VSP_LINES;
            draw_obj_vec.push_back( &m_FeatureDrawObj_vec[i] );
        }
    }

    // Load BoundingBox and Axes
    if ( m_Vehicle->IsGeomActive( m_ID ) )
    {
        m_HighlightDrawObj.m_Screen = DrawObj::VSP_MAIN_SCREEN;
        m_HighlightDrawObj.m_GeomID = BBOXHEADER + m_ID;
        m_HighlightDrawObj.m_LineWidth = 2.0;
        m_HighlightDrawObj.m_LineColor = vec3d( 1.0, 0., 0.0 );
        m_HighlightDrawObj.m_Type = DrawObj::VSP_LINES;
        draw_obj_vec.push_back( &m_HighlightDrawObj );

        for ( int i = 0; i < m_AxisDrawObj_vec.size(); i++ )
        {
            m_AxisDrawObj_vec[i].m_Screen = DrawObj::VSP_MAIN_SCREEN;
            sprintf( str, "_%d", i );
            m_AxisDrawObj_vec[i].m_GeomID = m_ID + "Axis_" + str;
            m_AxisDrawObj_vec[i].m_LineWidth = 2.0;
            m_AxisDrawObj_vec[i].m_Type = DrawObj::VSP_LINES;
            draw_obj_vec.push_back( &m_AxisDrawObj_vec[i] );
        }
    }

}

//==== Count Number of Sym Surfaces ====//
int HumanGeom::GetNumTotalMeshs() const
{
    return GetNumSymmCopies() * GetNumMainMeshs();
}

void HumanGeom::Scale()
{
    double currentScale = m_Scale() / m_LastScale();
    m_LastScale = m_Scale();
}

void HumanGeom::ApplyScale()
{
}

void HumanGeom::CreateDegenGeom( vector<DegenGeom> &dgs, bool preview )
{
    unsigned int num_meshes = GetNumTotalMeshs();

    for ( int i = 0; i < num_meshes; i++ )
    {
        DegenGeom degenGeom;

        degenGeom.setType( DegenGeom::MESH_TYPE );

        degenGeom.setParentGeom( this );
        degenGeom.setSurfNum( i );
        degenGeom.setFlipNormal( m_FlipNormal[i] );
        degenGeom.setMainSurfInd( m_SurfIndxVec[i] );
        degenGeom.setSymCopyInd( m_SurfCopyIndx[i] );

        vector < double > tmatvec;
        for ( int j = 0; j < 16; j++ )
        {
            tmatvec.push_back( m_TransMatVec[i].data()[ j ] );
        }
        degenGeom.setTransMat( tmatvec );

        degenGeom.setNumXSecs( 0 );
        degenGeom.setNumPnts( 0 );
        degenGeom.setName( GetName() );

        dgs.push_back(degenGeom);
    }
}

vector<TMesh*> HumanGeom::CreateTMeshVec() const
{
    vector<TMesh*> retTMeshVec;

    for ( int j = 0; j < m_Verts.size(); j++ )
    {
        int num_tris;
        num_tris = NUM_MESH_TRI;

        TMesh*  tMesh = new TMesh();

        tMesh->LoadGeomAttributes( this );

        tMesh->m_SurfNum = j;

        for ( int i = 0 ; i < num_tris ; i++ )
        {
            int i2 = 1;
            int i3 = 2;

            if ( m_FlipNormal[j] )
            {
                i2 = 2;
                i3 = 1;
            }

            vec3d p0 = vec3d( m_Verts[j][m_half_tris[i][0]] );
            vec3d p1 = vec3d( m_Verts[j][m_half_tris[i][i2]] );
            vec3d p2 = vec3d( m_Verts[j][m_half_tris[i][i3]] );

            //==== Compute Normal ====//
            vec3d p10 = p1 - p0;
            vec3d p20 = p2 - p0;
            vec3d norm = cross( p10, p20 );
            norm.normalize();

            //==== Add Valid Facet ====//
            tMesh->AddTri( p0, p1, p2, norm );
        }

        for ( int i = 0 ; i < num_tris ; i++ )
        {
            int i2 = 2;
            int i3 = 1;

            if ( m_FlipNormal[j] )
            {
                i2 = 1;
                i3 = 2;
            }

            vec3d p0 = vec3d( m_Verts[j][m_half_tris[i][0] + NUM_MESH_VERT] );
            vec3d p1 = vec3d( m_Verts[j][m_half_tris[i][i2] + NUM_MESH_VERT] );
            vec3d p2 = vec3d( m_Verts[j][m_half_tris[i][i3] + NUM_MESH_VERT] );

            //==== Compute Normal ====//
            vec3d p10 = p1 - p0;
            vec3d p20 = p2 - p0;
            vec3d norm = cross( p10, p20 );
            norm.normalize();

            //==== Add Valid Facet ====//
            tMesh->AddTri( p0, p1, p2, norm );
        }

        retTMeshVec.push_back( tMesh );
    }

    return retTMeshVec;
}

void HumanGeom::SetupMesh( Pinocchio::Mesh &m )
{
    m.algo = Pinocchio::Mesh::DQS;
//    m.algo = Pinocchio::Mesh::LBS;
    m.blendWeight = 0.5;

    m.vertices.clear();
    m.vertices.resize( NUM_MESH_VERT * 2 );

    m_VertCopySet.clear();

    // Seed initial verts as positive copies
    for ( int i = 0; i < NUM_MESH_VERT; i++ )
    {
        m.vertices[ i ].origVertID = i;
    }

    // Seed initial verts as negative copies -- except on center line, then reference positive one.
    for ( int i = 0; i < NUM_MESH_VERT; i++ )
    {
        double y = m_male_half_verts[i][1];
        if ( y == -y )
        {
            m_VertCopySet.emplace_hint( m_VertCopySet.end(), i );
        }

        m.vertices[ i + NUM_MESH_VERT ].origVertID = i + NUM_MESH_VERT;
    }

    REAL_T androg_half_verts[NUM_MESH_VERT][3];
    ComputeAvePts( m_male_half_verts, m_female_half_verts, NUM_MESH_VERT, androg_half_verts );

    CopyVertsToMesh( androg_half_verts, m );


    m.edges.reserve( 3 * 2 * NUM_MESH_TRI );

    // Setup first half of tris as edges.
    for ( int i = 0; i < NUM_MESH_TRI; i++ )
    {
        int first = m.edges.size();
        m.edges.resize( m.edges.size() + 3 );
        for( int j = 0; j < 3; ++j )
        {
            m.edges[ first + j ].vertex = m_half_tris[i][j];
        }
    }

    // Setup second half of tris as edges.
    for ( int i = 0; i < NUM_MESH_TRI; i++ )
    {
        int first = m.edges.size();
        m.edges.resize( m.edges.size() + 3 );
        for( int j = 0; j < 3; ++j )
        {
            int k = 2 - j;    // Reverse direction

            int ivert = m_half_tris[i][k] + NUM_MESH_VERT;

            if ( m_VertCopySet.count( m_half_tris[i][k] ) )  // This vert is a duplicate.
            {
                ivert = m_half_tris[i][k];
            }

            m.edges[ first + j ].vertex = ivert;
        }
    }

    m.fixDupFaces(); // Also eliminates un-used verts.  Causes re-numbering of m.vertices

    m.computeTopology();

    m.computeVertexNormals();

    m.normalizeBoundingBox();
}

template < typename vertmat >
void HumanGeom::SetupSkel( const vertmat & vm, Pinocchio::DataSkeleton &skeleton )
{
    // m_SkelVerts
    // const int HumanGeom::m_skel_indx[NUM_SKEL] = {0, 1, 2, 3, -4, -5, -6, -7, 4, 5, 6, 7, -8, -9, -10, 8, 9, 10};

    const int prevarr[] = {-1, 0, 1, 0, 2, 4, 5, 6, 2, 8, 9, 10, 0, 12, 13, 0, 15, 16};

    std::vector < int > previd( NUM_SKEL );
    std::vector < Vector3 > skel_pts( NUM_SKEL );
    for ( int i = 0; i < NUM_SKEL; i++ )
    {
        int ivert = m_skel_indx[i];
        double yref = 1.0;
        if ( ivert < 0 )
        {
            ivert = -ivert;
            yref = -1.0;
        }

        skel_pts[i] = Vector3( vm[ivert][0], yref * vm[ivert][1], vm[ivert][2] );
        previd[i] = prevarr[i];
    }

    // Set up skeleton.
    skeleton.init( skel_pts, previd );


    // Symmetry
    skeleton.makeSymmetric( LHIP, RHIP );
    skeleton.makeSymmetric( LKNEE, RKNEE );
    skeleton.makeSymmetric( LANKLE, RANKLE );
    skeleton.makeSymmetric( LTOE, RTOE );

    skeleton.makeSymmetric( LSHOULDER, RSHOULDER );
    skeleton.makeSymmetric( LELBOW, RELBOW );
    skeleton.makeSymmetric( LHAND, RHAND );

    skeleton.initCompressed();

    skeleton.setFoot( LTOE );
    skeleton.setFoot( RTOE );

    skeleton.setFat( WAIST );
    skeleton.setFat( ORIGIN );
    skeleton.setFat( HEAD );
}

Pinocchio::Attachment* HumanGeom::SetupAttach( const Pinocchio::Mesh &m, const Pinocchio::Skeleton &skeleton )
{
    //skip the fitting step--assume the skeleton is already correct for the mesh

    Pinocchio::TreeType *distanceField = constructDistanceField( m );

    Pinocchio::VisTester<Pinocchio::TreeType> *tester = new Pinocchio::VisTester<Pinocchio::TreeType>( distanceField );

    std::vector<Vector3> embedding;
    embedding = skeleton.fGraph().verts;
    for ( int i = 0; i < (int) embedding.size(); ++i )
    {
        embedding[i] = m.toAdd + embedding[i] * m.scale;
    }

    Pinocchio::Attachment *a = new Pinocchio::Attachment( m, skeleton, embedding, tester );

    delete tester;
    delete distanceField;

    return a;
}

template < typename vertmat >
void HumanGeom::CopyVertsToMesh( const vertmat & vm, Pinocchio::Mesh &m  )
{
    int nvert = m.vertices.size();

    for ( int i = 0; i < nvert; i++ )
    {
        int oVID = m.vertices[i].origVertID;

        // Use reflected point
        double refy = 1.0;

        if ( oVID >= NUM_MESH_VERT )  // Don't check first half -- no point.
        {
            oVID = oVID - NUM_MESH_VERT;

            if ( m_VertCopySet.count( oVID ) == 0 ) // Vert is a mirror.
            {
                refy = -1.0;
            }
        }

        m.vertices[i].pos = Vector3( vm[oVID][0], refy * vm[oVID][1], vm[oVID][2] );
    }
}

template < typename vertmat >
void HumanGeom::CopyMeshToVerts( const Pinocchio::Mesh &m, vertmat & vm )
{
    int nvert = m.vertices.size();

    for ( int i = 0; i < nvert; i++ )
    {
        int oVID = m.vertices[i].origVertID;

        // Copy point
        for ( int j = 0; j < 3; j++ )
        {
            vm[oVID][j] = m.vertices[i].pos[j];
        }

        if ( oVID < NUM_MESH_VERT )  // Don't check second half -- no point.
        {
            if ( m_VertCopySet.count( oVID ) ) // Vert is used as a copy, copy there too.
            {
                int idupper = oVID + NUM_MESH_VERT;
                for ( int j = 0; j < 3; j++ )
                {
                    vm[idupper][j] = m.vertices[i].pos[j];
                }
            }
        }
    }
}

template < typename vertmat >
void HumanGeom::ComputeAvePts( const vertmat &A, const vertmat &B, int n, vertmat &C )
{
    for ( int i = 0; i < n; i++ )
    {
        for ( int j = 0; j < 3; j++ )
        {
            C[i][j] = ( A[i][j] + B[i][j] ) * 0.5;
        }
    }
}

void HumanGeom::ComputeShoulderTrans( const int &ishoulder, const int &ielbow, const double &ang1, const double &ang2, Matrix4d &T )
{
    const vec3d &pshoulder = m_SkelVerts[ ishoulder ];
    const vec3d &pelbow = m_SkelVerts[ ielbow ];

    vec3d vbicep = pelbow - pshoulder;

    vec3d vfwd = vec3d( -1, 0, 0 );

    vec3d out = cross( vbicep, vfwd );
    out.normalize();
    vbicep.normalize();

    vfwd = cross( out, vbicep );
    vfwd.normalize();

    Matrix4d Tstart;
    Tstart.setBasis( vfwd, out, vbicep );
    vec3d angles = Tstart.getAngles();

    T.translatev( pshoulder );
    T.rotateX( ang1 ); // - angles.x() + 180  );
    T.rotateY( ang2 - angles.y() );
    T.translatev( -pshoulder );
}

void HumanGeom::ComputeElbowTrans( const int &ishoulder, const int &ielbow, const int &ihand, const double &ang, const double &ang2, Matrix4d &T )
{
    const vec3d &pshoulder = m_SkelVerts[ ishoulder ];
    const vec3d &pelbow = m_SkelVerts[ ielbow ];
    const vec3d &phand = m_SkelVerts[ ihand ];

    vec3d vforearm = phand - pelbow;
    vec3d vbicep = pelbow - pshoulder;

    double angle0 = angle( vforearm, vbicep );

    vec3d axis = cross( vforearm, vbicep );
    vbicep.normalize();

    T.translatev( pelbow );
    T.rotate( ang2, vbicep );
    T.rotate( ang - angle0, axis );
    T.translatev( -pelbow );
}

void HumanGeom::ComputeHipTrans( const int &iwaist, const int &ihip, const int &iknee, const double &ang1, const double &ang2, Matrix4d &T )
{
    const vec3d &pwaist = m_SkelVerts[ iwaist ];
    const vec3d &phip = m_SkelVerts[ ihip ];
    const vec3d &pknee = m_SkelVerts[ iknee ];

    vec3d vthigh = pknee - phip;

    vec3d vfwd = vec3d( -1, 0, 0 );

    vec3d out = cross( vthigh, vfwd );
    out.normalize();
    vthigh.normalize();

    vfwd = cross( out, vthigh );
    vfwd.normalize();

    Matrix4d Tstart;
    Tstart.setBasis( vfwd, out, vthigh );
    vec3d angles = Tstart.getAngles();

    T.translatev( phip );
    T.rotateX( ang1 - angles.x() + 180  );
    T.rotateY( ang2 - angles.y() );
    T.translatev( -phip );
}

void HumanGeom::ComputeKneeTrans( const int &ihip, const int &iknee, const int &iankle, const double &ang, Matrix4d &T )
{
    const vec3d &phip = m_SkelVerts[ ihip ];
    const vec3d &pknee = m_SkelVerts[ iknee ];
    const vec3d &pankle = m_SkelVerts[ iankle ];

//    vec3d vthigh = pknee - phip;
//    vec3d vshin = pankle - pknee;

    // double angle0 = angle( vshin, vthigh );

    vec3d axis = vec3d( 0, 1, 0 ); // cross( vshin, vthigh );

    T.translatev( pknee );
    T.rotate( ang , axis );
    T.translatev( -pknee );
}

void HumanGeom::ComputeAnkleTrans( const int &iknee, const int &iankle, const int &itoe, const double &ang, Matrix4d &T )
{
    const vec3d &pknee = m_SkelVerts[ iknee ];
    const vec3d &pankle = m_SkelVerts[ iankle ];
    const vec3d &ptoe = m_SkelVerts[ itoe ];

    vec3d vfoot = ptoe - pankle;
    vec3d vshin = pankle - pknee;

    // double angle0 = angle( vfoot, vshin );

    vec3d axis = cross( vfoot, vshin );

    T.translatev( pankle );
    T.rotate( ang, axis );
    T.translatev( -pankle );
}

void HumanGeom::ComputeBackTrans( const int &iorigin, const int &iback, const int &iwaist, const double &ang, Matrix4d &T )
{
    const vec3d &porigin = m_SkelVerts[ iorigin ];
    const vec3d &pback = m_SkelVerts[ iback ];
    const vec3d &pwaist = m_SkelVerts[ iwaist ];

    vec3d vupper = pback - porigin;
    vec3d vlower = pwaist - pback;

    double angle0 = angle( vlower, vupper );

    vec3d axis = vec3d( 0, 1, 0 ); // cross( vshin, vthigh );

    T.translatev( pback );
    T.rotate( -ang, axis );
    T.translatev( -pback );
}

void HumanGeom::ComputeWaistTrans( const int &iback, const int &iwaist, const int &ihip, const double &ang, Matrix4d &T )
{
    const vec3d &pback = m_SkelVerts[ iback ];
    const vec3d &pwaist = m_SkelVerts[ iwaist ];
    vec3d phip = m_SkelVerts[ ihip ];
    phip.set_y( 0.0 );

    vec3d vupper = pwaist - pback;
    vec3d vlower = phip - pwaist;

    double angle0 = angle( vlower, vupper );

    vec3d axis = vec3d( 0, 1, 0 ); // cross( vshin, vthigh );

    T.translatev( pwaist );
    T.rotate( -ang , axis );
    T.translatev( -pwaist );
}
