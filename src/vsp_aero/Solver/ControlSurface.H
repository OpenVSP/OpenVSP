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
    
    int Type_;
    
    char TypeName_[2000];

    // Control surface name
    
    char Name_[2000];
    
    // Control surface short name
    
    char ShortName_[2000];

    // XYZ coordinates of control surface box
    
    int NumberOfNodes_;
    
    VSPAERO_DOUBLE *XYZ_NodeList_;
    
    // List of loops on the control surface
    
    int NumberOfLoops_;
    int *LoopList_;
    
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
 
    /** Number of loops contained in this control surface definition **/
    
    int NumberOfLoops(void) { return NumberOfLoops_; };
    
    /** ith vortex loop contained in this control surface definition **/
    
    int &LoopList(int i) { return LoopList_[i]; };

    /** Size the node list that defines the polygon for this control surface **/
    
    void SizeNodeList(int NumberOfNodes);
    
    /** Number of nodes that define the control surface polygon **/
    
    int NumberOfNodes(void) { return NumberOfNodes_; };
    
    /** x location of i'th polygon node **/
    
    VSPAERO_DOUBLE &X(int i) { return XYZ_NodeList_[3*i-2]; };
    
    /** y location of i'th polygon node **/
    
    VSPAERO_DOUBLE &Y(int i) { return XYZ_NodeList_[3*i-1]; };

    /** z location of i'th polygon node **/
    
    VSPAERO_DOUBLE &Z(int i) { return XYZ_NodeList_[3*i  ]; };

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
    
    /** Short name given by openvsp to descriminate between control surfaces on various componets **/
    
    char *ShortName(void) { return ShortName_; };    
    
    /** Type of control surface by name **/
    
    char *TypeName(void) { return TypeName_; };

    /** Type of control surface **/
    
    int &Type(void) { return Type_; };
    
    /** Control surface deflection angle **/
    
    VSPAERO_DOUBLE &DeflectionAngle(void) { return DeflectionAngle_; };

    /** Load in the tag region file data **/
    
    void LoadFile(char *FileName, char *TagFileName);
    
};

#include "END_NAME_SPACE.H"

#endif
