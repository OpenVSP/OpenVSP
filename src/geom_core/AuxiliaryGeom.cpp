//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#define _USE_MATH_DEFINES
#include <cmath>

#include "APIDefines.h"
#include "VspUtil.h"
#include "AuxiliaryGeom.h"
#include "Vehicle.h"
#include "VSP_Geom_API.h"
#include "PropGeom.h"
#include "GearGeom.h"
#include "Geom.h"
#include "ParmMgr.h"
#include "StlHelper.h"
#include <cfloat>  //For DBL_EPSILON

#include "UnitConversion.h"

using namespace vsp;

//==== Constructor ====//
AuxiliaryGeom::AuxiliaryGeom( Vehicle* vehicle_ptr ) : Geom( vehicle_ptr )
{
    m_Name = "AuxiliaryGeom";
    m_Type.m_Name = "Auxiliary";
    m_Type.m_Type = AUXILIARY_GEOM_TYPE;


    m_AuxuliaryGeomMode.Init( "AuxiliaryGeomType", "Design", this, AUX_GEOM_ROTOR_TIP_PATH, AUX_GEOM_ROTOR_TIP_PATH, NUM_AUX_GEOM_MODES - 1 );
    m_AuxuliaryGeomMode.SetDescript( "Type of auxiliary geometry." );

    m_AutoDiam.Init( "AutoDiam", "Design", this, true, false, true );
    m_AutoDiam.SetDescript( "Flag to set the diameter automatically." );

    m_Diameter.Init( "Diameter", "Design", this, 30.0, 0.0, 1.0e12 );
    m_Diameter.SetDescript( "Diameter of auxiliary geometry" );

    m_FlapRadiusFract.Init( "FlapRadiusFract", "Design", this, 0.0, 0.0, 1.0 );
    m_FlapRadiusFract.SetDescript( "Radius fraction for the flappling hinge location" );

    m_RootLength.Init( "RootLength", "Design", this, 0.0, 0.0, 1.0e12 );
    m_RootLength.SetDescript( "Axial length of rotor at root of burst cone" );

    m_RootOffset.Init( "RootOffset", "Design", this, 0.5, 0.0, 1.0 );
    m_RootOffset.SetDescript( "Axial offset of burst cone as ratio of length" );

    m_ThetaThrust.Init( "ThetaThrust", "Design", this, 15.0, 0.0, 1.0e12 );
    m_ThetaThrust.SetDescript( "Cone angle in the direction of thrust." );

    m_ThetaAntiThrust.Init( "ThetaAntiThrust", "Design", this, 15.0, 0.0, 1.0e12 );
    m_ThetaAntiThrust.SetDescript( "Cone angle in the direction opposite thrust." );

    m_RotorFragmentMode.Init( "RotorFragmentMode", "Design", this, vsp::ONE_THIRD_ROTOR_FRAGMENT, vsp::ONE_THIRD_ROTOR_FRAGMENT, vsp::NUM_FRAGMENT_MODES - 1 );
    m_RotorFragmentMode.SetDescript( "AC 20.128A Rotor fragment mode" );

    m_DiskRadius.Init( "DiskRadius", "Design", this, 2.0, 0.0, 1.0e12 );
    m_DiskRadius.SetDescript( "Disk radius of failing rotor" );

    m_BladeLength.Init( "BladeLength", "Design", this, 1.0, 0.0, 1.0e12 );
    m_BladeLength.SetDescript( "Blade length of failing rotor or fan" );

    m_BladeRootRadius.Init( "BladeRootRadius", "Design", this, 0.1, 0.0, 1.0e12 );
    m_BladeRootRadius.SetDescript( "Blade root radius of failing fan" );

    m_FragLength.Init( "FragLength", "Design", this, 1.0, 0.0, 1.0e12 );
    m_FragLength.SetDescript( "Fragment length of failing rotor" );

    m_CGRadius.Init( "CGRadius", "Design", this, 1.0, 0.0, 1.0e12 );
    m_CGRadius.SetDescript( "CG Radius of failing rotor" );

    m_ReleaseAngle.Init( "ReleaseAngle", "Design", this, 0.0, 0.0, 360.0 );
    m_ReleaseAngle.SetDescript( "Release angle of rotor fragment" );

    m_RotDir.Init( "RotDir", "Design", this, false, false, true );
    m_RotDir.SetDescript( "Rotation direction of failing rotor" );

    m_ThrownBladeMode.Init( "ThrownBladeMode", "Design", this, vsp::PROP_BLADE_TRADITIONAL, vsp::PROP_BLADE_TRADITIONAL, vsp::NUM_PROP_BLADE_MODES - 1);
    m_ThrownBladeMode.SetDescript( "Mode for thrown blade model" );

    m_ThrownBladeCGFrac.Init( "ThrownBladeCGFrac", "Design", this, 0.34, 0.0, 1.0 );
    m_ThrownBladeCGFrac.SetDescript( "CG location of thrown blade" );

    m_ContactPt1_Isymm.Init( "ContactPt1_Isymm", "Design", this, 0, 0, 1 );
    m_ContactPt1_SuspensionMode.Init( "ContactPt1_SuspensionMode", "Design", this, vsp::GEAR_SUSPENSION_NOMINAL, vsp::GEAR_SUSPENSION_NOMINAL, vsp::NUM_GEAR_SUSPENSION_MODES - 1 );
    m_ContactPt1_TireMode.Init( "ContactPt1_TireMode", "Design", this, vsp::TIRE_STATIC_LODED_CONTACT, vsp::TIRE_STATIC_LODED_CONTACT, vsp::NUM_TIRE_CONTACT_MODES - 1 );
    m_ContactPt1_ClearanceMode.Init( "ContactPt1_ClearanceMode", "Design", this, vsp::TIRE_NOMINAL_CLEARANCE, vsp::TIRE_NOMINAL_CLEARANCE, vsp::NUM_TIRE_CLEARANCE_MODES - 1 );
    m_ContactPt1_GearMode.Init( "ContactPt1_GearMode", "Design", this, vsp::GEAR_CONFIGURATION_DOWN, vsp::GEAR_CONFIGURATION_DOWN, vsp::NUM_GEAR_CONFIGURATION_MODES - 1 );
    m_ContactPt1_KRetract.Init( "ContactPt1_KRetract", "Design", this, 0.0, 0.0, 1.0 );

    m_ContactPt2_Isymm.Init( "ContactPt2_Isymm", "Design", this, 0, 0, 1 );
    m_ContactPt2_SuspensionMode.Init( "ContactPt2_SuspensionMode", "Design", this, vsp::GEAR_SUSPENSION_NOMINAL, vsp::GEAR_SUSPENSION_NOMINAL, vsp::NUM_GEAR_SUSPENSION_MODES - 1 );
    m_ContactPt2_TireMode.Init( "ContactPt2_TireMode", "Design", this, vsp::TIRE_STATIC_LODED_CONTACT, vsp::TIRE_STATIC_LODED_CONTACT, vsp::NUM_TIRE_CONTACT_MODES - 1 );

    m_ContactPt3_Isymm.Init( "ContactPt3_Isymm", "Design", this, 0, 0, 1 );
    m_ContactPt3_SuspensionMode.Init( "ContactPt3_SuspensionMode", "Design", this, vsp::GEAR_SUSPENSION_NOMINAL, vsp::GEAR_SUSPENSION_NOMINAL, vsp::NUM_GEAR_SUSPENSION_MODES - 1 );
    m_ContactPt3_TireMode.Init( "ContactPt3_TireMode", "Design", this, vsp::TIRE_STATIC_LODED_CONTACT, vsp::TIRE_STATIC_LODED_CONTACT, vsp::NUM_TIRE_CONTACT_MODES - 1 );

    m_CCEUnits.Init( "CCEUnits", "Design", this, vsp::LEN_IN, vsp::LEN_MM, vsp::LEN_UNITLESS );

    m_CCEMainGearOffset.Init( "CCEMainGearOffset", "Design", this, 0, -1e12, 1e12 );


    m_SprayTireContactWidth.Init( "SprayTireContactWidth", "Design", this, 0.25, 0.0, 10.0 );
    m_SprayTireContactWidth.SetDescript( "b_g/D Non dimensional tire contact width" );

    m_SprayTireContactHalfLength.Init( "SprayTireContactHalfLength", "Design", this, 0.22, 0.0, 10.0 );
    m_SprayTireContactHalfLength.SetDescript( "h/D Non dimensional tire contact half length" );

    m_SpraySideElevationAngle.Init( "SpraySideElevationAngle", "Design", this, 15.0, 0.0, 90.0 );
    m_SpraySideElevationAngle.SetDescript( "gamma_s Side spray elevation angle" );

    m_SpraySidePlanAngle.Init( "SpraySidePlanAngle", "Design", this, 10.0, 0.0, 90.0 );
    m_SpraySidePlanAngle.SetDescript( "theta_s Side spray plan angle" );

    m_SpraySideIncrementalAngle.Init( "SpraySideIncrementalAngle", "Design", this, 4.0, 0.0, 90.0 );
    m_SpraySideIncrementalAngle.SetDescript( "lambda_s Side spray incremental angle" );

    m_SpraySideInclinationAngle.Init( "SpraySideInclinationAngle", "Design", this, 7.0, 0.0, 90.0 );
    m_SpraySideInclinationAngle.SetDescript( "phi_s Side spray inclination angle" );

    m_SprayCenterElevationAngle.Init( "SprayCenterElevationAngle", "Design", this, 20.0, 0.0, 90.0 );
    m_SprayCenterElevationAngle.SetDescript( "gamma_c Center spray elevation angle" );

    m_SprayCenterWidth.Init( "SprayCenterWidth", "Design", this, 0.1, 0.0, 1.0e2 );
    m_SprayCenterWidth.SetDescript( "delta y_c/D Non dimensional center spray width" );


    m_WheelTireFailureMode.Init( "m_WheelTireFailureMode", "Design", this, vsp::WHEEL_TIRE_1LG, vsp::WHEEL_TIRE_1LG, vsp::NUM_WHEEL_TIRE_FAILURE_MODES - 1 );
    m_WheelTireFailureMode.SetDescript( "Mode for AMC 25.734 wheel and tire failure model" );



    m_SCWorldAligned.Init( "SCWorldAligned", "Design", this, true, false, true );

    m_BogieTheta.Init( "BogieTheta", "Design", this, 0.0, -180.0, 180.0 );
    m_WheelTheta.Init( "WheelTheta", "Design", this, 0.0, -180.0, 180.0 );
    m_RollTheta.Init( "RollTheta", "Design", this, 0.0, -180.0, 180.0 );

    m_ParentType = -1;

    // Set up default CCE points and curve.
    vector < vec3d > pnt_vec;
    pnt_vec.push_back( vec3d( -1000.0, 0.0, 0.0 ) );
    pnt_vec.push_back( vec3d( 1000.0, 0.0, 0.0 ) );
    SetPnts( pnt_vec );

    m_XSCurve = nullptr;
    SetXSecCurveType( vsp::XS_AC25_773 );
}

//==== Destructor ====//
AuxiliaryGeom::~AuxiliaryGeom()
{

}

void AuxiliaryGeom::ComputeCenter()
{
}

void AuxiliaryGeom::Scale()
{
    double currentScale = m_Scale() / m_LastScale();

    m_XSCurve->SetScale( currentScale );

    m_Diameter *= currentScale;

    m_RootLength *= currentScale;

    m_CCEMainGearOffset *= currentScale;

    m_LastScale = m_Scale();
}

void AuxiliaryGeom::AddDefaultSources( double base_len )
{
}

void AuxiliaryGeom::OffsetXSecs( double off )
{
    m_XSCurve->OffsetCurve( off );
}

void AuxiliaryGeom::SetDirtyFlags( Parm* parm_ptr )
{
    if ( !parm_ptr )
    {
        return;
    }

    Geom::SetDirtyFlags( parm_ptr );

    if ( parm_ptr == &m_AuxuliaryGeomMode )
    {
        m_XFormDirty = true;
        m_SurfDirty = true;
    }
}

