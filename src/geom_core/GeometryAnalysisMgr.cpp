//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// GeometryAnalysisMgr.cpp
//
// Rob McDonald
//////////////////////////////////////////////////////////////////////

#define _USE_MATH_DEFINES
#include <cmath>
#include <Eigen/SVD>

#include "GeometryAnalysisMgr.h"

#include "ModeMgr.h"
#include "StlHelper.h"
#include "Vehicle.h"
#include "ParmMgr.h"
#include "SnapTo.h"
#include "ProjectionMgr.h"

#include "AuxiliaryGeom.h"
#include "GearGeom.h"
#include "HumanGeom.h"

#include "MeshGeom.h"
#include "TMesh.h"

GeometryAnalysisCase::GeometryAnalysisCase()
{
    string groupname = "InterferenceCase";

    m_PrimarySet.Init( "PrimarySet", groupname, this, DEFAULT_SET, 0, vsp::MAX_NUM_SETS );
    m_PrimarySet.SetDescript( "Selected primary set for operation" );

    m_PrimaryType.Init( "PrimaryType", "Projection", this, vsp::SET_TARGET, vsp::SET_TARGET, vsp::NUM_PROJ_TGT_OPTIONS - 1 );


    m_SecondarySet.Init( "SecondarySet", groupname, this, DEFAULT_SET, 0, vsp::MAX_NUM_SETS );
    m_SecondarySet.SetDescript( "Selected secondary set for operation" );

    m_SecondaryType.Init( "SecondaryType", "Projection", this, vsp::SET_TARGET, vsp::SET_TARGET, vsp::NUM_PROJ_TGT_OPTIONS - 1 );

    m_SecondaryZGround.Init( "SecondaryZGround", "Projection", this, 0.0, -1e12, 1e12 );

    m_SecondaryCCWFlag.Init( "SecondaryCCWFlag", "Projection", this, false, false, true );

    m_PolyVisibleFlag.Init( "PolyVisibleFlag", "Projection", this, false, false, true );

    m_SecondaryX.Init( "SecondaryX", "Projection", this, 0.0, -1e12, 1e12 );
    m_SecondaryY.Init( "SecondaryY", "Projection", this, 0.0, -1e12, 1e12 );
    m_SecondaryZ.Init( "SecondaryZ", "Projection", this, 0.0, -1e12, 1e12 );

    m_GeometryAnalysisType.Init( "IntererenceCheckType", groupname, this, vsp::EXTERNAL_INTERFERENCE, vsp::EXTERNAL_INTERFERENCE, vsp::NUM_INTERFERENCE_TYPES - 1 );

    m_LastResultValue.Init( "LastResult", groupname, this, 0.0, -1e12, 1e12 );
}

void GeometryAnalysisCase::Update()
{
    // Dual-geometry analyses that require special geometry for secondary
    if ( m_GeometryAnalysisType() == vsp::PLANE_2PT_ANGLE_INTERFERENCE ||
         m_GeometryAnalysisType() == vsp::PLANE_1PT_ANGLE_INTERFERENCE ||
         m_GeometryAnalysisType() == vsp::GEAR_TURN_ANALYSIS ||
         m_GeometryAnalysisType() == vsp::CCE_INTERFERENCE )
    {
        m_SecondaryType = vsp::GEOM_TARGET;
    }

    UpdateDrawObj_Live();
}

string GeometryAnalysisCase::GetPrimaryName() const
{
    if ( m_PrimaryType() == vsp::SET_TARGET )
    {
        Vehicle *veh = VehicleMgr.GetVehicle();
        if ( veh )
        {
            vector < string > setNameVec = veh->GetSetNameVec();
            int pset = m_PrimarySet();
            if ( pset >= 0 && pset < setNameVec.size() )
            {
                return setNameVec[ pset ];
            }
        }
    }
    else if ( m_PrimaryType() == vsp::GEOM_TARGET )
    {
        Vehicle *veh = VehicleMgr.GetVehicle();
        if ( veh )
        {
            Geom *g = veh->FindGeom( m_PrimaryGeomID );
            if ( g )
            {
                return g->GetName();
            }
        }
    }
    else if ( m_PrimaryType() == vsp::MODE_TARGET )
    {
        Mode *m = ModeMgr.GetMode( m_PrimaryModeID );
        if ( m )
        {
            return m->GetName();
        }
    }

    return string();
}

string GeometryAnalysisCase::GetSecondaryName() const
{
    if ( m_SecondaryType() == vsp::SET_TARGET )
    {
        Vehicle *veh = VehicleMgr.GetVehicle();
        if ( veh )
        {
            vector < string > setNameVec = veh->GetSetNameVec();
            int pset = m_SecondarySet();
            if ( pset >= 0 && pset < setNameVec.size() )
            {
                return setNameVec[ pset ];
            }
        }
    }
    else if ( m_SecondaryType() == vsp::GEOM_TARGET )
    {
        Vehicle *veh = VehicleMgr.GetVehicle();
        if ( veh )
        {
            Geom *g = veh->FindGeom( m_SecondaryGeomID );
            if ( g )
            {
                return g->GetName();
            }
        }
    }

    return string();
}

vector< TMesh* > GeometryAnalysisCase::GetPrimaryTMeshVec()
{
    vector< TMesh* > tmv;

    Vehicle *veh = VehicleMgr.GetVehicle();
    if ( veh )
    {
        if ( m_PrimaryType() == vsp::SET_TARGET || m_PrimaryType() == vsp::MODE_TARGET )
        {
            int set = vsp::SET_NONE;

            if ( m_PrimaryType() == vsp::SET_TARGET )
            {
                set = m_PrimarySet();
            }
            else
            {
                Mode *m = ModeMgr.GetMode( m_PrimaryModeID );
                if ( m )
                {
                    set = m->m_NormalSet();
                }
            }

            tmv = veh->CreateTMeshVec( set );
        }
        else if ( m_PrimaryType() == vsp::GEOM_TARGET )
        {
            tmv = veh->CreateTMeshVec( m_PrimaryGeomID );
        }
    }

    return tmv;
}

vector< TMesh* > GeometryAnalysisCase::GetSecondaryTMeshVec()
{
    vector< TMesh* > tmv;
    Vehicle *veh = VehicleMgr.GetVehicle();
    if ( veh )
    {
        if ( m_SecondaryType() == vsp::SET_TARGET )
        {
            int set = m_SecondarySet();

            tmv = veh->CreateTMeshVec( set );
        }
        else if ( m_SecondaryType() == vsp::GEOM_TARGET )
        {
            tmv = veh->CreateTMeshVec( m_SecondaryGeomID );
        }
    }

    return tmv;
}

bool GeometryAnalysisCase::GetPrimaryTwoPtSideContactPtsNormal( vec3d &p1, vec3d &p2, vec3d &normal )
{
    AuxiliaryGeom* auxiliary_ptr = GetPrimaryAuxiliaryGeom();
    if ( auxiliary_ptr )
    {
        return auxiliary_ptr->GetTwoPtSideContactPtsNormal( p1, p2, normal );
    }
    return false;
}

bool GeometryAnalysisCase::GetPrimaryContactPointVecNormal( vector < vec3d > &ptvec, vec3d &normal )
{
    AuxiliaryGeom* auxiliary_ptr = GetPrimaryAuxiliaryGeom();
    if ( auxiliary_ptr )
    {
        return auxiliary_ptr->GetContactPointVecNormal( ptvec, normal );
    }
    return false;
}

bool GeometryAnalysisCase::GetPrimaryCG( vec3d &cgnom, vector < vec3d > &cgbounds )
{
    AuxiliaryGeom* auxiliary_ptr = GetPrimaryAuxiliaryGeom();
    if ( auxiliary_ptr )
    {
        return auxiliary_ptr->GetCG( cgnom, cgbounds );
    }
    return false;
}

