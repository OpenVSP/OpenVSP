//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//////////////////////////////////////////////////////////////////////

#ifndef OPTIMIZATION_NODE_H
#define OPTIMIZATION_NODE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>

// Definition of the OPT_NODE class

class OPT_NODE {

private:

    // Node based data
    
    int NumberOfOptNodes_;

    double *x_;
    double *y_;         
    double *z_;
    double *dFdx_;
    double *dFdy_;
    double *dFdz_; 
    double *Psi_;

public:

    // Constructor, Destructor, Copy

    OPT_NODE(void);
   ~OPT_NODE(void);
    OPT_NODE(const OPT_NODE &OPTNode);
    OPT_NODE& operator=(const OPT_NODE &OPTNode);
    
    // Size the list
    
    void SizeList(int NumberOfFEMNodes);

    // Accesss to node data
    
    int NumberOfOptNodes(void) { return NumberOfOptNodes_; };

    double &x(int i) { return x_[i]; };
    double &y(int i) { return y_[i]; };
    double &z(int i) { return z_[i]; };

    double &dFdx(int i) { return dFdx_[i]; };
    double &dFdy(int i) { return dFdy_[i]; };
    double &dFdz(int i) { return dFdz_[i]; };
    
    double &Psi(int i) { return Psi_[i]; };
        
};

#endif
