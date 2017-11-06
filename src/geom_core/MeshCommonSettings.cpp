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
    ParmContainer::EncodeXml( node );

    return node;
}

xmlNodePtr MeshCommonSettings::DecodeXml( xmlNodePtr & node )
{
    if ( node )
    {
        ParmContainer::DecodeXml( node );
    }

    return node;
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