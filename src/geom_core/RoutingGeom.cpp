//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#include "APIDefines.h"
#include "VspUtil.h"
#include "RoutingGeom.h"
#include "Vehicle.h"
#include "VSP_Geom_API.h"
#include "WingGeom.h"
#include <cfloat>  //For DBL_EPSILON
#include "ParmMgr.h"
#include "StlHelper.h"

using namespace vsp;

//=========================================================================//
//========================        RoutingPoint       ======================//
//=========================================================================//

RoutingPoint::RoutingPoint()
{

    m_SurfIndx.Init( "SurfIndx", "RoutePt", this, 0, 0, 1e6 );

    m_CoordType.Init( "CoordType", "RoutePt", this, vsp::ROUTE_PT_UV, vsp::ROUTE_PT_COMP, vsp::ROUTE_PT_NUM_TYPES - 1 );
    m_DeltaType.Init( "DeltaType", "RoutePt", this, vsp::REL, vsp::ABS, vsp::REL );

    m_U.Init( "U", "RoutePt", this, 0.5, 0.0, 1.0 );
    m_U0N.Init( "U0N", "RoutePt", this, 0, 0, 1e12 );
    m_U01Flag.Init( "U_01Flag", "RoutePt", this, true, false, true );
    m_W.Init( "W", "RoutePt", this, 0.5, 0.0, 1.0 );

    m_R.Init( "R", "RoutePt", this, 0.0, 0.0, 1.0 );
    m_R01Flag.Init( "R_01", "RoutePt", this, true, false, true );
    m_R0N.Init( "R0N", "RoutePt", this, 0, 0, 1e12 );
    m_S.Init( "S", "RoutePt", this, 0.5, 0.0, 1.0 );
    m_T.Init( "T", "RoutePt", this, 0.5, 0.0, 1.0 );

    m_L.Init( "L", "RoutePt", this, 0.0, 0.0, 1.0 );
    m_L01Flag.Init( "L_01", "RoutePt", this, true, false, true );
    m_L0Len.Init( "L0Len", "RoutePt", this, 0, 0, 1e12 );
    m_M.Init( "M", "RoutePt", this, 0.5, 0.0, 1.0 );
    m_N.Init( "N", "RoutePt", this, 0.5, 0.0, 1.0 );

    m_Eta.Init( "Eta", "RoutePt", this, 0.0, 0.0, 1.0 );

    m_DeltaX.Init( "DeltaX", "RoutePt", this, 0.0, -1.0e12, 1.0e12 );
    m_DeltaY.Init( "DeltaY", "RoutePt", this, 0.0, -1.0e12, 1.0e12 );
    m_DeltaZ.Init( "DeltaZ", "RoutePt", this, 0.0, -1.0e12, 1.0e12 );

    m_DeltaXRel.Init( "DeltaXRel", "RoutePt", this, 0.0, -1.0e12, 1.0e12 );
    m_DeltaYRel.Init( "DeltaYRel", "RoutePt", this, 0.0, -1.0e12, 1.0e12 );
    m_DeltaZRel.Init( "DeltaZRel", "RoutePt", this, 0.0, -1.0e12, 1.0e12 );

    m_Placed = true;
}