AuxiliaryGeom* GeometryAnalysisCase::GetPrimaryAuxiliaryGeom() const
{
    Vehicle *veh = VehicleMgr.GetVehicle();

    AuxiliaryGeom* auxiliary_ptr = nullptr;

    if ( veh )
    {
        if ( m_PrimaryType() == vsp::SET_TARGET || m_PrimaryType() == vsp::MODE_TARGET )
        {
            int set = vsp::SET_NONE;

            if ( m_PrimaryType() == vsp::SET_TARGET )
            {
                set = m_PrimarySet();
            }
            else
            {
                Mode *m = ModeMgr.GetMode( m_PrimaryModeID );
                if ( m )
                {
                    set = m->m_NormalSet();
                }
            }

            // Find first AuxiliaryGeom in set
            vector<string> geom_vec = veh->GetGeomVec();
            for ( int i = 0 ; i < ( int )geom_vec.size() ; i++ )
            {
                Geom* geom = veh->FindGeom( geom_vec[i] );
                if ( geom )
                {
                    if ( geom->GetSetFlag( set ) )
                    {
                        AuxiliaryGeom* aux = dynamic_cast< AuxiliaryGeom* >( geom );
                        if ( aux )
                        {
                            auxiliary_ptr = aux;
                            break;
                        }
                    }
                }
            }
        }
        else if ( m_PrimaryType() == vsp::GEOM_TARGET )
        {
            Geom* geom = veh->FindGeom( m_PrimaryGeomID );

            auxiliary_ptr = dynamic_cast< AuxiliaryGeom* >( geom );
        }
    }

    return auxiliary_ptr;
}

bool GeometryAnalysisCase::GetPrimaryPtNormalMeanContactPtPivotAxisCG( vec3d &pt, vec3d &normal, vec3d &ptaxis, vec3d &axis, bool &usepivot, double &mintheta, double &maxtheta, vec3d &cgnom, vector < vec3d > &cgbounds ) const
{
    AuxiliaryGeom* auxiliary_ptr = GetPrimaryAuxiliaryGeom();

    if ( auxiliary_ptr )
    {
        bool r1 = auxiliary_ptr->GetPtNormalMeanContactPtPivotAxis( pt, normal, ptaxis, axis, usepivot, mintheta, maxtheta );
        bool r2 = auxiliary_ptr->GetCG( cgnom, cgbounds );
        return r1 && r2;
    }
    return false;
}

bool GeometryAnalysisCase::GetSecondaryPt( vec3d &pt )
{
    if ( m_SecondaryType() == vsp::XYZ_TARGET )
    {
        pt.set_xyz( m_SecondaryX(), m_SecondaryY(), m_SecondaryZ() );
        return true;
    }
    else
    {
        pt.set_xyz( 0, 0, 1 );
        Vehicle *veh = VehicleMgr.GetVehicle();
        if ( veh )
        {
            if ( m_SecondaryType() == vsp::GEOM_TARGET )
            {
                Geom* geom = veh->FindGeom( m_SecondaryGeomID );

                HumanGeom* human_ptr = dynamic_cast< HumanGeom* >( geom );

                if ( human_ptr )
                {
                    pt = human_ptr->GetDesignEye();
                    return true;
                }
                else if ( geom )
                {
                    Matrix4d mat = geom->getModelMatrix();
                    pt = mat.xform( vec3d() );
                    return true;
                }
            }
        }
    }
    return false;
}

bool GeometryAnalysisCase::GetSecondaryPtNormal( vec3d &pt, vec3d &normal )
{
    Vehicle *veh = VehicleMgr.GetVehicle();
    if ( veh )
    {
        if ( m_SecondaryType() == vsp::GEOM_TARGET )
        {
            Geom* geom = veh->FindGeom( m_SecondaryGeomID );

            AuxiliaryGeom* auxiliary_ptr = dynamic_cast< AuxiliaryGeom* >( geom );
            GearGeom* gear_ptr = dynamic_cast< GearGeom* >( geom );

            if ( auxiliary_ptr )
            {
                return auxiliary_ptr->GetPtNormal( pt, normal );
            }
            else if ( gear_ptr )
            {
                gear_ptr->GetNominalPtNormalInWorld( pt, normal );
                return true;
            }
        }
    }
    return false;
}

bool GeometryAnalysisCase::GetSecondarySideContactPtRollAxisNormal( vec3d &pt, vec3d &axis, vec3d &normal, int &ysign )
{
    Vehicle *veh = VehicleMgr.GetVehicle();
    if ( veh )
    {
        if ( m_SecondaryType() == vsp::GEOM_TARGET )
        {
            Geom* geom = veh->FindGeom( m_SecondaryGeomID );

            AuxiliaryGeom* auxiliary_ptr = dynamic_cast< AuxiliaryGeom* >( geom );

            if ( auxiliary_ptr )
            {
                return auxiliary_ptr->GetSideContactPtRollAxisNormal( pt, axis, normal, ysign );
            }
        }
    }
    return false;
}


bool GeometryAnalysisCase::GetSecondaryPtNormalMeanContactPivotAxis( vec3d &pt, vec3d &normal, vec3d &ptaxis, vec3d &axis, bool &usepivot, double &mintheta, double &maxtheta )
{
    Vehicle *veh = VehicleMgr.GetVehicle();
    if ( veh )
    {
        if ( m_SecondaryType() == vsp::GEOM_TARGET )
        {
            Geom* geom = veh->FindGeom( m_SecondaryGeomID );

            AuxiliaryGeom* auxiliary_ptr = dynamic_cast< AuxiliaryGeom* >( geom );

            if ( auxiliary_ptr )
            {
                return auxiliary_ptr->GetPtNormalMeanContactPtPivotAxis( pt, normal, ptaxis, axis, usepivot, mintheta, maxtheta );
            }
        }
    }
    return false;
}

bool GeometryAnalysisCase::GetSecondaryPtNormalAftAxleAxis( double thetabogie, vec3d &pt, vec3d &normal, vec3d &ptaxis, vec3d &axis  )
{
    Vehicle *veh = VehicleMgr.GetVehicle();
    if ( veh )
    {
        if ( m_SecondaryType() == vsp::GEOM_TARGET )
        {
            Geom* geom = veh->FindGeom( m_SecondaryGeomID );

            AuxiliaryGeom* auxiliary_ptr = dynamic_cast< AuxiliaryGeom* >( geom );

            if ( auxiliary_ptr )
            {
                return auxiliary_ptr->GetPtNormalAftAxleAxis( thetabogie, pt, normal, ptaxis, axis );
            }
        }
    }
    return false;
}

bool GeometryAnalysisCase::GetSecondaryPtNormalFwdAxleAxis( double thetabogie, vec3d &pt, vec3d &normal, vec3d &ptaxis, vec3d &axis  )
{
    Vehicle *veh = VehicleMgr.GetVehicle();
    if ( veh )
    {
        if ( m_SecondaryType() == vsp::GEOM_TARGET )
        {
            Geom* geom = veh->FindGeom( m_SecondaryGeomID );

            AuxiliaryGeom* auxiliary_ptr = dynamic_cast< AuxiliaryGeom* >( geom );

            if ( auxiliary_ptr )
            {
                return auxiliary_ptr->GetPtNormalFwdAxleAxis( thetabogie, pt, normal, ptaxis, axis );
            }
        }
    }
    return false;
}

xmlNodePtr GeometryAnalysisCase::EncodeXml( xmlNodePtr & node )
{
    xmlNodePtr gcase_node = xmlNewChild( node, NULL, BAD_CAST"GeometryAnalysis", NULL );

    ParmContainer::EncodeXml( gcase_node );
    XmlUtil::AddStringNode( gcase_node, "PrimaryModeID", m_PrimaryModeID );
    XmlUtil::AddStringNode( gcase_node, "PrimaryGeomID", m_PrimaryGeomID );
    XmlUtil::AddStringNode( gcase_node, "SecondaryGeomID", m_SecondaryGeomID );

    //===== Cutout Subsurfaces ====//
    xmlNodePtr cutoutSS_list_node = xmlNewChild( gcase_node, NULL, ( const xmlChar * )"CutoutSS_List", NULL );
    for ( int i = 0 ; i < ( int )m_CutoutVec.size() ; i++ )
    {
        xmlNodePtr cutoutSS_node = xmlNewChild( cutoutSS_list_node, NULL, ( const xmlChar * )"CutoutSS", NULL );
        XmlUtil::AddStringNode( cutoutSS_node, "cutoutSS_ID", m_CutoutVec[i] );
    }

    return gcase_node;
}