void AuxiliaryGeom::UpdateSurf()
{
    m_MainSurfVec.resize( 1 );
    m_MainSurfVec[0] = VspSurf();

    //===== Find Parent ====//
    Geom* parent_geom = m_Vehicle->FindGeom( m_ParentID );
    if ( !parent_geom )
    {
        return;
    }

    m_ParentType = parent_geom->GetType().m_Type;


    double refLen = 30.0;

    Vehicle *veh = VehicleMgr.GetVehicle();
    if ( veh )
    {
        BndBox emptybbox;
        BndBox b = VehicleMgr.GetVehicle()->GetScaleIndependentBndBox();
        if ( b != emptybbox )
        {
            refLen = 1.5 * b.DiagDist();
        }
    }

    m_DiskRadius.Activate();
    m_BladeLength.Activate();
    m_BladeRootRadius.Activate();
    m_FragLength.Activate();
    m_CGRadius.Activate();
    m_ThetaThrust.Activate();
    m_ThetaAntiThrust.Activate();

    if ( m_ParentType == PROP_GEOM_TYPE )
    {
        PropGeom * parent_prop = dynamic_cast< PropGeom* > ( parent_geom );

        if ( parent_prop )
        {
            if ( m_AutoDiam() )
            {
                if ( m_AuxuliaryGeomMode() == AUX_GEOM_ROTOR_TIP_PATH )
                {
                    m_Diameter.Set( parent_prop->m_Diameter() );
                }
                else // AUX_GEOM_ROTOR_BURST
                {
                    m_Diameter.Set( refLen );
                }
                m_Diameter.Deactivate();
            }
            else
            {
                m_Diameter.Activate();
            }
        }


        if ( m_AuxuliaryGeomMode() == vsp::AUX_GEOM_ROTOR_TIP_PATH )
        {
            double radius = m_Diameter() * 0.5;
            double flapr = radius * m_FlapRadiusFract();
            double bladelen = radius - flapr;

            // Fraction of radius to place cubic control point
            // f = k * tan( theta * 0.25 );
            const double k = 1.332440737409712163877261103501904586378054505346634846763928262809148692267066998533212237794404131;

            curve_segment_type linear_seg( 1 );
            curve_segment_type cubic_seg( 3 );

            curve_segment_type::control_point_type thDir;
            curve_segment_type::control_point_type antDir;

            curve_segment_type::control_point_type cpOrigin;
            curve_segment_type::control_point_type cpThrust, cpThrust1, cpThrust2;
            curve_segment_type::control_point_type cpMid;
            curve_segment_type::control_point_type cpAnti, cpAnti1, cpAnti2;

            cpOrigin << 0.0, flapr, 0.0;
            cpThrust << sin( -m_ThetaThrust() * M_PI / 180.0 ) * bladelen, flapr + cos( -m_ThetaThrust() * M_PI / 180.0 ) * bladelen, 0.0;
            cpMid << 0.0, radius, 0.0;
            cpAnti << sin( m_ThetaAntiThrust() * M_PI / 180.0 ) * bladelen, flapr + cos( m_ThetaAntiThrust() * M_PI / 180.0 ) * bladelen, 0.0;

            thDir << cos( -m_ThetaThrust() * M_PI / 180.0 ), -sin( -m_ThetaThrust() * M_PI / 180.0 ), 0.0;
            cpThrust1 = cpThrust + thDir * k * bladelen * tan( m_ThetaThrust() * 0.25 * M_PI / 180.0 );
            cpThrust2 << k * tan( -m_ThetaThrust() * 0.25 * M_PI / 180.0 ) * bladelen, radius, 0.0;;

            antDir << -cos( m_ThetaAntiThrust() * M_PI / 180.0 ), sin( m_ThetaAntiThrust() * M_PI / 180.0 ), 0.0;
            cpAnti1 << k * tan( m_ThetaAntiThrust() * 0.25 * M_PI / 180.0 ) * bladelen, radius, 0.0;;
            cpAnti2 = cpAnti + antDir * k * bladelen * tan( m_ThetaAntiThrust() * 0.25 * M_PI / 180.0 );

            VspCurve crv;
            linear_seg.set_control_point( cpOrigin, 0 );
            linear_seg.set_control_point( cpThrust, 1 );
            crv.AppendCurveSegment( linear_seg );

            cubic_seg.set_control_point( cpThrust, 0 );
            cubic_seg.set_control_point( cpThrust1, 1 );
            cubic_seg.set_control_point( cpThrust2, 2 );
            cubic_seg.set_control_point( cpMid, 3 );
            crv.AppendCurveSegment( cubic_seg );

            cubic_seg.set_control_point( cpMid, 0 );
            cubic_seg.set_control_point( cpAnti1, 1 );
            cubic_seg.set_control_point( cpAnti2, 2 );
            cubic_seg.set_control_point( cpAnti, 3 );
            crv.AppendCurveSegment( cubic_seg );

            linear_seg.set_control_point( cpAnti, 0 );
            linear_seg.set_control_point( cpOrigin, 1 );
            crv.AppendCurveSegment( linear_seg );

            m_MainSurfVec[0].CreateBodyRevolution( crv, true );
            m_MainSurfVec[0].SetMagicVParm( false );
        }
    }
    if ( m_ParentType == GEAR_GEOM_TYPE )
    {
        GearGeom * gear = dynamic_cast< GearGeom* > ( parent_geom );
        if ( gear )
        {
            vector < Bogie* > bogie_vec = gear->GetBogieVec();
            vector < string > bogie_id_vec;
            for ( unsigned int i = 0; i < bogie_vec.size(); i++ )
            {
                bogie_id_vec.push_back( bogie_vec[i]->GetID() );
            }

            int indx = vector_find_val( bogie_id_vec, m_ContactPt1_ID );
            if ( ( indx < 0 || indx >= bogie_id_vec.size() ) && !bogie_id_vec.empty() )
            {
                m_ContactPt1_ID = bogie_id_vec[0];
            }

            indx = vector_find_val( bogie_id_vec, m_ContactPt2_ID );
            if ( ( indx < 0 || indx >= bogie_id_vec.size() ) && !bogie_id_vec.empty() )
            {
                m_ContactPt2_ID = bogie_id_vec[0];
            }

            indx = vector_find_val( bogie_id_vec, m_ContactPt3_ID );
            if ( ( indx < 0 || indx >= bogie_id_vec.size() ) && !bogie_id_vec.empty() )
            {
                m_ContactPt3_ID = bogie_id_vec[0];
            }
        }

        if ( m_AuxuliaryGeomMode() == vsp::AUX_GEOM_THREE_PT_GROUND )
        {
            GearGeom * gear = dynamic_cast< GearGeom* > ( parent_geom );
            if ( gear )
            {
                Matrix4d mat;

                gear->BuildThreePtBasis( m_ContactPt1_ID, m_ContactPt1_Isymm(), m_ContactPt1_SuspensionMode(), m_ContactPt1_TireMode(),
                                         m_ContactPt2_ID, m_ContactPt2_Isymm(), m_ContactPt2_SuspensionMode(), m_ContactPt2_TireMode(),
                                         m_ContactPt3_ID, m_ContactPt3_Isymm(), m_ContactPt3_SuspensionMode(), m_ContactPt3_TireMode(),
                                         mat );


                m_MainSurfVec[0].CreatePlane( -refLen, refLen, -refLen, refLen );
                m_MainSurfVec[0].Transform( mat );

                AppendContact1Surfs( gear );
                AppendContact2Surfs( gear );
                AppendContact3Surfs( gear );
            }
        }
        else if ( m_AuxuliaryGeomMode() == vsp::AUX_GEOM_TWO_PT_GROUND )
        {
            GearGeom * gear = dynamic_cast< GearGeom* > ( parent_geom );
            if ( gear )
            {
                Matrix4d basis;
                m_ContactPts.resize( 2 );
                double bogietheta = m_BogieTheta() * M_PI / 180.0;

                gear->BuildTwoPtBasis( m_ContactPt1_ID, m_ContactPt1_Isymm(), m_ContactPt1_SuspensionMode(), m_ContactPt1_TireMode(),
                                       m_ContactPt2_ID, m_ContactPt2_Isymm(), m_ContactPt2_SuspensionMode(), m_ContactPt2_TireMode(),
                                       bogietheta, basis, m_ContactPts[0], m_ContactPts[1] );

                vec3d ptaxis, axis;
                if ( m_WheelTheta() > 0 )
                {
                    gear->GetTwoPtAftAxleAxis( m_ContactPt1_ID, m_ContactPt1_Isymm(), m_ContactPt1_SuspensionMode(),
                                           m_ContactPt2_ID, m_ContactPt2_Isymm(), m_ContactPt2_SuspensionMode(),
                                           bogietheta, ptaxis, axis );
                }
                else
                {
                    gear->GetTwoPtFwdAxleAxis( m_ContactPt1_ID, m_ContactPt1_Isymm(), m_ContactPt1_SuspensionMode(),
                                           m_ContactPt2_ID, m_ContactPt2_Isymm(), m_ContactPt2_SuspensionMode(),
                                           bogietheta, ptaxis, axis );
                }

                Matrix4d mat;
                mat.translatev( ptaxis );
                mat.rotate( m_WheelTheta() * M_PI / 180.0, axis );
                mat.translatev( -ptaxis );

                mat.matMult( basis );


                m_BasisOrigin = mat.xform( vec3d( 0.0, 0.0, 0.0 ) );

                m_BasisAxis.clear();
                m_BasisAxis.resize( 3 );
                for ( int i = 0; i < 3; i++ )
                {
                    vec3d pt = vec3d( 0.0, 0.0, 0.0 );
                    pt.v[i] = 1;
                    m_BasisAxis[i] = mat.xform( pt );
                }


                m_MainSurfVec[0].CreatePlane( -refLen, refLen, -refLen, refLen );
                m_MainSurfVec[0].Transform( mat );

                AppendContact1Surfs( gear, m_BogieTheta() );
                AppendContact2Surfs( gear, m_BogieTheta() );

                // Return results in world coordinates
                Matrix4d world = gear->getModelMatrix();
                world.xformvec( m_ContactPts );

                // Visualize fwd/aft contact point and axis on rotated bogie.
                // vec3d pt, normal, ptaxis2, axis2;
                // GetPtNormalAftAxleAxis( m_BogieTheta() * M_PI / 180.0, pt, normal, ptaxis2, axis2 );
                // m_ContactPts.push_back( pt );
                // m_ContactPts.push_back( ptaxis2 );
                // GetPtNormalFwdAxleAxis( m_BogieTheta() * M_PI / 180.0, pt, normal, ptaxis2, axis2 );
                // m_ContactPts.push_back( pt );
                // m_ContactPts.push_back( ptaxis2 );
            }
        }
        else if ( m_AuxuliaryGeomMode() == vsp::AUX_GEOM_ONE_PT_GROUND )
        {
            GearGeom * gear = dynamic_cast< GearGeom* > ( parent_geom );
            if ( gear )
            {
                Matrix4d basis;
                vec3d p1;
                double bogietheta = m_BogieTheta() * M_PI / 180.0;

                gear->BuildOnePtBasis( m_ContactPt1_ID, m_ContactPt1_Isymm(), m_ContactPt1_SuspensionMode(), m_ContactPt1_TireMode(),
                                       bogietheta, m_WheelTheta() * M_PI / 180.0, m_RollTheta() * M_PI / 180.0, basis, p1);

                m_BasisOrigin = basis.xform( vec3d( 0.0, 0.0, 0.0 ) );

                m_BasisAxis.clear();
                m_BasisAxis.resize( 3 );
                for ( int i = 0; i < 3; i++ )
                {
                    vec3d pt = vec3d( 0.0, 0.0, 0.0 );
                    pt.v[i] = 1;
                    m_BasisAxis[i] = basis.xform( pt );
                }


                m_MainSurfVec[0].CreatePlane( -refLen, refLen, -refLen, refLen );
                m_MainSurfVec[0].Transform( basis );

                AppendContact1Surfs( gear, m_BogieTheta() );
            }
        }
        else if ( m_AuxuliaryGeomMode() == vsp::AUX_GEOM_THREE_PT_CCE )
        {
            GearGeom * gear = dynamic_cast< GearGeom* > ( parent_geom );
            if ( gear )
            {
                Matrix4d mat;

                gear->BuildThreePtOffAxisBasis( m_ContactPt1_ID, m_ContactPt1_Isymm(), m_ContactPt1_SuspensionMode(), m_ContactPt1_TireMode(),
                                                m_ContactPt2_ID, m_ContactPt2_Isymm(), m_ContactPt2_SuspensionMode(), m_ContactPt2_TireMode(),
                                                m_ContactPt3_ID, m_ContactPt3_Isymm(), m_ContactPt3_SuspensionMode(), m_ContactPt3_TireMode(),
                                                m_CCEMainGearOffset(),
                                                mat );


                vector < VspCurve > crv_vec( 2, m_CCECurve );

                double cce2model = ConvertLength( 1, m_CCEUnits(), gear->m_ModelLenUnits() );


                Matrix4d start;
                start.translatef( refLen, 0, 0 );
                start.scale( cce2model );
                crv_vec[0].Transform( start );

                Matrix4d end;
                end.translatef( -refLen, 0, 0 );
                end.scale( cce2model );
                crv_vec[1].Transform( end );

                m_MainSurfVec[0].SkinC0( crv_vec, false );

                // Swap UW because U tessellation will better obey patch boundaries and thereby resolve
                // the shape of the envelope.
                m_MainSurfVec[0].SwapUWDirections();

                m_MainSurfVec[0].Transform( mat );

                AppendContact1Surfs( gear );
                AppendContact2Surfs( gear );
                AppendContact3Surfs( gear );
            }
        }
        else if ( m_AuxuliaryGeomMode() == vsp::AUX_GEOM_SINGLE_GEAR )
        {
            GearGeom * gear = dynamic_cast< GearGeom* > ( parent_geom );
            if ( gear )
            {
                m_MainSurfVec.clear();

                AppendContact1Surfs( gear );
            }
        }
        else if ( m_AuxuliaryGeomMode() == vsp::AUX_GEOM_TIRE_SPRAY )
        {
            GearGeom * gear = dynamic_cast< GearGeom* > ( parent_geom );
            if ( gear )
            {
                m_MainSurfVec.clear();

                const Bogie *b1 = gear->GetBogie( m_ContactPt1_ID );
                if ( b1 )
                {
                    double D = b1->m_DiameterModel();
                    double h = m_SprayTireContactHalfLength() * D;
                    double bg = m_SprayTireContactWidth() * D;

                    double gammas = m_SpraySideElevationAngle() * M_PI / 180.0;
                    double thetas = m_SpraySidePlanAngle() * M_PI / 180.0;
                    double lambdas = m_SpraySideIncrementalAngle() * M_PI / 180.0;
                    double phis = m_SpraySideInclinationAngle() * M_PI / 180.0;

                    double x0 = -2.0 * h;
                    double y0 = 0.75 * bg;
                    double z0 = 0.0;
                    vec3d p0 = vec3d( x0, y0, z0 );

                    double x = refLen;

                    double zs = ( 2.0 * h + x ) * tan( gammas );
                    double ys = y0 + ( 2.0 * h + x ) * tan( thetas );
                    double deltays = ( 2.0 * h + x ) * tan( lambdas );

                    double deltay_inc = zs * tan( phis );


                    vector < double > ts = { 0, 1.0, 2.0, 3.0, 4.0 };
                    vector < vec3d > pts;
                    pts.emplace_back( p0 );
                    pts.emplace_back( p0 );
                    pts.emplace_back( p0 );
                    pts.emplace_back( p0 );
                    pts.emplace_back( pts[ 0 ] );

                    vector < VspCurve > crv_vec(3);
                    crv_vec[0].InterpolateLinear( pts, ts, false );

                    pts.clear();
                    pts.emplace_back( vec3d( x, ys, zs ) );
                    pts.emplace_back( vec3d( x, ys - deltay_inc, 0.0 ) );
                    pts.emplace_back( vec3d( x, ys - deltays - deltay_inc, 0.0 ) );
                    pts.emplace_back( vec3d( x, ys - deltays, zs ) );
                    pts.emplace_back( pts[ 0 ] );

                    crv_vec[1].InterpolateLinear( pts, ts, false );

                    vec3d pcen;
                    for ( int i = 0; i < pts.size() - 1; i++ )
                    {
                        pcen += pts[i];
                    }
                    pcen *= 0.25;

                    for ( int i = 0; i < pts.size(); i++ )
                    {
                        pts[i] = pcen;
                    }

                    crv_vec[2].InterpolateLinear( pts, ts, false );


                    m_MainSurfVec.resize( 1 );
                    m_MainSurfVec[0].SkinC0( crv_vec, false );
                    m_MainSurfVec.push_back( m_MainSurfVec[0] );

                    vec3d pt = b1->GetFwdSideContactPoint( m_ContactPt1_Isymm(), m_ContactPt1_SuspensionMode(),  m_ContactPt1_TireMode(), 1 );


                    Matrix4d M;
                    M.translatev( pt );

                    m_MainSurfVec[0].Transform( M );

                    pt = b1->GetFwdSideContactPoint( m_ContactPt1_Isymm(), m_ContactPt1_SuspensionMode(),  m_ContactPt1_TireMode(), -1 );

                    M.loadIdentity();
                    M.loadXZRef();
                    m_MainSurfVec[1].Transform( M );
                    m_MainSurfVec[1].FlipNormal();
                    M.loadIdentity();
                    M.translatev( pt );

                    m_MainSurfVec[1].Transform( M );


                    if ( b1->m_NAcross() > 1 && b1->m_NTandem() == 1 )
                    {
                        double gammac = m_SprayCenterElevationAngle() * M_PI / 180.0;

                        double zc = ( 2.0 * h + x ) * tan( gammac );
                        double yc = 0.5 * m_SprayCenterWidth() * D;

                        pts.clear();
                        pts.emplace_back( vec3d( x0, yc, 0.0 ) );
                        pts.emplace_back( vec3d( x0, yc, 0.0 ) );
                        pts.emplace_back( vec3d( x0, -yc, 0.0 ) );
                        pts.emplace_back( vec3d( x0, -yc, 0.0 ) );
                        pts.emplace_back( pts[ 0 ] );

                        crv_vec[0].InterpolateLinear( pts, ts, false );

                        pts.clear();
                        pts.emplace_back( vec3d( x, yc, zc ) );
                        pts.emplace_back( vec3d( x, yc, 0.0 ) );
                        pts.emplace_back( vec3d( x, -yc, 0.0 ) );
                        pts.emplace_back( vec3d( x, -yc, zc ) );
                        pts.emplace_back( pts[ 0 ] );

                        crv_vec[1].InterpolateLinear( pts, ts, false );

                        pcen = vec3d();
                        for ( int i = 0; i < pts.size() - 1; i++ )
                        {
                            pcen += pts[i];
                        }
                        pcen *= 0.25;

                        for ( int i = 0; i < pts.size(); i++ )
                        {
                            pts[i] = pcen;
                        }

                        crv_vec[2].InterpolateLinear( pts, ts, false );

                        VspSurf cen_spray;
                        cen_spray.SkinC0( crv_vec, false );

                        for ( int igap = 0; igap < b1->m_NAcross() - 1; igap++ )
                        {
                            vec3d pc = b1->GetFwdCenterContactPoint( m_ContactPt1_Isymm(), m_ContactPt1_SuspensionMode(),  m_ContactPt1_TireMode(), igap );

                            Matrix4d Mc;
                            Mc.translatev( pc );

                            m_MainSurfVec.push_back( cen_spray );
                            m_MainSurfVec.back().Transform( Mc );
                        }

                    }

                }


            }
        }
        else if ( m_AuxuliaryGeomMode() == vsp::AUX_GEOM_WHEEL_TIRE_FAILURE )
        {
            m_MainSurfVec.clear();

            GearGeom * gear = dynamic_cast< GearGeom* > ( parent_geom );
            if ( gear )
            {

                const Bogie *b1 = gear->GetBogie( m_ContactPt1_ID );
                if ( b1 )
                {
                    if ( m_WheelTireFailureMode() == vsp::WHEEL_TIRE_1LG ||
                         m_WheelTireFailureMode() == vsp::WHEEL_TIRE_1SM )
                    {
                        // Large debris angle.
                        double theta = 15 * M_PI / 180.0;
                        if ( m_WheelTireFailureMode() == vsp::WHEEL_TIRE_1SM )
                        {
                            theta = 30 * M_PI / 180.0;
                        }

                        double bsw = b1->GetBogieSemiWidth();
                        double dg = b1->m_DGModel();
                        double aftaxle = b1->GetAxleArm();

                        double r = refLen;
                        double dy = r * tan( theta );
                        double y = bsw + dy;

                        vector < vec3d > pts;
                        pts.emplace_back( vec3d( 0.0, 0.0, 0.0 ) );
                        pts.emplace_back( vec3d( 0.0, -bsw, 0.0 ) );
                        pts.emplace_back( vec3d( 0.5 * r, -bsw - 0.5 * dy, 0.0 ) );
                        pts.emplace_back( vec3d( r, -y, 0.0 ) );
                        pts.emplace_back( vec3d( r, 0, 0.0 ) );
                        pts.emplace_back( vec3d( r, y, 0.0 ) );
                        pts.emplace_back( vec3d( 0.5 * r, bsw + 0.5 * dy, 0.0 ) );
                        pts.emplace_back( vec3d( 0.0, bsw, 0.0 ) );
                        pts.emplace_back( pts[ 0 ] );

                        vector < double > ts = linspace( 0, 4, 9 );

                        VspCurve crv;
                        crv.InterpolateLinear( pts, ts, false );

                        VspSurf zone;
                        zone.CreateBodyRevolution( crv, true, 1 );
                        zone.SetMagicVParm( false );

                        zone.TrimV( 2.0, true );
                        zone.SwapUWDirections();
                        zone.FlipNormal();

                        vec3d zero( 0.0, 0.0, 0.0 );
                        zone.CapUMin( FLAT_END_CAP, 1.0, 0.0, 0.0, zero, false );
                        zone.CapUMax( FLAT_END_CAP, 1.0, 0.0, 0.0, zero, false );

                        // Add 1% exra width to clear zones.
                        y *= 1.01;

                        pts.clear();
                        pts.emplace_back( vec3d( -aftaxle, -y, -0.1 * dg ) );
                        pts.emplace_back( vec3d( -aftaxle, -y, 0.5 * dg ) );
                        pts.emplace_back( vec3d( -aftaxle - 0.5 * refLen * 1.01, -y, 0.5 * dg ) );
                        pts.emplace_back( vec3d( -aftaxle - refLen * 1.01, -y, 0.5 * dg ) );
                        pts.emplace_back( vec3d( -aftaxle - refLen * 1.01, -y, -0.1 * dg ) );
                        pts.emplace_back( vec3d( -aftaxle - refLen * 1.01, -y, -0.5 * dg ) );
                        pts.emplace_back( vec3d( -aftaxle - 0.5 * refLen * 1.01, -y, -0.5 * dg ) );
                        pts.emplace_back( vec3d( -aftaxle, -y, -0.5 * dg ) );
                        pts.emplace_back( pts[ 0 ] );


                        VspCurve c1;
                        c1.InterpolateLinear( pts, ts, false );

                        for ( int i = 0; i < pts.size(); i++ )
                        {
                            pts[ i ].set_y( y );
                        }

                        VspCurve c2;
                        c2.InterpolateLinear( pts, ts, false );

                        vector < VspCurve > crv_vec = { c1, c2 };

                        VspSurf clear_fwd;
                        clear_fwd.SkinC0( crv_vec, false );
                        clear_fwd.SetMagicVParm( false );
                        clear_fwd.SetSurfCfdType( vsp::CFD_NEGATIVE );

                        clear_fwd.CapUMin( POINT_END_CAP, 1.0, 0.0, 0.5, zero, false );
                        clear_fwd.CapUMax( POINT_END_CAP, 1.0, 0.0, 0.5, zero, false );


                        double k = sqrt( 0.5 );

                        double x0 = aftaxle + 0.5 * dg * k;
                        double z0 = -0.5 * dg * k;
                        double dl = 1.01 * refLen * k;
                        double dw = 1.01 * refLen * 0.5;


                        pts.clear();
                        pts.emplace_back( vec3d( x0 + 0.333 * ( dw + dl ), -y, z0 ) );
                        pts.emplace_back( vec3d( x0, -y, z0 ) );
                        pts.emplace_back( vec3d( x0 + 0.5 * dl, -y, z0 + 0.5 * dl ) );
                        pts.emplace_back( vec3d( x0 + dl, -y, z0 + dl ) );
                        pts.emplace_back( vec3d( x0 + dl + 0.5 * dw, -y, z0 + dl - 0.5 * dw ) );
                        pts.emplace_back( vec3d( x0 + dl + dw, -y, z0 + dl - dw ) );
                        pts.emplace_back( vec3d( x0 + dl + dw, -y, z0 ) );
                        pts.emplace_back( vec3d( x0 + 0.667 * ( dl + dw ), -y, z0 ) );
                        pts.emplace_back( pts[ 0 ] );

                        c1.InterpolateLinear( pts, ts, false );

                        for ( int i = 0; i < pts.size(); i++ )
                        {
                            pts[ i ].set_y( y );
                        }

                        c2.InterpolateLinear( pts, ts, false );

                        vector < VspCurve > crv2_vec = { c1, c2 };

                        VspSurf clear_aft;
                        clear_aft.SkinC0( crv2_vec, false );
                        clear_aft.SetMagicVParm( false );
                        clear_aft.FlipNormal();
                        clear_aft.SetSurfCfdType( vsp::CFD_NEGATIVE );

                        clear_aft.CapUMin( POINT_END_CAP, 1.0, 0.0, 0.5, zero, false );
                        clear_aft.CapUMax( POINT_END_CAP, 1.0, 0.0, 0.5, zero, false );


                        m_MainSurfVec.push_back( zone );

                        if ( b1->m_NTandem() > 1 )
                        {
                            Matrix4d fwd;
                            fwd.translatef( -aftaxle, 0, 0 );
                            m_MainSurfVec[ 0 ].Transform( fwd );

                            Matrix4d aft;
                            aft.translatef( aftaxle, 0, 0 );
                            // Rotate by a small angle to prevent bottom surfaces of zones from being coincident.
                            // Angle calculated to stay within clear_fwd and clear_aft.
                            aft.rotateY( atan( -0.25 * dg * k / refLen ) * 180.0 / M_PI ); // deg
                            m_MainSurfVec.push_back( zone );
                            m_MainSurfVec[ 1 ].Transform( aft );
                        }

                        m_MainSurfVec.push_back( clear_fwd );
                        m_MainSurfVec.push_back( clear_aft );

                        Matrix4d pivot;
                        pivot.translatev( b1->GetPivotPoint( m_ContactPt1_Isymm(), m_ContactPt1_SuspensionMode() ) );

                        for ( int i = 0; i < m_MainSurfVec.size(); i++ )
                        {
                            m_MainSurfVec[ i ].Transform( pivot );
                        }
                    }
                    else if ( m_WheelTireFailureMode() == vsp::WHEEL_TIRE_2 )
                    {
                        double w2rim = b1->m_WrimModel() * 0.5;
                        double dflange = b1->m_DFlangeModel();

                        double theta = 20 * M_PI / 180.0;

                        vector < vec3d > pts;
                        pts.emplace_back( vec3d( 0.0, w2rim, 0.0 ) );
                        pts.emplace_back( vec3d( 0.0, w2rim, 0.5 * dflange ) );
                        pts.emplace_back( vec3d( 0.0, w2rim + refLen, 0.5 * dflange + refLen * tan( theta ) ) );
                        pts.emplace_back( vec3d( 0.0, w2rim + refLen, 0.0 ) );

                        vector < double > ts = { 0, 4.0*1.0/3.0, 4.0*2.0/3.0, 4.0 };

                        VspCurve crv;
                        crv.InterpolateLinear( pts, ts, false );

                        VspSurf rim_burst;
                        rim_burst.CreateBodyRevolution( crv, true, 1 );
                        rim_burst.SetMagicVParm( false );


                        double pitch = b1->m_Pitch();
                        double ax = b1->GetAxleArm();
                        double bogiew = b1->GetBogieSemiWidth();

                        vec3d pivot = b1->GetPivotPoint( m_ContactPt1_Isymm(), m_ContactPt1_SuspensionMode() );

                        for ( int itandem = 0; itandem < b1->m_NTandem(); itandem++ )
                        {
                            vec3d pt = pivot + vec3d( ax - itandem * pitch, bogiew, 0 );

                            Matrix4d M;
                            M.translatev( pt );

                            m_MainSurfVec.push_back( rim_burst );
                            m_MainSurfVec.back().Transform( M );
                        }

                        Matrix4d Mflip;
                        Mflip.loadXZRef();

                        rim_burst.Transform( Mflip );
                        rim_burst.FlipNormal();

                        for ( int itandem = 0; itandem < b1->m_NTandem(); itandem++ )
                        {
                            vec3d pt = pivot + vec3d( ax - itandem * pitch, -bogiew, 0 );

                            Matrix4d M;
                            M.translatev( pt );

                            m_MainSurfVec.push_back( rim_burst );
                            m_MainSurfVec.back().Transform( M );
                        }
                    }
                    if ( m_WheelTireFailureMode() == vsp::WHEEL_TIRE_3E ||
                         m_WheelTireFailureMode() == vsp::WHEEL_TIRE_3R )
                    {
                        double r = 0.5 * b1->m_DiameterModel();
                        double wsg = b1->m_WsGModel();
                        double lstrip = 2.5 * wsg;
                        double rflail = sqrt( r * r + lstrip * lstrip );
                        double hflail = rflail - r;

                        double theta = 15 * M_PI / 180.0;
                        double wflail = 0.25 * wsg + lstrip * tan( theta );

                        vector < vec3d > pts;
                        pts.emplace_back( vec3d( 0.0, 0.25 * wsg, r ) );
                        pts.emplace_back( vec3d( 0.0, -0.25 * wsg, r ) );
                        pts.emplace_back( vec3d( 0.0, -0.25 * wsg - wflail, rflail ) );
                        pts.emplace_back( vec3d( 0.0, 0.25 * wsg + wflail, rflail ) );
                        pts.emplace_back( pts[0] );

                        vector < double > ts = { 0, 1.0, 2.0, 3.0, 4.0 };

                        VspCurve crv;
                        crv.InterpolateLinear( pts, ts, false );

                        VspSurf flail;
                        flail.CreateBodyRevolution( crv, true, 1 );
                        flail.SetMagicVParm( false );

                        bool stow = false;
                        bool ret = false;
                        double kretract = 0.0;
                        if ( m_WheelTireFailureMode() == vsp::WHEEL_TIRE_3R )
                        {
                            if ( b1->m_RetMode() == vsp::GEAR_STOWED_POSITION )
                            {
                                stow = true;
                            }
                            else
                            {
                                ret = true;
                                kretract = 1.0;
                            }
                        }

                        b1->TireToBogie( flail, m_MainSurfVec, m_ContactPt1_Isymm(), m_ContactPt1_SuspensionMode(), 0.0, stow, ret, kretract );

                    }

                }
            }
        }
    }
    else if ( m_AuxuliaryGeomMode() == vsp::AUX_GEOM_SUPER_CONE )
    {
        if ( !m_XSCurve )
        {
            return;
        }

        m_XSCurve->Update(); // May not need to force Update here()

        double w = m_XSCurve->GetWidth();
        VspCurve c = m_XSCurve->GetCurve();

        Matrix4d basicmat;
        basicmat.translatef( -w * 0.5, 0, 0 );
        c.Transform( basicmat );

        c.Scale( M_PI / 180.0 );

        Matrix4d mat;
        mat.translatef( 0, 0, refLen );

        c.Transform( mat );

        BndBox bb;
        c.GetBoundingBox( bb );
        vec3d cen = bb.GetCenter();

        c.EvaluateOnSphere( false );
        c.Reverse();

        // Evaluate cen on sphere.
        double cx = std::cos( cen.x() );
        double sx = std::sin( cen.x() );
        double cy = std::cos( cen.y() );
        double sy = std::sin( cen.y() );
        vec3d censph( cen.z() * cy * sx, cen.z() * sy, cen.z() * cy * cx );

        Matrix4d orient;
        orient.scaley( -1.0 );
        orient.rotateX( 90 );
        orient.rotateY( -90 );

        c.Transform( orient );
        c.Reverse();

        censph.Transform( orient );

        // Origin point.
        vec3d o;
        m_MainSurfVec[0].SkinPCPC0( censph, c, o );
    }
    else if ( m_AuxuliaryGeomMode() == vsp::AUX_GEOM_ROTOR_BURST )
    {
        double radius = m_Diameter() * 0.5;

        double xstart = -m_RootOffset() * m_RootLength();
        double xend = xstart + m_RootLength();

        vector < vec3d > pts;
        pts.emplace_back( vec3d( xstart, 0.0, 0.0 ) );
        pts.emplace_back( vec3d( xstart + tan( -m_ThetaThrust() * M_PI / 180.0 ) * radius, radius, 0.0 ) );
        pts.emplace_back( vec3d( xend + tan( m_ThetaAntiThrust() * M_PI / 180.0 ) * radius, radius, 0.0 ) );
        pts.emplace_back( vec3d( xend, 0.0, 0.0 ) );

        vector < double > ts = { 0, 4.0*1.0/3.0, 4.0*2.0/3.0, 4.0 };

        VspCurve crv;
        crv.InterpolateLinear( pts, ts, false );

        m_MainSurfVec[0].CreateBodyRevolution( crv, true );
        m_MainSurfVec[0].SetMagicVParm( false );
    }
    else if ( m_AuxuliaryGeomMode() == vsp::AUX_GEOM_ROTOR_FRAGMENT ||
              m_AuxuliaryGeomMode() == vsp::AUX_GEOM_THROWN_BLADE )
    {

        m_DiskRadius.Deactivate();
        m_BladeLength.Deactivate();
        m_BladeRootRadius.Deactivate();
        m_ThrownBladeCGFrac.Deactivate();

        m_FragLength.Deactivate();
        m_CGRadius.Deactivate();

        m_ThetaThrust.Deactivate();
        m_ThetaAntiThrust.Deactivate();

        if ( m_AuxuliaryGeomMode() == vsp::AUX_GEOM_ROTOR_FRAGMENT )
        {
            if ( m_RotorFragmentMode() == vsp::ONE_THIRD_ROTOR_FRAGMENT ||
                 m_RotorFragmentMode() == vsp::INTERMEDIATE_FRAGMENT ||
                 m_RotorFragmentMode() == vsp::ALTERNATE_FRAGMENT )
            {
                m_DiskRadius.Activate();
                m_BladeLength.Activate();
            }
            else if ( m_RotorFragmentMode() == vsp::FAN_FRAGMENT ||
                      m_RotorFragmentMode() == vsp::SMALL_FRAGMENT )
            {
                m_BladeLength.Activate();
                m_BladeRootRadius.Activate();
            }
            else if ( m_RotorFragmentMode() == vsp::GENERIC_FRAGMENT )
            {
                m_FragLength.Activate();
                m_CGRadius.Activate();

                m_ThetaThrust.Activate();
                m_ThetaAntiThrust.Activate();
            }
        }
        else // AUX_GEOM_THROWN_BLADE
        {
            if ( m_ThrownBladeMode() == vsp::PROP_BLADE_NONTRADITIONAL )
            {
                m_ThrownBladeCGFrac.Activate();

                m_ThetaThrust.Activate();
                m_ThetaAntiThrust.Activate();
            }
            // else // PROP_BLADE_TRADITIONAL
        }


        if ( m_AuxuliaryGeomMode() == vsp::AUX_GEOM_ROTOR_FRAGMENT )
        {
            // vsp::AUX_GEOM_GENERIC_FRAGMENT Skips this logic tree.
            if ( m_RotorFragmentMode() == vsp::ONE_THIRD_ROTOR_FRAGMENT ||
                 m_RotorFragmentMode() == vsp::ALTERNATE_FRAGMENT )
            {
                const double theta = 2.0 * M_PI / 3.0;

                const double r = m_DiskRadius() + m_BladeLength() / 3.0;
                m_FragLength = 2 * r * sin ( theta * 0.5 );
                m_CGRadius = r * cos ( theta * 0.5 );

                if ( m_RotorFragmentMode() == vsp::ONE_THIRD_ROTOR_FRAGMENT )
                {
                    m_ThetaThrust = 3;
                    m_ThetaAntiThrust = 3;
                }
                else // AUX_GEOM_ALTERNATE_FRAGMENT
                {
                    m_ThetaThrust = 5;
                    m_ThetaAntiThrust = 5;
                }
            }
            else if ( m_RotorFragmentMode() == vsp::INTERMEDIATE_FRAGMENT )
            {
                m_FragLength = ( m_DiskRadius() + m_BladeLength() ) / 3.0;
                m_CGRadius = m_DiskRadius();

                m_ThetaThrust = 5;
                m_ThetaAntiThrust = 5;
            }
            else if ( m_RotorFragmentMode() == vsp::FAN_FRAGMENT )
            {
                m_FragLength = m_BladeLength() / 3.0;
                m_CGRadius = m_BladeRootRadius() + m_BladeLength() * ( 5.0 / 6.0 );

                m_ThetaThrust = 15;
                m_ThetaAntiThrust = 15;
            }
            else if ( m_RotorFragmentMode() == vsp::SMALL_FRAGMENT )
            {
                m_FragLength = m_BladeLength() / 2.0;
                m_CGRadius = m_BladeRootRadius() + m_BladeLength() * ( 3.0 / 4.0 );

                m_ThetaThrust = 15;
                m_ThetaAntiThrust = 15;
            }
        }
        else
        {
            if ( m_ThrownBladeMode() == vsp::PROP_BLADE_TRADITIONAL )
            {
                m_ThetaThrust = 5;
                m_ThetaAntiThrust = 5;

                m_ThrownBladeCGFrac = 0.34;
            }
            // else // PROP_BLADE_NONTRADITIONAL

            if ( m_ParentType == PROP_GEOM_TYPE )
            {
                PropGeom * parent_prop = dynamic_cast< PropGeom* > ( parent_geom );

                if ( parent_prop )
                {
                    double r0 = parent_prop->GetR0();
                    double r = 0.5 * parent_prop->m_Diameter();

                    double bladelen = r * ( 1.0 - r0 );
                    m_FragLength = 2.0 * std::max( 1.0 - m_ThrownBladeCGFrac(), m_ThrownBladeCGFrac() ) * bladelen;
                    m_CGRadius = r0 * r + m_ThrownBladeCGFrac() * bladelen;

                    m_RotDir = !parent_prop->m_ReverseFlag();
                }
            }
        }

        const double c = m_FragLength();
        const double h = m_CGRadius();

        int flip = 1;
        if ( !m_RotDir() )
        {
            flip = -1;
        }

        vector < double > ts = { 0, 1.0, 2.0, 3.0, 4.0 };
        vector < vec3d > pts;
        pts.emplace_back( vec3d( 0.0, 0.0, h - 0.5 * c ) );
        pts.emplace_back( vec3d( 0.0, 0.0, h - 0.5 * c ) );
        pts.emplace_back( vec3d( 0.0, 0.0, h + 0.5 * c ) );
        pts.emplace_back( vec3d( 0.0, 0.0, h + 0.5 * c ) );
        pts.emplace_back( pts[ 0 ] );

        vector < VspCurve > crv_vec(3);
        crv_vec[0].InterpolateLinear( pts, ts, false );

        pts.clear();
        pts.emplace_back( vec3d( refLen * tan ( -m_ThetaThrust() * M_PI / 180.0 ), flip * refLen, h - 0.5 * c ) );
        pts.emplace_back( vec3d( refLen * tan ( m_ThetaAntiThrust() * M_PI / 180.0 ), flip * refLen, h - 0.5 * c ) );
        pts.emplace_back( vec3d( refLen * tan ( m_ThetaAntiThrust() * M_PI / 180.0 ), flip * refLen, h + 0.5 * c ) );
        pts.emplace_back( vec3d( refLen * tan ( -m_ThetaThrust() * M_PI / 180.0 ), flip * refLen, h + 0.5 * c ) );
        pts.emplace_back( pts[ 0 ] );

        crv_vec[1].InterpolateLinear( pts, ts, false );

        pts.clear();
        pts.emplace_back( vec3d( 0.0, flip * refLen, h ) );
        pts.emplace_back( vec3d( 0.0, flip * refLen, h ) );
        pts.emplace_back( vec3d( 0.0, flip * refLen, h ) );
        pts.emplace_back( vec3d( 0.0, flip * refLen, h ) );
        pts.emplace_back( pts[ 0 ] );

        crv_vec[2].InterpolateLinear( pts, ts, false );

        m_MainSurfVec[0].SkinC0( crv_vec, false );

        Matrix4d T;
        T.rotateX( -flip * m_ReleaseAngle() );
        m_MainSurfVec[0].Transform( T );

        if ( !m_RotDir() )
        {
            m_MainSurfVec[0].FlipNormal();
        }
    }

}

