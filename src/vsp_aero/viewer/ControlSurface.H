//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//////////////////////////////////////////////////////////////////////

#ifndef CONTROL_SURFACE_H
#define CONTROL_SURFACE_H

struct CONTROL_SURFACE_STRUCT
{

    char Name[2000];
    int Type;
    float Node1[3];
    float Node2[3];
    float Node3[3];
    float Node4[3];
        
};

typedef struct CONTROL_SURFACE_STRUCT CONTROL_SURFACE;


#endif
