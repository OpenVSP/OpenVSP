//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#include "APIDefines.h"
#include "VspUtil.h"
#include "ClearanceGeom.h"
#include "Vehicle.h"
#include "VSP_Geom_API.h"
#include "PropGeom.h"
#include "GearGeom.h"
#include "Geom.h"
#include "ParmMgr.h"
#include <cfloat>  //For DBL_EPSILON

using namespace vsp;

//==== Constructor ====//
ClearanceGeom::ClearanceGeom( Vehicle* vehicle_ptr ) : Geom( vehicle_ptr )
{
    m_Name = "ClearanceGeom";
    m_Type.m_Name = "Clearance";
    m_Type.m_Type = CLEARANCE_GEOM_TYPE;


    m_ClearanceMode.Init( "ClearanceType", "Design", this, CLEARANCE_ROTOR_TIP_PATH, CLEARANCE_ROTOR_TIP_PATH, NUM_CLEARANCE_MODES - 1 );
    m_ClearanceMode.SetDescript( "Type of clearance geometry." );

    m_AutoDiam.Init( "AutoDiam", "Design", this, true, false, true );
    m_AutoDiam.SetDescript( "Flag to set the diameter automatically." );

    m_Diameter.Init( "Diameter", "Design", this, 30.0, 0.0, 1.0e12 );
    m_Diameter.SetDescript( "Diameter of clearance geometry" );

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

    m_BogieTheta.Init( "BogieTheta", "Design", this, 0.0, -180.0, 180.0 );
    m_WheelTheta.Init( "WheelTheta", "Design", this, 0.0, -180.0, 180.0 );

    m_ParentType = -1;

}

//==== Destructor ====//
ClearanceGeom::~ClearanceGeom()
{

}

void ClearanceGeom::ComputeCenter()
{
}

void ClearanceGeom::Scale()
{
    double currentScale = m_Scale() / m_LastScale();

    m_LastScale = m_Scale();
}

void ClearanceGeom::AddDefaultSources( double base_len )
{
}

void ClearanceGeom::OffsetXSecs( double off )
{
}

void ClearanceGeom::UpdateSurf()
{
    m_MainSurfVec[0] = VspSurf();

    //===== Find Parent ====//
    Geom* parent_geom = m_Vehicle->FindGeom( m_ParentID );
    if ( !parent_geom )
    {
        return;
    }

    m_ParentType = parent_geom->GetType().m_Type;

    //==== Copy XForm/Tess Data From Parent ====//
    CopyDataFrom( parent_geom );


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
                if ( m_ClearanceMode() == CLEARANCE_ROTOR_TIP_PATH )
                {
                    m_Diameter.Set( parent_prop->m_Diameter() );
                }
                else // CLEARANCE_ROTOR_BURST
                {
                    m_Diameter.Set( refLen );
                }
                m_Diameter.Deactivate();
            }
            else
            {
                m_Diameter.Activate();
            }

            if ( m_ClearanceMode() == CLEARANCE_ROTOR_TIP_PATH )
            {
                m_FlapRadiusFract.Activate();
            }
            else // CLEARANCE_ROTOR_BURST
            {
                m_FlapRadiusFract.Deactivate();
            }

        }


        if ( m_ClearanceMode() == vsp::CLEARANCE_ROTOR_TIP_PATH )
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
        else if ( m_ClearanceMode() == vsp::CLEARANCE_ROTOR_BURST )
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
        if ( m_ClearanceMode() == vsp::CLEARANCE_THREE_PT_GROUND )
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
            }
        }
        else if ( m_ClearanceMode() == vsp::CLEARANCE_TWO_PT_GROUND )
        {
            GearGeom * gear = dynamic_cast< GearGeom* > ( parent_geom );
            if ( gear )
            {
                Matrix4d basis;
                m_ContactPts.resize( 2 );

                gear->BuildTwoPtBasis( m_ContactPt1_ID, m_ContactPt1_Isymm(), m_ContactPt1_SuspensionMode(), m_ContactPt1_TireMode(),
                                       m_ContactPt2_ID, m_ContactPt2_Isymm(), m_ContactPt2_SuspensionMode(), m_ContactPt2_TireMode(),
                                       m_BogieTheta() * M_PI / 180.0, basis, m_ContactPts[0], m_ContactPts[1] );

                vec3d ptaxis, axis;
                if ( m_WheelTheta() > 0 )
                {
                    gear->GetTwoPtAftAxleAxis( m_ContactPt1_ID, m_ContactPt1_Isymm(), m_ContactPt1_SuspensionMode(),
                                           m_ContactPt2_ID, m_ContactPt2_Isymm(), m_ContactPt2_SuspensionMode(),
                                           m_BogieTheta() * M_PI / 180.0, ptaxis, axis );
                }
                else
                {
                    gear->GetTwoPtFwdAxleAxis( m_ContactPt1_ID, m_ContactPt1_Isymm(), m_ContactPt1_SuspensionMode(),
                                           m_ContactPt2_ID, m_ContactPt2_Isymm(), m_ContactPt2_SuspensionMode(),
                                           m_BogieTheta() * M_PI / 180.0, ptaxis, axis );
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
    }
}


void ClearanceGeom::CopyDataFrom( Geom* geom_ptr )
{
    //==== Force Attached So Clearance Moves With Parent =====//
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
    //m_TessU = geom_ptr->m_TessU();
    //m_TessW = geom_ptr->m_TessW();

    m_SymAncestor = geom_ptr->m_SymAncestor();
    if ( m_SymAncestor() != 0 ) // Not global ancestor.
    {
        m_SymAncestor = m_SymAncestor() + 1;  // + 1 increment for parent
    }
    m_SymAncestOriginFlag = geom_ptr->m_SymAncestOriginFlag();
    m_SymPlanFlag = geom_ptr->m_SymPlanFlag();
    m_SymAxFlag = geom_ptr->m_SymAxFlag();
    m_SymRotN = geom_ptr->m_SymRotN();

    m_SymAncestor.Deactivate();
    m_SymAncestOriginFlag.Deactivate();
    m_SymPlanFlag.Deactivate();
    m_SymAxFlag.Deactivate();
    m_SymRotN.Deactivate();
}

void ClearanceGeom::UpdateDrawObj()
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


void ClearanceGeom::LoadDrawObjs( vector< DrawObj* > & draw_obj_vec )
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
        draw_obj_vec.push_back( &m_BasisDrawObj_vec[i] );
    }


    draw_obj_vec.push_back( &m_ContactDrawObj );
}