void AuxiliaryGeom::UpdateMainTessVec()
{
    if ( m_ParentType == GEAR_GEOM_TYPE &&
         m_AuxuliaryGeomMode() != vsp::AUX_GEOM_TIRE_SPRAY &&
         m_AuxuliaryGeomMode() != AUX_GEOM_WHEEL_TIRE_FAILURE )
    {
        int nmain = GetNumMainSurfs();

        m_MainTessVec.clear();
        m_MainFeatureTessVec.clear();

        m_MainTessVec.reserve( nmain );
        m_MainFeatureTessVec.reserve( nmain );

        m_MainTessVec.resize( 1 );
        m_MainFeatureTessVec.resize( 1 );

        // Update primary main surf tess.
        UpdateTess( m_MainSurfVec[ 0 ], m_CapUMinSuccess[ m_MainSurfIndxVec[ 0 ] ], m_CapUMaxSuccess[ m_MainSurfIndxVec[ 0 ] ], m_MainTessVec[ 0 ], m_MainFeatureTessVec[ 0 ] );

        Geom* parent_geom = m_Vehicle->FindGeom( m_ParentID );
        GearGeom * gear = dynamic_cast< GearGeom* > ( parent_geom );
        if ( gear )
        {
            if ( m_AuxuliaryGeomMode() == vsp::AUX_GEOM_THREE_PT_GROUND ||
                 m_AuxuliaryGeomMode() == vsp::AUX_GEOM_THREE_PT_CCE )
            {
                TessContact1( gear );
                TessContact2( gear );
                TessContact3( gear );
            }
            else if ( m_AuxuliaryGeomMode() == vsp::AUX_GEOM_TWO_PT_GROUND )
            {
                TessContact1( gear, m_BogieTheta() );
                TessContact2( gear, m_BogieTheta() );
            }
            else if ( m_AuxuliaryGeomMode() == vsp::AUX_GEOM_ONE_PT_GROUND )
            {
                TessContact1( gear, m_BogieTheta() );
            }
            else if ( m_AuxuliaryGeomMode() == vsp::AUX_GEOM_SINGLE_GEAR )
            {
                m_MainTessVec.clear();
                m_MainFeatureTessVec.clear();

                TessContact1( gear, m_BogieTheta() );
            }
        }
    }
    else
    {
        Geom::UpdateMainTessVec();
    }
}

