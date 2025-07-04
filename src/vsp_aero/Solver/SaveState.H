//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//////////////////////////////////////////////////////////////////////

#ifndef SAVE_STATE_H
#define SAVE_STATE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "utils.H"
#include "VSP_Grid.H"
#include "ComponentGroup.H"

#include "START_NAME_SPACE.H"

// Definition of the SAVE_STATE class

class SAVE_STATE {

private:

    int NumberOfNodes_;
    int NumberOfGroups_;
    int NumberOfLoops_;
    int NumberOfKuttaEdges_;
    
    VSP_GRID *Grid_;
    
    COMPONENT_GROUP *Group_;

    double Time_;
    
    double *xyz_[3];
    
    double *Gamma_;

    double *StallFactor_;

    double *VortexStretchingRatio_;
   
    double *VortexCircumference_;
     
public:

    // Constructor, Destructor, Copy

    SAVE_STATE(void);
   ~SAVE_STATE(void);
    SAVE_STATE(const SAVE_STATE &SaveState);
    SAVE_STATE& operator=(const SAVE_STATE &SaveState);
    
    /** Size the lists **/
    
    void SizeLists(int NumberOfNodes, int NumberOfLoops, int NumberOfGroups, int NumberOfKuttaEdges);
    
    /** Access the xyz node data **/
    
    double &x(int i) { return xyz_[0][i]; };
    double &y(int i) { return xyz_[1][i]; };
    double &z(int i) { return xyz_[2][i]; };

    /** Access the group data **/
    
    COMPONENT_GROUP &Group(int i) { return Group_[i]; };
    
    /** Time at save state **/
    
    double &Time(void) { return Time_; };
    
    /** Access vorticity **/
    
    double &Gamma(int i) { return Gamma_[i]; };

    /** Access the stall factor **/
    
    double &StallFactor(int i) { return StallFactor_[i]; };
    
    /** Access the vortex stretching ratio **/
    
    double &VortexStretchingRatio(int i) { return VortexStretchingRatio_[i]; };
    
    /** Access to the vortex loop circumference **/
    
    double &VortexCircumference(int i) { return VortexCircumference_[i]; };
    
};

#include "END_NAME_SPACE.H"

#endif
