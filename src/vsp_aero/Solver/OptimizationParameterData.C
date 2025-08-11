//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//////////////////////////////////////////////////////////////////////

#include "OptimizationParameterData.H"

/*##############################################################################
#                                                                              #
#                  OPTIMIZATION_PARAMETERS constructor                         #
#                                                                              #
##############################################################################*/

OPTIMIZATION_PARAMETERS::OPTIMIZATION_PARAMETERS(void)
{

   NumberOfOpenVSP_Parameters_ = 0;
   
   OpenVSP_ParameterNames_ = NULL;

   ParameterFullNamePath_ = NULL;

   dMesh_dOpenVSP_Parameter_ = NULL;


   NumberOfVSPAERO_Parameters_ = 0;
   
   VSPAERO_ParameterNames_ = NULL;   


   NumberOfParameters_ = 0;

   
   ParameterValues_ = NULL;   

}

/*##############################################################################
#                                                                              #
#                       OPTIMIZATION_PARAMETERS Copy                           #
#                                                                              #
##############################################################################*/

OPTIMIZATION_PARAMETERS::OPTIMIZATION_PARAMETERS(const OPTIMIZATION_PARAMETERS &Gradient)
{

    printf("Copy not implemented for OPTIMIZATION_PARAMETERS! \n");

    exit(1);

}

/*##############################################################################
#                                                                              #
#                     OPTIMIZATION_PARAMETERS destructor                       #
#                                                                              #
##############################################################################*/

OPTIMIZATION_PARAMETERS::~OPTIMIZATION_PARAMETERS(void)
{

   // Nothing to do...
   
}

/*##############################################################################
#                                                                              #
#                 OPTIMIZATION_PARAMETERS ReadOpenVSPDesFile                   #
#                                                                              #
##############################################################################*/

void OPTIMIZATION_PARAMETERS::ReadOpenVSPDesFile(char *FileName)
{
    
    int i, j, ConGroup;
    double *ParameterValues;
    char DesignFileName[MAX_CHAR_SIZE], Variable[MAX_CHAR_SIZE];
    char DumChar[MAX_CHAR_SIZE];
    FILE *DesignFile;

    // Open the OpenVSP des file

    snprintf(DesignFileName,sizeof(DesignFileName)*sizeof(char),"%s.des",FileName);
    
    printf("Opening: %s \n",DesignFileName);fflush(NULL);

    if ( (DesignFile = fopen(DesignFileName, "r")) == NULL ) {
    
       printf("Could not open the OpenVSP des file! \n");
    
       exit(1);
    
    }
    
    // Parse the des file   
    
    fscanf(DesignFile,"%d \n",&NumberOfOpenVSP_Parameters_);
    
    printf("NumberOfOpenVSP_Parameters_: %d \n",NumberOfOpenVSP_Parameters_);fflush(NULL);

    ParameterValues_ = new double[NumberOfOpenVSP_Parameters_ + 100];
    
    OpenVSP_ParameterNames_ = new char*[NumberOfOpenVSP_Parameters_ + 100];

    ParameterFullNamePath_ = new char*[NumberOfOpenVSP_Parameters_ + 100];

    for ( i = 1 ; i <= NumberOfOpenVSP_Parameters_ ; i++ ) {

       OpenVSP_ParameterNames_[i] = new char[MAX_CHAR_SIZE];

       ParameterFullNamePath_[i] = new char[MAX_CHAR_SIZE];

       fscanf(DesignFile,"%s%lf\n",(Variable),&( ParameterValues_[i]));
              
       snprintf(OpenVSP_ParameterNames_[i],12*sizeof(char),"%s",Variable);
       
       snprintf(ParameterFullNamePath_[i],MAX_CHAR_SIZE*sizeof(char),"%s",Variable);

       printf("%s --> %f \n",OpenVSP_ParameterNames_[i],ParameterValues_[i]);fflush(NULL);

    }
    
    NumberOfParameters_ = NumberOfOpenVSP_Parameters_;
    
    // Now search for VSPAERO parameters
    
    fgets(DumChar,200,DesignFile);
    fgets(DumChar,200,DesignFile);
    
    if ( strstr(DumChar,"VSPAERO") != NULL ) {
       
       fgets(DumChar,200,DesignFile);

       fscanf(DesignFile,"%d \n",&NumberOfVSPAERO_Parameters_);
       
       NumberOfParameters_ += NumberOfVSPAERO_Parameters_;

       VSPAERO_ParameterNames_ = new char*[NumberOfVSPAERO_Parameters_ + 1];

       for ( i = 1 ; i <= NumberOfVSPAERO_Parameters_ ; i++ ) {
                    
          j = i + NumberOfOpenVSP_Parameters_;
                    
          VSPAERO_ParameterNames_[i] = new char[MAX_CHAR_SIZE];
                    
          ParameterFullNamePath_[j] = new char[MAX_CHAR_SIZE];
                    
          fscanf(DesignFile,"%s%lf\n",(Variable),&( ParameterValues_[j]));

          printf("Variable: %s \n",Variable);fflush(NULL);

          snprintf(ParameterFullNamePath_[j],MAX_CHAR_SIZE*sizeof(char),"%s",Variable);

          // Mach number
          
          if ( strstr(Variable,"Mach") != NULL ) {
             
             snprintf(VSPAERO_ParameterNames_[i],MAX_CHAR_SIZE*sizeof(char),"MACH");
          
          }
          
          // Alpha

          if ( strstr(Variable,"AngleOfAttack") != NULL ) {
             
             snprintf(VSPAERO_ParameterNames_[i],MAX_CHAR_SIZE*sizeof(char),"ALPHA");
          
          }

          // Beta

          if ( strstr(Variable,"AngleOfBeta") != NULL ) {
             
             snprintf(VSPAERO_ParameterNames_[i],MAX_CHAR_SIZE*sizeof(char),"BETA");
          
          }          
          
          // Control groups

          if ( strstr(Variable,"ConGroup") != NULL ) {
             
            sscanf(Variable,"%21s%d",DumChar,&ConGroup);
             
            printf("ConGroup: %d \n",ConGroup);           
                
            snprintf(VSPAERO_ParameterNames_[i],MAX_CHAR_SIZE*sizeof(char),"ConGroup:%d",ConGroup);
          
          }
          
          // XCG
          
          if ( strstr(Variable,"XCG") != NULL ) {
             
             snprintf(VSPAERO_ParameterNames_[i],MAX_CHAR_SIZE*sizeof(char),"XCG");


          }            
          
          // YCG
          
          if ( strstr(Variable,"YCG") != NULL ) {
             
             snprintf(VSPAERO_ParameterNames_[i],MAX_CHAR_SIZE*sizeof(char),"YCG");

                       
          }    
          
          // ZCG
          
          if ( strstr(Variable,"ZCG") != NULL ) {
             
             snprintf(VSPAERO_ParameterNames_[i],MAX_CHAR_SIZE*sizeof(char),"ZCG");

                       
          }    
              
       }
       
    }

    fclose(DesignFile);    
 
}