void AuxiliaryGeom::UpdateMainDegenGeomPreview()
{
    if ( m_ParentType == GEAR_GEOM_TYPE &&
         m_AuxuliaryGeomMode() != vsp::AUX_GEOM_TIRE_SPRAY &&
         m_AuxuliaryGeomMode() != AUX_GEOM_WHEEL_TIRE_FAILURE )
    {
        int nmain = GetNumMainSurfs();

        m_MainDegenGeomPreviewVec.clear();
        m_MainDegenGeomPreviewVec.reserve( nmain );
        m_MainDegenGeomPreviewVec.resize( 1 );

        // Update primary main surf tess.
        CreateDegenGeom( m_MainSurfVec[ 0 ], 0, m_MainDegenGeomPreviewVec[ 0 ], true, 1 );

        Geom* parent_geom = m_Vehicle->FindGeom( m_ParentID );
        GearGeom * gear = dynamic_cast< GearGeom* > ( parent_geom );
        if ( gear )
        {
            if ( m_AuxuliaryGeomMode() == vsp::AUX_GEOM_THREE_PT_GROUND ||
                 m_AuxuliaryGeomMode() == vsp::AUX_GEOM_THREE_PT_CCE )
            {
                DegenContact1( gear );
                DegenContact2( gear );
                DegenContact3( gear );
            }
            else if ( m_AuxuliaryGeomMode() == vsp::AUX_GEOM_TWO_PT_GROUND )
            {
                DegenContact1( gear, m_BogieTheta() );
                DegenContact2( gear, m_BogieTheta() );
            }
            else if ( m_AuxuliaryGeomMode() == vsp::AUX_GEOM_ONE_PT_GROUND )
            {
                DegenContact1( gear, m_BogieTheta() );
            }
            else if ( m_AuxuliaryGeomMode() == vsp::AUX_GEOM_SINGLE_GEAR )
            {
                m_MainDegenGeomPreviewVec.clear();

                DegenContact1( gear, m_BogieTheta() );
            }
        }
    }
    else
    {
        Geom::UpdateMainDegenGeomPreview();
    }
}