void RoutingPoint::Update()
{
    UpdateParms();

    Vehicle* veh = VehicleMgr.GetVehicle();

    if ( veh )
    {
        Geom * g = veh->FindGeom( m_ParentID );
        if ( g )
        {
            Matrix4d transMat;
            Matrix4d rotMat;

            Matrix4d parentMat;
            parentMat = g->getModelMatrix();
            double tempMat[16];
            parentMat.getMat( tempMat );

            WingGeom* wing_parent = dynamic_cast < WingGeom * > ( g );

            // Parent CompXXXCoordSys methods query the positioned m_SurfVec[0] surface,
            // not m_MainSurfVec[0].  Consequently, m_ModelMatrix is already implied in
            // these calculations and does not need to be applied again.

            if ( m_SurfIndx() >= 0 && m_SurfIndx() < g->GetNumTotalSurfs() )
            {
                switch ( m_CoordType() )
                {
                    case vsp::ROUTE_PT_UV:
                        g->CompTransCoordSys( m_SurfIndx(), m_U(), m_W(), transMat );
                        g->CompRotCoordSys( m_SurfIndx(), m_U(), m_W(), rotMat );
                        break;
                    case vsp::ROUTE_PT_RST:
                        g->CompTransCoordSysRST( m_SurfIndx(), m_R(), m_S(), m_T(), transMat );
                        g->CompRotCoordSysRST( m_SurfIndx(), m_R(), m_S(), m_T(), rotMat );
                        break;
                    case vsp::ROUTE_PT_LMN:
                        g->CompTransCoordSysLMN( m_SurfIndx(), m_L(), m_M(), m_N(), transMat );
                        g->CompRotCoordSysLMN( m_SurfIndx(), m_L(), m_M(), m_N(), rotMat );
                        break;
                    case vsp::ROUTE_PT_EtaMN:
                    {
                        double l = m_Eta();

                        if (wing_parent)
                        {
                            double umax = g->GetMainUMapMax( m_SurfIndx() );
                            double u = wing_parent->EtatoU( m_Eta()) / umax;

                            double r = u;
                            g->ConvertRtoL( m_SurfIndx(), r, l );
                        }

                        g->CompTransCoordSysLMN( m_SurfIndx(), l, m_M(), m_N(), transMat );
                        g->CompRotCoordSysLMN( m_SurfIndx(), l, m_M(), m_N(), rotMat );
                    }
                        break;
                    case vsp::ROUTE_PT_COMP:
                        transMat.translatef( tempMat[ 12 ], tempMat[ 13 ], tempMat[ 14 ] );

                        tempMat[12] = tempMat[13] = tempMat[14] = 0;
                        rotMat.initMat( tempMat );
                        break;
                }
            }
            else
            {
                transMat.translatef( tempMat[12], tempMat[13], tempMat[14] );

                tempMat[12] = tempMat[13] = tempMat[14] = 0;
                rotMat.initMat( tempMat );
            }

            Matrix4d absMat;
            absMat.initMat( transMat.data() );
            transMat.matMult( rotMat.data() );

            vec3d pt;
            if ( m_DeltaType() == vsp::REL )
            {
                pt = vec3d( m_DeltaXRel(), m_DeltaYRel(), m_DeltaZRel() );
                m_Pt = transMat.xform( pt );

                absMat.affineInverse();

                vec3d abspt = absMat.xform( m_Pt );

                m_DeltaX = abspt.x();
                m_DeltaY = abspt.y();
                m_DeltaZ = abspt.z();
            }
            else
            {
                pt = vec3d( m_DeltaX(), m_DeltaY(), m_DeltaZ() );
                m_Pt = absMat.xform( pt );

                transMat.affineInverse();

                vec3d relpt = transMat.xform( m_Pt );

                m_DeltaXRel = relpt.x();
                m_DeltaYRel = relpt.y();
                m_DeltaZRel = relpt.z();
            }


        }
    }

    m_LateUpdateFlag = false;
}

vec3d RoutingPoint::GetPt()
{
    if ( m_LateUpdateFlag )
    {
        Update();
    }

    return m_Pt;
}

void RoutingPoint::SetParentID( const string &id )
{
    m_ParentID = id;

    //==== Notify Parent Container (XSecSurf) ====//
    ParmContainer* pc = GetParentContainerPtr();
    if ( pc )
    {
        RoutingGeom * rg = dynamic_cast< RoutingGeom* >( pc );
        if ( rg )
        {
            rg->UpdateParents();
            rg->Update();
        }
    }
}

