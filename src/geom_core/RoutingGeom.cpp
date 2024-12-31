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

using namespace vsp;

//=========================================================================//
//========================        RoutingPoint       ======================//
//=========================================================================//

RoutingPoint::RoutingPoint()
{



    m_U.Init( "U", "RoutePt", this, 0.5, 0.0, 1.0 );
    m_W.Init( "W", "RoutePt", this, 0.5, 0.0, 1.0 );
}

void RoutingPoint::Update()
{
}

void RoutingPoint::SetParentID( const string &id )
{
    m_ParentID = id;
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

//=========================================================================//
//=========================        RoutingGeom       ======================//
//=========================================================================//

//==== Constructor ====//
RoutingGeom::RoutingGeom( Vehicle* vehicle_ptr ) : Geom( vehicle_ptr )
{
    m_Name = "RoutingGeom";
    m_Type.m_Name = "Routing";
    m_Type.m_Type = ROUTING_GEOM_TYPE;

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

    return routingpoints_node;
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
    m_RoutingPointVec.push_back( rpt );
    return rpt;
}

void RoutingGeom::DelPt( int index )
{
    if ( index >= 0 && index < m_RoutingPointVec.size() )
    {
        RoutingPoint *rpt = m_RoutingPointVec[ index ];
        m_RoutingPointVec.erase( m_RoutingPointVec.begin() + index );

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
}

RoutingPoint * RoutingGeom::GetPt( int index )
{
    if ( index >= 0 && index < m_RoutingPointVec.size() )
    {
        return m_RoutingPointVec[ index ];
    }
    return nullptr;
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

void RoutingGeom::UpdateDrawObj()
{
    Geom::UpdateDrawObj();
}


void RoutingGeom::LoadDrawObjs( vector< DrawObj* > & draw_obj_vec )
{
    Geom::LoadDrawObjs( draw_obj_vec );
}