void AuxiliaryGeom::UpdateCopyParms()
{
    //===== Find Parent ====//
    Geom* parent_geom = m_Vehicle->FindGeom( m_ParentID );
    if ( !parent_geom )
    {
        return;
    }

    //==== Force Attached So Auxiliary Moves With Parent =====//
    m_TransAttachFlag = vsp::ATTACH_TRANS_COMP;
    m_RotAttachFlag = vsp::ATTACH_ROT_COMP;

    m_TransAttachFlag.Deactivate();
    m_RotAttachFlag.Deactivate();

    m_ULoc.Deactivate();
    m_U0NLoc.Deactivate();
    m_U01.Deactivate();
    m_WLoc.Deactivate();
    m_RLoc.Deactivate();
    m_R01.Deactivate();
    m_R0NLoc.Deactivate();
    m_SLoc.Deactivate();
    m_TLoc.Deactivate();
    m_LLoc.Deactivate();
    m_L01.Deactivate();
    m_L0LenLoc.Deactivate();
    m_MLoc.Deactivate();
    m_NLoc.Deactivate();
    m_EtaLoc.Deactivate();

    // //==== Copy Cap Options ====//
    m_CapUMinOption.Deactivate();
    m_CapUMinTess.Deactivate();
    m_CapUMaxOption.Deactivate();

    m_CapUMinLength.Deactivate();
    m_CapUMinOffset.Deactivate();
    m_CapUMinStrength.Deactivate();
    m_CapUMinSweepFlag.Deactivate();

    m_CapUMaxLength.Deactivate();
    m_CapUMaxOffset.Deactivate();
    m_CapUMaxStrength.Deactivate();
    m_CapUMaxSweepFlag.Deactivate();

    //=== Let User Change Tess
    //m_TessU = parent_geom->m_TessU();
    //m_TessW = parent_geom->m_TessW();

    m_SymAncestor = parent_geom->m_SymAncestor();
    if ( m_SymAncestor() != 0 ) // Not global ancestor.
    {
        m_SymAncestor = m_SymAncestor() + 1;  // + 1 increment for parent
    }
    m_SymAncestOriginFlag = parent_geom->m_SymAncestOriginFlag();
    m_SymPlanFlag = parent_geom->m_SymPlanFlag();
    m_SymAxFlag = parent_geom->m_SymAxFlag();
    m_SymRotN = parent_geom->m_SymRotN();

    m_SymAncestor.Deactivate();
    m_SymAncestOriginFlag.Deactivate();
    m_SymPlanFlag.Deactivate();
    m_SymAxFlag.Deactivate();
    m_SymRotN.Deactivate();
}

bool AuxiliaryGeom::ReadCCEFile( const string &fname )
{
    FILE* file_id =  fopen( fname.c_str(), "r" );
    if ( !file_id )
    {
        return false;
    }

    if ( ReadCCEFile( file_id ) )
    {
        fclose( file_id );
        return true;
    }

    fclose( file_id );
    return false;
}

bool AuxiliaryGeom::ReadCCEFile( FILE* file_id )
{
    vector < vec3d > pnt_vec;

    if ( !file_id )
    {
        return false;
    }
    else
    {
        pnt_vec.clear();
        int stopFlag = 0;
        while ( !stopFlag )
        {
            double x, y, z;
            if ( EOF == fscanf( file_id, "%lf %lf\n", &x, &y ) )
            {
                break;
            }
            pnt_vec.push_back( vec3d( 0.0, x, y ) );
        }
        fclose( file_id );
    }

    if ( pnt_vec.size() == 0 )
    {
        return false;
    }

    SetPnts( pnt_vec );

    return true;
}

void AuxiliaryGeom::SetPnts( const vector<vec3d> &pnt_vec )
{
    if ( pnt_vec.size() >= 2 )
    {
        m_CCEFilePnts = pnt_vec;
    }
    else
    {
        m_CCEFilePnts.clear();
        m_CCEFilePnts.push_back( vec3d( -1000.0, 0.0, 0.0 ) );
        m_CCEFilePnts.push_back( vec3d( 1000.0, 0.0, 0.0 ) );
    }

    UpdateCCECurve();
}

