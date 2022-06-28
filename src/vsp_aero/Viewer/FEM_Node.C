//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//////////////////////////////////////////////////////////////////////

#include "FEM_Node.H"

/*##############################################################################
#                                                                              #
#                              FEM_NODE constructor                            #
#                                                                              #
##############################################################################*/

FEM_NODE::FEM_NODE(void)
{

    NumberOfFEMNodes_ = 0;

    x_ = NULL;
    y_ = NULL;         
    z_ = NULL;
    
    delta_xv_ = NULL;
    delta_yv_ = NULL;
    delta_zv_ = NULL; 
    
    delta_phix_ = NULL; 
    delta_phiy_ = NULL; 
    delta_phiz_ = NULL;

}

/*##############################################################################
#                                                                              #
#                                 FEM_NODE Copy                                #
#                                                                              #
##############################################################################*/

FEM_NODE::FEM_NODE(const FEM_NODE &FEMNode)
{

    // Just use operator = code
    
    *this = FEMNode;

}

/*##############################################################################
#                                                                              #
#                             FEM_NODE SizeList                                #
#                                                                              #
##############################################################################*/

void FEM_NODE::SizeList(int NumberOfFEMNodes)
{

    NumberOfFEMNodes_= NumberOfFEMNodes;

    if ( x_          != NULL ) delete [] x_;         
    if ( y_          != NULL ) delete [] y_;       
    if ( z_          != NULL ) delete [] z_;        
              
    if ( delta_xv_   != NULL ) delete [] delta_xv_;  
    if ( delta_yv_   != NULL ) delete [] delta_yv_;  
    if ( delta_zv_   != NULL ) delete [] delta_zv_;   
    
    if ( delta_phix_ != NULL ) delete [] delta_phix_; 
    if ( delta_phiy_ != NULL ) delete [] delta_phiy_; 
    if ( delta_phiz_ != NULL ) delete [] delta_phiz_;
    
    
    x_ = new double[NumberOfFEMNodes_ + 1];
    y_ = new double[NumberOfFEMNodes_ + 1];     
    z_ = new double[NumberOfFEMNodes_ + 1];     
                
    delta_xv_ = new double[NumberOfFEMNodes_ + 1];  
    delta_yv_ = new double[NumberOfFEMNodes_ + 1];  
    delta_zv_ = new double[NumberOfFEMNodes_ + 1];   
    
    delta_phix_ = new double[NumberOfFEMNodes_ + 1]; 
    delta_phiy_ = new double[NumberOfFEMNodes_ + 1]; 
    delta_phiz_ = new double[NumberOfFEMNodes_ + 1];
    
}

/*##############################################################################
#                                                                              #
#                            FEM_NODE operator=                                #
#                                                                              #
##############################################################################*/

FEM_NODE& FEM_NODE::operator=(const FEM_NODE &FEMNode)
{

    int i;
    
    NumberOfFEMNodes_ = FEMNode.NumberOfFEMNodes_;
    
    SizeList(NumberOfFEMNodes_);
    
    for ( i = 1 ; i <= NumberOfFEMNodes_ ; i ++ ) {
       
       x_[i]          = FEMNode.x_[i];                 
       y_[i]          = FEMNode.y_[i];               
       z_[i]          = FEMNode.z_[i];   
                            
       delta_xv_[i]   = FEMNode.delta_xv_[i];    
       delta_yv_[i]   = FEMNode.delta_yv_[i];   
       delta_zv_[i]   = FEMNode.delta_zv_[i];    
                        
       delta_phix_[i] = FEMNode.delta_phix_[i]; 
       delta_phiy_[i] = FEMNode.delta_phiy_[i]; 
       delta_phiz_[i] = FEMNode.delta_phiz_[i];

    }

    return *this;

}

/*##############################################################################
#                                                                              #
#                               FEM_NODE destructor                            #
#                                                                              #
##############################################################################*/

