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

    IsTrailingEdgeNode_     = 0;
    IsBoundaryEdgeNode_     = 0;
    IsIntersectionNode_     = 0;
    IsBoundaryCornerNode_   = 0;
    IsWakeTrailingEdgeNode_ = 0;
    IsSymmetryPlaneNode_    = 0;
    
    x_                      = 0.;
    y_                      = 0.;
    z_                      = 0.;
                           
    dxyz_[0]                = 0.;
    dxyz_[1]                = 0.;
    dxyz_[2]                = 0.;
    
    Psi_[0]                 = 0.;
    Psi_[1]                 = 0.;
    Psi_[2]                 = 0.;
                           
    CoarseGridNode_         = 0;
    FineGridNode_           = 0;
                           
    ComponentID_            = 0.;
    SurfaceID_              = 0.;

    pForces_pMesh_[0][0] = 0.;
    pForces_pMesh_[0][1] = 0.;
    pForces_pMesh_[0][2] = 0.;
                        
    pForces_pMesh_[1][0] = 0.;
    pForces_pMesh_[1][1] = 0.;
    pForces_pMesh_[1][2] = 0.;
                       
    pForces_pMesh_[2][0] = 0.;
    pForces_pMesh_[2][1] = 0.;
    pForces_pMesh_[2][2] = 0.;

    pForces_pMesh_[3][0] = 0.;
    pForces_pMesh_[3][1] = 0.;
    pForces_pMesh_[3][2] = 0.;
                        
    pForces_pMesh_[4][0] = 0.;
    pForces_pMesh_[4][1] = 0.;
    pForces_pMesh_[4][2] = 0.;
                       
    pForces_pMesh_[5][0] = 0.;
    pForces_pMesh_[5][1] = 0.;
    pForces_pMesh_[5][2] = 0.;
    
    pForces_pMesh_[6][0] = 0.;
    pForces_pMesh_[6][1] = 0.;
    pForces_pMesh_[6][2] = 0.;
                        
    pForces_pMesh_[7][0] = 0.;
    pForces_pMesh_[7][1] = 0.;
    pForces_pMesh_[7][2] = 0.;
                       
    pForces_pMesh_[8][0] = 0.;
    pForces_pMesh_[8][1] = 0.;
    pForces_pMesh_[8][2] = 0.;    

    DForces_DMesh_[0][0] = 0.;
    DForces_DMesh_[0][1] = 0.;
    DForces_DMesh_[0][2] = 0.;
                        
    DForces_DMesh_[1][0] = 0.;
    DForces_DMesh_[1][1] = 0.;
    DForces_DMesh_[1][2] = 0.;
                       
    DForces_DMesh_[2][0] = 0.;
    DForces_DMesh_[2][1] = 0.;
    DForces_DMesh_[2][2] = 0.;

    DForces_DMesh_[3][0] = 0.;
    DForces_DMesh_[3][1] = 0.;
    DForces_DMesh_[3][2] = 0.;
                        
    DForces_DMesh_[4][0] = 0.;
    DForces_DMesh_[4][1] = 0.;
    DForces_DMesh_[4][2] = 0.;
                       
    DForces_DMesh_[5][0] = 0.;
    DForces_DMesh_[5][1] = 0.;
    DForces_DMesh_[5][2] = 0.;

    DForces_DMesh_[6][0] = 0.;
    DForces_DMesh_[6][1] = 0.;
    DForces_DMesh_[6][2] = 0.;
                        
    DForces_DMesh_[7][0] = 0.;
    DForces_DMesh_[7][1] = 0.;
    DForces_DMesh_[7][2] = 0.;
                       
    DForces_DMesh_[8][0] = 0.;
    DForces_DMesh_[8][1] = 0.;
    DForces_DMesh_[8][2] = 0.;    
    
    pMoments_pMesh_[0][0] = 0.;
    pMoments_pMesh_[0][1] = 0.;
    pMoments_pMesh_[0][2] = 0.;
                  
    pMoments_pMesh_[1][0] = 0.;
    pMoments_pMesh_[1][1] = 0.;
    pMoments_pMesh_[1][2] = 0.;
                 
    pMoments_pMesh_[2][0] = 0.;
    pMoments_pMesh_[2][1] = 0.;
    pMoments_pMesh_[2][2] = 0.;
     
    DMoments_DMesh_[0][0] = 0.;
    DMoments_DMesh_[0][1] = 0.;
    DMoments_DMesh_[0][2] = 0.;
                  
    DMoments_DMesh_[1][0] = 0.;
    DMoments_DMesh_[1][1] = 0.;
    DMoments_DMesh_[1][2] = 0.;
                 
    DMoments_DMesh_[2][0] = 0.;
    DMoments_DMesh_[2][1] = 0.;
    DMoments_DMesh_[2][2] = 0.;

    pAeroForces_pMesh_[0][0] = 0.;
    pAeroForces_pMesh_[0][1] = 0.;
    pAeroForces_pMesh_[0][2] = 0.;
                    
    pAeroForces_pMesh_[1][0] = 0.;
    pAeroForces_pMesh_[1][1] = 0.;
    pAeroForces_pMesh_[1][2] = 0.;
                 
    pAeroForces_pMesh_[2][0] = 0.;
    pAeroForces_pMesh_[2][1] = 0.;
    pAeroForces_pMesh_[2][2] = 0.;
 
    pAeroForces_pMesh_[3][0] = 0.;
    pAeroForces_pMesh_[3][1] = 0.;
    pAeroForces_pMesh_[3][2] = 0.;
                
    pAeroForces_pMesh_[4][0] = 0.;
    pAeroForces_pMesh_[4][1] = 0.;
    pAeroForces_pMesh_[4][2] = 0.;
                   
    pAeroForces_pMesh_[5][0] = 0.;
    pAeroForces_pMesh_[5][1] = 0.;
    pAeroForces_pMesh_[5][2] = 0.;
     
    pAeroForces_pMesh_[6][0] = 0.;
    pAeroForces_pMesh_[6][1] = 0.;
    pAeroForces_pMesh_[6][2] = 0.;
                     
    pAeroForces_pMesh_[7][0] = 0.;
    pAeroForces_pMesh_[7][1] = 0.;
    pAeroForces_pMesh_[7][2] = 0.;
                   
    pAeroForces_pMesh_[8][0] = 0.;
    pAeroForces_pMesh_[8][1] = 0.;
    pAeroForces_pMesh_[8][2] = 0.;    
   
    DAeroForces_DMesh_[0][0] = 0.;
    DAeroForces_DMesh_[0][1] = 0.;
    DAeroForces_DMesh_[0][2] = 0.;
                      
    DAeroForces_DMesh_[1][0] = 0.;
    DAeroForces_DMesh_[1][1] = 0.;
    DAeroForces_DMesh_[1][2] = 0.;
                  
    DAeroForces_DMesh_[2][0] = 0.;
    DAeroForces_DMesh_[2][1] = 0.;
    DAeroForces_DMesh_[2][2] = 0.;
    
    DAeroForces_DMesh_[3][0] = 0.;
    DAeroForces_DMesh_[3][1] = 0.;
    DAeroForces_DMesh_[3][2] = 0.;
              
    DAeroForces_DMesh_[4][0] = 0.;
    DAeroForces_DMesh_[4][1] = 0.;
    DAeroForces_DMesh_[4][2] = 0.;
               
    DAeroForces_DMesh_[5][0] = 0.;
    DAeroForces_DMesh_[5][1] = 0.;
    DAeroForces_DMesh_[5][2] = 0.;
    
    DAeroForces_DMesh_[6][0] = 0.;
    DAeroForces_DMesh_[6][1] = 0.;
    DAeroForces_DMesh_[6][2] = 0.;
             
    DAeroForces_DMesh_[7][0] = 0.;
    DAeroForces_DMesh_[7][1] = 0.;
    DAeroForces_DMesh_[7][2] = 0.;
          
    DAeroForces_DMesh_[8][0] = 0.;
    DAeroForces_DMesh_[8][1] = 0.;
    DAeroForces_DMesh_[8][2] = 0.;  
    
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

    IsTrailingEdgeNode_     = VSPNode.IsTrailingEdgeNode_;
    IsBoundaryEdgeNode_     = VSPNode.IsBoundaryEdgeNode_;  
    IsIntersectionNode_     = VSPNode.IsIntersectionNode_;
    IsBoundaryCornerNode_   = VSPNode.IsBoundaryCornerNode_;
    IsWakeTrailingEdgeNode_ = VSPNode.IsWakeTrailingEdgeNode_;
    IsSymmetryPlaneNode_    = VSPNode.IsSymmetryPlaneNode_;

    ComponentID_            = VSPNode.ComponentID_;
    SurfaceID_              = VSPNode.SurfaceID_;
                            
    x_                      = VSPNode.x_;
    y_                      = VSPNode.y_;
    z_                      = VSPNode.z_;
                            
    dxyz_[0]                = VSPNode.dxyz_[0];
    dxyz_[1]                = VSPNode.dxyz_[1];
    dxyz_[2]                = VSPNode.dxyz_[2];
                   
    Psi_[0]                 = VSPNode.Psi_[0];
    Psi_[1]                 = VSPNode.Psi_[1];
    Psi_[2]                 = VSPNode.Psi_[2];
                            
    CoarseGridNode_         = VSPNode.CoarseGridNode_;
    FineGridNode_           = VSPNode.FineGridNode_;  

    pForces_pMesh_[0][0]    = VSPNode.pForces_pMesh_[0][0];
    pForces_pMesh_[0][1]    = VSPNode.pForces_pMesh_[0][1];
    pForces_pMesh_[0][2]    = VSPNode.pForces_pMesh_[0][2];
                                                         
    pForces_pMesh_[1][0]    = VSPNode.pForces_pMesh_[1][0];
    pForces_pMesh_[1][1]    = VSPNode.pForces_pMesh_[1][1];
    pForces_pMesh_[1][2]    = VSPNode.pForces_pMesh_[1][2];
                                                         
    pForces_pMesh_[2][0]    = VSPNode.pForces_pMesh_[2][0];
    pForces_pMesh_[2][1]    = VSPNode.pForces_pMesh_[2][1];
    pForces_pMesh_[2][2]    = VSPNode.pForces_pMesh_[2][2];
                          
    DForces_DMesh_[0][0]    = VSPNode.DForces_DMesh_[0][0];
    DForces_DMesh_[0][1]    = VSPNode.DForces_DMesh_[0][1];
    DForces_DMesh_[0][2]    = VSPNode.DForces_DMesh_[0][2];
                                                         
    DForces_DMesh_[1][0]    = VSPNode.DForces_DMesh_[1][0];
    DForces_DMesh_[1][1]    = VSPNode.DForces_DMesh_[1][1];
    DForces_DMesh_[1][2]    = VSPNode.DForces_DMesh_[1][2];
                                                         
    DForces_DMesh_[2][0]    = VSPNode.DForces_DMesh_[2][0];
    DForces_DMesh_[2][1]    = VSPNode.DForces_DMesh_[2][1];
    DForces_DMesh_[2][2]    = VSPNode.DForces_DMesh_[2][2];

    pMoments_pMesh_[0][0]   = VSPNode.pMoments_pMesh_[0][0];
    pMoments_pMesh_[0][1]   = VSPNode.pMoments_pMesh_[0][1];
    pMoments_pMesh_[0][2]   = VSPNode.pMoments_pMesh_[0][2];
                                             
    pMoments_pMesh_[1][0]   = VSPNode.pMoments_pMesh_[1][0];
    pMoments_pMesh_[1][1]   = VSPNode.pMoments_pMesh_[1][1];
    pMoments_pMesh_[1][2]   = VSPNode.pMoments_pMesh_[1][2];
                                             
    pMoments_pMesh_[2][0]   = VSPNode.pMoments_pMesh_[2][0];
    pMoments_pMesh_[2][1]   = VSPNode.pMoments_pMesh_[2][1];
    pMoments_pMesh_[2][2]   = VSPNode.pMoments_pMesh_[2][2];
                                       
    DMoments_DMesh_[0][0]   = VSPNode.DMoments_DMesh_[0][0];
    DMoments_DMesh_[0][1]   = VSPNode.DMoments_DMesh_[0][1];
    DMoments_DMesh_[0][2]   = VSPNode.DMoments_DMesh_[0][2];
                                             
    DMoments_DMesh_[1][0]   = VSPNode.DMoments_DMesh_[1][0];
    DMoments_DMesh_[1][1]   = VSPNode.DMoments_DMesh_[1][1];
    DMoments_DMesh_[1][2]   = VSPNode.DMoments_DMesh_[1][2];
                                             
    DMoments_DMesh_[2][0]   = VSPNode.DMoments_DMesh_[2][0];
    DMoments_DMesh_[2][1]   = VSPNode.DMoments_DMesh_[2][1];
    DMoments_DMesh_[2][2]   = VSPNode.DMoments_DMesh_[2][2];
            
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

/*##############################################################################
#                                                                              #
#                       VSP_NODE UpdateGeometryLocation                        #
#                                                                              #
##############################################################################*/

void VSP_NODE::UpdateGeometryLocation(double *TVec, double *OVec, QUAT &Quat, QUAT &InvQuat)
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


