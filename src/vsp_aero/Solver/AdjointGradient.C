//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//////////////////////////////////////////////////////////////////////

#include "AdjointGradient.H"

#include "START_NAME_SPACE.H"

/*##############################################################################
#                                                                              #
#                              ADJOINT_GRADIENT constructor                    #
#                                                                              #
##############################################################################*/

ADJOINT_GRADIENT::ADJOINT_GRADIENT(void)
{

    NumberOfDerivatives_           = 0;
    NumberOfOptimizationFunctions_ = 0;
    NumberOfTimeSteps_             = 0;
    
         dF_dMesh_ = NULL;
         pF_pMesh_ = NULL;         
    PsiT_pR_pMesh_ = NULL;

}

/*##############################################################################
#                                                                              #
#                                 ADJOINT_GRADIENT Copy                        #
#                                                                              #
##############################################################################*/

ADJOINT_GRADIENT::ADJOINT_GRADIENT(const ADJOINT_GRADIENT &ADJOINT_GRADIENT)
{
   
    printf("ADJOINT_GRADIENT copy not implemented! \n");fflush(NULL);exit(1);

}

/*##############################################################################
#                                                                              #
#                             ADJOINT_GRADIENT SizeList                        #
#                                                                              #
##############################################################################*/

void ADJOINT_GRADIENT::SizeList(int NumberOfDerivatives, int NumberOfOptimizationFunctions, int NumberOfTimeSteps)
{

    int i, j;
    
    NumberOfDerivatives_           = NumberOfDerivatives;
    NumberOfOptimizationFunctions_ = NumberOfOptimizationFunctions;
    NumberOfTimeSteps_             = NumberOfTimeSteps;
    
         dF_dMesh_ = new double**[NumberOfTimeSteps_ + 1];    
         pF_pMesh_ = new double**[NumberOfTimeSteps_ + 1];    
    PsiT_pR_pMesh_ = new double**[NumberOfTimeSteps_ + 1];    
    
    for ( i = 0 ; i <= NumberOfTimeSteps_ ; i++ ) {
       
            dF_dMesh_[i] = new double*[NumberOfOptimizationFunctions_ + 1];
            pF_pMesh_[i] = new double*[NumberOfOptimizationFunctions_ + 1];
       PsiT_pR_pMesh_[i] = new double*[NumberOfOptimizationFunctions_ + 1];
    
       for ( j = 1 ; j <= NumberOfOptimizationFunctions_ ; j++ ) {
       
               dF_dMesh_[i][j] = new double[NumberOfDerivatives + 1];
               pF_pMesh_[i][j] = new double[NumberOfDerivatives + 1];
          PsiT_pR_pMesh_[i][j] = new double[NumberOfDerivatives + 1];
          
          zero_double_array(     dF_dMesh_[i][j], NumberOfDerivatives);
          zero_double_array(     pF_pMesh_[i][j], NumberOfDerivatives);
          zero_double_array(PsiT_pR_pMesh_[i][j], NumberOfDerivatives);
          
       }
    
    }
    
}

/*##############################################################################
#                                                                              #
#                               ADJOINT_GRADIENT destructor                    #
#                                                                              #
##############################################################################*/

ADJOINT_GRADIENT::~ADJOINT_GRADIENT(void)
{

    int i, j;

    if ( NumberOfTimeSteps_ > 0 ) {
           
       for ( i = 0 ; i <= NumberOfTimeSteps_ ; i++ ) {
   
          for ( j = 1 ; j <= NumberOfOptimizationFunctions_ ; j++ ) {
          
             if (      dF_dMesh_[i][j] != NULL ) delete [] dF_dMesh_[i][j];
             if (      pF_pMesh_[i][j] != NULL ) delete [] pF_pMesh_[i][j];
             if ( PsiT_pR_pMesh_[i][j] != NULL ) delete [] PsiT_pR_pMesh_[i][j];
   
          }
   
          if (      dF_dMesh_[i] != NULL ) delete [] dF_dMesh_[i];
          if (      pF_pMesh_[i] != NULL ) delete [] pF_pMesh_[i];
          if ( PsiT_pR_pMesh_[i] != NULL ) delete [] PsiT_pR_pMesh_[i];
   
       }
   
       if (      dF_dMesh_ != NULL ) delete [] dF_dMesh_;
       if (      pF_pMesh_ != NULL ) delete [] pF_pMesh_;
       if ( PsiT_pR_pMesh_ != NULL ) delete [] PsiT_pR_pMesh_;  
       
    }
      
}

#include "END_NAME_SPACE.H"