xmlNodePtr GeometryAnalysisCase::DecodeXml( xmlNodePtr & node )
{
    ParmContainer::DecodeXml( node );
    m_PrimaryModeID = ParmMgr.RemapID( XmlUtil::FindString( node, "PrimaryModeID", m_PrimaryModeID ) );
    m_PrimaryGeomID = ParmMgr.RemapID( XmlUtil::FindString( node, "PrimaryGeomID", m_PrimaryGeomID ) );
    m_SecondaryGeomID = ParmMgr.RemapID( XmlUtil::FindString( node, "SecondaryGeomID", m_SecondaryGeomID ) );

    //==== Cutout Subsurfaces ====//
    xmlNodePtr cutoutSS_list_node = XmlUtil::GetNode( node, "CutoutSS_List", 0 );

    if ( cutoutSS_list_node )
    {
        int num_cutoutSS =  XmlUtil::GetNumNames( cutoutSS_list_node, "CutoutSS" );
        for ( int i = 0 ; i < num_cutoutSS ; i++ )
        {
            xmlNodePtr cutoutSS_node = XmlUtil::GetNode( cutoutSS_list_node, "CutoutSS", i );
            string new_cutoutSS = ParmMgr.RemapID( XmlUtil::FindString( cutoutSS_node, "cutoutSS_ID", string() ) );
            m_CutoutVec.push_back( new_cutoutSS );
        }
    }

    return node;
}

