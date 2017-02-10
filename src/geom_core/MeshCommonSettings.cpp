//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// MeshCommonSettings.cpp
//
//////////////////////////////////////////////////////////////////////

#include "MeshCommonSettings.h"

#include "Vehicle.h"

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

MeshCommonSettings::MeshCommonSettings() : ParmContainer()
{
    m_Name = "NONE";
}

MeshCommonSettings::~MeshCommonSettings()
{
}

void MeshCommonSettings::InitCommonParms()
{
    m_DrawMeshFlag.Init( "DrawMeshFlag", "DrawMesh", this, true, 0, 1 );
    m_DrawBadFlag.Init( "DrawBadElementsFlag", "DrawMesh", this, true, 0, 1 );
    m_ColorTagsFlag.Init( "ColorTagsFlag", "DrawMesh", this, true, 0, 1 );

    m_IntersectSubSurfs.Init( "IntersectSubSurfs", "Global", this, true, 0, 1 );
    m_IntersectSubSurfs.SetDescript( "Flag to intersect subsurfaces" );

    m_FarCompFlag.Init( "FarComp", "FarField", this, false, 0, 1 );
    m_FarMeshFlag.Init( "FarMesh", "FarField", this, false, 0, 1 );
    m_HalfMeshFlag.Init( "HalfMesh", "FarField", this, false, 0, 1 );

    //Symmetry Plane Splitting Default Value: OFF
    m_SymSplittingOnFlag.Init( "SymmetrySplitting", "FarField", this, false, 0, 1 );
}

xmlNodePtr MeshCommonSettings::EncodeXml( xmlNodePtr & node )
{
    xmlNodePtr meshcommonnode = xmlNewChild( node, NULL, BAD_CAST m_Name.c_str(), NULL );

    ParmContainer::EncodeXml( meshcommonnode );

    return meshcommonnode;
}

xmlNodePtr MeshCommonSettings::DecodeXml( xmlNodePtr & node )
{
    xmlNodePtr meshcommonnode = XmlUtil::GetNode( node, m_Name.c_str(), 0 );
    if ( meshcommonnode )
    {
        ParmContainer::DecodeXml( meshcommonnode );
    }

    return meshcommonnode;
}

//==== Parm Changed ====//
void MeshCommonSettings::ParmChanged( Parm* parm_ptr, int type )
{
    Vehicle* veh = VehicleMgr.GetVehicle();

    if ( veh )
    {
        veh->ParmChanged( parm_ptr, type );
    }
}