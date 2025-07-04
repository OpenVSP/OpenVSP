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
    
    double xyz_[3];
    double velocity_[3];
    double Cp_;

public:

    QUAD_NODE(void);
   ~QUAD_NODE(void);
    QUAD_NODE(const QUAD_NODE &QuadNode);
    
    /** Node is inside the body **/
    
    int &InsideBody(void) { return InsideBody_; };
    
    /** xyz coordinates for this quad tree node **/
    
    double &xyz(int i) { return xyz_[i]; };
    
    /** velocity for this quad tree node **/
    
    double &velocity(int i) { return velocity_[i]; };

    /** xyz coordinate vector for this quad tree node **/
    
    double *xyz(void) { return xyz_; };
    
    /** Velocity vector for this quad tree node **/
     
    double *velocity(void) { return velocity_; };
    
    /** Pressure coefficient for this quad tree node **/
    
    double &Cp(void) { return Cp_;  };    
    
};

#include "END_NAME_SPACE.H"

#endif
