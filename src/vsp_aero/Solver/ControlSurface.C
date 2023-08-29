//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//////////////////////////////////////////////////////////////////////

#include "ControlSurface.H"

#include "START_NAME_SPACE.H"

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

    // Node list
    
    NumberOfNodes_ = 0;
    
    XYZ_NodeList_ = NULL;
    
    // Control surface type
    
    Type_ = 0;

    // Hinge line data
    
    HingeNode_1_[0] = HingeNode_1_[1] = HingeNode_1_[2] = 0.;
    
    HingeNode_2_[0] = HingeNode_2_[1] = HingeNode_2_[2] = 0.;
    
    HingeVec_[0] = HingeVec_[1] = HingeVec_[2] = 0.;
    
    // Deflection angle
    
    DeflectionAngle_ = 0.;

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
    
    if ( XYZ_NodeList_ != NULL ) delete [] XYZ_NodeList_;
       
    NumberOfNodes_ = NumberOfNodes;

    XYZ_NodeList_ = new VSPAERO_DOUBLE[3*NumberOfNodes + 1];

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

    if ( LoopList_ != NULL ) delete [] LoopList_;     
    
    LoopList_ = NULL;    

    NumberOfLoops_= 0;;
    
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
#                          CONTROL_SURFACE LoadFile                            #
#                                                                              #
##############################################################################*/

void CONTROL_SURFACE::LoadFile(char *FileName, char *TagFileName)
{

    int i, Found, Surface;
    double x1,y1,z1, x2,y2,z2;
    VSPAERO_DOUBLE Mag;
    char FileNameWithExtension[2000], DumChar[2000];
    FILE *TagFile, *CSFFile;
    
    // Save file name
    
    sprintf(Name_,"%s",TagFileName);
    
    // Read in tag file data
    
    SPRINTF(FileNameWithExtension,"%s.tag",TagFileName);

    if ( (TagFile = fopen(FileNameWithExtension,"r")) == NULL ) {

       PRINTF("Could not load %s tag file... \n", FileNameWithExtension);fflush(NULL);

       exit(1);

    }   
    
    fscanf(TagFile,"%d",&NumberOfLoops_);    
    
    PRINTF("Control surface has %d loops \n",NumberOfLoops_);

    fgets(DumChar,2000,TagFile);

    SizeLoopList(NumberOfLoops_);

    for ( i = 1 ; i <= NumberOfLoops_ ; i ++ ) {
       
       fscanf(TagFile,"%d",&(LoopList_[i]));    
  
    }
    
    fclose(TagFile);
    
    // Read in .csf file data
        
    SPRINTF(FileNameWithExtension,"%s.csf",FileName);
    
    if ( (CSFFile = fopen(FileNameWithExtension,"r")) == NULL ) {

       PRINTF("Could not load %s tag file... \n", FileNameWithExtension);fflush(NULL);

       exit(1);

    }   
    
    Found = 0;

    while ( !Found && fgets(DumChar,2000,CSFFile) != NULL ) {
       
       if ( strstr(DumChar,TagFileName) != NULL ) Found = 1;
       
    }
    
    if ( !Found ) {
       
       PRINTF("Could not find start of control surface %s data in csf for control surface: %s \n",
       FileNameWithExtension, TagFileName);
       
       fflush(NULL);exit(1);
       
    }
    
    else {
       
       HingeNode_1_[0] = 0.;
       HingeNode_1_[1] = 0.;
       HingeNode_1_[2] = 0.;
                      
       HingeNode_2_[0] = 0.;
       HingeNode_2_[1] = 0.;
       HingeNode_2_[2] = 0.;
                
       // Loop over hinge location for upper and lower surface
                
       for ( Surface = 1 ; Surface <= 2 ; Surface++ ) {
                                 
          Found = 0;
          
          while ( !Found && fgets(DumChar,2000,CSFFile) != NULL ) {
          
             if ( strstr(DumChar,"Hinge XYZ") != NULL ) Found = 1;
             
          }
          
          if ( !Found ) {
             
             PRINTF("Could not find corresponding hinge line data in %s file for control surface: %s \n",
             FileNameWithExtension, TagFileName);
             
             fflush(NULL);exit(1);
          
          }
          
          else {
             
             fscanf(CSFFile,"%lf %lf %lf \n",&x1,&y1,&z1);
             fscanf(CSFFile,"%lf %lf %lf \n",&x2,&y2,&z2);
             
             // Average hinge location across upper and lower surface
             
             HingeNode_1_[0] += 0.5*x1;
             HingeNode_1_[1] += 0.5*y1;
             HingeNode_1_[2] += 0.5*z1;
   
             HingeNode_2_[0] += 0.5*x2;
             HingeNode_2_[1] += 0.5*y2;
             HingeNode_2_[2] += 0.5*z2;
                       
             PRINTF("xyz1: %f %f %f \n",x1,y1,z1);
             PRINTF("xyz2: %f %f %f \n",x2,y2,z2);
          
             PRINTF("HingeNode_1_: %f %f %f \n",HingeNode_1_[0],HingeNode_1_[1],HingeNode_1_[2]);
             PRINTF("HingeNode_2_: %f %f %f \n",HingeNode_2_[0],HingeNode_2_[1],HingeNode_2_[2]);
          
          }
          
       }
       
       HingeVec_[0] = HingeNode_2_[0] - HingeNode_1_[0];
       HingeVec_[1] = HingeNode_2_[1] - HingeNode_1_[1];
       HingeVec_[2] = HingeNode_2_[2] - HingeNode_1_[2];

       Mag = sqrt(vector_dot(HingeVec_,HingeVec_));
       
       HingeVec_[0] /= Mag;
       HingeVec_[1] /= Mag;
       HingeVec_[2] /= Mag;        
                          
       fclose(CSFFile);
       
    }
       

}

#include "END_NAME_SPACE.H"

