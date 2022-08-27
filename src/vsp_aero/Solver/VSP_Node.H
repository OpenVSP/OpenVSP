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

#include "START_NAME_SPACE.H"

// Definition of the VSP_NODE class

class VSP_NODE {

private:

    // Node based data
    
    int IsTrailingEdgeNode_;
    int IsLeadingEdgeNode_;
    int IsBoundaryEdgeNode_;
    int IsBoundaryCornerNode_;
    
    int ComponentID_;
    int SurfaceID_;
    int GeomID_;

    VSPAERO_DOUBLE x_;
    VSPAERO_DOUBLE y_;
    VSPAERO_DOUBLE z_;
    
    VSPAERO_DOUBLE dGamma_;
    
    // Agglomeration data
    
    int CoarseGridNode_;
    int FineGridNode_;

public:

    // Constructor, Destructor, Copy

    VSP_NODE(void);
   ~VSP_NODE(void);
    VSP_NODE(const VSP_NODE &VSPNode);
    VSP_NODE& operator=(const VSP_NODE &VSPNode);

    /** X coordinate of node **/

    VSPAERO_DOUBLE &x(void) { return x_; };
    
    /** Y coordinate of node **/
    
    VSPAERO_DOUBLE &y(void) { return y_; };

    /** Z coordinate of node **/

    VSPAERO_DOUBLE &z(void) { return z_; };
    
    /** Node is on trailing edge **/
    
    int &IsTrailingEdgeNode(void) { return IsTrailingEdgeNode_; };
    
    /** Node is on leading edge **/
    
    int &IsLeadingEdgeNode(void) { return IsLeadingEdgeNode_; };

    /** Node is on boundary edge **/

    int &IsBoundaryEdgeNode(void) { return IsBoundaryEdgeNode_; };

    /** Node is on a corner **/

    int &IsBoundaryCornerNode(void) { return IsBoundaryCornerNode_; };
    
    /** Component ID **/
    
    int &ComponentID(void) { return ComponentID_; };
    
    /** Surface ID **/
    
    int &SurfaceID(void) { return SurfaceID_; };
    
    /** Geom ID **/
    
    int &GeomID(void) { return GeomID_; };
    
    /** Delta gamma **/
    
    VSPAERO_DOUBLE &dGamma(void) { return dGamma_; };
    
    /** Coarse grid node **/
  
    int &CoarseGridNode(void) { return CoarseGridNode_; };
    
    /** Fine grid node **/
    
    int &FineGridNode(void) { return FineGridNode_; };
    
    /** Update geometry given translation vector and quaternion rotation **/
    
    void UpdateGeometryLocation(VSPAERO_DOUBLE *TVec, VSPAERO_DOUBLE *OVec, QUAT &Quat, QUAT &InvQuat);

};

#include "END_NAME_SPACE.H"

#endif
