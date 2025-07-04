//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//////////////////////////////////////////////////////////////////////

#ifndef WAKE_EDGE_DATA_H
#define WAKE_EDGE_DATA_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "utils.H"
#include "time.H"

#include "START_NAME_SPACE.H"

// Small wake edge class

class WAKE_EDGE {

private:

    // 1st of two (possible) trailing edge loops this wake look extends from
    
    int GlobalTrailingEdgeLoopL_;
    
    // 2nd of two (possible) trailing edge loops this wake look extends from
    
    int GlobalTrailingEdgeLoopR_;
    
    // Trailing edge edge this wake look extends from
     
    int GlobalTrailingEdgeEdge_;

    // Wake residual equation number for the 'x' equation
    
    int WakeResidualEquationNumberForX_;

    // Direction of this edge wrt wake
    
    int Direction_;
    
    // Length of this wake edge... this is fixed
        
    double dS_;

    // Rotor flag
    
    int IsRotorWake_;
    
    // Rotor thrust direction
    
    double RotorThrustVector_[3];

public:

    // Constructor, Destructor, Copy

    WAKE_EDGE(void);
   ~WAKE_EDGE(void);
    WAKE_EDGE(const WAKE_EDGE &WakeEdge);
    WAKE_EDGE& operator=(const WAKE_EDGE &WakeEdge);

    /** 1st of two (possible) trailing edge loops this wake look extends from **/

    int &GlobalTrailingEdgeLoopL(void) { return GlobalTrailingEdgeLoopL_; };

    /** 2nd of two (possible) trailing edge loops this wake look extends from **/
 
    int &GlobalTrailingEdgeLoopR(void) { return GlobalTrailingEdgeLoopR_; };

    /** Trailing edge edge this wake look extends from **/

    int &GlobalTrailingEdgeEdge(void) { return GlobalTrailingEdgeEdge_; };

    /** Wake residual equation number for the 'x' equation **/

    int &WakeResidualEquationNumberForX(void) { return WakeResidualEquationNumberForX_    ; };

    /** Wake residual equation number for the 'y' equation **/

    int  WakeResidualEquationNumberForY(void) { return WakeResidualEquationNumberForX_ + 1; };

    /** Wake residual equation number for the 'z' equation **/

    int  WakeResidualEquationNumberForZ(void) { return WakeResidualEquationNumberForX_ + 2; };

    /** Direction of this edge wrt wake **/
    
    int &Direction(void) { return Direction_; };

    /** Length of this wake edge... this is fixed **/
   
    double &dS(void) { return dS_; };

    /** Rotor flag **/
    
    int &IsRotorWake(void) { return IsRotorWake_;  };
    
    /** Rotor thrust direction **/
    
    double &RotorThrustVector(int i) { return RotorThrustVector_[i]; };
    
    double *RotorThrustVector(void) { return RotorThrustVector_; };

};

#include "END_NAME_SPACE.H"

#endif
