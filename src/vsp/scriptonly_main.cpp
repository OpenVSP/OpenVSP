//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

#include "VehicleMgr.h"
#include "common.h"
#include "main.h"

//========================================================//
//========================================================//
//========================= Main =========================//

int main( int argc, char** argv )
{
    //==== Get Vehicle Ptr ====//
    Vehicle* vPtr = VehicleMgr.GetVehicle();

    int ret;
    ScreenDumpArgs dump;
    batchMode( argc, argv, vPtr, ret, dump );
    if ( dump.m_Dump )
    {
        printf( "vspscript has no screens to dump; use vsp\n" );
        ret = ESTATUS_GENERAL_ERROR;
    }
    return ret;
}
