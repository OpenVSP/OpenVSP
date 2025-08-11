//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//////////////////////////////////////////////////////////////////////

#ifndef OPTIMIZATION_PARAMETERS_H
#define OPTIMIZATION_PARAMETERS_H


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <string.h>
#include "CharSizes.H"
#include "utils.H"

// Definition of the OPTIMIZATION_PARAMETERS class

class OPTIMIZATION_PARAMETERS {

private:

   // OpenVSP parameters
   
   int NumberOfOpenVSP_Parameters_;
   
   char **OpenVSP_ParameterNames_;

   double **dMesh_dOpenVSP_Parameter_;
   
   // VSPAERO parameters
   
   int NumberOfVSPAERO_Parameters_;
   
   char **VSPAERO_ParameterNames_;   

   // Parameter values for both OpenVSP and VSPAERO design variables
   
   int NumberOfParameters_;
   
   char **ParameterFullNamePath_;

   double *ParameterValues_;   
   
   void WriteVSPDesFile_(char *FileName, int Iter, double *ParameterValues);
   
public:

    // Constructor, Destructor, Copy

    OPTIMIZATION_PARAMETERS(void);
   ~OPTIMIZATION_PARAMETERS(void);
    OPTIMIZATION_PARAMETERS(const OPTIMIZATION_PARAMETERS &OptimizationParameters);

   // OpenVSP parameters
   
   int NumberOfOpenVSP_Parameters(void) { return NumberOfOpenVSP_Parameters_; };
   
   char *OpenVSP_ParameterNames(int i) { return OpenVSP_ParameterNames_[i]; };

   char *ParameterFullNamePath(int i) { return ParameterFullNamePath_[i]; };

   double &dMesh_dOpenVSP_Parameter(int i, int j) { return dMesh_dOpenVSP_Parameter_[i][j]; };
   
   // VSPAERO parameters
   
   int NumberOfVSPAERO_Parameters(void) { return NumberOfVSPAERO_Parameters_; };
   
   char *VSPAERO_ParameterNames(int i) { return VSPAERO_ParameterNames_[i]; };

   // Parameter values for both OpenVSP and VSPAERO design variables
   
   int NumberOfParameters(void) { return NumberOfParameters_; };
   
   double &ParameterValues(int i) { return ParameterValues_[i]; };
   
   double *ParameterValues(void) { return ParameterValues_; };   

   void ReadOpenVSPDesFile(char *FileName);

   void WriteFinalVSPDesFile(char *FileName, double *ParameterValues);

   void WriteVSPDesFile(char *FileName, int Iter, double *ParameterValues);

};

#endif
