//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//////////////////////////////////////////////////////////////////////

#ifndef VSP_NODE_H
#define VSP_NODE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "utils.H"
#include "quat.H"

// Definition of the VSP_NODE class

class VSP_NODE {

private:

    // Node based data
    
    int IsTrailingEdgeNode_;
    int IsLeadingEdgeNode_;
    int IsBoundaryEdgeNode_;
    int IsBoundaryCornerNode_;
    
    int ComponentID_;

    VSPAERO_DOUBLE x_;
    VSPAERO_DOUBLE y_;
    VSPAERO_DOUBLE z_;
    
    VSPAERO_DOUBLE dGamma_;
    VSPAERO_DOUBLE Area_;
    
    // Agglomeration data
    
    int CoarseGridNode_;
    int FineGridNode_;

public:

    // Constructor, Destructor, Copy

    VSP_NODE(void);
   ~VSP_NODE(void);
    VSP_NODE(const VSP_NODE &VSPNode);
    VSP_NODE& operator=(const VSP_NODE &VSPNode);

    // Accesss to node data

    VSPAERO_DOUBLE &x(void) { return x_; };
    VSPAERO_DOUBLE &y(void) { return y_; };
    VSPAERO_DOUBLE &z(void) { return z_; };
    
    int &IsTrailingEdgeNode(void) { return IsTrailingEdgeNode_; };
    int &IsLeadingEdgeNode(void) { return IsLeadingEdgeNode_; };
    int &IsBoundaryEdgeNode(void) { return IsBoundaryEdgeNode_; };
    int &IsBoundaryCornerNode(void) { return IsBoundaryCornerNode_; };
    
    int &ComponentID(void) { return ComponentID_; };
    
    VSPAERO_DOUBLE &dGamma(void) { return dGamma_; };
    
    VSPAERO_DOUBLE &Area(void) { return Area_; };
    
    int &CoarseGridNode(void) { return CoarseGridNode_; };
    int &FineGridNode(void) { return FineGridNode_; };
    
    void UpdateGeometryLocation(VSPAERO_DOUBLE *TVec, VSPAERO_DOUBLE *OVec, QUAT &Quat, QUAT &InvQuat);

};

#endif