void AuxiliaryGeom::UpdateCCECurve()
{
    int npts = m_CCEFilePnts.size();

    vector < double > arclen( npts );
    for ( int i = 0 ; i < npts ; i++ )
    {
        arclen[i] = i;
    }

    m_CCECurve.InterpolateLinear( m_CCEFilePnts, arclen, false );
}

string AuxiliaryGeom::GetNotes()
{
    switch ( m_AuxuliaryGeomMode() )
    {
        case vsp::AUX_GEOM_ROTOR_TIP_PATH:
        case vsp::AUX_GEOM_THROWN_BLADE:
            return string( "Rotor Tip Path and Thrown Blade Auxiliary Geoms must be children of a Prop Geom." );
            break;
        case vsp::AUX_GEOM_ROTOR_BURST:
        case vsp::AUX_GEOM_ROTOR_FRAGMENT:
            return string( "Rotor Burst and Fragment Auxiliary Geoms can be children of any Geom type." );
            break;
        case vsp::AUX_GEOM_THREE_PT_GROUND:
        case vsp::AUX_GEOM_TWO_PT_GROUND:
        case vsp::AUX_GEOM_ONE_PT_GROUND:
        case vsp::AUX_GEOM_SINGLE_GEAR:
        case vsp::AUX_GEOM_THREE_PT_CCE:
        case vsp::AUX_GEOM_TIRE_SPRAY:
        case vsp::AUX_GEOM_WHEEL_TIRE_FAILURE:
            return string( "1pt, 2pt, 3pt Ground Plane, Single Gear, 3pt CCE, Tire Spray, and Wheel and Tire Failure Auxiliary Geoms must be children of a Gear Geom." );
            break;
        case vsp::AUX_GEOM_SUPER_CONE:
            return string( "Super Cone Auxiliary Geoms can be children of any Geom type.  "
                           "When the Super Cone is a child of a Human Geom, it will attach to and align with the design eye point.  "
                           "For all other parent types, it will attach to and align with the component coordinate system.  "
                           "Use a Blank Geom if more flexible attachment is required." );
            break;
        default:
            break;
    }
    return string( "" );
}

void AuxiliaryGeom::SetXSecCurveType( int type )
{
    double w = 1;
    double h = 1;

    XSecCurve *oldXSCurve = m_XSCurve;

    if ( m_XSCurve )
    {
        if ( type == m_XSCurve->GetType() )
        {
            return;
        }

        w = m_XSCurve->GetWidth();
        h = m_XSCurve->GetHeight();
    }

    m_XSCurve = XSecSurf::CreateXSecCurve( type ) ;

    if ( m_XSCurve )
    {
        m_XSCurve->SetParentContainer( m_ID );

        if ( oldXSCurve )
        {
            m_XSCurve->CopyFrom( oldXSCurve );
            delete oldXSCurve;
        }

        m_XSCurve->SetWidthHeight( w, h );
    }
    else  // Failed to create new curve, revert to saved.
    {
        m_XSCurve = oldXSCurve;
    }

    m_XFormDirty = true;
    m_SurfDirty = true;

    Update();
}

int AuxiliaryGeom::GetXSecCurveType()
{
    return m_XSCurve->GetType();
}

EditCurveXSec* AuxiliaryGeom::ConvertToEdit()
{
    EditCurveXSec* xscrv_ptr = m_XSCurve->ConvertToEdit();

    if ( xscrv_ptr && xscrv_ptr != m_XSCurve )
    {
        delete m_XSCurve;

        m_XSCurve = xscrv_ptr;
        m_XSCurve->SetParentContainer( m_ID );
    }

    return xscrv_ptr;
}

void AuxiliaryGeom::UpdateFlags()
{
    // Type 1 tire failure geom.  Ignore m_NegativeVolumeFlag and use hard-wired definitions.
    if ( m_AuxuliaryGeomMode() == vsp::AUX_GEOM_WHEEL_TIRE_FAILURE &&
         ( m_WheelTireFailureMode() == vsp::WHEEL_TIRE_1LG ||
           m_WheelTireFailureMode() == vsp::WHEEL_TIRE_1SM ) )
    {
    }
    else // All others.
    {
        Geom::UpdateFlags();
    }
}

void AuxiliaryGeom::UpdateDrawObj()
{
    Geom::UpdateDrawObj();

    //=== Axis ===//
    int n = m_BasisAxis.size();
    m_BasisDrawObj_vec.clear();
    m_BasisDrawObj_vec.resize( n );
    for ( int i = 0; i < n; i++ )
    {
        MakeDashedLine( m_BasisOrigin,  m_BasisAxis[i], 4, m_BasisDrawObj_vec[i].m_PntVec );
        vec3d c;
        c.v[i] = 1.0;
        m_BasisDrawObj_vec[i].m_LineColor = c;
        m_BasisDrawObj_vec[i].m_GeomChanged = true;
    }

    m_ContactDrawObj.m_PntVec = m_ContactPts;

    m_ContactDrawObj.m_GeomID = m_ID + "Contact";
    m_ContactDrawObj.m_Screen = DrawObj::VSP_MAIN_SCREEN;
    m_ContactDrawObj.m_PointSize = 12.0;
    m_ContactDrawObj.m_Type = DrawObj::VSP_POINTS;
    m_ContactDrawObj.m_PointColor = vec3d( 0, 0, 1 );
    m_ContactDrawObj.m_GeomChanged = true;
}


void AuxiliaryGeom::LoadDrawObjs( vector< DrawObj* > & draw_obj_vec )
{
    Geom::LoadDrawObjs( draw_obj_vec );

    char str[255];
    bool isactive = m_Vehicle->IsGeomActive( m_ID );

    for ( int i = 0; i < m_BasisDrawObj_vec.size(); i++ )
    {
        m_BasisDrawObj_vec[i].m_Screen = DrawObj::VSP_MAIN_SCREEN;
        snprintf( str, sizeof( str ),  "_%d", i );
        m_BasisDrawObj_vec[i].m_GeomID = m_ID + "Axis_" + str;
        m_BasisDrawObj_vec[i].m_Visible = isactive;
        m_BasisDrawObj_vec[i].m_LineWidth = 10.0;
        m_BasisDrawObj_vec[i].m_Type = DrawObj::VSP_LINES;
        // Debug visualization of basis for two-point and one-point landing gear.
        // draw_obj_vec.push_back( &m_BasisDrawObj_vec[i] );
    }


    // Debug visualization of contact points for two-point landing gear only.
    // draw_obj_vec.push_back( &m_ContactDrawObj );
}

void AuxiliaryGeom::AppendContact1Surfs( GearGeom * gear, double bogietheta )
{
    if ( gear )
    {
        Bogie *b1 = gear->GetBogie( m_ContactPt1_ID );
        if ( b1 )
        {
            bool stow = false;
            bool ret = false;
            double kretract = 0.0;
            if ( m_ContactPt1_GearMode() == vsp::GEAR_CONFIGURATION_UP )
            {
                if ( b1->m_RetMode() == vsp::GEAR_STOWED_POSITION )
                {
                    stow = true;
                }
                else
                {
                    ret = true;
                    kretract = 1.0;
                }
            }
            else if ( m_ContactPt1_GearMode() == vsp::GEAR_CONFIGURATION_INTERMEDIATE )
            {
                ret = true;
                kretract = m_ContactPt1_KRetract();
            }

            if ( m_ContactPt1_ClearanceMode() == vsp::TIRE_GROWTH_CLEARANCE )
            {
                b1->TireToBogie( b1->m_GrownTireSurface, m_MainSurfVec, m_ContactPt1_Isymm(), m_ContactPt1_SuspensionMode(), bogietheta, stow, ret, kretract );
            }
            else if ( m_ContactPt1_ClearanceMode() == vsp::TIRE_CLEARANCE )
            {
                b1->TireToBogie( b1->m_ClearanceSurface, m_MainSurfVec, m_ContactPt1_Isymm(), m_ContactPt1_SuspensionMode(), bogietheta, stow, ret, kretract );
            }
            else
            {
                b1->TireToBogie( b1->m_TireSurface, m_MainSurfVec, m_ContactPt1_Isymm(), m_ContactPt1_SuspensionMode(), bogietheta, stow, ret, kretract );
            }
        }
    }
}

void AuxiliaryGeom::AppendContact2Surfs( GearGeom * gear, double bogietheta )
{
    if ( gear )
    {
        Bogie *b2 = gear->GetBogie( m_ContactPt2_ID );
        if ( b2 )
        {
            if ( m_ContactPt1_ClearanceMode() == vsp::TIRE_GROWTH_CLEARANCE )
            {
                b2->TireToBogie( b2->m_GrownTireSurface, m_MainSurfVec, m_ContactPt2_Isymm(), m_ContactPt2_SuspensionMode(), bogietheta );
            }
            else if ( m_ContactPt1_ClearanceMode() == vsp::TIRE_CLEARANCE )
            {
                b2->TireToBogie( b2->m_ClearanceSurface, m_MainSurfVec, m_ContactPt2_Isymm(), m_ContactPt2_SuspensionMode(), bogietheta );
            }
            else
            {
                b2->TireToBogie( b2->m_TireSurface, m_MainSurfVec, m_ContactPt2_Isymm(), m_ContactPt2_SuspensionMode(), bogietheta );
            }
        }
    }
}

void AuxiliaryGeom::AppendContact3Surfs( GearGeom * gear, double bogietheta )
{
    if ( gear )
    {
        Bogie *b3 = gear->GetBogie( m_ContactPt3_ID );
        if ( b3 )
        {
            if ( m_ContactPt1_ClearanceMode() == vsp::TIRE_GROWTH_CLEARANCE )
            {
                b3->TireToBogie( b3->m_GrownTireSurface, m_MainSurfVec, m_ContactPt3_Isymm(), m_ContactPt3_SuspensionMode(), bogietheta );
            }
            else if ( m_ContactPt1_ClearanceMode() == vsp::TIRE_CLEARANCE )
            {
                b3->TireToBogie( b3->m_ClearanceSurface, m_MainSurfVec, m_ContactPt3_Isymm(), m_ContactPt3_SuspensionMode(), bogietheta );
            }
            else
            {
                b3->TireToBogie( b3->m_TireSurface, m_MainSurfVec, m_ContactPt3_Isymm(), m_ContactPt3_SuspensionMode(), bogietheta );
            }
        }
    }
}

void AuxiliaryGeom::TessContact1( GearGeom * gear, double bogietheta )
{
    if ( gear )
    {
        Bogie *b1 = gear->GetBogie( m_ContactPt1_ID );
        if ( b1 )
        {
            bool stow = false;
            bool ret = false;
            double kretract = 0.0;
            if ( m_ContactPt1_GearMode() == vsp::GEAR_CONFIGURATION_UP )
            {
                if ( b1->m_RetMode() == vsp::GEAR_STOWED_POSITION )
                {
                    stow = true;
                }
                else
                {
                    ret = true;
                    kretract = 1.0;
                }
            }
            else if ( m_ContactPt1_GearMode() == vsp::GEAR_CONFIGURATION_INTERMEDIATE )
            {
                ret = true;
                kretract = m_ContactPt1_KRetract();
            }

            if ( m_ContactPt1_ClearanceMode() == vsp::TIRE_GROWTH_CLEARANCE )
            {
                b1->TireToBogie( b1->m_GrownTireTess, m_MainTessVec, m_ContactPt1_Isymm(), m_ContactPt1_SuspensionMode(), bogietheta, stow, ret, kretract );
                b1->TireToBogie( b1->m_GrownTireFeatureTess, m_MainFeatureTessVec, m_ContactPt1_Isymm(), m_ContactPt1_SuspensionMode(), bogietheta, stow, ret, kretract );
            }
            else if ( m_ContactPt1_ClearanceMode() == vsp::TIRE_CLEARANCE )
            {
                b1->TireToBogie( b1->m_ClearanceTess, m_MainTessVec, m_ContactPt1_Isymm(), m_ContactPt1_SuspensionMode(), bogietheta, stow, ret, kretract );
                b1->TireToBogie( b1->m_ClearanceFeatureTess, m_MainFeatureTessVec, m_ContactPt1_Isymm(), m_ContactPt1_SuspensionMode(), bogietheta, stow, ret, kretract );
            }
            else
            {
                b1->TireToBogie( b1->m_TireTess, m_MainTessVec, m_ContactPt1_Isymm(), m_ContactPt1_SuspensionMode(), bogietheta, stow, ret, kretract );
                b1->TireToBogie( b1->m_TireFeatureTess, m_MainFeatureTessVec, m_ContactPt1_Isymm(), m_ContactPt1_SuspensionMode(), bogietheta, stow, ret, kretract );
            }
        }
    }
}

