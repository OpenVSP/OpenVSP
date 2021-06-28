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
    
    // Node list
    
    NumberOfNodes_ = 0;
    
    XYZ_NodeList_ = NULL;
    
     UV_NodeList_ = NULL;

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
#                          CONTROL_SURFACE SizeNodeList                        #
#                                                                              #
##############################################################################*/

void CONTROL_SURFACE::SizeNodeList(int NumberOfNodes)
{

    int i;
    
    if ( NumberOfNodes_ != 0 ) {
       
       for ( i = 1 ; i <= NumberOfNodes_ ; i++ ) {
          
          delete [] XYZ_NodeList_[i];
          delete []  UV_NodeList_[i];
          
       }
       
       delete [] XYZ_NodeList_;
       delete []  UV_NodeList_;
       
    } 
           
    NumberOfNodes_ = NumberOfNodes;

    XYZ_NodeList_ = new VSPAERO_DOUBLE*[NumberOfNodes + 1];
     UV_NodeList_ = new VSPAERO_DOUBLE*[NumberOfNodes + 1];
    
    for ( i = 1 ; i <= NumberOfNodes_ ; i++ ) {
       
       XYZ_NodeList_[i] = new VSPAERO_DOUBLE[3];
        UV_NodeList_[i] = new VSPAERO_DOUBLE[3];
       
    }
  
}

/*##############################################################################
#                                                                              #
#                        CONTROL_SURFACE SizeLoopList                          #
#                                                                              #
##############################################################################*/

void CONTROL_SURFACE::SizeLoopList(int NumberOfLoops)
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
    
    SPRINTF(Name_,"%s",ControlSurface.Name_);
    
    NumberOfLoops_ = ControlSurface.NumberOfLoops_;
    
    SPRINTF(TypeName_,"%s",ControlSurface.TypeName_);
    
    Type_ = ControlSurface.Type_;

    SizeLoopList(NumberOfLoops_);

    for ( i = 1 ; i <= NumberOfLoops_ ; i ++ ) {
       
       LoopList_[i] = ControlSurface.LoopList_[i];

    }

    u_min_ = ControlSurface.u_min_;
    u_max_ = ControlSurface.u_max_;
    v_min_ = ControlSurface.v_min_;
    v_max_ = ControlSurface.v_max_;
    
    // XYZ coordinates of control surface box
    
    for ( i = 1 ; i <= NumberOfNodes_ ; i++ ) {
    
       XYZ_NodeList_[i][0] = ControlSurface. XYZ_NodeList_[i][0];
       XYZ_NodeList_[i][1] = ControlSurface. XYZ_NodeList_[i][1];
       XYZ_NodeList_[i][1] = ControlSurface. XYZ_NodeList_[i][2];

        UV_NodeList_[i][0] =   ControlSurface.UV_NodeList_[i][0];
        UV_NodeList_[i][1] =   ControlSurface.UV_NodeList_[i][1];
        
    }

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

void CONTROL_SURFACE::RotateNormal(VSPAERO_DOUBLE *Normal) 
{
   
    VSPAERO_DOUBLE Angle;
    QUAT Quat, InvQuat, Vec;
   
    // Modify angle ... assume lift ~ cos (alpha) * sin (alpha)... so add in
    // the cos part...

    Angle = cos(DeflectionAngle_)*DeflectionAngle_;
    
    // Build quaternion about this hinge vector

    Quat.FormRotationQuat(HingeVec_,Angle);

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

int CONTROL_SURFACE::TriInside(VSPAERO_DOUBLE *UV) 
{
  
    int i, j, Intersect;
    VSPAERO_DOUBLE Uint;
 
    // Check bounding box
    
    if ( UV[0] >= u_min_ && UV[0] <= u_max_ ) {

       if ( UV[1] >= v_min_ && UV[1] <= v_max_ ) {
          
          // Simple box... we are inside
          
          if ( NumberOfNodes_ == 5 ) {
             
             return 1;
             
          }
          
          // More complex polygon, so must do further checking
          
          else {
          
             Intersect = 0;
             
             for ( i = 1 ; i < NumberOfNodes_ ; i++ ) {
                
                j = i + 1;
                
                if ( UV_NodeList_[i][1] > UV[1] != UV_NodeList_[j][1] > UV[1] ) {
                   
                   Uint = ( UV_NodeList_[j][0] - UV[0] ) * ( UV[1] - UV_NodeList_[i][1] ) / ( UV_NodeList_[j][1] - UV_NodeList_[i][1] ) + UV_NodeList_[i][0];
                   
                   if ( UV[0] < Uint ) Intersect = !Intersect;
                   
                }
                
             }
 
             return Intersect;
             
          }
          
       }

    }
    
    return 0;
  
}

