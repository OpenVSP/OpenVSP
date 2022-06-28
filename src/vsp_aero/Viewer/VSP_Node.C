//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//////////////////////////////////////////////////////////////////////

#include "VSP_Node.H"

/*##############################################################################
#                                                                              #
#                              VSP_NODE constructor                            #
#                                                                              #
##############################################################################*/

VSP_NODE::VSP_NODE(void)
{

    // Nothing to do.... woohoo!

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

    CoarseGridNode_       = VSPNode.CoarseGridNode_;
    FineGridNode_         = VSPNode.FineGridNode_;  

    return *this;
}

/*##############################################################################
#                                                                              #
#                               VSP_NODE destructor                            #
#                                                                              #
##############################################################################*/

VSP_NODE::~VSP_NODE(void)
{

    // Nothing to do... double woohoo!

}