string GeometryAnalysisCase::Evaluate()
{
    m_LastResultValue = 1.0;
    m_LastResult.clear();

    DeleteTMeshVec( m_TMeshVec );
    m_PtsVec.clear();

    Vehicle *veh = VehicleMgr.GetVehicle();
    if ( veh )
    {
        if ( m_PrimaryType() == vsp::MODE_TARGET )
        {
            Mode *m = ModeMgr.GetMode( m_PrimaryModeID );
            if ( m )
            {
                m->ApplySettings();
                // fullupdate = false skips feature lines, subsurfaces, and structured tessellation.
                veh->Update( false );
            }
        }

        vector< TMesh* > primary_tmv;
        vector< TMesh* > secondary_tmv;

        switch ( m_GeometryAnalysisType() )
        {
            case vsp::EXTERNAL_INTERFERENCE:
            {
                primary_tmv = GetPrimaryTMeshVec();
                secondary_tmv = GetSecondaryTMeshVec();
                if ( !primary_tmv.empty() && !secondary_tmv.empty() )
                {
                    Results *res = ResultsMgr.CreateResults( "External_Interference", "External interference check." );
                    if ( res )
                    {
                        m_LastResult = res->GetID();
                        ExteriorInterferenceCheck( primary_tmv, secondary_tmv, m_LastResult, m_TMeshVec );
                        m_PtsVec = ResultsMgr.GetVec3dResults( m_LastResult, "Pts", 0 );
                    }
                }
                else
                {
                    MessageData errMsgData;
                    errMsgData.m_String = "Error";
                    errMsgData.m_IntVec.push_back( vsp::VSP_WRONG_GEOM_TYPE );
                    char buf[255];
                    snprintf( buf, sizeof( buf ), "Error:  Empty primary or secondary mesh in %s.", m_Name.c_str() );
                    errMsgData.m_StringVec.emplace_back( string( buf ) );

                    MessageMgr::getInstance().SendAll( errMsgData );
                }
                break;
            }
            case vsp::PACKAGING_INTERFERENCE:
            {
                primary_tmv = GetPrimaryTMeshVec();
                secondary_tmv = GetSecondaryTMeshVec();
                if ( !primary_tmv.empty() && !secondary_tmv.empty() )
                {
                    m_LastResult = PackagingInterferenceCheck( primary_tmv, secondary_tmv, m_TMeshVec );
                    m_PtsVec = ResultsMgr.GetVec3dResults( m_LastResult, "Pts", 0 );
                }
                else
                {
                    MessageData errMsgData;
                    errMsgData.m_String = "Error";
                    errMsgData.m_IntVec.push_back( vsp::VSP_WRONG_GEOM_TYPE );
                    char buf[255];
                    snprintf( buf, sizeof( buf ), "Error:  Empty primary or secondary mesh in %s.", m_Name.c_str() );
                    errMsgData.m_StringVec.emplace_back( string( buf ) );

                    MessageMgr::getInstance().SendAll( errMsgData );
                }
                break;
            }
            case vsp::EXTERNAL_SELF_INTERFERENCE:
            {
                primary_tmv = GetPrimaryTMeshVec();
                if ( !primary_tmv.empty() )
                {
                    m_LastResult = ExteriorSelfInterferenceCheck( primary_tmv, m_TMeshVec );
                    m_PtsVec = ResultsMgr.GetVec3dResults( m_LastResult, "Pts", 0 );
                }
                else
                {
                    MessageData errMsgData;
                    errMsgData.m_String = "Error";
                    errMsgData.m_IntVec.push_back( vsp::VSP_WRONG_GEOM_TYPE );
                    char buf[255];
                    snprintf( buf, sizeof( buf ), "Error:  Empty primary mesh in %s.", m_Name.c_str() );
                    errMsgData.m_StringVec.emplace_back( string( buf ) );

                    MessageMgr::getInstance().SendAll( errMsgData );
                }
                break;
            }
            case vsp::PLANE_STATIC_DISTANCE_INTERFERENCE:
            {
                Results *res = ResultsMgr.CreateResults( "Static_Plane_Distance_Interference", "Static point plane distance interference check." );
                if( res )
                {
                    m_LastResult = res->GetID();
                    primary_tmv = GetPrimaryTMeshVec();

                    if ( !primary_tmv.empty() )
                    {
                        CSGMesh( primary_tmv );
                        FlattenTMeshVec( primary_tmv );

                        TMesh *primary_tm = MergeTMeshVec( primary_tmv );
                        primary_tm->LoadBndBox();

                        bool success = false;

                        vec3d org, norm;
                        if ( m_SecondaryType() == vsp::Z_TARGET )
                        {
                            org.set_z( m_SecondaryZGround() );
                            norm.set_z( 1 );
                            success = true;
                        }
                        else
                        {
                            success = GetSecondaryPtNormal( org, norm );
                        }

                        if ( success )
                        {
                            // Create MeshGeom of matching plane.
                            // Vehicle * veh = VehicleMgr.GetVehicle();
                            // string meshid = veh->AddMeshGeom( vsp::SET_NONE, vsp::SET_NONE, false );
                            // Geom* geom_ptr = veh->FindGeom( meshid );
                            // MeshGeom *mg = dynamic_cast<  MeshGeom* > ( geom_ptr );
                            // mg->m_TMeshVec.push_back( MakeSlice( org, norm, 10 ) );
                            // mg->m_SurfDirty = true;
                            // mg->Update();

                            PlaneInterferenceCheck( primary_tm, org, norm, m_LastResult, m_TMeshVec );
                            m_PtsVec = ResultsMgr.GetVec3dResults( m_LastResult, "Pts", 0 );

                            bool interference_flag = true;
                            NameValData* nvd = res->FindPtr( "Interference", 0 );
                            if( nvd )
                            {
                                interference_flag = nvd->GetBool( 0 );
                            }

                            // When interference_flag is true, primary_tm has been placed in m_TMeshVec
                            if ( !interference_flag )
                            {
                                delete primary_tm;
                            }
                        }
                        else
                        {
                            MessageData errMsgData;
                            errMsgData.m_String = "Error";
                            errMsgData.m_IntVec.push_back( vsp::VSP_WRONG_GEOM_TYPE );
                            char buf[255];
                            snprintf( buf, sizeof( buf ), "Error:  Wrong geom type in %s.", m_Name.c_str() );
                            errMsgData.m_StringVec.emplace_back( string( buf ) );

                            MessageMgr::getInstance().SendAll( errMsgData );
                        }
                    }
                    else
                    {
                        MessageData errMsgData;
                        errMsgData.m_String = "Error";
                        errMsgData.m_IntVec.push_back( vsp::VSP_WRONG_GEOM_TYPE );
                        char buf[255];
                        snprintf( buf, sizeof( buf ), "Error:  Empty primary mesh in %s.", m_Name.c_str() );
                        errMsgData.m_StringVec.emplace_back( string( buf ) );

                        MessageMgr::getInstance().SendAll( errMsgData );
                    }
                }
                break;
            }
            case vsp::PLANE_2PT_ANGLE_INTERFERENCE:
            {
                Results *res = ResultsMgr.CreateResults( "Plane_2pt_Angle_Interference", "Two point plane angle interference check." );
                if( res )
                {
                    m_LastResult = res->GetID();
                    primary_tmv = GetPrimaryTMeshVec();

                    if ( !primary_tmv.empty() )
                    {
                        CSGMesh( primary_tmv );
                        FlattenTMeshVec( primary_tmv );
                        TMesh *primary_tm = MergeTMeshVec( primary_tmv );
                        primary_tm->LoadBndBox();

                        vec3d pivot_org, pivot_norm;
                        vec3d pivot_ptaxis, pivot_axis;
                        bool usepivot = false;
                        double mintheta, maxtheta;
                        if ( GetSecondaryPtNormalMeanContactPivotAxis( pivot_org, pivot_norm, pivot_ptaxis, pivot_axis, usepivot, mintheta, maxtheta ) )
                        {
                            // Check un-rotated for collision
                            // If violated, intersect mesh, calculate volume, etc.
                            PlaneInterferenceCheck( primary_tm, pivot_org, pivot_norm, m_LastResult, m_TMeshVec );

                            bool interference_flag = true;
                            NameValData* nvd = res->FindPtr( "Interference", 0 );
                            if( nvd )
                            {
                                interference_flag = nvd->GetBool( 0 );
                            }

                            if ( !interference_flag )
                            {
                                int ccw = 1;
                                if ( m_SecondaryCCWFlag() )
                                {
                                    ccw = -1;
                                }

                                vector < vec3d > tip_pts;
                                vec3d p1, p2;

                                double tip_bogie = 1e12 * M_PI / 180;
                                double tip_wheel = 1e12 * M_PI / 180;;

                                bool skipwheel = false;

                                // Find if there are wheels (at lest) in tandem on bogie.
                                if ( usepivot )
                                {
                                    // Do tilt analysis
                                    tip_bogie = ccw * primary_tm->MinAngle( pivot_org, pivot_norm, pivot_ptaxis, pivot_axis, tip_bogie, ccw, p1, p2 );

                                    // Check back tilt limit
                                    if ( ccw > 0 && tip_bogie > maxtheta )
                                    {
                                        // If exceeded, set bogietheta to limit
                                        tip_bogie = maxtheta;
                                    }
                                    else if ( ccw < 0 && tip_bogie < mintheta )
                                    {
                                        // If exceeded, set bogietheta to limit
                                        tip_bogie = mintheta;
                                    }
                                    else
                                    {
                                        tip_pts.push_back( p1 );
                                        tip_pts.push_back( p2 );
                                        skipwheel = true;
                                    }
                                }
                                else
                                {
                                    tip_bogie = 0;
                                }

                                if ( !skipwheel )
                                {
                                    vec3d aft_org, aft_norm;
                                    vec3d aft_ptaxis, aft_axis;

                                    // Get aft axle parms
                                    if ( ccw > 0 )
                                    {
                                        // Ignore return value because we could not get here with wrong Geom types.
                                        (void) GetSecondaryPtNormalAftAxleAxis( tip_bogie, aft_org, aft_norm, aft_ptaxis, aft_axis );
                                    }
                                    else
                                    {
                                        // Ignore return value because we could nto get here with wrong Geom types.
                                        (void) GetSecondaryPtNormalFwdAxleAxis( tip_bogie, aft_org, aft_norm, aft_ptaxis, aft_axis );
                                    }

                                    // Do tilt analysis
                                    tip_wheel = ccw * primary_tm->MinAngle( aft_org, aft_norm, aft_ptaxis, aft_axis, tip_wheel, ccw, p1, p2 );
                                    tip_pts.push_back( p1 );
                                    tip_pts.push_back( p2 );

                                    vec3d p3 = pivot_ptaxis + RotateArbAxis( p2 - pivot_ptaxis, tip_bogie, pivot_axis );
                                    tip_pts.push_back( p2 );
                                    tip_pts.push_back( p3 );
                                }
                                else
                                {
                                    tip_wheel = 0;
                                }

                                double tip = ( tip_bogie + tip_wheel ) * 180.0 / M_PI;

                                res->Add( new NameValData( "CCW", m_SecondaryCCWFlag(), "CCW tip direction flag." ) );
                                res->Add( new NameValData( "TipBogie", tip_bogie * 180.0 / M_PI, "Bogie tip angle to contact." ) );
                                res->Add( new NameValData( "TipWheel", tip_wheel * 180.0 / M_PI, "Wheel tip angle to contact." ) );
                                res->Add( new NameValData( "Tip", tip, "Total tip angle to contact." ) );
                                res->Add( new NameValData( "TipPts", tip_pts, "Tip contact arc end points." ) );

                                // Over-write previous results values from planar distance check
                                nvd = res->FindPtr( "Con_Val", 0 );
                                if ( nvd )
                                {
                                    nvd->SetDoubleData( { tip } );
                                }

                                nvd = res->FindPtr( "Result", 0 );
                                if ( nvd )
                                {
                                    nvd->SetDoubleData( { tip } );
                                }

                                m_PtsVec.insert( m_PtsVec.end(), tip_pts.begin(), tip_pts.end() );

                                delete primary_tm;
                            }
                        }
                        else
                        {
                            MessageData errMsgData;
                            errMsgData.m_String = "Error";
                            errMsgData.m_IntVec.push_back( vsp::VSP_WRONG_GEOM_TYPE );
                            char buf[255];
                            snprintf( buf, sizeof( buf ), "Error:  Wrong geom type in %s.", m_Name.c_str() );
                            errMsgData.m_StringVec.emplace_back( string( buf ) );

                            MessageMgr::getInstance().SendAll( errMsgData );
                        }
                    }
                    else
                    {
                        MessageData errMsgData;
                        errMsgData.m_String = "Error";
                        errMsgData.m_IntVec.push_back( vsp::VSP_WRONG_GEOM_TYPE );
                        char buf[255];
                        snprintf( buf, sizeof( buf ), "Error:  Empty primary mesh in %s.", m_Name.c_str() );
                        errMsgData.m_StringVec.emplace_back( string( buf ) );

                        MessageMgr::getInstance().SendAll( errMsgData );
                    }
                }
                break;
            }
            case vsp::PLANE_1PT_ANGLE_INTERFERENCE:
            {
                Results *res = ResultsMgr.CreateResults( "Plane_1pt_Angle_Interference", "One point plane angle interference check." );
                if( res )
                {
                    m_LastResult = res->GetID();
                    primary_tmv = GetPrimaryTMeshVec();

                    if ( !primary_tmv.empty() )
                    {
                        CSGMesh( primary_tmv );
                        FlattenTMeshVec( primary_tmv );
                        TMesh *primary_tm = MergeTMeshVec( primary_tmv );
                        primary_tm->LoadBndBox();

                        vec3d pt, normal;
                        vec3d roll_axis;
                        int ysign;
                        if ( GetSecondarySideContactPtRollAxisNormal( pt, roll_axis, normal, ysign ) )
                        {
                            // Check un-rotated for collision
                            // If violated, intersect mesh, calculate volume, etc.
                            PlaneInterferenceCheck( primary_tm, pt, normal, m_LastResult, m_TMeshVec );

                            bool interference_flag = true;
                            NameValData* nvd = res->FindPtr( "Interference", 0 );
                            if( nvd )
                            {
                                interference_flag = nvd->GetBool( 0 );
                            }

                            if ( !interference_flag )
                            {
                                int ccw = -ysign;

                                vector < vec3d > tip_pts;
                                vec3d p1, p2;

                                double roll = 1e12 * M_PI / 180;

                                // Do tilt analysis
                                roll = ccw * primary_tm->MinAngle( pt, normal, pt, roll_axis, roll, ccw, p1, p2 );

                                tip_pts.push_back( p1 );
                                tip_pts.push_back( p2 );

                                res->Add( new NameValData( "RollAngle", roll * 180.0 / M_PI, "Roll angle to contact." ) );
                                res->Add( new NameValData( "RollPts", tip_pts, "Roll contact arc end points." ) );

                                // Over-write previous results values from planar distance check
                                nvd = res->FindPtr( "Con_Val", 0 );
                                if ( nvd )
                                {
                                    nvd->SetDoubleData( { roll * 180.0 / M_PI } );
                                }

                                nvd = res->FindPtr( "Result", 0 );
                                if ( nvd )
                                {
                                    nvd->SetDoubleData( { roll * 180.0 / M_PI } );
                                }

                                m_PtsVec.insert( m_PtsVec.end(), tip_pts.begin(), tip_pts.end() );

                                delete primary_tm;
                            }
                        }
                        else
                        {
                            MessageData errMsgData;
                            errMsgData.m_String = "Error";
                            errMsgData.m_IntVec.push_back( vsp::VSP_WRONG_GEOM_TYPE );
                            char buf[255];
                            snprintf( buf, sizeof( buf ), "Error:  Wrong geom type in %s.", m_Name.c_str() );
                            errMsgData.m_StringVec.emplace_back( string( buf ) );

                            MessageMgr::getInstance().SendAll( errMsgData );
                        }
                    }
                    else
                    {
                        MessageData errMsgData;
                        errMsgData.m_String = "Error";
                        errMsgData.m_IntVec.push_back( vsp::VSP_WRONG_GEOM_TYPE );
                        char buf[255];
                        snprintf( buf, sizeof( buf ), "Error:  Empty primary mesh in %s.", m_Name.c_str() );
                        errMsgData.m_StringVec.emplace_back( string( buf ) );

                        MessageMgr::getInstance().SendAll( errMsgData );
                    }
                }
                break;
            }
            case vsp::GEAR_CG_TIPBACK_ANALYSIS:
            {
                Results *res = ResultsMgr.CreateResults( "Gear_CG_Tipback", "Gear / CG tipback angle analysis." );
                if( res )
                {
                    m_LastResult = res->GetID();

                    vec3d pt, normal, ptaxis, axis;
                    bool usepivot;
                    double mintheta, maxtheta;

                    vec3d cgnom;
                    vector < vec3d > cgbounds;
                    if ( GetPrimaryPtNormalMeanContactPtPivotAxisCG( pt, normal, ptaxis, axis, usepivot, mintheta, maxtheta, cgnom, cgbounds ) )
                    {
                        vec3d p0, p1;
                        double anglenominal = tipback( cgnom, normal, ptaxis, axis, p0, p1 );

                        vector < vec3d > tip_pts;
                        tip_pts.reserve( 12 );
                        tip_pts.push_back( p0 );
                        tip_pts.push_back( p1 );
                        tip_pts.push_back( p0 );
                        tip_pts.push_back( cgnom );


                        double anglemin = 10;
                        double anglemax = -10;
                        vec3d p0min, p1min;
                        vec3d p0max, p1max;
                        int imin;
                        int imax;

                        for ( int i = 0; i < cgbounds.size(); ++i )
                        {
                            double angle = tipback( cgbounds[i], normal, ptaxis, axis, p0, p1 );
                            if ( angle < anglemin )
                            {
                                anglemin = angle;
                                p0min = p0;
                                p1min = p1;
                                imin = i;
                            }
                            if ( angle > anglemax )
                            {
                                anglemax = angle;
                                p0max = p0;
                                p1max = p1;
                                imax = i;
                            }
                        }

                        tip_pts.push_back( p0min );
                        tip_pts.push_back( p1min );
                        tip_pts.push_back( p0min );
                        tip_pts.push_back( cgbounds[ imin ] );

                        tip_pts.push_back( p0max );
                        tip_pts.push_back( p1max );
                        tip_pts.push_back( p0max );
                        tip_pts.push_back( cgbounds[ imax ] );

                        res->Add( new NameValData( "NominalTip", anglenominal * 180.0 / M_PI, "Nominal tipback angle." ) );
                        res->Add( new NameValData( "MinTip", anglemin * 180.0 / M_PI, "Minimum tipback angle." ) );
                        res->Add( new NameValData( "MaxTip", anglemax * 180.0 / M_PI, "Maximum tipback angle." ) );
                        res->Add( new NameValData( "Pts", tip_pts, "Tipback arc end points." ) );
                        res->Add( new NameValData( "Result", anglenominal * 180.0 / M_PI, "Interference result" ) );
                    }
                    else
                    {
                        MessageData errMsgData;
                        errMsgData.m_String = "Error";
                        errMsgData.m_IntVec.push_back( vsp::VSP_WRONG_GEOM_TYPE );
                        char buf[255];
                        snprintf( buf, sizeof( buf ), "Error:  Wrong geom type in %s.", m_Name.c_str() );
                        errMsgData.m_StringVec.emplace_back( string( buf ) );

                        MessageMgr::getInstance().SendAll( errMsgData );
                    }
                }

                m_PtsVec = ResultsMgr.GetVec3dResults( m_LastResult, "Pts", 0 );
                break;
            }
            case vsp::GEAR_WEIGHT_DISTRIBUTION_ANALYSIS:
            {
                Results *res = ResultsMgr.CreateResults( "Gear_Weight_Distribution", "Gear weight distribution analysis." );
                if( res )
                {
                    m_LastResult = res->GetID();

                    vec3d cgnom;
                    vector < vec3d > cgbounds;
                    bool s1 = GetPrimaryCG( cgnom, cgbounds );

                    vector < vec3d > ptvec;
                    vec3d normal;
                    bool s2 = GetPrimaryContactPointVecNormal( ptvec, normal );

                    if ( s1 && s2 )
                    {
                        vec3d resnom = weightdist( cgnom, ptvec, normal );
                        double minnom = resnom[ resnom.minor_comp() ];

                        vector < vec3d > resvec;
                        resvec.resize( cgbounds.size() );
                        for ( int i = 0; i < cgbounds.size(); ++i )
                        {
                            resvec[i] = weightdist( cgbounds[i], ptvec, normal );
                        }

                        // Empty points vector for results.
                        vector < vec3d > pts;
                        res->Add( new NameValData( "NominalReactions", resnom, "Gear reactions for nominal CG." ) );
                        res->Add( new NameValData( "ExcursionReactions", resvec, "Gear reactions for CG range." ) );
                        res->Add( new NameValData( "Pts", pts, "Not used." ) );
                        res->Add( new NameValData( "Result", minnom, "Smallest reaction for nominal CG (typically nose gear reaction)." ) );
                    }
                    else
                    {
                        MessageData errMsgData;
                        errMsgData.m_String = "Error";
                        errMsgData.m_IntVec.push_back( vsp::VSP_WRONG_GEOM_TYPE );
                        char buf[255];
                        snprintf( buf, sizeof( buf ), "Error:  Wrong geom type in %s.", m_Name.c_str() );
                        errMsgData.m_StringVec.emplace_back( string( buf ) );

                        MessageMgr::getInstance().SendAll( errMsgData );
                    }
                }
                break;
            }
            case vsp::GEAR_TIPOVER_ANALYSIS:
            {
                Results *res = ResultsMgr.CreateResults( "Gear_Tipover", "Gear tipover analysis." );
                if( res )
                {
                    m_LastResult = res->GetID();

                    vec3d cgnom;
                    vector < vec3d > cgbounds;
                    bool s1 = GetPrimaryCG( cgnom, cgbounds );

                    vec3d pa, pb, normal;
                    bool s2 = GetPrimaryTwoPtSideContactPtsNormal( pa, pb, normal );

                    if ( s1 && s2 )
                    {

                        vec3d ax = pb - pa;
                        ax.normalize();

                        vec3d p0, p1, p2;
                        double anglenominal = tipover( cgnom, normal, pa, ax, p0, p1 );

                        double l = dist( p0, cgnom );

                        vector < vec3d > tip_pts;
                        tip_pts.reserve( 12 );

                        tip_pts.push_back( p0 );
                        tip_pts.push_back( p1 );

                        tip_pts.push_back( p0 );
                        tip_pts.push_back( cgnom );

                        double anglemin = 10;
                        double anglemax = -10;
                        vec3d p0min, p1min;
                        vec3d p0max, p1max;
                        int imin;
                        int imax;

                        for ( int i = 0; i < cgbounds.size(); ++i )
                        {
                            double angle = tipover( cgbounds[i], normal, pa, ax, p0, p1 );
                            if ( angle < anglemin )
                            {
                                anglemin = angle;
                                p0min = p0;
                                p1min = p1;
                                imin = i;
                            }
                            if ( angle > anglemax )
                            {
                                anglemax = angle;
                                p0max = p0;
                                p1max = p1;
                                imax = i;
                            }
                        }

                        tip_pts.push_back( p0min );
                        tip_pts.push_back( p1min );
                        tip_pts.push_back( p0min );
                        tip_pts.push_back( cgbounds[ imin ] );

                        tip_pts.push_back( p0max );
                        tip_pts.push_back( p1max );
                        tip_pts.push_back( p0max );
                        tip_pts.push_back( cgbounds[ imax ] );

                        res->Add( new NameValData( "NominalTip", anglenominal * 180.0 / M_PI, "Nominal tipover angle." ) );
                        res->Add( new NameValData( "MinTip", anglemin * 180.0 / M_PI, "Minimum tipover angle." ) );
                        res->Add( new NameValData( "MaxTip", anglemax * 180.0 / M_PI, "Maximum tipover angle." ) );
                        res->Add( new NameValData( "Pts", tip_pts, "Tipover arc end points." ) );
                        res->Add( new NameValData( "Result", anglenominal * 180.0 / M_PI, "Interference result" ) );
                    }
                    else
                    {
                        MessageData errMsgData;
                        errMsgData.m_String = "Error";
                        errMsgData.m_IntVec.push_back( vsp::VSP_WRONG_GEOM_TYPE );
                        char buf[255];
                        snprintf( buf, sizeof( buf ), "Error:  Wrong geom type in %s.", m_Name.c_str() );
                        errMsgData.m_StringVec.emplace_back( string( buf ) );

                        MessageMgr::getInstance().SendAll( errMsgData );
                    }
                }

                m_PtsVec = ResultsMgr.GetVec3dResults( m_LastResult, "Pts", 0 );
                break;
            }
            case vsp::GEAR_TURN_ANALYSIS:
            {
                Results *res = ResultsMgr.CreateResults( "Gear_Turn", "Gear turn analysis." );
                if( res )
                {
                    m_LastResult = res->GetID();
                    primary_tmv = GetPrimaryTMeshVec();

                    if ( !primary_tmv.empty() )
                    {
                        CSGMesh( primary_tmv );
                        FlattenTMeshVec( primary_tmv );

                        TMesh *primary_tm = MergeTMeshVec( primary_tmv );
                        primary_tm->LoadBndBox();

                        bool success = false;
                        if ( m_SecondaryType() == vsp::GEOM_TARGET )
                        {
                            Geom* geom = veh->FindGeom( m_SecondaryGeomID );

                            AuxiliaryGeom* auxiliary_ptr = dynamic_cast< AuxiliaryGeom* >( geom );
                            if ( auxiliary_ptr )
                            {
                                vec3d cor;
                                vec3d normal;
                                vector < double > rvec;
                                if ( auxiliary_ptr->CalculateTurn(cor, normal, rvec) )
                                {


                                    vector < vec3d > distpts(2);
                                    double min_dist = -1;
                                    double max_dist = primary_tm->MaxDistanceRay( cor, normal, min_dist, distpts[0], distpts[1] );


                                    vector < vec3d > pts;
                                    pts.push_back( cor );
                                    pts.push_back( cor + normal );

                                    int nseg = 36;

                                    vector < vec3d > cirpts;
                                    for ( int i = 0; i < rvec.size(); ++i )
                                    {
                                        MakeCircle( cor, normal, rvec[i], cirpts, nseg );
                                        pts.insert( pts.end(), cirpts.begin(), cirpts.end() );
                                    }

                                    MakeCircle( distpts[0], normal, max_dist, cirpts, nseg );
                                    pts.insert( pts.end(), cirpts.begin(), cirpts.end() );

                                    res->Add( new NameValData( "Pts", pts, "Visualization of rotation axis and swept circles." ) );
                                    res->Add( new NameValData( "COR", cor, "Center of rotation." ) );
                                    res->Add( new NameValData( "Axis", normal, "Axis of rotation." ) );
                                    res->Add( new NameValData( "GearTurnRadii", rvec, "Radii of gear turning circles." ) );
                                    res->Add( new NameValData( "MaxRadii", max_dist, "Maximum turning radius of primary geometry." ) );
                                    m_PtsVec = ResultsMgr.GetVec3dResults( m_LastResult, "Pts", 0 );
                                    success = true;
                                }
                            }
                        }

                        if ( !success )
                        {
                            MessageData errMsgData;
                            errMsgData.m_String = "Error";
                            errMsgData.m_IntVec.push_back( vsp::VSP_WRONG_GEOM_TYPE );
                            char buf[255];
                            snprintf( buf, sizeof( buf ), "Error:  Wrong geom type in %s.", m_Name.c_str() );
                            errMsgData.m_StringVec.emplace_back( string( buf ) );

                            MessageMgr::getInstance().SendAll( errMsgData );
                        }
                    }
                    else
                    {
                        MessageData errMsgData;
                        errMsgData.m_String = "Error";
                        errMsgData.m_IntVec.push_back( vsp::VSP_WRONG_GEOM_TYPE );
                        char buf[255];
                        snprintf( buf, sizeof( buf ), "Error:  Empty primary mesh in %s.", m_Name.c_str() );
                        errMsgData.m_StringVec.emplace_back( string( buf ) );

                        MessageMgr::getInstance().SendAll( errMsgData );
                    }
                }
                break;
            }
            case vsp::VISIBLE_FROM_POINT_ANALYSIS:
            {
                primary_tmv = GetPrimaryTMeshVec();

                if ( !primary_tmv.empty() )
                {

                    vec3d cen;
                    if ( GetSecondaryPt( cen ) )
                    {
                        m_LastResult = ProjectionMgr.PointVisibility( primary_tmv, cen, m_TMeshVec, m_PolyVisibleFlag(), m_CutoutVec );
                        m_PtsVec = ResultsMgr.GetVec3dResults( m_LastResult, "Pts", 0 );
                    }
                    else
                    {
                        MessageData errMsgData;
                        errMsgData.m_String = "Error";
                        errMsgData.m_IntVec.push_back( vsp::VSP_WRONG_GEOM_TYPE );
                        char buf[255];
                        snprintf( buf, sizeof( buf ), "Error:  Wrong geom type in %s.", m_Name.c_str() );
                        errMsgData.m_StringVec.emplace_back( string( buf ) );

                        MessageMgr::getInstance().SendAll( errMsgData );
                    }
                }
                else
                {
                    MessageData errMsgData;
                    errMsgData.m_String = "Error";
                    errMsgData.m_IntVec.push_back( vsp::VSP_WRONG_GEOM_TYPE );
                    char buf[255];
                    snprintf( buf, sizeof( buf ), "Error:  Empty primary mesh in %s.", m_Name.c_str() );
                    errMsgData.m_StringVec.emplace_back( string( buf ) );

                    MessageMgr::getInstance().SendAll( errMsgData );
                }
                break;
            }
            case vsp::CCE_INTERFERENCE:
            {
                Results *res = ResultsMgr.CreateResults( "Carrier_Composite_Envelope_Interference", "Carrier composite envelope interference check." );
                if( res )
                {
                    m_LastResult = res->GetID();
                    primary_tmv = GetPrimaryTMeshVec();
                    secondary_tmv = GetSecondaryTMeshVec();

                    if ( !primary_tmv.empty() && !secondary_tmv.empty() )
                    {
                        CSGMesh( primary_tmv );
                        FlattenTMeshVec( primary_tmv );
                        TMesh *primary_tm = MergeTMeshVec( primary_tmv );
                        primary_tm->LoadBndBox();

                        TMesh *secondary_tm = new TMesh();
                        secondary_tm->CopyFlatten( secondary_tmv[0] );
                        secondary_tm->LoadBndBox();

                        CCEInterferenceCheck( primary_tm, secondary_tm, m_LastResult, m_TMeshVec );
                        m_PtsVec = ResultsMgr.GetVec3dResults( m_LastResult, "Pts", 0 );
                    }
                    else
                    {
                        MessageData errMsgData;
                        errMsgData.m_String = "Error";
                        errMsgData.m_IntVec.push_back( vsp::VSP_WRONG_GEOM_TYPE );
                        char buf[255];
                        snprintf( buf, sizeof( buf ), "Error:  Empty primary or secondary mesh in %s.", m_Name.c_str() );
                        errMsgData.m_StringVec.emplace_back( string( buf ) );

                        MessageMgr::getInstance().SendAll( errMsgData );
                    }
                }
                break;
            }
            case vsp::LINEAR_SWEPT_VOLUME_ANALYSIS:
            {
                Results *res = ResultsMgr.CreateResults( "Linear_Swept_Volume_Interference", "Linear swept volume interference check." );
                if( res )
                {
                    m_LastResult = res->GetID();
                    primary_tmv = GetPrimaryTMeshVec();
                    secondary_tmv = GetSecondaryTMeshVec();

                    CSGMesh( primary_tmv );
                    FlattenTMeshVec( primary_tmv );
                    TMesh *primary_tm = MergeTMeshVec( primary_tmv );
                    primary_tm->LoadBndBox();

                    CSGMesh( secondary_tmv );
                    FlattenTMeshVec( secondary_tmv );
                    TMesh *secondary_tm = MergeTMeshVec( secondary_tmv );
                    secondary_tm->LoadBndBox();

                    vec3d disp( 0, 0, -100.0 );

                    SweptVolumeInterferenceCheck( primary_tm, secondary_tm, disp, m_LastResult, m_TMeshVec );
                    m_PtsVec = ResultsMgr.GetVec3dResults( m_LastResult, "Pts", 0 );
                }
                break;
            }
        }

        // These are safe for empty vectors.
        DeleteTMeshVec( primary_tmv );
        DeleteTMeshVec( secondary_tmv );


        vector < double > resvec = ResultsMgr.GetDoubleResults( m_LastResult, "Result", 0 );
        if ( resvec.size() > 0 )
        {
            m_LastResultValue = resvec[0];
        }

    }

    UpdateDrawObj_PostAnalysis();

    return m_LastResult;
}

