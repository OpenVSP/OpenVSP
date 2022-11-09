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
    
    /** Size the loop list... this is the list of of vortex loops contained within
     * the control surface definition **/
    
    void SizeLoopList(int NumberOfLoops);
    
    /** u min value of control surface bounding box in u,v space **/
    
    VSPAERO_DOUBLE &u_min(void) { return u_min_; };

    /** u max value of control surface bounding box in u,v space **/

    VSPAERO_DOUBLE &u_max(void) { return u_max_; };

    /** v min value of control surface bounding box in u,v space **/

    VSPAERO_DOUBLE &v_min(void) { return v_min_; };

    /** v max value of control surface bounding box in u,v space **/

    VSPAERO_DOUBLE &v_max(void) { return v_max_; };
    
    /** Number of loops contained in this control surface definition **/
    
    int NumberOfLoops(void) { return NumberOfLoops_; };
    
    /** ith vortex loop contained in this control surface definition **/
    
    int &LoopList(int i) { return LoopList_[i]; };

    /** Size the node list that defines the polygon for this control surface **/
    
    void SizeNodeList(int NumberOfNodes);
    
    /** Number of nodes that define the control surface polygon **/
    
    int NumberOfNodes(void) { return NumberOfNodes_; };
    
    /** XYZ vector of ith control surface defining node **/
    
    VSPAERO_DOUBLE *XYZ_Node(int i) { return XYZ_NodeList_[i]; };
    
    //** 1st node of control surface polygon, and ith direction 0:x, 1:y, 2:z ... **/
    
    VSPAERO_DOUBLE &XYZ_Node_1(int i) { return XYZ_NodeList_[1][i]; };
    
    //** 2nd node of control surface polygon, and ith direction 0:x, 1:y, 2:z ... **/
    
    VSPAERO_DOUBLE &XYZ_Node_2(int i) { return XYZ_NodeList_[2][i]; };

    //** 3rd node of control surface polygon, and ith direction 0:x, 1:y, 2:z ... **/

    VSPAERO_DOUBLE &XYZ_Node_3(int i) { return XYZ_NodeList_[3][i]; };

    //** 4th node of control surface polygon, and ith direction 0:x, 1:y, 2:z ... **/

    VSPAERO_DOUBLE &XYZ_Node_4(int i) { return XYZ_NodeList_[4][i]; };
    
    /** UV vector of ith control surface defining node **/
    
    VSPAERO_DOUBLE *UV_Node(int i) { return UV_NodeList_[i]; };
    
    //** 1st node of control surface polygon, and ith direction 0:y, 1:v ... **/
    
    VSPAERO_DOUBLE &UV_Node_1(int i) { return UV_NodeList_[1][i]; };
    
    //** 2nd node of control surface polygon, and ith direction 0:y, 1:v ... **/
    
    VSPAERO_DOUBLE &UV_Node_2(int i) { return UV_NodeList_[2][i]; };

    //** 3rd node of control surface polygon, and ith direction 0:y, 1:v ... **/

    VSPAERO_DOUBLE &UV_Node_3(int i) { return UV_NodeList_[3][i]; };

    //** 4th node of control surface polygon, and ith direction 0:y, 1:v ... **/

    VSPAERO_DOUBLE &UV_Node_4(int i) { return UV_NodeList_[4][i]; };    

    /** Hinge node 1 **/
    
    VSPAERO_DOUBLE &HingeNode_1(int i) { return HingeNode_1_[i]; };
    
    /** Hinge node 2 **/
    
    VSPAERO_DOUBLE &HingeNode_2(int i) { return HingeNode_2_[i]; };
    
    /** Hinge vector defined as vector from hinge node 1 to hinge node 2 **/
    
    VSPAERO_DOUBLE &HingeVec(int i) { return HingeVec_[i]; };
    
    /** Rotate the user supplied normal vector about the hinge line by an amount DeflectionAngle_ **/
    
    void RotateNormal(VSPAERO_DOUBLE *Normal);
    
    /** Determine in a tri/loop is inside the control surface definition **/
    
    int TriInside(VSPAERO_DOUBLE *UV);
    
    /** Control surface name **/
    
    char *Name(void) { return Name_; };
    
    /** Short name given by openvsp to discriminate between control surfaces on various components **/
    
    char *ShortName(void) { return ShortName_; };    
    
    /** Type of control surface by name **/
    
    char *TypeName(void) { return TypeName_; };

    // Type of control surface **/
    
    int &Type(void) { return Type_; };
    
    /** Control surface deflection angle **/
    
    VSPAERO_DOUBLE &DeflectionAngle(void) { return DeflectionAngle_; };
    
    /** Control group ID number **/
    
    int &ControlGroup(void) { return ControlGroup_; };
    
};

#include "END_NAME_SPACE.H"

#endif