void ClearanceGeom::UpdateBBox( )
{
    // Fill m_BBox like normal
    Geom::UpdateBBox();

    // Reset m_ScaleIndependentBBox to empty
    m_ScaleIndependentBBox.Reset();
}

xmlNodePtr ClearanceGeom::EncodeXml( xmlNodePtr & node )
{
    xmlNodePtr geom_node = Geom::EncodeXml( node );

    xmlNodePtr child_node = xmlNewChild( geom_node, NULL, BAD_CAST "Clearance", NULL );

    if ( child_node )
    {
        XmlUtil::AddStringNode( child_node, "ContactPt1_ID", m_ContactPt1_ID );
        XmlUtil::AddStringNode( child_node, "ContactPt2_ID", m_ContactPt2_ID );
        XmlUtil::AddStringNode( child_node, "ContactPt3_ID", m_ContactPt3_ID );
    }

    return child_node;
}

xmlNodePtr ClearanceGeom::DecodeXml( xmlNodePtr & node )
{
    xmlNodePtr geom_node = Geom::DecodeXml( node );

    xmlNodePtr child_node = XmlUtil::GetNode( geom_node, "Clearance", 0 );

    if ( child_node )
    {
        m_ContactPt1_ID = ParmMgr.RemapID( XmlUtil::FindString( child_node, "ContactPt1_ID", m_ContactPt1_ID ) );
        m_ContactPt2_ID = ParmMgr.RemapID( XmlUtil::FindString( child_node, "ContactPt2_ID", m_ContactPt2_ID ) );
        m_ContactPt3_ID = ParmMgr.RemapID( XmlUtil::FindString( child_node, "ContactPt3_ID", m_ContactPt3_ID ) );
    }

    return child_node;
}

void ClearanceGeom::SetContactPt1ID( const std::string& id )
{
    m_ContactPt1_ID = id;
    m_SurfDirty = true;
    Update();
}

void ClearanceGeom::SetContactPt2ID( const std::string& id )
{
    m_ContactPt2_ID = id;
    m_SurfDirty = true;
    Update();
}

void ClearanceGeom::SetContactPt3ID( const std::string& id )
{
    m_ContactPt3_ID = id;
    m_SurfDirty = true;
    Update();
}

void ClearanceGeom::GetCG( vec3d &cgnom, vector < vec3d > &cgbounds )
{
    if ( m_ClearanceMode() == vsp::CLEARANCE_THREE_PT_GROUND ||
         m_ClearanceMode() == vsp::CLEARANCE_TWO_PT_GROUND )
    {
        Geom* parent_geom = m_Vehicle->FindGeom( m_ParentID );

        GearGeom * gear = dynamic_cast< GearGeom* > ( parent_geom );
        if ( gear )
        {
            gear->GetCGInWorld( cgnom, cgbounds );
        }
    }
}

void ClearanceGeom::GetPtNormal( vec3d &pt, vec3d &normal ) const
{
    if ( m_ClearanceMode() == vsp::CLEARANCE_THREE_PT_GROUND )
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

void ClearanceGeom::GetPtPivotAxis( vec3d &ptaxis, vec3d &axis )
{
    if ( m_ClearanceMode() == vsp::CLEARANCE_TWO_PT_GROUND )
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

void ClearanceGeom::GetPtNormalMeanContactPtPivotAxis( vec3d &pt, vec3d &normal, vec3d &ptaxis, vec3d &axis, bool &usepivot, double &mintheta, double &maxtheta )
{
    if ( m_ClearanceMode() == vsp::CLEARANCE_TWO_PT_GROUND )
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
        }
    }
}

void ClearanceGeom::GetPtNormalAftAxleAxis( double thetabogie, vec3d &pt, vec3d &normal, vec3d &ptaxis, vec3d &axis )
{
    if ( m_ClearanceMode() == vsp::CLEARANCE_TWO_PT_GROUND )
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

void ClearanceGeom::GetPtNormalFwdAxleAxis( double thetabogie, vec3d &pt, vec3d &normal, vec3d &ptaxis, vec3d &axis )
{
    if ( m_ClearanceMode() == vsp::CLEARANCE_TWO_PT_GROUND )
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

void ClearanceGeom::GetTwoPtSideContactPtsNormal( vec3d &p1, vec3d &p2, vec3d &normal )
{
    if ( m_ClearanceMode() == vsp::CLEARANCE_TWO_PT_GROUND )
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

void ClearanceGeom::GetContactPointVecNormal( vector < vec3d > &ptvec, vec3d &normal )
{
    if ( m_ClearanceMode() == vsp::CLEARANCE_THREE_PT_GROUND )
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
