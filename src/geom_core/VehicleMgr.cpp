//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// VehicleMgr.cpp: implementation of the Vehicle Class and Vehicle Mgr Singleton.
//
//////////////////////////////////////////////////////////////////////

#include "Vehicle.h"

#ifdef WIN32
#include <windows.h>
#endif

bool VehicleMgrSingleton::m_firsttime = true;

//==== Constructor ====//
VehicleMgrSingleton::VehicleMgrSingleton()
{
    m_Vehicle = new Vehicle();
}

VehicleMgrSingleton& VehicleMgrSingleton::getInstance()
{
    static VehicleMgrSingleton instance;

    if( m_firsttime )
    {
        m_firsttime = false;
        instance.m_Vehicle->Init();
    }

    return instance;
}

Vehicle* VehicleMgrSingleton::GetVehicle()
{
    return m_Vehicle;
}