void AuxiliaryGeom::TessContact2( GearGeom * gear, double bogietheta )
{
    if ( gear )
    {
        Bogie *b2 = gear->GetBogie( m_ContactPt2_ID );
        if ( b2 )
        {
            if ( m_ContactPt1_ClearanceMode() == vsp::TIRE_GROWTH_CLEARANCE )
            {
                b2->TireToBogie( b2->m_GrownTireTess, m_MainTessVec, m_ContactPt2_Isymm(), m_ContactPt2_SuspensionMode(), bogietheta );
                b2->TireToBogie( b2->m_GrownTireFeatureTess, m_MainFeatureTessVec, m_ContactPt2_Isymm(), m_ContactPt2_SuspensionMode(), bogietheta );
            }
            else if ( m_ContactPt1_ClearanceMode() == vsp::TIRE_CLEARANCE )
            {
                b2->TireToBogie( b2->m_ClearanceTess, m_MainTessVec, m_ContactPt2_Isymm(), m_ContactPt2_SuspensionMode(), bogietheta );
                b2->TireToBogie( b2->m_ClearanceFeatureTess, m_MainFeatureTessVec, m_ContactPt2_Isymm(), m_ContactPt2_SuspensionMode(), bogietheta );
            }
            else
            {
                b2->TireToBogie( b2->m_TireTess, m_MainTessVec, m_ContactPt2_Isymm(), m_ContactPt2_SuspensionMode(), bogietheta );
                b2->TireToBogie( b2->m_TireFeatureTess, m_MainFeatureTessVec, m_ContactPt2_Isymm(), m_ContactPt2_SuspensionMode(), bogietheta );
            }
        }
    }
}

void AuxiliaryGeom::TessContact3( GearGeom * gear, double bogietheta )
{
    if ( gear )
    {
        Bogie *b3 = gear->GetBogie( m_ContactPt3_ID );
        if ( b3 )
        {
            if ( m_ContactPt1_ClearanceMode() == vsp::TIRE_GROWTH_CLEARANCE )
            {
                b3->TireToBogie( b3->m_GrownTireTess, m_MainTessVec, m_ContactPt3_Isymm(), m_ContactPt3_SuspensionMode(), bogietheta );
                b3->TireToBogie( b3->m_GrownTireFeatureTess, m_MainFeatureTessVec, m_ContactPt3_Isymm(), m_ContactPt3_SuspensionMode(), bogietheta );
            }
            else if ( m_ContactPt1_ClearanceMode() == vsp::TIRE_CLEARANCE )
            {
                b3->TireToBogie( b3->m_ClearanceTess, m_MainTessVec, m_ContactPt3_Isymm(), m_ContactPt3_SuspensionMode(), bogietheta );
                b3->TireToBogie( b3->m_ClearanceFeatureTess, m_MainFeatureTessVec, m_ContactPt3_Isymm(), m_ContactPt3_SuspensionMode(), bogietheta );
            }
            else
            {
                b3->TireToBogie( b3->m_TireTess, m_MainTessVec, m_ContactPt3_Isymm(), m_ContactPt3_SuspensionMode(), bogietheta );
                b3->TireToBogie( b3->m_TireFeatureTess, m_MainFeatureTessVec, m_ContactPt3_Isymm(), m_ContactPt3_SuspensionMode(), bogietheta );
            }
        }
    }
}

void AuxiliaryGeom::DegenContact1( GearGeom * gear, double bogietheta )
{
    if ( gear )
    {
        Bogie *b1 = gear->GetBogie( m_ContactPt1_ID );
        if ( b1 )
        {
            bool stow = false;
            bool ret = false;
            double kretract = 0.0;
            if ( m_ContactPt1_GearMode() == vsp::GEAR_CONFIGURATION_UP )
            {
                if ( b1->m_RetMode() == vsp::GEAR_STOWED_POSITION )
                {
                    stow = true;
                }
                else
                {
                    ret = true;
                    kretract = 1.0;
                }
            }
            else if ( m_ContactPt1_GearMode() == vsp::GEAR_CONFIGURATION_INTERMEDIATE )
            {
                ret = true;
                kretract = m_ContactPt1_KRetract();
            }

            DegenGeom degenGeom;
            if ( m_ContactPt1_ClearanceMode() == vsp::TIRE_GROWTH_CLEARANCE )
            {
                CreateDegenGeom( b1->m_GrownTireSurface, 0, degenGeom, true, 1 );
            }
            else if ( m_ContactPt1_ClearanceMode() == vsp::TIRE_CLEARANCE )
            {
                CreateDegenGeom( b1->m_ClearanceSurface, 0, degenGeom, true, 1 );
            }
            else
            {
                CreateDegenGeom( b1->m_TireSurface, 0, degenGeom, true, 1 );
            }

            b1->TireToBogie( degenGeom, m_MainDegenGeomPreviewVec, m_ContactPt1_Isymm(), m_ContactPt1_SuspensionMode(), bogietheta, stow, ret, kretract );
        }
    }
}

void AuxiliaryGeom::DegenContact2( GearGeom * gear, double bogietheta )
{
    if ( gear )
    {
        Bogie *b2 = gear->GetBogie( m_ContactPt2_ID );
        if ( b2 )
        {
            DegenGeom degenGeom;
            if ( m_ContactPt1_ClearanceMode() == vsp::TIRE_GROWTH_CLEARANCE )
            {
                CreateDegenGeom( b2->m_GrownTireSurface, 0, degenGeom, true, 1 );
            }
            else if ( m_ContactPt1_ClearanceMode() == vsp::TIRE_CLEARANCE )
            {
                CreateDegenGeom( b2->m_ClearanceSurface, 0, degenGeom, true, 1 );
            }
            else
            {
                CreateDegenGeom( b2->m_TireSurface, 0, degenGeom, true, 1 );
            }

            b2->TireToBogie( degenGeom, m_MainDegenGeomPreviewVec, m_ContactPt2_Isymm(), m_ContactPt2_SuspensionMode(), bogietheta );
        }
    }
}

void AuxiliaryGeom::DegenContact3( GearGeom * gear, double bogietheta )
{
    if ( gear )
    {
        Bogie *b3 = gear->GetBogie( m_ContactPt3_ID );
        if ( b3 )
        {
            DegenGeom degenGeom;
            if ( m_ContactPt1_ClearanceMode() == vsp::TIRE_GROWTH_CLEARANCE )
            {
                CreateDegenGeom( b3->m_GrownTireSurface, 0, degenGeom, true, 1 );
            }
            else if ( m_ContactPt1_ClearanceMode() == vsp::TIRE_CLEARANCE )
            {
                CreateDegenGeom( b3->m_ClearanceSurface, 0, degenGeom, true, 1 );
            }
            else
            {
                CreateDegenGeom( b3->m_TireSurface, 0, degenGeom, true, 1 );
            }

            b3->TireToBogie( degenGeom, m_MainDegenGeomPreviewVec, m_ContactPt3_Isymm(), m_ContactPt3_SuspensionMode(), bogietheta );
        }
    }
}

void AuxiliaryGeom::UpdateBBox( )
{
    // Fill m_BBox like normal
    Geom::UpdateBBox();

    // Reset m_ScaleIndependentBBox to empty
    m_ScaleIndependentBBox.Reset();
}

xmlNodePtr AuxiliaryGeom::EncodeXml( xmlNodePtr & node )
{
    xmlNodePtr geom_node = Geom::EncodeXml( node );

    xmlNodePtr child_node = xmlNewChild( geom_node, NULL, BAD_CAST "Auxiliary", NULL );

    if ( child_node )
    {
        XmlUtil::AddStringNode( child_node, "ContactPt1_ID", m_ContactPt1_ID );
        XmlUtil::AddStringNode( child_node, "ContactPt2_ID", m_ContactPt2_ID );
        XmlUtil::AddStringNode( child_node, "ContactPt3_ID", m_ContactPt3_ID );

        XmlUtil::AddVectorVec3dNode( child_node, "CCEFilePnts", m_CCEFilePnts );

        xmlNodePtr xscrv_node = xmlNewChild( child_node, NULL, BAD_CAST "XSecCurve", NULL );
        if ( xscrv_node )
        {
            m_XSCurve->EncodeXml( xscrv_node );
        }
    }

    return child_node;
}

xmlNodePtr AuxiliaryGeom::DecodeXml( xmlNodePtr & node )
{
    xmlNodePtr geom_node = Geom::DecodeXml( node );

    xmlNodePtr child_node = XmlUtil::GetNode( geom_node, "Auxiliary", 0 );

    if ( child_node )
    {
        m_ContactPt1_ID = ParmMgr.RemapID( XmlUtil::FindString( child_node, "ContactPt1_ID", m_ContactPt1_ID ) );
        m_ContactPt2_ID = ParmMgr.RemapID( XmlUtil::FindString( child_node, "ContactPt2_ID", m_ContactPt2_ID ) );
        m_ContactPt3_ID = ParmMgr.RemapID( XmlUtil::FindString( child_node, "ContactPt3_ID", m_ContactPt3_ID ) );

        vector < vec3d > pnt_vec = XmlUtil::ExtractVectorVec3dNode( child_node, "CCEFilePnts" );
        SetPnts( pnt_vec );

        xmlNodePtr xscrv_node = XmlUtil::GetNode( child_node, "XSecCurve", 0 );
        if ( xscrv_node )
        {

            xmlNodePtr xscrv_node2 = XmlUtil::GetNode( xscrv_node, "XSecCurve", 0 );
            if ( xscrv_node2 )
            {

                int xsc_type = XmlUtil::FindInt( xscrv_node2, "Type", vsp::XS_CIRCLE );

                if ( m_XSCurve )
                {
                    if ( m_XSCurve->GetType() != xsc_type )
                    {
                        delete m_XSCurve;

                        m_XSCurve = XSecSurf::CreateXSecCurve( xsc_type );
                        m_XSCurve->SetParentContainer( m_ID );
                    }
                }
            }

            if ( m_XSCurve )
            {
                m_XSCurve->DecodeXml( xscrv_node );
            }
        }
    }

    return child_node;
}

//==== Look Though All Parms and Load Linkable Ones ===//
void AuxiliaryGeom::AddLinkableParms( vector< string > & parm_vec, const string & link_container_id )
{
    Geom::AddLinkableParms( parm_vec );

    if ( m_XSCurve  )
    {
        m_XSCurve->AddLinkableParms( parm_vec, m_ID );
    }
}

void AuxiliaryGeom::SetContactPt1ID( const std::string& id )
{
    m_ContactPt1_ID = id;
    m_SurfDirty = true;
    Update();
}

void AuxiliaryGeom::SetContactPt2ID( const std::string& id )
{
    m_ContactPt2_ID = id;
    m_SurfDirty = true;
    Update();
}

void AuxiliaryGeom::SetContactPt3ID( const std::string& id )
{
    m_ContactPt3_ID = id;
    m_SurfDirty = true;
    Update();
}

bool AuxiliaryGeom::GetCG( vec3d &cgnom, vector < vec3d > &cgbounds )
{
    if ( m_AuxuliaryGeomMode() == vsp::AUX_GEOM_THREE_PT_GROUND ||
         m_AuxuliaryGeomMode() == vsp::AUX_GEOM_TWO_PT_GROUND ||
         m_AuxuliaryGeomMode() == vsp::AUX_GEOM_ONE_PT_GROUND ||
         m_AuxuliaryGeomMode() == vsp::AUX_GEOM_THREE_PT_CCE )
    {
        Geom* parent_geom = m_Vehicle->FindGeom( m_ParentID );

        GearGeom * gear = dynamic_cast< GearGeom* > ( parent_geom );
        if ( gear )
        {
            gear->GetCGInWorld( cgnom, cgbounds );
            return true;
        }
    }
    return false;
}

