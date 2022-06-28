//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//////////////////////////////////////////////////////////////////////

#ifndef PROP_ELEMENT_H
#define PROP_ELEMENT_H

#include "RotorDisk.H"
#include "EngineFace.H"

#define PROP_ROTOR  1
#define PROP_INLET  2
#define PROP_NOZZLE 3

#define ENGINE_INLET  4
#define ENGINE_NOZZLE 5

struct PROPULSION_ELEM_STRUCT
{

    char Name[200];
    int Type;
    ROTOR_DISK Rotor;
    ENGINE_FACE Engine;
    
};

typedef struct PROPULSION_ELEM_STRUCT PROPULSION_ELEMENT;


#endif
