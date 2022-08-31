//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
// Rob McDonald
//////////////////////////////////////////////////////////////////////

#include "AeroStructMgr.h"
#include "Vehicle.h"
#include "MeshGeom.h"
#include "WingGeom.h"


AeroStructSingleton::AeroStructSingleton() : ParmContainer()
{
    m_Name = "AeroStructSettings";


}

xmlNodePtr AeroStructSingleton::EncodeXml( xmlNodePtr & node )
{
    xmlNodePtr AeroStructnode = xmlNewChild( node, NULL, BAD_CAST"AeroStructMgr", NULL );

    ParmContainer::EncodeXml( AeroStructnode );


    return AeroStructnode;
}

xmlNodePtr AeroStructSingleton::DecodeXml( xmlNodePtr & node )
{
    xmlNodePtr AeroStructnode = XmlUtil::GetNode( node, "AeroStructMgr", 0 );

    if ( AeroStructnode )
    {
        ParmContainer::DecodeXml( AeroStructnode );
    }

    return AeroStructnode;
}

void AeroStructSingleton::Update()
{

}

void AeroStructSingleton::LoadDrawObjs( vector< DrawObj* > &draw_obj_vec )
{
    Vehicle *veh = VehicleMgr.GetVehicle();
    if ( !veh )
    {
        assert( false );
        return;
    }


}
