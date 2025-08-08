//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

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

    m_ContactPt1_Isymm.Init( "ContactPt1_Isymm", "Design", this, 0, 0, 1 );
    m_ContactPt1_SuspensionMode.Init( "ContactPt1_SuspensionMode", "Design", this, vsp::GEAR_SUSPENSION_NOMINAL, vsp::GEAR_SUSPENSION_NOMINAL, vsp::NUM_GEAR_SUSPENSION_MODES - 1 );
    m_ContactPt1_TireMode.Init( "ContactPt1_TireMode", "Design", this, vsp::TIRE_STATIC_LODED_CONTACT, vsp::TIRE_STATIC_LODED_CONTACT, vsp::NUM_TIRE_CONTACT_MODES - 1 );

    m_ContactPt2_Isymm.Init( "ContactPt2_Isymm", "Design", this, 0, 0, 1 );
    m_ContactPt2_SuspensionMode.Init( "ContactPt2_SuspensionMode", "Design", this, vsp::GEAR_SUSPENSION_NOMINAL, vsp::GEAR_SUSPENSION_NOMINAL, vsp::NUM_GEAR_SUSPENSION_MODES - 1 );
    m_ContactPt2_TireMode.Init( "ContactPt2_TireMode", "Design", this, vsp::TIRE_STATIC_LODED_CONTACT, vsp::TIRE_STATIC_LODED_CONTACT, vsp::NUM_TIRE_CONTACT_MODES - 1 );

    m_ContactPt3_Isymm.Init( "ContactPt3_Isymm", "Design", this, 0, 0, 1 );
    m_ContactPt3_SuspensionMode.Init( "ContactPt3_SuspensionMode", "Design", this, vsp::GEAR_SUSPENSION_NOMINAL, vsp::GEAR_SUSPENSION_NOMINAL, vsp::NUM_GEAR_SUSPENSION_MODES - 1 );
    m_ContactPt3_TireMode.Init( "ContactPt3_TireMode", "Design", this, vsp::TIRE_STATIC_LODED_CONTACT, vsp::TIRE_STATIC_LODED_CONTACT, vsp::NUM_TIRE_CONTACT_MODES - 1 );

    m_CCEUnits.Init( "CCEUnits", "Design", this, vsp::LEN_IN, vsp::LEN_MM, vsp::LEN_UNITLESS );

    m_CCEMainGearOffset.Init( "CCEMainGearOffset", "Design", this, 0, -1e12, 1e12 );

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

            if ( m_AuxuliaryGeomMode() == AUX_GEOM_ROTOR_TIP_PATH )
            {
                m_FlapRadiusFract.Activate();
            }
            else // AUX_GEOM_ROTOR_BURST
            {
                m_FlapRadiusFract.Deactivate();
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
            cpThrust << sin( -m_ThetaThrust() * PI / 180.0 ) * bladelen, flapr + cos( -m_ThetaThrust() * PI / 180.0 ) * bladelen, 0.0;
            cpMid << 0.0, radius, 0.0;
            cpAnti << sin( m_ThetaAntiThrust() * PI / 180.0 ) * bladelen, flapr + cos( m_ThetaAntiThrust() * PI / 180.0 ) * bladelen, 0.0;

            thDir << cos( -m_ThetaThrust() * PI / 180.0 ), -sin( -m_ThetaThrust() * PI / 180.0 ), 0.0;
            cpThrust1 = cpThrust + thDir * k * bladelen * tan( m_ThetaThrust() * 0.25 * PI / 180.0 );
            cpThrust2 << k * tan( -m_ThetaThrust() * 0.25 * PI / 180.0 ) * bladelen, radius, 0.0;;

            antDir << -cos( m_ThetaAntiThrust() * PI / 180.0 ), sin( m_ThetaAntiThrust() * PI / 180.0 ), 0.0;
            cpAnti1 << k * tan( m_ThetaAntiThrust() * 0.25 * PI / 180.0 ) * bladelen, radius, 0.0;;
            cpAnti2 = cpAnti + antDir * k * bladelen * tan( m_ThetaAntiThrust() * 0.25 * PI / 180.0 );

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
        else if ( m_AuxuliaryGeomMode() == vsp::AUX_GEOM_ROTOR_BURST )
        {
            double radius = m_Diameter() * 0.5;

            double xstart = -m_RootOffset() * m_RootLength();
            double xend = xstart + m_RootLength();

            vector < vec3d > pts;
            pts.emplace_back( vec3d( xstart, 0.0, 0.0 ) );
            pts.emplace_back( vec3d( xstart + tan( -m_ThetaThrust() * PI / 180.0 ) * radius, radius, 0.0 ) );
            pts.emplace_back( vec3d( xend + tan( m_ThetaAntiThrust() * PI / 180.0 ) * radius, radius, 0.0 ) );
            pts.emplace_back( vec3d( xend, 0.0, 0.0 ) );

            vector < double > ts = { 0, 4.0*1.0/3.0, 4.0*2.0/3.0, 4.0 };

            VspCurve crv;
            crv.InterpolateLinear( pts, ts, false );

            m_MainSurfVec[0].CreateBodyRevolution( crv, true );
            m_MainSurfVec[0].SetMagicVParm( false );
        }
    }
    if ( m_ParentType == GEAR_GEOM_TYPE )
    {
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
        if ( m_AuxuliaryGeomMode() == vsp::AUX_GEOM_THREE_PT_CCE )
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
        m_MainSurfVec[0].SkinPCPC0( o, c, censph );
    }
}

