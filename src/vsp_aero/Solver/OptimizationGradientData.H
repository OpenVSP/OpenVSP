//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//

#ifndef OPTIMIZATION_GRADIENT_DATA_H
#define OPTIMIZATION_GRADIENT_DATA_H

class GRADIENT_DATA {
   
public:
   
   double *dF_dParameter;
   double CL;
   double CD;
   double CS;
   double CML;
   double CMM;
   double CMN;
   
};

#endif
