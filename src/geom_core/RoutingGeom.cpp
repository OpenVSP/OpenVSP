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

using namespace vsp;

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

void RoutingGeom::UpdateSurf()
{
    //===== Find Parent ====//
    Geom* parent_geom = m_Vehicle->FindGeom( m_ParentID );
    if ( !parent_geom )
    {
        return;
    }

    //==== Copy XForm/Tess Data From Parent ====//
    CopyDataFrom( parent_geom );

}


void RoutingGeom::CopyDataFrom( Geom* geom_ptr )
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

    //==== Copy Cap Options ====//
    m_CapUMinOption = geom_ptr->m_CapUMinOption();
    m_CapUMinTess   = geom_ptr->m_CapUMinTess();
    m_CapUMaxOption = geom_ptr->m_CapUMaxOption();

    m_CapUMinLength = geom_ptr->m_CapUMinLength();
    m_CapUMinOffset = geom_ptr->m_CapUMinOffset();
    m_CapUMinStrength = geom_ptr->m_CapUMinStrength();
    m_CapUMinSweepFlag = geom_ptr->m_CapUMinSweepFlag();

    m_CapUMaxLength = geom_ptr->m_CapUMaxLength();
    m_CapUMaxOffset = geom_ptr->m_CapUMaxOffset();
    m_CapUMaxStrength = geom_ptr->m_CapUMaxStrength();
    m_CapUMaxSweepFlag = geom_ptr->m_CapUMaxSweepFlag();

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

void RoutingGeom::UpdateDrawObj()
{
    Geom::UpdateDrawObj();
}


void RoutingGeom::LoadDrawObjs( vector< DrawObj* > & draw_obj_vec )
{
    Geom::LoadDrawObjs( draw_obj_vec );
}