/*##############################################################################
#                                                                              #
#                 OPTIMIZATION_PARAMETERS WriteVSPDesFile                      #
#                                                                              #
##############################################################################*/

void OPTIMIZATION_PARAMETERS::WriteVSPDesFile(char *FileName, int Iter, double *ParameterValues)
{
   
    WriteVSPDesFile_(FileName,Iter,ParameterValues);
       
}

/*##############################################################################
#                                                                              #
#                 OPTIMIZATION_PARAMETERS WriteVSPDesFile                      #
#                                                                              #
##############################################################################*/

void OPTIMIZATION_PARAMETERS::WriteFinalVSPDesFile(char *FileName, double *ParameterValues)
{
   
    int Iter = -1;
    
    WriteVSPDesFile_(FileName,Iter,ParameterValues);
       
}

/*##############################################################################
#                                                                              #
#                 OPTIMIZATION_PARAMETERS WriteVSPDesFile                      #
#                                                                              #
##############################################################################*/

void OPTIMIZATION_PARAMETERS::WriteVSPDesFile_(char *FileName, int Iter, double *ParameterValues)
{
    
    int i, j, NumVars;
    double Value;
    char DesignFileName[MAX_CHAR_SIZE], Variable[MAX_CHAR_SIZE];
    FILE *DesignFile, *OptDesFile;
    
    // Open original des file
    
    snprintf(DesignFileName,sizeof(DesignFileName)*sizeof(char),"%s.des",FileName);
    
    if ( (DesignFile = fopen(DesignFileName, "r")) == NULL ) {
    
       printf("Could not open the original OpenVSP Opt des file! \n");
    
       printf("DesignFileName: %s \n",DesignFileName);fflush(NULL);
    
       exit(1);
    
    }
    
    // Open Opt des file
    
    if ( Iter >= 0 ) {
       
       snprintf(DesignFileName,sizeof(DesignFileName)*sizeof(char),"%s.Opt.%d.des",FileName,Iter);
       
       
       if ( (OptDesFile = fopen(DesignFileName, "w")) == NULL ) {
       
          printf("Could not open the OpenVSP Opt des file for iter: %d! \n",Iter);
       
          printf("DesignFileName: %s \n",DesignFileName);fflush(NULL);
       
          exit(1);
       
       }
       
    }
    
    else {
       
       snprintf(DesignFileName,sizeof(DesignFileName)*sizeof(char),"%s.Final.des",FileName);
       
       if ( (OptDesFile = fopen(DesignFileName, "w")) == NULL ) {
       
          printf("Could not open the OpenVSP Opt des file! \n");
       
          printf("DesignFileName: %s \n",DesignFileName);fflush(NULL);
       
          exit(1);
       
       }
       
    }
       
    // Parse the des file, replace OpenVSP parameters with new values    
    
    fscanf(DesignFile,"%d \n",&NumVars);
    
    if ( NumVars != NumberOfOpenVSP_Parameters_ ) {
       
       printf("Number of design variables does not match OpenVSP des file! \n");
       
       fflush(NULL);exit(1);
       
    }
    
    fprintf(OptDesFile,"%d\n",NumberOfOpenVSP_Parameters_);
    
    fflush(NULL);
    
    for ( i = 1 ; i <= NumberOfOpenVSP_Parameters_ ; i++ ) {

       fscanf(DesignFile,"%s%lf\n",Variable,&Value);

       fprintf(OptDesFile,"%s %20.10e\n",Variable,ParameterValues[i]);
       
    }

    // Parse the des file, replace VSPAERO parameters with new values    

    if ( NumberOfVSPAERO_Parameters_ > 0 ) {
       
       fgets(Variable,200,DesignFile); fprintf(OptDesFile,"%s\n",Variable);
       fgets(Variable,200,DesignFile); fprintf(OptDesFile,"%s\n",Variable);
       fgets(Variable,200,DesignFile); fprintf(OptDesFile,"%s\n",Variable);
           
       for ( i = 1 ; i <= NumberOfVSPAERO_Parameters_ ; i++ ) {
          
          j = i + NumberOfOpenVSP_Parameters_;
          
          fscanf(DesignFile,"%s%lf\n",Variable,&Value);
          
          fprintf(OptDesFile,"%s %20.10e\n",Variable,ParameterValues[j]);
       
       }             
       
    }
                      
    fclose(DesignFile);
    
    fclose(OptDesFile);       
    
}
