//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//


#if !defined(VSPCOMMON__INCLUDED_)
#define VSPCOMMON__INCLUDED_

#include "main.h"
#include "VSP_Geom_API.h"
#include "VehicleMgr.h"
#include "APIDefines.h"

int vsp_add_and_get_estatus( int ecode );
void vsp_exit();
int batchMode( int argc, char *argv[], Vehicle* vPtr );

#endif // VSPCOMMON__INCLUDED_
