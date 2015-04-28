//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//////////////////////////////////////////////////////////////////////

#ifndef VSP_EDGE_H
#define VSP_EDGE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>

// Small class for building edge list

class EDGE_ENTRY {

public:

    int node;
    int tri_1;
    int tri_2;
    int next;

};

// Definition of the VSP_EDGE class

class VSP_EDGE {

private:

    // Edge based data

    int Node1_;
    int Node2_;

    int Tri1_;
    int Tri2_;
    
    int VortexLoop1_;
    int VortexLoop2_;
    
    int VortexEdge_;
    
    int EdgeType_;
    int IsTrailingEdge_;
    int IsLeadingEdge_;
    int IsBoundaryEdge_;
    
    int CourseGridEdge_;
    int FineGridEdge_;
    
    double Forces_[3];
    double Trefftz_Forces_[3];

public:

    // Constructor, Destructor, Copy

    VSP_EDGE(void);
   ~VSP_EDGE(void);
    VSP_EDGE(const VSP_EDGE &VSPEdge);
    VSP_EDGE& operator=(const VSP_EDGE &VSPEdge);

    // Accesss to tri data

    int &Node1(void) { return Node1_; };
    int &Node2(void) { return Node2_; };

    int &Tri1(void) { return Tri1_; };
    int &Tri2(void) { return Tri2_; };
    
    // Access to vortex edge
    
    int &VortexEdge(void) { return VortexEdge_; };
    
    int &VortexLoop1(void) { return VortexLoop1_; };
    int &VortexLoop2(void) { return VortexLoop2_; };
    
    // Access to the forces
    
    double &Fx(void) { return Forces_[0]; };
    double &Fy(void) { return Forces_[1]; };
    double &Fz(void) { return Forces_[2]; };
    
    int &EdgeType(void) { return EdgeType_; };
    int &IsTrailingEdge(void) { return IsTrailingEdge_; };
    int &IsLeadingEdge(void) { return IsLeadingEdge_; };
    int &IsBoundaryEdge(void) { return IsBoundaryEdge_; };
    
    int &CourseGridEdge(void) { return CourseGridEdge_; };
    int &FineGridEdge(void) { return FineGridEdge_; };

};

#endif
