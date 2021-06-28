//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//////////////////////////////////////////////////////////////////////

#include "VSP_Loop.H"

/*##############################################################################
#                                                                              #
#                              VSP_LOOP constructor                            #
#                                                                              #
##############################################################################*/

VSP_LOOP::VSP_LOOP(void)
{

    NumberOfNodes_ = 0;
    
    NumberOfEdges_ = 0;
    
    NumberOfFineGridLoops_ = 0;
    
    NodeList_ = NULL;
    
    UVNodeList_ = NULL;
  
    EdgeList_ = NULL;
    
    EdgeDirection_ = NULL;
    
    EdgeIsUpwind_ = NULL;
    
    EdgeUpwindWeight_ = NULL;
    
    FineGridLoopList_ = NULL;
    
    XYZc_[0] = XYZc_[1] = XYZc_[2] = 0.;

    Normal_[0] = Normal_[1] = Normal_[2] = 0.;

    Velocity_[0] = Velocity_[1] = Velocity_[2] = 0.;
    
    Force_[0] = Force_[1] = Force_[2] = 0.;
    
    Area_ = 0.;
    
    Length_ = 0.;
    
    RefLength_ = 0.;
    
    Gamma_ = 0.;
    
    dCp_ = 0.;
    
    dCp_Unsteady_ = 0.;
    
    NormalForce_ = 0.;
    
    KTFact_ = 1.;
    
    SurfaceType_ = 0;
    
    DegenBodyID_ = 0;
    
    DegenWingID_ = 0;    

    SurfaceID_ = 0;
    
    ComponentID_ = 0;
    
    LocalFreeStreamVelocity_[0] = 0.;
    LocalFreeStreamVelocity_[1] = 0.;
    LocalFreeStreamVelocity_[2] = 0.;
    LocalFreeStreamVelocity_[3] = 0.;
    LocalFreeStreamVelocity_[4] = 0.;
    
    DownWash_Velocity_[0] = 0.;
    DownWash_Velocity_[1] = 0.;
    DownWash_Velocity_[2] = 0.;

}

/*##############################################################################
#                                                                              #
#                                 VSP_LOOP Copy                                #
#                                                                              #
##############################################################################*/

VSP_LOOP::VSP_LOOP(const VSP_LOOP &VSPTri)
{

    PRINTF("Copy not implemented for VSP_NODE! \n");

    exit(1);

}

/*##############################################################################
#                                                                              #
#                                 VSP_NODE Copy                                #
#                                                                              #
##############################################################################*/

