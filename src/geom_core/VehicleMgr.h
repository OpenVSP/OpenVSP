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
class VehicleMgr
{
private:
    VehicleMgr();
    VehicleMgr( VehicleMgr const& copy );          // Not Implemented
    VehicleMgr& operator=( VehicleMgr const& copy ); // Not Implemented

    Vehicle* m_Vehicle;

public:
    static VehicleMgr& getInstance()
    {
        static VehicleMgr instance;
        return instance;
    }
    Vehicle* GetVehicle();
};

#endif // !defined(VEHICLEMGR__INCLUDED_)
