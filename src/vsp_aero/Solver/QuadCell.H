//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//////////////////////////////////////////////////////////////////////

#ifndef QUAD_CELL_H
#define QUAD_CELL_H

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utils.H"
#include "QuadNode.H"
#include "QuadEdge.H"

#include "START_NAME_SPACE.H"

class QUAD_CELL {
   
private:

    int HasPoint_;
    int SurfaceEdge_;
    int Node_[4];    
    int Child_[4];
    int Edge_[8];
    int Parent_;
    int Level_;
    
    VSPAERO_DOUBLE InsertedPoint_[3];    

public:

    QUAD_CELL(void);
   ~QUAD_CELL(void);
    QUAD_CELL(const QUAD_CELL &QuadCell);
    
    int &HasPoint(void) { return HasPoint_; };
    int &SurfaceEdge(void) { return SurfaceEdge_; };
    int &Node(int i) { return Node_[i]; };
    int &Child(int i) { return Child_[i]; };
    int &Edge(int i) { return Edge_[i]; };
    int &Parent(void) { return Parent_; };
    int &Level(void) { return Level_; };
    int NeighborCell(int i);

    VSPAERO_DOUBLE *InsertedPoint(void) { return InsertedPoint_; };         
    VSPAERO_DOUBLE &InsertedPoint(int i) { return InsertedPoint_[i]; }; 

};

#include "END_NAME_SPACE.H"

#endif