VSP_LOOP& VSP_LOOP::operator=(const VSP_LOOP &VSPTri)
{

    int i;
    
    // Copy over node list
 
    NumberOfNodes_ = VSPTri.NumberOfNodes_;
 
    SizeNodeList(NumberOfNodes_);
    
    for ( i = 0 ; i < NumberOfNodes_ ; i++ ) {
     
       NodeList_[i] = VSPTri.NodeList_[i];

    }

    for ( i = 0 ; i < 2*NumberOfNodes_ ; i++ ) {

       UVNodeList_[i] = VSPTri.UVNodeList_[i];
       
    }    
    // Copy over edge list
 
    NumberOfEdges_ = VSPTri.NumberOfEdges_;
 
    SizeEdgeList(NumberOfEdges_);
    
    for ( i = 0 ; i < NumberOfEdges_ ; i++ ) {
     
       EdgeList_[i] = VSPTri.EdgeList_[i];
       
       EdgeDirection_[i] = VSPTri.EdgeDirection_[i];
       
       EdgeIsUpwind_[i] = VSPTri.EdgeIsUpwind_[i];

       EdgeUpwindWeight_[i] = VSPTri.EdgeUpwindWeight_[i];
       
    }

    SurfaceID_              = VSPTri.SurfaceID_;
    ComponentID_            = VSPTri.ComponentID_;
    IsTrailingEdgeTri_      = VSPTri.IsTrailingEdgeTri_;
    IsLeadingEdgeTri_       = VSPTri.IsLeadingEdgeTri_;
    VortexLoop_             = VSPTri.VortexLoop_;
    
    SpanStation_            = VSPTri.SpanStation_;
    
    ThicknessToChord_       = VSPTri.ThicknessToChord_;
    LocationOfMaxThickness_ = VSPTri.LocationOfMaxThickness_;
    RadiusToChord_          = VSPTri.RadiusToChord_;          

    Area_                   = VSPTri.Area_;
    Length_                 = VSPTri.Length_;
    RefLength_              = VSPTri.RefLength_;
    CentroidOffSet_         = VSPTri.CentroidOffSet_;
   
    Normal_[0]              = VSPTri.Normal_[0];
    Normal_[1]              = VSPTri.Normal_[1];
    Normal_[2]              = VSPTri.Normal_[2]; 

    XYZc_[0]                = VSPTri.XYZc_[0];
    XYZc_[1]                = VSPTri.XYZc_[1];
    XYZc_[2]                = VSPTri.XYZc_[2];
    
    Camber_                 = VSPTri.Camber_;
    
    UVc_[0]                 = VSPTri.UVc_[0];
    UVc_[1]                 = VSPTri.UVc_[1];

    Gamma_                  = VSPTri.Gamma_;
    dCp_                    = VSPTri.dCp_;
    
    Velocity_[0]            = VSPTri.Velocity_[0];
    Velocity_[1]            = VSPTri.Velocity_[1];
    Velocity_[2]            = VSPTri.Velocity_[2];

    Force_[0]               = VSPTri.Force_[0];
    Force_[1]               = VSPTri.Force_[1];
    Force_[2]               = VSPTri.Force_[2];
    
    NormalForce_            = VSPTri.NormalForce_;
    
    KTFact_                 = VSPTri.KTFact_;
    
    SurfaceType_            = VSPTri.SurfaceType_;
    DegenBodyID_            = VSPTri.DegenBodyID_;
    DegenWingID_            = VSPTri.DegenWingID_;
    Cart3dID_               = VSPTri.Cart3dID_;
    
    CoarseGridLoop_         = VSPTri.CoarseGridLoop_;
    
    NumberOfFineGridLoops_  = VSPTri.NumberOfFineGridLoops_;
    
    SizeFineGridLoopList(NumberOfFineGridLoops_);

    for ( i = 0 ; i < NumberOfFineGridLoops_ ; i++ ) {
     
       FineGridLoopList_[i] = VSPTri.FineGridLoopList_[i];
       
    }
    
    BoundBox_                    = VSPTri.BoundBox_;
    LoopUsed_                    = VSPTri.LoopUsed_;
          
    return *this;          

}

/*##############################################################################
#                                                                              #
#                               VSP_LOOP destructor                            #
#                                                                              #
##############################################################################*/

VSP_LOOP::~VSP_LOOP(void)
{

    if ( NumberOfEdges_ != 0 ) {
       
       delete [] EdgeList_;
       delete [] EdgeIsUpwind_;
       delete [] EdgeDirection_;
       delete [] EdgeUpwindWeight_;
       
       NumberOfEdges_ = 0;
    
    }
    
    if ( NumberOfNodes_ != 0 ) {
       
       delete [] NodeList_;
       delete [] UVNodeList_;
       
       NumberOfNodes_ = 0;
       
    }

}

/*##############################################################################
#                                                                              #
#                          VSP_GRID SizeNodeList                               #
#                                                                              #
##############################################################################*/

void VSP_LOOP::SizeNodeList(int NumberOfNodes)
{
    int i;
    
    // Delete any old list
    
    if ( NumberOfNodes_ != 0 ) delete [] NodeList_;
    
    // Allocate space for list
    
    NumberOfNodes_ = NumberOfNodes;

    NodeList_ = new int[NumberOfNodes_ + 1];
    
    UVNodeList_ = new VSPAERO_DOUBLE[2*NumberOfNodes_ + 1];
    
    for ( i = 0 ; i <= NumberOfNodes_ ; i++ ) {
       
       NodeList_[i] = 0;
 
    }

    for ( i = 0 ; i <= 2*NumberOfNodes_ ; i++ ) {
      
       UVNodeList_[i] = 0.;
       
    }
    
}

