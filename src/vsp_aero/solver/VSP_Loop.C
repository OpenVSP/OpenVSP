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

    EdgeList_ = NULL;
    
    EdgeDirection_ = NULL;
    
    EdgeIsUpwind_ = NULL;
    
    EdgeUpwindWeight_ = NULL;
    
    FineGridLoopList_ = NULL;
    
    XYZc_[0] = XYZc_[1] = XYZc_[2] = 0.;

    Normal_[0] = Normal_[1] = Normal_[2] = 0.;
    
    NormalCamber_[0] = NormalCamber_[1] = NormalCamber_[2] = 0.;
    
    Velocity_[0] = Velocity_[1] = Velocity_[2] = 0.;
    
    Force_[0] = Force_[1] = Force_[2] = 0.;
    
    Area_ = 0.;
    
    Gamma_ = 0.;
    
    NormalForce_ = 0.;
    
    SurfaceType_ = 0;
    
    DegenBodyID_ = 0;
    
    DegenWingID_ = 0;    
    
    CompressibilityFactor_ = 0.;

}

/*##############################################################################
#                                                                              #
#                                 VSP_LOOP Copy                                #
#                                                                              #
##############################################################################*/

VSP_LOOP::VSP_LOOP(const VSP_LOOP &VSPTri)
{

    printf("Copy not implemented for VSP_NODE! \n");

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
    
    // Copy over edge list
 
    NumberOfEdges_ = VSPTri.NumberOfEdges_;
 
    SizeEdgeList(NumberOfEdges_);
    
    for ( i = 0 ; i < NumberOfEdges_ ; i++ ) {
     
       EdgeList_[i] = VSPTri.EdgeList_[i];
       
       EdgeDirection_[i] = VSPTri.EdgeDirection_[i];
       
       EdgeIsUpwind_[i] = VSPTri.EdgeIsUpwind_[i];

       EdgeUpwindWeight_[i] = VSPTri.EdgeUpwindWeight_[i];
       
    }

    SurfaceID_             = VSPTri.SurfaceID_;
    IsTrailingEdgeTri_     = VSPTri.IsTrailingEdgeTri_;
    IsLeadingEdgeTri_      = VSPTri.IsLeadingEdgeTri_;
    VortexLoop_            = VSPTri.VortexLoop_;
    
    SpanStation_           = VSPTri.SpanStation_;

    Area_                  = VSPTri.Area_;
    Length_                = VSPTri.Length_;
    CentroidOffSet_        = VSPTri.CentroidOffSet_;
   
    Normal_[0]             = VSPTri.Normal_[0];
    Normal_[1]             = VSPTri.Normal_[1];
    Normal_[2]             = VSPTri.Normal_[2]; 
    
    NormalCamber_[0]       = VSPTri.NormalCamber_[0];
    NormalCamber_[1]       = VSPTri.NormalCamber_[1];
    NormalCamber_[2]       = VSPTri.NormalCamber_[2];

    XYZc_[0]               = VSPTri.XYZc_[0];
    XYZc_[1]               = VSPTri.XYZc_[1];
    XYZc_[2]               = VSPTri.XYZc_[2];
    
    Camber_                = VSPTri.Camber_;
    
    UVc_[0]                = VSPTri.UVc_[0];
    UVc_[1]                = VSPTri.UVc_[1];

    Gamma_                 = VSPTri.Gamma_;
    dCp_                   = VSPTri.dCp_;
    
    Velocity_[0]           = VSPTri.Velocity_[0];
    Velocity_[1]           = VSPTri.Velocity_[1];
    Velocity_[2]           = VSPTri.Velocity_[2];

    Force_[0]              = VSPTri.Force_[0];
    Force_[1]              = VSPTri.Force_[1];
    Force_[2]              = VSPTri.Force_[2];
    
    NormalForce_           = VSPTri.NormalForce_;
    
    SurfaceType_           = VSPTri.SurfaceType_;
    DegenBodyID_           = VSPTri.DegenBodyID_;
    DegenWingID_           = VSPTri.DegenWingID_;
    Cart3dID_              = VSPTri.Cart3dID_;
    
    CoarseGridLoop_        = VSPTri.CoarseGridLoop_;
    
    NumberOfFineGridLoops_ = VSPTri.NumberOfFineGridLoops_;
    
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
    
    for ( i = 0 ; i <= NumberOfNodes_ ; i++ ) {
       
       NodeList_[i] = 0;
       
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
    
    EdgeUpwindWeight_ = new double[NumberOfEdges_ + 1];
    
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