bool AuxiliaryGeom::GetPtNormal( vec3d &pt, vec3d &normal ) const
{
    if ( m_AuxuliaryGeomMode() == vsp::AUX_GEOM_THREE_PT_GROUND ||
         m_AuxuliaryGeomMode() == vsp::AUX_GEOM_THREE_PT_CCE )
    {
        Geom* parent_geom = m_Vehicle->FindGeom( m_ParentID );

        GearGeom * gear = dynamic_cast< GearGeom* > ( parent_geom );
        if ( gear )
        {
            gear->GetPtNormalInWorld( m_ContactPt1_ID, m_ContactPt1_Isymm(), m_ContactPt1_SuspensionMode(), m_ContactPt1_TireMode(),
                                      m_ContactPt2_ID, m_ContactPt2_Isymm(), m_ContactPt2_SuspensionMode(), m_ContactPt2_TireMode(),
                                      m_ContactPt3_ID, m_ContactPt3_Isymm(), m_ContactPt3_SuspensionMode(), m_ContactPt3_TireMode(),
                                      pt, normal );
            return true;
        }
    }
    return false;
}

bool AuxiliaryGeom::GetPtPivotAxis( vec3d &ptaxis, vec3d &axis )
{
    if ( m_AuxuliaryGeomMode() == vsp::AUX_GEOM_TWO_PT_GROUND )
    {
        Geom* parent_geom = m_Vehicle->FindGeom( m_ParentID );

        GearGeom * gear = dynamic_cast< GearGeom* > ( parent_geom );
        if ( gear )
        {
            gear->GetTwoPtPivotInWorld( m_ContactPt1_ID, m_ContactPt1_Isymm(), m_ContactPt1_SuspensionMode(),
                                        m_ContactPt2_ID, m_ContactPt2_Isymm(), m_ContactPt2_SuspensionMode(),
                                        ptaxis, axis );
            return true;
        }
    }
    return false;
}

bool AuxiliaryGeom::GetPtNormalMeanContactPtPivotAxis( vec3d &pt, vec3d &normal, vec3d &ptaxis, vec3d &axis, bool &usepivot, double &mintheta, double &maxtheta )
{
    if ( m_AuxuliaryGeomMode() == vsp::AUX_GEOM_TWO_PT_GROUND )
    {
        Geom* parent_geom = m_Vehicle->FindGeom( m_ParentID );

        GearGeom * gear = dynamic_cast< GearGeom* > ( parent_geom );
        if ( gear )
        {
            gear->GetTwoPtMeanContactPtNormalInWorld( m_ContactPt1_ID, m_ContactPt1_Isymm(), m_ContactPt1_SuspensionMode(), m_ContactPt1_TireMode(),
                                                      m_ContactPt2_ID, m_ContactPt2_Isymm(), m_ContactPt2_SuspensionMode(), m_ContactPt2_TireMode(),
                                                      0, pt, normal, usepivot, mintheta, maxtheta );


            gear->GetTwoPtPivotInWorld( m_ContactPt1_ID, m_ContactPt1_Isymm(), m_ContactPt1_SuspensionMode(),
                                        m_ContactPt2_ID, m_ContactPt2_Isymm(), m_ContactPt2_SuspensionMode(),
                                        ptaxis, axis );
            return true;
        }
    }
    return false;
}

bool AuxiliaryGeom::GetSideContactPtRollAxisNormal( vec3d &pt, vec3d &axis, vec3d &normal, int &ysign )
{
    if ( m_AuxuliaryGeomMode() == vsp::AUX_GEOM_ONE_PT_GROUND )
    {
        Geom* parent_geom = m_Vehicle->FindGeom( m_ParentID );

        GearGeom * gear = dynamic_cast< GearGeom* > ( parent_geom );
        if ( gear )
        {
            gear->GetOnePtSideContactPtAxisNormalInWorld( m_ContactPt1_ID, m_ContactPt1_Isymm(), m_ContactPt1_SuspensionMode(), m_ContactPt1_TireMode(),
                                                          m_BogieTheta() * M_PI / 180.0, m_WheelTheta() * M_PI / 180, 0, pt, axis, normal, ysign);
            return true;
        }
    }
    return false;
}

bool AuxiliaryGeom::GetPtNormalAftAxleAxis( double thetabogie, vec3d &pt, vec3d &normal, vec3d &ptaxis, vec3d &axis )
{
    if ( m_AuxuliaryGeomMode() == vsp::AUX_GEOM_TWO_PT_GROUND )
    {
        Geom* parent_geom = m_Vehicle->FindGeom( m_ParentID );

        GearGeom * gear = dynamic_cast< GearGeom* > ( parent_geom );
        if ( gear )
        {
            gear->GetTwoPtAftContactPtNormalInWorld( m_ContactPt1_ID, m_ContactPt1_Isymm(), m_ContactPt1_SuspensionMode(), m_ContactPt1_TireMode(),
                                                     m_ContactPt2_ID, m_ContactPt2_Isymm(), m_ContactPt2_SuspensionMode(), m_ContactPt2_TireMode(),
                                                     thetabogie, 0, pt, normal );


            gear->GetTwoPtAftAxleAxisInWorld( m_ContactPt1_ID, m_ContactPt1_Isymm(), m_ContactPt1_SuspensionMode(),
                                              m_ContactPt2_ID, m_ContactPt2_Isymm(), m_ContactPt2_SuspensionMode(),
                                              thetabogie, ptaxis, axis );
            return true;
        }
    }
    return false;
}

bool AuxiliaryGeom::GetPtNormalFwdAxleAxis( double thetabogie, vec3d &pt, vec3d &normal, vec3d &ptaxis, vec3d &axis )
{
    if ( m_AuxuliaryGeomMode() == vsp::AUX_GEOM_TWO_PT_GROUND )
    {
        Geom* parent_geom = m_Vehicle->FindGeom( m_ParentID );

        GearGeom * gear = dynamic_cast< GearGeom* > ( parent_geom );
        if ( gear )
        {
            gear->GetTwoPtFwdContactPtNormalInWorld( m_ContactPt1_ID, m_ContactPt1_Isymm(), m_ContactPt1_SuspensionMode(), m_ContactPt1_TireMode(),
                                                     m_ContactPt2_ID, m_ContactPt2_Isymm(), m_ContactPt2_SuspensionMode(), m_ContactPt2_TireMode(),
                                                     thetabogie, 0, pt, normal );


            gear->GetTwoPtFwdAxleAxisInWorld( m_ContactPt1_ID, m_ContactPt1_Isymm(), m_ContactPt1_SuspensionMode(),
                                              m_ContactPt2_ID, m_ContactPt2_Isymm(), m_ContactPt2_SuspensionMode(),
                                              thetabogie, ptaxis, axis );
            return true;
        }
    }
    return false;
}

bool AuxiliaryGeom::GetTwoPtSideContactPtsNormal( vec3d &p1, vec3d &p2, vec3d &normal )
{
    if ( m_AuxuliaryGeomMode() == vsp::AUX_GEOM_TWO_PT_GROUND )
    {
        Geom* parent_geom = m_Vehicle->FindGeom( m_ParentID );

        GearGeom * gear = dynamic_cast< GearGeom* > ( parent_geom );
        if ( gear )
        {
            gear->GetTwoPtSideContactPtsNormalInWorld( m_ContactPt1_ID, m_ContactPt1_Isymm(), m_ContactPt1_SuspensionMode(), m_ContactPt1_TireMode(),
                                                     m_ContactPt2_ID, m_ContactPt2_Isymm(), m_ContactPt2_SuspensionMode(), m_ContactPt2_TireMode(),
                                                     p1, p2, normal );
            return true;
        }
    }
    return false;
}

bool AuxiliaryGeom::GetContactPointVecNormal( vector < vec3d > &ptvec, vec3d &normal )
{
    if ( m_AuxuliaryGeomMode() == vsp::AUX_GEOM_THREE_PT_GROUND ||
         m_AuxuliaryGeomMode() == vsp::AUX_GEOM_THREE_PT_CCE )
    {
        Geom* parent_geom = m_Vehicle->FindGeom( m_ParentID );

        GearGeom * gear = dynamic_cast< GearGeom* > ( parent_geom );
        if ( gear )
        {
            gear->GetContactPointVecNormalInWorld( m_ContactPt1_ID, m_ContactPt1_Isymm(), m_ContactPt1_SuspensionMode(), m_ContactPt1_TireMode(),
                                                   m_ContactPt2_ID, m_ContactPt2_Isymm(), m_ContactPt2_SuspensionMode(), m_ContactPt2_TireMode(),
                                                   m_ContactPt3_ID, m_ContactPt3_Isymm(), m_ContactPt3_SuspensionMode(), m_ContactPt3_TireMode(),
                                                   ptvec, normal );
            return true;
        }
    }
    return false;
}

bool AuxiliaryGeom::CalculateTurn( vec3d &cor, vec3d &normal, vector<double> &rvec )
{
    if ( m_AuxuliaryGeomMode() == vsp::AUX_GEOM_THREE_PT_GROUND ||
         m_AuxuliaryGeomMode() == vsp::AUX_GEOM_THREE_PT_CCE )
    {
        Geom* parent_geom = m_Vehicle->FindGeom( m_ParentID );

        GearGeom * gear = dynamic_cast< GearGeom* > ( parent_geom );
        if ( gear )
        {
            vector < vec3d > ptvec;
            gear->GetContactPointVecNormal( m_ContactPt1_ID, m_ContactPt1_Isymm(), m_ContactPt1_SuspensionMode(), m_ContactPt1_TireMode(),
                                            m_ContactPt2_ID, m_ContactPt2_Isymm(), m_ContactPt2_SuspensionMode(), m_ContactPt2_TireMode(),
                                            m_ContactPt3_ID, m_ContactPt3_Isymm(), m_ContactPt3_SuspensionMode(), m_ContactPt3_TireMode(),
                                            ptvec, normal );

            int isteer;
            double steerangle;
            gear->GetSteerAngle( m_ContactPt1_ID, m_ContactPt2_ID, m_ContactPt3_ID, isteer, steerangle );

            // Identify normal wheels as non-steering wheels.
            vector < int > inormal = {0, 1, 2};
            vector_remove_val( inormal, isteer );

            // Construct axle axis between two 'normal' wheels.
            vec3d axle = ptvec[ inormal[0] ] - ptvec[ inormal[ 1 ] ];
            axle.normalize();

            // Directions in nominal gear coordinates.
            vec3d up( 0, 0, 1 );
            // Steering axle before rotating.
            vec3d steeraxle( 0, 1, 0 );

            double steerideal = 0;
            for( int i = 0; i < ptvec.size(); ++i )
            {
                if ( i != isteer )
                {
                    // better to do some sort of projection / rejection to find angle in plane of rotation of gear.
                    vec3d v = ptvec[ i ] - ptvec[ isteer ];

                    vec3d vperp = proj_u_on_v( v, up );
                    vec3d vpar = v - vperp;

                    double si = signed_angle( vpar, steeraxle, up );

                    if ( si > M_PI * 0.5 )
                    {
                        si -= M_PI;
                    }

                    if ( std::abs( si ) > steerideal )
                    {
                        steerideal = si;
                    }
                }
            }

            // If steerable bogie is capable of turning more than ideal angle, limit turn to ideal angle.
            if ( steerangle > std::abs( steerideal ) )
            {
                steerangle = steerideal;
            }
            else // Turn in direction of ideal angle.
            {
                steerangle *= sgn( steerideal );
            }

            steeraxle.rotate_z( steerangle );

            vec3d steerperp = proj_u_on_v( steeraxle, normal );
            vec3d steerpar = steeraxle - steerperp;

            // Need to project rotated steeraxle into plane.

            cor.set_xyz( 0, 0, 0 );
            for( int i = 0; i < ptvec.size(); ++i )
            {
                vec3d psteer1 = ptvec[isteer];
                vec3d psteer2 = psteer1 + steerpar;

                if ( i != isteer )
                {
                    vec3d p1 = ptvec[i];
                    vec3d p2 = p1 + axle;

                    double s, t;
                    line_line_intersect( psteer1, psteer2, p1, p2, &s, &t );
                    vec3d pint = psteer1 + s * steerpar;

                    cor = cor + pint;
                }
            }
            cor = cor * 0.5;

            rvec.resize( ptvec.size() );
            for( int i = 0; i < ptvec.size(); ++i )
            {
                rvec[i] = dist( ptvec[ i ], cor );
            }

            // Return results in world coordinates
            Matrix4d world = gear->getModelMatrix();
            normal = world.xformnorm( normal );
            cor = world.xform( cor );

            return true;
        }
    }
    return false;
}
