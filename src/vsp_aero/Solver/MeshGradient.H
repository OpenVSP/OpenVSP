//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//////////////////////////////////////////////////////////////////////

#ifndef MESH_GRADIENT_H
#define MESH_GRADIENT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>

// Definition of the MESH_GRADIENT class

class MESH_GRADIENT {

private:

    // wrt mesh
    
    double px_;
    double py_;
    double pz_;

public:

    // Constructor, Destructor, Copy

    MESH_GRADIENT(void);
   ~MESH_GRADIENT(void);
    MESH_GRADIENT(const MESH_GRADIENT &Gradient);

    /** Gradients wrt mesh **/

    double &px(void) { return px_; };
    double &py(void) { return py_; };
    double &pz(void) { return pz_; };

    /** Zero the gradients **/
    
    void Zero(void);
    
};

#endif