void RoutingPoint::UpdateParms()
{
    Vehicle* veh = VehicleMgr.GetVehicle();

    Geom* parent = veh->FindGeom( GetParentID() );

    if ( parent )
    {
        if ( parent->GetType().m_Type == MESH_GEOM_TYPE ||
             parent->GetType().m_Type == WIRE_FRAME_GEOM_TYPE ||
             parent->GetType().m_Type == BLANK_GEOM_TYPE ||
             parent->GetType().m_Type == HINGE_GEOM_TYPE ||
             parent->GetType().m_Type == HUMAN_GEOM_TYPE ||
             parent->GetType().m_Type == PT_CLOUD_GEOM_TYPE )
        {
            return;
        }

        WingGeom* wing_parent = dynamic_cast < WingGeom * > ( parent );

        const VspSurf * surf = parent->GetMainSurfPtr( parent->GetMainSurfID( m_SurfIndx() ) );

        if ( surf ) // May not be up-to-date if parent has not been updated.
        {
            double umax = surf->GetUMapMax();
            double lmax = surf->GetLMax();

            m_U0N.SetUpperLimit( umax );
            m_R0N.SetUpperLimit( umax );
            m_L0Len.SetUpperLimit( lmax );

            if ( m_U01Flag.Get() )
            {
                m_U0N.Set( m_U() * umax );
            }
            else
            {
                double val = clamp( m_U0N(), 0.0, umax );
                m_U0N.Set( val );
                m_U.Set( val / umax );
            }

            if ( m_R01Flag.Get() )
            {
                m_R0N.Set( m_R() * umax );
            }
            else
            {
                double val = clamp( m_R0N(), 0.0, umax );
                m_R0N.Set( val );
                m_R.Set( val / umax );
            }

            if ( m_L01Flag.Get() )
            {
                m_L0Len.Set( m_L() * lmax );
            }
            else
            {
                double val = clamp( m_L0Len(), 0.0, lmax );
                m_L0Len.Set( val );
                m_L.Set( val / lmax );
            }

            if ( wing_parent )
            {
                if ( m_CoordType() == vsp::ROUTE_PT_EtaMN ) // Eta is active.
                {
                    double u = wing_parent->EtatoU( m_Eta() ) / umax;

                    double r;
                    r = u;

                    double l;
                    parent->ConvertRtoL( m_SurfIndx(), r, l );

                    double s, t;
                    parent->ConvertLMNtoRST( m_SurfIndx(), l, m_M(), m_N(), r, s, t );

                    double w;
                    if ( t < 0.5 )
                    {
                        w = 0.5 * s;
                    }
                    else
                    {
                        w = 1.0 - 0.5 * s;
                    }

                    m_U.Set( u );
                    m_U0N.Set( m_U() * umax );
                    m_W.Set( w );

                    m_R.Set( r );
                    m_R0N.Set( m_R() * umax );
                    m_S.Set( s );
                    m_T.Set( t );

                    m_L.Set( l );
                    m_L0Len.Set( m_L() * lmax );
                }
                else if ( m_CoordType() == vsp::ROUTE_PT_UV ) // UV is active
                {
                    m_Eta = wing_parent->UtoEta( m_U() * umax );
                }
                else if ( m_CoordType() == vsp::ROUTE_PT_RST ) // RST is active
                {
                    double r = m_R();
                    double u = r;
                    m_Eta = wing_parent->UtoEta( u * umax );
                }
                else if ( m_CoordType() == vsp::ROUTE_PT_LMN ) // LMN is active
                {
                    double l = m_L();
                    double r;
                    parent->ConvertLtoR( m_SurfIndx(), l, r );
                    double u = r;
                    m_Eta = wing_parent->UtoEta( u * umax );
                }
                else // Nothing is active, use U value anyway.
                {
                    m_Eta = wing_parent->UtoEta( m_U() * umax );
                }
            }

            if ( m_CoordType() == vsp::ROUTE_PT_UV )
            {
                double u, w;
                u = m_U();
                w = m_W();
                double r, s, t;
                r = u;
                s = 2.0 * w;
                t = 0.0;
                if ( w > 0.5 )
                {
                    s = 2.0 * ( 1.0 - w );
                    t = 1.0;
                }

                m_R.Set( r );
                m_R0N.Set( m_R() * umax );
                m_S.Set( s );
                m_T.Set( t );

                double l, m, n;

                parent->ConvertRSTtoLMN( m_SurfIndx(), r, s, t, l, m, n );
                m_L.Set( l );
                m_L0Len.Set( m_L() * lmax );
                m_M.Set( m );
                m_N.Set( n );
            }

            if ( m_CoordType() == vsp::ROUTE_PT_RST )
            {
                double u, w;
                double r = m_R();
                double s = m_S();
                double t = m_T();

                u = r;
                if ( t < 0.5 )
                {
                    w = 0.5 * s;
                }
                else
                {
                    w = 1.0 - 0.5 * s;
                }

                m_U.Set( u );
                m_U0N.Set( m_U() * umax );
                m_W.Set( w );

                double l, m, n;
                parent->ConvertRSTtoLMN( m_SurfIndx(), m_R(), m_S(), m_T(), l, m, n );
                m_L.Set( l );
                m_L0Len.Set( m_L() * lmax );
                m_M.Set( m );
                m_N.Set( n );
            }

            if ( m_CoordType() == vsp::ROUTE_PT_LMN )
            {
                double u, w;
                double r, s, t;

                parent->ConvertLMNtoRST( m_SurfIndx(), m_L(), m_M(), m_N(), r, s, t );

                u = r;
                if ( t < 0.5 )
                {
                    w = 0.5 * s;
                }
                else
                {
                    w = 1.0 - 0.5 * s;
                }

                m_U.Set( u );
                m_U0N.Set( m_U() * umax );
                m_W.Set( w );

                parent->ConvertLMNtoRST( m_SurfIndx(), m_L(), m_M(), m_N(), r, s, t );
                m_R.Set( r );
                m_R0N.Set( m_R() * umax );
                m_S.Set( s );
                m_T.Set( t );
            }
        }
    }

    m_U.Activate();
    m_U0N.Activate();
    m_W.Activate();
    m_R.Activate();
    m_R0N.Activate();
    m_S.Activate();
    m_T.Activate();
    m_L.Activate();
    m_L0Len.Activate();
    m_M.Activate();
    m_N.Activate();
    m_CoordType.Activate();

    m_DeltaX.Activate();
    m_DeltaY.Activate();
    m_DeltaZ.Activate();

    m_DeltaXRel.Activate();
    m_DeltaYRel.Activate();
    m_DeltaZRel.Activate();

    if ( m_U01Flag() )
    {
        m_U0N.Deactivate();
    }
    else
    {
        m_U.Deactivate();
    }

    if ( m_R01Flag() )
    {
        m_R0N.Deactivate();
    }
    else
    {
        m_R.Deactivate();
    }

    if ( m_L01Flag() )
    {
        m_L0Len.Deactivate();
    }
    else
    {
        m_L.Deactivate();
    }

    if ( m_DeltaType() == vsp::REL )
    {
        m_DeltaX.Deactivate();
        m_DeltaY.Deactivate();
        m_DeltaZ.Deactivate();
    }
    else
    {
        m_DeltaXRel.Deactivate();
        m_DeltaYRel.Deactivate();
        m_DeltaZRel.Deactivate();
    }
}