/*##############################################################################
#                                                                              #
#                          VSP_GRID SizeEdgeList                               #
#                                                                              #
##############################################################################*/

void VSP_LOOP::SizeEdgeList(int NumberOfEdges)
{

    // Delete any old list
    
    if ( NumberOfEdges_ != 0 ) {
       
       delete [] EdgeList_;
       delete [] EdgeIsUpwind_;
       delete [] EdgeDirection_;
       delete [] EdgeUpwindWeight_;
    
    }
    
    // Allocate space for list
    
    NumberOfEdges_ = NumberOfEdges;

    EdgeList_ = new int[NumberOfEdges_ + 1];    
    EdgeIsUpwind_ = new int[NumberOfEdges_ + 1];
    EdgeDirection_ = new int[NumberOfEdges_ + 1];
    
    EdgeUpwindWeight_ = new VSPAERO_DOUBLE[NumberOfEdges_ + 1];
    
    zero_int_array(EdgeList_, NumberOfEdges_);
    zero_int_array(EdgeIsUpwind_, NumberOfEdges_);
    zero_int_array(EdgeDirection_, NumberOfEdges_);
    
    zero_double_array(EdgeUpwindWeight_, NumberOfEdges_);

}

/*##############################################################################
#                                                                              #
#                          VSP_GRID SizeFineGridLoopList                       #
#                                                                              #
##############################################################################*/

void VSP_LOOP::SizeFineGridLoopList(int NumberOfLoops)
{
   
    if ( NumberOfFineGridLoops_ != 0 ) delete [] FineGridLoopList_;
    
    NumberOfFineGridLoops_ = NumberOfLoops;

    FineGridLoopList_ = new int[NumberOfFineGridLoops_ + 1];

}

/*##############################################################################
#                                                                              #
#                      VSP_LOOP UpdateGeometryLocation                         #
#                                                                              #
##############################################################################*/

void VSP_LOOP::UpdateGeometryLocation(VSPAERO_DOUBLE *TVec, VSPAERO_DOUBLE *OVec, QUAT &Quat, QUAT &InvQuat)
{

    QUAT Vec;

    // Update centroid

    Vec(0) = XYZc_[0] - OVec[0];
    Vec(1) = XYZc_[1] - OVec[1];
    Vec(2) = XYZc_[2] - OVec[2];

    Vec = Quat * Vec * InvQuat;

    XYZc_[0] = Vec(0) + OVec[0] + TVec[0];
    XYZc_[1] = Vec(1) + OVec[1] + TVec[1];
    XYZc_[2] = Vec(2) + OVec[2] + TVec[2];    
    
    // Update normal
    
    Vec(0) = Normal_[0];
    Vec(1) = Normal_[1];
    Vec(2) = Normal_[2];

    Vec = Quat * Vec * InvQuat;
    
    Normal_[0] = Vec(0);
    Normal_[1] = Vec(1);
    Normal_[2] = Vec(2);
    
    // Update bounding box min
    
    Vec(0) = BoundBox_.x_min - OVec[0];
    Vec(1) = BoundBox_.y_min - OVec[1];
    Vec(2) = BoundBox_.z_min - OVec[2];

    Vec = Quat * Vec * InvQuat;

    BoundBox_.x_min = Vec(0) + OVec[0] + TVec[0];
    BoundBox_.y_min = Vec(1) + OVec[1] + TVec[1];
    BoundBox_.z_min = Vec(2) + OVec[2] + TVec[2];      
    
    // Update bounding box max
    
    Vec(0) = BoundBox_.x_max - OVec[0];
    Vec(1) = BoundBox_.y_max - OVec[1];
    Vec(2) = BoundBox_.z_max - OVec[2];

    Vec = Quat * Vec * InvQuat;

    BoundBox_.x_max = Vec(0) + OVec[0] + TVec[0];
    BoundBox_.y_max = Vec(1) + OVec[1] + TVec[1];
    BoundBox_.z_max = Vec(2) + OVec[2] + TVec[2];            

}

