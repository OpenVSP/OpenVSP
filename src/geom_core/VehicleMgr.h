//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// VehicleMgr.h: interface for the Vehicle Class and Vehicle Mgr Singleton.
// J.R Gloudemans
//
//////////////////////////////////////////////////////////////////////

#if !defined(VEHICLEMGR__INCLUDED_)
#define VEHICLEMGR__INCLUDED_

class Vehicle;

//==== Vehicle Manager ====//
class VehicleMgrSingleton
{
private:
    VehicleMgrSingleton();
    VehicleMgrSingleton( VehicleMgrSingleton const& copy );          // Not Implemented
    VehicleMgrSingleton& operator=( VehicleMgrSingleton const& copy ); // Not Implemented

    Vehicle* m_Vehicle;

    static bool m_firsttime;

public:
    static VehicleMgrSingleton& getInstance();

    Vehicle* GetVehicle();
};


#define VehicleMgr VehicleMgrSingleton::getInstance()


#endif // !defined(VEHICLEMGR__INCLUDED_)