xmlNodePtr RoutingPoint::EncodeXml( xmlNodePtr & node )
{
    xmlNodePtr routingpoint_node = xmlNewChild( node, NULL, BAD_CAST"RoutingPoint", NULL );

    ParmContainer::EncodeXml( routingpoint_node );

    if ( routingpoint_node )
    {
        XmlUtil::AddStringNode( routingpoint_node, "ParentID", m_ParentID );
    }

    return routingpoint_node;
}

xmlNodePtr RoutingPoint::DecodeXml( xmlNodePtr & node )
{
    ParmContainer::DecodeXml( node );

    if ( node )
    {
        m_ParentID = ParmMgr.RemapID( XmlUtil::FindString( node, "ParentID", m_ParentID ) );
    }

    return node;
}

//==== Parm Changed ====//
void RoutingPoint::ParmChanged( Parm* parm_ptr, int type )
{
    if ( type == Parm::SET )
    {
        m_LateUpdateFlag = true;
    }
    else
    {
        Update();
    }

    //==== Notify Parent Container (XSecSurf) ====//
    ParmContainer* pc = GetParentContainerPtr();
    if ( pc )
    {
        pc->ParmChanged( parm_ptr, type );
    }
}

//=========================================================================//
//=========================        RoutingGeom       ======================//
//=========================================================================//

