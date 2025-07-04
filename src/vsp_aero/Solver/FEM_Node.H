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
#include "utils.H"

#include "START_NAME_SPACE.H"

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
    
    /** Size the list **/
    
    void SizeList(int NumberOfFEMNodes);

    /** Number of nodes in the list **/
    
    int NumberOfFEMNodes(void) { return NumberOfFEMNodes_; };
    
    /** X location of i'th node **/

    double &x(int i) { return x_[i]; };
    
    /** Y location of i'th node **/
    
    double &y(int i) { return y_[i]; };

    /** Z location of i'th node **/

    double &z(int i) { return z_[i]; };
    
    /** X deflection **/

    double &delta_xv(int i) { return delta_xv_[i]; };
    
    /** Y deflection **/
    
    double &delta_yv(int i) { return delta_yv_[i]; };

    /** Z deflection **/

    double &delta_zv(int i) { return delta_zv_[i]; };
    
    /** X rotation **/
    
    double &delta_phix(int i) { return delta_phix_[i]; };
    
    /** Y rotation **/
    
    double &delta_phiy(int i) { return delta_phiy_[i]; };

    /** Z rotation **/

    double &delta_phiz(int i) { return delta_phiz_[i]; };    
    
    /** X given a span wise coordinate value s **/
    
    double x(double s);
    
    /** Z given a span wise coordinate value s **/
    
    double z(double s);
    
    /** delta_xv given a span wise coordinate value s ... hence we are interpolating in the spanwise direction **/
    
    double delta_xv(double s);
    
    /** delta_yv given a span wise coordinate value s ... hence we are interpolating in the spanwise direction **/
    
    double delta_yv(double s);

    /** delta_zv given a span wise coordinate value s ... hence we are interpolating in the spanwise direction **/

    double delta_zv(double s);
    
    /** delta_phix given a span wise coordinate value s ... hence we are interpolating in the spanwise direction **/
    
    double delta_phix(double s);
    
    /** delta_phiy given a span wise coordinate value s ... hence we are interpolating in the spanwise direction **/
    
    double delta_phiy(double s);

    /** delta_phiz given a span wise coordinate value s ... hence we are interpolating in the spanwise direction **/

    double delta_phiz(double s);
    
};

#include "END_NAME_SPACE.H"

#endif
