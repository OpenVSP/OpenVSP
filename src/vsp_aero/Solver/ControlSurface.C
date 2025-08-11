//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//////////////////////////////////////////////////////////////////////

#include "ControlSurface.H"

#include <filesystem>

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

    // Aero Control derivatives
    
    pCLi_pDelta_  = 0.;
    pCDi_pDelta_  = 0.;
    pCSi_pDelta_  = 0.;
                 
    pCMli_pDelta_ = 0.;    
    pCMmi_pDelta_ = 0.;    
    pCMni_pDelta_ = 0.;    
                 
    pCLo_pDelta_  = 0.;
    pCDo_pDelta_  = 0.;
    pCSo_pDelta_  = 0.;
               
    pCMlo_pDelta_ = 0.;    
    pCMmo_pDelta_ = 0.;    
    pCMno_pDelta_ = 0.;    

    // Force coefficient derivatives
    
    pCFix_pDelta_  = 0.;
    pCFiy_pDelta_  = 0.;
    pCFiz_pDelta_  = 0.;
                
    pCMix_pDelta_ = 0.;
    pCMiy_pDelta_ = 0.;
    pCMiz_pDelta_ = 0.;
             
    pCFox_pDelta_  = 0.;
    pCFoy_pDelta_  = 0.;
    pCFoz_pDelta_  = 0.;
                 
    pCMox_pDelta_ = 0.;
    pCMoy_pDelta_ = 0.;
    pCMoz_pDelta_ = 0.;      
    
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

    XYZ_NodeList_ = new double[3*NumberOfNodes + 1];

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
    
    snprintf(Name_,sizeof(Name_)*sizeof(char),"%s",ControlSurface.Name_);
    
    NumberOfLoops_ = ControlSurface.NumberOfLoops_;
    
    snprintf(TypeName_,sizeof(TypeName_)*sizeof(char),"%s",ControlSurface.TypeName_);
    
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
#                        CONTROL_SURFACE RotateXYZPoint                        #
#                                                                              #
##############################################################################*/

void CONTROL_SURFACE::RotateXYZPoint(double *xyz) 
{
   
    double Angle;
    QUAT Quat, InvQuat, Vec;
    
    // Shift xyz
    
    xyz[0] -= HingeNode_1_[0];
    xyz[1] -= HingeNode_1_[1];
    xyz[2] -= HingeNode_1_[2];
   
    // Rotation angle

    Angle = DeflectionAngle_;
  
    // Build quaternion about this hinge vector

    Quat.FormRotationQuat(HingeVec_,Angle);

    InvQuat = Quat;
   
    InvQuat.FormInverse();   
    
    // Rotate the xyz point
    
    Vec(0) = xyz[0];
    Vec(1) = xyz[1];
    Vec(2) = xyz[2];
    
    Vec = Quat * Vec * InvQuat;
    
    // Shift xyz back
    
    xyz[0] = Vec(0) + HingeNode_1_[0];
    xyz[1] = Vec(1) + HingeNode_1_[1];
    xyz[2] = Vec(2) + HingeNode_1_[2];
       
}

/*##############################################################################
#                                                                              #
#                        CONTROL_SURFACE UnRotateXYZPoint                      #
#                                                                              #
##############################################################################*/

void CONTROL_SURFACE::UnRotateXYZPoint(double *xyz) 
{
   
    double Angle;
    QUAT Quat, InvQuat, Vec;
    
    // Shift xyz
    
    xyz[0] -= HingeNode_1_[0];
    xyz[1] -= HingeNode_1_[1];
    xyz[2] -= HingeNode_1_[2];
   
    // Modify angle ... assume lift ~ cos (alpha) * sin (alpha)... so add in
    // the cos part...

    Angle = -DeflectionAngle_;
  
    // Build quaternion about this hinge vector

    Quat.FormRotationQuat(HingeVec_,Angle);

    InvQuat = Quat;
   
    InvQuat.FormInverse();   
    
    // Rotate the xyz point
    
    Vec(0) = xyz[0];
    Vec(1) = xyz[1];
    Vec(2) = xyz[2];
    
    Vec = Quat * Vec * InvQuat;
    
    // Shift xyz back
    
    xyz[0] = Vec(0) + HingeNode_1_[0];
    xyz[1] = Vec(1) + HingeNode_1_[1];
    xyz[2] = Vec(2) + HingeNode_1_[2];
       
}


/*##############################################################################
#                                                                              #
#                        CONTROL_SURFACE RotateNormal                          #
#                                                                              #
##############################################################################*/

void CONTROL_SURFACE::RotateNormal(double *Normal) 
{
   
    double Angle;
    QUAT Quat, InvQuat, Vec;
   
    // Modify angle ... assume lift ~ cos (alpha) * sin (alpha)... so add in
    // the cos part...

    Angle = DeflectionAngle_;
  
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
    double Mag;
    char FileNameWithExtension[MAX_CHAR_SIZE], DumChar[MAX_CHAR_SIZE];
    FILE *TagFile, *CSFFile;

    std::filesystem::path fpath( FileName );
    fpath.remove_filename();


    // Save file name
    
    snprintf(Name_,sizeof(Name_)*sizeof(char),"%s",TagFileName);
    
    // Read in tag file data
    
    snprintf(FileNameWithExtension,sizeof(FileNameWithExtension)*sizeof(char),"%s%s.tag",fpath.c_str(), TagFileName);

    if ( (TagFile = fopen(FileNameWithExtension,"r")) == NULL ) {

       printf("Could not load %s tag file... \n", FileNameWithExtension);fflush(NULL);

       exit(1);

    }   
    
    fscanf(TagFile,"%d",&NumberOfLoops_);    
    
    printf("Control surface has %d loops \n",NumberOfLoops_);

    fgets(DumChar,MAX_CHAR_SIZE,TagFile);

    SizeLoopList(NumberOfLoops_);

    for ( i = 1 ; i <= NumberOfLoops_ ; i ++ ) {
       
       fscanf(TagFile,"%d",&(LoopList_[i]));    
  
    }
    
    fclose(TagFile);
    
    // Read in .csf file data
        
    snprintf(FileNameWithExtension,sizeof(FileNameWithExtension)*sizeof(char),"%s.csf",FileName);
    
    if ( (CSFFile = fopen(FileNameWithExtension,"r")) == NULL ) {

       printf("Could not load %s tag file... \n", FileNameWithExtension);fflush(NULL);

       exit(1);

    }   
    
    Found = 0;

    while ( !Found && fgets(DumChar,MAX_CHAR_SIZE,CSFFile) != NULL ) {
       
       if ( strstr(DumChar,TagFileName) != NULL ) Found = 1;
       
    }
    
    if ( !Found ) {
       
       printf("Could not find start of control surface %s data in csf for control surface: %s \n",
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
          
          while ( !Found && fgets(DumChar,MAX_CHAR_SIZE,CSFFile) != NULL ) {
          
             if ( strstr(DumChar,"Hinge XYZ") != NULL ) Found = 1;
             
          }
          
          if ( !Found ) {
             
             printf("Could not find corresponding hinge line data in %s file for control surface: %s \n",
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
                       
             printf("xyz1: %f %f %f \n",x1,y1,z1);
             printf("xyz2: %f %f %f \n",x2,y2,z2);
          
             printf("HingeNode_1_: %f %f %f \n",HingeNode_1_[0],HingeNode_1_[1],HingeNode_1_[2]);
             printf("HingeNode_2_: %f %f %f \n",HingeNode_2_[0],HingeNode_2_[1],HingeNode_2_[2]);
          
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

