//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// VehicleMgr.cpp: implementation of the Vehicle Class and Vehicle Mgr Singleton.
//
//////////////////////////////////////////////////////////////////////

#ifdef WIN32
#undef _HAS_STD_BYTE
#define _HAS_STD_BYTE 0
#include <windows.h>
#endif

#include "Vehicle.h"
#include "MainThreadIDMgr.h"

bool VehicleMgrSingleton::m_firsttime = true;

//==== Constructor ====//
VehicleMgrSingleton::VehicleMgrSingleton()
{
    // Set up MainThreadID if this is entry point.
    MainThreadIDMgr.getInstance();

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