void AuxiliaryGeom::UpdateMainTessVec()
{
    double tol = 1e-3;

    int nmain = GetNumMainSurfs();

    m_MainTessVec.clear();
    m_MainFeatureTessVec.clear();

    m_MainTessVec.reserve( nmain );
    m_MainFeatureTessVec.reserve( nmain );

    m_MainTessVec.resize( 1 );
    m_MainFeatureTessVec.resize( 1 );

    // Update primary main surf tess.
    UpdateTess( m_MainSurfVec[ 0 ], m_CapUMinSuccess[ m_MainSurfIndxVec[ 0 ] ], m_CapUMaxSuccess[ m_MainSurfIndxVec[ 0 ] ], m_MainTessVec[ 0 ], m_MainFeatureTessVec[ 0 ] );

    if ( m_ParentType == GEAR_GEOM_TYPE )
    {
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
        }
    }
}

void AuxiliaryGeom::UpdateMainDegenGeomPreview()
{
    int nmain = GetNumMainSurfs();

    m_MainDegenGeomPreviewVec.clear();
    m_MainDegenGeomPreviewVec.reserve( nmain );
    m_MainDegenGeomPreviewVec.resize( 1 );

    // Update primary main surf tess.
    CreateDegenGeom( m_MainSurfVec[ 0 ], 0, m_MainDegenGeomPreviewVec[ 0 ], true );

    if ( m_ParentType == GEAR_GEOM_TYPE )
    {
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
        }
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
    m_WLoc.Deactivate();
    m_RLoc.Deactivate();
    m_SLoc.Deactivate();
    m_TLoc.Deactivate();
    m_LLoc.Deactivate();
    m_MLoc.Deactivate();
    m_NLoc.Deactivate();

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
            b1->TireToBogie( b1->m_TireSurface, m_MainSurfVec, m_ContactPt1_Isymm(), m_ContactPt1_SuspensionMode(), bogietheta );
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
            b2->TireToBogie( b2->m_TireSurface, m_MainSurfVec, m_ContactPt2_Isymm(), m_ContactPt2_SuspensionMode(), bogietheta );
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
            b3->TireToBogie( b3->m_TireSurface, m_MainSurfVec, m_ContactPt3_Isymm(), m_ContactPt3_SuspensionMode(), bogietheta );
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
            SimpleTess tireTess;
            SimpleFeatureTess tireFeatureTess;

            UpdateTess( b1->m_TireSurface, false, false, tireTess, tireFeatureTess );

            b1->TireToBogie( tireTess, m_MainTessVec, m_ContactPt1_Isymm(), m_ContactPt1_SuspensionMode(), bogietheta );
            b1->TireToBogie( tireFeatureTess, m_MainFeatureTessVec, m_ContactPt1_Isymm(), m_ContactPt1_SuspensionMode(), bogietheta );
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
            SimpleTess tireTess;
            SimpleFeatureTess tireFeatureTess;

            UpdateTess( b2->m_TireSurface, false, false, tireTess, tireFeatureTess );

            b2->TireToBogie( tireTess, m_MainTessVec, m_ContactPt2_Isymm(), m_ContactPt2_SuspensionMode(), bogietheta );
            b2->TireToBogie( tireFeatureTess, m_MainFeatureTessVec, m_ContactPt2_Isymm(), m_ContactPt2_SuspensionMode(), bogietheta );
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
            SimpleTess tireTess;
            SimpleFeatureTess tireFeatureTess;

            UpdateTess( b3->m_TireSurface, false, false, tireTess, tireFeatureTess );

            b3->TireToBogie( tireTess, m_MainTessVec, m_ContactPt3_Isymm(), m_ContactPt3_SuspensionMode(), bogietheta );
            b3->TireToBogie( tireFeatureTess, m_MainFeatureTessVec, m_ContactPt3_Isymm(), m_ContactPt3_SuspensionMode(), bogietheta );
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
            DegenGeom degenGeom;
            CreateDegenGeom( b1->m_TireSurface, 0, degenGeom, true );

            b1->TireToBogie( degenGeom, m_MainDegenGeomPreviewVec, m_ContactPt1_Isymm(), m_ContactPt1_SuspensionMode(), bogietheta );
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
            CreateDegenGeom( b2->m_TireSurface, 0, degenGeom, true );

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
            CreateDegenGeom( b3->m_TireSurface, 0, degenGeom, true );

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

void AuxiliaryGeom::GetPtNormal( vec3d &pt, vec3d &normal ) const
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
        }
    }
}

