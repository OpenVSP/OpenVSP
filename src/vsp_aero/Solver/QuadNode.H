//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//////////////////////////////////////////////////////////////////////

#ifndef QUAD_NODE_H
#define QUAD_NODE_H

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utils.H"
#include "QuadEdge.H"

#include "START_NAME_SPACE.H"

class QUAD_NODE {

private:

    int InsideBody_;
    
    VSPAERO_DOUBLE xyz_[3];
    VSPAERO_DOUBLE velocity_[3];
    VSPAERO_DOUBLE Cp_;

public:

    QUAD_NODE(void);
   ~QUAD_NODE(void);
    QUAD_NODE(const QUAD_NODE &QuadNode);
    
    int &InsideBody(void) { return InsideBody_; };
    
    VSPAERO_DOUBLE &xyz(int i) { return xyz_[i]; };
    VSPAERO_DOUBLE &velocity(int i) { return velocity_[i]; };

    VSPAERO_DOUBLE *xyz(void) { return xyz_; };
    VSPAERO_DOUBLE *velocity(void) { return velocity_; };
    
    VSPAERO_DOUBLE &Cp(void) { return Cp_;  };    
    
};

#include "END_NAME_SPACE.H"

#endif
