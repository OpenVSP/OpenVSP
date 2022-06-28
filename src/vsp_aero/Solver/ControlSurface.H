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

#include "START_NAME_SPACE.H"

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
    
    // Control surface short name
    
    char ShortName_[2000];
    
    // List of affected vortex loops
    
    int NumberOfLoops_;
    int *LoopList_;
    
    // Control surface bounding box
    
    VSPAERO_DOUBLE u_min_, u_max_;
    VSPAERO_DOUBLE v_min_, v_max_;
    
    // XYZ coordinates of control surface box
    
    int NumberOfNodes_;
    VSPAERO_DOUBLE **UV_NodeList_;
    VSPAERO_DOUBLE **XYZ_NodeList_;

    // Hinge line
    
    VSPAERO_DOUBLE HingeNode_1_[3];
    VSPAERO_DOUBLE HingeNode_2_[3];
    VSPAERO_DOUBLE HingeVec_[3];
    
    // Deflection Angle
    
    VSPAERO_DOUBLE DeflectionAngle_;
    
public:

    // Constructor, Destructor, Copy

    CONTROL_SURFACE(void);
   ~CONTROL_SURFACE(void);
    CONTROL_SURFACE(const CONTROL_SURFACE &ControlSurface);
    CONTROL_SURFACE& operator=(const CONTROL_SURFACE &ControlSurface);
    
    // Size the loop list
    
    void SizeLoopList(int NumberOfLoops);
    
    // Control surface bounding box
    
    VSPAERO_DOUBLE &u_min(void) { return u_min_; };
    VSPAERO_DOUBLE &u_max(void) { return u_max_; };
    VSPAERO_DOUBLE &v_min(void) { return v_min_; };
    VSPAERO_DOUBLE &v_max(void) { return v_max_; };
    
    // Access the list of affected vortex loops
    
    int NumberOfLoops(void) { return NumberOfLoops_; };
    
    int &LoopList(int i) { return LoopList_[i]; };

    // XYZ coordinates
    
    void SizeNodeList(int NumberOfNodes);
    
    int NumberOfNodes(void) { return NumberOfNodes_; };
    
    VSPAERO_DOUBLE *XYZ_Node(int i) { return XYZ_NodeList_[i]; };
    
    VSPAERO_DOUBLE &XYZ_Node_1(int i) { return XYZ_NodeList_[1][i]; };
    VSPAERO_DOUBLE &XYZ_Node_2(int i) { return XYZ_NodeList_[2][i]; };
    VSPAERO_DOUBLE &XYZ_Node_3(int i) { return XYZ_NodeList_[3][i]; };
    VSPAERO_DOUBLE &XYZ_Node_4(int i) { return XYZ_NodeList_[4][i]; };
    
    VSPAERO_DOUBLE *UV_Node(int i) { return UV_NodeList_[i]; };
    
    VSPAERO_DOUBLE &UV_Node_1(int i) { return UV_NodeList_[1][i]; };
    VSPAERO_DOUBLE &UV_Node_2(int i) { return UV_NodeList_[2][i]; };
    VSPAERO_DOUBLE &UV_Node_3(int i) { return UV_NodeList_[3][i]; };
    VSPAERO_DOUBLE &UV_Node_4(int i) { return UV_NodeList_[4][i]; };    

    // Hinge line  
    
    VSPAERO_DOUBLE &HingeNode_1(int i) { return HingeNode_1_[i]; };
    VSPAERO_DOUBLE &HingeNode_2(int i) { return HingeNode_2_[i]; };
    VSPAERO_DOUBLE &HingeVec(int i) { return HingeVec_[i]; };
    
    // Rotate a normal vector
    
    void RotateNormal(VSPAERO_DOUBLE *Normal);
    
    // Check to see if a tri is inside this control surface
    
    int TriInside(VSPAERO_DOUBLE *UV);
    
    // Name
    
    char *Name(void) { return Name_; };
    
    // Short name
    
    char *ShortName(void) { return ShortName_; };    
    
    // TypeName
    
    char *TypeName(void) { return TypeName_; };

    // Type
    
    int &Type(void) { return Type_; };
    
    // Deflection angle
    
    VSPAERO_DOUBLE &DeflectionAngle(void) { return DeflectionAngle_; };
    
    // Control group
    
    int &ControlGroup(void) { return ControlGroup_; };
    
};

#include "END_NAME_SPACE.H"

#endif
