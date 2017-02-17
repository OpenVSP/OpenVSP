//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//////////////////////////////////////////////////////////////////////

#ifndef CONTROL_SURFACE_H
#define CONTROL_SURFACE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "utils.H"
#include "quat.H"

#define LOWER_SURFACE           1
#define UPPER_SURFACE           2
#define LOWER_AND_UPPER_SURFACE 3

// Definition of the CONTROL_SURFACE class

class CONTROL_SURFACE {

private:

    // Control surface type
    
    char TypeName_[2000];
    int Type_;
    
    // Control Surface group 
    
    int ControlGroup_;
    
    // Control surface name
    
    char Name_[2000];
    char ShortName_[2000];
    
    // List of affected vortex loops
    
    int NumberOfLoops_;
    int *LoopList_;
    
    // Control surface bounding box
    
    double u_min_, u_max_;
    double v_min_, v_max_;
    
    // XYZ coordinates of control surface box
    
    double Node_1_[3];
    double Node_2_[3];
    double Node_3_[3];
    double Node_4_[3];
    
    // Hinge line
    
    double HingeNode_1_[3];
    double HingeNode_2_[3];
    double HingeVec_[3];
    
    // Deflection Angle
    
    double DeflectionAngle_;
    
public:

    // Constructor, Destructor, Copy

    CONTROL_SURFACE(void);
   ~CONTROL_SURFACE(void);
    CONTROL_SURFACE(const CONTROL_SURFACE &ControlSurface);
    CONTROL_SURFACE& operator=(const CONTROL_SURFACE &ControlSurface);
    
    // Size the list
    
    void SizeList(int NumberOfLoops);
    
    // Control surface bounding box
    
    double &u_min(void) { return u_min_; };
    double &u_max(void) { return u_max_; };
    double &v_min(void) { return v_min_; };
    double &v_max(void) { return v_max_; };
    
    // Access the list of affected vortex loops
    
    int NumberOfLoops(void) { return NumberOfLoops_; };
    
    int &LoopList(int i) { return LoopList_[i]; };

    // XYZ coordinates
    
    double &Node_1(int i) { return Node_1_[i]; };
    double &Node_2(int i) { return Node_2_[i]; };
    double &Node_3(int i) { return Node_3_[i]; };
    double &Node_4(int i) { return Node_4_[i]; };

    // Hinge line  
    
    double &HingeNode_1(int i) { return HingeNode_1_[i]; };
    double &HingeNode_2(int i) { return HingeNode_2_[i]; };
    double &HingeVec(int i) { return HingeVec_[i]; };
    
    // Rotate a normal vector
    
    void RotateNormal(double *Normal);
    
    // Check to see if a tri is inside this control surface
    
    int TriInside(double *UV);
    
    // Name
    
    char *Name(void) { return Name_; };

    char *ShortName( void ) { return ShortName_; };
    
    // TypeName
    
    char *TypeName(void) { return TypeName_; };

    // Type
    
    int &Type(void) { return Type_; };
    
    // Deflection angle
    
    double &DeflectionAngle(void) { return DeflectionAngle_; };
    
    // Control group
    
    int &ControlGroup(void) { return ControlGroup_; };
    
};

#endif
