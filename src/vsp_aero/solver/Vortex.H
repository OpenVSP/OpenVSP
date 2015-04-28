//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//////////////////////////////////////////////////////////////////////

#ifndef VORTEX_H
#define VORTEX_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "utils.H"

// Definition of the VORTEX class

class VORTEX {

private:

    // Function data
    
    void init(void);
    

public:

    // Constructor, Destructor, Copy

    VORTEX(void);
   ~VORTEX(void);
    VORTEX(const VORTEX &Vortex);

    // Copy function

    VORTEX& operator=(const VORTEX &Vortex);
    
    void Velocity(float xyz_1[3], float xyz_2[3], float xyz_p[3], float Mach, float Gamma, float q[3]);


};

#endif