vec3d GeometryAnalysisCase::weightdist( const vec3d &cg, const vector < vec3d > &ptvec, const vec3d &normal )
{
    typedef Eigen::Matrix< double, 3, 3 > mat3;
    typedef Eigen::Matrix< double, 4, 3 > mat43;
    typedef Eigen::Matrix< double, 1, 3 > row3;
    typedef Eigen::Matrix< double, 3, 1 > vec3;

    mat3 moments = mat3::Zero();
    for ( unsigned int i = 0; i < ptvec.size(); ++i )
    {
        vec3d v = ptvec[i] - cg;
        vec3d m = cross( normal, v );

        row3 col;
        m.get_pnt( col );

        moments.col( i ) = col.transpose();
    }

    row3 forces;
    forces << 1, 1, 1;

    mat43 sys;
    sys.topRows( 1 ) = forces;
    sys.bottomRows( 3 ) = moments;

    Eigen::Matrix< double, 4, 1 > rhs;
    rhs << 1, 0, 0, 0;

    Eigen::JacobiSVD < mat43 > svd( sys, Eigen::ComputeThinU | Eigen::ComputeThinV );
    vec3 sol = svd.solve( rhs );

    vec3d res( sol );
    return res;
}

double GeometryAnalysisCase::tipback( const vec3d &cg, const vec3d &normal, const vec3d &ptaxis, const vec3d &axis, vec3d &p0, vec3d &p1 )
{
    const vec3d v = cg - ptaxis;

    const vec3d vpar = dot( v, axis ) * axis;
    const vec3d vperp = v - vpar;
    const vec3d vup = dot( vperp, normal ) * normal;

    p0 = ptaxis + vpar;
    p1 = p0 + vup;
    return signed_angle( vperp, normal, axis );
}

