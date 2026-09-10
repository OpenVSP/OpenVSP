//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#include "CloneGeom.h"
#include "Vehicle.h"

//==== Constructor ====//
CloneGeom::CloneGeom( Vehicle* vehicle_ptr ) : Geom( vehicle_ptr )
{
    m_Name = "CloneGeom";
    m_Type.m_Name = "Clone";
    m_Type.m_Type = CLONE_GEOM_TYPE;

    // Tessellation comes from the original.
    m_TessU.Deactivate();
    m_TessW.Deactivate();
}

//==== Destructor ====//
CloneGeom::~CloneGeom()
{

}

void CloneGeom::UpdateSurf()
{
    m_MainSurfVec.clear();
}

//==== Compute Rotation Center ====//
void CloneGeom::ComputeCenter()
{
}

// A Clone has no cross sections and no shape of its own to put a mesh source on.
void CloneGeom::AddDefaultSources( double base_len )
{
}


