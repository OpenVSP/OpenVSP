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

// Definition of the VSP_NODE class

class VSP_NODE {

private:

    // Node based data
    
    int IsTrailingEdgeNode_;
    int IsLeadingEdgeNode_;
    int IsBoundaryEdgeNode_;
    int IsBoundaryCornerNode_;

    double x_;
    double y_;
    double z_;
    
    double dGamma_;
    double Area_;
    
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

    double &x(void) { return x_; };
    double &y(void) { return y_; };
    double &z(void) { return z_; };
    
    int &IsTrailingEdgeNode(void) { return IsTrailingEdgeNode_; };
    int &IsLeadingEdgeNode(void) { return IsLeadingEdgeNode_; };
    int &IsBoundaryEdgeNode(void) { return IsBoundaryEdgeNode_; };
    int &IsBoundaryCornerNode(void) { return IsBoundaryCornerNode_; };
    
    double &dGamma(void) { return dGamma_; };
    
    double &Area(void) { return Area_; };
    
    int &CoarseGridNode(void) { return CoarseGridNode_; };
    int &FineGridNode(void) { return FineGridNode_; };

};

#endif
