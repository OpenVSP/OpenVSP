//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//////////////////////////////////////////////////////////////////////

#include "VSP_Edge.H"

/*##############################################################################
#                                                                              #
#                              VSP_EDGE constructor                            #
#                                                                              #
##############################################################################*/

VSP_EDGE::VSP_EDGE(void)
{

    // Nothing to do...

}

/*##############################################################################
#                                                                              #
#                                 VSP_EDGE Copy                                #
#                                                                              #
##############################################################################*/

VSP_EDGE::VSP_EDGE(const VSP_EDGE &VSPEdge)
{

    // Just use operator= code
    
    *this = VSPEdge;

}

/*##############################################################################
#                                                                              #
#                                 VSP_EDGE Operator =                          #
#                                                                              #
##############################################################################*/

VSP_EDGE& VSP_EDGE::operator=(const VSP_EDGE &VSPEdge)
{
 
    Node1_          = VSPEdge.Node1_;         
    Node2_          = VSPEdge.Node2_;         

    Tri1_           = VSPEdge.Tri1_;          
    Tri2_           = VSPEdge.Tri2_;          
    
    VortexLoop1_    = VSPEdge.VortexLoop1_;   
    VortexLoop2_    = VSPEdge.VortexLoop2_;   
    
    VortexEdge_     = VSPEdge.VortexEdge_;    
    
    EdgeType_       = VSPEdge.EdgeType_;     
    IsTrailingEdge_ = VSPEdge.IsTrailingEdge_;
    IsLeadingEdge_  = VSPEdge.IsLeadingEdge_; 
    IsBoundaryEdge_ = VSPEdge.IsBoundaryEdge_;

    return *this;
}

/*##############################################################################
#                                                                              #
#                               VSP_EDGE destructor                            #
#                                                                              #
##############################################################################*/

VSP_EDGE::~VSP_EDGE(void)
{

   // Nothing to do...
   
}

