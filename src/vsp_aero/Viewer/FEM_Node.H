//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//////////////////////////////////////////////////////////////////////

#ifndef FEM_NODE_H
#define FEM_NODE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>

// Definition of the FEM_NODE class

class FEM_NODE {

private:

    // Node based data
    
    int NumberOfFEMNodes_;

    double *x_;
    double *y_;         
    double *z_;
    double *delta_xv_;
    double *delta_yv_;
    double *delta_zv_; 
    double *delta_phix_; 
    double *delta_phiy_; 
    double *delta_phiz_;
    
    double InterpolateFunction_(double *x, double *f, double s);

public:

    // Constructor, Destructor, Copy

    FEM_NODE(void);
   ~FEM_NODE(void);
    FEM_NODE(const FEM_NODE &FEMNode);
    FEM_NODE& operator=(const FEM_NODE &FEMNode);
    
    // Size the list
    
    void SizeList(int NumberOfFEMNodes);

    // Accesss to node data
    
    int NumberOfFEMNodes(void) { return NumberOfFEMNodes_; };

    double &x(int i) { return x_[i]; };
    double &y(int i) { return y_[i]; };
    double &z(int i) { return z_[i]; };

    double &delta_xv(int i) { return delta_xv_[i]; };
    double &delta_yv(int i) { return delta_yv_[i]; };
    double &delta_zv(int i) { return delta_zv_[i]; };
    
    double &delta_phix(int i) { return delta_phix_[i]; };
    double &delta_phiy(int i) { return delta_phiy_[i]; };
    double &delta_phiz(int i) { return delta_phiz_[i]; };    
    
    double x(double s);
    double z(double s);
    
    double delta_xv(double s);
    double delta_yv(double s);
    double delta_zv(double s);
    
    double delta_phix(double s);
    double delta_phiy(double s);
    double delta_phiz(double s);
    
};

#endif