//==== Constructor ====//
RoutingGeom::RoutingGeom( Vehicle* vehicle_ptr ) : Geom( vehicle_ptr )
{
    m_Name = "RoutingGeom";
    m_Type.m_Name = "Routing";
    m_Type.m_Type = ROUTING_GEOM_TYPE;

    m_Picking = false;
    m_ActivePointIndex = -1;

    m_RouteLineDO.m_Type = DrawObj::VSP_LINE_STRIP;
    m_RouteLineDO.m_Screen = DrawObj::VSP_MAIN_SCREEN;
    m_RouteLineDO.m_LineWidth = 2.0;
    m_RouteLineDO.m_LineColor = vec3d( 0.0, 0.0, 0.0 );

    m_DynamicRouteDO.m_Type = DrawObj::VSP_ROUTING;
    m_DynamicRouteDO.m_Screen = DrawObj::VSP_MAIN_SCREEN;

    m_ActivePointDO.m_Type = DrawObj::VSP_POINTS;
    m_ActivePointDO.m_Screen = DrawObj::VSP_MAIN_SCREEN;
    m_ActivePointDO.m_PointSize = 10;
    m_ActivePointDO.m_PointColor = vec3d( 0, 0, 1 );

}

//==== Destructor ====//
RoutingGeom::~RoutingGeom()
{
    for ( int i = 0; i < m_RoutingPointVec.size(); i++ )
    {
        delete m_RoutingPointVec[i];
    }
    m_RoutingPointVec.clear();

}

xmlNodePtr RoutingGeom::EncodeXml( xmlNodePtr & node )
{
    Geom::EncodeXml( node );

    xmlNodePtr routingpoints_node = xmlNewChild( node, NULL, BAD_CAST "RoutingPoints", NULL );

    for ( int i = 0; i < ( int )m_RoutingPointVec.size(); i++ )
    {
        m_RoutingPointVec[i]->EncodeXml( routingpoints_node );
    }

    return routingpoints_node;
}

xmlNodePtr RoutingGeom::DecodeXml( xmlNodePtr & node )
{
    Geom::DecodeXml( node );

    xmlNodePtr routingpoints_node = XmlUtil::GetNode( node, "RoutingPoints", 0 );

    if ( routingpoints_node )
    {
        int num = XmlUtil::GetNumNames( routingpoints_node, "RoutingPoint" );
        for ( int i = 0 ; i < num ; i++ )
        {
            xmlNodePtr rpt_node = XmlUtil::GetNode( routingpoints_node, "RoutingPoint", i );
            if ( rpt_node )
            {
                RoutingPoint* rpt = AddPt();
                rpt->DecodeXml( rpt_node );
            }
        }
    }

    UpdateParents();
    return routingpoints_node;
}

void RoutingGeom::ChangeID( string id )
{
    Geom::ChangeID( id );

    for ( int i = 0; i < m_RoutingPointVec.size(); i++ )
    {
        m_RoutingPointVec[i]->SetParentContainer( id );
    }
}

void RoutingGeom::UpdateParents()
{
    m_SurfDirty = true;

    Vehicle* veh = VehicleMgr.GetVehicle();

    if ( !veh )
    {
        return;
    }

    vector < string > parent_vec;
    parent_vec.reserve( m_RoutingPointVec.size() );
    for ( int i = 0; i < m_RoutingPointVec.size(); i++ )
    {
        parent_vec.push_back( m_RoutingPointVec[i]->GetParentID() );
    }

    std::sort( parent_vec.begin(), parent_vec.end() );
    parent_vec.erase(std::unique( parent_vec.begin(), parent_vec.end()), parent_vec.end() );

    // Serialize m_ParmIDs into single long string.
    string str = string_vec_serialize( parent_vec );
    // Calculate hash to detect changes in m_ParmIDs
    std::size_t str_hash = std::hash < std::string >{}( str );

    // Relies on currency of m_ParmIDs by UpdateVarBrowser()
    if ( str_hash != m_ParentHash )
    {
        // Remove parents.
        for ( int i = 0; i < m_ParentVec.size(); i++ )
        {
            Geom * g = veh->FindGeom( m_ParentVec[i] );
            if ( g )
            {
                g->RemoveStepChildID( m_ID );
            }
        }

        // Add to parents.
        for ( int i = 0; i < parent_vec.size(); i++ )
        {
            Geom * g = veh->FindGeom( parent_vec[i] );
            if ( g )
            {
                g->AddStepChildID( m_ID );
            }
        }

        m_ParentVec = parent_vec;
        m_ParentHash = str_hash;
    }
}


