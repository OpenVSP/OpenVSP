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

    VSPAERO_DOUBLE *x_;
    VSPAERO_DOUBLE *y_;         
    VSPAERO_DOUBLE *z_;
    VSPAERO_DOUBLE *delta_xv_;
    VSPAERO_DOUBLE *delta_yv_;
    VSPAERO_DOUBLE *delta_zv_; 
    VSPAERO_DOUBLE *delta_phix_; 
    VSPAERO_DOUBLE *delta_phiy_; 
    VSPAERO_DOUBLE *delta_phiz_;
    
    VSPAERO_DOUBLE InterpolateFunction_(VSPAERO_DOUBLE *x, VSPAERO_DOUBLE *f, VSPAERO_DOUBLE s);

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

    VSPAERO_DOUBLE &x(int i) { return x_[i]; };
    
    /** Y location of i'th node **/
    
    VSPAERO_DOUBLE &y(int i) { return y_[i]; };

    /** Z location of i'th node **/

    VSPAERO_DOUBLE &z(int i) { return z_[i]; };
    
    /** X deflection **/

    VSPAERO_DOUBLE &delta_xv(int i) { return delta_xv_[i]; };
    
    /** Y deflection **/
    
    VSPAERO_DOUBLE &delta_yv(int i) { return delta_yv_[i]; };

    /** Z deflection **/

    VSPAERO_DOUBLE &delta_zv(int i) { return delta_zv_[i]; };
    
    /** X rotation **/
    
    VSPAERO_DOUBLE &delta_phix(int i) { return delta_phix_[i]; };
    
    /** Y rotation **/
    
    VSPAERO_DOUBLE &delta_phiy(int i) { return delta_phiy_[i]; };

    /** Z rotation **/

    VSPAERO_DOUBLE &delta_phiz(int i) { return delta_phiz_[i]; };    
    
    /** X given a span wise coordinate value s **/
    
    VSPAERO_DOUBLE x(VSPAERO_DOUBLE s);
    
    /** Z given a span wise coordinate value s **/
    
    VSPAERO_DOUBLE z(VSPAERO_DOUBLE s);
    
    /** delta_xv given a span wise coordinate value s ... hence we are interpolating in the spanwise direction **/
    
    VSPAERO_DOUBLE delta_xv(VSPAERO_DOUBLE s);
    
    /** delta_yv given a span wise coordinate value s ... hence we are interpolating in the spanwise direction **/
    
    VSPAERO_DOUBLE delta_yv(VSPAERO_DOUBLE s);

    /** delta_zv given a span wise coordinate value s ... hence we are interpolating in the spanwise direction **/

    VSPAERO_DOUBLE delta_zv(VSPAERO_DOUBLE s);
    
    /** delta_phix given a span wise coordinate value s ... hence we are interpolating in the spanwise direction **/
    
    VSPAERO_DOUBLE delta_phix(VSPAERO_DOUBLE s);
    
    /** delta_phiy given a span wise coordinate value s ... hence we are interpolating in the spanwise direction **/
    
    VSPAERO_DOUBLE delta_phiy(VSPAERO_DOUBLE s);

    /** delta_phiz given a span wise coordinate value s ... hence we are interpolating in the spanwise direction **/

    VSPAERO_DOUBLE delta_phiz(VSPAERO_DOUBLE s);
    
};

#include "END_NAME_SPACE.H"

#endif