double GeometryAnalysisCase::tipover( const vec3d &cg, const vec3d &normal, const vec3d &ptaxis, const vec3d &axis, vec3d &p0, vec3d &p1 )
{
    const vec3d v = cg - ptaxis;

    const vec3d vpar = dot( v, axis ) * axis;
    const vec3d vperp = v - vpar;

    const vec3d vup = dot( vperp, normal ) * normal;
    const vec3d vrem = vperp - vup;

    p0 = ptaxis + vpar;
    p1 = p0 + vrem;
    return angle( vperp, vrem );
}

void GeometryAnalysisCase::ShowBoth()
{
    ShowPrimary();
    ShowSecondary();
}

void GeometryAnalysisCase::ShowOnlyBoth()
{
    Vehicle *veh = VehicleMgr.GetVehicle();
    if ( veh )
    {
        veh->NoShowSet( vsp::SET_ALL );

        ShowBoth();
    }
}

void GeometryAnalysisCase::ShowPrimary()
{
    Vehicle *veh = VehicleMgr.GetVehicle();
    if ( veh )
    {
        if ( m_PrimaryType() == vsp::SET_TARGET )
        {
            veh->ShowSet( m_PrimarySet() );
        }
        else if ( m_PrimaryType() == vsp::GEOM_TARGET )
        {
            Geom *g = veh->FindGeom( m_PrimaryGeomID );
            if ( g )
            {
                g->Show();
            }
        }
        else if ( m_PrimaryType() == vsp::MODE_TARGET )
        {
            Mode *m = ModeMgr.GetMode( m_PrimaryModeID );
            if ( m )
            {
                m->ApplySettings();

                m->Show();
            }
        }
    }
}

