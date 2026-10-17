//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//


#if !defined(VSPCOMMON__INCLUDED_)
#define VSPCOMMON__INCLUDED_

#include "Vehicle.h"

int vsp_add_and_get_estatus( unsigned int ecode );
void vsp_exit( int ret );
// The screen dump the command line asks for; see GuiInterface::DumpScreens
struct ScreenDumpArgs
{
    bool m_Dump = false;
    int m_Screen = -1;
    int m_GeomScreen = -1;
    string m_Dir = ".";
    double m_Scale = 1.0;
};

int batchMode( int argc, char *argv[], Vehicle* vPtr, int &ret, ScreenDumpArgs &dump );

#endif // VSPCOMMON__INCLUDED_