void AuxiliaryGeom::GetPtPivotAxis( vec3d &ptaxis, vec3d &axis )
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
        }
    }
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

void AuxiliaryGeom::GetSideContactPtRollAxisNormal( vec3d &pt, vec3d &axis, vec3d &normal, int &ysign )
{
    if ( m_AuxuliaryGeomMode() == vsp::AUX_GEOM_ONE_PT_GROUND )
    {
        Geom* parent_geom = m_Vehicle->FindGeom( m_ParentID );

        GearGeom * gear = dynamic_cast< GearGeom* > ( parent_geom );
        if ( gear )
        {
            gear->GetOnePtSideContactPtAxisNormalInWorld( m_ContactPt1_ID, m_ContactPt1_Isymm(), m_ContactPt1_SuspensionMode(), m_ContactPt1_TireMode(),
                                                          m_BogieTheta() * M_PI / 180.0, m_WheelTheta() * M_PI / 180, 0, pt, axis, normal, ysign);
        }
    }
}

void AuxiliaryGeom::GetPtNormalAftAxleAxis( double thetabogie, vec3d &pt, vec3d &normal, vec3d &ptaxis, vec3d &axis )
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
        }
    }
}

void AuxiliaryGeom::GetPtNormalFwdAxleAxis( double thetabogie, vec3d &pt, vec3d &normal, vec3d &ptaxis, vec3d &axis )
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
        }
    }
}

void AuxiliaryGeom::GetTwoPtSideContactPtsNormal( vec3d &p1, vec3d &p2, vec3d &normal )
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
        }
    }
}

void AuxiliaryGeom::GetContactPointVecNormal( vector < vec3d > &ptvec, vec3d &normal )
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
        }
    }
}

void AuxiliaryGeom::CalculateTurn( vec3d &cor, vec3d &normal, vector<double> &rvec )
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
        }
    }
}