void GeometryAnalysisCase::ShowOnlyPrimary()
{
    Vehicle *veh = VehicleMgr.GetVehicle();
    if ( veh )
    {
        veh->NoShowSet( vsp::SET_ALL );

        ShowPrimary();
    }
}

void GeometryAnalysisCase::ShowSecondary()
{
    Vehicle *veh = VehicleMgr.GetVehicle();
    if ( veh )
    {
        if ( m_SecondaryType() == vsp::SET_TARGET )
        {
            veh->ShowSet( m_SecondarySet() );
        }
        else if ( m_SecondaryType() == vsp::GEOM_TARGET )
        {
            Geom *g = veh->FindGeom( m_SecondaryGeomID );
            if ( g )
            {
                g->Show();
            }
        }
    }
}

void GeometryAnalysisCase::ShowOnlySecondary()
{
    Vehicle *veh = VehicleMgr.GetVehicle();
    if ( veh )
    {
        veh->NoShowSet( vsp::SET_ALL );

        ShowSecondary();
    }
}

void GeometryAnalysisCase::UpdateDrawObj_PostAnalysis()
{
    Material redmat;
    redmat.SetMaterial( "Red Default" );
    redmat.m_Diff[3] = 0.25; // Make translucent

    vector < Material > matvec( m_TMeshVec.size(), redmat );
    vector < vec3d > colorvec( m_TMeshVec.size(), DrawObj::Color( DrawObj::RED ) );

    if ( m_TMeshVec.size() > 0 &&
         m_GeometryAnalysisType() == vsp::LINEAR_SWEPT_VOLUME_ANALYSIS )
    {
        matvec[0].SetMaterialToDefault();
        matvec[0].m_Diff[3] = 0.25; // Make translucent

        colorvec[0] = DrawObj::Color( DrawObj::GRAY );
    }

    m_MeshResultDO_vec.resize( m_TMeshVec.size(), DrawObj() );

    for ( int i = 0 ; i < ( int )m_TMeshVec.size() ; i++ )
    {
        unsigned int num_tris = m_TMeshVec[i]->m_TVec.size();

        unsigned int pi = 0;
        vector<TTri*>& tris = m_TMeshVec[i]->m_TVec;
        m_MeshResultDO_vec[i].m_PntVec.resize( num_tris * 3 );
        m_MeshResultDO_vec[i].m_NormVec.resize( num_tris * 3 );
        for ( int t = 0 ; t < ( int ) num_tris ; t++ )
        {
            m_MeshResultDO_vec[i].m_PntVec[pi] = tris[t]->m_N0->m_Pnt;
            m_MeshResultDO_vec[i].m_PntVec[pi + 1] = tris[t]->m_N1->m_Pnt;
            m_MeshResultDO_vec[i].m_PntVec[pi + 2] = tris[t]->m_N2->m_Pnt;
            m_MeshResultDO_vec[i].m_NormVec[pi] = tris[t]->m_Norm;
            m_MeshResultDO_vec[i].m_NormVec[pi + 1] = tris[t]->m_Norm;
            m_MeshResultDO_vec[i].m_NormVec[pi + 2] = tris[t]->m_Norm;
            pi += 3;
        }

        // Flag the DrawObjects as changed
        m_MeshResultDO_vec[i].m_GeomChanged = true;

        for ( int j = 0; j < 4; j++ )
        {
            m_MeshResultDO_vec[i].m_MaterialInfo.Ambient[j] = (float)matvec[i].m_Ambi[j];
            m_MeshResultDO_vec[i].m_MaterialInfo.Diffuse[j] = (float)matvec[i].m_Diff[j];
            m_MeshResultDO_vec[i].m_MaterialInfo.Specular[j] = (float)matvec[i].m_Spec[j];
            m_MeshResultDO_vec[i].m_MaterialInfo.Emission[j] = (float)matvec[i].m_Emis[j];
        }
        m_MeshResultDO_vec[i].m_MaterialInfo.Shininess = (float)matvec[i].m_Shininess;

        m_MeshResultDO_vec[i].m_LineColor = colorvec[ i ];

        char str[255];
        snprintf( str, sizeof( str ),  "_%d", i );
        m_MeshResultDO_vec[i].m_GeomID = m_ID + str;
        m_MeshResultDO_vec[i].m_Screen = DrawObj::VSP_MAIN_SCREEN;
        m_MeshResultDO_vec[i].m_Type = DrawObj::VSP_WIRE_SHADED_TRIS;
    }

    m_LineResultDO.m_GeomID = m_ID + "Line";
    m_LineResultDO.m_Screen = DrawObj::VSP_MAIN_SCREEN;
    m_LineResultDO.m_Type = DrawObj::VSP_LINES;
    m_LineResultDO.m_LineColor = DrawObj::Color( DrawObj::RED );
    m_LineResultDO.m_LineWidth = 3.0;

    m_LineResultDO.m_GeomChanged = true;
    m_LineResultDO.m_PntVec = m_PtsVec;
}