FEM_NODE::~FEM_NODE(void)
{

    NumberOfFEMNodes_= 0;;

    if ( x_          != NULL ) delete [] x_;         
    if ( y_          != NULL ) delete [] y_;         
    if ( z_          != NULL ) delete [] z_;                 
    if ( delta_xv_   != NULL ) delete [] delta_xv_;  
    if ( delta_yv_   != NULL ) delete [] delta_yv_;  
    if ( delta_zv_   != NULL ) delete [] delta_zv_;   
    if ( delta_phix_ != NULL ) delete [] delta_phix_; 
    if ( delta_phiy_ != NULL ) delete [] delta_phiy_; 
    if ( delta_phiz_ != NULL ) delete [] delta_phiz_;

}

/*##############################################################################
#                                                                              #
#                               FEM_NODE x                                     #
#                                                                              #
##############################################################################*/

double FEM_NODE::x(double s) 
{
   
   return InterpolateFunction_(y_, x_, s);
   
}

/*##############################################################################
#                                                                              #
#                               FEM_NODE z                                     #
#                                                                              #
##############################################################################*/

double FEM_NODE::z(double s) 
{
   
   return InterpolateFunction_(y_, z_, s);
   
}

/*##############################################################################
#                                                                              #
#                               FEM_NODE delta_xv                              #
#                                                                              #
##############################################################################*/

double FEM_NODE::delta_xv(double s) 
{
   
   return InterpolateFunction_(y_, delta_xv_, s);
   
}

/*##############################################################################
#                                                                              #
#                               FEM_NODE delta_yv                              #
#                                                                              #
##############################################################################*/

double FEM_NODE::delta_yv(double s) 
{
   
   return InterpolateFunction_(y_, delta_yv_, s);
   
}

/*##############################################################################
#                                                                              #
#                               FEM_NODE delta_zv                              #
#                                                                              #
##############################################################################*/

double FEM_NODE::delta_zv(double s) 
{
   
   return InterpolateFunction_(y_, delta_zv_, s);
   
}

/*##############################################################################
#                                                                              #
#                               FEM_NODE delta_phix                            #
#                                                                              #
##############################################################################*/

double FEM_NODE::delta_phix(double s) 
{
   
   return InterpolateFunction_(y_, delta_phix_, s);
   
}

/*##############################################################################
#                                                                              #
#                               FEM_NODE delta_phiy                            #
#                                                                              #
##############################################################################*/

double FEM_NODE::delta_phiy(double s) 
{
   
   return InterpolateFunction_(y_, delta_phiy_, s);
   
}

/*##############################################################################
#                                                                              #
#                               FEM_NODE delta_phiz                            #
#                                                                              #
##############################################################################*/

double FEM_NODE::delta_phiz(double s) 
{
   
   return InterpolateFunction_(y_, delta_phiz_, s);
   
}

/*##############################################################################
#                                                                              #
#                         FEM_NODE InterpolateFunction_                        #
#                                                                              #
##############################################################################*/

double FEM_NODE::InterpolateFunction_(double *x, double *f, double s) 
{
   
   int i;
   double m;
   
   // S below lower bound
   
   if ( s <= x[1] ) return f[1];
   
   if ( s >= x[NumberOfFEMNodes_] ) return f[NumberOfFEMNodes_];

   // S above upper bound
  
   if ( 0&&s >= x[NumberOfFEMNodes_] ) {
   
      i = NumberOfFEMNodes_ - 1;
      
      m = (f[i+1] - f[i])/(x[i+1] - x[i]);
      
      return f[i] + m * (s - x[i]);

   }
   
   // S within interpolation region
      
   i = 1;
   
   while ( i < NumberOfFEMNodes_ ) {
      
      if ( s >= x[i] && s <= x[i+1] ) {
         
         m = (f[i+1] - f[i])/(x[i+1] - x[i]);
         
         return f[i] + m * (s - x[i]);
         
      }
      
      i++;
      
   }
   
   printf("How did I get here! \n");fflush(NULL);
   exit(1);
   
}   


