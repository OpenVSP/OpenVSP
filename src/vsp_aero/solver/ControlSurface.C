//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//////////////////////////////////////////////////////////////////////

#include "ControlSurface.H"

/*##############################################################################
#                                                                              #
#                              CONTROL_SURFACE constructor                     #
#                                                                              #
##############################################################################*/

CONTROL_SURFACE::CONTROL_SURFACE(void)
{

    // Loop list
    
    NumberOfLoops_ = 0;

    LoopList_ = NULL;
    
    // Control surface type
    
    Type_ = 0;
    
    // Control surface bounding box
    
    u_min_ = u_max_ = 0.;
    v_min_ = v_max_ = 0.;

    // Hinge line
    
    HingeNode_1_[0] = HingeNode_1_[1] = HingeNode_1_[2] = 0.;
    HingeNode_2_[0] = HingeNode_2_[1] = HingeNode_2_[2] = 0.;
    
    HingeVec_[0] = HingeVec_[1] = HingeVec_[2] = 0.;
    
    DeflectionAngle_ = 0.;
    
    ControlGroup_ = 0;

}

/*##############################################################################
#                                                                              #
#                                 CONTROL_SURFACE Copy                         #
#                                                                              #
##############################################################################*/

CONTROL_SURFACE::CONTROL_SURFACE(const CONTROL_SURFACE &ControlSurface)
{

    // Just use operator = code
    
    *this = ControlSurface;

}

/*##############################################################################
#                                                                              #
#                             CONTROL_SURFACE SizeList                         #
#                                                                              #
##############################################################################*/

void CONTROL_SURFACE::SizeList(int NumberOfLoops)
{

    NumberOfLoops_ = NumberOfLoops;

    if ( LoopList_ != NULL ) delete [] LoopList_; 
    
    LoopList_ = new int[NumberOfLoops + 1];
  
}

/*##############################################################################
#                                                                              #
#                          CONTROL_SURFACE operator=                           #
#                                                                              #
##############################################################################*/

CONTROL_SURFACE& CONTROL_SURFACE::operator=(const CONTROL_SURFACE &ControlSurface)
{

    int i;
    
    sprintf(Name_,"%s",ControlSurface.Name_);
    
    NumberOfLoops_ = ControlSurface.NumberOfLoops_;
    
    Type_ = ControlSurface.Type_;

    SizeList(NumberOfLoops_);

    for ( i = 1 ; i <= NumberOfLoops_ ; i ++ ) {
       
       LoopList_[i] = ControlSurface.LoopList_[i];

    }

    u_min_ = ControlSurface.u_min_;
    u_max_ = ControlSurface.u_max_;
    v_min_ = ControlSurface.v_min_;
    v_max_ = ControlSurface.v_max_;
    
    // XYZ coordinates of control surface box
    
    Node_1_[0] = ControlSurface.Node_1_[0];
    Node_1_[1] = ControlSurface.Node_1_[1];
    Node_1_[2] = ControlSurface.Node_1_[2];
    
    Node_2_[0] = ControlSurface.Node_2_[0];
    Node_2_[1] = ControlSurface.Node_2_[1];
    Node_2_[2] = ControlSurface.Node_2_[2];
        
    Node_3_[0] = ControlSurface.Node_3_[0];
    Node_3_[1] = ControlSurface.Node_3_[1];
    Node_3_[2] = ControlSurface.Node_3_[2];
    
    Node_4_[0] = ControlSurface.Node_4_[0];
    Node_4_[1] = ControlSurface.Node_4_[1];
    Node_4_[2] = ControlSurface.Node_4_[2];
            
    // Hinge line
    
    HingeNode_1_[0] = ControlSurface.HingeNode_1_[0];
    HingeNode_1_[1] = ControlSurface.HingeNode_1_[1];
    HingeNode_1_[2] = ControlSurface.HingeNode_1_[2];
                        
    HingeNode_2_[0] = ControlSurface.HingeNode_2_[0];
    HingeNode_2_[1] = ControlSurface.HingeNode_2_[1];
    HingeNode_2_[2] = ControlSurface.HingeNode_2_[2];
    
    HingeVec_[0] = ControlSurface.HingeVec_[0];
    HingeVec_[1] = ControlSurface.HingeVec_[1];
    HingeVec_[2] = ControlSurface.HingeVec_[2];
    
    return *this;

}

/*##############################################################################
#                                                                              #
#                               CONTROL_SURFACE destructor                     #
#                                                                              #
##############################################################################*/

CONTROL_SURFACE::~CONTROL_SURFACE(void)
{

    NumberOfLoops_= 0;;

    if ( LoopList_ != NULL ) delete [] LoopList_;         

}

/*##############################################################################
#                                                                              #
#                        CONTROL_SURFACE RotateNormal                          #
#                                                                              #
##############################################################################*/

void CONTROL_SURFACE::RotateNormal(double *Normal) 
{
   
    QUAT Quat, InvQuat, Vec;
   
    // Build quaternion about this hinge vector

    Quat.FormRotationQuat(HingeVec_,DeflectionAngle_);
   
    InvQuat = Quat;
   
    InvQuat.FormInverse();   
    
    // Rotate the normal
    
    Vec(0) = Normal[0];
    Vec(1) = Normal[1];
    Vec(2) = Normal[2];
    
    Vec = Quat * Vec * InvQuat;
    
    // Update normal
    
    Normal[0] = Vec(0);
    Normal[1] = Vec(1);
    Normal[2] = Vec(2);
       
}

/*##############################################################################
#                                                                              #
#                          CONTROL_SURFACE TriInside                           #
#                                                                              #
##############################################################################*/

int CONTROL_SURFACE::TriInside(double *UV) 
{
  
    if ( UV[0] >= u_min_ && UV[0] <= u_max_ ) {

       if ( UV[1] >= v_min_ && UV[1] <= v_max_ ) return 1;

    }
    
    return 0;
  
}