void RoutingGeom::ComputeCenter()
{
}

void RoutingGeom::Scale()
{
    double currentScale = m_Scale() / m_LastScale();

    m_LastScale = m_Scale();
}

void RoutingGeom::AddDefaultSources( double base_len )
{
}

void RoutingGeom::OffsetXSecs( double off )
{
}


RoutingPoint* RoutingGeom::AddPt()
{
    RoutingPoint *rpt = new RoutingPoint();

    rpt->SetParentContainer( m_ID );

    m_RoutingPointVec.push_back( rpt );
    UpdateParents();
    return rpt;
}

RoutingPoint* RoutingGeom::InsertPt( int index )
{
    if ( index >= 0 && index <= m_RoutingPointVec.size() )
    {
        RoutingPoint *rpt = new RoutingPoint();

        rpt->SetParentContainer( m_ID );

        m_RoutingPointVec.insert( m_RoutingPointVec.begin() + index, rpt );
        UpdateParents();
        return rpt;
    }
    return nullptr;
}

void RoutingGeom::DelPt( int index )
{
    if ( index >= 0 && index < m_RoutingPointVec.size() )
    {
        RoutingPoint *rpt = m_RoutingPointVec[ index ];
        m_RoutingPointVec.erase( m_RoutingPointVec.begin() + index );
        UpdateParents();
        delete rpt;
    }
}

void RoutingGeom::DelAllPt()
{
    for ( int i = 0; i < m_RoutingPointVec.size(); i++ )
    {
        delete m_RoutingPointVec[i];
    }
    m_RoutingPointVec.clear();
    UpdateParents();
}

int RoutingGeom::MovePt( int index, int reorder_type )
{
    if ( index >= 0 && index < m_RoutingPointVec.size() )
    {
        int newindx = ReorderVectorIndex( m_RoutingPointVec, index, reorder_type );
        UpdateParents();
        return newindx;
    }
    return index;
}

RoutingPoint * RoutingGeom::GetPt( int index )
{
    if ( index >= 0 && index < m_RoutingPointVec.size() )
    {
        return m_RoutingPointVec[ index ];
    }
    return nullptr;
}

void RoutingGeom::SetPlaced( int index, bool p )
{
    if ( index >= 0 && index < m_RoutingPointVec.size() )
    {
        m_RoutingPointVec[ index ]->SetPlaced( p );
    }
}

bool RoutingGeom::IsPlaced( int index ) const
{
    if ( index >= 0 && index < m_RoutingPointVec.size() )
    {
        return m_RoutingPointVec[ index ]->IsPlaced();
    }
    return false; // Not sure if this is the right default.
}

void RoutingGeom::UpdateSurf()
{
    DisableParms();

    for ( int i = 0; i < m_RoutingPointVec.size(); i++ )
    {
        m_RoutingPointVec[i]->Update();
    }
}


