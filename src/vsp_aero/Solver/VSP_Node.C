//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//////////////////////////////////////////////////////////////////////

#include "VSP_Node.H"

#include "START_NAME_SPACE.H"

/*##############################################################################
#                                                                              #
#                              VSP_NODE constructor                            #
#                                                                              #
##############################################################################*/

VSP_NODE::VSP_NODE(void)
{

    IsTrailingEdgeNode_   = 0;
    IsLeadingEdgeNode_    = 0;
    IsBoundaryEdgeNode_   = 0;
    IsBoundaryCornerNode_ = 0;

    x_                    = 0.;
    y_                    = 0.;
    z_                    = 0.;

    dGamma_               = 0.;
    Area_                 = 0.;
    
    CoarseGridNode_       = 0;
    FineGridNode_         = 0;
    
    ComponentID_          = 0.;
    SurfaceID_            = 0.;
    GeomID_               = 0.;
    
}

/*##############################################################################
#                                                                              #
#                                 VSP_NODE Copy                                #
#                                                                              #
##############################################################################*/

VSP_NODE::VSP_NODE(const VSP_NODE &VSPNode)
{

    // Just use operator = code
    
    *this = VSPNode;

}

/*##############################################################################
#                                                                              #
#                                 VSP_NODE Copy                                #
#                                                                              #
##############################################################################*/

VSP_NODE& VSP_NODE::operator=(const VSP_NODE &VSPNode)
{

    IsTrailingEdgeNode_   = VSPNode.IsTrailingEdgeNode_;
    IsLeadingEdgeNode_    = VSPNode.IsLeadingEdgeNode_;  
    IsBoundaryEdgeNode_   = VSPNode.IsBoundaryEdgeNode_;  
    IsBoundaryCornerNode_ = VSPNode.IsBoundaryCornerNode_;

    x_                    = VSPNode.x_;
    y_                    = VSPNode.y_;
    z_                    = VSPNode.z_;

    dGamma_               = VSPNode.dGamma_;
    Area_                 = VSPNode.Area_;
    
    CoarseGridNode_       = VSPNode.CoarseGridNode_;
    FineGridNode_         = VSPNode.FineGridNode_;  
    
    ComponentID_          = VSPNode.ComponentID_;
    SurfaceID_            = VSPNode.SurfaceID_;
    GeomID_               = VSPNode.GeomID_;

    return *this;

}

/*##############################################################################
#                                                                              #
#                               VSP_NODE destructor                            #
#                                                                              #
##############################################################################*/

VSP_NODE::~VSP_NODE(void)
{

    // Nothing to do... VSPAERO_DOUBLE woohoo!

}

/*##############################################################################
#                                                                              #
#                       VSP_NODE UpdateGeometryLocation                        #
#                                                                              #
##############################################################################*/

void VSP_NODE::UpdateGeometryLocation(VSPAERO_DOUBLE *TVec, VSPAERO_DOUBLE *OVec, QUAT &Quat, QUAT &InvQuat)
{

    QUAT Vec;
 
    // Update location
    
    Vec(0) = x_ - OVec[0];
    Vec(1) = y_ - OVec[1];
    Vec(2) = z_ - OVec[2];

    Vec = Quat * Vec * InvQuat;

    x_ = Vec(0) + OVec[0] + TVec[0];
    y_ = Vec(1) + OVec[1] + TVec[1];
    z_ = Vec(2) + OVec[2] + TVec[2];    

}

#include "END_NAME_SPACE.H"