void GeometryAnalysisCase::UpdateDrawObj_Live()
{
    m_SecondaryVizPointDO.m_PntVec.clear();
    if ( m_GeometryAnalysisType() == vsp::VISIBLE_FROM_POINT_ANALYSIS )
    {
        vec3d pt;
        if ( GetSecondaryPt( pt ) )
        {
            m_SecondaryVizPointDO.m_GeomID = m_ID + "VizPoint";
            m_SecondaryVizPointDO.m_Screen = DrawObj::VSP_MAIN_SCREEN;
            m_SecondaryVizPointDO.m_Type = DrawObj::VSP_POINTS;
            m_SecondaryVizPointDO.m_PntVec.resize( 1 );
            m_SecondaryVizPointDO.m_PntVec[ 0 ] = pt;
            m_SecondaryVizPointDO.m_PointColor = DrawObj::Color( DrawObj::RED );
            m_SecondaryVizPointDO.m_PointSize = 20.0;
            m_SecondaryVizPointDO.m_GeomChanged = true;
        }
    }
}


void GeometryAnalysisCase::LoadDrawObjs( vector< DrawObj* > & draw_obj_vec )
{
    for ( int i = 0 ; i < ( int )m_MeshResultDO_vec.size() ; i++ )
    {
        m_MeshResultDO_vec[i].m_Visible = true;
        draw_obj_vec.push_back( &m_MeshResultDO_vec[i] );
    }

    m_LineResultDO.m_Visible = true;
    draw_obj_vec.push_back( &m_LineResultDO );

    if ( m_GeometryAnalysisType() == vsp::VISIBLE_FROM_POINT_ANALYSIS )
    {
        m_SecondaryVizPointDO.m_Visible = true;
        draw_obj_vec.push_back( &m_SecondaryVizPointDO );
    }
    else
    {
        m_SecondaryVizPointDO.m_Visible = false;
    }
}

//===============================================================================//
//===============================================================================//
//===============================================================================//


GeometryAnalysisMgrSingleton::GeometryAnalysisMgrSingleton()
{

}

GeometryAnalysisMgrSingleton::~GeometryAnalysisMgrSingleton()
{
    Wype();
}

xmlNodePtr GeometryAnalysisMgrSingleton::EncodeXml( xmlNodePtr & node )
{
    xmlNodePtr geometryanalysismgr_node = xmlNewChild( node, NULL, BAD_CAST"GeometryAnalysisMgr", NULL );

    if ( geometryanalysismgr_node )
    {
        for ( int i = 0; i < m_GeometryAnalysisVec.size(); i++ )
        {
            m_GeometryAnalysisVec[i]->EncodeXml( geometryanalysismgr_node );
        }
    }

    return geometryanalysismgr_node;
}

xmlNodePtr GeometryAnalysisMgrSingleton::DecodeXml( xmlNodePtr & node )
{
    xmlNodePtr geometryanalysismgr_node = XmlUtil::GetNode( node, "GeometryAnalysisMgr", 0 );

    if ( geometryanalysismgr_node )
    {


        int num_gcase = XmlUtil::GetNumNames( geometryanalysismgr_node, "GeometryAnalysis" );
        for ( int i = 0; i < num_gcase; i++ )
        {
            xmlNodePtr gcasenode = XmlUtil::GetNode( geometryanalysismgr_node, "GeometryAnalysis", i );

            if ( gcasenode )
            {
                string id = AddGeometryAnalysis();
                GeometryAnalysisCase* gcase = GetGeometryAnalysis( id );

                if ( gcase )
                {
                    gcase->DecodeXml( gcasenode );
                }
            }
        }
    }

    return node;
}

void GeometryAnalysisMgrSingleton::Renew()
{
    Wype();
}

void GeometryAnalysisMgrSingleton::Wype()
{
    for ( int i = 0; i < (int)m_GeometryAnalysisVec.size(); i++ )
    {
        delete m_GeometryAnalysisVec[i];
    }
    m_GeometryAnalysisVec.clear();
}

void GeometryAnalysisMgrSingleton::Update()
{
    for ( int i = 0; i < (int)m_GeometryAnalysisVec.size(); i++ )
    {
        m_GeometryAnalysisVec[i]->Update();
    }

}

string GeometryAnalysisMgrSingleton::EvaluateAll()
{
    std::vector <string> res_id_vector;

    for ( int i = 0; i < (int)m_GeometryAnalysisVec.size(); i++ )
    {
        string rid = m_GeometryAnalysisVec[i]->Evaluate();
        res_id_vector.push_back( rid );
    }

    Results *res = ResultsMgr.CreateResults( "GeometryAnalysisAll", "All geometry analysis results for model." );
    if( res )
    {
        res->Add( new NameValData( "ResultsVec", res_id_vector, "ID's of geometry analysis results." ) );
        return res->GetID();
    }
    return string();
}

void GeometryAnalysisMgrSingleton::AddLinkableContainers( vector< string > & linkable_container_vec )
{
    for ( int i = 0; i < (int)m_GeometryAnalysisVec.size(); i++ )
    {
        if ( m_GeometryAnalysisVec[i] )
        {
            m_GeometryAnalysisVec[i]->AddLinkableContainers( linkable_container_vec );
        }
    }
}

string GeometryAnalysisMgrSingleton::AddGeometryAnalysis()
{
    GeometryAnalysisCase* ic = new GeometryAnalysisCase();

    m_GeometryAnalysisVec.push_back( ic );
    return ic->GetID();
}

void GeometryAnalysisMgrSingleton::DeleteGeometryAnalysis( const string &id )
{
    int indx = GetGeometryAnalysisIndex( id );
    DeleteGeometryAnalysis( indx );
}

void GeometryAnalysisMgrSingleton::DeleteGeometryAnalysis( int indx )
{
    if ( indx >= 0 && indx < m_GeometryAnalysisVec.size() )
    {
        delete m_GeometryAnalysisVec[ indx ];

        m_GeometryAnalysisVec.erase( m_GeometryAnalysisVec.begin() + indx );
    }
}

void GeometryAnalysisMgrSingleton::DeleteAllGeometryAnalyses()
{
    for ( int i = 0; i < (int)m_GeometryAnalysisVec.size(); i++ )
    {
        delete m_GeometryAnalysisVec[i];
    }
    m_GeometryAnalysisVec.clear();
}

GeometryAnalysisCase * GeometryAnalysisMgrSingleton::GetGeometryAnalysis( int indx ) const
{
    if ( indx >= 0 && indx < m_GeometryAnalysisVec.size() )
    {
        return m_GeometryAnalysisVec[ indx ];
    }
    return nullptr;
}

GeometryAnalysisCase * GeometryAnalysisMgrSingleton::GetGeometryAnalysis( const string &id ) const
{
    for ( int i = 0; i < (int)m_GeometryAnalysisVec.size(); i++ )
    {
        if ( m_GeometryAnalysisVec[i]->GetID() == id )
        {
            return m_GeometryAnalysisVec[i];
        }
    }

    return nullptr;
}

int GeometryAnalysisMgrSingleton::GetGeometryAnalysisIndex( const string &id ) const
{
    for ( int i = 0; i < (int)m_GeometryAnalysisVec.size(); i++ )
    {
        if ( m_GeometryAnalysisVec[i]->GetID() == id )
        {
            return i;
        }
    }

    return -1;
}