void RoutingGeom::DisableParms()
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
    // m_CapUMinOption = geom_ptr->m_CapUMinOption();
    // m_CapUMinTess   = geom_ptr->m_CapUMinTess();
    // m_CapUMaxOption = geom_ptr->m_CapUMaxOption();
    //
    // m_CapUMinLength = geom_ptr->m_CapUMinLength();
    // m_CapUMinOffset = geom_ptr->m_CapUMinOffset();
    // m_CapUMinStrength = geom_ptr->m_CapUMinStrength();
    // m_CapUMinSweepFlag = geom_ptr->m_CapUMinSweepFlag();
    //
    // m_CapUMaxLength = geom_ptr->m_CapUMaxLength();
    // m_CapUMaxOffset = geom_ptr->m_CapUMaxOffset();
    // m_CapUMaxStrength = geom_ptr->m_CapUMaxStrength();
    // m_CapUMaxSweepFlag = geom_ptr->m_CapUMaxSweepFlag();

    //=== Let User Change Tess
    //m_TessU = geom_ptr->m_TessU();
    //m_TessW = geom_ptr->m_TessW();

    // m_SymAncestor = geom_ptr->m_SymAncestor();
    // if ( m_SymAncestor() != 0 ) // Not global ancestor.
    // {
    //     m_SymAncestor = m_SymAncestor() + 1;  // + 1 increment for parent
    // }
    // m_SymAncestOriginFlag = geom_ptr->m_SymAncestOriginFlag();
    // m_SymPlanFlag = geom_ptr->m_SymPlanFlag();
    // m_SymAxFlag = geom_ptr->m_SymAxFlag();
    // m_SymRotN = geom_ptr->m_SymRotN();

    m_SymAncestor.Deactivate();
    m_SymAncestOriginFlag.Deactivate();
    m_SymPlanFlag.Deactivate();
    m_SymAxFlag.Deactivate();
    m_SymRotN.Deactivate();
}

void RoutingGeom::UpdateBBox()
{
    //==== Load Bounding Box ====//
    BndBox new_box;

    int npt = m_RoutingPointVec.size();
    for ( int i = 0; i < npt; i++ )
    {
        new_box.Update( m_RoutingPointVec[i]->GetPt() );
    }

    if ( new_box != m_BBox )
    {
        m_BbXLen = new_box.GetMax( 0 ) - new_box.GetMin( 0 );
        m_BbYLen = new_box.GetMax( 1 ) - new_box.GetMin( 1 );
        m_BbZLen = new_box.GetMax( 2 ) - new_box.GetMin( 2 );

        m_BbXMin = new_box.GetMin( 0 );
        m_BbYMin = new_box.GetMin( 1 );
        m_BbZMin = new_box.GetMin( 2 );

        m_BBox = new_box;
        m_OrigBBox = m_BBox;
    }
}

void RoutingGeom::UpdateDrawObj()
{
    Geom::UpdateDrawObj();

    m_RouteLineDO.m_PntVec.clear();
    m_DynamicRouteDO.m_PntVec.clear();

    m_RouteLineDO.m_GeomChanged = true;
    m_DynamicRouteDO.m_GeomChanged = true;

    m_RouteLineDO.m_GeomID = "Rte_" + m_ID;
    m_DynamicRouteDO.m_GeomID = "DyRte_" + m_ID;

    int npt = m_RoutingPointVec.size();
    m_RouteLineDO.m_PntVec.reserve( npt );
    m_DynamicRouteDO.m_PntVec.reserve( npt );

    for ( int i = 0; i < npt; i++ )
    {
        m_RouteLineDO.m_PntVec.push_back( m_RoutingPointVec[i]->GetPt() );
        m_DynamicRouteDO.m_PntVec.push_back( m_RoutingPointVec[i]->GetPt() );
    }
}


void RoutingGeom::LoadDrawObjs( vector< DrawObj* > & draw_obj_vec )
{
    Geom::LoadDrawObjs( draw_obj_vec );

    m_RouteLineDO.m_Visible = !m_Picking && GetSetFlag( vsp::SET_SHOWN );
    draw_obj_vec.push_back( &m_RouteLineDO );

    m_DynamicRouteDO.m_Visible = m_Picking;
    draw_obj_vec.push_back( &m_DynamicRouteDO );


    m_ActivePointDO.m_PntVec.clear();
    m_ActivePointDO.m_GeomChanged = true;
    if ( m_ActivePointIndex >= 0 && m_ActivePointIndex < m_RoutingPointVec.size() )
    {
        m_ActivePointDO.m_GeomID = "AcRte_" + m_ID;;
        m_ActivePointDO.m_PntVec.push_back( m_RoutingPointVec[m_ActivePointIndex]->GetPt() );
        m_ActivePointDO.m_Visible = !m_Picking && m_Vehicle->IsGeomActive( m_ID );
        draw_obj_vec.push_back( &m_ActivePointDO );
    }
}
