//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

#include "VehicleMgr.h"
#include "common.h"

//========================================================//
//========================================================//
//========================= Main =========================//

int main( int argc, char** argv )
{
    //==== Get Vehicle Ptr ====//
    Vehicle* vPtr = VehicleMgr.GetVehicle();

    int ret;
    batchMode( argc, argv, vPtr, ret );
    return ret;
}
